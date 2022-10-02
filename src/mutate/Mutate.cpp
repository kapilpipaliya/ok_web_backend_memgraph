#include "mutate/Mutate.hpp"
#include <trantor/utils/Logger.h>
#include "utils/BatchArrayMessage.hpp"
#include "mutate/MutateSchema.hpp"
#include "utils/GlobalSettings.hpp"
#include "db/Permission.hpp"
#include "table/TableSchemaExternal.hpp"
#include "db/DatabaseApi.hpp"
#include "utils/json_functions.hpp"
#include <velocypack/Collection.h>
#include "utils/ErrorConstants.hpp"
#include "mutate/MutatePosition.hpp"
#include <magic_enum.hpp>
#include "db/db_functions.hpp"
#include "actor_system/Routes.hpp"
namespace ok
{
namespace mutate_actor
{
ok::smart_actor::connection::MutateEventType getMutateEventType(const WsEvent &event)
{
  auto const event_type = event[0].as_string_view();
  if (event_type == "insert") return ok::smart_actor::connection::MutateEventType::insert;
  if (event_type == "update") return ok::smart_actor::connection::MutateEventType::update;
  if (event_type == "replace") return ok::smart_actor::connection::MutateEventType::replace;
  if (event_type == "delete_") return ok::smart_actor::connection::MutateEventType::delete_;
  if (event_type == "batchUpdate") return ok::smart_actor::connection::MutateEventType::batchUpdate;
  if (event_type == "batchDelete") return ok::smart_actor::connection::MutateEventType::batchDelete;
  if (event_type == "changePosition") return ok::smart_actor::connection::MutateEventType::changePosition;
  return ok::smart_actor::connection::MutateEventType::insert;
}
std::tuple<ErrorCode, AQLQuery, ok::mutate_schema::MutateQueryParts, arangodb::velocypack::Builder, std::unordered_set<DocumentKey>, ok::mutate_schema::Fields> save(
    Database const &database,
    DocumentKey const &memberKey,
    DocumentKey const &schemaKey,
    VPackSlice const &schemaJson,
    VPackSlice const &columnSchemaJson,
    ok::smart_actor::connection::MutateEventType et,
    WsArguments const &args,
    WsArguments const &backendValue,
    bool permissionCheck,
    bool ignoreMissingFieldWithValue,
    MutateCallbackFunction preProcess,
    MutateCallbackFunction postProcess)
{
  std::string projectKey;
  ok::ErrorCode er;
  Collection collection;
  ok::mutate_schema::Relation tree;
  ok::mutate_schema::Fields fields;
  ok::mutate_schema::FieldOrder allIds;
  ok::mutate_schema::MutateQueryParts queryParts;
  jsoncons::ojson saveData;
  AQLQuery query;
  arangodb::velocypack::Builder mergeBindVars;
  ok::table_schema::Columns columns;
  ok::table_schema::TableWhere where;
  if (args.is_object() && jsoncons::ObjectMemberIsString(args, "project")) projectKey = args["project"].as_string();
  if (permissionCheck)
  {
    if (schemaJson.hasKey("per") && schemaJson["per"].isBool() && !schemaJson["per"].getBool()) {}
    else
    {
      if (auto er = ok::permission::checkPermission(database, memberKey, projectKey, schemaKey, et, true, true); ok::isEr(er)) return {er, "", queryParts, mergeBindVars, {}, {}};
      // now guest can also do mutation for chat.
      /*if (memberKey.empty())
      {
        LOG_DEBUG << "only logged in member can do any mutation";
        return {ok::ErrorCode::ERROR_HTTP_UNAUTHORIZED, query, queryParts, mergeBindVars, {}, fields};
      }*/
    }
  }
  switch (et)
  {
    case ok::smart_actor::connection::MutateEventType::insert:
    {
      std::tie(er, collection, tree, fields, queryParts, saveData) = impl::initializeInsert(database, memberKey, schemaJson, args, backendValue, ignoreMissingFieldWithValue);
      if (ok::isEr(er)) break;
      if (er = impl::locked_row_permission_check::insert(); ok::isEr(er)) break;
      if (er = impl::unique_check::insert(schemaJson, database, collection, fields); ok::isEr(er)) break;
      std::tie(er, query) = impl::make_query::insert(queryParts);
      if (ok::isEr(er)) break;
      std::tie(er, mergeBindVars) = impl::get_merged_bind_vars::insert(collection, saveData, fields, args, memberKey);
      if (ok::isEr(er)) break;
      break;
    }
    case ok::smart_actor::connection::MutateEventType::replace:
    {
      std::tie(er, collection, tree, columns, where) = impl::columnSchemaInitialize(database, memberKey, columnSchemaJson, args, false);
      if (ok::isEr(er)) break;
      if (jsoncons::ObjectMemberIsObject(args, "value"))
      {
        // database, memberKey
        std::tie(er, collection, tree, fields, allIds) = ok::mutate_schema::initialize(schemaJson, ok::smart_actor::connection::MutateEventType::update);
        if (ok::isEr(er)) break;
        auto [er_, fieldsNew, allIdsNew] =
            processDataPart(schemaJson, args, ok::smart_actor::connection::MutateEventType::update, collection, tree, queryParts, memberKey, ignoreMissingFieldWithValue);
        if (ok::isEr(er_))
        {
          er = er_;
          break;
        }
        else
        {
          fields.insert(std::begin(fieldsNew), std::end(fieldsNew));
          allIds.insert(std::end(allIds), std::begin(allIdsNew), std::end(allIdsNew));
        }
        er = ok::mutate_schema::make_bind_vars::setNonRelationFieldValues(fields, args["value"], backendValue, ignoreMissingFieldWithValue);
        if (ok::isEr(er)) break;
        std::tie(er, saveData) = ok::mutate_schema::make_bind_vars::makeFormObject(fields, allIds);
        if (ok::isEr(er)) break;
      }
      else
      {
        LOG_DEBUG << "Invalid Arguments shape";
        er = ok::ErrorCode::ERROR_BAD_PARAMETER;
        break;
      }
      arangodb::velocypack::Builder b;
      if (er = impl::locked_row_permission_check::update(schemaJson, args, collection, tree, database, columns, where); ok::isEr(er)) break;
      if (er = impl::unique_check::update(schemaJson, database, collection, fields, true); ok::isEr(er)) break;
      std::tie(er, query) = impl::make_query::update(queryParts, true);
      if (ok::isEr(er)) break;
      std::tie(er, mergeBindVars) = impl::get_merged_bind_vars::update(collection, saveData, fields, args, memberKey);
      if (ok::isEr(er)) break;
      break;
    }
    case ok::smart_actor::connection::MutateEventType::update:
    {
      std::tie(er, collection, tree, columns, where) = impl::columnSchemaInitialize(database, memberKey, columnSchemaJson, args, false);
      if (ok::isEr(er)) break;
      if (jsoncons::ObjectMemberIsObject(args, "value"))
      {
        // database, memberKey
        std::tie(er, collection, tree, fields, allIds) = ok::mutate_schema::initialize(schemaJson, ok::smart_actor::connection::MutateEventType::update);
        if (ok::isEr(er)) break;
        auto [er_, fieldsNew, allIdsNew] =
            processDataPart(schemaJson, args, ok::smart_actor::connection::MutateEventType::update, collection, tree, queryParts, memberKey, ignoreMissingFieldWithValue);
        if (ok::isEr(er_))
        {
          er = er_;
          break;
        }
        else
        {
          fields.insert(std::begin(fieldsNew), std::end(fieldsNew));
          allIds.insert(std::end(allIds), std::begin(allIdsNew), std::end(allIdsNew));
        }
        er = ok::mutate_schema::make_bind_vars::setNonRelationFieldValues(fields, args["value"], backendValue, ignoreMissingFieldWithValue);
        if (ok::isEr(er)) break;
        std::tie(er, saveData) = ok::mutate_schema::make_bind_vars::makeFormObject(fields, allIds);
        if (ok::isEr(er)) break;
      }
      else
      {
        LOG_DEBUG << "Invalid Arguments shape";
        er = ok::ErrorCode::ERROR_BAD_PARAMETER;
        break;
      }
      arangodb::velocypack::Builder b;
      if (er = impl::locked_row_permission_check::update(schemaJson, args, collection, tree, database, columns, where); ok::isEr(er)) break;
      if (er = impl::unique_check::update(schemaJson, database, collection, fields, false); ok::isEr(er)) break;
      std::tie(er, query) = impl::make_query::update(queryParts, false);
      if (ok::isEr(er)) break;
      std::tie(er, mergeBindVars) = impl::get_merged_bind_vars::update(collection, saveData, fields, args, memberKey);
      if (ok::isEr(er)) break;
      break;
    }
    case ok::smart_actor::connection::MutateEventType::batchUpdate:
    {
      std::tie(er, collection, tree, columns, where) = impl::columnSchemaInitialize(database, memberKey, columnSchemaJson, args, false);
      if (ok::isEr(er)) break;
      if (jsoncons::ObjectMemberIsObject(args, "value"))
      {
        // database, memberKey
        std::tie(er, collection, tree, fields, allIds) = ok::mutate_schema::initialize(schemaJson, ok::smart_actor::connection::MutateEventType::batchUpdate);
        if (ok::isEr(er)) break;
        auto [er_, fieldsNew, allIdsNew] =
            processDataPart(schemaJson, args, ok::smart_actor::connection::MutateEventType::batchUpdate, collection, tree, queryParts, memberKey, ignoreMissingFieldWithValue);
        if (ok::isEr(er_))
        {
          er = er_;
          break;
        }
        else
        {
          fields.insert(std::begin(fieldsNew), std::end(fieldsNew));
          allIds.insert(std::end(allIds), std::begin(allIdsNew), std::end(allIdsNew));
        }
        er = ok::mutate_schema::make_bind_vars::setNonRelationFieldValues(fields, args["value"], backendValue, ignoreMissingFieldWithValue);
        if (ok::isEr(er)) break;
        std::tie(er, saveData) = ok::mutate_schema::make_bind_vars::makeFormObject(fields, allIds);
        if (ok::isEr(er)) break;
      }
      else
      {
        LOG_DEBUG << "Invalid Arguments shape";
        er = ok::ErrorCode::ERROR_BAD_PARAMETER;
        break;
      }
      arangodb::velocypack::Builder b;
      if (er = impl::locked_row_permission_check::update(schemaJson, args, collection, tree, database, columns, where); ok::isEr(er)) break;
      if (er = impl::unique_check::update(schemaJson, database, collection, fields, false); ok::isEr(er)) break;
      std::tie(er, query) = impl::make_query::batchUpdate(queryParts, columns, where, false);
      if (ok::isEr(er)) break;
      std::tie(er, mergeBindVars) = impl::get_merged_bind_vars::update(collection, saveData, fields, args, memberKey);
      if (ok::isEr(er)) break;
      break;
    }
    case ok::smart_actor::connection::MutateEventType::changePosition:
    {
      Collection collection;
      mutate_schema::Relation tree;
      PositionData positionData;
      if (jsoncons::ObjectMemberIsArray(args, "value") && args["value"].size() == 5)  // [from, fromKey, to, toKey, position]
      {
        // database, memberKey
        std::tie(er, collection, tree, fields, allIds) = ok::mutate_schema::initialize(schemaJson, ok::smart_actor::connection::MutateEventType::changePosition);
        if (ok::isEr(er)) break;
        auto [er_, fieldsNew, allIdsNew] =
            processDataPart(schemaJson, args, ok::smart_actor::connection::MutateEventType::changePosition, collection, tree, queryParts, memberKey, ignoreMissingFieldWithValue);
        if (ok::isEr(er_))
        {
          er = er_;
          break;
        }
        else
        {
          fields.insert(std::begin(fieldsNew), std::end(fieldsNew));
          allIds.insert(std::end(allIds), std::begin(allIdsNew), std::end(allIdsNew));
        }
        saveData = args["value"];
        if (!(saveData[0].is_string() && saveData[1].is_string() && saveData[2].is_string() && saveData[3].is_string() && saveData[4].is_number()))
        {
          LOG_DEBUG << "Arguments is not valid. all argument must be strings " + saveData.to_string();
          er = ok::ErrorCode::ERROR_BAD_PARAMETER;
          break;
        }
      }
      else
      {
        LOG_DEBUG << "Invalid Arguments shape: " << args.to_string();
        er = ok::ErrorCode::ERROR_BAD_PARAMETER;
        break;
      }
      if (ok::isEr(er)) break;
      arangodb::velocypack::Builder b;
      // // Todo also check for locked row in future. to check locked row
      // permission have to initialize
      if (er = impl::locked_row_permission_check::position(); ok::isEr(er)) break;
      // no unique check
      std::tie(er, positionData) = preparePositionData(database, collection, tree, saveData);
      if (ok::isEr(er)) break;
      std::tie(er, query) = impl::make_query::position(positionData);
      if (ok::isEr(er)) break;
      std::tie(er, mergeBindVars) = impl::get_merged_bind_vars::position(collection, saveData, fields, args, positionData);
      if (ok::isEr(er)) break;
      auto [erC1, changedKeys] = manageRelation(database, collection, tree.edge, positionData);
      if (ok::isEr(er))
      {
        er = erC1;
        break;
      }
      auto [erC2, changedKeys2] = impl::update(query, mergeBindVars, database, queryParts.relationQueries);
      if (ok::isEr(er))
      {
        er = erC2;
        break;
      }
      else
      {
        changedKeys.insert(std::begin(changedKeys2), std::end(changedKeys2));
      }
      return {er, query, queryParts, mergeBindVars, changedKeys, fields};
    }
    case ok::smart_actor::connection::MutateEventType::delete_:
    {
      std::tie(er, collection, tree, columns, where) = impl::columnSchemaInitialize(database, memberKey, columnSchemaJson, args, false);
      if (ok::isEr(er)) break;
      if (jsoncons::ObjectMemberIsBool(args, "DEL") && args["DEL"].as_bool())
      {
        // database, memberKey
        std::tie(er, collection, tree, fields, allIds) = ok::mutate_schema::initialize(schemaJson, ok::smart_actor::connection::MutateEventType::delete_);
        if (ok::isEr(er)) break;
        auto [er_, fieldsNew, allIdsNew] =
            processDataPart(schemaJson, args, ok::smart_actor::connection::MutateEventType::delete_, collection, tree, queryParts, memberKey, ignoreMissingFieldWithValue);
        if (ok::isEr(er_))
        {
          er = er_;
          break;
        }
        else
        {
          fields.insert(std::begin(fieldsNew), std::end(fieldsNew));
          allIds.insert(std::end(allIds), std::begin(allIdsNew), std::end(allIdsNew));
        }
      }
      else
      {
        LOG_DEBUG << "First key should be 'DEL'";
        er = ok::ErrorCode::ERROR_BAD_PARAMETER;
        break;
      }
      arangodb::velocypack::Builder b;
      if (er = impl::locked_row_permission_check::delete_(schemaJson, args, collection, tree, database, columns, where); ok::isEr(er)) break;
      // no unique check
      std::tie(er, query) = impl::make_query::delete_(queryParts);
      if (ok::isEr(er)) break;
      std::tie(er, mergeBindVars) = impl::get_merged_bind_vars::delete_(collection, saveData, fields, args);
      if (ok::isEr(er)) break;
      break;
    }
    case ok::smart_actor::connection::MutateEventType::batchDelete:
    {
      std::tie(er, collection, tree, columns, where) = impl::columnSchemaInitialize(database, memberKey, columnSchemaJson, args, false);
      if (ok::isEr(er)) break;
      if (jsoncons::ObjectMemberIsBool(args, "DEL") && args["DEL"].as_bool())
      {
        // database, memberKey
        std::tie(er, collection, tree, fields, allIds) = ok::mutate_schema::initialize(schemaJson, ok::smart_actor::connection::MutateEventType::batchDelete);
        if (ok::isEr(er)) break;
        auto [er_, fieldsNew, allIdsNew] =
            processDataPart(schemaJson, args, ok::smart_actor::connection::MutateEventType::batchDelete, collection, tree, queryParts, memberKey, ignoreMissingFieldWithValue);
        if (ok::isEr(er_))
        {
          er = er_;
          break;
        }
        else
        {
          fields.insert(std::begin(fieldsNew), std::end(fieldsNew));
          allIds.insert(std::end(allIds), std::begin(allIdsNew), std::end(allIdsNew));
        }
      }
      else
      {
        LOG_DEBUG << "First key should be 'DEL'";
        er = ok::ErrorCode::ERROR_BAD_PARAMETER;
        break;
      }
      arangodb::velocypack::Builder b;
      if (er = impl::locked_row_permission_check::delete_(schemaJson, args, collection, tree, database, columns, where); ok::isEr(er)) break;
      // no unique check
      std::tie(er, query) = impl::make_query::batchDelete(queryParts, columns, where);
      if (ok::isEr(er)) break;
      std::tie(er, mergeBindVars) = impl::get_merged_bind_vars::delete_(collection, saveData, fields, args);
      if (ok::isEr(er)) break;
      break;
    }
    default:
    {
      er = ok::ErrorCode::ERROR_BAD_PARAMETER;
      break;
    }
  }
  if (ok::isEr(er)) return {er, query, queryParts, mergeBindVars, {}, fields};
  if (auto er = preProcess(database, args, et, fields); ok::isEr(er)) { return {er, query, queryParts, mergeBindVars, {}, fields}; }
  auto [erU, changedKeys] = impl::update(query, mergeBindVars, database, queryParts.relationQueries);
  if (ok::isEr(erU)) return {erU, query, queryParts, mergeBindVars, changedKeys, fields};
  else
  {
    postProcess(database, args, et, fields);  // be sure post Process dont yield any error not checking for errors. may be not appropriate to run postProcess() here.
    return {erU, query, queryParts, mergeBindVars, changedKeys, fields};
  }
}
bool isModifyEvent(ok::smart_actor::connection::MutateEventType eventType)
{
  if (eventType == ok::smart_actor::connection::MutateEventType::insert || eventType == ok::smart_actor::connection::MutateEventType::update ||
      eventType == ok::smart_actor::connection::MutateEventType::replace || eventType == ok::smart_actor::connection::MutateEventType::batchUpdate ||
      eventType == ok::smart_actor::connection::MutateEventType::changePosition)
    return true;
  else
    return false;
}
bool isDeleteEvent(ok::smart_actor::connection::MutateEventType eventType)
{
  if (eventType == ok::smart_actor::connection::MutateEventType::delete_ || eventType == ok::smart_actor::connection::MutateEventType::batchDelete) return true;
  else
    return false;
}
void runSameQueryOnAllUsers(AQLQuery const &query, const arangodb::velocypack::Builder &bindVars, const ok::mutate_schema::RelationQueryMap &relationQueries) noexcept
{
  // find all users
  jsoncons::ojson b = jsoncons::ojson::parse(bindVars.toJson());
  if (b.contains("insData")) b["insData"].erase("_rev");
  if (b.contains("userSuppliedDocument")) b["userSuppliedDocument"].erase("_rev");
  arangodb::velocypack::Builder newBindVars;
  VPackParser parser(newBindVars);
  parser.parse(b.to_string());
  constexpr auto queryAllDb = "FOR o IN user RETURN o.database";
  auto [erDb, response] = Api::Cursor::PostCursor::request("user", queryAllDb, {});
  if (isEr(erDb))
  {
    if (response)
    {
      auto slice = response->slices().front();
      LOG_FATAL << "Error: " << slice.toJson();
    }
    auto slice = response->slices().front();
    LOG_FATAL << "Error: " << errno_string(erDb) << slice.toJson() << " | executed query: " << query << " bindVars is: " << newBindVars.toString();
  }
  else
  {
    auto slice = response->slices().front();
    if (auto r2 = slice.get("result"); r2.isArray() && r2.length() > 0)
    {
      for (auto const &it : arangodb::velocypack::ArrayIterator(r2))
      {
        if (it.isString())
        {
          auto database = it.copyString();
          // i->checkPermissions = false;  // Todo Fix this
          auto [erU, changedKeys] = impl::update(query, newBindVars, database, relationQueries);
          if (ok::isEr(erU))
          {
            LOG_DEBUG << "Error on run same Query on all users";
            LOG_DEBUG << "Error when copying to: " << database << " can't be persisted ";
          }
        }
      }
    }
    else
    {
      LOG_DEBUG << "cant connect to database server while running same mutation query on all user databases.";
    }
  }
}
namespace impl
{
void deleteNestedEntries(VPackSlice outer1, const ok::mutate_schema::RelationQueryMap &relationQueries, Database const &database) noexcept
{
  unsigned int i = 1;
  for (auto const &it : relationQueries)
  {
    if (outer1.isArray() && outer1.length() > i)
    {
      if (auto array = outer1[i]; !array.isEmptyArray())
      {
        constexpr auto deleteQuery = R"aql(
FOR d IN @@collection
    FILTER d._key IN @input
    REMOVE d IN @@collection
    RETURN OLD._key
)aql";
        arangodb::velocypack::Builder bindVars;
        bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
        bindVars.add("@collection", arangodb::velocypack::Value(it.first));
        bindVars.add("input", array);
        bindVars.close();
        auto [erDb, response] = Api::Cursor::PostCursor::request(database, deleteQuery, bindVars.slice());
        if (isEr(erDb))
        {
          if (response)
          {
            auto slice = response->slices().front();
            LOG_FATAL << "Error: " << slice.toJson();
          }
          LOG_FATAL << "Error: "
                    << " | delete executed query: " << deleteQuery << " | collection is: " << it.first << " | bindVars is: " << bindVars.toString();
        }
        else
        {
          auto slice = response->slices().front();
          if (ok::smart_actor::connection::getPrintVal("delete"))
          {
            LOG_DEBUG << "delete result: " << slice.toJson() << " | delete executed query: " << deleteQuery << " | collection is: " << it.first << " | bindVars is: " << bindVars.toString();
          }
          auto r2 = slice.get("result");
          jsoncons::ojson js_arr = jsoncons::ojson::array();
          for (unsigned int i = 0; i < r2.length(); ++i) { js_arr.push_back(r2[i].copyString()); }
          /* TODO TODO TODO FIX THIS: fix
                          delete_object and send jsoncons::ojson
                          delete_object;  // Fix shadow member
                          variable delete_object["d"] = js_arr; if
                          (it.second.global_delete_act_key !=
                          uninitialized) {
                            actor::sendToSuperListActor(
                                act, it.second.global_delete_act_key,
                          delete_object);
                          }
                          if (it.second.user_delete_act_key !=
                          uninitialized_user) {
                            actor::sendToListActor(act, it.first,
                                                   it.second.user_delete_act_key,
                                                   delete_object);
                          }
                          */
        }
      }
      i++;
    }
  }
}
std::tuple<ErrorCode, Collection, ok::mutate_schema::Relation, ok::table_schema::Columns, ok::table_schema::TableWhere> columnSchemaInitialize(
    Database const &database, DocumentKey const &memberKey, VPackSlice const &columnSchemaJson, WsArguments const &args, bool doProcessFilters) noexcept
{
  auto [er, collection, tree, columns, allSelectors, wrapperFilters, sort, filterTop, filterBottom, queryGeneratedForUnAuth, tableProps] =
      ok::table_schema::initialize(args, columnSchemaJson, database, memberKey, true, doProcessFilters);
  if (ok::isEr(er)) { return {er, collection, tree, columns, {}}; }
  // TODO properly test list permission, must check permissionCheck function.
  // IMP: used in locked row finding..
  if (jsoncons::ObjectMemberIsObject(args, "f"))
  {
    ok::table_schema::TableWhere where;
    auto [er] = ok::table_schema::setFilterConfig(args, columns, where);
    if (ok::isEr(er)) { return {er, collection, tree, columns, where}; }
    return {ok::ErrorCode::ERROR_NO_ERROR, collection, tree, columns, where};
  }
  else
  {
    const auto err = "filter args must be object when mutate.";
    LOG_DEBUG << err << " args: " << (args.contains("f") ? args["f"].to_string() : args.to_string());
    return {ok::ErrorCode::ERROR_BAD_PARAMETER, collection, tree, columns, {}};
  }
}
ErrorCode initializeBase(VPackSlice const &formSchemaJson, VPackSlice const &columnSchemaJson, const ok::smart_actor::connection::MutateEventType &eventType) noexcept
{
  // Todo fix this
  /*if (auto er = basicInitialize(); ok::isEr(er))
  {
    return er;
  }*/
  // Todo fix this
  /*
  if (auto [er, colSchema_] = columnSchemaInitialize(database, memberKey, columnSchemaJson, args);
      ok::isEr(er))
  {
    return er;
  }
  else
  {
    colSchema = std::move(colSchema_);
  }*/
  return ok::ErrorCode::ERROR_NO_ERROR;
}
std::tuple<ErrorCode, Collection, ok::mutate_schema::Relation, ok::mutate_schema::Fields, ok::mutate_schema::MutateQueryParts, jsoncons::ojson> initializeInsert(
    Database const &database, DocumentKey const &memberKey, VPackSlice const &formSchemaJson, WsArguments const &args, WsArguments const &backendValue, bool ignoreMissingFieldWithValue) noexcept
{
  // MutateBase::initialize();
  // columnSchemaInitialize cant initialize because filter = null
  // now guest can also do mutation on chat.
  /*if (memberKey.empty())
{
  LOG_DEBUG << "only logged in member can do any mutation";
  return {ok::ErrorCode::ERROR_HTTP_UNAUTHORIZED, "", {}, {}, {}, {}};
}*/
  if (!jsoncons::ObjectMemberIsObject(args, "value"))
  {
    LOG_DEBUG << "Invalid Arguments shape. Arguments must has key value, and it should be an object" << args.to_string();
    return {ok::ErrorCode::ERROR_BAD_PARAMETER, "", {}, {}, {}, {}};
  }
  if (!backendValue.is_object())
  {
    LOG_DEBUG << "Invalid backendValue shape. backendValue must be object" << args.to_string();
    return {ok::ErrorCode::ERROR_BAD_PARAMETER, "", {}, {}, {}, {}};
  }
  {
    // database, memberKey
    auto [er2, collection, tree, fields, allIds] = ok::mutate_schema::initialize(formSchemaJson, ok::smart_actor::connection::MutateEventType::insert);
    if (ok::isEr(er2)) { return {er2, collection, tree, fields, {}, {}}; }
    ok::mutate_schema::MutateQueryParts queryParts;
    auto [er3, fieldsNew, allIdsNew] =
        processDataPart(formSchemaJson, args, ok::smart_actor::connection::MutateEventType::insert, collection, tree, queryParts, memberKey, ignoreMissingFieldWithValue);
    if (ok::isEr(er3)) { return {er3, collection, tree, fields, queryParts, {}}; }
    else
    {
      fields.insert(std::begin(fieldsNew), std::end(fieldsNew));
      allIds.insert(std::end(allIds), std::begin(allIdsNew), std::end(allIdsNew));
    }
    auto er4 = ok::mutate_schema::make_bind_vars::setNonRelationFieldValues(fields, args["value"], backendValue, ignoreMissingFieldWithValue);
    if (ok::isEr(er4)) return {er4, collection, tree, fields, queryParts, {}};
    auto [er5, saveData] = ok::mutate_schema::make_bind_vars::makeFormObject(fields, allIds);
    if (ok::isEr(er5)) return {er5, collection, tree, fields, queryParts, saveData};
    return {ok::ErrorCode::ERROR_NO_ERROR, collection, tree, fields, queryParts, saveData};
  }
}
std::tuple<ErrorCode, std::unordered_set<DocumentKey>> update(AQLQuery const &query,
                                                              arangodb::velocypack::Builder const &bindVars,
                                                              Database const &database,
                                                              ok::mutate_schema::RelationQueryMap const &relationQueries) noexcept
{
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  if (isEr(erDb))
  {
    if (response)
    {
      auto slice = response->slices().front();
      LOG_FATAL << "Error: " << slice.toJson();
    }
    LOG_FATAL << "Error: " << errno_string(erDb) << " | executed query: " << query << " | database is: " << database;
    if (!bindVars.slice().isNone()) { LOG_DEBUG << " | bindVars is: " << bindVars.slice().toString(); }
    else
    {
      auto slice = response->slices().front();
      LOG_DEBUG << "mergeBindVars should not be none";
    }
    return {ok::ErrorCode::BAD_QUERY, {}};
  }
  else
  {
    auto slice = response->slices().front();
    if (ok::smart_actor::connection::getPrintVal("update"))
    {
      LOG_DEBUG << "executed query: \n" << query << "\n | database is: " << database << " | bindVars is: " << bindVars.slice().toString() << " | Result: " << slice.toJson();
    }
    if (slice.get("error").getBool())
    {
      if (slice.get("errorNum").getInt() == 1200)
      {
        LOG_DEBUG << "CONFLICT ERROR";
        return {ok::ErrorCode::ERROR_INCOMPATIBLE_VERSION, {}};
      }
      else
      {
        LOG_DEBUG << "Bad query";
        return {ok::ErrorCode::BAD_QUERY, {}};
      }
    }
    else
    {
      if (auto r2 = slice.get("result"); r2.isArray() && r2.length() > 0)
      {
        if (auto outer1 = r2[0]; outer1.isArray() && outer1.length() > 0)
        {
          auto outer2 = outer1[0];
          auto [er, changedKeys] = getChangedKeys(outer2);
          if (ok::isEr(er)) { return {er, {}}; }
          deleteNestedEntries(outer1, relationQueries, database);
          return {ok::ErrorCode::ERROR_NO_ERROR, changedKeys};
        }
      }
      LOG_DEBUG << "result should not be empty";
      return {ok::ErrorCode::BAD_QUERY, {}};
    }
  }
}
std::pair<ErrorCode, std::unordered_set<DocumentKey>> getChangedKeys(VPackSlice outer2) noexcept
{
  std::unordered_set<DocumentKey> changedKeys;
  if (!outer2.isArray())
  {
    LOG_DEBUG << "Outer2 is not array";
    return {ok::ErrorCode::BAD_QUERY, changedKeys};
  }
  for (unsigned int i = 0; i < outer2.length(); ++i)
  {
    if (outer2[i].isString()) { changedKeys.insert(outer2[i].copyString()); }
    else
    {
      LOG_DEBUG << "Result Array must be string[]";
      return {ok::ErrorCode::BAD_QUERY, changedKeys};
    }
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, changedKeys};
}
//
// void sendNotification(VPackSlice dataSlice, Database const &database, WsArguments const &args, const TRI_voc_document_operation_e operation_type, caf::event_based_actor *act) noexcept
//{
/*if (operation_type == TRI_VOC_DOCUMENT_OPERATION_INSERT)
{
  sendToActivitiesActor("c");
}
else if (operation_type == TRI_VOC_DOCUMENT_OPERATION_UPDATE || operation_type == TRI_VOC_DOCUMENT_OPERATION_REPLACE)
{
  sendToActivitiesActor("u");
}
else if (operation_type == TRI_VOC_DOCUMENT_OPERATION_REMOVE)
{
  sendToActivitiesActor("d");
}*/
//}
void sendToActivitiesActor(std::string const &key) noexcept
{
  /* if (dataSlice.hasKey(key))
   {
     if (dataSlice.get(key).isArray())
     {
       auto msg = dataSlice.get(key);
       updateActivities(msg);
     }
   }*/
}
void updateActivities(VPackSlice const &msgSlice) noexcept
{
  /*std::string msg;
  for (auto const &c : arangodb::velocypack::ArrayIterator(msgSlice))
  {
    if (args.is_array())
    {  // if array form
      if (c.isString())
      {
        if (auto idx = formSchema->getFieldIndex(c.copyString()); idx > -1)
        {
          msg += args[idx].as<std::string>();
        }
        else
        {
          msg += c.copyString();
        }
      }
      // Todo: Make Query generation for object form working.
    }
    else if (args.is_object())
    {  // if object form
      if (operation_type == TRI_VOC_DOCUMENT_OPERATION_INSERT)
      {
        // TODO make query
      }
      else if (operation_type == TRI_VOC_DOCUMENT_OPERATION_UPDATE)
      {
        // TODO make query
      }
    }
    else
    {
      LOG_DEBUG << "Arguments must be object or array";
    }
  }

  auto query = "INSERT {msg: '" + msg + "'} INTO activity RETURN NEW._key";

  auto [ok, response] = Api::Cursor::PostCursor::request(database, query, {});
  auto slice = response->slices().front();
  if (!ok)
  {
    LOG_DEBUG << "Error: " << slice.toJson() << " | executed query: " << query << " | database is: " << database;
  }
  else
  {
    if (auto result = slice.get("result"); result.length() > 0)
    {
      // send to list actor
      //      Temporary disabled
      using key_ = std::string;
      std::vector<key_> filterKey;
      filterKey.push_back(result[0].copyString());
      act->send( ok::smart_actor::supervisor::globalActor, table_dispatch_atom_v, database, "activity",
                         filterKey);

    }
  }*/
}
namespace make_query
{
std::tuple<ErrorCode, AQLQuery> insert(ok::mutate_schema::MutateQueryParts &queryParts) noexcept
{
  // Fix this if wrapper effect wrong updates.
  auto as = "t";
  std::string query;
  for (auto &p : queryParts.prefix) { query += p; }
  std::string data = "";
  for (auto &m : queryParts.mergeQueryVarToData) { data += ", " + m; }
  data = data.empty() ? "@insData" : "MERGE(@insData" + data + ")";
  auto deepQuery = ok::mutate_schema::mutation::getDeepQueryFromResult(queryParts.relationQueries);
  query += R"aql(LET i = (INSERT )aql" + data + R"aql( INTO @@insCollection RETURN NEW._key ))aql";
  query += deepQuery + "\n";
  query += R"aql(RETURN [i])aql";
  return {ok::ErrorCode::ERROR_NO_ERROR, query};
}
std::tuple<ErrorCode, AQLQuery> update(ok::mutate_schema::MutateQueryParts &queryParts, bool isReplaceData) noexcept
{
  auto deepQuery = ok::mutate_schema::mutation::getDeepQueryFromResult(queryParts.relationQueries);
  auto projectionQuery = ok::mutate_schema::mutation::getProjectionFromResult(queryParts.relationQueries);
  std::string query = R"aql(
LET k = ()aql";
  query += !isReplaceData ? "UPDATE" : "REPLACE";
  query += R"aql( @userSuppliedDocument
  WITH @insData
  IN  @@insCollection
  OPTIONS { ignoreRevs: false }
  RETURN NEW._key)
)aql";
  query += deepQuery + "\n";
  query += R"aql(RETURN [k )aql" + projectionQuery + "]";
  return {ok::ErrorCode::ERROR_NO_ERROR, query};
}
std::tuple<ErrorCode, AQLQuery> delete_(ok::mutate_schema::MutateQueryParts &queryParts) noexcept
{
  auto deepQuery = ok::mutate_schema::mutation::getDeepQueryFromResult(queryParts.relationQueries);
  auto projectionQuery = ok::mutate_schema::mutation::getProjectionFromResult(queryParts.relationQueries);
  std::string query = R"aql(
LET k = (REMOVE @userSuppliedDocument IN @@insCollection
  OPTIONS { ignoreRevs: false }
  RETURN OLD._key)
)aql";
  query += deepQuery + "\n";
  query += R"aql(RETURN [k )aql" + projectionQuery + "]";
  return {ok::ErrorCode::ERROR_NO_ERROR, query};
}
// Make conflict detection working for every case
std::tuple<ErrorCode, AQLQuery> position(PositionData const &data) noexcept
{
  std::string query;
  switch (data.position)
  {
    case dropPosition::top:
      query += R"aql(
let to_position = FIRST(FOR u in @@insCollection
    FILTER u._key == @toKey
    return u.position
    )
)aql";
      if (data.toParentKey.empty())
      {
        auto d = data.direction == TRI_EDGE_IN ? std::string{"OUTBOUND"} : std::string{"INBOUND"};
        query += R"aql(
let prev_position = FIRST(FOR u IN @@insCollection
  FILTER u.position < to_position
  FILTER LENGTH(FOR v IN 1..1 )aql" +
                 d + R"aql( u @@edge RETURN v) == 0
  SORT u.position DESC
  LIMIT 1
  RETURN u.position
 )

let new_position = prev_position ? (prev_position + to_position) / 2 : FIRST(FOR u IN @@insCollection
  FILTER LENGTH(FOR v IN 1..1 )aql" +
                 d + R"aql( u @@edge RETURN v) == 0
  COLLECT
  AGGREGATE minNo = MIN(TO_NUMBER(u.position))
  RETURN (minNo || 0) - 1
 )
)aql";
      }
      else
      {
        auto d = data.direction == TRI_EDGE_IN ? std::string{"INBOUND"} : std::string{"OUTBOUND"};
        query += R"aql(
let prev_position = FIRST(FOR u IN 1..1 )aql" +
                 d + R"aql( CONCAT(@insCollection, "/", @toParentKey) @@edge
  FILTER u.position < to_position
  SORT u.position DESC
  LIMIT 1
  RETURN u.position
 )

let new_position = prev_position ? (prev_position + to_position) / 2 : FIRST(
FOR u IN 1..1 )aql" +
                 d + R"aql( CONCAT(@insCollection, "/", @toParentKey) @@edge
  COLLECT
  AGGREGATE minNo = MIN(TO_NUMBER(u.position))
  RETURN (minNo || 0) - 1
 )
)aql";
      }
      query += R"aql(
LET new_position_2 = FIRST(UPDATE {_key: @fromKey, _rev: @fromRev}
    WITH {position: new_position}
    IN  @@insCollection
    OPTIONS { ignoreRevs: false }
    RETURN NEW.position)

 RETURN [[@fromKey]]
)aql";
      break;
    case dropPosition::bottom:
      query += R"aql(
let to_position = FIRST(FOR u in @@insCollection
    FILTER u._key == @toKey
    return u.position
    )
)aql";
      if (data.toParentKey.empty())
      {
        auto d = data.direction == TRI_EDGE_IN ? std::string{"OUTBOUND"} : std::string{"INBOUND"};
        query += R"aql(
let next_position = FIRST(FOR u IN @@insCollection
  FILTER u.position > to_position
  FILTER LENGTH(FOR v IN 1..1 )aql" +
                 d + R"aql( u @@edge RETURN v) == 0
  SORT u.position ASC
  LIMIT 1
  RETURN u.position
 )

let new_position = next_position ? (next_position + to_position) / 2 : FIRST(FOR u IN @@insCollection
  FILTER LENGTH(FOR v IN 1..1 )aql" +
                 d + R"aql( u @@edge RETURN v) == 0
  COLLECT
  AGGREGATE maxNo = MAX(TO_NUMBER(u.position))
  RETURN (maxNo || 0.0) + 1
 )
)aql";
      }
      else
      {
        auto d = data.direction == TRI_EDGE_IN ? std::string{"INBOUND"} : std::string{"OUTBOUND"};
        query += R"aql(
let next_position = FIRST(FOR u IN 1..1 )aql" +
                 d + R"aql( CONCAT(@insCollection, "/", @toParentKey) @@edge
  FILTER u.position > to_position
  SORT u.position ASC
  LIMIT 1
  RETURN u.position
 )

let new_position = next_position ? (next_position + to_position) / 2 : FIRST(
FOR u IN 1..1 )aql" +
                 d + R"aql( CONCAT(@insCollection, "/", @toParentKey) @@edge
  COLLECT
  AGGREGATE maxNo = MAX(TO_NUMBER(u.position))
  RETURN (maxNo || 0.0) + 1
 )
)aql";
      }
      query += R"aql(
LET new_position_2 = FIRST(UPDATE {_key: @fromKey, _rev: @fromRev}
    WITH {position: new_position}
    IN  @@insCollection
    OPTIONS { ignoreRevs: false }
    RETURN NEW.position)

 RETURN [[@fromKey]])aql";
      break;
    case dropPosition::center:
      if (data.toKey.empty()) { LOG_DEBUG << "This should never happen"; }
      else
      {
        auto d = data.direction == TRI_EDGE_IN ? std::string{"INBOUND"} : std::string{"OUTBOUND"};
        query += R"aql(
let new_position = FIRST(
FOR u IN 1..1 )aql" +
                 d + R"aql( CONCAT(@insCollection, "/", @toKey) @@edge
  COLLECT
  AGGREGATE maxNo = MAX(TO_NUMBER(u.position))
  RETURN (maxNo || 0.0) + 1
 )
)aql";
      }
      query += R"aql(
LET new_position_2 = FIRST(UPDATE {_key: @fromKey, _rev: @fromRev}
    WITH {position: new_position}
    IN  @@insCollection
    OPTIONS { ignoreRevs: false }
    RETURN NEW.position)

 RETURN [[@fromKey]])aql";
      break;
    default: query = ""; break;
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, query};
}
std::tuple<ErrorCode, AQLQuery> batchUpdate(ok::mutate_schema::MutateQueryParts &queryParts,
                                            ok::table_schema::Columns const &columns,
                                            ok::table_schema::TableWhere const &where,
                                            bool isReplaceData) noexcept
{
  // Fix this if wrapper effect wrong updates.
  AQLQuery as{"t"};
  auto [er, filterQuery] = ok::table_schema::buildWherePart(columns, as, where, "");
  if (isEr(er)) return {er, filterQuery};
  auto deepQuery = ok::mutate_schema::mutation::getDeepQueryFromResult(queryParts.relationQueries);
  auto projectionQuery = ok::mutate_schema::mutation::getProjectionFromResult(queryParts.relationQueries);
  /*constexpr absl::string_view u = R"aql(
LET k = (FOR %s IN @@insCollection
    %s
    %s %s WITH @insData IN  @@insCollection
    OPTIONS { ignoreRevs: false }
    RETURN NEW._key)
%s
RETURN [k %s]
)aql";*/
  std::string query = R"aql(
LET k = (FOR )aql" + as +
                      R"aql( IN @@insCollection
    )aql" + filterQuery +
                      R"aql(
    )aql";
  query += !isReplaceData ? "UPDATE " : "REPLACE ";
  query += as + R"aql( WITH @insData IN  @@insCollection
    OPTIONS { ignoreRevs: false }
    RETURN NEW._key)
)aql";
  query += deepQuery + R"aql(
RETURN [k )aql" +
           projectionQuery + "]\n";
  return {ok::ErrorCode::ERROR_NO_ERROR, query};
}
std::tuple<ErrorCode, AQLQuery> batchDelete(ok::mutate_schema::MutateQueryParts &queryParts, ok::table_schema::Columns const &columns, ok::table_schema::TableWhere const &where) noexcept
{
  // Fix this if wrapper effect wrong updates.
  AQLQuery as{"t"};
  auto [er, filterQuery] = ok::table_schema::buildWherePart(columns, as, where, "");
  if (isEr(er)) return {er, filterQuery};
  auto deepQuery = ok::mutate_schema::mutation::getDeepQueryFromResult(queryParts.relationQueries);
  auto projectionQuery = ok::mutate_schema::mutation::getProjectionFromResult(queryParts.relationQueries);
  /*constexpr absl::string_view d = R"aql(
LET k = (FOR %s IN @@insCollection
    %s
    REMOVE %s IN @@insCollection
    RETURN OLD._key)
%s
RETURN [k %s]
)aql";*/
  std::string query = R"aql(
LET k = (FOR )aql" + as +
                      R"aql( IN @@insCollection
    )aql";
  query += filterQuery + R"aql(
    REMOVE )aql" +
           as + R"aql( IN @@insCollection
    RETURN OLD._key)
)aql" + deepQuery;
  query += R"aql(
RETURN [k )aql" +
           projectionQuery + "]\n";
  return {ok::ErrorCode::ERROR_NO_ERROR, query};
}
}  // namespace make_query
namespace get_merged_bind_vars
{
std::tuple<ErrorCode, arangodb::velocypack::Builder> base(
    Collection const &collection, jsoncons::ojson &saveData, ok::mutate_schema::Fields const &fields, WsArguments const &args, DocumentKey const &memberKey) noexcept
{
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  auto [er, builder] = arangodb::velocypack::parseJson(saveData.to_string());
  if (ok::isEr(er)) { return {er, bindVars}; }
  bindVars.add("insData", builder->slice());
  bindVars.add("@insCollection", arangodb::velocypack::Value(collection));
  bindVars.close();
  if (jsoncons::ObjectMemberIsObject(args, "value"))
  {
    auto [er, relationVars] = ok::mutate_schema::makeRelationBindVarsBase(fields, args, memberKey);
    if (ok::isEr(er)) { return {er, relationVars}; }
    return {ok::ErrorCode::ERROR_NO_ERROR, arangodb::velocypack::Collection::merge(bindVars.slice(), relationVars.slice(), true, false)};
  }
  else
  {
    LOG_DEBUG << "Form Data must be an object";
    return {ok::ErrorCode::ERROR_BAD_PARAMETER, bindVars};
  }
}
std::tuple<ErrorCode, arangodb::velocypack::Builder> insert(
    Collection const &collection, jsoncons::ojson &saveData, ok::mutate_schema::Fields const &fields, WsArguments const &args, DocumentKey const &memberKey) noexcept
{
  return base(collection, saveData, fields, args, memberKey);
}
std::tuple<ErrorCode, jsoncons::ojson> getKeyRev(WsArguments const &args)
{
  jsoncons::ojson userSuppliedDocumentJson;  // currently, it is taken from filter
  if (jsoncons::ObjectMemberIsObject(args, "f") && args["f"].contains("_key") && args["f"].contains("_rev"))
  {
    auto &filter = args["f"];
    if (filter.contains("_key") && filter.contains("_rev"))
    {
      userSuppliedDocumentJson["_key"] = args["f"]["_key"];
      userSuppliedDocumentJson["_rev"] = args["f"]["_rev"];
      return {ok::ErrorCode::ERROR_NO_ERROR, userSuppliedDocumentJson};
    }
    else
    {
      LOG_DEBUG << "UserSuppliedDocument filter error: _key and _rev must exist" << filter.to_string();
      return {ok::ErrorCode::ERROR_BAD_PARAMETER, userSuppliedDocumentJson};
    }
  }
  else
  {
    LOG_DEBUG << "UserSuppliedDocument argument error: " << args.to_string();
    return {ok::ErrorCode::ERROR_BAD_PARAMETER, userSuppliedDocumentJson};
  }
}
std::tuple<ErrorCode, arangodb::velocypack::Builder> update(
    Collection const &collection, jsoncons::ojson &saveData, ok::mutate_schema::Fields const &fields, WsArguments const &args, DocumentKey const &memberKey) noexcept
{
  arangodb::velocypack::Builder bindVars;
  auto [er0, userSuppliedDocumentJ] = getKeyRev(args);
  if (isEr(er0)) return {er0, bindVars};
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  auto [er, builder] = arangodb::velocypack::parseJson(saveData.to_string());
  if (ok::isEr(er)) { return {er, bindVars}; }
  bindVars.add("insData", builder->slice());
  auto [erU, builderUserSuppliedDocument] = arangodb::velocypack::parseJson(userSuppliedDocumentJ.to_string());
  if (ok::isEr(erU)) { return {erU, bindVars}; }
  bindVars.add("userSuppliedDocument", builderUserSuppliedDocument->slice());
  bindVars.add("@insCollection", arangodb::velocypack::Value(collection));
  bindVars.close();
  if (jsoncons::ObjectMemberIsObject(args, "value"))
  {
    auto [er, bindVars2] = ok::mutate_schema::makeRelationBindVarsBase(fields, args, memberKey);
    if (ok::isEr(er)) { return {er, bindVars2}; }
    return {ok::ErrorCode::ERROR_NO_ERROR, arangodb::velocypack::Collection::merge(bindVars.slice(), bindVars2.slice(), true, false)};
  }
  else
  {
    LOG_DEBUG << "Form Arguments data must be object" << args.to_string();
    return {ok::ErrorCode::ERROR_BAD_PARAMETER, bindVars};
  }
}
std::tuple<ErrorCode, arangodb::velocypack::Builder> delete_(Collection const &collection, jsoncons::ojson &saveData, ok::mutate_schema::Fields const &fields, WsArguments const &args) noexcept
{
  arangodb::velocypack::Builder bindVars;
  auto [er0, userSuppliedDocumentJ] = getKeyRev(args);
  if (isEr(er0)) return {er0, bindVars};
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  auto [erU, builderUserSuppliedDocument] = arangodb::velocypack::parseJson(userSuppliedDocumentJ.to_string());
  if (isEr(erU)) { return {erU, bindVars}; }
  bindVars.add("userSuppliedDocument", builderUserSuppliedDocument->slice());
  bindVars.add("@insCollection", arangodb::velocypack::Value(collection));
  bindVars.close();
  auto relationVars = ok::mutate_schema::makeBindVarsDelete();
  return {ok::ErrorCode::ERROR_NO_ERROR, arangodb::velocypack::Collection::merge(bindVars.slice(), relationVars.slice(), true, false)};
}
// todo make this function return shared_ptr of builder
std::tuple<ErrorCode, arangodb::velocypack::Builder> position(
    Collection const &collection, jsoncons::ojson &saveData, ok::mutate_schema::Fields const &fields, WsArguments const &args, PositionData const &data) noexcept
{
  jsoncons::ojson bindVars;
  bindVars["fromKey"] = data.fromKey;
  bindVars["fromRev"] = data.fromRev;
  bindVars["toKey"] = data.toKey;
  bindVars["@insCollection"] = collection;
  bindVars["@edge"] = data.edge;
  if (data.position == dropPosition::center)
  {
    bindVars["toKey"] = data.toKey;
    bindVars["insCollection"] = collection;
  }
  else if (!data.toParentKey.empty())
  {
    bindVars["toParentKey"] = data.toParentKey;
    bindVars["insCollection"] = collection;
  }
  auto [er, builder] = arangodb::velocypack::parseJson(bindVars.to_string());
  if (isEr(er)) { return {er, arangodb::velocypack::Builder{}}; }
  return {ok::ErrorCode::ERROR_NO_ERROR, *builder};  // todo avoid copying..
}
std::tuple<ErrorCode, arangodb::velocypack::Builder> batchUpdate(
    Collection const &collection, jsoncons::ojson &saveData, ok::mutate_schema::Fields const &fields, WsArguments const &args, DocumentKey const &memberKey) noexcept
{
  // todo fix
  return base(collection, saveData, fields, args, memberKey);
}
std::tuple<ErrorCode, arangodb::velocypack::Builder> batchDelete(Collection const &collection, jsoncons::ojson &saveData, ok::mutate_schema::Fields const &fields, WsArguments const &args) noexcept
{
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("@insCollection", arangodb::velocypack::Value(collection));
  bindVars.close();
  auto relationVars = ok::mutate_schema::makeBindVarsDelete();
  return {ok::ErrorCode::ERROR_NO_ERROR, arangodb::velocypack::Collection::merge(bindVars.slice(), relationVars.slice(), true, false)};
}
}  // namespace get_merged_bind_vars
namespace locked_row_permission_check
{
ErrorCode base(VPackSlice const &formSchemaJson,
               jsoncons::ojson const &args,
               std::string const &key,
               Collection const &collection,
               ok::mutate_schema::Relation const &tree,
               Database const &database,
               ok::table_schema::Columns const &columns,
               ok::table_schema::TableWhere const &where) noexcept
{
  if (!key.empty())
  {
    if (formSchemaJson.hasKey("lock") && formSchemaJson["lock"].isArray())
    {
      for (auto const &vit : arangodb::velocypack::ArrayIterator(formSchemaJson["lock"]))
      {
        if (vit.isArray())
        {
          if (vit.length() > 2 && vit[0].isArray())
          {
            auto findSlice = arangodb::velocypack::Collection::find(vit[0], [&](VPackSlice const &value, unsigned long index) { return value.isString() && value.copyString() == key; });
            if (!findSlice.isNone())
            {
              if (vit[1].isString())
              {
                if (auto func = vit[1].copyString(); func == "sel")
                {
                  if (vit[2].isString())
                  {
                    auto k = vit[2].copyString();
                    std::string as = "t";
                    ok::table_schema::WrapperFilters wrapperFilters;
                    auto [er, query] = ok::table_schema::buildQuery(as, columns, args, tree, wrapperFilters, where, {}, {}, as, collection, tree.edge, "", "", "");
                    if (ok::isEr(er)) { return er; }
                    auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, {});
                    auto slice = response->slices().front();
                    if (isEr(erDb))
                    {
                      if (response)
                      {
                        auto slice = response->slices().front();
                        LOG_FATAL << "Error: " << slice.toJson();
                      }
                      LOG_FATAL << "Error: " << errno_string(erDb) << " | executed query: " << query;
                    }
                    else
                    {
                      auto result = slice.get("result");
                      if (result.length() > 0)
                      {
                        for (auto const &vit : arangodb::velocypack::ArrayIterator(result))
                        {
                          if (vit.isObject() && vit.hasKey(k))
                          {
                            if (vit[k].getBool())
                            {
                              LOG_DEBUG << "Row is locked, executed query: \n" << query;
                              return ok::ErrorCode::ERROR_HTTP_UNAUTHORIZED;
                            }
                            else
                            {
                              return ok::ErrorCode::ERROR_NO_ERROR;
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
ErrorCode insert() noexcept { return ok::ErrorCode::ERROR_NO_ERROR; }
ErrorCode update(VPackSlice const &formSchemaJson,
                 jsoncons::ojson const &args,
                 Collection const &collection,
                 ok::mutate_schema::Relation const &tree,
                 Database const &database,
                 ok::table_schema::Columns const &columns,
                 ok::table_schema::TableWhere const &where) noexcept
{
  return base(formSchemaJson, args, "upd", collection, tree, database, columns, where);
}
ErrorCode delete_(VPackSlice const &formSchemaJson,
                  jsoncons::ojson const &args,
                  Collection const &collection,
                  ok::mutate_schema::Relation const &tree,
                  Database const &database,
                  ok::table_schema::Columns const &columns,
                  ok::table_schema::TableWhere const &where) noexcept
{
  return base(formSchemaJson, args, "del", collection, tree, database, columns, where);
}
ErrorCode position() noexcept { return ok::ErrorCode::ERROR_NO_ERROR; }
ErrorCode batchUpdate(VPackSlice const &formSchemaJson,
                      jsoncons::ojson const &args,
                      Collection const &collection,
                      ok::mutate_schema::Relation const &tree,
                      Database const &database,
                      ok::table_schema::Columns const &columns,
                      ok::table_schema::TableWhere const &where) noexcept
{
  return base(formSchemaJson, args, "upd", collection, tree, database, columns, where);
}
ErrorCode batchDelete(VPackSlice const &formSchemaJson,
                      jsoncons::ojson const &args,
                      Collection const &collection,
                      ok::mutate_schema::Relation const &tree,
                      Database const &database,
                      ok::table_schema::Columns const &columns,
                      ok::table_schema::TableWhere const &where) noexcept
{
  return base(formSchemaJson, args, "del", collection, tree, database, columns, where);
}
}  // namespace locked_row_permission_check
namespace unique_check
{
ErrorCode base(VPackSlice dataSlice, Database const &database, Collection const &collection, TRI_voc_document_operation_e operation_type, ok::mutate_schema::Fields &fields) noexcept
{
  for (auto const &vit : arangodb::velocypack::ArrayIterator(dataSlice))
  {
    if (vit.isArray() && vit.length() > 0)
    {
      std::string query;
      // int last_found_idx = -1;  // must initialize otherwise undefined
      ID last_found_ID;
      query += "FOR t in " + collection + "\n";
      // Add filters of single unique condition:
      std::vector<std::string> filters;
      for (auto const &c : arangodb::velocypack::ArrayIterator(vit))
      {
        if (operation_type == TRI_VOC_DOCUMENT_OPERATION_UPDATE)
        {
          if (fields.size() > 0) { filters.push_back("FILTER t._key !='" + fields["_key"].value.as_string() + "'\n"); }
          else
          {
            return ok::ErrorCode::ERROR_BAD_PARAMETER;
          }
        }
        if (c.isString())
        {
          if (fields.contains(c.copyString()))
          {
            auto field = fields[c.copyString()];
            last_found_ID = field.id;
            filters.push_back("FILTER t." + field.selector + " == '" + fields[last_found_ID].value.as<std::string>() + "'\n");
          }
        }
        // Todo: Make Query generation for object form working.
      }
      if (!filters.empty())
      {
        for (auto &f : filters) { query += f; }
      }
      else
      {
        LOG_DEBUG << "no filter found";
        continue;
      }
      query += "RETURN t";
      auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, {});
      if (isEr(erDb))
      {
        if (response)
        {
          auto slice = response->slices().front();
          LOG_FATAL << "Error: " << slice.toJson();
        }
        LOG_FATAL << "Error: " << errno_string(erDb) << " | executed query: " << query;
        return ok::ErrorCode::BAD_QUERY;
      }
      else
      {
        auto slice = response->slices().front();
        if (auto result = slice.get("result"); result.length() > 0)
        {
          if (!last_found_ID.empty())
          {
            auto field = fields[last_found_ID];
            auto error = field.label + " Must be unique";
            return ok::ErrorCode::UNIQUE_ERROR;  // error
          }
        }
      }
    }
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
ErrorCode insert(VPackSlice const &formSchemaJson, Database const &database, Collection const &collection, ok::mutate_schema::Fields &fields) noexcept
{
  if (formSchemaJson.hasKey("u") && formSchemaJson["u"].isArray()) { return base(formSchemaJson["u"], database, collection, TRI_VOC_DOCUMENT_OPERATION_INSERT, fields); }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
ErrorCode update(VPackSlice const &formSchemaJson, Database const &database, Collection const &collection, ok::mutate_schema::Fields &fields, bool isReplaceData) noexcept
{
  if (formSchemaJson.hasKey("u") && formSchemaJson["u"].isArray())
  {
    return base(formSchemaJson["u"], database, collection, isReplaceData ? TRI_VOC_DOCUMENT_OPERATION_REPLACE : TRI_VOC_DOCUMENT_OPERATION_UPDATE, fields);
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
}  // namespace unique_check
}  // namespace impl
// namespace mutate_actor
}  // namespace mutate_actor
}  // namespace ok
