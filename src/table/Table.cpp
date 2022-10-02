#include "table/Table.hpp"
#include "table/TableSchemaExternal.hpp"
#include "mutate/MutateSchema.hpp"
#include <trantor/utils/Logger.h>
#include <velocypack/Collection.h>
#include <jsoncons/json.hpp>
#include "db/DatabaseApi.hpp"
#include "utils/BatchArrayMessage.hpp"
#include "utils/GlobalSettings.hpp"
#include "db/Permission.hpp"
#include "db/Session.hpp"
#include "utils/json_functions.hpp"
#include "utils/ErrorConstants.hpp"
#include "actor_system/Routes.hpp"
#include "db/db_functions.hpp"
namespace ok
{
namespace table_actor
{
bool isDefaultFormValue(WsArguments const &args) noexcept
{
  if (args.contains("f") && args["f"].empty()) return true;
  return false;
}
std::tuple<ErrorCode, ok::table_schema::Columns, ok::mutate_schema::Fields, AQLQuery, jsoncons::ojson> getData(WsArguments const &args,
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
                                                                                                               bool isPermissionCheck) noexcept
{
  Collection formCollection;
  ok::mutate_schema::Fields fields;
  ok::mutate_schema::FieldOrder allIds;
  jsoncons::ojson form_options;
  ok::ErrorCode err_p = ok::ErrorCode::ERROR_NO_ERROR;
  if (isPermissionCheck)
  {
    err_p = impl::checkPermission(database, memberKey, projectKey, schemaKey, is_form, isMutateSchemaKeyIsCollectionKey);
    if (ok::isEr(err_p) && err_p != ok::ErrorCode::ERROR_HTTP_UNAUTHORIZED) { return {err_p, {}, fields, "", {}}; }
  }
  auto [er, collection, tree, columns, allSelectors, wrapperFilters, sort, filterTop, filterBottom, queryGeneratedForUnAuth, tableProps] =
      ok::table_schema::initialize(args, columnSchemaJson, database, memberKey, !ok::isEr(err_p), true);
  // colSchema->processColumnHiddenOrderSort(); now doing this on constructor.
  if (ok::isEr(er)) { return {er, columns, fields, "", {}}; }
  ok::table_schema::TableWhere where;
  AQLQuery as{"t"};
  if (is_form)
  {
    if (isMutateSchemaKeyIsCollectionKey)
    {
      // Todo check permissions.// currently its allowed.
      if (auto [er] = ok::table_schema::setFilterConfig(args, columns, where); ok::isEr(er)) { return {er, columns, fields, "", {}}; }
      if (auto [er, formCollection_, tree, fields_, allIds_, queryParts, form_options_] = impl::loadFormSchema(ok::table_schema::getKeyFromWhere(where), args, memberKey); ok::isEr(er))
      {
        return {er, columns, fields, "", {}};
      }
      else
      {
        formCollection = std::move(formCollection_);
        fields = std::move(fields_);
        allIds = std::move(allIds_);
        form_options = std::move(form_options_);
      }
    }
    else
    {
      if (ok::isEr(err_p)) { return {err_p, columns, fields, "", {}}; }
      if (auto [er, formCollection_, tree, fields_, allIds_, queryParts, form_options_] = impl::loadFormSchema(schemaKey, args, memberKey); ok::isEr(er)) { return {er, columns, fields, "", {}}; }
      else
      {
        formCollection = std::move(formCollection_);
        fields = std::move(fields_);
        allIds = std::move(allIds_);
        form_options = std::move(form_options_);
      }
    }
  }
  else if (ok::isEr(err_p) && !queryGeneratedForUnAuth)
  {
    return {ok::ErrorCode::ERROR_HTTP_UNAUTHORIZED, columns, fields, "", {}};
  }
  /*if (getOption("pdf")) {
    // to pdf actor...
    // Send Pdf Download ...
    // oh, prevent it to subscribe.
    return;
  }
  if (getOption("csv")) {
    // Send csv ...
    return;
  }*/
  std::string selector;
  // auto as{"t"};
  if (is_form)
  {
    if (auto [er, selector_] = ok::mutate_schema::table_actor::makeFormFieldsObjectProjection(fields, allIds, formCollection, as); ok::isEr(er)) { return {er, columns, fields, "", {}}; }
    else
      selector = selector_;
  }
  else
  {
    // selector = colSchema->as;
    // selector = as;
  }
  ok::table_schema::Pagination pagination;
  AQLQuery query;
  ok::ErrorCode erQ;
  if (changeQuery.empty())
  {
    if (auto [er] = ok::table_schema::setFilterConfig(args, columns, where); ok::isEr(er)) { return {er, columns, fields, "", {}}; }
    if (auto [er] = ok::table_schema::setSortConfig(args, columns, sort); ok::isEr(er)) { return {er, columns, fields, "", {}}; }
    if (auto [er] = ok::table_schema::setPaginationConfig(args, pagination); ok::isEr(er)) { return {er, columns, fields, "", {}}; }
    auto [erDef, default_where] = ok::table_schema::setDefaultWhereArray(filterKey, as);
    if (ok::isEr(erDef)) { return {erDef, columns, fields, "", {}}; }
    std::tie(erQ, query) = ok::table_schema::buildQuery(selector, columns, args, tree, wrapperFilters, where, sort, pagination, as, collection, tree.edge, filterTop, filterBottom, default_where);
    if (ok::isEr(erQ)) { return {erQ, columns, fields, query, {}}; }
  }
  else
  {
    query = changeQuery;
  }
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  if (!filterKey.empty())
  {
    arangodb::velocypack::ArrayBuilder a{&bindVars, "filter"};
    for (auto const &f : filterKey) { bindVars.add(arangodb::velocypack::Value(f)); }
  }
  bindVars.close();
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  if (isEr(erDb))
  {
    if (response)
    {
      auto slice = response->slices().front();
      LOG_FATAL << "Error: " << slice.toJson();
    }
    LOG_FATAL << "Error: " << errno_string(erDb) << " | executed query: " << query << " | database: " << database << "| bindVars: " << bindVars.toString();
    return {ok::ErrorCode::BAD_QUERY, columns, fields, query, {}};
  }
  auto slice = response->slices().front();
  if (ok::smart_actor::connection::getPrintVal("table")) LOG_DEBUG << "executed query: " << query << " | database: " << database << " | Result: " << slice.toJson();
  if (is_form && slice.get("result").length() == 0 && isMutateSchemaKeyIsCollectionKey)
  {
    // currently, only doing for array form:
    auto h = ok::mutate_schema::table_actor::getFormHeaderData(fields, allIds, form_options, jsoncons::ojson::array());
    auto r = ok::mutate_schema::table_actor::getFormDefaultObject(fields, allIds);
    h.merge(r);
    return {ok::ErrorCode::ERROR_NO_ERROR, columns, fields, query, h};
  }
  jsoncons::ojson h = jsoncons::ojson::object();
  jsoncons::ojson r = jsoncons::ojson::object();
  if (filterKey.empty())
  {
    r = impl::processNormalResult(slice, pagination);
    if (!r["r"]["result"].is_array())
    {
      LOG_ERROR << "table result must be array: " << r.to_string();
      return {ok::ErrorCode::ERROR_INTERNAL, columns, fields, query, r};
    }
    if (is_form) h = ok::mutate_schema::table_actor::getFormHeaderData(fields, allIds, form_options, r["r"]["result"]);
    else if (sendHeader)
      h = ok::table_schema::getJsonHeaderData(columns, allSelectors, sort, tableProps);
  }
  else
  {
    r = impl::processChangedResult(slice, savedKeys, filterKey);
  }
  // if (slice.get("result").length() == 0 && !sendEmpty)  // not send empty
  // return {erDb::ErrorCode::ERROR_NO_ERROR, columns, fields, query, h, jsoncons::ojson::null()};
  h.merge(r);
  if (r.contains("r") && jsoncons::ObjectMemberIsArray(r["r"], "result"))
  {
    if (r["r"]["result"].empty() && !sendEmpty)  // not send empty
      return {ok::ErrorCode::ERROR_NO_ERROR, columns, fields, query, h};
    auto er = impl::saveKeys(r["r"]["result"], savedKeys);
    if (ok::isEr(er)) return {er, columns, fields, query, h};
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, columns, fields, query, h};
}
std::tuple<ErrorCode, ok::mutate_schema::Fields, ok::mutate_schema::FieldOrder, jsoncons::ojson> fetchFormDefaultInitialize(
    Database const &database, DocumentKey const &memberKey, DocumentKey const &projectKey, SchemaKey const &schemaKey, jsoncons::ojson const &args) noexcept
{
  // using "category" = schemaKey
  if (auto er = impl::checkPermission(database, memberKey, projectKey, schemaKey, "create"); ok::isEr(er)) return {er, {}, {}, {}};
  /*if (valueType == arangodb::velocypack::ValueType::Object)
  {
    jsoncons::ojson r;
    auto [erS, myRespFormSchema] = ok::db::getSchema("global", "schema", schemaKey, ".form");
    if (ok::isEr(erS)) { return {erS, {}, {}, {}}; }
    else
    {
      auto slice = myRespFormSchema->slices().front().get("result");
      r["r"]["result"] = jsoncons::ojson::parse(slice.toJson());
    }
    return {ok::ErrorCode::ERROR_NO_ERROR, {}, {}, r};  // Fix this to pass allIds
  }*/
  auto [erS, myRespFormSchema] = ok::db::getSchema("global", "schema", schemaKey, ".fields");
  if (ok::isEr(erS)) { return {ok::ErrorCode::CANNOT_LOAD_SCHEMA, {}, {}, jsoncons::ojson::array()}; }
  else
  {
    auto formSchemaJson = myRespFormSchema->slices().front().get("result")[0];
    // database, memberKey
    auto [er2, collection_, tree_, fields, allIds] = ok::mutate_schema::initialize(formSchemaJson, ok::smart_actor::connection::MutateEventType::insert);
    if (ok::isEr(er2)) return {er2, {}, {}, jsoncons::ojson::array()};
    ok::mutate_schema::MutateQueryParts queryParts;
    if (auto [er3, fieldsNew, allIdsNew] =
            ok::mutate_schema::processDataPart(formSchemaJson, args, ok::smart_actor::connection::MutateEventType::insert, collection_, tree_, queryParts, memberKey, true);
        ok::isEr(er3))
      return {er3, {}, {}, jsoncons::ojson::array()};
    else
    {
      fields.insert(std::begin(fieldsNew), std::end(fieldsNew));
      allIds.insert(std::end(allIds), std::begin(allIdsNew), std::end(allIdsNew));
    }
    auto [er4, options] = ok::mutate_schema::processFormProps(formSchemaJson, fields, memberKey);
    if (ok::isEr(er4)) return {er4, fields, allIds, options};
    // auto header = ok::mutate_schema::getFormHeaderData(fields_, options);
    // auto data = ok::mutate_schema::getFormDefaultArray(fields_);
    return {ok::ErrorCode::ERROR_NO_ERROR, fields, allIds, options};
  }
}
namespace impl
{
ErrorCode checkPermission(Database const &database, DocumentKey const &memberKey, DocumentKey const &projectKey, SchemaKey const &schemaKey, const std::string &property) noexcept
{
  return ok::permission::checkPermission(database, memberKey, projectKey, schemaKey, property, true, true);
}
ErrorCode checkPermission(
    Database const &database, DocumentKey const &memberKey, DocumentKey const &projectKey, SchemaKey const &schemaKey, bool is_form, bool isMutateSchemaKeyIsCollectionKey) noexcept
{
  if (is_form)
  {
    if (!isMutateSchemaKeyIsCollectionKey) { return checkPermission(database, memberKey, projectKey, schemaKey, "edit"); }
    else
    {
      return ok::ErrorCode::ERROR_NO_ERROR;
    }
  }
  else
  {
    return checkPermission(database, memberKey, projectKey, schemaKey, "list");
  }
}
std::tuple<ErrorCode, Collection, ok::mutate_schema::Relation, ok::mutate_schema::Fields, ok::mutate_schema::FieldOrder, ok::mutate_schema::MutateQueryParts, jsoncons::ojson> loadFormSchema(
    const std::string &schemaKey, jsoncons::ojson const &args, DocumentKey const &memberKey) noexcept
{
  auto [erS, myRespFormSchema] = ok::db::getSchema("global", "schema", schemaKey, ".fields");
  if (ok::isEr(erS)) { return {erS, "", {}, {}, {}, {}, {}}; }
  else
  {
    auto formSchemaJson = myRespFormSchema->slices().front().get("result")[0];
    // database, memberKey
    auto [er2, collection, tree, fields, allIds] = ok::mutate_schema::initialize(formSchemaJson, ok::smart_actor::connection::MutateEventType::update);
    if (ok::isEr(er2)) { return {er2, collection, tree, fields, allIds, {}, {}}; }
    ok::mutate_schema::MutateQueryParts queryParts;
    auto [er3, fieldsNew, allIdsNew] = processDataPart(formSchemaJson, args, ok::smart_actor::connection::MutateEventType::update, collection, tree, queryParts, memberKey, true);
    if (ok::isEr(er3)) { return {er3, collection, tree, fields, allIds, queryParts, {}}; }
    else
    {
      fields.insert(std::begin(fieldsNew), std::end(fieldsNew));
      allIds.insert(std::end(allIds), std::begin(allIdsNew), std::end(allIdsNew));
    }
    auto [er4, options] = ok::mutate_schema::processFormProps(formSchemaJson, fields, memberKey);
    if (ok::isEr(er4)) { return {er4, collection, tree, fields, allIds, queryParts, {}}; }
    return {ok::ErrorCode::ERROR_NO_ERROR, collection, tree, fields, allIds, queryParts, options};
  }
}
jsoncons::ojson processNormalResult(VPackSlice const &slice, ok::table_schema::Pagination const &pagination)
{
  jsoncons::ojson r;
  r["r"] = jsoncons::ojson::object();
  auto b_ = r["r"];
  b_["result"] = jsoncons::ojson::parse(slice.get("result").toString());
  b_["hasMore"] = slice.get("hasMore").getBool();
  b_["count"] = slice.get("count").getInt();
  b_["fullCount"] = slice.get("extra").get("stats").get("fullCount").getUInt();
  b_["pagination"] = ok::table_schema::getPaginationArray(pagination);
  return r;
}
jsoncons::ojson processChangedResult(VPackSlice const &slice, std::unordered_set<DocumentKey> &savedKeys, std::unordered_set<DocumentKey> const &filterKey)
{
  jsoncons::ojson inserted = jsoncons::ojson::array();
  jsoncons::ojson modified = jsoncons::ojson::array();
  jsoncons::ojson deleted = jsoncons::ojson::array();
  auto resultSlice = slice.get("result");
  for (auto &fKey : filterKey)
  {
    if (bool isNewResultHasKey = isResultContainsKey(resultSlice, fKey); savedKeys.contains(fKey) && isNewResultHasKey)
    {
      for (auto const &it : arangodb::velocypack::ArrayIterator(resultSlice))
      {
        if (it.isObject() && it.hasKey("_key") && it["_key"].isString() && it["_key"].copyString() == fKey)
        {
          modified.push_back(jsoncons::ojson::parse(it.toString()));
          break;
        }
      }
    }
    else if (savedKeys.contains(fKey) && !isNewResultHasKey)
    {
      deleted.push_back(fKey);
      savedKeys.erase(fKey);
    }
    else if (!savedKeys.contains(fKey) && isNewResultHasKey)
    {
      for (auto const &it : arangodb::velocypack::ArrayIterator(resultSlice))
      {
        if (it.isObject() && it.hasKey("_key") && it["_key"].isString() && it["_key"].copyString() == fKey)
        {
          inserted.push_back(jsoncons::ojson::parse(it.toString()));
          break;
        }
      }
      savedKeys.insert(fKey);
    }
  }
  jsoncons::ojson r;
  if (inserted.size() > 0)
  {
    jsoncons::ojson o;
    o["result"] = inserted;
    r["n"] = o;
  }
  if (modified.size() > 0)
  {
    jsoncons::ojson o;
    o["result"] = modified;
    r["m"] = o;
  }
  if (deleted.size() > 0)
  {
    jsoncons::ojson o;
    o["result"] = deleted;
    r["d"] = o;
  }
  return r;
}
ErrorCode saveKeys(jsoncons::ojson &jsonResult, std::unordered_set<DocumentKey> &savedKeys) noexcept
{
  for (auto const &it : jsonResult.array_range())
  {
    if (it.is_array() && !it.empty())
    {
      if (it[0].is_string()) { savedKeys.insert(it[0].as_string()); }
      else if (it[0].is_array() && it[0].size() > 0 && it[0][0].is_string())
      {
        savedKeys.insert(it[0][0].as_string());
      }
      else
      {
        LOG_ERROR << "cant save keys! array first item must be string ";
        return ok::ErrorCode::BAD_QUERY;
      }
    }
    else if (it.is_object())
    {
      if (it.contains("_key")) { savedKeys.insert(it["_key"].as_string()); }
      else
      {
        LOG_ERROR << "cant save keys! objec must have _key member";
        return ok::ErrorCode::BAD_QUERY;
      }
    }
    else
    {
      LOG_ERROR << "cant save keys! result array should have at lest one string element" << jsonResult.to_string();
      return ok::ErrorCode::BAD_QUERY;
    }
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
bool isResultContainsKey(VPackSlice const &resultSlice, std::string const &key) noexcept
{
  return arangodb::velocypack::Collection::contains(resultSlice,
                                                    [&](VPackSlice const &slice, arangodb::velocypack::ValueLength index)
                                                    { return slice.isObject() && slice.hasKey("_key") && slice["_key"].isString() && slice["_key"].copyString() == key; });
}
}  // namespace impl
}  // namespace table_actor
}  // namespace ok
