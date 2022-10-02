#pragma once
#include <jsoncons/json.hpp>
#include "caf/all.hpp"
#include "alias.hpp"
#include "CAF.hpp"
#include "utils/ErrorConstants.hpp"
#include <fuerte/connection.h>
#include "utils/json_functions.hpp"
#include "utils/BatchArrayMessage.hpp"
#include "db/db_functions.hpp"
#include "table/Table.hpp"
namespace ok
{
enum class ErrorCode;
namespace smart_actor::table_actor
{
template <typename T>
struct TableActorConfig
{
  T actor;
  DocumentKey MemberKey;
  WsEvent event;
  WsArguments args;
  DocumentKey projectKey;
  bool is_form{false};
  std::unordered_set<DocumentKey> savedKeys;
  AQLQuery changeQuery;
};
//\todo (optional)make join table(wrapper) projections. needed for more complex.
//\todo by default max rows are 1000. fix it.
//\todo some table rows cant be deleted or edited, pass option from server with
// header. \todo BUG make org_id unmovable, as it is used in LAYOUT or pass
// column order
// when fetching
template <typename T>
struct table_actor_state
{
  static inline constexpr char const *name = "table-actor";
  Database database;
  SchemaKey schemaKey;
  bool isMutateSchemaKeyIsCollectionKey{false};
  // std::vector<TableActorConfig> subscribedActors;
  std::vector<TableActorConfig<T>> subscribedActors;
  bool schemaUpdated{true};
  std::unique_ptr<arangodb::fuerte::Response> respColumnSchema;
  VPackSlice columnSchemaJson;
};
template <typename T>
using TableActorPtr = typename table_actor_int<T>::template stateful_pointer<table_actor_state<T>>;
bool argsSchemaCheck(WsArguments const &args) noexcept;
template <typename T>
bool saveListActorOrUpdateArgsAndSend(TableActorPtr<T> act,
                                      table_actor_state<T> &state,
                                      std::vector<TableActorConfig<T>> &subscribedActors,
                                      Database database,
                                      DocumentKey memberKey,
                                      SchemaKey schemaKey,
                                      bool isMutateSchemaKeyIsCollectionKey,
                                      WsEvent const &event,
                                      WsArguments const &args,
                                      T connectionActor,
                                      bool permissionCheck) noexcept
{
  bool sendHeader = false;
  if (auto findActorConfig = std::find_if(std::begin(subscribedActors), std::end(subscribedActors), [&](TableActorConfig<T> const &a_) { return a_.event == event && a_.actor == connectionActor; });
      findActorConfig == std::end(subscribedActors))
  {
    TableActorConfig<T> config{connectionActor, std::move(memberKey), event, args, jsoncons::getStringKey(args, "project"), jsoncons::getBoolOption(args, "form"), {}, ""};
    sendHeader = true;
    auto [er, resultData] = getDataOfOneActor(act, state, database, schemaKey, isMutateSchemaKeyIsCollectionKey, config, {}, permissionCheck, true, sendHeader);
    auto msg = ok::smart_actor::connection::wsMessageBase();
    auto event = config.event;
    // event[0] = event[0].as<std::string>() + "_confirm"; // Todo fix
    if (!ok::isEr(er))
    {
      ok::smart_actor::connection::addEventAndJson(msg, config.event, resultData);
      subscribedActors.push_back(std::move(config));
    }
    else
      ok::smart_actor::connection::addFailure(msg, event, er);
    act->send(connectionActor, caf::forward_atom_v, msg);
    return true;
  }
  else
  {
    findActorConfig->args = args;
    findActorConfig->changeQuery = "";
    findActorConfig->projectKey = jsoncons::getStringKey(args, "project");
    findActorConfig->is_form = jsoncons::getBoolOption(args, "form");
    findActorConfig->savedKeys.clear();
    if (jsoncons::ObjectMemberIsBool(args, "header") && args["header"].as_bool()) sendHeader = true;
    else
      sendHeader = false;
    auto [er, resultData] = getDataOfOneActor(act, state, database, schemaKey, isMutateSchemaKeyIsCollectionKey, *findActorConfig, {}, permissionCheck, true, sendHeader);
    sendData(act, *findActorConfig, resultData);
    return false;
  }
}
template <typename T>
void eraseActor(std::vector<TableActorConfig<T>> &subscribedActors, const WsEvent &event, T connectionActor) noexcept
{
  auto result1 = std::find_if(std::begin(subscribedActors),
                              std::end(subscribedActors),
                              [&](TableActorConfig<T> &a_)
                              {
                                if (a_.event.size() < 3)
                                {
                                  LOG_DEBUG << "table event only support event with exactly 3 elements";
                                  return false;
                                }
                                else
                                {
                                  return a_.event[2] == event[2] && a_.actor == connectionActor;
                                }
                              });
  if (result1 != std::end(subscribedActors)) { subscribedActors.erase(result1); }
}
// currently, it sends all the delete keys, I can optimize it. also delete keys from the list.
template <typename T>
void dispatchToIfKey(TableActorPtr<T> act, std::vector<TableActorConfig<T>> &subscribedActors, std::unordered_set<std::string> deleteKeys) noexcept
{
  for (auto &m : subscribedActors)
  {
    if (auto a = std::find_if(std::begin(m.savedKeys),
                              std::end(m.savedKeys),
                              [&](std::string const &s_key)
                              {
                                auto d = std::find(std::begin(deleteKeys), std::end(deleteKeys), s_key);
                                return d != std::end(deleteKeys);
                              });
        a != std::end(m.savedKeys))
    {
      jsoncons::ojson result = jsoncons::ojson::array();
      jsoncons::ojson one = jsoncons::ojson::array();
      one.push_back(m.event);
      jsoncons::ojson d;
      jsoncons::ojson js_arr = jsoncons::ojson::array();
      for (auto &del : deleteKeys)
      {
        if (!del.empty()) { js_arr.push_back(del); }
        else
        {
          LOG_DEBUG << "Result Array must be string[]";
        }
      }
      d["result"] = js_arr;
      jsoncons::ojson obj;
      obj["d"] = d;
      one.push_back(obj);
      result.push_back(one);
      act->send(m.actor, caf::forward_atom_v, result);  // dispatch_atom
    }
  }
}
template <typename T>
void dispatchNewKeysToAll(TableActorPtr<T> act,
                          table_actor_state<T> &state,
                          Database database,
                          SchemaKey schemaKey,
                          bool isMutateSchemaKeyIsCollectionKey,
                          std::vector<TableActorConfig<T>> &subscribedActors,
                          std::unordered_set<DocumentKey> const &filterKey,
                          bool permissionCheck) noexcept
{
  if (filterKey.empty())
  {
    LOG_DEBUG << "empty key not dispatched.";
    return;
  }
  for (auto &config : subscribedActors)
  {
    auto [er, resultData] = getDataOfOneActor(act, state, database, schemaKey, isMutateSchemaKeyIsCollectionKey, config, filterKey, permissionCheck, false, false);
    sendData(act, config, resultData);
  }
}
template <typename T>
std::pair<ErrorCode, jsoncons::ojson> getDataOfOneActor(TableActorPtr<T> act,
                                                        table_actor_state<T> &state,
                                                        Database database,
                                                        SchemaKey schemaKey,
                                                        bool isMutateSchemaKeyIsCollectionKey,
                                                        TableActorConfig<T> &config,
                                                        std::unordered_set<DocumentKey> const &filterKey,
                                                        bool permissionCheck,
                                                        bool sendEmpty = false,
                                                        bool sendHeader = true) noexcept
{
  LOG_DEBUG << "==================================" << config.event.as_string() << " Start ============================================";
  if (config.is_form && ok::table_actor::isDefaultFormValue(config.args))
  {
    auto [errorCode, fields, allIds, options] = ok::table_actor::fetchFormDefaultInitialize(database, config.MemberKey, config.projectKey, schemaKey, config.args);
    auto headerData = ok::mutate_schema::table_actor::getFormHeaderData(fields, allIds, options, jsoncons::ojson::array());
    auto resultData = ok::mutate_schema::table_actor::getFormDefaultObject(fields, allIds);
    headerData.merge(resultData);
    if (ok::isEr(errorCode))
    {
      jsoncons::ojson resultData2 = jsoncons::ojson::object();
      resultData2["error"] = ok::isEr(errorCode);
      resultData2["description"] = ok::errno_string(errorCode);
      LOG_DEBUG << "==================================" << config.event.as_string() << " End ============================================";
      return {errorCode, resultData2};
    }
    else
    {
      LOG_DEBUG << "==================================" << config.event.as_string() << " End ============================================";
      return {ok::ErrorCode::ERROR_NO_ERROR, headerData};
    }
  }
  else
  {
    if (state.schemaUpdated)
    {
      auto [erS, myRespColumnSchema] = ok::db::getSchema("global", "schema", schemaKey, ".columns");
      if (ok::isEr(erS))
      {
        jsoncons::ojson resultData = jsoncons::ojson::object();
        resultData["error"] = ok::isEr(ok::ErrorCode::CANNOT_LOAD_SCHEMA);
        resultData["description"] = ok::errno_string(ok::ErrorCode::CANNOT_LOAD_SCHEMA);
        LOG_DEBUG << "==================================" << config.event.as_string() << " End ============================================";
        return {ok::ErrorCode::CANNOT_LOAD_SCHEMA, resultData};
      }
      state.columnSchemaJson = myRespColumnSchema->slices().front().get("result")[0];
      state.respColumnSchema = std::move(myRespColumnSchema);
      state.schemaUpdated = false;
    }
    auto [errorCode, columns, fields, query, resultData] = ok::table_actor::getData(config.args,
                                                                                    database,
                                                                                    config.MemberKey,
                                                                                    config.projectKey,
                                                                                    schemaKey,
                                                                                    state.columnSchemaJson,
                                                                                    config.is_form,
                                                                                    isMutateSchemaKeyIsCollectionKey,
                                                                                    config.changeQuery,
                                                                                    config.savedKeys,
                                                                                    filterKey,
                                                                                    sendEmpty,
                                                                                    sendHeader,
                                                                                    permissionCheck);
    if (ok::isEr(errorCode))
    {
      jsoncons::ojson resultData2 = jsoncons::ojson::object();
      resultData2["error"] = ok::isEr(errorCode);
      resultData2["description"] = ok::errno_string(errorCode);
      LOG_DEBUG << "==================================" << config.event.as_string() << " End ============================================";
      return {errorCode, resultData};
    }
    else
    {
      LOG_DEBUG << "==================================" << config.event.as_string() << " End ============================================";
      return {ok::ErrorCode::ERROR_NO_ERROR, resultData};
    }
  }
}
inline jsoncons::ojson prepareResult(WsEvent const &event, jsoncons::ojson &resultData)
{
  if (!resultData.is_null() && !resultData.empty())
  {
    auto msg = ok::smart_actor::connection::wsMessageBase();
    ok::smart_actor::connection::addEventAndJson(msg, event, resultData);
    return msg;
  }
  else
  {
    return jsoncons::ojson::object();
  }
}
template <typename T>
void sendData(TableActorPtr<T> act, TableActorConfig<T> &config, jsoncons::ojson &resultData)
{
  if (!resultData.is_null() && !resultData.empty())
  {
    auto msg = ok::smart_actor::connection::wsMessageBase();
    ok::smart_actor::connection::addEventAndJson(msg, config.event, resultData);
    act->send(config.actor, caf::forward_atom_v, msg);  // or dispatch_atom
  }
}
template <typename T>
void removeAllSubscriptions(std::vector<TableActorConfig<T>> &subscribedActors, T connectionActor)
{
  auto end = std::remove_if(std::begin(subscribedActors), std::end(subscribedActors), [&](TableActorConfig<T> &m) { return connectionActor == m.actor; });
  subscribedActors.erase(end, std::end(subscribedActors));
}
template <typename T>
typename table_actor_int<T>::behavior_type TableActor(TableActorPtr<T> self, Database database, SchemaKey schemaKey, bool isMutateSchemaKeyIsCollectionKey, bool permissionCheck)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("Table Actor"));
  self->set_exit_handler(
      [](caf::scheduled_actor *, caf::exit_msg &msg)
      {
        LOG_ERROR << "Table Actor exit_handler error";
        LOG_ERROR << "source: " << caf::deep_to_string(msg.source) << " reason: " << caf::deep_to_string(msg.reason);
      });
  return {[=](send_to_one_atom, WsEvent const &event, WsArguments const &args, DocumentKey const &memberKey, T connectionActor)
          {
            if (!argsSchemaCheck(args)) return;
            std::unordered_set<std::string> saved_keys;
            bool sendHeader = jsoncons::getBoolOption(args, "h", true);
            TableActorConfig<T> config{connectionActor, std::move(memberKey), event, args, jsoncons::getStringKey(args, "project"), jsoncons::getBoolOption(args, "form"), saved_keys, ""};
            auto [er, resultData] = getDataOfOneActor(self, self->state, database, schemaKey, isMutateSchemaKeyIsCollectionKey, config, {}, permissionCheck, true, sendHeader);
            sendData(self, config, resultData);
          },
          [=](get_initial_data_atom, WsEvent const &event, WsArguments const &args, DocumentKey const &memberKey, T connectionActor)
          {
            if (!argsSchemaCheck(args)) return jsoncons::ojson{};
            std::unordered_set<std::string> saved_keys;
            bool sendHeader = jsoncons::getBoolOption(args, "h", true);
            TableActorConfig<T> config{connectionActor, std::move(memberKey), event, args, jsoncons::getStringKey(args, "project"), jsoncons::getBoolOption(args, "form"), saved_keys, ""};
            auto [er, resultData] = getDataOfOneActor(self, self->state, database, schemaKey, isMutateSchemaKeyIsCollectionKey, config, {}, permissionCheck, true, sendHeader);
            return resultData;
          },
          [=](caf::subscribe_atom, WsEvent const &event, WsArguments const &args, DocumentKey const &memberKey, T connectionActor)
          {
            if (!argsSchemaCheck(args)) return false;
            return saveListActorOrUpdateArgsAndSend<T>(
                self, self->state, self->state.subscribedActors, database, std::move(memberKey), schemaKey, isMutateSchemaKeyIsCollectionKey, event, args, connectionActor, permissionCheck);
          },
          [=](caf::unsubscribe_atom, WsEvent const &event, T connectionActor) { eraseActor<T>(self->state.subscribedActors, event, connectionActor); },
          [=](erase_atom, std::unordered_set<DocumentKey> const &delete_keys) { dispatchToIfKey<T>(self, self->state.subscribedActors, delete_keys); },
          [=](table_dispatch_atom, std::unordered_set<DocumentKey> const &filterKey)
          { dispatchNewKeysToAll<T>(self, self->state, database, schemaKey, isMutateSchemaKeyIsCollectionKey, self->state.subscribedActors, filterKey, permissionCheck); },
          [=](schema_changed_atom) { self->state.schemaUpdated = true; },
          [=](conn_exit_atom, T connectionActor) { removeAllSubscriptions<T>(self->state.subscribedActors, connectionActor); },
          CONN_EXIT};
}
}  // namespace smart_actor::table_actor
}  // namespace ok
