#include "actor_system/GlobalActor.hpp"
#include <magic_enum.hpp>
#include "actor_system/WsConnectionActor.hpp"
#include "actor_system/MutateActors.hpp"
#include "actor_system/ChatRoomActor.hpp"
#include "actor_system/TableActor.hpp"
namespace ok::smart_actor
{
namespace supervisor
{
void sendSchemaKeyToAllActors(GlobalActorPointer self, std::unordered_set<DocumentKey> const &schemaKeys)
{
  for (auto &s : schemaKeys)
  {
    auto it = self->state.listActors.get<4>().equal_range(std::make_tuple("appName", s));
    for (auto Itr = it.first; Itr != it.second; ++Itr) { self->send(Itr->v8Actor, schema_changed_atom_v); }
    auto it2 = self->state.mutateActors.get<4>().equal_range(std::make_tuple("appName", s));
    for (auto Itr = it2.first; Itr != it2.second; ++Itr) { self->send(Itr->v8Actor, schema_changed_atom_v); }
    if (s == "participant" || s == "message" || s == "conversation")
    {
      auto it3 = self->state.chatActors.get<2>().equal_range(std::make_tuple("appName"));
      for (auto Itr = it3.first; Itr != it3.second; ++Itr) { self->send(Itr->v8Actor, schema_changed_atom_v, s); }
    }
  }
}
global_actor_int::behavior_type GlobalActor(GlobalActorPointer self)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("User Actor"));
  self->set_down_handler(
      [](caf::scheduled_actor *act, caf::down_msg &msg)
      {
        LOG_DEBUG << "Monitored Actor by Global Actor Error Down Error :" << ok::smart_actor::supervisor::getReasonString(msg.reason) << " name: " << act->name();
        LOG_DEBUG << "actor[" << caf::deep_to_string(msg.source) << "] down_msg: " << caf::deep_to_string(msg.reason);
      });
  return {[=](global_list_atom, Database const &database, std::string const &id) { return getExistingOrSpawnNewListActor(self, database, id); },
          [=](user_mutate_atom, Database const &database, std::string const &id) { return caf::make_result(getExistingOrSpawnNewMutateActor(self, database, id)); },
          [=](schema_changed_atom, std::unordered_set<DocumentKey> const &schemaKeys) { sendSchemaKeyToAllActors(self, schemaKeys); },
          [=](table_dispatch_atom, Database const &database, std::string const &userListActorKey, std::unordered_set<DocumentKey> const &changedKeys)
          { sendToListActor(self, std::move(database), userListActorKey, changedKeys); },
          [=](table_erase_atom, Database const &database, std::string const &userListActorKey, std::unordered_set<DocumentKey> const &changedKeys)
          { sendDeleteToListActor(self, std::move(database), userListActorKey, changedKeys); },
          // conversation actor get:
          [=](get_chat_room_atom, Database const &database, DocumentKey projectKey, DocumentKey conversationKey)
          { return getExistingOrSpawnNewChatActor(self, database, projectKey, conversationKey); },
          [=](shutdown_atom) { shutdownNow(self); }};
}
table_actor_int<ws_connector_actor_int> getExistingOrSpawnNewListActor(GlobalActorPointer act, Database const &databaseName, std::string const &id) noexcept
{
  if (auto it = act->state.listActors.get<1>().find(std::make_tuple("appName", databaseName, "", id)); it != std::end(act->state.listActors.get<1>())) { return it->v8Actor; }
  else
  {
    auto v8Actor = spawnNewListActor(act, databaseName, id);
    auto [it2, isOk] = act->state.listActors.insert(singleStateEntry<table_actor_int<ws_connector_actor_int>>{"appName", databaseName, "", id, v8Actor});
    if (isOk) act->monitor(it2->v8Actor);
    else
      LOG_DEBUG << "-----Should not happen-------";
    return v8Actor;
  }
}
chat_room_actor_int getExistingOrSpawnNewChatActor(GlobalActorPointer act, Database const &databaseName, DocumentKey const &projectKey, DocumentKey const &conversationKey) noexcept
{
  if (auto it = act->state.chatActors.get<1>().find(std::make_tuple("appName", databaseName, projectKey, conversationKey)); it != std::end(act->state.chatActors.get<1>())) { return it->v8Actor; }
  else
  {
    auto chat_actor = act->spawn(ok::smart_actor::chat_room_actor::ChatRoomActor, databaseName, projectKey, conversationKey);
    auto [it2, isOk] = act->state.chatActors.insert(singleStateEntry<chat_room_actor_int>{"appName", databaseName, projectKey, conversationKey, chat_actor});
    if (isOk) act->monitor(it2->v8Actor);
    else
      LOG_DEBUG << "-----Should not happen-------";
    return chat_actor;
  }
}
void sendExitToAllActors(GlobalActorPointer self)
{
  auto &l = self->state.listActors.get<3>();
  for (auto it = l.begin(); it != l.end(); ++it)
  {
    self->send(it->v8Actor, conn_exit_atom_v);
    self->demonitor(it->v8Actor.address());
    anon_send_exit(it->v8Actor, caf::exit_reason::user_shutdown);
  }
  auto &m = self->state.mutateActors.get<3>();
  for (auto it = m.begin(); it != m.end(); ++it)
  {
    self->send(it->v8Actor, conn_exit_atom_v);
    self->demonitor(it->v8Actor.address());
    anon_send_exit(it->v8Actor, caf::exit_reason::user_shutdown);
  }
  auto &c = self->state.chatActors.get<3>();
  for (auto it = c.begin(); it != c.end(); ++it)
  {
    self->send(it->v8Actor, conn_exit_atom_v);
    self->demonitor(it->v8Actor.address());
    anon_send_exit(it->v8Actor, caf::exit_reason::user_shutdown);
  }
  self->unbecome();
}
void shutdownNow(GlobalActorPointer act) noexcept { sendExitToAllActors(act); }
void sendToListActor(GlobalActorPointer act, Database const &database, std::string const &userListActorKey, std::unordered_set<DocumentKey> const &filterKey) noexcept
{
  auto list_actor = getExistingOrSpawnNewListActor(act, database, userListActorKey);
  act->send(list_actor, table_dispatch_atom_v, filterKey);
}
void sendDeleteToListActor(GlobalActorPointer act, Database const &database, std::string const &userListActorKey, const std::unordered_set<DocumentKey> &deleteKeys) noexcept
{
  auto list_actor = getExistingOrSpawnNewListActor(act, database, userListActorKey);
  act->send(list_actor, erase_atom_v, deleteKeys);
}
// Mutate:
base_mutate_actor_int<ws_connector_actor_int> getExistingOrSpawnNewMutateActor(GlobalActorPointer act, Database const &databaseName, const std::string &id) noexcept
{
  if (auto it = act->state.mutateActors.get<1>().find(std::make_tuple("appName", databaseName, "", id)); it != std::end(act->state.mutateActors.get<1>())) { return it->v8Actor; }
  else
  {
    auto v8Actor = spawnNewMutateActor(act, databaseName, id);
    act->state.mutateActors.insert(singleStateEntry<base_mutate_actor_int<ws_connector_actor_int>>("appName", databaseName, "", id, v8Actor));
    return v8Actor;
  }
}
using LA = std::vector<table_actor_int<ws_connector_actor_int>>;
base_mutate_actor_int<ws_connector_actor_int> spawnNewMutateActor(GlobalActorPointer act, Database const &database, std::string const &id) noexcept
{
  if (id == "organization")
  {
    auto listActors = LA{getExistingOrSpawnNewListActor(act, database, "organization")};
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>, listActors, database, id, ok::smart_actor::user::emptyPreProcess, ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "project")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "project")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "group")
  {
    auto listActors = LA{
        getExistingOrSpawnNewListActor(act, database, "group"),
        getExistingOrSpawnNewListActor(act, database, "organization_group"),  // I think this not needed
        getExistingOrSpawnNewListActor(act, database, "project_group"),       // I think this not needed
    };
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>, listActors, database, id, ok::smart_actor::user::emptyPreProcess, ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "role")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "role")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "organization_group")
  {
    auto listActors = LA{
        getExistingOrSpawnNewListActor(act, database, "organization_group"),
        getExistingOrSpawnNewListActor(act, database, "group"),
    };
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>, listActors, database, id, ok::smart_actor::user::emptyPreProcess, ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "organization_role")
  {
    auto listActors = LA{
        getExistingOrSpawnNewListActor(act, database, "organization_role"),
        getExistingOrSpawnNewListActor(act, database, "role"),
    };
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>, listActors, database, id, ok::smart_actor::user::emptyPreProcess, ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "project_group")
  {
    auto listActors = LA{
        getExistingOrSpawnNewListActor(act, database, "project_group"),
        getExistingOrSpawnNewListActor(act, database, "group"),
    };
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>, listActors, database, id, ok::smart_actor::user::emptyPreProcess, ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "project_role")
  {
    auto listActors = LA{
        getExistingOrSpawnNewListActor(act, database, "project_role"),
        getExistingOrSpawnNewListActor(act, database, "role"),
    };
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "project_role")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "member")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "member")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "permission")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "permission")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "color")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "color")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "type")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "type")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "priority")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "priority")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "status")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "status")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "activity")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "activity")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "doc_category")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "doc_category")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "announcement")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "announcement")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "news")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "news")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "wiki")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "wiki")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "work_package")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "work_package")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "forum")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "forum")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "post")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "post")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "setting")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "setting")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "menu")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "menu")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "preferences")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "preferences")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "conversation")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "conversation")},
                            database,
                            id,
                            ok::smart_actor::chat_room_actor::conversationDeletePreProcess,
                            ok::smart_actor::chat_room_actor::conversationDeletePostProcess);
    act->monitor(actor);
    return actor;
  }
  /*else if (id == "message")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "message")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }*/
  else if (id == "participant")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "participant")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "alias")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "alias")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else if (id == "chat_department")
  {
    auto actor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                            LA{getExistingOrSpawnNewListActor(act, database, "chat_department")},
                            database,
                            id,
                            ok::smart_actor::user::emptyPreProcess,
                            ok::smart_actor::user::emptyPreProcess);
    act->monitor(actor);
    return actor;
  }
  else
  {
    LOG_DEBUG << "key must exist in switch when finding new Mutate actor";
    assert(false);
  }
}
#define newActorForKeyMacro(actor_id, is_form_is_c_key, permissionCheck)                                                                              \
  }                                                                                                                                                   \
  else if (id == (actor_id))                                                                                                                          \
  {                                                                                                                                                   \
    auto actor = act->spawn(ok::smart_actor::table_actor::TableActor<ws_connector_actor_int>, database, actor_id, is_form_is_c_key, permissionCheck); \
    return actor;
table_actor_int<ws_connector_actor_int> spawnNewListActor(GlobalActorPointer act, Database const &database, std::string const &id) noexcept
{
  {
    // clang-format off
    if(id == "organization"){
        auto actor = act->spawn(ok::smart_actor::table_actor::TableActor<ws_connector_actor_int>, database, "organization", false, true);
        return actor;
    newActorForKeyMacro("project", false,true)
    newActorForKeyMacro("group", false,true)
    newActorForKeyMacro("role", false,true)
    newActorForKeyMacro("organization_group", false,true)
    newActorForKeyMacro("organization_role", false,true)
    newActorForKeyMacro("project_group", false,true)
    newActorForKeyMacro("project_role", false,true)
    newActorForKeyMacro("member", false,true)
    newActorForKeyMacro("permission", false,true)
    newActorForKeyMacro("color", false,true)
    newActorForKeyMacro("type", false,true)
    newActorForKeyMacro("priority", false,true)
    newActorForKeyMacro("status", false,true)
    newActorForKeyMacro("activity", false,true)
    newActorForKeyMacro("doc_category", false,true)
    newActorForKeyMacro("announcement", false,true)
    newActorForKeyMacro("news", false,true)
    newActorForKeyMacro("wiki", false,true)
    newActorForKeyMacro("work_package", false,true)
    newActorForKeyMacro("forum", false,true)
    newActorForKeyMacro("post", false,true)
    newActorForKeyMacro("setting", true,true)
    newActorForKeyMacro("menu", false,true)
    newActorForKeyMacro("preferences", false,true)
    newActorForKeyMacro("conversation", false,true)
    //newActorForKeyMacro("message", false,true)
    newActorForKeyMacro("participant", false,true)
    newActorForKeyMacro("alias", false,true)
    newActorForKeyMacro("chat_department", false,true)
    }else{
        LOG_DEBUG << "key must exist: " << id;
      return {};
    }
    // clang-format on
  }
}
}  // namespace supervisor
namespace connection
{
login_actor_int::behavior_type LoginStatusActor(LoginActorPointer self)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("Mutate Connect Actor"));
  return {
      [=](caf::add_atom, ok::smart_actor::connection::Session const &session, ws_connector_actor_int connectionActor)
      { addNewSubscriber(self->state.loginActorsPerDb, session, std::move(connectionActor)); },
      [=](remove_atom, ok::smart_actor::connection::Session const &session, ws_connector_actor_int connectionActor)
      { removeNewSubscriber(self->state.loginActorsPerDb, session.database, connectionActor); },
      [=](send_to_one_database_actors_atom, Database const &database, jsoncons::ojson const &msg) { sendMsgToAllSubscribers(self, self->state, database, msg); },
      [=](send_to_same_browser_tab_atom, Database const &database, DocumentKey const &sessionKey, jsoncons::ojson const &msg)
      { sendMsgToSameBrowserTab(self, self->state, database, sessionKey, msg); },
      CONN_EXIT
      // Todo : Make Routes to subscribe/unsubscribe.
      // Todo : Make same like tableActor.
  };
}
void addNewSubscriber(logged_in_state_multi_index &loginActorsPerDb, ok::smart_actor::connection::Session const &session, ws_connector_actor_int connectionActor)
{
  if (auto it = loginActorsPerDb.get<4>().find(std::make_tuple("appName", session.database, connectionActor)); it != loginActorsPerDb.get<4>().end())
  {
    // it->loginType = "guest";
  }
  else
  {
    loginActorsPerDb.insert(singleLoggedInActorStateEntry("appName", session.database, session.sessionKey, std::move(connectionActor)));
  }
}
void removeNewSubscriber(logged_in_state_multi_index &loginActorsPerDb, Database const &database, ws_connector_actor_int connectionActor)
{
  auto it = loginActorsPerDb.get<4>().equal_range(std::make_tuple("appName", database, connectionActor));
  loginActorsPerDb.get<4>().erase(it.first, it.second);
}
void sendMsgToAllSubscribers(LoginActorPointer self, login_actor_state &state, Database const &database, jsoncons::ojson const &msg)
{
  auto it = state.loginActorsPerDb.get<2>().equal_range(std::make_tuple("appName", database));
  for (auto Itr = it.first; Itr != it.second; ++Itr) { self->send(Itr->connectionActor, caf::forward_atom_v, msg); }
}
void sendMsgToSameBrowserTab(LoginActorPointer self, login_actor_state &state, Database const &database, DocumentKey const &sessionKey, jsoncons::ojson const &msg)
{
  auto it = state.loginActorsPerDb.get<0>().equal_range(std::make_tuple("appName", database, sessionKey));
  for (auto Itr = it.first; Itr != it.second; ++Itr) { self->send(Itr->connectionActor, caf::forward_atom_v, msg); }
}
}  // namespace connection
}  // namespace ok::smart_actor
