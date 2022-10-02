#pragma once
#include <drogon/WebSocketController.h>
#include <jsoncons/json.hpp>
#include "alias.hpp"
#include "CAF.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
namespace ok
{
namespace smart_actor::connection
{
using namespace boost::multi_index;
struct singleConnection
{
  singleConnection(drogon::WebSocketConnectionPtr wsConnPtr_, std::string name_, std::string arguments_) : wsConnPtr(wsConnPtr_), name(name_), arguments(arguments_) {}
  drogon::WebSocketConnectionPtr wsConnPtr;
  std::string name;
  std::string arguments;
  friend std::ostream& operator<<(std::ostream& os, const singleConnection& f)
  {
    os << f.name << "\t" << f.arguments;
    return os;
  }
};

struct wsConnPtr_name_key : composite_key<singleConnection, BOOST_MULTI_INDEX_MEMBER(singleConnection, std::string, name), BOOST_MULTI_INDEX_MEMBER(singleConnection, drogon::WebSocketConnectionPtr, wsConnPtr)>
{
};
struct ws_ptr_key : composite_key<singleConnection, BOOST_MULTI_INDEX_MEMBER(singleConnection, drogon::WebSocketConnectionPtr, wsConnPtr)>
{
};
struct name_key : composite_key<singleConnection, BOOST_MULTI_INDEX_MEMBER(singleConnection, std::string, name)>
{
};
typedef multi_index_container<singleConnection, indexed_by<ordered_non_unique<wsConnPtr_name_key>,ordered_non_unique<ws_ptr_key>,ordered_non_unique<name_key> > > v8_stat_multi_index;
typedef nth_index<v8_stat_multi_index, 0>::type v8_stat_multi_index_by_name;
typedef nth_index<v8_stat_multi_index, 1>::type v8_stat_multi_index_by_ptr;
struct V8ActorState
{
  int count{0};
  static inline const char* name = "v8Actor";
  v8_stat_multi_index wsConnectionActorMultiIndex;
  std::string v8State;
};
v8_actor_int::behavior_type v8Actor(v8_actor_int::stateful_pointer<V8ActorState> self);
}  // namespace smart_actor::connection
}  // namespace ok
