#include "actor_system/SyncActor.hpp"
#include <magic_enum.hpp>
namespace ok::smart_actor
{
namespace supervisor
{
sync_actor_int::behavior_type SyncActor(SyncActorPointer self)
{
  self->set_error_handler(
      [](caf::error &err)
      {
        LOG_DEBUG << "Sync Actor Error :";
        LOG_DEBUG << ok::smart_actor::supervisor::getReasonString(err);
      });
  self->set_down_handler(
      [](caf::scheduled_actor *act, caf::down_msg &msg) noexcept
      {
        LOG_DEBUG << "Monitored Actor Error Down Error :" << act->name();
        LOG_DEBUG << ok::smart_actor::supervisor::getReasonString(msg.reason);
      });
  // If self exception error occur: server freeze.
  self->set_exception_handler(
      [](caf::scheduled_actor *, std::exception_ptr &eptr) noexcept -> caf::error
      {
        try
        {
          if (eptr) { std::rethrow_exception(eptr); }
        }
        catch (std::exception const &e)
        {
          LOG_DEBUG << "Sync Actor Exception Error : " << e.what();
        }
        return caf::make_error(caf::pec::success);  // self will not resume actor.
      });
  self->set_default_handler(
      [](caf::scheduled_actor *ptr, caf::message &x) noexcept -> caf::skippable_result
      {
        LOG_DEBUG << "unexpected message, I will Quit";
        LOG_DEBUG << "*** unexpected message [id: " << ptr->id() << ", name: " << ptr->name() << "]: " << caf::deep_to_string(x);
        return caf::message{};
      });
  return {[=](create_atom, jsoncons::ojson request) {}, [=](set_atom, jsoncons::ojson request) {}, [=](remove_atom, jsoncons::ojson request) {}, [=](shutdown_atom) { self->unbecome(); }};
}
}  // namespace supervisor
}  // namespace ok::smart_actor
