#include "actor_system/TableActor.hpp"
#include <magic_enum.hpp>
#include "utils/ErrorConstants.hpp"
namespace ok::smart_actor
{
namespace table_actor
{
bool argsSchemaCheck(WsArguments const &args) noexcept
{
  if (!args.is_object())
  {
    LOG_DEBUG << "Table arguments must be an array: " << args.to_string();
    return false;
  }
  return true;
}
}  // namespace table_actor
}  // namespace ok::smart_actor
