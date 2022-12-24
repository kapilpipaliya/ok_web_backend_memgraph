#pragma once
#include "caf/all.hpp"
#include "actor_system/SyncActor.hpp"
#include <jsoncons/json.hpp>
#include <drogon/WebSocketController.h>
#include "db/Session.hpp"
#include "actor_system/CAF.hpp"
namespace ok
{
namespace smart_actor::connection
{
struct ws_controller_state
{
  static inline constexpr char const *name = "connection-actor";
  drogon::WebSocketConnectionPtr wsConnPtr = nullptr;
  std::string subDomain;
  ok::smart_actor::connection::Session session;
  mutation_actor_int mutationActor;
};
inline std::map<std::string,
                std::function<void(jsoncons::ojson const &event,
                                   jsoncons::ojson const &args,
                                   Session &session,
                                   jsoncons::ojson &resultMsg,
                                   ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor,
                                   std::string const &subDomain)>>
    routeFunctions;
ws_connector_actor_int::behavior_type WsControllerActor(ws_connector_actor_int::stateful_pointer<ws_controller_state> self);
void sendJson(drogon::WebSocketConnectionPtr wsConnPtr, const jsoncons::ojson &json) noexcept;
void saveNewConnection(ws_connector_actor_int::stateful_pointer<ws_controller_state> self, ws_controller_state &state, std::string const &jwtEncoded, std::string const &firstSubDomain);
// clang-format off
std::tuple<bool, jsoncons::ojson> processEvent(jsoncons::ojson const &valin, Session &session, std::string const &subDomain, ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor);
namespace impl
{
std::tuple<bool, jsoncons::ojson> parseJson(std::string message);
bool checkSchema(jsoncons::ojson const &valin);
// clang-format on
}  // namespace impl
}  // namespace smart_actor::connection
}  // namespace ok
