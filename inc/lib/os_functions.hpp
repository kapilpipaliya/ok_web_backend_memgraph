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
void watchFile(std::string const &pathname, std::function<void()> func) noexcept;
jsoncons::ojson converToJson(mg::Value);

namespace impl
{
void handleEvents(int fileDescriptor, int uniqueWatchDescriptor, std::function<void()> func) noexcept;
}
}  // namespace utils::file
}  // namespace ok
