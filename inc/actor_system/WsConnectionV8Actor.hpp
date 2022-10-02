#pragma once
#include "caf/all.hpp"
#include <jsoncons/json.hpp>
#include <drogon/WebSocketController.h>
#include "CAF.hpp"
#include "Endpoint/ConnectionInfo.h"
namespace ok
{
enum class ErrorCode;
namespace smart_actor::connection
{
struct ws_controller_v8_state
{
  static inline constexpr char const *name = "connection-v8-actor";
  drogon::WebSocketConnectionPtr wsConnPtr = nullptr;
  arangodb::ConnectionInfo info;
};
ws_connector_v8_actor_int::behavior_type WsConnectionV8Actor(ws_connector_v8_actor_int::stateful_pointer<ws_controller_v8_state> self,
                                                             drogon::WebSocketConnectionPtr wsConnPtr,
                                                             arangodb::ConnectionInfo info);
void saveNewConnection(ws_connector_v8_actor_int::stateful_pointer<ws_controller_v8_state> self, ws_controller_v8_state &state, arangodb::ConnectionInfo const &info);
namespace impl
{
}  // namespace impl
}  // namespace smart_actor::connection
}  // namespace ok
