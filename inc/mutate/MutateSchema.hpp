#pragma once
#include "alias.hpp"
#include "tsl/ordered_map.h"
#include "Field.hpp"
#include "utils/json_functions.hpp"
#include "parallel_hashmap/phmap.h"
namespace ok
{
enum class ErrorCode;
namespace smart_actor::connection
{
enum class MutateEventType : uint8_t;
}
namespace mutate_schema
{
enum class FieldType : uint8_t;
struct Relation;
struct RelationQuery
{
  AQLQuery query;
  AQLQuery projection;
  std::string globalDeleteActorKey;
  std::string userDeleteActorKey;
};
using RelationQueryMap = tsl::ordered_map<Collection, RelationQuery>;
struct MutateQueryParts
{
  std::vector<AQLQuery> prefix;
  std::vector<AQLQuery> mergeQueryVarToData;
  RelationQueryMap relationQueries;
};
// using Fields = phmap::parallel_flat_hash_map<ID, Field>; // unordered but fast
using Fields = tsl::ordered_map<ID, Field>;
using FieldOrder = std::vector<ID>;
std::tuple<ErrorCode, Collection, Relation, Fields, FieldOrder> initialize(VPackSlice const &schemaJson, ok::smart_actor::connection::MutateEventType const &eventType) noexcept;
std::tuple<ErrorCode, Fields, FieldOrder> processDataPart(VPackSlice const &schemaJson,
                                                          jsoncons::ojson const &args,
                                                          ok::smart_actor::connection::MutateEventType const &eventType,
                                                          Collection const &collection,
                                                          Relation const &tree,
                                                          MutateQueryParts &queryParts,
                                                          DocumentKey const &memberKey,
                                                          bool ignoreMissingFieldWithValue) noexcept;
inline std::tuple<ErrorCode, Field &> getField(Fields &fields, const std::string &name)
{
  static Field empty;
  if (fields.contains(name)) return {ok::ErrorCode::ERROR_NO_ERROR, const_cast<Fields &>(fields)[name]};
  else
    return {ok::ErrorCode::ERROR_ILLEGAL_OPTION, empty};
}
inline std::tuple<ErrorCode, const Field &> getField(Fields const &fields, const std::string &name) { return getField(const_cast<Fields &>(fields), name); }
inline jsoncons::ojson fieldErrors(Fields const &fields)
{
  jsoncons::ojson errors;
  // todo make nested errors work too
  for (auto &f : fields)
    if (!f.second.error.empty()) errors[f.first] = f.second.error;
  return errors;
}
std::tuple<ErrorCode, jsoncons::ojson> processFormProps(VPackSlice const &schemaJson, Fields const &fields, DocumentKey const &memberKey) noexcept;
std::tuple<ErrorCode, arangodb::velocypack::Builder> makeRelationBindVarsBase(Fields const &fields, WsArguments const &args, DocumentKey const &memberKey) noexcept;
arangodb::velocypack::Builder makeBindVarsDelete() noexcept;
namespace table_actor
{
jsoncons::ojson getFormHeaderData(Fields &fields, FieldOrder const &allIds, jsoncons::ojson const &options, jsoncons::ojson const &valueArray) noexcept;
jsoncons::ojson getFormDefaultObject(Fields const &fields, const FieldOrder &allIds) noexcept;
std::tuple<ErrorCode, AQLQuery> makeFormFieldsObjectProjection(Fields const &fields, FieldOrder const &allIds, Collection const &collection, std::string const &as) noexcept;
}  // namespace table_actor
namespace mutation
{
AQLQuery getDeepQueryFromResult(RelationQueryMap const &relationQueries) noexcept;
AQLQuery getProjectionFromResult(RelationQueryMap const &relationQueries) noexcept;
}  // namespace mutation
namespace make_bind_vars
{
ErrorCode setNonRelationFieldValues(Fields &fields, WsArguments const &formValue, WsArguments const &backendValue, bool ignoreMissingFieldWithValue) noexcept;
std::tuple<ErrorCode, jsoncons::ojson> makeFormObject(Fields &fields, FieldOrder const &allIds) noexcept;
}  // namespace make_bind_vars
}  // namespace mutate_schema
}  // namespace ok
