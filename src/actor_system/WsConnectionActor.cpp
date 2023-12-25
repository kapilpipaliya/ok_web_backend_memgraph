#include "actor_system/WsConnectionActor.hpp"
#include "actor_system/MutationActor.hpp"
#include "db/Session.hpp"
#include "utils/BatchArrayMessage.hpp"
#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <jsoncons/json.hpp>
#include <magic_enum.hpp>
#include <utility>
#include "lib/json_functions.hpp"
#include "db/auth_fns.hpp"
namespace ok::smart_actor
{
namespace connection
{
ws_connector_actor_int::behavior_type WsControllerActor(
    ws_connector_actor_int::stateful_pointer<ws_controller_state> self)
{
    self->set_exception_handler(
        ok::smart_actor::supervisor::default_exception_handler(
            "Mutate Connection Actor"));
    self->set_down_handler([=](caf::scheduled_actor *, caf::down_msg &msg) {
        LOG_ERROR << "Monitored Actor(by connection) Error Down Error :";
        LOG_ERROR << ok::smart_actor::supervisor::getReasonString(msg.reason);
    });
    // MutationActor will use IO
    // and should thus be spawned in a separate thread
    self->state.mutationActor = self->spawn<caf::detached + caf::linked>(
        ok::smart_actor::supervisor::MutationActor);
    return {
        // Fix can't use this message because cyclic dependency in types in
        // CAF.hpp
        [=](drogon::WebSocketConnectionPtr const &wsConnPtr,
            const sync_actor_wrapper& syncActorWrapper,
            std::string const &jwtEncoded,
            std::string const &firstSubDomain) {
            self->state.wsConnPtr = wsConnPtr;
            self->state.syncActor = syncActorWrapper.syncActor;
            impl::saveNewConnection(self, self->state, jwtEncoded, firstSubDomain);
        },
        [=](get_session_atom) { return self->state.session; },
        [=](set_context_atom, ok::smart_actor::connection::Session const &s) {
            // self->send(ok::smart_actor::supervisor::connectedActor,
            // remove_atom_v, self->state.session, self);
            // self->send(ok::smart_actor::supervisor::connectedActor,
            // caf::add_atom_v, s, self);
            self->state.session = std::move(s);
        },
        [=](std::string const &message,
            drogon::WebSocketMessageType const &type) {
            auto [er, valin] = impl::parseJson(message);
            if (er)
                return impl::sendJson(self->state.wsConnPtr, valin);
            if (auto [er, result] = impl::processEvent(
                    valin, self->state.session, self->state.session.subDomain, self);
                er)
                impl::sendJson(self->state.wsConnPtr, result);
            else if (!result.is_null() && result.is_array() && !result.empty())
                impl::sendJson(self->state.wsConnPtr, result);
        },
        [=](caf::forward_atom, jsoncons::ojson const &result) {
            impl::sendJson(self->state.wsConnPtr, result);
        },
        [=](dispatch_atom, jsoncons::ojson &result_) {
            if (!self->state.wsConnPtr)
            {
                LOG_FATAL << "this must not happened on " << self->name();
                exit(1);
            }
            if (auto [er, result] = impl::processEvent(
                    result_, self->state.session, self->state.session.subDomain, self);
                er)
                impl::sendJson(self->state.wsConnPtr, result);
            else if (!result.is_null() && result.is_array() && !result.empty())
                impl::sendJson(self->state.wsConnPtr, result);
        },
        [=](conn_exit_atom) {
            self->send(self->state.mutationActor, shutdown_atom_v);
            //            LOG_DEBUG << "exiting " << self->name();
            self->unbecome();
        },
    };
}


namespace impl
{
void sendJson(drogon::WebSocketConnectionPtr wsConnPtr,
              jsoncons::ojson const &json) noexcept
{
    // LOG_DEBUG << json.to_string();
    // if (ok::smart_actor::connection::getServerVal("msgpack"))
    // {
    //   //
    //   https://stackoverflow.com/questions/658913/c-style-cast-from-unsigned-char-to-const-char
    //   std::vector<uint8_t> v;
    //   jsoncons::msgpack::encode_msgpack(json, v);
    //   wsConnPtr->send(reinterpret_cast<char const *>(v.data()), v.size(),
    //   drogon::WebSocketMessageType::Binary);
    // }
    // else
    // {
    std::string dump;
    json.dump(dump);

    wsConnPtr->send(dump, drogon::WebSocketMessageType::Text);
    // }
}
void saveNewConnection(
    ws_connector_actor_int::stateful_pointer<ws_controller_state> self,
    ws_controller_state &state,
    std::string const &jwtEncoded,
    std::string const &firstSubDomain)
{
    auto memberMsg = ok::smart_actor::connection::wsMessageBase();
    WsEvent event = jsoncons::ojson::array();
    event.push_back("get");
    event.push_back("member");
    event.push_back(0);
    jsoncons::ojson one = jsoncons::ojson::array();
    one.push_back(event);

    auto [memberKey, member] = ok::db::auth::loginJwt(jwtEncoded);
    state.session.memberKey = memberKey;
    state.session.subDomain = firstSubDomain;
    state.session.mg_port = ok::db::auth::getSubDomainPort(firstSubDomain);

    one.push_back(member);
    memberMsg.push_back(one);
    sendJson(state.wsConnPtr, memberMsg);
    // self->send(ok::smart_actor::supervisor::connectedActor, caf::add_atom_v,
    // state.session, self);
}
std::tuple<bool, jsoncons::ojson> processEvent(
    jsoncons::ojson const &valin,
    Session &session,
    std::string const &subDomain,
    ws_connector_actor_int::stateful_pointer<
        ok::smart_actor::connection::ws_controller_state> currentActor)
{
    jsoncons::ojson resultMsg = ok::smart_actor::connection::wsMessageBase();
    if (auto er = impl::checkSchema(valin); er)
        return {er,
                jsoncons::ojson{jsoncons::json_object_arg,
                                {{"error", "json schema validation error"}}}};
    unsigned int eventNo{0};
    while (eventNo < valin.size())
    {
        auto &event = valin[eventNo][0];
        auto args = [&]() {
            if (valin[eventNo].size() > 1)
                return valin[eventNo][1];
            else
                return jsoncons::ojson{};
        }();
        if (jsoncons::ArrayPosIsString(event, 0) &&
            jsoncons::ArrayPosIsString(event, 1))
        {
            auto e = event[1].as<std::string>();
            if (routeFunctions.contains(e))
                routeFunctions[e](
                    event, args, session, resultMsg, currentActor, subDomain);
        }
        else
        {
            LOG_DEBUG << "Event id must be string" << event.to_string();
        }
        eventNo++;
    }
    return {false, resultMsg};
}
std::tuple<bool, jsoncons::ojson> parseJson(std::string message)
{
    try
    {
        return {false, jsoncons::ojson::parse(message)};
    }
    catch (jsoncons::ser_error const &e)
    {
        return {true,
                jsoncons::ojson{jsoncons::json_object_arg,
                                {{"error", "json parse error"}}}};
    }
}
bool checkSchema(jsoncons::ojson const &valin)
{
    bool is_schema_ok = true;
    if (valin.is_array())
    {
        for (auto const &a : valin.array_range())
        {
            if (!jsoncons::ArrayPosIsArray(a, 0))
                is_schema_ok = false;
        }
    }
    else
        is_schema_ok = false;
    return !is_schema_ok;
}
}  // namespace impl
}  // namespace connection
}  // namespace ok::smart_actor
