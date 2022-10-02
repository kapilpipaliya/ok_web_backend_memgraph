#pragma once
#include "CAF.hpp"
#include "V8Server/V8DealerFeature.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
namespace ok
{
struct singleV8StateEntry
{
  singleV8StateEntry(std::string appName_, std::string databaseName_, std::string actorName_, v8_actor_int v8Actor_)
      : appName(appName_), databaseName(databaseName_), actorName(actorName_), v8Actor(v8Actor_)
  {
  }
  singleV8StateEntry(std::string appName_, std::string databaseName_, std::string actorName_, v8_actor_int v8Actor_, ws_connector_v8_actor_int subscriber_)
      : appName(appName_), databaseName(databaseName_), actorName(actorName_), v8Actor(v8Actor_), subscriber(subscriber_)
  {
  }
  std::string appName;
  std::string databaseName;
  std::string actorName;
  v8_actor_int v8Actor;
  ws_connector_v8_actor_int subscriber;
  friend std::ostream &operator<<(std::ostream &os, const singleV8StateEntry &f)
  {
    os << f.appName << "\t" << f.databaseName << "\t" << f.actorName;
    return os;
  }
};
struct app_name_key : boost::multi_index::composite_key<singleV8StateEntry, BOOST_MULTI_INDEX_MEMBER(singleV8StateEntry, std::string, appName)>
{
};
struct app_name_database_key
    : boost::multi_index::composite_key<singleV8StateEntry, BOOST_MULTI_INDEX_MEMBER(singleV8StateEntry, std::string, appName), BOOST_MULTI_INDEX_MEMBER(singleV8StateEntry, std::string, databaseName)>
{
};
struct name_key : boost::multi_index::composite_key<singleV8StateEntry,
                                                    BOOST_MULTI_INDEX_MEMBER(singleV8StateEntry, std::string, appName),
                                                    BOOST_MULTI_INDEX_MEMBER(singleV8StateEntry, std::string, databaseName),
                                                    BOOST_MULTI_INDEX_MEMBER(singleV8StateEntry, std::string, actorName)>
{
};
struct subscriber_key : boost::multi_index::composite_key<singleV8StateEntry,
                                                          BOOST_MULTI_INDEX_MEMBER(singleV8StateEntry, v8_actor_int, v8Actor),
                                                          BOOST_MULTI_INDEX_MEMBER(singleV8StateEntry, ws_connector_v8_actor_int, subscriber)>
{
};
struct subscribers_key : boost::multi_index::composite_key<singleV8StateEntry, BOOST_MULTI_INDEX_MEMBER(singleV8StateEntry, ws_connector_v8_actor_int, subscriber)>
{
};
typedef boost::multi_index::multi_index_container<singleV8StateEntry,
                                                  boost::multi_index::indexed_by<boost::multi_index::ordered_unique<name_key>,
                                                                                 boost::multi_index::ordered_non_unique<app_name_key>,
                                                                                 boost::multi_index::ordered_non_unique<app_name_database_key>,
                                                                                 boost::multi_index::ordered_non_unique<subscriber_key>,
                                                                                 boost::multi_index::ordered_non_unique<subscribers_key> > >
    v8_state_multi_index;
// typedef nth_index<v8_state_multi_index, 0>::type v8_stat_multi_index_by_name;
// typedef nth_index<v8_state_multi_index, 1>::type v8_stat_multi_index_by_ptr;
struct v8StateActorState
{
  TRI_vocbase_t *vocbase;
  // std::unique_ptr<arangodb::V8ContextGuard> contextGuard;
  // using DatabaseV8Map = std::unordered_map<std::string, v8_actor_int>;
  // std::unordered_map<std::string, DatabaseV8Map> allApps;
  v8_state_multi_index allApps;  // allActors
  static inline const char *name = "v8Actors";
};
v8_actor_map_int::behavior_type V8StateActor(v8_actor_map_int::stateful_pointer<v8StateActorState> self);
}  // namespace ok
