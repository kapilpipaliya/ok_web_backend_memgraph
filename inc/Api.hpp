#pragma once
#include <drogon/HttpController.h>
#include <drogon/WebSocketController.h>
#include "actor_system/CAF.hpp"
#include "utils/html_functions.hpp"
#include "lib/string_functions.hpp"
namespace ok
{
namespace smart_actor::connection
{
struct Session;
}
namespace api
{
void registerApi();
void registerAuthApi();
void registerRegexApi();
namespace ws
{
inline void setPingReplyMessage(drogon::WebSocketConnectionPtr const &wsConnPtr) { wsConnPtr->setPingMessage("Pong", std::chrono::duration<long double>(15000)); }
inline void sendMessageToMainActor(drogon::WebSocketConnectionPtr const &wsConnPtr, std::string &&message, drogon::WebSocketMessageType const &type)
{
  switch (type)
  {
    case drogon::WebSocketMessageType::Text:
    case drogon::WebSocketMessageType::Binary:
    {
      caf::anon_send(ok::smart_actor::supervisor::mainActor, pass_to_ws_connection_atom_v, wsConnPtr, std::move(message), type);
      break;
    }
    case drogon::WebSocketMessageType::Close: break;
    case drogon::WebSocketMessageType::Ping:
    default: LOG_ERROR << "Not handled message:" << static_cast<int>(type); break;
  }
}
inline void sendExitToMainActor(drogon::WebSocketConnectionPtr const &wsConnPtr)
{
  caf::anon_send(ok::smart_actor::supervisor::mainActor, conn_exit_atom_v, wsConnPtr);
}
}  // namespace ws
class Ws : public drogon::WebSocketController<Ws>
{
public:
  inline void handleNewConnection(drogon::HttpRequestPtr const &req, drogon::WebSocketConnectionPtr const &wsConnPtr) override
  {
//    LOG_DEBUG << req->getHeader("host") << " " << req->getLocalAddr().toIp();
    ws::setPingReplyMessage(wsConnPtr);
    caf::anon_send(ok::smart_actor::supervisor::mainActor, save_wsconnptr_atom_v, wsConnPtr, req->getCookie("jwt"), ok::utils::string::getLastThirdSegment(req->getHeader("host")));
  }
  inline void handleNewMessage(drogon::WebSocketConnectionPtr const &wsConnPtr, std::string &&message, drogon::WebSocketMessageType const &type) override
  {
    ws::sendMessageToMainActor(wsConnPtr, std::move(message), type);
  }
  inline void handleConnectionClosed(drogon::WebSocketConnectionPtr const &wsConnPtr) override {
    ws::sendExitToMainActor(wsConnPtr); }
  WS_PATH_LIST_BEGIN
  WS_PATH_ADD("/ws");
  WS_PATH_LIST_END
};

namespace file
{
// todo check permission
void upload(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback);
// todo check permissions
void download(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback, int version, std::string const &key, std::string const &name);
namespace impl
{
ok::smart_actor::connection::Session getSession(drogon::HttpRequestPtr const &req);
bool initializeUser(ok::smart_actor::connection::Session &session) noexcept;
bool isPermissionsOk() noexcept;
// Fix this
void sendSuccess(const jsoncons::ojson &savedKeys, std::function<void(drogon::HttpResponsePtr const &)> &callback) noexcept;
void sendFailure( ErrorMsg const &error, std::function<void(drogon::HttpResponsePtr const &)> &callback) noexcept;
}  // namespace impl
}  // namespace file
namespace member
{
void confirm(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback, std::string const &key, std::string const &token);
};  // namespace member
namespace system_
{
void sendError(std::function<void(drogon::HttpResponsePtr const &)> &callback, std::string message);
void showServerHealthReport(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback);
void gracefullyShutdown(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback);
}  // namespace system_
namespace Webhook
{
}
}  // namespace api
}  // namespace ok
