#include "actor_system/MainActor.hpp"
#include "actor_system/WsConnectionActor.hpp"
#include <magic_enum.hpp>
namespace ok::smart_actor
{
namespace supervisor
{
main_actor_int::behavior_type MainActor(MainActorPointer self)
{
    self->set_error_handler([](caf::error &err) {
        LOG_DEBUG << "Main Actor Error :";
        LOG_DEBUG << ok::smart_actor::supervisor::getReasonString(err);
    });
    self->set_down_handler([](caf::scheduled_actor *act,
                              caf::down_msg &msg) noexcept {
        LOG_DEBUG << "Monitored Actor Error Down Error :" << act->name();
        LOG_DEBUG << ok::smart_actor::supervisor::getReasonString(msg.reason);
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
                LOG_DEBUG << "Main Actor Exception Error : " << e.what();
            }
            return caf::make_error(
                caf::pec::success);  // self will not resume actor.
        });
    self->set_default_handler(
        [](caf::scheduled_actor *ptr,
           caf::message &x) noexcept -> caf::skippable_result {
            LOG_DEBUG << "unexpected message, I will Quit";
            LOG_DEBUG << "*** unexpected message [id: " << ptr->id()
                      << ", name: " << ptr->name()
                      << "]: " << caf::deep_to_string(x);
            return caf::message{};
        });
    return {[=](spawn_and_monitor_atom) {
                impl::spawnAndMonitorSuperActor(self);
            },
            [=](save_old_wsconnptr_atom,
                drogon::WebSocketConnectionPtr const &wsConnPtr,
                std::string const &jwtEncoded,
                std::string const &firstSubDomain) {
                impl::spanAndSaveConnectionActor(self,
                                                 wsConnPtr,
                                                 jwtEncoded,
                                                 firstSubDomain);
            },
            [=](pass_to_ws_connection_atom,
                drogon::WebSocketConnectionPtr const &wsConnPtr,
                std::string &message,
                drogon::WebSocketMessageType const &type) {
                impl::passToWsControllerActor(self,
                                              wsConnPtr,
                                              std::move(message),
                                              type);
            },
            [=](conn_exit_old_atom,
                drogon::WebSocketConnectionPtr const &wsConnPtr) {
                impl::connectionExit(self, wsConnPtr);
            },
            [=](shutdown_atom) { impl::shutdownNow(self); }};
}
namespace impl
{
void spawnAndMonitorSuperActor(MainActorPointer act) noexcept
{
    // ok::smart_actor::supervisor::connectedActor =
    // act->spawn(ok::smart_actor::connection::LoginStatusActor);
    // act->monitor(ok::smart_actor::supervisor::connectedActor);
    // ok::smart_actor::supervisor::sessionCleanActor =
    // act->spawn(ok::smart_actor::connection::SessionCleanActor);
    // act->send(ok::smart_actor::supervisor::sessionCleanActor,
    // session_clean_atom_v);  // self will send message when it down
    // act->monitor(ok::smart_actor::supervisor::sessionCleanActor);
    // ok::smart_actor::supervisor::databaseHealthCheckActor =
    // act->spawn(ok::smart_actor::connection::DBHealthCheckActor);
    // act->send(ok::smart_actor::supervisor::databaseHealthCheckActor,
    // db_health_check_atom_v);
    // act->monitor(ok::smart_actor::supervisor::databaseHealthCheckActor);
    // List:
    // Mutate:
}
void spanAndSaveConnectionActor(MainActorPointer act,
                                drogon::WebSocketConnectionPtr const &wsConnPtr,
                                std::string const &jwtEncoded,
                                std::string const &firstSubDomain) noexcept
{
    ws_connector_actor_int connectionActor =
        act->spawn(ok::smart_actor::connection::WsControllerActor);
    act->monitor(connectionActor);  // self will send message when it down
    act->send(connectionActor, wsConnPtr, jwtEncoded, firstSubDomain);
    act->state.oldActorMap.insert({wsConnPtr, std::move(connectionActor)});
}
void passToWsControllerActor(MainActorPointer act,
                             drogon::WebSocketConnectionPtr const &wsConnPtr,
                             std::string &&message,
                             drogon::WebSocketMessageType const &type) noexcept
{
    if (auto const &it = act->state.oldActorMap.find(wsConnPtr);
        it == std::end(act->state.oldActorMap))
    {
        LOG_DEBUG << "This should never happen when passing message to the "
                     "connection.";
        exit(1);
    }
    else
        act->send(it->second, std::move(message), type);
}
void connectionExit(MainActorPointer act,
                    drogon::WebSocketConnectionPtr const &wsConnPtr)
{
    if (auto const &it = act->state.oldActorMap.find(wsConnPtr);
        it == std::end(act->state.oldActorMap))
    {
        LOG_ERROR << "Not Possible";
    }
    else
    {
        ws_connector_actor_int connectionActor = it->second;
        act->send<caf::message_priority::high>(connectionActor,
                                               conn_exit_atom_v);
        act->demonitor(connectionActor.address());
        act->send(ok::smart_actor::supervisor::syncActor,
                  conn_exit_atom_v,
                  connectionActor);
        act->state.oldActorMap.erase(it);
    }
}
void shutdownNow(MainActorPointer act) noexcept
{
    /*
      // act->demonitor(ok::smart_actor::supervisor::v8StateActor.address());
      // act->send(ok::smart_actor::supervisor::v8StateActor,
      caf::close_atom_v);
      act->demonitor(ok::smart_actor::supervisor::connectedActor.address());
      act->send(ok::smart_actor::supervisor::connectedActor, conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::sessionCleanActor.address());
      act->send(ok::smart_actor::supervisor::sessionCleanActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::databaseHealthCheckActor.address());
      act->send(ok::smart_actor::supervisor::databaseHealthCheckActor,
      conn_exit_atom_v);
      //
      act->demonitor(ok::smart_actor::supervisor::userListActor.address());
      act->send(ok::smart_actor::supervisor::userListActor, conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::schemaListActor.address());
      act->send(ok::smart_actor::supervisor::schemaListActor, conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::translationListActor.address());
      act->send(ok::smart_actor::supervisor::translationListActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::sessionListActor.address());
      act->send(ok::smart_actor::supervisor::sessionListActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::confirmListActor.address());
      act->send(ok::smart_actor::supervisor::confirmListActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::colorListActor.address());
      act->send(ok::smart_actor::supervisor::colorListActor, conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::permissionListActor.address());
      act->send(ok::smart_actor::supervisor::permissionListActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::roleListActor.address());
      act->send(ok::smart_actor::supervisor::roleListActor, conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::templateListActor.address());
      act->send(ok::smart_actor::supervisor::templateListActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::menuListActor.address());
      act->send(ok::smart_actor::supervisor::menuListActor, conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::nodeListActor.address());
      act->send(ok::smart_actor::supervisor::nodeListActor, conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::edgeListActor.address());
      act->send(ok::smart_actor::supervisor::edgeListActor, conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::attributeListActor.address());
      act->send(ok::smart_actor::supervisor::attributeListActor,
      conn_exit_atom_v);
      //
      act->demonitor(ok::smart_actor::supervisor::emailMutateActor.address());
      act->send(ok::smart_actor::supervisor::emailMutateActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::registerMutateActor.address());
      act->send(ok::smart_actor::supervisor::registerMutateActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::confirmMutateActor.address());
      act->send(ok::smart_actor::supervisor::confirmMutateActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::memberRegisterMutateActor.address());
      act->send(ok::smart_actor::supervisor::memberRegisterMutateActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::logoutMutateActor.address());
      act->send(ok::smart_actor::supervisor::logoutMutateActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::userMutateActor.address());
      act->send(ok::smart_actor::supervisor::userMutateActor, conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::schemaMutateActor.address());
      act->send(ok::smart_actor::supervisor::schemaMutateActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::translationMutateActor.address());
      act->send(ok::smart_actor::supervisor::translationMutateActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::sessionMutateActor.address());
      act->send(ok::smart_actor::supervisor::sessionMutateActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::colorMutateActor.address());
      act->send(ok::smart_actor::supervisor::colorMutateActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::permissionMutateActor.address());
      act->send(ok::smart_actor::supervisor::permissionMutateActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::roleMutateActor.address());
      act->send(ok::smart_actor::supervisor::roleMutateActor, conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::menuMutateActor.address());
      act->send(ok::smart_actor::supervisor::menuMutateActor, conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::nodeMutateActor.address());
      act->send(ok::smart_actor::supervisor::nodeMutateActor, conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::edgeMutateActor.address());
      act->send(ok::smart_actor::supervisor::edgeMutateActor, conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::attributeMutateActor.address());
      act->send(ok::smart_actor::supervisor::attributeMutateActor,
      conn_exit_atom_v);
      act->demonitor(ok::smart_actor::supervisor::templateMutateActor.address());
      act->send(ok::smart_actor::supervisor::templateMutateActor,
      conn_exit_atom_v);
      */
    act->unbecome();
    //
    // act->send(a, conn_exit_atom_v);
    // act->demonitor(a.address());
    LOG_DEBUG << "shutdown main actor";
}
}  // namespace impl
}  // namespace supervisor
}  // namespace ok::smart_actor
