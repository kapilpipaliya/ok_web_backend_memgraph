#pragma once
#include "Mutate.hpp"
// #include "VocBase/voc-types.h"
namespace ok
{
namespace mutate_actor
{
enum class dropPosition : uint8_t
{
  none = 0,
  top,
  center,
  bottom
};
enum class dropPositionIndex : uint8_t
{
  FROM_KEY = 0,
  FROM_REV,
  TO_KEY,
  TO_REV,
  DRAG_POSITION
};
struct PositionData
{
  dropPosition position;
  EdgeFrom fromKey;
  std::string fromRev;
  EdgeTo toKey;
  std::string toRev;
  DocumentKey fromParentKey;
  EdgeFrom fromEdgeKey;
  DocumentKey toParentKey;
  EdgeTo toEdgeKey;
  // DocumentKey targetParentId;
  // DocumentKey fromParentId;
  Edge edge;
  TRI_edge_direction_e direction{TRI_EDGE_OUT};
  bool noInitialize{false};
};
std::pair<ErrorCode, PositionData> preparePositionData(Database const &database, Collection const &collection, ok::mutate_schema::Relation const &tree, jsoncons::ojson const &saveData) noexcept;
std::pair<ErrorCode, std::unordered_set<DocumentKey>> manageRelation(Database const &database, Collection const &collection, Edge const &edge, PositionData const &d) noexcept;
namespace impl
{
std::tuple<std::string, std::string> getParentId(Database const &database,
                                                 Collection const &collection,
                                                 TRI_edge_direction_e direction,
                                                 Edge const &edge,
                                                 const DocumentKey &key) noexcept;  // get targetParentId
std::tuple<ErrorCode, std::vector<DocumentBase>> checkKeysValid(Database const &database, Collection const &collection, const std::vector<DocumentKey> key) noexcept;
ErrorCode removeRelation(Database const &database, Edge const &edge, EdgeFrom const &fromEdgeKey, std::string const &fromRev);
// can also return rev
std::tuple<ErrorCode, DocumentKey> createRelation(Database const &database, Collection const &collection, Edge const &edge, EdgeFrom const &fromKey, DocumentKey const &toParentKey);
ErrorCode modifyRelation(Database const &database,
                         Collection const &collection,
                         Edge const &edge,
                         EdgeFrom const &fromKey,
                         DocumentRev const &fromRev,
                         EdgeFrom const &fromEdgeKey,
                         EdgeTo const &toKey,
                         DocumentRev const &toRev,
                         DocumentKey const &toParentKey);
bool hasParent(Database const &database, Collection const &collection, TRI_edge_direction_e direction, Edge const &edge, const std::string &parent, const std::string &child);
}  // namespace impl
}  // namespace mutate_actor
}  // namespace ok
