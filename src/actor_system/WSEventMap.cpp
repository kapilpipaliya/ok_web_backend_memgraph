#include "actor_system/WSEventMap.hpp"
#include "actor_system/WsConnectionActor.hpp"
#include "db/Session.hpp"
#include "db/auth_fns.hpp"
#include "utils/BatchArrayMessage.hpp"
#include <jsoncons/json.hpp>
#include <magic_enum.hpp>
#include <utility>
#include "caf/all.hpp"
#include "pystring.hpp"
#include "lib/json_functions.hpp"
#include "utils/time_functions.hpp"
namespace ok::smart_actor
{
namespace connection
{
#define RouteArgs                                              \
    jsoncons::ojson const &event, jsoncons::ojson const &args, \
        Session &session, jsoncons::ojson &resultMsg,          \
        ws_connector_actor_int::stateful_pointer<              \
            ok::smart_actor::connection::ws_controller_state>  \
            currentActor, std::string const &subDomain
namespace impl
{
void addAuthRoutes()
{
    routeFunctions["register"] = [](RouteArgs) {
        if (auto [error, userId] = ok::db::auth::registerFn(args, session.subDomain, session.mg_port);
            error.empty() && userId != -1)
        {
            session.memberKey = userId;
            ok::smart_actor::connection::addSuccess(resultMsg, event);
            ok::smart_actor::connection::addJwtCookie(
                resultMsg,
                {{"subDomain", session.subDomain},
                 {"memberKey", std::to_string(session.memberKey)},
                 {"createdAt",
                  std::to_string(utils::time::getEpochMilliseconds())}},
                60 * 60 * 24);
            ok::smart_actor::connection::addCurrentMember(resultMsg,
                                                          session.memberKey, session.mg_port);
        }
        else
        {
            session.memberKey = -1;
            ok::smart_actor::connection::addFailure(resultMsg, event, error);
        }
    };
    routeFunctions["login"] = [](RouteArgs) {
        if (auto [error, userId] = ok::db::auth::login(args, session.mg_port);
            error.empty() && userId != -1)
        {
            session.memberKey = userId;
            ok::smart_actor::connection::addSuccess(resultMsg, event);
            ok::smart_actor::connection::addJwtCookie(
                resultMsg,
                {{"subDomain", session.subDomain},{"memberKey", std::to_string(session.memberKey)},
                 {"createdAt",
                  std::to_string(utils::time::getEpochMilliseconds())}},
                60 * 60 * 24);
            ok::smart_actor::connection::addCurrentMember(resultMsg,
                                                          session.memberKey, session.mg_port);
        }
        else
        {
            session.memberKey = -1;
            if (error.empty())
            {
                error = "Email or Password is invalid";
            }
            ok::smart_actor::connection::addFailure(resultMsg, event, error);
        }
    };

    routeFunctions["loginJwt"] = [](RouteArgs) {
        if (!args.contains("jwt") || !args["jwt"].is_string())
        {
            session.memberKey = -1;
            ok::smart_actor::connection::addFailure(resultMsg,
                                                    event,
                                                    "Invalid Jwt Key");
            return;
        }
        auto jwtEncoded = args["jwt"].as_string();
        auto [memberKey, member] = ok::db::auth::loginJwt(jwtEncoded);

        session.memberKey = memberKey;
        if (memberKey != -1)
        {
            ok::smart_actor::connection::addEventAndJson(resultMsg,
                                                         event,
                                                         member);
        }
        else
        {
            ok::smart_actor::connection::addFailure(resultMsg,
                                                    event,
                                                    "Invalid Jwt");
        }
    };
    // wip
    routeFunctions["change_password"] = [](RouteArgs) {
        if (auto [error, successMsg] =
                ok::db::auth::change_password(session.memberKey, session.mg_port, args);
            error.empty())
            ok::smart_actor::connection::addSuccess(resultMsg, event);
        else
        {
            session.memberKey = -1;
            ok::smart_actor::connection::addFailure(resultMsg, event, error);
        }
    };
    routeFunctions["user"] = [](RouteArgs) {
        if (!session.memberKey)
        {
            ok::smart_actor::connection::addFailure(resultMsg,
                                                    event,
                                                    "Not Logged In");
            return;
        }
        if (auto [error, user] = ok::db::auth::user(session.memberKey, session.mg_port);
            error.empty())
        {
            ok::smart_actor::connection::addEventAndJson(
                resultMsg,
                event,
                jsoncons::ojson{jsoncons::json_object_arg,
                                {{"id", user["id"].as_string()}}});
        }
        else
        {
            session.memberKey = -1;
            ok::smart_actor::connection::addFailure(resultMsg, event, error);
        }
    };
    routeFunctions["logout"] = [](RouteArgs) {
        session.memberKey = -1;
        ok::smart_actor::connection::addSuccess(resultMsg, event);
        ok::smart_actor::connection::addJwtCookie(
            resultMsg, {{"subDomain", session.subDomain},{"memberKey", std::to_string(-1)}}, 60 * 60 * 24);
    };
    routeFunctions["confirm_email"] = [](RouteArgs) {

    };
    routeFunctions["confirm_email_status"] = [](RouteArgs) {

    };
    routeFunctions["is_logged_in"] = [](RouteArgs) {
        constexpr unsigned long milliseconds = 24 * 60 * 60 * 1000;
        if (!session.memberKey)
        {
            ok::smart_actor::connection::addSuccess(resultMsg, event);
        }
        else
        {
            ok::smart_actor::connection::addFailure(resultMsg,
                                                    event,
                                                    "not logged in");
        }
    };

    routeFunctions["lost_password"] = [](RouteArgs) {

    };
}
void addSyncRoutes()
{
    routeFunctions["sync"] = [](RouteArgs) {
        currentActor->send(currentActor->state.syncActor,
                           caf::subscribe_atom_v,
                           event,
                           args,
                           session,
                           currentActor);
    };
}
void addMutateRoutes()
{
    routeFunctions["mutate"] = [](RouteArgs) {
        currentActor->send(currentActor->state.mutationActor,
                           create_atom_v,
                           session,
                           args,
                           currentActor);
        jsoncons::ojson result;
        std::vector<int> txnIds;
        if (args.is_array())
            for (auto const &txn : args.array_range())
                if (txn.is_array())
                    txnIds.push_back(txn[0].as<int>());
        result["txnIds"] = txnIds;
        result["ack"] = true;
        ok::smart_actor::connection::addEventAndJson(resultMsg, event, result);
    };
}
void addHelperRoutes()
{
    routeFunctions["test_mail"] = [](RouteArgs) {};
    routeFunctions["read_log"] = [](RouteArgs) {
        auto content = pystring::read_all("./drogon.log");
        jsoncons::ojson j = content;
        ok::smart_actor::connection::addEventAndJson(resultMsg, event, j);
    };
    routeFunctions["subscribe_online_users"] = [](RouteArgs) {};
    routeFunctions["countries"] = [](RouteArgs) {};
    routeFunctions["languages"] = [](RouteArgs) {};
    routeFunctions["timezones"] = [](RouteArgs) {};
    routeFunctions["collections_size"] = [](RouteArgs) {};
    routeFunctions["get_uuid"] = [](RouteArgs) {
        auto uuid = drogon::utils::getUuid();
        jsoncons::ojson result;
        result["uuid"] = uuid;
        ok::smart_actor::connection::addEventAndJson(resultMsg, event, result);
    };
}
}  // namespace impl
}  // namespace connection
}  // namespace ok::smart_actor
