#pragma once
#include <drogon/WebSocketController.h>
#include <jsoncons/json.hpp>
#include "CAF.hpp"
namespace ok::smart_actor
{
namespace supervisor
{
struct syncActorState
{
  static inline constexpr char const *name = "sync-actor";
  ws_connector_actor_int wsActor;
};
using SyncActorPointer = sync_actor_int::stateful_pointer<syncActorState>;
sync_actor_int::behavior_type SyncActor(SyncActorPointer self);
namespace impl
{
}  // namespace impl
}  // namespace supervisor
}  // namespace ok::smart_actor
