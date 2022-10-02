#pragma once
#include <string>
#include <trantor/utils/Logger.h>
#include <filesystem>
#include <chrono>
#include <filesystem>
#include <iostream>
#include "date/date.h"
#include <thread>
#include <chrono>
// #include "date/tz.h"
//#include "absl/time/time.h"
// https://abseil.io/docs/cpp/guides/time
namespace ok::utils::time
{
// https://github.com/HowardHinnant/date/issues/407#issuecomment-445380040
// other reads:
// clock epochs
// https://github.com/HowardHinnant/date/issues/129
// https://github.com/HowardHinnant/date/issues/406
// time_since_epoch
// https://en.cppreference.com/w/cpp/chrono/time_point/time_since_epoch
inline unsigned long getEpochMilliseconds() noexcept
{
  std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
  // using date::operator<<;
  // LOG_DEBUG << t << '\n'; // 2019-11-17 15:49:25.451827963
  // LOG_DEBUG << date::floor<std::chrono::seconds>(t.time_since_epoch()) << '\n';
  return date::floor<std::chrono::milliseconds>(t.time_since_epoch()).count();
}
inline unsigned long long file_time_to_epoch(std::string const filePath) noexcept
{
  /*auto p = std::filesystem::path(filePath);
  auto lw = std::filesystem::last_write_time(p);
  auto epoch = lw.time_since_epoch();
  // This not work because file_system_time is not convertable to system_clock
  yet
  // also this is not epoch of time_point like above function
  return std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
  */
  // https://stackoverflow.com/questions/40504281/c-how-to-check-the-last-modified-time-of-a-file
  auto p = std::filesystem::path(filePath);
  auto t = std::filesystem::last_write_time(p);
  return date::floor<std::chrono::milliseconds>(t.time_since_epoch()).count();
}
inline void waitSeconds(int seconds){
  std::this_thread::sleep_for (std::chrono::seconds(5));
}
namespace impl
{
// https://github.com/HowardHinnant/date/wiki/Examples-and-Recipes#localtime
// inline void printLocalTime() noexcept { LOG_DEBUG << date::zoned_time{date::current_zone(), std::chrono::system_clock::now()}; }
// from chaiscript
inline double now()
{
  using namespace std::chrono;
  auto now = high_resolution_clock::now();
  return duration_cast<duration<double>>(now.time_since_epoch()).count();
}
}  // namespace impl
}  // namespace ok::utils::time
// date::remote_download("2019c");
// date::remote_install("2019c");
// auto &a = date::get_tzdb();
