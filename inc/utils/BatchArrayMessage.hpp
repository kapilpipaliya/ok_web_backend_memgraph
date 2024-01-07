#pragma once
#include "alias.hpp"

namespace ok::smart_actor::connection
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
inline jsoncons::ojson addSuccess(jsoncons::ojson &&array, WsEvent const &event) noexcept { return addSuccess(array, event); };
jsoncons::ojson addFailure(jsoncons::ojson &array, WsEvent const &event, std::string const description) noexcept;
jsoncons::ojson addCurrentMember(jsoncons::ojson &array, VertexId const &memberKey, int mgPort) noexcept;
jsoncons::ojson addEmptyMember(jsoncons::ojson &array) noexcept;
jsoncons::ojson addJwtCookie(jsoncons::ojson &array, std::map<std::string, std::string> const &cookie, long maxAge) noexcept;
jsoncons::ojson addSimpleCookie(jsoncons::ojson &array, std::map<std::string, std::string> const &cookie, long maxAge) noexcept;
jsoncons::ojson addLogout(jsoncons::ojson &array) noexcept;
jsoncons::ojson addNotification(jsoncons::ojson &array, NotificationType const type, int const timeout = 3000) noexcept;
} // namespace ok::smart_actor::connection

