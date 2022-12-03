#pragma once
#include <drogon/HttpController.h>
#include "alias.hpp"
#include <drogon/WebSocketController.h>
#include "actor_system/CAF.hpp"
#include "utils/html_functions.hpp"
#include "utils/BatchArrayMessage.hpp"
#include "actor_system/WsConnectionActor.hpp"
// #include "Actions/RestActionHandler.h"
namespace ok
{
enum class ErrorCode;
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
inline void sendMessageToMainActorOld(drogon::WebSocketConnectionPtr const &wsConnPtr, std::string &&message, drogon::WebSocketMessageType const &type)
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
    default: LOG_DEBUG << "Not handled message:" << static_cast<int>(type); break;
  }
}
inline void sendExitToMainActorOld(drogon::WebSocketConnectionPtr const &wsConnPtr)
{
  // caf::scoped_actor self{*ok::smart_actor::supervisor::actorSystem};
  caf::anon_send(ok::smart_actor::supervisor::mainActor, conn_exit_old_atom_v, wsConnPtr);
}
}  // namespace ws
class Ws : public drogon::WebSocketController<Ws>
{
public:
  inline void handleNewConnection(drogon::HttpRequestPtr const &req, drogon::WebSocketConnectionPtr const &wsConnPtr) override
  {
    LOG_DEBUG << req->getHeader("host") << " " << req->getLocalAddr().toIp();
    ws::setPingReplyMessage(wsConnPtr);
    caf::anon_send(ok::smart_actor::supervisor::mainActor, save_old_wsconnptr_atom_v, wsConnPtr, req->getCookie("jwt"), ok::utils::html::getSubdomain(req->getHeader("host")));
  }
  inline void handleNewMessage(drogon::WebSocketConnectionPtr const &wsConnPtr, std::string &&message, drogon::WebSocketMessageType const &type) override
  {
    ws::sendMessageToMainActorOld(wsConnPtr, std::move(message), type);
  }
  inline void handleConnectionClosed(drogon::WebSocketConnectionPtr const &wsConnPtr) override { ws::sendExitToMainActorOld(wsConnPtr); }
  WS_PATH_LIST_BEGIN
  WS_PATH_ADD("/ws");
  WS_PATH_LIST_END
};

namespace file
{
// todo check permission
void upload(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback);
// todo check permissions
namespace impl
{
ok::smart_actor::connection::Session getSession(drogon::HttpRequestPtr const &req);
bool initializeUser(ok::smart_actor::connection::Session &session) noexcept;
bool isPermissionsOk() noexcept;
// Fix this
void sendSuccess(std::vector<std::string> const &savedKeys, std::function<void(drogon::HttpResponsePtr const &)> &callback) noexcept;
void sendFailure(ErrorCode const error, std::function<void(drogon::HttpResponsePtr const &)> &callback) noexcept;
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
/*std::pair<ErrorCode, ErrorMsg> foxxApi(drogon::HttpRequestPtr const &req,
                                       std::string &&urlPart,
                                       std::function<void(arangodb::rest::RestHandler *)> &&handler1,
                                       std::function<void(drogon::HttpResponsePtr const &)> &callback);
void foxxApiResponse(drogon::HttpRequestPtr const &req, std::function<void(drogon::HttpResponsePtr const &)> &&callback, std::string &&urlPart);*/
}  // namespace system_
namespace Webhook
{
}
}  // namespace api
}  // namespace ok
