#include "actor_system/WSEventMap.hpp"
#include "actor_system/WsConnectionActor.hpp"
#include "db/Session.hpp"
#include "utils/BatchArrayMessage.hpp"
#include "db/db_functions.hpp"
#include "utils/ErrorConstants.hpp"
#include <velocypack/Slice.h>
#include <jsoncons/json.hpp>
#include <magic_enum.hpp>
#include <utility>
#include "mutate/MutateSchema.hpp"
#include <velocypack/Iterator.h>
#include "actor_system/Routes.hpp"
#include "db/DatabaseApi.hpp"
#include "caf/all.hpp"
#include "pystring.hpp"
#include "utils/json_functions.hpp"
#include "utils/time_functions.hpp"
#include "table/Table.hpp"
#include "actor_system/MutateActors.hpp"
#include "utils/os_functions.hpp"
#include "Api.hpp"
namespace ok::smart_actor
{
namespace connection
{
#define RouteArgs                                                                                                                     \
  jsoncons::ojson const &valin, unsigned int eventNo, std::string const &routeNo, Session const &session, jsoncons::ojson &resultMsg, \
      ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor, std::string const &subDomain, bool isDispatching
namespace impl
{
void addMyRoute()
{
  routeFunctions["get_member_setting"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    if (jsoncons::ArrayPosIsString(args, 0))
    {
      std::string projections = ".columns";
      if (jsoncons::ArrayPosIsString(args, 1)) { projections = args[1].as<std::string>(); }
      // on First time, table fetch if empty, member_settings is created
      if (auto [er, myResp] = ok::db::getSchema(session.database, "member_setting", args[0].as<std::string>(), projections); ok::isEr(er))
      {
        // error
      }
      else
      {
        auto slice = myResp->slices().front().get("result");
        if (slice.length() == 1) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, slice[0]); }
      }
    }
  };
  routeFunctions["member_setting_mutate"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    if (jsoncons::ArrayPosIsString(args, 0))
    {
      auto schemaQuery = "UPDATE '" + args[0].as<std::string>() + "' WITH " + args[1].to_string() + " IN member_setting";
      auto [ok, response] = Api::Cursor::PostCursor::request(session.database, schemaQuery, {});
      auto slice = response->slices().front();
      if (slice.get("error").getBool()) { LOG_DEBUG << "Error: " << slice.toJson(); }
      else
      {
        ok::smart_actor::connection::addSuccess(resultMsg, event);
      }
    }
  };
  routeFunctions["form_schema_get"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    // this function return schema and initial data only.
    // edit data is returned by tableActor.
    // this type support object and array both form:
    if ((jsoncons::ObjectMemberIsObject(args, "f") && args["f"].empty()))
    {
      // get schemaKey:
      SchemaKey schemaKey;
      if (jsoncons::ObjectMemberIsString(args, "schema")) { schemaKey = args["schema"].as<std::string>(); }
      if (schemaKey.empty()) { return; }
      // session.database -> "global" // hack to pass permissions on signup schema get.
      auto [er, fields, allIds, options] = ok::table_actor::fetchFormDefaultInitialize("global", session.memberKey, "", schemaKey, args);
      if (ok::isEr(er)) { ok::smart_actor::connection::addFailure(resultMsg, event, er); }
      else
      {
        auto headerData = ok::mutate_schema::table_actor::getFormHeaderData(fields, allIds, options, jsoncons::ojson::array());
        auto resultData = ok::mutate_schema::table_actor::getFormDefaultObject(fields, allIds);
        headerData.merge(resultData);
        ok::smart_actor::connection::addEventAndJson(resultMsg, event, headerData);
      }
      // This only support objet form:
    }
    else if (jsoncons::ArrayPosIsString(args, 0))
    {
      std::string projections = ".form";
      if (jsoncons::ArrayPosIsString(args, 1)) { projections = args[1].as<std::string>(); }
      jsoncons::ojson r;
      auto [erS, myRespFormSchema] = ok::db::getSchema("global", "schema", args[0].as<std::string>(), projections);
      if (ok::isEr(erS))
      {
        // err
      }
      else
      {
        auto slice = myRespFormSchema->slices().front().get("result");
        ok::smart_actor::connection::addEventAndJson(resultMsg, event, jsoncons::ojson::parse(slice.toJson()));
      }
    }
    else
    {
      LOG_DEBUG << "unhandled event: " << event.to_string() << "args: " << args.to_string();
    }
  };
}
void addAccountRoute()
{
  routeFunctions["register_user"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
    else
    {
      currentActor->send(ok::smart_actor::supervisor::registerMutateActor, session, event, args, currentActor);
    }
  };
  routeFunctions["register_member"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
    else
    {
      currentActor->send(ok::smart_actor::supervisor::memberRegisterMutateActor, session, event, args, currentActor, subDomain);
    }
  };
  routeFunctions["login"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
    else
    {
      auto loginActor = currentActor->spawn(ok::smart_actor::auth::AuthLoginActor);
      currentActor->send(loginActor, session, event, args, currentActor);
    }
  };
  routeFunctions["member_login"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
    else
    {
      auto loginActor = currentActor->spawn(ok::smart_actor::auth::AuthMemberLoginActor);
      currentActor->send(loginActor, session, event, args, currentActor, subDomain);
    }
  };
  routeFunctions["logout"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
    else
    {
      currentActor->send(ok::smart_actor::supervisor::logoutMutateActor, session, event, currentActor);
    }
  };
  routeFunctions["confirm_email"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
    else
    {
      currentActor->send(ok::smart_actor::supervisor::confirmMutateActor, session, event, args, currentActor);
    }
  };
  routeFunctions["confirm_email_status"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
  };
  routeFunctions["get_menu"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
  };
  routeFunctions["is_logged_in"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    constexpr unsigned long milliseconds = 24 * 60 * 60 * 1000;
    if (!session.sessionKey.empty() && utils::time::getEpochMilliseconds() < session.sessionEpoch + milliseconds) { ok::smart_actor::connection::addSuccess(resultMsg, event); }
    else
    {
      ok::smart_actor::connection::addFailure(resultMsg, event, ok::ErrorCode::ERROR_NO_ERROR);
    }
  };
  routeFunctions["change_password"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
  };
  routeFunctions["lost_password"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
  };
}
auto checkProjectExist(std::string const &projectKey, std::string const &database)
{
  constexpr auto getProjectQuery = R"aql(FOR d IN project
        FILTER d.type == "project" and d._key == @key
        RETURN d)aql";
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("key", arangodb::velocypack::Value(projectKey));
  bindVars.close();
  return Api::Cursor::PostCursor::request(database, getProjectQuery, bindVars.slice());
}
auto findConversationAndRun(ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor,
                            Session const &session,
                            DocumentKey const &projectKey,
                            DocumentKey const &conversationKey,
                            WsEvent const &event,
                            WsArguments const &args,
                            std::function<void(jsoncons::ojson result)> onConversationFound)
{
  // get normal chatRoom actor (returns project's conversation - see conversation table schema)
  currentActor->request(ok::smart_actor::supervisor::globalActor, caf::infinite, global_list_atom_v, session.database, "conversation")
      .then(
          [=](table_actor_int<ws_connector_actor_int> conversationListActor)
          {
            // check if conversation exist under a project
            jsoncons::ojson findConversationUnderProjectArgs;
            jsoncons::ojson f;
            f["_key"] = "=\"" + conversationKey + "\"";
            findConversationUnderProjectArgs["f"] = f;
            findConversationUnderProjectArgs["project"] = projectKey;
            findConversationUnderProjectArgs["h"] = false;  // no header
            currentActor->request(conversationListActor, caf::infinite, get_initial_data_atom_v, event, findConversationUnderProjectArgs, session.memberKey, currentActor).then(onConversationFound);
          });
}
void addSuperRoutes()
{
  routeFunctions["user_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, ok::smart_actor::supervisor::userListActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["user_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, ok::smart_actor::supervisor::userMutateActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["schema_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, ok::smart_actor::supervisor::schemaListActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["schema_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, ok::smart_actor::supervisor::schemaMutateActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["translation_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, ok::smart_actor::supervisor::translationListActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["translation_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, ok::smart_actor::supervisor::translationMutateActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["session_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, ok::smart_actor::supervisor::sessionListActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["session_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, ok::smart_actor::supervisor::sessionMutateActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["confirm_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, ok::smart_actor::supervisor::confirmListActor, session, resultMsg, currentActor, isDispatching); };
  // routeFunctions["confirm_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, ok::smart_actor::supervisor::confirmMutateActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["super_color_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, ok::smart_actor::supervisor::colorListActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["super_color_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, ok::smart_actor::supervisor::colorMutateActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["super_permission_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, ok::smart_actor::supervisor::permissionListActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["super_permission_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, ok::smart_actor::supervisor::permissionMutateActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["super_menu_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, ok::smart_actor::supervisor::menuListActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["super_menu_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, ok::smart_actor::supervisor::menuMutateActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["super_node_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, ok::smart_actor::supervisor::nodeListActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["super_node_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, ok::smart_actor::supervisor::nodeMutateActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["super_edge_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, ok::smart_actor::supervisor::edgeListActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["super_edge_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, ok::smart_actor::supervisor::edgeMutateActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["super_attribute_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, ok::smart_actor::supervisor::attributeListActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["super_attribute_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, ok::smart_actor::supervisor::attributeMutateActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["super_role_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, ok::smart_actor::supervisor::roleListActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["super_role_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, ok::smart_actor::supervisor::roleMutateActor, session, resultMsg, currentActor, isDispatching); };
  // routeFunctions["support_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, "support", session, resultMsg, currentActor, isDispatching); };
  // routeFunctions["support_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, "support", session, resultMsg, currentActor, isDispatching); };
  // routeFunctions["support_admin_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, "support_admin", session, resultMsg, currentActor, isDispatching); };
  // routeFunctions["support_admin_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, "support_admin", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["template_list"] = [](RouteArgs) { handleSuperList(valin, eventNo, ok::smart_actor::supervisor::templateListActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["template_mutate"] = [](RouteArgs) { handleSuperMutate(valin, eventNo, ok::smart_actor::supervisor::templateMutateActor, session, resultMsg, currentActor, isDispatching); };
  routeFunctions["restart_server"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
    else
    {
      // std::exit(1);
      jsoncons::ojson result = jsoncons::ojson::array();
      jsoncons::ojson one = jsoncons::ojson::array();
      one.push_back(event);
      one.push_back(123);
      result.push_back(one);
      currentActor->send(currentActor, caf::forward_atom_v, result);
    }
  };
  routeFunctions["test_mail"] = [](RouteArgs)
  {
    auto connectionActor = currentActor;
    connectionActor->send(ok::smart_actor::supervisor::emailMutateActor, send_email_atom_v, "", "kapilpipaliya@gmail.com", "Test Email", "This is the test email\n");
  };
  routeFunctions["read_log"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
    else
    {
      auto content = pystring::read_all("./drogon.log");
      jsoncons::ojson j = content;
      ok::smart_actor::connection::addEventAndJson(resultMsg, event, j);
    }
  };
  routeFunctions["subscribe_online_users"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
    else
    {
      auto const event_type = event[0].as_string_view();
      if (event_type == "subscribe")
      {
        // currentActor->send(ok::smart_actor::supervisor::mainActor, subscribe_to_total_ws_connections_atom_v, event, args, currentActor);
      }
      else if (event_type == "subscribe")
      {
        // currentActor->send(ok::smart_actor::supervisor::mainActor, unsubscribe_to_total_ws_connections_atom_v, event, currentActor);
      }
    }
  };
  routeFunctions["countries"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    std::string q = R"aql(FOR c in schema
   FILTER c._key == 'countries'

   LET countries = c.form.country
   LET o = (FOR k IN ATTRIBUTES(countries, FALSE, TRUE)
   RETURN [k, countries[k].info.longName])

   RETURN o)aql";
    auto [erDb, response] = Api::Cursor::PostCursor::request("global", q, {});
    if (isEr(erDb)) { ok::smart_actor::connection::addFailure(resultMsg, event, ok::ErrorCode::BAD_QUERY); }
    else
    {
      auto resp = std::move(response);
      auto slice = resp->slices().front();
      if (slice.get("result").length() == 1) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, slice.get("result")[0]); }
      else
      {
        ok::smart_actor::connection::addFailure(resultMsg, event, ok::ErrorCode::BAD_QUERY);
      }
    }
  };
  routeFunctions["languages"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    // Todo fix this:
    std::string q = R"aql(FOR c in schema
   FILTER c._key == 'timezones'

   LET o = (FOR k IN c.form
   RETURN [k.abbr, k.text])

   RETURN o)aql";
    auto [erDb, response] = Api::Cursor::PostCursor::request("global", q, {});
    if (isEr(erDb)) { ok::smart_actor::connection::addFailure(resultMsg, event, ok::ErrorCode::BAD_QUERY); }
    else
    {
      auto slice = response->slices().front();
      if (slice.get("result").length() == 1) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, slice.get("result")[0]); }
      else
      {
        ok::smart_actor::connection::addFailure(resultMsg, event, ok::ErrorCode::BAD_QUERY);
      }
    }
  };
  routeFunctions["timezones"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    std::string q = R"aql(FOR c in schema
   FILTER c._key == 'timezones'

   LET o = (FOR k IN c.form
   RETURN [k.value, k.text])

   RETURN o)aql";
    auto [erDb, response] = Api::Cursor::PostCursor::request("global", q, {});
    if (isEr(erDb)) { ok::smart_actor::connection::addFailure(resultMsg, event, ok::ErrorCode::BAD_QUERY); }
    else
    {
      auto slice = response->slices().front();
      if (slice.get("result").length() == 1) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, slice.get("result")[0]); }
      else
      {
        ok::smart_actor::connection::addFailure(resultMsg, event, ok::ErrorCode::BAD_QUERY);
      }
    }
  };
  routeFunctions["collections_size"] = [](RouteArgs)
  {
    std::string query;
    std::string return_query = "RETURN [";
    if (auto [er, myResp] = ok::db::getSchema("global", "schema", "seed", ".form"); ok::isEr(er))
    {
      // error
    }
    else
    {
      auto slice = myResp->slices().front().get("result");
      if (slice.length() == 1)
      {
        auto array = slice[0];
        auto collections = array.get("collections");
        for (auto const &it : arangodb::velocypack::ArrayIterator(collections))
        {
          auto c = it.copyString();
          query += "LET " + c + " = LENGTH(" + c + ")\n";
          return_query += "CONCAT('" + c + ": ', " + c + "),\n";
        }
        auto edgesArray = array.get("edges");
        for (auto const &it : arangodb::velocypack::ArrayIterator(edgesArray))
        {
          auto c = it.copyString();
          query += "LET " + c + " = LENGTH(" + c + ")\n";
          return_query += "CONCAT('" + c + ": ', " + c + "),\n";
        }
        return_query += "]";
        LOG_DEBUG << query << " " << return_query;
      }
    }
  };
  routeFunctions["get_uuid"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    auto uuid = drogon::utils::getUuid();
    jsoncons::ojson result;
    result["uuid"] = uuid;
    ok::smart_actor::connection::addEventAndJson(resultMsg, event, result);
  };
}
void addAdminRoutes()
{
  routeFunctions["setting_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "setting", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["setting_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "setting", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["organization_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "organization", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["organization_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "organization", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["project_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "project", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["project_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "project", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["group_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "group", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["group_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "group", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["role_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "role", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["role_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "role", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["organization_group_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "organization_group", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["organization_group_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "organization_group", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["organization_role_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "organization_role", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["organization_role_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "organization_role", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["project_group_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "project_group", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["project_group_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "project_group", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["project_role_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "project_role", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["project_role_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "project_role", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["member_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "member", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["member_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "member", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["permission_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "permission", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["permission_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "permission", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["menu_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "menu", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["menu_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "menu", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["color_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "color", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["color_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "color", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["type_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "type", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["type_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "type", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["priority_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "priority", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["priority_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "priority", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["status_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "state", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["status_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "state", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["activity_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "activity", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["activity_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "activity", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["doc_category_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "doc_category", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["doc_category_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "doc_category", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["announcement_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "announcement", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["announcement_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "announcement", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["news_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "news", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["news_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "news", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["wiki_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "wiki", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["wiki_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "wiki", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["work_package_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "work_package", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["work_package_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "work_package", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["forum_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "forum", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["forum_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "forum", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["post_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "post", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["post_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "post", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["preferences_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "preferences", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["preferences_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "preferences", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["conversation_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "conversation", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["conversation_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "conversation", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["alias_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "alias", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["alias_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "alias", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["chat_department_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "chat_department", session, resultMsg, currentActor, isDispatching); };
  routeFunctions["chat_department_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "chat_department", session, resultMsg, currentActor, isDispatching); };
  //  routeFunctions["message_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "message", session, resultMsg, currentActor, isDispatching); };
  //  routeFunctions["message_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "message", session, resultMsg, currentActor, isDispatching); };
  //  routeFunctions["participant_list"] = [](RouteArgs) { handleUserList(valin, eventNo, "participant", session, resultMsg, currentActor, isDispatching); };
  //  routeFunctions["participant_mutate"] = [](RouteArgs) { handleUserMutate(valin, eventNo, "participant", session, resultMsg, currentActor, isDispatching); };
}
void handleSuperList(jsoncons::ojson const &valin,
                     unsigned int eventNo,
                     table_actor_int<ws_connector_actor_int> &listActor,
                     Session const &session,
                     jsoncons::ojson &resultMsg,
                     ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor,
                     bool isDispatching)
{
  auto &event = valin[eventNo][0];
  auto &args = valin[eventNo][1];
  if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
  else
  {
    auto const event_type = event[0].as_string_view();
    if (event_type == "get") { currentActor->send(listActor, send_to_one_atom_v, event, args, session.memberKey, currentActor); }
    else if (event_type == "subscribe")
    {
      currentActor->request(listActor, caf::infinite, caf::subscribe_atom_v, event, args, session.memberKey, currentActor)
          .then(
              [=](bool isSubscribed)
              {
                if (isSubscribed)
                {
                  if (std::find(currentActor->state.subscriptions.begin(), currentActor->state.subscriptions.end(), listActor) != currentActor->state.subscriptions.end())
                  {
                    currentActor->state.subscriptions.push_back(listActor);
                  }
                }
              });
    }
    else if (event_type == "unsubscribe")
    {
      currentActor->send(listActor, caf::unsubscribe_atom_v, event, currentActor);
      if (auto find_position = std::find(currentActor->state.subscriptions.begin(), currentActor->state.subscriptions.end(), listActor); find_position != currentActor->state.subscriptions.end())
      {
        currentActor->state.subscriptions.erase(find_position);
      }
    }
  }
}
void handleSuperMutate(jsoncons::ojson const &valin,
                       unsigned int eventNo,
                       base_mutate_actor_int<ws_connector_actor_int> &mutateActor,
                       Session const &session,
                       jsoncons::ojson &resultMsg,
                       ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor,
                       bool isDispatching)
{
  auto &event = valin[eventNo][0];
  auto &args = valin[eventNo][1];
  if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
  else
  {
    currentActor->send(mutateActor, insert_atom_v, session.memberKey, event, args, jsoncons::ojson{}, true, false, currentActor);
  }
}
void sendToListActor(WsEvent const &event,
                     WsArguments const &args,
                     Session const &session,
                     ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor,
                     table_actor_int<ws_connector_actor_int> listActor)
{
  auto const event_type = event[0].as_string_view();
  if (event_type == "get") { currentActor->send(listActor, send_to_one_atom_v, event, args, session.memberKey, currentActor); }
  else if (event_type == "subscribe")
  {
    currentActor->request(listActor, caf::infinite, caf::subscribe_atom_v, event, args, session.memberKey, currentActor)
        .then(
            [=](bool isSubscribed)
            {
              if (isSubscribed)
              {
                if (std::find(currentActor->state.subscriptions.begin(), currentActor->state.subscriptions.end(), listActor) == currentActor->state.subscriptions.end())
                {
                  currentActor->state.subscriptions.push_back(listActor);
                }
              }
            });
  }
  else if (event_type == "unsubscribe")
  {
    currentActor->send(listActor, caf::unsubscribe_atom_v, event, currentActor);
    if (auto find_position = std::find(currentActor->state.subscriptions.begin(), currentActor->state.subscriptions.end(), listActor); find_position != currentActor->state.subscriptions.end())
    {
      currentActor->state.subscriptions.erase(find_position);
    }
  }
}
void handleUserList(jsoncons::ojson const &valin,
                    unsigned int eventNo,
                    std::string const &userKey,
                    Session const &session,
                    jsoncons::ojson &resultMsg,
                    ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor,
                    bool isDispatching)
{
  auto &event = valin[eventNo][0];
  auto &args = valin[eventNo][1];
  if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
  else
  {
    currentActor->request(ok::smart_actor::supervisor::globalActor, caf::infinite, global_list_atom_v, session.database, userKey)
        .then([=](table_actor_int<ws_connector_actor_int> listActor) { sendToListActor(event, args, session, currentActor, listActor); });
  }
}
void handleUserMutate(jsoncons::ojson const &valin,
                      unsigned int eventNo,
                      std::string const &userKey,
                      Session const &session,
                      jsoncons::ojson &resultMsg,
                      ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor,
                      bool isDispatching)
{
  auto &event = valin[eventNo][0];
  auto &args = valin[eventNo][1];
  if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
  else
  {
    currentActor->request(ok::smart_actor::supervisor::globalActor, caf::infinite, user_mutate_atom_v, session.database, userKey)
        .then([=](base_mutate_actor_int<ws_connector_actor_int> mutateActor)
              { currentActor->send(mutateActor, insert_atom_v, session.memberKey, event, args, jsoncons::ojson{}, true, false, currentActor); });
  }
}
void sendConversationClosed(WsEvent const &event,
                            ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor,
                            std::string description = "conversation closed")
{
  jsoncons::ojson conversationClosedJson;
  conversationClosedJson["description"] = description;
  conversationClosedJson["error"] = true;
  auto msg = ok::smart_actor::connection::wsMessageBase();
  ok::smart_actor::connection::addEventAndJson(msg, event, conversationClosedJson);
  ok::smart_actor::connection::sendJson(currentActor->state.wsConnPtr, msg);
}
void projectNotFound(WsEvent const &event, ok::ErrorCode &erDb, ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor)
{
  jsoncons::ojson projectNotExistJson;
  projectNotExistJson["description"] = "project not exist (" + ok::errno_string(erDb) + ")";
  projectNotExistJson["error"] = true;
  auto msg = ok::smart_actor::connection::wsMessageBase();
  ok::smart_actor::connection::addEventAndJson(msg, event, projectNotExistJson);
  ok::smart_actor::connection::sendJson(currentActor->state.wsConnPtr, msg);
}
void downloadTranscript(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback)
{
  drogon::HttpResponsePtr resp = drogon::HttpResponse::newHttpResponse();
  auto session = ok::api::file::impl::getSession(req);
  std::string projectKey = req->getParameter("project");
  std::string conversationKey = req->getParameter("conversation");
  std::string participantKey = req->getParameter("participant");
  if (true)
  {
    resp->setBody("Chat Transcript");
    callback(resp);
  }
  else
  {
    resp->setStatusCode(drogon::k401Unauthorized);
    resp->setBody("UnAuthorized request");
    callback(resp);
  }
}
void sendConversationKey(WsEvent const &event, DocumentKey const &conversationKey, ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor)
{
  auto msg = ok::smart_actor::connection::wsMessageBase();
  jsoncons::ojson successJson;
  successJson["error"] = false;
  successJson["conversation"] = conversationKey;
  ok::smart_actor::connection::addEventAndJson(msg, event, successJson);
  ok::smart_actor::connection::sendJson(currentActor->state.wsConnPtr, msg);
}
void addChatRoutes()
{
  drogon::app().registerHandler("/api/download_transcript.txt", &downloadTranscript, {drogon::Get});
  /*
   * find project if not exist send error
   * find conversation if not exist and guest create new conversation else send error
   * find chatRoom actor and send subscribe event to it.
   * */
  routeFunctions["conversation"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    auto projectKey = jsoncons::getStringKey(args, "project");
    auto const event_type = event[0].as_string_view();
    if (event_type == "subscribe")
    {
      auto [erDb, response] = checkProjectExist(projectKey, session.database);
      if (!isEr(erDb) && response->slices().front().get("result").length() == 1)
      {
        auto conversationKey = jsoncons::getStringKey(args, "conversation");
        findConversationAndRun(currentActor,
                               session,
                               projectKey,
                               conversationKey,
                               event,
                               args,
                               [=](jsoncons::ojson result)
                               {
                                 // send to the appropriate chat room actor under project and conversation key and save the chatRoomActor to unusubscribe when connection exit.
                                 auto sendToChatRoomActor = [=](std::string conversationKey)
                                 {
                                   currentActor->request(ok::smart_actor::supervisor::globalActor, caf::infinite, get_chat_room_atom_v, session.database, projectKey, conversationKey)
                                       .then(
                                           [=](chat_room_actor_int chatRoomActor)
                                           {
                                             currentActor->request(chatRoomActor, caf::infinite, caf::subscribe_atom_v, event, args, session.memberKey, currentActor)
                                                 .then(
                                                     [=](bool isSubscribed)
                                                     {
                                                       if (isSubscribed)
                                                       {
                                                         if (std::find(currentActor->state.chat_subscriptions.begin(), currentActor->state.chat_subscriptions.end(), chatRoomActor) ==
                                                             currentActor->state.chat_subscriptions.end())
                                                         {
                                                           currentActor->state.chat_subscriptions.push_back(chatRoomActor);
                                                         }
                                                       }
                                                     });
                                           });
                                 };
                                 if (jsoncons::ObjectMemberBoolVal(args, "isAdmin"))
                                 {
                                   if (result["r"]["result"].size() == 0) sendConversationClosed(event, currentActor, "conversation not exist");
                                   else
                                     sendToChatRoomActor(conversationKey);
                                 }
                                 else if (result["r"]["result"].size() == 0 || (result["r"]["result"].size() == 1 && jsoncons::ObjectMemberIsString(result["r"]["result"][0], "state") &&
                                                                                result["r"]["result"][0]["state"].as_string() == "closed"))
                                 {
                                   currentActor->request(ok::smart_actor::supervisor::globalActor, caf::infinite, user_mutate_atom_v, session.database, "conversation")
                                       .then(
                                           [=](base_mutate_actor_int<ws_connector_actor_int> mutateActor)
                                           {
                                             auto mutateEvent = event;
                                             mutateEvent[0] = "insert";
                                             jsoncons::ojson newMutateArgs;
                                             newMutateArgs["project"] = projectKey;
                                             jsoncons::ojson value;
                                             value["_key"] = "";
                                             value["title"] = "New Conversation";  // Choose random word
                                             value["description"] = "";
                                             newMutateArgs["value"] = value;
                                             jsoncons::ojson backendValue;
                                             backendValue["state"] = "active";
                                             currentActor
                                                 ->request(mutateActor, caf::infinite, insert_and_get_atom_v, session.memberKey, mutateEvent, newMutateArgs, backendValue, true, false, currentActor)
                                                 .then(
                                                     [=](ok::ErrorCode er, std::unordered_set<DocumentKey> changedKeys)
                                                     {
                                                       if (changedKeys.size() > 0)
                                                       {
                                                         sendToChatRoomActor(*changedKeys.begin());
                                                         sendConversationKey(event, *changedKeys.begin(), currentActor);
                                                       }
                                                     });
                                           });
                                 }
                                 else
                                   sendToChatRoomActor(conversationKey);
                               });
      }
      else
        projectNotFound(event, erDb, currentActor);
    }
    /*
     * find project if not exist send error
     * find conversation if not exist send error
     * find chatRoom actor and send unsubscribe event to it.
     * */
    else if (event_type == "unsubscribe")
    {
      auto [erDb, response] = checkProjectExist(projectKey, session.database);
      if (!isEr(erDb) && response->slices().front().get("result").length() == 1)
      {
        auto conversationKey = jsoncons::getStringKey(args, "conversation");
        findConversationAndRun(currentActor,
                               session,
                               projectKey,
                               conversationKey,
                               event,
                               args,
                               [=](jsoncons::ojson result)
                               {
                                 auto sendToChatRoomActor = [=](std::string conversationKey, std::string conversationRevKey)
                                 {
                                   currentActor->request(ok::smart_actor::supervisor::globalActor, caf::infinite, get_chat_room_atom_v, session.database, projectKey, conversationKey)
                                       .then(
                                           [=](chat_room_actor_int chatRoomActor)
                                           {
                                             currentActor->send(chatRoomActor, caf::unsubscribe_atom_v, event, args, session.memberKey, conversationKey, conversationRevKey, currentActor);
                                             if (auto it = std::find(currentActor->state.chat_subscriptions.begin(), currentActor->state.chat_subscriptions.end(), chatRoomActor);
                                                 it != currentActor->state.chat_subscriptions.end())
                                             {
                                               currentActor->state.chat_subscriptions.erase(it);
                                             }
                                           });
                                 };
                                 if (result["r"]["result"].size() == 1) { sendToChatRoomActor(conversationKey, result["r"]["result"][0]["_rev"].as_string()); }
                                 else
                                   sendConversationClosed(event, currentActor, "conversation not exist");
                               });
      }
      else
        projectNotFound(event, erDb, currentActor);
    }
  };
  /*
   * find project if not exist send error
   * find conversation if not exist send error
   * find chatRoom actor and send conversation_end event to it.
   * */
  routeFunctions["conversation_end"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    auto projectKey = jsoncons::getStringKey(args, "project");
    auto [erDb, response] = checkProjectExist(projectKey, session.database);
    if (!isEr(erDb) && response->slices().front().get("result").length() == 1)
    {
      auto conversationKey = jsoncons::getStringKey(args, "conversation");
      findConversationAndRun(currentActor,
                             session,
                             projectKey,
                             conversationKey,
                             event,
                             args,
                             [=](jsoncons::ojson result)
                             {
                               auto sendToChatRoomActor = [=](std::string conversationKey, std::string conversationRevKey)
                               {
                                 currentActor->request(ok::smart_actor::supervisor::globalActor, caf::infinite, get_chat_room_atom_v, session.database, projectKey, conversationKey)
                                     .then(
                                         [=](chat_room_actor_int chatRoomActor)
                                         {
                                           currentActor->send(chatRoomActor, end_conversation_atom_v, event, args, session.memberKey, conversationKey, conversationRevKey, currentActor);
                                           if (auto it = std::find(currentActor->state.chat_subscriptions.begin(), currentActor->state.chat_subscriptions.end(), chatRoomActor);
                                               it != currentActor->state.chat_subscriptions.end())
                                           {
                                             currentActor->state.chat_subscriptions.erase(it);
                                           }
                                         });
                               };
                               if (result["r"]["result"].size() == 1)
                               {
                                 if (jsoncons::ObjectMemberIsString(result["r"]["result"][0], "state") && result["r"]["result"][0]["state"].as_string() != "closed")
                                 {
                                   sendToChatRoomActor(conversationKey, result["r"]["result"][0]["_rev"].as_string());
                                 }
                                 else
                                   sendConversationClosed(event, currentActor, "conversation is closed");
                               }
                               else
                                 sendConversationClosed(event, currentActor, "conversation not exist");
                             });
    }
    else
      projectNotFound(event, erDb, currentActor);
  };
  //  routeFunctions["join_participant"] = [](RouteArgs)
  //  {
  //    auto &event = valin[eventNo][0];
  //    auto &args = valin[eventNo][1];
  //    auto projectKey = "";
  //    auto conversationKey = "";
  //    currentActor->request(ok::smart_actor::supervisor::globalActor, caf::infinite, get_chat_room_atom_v, session.database, projectKey, conversationKey)
  //        .then([=](chat_room_actor_int chatRoomActor) { currentActor->send(chatRoomActor, join_participant_atom_v, event, args, session.memberKey, currentActor); });
  //  };
  /*
   * find project if not exist send error
   * find conversation if not exist send error
   * find messageListActor from chatRoom actor and send current event to it.
   * */
  routeFunctions["message_list"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    auto projectKey = jsoncons::getStringKey(args, "project");
    auto [erDb, response] = checkProjectExist(projectKey, session.database);
    if (!isEr(erDb) && response->slices().front().get("result").length() == 1)
    {
      auto conversationKey = jsoncons::getStringKey(args, "conversation");
      findConversationAndRun(currentActor,
                             session,
                             projectKey,
                             conversationKey,
                             event,
                             args,
                             [=](jsoncons::ojson result)
                             {
                               auto sendToChatRoomActor = [=](std::string conversationKey)
                               {
                                 currentActor->request(ok::smart_actor::supervisor::globalActor, caf::infinite, get_chat_room_atom_v, session.database, projectKey, conversationKey)
                                     .then(
                                         [=](chat_room_actor_int chatRoomActor)
                                         {
                                           currentActor->request(chatRoomActor, caf::infinite, get_message_list_actor_atom_v)
                                               .then([=](table_actor_int<ws_connector_actor_int> messageListActor) { sendToListActor(event, args, session, currentActor, messageListActor); });
                                         });
                               };
                               if (result["r"]["result"].size() == 1)
                               {
                                 if (jsoncons::ObjectMemberBoolVal(args, "isAdmin") ||
                                     (jsoncons::ObjectMemberIsString(result["r"]["result"][0], "state") && result["r"]["result"][0]["state"] != "closed"))
                                 {
                                   sendToChatRoomActor(conversationKey);
                                 }
                                 else
                                   sendConversationClosed(event, currentActor, "conversation is closed");
                               }
                               else
                                 sendConversationClosed(event, currentActor, "conversation not exist");
                             });
    }
    else
      projectNotFound(event, erDb, currentActor);
  };
  /*
   * find project if not exist send error
   * find conversation if not exist send error
   * find chatRoom actor and send create_message_atom_v event to it.
   * */
  routeFunctions["message_mutate"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    auto projectKey = jsoncons::getStringKey(args, "project");
    auto [erDb, response] = checkProjectExist(projectKey, session.database);
    if (!isEr(erDb) && response->slices().front().get("result").length() == 1)
    {
      auto conversationKey = jsoncons::getStringKey(args, "conversation");
      findConversationAndRun(currentActor,
                             session,
                             projectKey,
                             conversationKey,
                             event,
                             args,
                             [=](jsoncons::ojson result)
                             {
                               auto sendToChatRoomActor = [=](std::string conversationKey, std::string conversationRevKey)
                               {
                                 currentActor->request(ok::smart_actor::supervisor::globalActor, caf::infinite, get_chat_room_atom_v, session.database, projectKey, conversationKey)
                                     .then([=](chat_room_actor_int chatRoomActor)
                                           { currentActor->send(chatRoomActor, create_message_atom_v, event, args, session.memberKey, conversationKey, conversationRevKey, currentActor); });
                               };
                               if (result["r"]["result"].size() == 1)
                               {
                                 if (jsoncons::ObjectMemberBoolVal(args, "isAdmin") ||
                                     (jsoncons::ObjectMemberIsString(result["r"]["result"][0], "state") && result["r"]["result"][0]["state"] != "closed"))
                                 {
                                   sendToChatRoomActor(conversationKey, result["r"]["result"][0]["_rev"].as_string());
                                 }
                                 else
                                   sendConversationClosed(event, currentActor, "conversation is closed");
                               }
                               else
                                 sendConversationClosed(event, currentActor, "conversation not exist");
                             });
    }
    else
      projectNotFound(event, erDb, currentActor);
  };
  /*
   * find project if not exist send error
   * find conversation if not exist send error
   * find participantListActor from chatRoom actor and send current event to it.
   * */
  routeFunctions["participant_list"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    auto projectKey = jsoncons::getStringKey(args, "project");
    auto [erDb, response] = checkProjectExist(projectKey, session.database);
    if (!isEr(erDb) && response->slices().front().get("result").length() == 1)
    {
      auto conversationKey = jsoncons::getStringKey(args, "conversation");
      findConversationAndRun(currentActor,
                             session,
                             projectKey,
                             conversationKey,
                             event,
                             args,
                             [=](jsoncons::ojson result)
                             {
                               auto sendToChatRoomActor = [=](std::string conversationKey)
                               {
                                 currentActor->request(ok::smart_actor::supervisor::globalActor, caf::infinite, get_chat_room_atom_v, session.database, projectKey, conversationKey)
                                     .then(
                                         [=](chat_room_actor_int chatRoomActor)
                                         {
                                           currentActor->request(chatRoomActor, caf::infinite, get_participant_list_actor_atom_v)
                                               .then([=](table_actor_int<ws_connector_actor_int> participantListActor) { sendToListActor(event, args, session, currentActor, participantListActor); });
                                         });
                               };
                               if (result["r"]["result"].size() == 1)
                               {
                                 if (jsoncons::ObjectMemberIsString(result["r"]["result"][0], "state") && result["r"]["result"][0]["state"] != "closed") { sendToChatRoomActor(conversationKey); }
                                 else
                                   sendConversationClosed(event, currentActor, "conversation is closed");
                               }
                               else
                                 sendConversationClosed(event, currentActor, "conversation not exist");
                             });
    }
    else
      projectNotFound(event, erDb, currentActor);
  };
  /*
   * find project if not exist send error
   * find conversation if not exist send error
   * find chatRoom actor and send modify_participan_atom_v event to it.
   * */
  routeFunctions["participant_mutate"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    auto projectKey = jsoncons::getStringKey(args, "project");
    auto [erDb, response] = checkProjectExist(projectKey, session.database);
    if (!isEr(erDb) && response->slices().front().get("result").length() == 1)
    {
      auto conversationKey = jsoncons::getStringKey(args, "conversation");
      findConversationAndRun(currentActor,
                             session,
                             projectKey,
                             conversationKey,
                             event,
                             args,
                             [=](jsoncons::ojson result)
                             {
                               auto sendToChatRoomActor = [=](std::string conversationKey)
                               {
                                 currentActor->request(ok::smart_actor::supervisor::globalActor, caf::infinite, get_chat_room_atom_v, session.database, projectKey, conversationKey)
                                     .then([=](chat_room_actor_int chatRoomActor) { currentActor->send(chatRoomActor, modify_participan_atom_v, event, args, session.memberKey, currentActor); });
                               };
                               if (result["r"]["result"].size() == 1)
                               {
                                 if (jsoncons::ObjectMemberIsString(result["r"]["result"][0], "state") && result["r"]["result"][0]["state"] != "closed") { sendToChatRoomActor(conversationKey); }
                                 else
                                   sendConversationClosed(event, currentActor, "conversation is closed");
                               }
                               else
                                 sendConversationClosed(event, currentActor, "conversation not exist");
                             });
    }
    else
      projectNotFound(event, erDb, currentActor);
  };
  /*
   * find project if not exist send error
   * find conversation if not exist send error
   * find chatRoom actor and send typing_event_atom_v event to it.
   * */
  routeFunctions["typing_event"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    auto projectKey = jsoncons::getStringKey(args, "project");
    auto [erDb, response] = checkProjectExist(projectKey, session.database);
    if (!isEr(erDb) && response->slices().front().get("result").length() == 1)
    {
      auto conversationKey = jsoncons::getStringKey(args, "conversation");
      findConversationAndRun(currentActor,
                             session,
                             projectKey,
                             conversationKey,
                             event,
                             args,
                             [=](jsoncons::ojson result)
                             {
                               auto sendToChatRoomActor = [=](std::string conversationKey)
                               {
                                 currentActor->request(ok::smart_actor::supervisor::globalActor, caf::infinite, get_chat_room_atom_v, session.database, projectKey, conversationKey)
                                     .then([=](chat_room_actor_int chatRoomActor) { currentActor->send(chatRoomActor, typing_event_atom_v, event, args, session.memberKey, currentActor); });
                               };
                               if (result["r"]["result"].size() == 1)
                               {
                                 if (jsoncons::ObjectMemberIsString(result["r"]["result"][0], "state") && result["r"]["result"][0]["state"] != "closed") { sendToChatRoomActor(conversationKey); }
                                 else
                                   sendConversationClosed(event, currentActor, "conversation is closed");
                               }
                               else
                                 sendConversationClosed(event, currentActor, "conversation not exist");
                             });
    }
    else
      projectNotFound(event, erDb, currentActor);
  };
  /*
   * find project if not exist send error
   * find conversation if not exist send error
   * find chatRoom actor and send join_participan_atom_v event to it.
   * */
  routeFunctions["join_participant"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    auto projectKey = jsoncons::getStringKey(args, "project");
    auto [erDb, response] = checkProjectExist(projectKey, session.database);
    if (!isEr(erDb) && response->slices().front().get("result").length() == 1)
    {
      auto conversationKey = jsoncons::getStringKey(args, "conversation");
      findConversationAndRun(currentActor,
                             session,
                             projectKey,
                             conversationKey,
                             event,
                             args,
                             [=](jsoncons::ojson result)
                             {
                               auto sendToChatRoomActor = [=](std::string conversationKey)
                               {
                                 currentActor->request(ok::smart_actor::supervisor::globalActor, caf::infinite, get_chat_room_atom_v, session.database, projectKey, conversationKey)
                                     .then([=](chat_room_actor_int chatRoomActor) { currentActor->send(chatRoomActor, join_participant_atom_v, event, args, session.memberKey, currentActor); });
                               };
                               if (result["r"]["result"].size() == 1)
                               {
                                 if (jsoncons::ObjectMemberIsString(result["r"]["result"][0], "state") && result["r"]["result"][0]["state"] != "closed") { sendToChatRoomActor(conversationKey); }
                                 else
                                   sendConversationClosed(event, currentActor, "conversation is closed");
                               }
                               else
                                 sendConversationClosed(event, currentActor, "conversation not exist");
                             });
    }
    else
      projectNotFound(event, erDb, currentActor);
  };
  /*
   * get chat statistic for current database
   * */
  routeFunctions["chat_statistics"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto [er, result] = ok::db::chat::chat_statistics(session.database);
    if (ok::isEr(er)) return;
    else
    {
      jsoncons::ojson chatStatisticsJson;
      chatStatisticsJson["statistics"] = result;
      chatStatisticsJson["error"] = true;
      auto msg = ok::smart_actor::connection::wsMessageBase();
      ok::smart_actor::connection::addEventAndJson(msg, event, chatStatisticsJson);
      ok::smart_actor::connection::sendJson(currentActor->state.wsConnPtr, msg);
    }
  };
};
}  // namespace impl
}  // namespace connection
}  // namespace ok::smart_actor
