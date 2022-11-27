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
#include "db/mgclientPool.hpp"
#include "jwt/jwt.hpp"
#include "third_party/mgclient/src/mgvalue.h"
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
bool findByEmail(std::string const &email) {
    mg_map *extra = mg_map_make_empty(1);
    if (!extra)
      return false;
    mg_map_insert_unsafe(extra, "email", mg_value_make_string(email.c_str()));
    auto response = ok::db::memgraph_conns.requestDataRaw("MATCH (u:User {email: $email}) RETURN u", mg::ConstMap{extra});
    return response.size() > 0;
}
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
    drogon::app().registerHandler("/api/upload", &file::upload, {drogon::Post});
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
    // `CREATE CONSTRAINT ON (u:User) ASSERT u.email IS UNIQUE`)
    drogon::app().registerHandler("/register", [](drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback){
        auto o = jsoncons::ojson::parse(req->body());
        auto email = o["body"]["email"].as_string();
        auto password = o["body"]["password"].as_string();
        if(findByEmail(email)) {
            Json::Value ret;
            ret["message"] = "Email Adress already exist";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
            callback(resp);
        } else {
            mg_map *extra = mg_map_make_empty(2);
            if (!extra) {
              return nullptr;
            }
            mg_map_insert_unsafe(extra, "email", mg_value_make_string(email.c_str()));
            mg_map_insert_unsafe(extra, "password", mg_value_make_string(password.c_str()));
            mg::ConstMap params{extra};
            auto response = ok::db::memgraph_conns.requestDataRaw("CREATE (c:User {email: $email, password: $password}) return c;", params);
            mg_map_destroy(extra);
             /*jsoncons::json_options options;
             options.object_array_line_splits(jsoncons::line_split_kind::new_line);
    //         options.
             // taken encoder example from o.to_string();
             using string_type = std::basic_string<jsoncons::ojson::char_type>;
             string_type s;
             jsoncons::basic_compact_json_encoder<jsoncons::ojson::char_type, jsoncons::string_sink<string_type>> encoder(s);
             o.dump(s, options);
             std::cout << s << std::endl;*/
            int userId = 0;
            for (auto& row : response)
            {
              for (auto& matchPart : row)
              {
                  if(matchPart.type() == mg::Value::Type::Node) {
                      userId = matchPart.ValueNode().id().AsInt();
                  }
              }
            }
            Json::Value ret;
            ret["message"] = "Hello !" + email + password + std::to_string(userId);
            auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
            std::map<std::string, std::string> jwtMap{{"memberKey", std::to_string(userId)}};
            resp->addCookie("jwt", ok::utils::jwt_functions::encodeCookie(jwtMap));
            callback(resp);
        }
    }, {drogon::Post});
    drogon::app().registerHandler("/login", [](drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback){
        auto o = jsoncons::ojson::parse(req->body());
        auto email = o["body"]["email"].as_string();
        auto password = o["body"]["password"].as_string();

        mg_map *extra = mg_map_make_empty(2);
        if (!extra) {
            LOG_DEBUG << "unexpected error";
        }
        mg_map_insert_unsafe(extra, "email", mg_value_make_string(email.c_str()));
        mg_map_insert_unsafe(extra, "password", mg_value_make_string(password.c_str()));
        mg::ConstMap params{extra};
        auto response = ok::db::memgraph_conns.requestDataRaw("MATCH (u:User {email: $email, password: $password}) return u;", params);
        mg_map_destroy(extra);
        int userId = 0;
        for (auto& row : response)
        {
          for (auto& matchPart : row)
          {
              if(matchPart.type() == mg::Value::Type::Node) {
                  userId = matchPart.ValueNode().id().AsInt();
              }
          }
        }
        if(userId != 0) {
            Json::Value ret;
            ret["message"] = "Successful Login";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
            std::map<std::string, std::string> jwtMap{{"memberKey", std::to_string(userId)}};
            resp->addCookie("jwt", ok::utils::jwt_functions::encodeCookie(jwtMap));
            callback(resp);
        } else {
            Json::Value ret;
            ret["message"] = "Email or Password Invalid";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
            resp->addCookie("jwt", {});
            callback(resp);
        }
    }, {drogon::Post});

    drogon::app().registerHandler("/user", [](drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback){

        auto jwtEncodedCookie = req->getCookie("jwt");

        if (jwtEncodedCookie.empty())
        {
            Json::Value ret;
            ret["message"] = "Not Logged In";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
            callback(resp);
        } else {
            std::string memberKey;
              auto dec_obj = ok::utils::jwt_functions::decodeCookie(jwtEncodedCookie);
              if (dec_obj.payload().has_claim("memberKey")) { memberKey = dec_obj.payload().get_claim_value<std::string>("memberKey"); }
              if (memberKey.empty())
               {
                  Json::Value ret;
                  ret["message"] = "Not Logged In";
                  auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
                  callback(resp);
              } else {
                  // Todo find if the user exists in database
                  Json::Value ret;
                  ret["message"] = memberKey;
                  auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
                  callback(resp);
              }

        }

    }, {drogon::Get});

    drogon::app().registerHandler("/logout", [](drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback){
        Json::Value ret;
        ret["message"] = "Success";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
        resp->addCookie("jwt", {});
        callback(resp);

    }, {drogon::Get});
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
