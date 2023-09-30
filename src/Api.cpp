#include "Api.hpp"
#include <string>
#include "db/Session.hpp"
#include "actor_system/CAF.hpp"
#include <filesystem>
#include "jwt/jwt.hpp"
#include "db/mgclientPool.hpp"
#include "pystring.hpp"
#include "utils/mg_helper.hpp"
#include "utils/time_functions.hpp"

#define RequestHandlerParams           \
    drogon::HttpRequestPtr const &req, \
        std::function<void(drogon::HttpResponsePtr const &)> &&callback
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

void registerApi()
{
    drogon::app().registerHandler("/api/upload",
                                  &file::upload,
                                  {drogon::Post, drogon::Options});
    drogon::app().registerHandler("/api/download/{1}/{2}/{3}",
                                  &file::download,
                                  {drogon::Get, drogon::Options});
    /*drogon::app().registerHandler("/api/chat/drogon",
                                  [](RequestHandlerParams)
                                  {
                                    auto resp1 =
  drogon::HttpResponse::newHttpResponse(); auto sid = req->getCookie("sid");
                                    auto urlPart =
  "_system/okweb/chat/auth/whoami"; auto [er, reason] = system_::foxxApi( req,
                                        std::move(urlPart),
                                        [callback](arangodb::rest::RestHandler
  *handler)
                                        {
                                          auto newResp =
  handler->stealResponse(); #ifdef ARANGODB_ENABLE_MAINTAINER_MODE
                                          arangodb::HttpResponse &response =
  dynamic_cast<arangodb::HttpResponse &>(*baseRes); #else arangodb::HttpResponse
  &response = static_cast<arangodb::HttpResponse &>(*newResp); #endif auto
  _response = response.stealBody(); auto resp =
  drogon::HttpResponse::newHttpResponse();
                                          resp->setStatusCode(static_cast<drogon::HttpStatusCode>(response.responseCode()));
                                          resp->setContentTypeCodeAndCustomString(drogon::ContentType::CT_NONE,
  response.responseString(response.responseCode()));  // HttpRequest.cpp 492 for
  (auto &i : response.headers()) resp->addHeader(i.first, i.second); for (auto
  &i : response.cookies()) resp->addHeader("Set-Cookie", i);
                                          resp->setBody(_response->toString());
                                          LOG_DEBUG << _response->toString(); //
  why I need to log_debug to get response! callback(resp);
                                        },
                                        callback);
                                    if (er) { system_::sendError(callback,
  reason); }
                                  },
                                  {drogon::Get});*/
    drogon::app().registerHandler(
        "/",
        [](drogon::HttpRequestPtr const &,
           std::function<void(drogon::HttpResponsePtr const &)> &&cb) {
            auto res = drogon::HttpResponse::newRedirectionResponse(
                "/web", drogon::k301MovedPermanently);
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
                                        [=](drogon::HttpRequestPtr const &req,
    std::function<void(drogon::HttpResponsePtr const &cb)> &&cb) {
    system_::foxxApiResponse(req, std::move(cb), std::string{url}); },
                                        {drogon::Get});
        }
        else if (type == "regex")
        {
          if (url.find("$1") != std::string::npos)
          {
            auto const &defaults = (it.isMember("default")) ? it["default"] :
    Json::Value(Json::ValueType::arrayValue); auto const &firstDefault =
    defaults.size() > 0 ? defaults[0].asString() : "";
            drogon::app().registerHandler(route,
                                          [=](RequestHandlerParams, std::string
    &&p1) { return system_::foxxApiResponse(req, std::move(callback),
    pystring::replace(url, "$1", (p1.empty() ? firstDefault : p1))); },
                                          {drogon::Post, drogon::Get,
    drogon::Patch, drogon::Put, drogon::Options, drogon::Head, drogon::Delete});
          }
          else
          {
            drogon::app().registerHandler(route,
                                          [=](RequestHandlerParams)
                                          { return system_::foxxApiResponse(req,
    std::move(callback), std::string{url}); });
          }
        }
      }
    }
    drogon::app().registerHandlerViaRegex("/(.*)",
    &api::system_::foxxApiResponse, {drogon::Post, drogon::Get, drogon::Patch,
    drogon::Put, drogon::Options, drogon::Head, drogon::Delete});
    */

    drogon::app().registerHandlerViaRegex(
        "/(.*)",
        [](drogon::HttpRequestPtr const &req,
           std::function<void(drogon::HttpResponsePtr const &)> &&callback,
           std::string &&urlPart) {
            Json::Value result;
            auto resp = drogon::HttpResponse::newFileResponse(
                "/home/k/admin/dist/index.html");
            callback(resp);
        },

        {drogon::Get});
}
namespace file
{
namespace impl
{
ok::smart_actor::connection::Session getSession(
    drogon::HttpRequestPtr const &req)
{
    ok::smart_actor::connection::Session session;
    // auto jwtEncodedCookie = req->getCookie("jwt");
    // auto subdomain = ok::utils::html::getSubdomain(req->getHeader("host"));
    // ok::db::authenticateAndSaveSession(jwtEncodedCookie, session, subdomain);
    return session;
}
bool initializeUser(ok::smart_actor::connection::Session &session) noexcept
{
    return session.memberKey;
}
bool isPermissionsOk() noexcept
{
    return true;
}

void sendSuccess(
    std::vector<VertexId> const &savedKeys,
    std::function<void(drogon::HttpResponsePtr const &)> &callback) noexcept
{
    jsoncons::ojson j;
    j["error"] = false;
    j["media"] = savedKeys;
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setBody(j.to_string());
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    callback(resp);
}
void sendFailure(
    ErrorMsg const &error,
    std::function<void(drogon::HttpResponsePtr const &)> &callback) noexcept
{
    Json::Value json;
    json["error"] = true;
    json["description"] = error;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
    callback(resp);
}
std::string getFileName(std::string const &key,
                        ok::smart_actor::connection::Session &session) noexcept
{
    ok::db::MGParams p{{"id", mg_value_make_integer(std::stoi(key))}};

    // TODO: can make generic function for this:
    std::string query{"MATCH (u) WHERE ID(u) = $id RETURN u;"};
    const auto [error, maybeResult] = mgCall(query, p);

    if (!error.empty())
    {
        return "";
    }
    if (ok::db::getIdFromResponse(*maybeResult) == -1)
    {
        return "";
    }
    if (maybeResult)
    {
        if (maybeResult.value().size() > 0)
        {
            auto j = convertNodeToJson(maybeResult.value()[0][0].ValueNode());
            LOG_DEBUG << j.to_string();
            return j["P"]["name"].as_string();
        }
        else
            return "";
    }

    return "";
}
std::string makePath(std::string const &fileName,
                     ok::smart_actor::connection::Session &session) noexcept
{
    return std::filesystem::current_path().string() + "/user_data/" + fileName;
}
bool saveThumbnails(std::string const &fileName) noexcept
{
    // if(req.file.mimetype.substr(0, 6) == 'image/')
    return true;
}
std::string fileTypeToString(drogon::FileType const &fileType)
{
    switch (fileType)
    {
        case drogon::FT_UNKNOWN:
            return "unknown";
        case drogon::FT_CUSTOM:
            return "custom";
        case drogon::FT_DOCUMENT:
            return "document";
        case drogon::FT_ARCHIVE:
            return "archive";
        case drogon::FT_AUDIO:
            return "audio";
        case drogon::FT_MEDIA:
            return "media";
        case drogon::FT_IMAGE:
            return "image";
    }
}
std::tuple<ErrorMsg, VertexId> saveFileToDatabase(
    drogon::HttpFile const &file,
    std::string const &fileName,
    ok::smart_actor::connection::Session &session) noexcept
{
    ok::db::MGParams p{{"md5", mg_value_make_string(file.getMd5().c_str())}};

    std::string query{"MATCH (u:Media {md5: $md5}) RETURN u;"};
    const auto [error, maybeResult] = mgCall(query, p);
    if (!error.empty())
    {
        return {error, {}};
    }
    if (ok::db::getIdFromResponse(*maybeResult) != -1)
    {
        return {"File already exist", -1};
    }

    ok::db::MGParams p2{
        {"name", mg_value_make_string(fileName.c_str())},
        {"type",
         mg_value_make_string(fileTypeToString(file.getFileType()).c_str())},
        {"md5", mg_value_make_string(file.getMd5().c_str())},
        {"createdAt",
         mg_value_make_integer(utils::time::getEpochMilliseconds())}};

    const auto [error2, maybeResult2] = mgCall(
        "CREATE (c:Media {name: $name, type: $type, md5: $md5, createdAt: "
        "$createdAt}) return c;",
        p2);
    if (!error2.empty())
    {
        return {error2, -1};
    }

    auto userId = ok::db::getIdFromResponse(*maybeResult2);
    if (userId == -1)
    {
        return {"cant save new file to database", -1};
    }
    else
        return {"", ok::db::getIdFromResponse(*maybeResult2)};
}
std::tuple<ErrorMsg, VertexId> saveTo(
    drogon::HttpFile const &file,
    const std::string_view &fileContent,
    ok::smart_actor::connection::Session &session) noexcept
{
    auto f2 = pystring::replace(file.getFileName(), " ", "_");
    auto fileName = trantor::Date::now().toCustomedFormattedString("%Y-%m-%d-%H%M%S") + "-" + f2;
    auto filePath = makePath(fileName, session);
    std::ofstream file_ofstream(filePath, std::ofstream::out);
    if (!file_ofstream)
    {
        LOG_DEBUG << "cant uploaded file:" << filePath
                  << " Error: " << strerror(errno) << " working directory"
                  << std::filesystem::current_path();
    }
    if (file_ofstream.is_open())
    {
        file_ofstream << fileContent;
        file_ofstream.close();
        auto [er, key] = saveFileToDatabase(file, fileName, session);
        saveThumbnails(fileName);
        LOG_DEBUG << "save uploaded file:" << fileName << " path: " << filePath;
        return {er, key};
    }
    LOG_ERROR << "save failed!";
    return {"ERROR_INTERNAL", -1};
}
std::tuple<ErrorMsg, std::vector<VertexId> > saveFiles(
    drogon::HttpRequestPtr const &req,
    ok::smart_actor::connection::Session &session) noexcept
{
    req->setContentTypeString(req->getHeader("content-type") + ";");
    if (req->method() != drogon::Post && req->method() != drogon::Put)
        return {"ERROR_FILE_NOT_FOUND", {}};

    const std::string &contentType = req->getHeader("content-type");
    if (contentType.empty())
    {
        return {"ERROR_FILE_NOT_FOUND", {}};
    }
    std::string::size_type pos = contentType.find(';');
    if (pos == std::string::npos)
        return {"ERROR_FILE_NOT_FOUND", {}};

    std::string type = contentType.substr(0, pos);
    std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c) {
        return tolower(c);
    });
    if (type != "multipart/form-data")
        return {"ERROR_FILE_NOT_FOUND", {}};
    pos = contentType.find("boundary=");
    if (pos == std::string::npos)
        return {"ERROR_FILE_NOT_FOUND", {}};
    auto pos2 = contentType.find(';', pos);
    if (pos2 == std::string::npos)
        pos2 = contentType.size();

    drogon::MultiPartParser fileUpload;
    if (fileUpload.parse(req) != -1)
    {
        // LOG_DEBUG << "upload good!";
        auto files = fileUpload.getFiles();
        // LOG_DEBUG << "file size=" << files.size();
        std::vector<VertexId> savedKeys;
        for (auto const &file : files)
        {
            if (auto [er, key] = saveTo(file, file.fileContent(), session);
                key > -1)
            {
                savedKeys.push_back(key);
            }
        }
        if (savedKeys.empty())
        {
            return {"ERROR_FILE_NOT_FOUND", {}};
        }
        return {"", savedKeys};
    }
    return {"ERROR_FILE_NOT_FOUND", {}};
}

}  // namespace impl
void upload(RequestHandlerParams)
{
    // https://github.com/drogonframework/drogon/blob/e2e5d6d57f26b79bab83e87d1d3596d077925603/lib/src/HttpControllersRouter.cc#L740
    if (req->method() == drogon::Options)
    {
        std::cout << "Setting Options" << std::endl;
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setContentTypeCode(drogon::ContentType::CT_TEXT_PLAIN);
        std::string methods = "OPTIONS,";
        methods.append("POST,");
        methods.resize(methods.length() - 1);
        resp->addHeader("ALLOW", methods);
//        auto &origin = req->getHeader("Origin");
//        if (origin.empty())
//        {
            resp->addHeader("Access-Control-Allow-Origin", "*");
//        }
//        else
//        {
//            resp->addHeader("Access-Control-Allow-Origin", origin);
//        }
        resp->addHeader("Access-Control-Allow-Methods", methods);
        callback(resp);
        return;
    }
    auto session = impl::getSession(req);

    // token based authentication :
    // https://github.com/noris666/Meteor-Files-POST-Example/blob/master/avatars.example.js#L37
    // disabling member check, anyone can upload.
    /*if (!impl::initializeUser(session))
    {
      impl::sendFailure(ok::bool::ERROR_HTTP_UNAUTHORIZED, callback);
      return;
    }*/
    if (auto [err, savedKeys] = impl::saveFiles(req, session); !err.empty())
    {
        impl::sendFailure(err, callback);
        return;
    }
    else
    {
        impl::sendSuccess(savedKeys, callback);
    }
}
void download(drogon::HttpRequestPtr const &req,
              std::function<void(drogon::HttpResponsePtr const &)> &&callback,
              int version,
              std::string const &key, std::string const &name)
{
    auto session = impl::getSession(req);
    // disabling member check, anyone can upload.
    /*if (!impl::initializeUser(session))
  {
    impl::sendFailure("ERROR_HTTP_UNAUTHORIZED", callback);
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
        impl::sendFailure("ERROR_HTTP_NOT_FOUND", callback);
        return;
    }
}
}  // namespace file
namespace member
{
void confirm(RequestHandlerParams,
             std::string const &key,
             std::string const &token)
{
}
}  // namespace member
namespace system_
{
void showServerHealthReport(RequestHandlerParams)
{
    jsoncons::ojson j = jsoncons::ojson::array();
    j.push_back(1);
    auto resp = drogon::HttpResponse::newCustomHttpResponse(std::move(j));
    assert(resp->jsonObject().get());
    callback(resp);
}
void gracefullyShutdown(RequestHandlerParams)
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
void sendError(std::function<void(drogon::HttpResponsePtr const &)> &callback,
               std::string message)
{
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setBody(message);
    callback(resp);
}
}  // namespace system_
}  // namespace ok::api
