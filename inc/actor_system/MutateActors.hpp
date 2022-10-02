#pragma once
#include "TableActor.hpp"
#include "WsConnectionActor.hpp"
#include "utils/BatchArrayMessage.hpp"
#include "mutate/Mutate.hpp"
namespace ok
{
namespace smart_actor
{
struct base_mutate_actor_state
{
  static inline constexpr char const *name = "base-mutate-actor";
  bool schemaUpdated{true};
  std::unique_ptr<arangodb::fuerte::Response> respFormSchema;
  VPackSlice formSchemaJson;
  std::unique_ptr<arangodb::fuerte::Response> respColumnSchema;
  VPackSlice columnSchemaJson;
};
}  // namespace smart_actor
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
namespace impl
{
std::tuple<ErrorCode, ok::mutate_schema::Fields> initializeSchemaAndCheck(WsArguments const &args, std::string const &schemaKey, DocumentKey const &memberKey, std::vector<std::string> const &checkFields);
void createDirectories(ok::smart_actor::connection::Session const &session);
std::string settingRedirectionUrl(bool emailVerified);
template <typename A>
void sendUserRegistrationSuccessEmail(A act, ok::mutate_schema::Fields const &fields, std::string const &confirmUrl);
template <typename A>
void sendMemberRegistrationSuccessEmail(A act, ok::mutate_schema::Fields const &fields, std::string const &confirmUrl);
template <typename A>
void sendConfirmationSuccessEmail(A act, Email const &email);
void cleanContextAndSendResult(auth_logout_actor_int::pointer act, WsEvent const &event, ok::smart_actor::connection::Session const &session, ws_connector_actor_int connectionActor);
template <typename A>
void sendResult(ErrorCode er, A *act, const ok::smart_actor::connection::Session &session, ws_connector_actor_int connectionActor, WsEvent const &event, std::string const &redirectUrl, ok::mutate_schema::Fields const &fields);
// clang-format on
}  // namespace impl
}  // namespace smart_actor::auth
namespace smart_actor
{
template <typename T>
using BaseMutateActorPtr = typename base_mutate_actor_int<T>::template stateful_pointer<base_mutate_actor_state>;
template <typename T>
void sendToListActors(BaseMutateActorPtr<T> self,
                      ok::smart_actor::connection::MutateEventType eventType,
                      std::unordered_set<DocumentKey> const &changedKeys,
                      std::vector<table_actor_int<T>> const &actors) noexcept
{
  for (auto const &t : actors)
  {
    if (ok::mutate_actor::isModifyEvent(eventType)) self->send(t, table_dispatch_atom_v, changedKeys);
    else
      self->send(t, erase_atom_v, changedKeys);
  }
}
}  // namespace smart_actor
namespace smart_actor::user
{
inline ok::ErrorCode emptyPreProcess(DocumentKey const &database, WsArguments const &args, ok::smart_actor::connection::MutateEventType et, ok::mutate_schema::Fields &fields)
{
  return ok::ErrorCode::ERROR_NO_ERROR;
}
template <typename T>
ErrorCode saveState(BaseMutateActorPtr<T> self, SchemaKey const &schemaKey)
{
  if (self->state.schemaUpdated)
  {
    auto [erS, myRespFormSchema] = ok::db::getSchema("global", "schema", schemaKey, ".fields");
    if (ok::isEr(erS))
    {
      LOG_DEBUG << "SCHEMA is invalid: " << schemaKey;
      return erS;
    }
    auto [erC, myRespColumnSchema] = ok::db::getSchema("global", "schema", schemaKey, ".columns");
    if (ok::isEr(erC))
    {
      LOG_DEBUG << "SCHEMA is invalid: " << schemaKey;
      return erC;
    }
    self->state.formSchemaJson = myRespFormSchema->slices().front().get("result")[0];
    self->state.columnSchemaJson = myRespColumnSchema->slices().front().get("result")[0];
    self->state.respFormSchema = std::move(myRespFormSchema);
    self->state.respColumnSchema = std::move(myRespColumnSchema);
    self->state.schemaUpdated = false;
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
void sendNotification(VPackSlice const &schemaJson) noexcept;
template <typename T>
void unsubscribe(BaseMutateActorPtr<T> self, const jsoncons::ojson &args, T connectionActor, table_actor_int<T> listActor) noexcept
{
  if (args.contains("unSub") && args["unSub"].is_array())
  {
    WsEvent event_ = args["unSub"];
    self->send(listActor, caf::unsubscribe_atom_v, event_, connectionActor);
  }
  else
  {
    // (If not unsubscribed it will leave subscribed on tableActor)
    // LOG_DEBUG << "No unsubscribe events while update: " <<
    // args.to_string()
    //;
  }
}
template <typename T>
inline std::tuple<ok::ErrorCode, Fields, std::unordered_set<DocumentKey>> doMutatation(BaseMutateActorPtr<T> self,
                                                                                       std::vector<table_actor_int<T>> const &listActors,
                                                                                       Database const &database,
                                                                                       SchemaKey const &schemaKey,
                                                                                       ok::mutate_actor::MutateCallbackFunction preProcess,
                                                                                       ok::mutate_actor::MutateCallbackFunction postProcess,
                                                                                       DocumentKey &memberKey,
                                                                                       WsEvent &event,
                                                                                       WsArguments &args,
                                                                                       WsArguments const &backendValue,
                                                                                       bool permissionCheck,
                                                                                       bool ignoreMissingFieldWithValue,
                                                                                       T &connectionActor)
{
  if (auto er = user::saveState<T>(self, schemaKey); ok::isEr(er)) { return {er, {}, {}}; }
  auto et = ok::mutate_actor::getMutateEventType(event);
  if (auto [er, query, queryParts, mergeBindVars, changedKeys, fields] = ok::mutate_actor::save(
          database, memberKey, schemaKey, self->state.formSchemaJson, self->state.columnSchemaJson, et, args, backendValue, permissionCheck, ignoreMissingFieldWithValue, preProcess, postProcess);
      ok::isEr(er))
  {
    return {er, fields, changedKeys};
  }
  else
  {
    /* sendNotification(schemaJson); todo fix this*/
    user::unsubscribe(self, args, connectionActor, listActors[0]);
    ok::smart_actor::sendToListActors<T>(self, et, changedKeys, listActors);
    return {er, fields, changedKeys};
  }
}
template <typename T>
typename base_mutate_actor_int<T>::behavior_type BaseMutateActor(BaseMutateActorPtr<T> self,
                                                                 std::vector<table_actor_int<T>> const listActors,
                                                                 Database const database,
                                                                 SchemaKey const schemaKey,
                                                                 ok::mutate_actor::MutateCallbackFunction preProcess,
                                                                 ok::mutate_actor::MutateCallbackFunction postProcess)
{
  return {
      [=](insert_atom, DocumentKey &memberKey, WsEvent &event, WsArguments &args, WsArguments const &backendValue, bool permissionCheck, bool ignoreMissingFieldWithValue, T connectionActor)
      {
        LOG_DEBUG << "==================================" << event.as_string() << " Start ============================================";
        auto [er, fields, changedKeys] =
            doMutatation<T>(self, listActors, database, schemaKey, preProcess, postProcess, memberKey, event, args, backendValue, permissionCheck, ignoreMissingFieldWithValue, connectionActor);
        self->send(connectionActor, caf::forward_atom_v, ok::smart_actor::connection::addFailure(ok::smart_actor::connection::wsMessageBase(), event, er, fields));
        LOG_DEBUG << "==================================" << event.as_string() << " End ============================================";
      },
      [=](insert_and_get_atom, DocumentKey &memberKey, WsEvent &event, WsArguments &args, WsArguments const &backendValue, bool permissionCheck, bool ignoreMissingFieldWithValue, T connectionActor)
      {
        LOG_DEBUG << "==================================" << event.as_string() << " Start ============================================";
        auto [er, fields, changedKeys] =
            doMutatation<T>(self, listActors, database, schemaKey, preProcess, postProcess, memberKey, event, args, backendValue, permissionCheck, ignoreMissingFieldWithValue, connectionActor);
        LOG_DEBUG << "==================================" << event.as_string() << " End ============================================";
        return caf::make_result(er, changedKeys);
      },
      [=](schema_changed_atom) { self->state.schemaUpdated = true; },
      CONN_EXIT};
}
}  // namespace smart_actor::user
namespace smart_actor::global_collections
{
base_mutate_actor_int<ws_connector_actor_int>::behavior_type MutateUserActor(BaseMutateActorPtr<ws_connector_actor_int> self,
                                                                             table_actor_int<ws_connector_actor_int> listActor,
                                                                             Database database,
                                                                             SchemaKey schemaKey);
base_mutate_actor_int<ws_connector_actor_int>::behavior_type GlobalSchemaMutateActor(BaseMutateActorPtr<ws_connector_actor_int> self,
                                                                                     table_actor_int<ws_connector_actor_int> listActor,
                                                                                     Database database,
                                                                                     SchemaKey schemaKey);
base_mutate_actor_int<ws_connector_actor_int>::behavior_type MutateGlobalColorActor(BaseMutateActorPtr<ws_connector_actor_int> self,
                                                                                    table_actor_int<ws_connector_actor_int> listActor,
                                                                                    Database database,
                                                                                    SchemaKey schemaKey);
base_mutate_actor_int<ws_connector_actor_int>::behavior_type MutateGlobalMenuActor(BaseMutateActorPtr<ws_connector_actor_int> self,
                                                                                   table_actor_int<ws_connector_actor_int> listActor,
                                                                                   Database database,
                                                                                   SchemaKey schemaKey);
base_mutate_actor_int<ws_connector_actor_int>::behavior_type MutateGlobalPermissionActor(BaseMutateActorPtr<ws_connector_actor_int> self,
                                                                                         table_actor_int<ws_connector_actor_int> listActor,
                                                                                         Database database,
                                                                                         SchemaKey schemaKey);
base_mutate_actor_int<ws_connector_actor_int>::behavior_type MutateGlobalRoleActor(BaseMutateActorPtr<ws_connector_actor_int> self,
                                                                                   table_actor_int<ws_connector_actor_int> listActor,
                                                                                   Database database,
                                                                                   SchemaKey schemaKey);
}  // namespace smart_actor::global_collections
}  // namespace ok
