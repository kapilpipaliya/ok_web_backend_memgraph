#pragma once
#include <cstdint>
namespace ok::smart_actor
{
namespace connection
{
enum class GetEventType : uint8_t
{
  get = 1,
  subscribe,
  unsubscribe
};
enum class MutateEventType : uint8_t
{
  create=11,
  update,
  replace,
  remove,
  batchUpdate,
  batchRemove
};
}  // namespace connection
}  // namespace ok::smart_actor
