#include "table/TableSchemaExternal.hpp"
#include <velocypack/ValueType.h>
#include <trantor/utils/Logger.h>
#include <magic_enum.hpp>
#include "pystring.hpp"
#include "utils/json_functions.hpp"
#include "velocypack/Iterator.h"
#include "utils/ErrorConstants.hpp"
#include "mutate/MutateSchema.hpp"
#include <boost/algorithm/string.hpp>
namespace ok
{
namespace table_schema
{
jsoncons::ojson getJsonHeaderData(Columns const &columns, ColumnOrder const &allIds, Sort const &sort, jsoncons::ojson const &tableProps) noexcept
{
  jsoncons::ojson ret = jsoncons::ojson::object();
  jsoncons::ojson columnSchemaRow = jsoncons::ojson::object();
  jsoncons::ojson sortRow = jsoncons::ojson::object();
  jsoncons::ojson widthRow = jsoncons::ojson::object();
  jsoncons::ojson allIDsRow = jsoncons::ojson::array();
  jsoncons::ojson newALLIdLabels;
  for (auto const &id : allIds)
  {
    auto c = columns.find(id);
    if (c != std::end(columns))
    {
      allIDsRow.push_back(c->second.id);
      if (c->second.skipFrontEnd) continue;
      jsoncons::ojson v;
      v["type"] = (static_cast<unsigned int>(c->second.visible_type));
      if (!c->second.isVisible)
      {  // default is visible
        v["visible"] = c->second.isVisible;
      }
      if (!c->second.editableOptions.empty()) { v["editable"] = c->second.editableOptions; }  // default is not editable
      if (!c->second.props.empty()) v["props"] = c->second.props;
      if (c->second.width != 0) widthRow[id] = c->second.width;
      newALLIdLabels[id] = c->second.label;
      columnSchemaRow[id] = v;
    }
  }
  for (auto sf : sort)
  {
    auto c = columns.find(sf.id);
    if (c != std::end(columns))
    {
      if (sf.direction == Ascending) { sortRow[c->first] = (int)Ascending; }
      else
      {
        sortRow[c->first] = (int)Descending;
      }
    }
  }
  ret["columnSchema"] = columnSchemaRow;
  ret["sort"] = sortRow;
  ret["selectedColumns"] = allIDsRow;
  ret["width"] = widthRow;
  ret["allColumns"] = newALLIdLabels;
  ret["tableProps"] = tableProps;
  return ret;
}
WherePart getFilterPart(jsoncons::ojson const &filtersConfig, Columns const &columns)
{
  WherePart wherePart;
  for (auto &it : filtersConfig.object_range())
  {
    AQLQuery v;
    AQLQuery val1;
    AQLQuery val2;
    AQLQuery op = "LIKE";
    auto c = std::find_if(std::begin(columns), std::end(columns), [&](std::pair<ID, Column> const &c_) { return c_.first == it.key(); });
    if (c == std::end(columns)) continue;  // should return error.
    if (it.value().is_null()) { v = ""; }
    else if (it.value().is_int64())
    {
      v = std::to_string(it.value().as<long>());
    }
    else if (it.value().is_double())
    {
      v = std::to_string(it.value().as<double>());
    }
    else if (it.value().is_bool())
    {
      v = it.value().as<bool>() ? "true" : "false";
    }
    else if (it.value().is_string())
    {
      v = it.value().as<AQLQuery>();
    }
    else if (it.value().is_array())
    {
      val1 = it.value().to_string();
      op = "IN";
      wherePart.push_back({c->first, {c->second.selector, c->second.prefix, op, val1, val2}});
      continue;
    }
    else
    {
      v = "";
    }
    // AQLQuery whereClause;
    // Boost.Spirit X3
    // Check for any special comparison operators at the beginning of the v string. If there are none default to LIKE.
    bool isBetween = false;
    // range/BETWEEN operator
    //  if (v.find('~') != AQLQuery::npos) {
    //    auto sepIdx = v.find('~');
    //    val1 = v.substr(0, sepIdx);
    //    val2 = v.substr(sepIdx + 1);
    //    float val1f = convert_float(val1, 0);
    //    float val2f = convert_float(val2, 0);
    //    isBetween = val1f < val2f;
    //  }
    if (v.empty())
    {
      //
    }
    else if (isBetween)
    {
      // there is no 'between' in arangodb, fix this:
      op = "BETWEEN";
    }
    else
    {
      if (v.substr(0, 2) == ">=" || v.substr(0, 2) == "<=")
      {
        op = v.substr(0, 2);
        val1 = std::to_string(impl::convert_float(v.substr(2), 0));
        // user should take care when != , string should be
        // surrounded by
        // ".
      }
      else if (v.substr(0, 2) == "!=" || v.substr(0, 2) == "=~" || v.substr(0, 2) == "!~")
      {
        op = v.substr(0, 2);
        val1 = v.substr(2);
      }
      else if (v.substr(0, 1) == ">" || v.substr(0, 1) == "<")
      {
        op = v.substr(0, 1);
        val1 = std::to_string(impl::convert_float(v.substr(1), 0));
      }
      else if (v.substr(0, 1) == "=")
      {
        val1 = v.substr(1);
        op = "==";
      }
      else if (v.substr(0, 1) == "[" && v.substr(v.size() - 1, 1) == "]")
      {
        // sometime syntax error occur.
        val1 = v;
        op = "IN";
      }
      else if (v.substr(0, 2) == "![" && v.substr(v.size() - 1, 1) == "]")
      {
        // sometime syntax error occur.
        val1 = v.substr(1);
        op = "NOT IN";
      }
      else
      {
        // switch (columns[i].type) {
        //  case ValueType::Int:
        //    val1 = std::to_string(convert_float(v, 0));
        //    op = "==";
        //    break;
        //  case ValueType::String: {
        // Keep the default LIKE operator
        // Add % wildcards at the start and at the beginning of
        // the filter query, but only if there weren't set any
        // wildcards manually. The idea is to assume that a user
        // who's just typing characters expects the wildcards to
        // be added but a user who adds them herself knows what
        // she's doing and doesn't want us to mess up her query.
        if (v.find('%') == AQLQuery::npos) { val1 = "%" + v + "%"; }
        val1 = "'" + val1 + "'";
        //    break;
        //  }
        //  default:
        //    val1 = v;
        //}
      }
    }
    if (val1.empty() || val1 == "%" || val1 == "%%")
    {
      // wherePart.erase(c->first); // No need....
    }
    else
    {
      /*whereClause = op + " " + val1;
      if (!val2.empty()) {
        whereClause += " AND " + val2;
      }
      whereClause += " ";*/
      // Todo: Literal % and _ need to be escaped with a
      // backslash.
      // https://www.arangodb.com/docs/stable/aql/operators.html
      // If the value was set to an empty string remove any filter
      // for this column. Otherwise insert a new filter rule or
      // replace the old one if there is already one
      wherePart.push_back({c->first, {c->second.selector, c->second.prefix, op, val1, val2}});
    }
  }
  return wherePart;
}
std::tuple<ErrorCode> setFilterConfig(jsoncons::ojson const &args,
                                      Columns const &columns,
                                      TableWhere &where) noexcept  // fix where, in unordered_map data is organized into buckets depending on their hash values
{
  if (args.contains("f") && ((args["f"].is_object() || args["f"].is_array())))
  {
    auto const &filtersConfig = args["f"];
    if (filtersConfig.is_object())
    {
      /*if (filtersConfig.size() > columns.size())
    {
      LOG_DEBUG << "Filter size should be less then column size. filter size: " << filtersConfig.size() << " columns size: " << columns.size();
      return ok::ErrorCode::ERROR_BAD_PARAMETER;
    }*/
      // This loop some rare times crash (when unauthorized user is viewing table?)
      // terminating with uncaught exception of type std::out_of_range: vector
      auto one = getFilterPart(filtersConfig, columns);
      if (!one.empty()) where.push_back(one);
      return {ok::ErrorCode::ERROR_NO_ERROR};
    }
    else if (filtersConfig.is_array())
    {
      for (auto const &f : filtersConfig.array_range())
      {
        if (f.is_object())
        {
          auto one = getFilterPart(f, columns);
          if (!one.empty()) where.push_back(one);
        }
      }
      return {ok::ErrorCode::ERROR_NO_ERROR};
    }
    else
    {
      LOG_DEBUG << "Filter Argument must be object or array: " << filtersConfig.to_string();
      return ok::ErrorCode::ERROR_BAD_PARAMETER;
    }
  }
  return {ok::ErrorCode::ERROR_NO_ERROR};
}
std::tuple<ErrorCode> setSortConfig(const jsoncons::ojson &args, const Columns &columns, Sort &sort) noexcept
{
  if (args.contains("s") && args["s"].is_object())
  {
    auto &sort_config = args["s"];
    if (!sort_config.empty())
    {
      sort.clear();  // clear default sorting.
    }
    for (auto &o : sort_config.object_range())
    {
      auto key = o.key();
      auto value = o.value().as<int>();
      if (auto c = std::find_if(std::begin(columns), std::end(columns), [&](std::pair<ID, Column> const &c) { return c.first == key; }); c != std::end(columns))
      {
        if (value == None)
        {
          //
        }
        else if (value == Ascending)
        {
          if (auto s = std::find_if(std::begin(sort), std::end(sort), [&](const SortedColumn &sc) { return sc.id == c->first; }); s != std::end(sort)) { return ok::ErrorCode::ERROR_RESOURCE_LIMIT; }
          sort.push_back({c->first, Ascending});
        }
        else
        {
          if (auto s = std::find_if(std::begin(sort), std::end(sort), [&](const SortedColumn &sc) { return sc.id == c->first; }); s != std::end(sort)) { return ok::ErrorCode::ERROR_RESOURCE_LIMIT; }
          sort.push_back({c->first, Descending});
        }
      }
    }
  }
  return {ok::ErrorCode::ERROR_NO_ERROR};
}
std::tuple<ErrorCode> setPaginationConfig(const jsoncons::ojson &args, Pagination &pagination) noexcept
{
  if (args.contains("p") && args["p"].is_array())
  {
    auto &pagination_config = args["p"];
    if (pagination_config.empty())
    {
      // do nothing
    }
    else if (pagination_config.size() == 3 && pagination_config[0].is_int64() && pagination_config[1].is_int64() && pagination_config[2].is_int64())
    {
      pagination.limit = pagination_config[0].as<int>();
      pagination.offset = pagination_config[1].as<int>();
      pagination.currentPage = pagination_config[2].as<int>();
    }
    else
    {
      LOG_DEBUG << "Pagination element must be integer";
      return {ok::ErrorCode::ERROR_BAD_PARAMETER};
    }
  }
  return {ok::ErrorCode::ERROR_NO_ERROR};
}
std::tuple<ErrorCode, AQLQuery> setDefaultWhereArray(std::unordered_set<DocumentKey> const &filterKey, AQLQuery &as) noexcept
{
  AQLQuery w;
  if (!filterKey.empty())
  {
    w += as + "._key in @filter";
    /*w += as + "._key in [";
    for (auto &f : filterKey) { w += "\"" + f + "\","; }
    w.pop_back();
    w += "]";*/
    // Todo: fix if there is default where from schema, it should be merged.
    // // defaultWhere = w;
  }
  // single key example:
  //  if (!filterKey.empty()) {
  //    fetc.default_where = "o._key == \"" + filterKey + "\"";
  //  }
  return {ok::ErrorCode::ERROR_NO_ERROR, w};
}
std::tuple<ErrorCode, AQLQuery> buildQuery(AQLQuery &selector,
                                           Columns const &columns,
                                           jsoncons::ojson const &args,
                                           ok::mutate_schema::Relation const &tree,
                                           WrapperFilters &wrapperFilters,
                                           TableWhere const &where_,
                                           Sort sort_,
                                           Pagination pagination_,
                                           AQLQuery const &as,
                                           Collection const &collection,
                                           Edge const &edge,
                                           AQLQuery const &filterTop,
                                           AQLQuery const &filterBottom,
                                           AQLQuery const &defaultWhere) noexcept
{
  // AQLQuery join = buildJoinPart();
  auto [er, where] = buildWherePart(columns, as, where_, defaultWhere);
  if (isEr(er)) return {er, ""};
  // AQLQuery group_by = buildGroupByPart();
  AQLQuery sort = impl::buildOrderByPart(columns, as, sort_);
  AQLQuery pagination = impl::buildPaginationPart(pagination_);
  AQLQuery wrapperTop;
  for (auto &wt : wrapperFilters) { wrapperTop += wt.second.first; }
  AQLQuery query;
  AQLQuery wrapper_indent;
  if (!wrapperTop.empty())
  {
    query += wrapperTop;
    wrapper_indent = "";
    // query += "\n"; // filters already adding this.
  }
  //  if (!tree.edge.empty())
  //  {
  //    if (!wrapperTop.empty()) { wrapper_indent = ""; }
  /* No Loop Generated for tree, use wrapper.
  if (isTreeRoot(tree, args))
  {
    query += wrapper_indent + "FOR " + as + " IN " + collection;
  }
  else if (ok::mutate_schema::impl::isTreeChild(tree, args))
  {
    // Todo fix, inbound should be based on tree direction.
    query += wrapper_indent + "FOR " + as + " IN 1..1 INBOUND '" + collection + "/" + jsoncons::getStringKey(args, "parent") + "' " + edge;
  }
  else
  {
    // note : this not thought.
    query += wrapper_indent + "FOR " + as + " IN " + collection;
  }*/
  //  }
  else
  {
    query += wrapper_indent + "FOR " + as + " IN " + collection;
  }
  auto add = [&query, &wrapper_indent](auto s)
  {
    if (!s.empty())
    {
      query += "\n  ";
      query += wrapper_indent;
      query += s;
    }
  };
  query += filterTop;
  add(where);
  query += filterBottom;
  add(sort);
  add(pagination);
  query += "\n" + wrapper_indent + "  RETURN ";
  query += selector.empty() ? impl::buildTableObjectProjection(columns, as) : selector;
  return {ok::ErrorCode::ERROR_NO_ERROR, query};
}
std::tuple<ErrorCode, AQLQuery> buildWherePart(Columns const &columns, AQLQuery const &as, TableWhere const &where, AQLQuery const &defaultWhere) noexcept
{
  AQLQuery w;
  if (!defaultWhere.empty())
  {
    w = "FILTER ";
    w += defaultWhere;
    w += "\n";
  }
  if (!where.empty())
  {
    w += "FILTER ";
    std::vector<AQLQuery> filters;
    for (auto const &wh : where)
    {
      if (columns.size() < wh.size())
      {
        LOG_DEBUG << "columns size must be greater than filter size " << columns.size() << " " << wh.size();
        return {ok::ErrorCode::ERROR_BAD_PARAMETER, w};
      }
      AQLQuery filterOne;
      if (wh.size() == 0) continue;
      for (auto const &i : wh)
      {
        AQLQuery column;
        if (!i.second.prefix.empty()) { column += i.second.prefix; }
        else
        {
          column += as;
        }
        column += "." + i.second.selector;
        auto w1 = i.second.op + " " + i.second.val1;
        if (!i.second.val2.empty())
        {
          w1 += " AND " + i.second.val2;
          w1 += " ";
        }
        filterOne += column + " " + w1 + " AND ";
      }
      // Remove last ' AND '
      filterOne.erase(filterOne.size() - 5);
      filters.push_back(filterOne);
    }
    w += boost::algorithm::join(filters, " || ");
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, w};
}
namespace impl
{
AQLQuery buildOrderByPart(Columns const &columns, AQLQuery const &as,
                          Sort const &sort) noexcept  // Sorting
{
  AQLQuery order_by;
  for (auto const &sorted_column : sort)
  {
    auto c2 = columns.find(sorted_column.id);
    if (c2 != std::end(columns)) order_by += (!c2->second.prefix.empty() ? c2->second.prefix : as) + "." + c2->second.selector + " " + (sorted_column.direction == Ascending ? "ASC" : "DESC") + ",";
  }
  if (!order_by.empty())
  {
    order_by.pop_back();
    order_by = "SORT " + order_by;
  }
  return order_by;
}
AQLQuery buildPaginationPart(Pagination const &pagination) noexcept
{
  AQLQuery p;
  if (pagination.limit == 0) { return p; }
  int offset = 0;
  // what when offset is more than count? // fix this:
  if (pagination.offset == 0)
  {
    if (pagination.currentPage > 1) { offset = pagination.limit * (pagination.currentPage - 1); }
  }
  if (offset) { p += std::to_string(offset) + ", "; }
  p += std::to_string(pagination.limit);
  p = "LIMIT " + p;
  return p;
}
AQLQuery buildTableObjectProjection(Columns const &columns, AQLQuery const &as) noexcept
{
  AQLQuery selector;
  if (columns.empty())
  {
    selector += as;
    // LOG_DEBUG << "Error: Table Columns schema is empty";
  }
  else
  {
    selector += "{";
    for (auto const &it : columns)
    {
      if (it.first == "_key" || it.first == "_rev") {}
      else if (it.second.skipFrontEnd)
        continue;
      if (it.second.calculated.empty())
      {
        selector += it.second.selector + ":";
        if (!it.second.prefix.empty()) { selector += it.second.prefix; }
        else
        {
          selector += as;
        }
        selector += "." + it.second.selector + ",";
      }
      else
      {
        selector += it.second.id + ":" + it.second.calculated + ",";
      }
    }
    selector.pop_back();
    selector += "}";
  }
  return selector;
}
}  // namespace impl
// namespace impl
}  // namespace table_schema
}  // namespace ok
