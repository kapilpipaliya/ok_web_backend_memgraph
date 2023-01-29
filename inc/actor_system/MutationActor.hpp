#pragma once
#include <drogon/WebSocketController.h>
#include <jsoncons/json.hpp>
#include "CAF.hpp"
#include "mgclient.hpp"
namespace ok::smart_actor
{
namespace supervisor
{
struct mutationState
{
    static inline constexpr char const *name = "sync-actor";
    ws_connector_actor_int wsActor;
    using DbConnectionPtr = std::shared_ptr<mg::Client>;
    DbConnectionPtr connPtr;
    mg::Client::Params params;
    mutationState(){
        params.host = "localhost";
        params.port = global_var::mg_port;
        params.use_ssl = false;
    }
};
using MutationActorPointer = mutation_actor_int::stateful_pointer<mutationState>;
mutation_actor_int::behavior_type MutationActor(MutationActorPointer self);
namespace impl
{
}  // namespace impl
}  // namespace supervisor
}  // namespace ok::smart_actor
