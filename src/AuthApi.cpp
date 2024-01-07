#include "Api.hpp"
#include <string>
#include <filesystem>
#include "db/auth_fns.hpp"
#include "utils/time_functions.hpp"
#define RequestHandlerParams           \
    drogon::HttpRequestPtr const &req, \
        std::function<void(drogon::HttpResponsePtr const &)> &&callback

namespace ok::api
{
void errorResponse(
    std::string msg,
    std::function<void(drogon::HttpResponsePtr const &)> &&callback)
{
    Json::Value ret;
    ret["message"] = "Not Logged In";
    ret["error"] = true;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
    callback(resp);
}
void successResponse(
    std::string msg,
    std::function<void(drogon::HttpResponsePtr const &)> &&callback)
{
    Json::Value ret;
    if (!msg.empty())
        ret["message"] = msg;
    ret["error"] = false;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
    callback(resp);
}

void sendLoginCookie(
    int userId,
    std::function<void(drogon::HttpResponsePtr const &)> &&callback)
{
    if (userId != 0)
    {
        Json::Value ret;
        ret["message"] = "Success";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
        std::map<std::string, std::string> jwtMap{
            {"memberKey", std::to_string(userId)},
            {"createdAt", std::to_string(utils::time::getEpochMilliseconds())},
        };
        resp->addCookie("jwt", ok::utils::jwt_functions::encodeCookie(jwtMap));
        callback(resp);
    }
    else
    {
        Json::Value ret;
        ret["message"] = "Email or Password Invalid";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
        resp->addCookie("jwt", {});
        callback(resp);
    }
}

void registerAuthApi()
{
    drogon::app().registerHandler(
        "/register",
        [](RequestHandlerParams) {
            auto subDomain = ok::utils::string::getLastThirdSegment(req->getHeader("host"));
            auto mgPort = ok::db::auth::getSubDomainMGPort(subDomain);
            if (subDomain.empty() || mgPort == -1) {
                return errorResponse("Invalid account", std::move(callback));
            }
            if (auto [error, userId] = ok::db::auth::registerFn(
                    jsoncons::ojson::parse(req->body()), subDomain, mgPort);
                error.empty())
                sendLoginCookie(userId, std::move(callback));
            else
                return errorResponse(error, std::move(callback));
        },
        {drogon::Post});
    drogon::app().registerHandler(
        "/login",
        [](RequestHandlerParams) {
            auto subDomain = ok::utils::string::getLastThirdSegment(req->getHeader("host"));
            auto mgPort = ok::db::auth::getSubDomainMGPort(subDomain);
            if (subDomain.empty() || mgPort == -1) {
                return errorResponse("Invalid account", std::move(callback));
            }
            if (auto [error, userId] =
                    ok::db::auth::login(jsoncons::ojson::parse(req->body()),
                                        mgPort);
                error.empty())
                sendLoginCookie(userId, std::move(callback));
            else
                return errorResponse(error, std::move(callback));
        },
        {drogon::Post});
    drogon::app().registerHandler(
        "/change_password",
        [](RequestHandlerParams) {
            auto [subDomain, memberKey] =
                db::auth::getMemberKeyFromJwt(req->getCookie("jwt"));
            auto mgPort = ok::db::auth::getSubDomainMGPort(subDomain);
            if (subDomain.empty() || mgPort == -1 || memberKey == -1)
                return errorResponse("Not Logged In", std::move(callback));

            if (auto [error, successMsg] = ok::db::auth::change_password(
                    memberKey, mgPort, jsoncons::ojson::parse(req->body()));
                error.empty())
                return successResponse(successMsg, std::move(callback));
            else
                return errorResponse(error, std::move(callback));
        },
        {drogon::Post});

    drogon::app().registerHandler(
        "/user",
        [](RequestHandlerParams) {
            auto jwtEncodedCookie = req->getCookie("jwt");
            if (jwtEncodedCookie.empty())
                return errorResponse("Not Logged In", std::move(callback));

            auto [subDomain, memberKey] = db::auth::getMemberKeyFromJwt(jwtEncodedCookie);
            auto mgPort = ok::db::auth::getSubDomainMGPort(subDomain);
            if (subDomain.empty() || mgPort == -1)
                return errorResponse("Invalid account", std::move(callback));

            if (auto [error, member] = ok::db::auth::user(memberKey, mgPort);
                error.empty())
                return successResponse(member["id"].as_string(),
                                       std::move(callback));
            else
                return errorResponse(error, std::move(callback));
        },
        {drogon::Get});

    drogon::app().registerHandler(
        "/logout",
        [](RequestHandlerParams) {
            Json::Value ret;
            ret["message"] = "Success";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
            resp->addCookie("jwt", {});
            callback(resp);
        },
        {drogon::Get});
}
}  // namespace ok::api
