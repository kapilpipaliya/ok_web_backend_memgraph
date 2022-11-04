#pragma once
#include <drogon/WebSocketController.h>
#include <jsoncons/json.hpp>
#include "alias.hpp"
#include "CAF.hpp"
namespace ok::smart_actor
{
namespace supervisor
{
struct mainActorState
{
  static inline constexpr char const *name = "main-actor";
  std::unordered_map<::drogon::WebSocketConnectionPtr, ws_connector_actor_int> oldActorMap;
  // std::unordered_map<::drogon::WebSocketConnectionPtr, ws_connector_v8_actor_int> v8ActorMap;
};
using MainActorPointer = main_actor_int::stateful_pointer<mainActorState>;
main_actor_int::behavior_type MainActor(MainActorPointer self);
namespace impl
{
void spawnAndMonitorSuperActor(MainActorPointer act) noexcept;
void spanAndSaveConnectionActor(MainActorPointer act, drogon::WebSocketConnectionPtr const &wsConnPtr, std::string const &jwtEncoded, std::string const &firstSubDomain) noexcept;
void passToWsControllerActor(MainActorPointer act, drogon::WebSocketConnectionPtr const &wsConnPtr, std::string &&message, drogon::WebSocketMessageType const &type) noexcept;
void connectionExit(MainActorPointer act, drogon::WebSocketConnectionPtr const &wsConnPtr);
void shutdownNow(MainActorPointer act) noexcept;
}  // namespace impl
}  // namespace supervisor
}  // namespace ok::smart_actor
