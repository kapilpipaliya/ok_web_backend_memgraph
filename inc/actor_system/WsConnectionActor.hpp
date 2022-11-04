#pragma once
#include "caf/all.hpp"
#include "actor_system/SyncActor.hpp"
#include <jsoncons/json.hpp>
#include <drogon/WebSocketController.h>
#include "db/Session.hpp"
#include "actor_system/CAF.hpp"
namespace ok
{
enum class ErrorCode;
namespace smart_actor::connection
{
struct ws_controller_state
{
  static inline constexpr char const *name = "connection-actor";
  drogon::WebSocketConnectionPtr wsConnPtr = nullptr;
  std::string subDomain;
  ok::smart_actor::connection::Session session;
  sync_actor_int syncActor;
};
inline std::map<std::string,
                std::function<void(jsoncons::ojson const &valin,
                                   unsigned int eventNo,
                                   std::string const &routeNo,
                                   Session const &session,
                                   jsoncons::ojson &resultMsg,
                                   ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor,
                                   std::string const &subDomain)>>
    routeFunctions;
ws_connector_actor_int::behavior_type WsControllerActor(ws_connector_actor_int::stateful_pointer<ws_controller_state> self);
void sendJson(drogon::WebSocketConnectionPtr wsConnPtr, const jsoncons::ojson &json) noexcept;
void saveNewConnection(ws_connector_actor_int::stateful_pointer<ws_controller_state> self, ws_controller_state &state, std::string const &jwtEncoded, std::string const &firstSubDomain);
// clang-format off
std::tuple<ErrorCode, jsoncons::ojson> processEvent(jsoncons::ojson const &valin, Session const &session, std::string const &subDomain, ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor);
std::tuple<ErrorCode, jsoncons::ojson> processEvent(std::string message, Session const &session, std::string const &subDomain, ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor);
namespace impl
{
std::tuple<ErrorCode, jsoncons::ojson> preparing(std::string message, bool isDispatching = false);
ErrorCode checkSchema(jsoncons::ojson const &valin);
std::string start(jsoncons::ojson const &valin, unsigned int eventNo, bool isDispatching = false);
// clang-format on
}  // namespace impl
}  // namespace smart_actor::connection
}  // namespace ok
