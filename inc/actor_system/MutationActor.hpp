#pragma once
#include <drogon/WebSocketController.h>
#include <jsoncons/json.hpp>
#include "CAF.hpp"
namespace ok::smart_actor
{
namespace supervisor
{
struct mutationState
{
    static inline constexpr char const *name = "sync-actor";
    ws_connector_actor_int wsActor;
};
using MutationActorPointer = mutation_actor_int::stateful_pointer<mutationState>;
mutation_actor_int::behavior_type MutationActor(MutationActorPointer self);
namespace impl
{
}  // namespace impl
}  // namespace supervisor
}  // namespace ok::smart_actor
