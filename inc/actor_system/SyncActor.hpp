#pragma once
#include <drogon/WebSocketController.h>
#include <jsoncons/json.hpp>
#include "CAF.hpp"
namespace ok::smart_actor
{
namespace supervisor
{
// https://marknelson.us/posts/2011/09/03/hash-functions-for-c-unordered-containers.html
struct hash_name {
    size_t operator()(const ws_connector_actor_int &name ) const
    {
        return name.address().id();
    }
};

struct syncActorState
{
  static inline constexpr char const *name = "sync-actor";
  std::unordered_map<ws_connector_actor_int, jsoncons::ojson, hash_name> wsActorArgs;
};
using SyncActorPointer = sync_actor_int::stateful_pointer<syncActorState>;
sync_actor_int::behavior_type SyncActor(SyncActorPointer self);
namespace impl
{
}  // namespace impl
}  // namespace supervisor
}  // namespace ok::smart_actor
