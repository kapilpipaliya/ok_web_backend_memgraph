#pragma once
#include <drogon/WebSocketController.h>
#include <jsoncons/json.hpp>
#include "CAF.hpp"
#include "mgclient.hpp"

namespace ok::smart_actor::supervisor
{
struct mutationState
{
    static inline constexpr char const *name = "mutation-actor";
    ws_connector_actor_int wsActor;
    std::unique_ptr<mg::Client> mgClient;
    mg::Client::Params params;
    mutationState(){
        params.host = "localhost";
        params.port = 0;
        params.use_ssl = false;
    }
};
using MutationActorPointer = mutation_actor_int::stateful_pointer<mutationState>;
mutation_actor_int::behavior_type MutationActor(MutationActorPointer self);
namespace impl
{
}  // namespace impl
} // namespace ok::smart_actor::supervisor

