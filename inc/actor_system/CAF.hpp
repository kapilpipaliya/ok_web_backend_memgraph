#pragma once
#include "caf/all.hpp"
#include <drogon/WebSocketController.h>
#include <jsoncons/json.hpp>
#include "db/Session.hpp"

// #include "Endpoint/ConnectionInfo.h"
//#include "caf/io/middleman.hpp"
//#include "tsl/ordered_map.h"

CAF_BEGIN_TYPE_ID_BLOCK(okproject, first_custom_type_id)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(drogon::WebSocketConnectionPtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(drogon::WebSocketMessageType)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(jsoncons::ojson)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(ok::smart_actor::connection::Session)

CAF_ADD_ATOM(okproject, conn_exit_old_atom);
CAF_ADD_ATOM(okproject, conn_exit_atom);
CAF_ADD_ATOM(okproject, shutdown_atom);
CAF_ADD_ATOM(okproject, logout_atom);
CAF_ADD_ATOM(okproject, send_email_atom);
CAF_ADD_ATOM(okproject, spawn_and_monitor_atom);
CAF_ADD_ATOM(okproject, save_old_wsconnptr_atom);
CAF_ADD_ATOM(okproject, pass_to_ws_connection_atom);
CAF_ADD_ATOM(okproject, session_clean_atom);
CAF_ADD_ATOM(okproject, set_context_atom);
CAF_ADD_ATOM(okproject, dispatch_atom);
CAF_ADD_ATOM(okproject, get_session_atom);
CAF_ADD_ATOM(okproject, create_atom);
CAF_ADD_ATOM(okproject, set_atom);
CAF_ADD_ATOM(okproject, remove_atom);
CAF_ADD_TYPE_ID(okproject, (drogon::WebSocketConnectionPtr))
CAF_ADD_TYPE_ID(okproject, (drogon::WebSocketMessageType))
CAF_ADD_TYPE_ID(okproject, (jsoncons::ojson))
CAF_ADD_TYPE_ID(okproject, (ok::smart_actor::connection::Session))
CAF_ADD_TYPE_ID(okproject, (std::vector<std::string>))
CAF_ADD_TYPE_ID(okproject, (std::vector<VertexId>))
CAF_ADD_TYPE_ID(okproject, (std::unordered_set<std::string>))

using ws_connector_actor_int = caf::typed_actor<caf::reacts_to<drogon::WebSocketConnectionPtr, std::string, std::string>,
                                                caf::replies_to<get_session_atom>::with<ok::smart_actor::connection::Session>,
                                                caf::reacts_to<set_context_atom, ok::smart_actor::connection::Session>,
                                                caf::reacts_to<std::string, drogon::WebSocketMessageType>,
                                                caf::reacts_to<caf::forward_atom, jsoncons::ojson>,
                                                caf::reacts_to<dispatch_atom, jsoncons::ojson>,
                                                caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (ws_connector_actor_int))

using main_actor_int = caf::typed_actor<caf::reacts_to<spawn_and_monitor_atom>,
                                        caf::reacts_to<save_old_wsconnptr_atom, drogon::WebSocketConnectionPtr, std::string, std::string>,
                                        caf::reacts_to<pass_to_ws_connection_atom, drogon::WebSocketConnectionPtr, std::string, drogon::WebSocketMessageType>,
                                        caf::reacts_to<conn_exit_old_atom, drogon::WebSocketConnectionPtr>,
                                        caf::reacts_to<shutdown_atom>>;
CAF_ADD_TYPE_ID(okproject, (main_actor_int))

using sync_actor_int = caf::typed_actor<
    caf::reacts_to<caf::subscribe_atom, WsEvent, WsArguments, VertexId, ws_connector_actor_int>,
    caf::reacts_to<create_atom, std::vector<VertexId>, std::vector<EdgeId>>,
                                        caf::reacts_to<set_atom, std::vector<VertexId>, std::vector<EdgeId>>,
                                        caf::reacts_to<remove_atom, std::vector<VertexId>, std::vector<EdgeId>>,
                                        caf::reacts_to<conn_exit_atom, ws_connector_actor_int>,
                                        caf::reacts_to<shutdown_atom>>;
CAF_ADD_TYPE_ID(okproject, (sync_actor_int))

using mutation_actor_int = caf::typed_actor<
    caf::reacts_to<create_atom, VertexId, WsArguments, ws_connector_actor_int>,
    caf::reacts_to<shutdown_atom>>;
CAF_ADD_TYPE_ID(okproject, (mutation_actor_int))

CAF_END_TYPE_ID_BLOCK(okproject)
#define CONN_EXIT                            \
  [=](conn_exit_atom)                        \
  {                                          \
    LOG_DEBUG << "exiting " << self->name(); \
    self->unbecome();                        \
  }
namespace ok::smart_actor
{
namespace supervisor
{
inline std::unique_ptr<caf::actor_system_config> cfg;
inline std::unique_ptr<caf::actor_system> actorSystem;
inline main_actor_int mainActor;
inline sync_actor_int syncActor;
void initialiseMainActor(int argc, char *argv[]) noexcept;
std::string getReasonString(caf::error &err) noexcept;
using exception_handler = std::function<caf::error(caf::scheduled_actor *, std::exception_ptr &)>;
exception_handler default_exception_handler(std::string const &msg);
}  // namespace supervisor
}  // namespace ok::smart_actor
