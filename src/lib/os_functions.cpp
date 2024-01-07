#include "lib/os_functions.hpp"
#include <drogon/drogon.h>
#include <unistd.h>
#include <poll.h>
#include <cstdlib>
#include <trantor/utils/Logger.h>
#include <functional>
#include <filesystem>
namespace ok
{
namespace utils::file
{
void assertDirectoryExist(const std::vector<std::string> &paths){
  for(const auto &path: paths){
    if (!(std::filesystem::exists(path) && std::filesystem::is_directory(path)))
    {
      LOG_DEBUG << "path directory not exist: " << path;
      LOG_DEBUG << "current path is: " << std::filesystem::current_path().string();
      exit(1);
    }
  }
}
namespace impl
{
}
}
}