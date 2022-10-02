#include "mutate/MutatePosition.hpp"
#include <magic_enum.hpp>
#include "mutate/MutateSchema.hpp"
#include "db/Permission.hpp"
#include "table/TableSchemaExternal.hpp"
#include "utils/json_functions.hpp"
#include <trantor/utils/Logger.h>
#include "db/DatabaseApi.hpp"
#include "utils/ErrorConstants.hpp"
namespace ok
{
namespace mutate_actor
{
std::pair<ErrorCode, PositionData> preparePositionData(Database const &database, Collection const &collection, ok::mutate_schema::Relation const &tree, jsoncons::ojson const &saveData) noexcept
{
  PositionData d;
  if (!tree.edge.empty())
  {
    d.edge = tree.edge;
    d.direction = tree.direction;
  }
  if (auto response_code = magic_enum::enum_cast<dropPosition>(saveData[4].as<int>()); response_code.has_value()) { d.position = response_code.value(); }
  else
  {
    LOG_DEBUG << "MISMATCH: Form Type not supported" + saveData[4].as_string();
    return {ok::ErrorCode::ERROR_ILLEGAL_OPTION, d};
  }
  d.fromKey = saveData[0].as_string();
  d.fromRev = saveData[1].as_string();
  d.toKey = saveData[2].as_string();
  d.toRev = saveData[3].as_string();
  auto [er, docs] = impl::checkKeysValid(database, collection, {d.fromKey, d.toKey});
  if (ok::isEr(er)) { return {er, d}; }
  if (auto find_from = std::find_if(std::begin(docs), std::end(docs), [&](DocumentBase const &doc) { return doc._key == d.fromKey && doc._rev == d.fromRev; }); find_from == std::end(docs))
  {
    LOG_DEBUG << "from Key not found or conflict";
    return {ok::ErrorCode::ERROR_HTTP_FORBIDDEN, d};
  }
  if (auto find_to = std::find_if(std::begin(docs), std::end(docs), [&](DocumentBase const &doc) { return doc._key == d.toKey && doc._rev == d.toRev; }); find_to == std::end(docs))
  {
    LOG_DEBUG << "to Key not found or conflict";
    return {ok::ErrorCode::ERROR_HTTP_FORBIDDEN, d};
  }
  // getDataParentKey
  auto [fromParentKey, fromEdgeKey] = impl::getParentId(database, collection, d.direction, d.edge, d.fromKey);
  auto [toParentKey, toEdgeKey] = impl::getParentId(database, collection, d.direction, d.edge, d.toKey);
  d.fromParentKey = std::move(fromParentKey);
  d.fromEdgeKey = std::move(fromEdgeKey);
  d.toParentKey = std::move(toParentKey);
  d.toEdgeKey = std::move(toEdgeKey);
  return {ok::ErrorCode::ERROR_NO_ERROR, d};
}
std::pair<ErrorCode, std::unordered_set<DocumentKey>> manageRelation(Database const &database, Collection const &collection, Edge const &edge, PositionData const &d) noexcept
{
  std::unordered_set<DocumentKey> changedKeys;
  if (d.position == dropPosition::center)
  {
    // parent key is not allowed to move in its children...
    // toKey's Parent should not be fromKey
    if (auto isChild = impl::hasParent(database, collection, d.direction, d.edge, d.fromKey, d.toKey); isChild)
    {
      LOG_DEBUG << "Cant move Parent into child";
      return {ok::ErrorCode::ERROR_HTTP_FORBIDDEN, changedKeys};
    }
    if (d.toKey.empty())
    {
      LOG_DEBUG << "This should never happen";
      LOG_DEBUG << "To Key Must not be empty";
      return {ok::ErrorCode::ERROR_INTERNAL, changedKeys};
    }
    else if (!d.fromParentKey.empty())
    {
      impl::modifyRelation(database, collection, d.edge, d.fromKey, d.fromRev, d.fromEdgeKey, d.toKey, d.toRev, d.toKey);
      changedKeys.insert(d.fromParentKey);
      changedKeys.insert(d.toKey);
    }
    else if (d.fromParentKey.empty())
    {
      auto [er, edgeKey] = impl::createRelation(database, collection, d.edge, d.fromKey, d.toKey);
      changedKeys.insert(d.toKey);
    }
  }
  else if (d.fromParentKey == d.toParentKey)
  {
    // nothing
  }
  else if (d.toParentKey.empty() && !d.fromParentKey.empty())
  {
    impl::removeRelation(database, edge, d.fromEdgeKey, d.fromRev);
    changedKeys.insert(d.fromParentKey);
  }
  else if (!d.toParentKey.empty() && d.fromParentKey.empty())
  {
    auto [er, edgeKey] = impl::createRelation(database, collection, d.edge, d.fromKey, d.toParentKey);
    changedKeys.insert(d.toParentKey);
  }
  else
  {
    impl::modifyRelation(database, collection, d.edge, d.fromKey, d.fromRev, d.fromEdgeKey, d.toKey, d.toRev, d.toParentKey);
    changedKeys.insert(d.fromParentKey);
    changedKeys.insert(d.toParentKey);
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, changedKeys};
}
namespace impl
{
std::tuple<std::string, std::string> getParentId(Database const &database, Collection const &collection, TRI_edge_direction_e direction, Edge const &edge, const DocumentKey &key) noexcept
{
  std::string key_ = collection + "/" + key;
  auto d = direction == TRI_EDGE_IN ? std::string{"OUTBOUND"} : std::string{"INBOUND"};
  auto query = R"aql(FOR v, e IN 1..1 )aql" + d + R"aql( @key @@edge
  LIMIT 1
  RETURN [v._key, e._key])aql";
  arangodb::velocypack::Builder bindVars;
  {
    VPackObjectBuilder operations(&bindVars);
    bindVars.add("key", arangodb::velocypack::Value(key_));
    bindVars.add("@edge", arangodb::velocypack::Value(edge));
  }
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  if (isEr(erDb))
  {
    if (response)
    {
      auto slice = response->slices().front();
      LOG_FATAL << "Error: " << slice.toJson();
    }
    LOG_FATAL << "Error: " << errno_string(erDb) << "Error updating session: "
              << " | executed query: " << query << " | database is: " << database << " | collection is: " << edge;
  }
  else
  {
    auto slice = response->slices().front();
    if (auto result = slice.get("result"); result.length() == 1) { return {result[0][0].copyString(), result[0][1].copyString()}; }
  }
  return {"", ""};
}
std::tuple<ErrorCode, std::vector<DocumentBase>> checkKeysValid(Database const &database, Collection const &collection, const std::vector<DocumentKey> key) noexcept
{
  auto query = R"aql(FOR t IN @@insCollection
 FILTER t._key in @keys
 return {_key: t._key, _rev: t._rev})aql";
  arangodb::velocypack::Builder bindVars;
  {
    VPackObjectBuilder operations(&bindVars);
    {
      VPackArrayBuilder array(&bindVars, "keys");
      for (auto &k : key) { array->add(arangodb::velocypack::Value(k)); }
    }
    bindVars.add("@insCollection", arangodb::velocypack::Value(collection));
  }
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  if (isEr(erDb))
  {
    if (response)
    {
      auto slice = response->slices().front();
      LOG_FATAL << "Error: " << slice.toJson();
    }
    LOG_FATAL << "Error: " << errno_string(erDb) << "Error checking keys validity: "
              << " | executed query: " << query << " | database is: " << database << " | collection is: " << collection;
  }
  else
  {
    auto slice = response->slices().front();
    if (auto result = slice.get("result"); result.length() == key.size())
    {
      std::string input = result.toString();
      return {ok::ErrorCode::ERROR_NO_ERROR, jsoncons::decode_json<std::vector<DocumentBase>>(input)};
    }
  }
  LOG_DEBUG << "Error: keys are not valid";
  return {ok::ErrorCode::ERROR_BAD_PARAMETER, {}};
}
ErrorCode removeRelation(Database const &database, Edge const &edge, EdgeFrom const &fromEdgeKey, std::string const &fromRev)
{
  // from set below parent
  // make query based on schema later...
  auto query = R"aql(REMOVE {_key: @fromEdgeKey} IN @@edge OPTIONS { ignoreRevs: false })aql";
  arangodb::velocypack::Builder bindVars;
  {
    VPackObjectBuilder operations(&bindVars);
    bindVars.add("@edge", arangodb::velocypack::Value(edge));
    bindVars.add("fromEdgeKey", arangodb::velocypack::Value(fromEdgeKey));
  }
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  if (isEr(erDb)) { return ok::ErrorCode::ERROR_NO_ERROR; }
  else
  {
    auto slice = response->slices().front();
    LOG_DEBUG << "Error removing relation: " << slice.toJson() << " | executed query: " << query << " | database is: " << database << " | edge is: " << edge
              << " | bindVars is: " << bindVars.slice().toString();
  }
  // return "cant remove relation";
  return ok::ErrorCode::ERROR_INTERNAL;
}
std::tuple<ErrorCode, DocumentKey> createRelation(Database const &database, Collection const &collection, Edge const &edge, EdgeFrom const &fromKey, DocumentKey const &toParentKey)
{
  // from set below parent
  // make query based on schema later...
  auto query = R"aql(INSERT {_from: @fromId, _to: @toParentId} INTO @@edge RETURN NEW._key)aql";
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("@edge", arangodb::velocypack::Value(edge));
  bindVars.add("fromId", arangodb::velocypack::Value(collection + "/" + fromKey));
  bindVars.add("toParentId", arangodb::velocypack::Value(collection + "/" + toParentKey));
  bindVars.close();
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  if (isEr(erDb))
  {
    if (response)
    {
      auto slice = response->slices().front();
      LOG_FATAL << "Error: " << slice.toJson();
    }
    LOG_FATAL << "Error: " << errno_string(erDb) << "Error creating relation: "
              << " | executed query: " << query << " | database is: " << database << " | edge is: " << edge << " | bindVars is: " << bindVars.slice().toString();
  }
  else
  {
    auto slice = response->slices().front();
    if (auto result = slice.get("result"); result.length() == 1) { return {ok::ErrorCode::ERROR_NO_ERROR, result[0].copyString()}; }
  }
  LOG_DEBUG << "cant create edge";
  return {ok::ErrorCode::ERROR_INTERNAL, ""};
}
ErrorCode modifyRelation(Database const &database,
                         Collection const &collection,
                         Edge const &edge,
                         EdgeFrom const &fromKey,
                         DocumentRev const &fromRev,
                         EdgeFrom const &fromEdgeKey,
                         EdgeTo const &toKey,
                         DocumentRev const &toRev,
                         DocumentKey const &toParentKey)
{
  // from set below parent
  // make query based on schema later...
  auto query = R"aql(UPDATE {_key: @fromEdgeKey} WITH {_from: @fromId, _to: @toParentId} IN @@edge OPTIONS { ignoreRevs: false })aql";
  arangodb::velocypack::Builder bindVars;
  {
    VPackObjectBuilder operations(&bindVars);
    bindVars.add("@edge", arangodb::velocypack::Value(edge));
    bindVars.add("fromEdgeKey", arangodb::velocypack::Value(fromEdgeKey));
    bindVars.add("fromId", arangodb::velocypack::Value(collection + "/" + fromKey));
    bindVars.add("toParentId", arangodb::velocypack::Value(collection + "/" + toParentKey));
  }
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  if (isEr(erDb))
  {
    if (response)
    {
      auto slice = response->slices().front();
      LOG_FATAL << "Error: " << slice.toJson();
    }
    LOG_FATAL << "Error: " << errno_string(erDb) << "Error updating relation: "
              << " | executed query: " << query << " | database is: " << database << " | edge is: " << edge << " | bindVars is: " << bindVars.toString();
  }
  else
  {
    return ok::ErrorCode::ERROR_NO_ERROR;
  }
  LOG_DEBUG << "cant update relation";
  return ok::ErrorCode::ERROR_INTERNAL;
}
bool hasParent(Database const &database, Collection const &collection, TRI_edge_direction_e direction, Edge const &edge, const std::string &parent, const std::string &child)
{
  // make query based on schema later...
  auto d = direction == TRI_EDGE_IN ? std::string{"OUTBOUND"} : std::string{"INBOUND"};
  auto query = R"aql(FOR t IN 1..9999 )aql" + d + R"aql( @childId @@edge
      FILTER t.type=='organization'
      FILTER t._key == @parentKey
      LIMIT 1
      RETURN true
)aql";
  arangodb::velocypack::Builder bindVars;
  {
    VPackObjectBuilder operations(&bindVars);
    bindVars.add("@edge", arangodb::velocypack::Value(edge));
    bindVars.add("childId", arangodb::velocypack::Value(collection + "/" + child));
    bindVars.add("parentKey", arangodb::velocypack::Value(parent));
  }
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  auto slice = response->slices().front();
  if (isEr(erDb))
  {
    if (response)
    {
      auto slice = response->slices().front();
      LOG_FATAL << "Error: " << slice.toJson();
    }
    LOG_FATAL << "Error: " << errno_string(erDb) << "Error removing relation: "
              << " | executed query: " << query << " | database is: " << database << " | edge is: " << edge << " | bindVars is: " << bindVars.slice().toString();
  }
  else
  {
    auto slice = response->slices().front();
    if (auto result = slice.get("result"); result.length() == 1 && result[0].isBool()) { return result[0].getBool(); }
  }
  return false;
}
}  // namespace impl
}  // namespace mutate_actor
}  // namespace ok
