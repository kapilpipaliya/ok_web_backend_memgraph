#include "actor_system/MutationActor.hpp"
#include <magic_enum.hpp>
#include "db/mgclientPool.hpp"
#include "third_party/mgclient/src/mgvalue.h"
#include "utils/BatchArrayMessage.hpp"
#include "lib/json_functions.hpp"
#include "jsoncons/json_encoder2.hpp"
#include "db/mutate_functions.hpp"
#include "db/Session.hpp"
namespace ok::smart_actor
{
namespace supervisor
{

// note: mutation action can save all txns and revert back to any point in time!
// TODO: save all txnIds that are done on mutationActor, so it cant be done
// twice.
// TODO: mutation actor update sync actor.
mutation_actor_int::behavior_type MutationActor(MutationActorPointer self)
{
    self->set_error_handler([](caf::error &err) {
        LOG_ERROR << "Mutation Actor Error :";
        LOG_ERROR << ok::smart_actor::supervisor::getReasonString(err);
    });
    self->set_down_handler([](caf::scheduled_actor *act,
                              caf::down_msg &msg) noexcept {
        LOG_ERROR << "Monitored Actor Error Down Error :" << act->name();
        LOG_ERROR << ok::smart_actor::supervisor::getReasonString(msg.reason);
    });
    // If self exception error occur: server freeze.
    self->set_exception_handler(
        [](caf::scheduled_actor *,
           std::exception_ptr &eptr) noexcept -> caf::error {
            try
            {
                if (eptr)
                {
                    std::rethrow_exception(eptr);
                }
            }
            catch (std::exception const &e)
            {
                LOG_ERROR << "Muration Actor Exception Error : " << e.what();
            }
            return caf::make_error(
                caf::pec::success);  // self will not resume actor.
        });
    self->set_default_handler(
        [](caf::scheduled_actor *ptr,
           caf::message &x) noexcept -> caf::skippable_result {
            LOG_ERROR << "unexpected message, I will Quit";
            LOG_ERROR << "*** unexpected message [id: " << ptr->id()
                      << ", name: " << ptr->name()
                      << "]: " << caf::deep_to_string(x);
            return caf::message{};
        });
    // NOTE: [r:$type] in query doesnt work replace $type with string manually
    return {
        [=](create_atom,
            ok::smart_actor::connection::Session const &session,
            WsArguments const &args,
            ws_connector_actor_int const &connectionActor) {
            auto event = jsoncons::ojson::array();
            event.push_back("post");
            event.push_back("mutate");
            event.push_back(0);


            // create new connection:
            // on transaction error, we need to create a new connection.
            // its better to always create a new connection because after some
            // inactivity connection is lost too.
            auto client = mg::Client::Connect(self->state.params);
            if (!client)
            {
                LOG_ERROR << "Failed to connect MG Server";
                auto responseResult = ok::smart_actor::connection::wsMessageBase();
                return self->send(connectionActor,
                                  caf::forward_atom_v,
                                  ok::smart_actor::connection::addFailure(
                                      responseResult,
                                      event,
                                      "Failed to connect MG Server"));
            }
            self->state.mgClient = std::move(client);

            self->send(connectionActor, caf::forward_atom_v, ok::db::mutate::mutate_data(event, args, self->state.mgClient, ok::smart_actor::connection::Session{}));
        },
        [=](shutdown_atom) { self->unbecome(); }};
}
}  // namespace supervisor
}  // namespace ok::smart_actor
