#pragma once
#include "alias.hpp"
namespace ok
{
enum class ErrorCode;
namespace smart_actor::connection
{
enum class MutateEventType : uint8_t;
}
namespace permission
{
/*!
 * \brief if project key is not passed it check for top level permission of member only.
 */
// clang-format off
ErrorCode checkPermission(Database const &database, DocumentKey const &memberKey, DocumentKey const &projectKey, std::string const &category, ok::smart_actor::connection::MutateEventType eventType, bool childPermissionCheck = true, bool adminPermissionCheck = true) noexcept;
ErrorCode checkPermission(Database const &database, DocumentKey memberKey, DocumentKey const &projectKey, std::string const &category, std::string const &property, bool childPermissionCheck = true, bool adminPermissionCheck = true) noexcept;
// clang-format on
namespace impl
{
ErrorCode checkPermissionTopLevel(Database const &database, DocumentKey const &memberKey, std::string const &category, std::string const &property) noexcept;
}
}  // namespace permission
}  // namespace ok
