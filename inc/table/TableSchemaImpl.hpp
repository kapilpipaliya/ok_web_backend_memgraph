#pragma once
#include "TableSchema.hpp"
namespace ok
{
enum class ErrorCode;
namespace table_schema
{
namespace impl
{
void processColumnHiddenOrderSort(VPackSlice const &schemaJson, jsoncons::ojson const &args, Columns &columns, Sort &sort) noexcept;
std::tuple<ErrorCode, AQLQuery, AQLQuery, bool> processFilters(VPackSlice const &schemaJson,
                                                               WrapperFilters &wrapperFilters,
                                                               mutate_schema::Relation const &tree,
                                                               jsoncons::ojson const &args,
                                                               Database const &database,
                                                               DocumentKey const &memberKey,
                                                               Collection const &collection,
                                                               std::string const &as,
                                                               bool isPermission) noexcept;
std::tuple<ErrorCode, mutate_schema::Relation> processTree(VPackSlice const &schemaJson, Collection &collection) noexcept;
std::tuple<ErrorCode, AQLQuery, bool> setFilterFromSchema(VPackSlice const &slice,
                                                          std::string const &filterKey,
                                                          WrapperFilters &wrapperFilters,
                                                          mutate_schema::Relation const &tree,
                                                          jsoncons::ojson const &args,
                                                          Database const &database,
                                                          DocumentKey const &memberKey,
                                                          Collection const &collection,
                                                          std::string const &as,
                                                          bool isPermission) noexcept;
std::tuple<ErrorCode, AQLQuery, bool> getAllFilters(mutate_schema::Relation const &tree,
                                                    jsoncons::ojson const &args,
                                                    jsoncons::ojson const &filters,
                                                    WrapperFilters &wrapperFilters,
                                                    Database const &database,
                                                    DocumentKey const &memberKey,
                                                    Collection const &collection,
                                                    std::string const &as,
                                                    bool isPermission,
                                                    std::string const &wrapper_indent,
                                                    bool isOptional = false) noexcept;
std::tuple<ErrorCode, AQLQuery> getFilterCount(jsoncons::ojson const &filterSchema, std::string const &as, std::string const &wrapper_indent) noexcept;
std::tuple<ErrorCode, AQLQuery> getFilterWrapper(jsoncons::ojson const &filterSchema, WrapperFilters &wrapperFilters, std::string const &wrapper_indent) noexcept;
std::tuple<ErrorCode, AQLQuery> getFilterPattern(
    jsoncons::ojson const &filterSchema, Database const &database, DocumentKey const &memberKey, std::string const &wrapper_indent, const WsArguments &args) noexcept;
std::tuple<ErrorCode, AQLQuery> getFilterNoAuth(bool isPermission, DocumentKey const &memberKey, Collection const &collection, std::string const &wrapper_indent) noexcept;
std::tuple<ErrorCode, Collection, std::string> getCollection(VPackSlice const &schemaJson) noexcept;
jsoncons::ojson getTableProps(VPackSlice const &schemaJson) noexcept;
std::tuple<bool, bool> getPermissions(VPackSlice const &schemaJson) noexcept;
std::tuple<ErrorCode, std::string, std::string> getFirstConfigValueFromArgs(jsoncons::ojson const &config, jsoncons::ojson const &args, DocumentKey const &memberKey);
ErrorCode getWrapperTop(
    mutate_schema::Relation const &tree, jsoncons::ojson const &args, std::string const &as, const jsoncons::ojson &wt, WrapperFilters &wrapperFilters, DocumentKey const &memberKey) noexcept;
namespace process_column
{
ErrorCode dataPart(VPackSlice const &schemaJson,
                   Columns &columns,
                   ColumnOrder &allIds,
                   jsoncons::ojson const &args,
                   mutate_schema::Relation const &tree,
                   WrapperFilters &wrapperFilters,
                   Database const &database,
                   DocumentKey const &memberKey,
                   Collection const &collection,
                   std::string const &as,
                   bool isPermission) noexcept;
// todo make this also work for number, boolean, null etc.
std::tuple<ErrorCode, AQLQuery> calculated(jsoncons::ojson const &args,
                                           jsoncons::ojson const &obj,
                                           Column &c,
                                           mutate_schema::Relation const &tree,
                                           WrapperFilters &wrapperFilters,
                                           Database const &database,
                                           DocumentKey const &memberKey,
                                           Collection const &collection,
                                           std::string const &as,
                                           bool isPermission) noexcept;
std::tuple<ErrorCode, AQLQuery> getCalculatedChildrenCount(jsoncons::ojson const &args,
                                                           jsoncons::ojson const &filterSchema,
                                                           mutate_schema::Relation const &tree,
                                                           WrapperFilters &wrapperFilters,
                                                           Database const &database,
                                                           DocumentKey const &memberKey,
                                                           Collection const &collection,
                                                           std::string const &as,
                                                           bool isPermission) noexcept;
void hidden(VPackSlice const &schemaJson, Columns &columns) noexcept;
Columns orderManual(Columns &columns, VPackSlice const &orderSlice) noexcept;
void order(VPackSlice const &schemaJson, Columns &columns) noexcept;
ErrorCode sort(VPackSlice const &schemaJson, Columns &columns, Sort &sort) noexcept;
// void processFormProps();
ErrorCode selector(VPackSlice const &obj, Column &c) noexcept;
ID id(VPackSlice const &obj);
void visibleType(VPackSlice const &obj, Column &c) noexcept;
void editable(VPackSlice const &obj, Column &c) noexcept;
void props(VPackSlice const &obj, Column &c) noexcept;
}  // namespace process_column
}  // namespace impl
}  // namespace table_schema
}  // namespace ok
