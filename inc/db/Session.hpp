#pragma once
#include "alias.hpp"
namespace ok::smart_actor
{
namespace connection
{
struct Session
{
  VertexId memberKey = -1;
  std::string subDomain;
  int mg_port = 0;
};
inline std::string to_string(ok::smart_actor::connection::Session const &session) noexcept { return "{sessionKey: }"; }
}  // namespace connection
}  // namespace ok::smart_actor
