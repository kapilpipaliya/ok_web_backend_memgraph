#pragma once
#include "CAF.hpp"
#include "WsConnectionV8Actor.hpp"
struct SimpleRequestHelper {
  ws_connector_v8_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_v8_state> wsConnectionActor;
};
