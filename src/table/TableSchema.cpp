#include "table/TableSchema.hpp"
#include "table/TableSchemaImpl.hpp"
#include <trantor/utils/Logger.h>
#include "utils/json_functions.hpp"
#include "velocypack/Iterator.h"
#include <boost/algorithm/string.hpp>
#include "utils/ErrorConstants.hpp"
#include "actor_system/Routes.hpp"
#include "mutate/Field.hpp"
#include <iostream>
namespace ok::table_schema
{
std::tuple<ErrorCode, Collection, mutate_schema::Relation, Columns, ColumnOrder, WrapperFilters, Sort, AQLQuery, AQLQuery, bool, jsoncons::ojson> initialize(
    jsoncons::ojson const &args, VPackSlice const &schemaJson, Database const &database, DocumentKey const &memberKey, bool isPermission, bool doProcessFilters)
{
  Columns columns;
  ColumnOrder allIds;
  WrapperFilters wrapperFilters;
  Sort sort;
  auto returnEmpty = [&](ok::ErrorCode er) { return std::make_tuple(er, "", mutate_schema::Relation{}, columns, allIds, wrapperFilters, sort, "", "", false, jsoncons::ojson{}); };
  if (!args.is_object())
  {
    LOG_DEBUG << "table config options must be an object :" << args.to_string();
    return returnEmpty(ok::ErrorCode::ERROR_BAD_PARAMETER);
  }
  auto [erC, collection, as] = impl::getCollection(schemaJson);
  if (ok::isEr(erC)) { return returnEmpty(erC); }
  auto [er, tree] = impl::processTree(schemaJson, collection);
  if (ok::isEr(er)) { return returnEmpty(er); }
  auto tableProps = impl::getTableProps(schemaJson);
  auto [childPermissionCheck, adminPermissionCheck] = impl::getPermissions(schemaJson);
  AQLQuery filterTop;
  AQLQuery filterBottom;
  bool queryGeneratedForUnAuth{false};
  if (doProcessFilters)
  {
    auto [erF, filterTop_, filterBottom_, queryGeneratedForUnAuth_] = impl::processFilters(schemaJson, wrapperFilters, tree, args, database, memberKey, collection, as, isPermission);
    // on delete configuration option is probably empty...
    if (ok::isEr(erF)) { return {erF, collection, tree, columns, {}, wrapperFilters, sort, filterTop_, filterBottom_, queryGeneratedForUnAuth_, tableProps}; }
    else
    {
      filterTop = std::move(filterTop_);
      filterBottom = std::move(filterBottom_);
      queryGeneratedForUnAuth = queryGeneratedForUnAuth_;
    }
  }
  if (auto er = impl::process_column::dataPart(schemaJson, columns, allIds, args, tree, wrapperFilters, database, memberKey, collection, as, isPermission); ok::isEr(er))
    return {er, collection, tree, columns, {}, wrapperFilters, sort, filterTop, filterBottom, queryGeneratedForUnAuth, tableProps};
  impl::processColumnHiddenOrderSort(schemaJson, args, columns, sort);
  return {ok::ErrorCode::ERROR_NO_ERROR, collection, tree, columns, allIds, wrapperFilters, sort, filterTop, filterBottom, queryGeneratedForUnAuth, tableProps};
}
SchemaKey getKeyFromWhere(TableWhere &where) noexcept
{
  if (!where.empty() && !where[0].empty())
  {
    auto wherePart = where[0];
    auto w = std::find_if(std::begin(wherePart), std::end(wherePart), [&](auto const w_) { return w_.first == "_key"; });
    if (w != std::end(wherePart))
    {
      Collection collection_key = w->second.val1;
      boost::algorithm::erase_all(collection_key, "\"");
      return collection_key;
    }
  }
  return "";
}
jsoncons::ojson getPaginationArray(Pagination const &pagination) noexcept
{
  jsoncons::ojson p = jsoncons::ojson::array();
  p.push_back(pagination.limit);
  p.push_back(pagination.offset);
  p.push_back(pagination.currentPage);
  return p;
}
bool getUserTreeOption(mutate_schema::Relation const &tree, jsoncons::ojson const &args) noexcept
{
  bool defaultOption{!tree.edge.empty()};
  return defaultOption && jsoncons::getBoolOption(args, "tree", defaultOption);
}
int getColumnIndex(Columns const &columns, ID const &id)
{
  if (auto find = std::find_if(std::begin(columns), std::end(columns), [&](std::pair<ID, Column> const &f) { return f.first == id; }); find != std::end(columns))
    return std::distance(std::begin(columns), find);
  else
    return -1;
}
std::string to_string(Filter const &filter) noexcept { return "Op: " + filter.op + " Val1: " + filter.val1 + " Val2: " + filter.val2; }
}  // namespace ok::table_schema
