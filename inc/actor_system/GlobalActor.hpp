#pragma once
#include "caf/all.hpp"
#include "parallel_hashmap/phmap.h"
#include "alias.hpp"
#include "CAF.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
namespace ok::smart_actor
{
namespace supervisor
{
template <typename T>
struct singleStateEntry
{
  singleStateEntry(std::string appName_, std::string databaseName_, DocumentKey projectKey_, std::string actorName_, T v8Actor_)
      : appName(appName_), databaseName(databaseName_), project(projectKey_), actorName(actorName_), v8Actor(v8Actor_)
  {
  }
  std::string appName;
  std::string databaseName;
  DocumentKey project;
  std::string actorName;
  T v8Actor;
  friend std::ostream &operator<<(std::ostream &os, const singleStateEntry<T> &f)
  {
    os << f.appName << "\t" << f.databaseName << "\t" << f.actorName;
    return os;
  }
};
template <typename T>
struct app_name_key : boost::multi_index::composite_key<singleStateEntry<T>, BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, std::string, appName)>
{
};
template <typename T>
struct app_name_database_key : boost::multi_index::composite_key<singleStateEntry<T>,
                                                                 BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, std::string, appName),
                                                                 BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, std::string, databaseName)>
{
};
template <typename T>
struct app_name_database_project_key : boost::multi_index::composite_key<singleStateEntry<T>,
                                                                         BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, std::string, appName),
                                                                         BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, std::string, databaseName),
                                                                         BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, DocumentKey, project)>
{
};
template <typename T>
struct name_key : boost::multi_index::composite_key<singleStateEntry<T>,
                                                    BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, std::string, appName),
                                                    BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, std::string, databaseName),
                                                    BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, DocumentKey, project),
                                                    BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, std::string, actorName)>
{
};
template <typename T>
struct actor_key : boost::multi_index::composite_key<singleStateEntry<T>,
                                                     BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, std::string, appName),
                                                     BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, std::string, databaseName),
                                                     BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, DocumentKey, project),
                                                     BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, std::string, actorName),
                                                     BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, T, v8Actor)>
{
};
template <typename T>
struct group_by_actor_key
    : boost::multi_index::composite_key<singleStateEntry<T>, BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, std::string, appName), BOOST_MULTI_INDEX_MEMBER(singleStateEntry<T>, std::string, actorName)>
{
};
template <typename T>
using global_state_multi_index = boost::multi_index::multi_index_container<singleStateEntry<T>,
                                                                           boost::multi_index::indexed_by<boost::multi_index::ordered_non_unique<app_name_database_key<T>>,
                                                                                                          boost::multi_index::ordered_non_unique<name_key<T>>,
                                                                                                          boost::multi_index::ordered_non_unique<app_name_key<T>>,
                                                                                                          boost::multi_index::ordered_unique<actor_key<T>>,
                                                                                                          boost::multi_index::ordered_non_unique<group_by_actor_key<T>>,
                                                                                                          boost::multi_index::ordered_non_unique<app_name_database_project_key<T>>>>;
struct globalActorState
{
  static inline constexpr char const *name = "global-actor";
  global_state_multi_index<table_actor_int<ws_connector_actor_int>> listActors;
  global_state_multi_index<base_mutate_actor_int<ws_connector_actor_int>> mutateActors;
  global_state_multi_index<chat_room_actor_int> chatActors;
};
using GlobalActorPointer = global_actor_int::stateful_pointer<globalActorState>;
global_actor_int::behavior_type GlobalActor(GlobalActorPointer self);
table_actor_int<ws_connector_actor_int> getExistingOrSpawnNewListActor(GlobalActorPointer act, Database const &database, std::string const &id) noexcept;
chat_room_actor_int getExistingOrSpawnNewChatActor(GlobalActorPointer act, Database const &databaseName, DocumentKey const &projectKey, DocumentKey const &conversationKey) noexcept;
table_actor_int<ws_connector_actor_int> spawnNewListActor(GlobalActorPointer act, Database const &database, const std::string &id) noexcept;
base_mutate_actor_int<ws_connector_actor_int> getExistingOrSpawnNewMutateActor(GlobalActorPointer act, Database const &database, std::string const &id) noexcept;
base_mutate_actor_int<ws_connector_actor_int> spawnNewMutateActor(GlobalActorPointer act, Database const &database, const std::string &id) noexcept;
void sendToListActor(GlobalActorPointer act, Database const &database, std::string const &userListActorKey, std::unordered_set<DocumentKey> const &filterKey) noexcept;
void sendDeleteToListActor(GlobalActorPointer act, Database const &database, std::string const &userListActorKey, std::unordered_set<DocumentKey> const &deleteKeys) noexcept;
void sendExitToAllActors(GlobalActorPointer self);
void shutdownNow(GlobalActorPointer act) noexcept;
}  // namespace supervisor
namespace connection
{
struct singleLoggedInActorStateEntry
{
  singleLoggedInActorStateEntry(std::string appName, std::string databaseName, DocumentKey sessionKey, ws_connector_actor_int connectionActor)
      : appName(appName), databaseName(databaseName), sessionKey(sessionKey), connectionActor(std::move(connectionActor))
  {
  }
  std::string appName;
  std::string databaseName;
  DocumentKey sessionKey;
  ws_connector_actor_int connectionActor;
  friend std::ostream &operator<<(std::ostream &os, const singleLoggedInActorStateEntry &f)
  {
    os << f.appName << "\t" << f.databaseName << "\t" << f.sessionKey;
    return os;
  }
};
struct app_name_key : boost::multi_index::composite_key<singleLoggedInActorStateEntry, BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, std::string, appName)>
{
};
struct app_name_database_key : boost::multi_index::composite_key<singleLoggedInActorStateEntry,
                                                                 BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, std::string, appName),
                                                                 BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, std::string, databaseName)>
{
};
struct name_key : boost::multi_index::composite_key<singleLoggedInActorStateEntry,
                                                    BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, std::string, appName),
                                                    BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, std::string, databaseName),
                                                    BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, DocumentKey, sessionKey)>
{
};
struct session_key : boost::multi_index::composite_key<singleLoggedInActorStateEntry,
                                                       BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, std::string, appName),
                                                       BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, std::string, databaseName),
                                                       BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, DocumentKey, sessionKey),
                                                       BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, ws_connector_actor_int, connectionActor)>
{
};
struct subscriber_key : boost::multi_index::composite_key<singleLoggedInActorStateEntry,
                                                          BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, std::string, appName),
                                                          BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, std::string, databaseName),
                                                          BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, ws_connector_actor_int, connectionActor)>
{
};
struct subscribers_key : boost::multi_index::composite_key<singleLoggedInActorStateEntry,
                                                           BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, std::string, appName),
                                                           BOOST_MULTI_INDEX_MEMBER(singleLoggedInActorStateEntry, ws_connector_actor_int, connectionActor)>
{
};
typedef boost::multi_index::multi_index_container<singleLoggedInActorStateEntry,
                                                  boost::multi_index::indexed_by<boost::multi_index::ordered_non_unique<name_key>,
                                                                                 boost::multi_index::ordered_non_unique<app_name_key>,
                                                                                 boost::multi_index::ordered_non_unique<app_name_database_key>,
                                                                                 boost::multi_index::ordered_non_unique<session_key>,
                                                                                 boost::multi_index::ordered_non_unique<subscriber_key>,
                                                                                 boost::multi_index::ordered_non_unique<subscribers_key>>>
    logged_in_state_multi_index;
struct login_actor_state
{
  static inline constexpr char const *name = "logged-in-status-actor";
  logged_in_state_multi_index loginActorsPerDb;
};
using LoginActorPointer = login_actor_int::stateful_pointer<login_actor_state>;
login_actor_int::behavior_type LoginStatusActor(LoginActorPointer self);
void addNewSubscriber(logged_in_state_multi_index &loginActorsPerDb, ok::smart_actor::connection::Session const &session, ws_connector_actor_int connectionActor);
void removeNewSubscriber(logged_in_state_multi_index &loginActorsPerDb, Database const &database, ws_connector_actor_int connectionActor);
void sendMsgToAllSubscribers(LoginActorPointer self, login_actor_state &state, Database const &database, jsoncons::ojson const &msg);
void sendMsgToSameBrowserTab(LoginActorPointer self, login_actor_state &state, Database const &database, DocumentKey const &sessionKey, jsoncons::ojson const &msg);
}  // namespace connection
}  // namespace ok::smart_actor
