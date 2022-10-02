#pragma once
#include "caf/all.hpp"
#include <drogon/WebSocketController.h>
#include <jsoncons/json.hpp>
#include "actor_system/Routes.hpp"
#include "db/Session.hpp"
#include "mutate/Field.hpp"
// #include "Endpoint/ConnectionInfo.h"
//#include "caf/io/middleman.hpp"
#include "tsl/ordered_map.h"
#include "utils/ErrorConstants.hpp"
CAF_BEGIN_TYPE_ID_BLOCK(okproject, first_custom_type_id)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(drogon::WebSocketConnectionPtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(drogon::WebSocketMessageType)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(jsoncons::ojson)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(ok::smart_actor::connection::Session)
// CAF_ALLOW_UNSAFE_MESSAGE_TYPE(arangodb::ConnectionInfo) 
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(ok::ErrorCode)
CAF_ADD_ATOM(okproject, conn_exit_old_atom);
CAF_ADD_ATOM(okproject, conn_exit_v8_atom);
CAF_ADD_ATOM(okproject, conn_exit_atom);
CAF_ADD_ATOM(okproject, shutdown_atom);
CAF_ADD_ATOM(okproject, schema_changed_atom);
CAF_ADD_ATOM(okproject, super_list_atom);
CAF_ADD_ATOM(okproject, global_list_atom);
CAF_ADD_ATOM(okproject, logout_atom);
CAF_ADD_ATOM(okproject, super_mutate_atom);
CAF_ADD_ATOM(okproject, user_mutate_atom);
CAF_ADD_ATOM(okproject, erase_atom);
CAF_ADD_ATOM(okproject, send_email_atom);
CAF_ADD_ATOM(okproject, spawn_and_monitor_atom);
CAF_ADD_ATOM(okproject, save_old_wsconnptr_atom);
CAF_ADD_ATOM(okproject, save_v8_wsconnptr_atom);
CAF_ADD_ATOM(okproject, save_new_wsconnptr_atom);
CAF_ADD_ATOM(okproject, pass_to_ws_connection_atom);
CAF_ADD_ATOM(okproject, pass_to_ws_v8_connection_atom);
CAF_ADD_ATOM(okproject, pass_to_browser_actor_atom);
CAF_ADD_ATOM(okproject, subscribe_to_total_ws_connections_atom);
CAF_ADD_ATOM(okproject, unsubscribe_to_total_ws_connections_atom);
CAF_ADD_ATOM(okproject, backup_atom);
CAF_ADD_ATOM(okproject, db_health_check_atom);
CAF_ADD_ATOM(okproject, session_clean_atom);
CAF_ADD_ATOM(okproject, remove_atom);
CAF_ADD_ATOM(okproject, send_to_same_browser_tab_atom);
CAF_ADD_ATOM(okproject, set_context_atom);
CAF_ADD_ATOM(okproject, dispatch_atom);
CAF_ADD_ATOM(okproject, send_to_one_atom);
CAF_ADD_ATOM(okproject, get_initial_data_atom);
CAF_ADD_ATOM(okproject, send_to_one_database_actors_atom);
CAF_ADD_ATOM(okproject, table_dispatch_atom);
CAF_ADD_ATOM(okproject, table_erase_atom);
CAF_ADD_ATOM(okproject, get_chat_room_atom);
CAF_ADD_ATOM(okproject, end_conversation_atom);
CAF_ADD_ATOM(okproject, file_notify_atom);
CAF_ADD_ATOM(okproject, insert_atom);
CAF_ADD_ATOM(okproject, insert_and_get_atom);
CAF_ADD_ATOM(okproject, sub_atom);
CAF_ADD_ATOM(okproject, get_session_atom);
CAF_ADD_ATOM(okproject, get_cloud_actor_atom);
CAF_ADD_ATOM(okproject, super_email_atom);
CAF_ADD_ATOM(okproject, super_auth_logout_atom);
CAF_ADD_ATOM(okproject, super_subdomain_mregister_mutate_atom);
CAF_ADD_ATOM(okproject, super_register_mutate_atom);
CAF_ADD_ATOM(okproject, super_confirm_mutate_atom);
CAF_ADD_ATOM(okproject, create_message_atom);
CAF_ADD_ATOM(okproject, modify_participan_atom);
CAF_ADD_ATOM(okproject, join_participant_atom);
CAF_ADD_ATOM(okproject, typing_event_atom);
CAF_ADD_ATOM(okproject, get_conversation_list_actor_atom);
CAF_ADD_ATOM(okproject, get_message_list_actor_atom);
CAF_ADD_ATOM(okproject, get_participant_list_actor_atom);
CAF_ADD_ATOM(okproject, get_conversation_muate_actor_atom);
CAF_ADD_ATOM(okproject, get_message_muate_actor_atom);
CAF_ADD_ATOM(okproject, get_participant_muate_actor_atom);
CAF_ADD_TYPE_ID(okproject, (drogon::WebSocketConnectionPtr))
CAF_ADD_TYPE_ID(okproject, (drogon::WebSocketMessageType))
CAF_ADD_TYPE_ID(okproject, (jsoncons::ojson))
CAF_ADD_TYPE_ID(okproject, (ok::smart_actor::connection::Session))
CAF_ADD_TYPE_ID(okproject, (std::vector<std::string>))
CAF_ADD_TYPE_ID(okproject, (std::unordered_set<std::string>))
// CAF_ADD_TYPE_ID(okproject, (arangodb::ConnectionInfo))
CAF_ADD_TYPE_ID(okproject, (ok::ErrorCode))
using file_notify_actor_int = caf::typed_actor<caf::reacts_to<file_notify_atom>, caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (file_notify_actor_int))
using backup_db_actor = caf::typed_actor<caf::reacts_to<backup_atom>, caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (backup_db_actor))
using db_health_check_actor_int = caf::typed_actor<caf::reacts_to<db_health_check_atom>, caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (db_health_check_actor_int))
using session_clean_actor_int = caf::typed_actor<caf::reacts_to<session_clean_atom>, caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (session_clean_actor_int))
using email_actor_int = caf::typed_actor<caf::reacts_to<send_email_atom, std::string, std::string, std::string, std::string>, caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (email_actor_int))
using ws_connector_actor_int = caf::typed_actor<caf::reacts_to<drogon::WebSocketConnectionPtr, std::string, std::string>,
                                                caf::replies_to<get_session_atom>::with<ok::smart_actor::connection::Session>,
                                                caf::reacts_to<set_context_atom, ok::smart_actor::connection::Session>,
                                                caf::reacts_to<std::string, drogon::WebSocketMessageType>,
                                                caf::reacts_to<caf::forward_atom, jsoncons::ojson>,
                                                caf::reacts_to<dispatch_atom, jsoncons::ojson>,
                                                caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (ws_connector_actor_int))
/*using ws_connector_v8_actor_int = caf::typed_actor<caf::reacts_to<drogon::WebSocketConnectionPtr, arangodb::ConnectionInfo>,
                                                   caf::reacts_to<std::string, drogon::WebSocketMessageType>,
                                                   caf::reacts_to<caf::forward_atom, jsoncons::ojson>,
                                                   caf::reacts_to<dispatch_atom, jsoncons::ojson>,
                                                   caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (ws_connector_v8_actor_int))*/
using main_actor_int = caf::typed_actor<caf::reacts_to<spawn_and_monitor_atom>,
                                        caf::reacts_to<save_old_wsconnptr_atom, drogon::WebSocketConnectionPtr, std::string, std::string>,
                                        caf::reacts_to<pass_to_ws_connection_atom, drogon::WebSocketConnectionPtr, std::string, drogon::WebSocketMessageType>,
                                        caf::reacts_to<conn_exit_old_atom, drogon::WebSocketConnectionPtr>,
                                        /*caf::reacts_to<save_v8_wsconnptr_atom, drogon::WebSocketConnectionPtr, arangodb::ConnectionInfo>,
                                        caf::reacts_to<pass_to_ws_v8_connection_atom, drogon::WebSocketConnectionPtr, std::string, drogon::WebSocketMessageType>,
                                        caf::reacts_to<conn_exit_v8_atom, drogon::WebSocketConnectionPtr>,*/
                                        // caf::reacts_to< std::shared_ptr<h2o_websocket_conn_t*>, uint8_t, std::string>,
                                        caf::reacts_to<shutdown_atom>>;
CAF_ADD_TYPE_ID(okproject, (main_actor_int))
template <typename T>
using table_actor_int = caf::typed_actor<caf::reacts_to<send_to_one_atom, WsEvent, WsArguments, DocumentKey, T>,
                                         typename caf::replies_to<get_initial_data_atom, WsEvent, WsArguments, DocumentKey, T>::template with<jsoncons::ojson>,
                                         typename caf::replies_to<caf::subscribe_atom, WsEvent, WsArguments, DocumentKey, T>::template with<bool>,
                                         caf::reacts_to<caf::unsubscribe_atom, WsEvent, T>,
                                         caf::reacts_to<erase_atom, std::unordered_set<DocumentKey>>,
                                         caf::reacts_to<table_dispatch_atom, std::unordered_set<DocumentKey>>,
                                         caf::reacts_to<schema_changed_atom>,
                                         caf::reacts_to<conn_exit_atom, T>,
                                         caf::reacts_to<conn_exit_atom>>;
using table_actor_int_ws = table_actor_int<ws_connector_actor_int>;
CAF_ADD_TYPE_ID(okproject, (table_actor_int<ws_connector_actor_int>))
CAF_ADD_TYPE_ID(okproject, (std::vector<table_actor_int<ws_connector_actor_int>>))
template <typename T>
using base_mutate_actor_int =
    caf::typed_actor<caf::reacts_to<insert_atom, DocumentKey, WsEvent, WsArguments, WsArguments, bool, bool, T>,
                     typename caf::replies_to<insert_and_get_atom, DocumentKey, WsEvent, WsArguments, WsArguments, bool, bool, T>::template with<ok::ErrorCode, std::unordered_set<DocumentKey>>,
                     caf::reacts_to<schema_changed_atom>,
                     caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (base_mutate_actor_int<ws_connector_actor_int>))
using auth_actor_int = caf::typed_actor<caf::reacts_to<ok::smart_actor::connection::Session, WsEvent, WsArguments, ws_connector_actor_int>, caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (auth_actor_int))
using subdomain_auth_actor_int = caf::typed_actor<caf::reacts_to<ok::smart_actor::connection::Session, WsEvent, WsArguments, ws_connector_actor_int, std::string>, caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (subdomain_auth_actor_int))
using auth_logout_actor_int = caf::typed_actor<caf::reacts_to<ok::smart_actor::connection::Session, WsEvent, ws_connector_actor_int>, caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (auth_logout_actor_int))
using login_actor_int = caf::typed_actor<caf::reacts_to<caf::add_atom, ok::smart_actor::connection::Session, ws_connector_actor_int>,
                                         caf::reacts_to<remove_atom, ok::smart_actor::connection::Session, ws_connector_actor_int>,
                                         caf::reacts_to<send_to_one_database_actors_atom, Database, jsoncons::ojson>,
                                         caf::reacts_to<send_to_same_browser_tab_atom, Database, DocumentKey, jsoncons::ojson>,
                                         caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (login_actor_int))
using chat_room_actor_int = caf::typed_actor<caf::replies_to<caf::subscribe_atom, WsEvent, WsArguments, DocumentKey, ws_connector_actor_int>::with<bool>,
                                             caf::reacts_to<caf::unsubscribe_atom, WsEvent, WsArguments, DocumentKey, DocumentKey, DocumentKey, ws_connector_actor_int>,
                                             caf::reacts_to<end_conversation_atom, WsEvent, WsArguments, DocumentKey, DocumentKey, DocumentKey, ws_connector_actor_int>,
                                             caf::reacts_to<join_participant_atom, WsEvent, WsArguments, DocumentKey, ws_connector_actor_int>,
                                             caf::reacts_to<create_message_atom, WsEvent, WsArguments, DocumentKey, DocumentKey, DocumentKey, ws_connector_actor_int>,
                                             caf::reacts_to<typing_event_atom, WsEvent, WsArguments, DocumentKey, ws_connector_actor_int>,
                                             caf::reacts_to<modify_participan_atom, WsEvent, WsArguments, DocumentKey, ws_connector_actor_int>,
                                             caf::replies_to<get_conversation_list_actor_atom>::with<table_actor_int<ws_connector_actor_int>>,
                                             caf::replies_to<get_message_list_actor_atom>::with<table_actor_int<ws_connector_actor_int>>,
                                             caf::replies_to<get_participant_list_actor_atom>::with<table_actor_int<ws_connector_actor_int>>,
                                             caf::replies_to<get_conversation_muate_actor_atom>::with<base_mutate_actor_int<ws_connector_actor_int>>,
                                             caf::replies_to<get_message_muate_actor_atom>::with<base_mutate_actor_int<ws_connector_actor_int>>,
                                             caf::replies_to<get_participant_muate_actor_atom>::with<base_mutate_actor_int<ws_connector_actor_int>>,
                                             caf::reacts_to<schema_changed_atom, std::string>,
                                             caf::reacts_to<caf::forward_atom, jsoncons::ojson>,
                                             caf::reacts_to<conn_exit_atom, ws_connector_actor_int>,
                                             caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (chat_room_actor_int))
CAF_ADD_TYPE_ID(okproject, (table_actor_int<chat_room_actor_int>))
CAF_ADD_TYPE_ID(okproject, (std::vector<table_actor_int<chat_room_actor_int>>))
using global_actor_int = caf::typed_actor<caf::replies_to<global_list_atom, Database, std::string>::with<table_actor_int<ws_connector_actor_int>>,
                                          caf::replies_to<user_mutate_atom, Database, std::string>::with<base_mutate_actor_int<ws_connector_actor_int>>,
                                          caf::reacts_to<schema_changed_atom, std::unordered_set<DocumentKey>>,
                                          caf::reacts_to<table_dispatch_atom, Database, std::string, std::unordered_set<DocumentKey>>,
                                          caf::reacts_to<table_erase_atom, Database, std::string, std::unordered_set<DocumentKey>>,
                                          caf::replies_to<get_chat_room_atom, Database, DocumentKey, DocumentKey>::with<chat_room_actor_int>,
                                          caf::reacts_to<shutdown_atom>>;
CAF_ADD_TYPE_ID(okproject, (global_actor_int))
using v8_actor_int =
    caf::typed_actor<caf::replies_to<drogon::WebSocketConnectionPtr, std::string>::with<std::string>, caf::reacts_to<conn_exit_atom, drogon::WebSocketConnectionPtr>, caf::reacts_to<conn_exit_atom>>;
CAF_ADD_TYPE_ID(okproject, (v8_actor_int))
/*using v8_actor_map_int = caf::typed_actor<caf::replies_to<caf::get_atom, std::string, std::string, std::string>::with<v8_actor_int>,
                                          caf::replies_to<erase_atom, std::string, std::string, std::string>::with<bool>,
                                          caf::replies_to<erase_atom, std::string>::with<bool>,
                                          caf::reacts_to<erase_atom, drogon::WebSocketConnectionPtr, ws_connector_v8_actor_int>,
                                          caf::reacts_to<caf::subscribe_atom, v8_actor_int, ws_connector_v8_actor_int>,
                                          caf::reacts_to<caf::unsubscribe_atom, v8_actor_int, ws_connector_v8_actor_int>,
                                          caf::reacts_to<caf::close_atom>>;
CAF_ADD_TYPE_ID(okproject, (v8_actor_map_int))*/
using graph_state_actor_int =
    caf::typed_actor<caf::replies_to<erase_atom, std::string, std::string, std::string>::with<bool>, caf::replies_to<erase_atom, std::string>::with<bool>, caf::reacts_to<caf::close_atom>>;
CAF_ADD_TYPE_ID(okproject, (graph_state_actor_int))
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
inline global_actor_int globalActor;
inline login_actor_int connectedActor;
// inline v8_actor_map_int v8StateActor;
// super/global actors:
inline table_actor_int<ws_connector_actor_int> userListActor;
inline table_actor_int<ws_connector_actor_int> schemaListActor;
inline table_actor_int<ws_connector_actor_int> translationListActor;
inline table_actor_int<ws_connector_actor_int> sessionListActor;
inline table_actor_int<ws_connector_actor_int> confirmListActor;
inline table_actor_int<ws_connector_actor_int> colorListActor;
inline table_actor_int<ws_connector_actor_int> permissionListActor;
inline table_actor_int<ws_connector_actor_int> roleListActor;
// inline table_actor_int<ws_connector_actor_int> supportListActor;
// inline table_actor_int<ws_connector_actor_int> supportAdminListActor;
inline table_actor_int<ws_connector_actor_int> templateListActor;
inline table_actor_int<ws_connector_actor_int> menuListActor;
inline table_actor_int<ws_connector_actor_int> nodeListActor;
inline table_actor_int<ws_connector_actor_int> edgeListActor;
inline table_actor_int<ws_connector_actor_int> attributeListActor;
inline caf::actor apiCountActor;
inline session_clean_actor_int sessionCleanActor;
inline db_health_check_actor_int databaseHealthCheckActor;
inline std::unordered_map<SchemaKey, table_actor_int<ws_connector_actor_int>> listActors;
// mutate actors stay live
inline base_mutate_actor_int<ws_connector_actor_int> userMutateActor;
inline base_mutate_actor_int<ws_connector_actor_int> schemaMutateActor;
inline base_mutate_actor_int<ws_connector_actor_int> translationMutateActor;
inline base_mutate_actor_int<ws_connector_actor_int> sessionMutateActor;
inline base_mutate_actor_int<ws_connector_actor_int> colorMutateActor;
inline base_mutate_actor_int<ws_connector_actor_int> permissionMutateActor;
inline base_mutate_actor_int<ws_connector_actor_int> roleMutateActor;
inline base_mutate_actor_int<ws_connector_actor_int> menuMutateActor;
inline base_mutate_actor_int<ws_connector_actor_int> nodeMutateActor;
inline base_mutate_actor_int<ws_connector_actor_int> edgeMutateActor;
inline base_mutate_actor_int<ws_connector_actor_int> attributeMutateActor;
inline email_actor_int emailMutateActor;
inline auth_actor_int registerMutateActor;
inline auth_actor_int confirmMutateActor;
inline subdomain_auth_actor_int memberRegisterMutateActor;
inline auth_logout_actor_int logoutMutateActor;
// inline base_mutate_actor_int<ws_connector_actor_int> supportMutateActor;
// inline base_mutate_actor_int<ws_connector_actor_int> supportAdminMutateActor;
inline base_mutate_actor_int<ws_connector_actor_int> templateMutateActor;
inline std::unordered_map<SchemaKey, base_mutate_actor_int<ws_connector_actor_int>> mutateActors;
void initialiseMainActor() noexcept;
std::string getReasonString(caf::error &err) noexcept;
using exception_handler = std::function<caf::error(caf::scheduled_actor *, std::exception_ptr &)>;
exception_handler default_exception_handler(std::string const &msg);
}  // namespace supervisor
}  // namespace ok::smart_actor
