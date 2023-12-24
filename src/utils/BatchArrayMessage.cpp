#include "utils/BatchArrayMessage.hpp"
#include <trantor/utils/Logger.h>
#include "db/auth_fns.hpp"
#include "db/mgclientPool.hpp"
#include "utils/html_functions.hpp"
// #include "MutateSchema.hpp"
#include "drogon/Cookie.h"
namespace ok::smart_actor
{
namespace connection
{
jsoncons::ojson wsMessageBase() noexcept
{
    return jsoncons::ojson::array();
}
jsoncons::ojson addEventAndJson(jsoncons::ojson &array,
                                WsEvent const &event,
                                WsArguments const &args) noexcept
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
jsoncons::ojson addEventAndTwoJsonArgs(jsoncons::ojson &array,
                                       WsEvent const &event,
                                       WsArguments const &args1,
                                       WsArguments const &args2) noexcept
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
jsoncons::ojson addSuccess(jsoncons::ojson &array,
                           WsEvent const &event) noexcept
{
    jsoncons::ojson one = jsoncons::ojson::array();
    one.push_back(event);
    jsoncons::ojson r = jsoncons::ojson::object();
    r["error"] = false;
    one.push_back(r);
    array.push_back(one);
    return array;
}
jsoncons::ojson addFailure(jsoncons::ojson &array,
                           WsEvent const &event,
                           std::string const description) noexcept
{
    jsoncons::ojson one = jsoncons::ojson::array();
    one.push_back(event);
    jsoncons::ojson r = jsoncons::ojson::object();
    r["error"] = true;
    r["description"] = description;
    one.push_back(r);
    array.push_back(one);
    return array;
}

jsoncons::ojson addCurrentMember(jsoncons::ojson &array,
                                 VertexId const &memberKey) noexcept
{
    auto [error, user] = ok::db::auth::user(memberKey);
    if (!error.empty())
    {
        LOG_ERROR << error;
        return array;
    }

    WsEvent event = jsoncons::ojson::array();
    event.push_back("get");
    event.push_back("member");
    event.push_back(0);
    jsoncons::ojson one = jsoncons::ojson::array();
    one.push_back(event);
    one.push_back(user);
    array.push_back(one);
    return array;
}
jsoncons::ojson addEmptyMember(jsoncons::ojson &array) noexcept
{
    // [e,member]
    WsEvent event = jsoncons::ojson::array();
    event.push_back("get");
    event.push_back("member");
    event.push_back(0);
    jsoncons::ojson one = jsoncons::ojson::array();
    one.push_back(event);
    jsoncons::ojson r;
    one.push_back(r);
    array.push_back(one);
    return array;
}

jsoncons::ojson addJwtCookie(jsoncons::ojson &array,
                             std::map<std::string, std::string> const &cookie,
                             long maxAge) noexcept
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
    // drogon::Cookie loginCookie{"jwt",
    // ok::utils::jwt_functions::encodeCookie(cookie)}; no function to set
    // max_age on drogon auto cookieStr = loginCookie.cookieString();
    one.push_back(r);
    array.push_back(one);
    return array;
}
jsoncons::ojson addSimpleCookie(
    jsoncons::ojson &array,
    std::map<std::string, std::string> const &cookie,
    long maxAge) noexcept
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

/*void setMemberFromJson(connection::Member &member, std::string const &json)
{
  try
  {
    member = jsoncons::decode_json<ok::smart_actor::connection::Member>(json);
  }
  catch (std::exception const &e)
  {
    LOG_ERROR << "cant set member schema.";
    LOG_ERROR << "member json: " << json;
    LOG_ERROR << "reason: " << e.what();
  }
}*/
}  // namespace connection
}  // namespace ok::smart_actor
