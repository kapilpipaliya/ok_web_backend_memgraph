#include "Api.hpp"
#include <string>
#include "db/Session.hpp"
#include "actor_system/CAF.hpp"
// #include "SimpleRequestHelper.hpp"
// #include "SimpleRequest.hpp"
// #include "Rest/HttpResponse.h"
// #include "GeneralServer/GeneralServerFeature.h"
// #include "RestServer/ServerGlobal.hpp"
#include <filesystem>
template <>
drogon::HttpResponsePtr drogon::toResponse(jsoncons::ojson &&pJson)
{
  auto resp = drogon::HttpResponse::newHttpResponse();
  resp->setBody(pJson.to_string());
  resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
  return resp;
}
namespace ok::api
{
using drogon::HttpRequestPtr;
using drogon::HttpResponsePtr;
std::string HttpMethodToString(drogon::HttpMethod method)
{
  switch (method)
  {
    case drogon::Get: return "get";
    case drogon::Post: return "post";
    case drogon::Head: return "head";
    case drogon::Put: return "put";
    case drogon::Delete: return "delete";
    case drogon::Options: return "options";
    case drogon::Patch: return "patch";
    case drogon::Invalid: return "invalid";
  }
}
void registerApi()
{
  /*drogon::app().registerHandler("/api/chat/drogon",
                                [](drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback)
                                {
                                  auto resp1 = drogon::HttpResponse::newHttpResponse();
                                  auto sid = req->getCookie("sid");
                                  auto urlPart = "_system/okweb/chat/auth/whoami";
                                  auto [er, reason] = system_::foxxApi(
                                      req,
                                      std::move(urlPart),
                                      [callback](arangodb::rest::RestHandler *handler)
                                      {
                                        auto newResp = handler->stealResponse();
#ifdef ARANGODB_ENABLE_MAINTAINER_MODE
                                        arangodb::HttpResponse &response = dynamic_cast<arangodb::HttpResponse &>(*baseRes);
#else
          arangodb::HttpResponse &response = static_cast<arangodb::HttpResponse &>(*newResp);
#endif
                                        auto _response = response.stealBody();
                                        auto resp = drogon::HttpResponse::newHttpResponse();
                                        resp->setStatusCode(static_cast<drogon::HttpStatusCode>(response.responseCode()));
                                        resp->setContentTypeCodeAndCustomString(drogon::ContentType::CT_NONE, response.responseString(response.responseCode()));  // HttpRequest.cpp 492
                                        for (auto &i : response.headers()) resp->addHeader(i.first, i.second);
                                        for (auto &i : response.cookies()) resp->addHeader("Set-Cookie", i);
                                        resp->setBody(_response->toString());
                                        LOG_DEBUG << _response->toString();  // why I need to log_debug to get response!
                                        callback(resp);
                                      },
                                      callback);
                                  if (er) { system_::sendError(callback, reason); }
                                },
                                {drogon::Get});*/
  drogon::app().registerHandler("/api/upload", &file::upload, {drogon::Post});
  drogon::app().registerHandler("/",
                                [](drogon::HttpRequestPtr const &, std::function<void(drogon::HttpResponsePtr const &)> &&cb)
                                {
                                  auto res = drogon::HttpResponse::newRedirectionResponse("/web", drogon::k301MovedPermanently);
                                  cb(res);
                                },
                                {drogon::Get});
}
void registerRegexApi()
{
  /*auto const &config = drogon::app().getCustomConfig();
  auto const &routes = config["routes"];
  for (auto const &key : routes.getMemberNames())
  {
    for (auto const &it : routes[key])
    {
      auto const &route = it["route"].asString();
      auto const &type = it["type"].asString();
      auto const &url = it["url"].asString();
      if (type == "static")
      {
        drogon::app().registerHandler(route,
                                      [=](drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &cb)> &&cb)
                                      { system_::foxxApiResponse(req, std::move(cb), std::string{url}); },
                                      {drogon::Get});
      }
      else if (type == "regex")
      {
        if (url.find("$1") != std::string::npos)
        {
          auto const &defaults = (it.isMember("default")) ? it["default"] : Json::Value(Json::ValueType::arrayValue);
          auto const &firstDefault = defaults.size() > 0 ? defaults[0].asString() : "";
          drogon::app().registerHandler(route,
                                        [=](drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback, std::string &&p1)
                                        { return system_::foxxApiResponse(req, std::move(callback), pystring::replace(url, "$1", (p1.empty() ? firstDefault : p1))); },
                                        {drogon::Post, drogon::Get, drogon::Patch, drogon::Put, drogon::Options, drogon::Head, drogon::Delete});
        }
        else
        {
          drogon::app().registerHandler(route,
                                        [=](drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback)
                                        { return system_::foxxApiResponse(req, std::move(callback), std::string{url}); });
        }
      }
    }
  }
  drogon::app().registerHandlerViaRegex("/(.*)", &api::system_::foxxApiResponse, {drogon::Post, drogon::Get, drogon::Patch, drogon::Put, drogon::Options, drogon::Head, drogon::Delete});
  */
}
namespace file
{
void upload(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback)
{
  auto session = impl::getSession(req);
  // token based authentication : https://github.com/noris666/Meteor-Files-POST-Example/blob/master/avatars.example.js#L37
  // disabling member check, anyone can upload.
  /*if (!impl::initializeUser(session))
  {
    impl::sendFailure(ok::bool::ERROR_HTTP_UNAUTHORIZED, callback);
    return;
  }*/
  // if (auto [err, savedKeys] = impl::saveFiles(req, session); ok::isEr(err))
  // {
  //   impl::sendFailure(err, callback);
  //   return;
  // }
  // else
  // {
  //   impl::sendSuccess(savedKeys, callback);
  // }
}
namespace impl
{
ok::smart_actor::connection::Session getSession(drogon::HttpRequestPtr const &req)
{
  ok::smart_actor::connection::Session session;
  // auto jwtEncodedCookie = req->getCookie("jwt");
  // auto subdomain = ok::utils::html::getSubdomain(req->getHeader("host"));
  // ok::db::authenticateAndSaveSession(jwtEncodedCookie, session, subdomain);
  return session;
}
bool initializeUser(ok::smart_actor::connection::Session &session) noexcept { return session.memberKey.empty() ? false : true; }
bool isPermissionsOk() noexcept { return true; }
void sendSuccess(std::vector<std::string> const &savedKeys, std::function<void(drogon::HttpResponsePtr const &)> &callback) noexcept
{
  jsoncons::ojson j;
  j["error"] = false;
  j["media"] = savedKeys;
  auto resp = drogon::HttpResponse::newHttpResponse();
  resp->setBody(j.to_string());
  resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
  callback(resp);
}
void sendFailure(bool const error, std::function<void(drogon::HttpResponsePtr const &)> &callback) noexcept
{
  Json::Value json;
  json["error"] = true;
  json["description"] = static_cast<int>(error);
  auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
  callback(resp);
}
}  // namespace impl
}  // namespace file
namespace member
{
void confirm(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback, std::string const &key, std::string const &token) {}
}  // namespace member
namespace system_
{
void showServerHealthReport(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback)
{
  jsoncons::ojson j = jsoncons::ojson::array();
  j.push_back(1);
  auto resp = drogon::HttpResponse::newCustomHttpResponse(std::move(j));
  assert(resp->jsonObject().get());
  callback(resp);
}
void gracefullyShutdown(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback)
{
  LOG_DEBUG << "shutting down";
  drogon::app().quit();
  // caf::scoped_actor self{*ok::smart_actor::supervisor::actorSystem};
  // self->send(ok::smart_actor::supervisor::mainActor, conn_exit_atom_v);
  caf::anon_send(ok::smart_actor::supervisor::mainActor, shutdown_atom_v);
  auto resp = drogon::HttpResponse::newHttpResponse();
  resp->setBody("Success");
  callback(resp);
}
void sendError(std::function<void(drogon::HttpResponsePtr const &)> &callback, std::string message)
{
  auto resp = drogon::HttpResponse::newHttpResponse();
  resp->setBody(message);
  callback(resp);
}
}  // namespace system_
}  // namespace ok::api
