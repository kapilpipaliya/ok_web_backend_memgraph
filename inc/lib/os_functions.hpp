#pragma once
#include <string>
#include <functional>
#include <trantor/utils/Logger.h>
#include "jsoncons/json.hpp"
#include "mgclient.hpp"
namespace ok
{
namespace utils::file
{
void assertDirectoryExist(const std::vector<std::string> &paths);
namespace impl
{
}
}  // namespace utils::file
}  // namespace ok
