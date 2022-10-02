#pragma once
#include "alias.hpp"
namespace ok::smart_actor
{
namespace connection
{
struct Session
{
  DocumentKey sessionKey;
  unsigned long sessionEpoch{0};  // to check expired on every connection
  Database database;
  DocumentKey memberKey;
};
inline std::string to_string(ok::smart_actor::connection::Session const &session) noexcept { return "{sessionKey: " + session.sessionKey + " | database: " + session.database + "}"; }
}  // namespace connection
}  // namespace ok::smart_actor
