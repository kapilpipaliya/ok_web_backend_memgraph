#include "actor_system/MainActor.hpp"
#include "utils/BatchArrayMessage.hpp"
#include "actor_system/GlobalActor.hpp"
// #include "actor_system/WsConnectionV8Actor.hpp"
// #include "actor_system/V8StateActor.hpp"
#include <magic_enum.hpp>
#include "actor_system/MutateActors.hpp"
#include "actor_system/TableActor.hpp"
#include "actor_system/IntegrationActors.hpp"
namespace ok::smart_actor
{
namespace supervisor
{
main_actor_int::behavior_type MainActor(MainActorPointer self)
{
  self->set_error_handler(
      [](caf::error &err)
      {
        LOG_DEBUG << "Main Actor Error :";
        LOG_DEBUG << ok::smart_actor::supervisor::getReasonString(err);
      });
  self->set_down_handler(
      [](caf::scheduled_actor *act, caf::down_msg &msg) noexcept
      {
        LOG_DEBUG << "Monitored Actor Error Down Error :" << act->name();
        LOG_DEBUG << ok::smart_actor::supervisor::getReasonString(msg.reason);
      });
  // If self exception error occur: server freeze.
  self->set_exception_handler(
      [](caf::scheduled_actor *, std::exception_ptr &eptr) noexcept -> caf::error
      {
        try
        {
          if (eptr) { std::rethrow_exception(eptr); }
        }
        catch (std::exception const &e)
        {
          LOG_DEBUG << "Main Actor Exception Error : " << e.what();
        }
        return caf::make_error(caf::pec::success);  // self will not resume actor.
      });
  self->set_default_handler(
      [](caf::scheduled_actor *ptr, caf::message &x) noexcept -> caf::skippable_result
      {
        LOG_DEBUG << "unexpected message, I will Quit";
        LOG_DEBUG << "*** unexpected message [id: " << ptr->id() << ", name: " << ptr->name() << "]: " << caf::deep_to_string(x);
        return caf::message{};
      });
  return {[=](spawn_and_monitor_atom) { impl::spawnAndMonitorSuperActor(self); },
          [=](save_old_wsconnptr_atom, drogon::WebSocketConnectionPtr const &wsConnPtr, std::string const &jwtEncoded, std::string const &firstSubDomain)
          { impl::spanAndSaveConnectionActor(self, wsConnPtr, jwtEncoded, firstSubDomain); },
          [=](pass_to_ws_connection_atom, drogon::WebSocketConnectionPtr const &wsConnPtr, std::string &message, drogon::WebSocketMessageType const &type)
          { impl::passToWsControllerActor(self, wsConnPtr, std::move(message), type); },
          [=](conn_exit_old_atom, drogon::WebSocketConnectionPtr const &wsConnPtr) { impl::connectionExit(self, wsConnPtr); },
          // v8
          /*[=](save_v8_wsconnptr_atom, drogon::WebSocketConnectionPtr const &wsConnPtr, arangodb::ConnectionInfo &info) { impl::spanAndSaveV8ConnectionActor(self, wsConnPtr, info); },
          [=](pass_to_ws_v8_connection_atom, drogon::WebSocketConnectionPtr const &wsConnPtr, std::string &message, drogon::WebSocketMessageType const &type)
          { impl::passToWsV8ControllerActor(self, wsConnPtr, std::move(message), type); },
          [=](conn_exit_v8_atom, drogon::WebSocketConnectionPtr const &wsConnPtr) { impl::connectionExitV8(self, wsConnPtr); },*/
          // h20;
          /*[=]( std::shared_ptr<h2o_websocket_conn_t*> conn, uint8_t opcode, std::string) {
           //struct wslay_event_msg msgarg = {arg->opcode, arg->msg, arg->msg_length};
           //wslay_event_queue_msg(conn->ws_ctx, &msgarg);
         },*/
          [=](shutdown_atom) { impl::shutdownNow(self); }};
}
namespace impl
{
table_actor_int<ws_connector_actor_int> newSuperListAct(
    MainActorPointer act_, std::string const &id, table_actor_int<ws_connector_actor_int> &act, std::string const &actor_id, std::string const &db, bool ck, bool permissionCheck)
{
  if (act) { return act; }
  else
  {
    act = act_->spawn(ok::smart_actor::table_actor::TableActor<ws_connector_actor_int>, db, id, ck, permissionCheck);
    act_->monitor(act);
    ok::smart_actor::supervisor::listActors.insert({id, act});
    return act;
  }
}
void spawnAndMonitorSuperActor(MainActorPointer act) noexcept
{
  ok::smart_actor::supervisor::connectedActor = act->spawn(ok::smart_actor::connection::LoginStatusActor);
  act->monitor(ok::smart_actor::supervisor::connectedActor);
  ok::smart_actor::supervisor::sessionCleanActor = act->spawn(ok::smart_actor::connection::SessionCleanActor);
  act->send(ok::smart_actor::supervisor::sessionCleanActor, session_clean_atom_v);  // self will send message when it down
  act->monitor(ok::smart_actor::supervisor::sessionCleanActor);
  ok::smart_actor::supervisor::databaseHealthCheckActor = act->spawn(ok::smart_actor::connection::DBHealthCheckActor);
  act->send(ok::smart_actor::supervisor::databaseHealthCheckActor, db_health_check_atom_v);
  act->monitor(ok::smart_actor::supervisor::databaseHealthCheckActor);
  ok::smart_actor::supervisor::globalActor = act->spawn(GlobalActor);
  // ok::smart_actor::supervisor::v8StateActor = act->spawn(ok::V8StateActor);
  act->monitor(ok::smart_actor::supervisor::globalActor);
  // act->monitor(ok::smart_actor::supervisor::v8StateActor);
  // List:
  newSuperListAct(act, "user", ok::smart_actor::supervisor::userListActor, "user", "user", false, true);
  newSuperListAct(act, "schema", ok::smart_actor::supervisor::schemaListActor, "schema", "global", false, true);
  newSuperListAct(act, "translation", ok::smart_actor::supervisor::translationListActor, "translation", "global", false, true);
  newSuperListAct(act, "session", ok::smart_actor::supervisor::sessionListActor, "session", "user", false, true);
  newSuperListAct(act, "confirm", ok::smart_actor::supervisor::confirmListActor, "confirm", "user", false, true);
  newSuperListAct(act, "color", ok::smart_actor::supervisor::colorListActor, "color", "global", false, true);
  newSuperListAct(act, "permission", ok::smart_actor::supervisor::permissionListActor, "permission", "global", false, true);
  newSuperListAct(act, "role", ok::smart_actor::supervisor::roleListActor, "role", "global", false, true);
  // newSuperListAct(act, "support", ok::smart_actor::supervisor::supportListActor, "support", "user", false, false);
  // newSuperListAct(act, "support_admin", ok::smart_actor::supervisor::supportAdminListActor, "support_admin", "user", false, false);
  newSuperListAct(act, "template", ok::smart_actor::supervisor::templateListActor, "template", "global", false, true);
  newSuperListAct(act, "menu", ok::smart_actor::supervisor::menuListActor, "menu", "global", false, true);
  newSuperListAct(act, "node", ok::smart_actor::supervisor::nodeListActor, "node", "_system", false, false);
  newSuperListAct(act, "edge", ok::smart_actor::supervisor::edgeListActor, "edge", "_system", false, false);
  newSuperListAct(act, "attribute", ok::smart_actor::supervisor::attributeListActor, "attribute", "_system", false, false);
  // Mutate:
  ok::smart_actor::supervisor::emailMutateActor = act->spawn(ok::smart_actor::integrations::email::EmailActor);
  act->monitor(ok::smart_actor::supervisor::emailMutateActor);
  //
  ok::smart_actor::supervisor::registerMutateActor = act->spawn(ok::smart_actor::auth::AuthRegisterActor);
  act->monitor(ok::smart_actor::supervisor::registerMutateActor);
  //
  ok::smart_actor::supervisor::confirmMutateActor = act->spawn(ok::smart_actor::auth::AuthConfirmationActor);
  act->monitor(ok::smart_actor::supervisor::confirmMutateActor);
  //
  ok::smart_actor::supervisor::memberRegisterMutateActor = act->spawn(ok::smart_actor::auth::AuthMemberRegisterActor);
  act->monitor(ok::smart_actor::supervisor::memberRegisterMutateActor);
  //
  ok::smart_actor::supervisor::logoutMutateActor = act->spawn(ok::smart_actor::auth::AuthLogoutActor);
  act->monitor(ok::smart_actor::supervisor::logoutMutateActor);
  //
  using LA = std::vector<table_actor_int<ws_connector_actor_int>>;
  ok::smart_actor::supervisor::userMutateActor = act->spawn(ok::smart_actor::global_collections::MutateUserActor, ok::smart_actor::supervisor::userListActor, "user", "user");
  act->monitor(ok::smart_actor::supervisor::userMutateActor);
  ok::smart_actor::supervisor::mutateActors.insert({"user", ok::smart_actor::supervisor::userMutateActor});
  ok::smart_actor::supervisor::schemaMutateActor = act->spawn(ok::smart_actor::global_collections::GlobalSchemaMutateActor, ok::smart_actor::supervisor::schemaListActor, "global", "schema");
  act->monitor(ok::smart_actor::supervisor::schemaMutateActor);
  ok::smart_actor::supervisor::mutateActors.insert({"schema", ok::smart_actor::supervisor::schemaMutateActor});
  ok::smart_actor::supervisor::translationMutateActor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                                                                   LA{ok::smart_actor::supervisor::translationListActor},
                                                                   "global",
                                                                   "translation",
                                                                   ok::smart_actor::user::emptyPreProcess,
                                                                   ok::smart_actor::user::emptyPreProcess);
  act->monitor(ok::smart_actor::supervisor::translationMutateActor);
  ok::smart_actor::supervisor::mutateActors.insert({"translation", ok::smart_actor::supervisor::translationMutateActor});
  ok::smart_actor::supervisor::sessionMutateActor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                                                               LA{ok::smart_actor::supervisor::sessionListActor},
                                                               "user",
                                                               "session",
                                                               ok::smart_actor::user::emptyPreProcess,
                                                               ok::smart_actor::user::emptyPreProcess);
  act->monitor(ok::smart_actor::supervisor::sessionMutateActor);
  ok::smart_actor::supervisor::mutateActors.insert({"session", ok::smart_actor::supervisor::sessionMutateActor});
  //  ok::smart_actor::supervisor::confirmMutateActor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>, LA{ok::smart_actor::supervisor::confirmListActor}, "user", "confirm",
  //  ok::smart_actor::user::emptyPreProcess); act->monitor(ok::smart_actor::supervisor::confirmMutateActor); ok::smart_actor::supervisor::mutateActors.insert({"confirm",
  //  ok::smart_actor::supervisor::confirmMutateActor});
  ok::smart_actor::supervisor::colorMutateActor = act->spawn(ok::smart_actor::global_collections::MutateGlobalColorActor, ok::smart_actor::supervisor::colorListActor, "global", "color");
  act->monitor(ok::smart_actor::supervisor::colorMutateActor);
  ok::smart_actor::supervisor::mutateActors.insert({"color", ok::smart_actor::supervisor::colorMutateActor});
  ok::smart_actor::supervisor::permissionMutateActor =
      act->spawn(ok::smart_actor::global_collections::MutateGlobalPermissionActor, ok::smart_actor::supervisor::permissionListActor, "global", "permission");
  act->monitor(ok::smart_actor::supervisor::permissionMutateActor);
  ok::smart_actor::supervisor::mutateActors.insert({"permission", ok::smart_actor::supervisor::permissionMutateActor});
  ok::smart_actor::supervisor::roleMutateActor = act->spawn(ok::smart_actor::global_collections::MutateGlobalRoleActor, ok::smart_actor::supervisor::roleListActor, "global", "role");
  act->monitor(ok::smart_actor::supervisor::roleMutateActor);
  ok::smart_actor::supervisor::mutateActors.insert({"role", ok::smart_actor::supervisor::roleMutateActor});
  ok::smart_actor::supervisor::menuMutateActor = act->spawn(ok::smart_actor::global_collections::MutateGlobalMenuActor, ok::smart_actor::supervisor::menuListActor, "global", "menu");
  act->monitor(ok::smart_actor::supervisor::menuMutateActor);
  ok::smart_actor::supervisor::mutateActors.insert({"menu", ok::smart_actor::supervisor::menuMutateActor});
  ok::smart_actor::supervisor::nodeMutateActor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                                                            LA{ok::smart_actor::supervisor::nodeListActor},
                                                            "_system",
                                                            "node",
                                                            ok::smart_actor::user::emptyPreProcess,
                                                            ok::smart_actor::user::emptyPreProcess);
  act->monitor(ok::smart_actor::supervisor::nodeMutateActor);
  ok::smart_actor::supervisor::mutateActors.insert({"node", ok::smart_actor::supervisor::nodeMutateActor});
  ok::smart_actor::supervisor::edgeMutateActor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                                                            LA{ok::smart_actor::supervisor::edgeListActor},
                                                            "_system",
                                                            "edge",
                                                            ok::smart_actor::user::emptyPreProcess,
                                                            ok::smart_actor::user::emptyPreProcess);
  act->monitor(ok::smart_actor::supervisor::edgeMutateActor);
  ok::smart_actor::supervisor::mutateActors.insert({"edge", ok::smart_actor::supervisor::edgeMutateActor});
  ok::smart_actor::supervisor::attributeMutateActor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                                                                 LA{ok::smart_actor::supervisor::attributeListActor},
                                                                 "_system",
                                                                 "attribute",
                                                                 ok::smart_actor::user::emptyPreProcess,
                                                                 ok::smart_actor::user::emptyPreProcess);
  act->monitor(ok::smart_actor::supervisor::attributeMutateActor);
  ok::smart_actor::supervisor::mutateActors.insert({"attribute", ok::smart_actor::supervisor::attributeMutateActor});
  ok::smart_actor::supervisor::templateMutateActor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                                                                LA{ok::smart_actor::supervisor::templateListActor},
                                                                "global",
                                                                "template",
                                                                ok::smart_actor::user::emptyPreProcess,
                                                                ok::smart_actor::user::emptyPreProcess);
  act->monitor(ok::smart_actor::supervisor::templateMutateActor);
  ok::smart_actor::supervisor::mutateActors.insert({"template", ok::smart_actor::supervisor::templateMutateActor});
  /*else if (id == "support")
  {
    if (ok::smart_actor::supervisor::supportMutateActor) { return ok::smart_actor::supervisor::supportMutateActor; }
    auto schemaKey = id;
    auto listActors = std::vector<table_actor_int<ws_connector_actor_int>>{ok::smart_actor::supervisor::(act, "support"), ok::smart_actor::supervisor::(act, "support_admin")};
    ok::smart_actor::supervisor::supportMutateActor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>, listActors, "user", schemaKey, ok::smart_actor::user::emptyPreProcess);
    act->monitor(ok::smart_actor::supervisor::supportMutateActor);
    ok::smart_actor::supervisor::mutateActors.insert({schemaKey, ok::smart_actor::supervisor::supportMutateActor});
    return ok::smart_actor::supervisor::supportMutateActor;
  }
  else if (id == "support_admin")
  {
    if (ok::smart_actor::supervisor::supportAdminMutateActor) { return ok::smart_actor::supervisor::supportAdminMutateActor; }
    auto schemaKey = id;
    auto listActors = std::vector<table_actor_int<ws_connector_actor_int>>{ok::smart_actor::supervisor::(act, "support_admin"), ok::smart_actor::supervisor::(act, "support")};
    ok::smart_actor::supervisor::supportAdminMutateActor = act->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>, listActors, "user", schemaKey,
  ok::smart_actor::user::emptyPreProcess); act->monitor(ok::smart_actor::supervisor::supportAdminMutateActor); ok::smart_actor::supervisor::mutateActors.insert({schemaKey,
  ok::smart_actor::supervisor::supportAdminMutateActor}); return ok::smart_actor::supervisor::supportAdminMutateActor;
  }*/
}
void spanAndSaveConnectionActor(MainActorPointer act, drogon::WebSocketConnectionPtr const &wsConnPtr, std::string const &jwtEncoded, std::string const &firstSubDomain) noexcept
{
  ws_connector_actor_int connectionActor = act->spawn(ok::smart_actor::connection::WsControllerActor);
  act->monitor(connectionActor);  // self will send message when it down
  act->send(connectionActor, wsConnPtr, jwtEncoded, firstSubDomain);
  act->state.oldActorMap.insert({wsConnPtr, std::move(connectionActor)});
  // sendDataToAllSubscribedActor(act);
}
/*void spanAndSaveV8ConnectionActor(MainActorPointer act, drogon::WebSocketConnectionPtr const &wsConnPtr, arangodb::ConnectionInfo &info) noexcept
{
  ws_connector_v8_actor_int connectionActor = act->spawn(ok::smart_actor::connection::WsConnectionV8Actor, wsConnPtr, std::move(info));
  act->monitor(connectionActor);  // self will send message when it down
  act->state.v8ActorMap.insert({wsConnPtr, connectionActor});
  // sendDataToAllSubscribedActor(act);
}*/
void passToWsControllerActor(MainActorPointer act, drogon::WebSocketConnectionPtr const &wsConnPtr, std::string &&message, drogon::WebSocketMessageType const &type) noexcept
{
  if (auto const &it = act->state.oldActorMap.find(wsConnPtr); it == std::end(act->state.oldActorMap))
  {
    LOG_DEBUG << "This should never happen when passing message to the connection.";
    exit(1);
  }
  else
    act->send(it->second, std::move(message), type);
  // act->send(act->state.apiCount, caf::add_atom_v);  // todo pass todo_actor // todo fix this
}
/*void passToWsV8ControllerActor(MainActorPointer act, drogon::WebSocketConnectionPtr const &wsConnPtr, std::string &&message, drogon::WebSocketMessageType const &type) noexcept
{
  if (auto const &it = act->state.v8ActorMap.find(wsConnPtr); it == std::end(act->state.v8ActorMap))
  {
    LOG_DEBUG << "This should never happen when passing message to the connection.";
    exit(1);
  }
  else
    act->send(it->second, std::move(message), type);
  // act->send(act->state.apiCount, caf::add_atom_v);  // todo pass todo_actor // todo fix this
}*/
void connectionExit(MainActorPointer act, drogon::WebSocketConnectionPtr const &wsConnPtr)
{
  if (auto const &it = act->state.oldActorMap.find(wsConnPtr); it == std::end(act->state.oldActorMap)) { LOG_ERROR << "Not Possible"; }
  else
  {
    ws_connector_actor_int connectionActor = it->second;
    {
      act->send<caf::message_priority::high>(connectionActor, conn_exit_atom_v);
      act->demonitor(connectionActor.address());
    }
    {
      // eraseSubscribedActor(act->state, it->second);
      act->state.oldActorMap.erase(it);
      // sendDataToAllSubscribedActor(act);
    }
  }
}
/*void connectionExitV8(MainActorPointer act, drogon::WebSocketConnectionPtr const &wsConnPtr)
{
  if (auto const &it = act->state.v8ActorMap.find(wsConnPtr); it == std::end(act->state.v8ActorMap)) { LOG_ERROR << "Not Possible"; }
  else
  {
    ws_connector_v8_actor_int connectionActor = it->second;
    {
      act->send<caf::message_priority::high>(connectionActor, conn_exit_atom_v);
      act->demonitor(connectionActor.address());
    }
    {
      // eraseSubscribedActor(act->state, it->second);
      act->state.v8ActorMap.erase(it);
      // sendDataToAllSubscribedActor(act);
    }
    act->send(ok::smart_actor::supervisor::v8StateActor, erase_atom_v, wsConnPtr, connectionActor);
  }
}*/
void shutdownNow(MainActorPointer act) noexcept
{
  act->demonitor(ok::smart_actor::supervisor::globalActor.address());
  act->send(ok::smart_actor::supervisor::globalActor, shutdown_atom_v);
  // act->demonitor(ok::smart_actor::supervisor::v8StateActor.address());
  // act->send(ok::smart_actor::supervisor::v8StateActor, caf::close_atom_v);
  act->demonitor(ok::smart_actor::supervisor::connectedActor.address());
  act->send(ok::smart_actor::supervisor::connectedActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::sessionCleanActor.address());
  act->send(ok::smart_actor::supervisor::sessionCleanActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::databaseHealthCheckActor.address());
  act->send(ok::smart_actor::supervisor::databaseHealthCheckActor, conn_exit_atom_v);
  //
  act->demonitor(ok::smart_actor::supervisor::userListActor.address());
  act->send(ok::smart_actor::supervisor::userListActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::schemaListActor.address());
  act->send(ok::smart_actor::supervisor::schemaListActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::translationListActor.address());
  act->send(ok::smart_actor::supervisor::translationListActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::sessionListActor.address());
  act->send(ok::smart_actor::supervisor::sessionListActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::confirmListActor.address());
  act->send(ok::smart_actor::supervisor::confirmListActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::colorListActor.address());
  act->send(ok::smart_actor::supervisor::colorListActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::permissionListActor.address());
  act->send(ok::smart_actor::supervisor::permissionListActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::roleListActor.address());
  act->send(ok::smart_actor::supervisor::roleListActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::templateListActor.address());
  act->send(ok::smart_actor::supervisor::templateListActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::menuListActor.address());
  act->send(ok::smart_actor::supervisor::menuListActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::nodeListActor.address());
  act->send(ok::smart_actor::supervisor::nodeListActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::edgeListActor.address());
  act->send(ok::smart_actor::supervisor::edgeListActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::attributeListActor.address());
  act->send(ok::smart_actor::supervisor::attributeListActor, conn_exit_atom_v);
  //
  act->demonitor(ok::smart_actor::supervisor::emailMutateActor.address());
  act->send(ok::smart_actor::supervisor::emailMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::registerMutateActor.address());
  act->send(ok::smart_actor::supervisor::registerMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::confirmMutateActor.address());
  act->send(ok::smart_actor::supervisor::confirmMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::memberRegisterMutateActor.address());
  act->send(ok::smart_actor::supervisor::memberRegisterMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::logoutMutateActor.address());
  act->send(ok::smart_actor::supervisor::logoutMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::userMutateActor.address());
  act->send(ok::smart_actor::supervisor::userMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::schemaMutateActor.address());
  act->send(ok::smart_actor::supervisor::schemaMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::translationMutateActor.address());
  act->send(ok::smart_actor::supervisor::translationMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::sessionMutateActor.address());
  act->send(ok::smart_actor::supervisor::sessionMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::colorMutateActor.address());
  act->send(ok::smart_actor::supervisor::colorMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::permissionMutateActor.address());
  act->send(ok::smart_actor::supervisor::permissionMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::roleMutateActor.address());
  act->send(ok::smart_actor::supervisor::roleMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::menuMutateActor.address());
  act->send(ok::smart_actor::supervisor::menuMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::nodeMutateActor.address());
  act->send(ok::smart_actor::supervisor::nodeMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::edgeMutateActor.address());
  act->send(ok::smart_actor::supervisor::edgeMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::attributeMutateActor.address());
  act->send(ok::smart_actor::supervisor::attributeMutateActor, conn_exit_atom_v);
  act->demonitor(ok::smart_actor::supervisor::templateMutateActor.address());
  act->send(ok::smart_actor::supervisor::templateMutateActor, conn_exit_atom_v);
  act->unbecome();
  //
  // act->send(a, conn_exit_atom_v);
  // act->demonitor(a.address());
  LOG_DEBUG << "shutdown main actor";
}
}  // namespace impl
}  // namespace supervisor
}  // namespace ok::smart_actor
