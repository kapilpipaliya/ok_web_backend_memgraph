#pragma once
#include <drogon/WebSocketController.h>
#include <jsoncons/json.hpp>
#include "alias.hpp"
#include "CAF.hpp"
namespace ok::smart_actor
{
namespace supervisor
{
using SubDomain  = std::string;
struct mainActorState
{
  static inline constexpr char const *name = "main-actor";
  std::unordered_map<::drogon::WebSocketConnectionPtr, ws_connector_actor_int> wsPtrToWsActorMap;
  std::unordered_map<SubDomain, std::set<ws_connector_actor_int>> subDomainToWsActorListMap;
  std::unordered_map<SubDomain, sync_actor_int> subDomainToSyncActorMap;
//  inline ;

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
