#pragma once
#include <jsoncons/json.hpp>
#include "alias.hpp"
#include <unordered_set>
#include "mutate/MutateSchema.hpp"
#include "mutate/Field.hpp"
#include "db/fuerteclient.hpp"
#include "table/TableSchema.hpp"
namespace ok
{
enum class ErrorCode;
namespace smart_actor::connection
{
struct Session;
}  // namespace smart_actor::connection
namespace table_actor
{
//!
//\todo: optional:
// 1.on Update only send data, but not header
// 2.I think it send unauthorized to subscribed user test it.
// 3.Send pdf/csv, make header+data report and send
// https://ourcodeworld.com/articles/read/189/how-to-create-a-file-and-generate-a-download-with-javascript-in-the-browser-without-a-server
// 4.Make table config save working.
bool isDefaultFormValue(WsArguments const &args) noexcept;
std::tuple<ErrorCode, table_schema::Columns, mutate_schema::Fields, AQLQuery, jsoncons::ojson> getData(WsArguments const &args,
                                                                                                       Database const &database,
                                                                                                       DocumentKey const &memberKey,
                                                                                                       DocumentKey const &projectKey,
                                                                                                       SchemaKey const &schemaKey,
                                                                                                       VPackSlice const &columnSchemaJson,
                                                                                                       bool is_form,
                                                                                                       bool isMutateSchemaKeyIsCollectionKey,
                                                                                                       AQLQuery &changeQuery,
                                                                                                       std::unordered_set<DocumentKey> &savedKeys,
                                                                                                       std::unordered_set<DocumentKey> const &filterKey,
                                                                                                       bool sendEmpty,
                                                                                                       bool sendHeader,
                                                                                                       bool isPermissionCheck) noexcept;
std::tuple<ErrorCode, mutate_schema::Fields, mutate_schema::FieldOrder, jsoncons::ojson> fetchFormDefaultInitialize(
    Database const &database, DocumentKey const &memberKey, DocumentKey const &projectKey, SchemaKey const &schemaKey, jsoncons::ojson const &args) noexcept;
namespace impl
{
ErrorCode checkPermission(Database const &database, DocumentKey const &memberKey, DocumentKey const &projectKey, SchemaKey const &schemaKey, const std::string &property) noexcept;
ErrorCode checkPermission(
    Database const &database, DocumentKey const &memberKey, DocumentKey const &projectKey, SchemaKey const &schemaKey, bool is_form, bool isMutateSchemaKeyIsCollectionKey) noexcept;
std::tuple<ErrorCode, Collection, mutate_schema::Relation, mutate_schema::Fields, mutate_schema::FieldOrder, mutate_schema::MutateQueryParts, jsoncons::ojson> loadFormSchema(
    const std::string &schemaKey, jsoncons::ojson const &args, DocumentKey const &memberKey) noexcept;
jsoncons::ojson processNormalResult(VPackSlice const &slice, ok::table_schema::Pagination const &pagination);
jsoncons::ojson processChangedResult(VPackSlice const &slice, std::unordered_set<DocumentKey> &savedKeys, std::unordered_set<DocumentKey> const &filterKey);
ErrorCode saveKeys(jsoncons::ojson &jsonResult, std::unordered_set<DocumentKey> &savedKeys) noexcept;
bool isResultContainsKey(VPackSlice const &resultSlice, std::string const &key) noexcept;
}  // namespace impl
}  // namespace table_actor
}  // namespace ok
