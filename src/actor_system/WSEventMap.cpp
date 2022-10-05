#include "actor_system/WSEventMap.hpp"
#include "actor_system/WsConnectionActor.hpp"
#include "db/Session.hpp"
#include "utils/BatchArrayMessage.hpp"
#include "utils/ErrorConstants.hpp"
#include <jsoncons/json.hpp>
#include <magic_enum.hpp>
#include <utility>
#include "actor_system/Routes.hpp"
#include "caf/all.hpp"
#include "pystring.hpp"
#include "utils/json_functions.hpp"
#include "utils/time_functions.hpp"
#include "utils/os_functions.hpp"
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
    }
  };
  routeFunctions["member_setting_mutate"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
  };
}
void addAccountRoute()
{
  routeFunctions["register_user"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    // if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
    // else
    // {
    //   currentActor->send(ok::smart_actor::supervisor::registerMutateActor, session, event, args, currentActor);
    // }
  };
  routeFunctions["register_member"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];

  };
  routeFunctions["login"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    // if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
    // else
    // {
    //   auto loginActor = currentActor->spawn(ok::smart_actor::auth::AuthLoginActor);
    //   currentActor->send(loginActor, session, event, args, currentActor);
    // }
  };
  routeFunctions["member_login"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    // if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
    // else
    // {
    //   auto loginActor = currentActor->spawn(ok::smart_actor::auth::AuthMemberLoginActor);
    //   currentActor->send(loginActor, session, event, args, currentActor, subDomain);
    // }
  };
  routeFunctions["logout"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
    // if (isDispatching) { ok::smart_actor::connection::addEventAndJson(resultMsg, event, args); }
    // else
    // {
    //   currentActor->send(ok::smart_actor::supervisor::logoutMutateActor, session, event, currentActor);
    // }
  };
  routeFunctions["confirm_email"] = [](RouteArgs)
  {
    auto &event = valin[eventNo][0];
    auto &args = valin[eventNo][1];
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

void addSuperRoutes()
{
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

  };
  routeFunctions["countries"] = [](RouteArgs)
  {
 
  };
  routeFunctions["languages"] = [](RouteArgs)
  {
 
  };
  routeFunctions["timezones"] = [](RouteArgs)
  {

  };
  routeFunctions["collections_size"] = [](RouteArgs)
  {
 
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

}  // namespace impl
}  // namespace connection
}  // namespace ok::smart_actor
