#pragma once
#include "MutateSchema.hpp"
namespace ok
{
namespace mutate_schema
{
namespace impl
{
inline bool isTreeChild(Relation const &tree, jsoncons::ojson const &args)
{
  std::string parent;
  if (jsoncons::ObjectMemberIsString(args, "parent")) { parent = args["parent"].as_string(); }
  return !tree.edge.empty() && !parent.empty();
}
std::tuple<ErrorCode, Collection> getCollection(VPackSlice const &schemaJson) noexcept;
void makePrefixPositionQueryInsertTree(VPackSlice const &obj, Field const &f, jsoncons::ojson const &args, Relation const &tree, Collection const &collection, MutateQueryParts &queryParts) noexcept;
void makePrefixPositionQueryInsert(VPackSlice const &obj, Field const &f, MutateQueryParts &queryParts) noexcept;
bool isRelation(Field const &f) noexcept;
bool isEmptyStringOrNullArg(WsArguments const &formData, const int c) noexcept;
bool isSkipDefaultKeys(VPackSlice const &schemaJson) noexcept;
bool isFieldSkip(VPackSlice const &obj, Field const &f, bool isUpdate, DocumentKey const &memberKey) noexcept;
bool isFieldSkipUpdate(VPackSlice const &obj, Field const &f, DocumentKey const &memberKey) noexcept;
void addDefaultKeys(Fields &fields, FieldOrder &allIds, VPackSlice const &schemaJson) noexcept;
void addDefaultKeysInsert(Fields &fields, FieldOrder &allIds, VPackSlice const &schemaJson) noexcept;
namespace process_field
{
std::tuple<ErrorCode, Selector> selector(VPackSlice const &obj) noexcept;
std::tuple<ErrorCode, FieldType> type(VPackSlice const &obj) noexcept;
void visible(VPackSlice const &obj, Field &f) noexcept;
void props(VPackSlice const &obj, Field &f) noexcept;
void default_(VPackSlice const &obj, Field &f) noexcept;
ErrorCode event(VPackSlice const &obj, Field &f) noexcept;
ErrorCode relation(VPackSlice const &obj,
                   Field &f,
                   RelationQueryMap &relationQueries,
                   ok::smart_actor::connection::MutateEventType const &eventType,
                   Collection const &collection,
                   jsoncons::ojson const &args,
                   DocumentKey const &memberKey) noexcept;
}  // namespace process_field
namespace relation_query_generate
{
ErrorCode insert(VPackSlice const &obj, Field const &f, RelationQueryMap &relationQueries) noexcept;
ErrorCode update(VPackSlice const &obj, Field const &f, RelationQueryMap &relationQueries) noexcept;
ErrorCode delete_(VPackSlice const &obj, Field const &f, RelationQueryMap &relationQueries) noexcept;
ErrorCode position(VPackSlice const &obj, Field const &f, RelationQueryMap &relationQueries) noexcept;
ErrorCode batchUpdate(VPackSlice const &obj, Field const &f, RelationQueryMap &relationQueries) noexcept;
ErrorCode batchDelete(VPackSlice const &obj, Field const &f, RelationQueryMap &relationQueries) noexcept;
}  // namespace relation_query_generate
}  // namespace impl
}  // namespace mutate_schema
}  // namespace ok
