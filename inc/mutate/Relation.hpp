#pragma once
// #include "VocBase/voc-types.h"  // TRI_edge_direction_e
#include "alias.hpp"



namespace ok
{
/// @brief edge direction
enum TRI_edge_direction_e {
  TRI_EDGE_ANY = 0,  // can only be used for searching
  TRI_EDGE_IN = 1,
  TRI_EDGE_OUT = 2
};
namespace mutate_schema
{
enum class FieldType : uint8_t;
struct Relation
{
  TRI_edge_direction_e direction{TRI_EDGE_OUT};
  Edge edge;
  EdgeFrom from;
  EdgeTo to;
  std::string globalKey;
  std::string userKey;
  jsoncons::ojson getEdgeOppositeVertex;
  bool information{false};
  bool operator==(Relation const &rhs) const
  {
    return direction == rhs.direction && edge == rhs.edge && from == rhs.from && to == rhs.to && edge == rhs.edge && globalKey == rhs.globalKey && userKey == rhs.userKey &&
           information == rhs.information;
  }
};
inline std::string to_string(Relation const &r) noexcept
{
  std::string ret;
  if (r.direction == TRI_EDGE_ANY) ret += "direction: Any";
  else if (r.direction == TRI_EDGE_IN)
    ret += "direction: In";
  else if (r.direction == TRI_EDGE_OUT)
    ret += "direction: Out";
  if (!r.edge.empty()) ret += " Edge: " + r.edge;
  if (!r.from.empty()) ret += " From: " + r.from;
  if (!r.to.empty()) ret += " To: " + r.to;
  ret += "getEdgeOppositeVertex: " + r.getEdgeOppositeVertex.as_string() + "\n";
  return ret;
}
inline std::ostream &operator<<(std::ostream &out, Relation const &relation)
{
  out << to_string(relation);
  return out;
}
}  // namespace mutate_schema
}  // namespace ok
