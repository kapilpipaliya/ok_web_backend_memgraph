#pragma once
#include "alias.hpp"

namespace ok
{
namespace smart_actor::connection
{
struct Member;
enum class NotificationType : uint8_t
{
  default_,
  info,
  success,
  warning,
  danger
};
enum class Success
{
  FAIL,
  SUCCESS
};
jsoncons::ojson wsMessageBase() noexcept;
jsoncons::ojson addEventAndJson(jsoncons::ojson &array, WsEvent const &event, WsArguments const &args) noexcept;
jsoncons::ojson addEventAndTwoJsonArgs(jsoncons::ojson &array, WsEvent const &event, WsArguments const &args1, WsArguments const &args2) noexcept;
jsoncons::ojson addSuccess(jsoncons::ojson &array, WsEvent const &event) noexcept;
jsoncons::ojson addIsLoggedIn(jsoncons::ojson &array, bool v) noexcept;
inline jsoncons::ojson addSuccess(jsoncons::ojson &&array, WsEvent const &event) noexcept { return addSuccess(array, event); };
jsoncons::ojson addFailure(jsoncons::ojson &array, WsEvent const &event, std::string const error, std::string const description) noexcept;
jsoncons::ojson addCurrentMember(jsoncons::ojson &array, VertexId const &memberKey) noexcept;
jsoncons::ojson addEmptyMember(jsoncons::ojson &array) noexcept;
jsoncons::ojson addRedirection(jsoncons::ojson &array, std::string const &link, std::string const &msg = "", int const timeout = 0) noexcept;
jsoncons::ojson addJwtCookie(jsoncons::ojson &array, std::map<std::string, std::string> const &cookie, long maxAge) noexcept;
jsoncons::ojson addSimpleCookie(jsoncons::ojson &array, std::map<std::string, std::string> const &cookie, long maxAge) noexcept;
jsoncons::ojson addLogout(jsoncons::ojson &array) noexcept;
jsoncons::ojson addNotification(jsoncons::ojson &array, NotificationType const type, int const timeout = 3000) noexcept;
}  // namespace smart_actor::connection
}  // namespace ok
