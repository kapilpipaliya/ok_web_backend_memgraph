#include "Api.hpp"
#include <string>
#include "db/Session.hpp"
#include "utils/ErrorConstants.hpp"
#include "db/db_functions.hpp"
#include "actor_system/CAF.hpp"
#include "utils/GlobalSettings.hpp"
#include "db/DatabaseApi.hpp"
// #include "SimpleRequestHelper.hpp"
// #include "SimpleRequest.hpp"
// #include "Rest/HttpResponse.h"
// #include "GeneralServer/GeneralServerFeature.h"
// #include "RestServer/ServerGlobal.hpp"
#include <velocypack/Collection.h>
#include "utils/json_functions.hpp"
#include "utils/html_functions.hpp"
#include "json/json.h"
#include "pystring.hpp"
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
                                  if (ok::isEr(er)) { system_::sendError(callback, reason); }
                                },
                                {drogon::Get});*/
  drogon::app().registerHandler("/api/upload", &file::upload, {drogon::Post});
  drogon::app().registerHandler("/api/download/{1}/{2}", &file::download, {drogon::Get});
  drogon::app().registerHandler("/api/confirm/{1}/{2}", &member::confirm, {drogon::Get});
  drogon::app().registerHandler("/api/health", &system_::showServerHealthReport, {drogon::Get});
  drogon::app().registerHandler("/api/shutdown", &system_::gracefullyShutdown, {drogon::Get});
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
    impl::sendFailure(ok::ErrorCode::ERROR_HTTP_UNAUTHORIZED, callback);
    return;
  }*/
  if (auto [err, savedKeys] = impl::saveFiles(req, session); ok::isEr(err))
  {
    impl::sendFailure(err, callback);
    return;
  }
  else
  {
    impl::sendSuccess(savedKeys, callback);
  }
}
void download(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback, int version, std::string const &key)
{
  auto session = impl::getSession(req);
  // disabling member check, anyone can upload.
  /*if (!impl::initializeUser(session))
  {
    impl::sendFailure(ok::ErrorCode::ERROR_HTTP_UNAUTHORIZED, callback);
    return;
  }*/
  if (auto fileName = impl::getFileName(key, session); !fileName.empty())
  {
    auto filePath = impl::makePath(fileName, session);
    auto resp = drogon::HttpResponse::newFileResponse(filePath);
    resp->addHeader("Cache-Control", "max-age=2592000, public");
    // resp->setExpiredTime(0);
    callback(resp);
  }
  else
  {
    impl::sendFailure(ok::ErrorCode::ERROR_HTTP_NOT_FOUND, callback);
    return;
  }
}
namespace impl
{
ok::smart_actor::connection::Session getSession(drogon::HttpRequestPtr const &req)
{
  ok::smart_actor::connection::Session session;
  auto jwtEncodedCookie = req->getCookie("jwt");
  auto subdomain = ok::utils::html::getSubdomain(req->getHeader("host"));
  ok::db::authenticateAndSaveSession(jwtEncodedCookie, session, subdomain);
  return session;
}
bool initializeUser(ok::smart_actor::connection::Session &session) noexcept { return session.sessionKey.empty() ? false : true; }
bool isPermissionsOk() noexcept { return true; }
std::tuple<ErrorCode, std::vector<std::string> > saveFiles(drogon::HttpRequestPtr const &req, ok::smart_actor::connection::Session &session) noexcept
{
  drogon::MultiPartParser fileUpload;
  if (fileUpload.parse(req) == 0)
  {
    // LOG_DEBUG << "upload good!";
    auto files = fileUpload.getFiles();
    // LOG_DEBUG << "file size=" << files.size();
    std::vector<std::string> savedKeys;
    for (auto const &file : files)
    {
      if (auto [er, key] = saveTo(file, file.fileContent(), session); !key.empty()) { savedKeys.push_back(key); }
    }
    if (savedKeys.empty()) { return {ok::ErrorCode::ERROR_FILE_NOT_FOUND, {}}; }
    return {ok::ErrorCode::ERROR_NO_ERROR, savedKeys};
  }
  return {ok::ErrorCode::ERROR_FILE_NOT_FOUND, {}};
}
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
void sendFailure(ErrorCode const error, std::function<void(drogon::HttpResponsePtr const &)> &callback) noexcept
{
  Json::Value json;
  json["error"] = true;
  json["description"] = static_cast<int>(error);
  auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
  callback(resp);
}
std::string getFileName(std::string const &key, ok::smart_actor::connection::Session &session) noexcept
{
  auto query = "RETURN DOCUMENT('upload/" + key + "')";
  auto [erDb, response] = Api::Cursor::PostCursor::request(session.database, query, {});
  if (isEr(erDb))
  {
    if (response)
    {
      auto slice = response->slices().front();
      LOG_FATAL << "Error: " << slice.toJson();
    }
    LOG_FATAL << "error in query. session not found";
    return "";
  }
  auto slice = response->slices().front();
  if (auto result = slice.get("result"); result.length() == 0)
  {
    LOG_DEBUG << "No Document" << result.toString();
    return "";
  }
  else
  {
    return result[0].get("name").copyString();
  }
  return "";
}
std::string makePath(std::string const &fileName, ok::smart_actor::connection::Session &session) noexcept
{
  auto userDirectory = ok::smart_actor::connection::drogonRoot + "/user_data/" + session.database;
  return userDirectory + "/" + fileName;
}
bool saveThumbnails(std::string const &fileName) noexcept
{
  // if(req.file.mimetype.substr(0, 6) == 'image/')
  return true;
}
std::tuple<ok::ErrorCode, DocumentKey> saveTo(drogon::HttpFile const &file, const std::string_view &fileContent, ok::smart_actor::connection::Session &session) noexcept
{
  auto fileName = drogon::utils::getUuid() + "_" + file.getFileName();
  auto filePath = makePath(fileName, session);
  std::ofstream file_ofstream(filePath, std::ofstream::out);
  if (!file_ofstream) { LOG_DEBUG << "cant uploaded file:" << filePath << " Error: " << strerror(errno) << " working directory" << std::filesystem::current_path(); }
  if (file_ofstream.is_open())
  {
    file_ofstream << fileContent;
    file_ofstream.close();
    auto [er, key] = ok::db::saveFileToDatabase(file, fileName, session);
    saveThumbnails(fileName);
    LOG_DEBUG << "save uploaded file:" << fileName << " path: " << filePath;
    return {er, key};
  }
  LOG_ERROR << "save failed!";
  return {ok::ErrorCode::ERROR_INTERNAL, ""};
}
}  // namespace impl
}  // namespace file
namespace member
{
void confirm(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback, std::string const &key, std::string const &token)
{
  if (auto [er, dbName, memberKey] = ok::db::confirmMember(key, token); ok::isEr(er))
  {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setBody(ok::errno_string(er));
    callback(resp);
  }
  else
  {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setBody("success");
    callback(resp);
    // todo: send thank you email.
  }
}
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
/*std::function<void(arangodb::rest::RestHandler *)> foxxResponseHandler(std::function<void(drogon::HttpResponsePtr const &)> &callback)
{
  return [callback](arangodb::rest::RestHandler *handler)
  {
    auto newResp = handler->stealResponse();
#ifdef ARANGODB_ENABLE_MAINTAINER_MODE
    arangodb::HttpResponse &response = dynamic_cast<arangodb::HttpResponse &>(*baseRes);
#else
    arangodb::HttpResponse &response = static_cast<arangodb::HttpResponse &>(*newResp);
#endif
    auto _response = response.stealBody();
    auto resp = drogon::HttpResponse::newHttpResponse();
    // resp->setStatusCode(static_cast<drogon::HttpStatusCode>(response.responseCode()));
    // switch (response.contentType())
    // {
    //   case arangodb::rest::ContentType::CUSTOM:           // use Content-Type from _headers
    //     resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);  // CT_CUSTOM
    //     break;
    //   case arangodb::rest::ContentType::JSON:  // application/json
    //     resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    //     break;
    //   case arangodb::rest::ContentType::VPACK:  // application/x-velocypack
    //     resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
    //     break;
    //   case arangodb::rest::ContentType::TEXT:  // text/plain
    //     resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
    //     break;
    //   case arangodb::rest::ContentType::HTML:  // text/html
    //     resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    //     break;
    //   case arangodb::rest::ContentType::DUMP:  // application/x-arango-dump
    //     resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
    //     break;
    //   case arangodb::rest::ContentType::UNSET: resp->setContentTypeCode(drogon::CT_TEXT_PLAIN); break;
    // }
    resp->setContentTypeCodeAndCustomString(drogon::ContentType::CT_NONE, response.responseString(response.responseCode()));  // HttpRequest.cpp 492
    for (auto &i : response.headers()) resp->addHeader(i.first, i.second);
    for (auto &i : response.cookies()) resp->addHeader("Set-Cookie", i);
    resp->setBody(_response->toString());
    // LOG_INFO << "Sending Result";
    // LOG_TRACE<<"trace logs test3";
    callback(resp);
  };
};*/
/*std::pair<ErrorCode, ErrorMsg> foxxApi(drogon::HttpRequestPtr const &req,
                                       std::string &&urlPart,
                                       std::function<void(arangodb::rest::RestHandler *)> &&handler1,
                                       std::function<void(drogon::HttpResponsePtr const &)> &callback)
{
  try
  {
    // auto method = HttpMethodToString(req->method());
    // std::shared_ptr<arangodb::velocypack::Builder> builder;
    // ok::ErrorCode er = ok::ErrorCode::ERROR_NO_ERROR;
    // if (req->method() == drogon::HttpMethod::Get || req->method() == drogon::HttpMethod::Head) builder = std::make_shared<VPackBuilder>();
    // else
    //   std::tie(er, builder) = arangodb::velocypack::parseJson(std::string(req->getBody()));
    // if (ok::isEr(er)) { return {er, "Cant parse JSON"}; }
    // std::shared_ptr<VPackBuilder> payLoad;
    // payLoad = std::make_shared<VPackBuilder>(builder->slice());
    // auto simpleRequestHelper = std::make_unique<SimpleRequestHelper>();
    // // simpleRequestHelper->wsConnectionActor = self;
    // // set the endpoint
    // arangodb::ConnectionInfo info;
    // info.endpoint = "http+tcp://127.0.0.1:8529";
    // info.endpointType = arangodb::Endpoint::DomainType::IPV4;
    // info.encryptionType = arangodb::Endpoint::EncryptionType::NONE;
    // info.serverAddress = "127.0.0.1";
    // info.serverPort = 8529;
    // info.clientAddress = "127.0.0.1";
    // info.clientPort = 55566;
    // auto _request = std::make_unique<SimpleRestRequest>(info, callback, std::move(simpleRequestHelper), 1, "/_db/" + ok::utils::html::getSubdomain(req->getHeader("host")) + urlPart, method, payLoad);
    // _request->setHeader("rest", 4);
    // for (auto &h : req->headers()) { _request->setHeader(h.first.c_str(), h.first.length(), h.second.c_str(), h.second.length()); }
    // for (auto &c : req->cookies()) { _request->setCookie(const_cast<char *>(c.first.c_str()), c.first.length(), c.second.c_str()); }
    // // Step 3: Try to resolve vocbase and use
    // if (!ok::resolveRequestContext(*_request)) { return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, "CANT USE DATABASE"}; }
    // auto resp = std::make_unique<arangodb::HttpResponse>(arangodb::rest::ResponseCode::SERVER_ERROR, 1, nullptr);
    // resp->setContentType(_request->contentTypeResponse());
    // // ContentType stringToContentType(std::string const& val, ContentType def); // CommonDefines.cpp setting
    // for (auto &i : req->headers()) { _request->setHeaderV2(std::string{i.first}, std::string{i.second}); }
    // // create a handler and execute
    // // adding suffix and prefix...
    // auto &factory = arangodb::serverGlobal->getFeature<arangodb::GeneralServerFeature>().handlerFactory();
    // auto handler = factory.createHandler(*arangodb::serverGlobal, std::move(_request), std::move(resp));
    // // give up, if we cannot find a handler
    // if (handler == nullptr) { return {ok::ErrorCode::ERROR_FAILED, "handler not found"}; }
    // // queue the operation in the scheduler, ..
    // handler->runHandler(handler1);
    return {ok::ErrorCode::ERROR_NO_ERROR, ""};
  }
  catch (...)
  {
    LOG_FATAL << "Fatal Error";
    return {ok::ErrorCode::ERROR_INTERNAL, "Fatal Error"};
  }
}*/
/*void foxxApiResponse(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback, std::string &&urlPart)
{
  auto [er, reason] = foxxApi(req, std::move(urlPart), foxxResponseHandler(callback), callback);
  if (ok::isEr(er)) { sendError(callback, reason); }
}*/
}  // namespace system_
}  // namespace ok::api
