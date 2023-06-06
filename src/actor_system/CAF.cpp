#include "actor_system/CAF.hpp"
#include "actor_system/MainActor.hpp"
#include "actor_system/SyncActor.hpp"
namespace ok::smart_actor
{
namespace supervisor
{
void initialiseMainActor(int argc, char *argv[]) noexcept
{
    // Initialize the global type information before anything else.
    // caf::init_global_meta_objects<...>();
    caf::init_global_meta_objects<caf::id_block::okproject>();
    // caf::io::middleman::init_global_meta_objects();
    caf::core::init_global_meta_objects();
    // Create the config.
    cfg = std::make_unique<caf::actor_system_config>();
    // Read CLI options.
    // if (auto err = cfg->parse(0, nullptr))
    if (auto err = cfg->parse(argc, argv))
    // if (auto err = cfg->parse_config_file("caf-application.conf"))
    {
        std::cerr << to_string(err) << std::endl;
    }

    // Return immediately if a help text was printed.
    if (cfg->cli_helptext_printed)
        return;
    actorSystem = std::make_unique<caf::actor_system>(*cfg);
    // self = std::make_unique<caf::scoped_actor>(*actorSystem);
    mainActor = actorSystem->spawn<caf::detached>(
        ok::smart_actor::supervisor::MainActor);
    syncActor = actorSystem->spawn<caf::detached>(
        ok::smart_actor::supervisor::SyncActor);
    // or
    // main_actor = self->spawn<MainActor>();
    CAF_LOG_DEBUG("1initialized caf system");
    CAF_LOG_TRACE("2initialized caf system");
    CAF_LOG_INFO("3initialized caf system");
    std::cerr << "DONE" << std::endl;
}
std::string getReasonString(caf::error &err) noexcept
{
    return caf::to_string(err);
}
using exception_handler =
    std::function<caf::error(caf::scheduled_actor *, std::exception_ptr &)>;
exception_handler default_exception_handler(std::string const &msg)
{
    return [=](caf::scheduled_actor *,
               std::exception_ptr &exception_ptr) -> caf::error {
        try
        {
            if (exception_ptr)
            {
                std::rethrow_exception(exception_ptr);
            }
        }
        catch (std::exception const &e)
        {
            LOG_ERROR << msg;
            LOG_ERROR << "Exception Error : " << e.what();
        }
        return caf::make_error(
            caf::pec::success);  // This will not resume actor.
    };
}
}  // namespace supervisor
}  // namespace ok::smart_actor
