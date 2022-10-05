#pragma once
#include "WsConnectionActor.hpp"
namespace ok
{
enum class ErrorCode;
namespace smart_actor::connection
{
namespace impl
{
void addMyRoute();
void addAccountRoute();
void addSuperRoutes();
// clang-format off
void handleUserList(jsoncons::ojson const &valin, unsigned int eventNo, std::string const &userKey, Session const &session, jsoncons::ojson &resultMsg, ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor, bool isDispatching = false);
void handleUserMutate(jsoncons::ojson const &valin, unsigned int eventNo, std::string const &userKey, Session const &session, jsoncons::ojson &resultMsg, ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor, bool isDispatching = false);
// clang-format on
}  // namespace impl
}  // namespace smart_actor::connection
}  // namespace ok
