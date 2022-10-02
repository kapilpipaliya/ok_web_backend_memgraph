#include "table/TableSchemaImpl.hpp"
#include <trantor/utils/Logger.h>
#include <magic_enum.hpp>
#include <absl/strings/str_format.h>
#include <absl/strings/substitute.h>
#include "pystring.hpp"
#include "utils/json_functions.hpp"
#include "velocypack/Iterator.h"
#include "utils/ErrorConstants.hpp"
#include "actor_system/Routes.hpp"
#include "mutate/Field.hpp"
#include <iostream>
#include "mutate/MutateSchema.hpp"  // isTreeChild function
#include "mutate/MutateSchemaImpl.hpp"
namespace ok::table_schema
{
namespace impl
{
std::tuple<ErrorCode, AQLQuery, AQLQuery, bool> processFilters(VPackSlice const &schemaJson,
                                                               WrapperFilters &wrapperFilters,
                                                               ok::mutate_schema::Relation const &tree,
                                                               jsoncons::ojson const &args,
                                                               Database const &database,
                                                               DocumentKey const &memberKey,
                                                               Collection const &collection,
                                                               std::string const &as,
                                                               bool isPermission) noexcept
{
  // wrapper should be processed before filter.
  if (schemaJson.isObject() && schemaJson.hasKey("wt"))
  {
    auto wtJson = jsoncons::ojson::parse(schemaJson["wt"].toJson());
    if (auto er = getWrapperTop(tree, args, as, wtJson, wrapperFilters, memberKey); ok::isEr(er)) { return {er, "", "", false}; }
  }
  AQLQuery filterTop;
  AQLQuery filterBottom;
  bool queryGeneratedForUnAuth{false};
  if (schemaJson.isObject() && schemaJson.hasKey("ft"))
  {
    if (auto [erT, filterTop_, queryGeneratedForUnAuth_] = impl::setFilterFromSchema(schemaJson, "ft", wrapperFilters, tree, args, database, memberKey, collection, as, isPermission); ok::isEr(erT))
    {
      return {erT, filterTop_, "", queryGeneratedForUnAuth_};
    }
    else
    {
      filterTop = std::move(filterTop_);
      queryGeneratedForUnAuth = queryGeneratedForUnAuth_;
    }
  }
  if (schemaJson.isObject() && schemaJson.hasKey("fb"))
  {
    if (auto [erB, filterBottom_, queryGeneratedForUnAuth2] = setFilterFromSchema(schemaJson, "fb", wrapperFilters, tree, args, database, memberKey, collection, as, isPermission); ok::isEr(erB))
    {
      return {erB, filterTop, filterBottom_, queryGeneratedForUnAuth || queryGeneratedForUnAuth2};
    }
    else
    {
      filterBottom = std::move(filterBottom_);
      queryGeneratedForUnAuth = queryGeneratedForUnAuth || queryGeneratedForUnAuth2;
    }
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, filterTop, filterBottom, queryGeneratedForUnAuth};
}
void processColumnHiddenOrderSort(VPackSlice const &schemaJson, jsoncons::ojson const &args, Columns &columns, Sort &sort) noexcept
{
  process_column::hidden(schemaJson, columns);
  if (jsoncons::ObjectMemberIsArray(args, "selectedColumns") && args["selectedColumns"].size() > 0)
  {
    VPackParser parser;
    parser.parse(args["selectedColumns"].to_string());
    auto orderSlice = parser.steal()->slice();
    columns = process_column::orderManual(columns, orderSlice);
  }
  /*else
  {
    processColumnOrder(schemaJson, columns);
  }*/
  process_column::sort(schemaJson, columns, sort);
}
std::tuple<ErrorCode, std::string, std::string> getFirstConfigValueFromArgs(jsoncons::ojson const &config, jsoncons::ojson const &args, DocumentKey const &memberKey)
{
  std::string option;
  std::string configValue;
  bool required{true};
  if (config.is_string())
  {
    if (config.as_string_view() == "member")
    {
      if (memberKey.empty()) { return {ok::ErrorCode::ERROR_BAD_PARAMETER, "member", ""}; }
      LOG_DEBUG << "MemberKey empty";
      return {ok::ErrorCode::ERROR_NO_ERROR, "member", memberKey};
    }
    else if (!args.contains(config.as_string_view()))
    {
      LOG_DEBUG << "args not contain " << config.as_string() << " key";
      return {ok::ErrorCode::ERROR_BAD_PARAMETER, config.as_string(), ""};
    }
    configValue = args[config.as_string_view()].as_string();
  }
  else if (config.is_array())
  {
    std::tie(option, configValue, required) = jsoncons::getFirstStringOption(config, args);
  }
  else if (jsoncons::isEmptyObject(config))  // bydefault config is empty object.
  {
    return {ok::ErrorCode::ERROR_NO_ERROR, option, configValue};
  }
  //  else if (config.is_object()) {
  //  }
  else
  {
    LOG_DEBUG << "schema error config: " << config.to_string() << " args: " << args.to_string();
    return {ok::ErrorCode::ERROR_FORM_EMPTY, "", ""};
  }
  if (configValue.empty() && required)
  {
    LOG_DEBUG << "configuration option: " << args.to_string() << " not contain " << config.to_string() << " value";
    return {ok::ErrorCode::ERROR_BAD_PARAMETER, "", ""};
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, option, configValue};
}
ErrorCode getWrapperTop(
    ok::mutate_schema::Relation const &tree, jsoncons::ojson const &args, std::string const &as, const jsoncons::ojson &wt, WrapperFilters &wrapperFilters, DocumentKey const &memberKey) noexcept
{
  if (wt.is_array())
  {
    for (auto const &item : wt.array_range())
    {
      if (jsoncons::ArrayPosIsString(item, 0))
      {
        auto edge = item[0].as_string();
        if (edge == "tree_root")
        {
          if (ok::table_schema::isTreeRoot(tree, args) && jsoncons::ArrayPosIsArray(item, 1))
            if (auto er = getWrapperTop(tree, args, as, item[1], wrapperFilters, memberKey); ok::isEr(er)) return er;
          continue;
        }
        else if (edge == "tree_child")
        {
          if (ok::mutate_schema::impl::isTreeChild(tree, args) && jsoncons::ArrayPosIsArray(item, 1))
            if (auto er = getWrapperTop(tree, args, as, item[1], wrapperFilters, memberKey); ok::isEr(er)) return er;
          continue;
        }
        if (jsoncons::ArrayPosIsArray(item, 1))
        {
          auto &ops = item[1];
          std::string_view type = "";
          if (jsoncons::ArrayPosIsString(ops, 0)) type = ops[0].as_string_view();
          std::string_view wtAs = "";
          if (jsoncons::ArrayPosIsString(ops, 1)) wtAs = ops[1].as_string_view();
          std::string_view direction = "";
          if (jsoncons::ArrayPosIsString(ops, 2)) direction = ops[2].as_string_view();
          else
            return ok::ErrorCode::SCHEMA_ERROR;
          std::string_view collection;
          if (jsoncons::ArrayPosIsString(ops, 3)) collection = ops[3].as_string_view();
          if (type == "graph2")
          {  // simple type
            auto [er, option, configValue] = getFirstConfigValueFromArgs(ops[4], args, memberKey);
            if (ok::isEr(er)) return er;
            std::string filter = "FOR ";
            filter += wtAs;
            filter += " IN ";
            filter += edge;
            filter += "\n  FILTER ";
            // "%s%s == '%s/%s'";
            filter += wtAs;
            filter += direction == "out" ? "._to" : "._from";
            filter += " == '";
            filter += collection;
            filter += "/";
            filter += configValue;
            filter += "'";
            auto filterPart2 = "FILTER " + as + "._id == " + std::string(wtAs) + (direction == "out" ? "._from" : "._to");
            wrapperFilters.insert({std::string(wtAs), {filter, filterPart2}});
          }
          else if (type == "graph")
          {
            auto [er, option, configValue] = getFirstConfigValueFromArgs(ops[4], args, memberKey);
            if (ok::isEr(er)) return er;
            // auto parentKey = jsoncons::getStringKey(args,"parent");
            std::string wrapperTop = "FOR ";
            wrapperTop += wtAs;
            wrapperTop += getUserTreeOption(tree, args) ? " IN 1..1 " : " IN 1..1000 ";
            wrapperTop += (direction == "out" ? "IN" : "OUT");
            wrapperTop += "BOUND '";
            wrapperTop += collection;
            wrapperTop += "/" + configValue + "' " + edge;
            wrapperFilters.insert({std::string(wtAs), {wrapperTop, ""}});
          }
          else
          {  // simple type
            std::string filter = "FOR ";
            filter += wtAs;
            filter += " IN ";
            filter += collection;
            wrapperFilters.insert({std::string(wtAs), {filter, ""}});
          }
        }
      }
    }
    return ok::ErrorCode::ERROR_NO_ERROR;
  }
  return ok::ErrorCode::SCHEMA_ERROR;
}  // namespace impl
std::tuple<ErrorCode, AQLQuery, bool> setFilterFromSchema(VPackSlice const &slice,
                                                          std::string const &filterKey,
                                                          WrapperFilters &wrapperFilters,
                                                          ok::mutate_schema::Relation const &tree,
                                                          jsoncons::ojson const &args,
                                                          Database const &database,
                                                          DocumentKey const &memberKey,
                                                          Collection const &collection,
                                                          std::string const &as,
                                                          bool isPermission) noexcept
{
  std::string wrapper_indent;
  if (tree.edge.empty() && wrapperFilters.size() > 0)  // fix this according to size change indent.
  {
    wrapper_indent = "  ";
  }
  auto j = jsoncons::ojson::parse(slice[filterKey].toJson());
  if (auto [er, filter, isnoAuthQueryuery] = getAllFilters(tree, args, j, wrapperFilters, database, memberKey, collection, as, isPermission, wrapper_indent); ok::isEr(er))
  {
    return {er, filter, isnoAuthQueryuery};
  }
  else
  {
    // queryGeneratedForUnAuth = isnoAuthQueryuery; // passed on result
    // if (!filter.empty())
    //{
    // filter += filter;
    //}
    return {ok::ErrorCode::ERROR_NO_ERROR, filter, isnoAuthQueryuery};
  }
}
std::tuple<ErrorCode, AQLQuery, bool> getAllFilters(ok::mutate_schema::Relation const &tree,
                                                    jsoncons::ojson const &args,
                                                    jsoncons::ojson const &filters,
                                                    WrapperFilters &wrapperFilters,
                                                    Database const &database,
                                                    DocumentKey const &memberKey,
                                                    Collection const &collection,
                                                    std::string const &as,
                                                    bool isPermission,
                                                    std::string const &wrapper_indent,
                                                    bool isOptional) noexcept
{
  std::string filter;
  bool noAuthQuery{false};
  static constexpr auto default_indent{"  "};
  if (filters.is_array())
  {
    for (auto const &item : filters.array_range())
    {
      if (jsoncons::ArrayPosIsString(item, 0))
      {
        if (auto func = item[0].as_string_view(); func == "tree_root")
        {
          if (isTreeRoot(tree, args) && jsoncons::ArrayPosIsArray(item, 1))
          {
            if (auto [er, filter_, noAuthQuery] = getAllFilters(tree, args, item[1], wrapperFilters, database, memberKey, collection, as, isPermission, wrapper_indent); ok::isEr(er))
              return {er, "", false};
            else
              filter += std::move(filter_);
          }
        }
        else if (auto func = item[0].as_string_view(); func == "tree_root_op")
        {
          if (isTreeRoot(tree, args) && getUserTreeOption(tree, args) && jsoncons::ArrayPosIsArray(item, 1))
          {
            if (auto [er, filter_, noAuthQuery] = getAllFilters(tree, args, item[1], wrapperFilters, database, memberKey, collection, as, isPermission, wrapper_indent, true); ok::isEr(er))
              return {er, "", false};
            else
              filter += std::move(filter_);
          }
        }
        else if (func == "tree")
        {
          if (!isTreeRoot(tree, args) && jsoncons::ArrayPosIsArray(item, 1))  // !isTreeRoot() means parent key should exist
          {
            if (auto [er, filter_, noAuthQuery] = getAllFilters(tree, args, item[1], wrapperFilters, database, memberKey, collection, as, isPermission, wrapper_indent); ok::isEr(er))
              return {er, "", false};
            else
              filter += std::move(filter_);
          }
        }
        else if (func == "in" || func == "out")
        {
          if (!(isOptional && jsoncons::getBoolOption(args, "all", false)))
          {
            if (auto [er, filter_] = getFilterCount(item, as, wrapper_indent); ok::isEr(er)) return {er, "", false};
            else
              filter += std::move(filter_);
          }
        }
        else if (func == "wrapper")
        {
          if (auto [er, filter_] = getFilterWrapper(item, wrapperFilters, wrapper_indent); ok::isEr(er)) return {er, "", false};
          else
            filter += std::move(filter_);
        }
        else if (func == "static")
        {
          if (jsoncons::ArrayPosIsString(item, 1)) filter += "\n" + wrapper_indent + default_indent + item[1].as_string();
        }
        else if (func == "p2")
        {
          if (auto [er, filter_] = getFilterPattern(item, database, memberKey, wrapper_indent, args); ok::isEr(er)) return {er, "", false};
          else
            filter += std::move(filter_);
        }
        else if (func == "noauth")
        {
          if (auto [er, filter_] = getFilterNoAuth(isPermission, memberKey, collection, wrapper_indent); ok::isEr(er)) { return {er, "", false}; }
          else
          {
            if (!filter_.empty())
            {
              filter += std::move(filter_);
              noAuthQuery = true;
            }
          }
        }
      }
    }
  }
  else if (filters.is_object())
  {
    for (auto member : filters.object_range())
    {
      auto key = member.key();
      if (key == "guest")
      {
        if (memberKey.empty())
        {
          if (auto [er, filter_, noAuthQuery] = getAllFilters(tree, args, member.value(), wrapperFilters, database, memberKey, collection, as, isPermission, wrapper_indent, true); ok::isEr(er))
            return {er, "", false};
          else
            filter += std::move(filter_);
        }
      }
      else if (key == "static")
      {
        if (jsoncons::ObjectMemberIsString(filters, key)) filter += "\n" + wrapper_indent + default_indent + member.value().as_string();
      }
    }
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, filter, noAuthQuery};
}
std::tuple<ErrorCode, AQLQuery> getFilterCount(jsoncons::ojson const &filterSchema, std::string const &as, std::string const &wrapper_indent) noexcept
{
  static constexpr auto default_indent{"  "};
  std::string edge;
  std::string direction;
  if (jsoncons::ArrayPosIsString(filterSchema, 0)) { direction = filterSchema[0].as_string_view() == "in" ? "INBOUND" : "OUTBOUND"; }
  if (direction.empty()) { return {ok::ErrorCode::SCHEMA_ERROR, ""}; }
  if (jsoncons::ArrayPosIsString(filterSchema, 1)) { edge = filterSchema[1].as_string_view(); }
  int number = 0;
  if (jsoncons::ArrayPosIsInteger(filterSchema, 2)) { number = filterSchema[2].as_integer<int>(); }
  if (edge.empty()) { return {ok::ErrorCode::SCHEMA_ERROR, ""}; }
  std::string countQuery = "\n" + wrapper_indent + default_indent + "FILTER LENGTH(FOR v IN 1..1 " + direction + " " + as + " " + edge + " RETURN v) == " + std::to_string(number) + "";
  return {ok::ErrorCode::ERROR_NO_ERROR, countQuery};
}
std::tuple<ErrorCode, AQLQuery> getFilterWrapper(jsoncons::ojson const &filterSchema, WrapperFilters &wrapperFilters, std::string const &wrapper_indent) noexcept
{
  static constexpr auto default_indent{"  "};
  if (jsoncons::ArrayPosIsString(filterSchema, 1))
  {
    auto as = filterSchema[1].as_string();
    if (auto find = wrapperFilters.find(as); find != std::end(wrapperFilters)) { return {ok::ErrorCode::ERROR_NO_ERROR, "\n" + wrapper_indent + default_indent + wrapperFilters[as].second}; }
  }
  {
    return {ok::ErrorCode::SCHEMA_ERROR, ""};
  }
}
std::tuple<ErrorCode, AQLQuery> getFilterPattern(
    jsoncons::ojson const &filterSchema, Database const &database, DocumentKey const &memberKey, std::string const &wrapper_indent, WsArguments const &args) noexcept
{
  static constexpr auto default_indent{"  "};
  if (jsoncons::ArrayPosIsString(filterSchema, 1))
  {
    auto pattern = filterSchema[1].as_string();
    if (jsoncons::ArrayPosIsString(filterSchema, 2))
    {
      if (auto fun = filterSchema[2].as_string(); fun == "database")
      {
        if (database.empty()) { return {ok::ErrorCode::ERROR_BAD_PARAMETER, ""}; }
        else
        {
          return {ok::ErrorCode::ERROR_NO_ERROR, "\n" + wrapper_indent + default_indent + absl::Substitute(pattern, database)};
        }
      }
      else if (fun == "memberkey")
      {
        if (memberKey.empty()) { return {ok::ErrorCode::ERROR_BAD_PARAMETER, ""}; }
        else
        {
          return {ok::ErrorCode::ERROR_NO_ERROR, "\n" + wrapper_indent + default_indent + absl::Substitute(pattern, memberKey)};
        }
      }
      else if (fun == "args")
      {
        if (!jsoncons::ArrayPosIsArray(filterSchema, 3))
        {
          LOG_DEBUG << "P2 args must provide options";
          return {ok::ErrorCode::ERROR_BAD_PARAMETER, ""};
        }
        else
        {
          auto [er, option, configValue] = getFirstConfigValueFromArgs(filterSchema[3], args, memberKey);
          if (ok::isEr(er)) return {er, ""};
          return {ok::ErrorCode::ERROR_NO_ERROR, "\n" + wrapper_indent + default_indent + absl::Substitute(pattern, configValue)};
        }
      }
    }
  }
  return {ok::ErrorCode::SCHEMA_ERROR, ""};
}
std::tuple<ErrorCode, AQLQuery> getFilterNoAuth(bool isPermission, DocumentKey const &memberKey, Collection const &collection, std::string const &wrapper_indent) noexcept
{
  static constexpr auto default_indent{"  "};
  if (!isPermission)
  {
    constexpr auto y = R"aql(
let permission_sub =  FIRST(FOR m in member
FILTER m._key == '%s'
// Find has member is in:
FOR hm IN has_member
    FILTER hm._to == m._id
    // Find has_group of each group
    FOR hg in has_group
        FILTER hg._to == hm._from
        // parent_project <- project_childof <- child_project
        // take all projects whose parent is bind project and current project
        // it means each project must be in some group
        // this solves authorization!
        // TODO: currently this will work for project collection only(FOR t IN project), can be modified for any collection
        FILTER LENGTH(FOR v, e, p IN 0..9999 INBOUND hg._from project_childof
                      FILTER e._to == CONCAT('project/', t._key) || v._id == t._id
                      LIMIT 1 RETURN true) == 1

        // Find has_roles in each group
        FOR v IN 0..9999 INBOUND hg._from project_childof
            FOR hg2 in has_group
                FILTER hg2.from == v._to
                FOR gr in has_role
                    FILTER gr._from == hg2._to
                    // Find roles
                    FOR rp in has_permission
                      FILTER rp._from == gr._to AND rp._to == CONCAT("permission/", "%s")
                      FILTER rp.properties['list'] == true
                      LIMIT 1
                      RETURN true)

    FILTER permission_sub == true
)aql";
    if (memberKey.empty()) { return {ok::ErrorCode::ERROR_BAD_PARAMETER, ""}; }
    return {ok::ErrorCode::ERROR_NO_ERROR, "\n" + wrapper_indent + default_indent + absl::StrFormat(y, memberKey, collection)};  // collection = schemaKey
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, ""};
}
std::tuple<ErrorCode, Collection, std::string> getCollection(VPackSlice const &schemaJson) noexcept
{
  Collection collection;
  std::string as{"t"};
  if (schemaJson.isObject() && schemaJson.hasKey("c")) { collection = schemaJson.get("c").copyString(); }
  else
  {
    LOG_DEBUG << "collection key must exist in schema";
    return {ok::ErrorCode::SCHEMA_ERROR, collection, as};
  }
  if (schemaJson.isObject() && schemaJson.hasKey("as") && schemaJson.get("as").isString()) { as = schemaJson.get("as").copyString(); }
  return {ok::ErrorCode::ERROR_NO_ERROR, collection, as};
}
jsoncons::ojson getTableProps(VPackSlice const &schemaJson) noexcept
{
  jsoncons::ojson tableProps;
  if (schemaJson.isObject() && schemaJson.hasKey("p") && schemaJson.get("p").isObject()) { tableProps = jsoncons::ojson::parse(schemaJson.get("p").toJson()); }
  return tableProps;
}
std::tuple<bool, bool> getPermissions(VPackSlice const &schemaJson) noexcept
{
  bool childPermissionCheck{true};
  bool adminPermissionCheck{true};
  if (schemaJson.isObject() && schemaJson.hasKey("per") && schemaJson.get("per").isObject())
  {
    if (schemaJson.get("per").get("admin").isBool()) { adminPermissionCheck = schemaJson.get("per").get("admin").getBool(); }
    // don't worry child permission is not checked when 'project' key is
    // empty
    if (schemaJson.get("per").get("child").isBool()) { childPermissionCheck = schemaJson.get("per").get("child").getBool(); }
  }
  return {adminPermissionCheck, childPermissionCheck};
}
std::tuple<ok::ErrorCode, ok::mutate_schema::Relation> processTree(VPackSlice const &schemaJson, Collection &collection) noexcept
{
  ok::mutate_schema::Relation tree;
  if (schemaJson.isObject() && schemaJson.hasKey("tree"))
  {
    auto obj = schemaJson.get("tree");
    if (obj.hasKey("e")) { tree.edge = obj["e"].copyString(); }
    else
    {
      LOG_DEBUG << "tree node must contain edge";
      return {ok::ErrorCode::SCHEMA_ERROR, tree};
    }
    if (obj.hasKey("from")) { tree.from = obj["from"].copyString(); }
    if (obj.hasKey("to")) { tree.to = obj["to"].copyString(); }
    if (obj.hasKey("di"))
    {
      auto di = obj["di"].copyString();
      if (di == "in") { tree.direction = TRI_EDGE_IN; }
      else if (di == "out")
      {
        tree.direction = TRI_EDGE_OUT;
      }
    }
    if (tree.direction == TRI_EDGE_OUT)
    {
      if (tree.to.empty())
      {
        LOG_DEBUG << "relation to key must defined when direction is out";
        return {ok::ErrorCode::SCHEMA_ERROR, {}};
      }
      if (tree.from.empty()) { tree.from = collection; }
    }
    else if (tree.direction == TRI_EDGE_IN)
    {
      if (tree.from.empty())
      {
        LOG_DEBUG << "relation from key must defined when direction is in";
        return {ok::ErrorCode::SCHEMA_ERROR, {}};
      }
      if (tree.to.empty()) { tree.to = collection; }
    }
    // type = t
    // default = df
    // direction = di
    // global_event = ge
    // user_event = ue
    // information = i
    if (obj.hasKey("ge")) { tree.globalKey = obj["ge"].copyString(); }
    if (obj.hasKey("ue")) { tree.userKey = obj["ue"].copyString(); }
    else
    {
      tree.userKey = "";
    }
    if (obj.hasKey("i")) { tree.information = obj["i"].getBool(); }
    // if (!tree.information)
    //{
    // relationQueryGenerate(obj, tree);
    //}
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, tree};
}
namespace process_column
{
ID id(VPackSlice const &obj)
{
  if (obj.hasKey("id") && obj.get("id").isString()) { return obj.get("id").copyString(); }
  else
  {
    return "";
  }
}
ErrorCode dataPart(VPackSlice const &schemaJson,
                   Columns &columns,
                   ColumnOrder &allIds,
                   jsoncons::ojson const &args,
                   ok::mutate_schema::Relation const &tree,
                   WrapperFilters &wrapperFilters,
                   Database const &database,
                   DocumentKey const &memberKey,
                   Collection const &collection,
                   std::string const &as,
                   bool isPermission) noexcept
{
  Column columnKey;
  columnKey.label = "Key";
  columnKey.selector = "_key";
  columnKey.id = "_key";
  columnKey.visible_type = ColumnDisplayType::Text;
  columns.insert({columnKey.id, columnKey});
  allIds.push_back(columnKey.id);
  Column columnRev;
  columnRev.label = "Rev";
  columnRev.selector = "_rev";
  columnRev.id = "_rev";
  columnRev.visible_type = ColumnDisplayType::Text;
  columns.insert({columnRev.id, columnRev});
  allIds.push_back(columnRev.id);
  if (schemaJson.hasKey("d"))
  {
    if (auto data = schemaJson["d"]; !data.isArray()) { LOG_DEBUG << "Json fields schema must be an array. current type is: " << data.typeName() << " | json :" << data.toString(); }
    else
    {
      for (auto const &it : arangodb::velocypack::ArrayIterator(data))
      {
        if (!it.isObject())
        {
          LOG_DEBUG << "Json fields schema elememnt must me an object. current type is: " << it.typeName() << " | json :" << it.toString();
          continue;
        }
        Column c;
        c.isVisible = true;
        if (it.hasKey("cl"))
        {
          if (auto [er, calc] = calculated(args, jsoncons::ojson::parse(it.get("cl").toJson()), c, tree, wrapperFilters, database, memberKey, collection, as, isPermission); ok::isEr(er))
          {
            return er;
          }
          else
          {
            c.calculated = std::move(calc);
          }
        }
        if (it.hasKey("prefix") && it.get("prefix").isString()) { c.prefix = it.get("prefix").copyString(); }
        if (auto er = selector(it, c); ok::isEr(er)) { return er; }
        auto id_ = id(it);
        if (id_.empty()) c.id = c.selector;
        else
          c.id = id_;
        visibleType(it, c);
        editable(it, c);
        props(it, c);
        columns.insert({c.id, c});
        allIds.push_back(c.id);
      }
    }
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
ErrorCode selector(VPackSlice const &obj, Column &c) noexcept
{
  if (obj.hasKey("s"))
  {
    c.selector = obj.get("s").copyString();
    if (!c.selector.empty()) { return ok::ErrorCode::ERROR_NO_ERROR; }
  }
  return ok::ErrorCode::SCHEMA_ERROR;
}
void visibleType(VPackSlice const &obj, Column &c) noexcept
{
  if (obj.hasKey("v"))
  {
    auto v = obj.get("v").stringView();
    if (v == "bool") { c.visible_type = ColumnDisplayType::Checkbox; }
    else if (v == "number")
    {
      c.visible_type = ColumnDisplayType::Number;
    }
    else if (v == "double")
    {
      c.visible_type = ColumnDisplayType::Double;
    }
    else if (v == "date")
    {
      c.visible_type = ColumnDisplayType::Date;
    }
    else if (v == "datetime")
    {
      c.visible_type = ColumnDisplayType::DateTime;
    }
    else if (v == "color")
    {
      c.visible_type = ColumnDisplayType::Color;
    }
    else if (v == "url")
    {
      c.visible_type = ColumnDisplayType::Url;
    }
    else
    {
      c.visible_type = ColumnDisplayType::Text;
    }
  }
  else
  {
    c.visible_type = ColumnDisplayType::Text;
  }
}
void editable(VPackSlice const &obj, Column &c) noexcept
{
  if (obj.hasKey("e") && obj.get("e").isObject())
  {
    c.editableOptions["s"] = c.selector;
    if (obj.get("e").hasKey("t") && obj.get("e").get("t").isString())
    {
      auto type = obj.get("e").get("t").copyString();
      if (auto type_magic_enum = magic_enum::enum_cast<ok::mutate_schema::FieldType>(type); type_magic_enum.has_value())
      {
        c.editableOptions["t"] = static_cast<unsigned int>(type_magic_enum.value());
      }
      else
      {
        c.editableOptions["t"] = static_cast<unsigned int>(ok::mutate_schema::FieldType::text);
        LOG_DEBUG << "Value not Exist for editable type: " << type << " | selector: " << c.selector;
      }
    }
    else
    {
      c.editableOptions["t"] = static_cast<unsigned int>(ok::mutate_schema::FieldType::text);
    }
  }
}
void props(VPackSlice const &obj, Column &c) noexcept
{
  if (obj.hasKey("p")) { c.props = jsoncons::ojson::parse(obj.get("p").toJson()); }
  if (jsoncons::ObjectMemberIsString(c.props, "l"))
  {
    c.label = c.props["l"].as_string_view();
    c.props.erase("l");
  }
  if (c.label.empty()) { c.label = pystring::first_upper(pystring::replace(c.id, "_", " ")); }
}
std::tuple<ErrorCode, AQLQuery> getCalculatedChildrenCountNew(jsoncons::ojson const &args,
                                                              std::string const &key,
                                                              jsoncons::ojson const &filterSchema,
                                                              ok::mutate_schema::Relation const &tree,
                                                              WrapperFilters &wrapperFilters,
                                                              Database const &database,
                                                              DocumentKey const &memberKey,
                                                              Collection const &collection,
                                                              std::string const &as,
                                                              bool isPermission) noexcept
{
  constexpr auto y = R"aql(LENGTH(FOR %s IN 1..1 %s %s %s%s RETURN %s))aql";
  const auto edge = [&]()
  {
    if (jsoncons::ObjectMemberIsString(filterSchema, "edge")) { return filterSchema["edge"].as_string_view(); }
    else
      return std::string_view{};
  }();
  if (edge.empty()) { return {ok::ErrorCode::SCHEMA_ERROR, ""}; }
  std::string direction{key == "incount" ? "INBOUND" : "OUTBOUND"};
  std::string filter;
  if (jsoncons::ObjectMemberIsArray(filterSchema, "filter"))
  {
    if (auto [er, filter_, _] = getAllFilters(tree, args, filterSchema["filter"], wrapperFilters, database, memberKey, collection, as, isPermission, ""); ok::isEr(er)) { return {er, ""}; }
    else
      filter = std::move(filter_);
  }
  std::string vertex{jsoncons::ObjectMemberIsString(filterSchema, "vertex") ? filterSchema["vertex"].as_string() : "v"};
  return {ok::ErrorCode::ERROR_NO_ERROR, absl::StrFormat(y, vertex, direction, as, edge, filter, vertex)};
}
std::tuple<ErrorCode, AQLQuery> calculated(jsoncons::ojson const &args,
                                           jsoncons::ojson const &obj,
                                           Column &c,
                                           ok::mutate_schema::Relation const &tree,
                                           WrapperFilters &wrapperFilters,
                                           Database const &database,
                                           DocumentKey const &memberKey,
                                           Collection const &collection,
                                           std::string const &as,
                                           bool isPermission) noexcept
{
  std::string calc;
  if (obj.is_string()) { return {ok::ErrorCode::ERROR_NO_ERROR, obj.as_string()}; }
  else if (obj.is_array())
  {
    for (auto const &item : obj.array_range())
    {
      if (item.is_array())
      {
        if (jsoncons::ArrayPosIsString(item, 0))
        {
          if (auto func = item[0].as_string_view(); func == "tree")
          {
            // set router arguments: "params": { "fetchConfig": { "tree": true } } else count will be _key
            if (jsoncons::getBoolOption(args, "tree", false) && jsoncons::ArrayPosIsArray(item, 1))
            {
              if (auto [err, calc_] = calculated(args, item[1], c, tree, wrapperFilters, database, memberKey, collection, as, isPermission); ok::isEr(err)) { return {err, ""}; }
              else
              {
                return {ok::ErrorCode::ERROR_NO_ERROR, calc_};
              }
            }
          }
          else if (func == "incount" || func == "outcount")
          {
            if (auto [err, calc_] = getCalculatedChildrenCount(args, item, tree, wrapperFilters, database, memberKey, collection, as, isPermission); ok::isEr(err)) { return {err, ""}; }
            else
            {
              calc += std::move(calc_);
            }
          }
        }
      }
      else if (item.is_string())
      {
        calc += item.as_string();
      }
    }
  }
  else if (obj.is_object())
  {
    for (auto const &item : obj.object_range())
    {
      if (item.key() == "incount" || item.key() == "outcount")
      {
        if (auto [err, calc_] = getCalculatedChildrenCountNew(args, item.key(), item.value(), tree, wrapperFilters, database, memberKey, collection, as, isPermission); ok::isEr(err))
        {
          return {err, ""};
        }
        else
        {
          calc += std::move(calc_);
        }
      }
    }
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, calc};
}
std::tuple<ErrorCode, AQLQuery> getCalculatedChildrenCount(jsoncons::ojson const &args,
                                                           jsoncons::ojson const &filterSchema,
                                                           ok::mutate_schema::Relation const &tree,
                                                           WrapperFilters &wrapperFilters,
                                                           Database const &database,
                                                           DocumentKey const &memberKey,
                                                           Collection const &collection,
                                                           std::string const &as,
                                                           bool isPermission) noexcept
{
  constexpr auto y = R"aql(LENGTH(FOR %s IN 1..1 %s %s %s%s RETURN %s))aql";
  const auto edge = [&]()
  {
    if (jsoncons::ArrayPosIsString(filterSchema, 2)) { return filterSchema[2].as_string_view(); }
    else
      return std::string_view{};
  }();
  if (edge.empty()) { return {ok::ErrorCode::SCHEMA_ERROR, ""}; }
  std::string direction;
  if (jsoncons::ArrayPosIsString(filterSchema, 0)) { direction = filterSchema[0].as_string_view() == "incount" ? "INBOUND" : "OUTBOUND"; }
  if (direction.empty()) { return {ok::ErrorCode::SCHEMA_ERROR, ""}; }
  std::string filter;
  if (jsoncons::ArrayPosIsArray(filterSchema, 3))
  {
    if (auto [er, filter_, _] = getAllFilters(tree, args, filterSchema[3], wrapperFilters, database, memberKey, collection, as, isPermission, ""); ok::isEr(er)) { return {er, ""}; }
    else
    {
      filter = std::move(filter_);
    }
  }
  std::string vertex;
  if (jsoncons::ArrayPosIsString(filterSchema, 1)) { vertex = filterSchema[1].is_string() ? filterSchema[1].as_string() : "v"; }
  if (vertex.empty()) { return {ok::ErrorCode::SCHEMA_ERROR, ""}; }
  return {ok::ErrorCode::ERROR_NO_ERROR, absl::StrFormat(y, vertex, direction, as, edge, filter, vertex)};
}
void hidden(VPackSlice const &schemaJson, Columns &columns) noexcept
{
  if (schemaJson.hasKey("h") && schemaJson["h"].isArray() && schemaJson["h"].length() > 0)
  {
    auto h = schemaJson["h"];
    for (auto const &it : arangodb::velocypack::ArrayIterator(h))
    {
      for (auto &c : columns)
      {
        if (c.second.selector == it.copyString()) { c.second.isVisible = false; }
      }
    }
  }
}
Columns orderManual(Columns &columns, VPackSlice const &orderSlice) noexcept
{
  Columns columnsNew;
  for (auto const &it : arangodb::velocypack::ArrayIterator(orderSlice))
  {
    if (auto c = std::find_if(std::begin(columns), std::end(columns), [&](std::pair<ID, Column> const &c) { return c.first == it.copyString(); }); c != std::end(columns))
    {
      columnsNew.insert({c->first, c->second});
      columns.erase(c);
    }
  }
  for (auto &c : columns) { c.second.skipFrontEnd = true; }
  columnsNew.insert(std::begin(columns), std::end(columns));
  return columnsNew;
}
/*void processColumnOrder(VPackSlice const &schemaJson, Columns &columns) noexcept
{
  if (schemaJson.hasKey("o") && schemaJson["o"].isArray() && schemaJson["o"].length() > 0)
  {
    auto orderSlice = schemaJson["o"];
    columns = processColumnOrderManual(columns, orderSlice);  // skipFrontEnd should be not changed.
  }
}*/
ErrorCode sort(VPackSlice const &schemaJson, Columns &columns, Sort &sort) noexcept
{
  if (schemaJson.hasKey("so") && schemaJson["so"].isArray() && schemaJson["so"].length() > 0)
  {
    auto so = schemaJson["so"];
    for (auto const &it : arangodb::velocypack::ArrayIterator(so))
    {
      if (it.isArray() && it.length() == 2 && it[0].isString() && it[1].isString())
      {
        if (auto c = std::find_if(std::begin(columns), std::end(columns), [&](std::pair<ID, Column> const &c) { return c.first == it[0].copyString(); }); c != std::end(columns))
        {
          if (it[1].copyString() == "a") { sort.push_back({c->first, Ascending}); }
          else
          {
            sort.push_back({c->first, Descending});
          }
        }
      }
      else
      {
        // LOG_DEBUG << "Array Length should be 2 for short";
        return ok::ErrorCode::SCHEMA_ERROR;
      }
    }
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
}  // namespace process_column
}  // namespace impl
}  // namespace ok::table_schema
