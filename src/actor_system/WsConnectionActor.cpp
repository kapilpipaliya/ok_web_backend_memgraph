#include "actor_system/WsConnectionActor.hpp"
#include "db/Session.hpp"
#include "utils/BatchArrayMessage.hpp"
#include <jsoncons_ext/msgpack/msgpack.hpp>
#include "utils/ErrorConstants.hpp"
#include <jsoncons/json.hpp>
#include <magic_enum.hpp>
#include <utility>
#include "actor_system/Routes.hpp"
#include "caf/all.hpp"
#include "pystring.hpp"
#include "utils/json_functions.hpp"
#include "utils/os_functions.hpp"
namespace ok::smart_actor
{
namespace connection
{
ws_connector_actor_int::behavior_type WsControllerActor(ws_connector_actor_int::stateful_pointer<ws_controller_state> self)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("Mutate Connection Actor"));
  self->set_down_handler(
      [=](caf::scheduled_actor *, caf::down_msg &msg)
      {
        LOG_DEBUG << "Monitored Actor(by connection) Error Down Error :";
        LOG_DEBUG << ok::smart_actor::supervisor::getReasonString(msg.reason);
      });
  return {
      // Fix can't use this message because cyclic dependency in types in CAF.hpp
      [=](drogon::WebSocketConnectionPtr const &wsConnPtr, std::string const &jwtEncoded, std::string const &firstSubDomain)
      {
        self->state.wsConnPtr = wsConnPtr;
        saveNewConnection(self, self->state, jwtEncoded, firstSubDomain);
      },
      [=](get_session_atom) { return self->state.session; },
      [=](set_context_atom, ok::smart_actor::connection::Session const &s)
      {
        self->send(ok::smart_actor::supervisor::connectedActor, remove_atom_v, self->state.session, self);
        self->send(ok::smart_actor::supervisor::connectedActor, caf::add_atom_v, s, self);
        self->state.session = std::move(s);
      },
      [=](std::string const &message, drogon::WebSocketMessageType const &type)
      {
        if (auto [er, result] = ok::smart_actor::connection::processEvent(message, self->state.session, self->state.subDomain, self, false); ok::isEr(er))
        {
          sendJson(self->state.wsConnPtr, ok::errno_string(er));
        }
        else if (!result.is_null() && result.is_array() && !result.empty())
        {
          sendJson(self->state.wsConnPtr, result);
        }
      },
      [=](caf::forward_atom, jsoncons::ojson const &result) { sendJson(self->state.wsConnPtr, result); },
      [=](dispatch_atom, jsoncons::ojson &result_)
      {
        if (!self->state.wsConnPtr)
        {
          LOG_FATAL << "this must not happened on " << self->name();
          exit(1);
        }
        if (auto [er, result] = ok::smart_actor::connection::processEvent(result_, self->state.session, self->state.subDomain, self, true); ok::isEr(er))
        {
          sendJson(self->state.wsConnPtr, ok::errno_string(er));
        }
        else if (!result.is_null() && result.is_array() && !result.empty())
        {
          sendJson(self->state.wsConnPtr, result);
        }
      },
      [=](conn_exit_atom)
      {
        LOG_DEBUG << "exiting " << self->name();
        for (auto &t : self->state.subscriptions) { self->send(t, conn_exit_atom_v, self); }
        for (auto &t : self->state.chat_subscriptions) { self->send(t, conn_exit_atom_v, self); }
        self->send(ok::smart_actor::supervisor::connectedActor, remove_atom_v, self->state.session, self);
        self->unbecome();
      },
  };
}
void sendJson(drogon::WebSocketConnectionPtr wsConnPtr, jsoncons::ojson const &json) noexcept
{
  if (ok::smart_actor::connection::getServerVal("msgpack"))
  {
    // https://stackoverflow.com/questions/658913/c-style-cast-from-unsigned-char-to-const-char
    std::vector<uint8_t> v;
    jsoncons::msgpack::encode_msgpack(json, v);
    wsConnPtr->send(reinterpret_cast<char const *>(v.data()), v.size(), drogon::WebSocketMessageType::Binary);
  }
  else
  {
    std::string dump;
    json.dump(dump);
    wsConnPtr->send(dump, drogon::WebSocketMessageType::Text);
  }
}
void saveNewConnection(ws_connector_actor_int::stateful_pointer<ws_controller_state> self, ws_controller_state &state, std::string const &jwtEncoded, std::string const &firstSubDomain)
{
  state.subDomain = firstSubDomain;
  ok::db::authenticateAndSaveSession(jwtEncoded, state.session, state.subDomain);
  /*if (isTest) {
    self->send(ok::smart_actor::supervisor::connectedActor, caf::add_atom_v, state.session, self);
    return; }*/
  auto memberMsg = ok::smart_actor::connection::wsMessageBase();
  if (!state.session.memberKey.empty())
  {
    ok::smart_actor::connection::addIsLoggedIn(memberMsg, true);
    ok::smart_actor::connection::addCurrentMember(memberMsg, state.session.database, state.session.memberKey);
  }
  else
  {
    ok::smart_actor::connection::addIsLoggedIn(memberMsg, false);
  }
  sendJson(state.wsConnPtr, memberMsg);
  self->send(ok::smart_actor::supervisor::connectedActor, caf::add_atom_v, state.session, self);
}
std::tuple<ErrorCode, jsoncons::ojson> processEvent(jsoncons::ojson const &valin,
                                                    Session const &session,
                                                    std::string const &subDomain,
                                                    ws_connector_actor_int::stateful_pointer<ok::smart_actor::connection::ws_controller_state> currentActor,
                                                    bool isDispatching)
{
  jsoncons::ojson resultMsg = ok::smart_actor::connection::wsMessageBase();
  if (auto er = impl::checkSchema(valin); ok::isEr(er)) { return {er, resultMsg}; }
  unsigned int eventNo{0};
  while (eventNo < valin.size())
  {
    auto e = impl::start(valin, eventNo);
    if (routeFunctions.contains(e)) routeFunctions[e](valin, eventNo, e, session, resultMsg, currentActor, subDomain, isDispatching);
    eventNo++;
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, resultMsg};
}
std::tuple<ErrorCode, jsoncons::ojson> processEvent(
    std::string message, Session const &session, std::string const &subDomain, ws_connector_actor_int::stateful_pointer<ws_controller_state> currentActor, bool isDispatching)
{
  auto [er, valin] = impl::preparing(message);
  if (ok::isEr(er)) { return {er, {}}; }
  return processEvent(valin, session, subDomain, currentActor, isDispatching);
}
namespace impl
{
std::tuple<ErrorCode, jsoncons::ojson> preparing(std::string message, bool isDispatching)
{
  // resultMsg = ok::smart_actor::connection::wsMessageBase();
  if (isDispatching) return {ok::ErrorCode::ERROR_NO_ERROR, {}};
  try
  {
    auto valin = jsoncons::ojson::parse(message);
    return {ok::ErrorCode::ERROR_NO_ERROR, valin};
  }
  catch (jsoncons::ser_error const &e)
  {
    return {ok::ErrorCode::ERROR_BAD_PARAMETER, {}};
  }
}
ErrorCode checkSchema(jsoncons::ojson const &valin)
{
  bool is_schema_ok = true;
  if (valin.is_array())
  {
    for (auto const &a : valin.array_range())
    {
      if (!jsoncons::ArrayPosIsArray(a, 0)) { is_schema_ok = false; }
    }
  }
  else
  {
    is_schema_ok = false;
  }
  if (!is_schema_ok)
  {
    return ok::ErrorCode::ERROR_BAD_PARAMETER;  // ("Event Json Must be array.");
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
std::string start(jsoncons::ojson const &valin, unsigned int eventNo, bool isDispatching)
{
  if (valin.is_array() && !valin.empty() && valin[eventNo].size() == 2)
  {
    auto &event = valin[eventNo][0];
    if (jsoncons::ArrayPosIsString(event, 0) && jsoncons::ArrayPosIsString(event, 1)) { return event[1].as<std::string>(); }
    LOG_DEBUG << "Event id must be string" << event.to_string();
  }
  else
  {
    LOG_DEBUG << "Event Array must have 2 elements only: " << valin.to_string() << (isDispatching ? " on dispatching event" : "");
    // throwing stops actor working...
  }
  return "";
}
}  // namespace impl
}  // namespace connection
}  // namespace ok::smart_actor
