#include "actor_system/GraphStateActor.hpp"
namespace ok::smart_actor
{
namespace supervisor
{
graph_state_actor_int::behavior_type GraphStateActor(graph_state_actor_int::stateful_pointer<GraphState> self)
{
  return {[=](erase_atom, std::string, std::string, std::string) { return true; }, [=](erase_atom, std::string) { return true; }, [=](caf::close_atom) {}};
}
namespace impl
{
}
}  // namespace supervisor
}  // namespace ok::smart_actor
