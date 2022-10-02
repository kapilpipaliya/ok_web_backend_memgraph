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
  insert=11,
  update,
  replace,
  delete_,
  batchUpdate,
  batchDelete,
  changePosition
};
}  // namespace connection
}  // namespace ok::smart_actor
