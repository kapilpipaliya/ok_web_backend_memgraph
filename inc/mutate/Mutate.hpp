#pragma once
#include "alias.hpp"
#include <unordered_set>
#include "mutate/Relation.hpp"
#include "mutate/Field.hpp"
#include "mutate/MutateSchema.hpp"
#include "table/TableSchema.hpp"
class FuerteClient;
using Fields = ok::mutate_schema::Fields;
namespace ok
{
// Basics/voc-types.h
enum TRI_voc_document_operation_e : uint8_t {
  TRI_VOC_DOCUMENT_OPERATION_UNKNOWN = 0,
  TRI_VOC_DOCUMENT_OPERATION_INSERT,
  TRI_VOC_DOCUMENT_OPERATION_UPDATE,
  TRI_VOC_DOCUMENT_OPERATION_REPLACE,
  TRI_VOC_DOCUMENT_OPERATION_REMOVE
};
enum class ErrorCode;
namespace mutate_actor
{
struct PositionData;
// todo fix query return inserted/updated/deleted member, based on int spawn messages to table actor
ok::smart_actor::connection::MutateEventType getMutateEventType(WsEvent const &event);
// clang-format off
using MutateCallbackFunction = std::function<ErrorCode(DocumentKey const &database, WsArguments const &args, ok::smart_actor::connection::MutateEventType et, ok::mutate_schema::Fields &fields)>;
std::tuple<ErrorCode, AQLQuery, mutate_schema::MutateQueryParts, arangodb::velocypack::Builder, std::unordered_set<DocumentKey>, ok::mutate_schema::Fields> save(Database const &database, DocumentKey const &memberKey, const DocumentKey &schemaKey, const VPackSlice &schemaJson, const VPackSlice &columnSchemaJson, smart_actor::connection::MutateEventType et, WsArguments const &args, WsArguments const &backendValue, bool permissionCheck, bool ignoreMissingFieldWithValue, MutateCallbackFunction preProcess, MutateCallbackFunction postProcess);
bool isModifyEvent(ok::smart_actor::connection::MutateEventType eventType);
bool isDeleteEvent(ok::smart_actor::connection::MutateEventType eventType);
void runSameQueryOnAllUsers(AQLQuery const &query, const arangodb::velocypack::Builder &bindVars, const ok::mutate_schema::RelationQueryMap &relationQueries) noexcept;
namespace impl
{
ErrorCode initializeBase(VPackSlice const &formSchemaJson, VPackSlice const &columnSchemaJson, const ok::smart_actor::connection::MutateEventType &eventType) noexcept;
std::tuple<ErrorCode, Collection, mutate_schema::Relation, ok::mutate_schema::Fields, mutate_schema::MutateQueryParts,  jsoncons::ojson> initializeInsert(Database const &database, DocumentKey const &memberKey, VPackSlice const &formSchemaJson, WsArguments const &args, WsArguments const &backendValue, bool ignoreMissingFieldWithValue) noexcept;
std::tuple<ErrorCode, Collection, mutate_schema::Relation, table_schema::Columns, table_schema::TableWhere> columnSchemaInitialize(Database const &database, DocumentKey const &memberKey, VPackSlice const &columnSchemaJson, WsArguments const &args, bool doProcessFilters = true) noexcept;
void deleteNestedEntries(VPackSlice outer1, ok::mutate_schema::RelationQueryMap const &relationQueries, Database const &database) noexcept;
std::tuple<ErrorCode, std::unordered_set<DocumentKey> > update(AQLQuery const &query, arangodb::velocypack::Builder const &bindVars, Database const &database, ok::mutate_schema::RelationQueryMap const &relationQueries) noexcept;
std::pair<ErrorCode, std::unordered_set<DocumentKey>> getChangedKeys(VPackSlice outer2) noexcept;
void sendToActivitiesActor(std::string const &key) noexcept;
void updateActivities(VPackSlice const &msgSlice) noexcept;
//void sendNotification(VPackSlice dataSlice, Database const &database, WsArguments const &args, const TRI_voc_document_operation_e operation_type, caf::event_based_actor *act) noexcept;
namespace make_query
{
std::tuple<ErrorCode, AQLQuery> insert(ok::mutate_schema::MutateQueryParts &queryParts) noexcept;
std::tuple<ErrorCode, AQLQuery> update(mutate_schema::MutateQueryParts &queryParts, bool isReplaceData) noexcept;
std::tuple<ErrorCode, AQLQuery> delete_(mutate_schema::MutateQueryParts &queryParts) noexcept;
std::tuple<ErrorCode, AQLQuery> position(ok::mutate_actor::PositionData const &data) noexcept;
std::tuple<ErrorCode, AQLQuery> batchUpdate(ok::mutate_schema::MutateQueryParts &queryParts, const table_schema::Columns &columns, const table_schema::TableWhere &where, bool isReplaceData = false) noexcept;
std::tuple<ErrorCode, AQLQuery> batchDelete(ok::mutate_schema::MutateQueryParts &queryParts, ok::table_schema::Columns const &columns, const table_schema::TableWhere &where) noexcept;
}
namespace get_merged_bind_vars
{
std::tuple<ErrorCode, jsoncons::ojson> getKeyRev(WsArguments const &args);
std::tuple<ErrorCode, arangodb::velocypack::Builder> base(Collection const &collection, jsoncons::ojson &saveData, ok::mutate_schema::Fields const &fields, WsArguments const &args, DocumentKey const &memberKey) noexcept;
std::tuple<ErrorCode, arangodb::velocypack::Builder> insert(Collection const &collection, jsoncons::ojson &saveData, const ok::mutate_schema::Fields &fields, WsArguments const &args, DocumentKey const &memberKey) noexcept;
std::tuple<ErrorCode, arangodb::velocypack::Builder> update(Collection const &collection, jsoncons::ojson &saveData, const ok::mutate_schema::Fields &fields, WsArguments const &args, DocumentKey const &memberKey) noexcept;
std::tuple<ErrorCode, arangodb::velocypack::Builder> delete_(Collection const &collection, jsoncons::ojson &saveData, const ok::mutate_schema::Fields &fields, WsArguments const &args) noexcept;
std::tuple<ErrorCode, arangodb::velocypack::Builder> position(Collection const &collection, jsoncons::ojson &saveData, const ok::mutate_schema::Fields &fields, WsArguments const &args, ok::mutate_actor::PositionData const &data) noexcept;
std::tuple<ErrorCode, arangodb::velocypack::Builder> batchUpdate(Collection const &collection, jsoncons::ojson &saveData, const ok::mutate_schema::Fields &fields, WsArguments const &args, DocumentKey const &memberKey) noexcept;
std::tuple<ErrorCode, arangodb::velocypack::Builder> batchDelete( Collection const &collection, jsoncons::ojson &saveData, const ok::mutate_schema::Fields &fields, WsArguments const &args) noexcept;
}
namespace locked_row_permission_check
{
ErrorCode base(VPackSlice const &formSchemaJson, jsoncons::ojson const &args, std::string const &key, Collection const &collection, mutate_schema::Relation const &tree, Database const &database, const table_schema::Columns &columns, const table_schema::TableWhere &where) noexcept;
ErrorCode insert() noexcept;
ErrorCode update(VPackSlice const &formSchemaJson, jsoncons::ojson const &args, Collection const &collection, mutate_schema::Relation const &tree, Database const &database, ok::table_schema::Columns const &columns, const table_schema::TableWhere &where) noexcept;
ErrorCode delete_(VPackSlice const &formSchemaJson, jsoncons::ojson const &args, Collection const &collection, mutate_schema::Relation const &tree, Database const &database, ok::table_schema::Columns const &columns, const table_schema::TableWhere &where) noexcept;
ErrorCode position() noexcept;
ErrorCode batchUpdate(VPackSlice const &formSchemaJson, jsoncons::ojson const &args, Collection const &collection, mutate_schema::Relation const &tree, Database const &database, ok::table_schema::Columns const &columns, ok::table_schema::TableWhere const &where) noexcept;
ErrorCode batchDelete(VPackSlice const &formSchemaJson, jsoncons::ojson const &args, Collection const &collection, mutate_schema::Relation const &tree, Database const &database, ok::table_schema::Columns const &columns, ok::table_schema::TableWhere const &where) noexcept;
}
namespace unique_check
{
ErrorCode base(VPackSlice dataSlice, Database const &database, Collection const &collection, TRI_voc_document_operation_e operation_type, ok::mutate_schema::Fields &fields) noexcept;
ErrorCode insert(VPackSlice const &formSchemaJson, Database const &database, Collection const &collection, ok::mutate_schema::Fields &fields) noexcept;
ErrorCode update(VPackSlice const &formSchemaJson, Database const &database, Collection const &collection, ok::mutate_schema::Fields &fields, bool isReplaceData = false) noexcept;
}
// clang-format on
}  // namespace impl
}  // namespace mutate_actor
}  // namespace ok
