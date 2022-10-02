#pragma once
#include <velocypack/Slice.h>
#include <memory>
#include "alias.hpp"
#include "db/fuerteclient.hpp"
#include "db/db_functions.hpp"
#include "utils/ErrorConstants.hpp"
#include <drogon/drogon.h>
namespace ok::smart_actor
{
namespace connection
{
inline std::string serverUrl;
inline std::string drogonRoot;
inline std::unique_ptr<arangodb::fuerte::v1::Response> response;
inline VPackSlice globalFormSlice;
inline VPackSlice printSlice;
inline VPackSlice serverSlice;
inline VPackSlice smtpSlice;
inline VPackSlice testSlice;
inline void fetch() noexcept
{
  if (auto [er, myResp] = ok::db::getSchema("global", "schema", "global_setting", ".form"); ok::isEr(er))
  {
    // error
  }
  else
  {
    auto s = myResp->slices().front().get("result");
    if (s.length() == 1)
    {
      globalFormSlice = s[0];
      printSlice = s[0].get("print");
      serverSlice = s[0].get("server");
      smtpSlice = s[0].get("smtp");
      testSlice = s[0].get("test");
      response = std::move(myResp);
    }
  }
}
inline bool getPrintVal(std::string const &member) noexcept
{
  if (!printSlice.isNone() && printSlice.hasKey(member) && printSlice.get(member).isBool()) return printSlice.get(member).getBool();
  return false;
}
inline bool getServerVal(std::string const &member) noexcept
{
  if (!serverSlice.isNone() && serverSlice.hasKey(member) && serverSlice.get(member).isBool()) return serverSlice.get(member).getBool();
  return false;
}
inline VPackSlice getSmtpVal(std::string const &member) noexcept
{
  if (!smtpSlice.isNone() && smtpSlice.hasKey(member)) return smtpSlice.get(member);
  return VPackSlice();
}
inline VPackSlice getTestVal(std::string const &member) noexcept
{
  if (!testSlice.isNone() && testSlice.hasKey(member)) return testSlice.get(member);
  return VPackSlice();
}
inline void setGlobalVariables()
{
  auto customConfig = drogon::app().getCustomConfig();
  ok::smart_actor::connection::serverUrl = customConfig["server_url"].asString();
  ok::smart_actor::connection::drogonRoot = customConfig["drogon_root"].asString();
  ok::InitializeError();
}
}  // namespace connection
}  // namespace ok::smart_actor
