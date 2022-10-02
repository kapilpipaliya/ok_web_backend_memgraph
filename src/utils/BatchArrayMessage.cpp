#include "utils/BatchArrayMessage.hpp"
#include <trantor/utils/Logger.h>
#include "utils/html_functions.hpp"
#include "db/db_functions.hpp"
#include "utils/ErrorConstants.hpp"
// #include "MutateSchema.hpp"
#include "drogon/Cookie.h"
namespace ok::smart_actor
{
namespace connection
{
jsoncons::ojson wsMessageBase() noexcept { return jsoncons::ojson::array(); }
jsoncons::ojson addEventAndJson(jsoncons::ojson &array, WsEvent const &event, WsArguments const &args) noexcept
{
  // [e, j]
  // if (!args.is_null()) {
  jsoncons::ojson one = jsoncons::ojson::array();
  one.push_back(event);
  one.push_back(args);
  array.push_back(one);
  return array;
  //}
}
jsoncons::ojson addEventAndTwoJsonArgs(jsoncons::ojson &array, WsEvent const &event, WsArguments const &args1, WsArguments const &args2) noexcept
{
  // [e, [j,j]]
  jsoncons::ojson one = jsoncons::ojson::array();
  one.push_back(event);
  jsoncons::ojson two = jsoncons::ojson::array();
  two.push_back(args1);
  two.push_back(args2);
  one.push_back(two);
  array.push_back(one);
  return array;
}
jsoncons::ojson addSuccess(jsoncons::ojson &array, WsEvent const &event) noexcept
{
  jsoncons::ojson one = jsoncons::ojson::array();
  one.push_back(event);
  jsoncons::ojson r = jsoncons::ojson::object();
  r["error"] = false;
  one.push_back(r);
  array.push_back(one);
  return array;
}
jsoncons::ojson addFailure(jsoncons::ojson &array, WsEvent const &event, const ok::ErrorCode errorCode) noexcept
{
  // [e,[0,"desc"]]
  jsoncons::ojson one = jsoncons::ojson::array();
  one.push_back(event);
  jsoncons::ojson r = jsoncons::ojson::object();
  auto isEr = ok::isEr(errorCode);
  r["error"] = isEr;
  if (isEr) r["description"] = ok::errno_string(errorCode);
  one.push_back(r);
  array.push_back(one);
  return array;
}
jsoncons::ojson addFailure(jsoncons::ojson &array, WsEvent const &event, const ErrorCode errorCode, const ok::mutate_schema::Fields &fields) noexcept
{
  // [e,[0,"desc", fields]]
  jsoncons::ojson one = jsoncons::ojson::array();
  one.push_back(event);
  jsoncons::ojson r = jsoncons::ojson::object();
  auto isEr = ok::isEr(errorCode);
  r["error"] = isEr;
  if (isEr)
  {
    r["description"] = ok::errno_string(errorCode);
    if (errorCode == ok::ErrorCode::ERROR_FORM_FIELD_ERROR || errorCode == ok::ErrorCode::UNIQUE_ERROR || errorCode == ok::ErrorCode::ERROR_FORM_EMPTY)
      r["fieldErrors"] = (ok::mutate_schema::fieldErrors(fields));
  }
  one.push_back(r);
  array.push_back(one);
  return array;
}
jsoncons::ojson addEventAndJson(jsoncons::ojson &array, WsEvent const &event, VPackSlice const &json) noexcept
{
  // if (!json.isNull()){
  jsoncons::ojson a = jsoncons::ojson::array();
  a.push_back(event);
  a.push_back(jsoncons::ojson::parse(json.toJson()));
  array.push_back(a);
  //}
  return array;
}
jsoncons::ojson addCurrentMember(jsoncons::ojson &array, Database const &database, DocumentKey const &memberKey) noexcept
{
  // [e,member]
  if (database.empty())
  {
    LOG_DEBUG << "Database Must not be empty, while fetching member.";
    return array;
  }
  WsEvent event = jsoncons::ojson::array();
  event.push_back("get");
  event.push_back("current_member_event");
  event.push_back(0);
  jsoncons::ojson one = jsoncons::ojson::array();
  one.push_back(event);
  jsoncons::ojson r;
  if (auto [er, myResp] = ok::db::getDocumentWithProjection(database, "member", memberKey, ""); ok::isEr(er))
  {
    // error
  }
  else
  {
    auto slice = myResp->slices().front().get("result");
    if (slice.length() == 1)
    {
      r["_key"] = slice[0].get("_key").copyString();
      r["email"] = slice[0].get("email").copyString();
      if (slice[0].hasKey("firstName")) r["firstName"] = slice[0].get("firstName").copyString();
      if (slice[0].hasKey("firstName")) r["lastName"] = slice[0].get("lastName").copyString();
    }
  }
  one.push_back(r);
  array.push_back(one);
  return array;
}
jsoncons::ojson addEmptyMember(jsoncons::ojson &array) noexcept
{
  // [e,member]
  WsEvent event = jsoncons::ojson::array();
  event.push_back("get");
  event.push_back("current_member_event");
  event.push_back(0);
  jsoncons::ojson one = jsoncons::ojson::array();
  one.push_back(event);
  jsoncons::ojson r;
  one.push_back(r);
  array.push_back(one);
  return array;
}
jsoncons::ojson addRedirection(jsoncons::ojson &array, std::string const &link, std::string const &msg, int const timeout) noexcept
{
  // [e,redirectUrl]
  WsEvent event = jsoncons::ojson::array();
  event.push_back("get");
  event.push_back("redirection_event");
  event.push_back(0);
  jsoncons::ojson one = jsoncons::ojson::array();
  one.push_back(event);
  jsoncons::ojson r = jsoncons::ojson::array();
  r.push_back(link);
  if (timeout > 0)
  {
    r.push_back(timeout);
    r.push_back(msg);
  }
  one.push_back(r);
  array.push_back(one);
  return array;
}
jsoncons::ojson addJwtCookie(jsoncons::ojson &array, std::map<std::string, std::string> const &cookie, long maxAge) noexcept
{
  WsEvent event = jsoncons::ojson::array();
  event.push_back("get");
  event.push_back("cookie_event");
  event.push_back(0);
  jsoncons::ojson one = jsoncons::ojson::array();
  one.push_back(event);
  jsoncons::ojson r;
  jsoncons::ojson obj;
  obj["jwt"] = ok::utils::jwt_functions::encodeCookie(cookie);
  r["cookie"] = obj;
  r["max_age"] = maxAge;
  // drogon::Cookie loginCookie{"jwt", ok::utils::jwt_functions::encodeCookie(cookie)};
  // no function to set max_age on drogon
  // auto cookieStr = loginCookie.cookieString();
  one.push_back(r);
  array.push_back(one);
  return array;
}
jsoncons::ojson addSimpleCookie(jsoncons::ojson &array, std::map<std::string, std::string> const &cookie, long maxAge) noexcept
{
  WsEvent event = jsoncons::ojson::array();
  event.push_back("get");
  event.push_back("cookie_event");
  event.push_back(0);
  jsoncons::ojson one = jsoncons::ojson::array();
  one.push_back(event);
  jsoncons::ojson r;
  jsoncons::ojson obj;
  obj.insert(std::begin(cookie), std::end(cookie));
  r["cookie"] = obj;
  r["max_age"] = maxAge;
  one.push_back(r);
  array.push_back(one);
  return array;
}
jsoncons::ojson addLogout(jsoncons::ojson &array) noexcept
{
  WsEvent event = jsoncons::ojson::array();
  event.push_back("get");
  event.push_back("logout");
  event.push_back(0);
  jsoncons::ojson one = jsoncons::ojson::array();
  one.push_back(event);
  jsoncons::ojson j;
  one.push_back(j);
  array.push_back(one);
  return array;
}
jsoncons::ojson addNotification(jsoncons::ojson &array, const NotificationType type, int const timeout) noexcept
{
  // Todo make this working.
  return array;
}
jsoncons::ojson addIsLoggedIn(jsoncons::ojson &array, bool v) noexcept
{
  auto loginEvent = jsoncons::ojson(jsoncons::json_array_arg, {"get", "is_logged_in", 0});
  if (v) { ok::smart_actor::connection::addSuccess(array, loginEvent); }
  else
  {
    ok::smart_actor::connection::addFailure(array, loginEvent, ok::ErrorCode::ERROR_FORM_EMPTY);
  }
  return array;
}
/*void setMemberFromJson(connection::Member &member, std::string const &json)
{
  try
  {
    member = jsoncons::decode_json<ok::smart_actor::connection::Member>(json);
  }
  catch (std::exception const &e)
  {
    LOG_DEBUG << "cant set member schema.";
    LOG_DEBUG << "member json: " << json;
    LOG_DEBUG << "reason: " << e.what();
  }
}*/
}  // namespace connection
}  // namespace ok::smart_actor
