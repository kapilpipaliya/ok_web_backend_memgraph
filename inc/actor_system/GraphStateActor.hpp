#pragma once
#include <drogon/WebSocketController.h>
#include <jsoncons/json.hpp>
#include "alias.hpp"
#include "CAF.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/property_map/property_map.hpp>
namespace ok::smart_actor
{
namespace supervisor
{
struct ArangodbEdgeProperties
{
  ArangodbEdgeProperties(const std::string& n) : name(n) {}
  std::string name;
  std::string _from;
  std::string _to;
};
struct ArangodbVertexProperties
{
  std::string name;
};
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, ArangodbVertexProperties, ArangodbEdgeProperties> Graph;
struct GraphState
{
  static inline constexpr char const *name = "graph-state-actor";
  std::set<Database, Graph> graphs;
};
graph_state_actor_int::behavior_type GraphStateActor(graph_state_actor_int::stateful_pointer<GraphState> self);
namespace impl
{
}
}  // namespace supervisor
}  // namespace ok::smart_actor
