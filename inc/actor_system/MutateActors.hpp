#pragma once
#include "TableActor.hpp"
#include "WsConnectionActor.hpp"
#include "utils/BatchArrayMessage.hpp"
namespace ok
{
namespace smart_actor::auth
{
auth_actor_int::behavior_type AuthRegisterActor(auth_actor_int::pointer self);
subdomain_auth_actor_int::behavior_type AuthMemberRegisterActor(subdomain_auth_actor_int::pointer self);
auth_actor_int::behavior_type AuthLoginActor(auth_actor_int::pointer self);
subdomain_auth_actor_int::behavior_type AuthMemberLoginActor(subdomain_auth_actor_int::pointer self);
auth_actor_int::behavior_type AuthConfirmationActor(auth_actor_int::pointer self);
auth_logout_actor_int::behavior_type AuthLogoutActor(auth_logout_actor_int::pointer self);
void userRegister(auth_actor_int::typed_actor::pointer act, WsEvent const &event, WsArguments const &args, ok::smart_actor::connection::Session const &session, ws_connector_actor_int connectionActor);
// clang-format off
void memberRegister(subdomain_auth_actor_int::pointer act, WsEvent const &event, WsArguments const &args, ok::smart_actor::connection::Session const &session, std::string const &subDomain, ws_connector_actor_int connectionActor);
void login(auth_actor_int::typed_actor::pointer act, WsEvent const &event, WsArguments const &args, ok::smart_actor::connection::Session const &session, ws_connector_actor_int connectionActor);
void memberLogin(subdomain_auth_actor_int::pointer act, WsEvent const &event, WsArguments const &args, ok::smart_actor::connection::Session const &session, std::string const &subDomain, ws_connector_actor_int connectionActor);
ErrorCode confirmMember(auth_actor_int::typed_actor::pointer act, WsEvent const &event, WsArguments const &args, ok::smart_actor::connection::Session const &session, ws_connector_actor_int connectionActor);
void logout(auth_logout_actor_int::pointer act, WsEvent const &event, const ok::smart_actor::connection::Session &session, ws_connector_actor_int connectionActor);
}  // namespace smart_actor::auth
namespace smart_actor::user
{
inline ok::ErrorCode emptyPreProcess(DocumentKey const &database, WsArguments const &args, ok::smart_actor::connection::MutateEventType et, ok::mutate_schema::Fields &fields)
{
  return ok::ErrorCode::ERROR_NO_ERROR;
}
}
}  // namespace ok
