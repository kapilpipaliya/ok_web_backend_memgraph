#include "actor_system/MutateActors.hpp"
#include <filesystem>
#include "mutate/Mutate.hpp"
#include "utils/ErrorConstants.hpp"
#include "utils/json_functions.hpp"
#include "utils/string_functions.hpp"
namespace ok::smart_actor
{
namespace auth
{
auth_actor_int::behavior_type AuthRegisterActor(auth_actor_int::pointer self)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("Mutate Register Actor"));
  return {[=](ok::smart_actor::connection::Session const &session, WsEvent const &event, WsArguments const &args, ws_connector_actor_int connectionActor)
          { userRegister(self, event, args, session, connectionActor); },
          CONN_EXIT};
}
subdomain_auth_actor_int::behavior_type AuthMemberRegisterActor(subdomain_auth_actor_int::pointer self)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("MutateMRegisterActor"));
  return {[=](ok::smart_actor::connection::Session const &session, WsEvent const &event, WsArguments const &args, ws_connector_actor_int connectionActor, std::string const &subDomain)
          { memberRegister(self, event, args, session, subDomain, connectionActor); },
          CONN_EXIT};
}
auth_actor_int::behavior_type AuthLoginActor(auth_actor_int::pointer self)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("Mutate Login Actor"));
  return {[=](ok::smart_actor::connection::Session const &session, WsEvent const &event, WsArguments const &args, ws_connector_actor_int connectionActor)
          { login(self, event, args, session, connectionActor); },
          CONN_EXIT};
}
subdomain_auth_actor_int::behavior_type AuthMemberLoginActor(subdomain_auth_actor_int::pointer self)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("Mutate Member Login Actor"));
  return {[=](ok::smart_actor::connection::Session const &session, WsEvent const &event, WsArguments const &args, ws_connector_actor_int connectionActor, std::string const &subDomain)
          { memberLogin(self, event, args, session, subDomain, connectionActor); },
          CONN_EXIT};
}
auth_actor_int::behavior_type AuthConfirmationActor(auth_actor_int::pointer self)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("Mutate confirmation Actor"));
  return {[=](ok::smart_actor::connection::Session const &session, WsEvent const &event, WsArguments const &args, ws_connector_actor_int connectionActor)
          { confirmMember(self, event, args, session, connectionActor); },
          CONN_EXIT};
}
auth_logout_actor_int::behavior_type AuthLogoutActor(auth_logout_actor_int::pointer self)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("Mutate Logout Actor"));
  return {[=](ok::smart_actor::connection::Session const &session, WsEvent const &event, ws_connector_actor_int connectionActor) { logout(self, event, session, connectionActor); }, CONN_EXIT};
}
inline ok::ErrorCode validateEmail(ok::mutate_schema::Field emailField)
{
  if (!emailField.value.is_string())
  {
    emailField.error = emailField.label + " should be string, but got: " + std::to_string(static_cast<int>(emailField.value.type()));
    return ok::ErrorCode::ERROR_FORM_EMPTY;
  }
  if (!ok::utils::string::ValidateEmail(emailField.value.as_string()))
  {
    emailField.error = emailField.label + " is not valid";
    return ok::ErrorCode::ERROR_FORM_EMPTY;
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
inline ok::ErrorCode validatePassword(ok::mutate_schema::Field &passField)
{
  auto password = passField.value.as_string();
  if (password.length() < 6)
  {
    passField.error = passField.label + " should be minimum 6 characters";
    return ok::ErrorCode::ERROR_FORM_EMPTY;
  }
  if (!ok::utils::string::isAlphaNumeric(password))
  {
    passField.error = passField.label + " should be alphanumeric";
    return ok::ErrorCode::ERROR_FORM_EMPTY;
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
inline ok::ErrorCode validateUrl(ok::mutate_schema::Field &urlField)
{
  if (!ok::utils::string::validateSubdomain(urlField.value.as_string()))
  {
    urlField.error = urlField.label + " is not valid";
    return ok::ErrorCode::ERROR_FORM_EMPTY;
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
inline ok::ErrorCode uniqueFieldCheck(std::string const &database, const std::string &schema, ok::mutate_schema::Fields &fields, Selector const &selector)
{
  auto [er, field] = ok::mutate_schema::getField(fields, selector);
  if (isEr(er)) return er;
  if (auto er = ok::db::uniqueMemberFieldCheck(database, schema, selector, field.value.as_string()); isEr(er))
  {
    if (er == ok::ErrorCode::UNIQUE_ERROR)
    {
      er = ok::ErrorCode::ERROR_FORM_EMPTY;
      field.error = field.label + " is already taken. value: " + field.value.as<std::string>();
      return er;
    }
    else
      return er;
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
void userRegister(auth_actor_int::pointer act, WsEvent const &event, WsArguments const &args, ok::smart_actor::connection::Session const &session, ws_connector_actor_int connectionActor)
{
  // clang-format off
  auto [er1, fields] = impl::initializeSchemaAndCheck(args, "register",session.memberKey, {"email", "pass", "url"});
  auto returnOldSession = [&, fields=std::ref(fields)](auto er){return impl::sendResult(er, act, session, connectionActor, event, "", fields);};
  if (isEr(er1)) return returnOldSession(er1);
  auto [erF, fieldFirstName] = ok::mutate_schema::getField(fields, "firstName"); if(ok::isEr(erF)) return returnOldSession(erF);
  auto [erL, fieldLastName] = ok::mutate_schema::getField(fields, "lastName"); if(ok::isEr(erL)) return returnOldSession(erL);
  auto [erE, fieldEmail] = ok::mutate_schema::getField(fields, "email"); if(ok::isEr(erE)) return returnOldSession(erE);
  auto [erP, fieldPass] = ok::mutate_schema::getField(fields, "pass"); if(ok::isEr(erP)) return returnOldSession(erP);
  auto [erU, fieldUrl] = ok::mutate_schema::getField(fields, "url"); if(ok::isEr(erU)) return returnOldSession(erU);
  if(auto er2 = validateEmail(fieldEmail); isEr(er2)) return returnOldSession(er2);
  if(auto er3 = validatePassword(fieldPass); isEr(er3)) return returnOldSession(er3);
  if(auto er4 = validateUrl(fieldUrl); isEr(er4)) return returnOldSession(er4);
  if(auto er5 = uniqueFieldCheck("user", "user", fields, "email"); isEr(er5)) return returnOldSession(er5);
  if(auto er6 = uniqueFieldCheck("user", "user", fields, "url"); isEr(er6)) return returnOldSession(er6);
  ok::smart_actor::connection::Session newSession{session};
  auto returnNewSession = [&, fields=std::ref(fields)](auto er){return impl::sendResult(er, act, newSession, connectionActor, event, "", fields);};
  if (auto [er7, database] = ok::db::saveUser(fieldEmail.value.as_string(), fieldPass.value.as_string(), fieldUrl.value.as_string()); isEr(er7)) return returnNewSession(er7);
  else newSession.database = std::move(database);
  if (auto er8 = ok::db::createDatabase(newSession.database); isEr(er8)) return returnNewSession(er8);
  if (auto er9 = ok::db::createCollections(newSession.database); isEr(er9)) return returnNewSession(er9);
  if (auto er10 = ok::db::copyGlobalCollections(newSession.database); isEr(er10)) return returnNewSession(er10);
  if (auto [er11, memberKey] = ok::db::createMember(newSession.database,fieldFirstName.value.as_string(), fieldLastName.value.as_string(), fieldEmail.value.as_string(), fieldPass.value.as_string(), "1", true, true); isEr(er11)) return returnNewSession(er11);
  else newSession.memberKey = std::move(memberKey);
  if (auto [er11, memberKey] = ok::db::createMember(newSession.database,fieldFirstName.value.as_string(), fieldLastName.value.as_string(), "guest@guest.com", "password", "guest", false, true); isEr(er11)) return returnNewSession(er11);
  if (auto er12 = ok::db::createPermissions(newSession.database); isEr(er12)) return returnNewSession(er12);
  impl::createDirectories(newSession);
  if (auto [er13, confirmUrl] = ok::db::generateConfirmUrl(newSession.database, newSession.memberKey); isEr(er13)) return returnNewSession(er13);
  else impl::sendUserRegistrationSuccessEmail(act, fields, confirmUrl);
  if (auto er14 = ok::db::createLoginDbSession(newSession); isEr(er14)) return returnNewSession(er14);
  return impl::sendResult(ok::ErrorCode::ERROR_NO_ERROR, act, newSession, connectionActor, event, "/account/confirm", fields);
}
void memberRegister(subdomain_auth_actor_int::pointer act, WsEvent const &event, WsArguments const &args, ok::smart_actor::connection::Session const &session, std::string const &subDomain, ws_connector_actor_int connectionActor)
{
  if (subDomain.empty()) return impl::sendResult(ok::ErrorCode::ERROR_INTERNAL, act, session, connectionActor, event, "", {});
  auto [er1, fields] = impl::initializeSchemaAndCheck(args, "mregister",session.memberKey, {"email", "pass"});
  auto returnOldSession = [&, fields=std::ref(fields)](auto er){return impl::sendResult(er, act, session, connectionActor, event, "", fields);};
  if (isEr(er1)) return returnOldSession(er1);
  auto [erF, fieldFirstName] = ok::mutate_schema::getField(fields, "firstName"); if(ok::isEr(erF)) return returnOldSession(erF);
  auto [erL, fieldLastName] = ok::mutate_schema::getField(fields, "lastName"); if(ok::isEr(erL)) return returnOldSession(erL);
  auto [erE, fieldEmail] = ok::mutate_schema::getField(fields, "email"); if(ok::isEr(erE)) return returnOldSession(erE);
  auto [erP, fieldPass] = ok::mutate_schema::getField(fields, "pass"); if(ok::isEr(erP)) return returnOldSession(erP);
  if(auto er2 = validateEmail(fieldEmail); isEr(er2)) return returnOldSession(er2);
  if(auto er3 = validatePassword(fieldPass); isEr(er3)) return returnOldSession(er3);
  ok::smart_actor::connection::Session newSession{session};
  auto returnNewSession = [&, fields=std::ref(fields)](auto er){return impl::sendResult(er, act, newSession, connectionActor, event, "", fields);};
  if (auto [er4, database] = ok::db::findDatabaseNameFromSubdomain(subDomain); isEr(er4)) return returnNewSession(er4); else newSession.database = std::move(database);
  if(auto er5 = uniqueFieldCheck(newSession.database, "member", fields, "email"); isEr(er5)) return returnNewSession(er5);
  if (auto [er6, memberKey] = ok::db::createMember(newSession.database,fieldFirstName.value.as_string(), fieldLastName.value.as_string(), fieldEmail.value.as_string(), fieldPass.value.as_string()); isEr(er6)) return returnNewSession(er6); else newSession.memberKey = std::move(memberKey);
  if (auto er7 = ok::db::createMemberPermissions(newSession.database, newSession.memberKey); isEr(er7)) return returnNewSession(er7);
  // createDirectories();
  if (auto [er8, confirmUrl] = ok::db::generateConfirmUrl(newSession.database, newSession.memberKey); isEr(er8)) return returnNewSession(er8);
  else impl::sendMemberRegistrationSuccessEmail(act, fields, confirmUrl);
  if (auto er9 = ok::db::createLoginDbSession(newSession); isEr(er9)) return returnNewSession(er9);
  return impl::sendResult(ok::ErrorCode::ERROR_NO_ERROR, act, newSession, connectionActor, event, "/account/confirm", {});
}
void login(auth_actor_int::typed_actor::pointer act, WsEvent const &event, WsArguments const &args, ok::smart_actor::connection::Session const &session, ws_connector_actor_int connectionActor)
{
  auto [er1, fields] = impl::initializeSchemaAndCheck(args, "login",session.memberKey, {"email", "pass"});
  auto returnOldSession = [&, fields=std::ref(fields)](auto er){return impl::sendResult(er, act, session, connectionActor, event, "", fields);};
  if (isEr(er1)) return returnOldSession(er1);
  auto [erE, fieldEmail] = ok::mutate_schema::getField(fields, "email"); if(ok::isEr(erE)) return returnOldSession(erE);
  auto [erP, fieldPass] = ok::mutate_schema::getField(fields, "pass"); if(ok::isEr(erP)) return returnOldSession(erP);
  ok::smart_actor::connection::Session newSession{session};
  auto returnNewSession = [&, fields=std::ref(fields)](auto er){return impl::sendResult(er, act, newSession, connectionActor, event, "", fields);};
  if (auto [er2, database] = ok::db::findDatabaseFromUserEmail(fieldEmail.value.as_string()); isEr(er2)) return returnNewSession(er2); else newSession.database = std::move(database);
  auto [er3, memberKey, email,  emailVerified] = ok::db::findMember(newSession.database, fieldEmail.value.as_string(), fieldPass.value.as_string());
  if (isEr(er3)) return returnNewSession(er3);
     else newSession.memberKey = memberKey;
  if (auto er4 = ok::db::createLoginDbSession(newSession); isEr(er4)) return returnNewSession(er4);
  impl::sendResult(ok::ErrorCode::ERROR_NO_ERROR, act, newSession, connectionActor, event, impl::settingRedirectionUrl(emailVerified), fields);
  // todo: send login to same user on same browser.
  // send(ok::smart_actor::supervisor::connectedActor,  member_atom_v, "","",f.result);
}
void memberLogin(subdomain_auth_actor_int::pointer act, WsEvent const &event, WsArguments const &args, const connection::Session &session, std::string const &subDomain, ws_connector_actor_int connectionActor)
{
  if (subDomain.empty()) return impl::sendResult(ok::ErrorCode::ERROR_INTERNAL, act, session, connectionActor, event, "", {});
  auto [er1, fields] = impl::initializeSchemaAndCheck(args, "mlogin",session.memberKey, {"email", "pass"});
  auto returnOldSession = [&, fields=std::ref(fields)](auto er){return impl::sendResult(er, act, session, connectionActor, event, "", fields);};
  if (isEr(er1)) return returnOldSession(er1);
  auto [erE, fieldEmail] = ok::mutate_schema::getField(fields, "email"); if(ok::isEr(erE)) return returnOldSession(erE);
  auto [erP, fieldPass] = ok::mutate_schema::getField(fields, "pass"); if(ok::isEr(erP)) return returnOldSession(erP);
  ok::smart_actor::connection::Session newSession{session};
  auto returnNewSession = [&, fields=std::ref(fields)](auto er){return impl::sendResult(er, act, newSession, connectionActor, event, "", fields);};
  if (auto [er2, database] = ok::db::findDatabaseNameFromSubdomain(subDomain); isEr(er2)) return returnNewSession(er2); else newSession.database = std::move(database);
  if (auto er3 = ok::db::isDatabaseExist(newSession.database); isEr(er3)) return returnNewSession(er3);
  auto [er4, memberKey, email,  emailVerified] = ok::db::findMember(newSession.database, fieldEmail.value.as_string(), fieldPass.value.as_string());
  if (isEr(er4)) return returnNewSession(er4); else newSession.memberKey = memberKey;
  if (auto er5 = ok::db::createLoginDbSession(newSession); isEr(er5)) return returnNewSession(er5);
  return impl::sendResult(ok::ErrorCode::ERROR_NO_ERROR, act, newSession, connectionActor, event, impl::settingRedirectionUrl(emailVerified), fields);
  // todo: send login to same user on same browser.
  // send(ok::smart_actor::supervisor::connectedActor,  member_atom_v, "","",f.result);
  // clang-format on
}
ErrorCode confirmMember(
    auth_actor_int::typed_actor::pointer act, WsEvent const &event, WsArguments const &args, ok::smart_actor::connection::Session const &session, ws_connector_actor_int connectionActor)
{
  if (jsoncons::ObjectMemberIsString(args, "key") && jsoncons::ObjectMemberIsString(args, "token") && !args["key"].as_string_view().empty() && !args["token"].as_string_view().empty())
  {
    if (auto [er, dbName, memberKey] = ok::db::confirmMember(args["key"].as<std::string>(), args["token"].as<std::string>()); ok::isEr(er))
    {
      act->send(connectionActor, caf::forward_atom_v, ok::smart_actor::connection::addFailure(ok::smart_actor::connection::wsMessageBase(), event, ok::ErrorCode::EXPIRED));
      return er;
    }
    else
    {
      if (auto [error, memberKey, email, emailVerified] = ok::db::findMember(dbName, session.memberKey); ok::isEr(error))
      {
        ok::smart_actor::connection::Session newSession{session};
        newSession.database = dbName;
        newSession.memberKey = std::move(memberKey);
        if (auto err = ok::db::createLoginDbSession(newSession); ok::isEr(err))
        {
          LOG_ERROR << "this should not happen, when creating login session.";
          act->send(connectionActor, caf::forward_atom_v, ok::smart_actor::connection::addFailure(ok::smart_actor::connection::wsMessageBase(), event, ok::ErrorCode::EXPIRED));
          return err;
        }
        else
        {
          act->send(connectionActor, set_context_atom_v, newSession);
          impl::sendConfirmationSuccessEmail(act, email);
          act->send(connectionActor, caf::forward_atom_v, ok::smart_actor::connection::addSuccess(ok::smart_actor::connection::wsMessageBase(), event));
          return ok::ErrorCode::ERROR_NO_ERROR;
        }
      }
    }
  }
  return ok::ErrorCode::EXPIRED;
}
void logout(auth_logout_actor_int::pointer act, WsEvent const &event, ok::smart_actor::connection::Session const &session, ws_connector_actor_int connectionActor)
{
  auto db = session.database;
  if (ok::db::cleanDBSession(session.sessionKey)) impl::cleanContextAndSendResult(act, event, session, connectionActor);
  // act->send(ok::smart_actor::supervisor::globalActor, logout_atom_v, db, connectionActor); // todo fix make new actor same like global actor to manage state
  // todo: send logout to same user on same browser. Todo!
  // send(ok::smart_actor::supervisor::connectedActor,  member_atom_v, "", "", red.result);
}
namespace impl
{
std::tuple<ErrorCode, ok::mutate_schema::Fields> initializeSchemaAndCheck(WsArguments const &args,
                                                                          std::string const &schemaKey,
                                                                          DocumentKey const &memberKey,
                                                                          std::vector<std::string> const &checkFields)
{
  if (args.is_object() && args.contains("value") && args["value"].is_object())
  {
    auto [erS, myRespFormSchema] = ok::db::getSchema("global", "schema", schemaKey, ".fields");
    if (ok::isEr(erS)) { return {ok::ErrorCode::SCHEMA_ERROR, {}}; }
    else
    {
      auto schemaJson = myRespFormSchema->slices().front().get("result")[0];
      auto [er, collection, tree, fields, allIds] = ok::mutate_schema::initialize(schemaJson, ok::smart_actor::connection::MutateEventType::insert);
      if (isEr(er)) return {er, fields};
      ok::mutate_schema::MutateQueryParts queryParts;
      if (auto [er, fieldsNew, allIdsNew] = processDataPart(schemaJson, args, ok::smart_actor::connection::MutateEventType::insert, collection, tree, queryParts, memberKey, false); isEr(er), false)
        return {er, fields};
      else
      {
        fields.insert(std::begin(fieldsNew), std::end(fieldsNew));
        allIds.insert(std::end(allIds), std::begin(allIdsNew), std::end(allIdsNew));
      }
      if (auto er = ok::mutate_schema::make_bind_vars::setNonRelationFieldValues(fields, args["value"], jsoncons::ojson{}, false); isEr(er))
      {
        auto fieldErrors = ok::mutate_schema::fieldErrors(fields);  // todo use this errors.
        return {er, fields};
      }
      // LOG_DEBUG << caf::deep_to_string(allIds);
      // if (auto [er, _] = ok::mutate_schema::makeFormObject(fields, allIds); isEr(er)) return {er, fields};
      bool isEmptyField{false};
      for (auto &f : checkFields)
      {
        auto [er, field] = ok::mutate_schema::getField(fields, f);
        if (ok::isEr(er)) { return {ok::ErrorCode::ERROR_BAD_PARAMETER, fields}; }
        if (!field.value.is_string())
        {
          LOG_DEBUG << "field: " << field.id << " should be string";
          return {ok::ErrorCode::ERROR_BAD_PARAMETER, fields};
        }
        if (field.value.empty())
        {
          field.error = field.label + " should not empty";
          isEmptyField = true;
        }
      }
      if (isEmptyField) return {ok::ErrorCode::ERROR_FORM_FIELD_ERROR, fields};
      return {ok::ErrorCode::ERROR_NO_ERROR, fields};
    }
  }
  LOG_DEBUG << "Form argument data must be object";
  return {ok::ErrorCode::SCHEMA_ERROR, {}};
}
void createDirectories(ok::smart_actor::connection::Session const &session)
{
  // no other thread is allowed to change environment while getenv() is working
  // otherwise that looks ok
  auto userPath = std::filesystem::path(ok::smart_actor::connection::drogonRoot).append("user_data").append(session.database);
  std::filesystem::create_directories(userPath);
}
std::string settingRedirectionUrl(bool emailVerified)
{
  if (!emailVerified) { return "/account/confirm"; }
  else
  {
    return "/";
  }
}
template <typename A>
void sendUserRegistrationSuccessEmail(A act, ok::mutate_schema::Fields const &fields, std::string const &confirmUrl)
{
  // spawn an actor to send email
  auto [erE, fieldEmail] = ok::mutate_schema::getField(fields, "email");
  if (ok::isEr(erE)) return;
  auto [erP, fieldPass] = ok::mutate_schema::getField(fields, "pass");
  if (ok::isEr(erP)) return;
  auto [erU, fieldUrl] = ok::mutate_schema::getField(fields, "url");
  if (ok::isEr(erU)) return;
  std::string body = "Registration Confirmation: \n";
  body += "Your server url is: https:://" + fieldUrl.value.as_string() + ".o-k.tech\n";
  body +=
      "To confirm your registration please "
      "visit the link below\n" +
      confirmUrl + "\n";
  act->send(ok::smart_actor::supervisor::emailMutateActor, send_email_atom_v, "", fieldEmail.value.as_string(), "Please Confirm your Email", body);
}
template <typename A>
void sendMemberRegistrationSuccessEmail(A act, ok::mutate_schema::Fields const &fields, std::string const &confirmUrl)
{
  // spawn an actor to send email
  auto [erE, fieldEmail] = ok::mutate_schema::getField(fields, "email");
  if (ok::isEr(erE)) return;
  // auto mail_mutate_actor = spawn<MutateEmailActor>();
  act->send(ok::smart_actor::supervisor::emailMutateActor,
            send_email_atom_v,
            "",
            fieldEmail.value.as_string(),
            "Registration Confirmation",
            "To confirm your registration please visit the link below\n" + confirmUrl + "\n");
}
template <typename A>
void sendConfirmationSuccessEmail(A act, Email const &email)
{
  std::string body = "Email Confirmation Successful: \n";
  body += "Your email address : " + email + "is verified successfully.\n";
  act->send(ok::smart_actor::supervisor::emailMutateActor, send_email_atom_v, "", email, "Thank you", body);
}
void cleanContextAndSendResult(auth_logout_actor_int::pointer act, WsEvent const &event, ok::smart_actor::connection::Session const &session, ws_connector_actor_int connectionActor)
{
  auto successMsg = ok::smart_actor::connection::wsMessageBase();
  ok::smart_actor::connection::addSuccess(successMsg, event);
  int max_age{0};  // this will clear cookie
  ok::smart_actor::connection::addJwtCookie(successMsg, {{"sessionKey", ""}, {"database", ""}}, max_age);
  ok::smart_actor::connection::addIsLoggedIn(successMsg, false);
  auto redirectMsg = ok::smart_actor::connection::wsMessageBase();
  ok::smart_actor::connection::addEmptyMember(redirectMsg);
  ok::smart_actor::connection::addRedirection(redirectMsg, "/");
  // send logout to same user on same browser.
  act->send(ok::smart_actor::supervisor::connectedActor, send_to_same_browser_tab_atom_v, session.database, session.sessionKey, successMsg);
  act->send(ok::smart_actor::supervisor::connectedActor, send_to_same_browser_tab_atom_v, session.database, session.sessionKey, redirectMsg);
  ok::smart_actor::connection::Session newSession{};
  act->send(connectionActor, set_context_atom_v, newSession);
}
// clang-format off
template <typename A>
void sendResult(ErrorCode er, A *act, ok::smart_actor::connection::Session const &session, ws_connector_actor_int connectionActor, WsEvent const &event, std::string const &redirectUrl, ok::mutate_schema::Fields const &fields)
{
  // clang-format on
  auto result = ok::smart_actor::connection::wsMessageBase();
  if (isEr(er)) { ok::smart_actor::connection::addFailure(result, event, er, fields); }
  else
  {
    act->send(connectionActor, set_context_atom_v, session);
    ok::smart_actor::connection::addSuccess(result, event);
    ok::smart_actor::connection::addJwtCookie(result, {{"sessionKey", session.sessionKey}, {"database", session.database}}, 60 * 60 * 24);
    ok::smart_actor::connection::addCurrentMember(result, session.database, session.memberKey);
    ok::smart_actor::connection::addIsLoggedIn(result, true);
    ok::smart_actor::connection::addRedirection(result, redirectUrl);
  }
  act->send(connectionActor, caf::forward_atom_v, result);
}
}  // namespace impl
}  // namespace auth

}  // namespace ok::smart_actor
