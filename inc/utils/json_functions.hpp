#pragma once
#include <jsoncons/json.hpp>
#include "alias.hpp"
#include <trantor/utils/Logger.h>
namespace jsoncons
{
inline bool ObjectMemberIsString(WsArguments const &args, std::string const &member) noexcept { return args.is_object() && args.contains(member) && args[member].is_string(); }
inline bool ObjectMemberIsBool(WsArguments const &args, std::string const &member) noexcept { return args.is_object() && args.contains(member) && args[member].is_bool(); }
inline bool ObjectMemberBoolVal(WsArguments const &args, std::string const &member) noexcept
{
  if (args.is_object() && args.contains(member) && args[member].is_bool()) return args[member].as_bool();
  else
    return false;
}
inline bool ObjectMemberIsArray(WsArguments const &args, std::string const &member) noexcept { return args.is_object() && args.contains(member) && args[member].is_array(); }
inline bool ObjectMemberIsObject(WsArguments const &args, std::string const &member) noexcept { return args.is_object() && args.contains(member) && args[member].is_object(); }
inline bool ObjectMemberIsInteger(WsArguments const &args, std::string const &member) noexcept { return args.is_object() && args.contains(member) && args[member].is_int64(); }
inline bool ArrayPosIsString(WsArguments const &args, const unsigned long pos) noexcept { return args.is_array() && args.size() > pos && args[pos].is_string(); }
inline bool ArrayPosIsArray(WsArguments const &args, const unsigned long pos) noexcept { return args.is_array() && args.size() > pos && args[pos].is_array(); }
inline bool ArrayPosIsInteger(WsArguments const &args, const unsigned long pos) noexcept { return args.is_array() && args.size() > pos && args[pos].is_int64(); }
inline bool ArrayPosIsObject(WsArguments const &args, const unsigned long pos) noexcept { return args.is_array() && args.size() > pos && args[pos].is_object(); }
inline bool ArrayPosArrayIndexIsString(WsArguments const &args, const unsigned long pos, const unsigned long pos2) noexcept
{
  return args.is_array() && args.size() > pos && ArrayPosIsString(args[pos], pos2);
}
inline bool ArrayPosObjectMemberIsBool(WsArguments const &args, const unsigned long pos, std::string const &member) noexcept
{
  return args.is_array() && args.size() > pos && ObjectMemberIsBool(args[pos], member);
}
inline bool ArrayPosObjectMemberIsInteger(WsArguments const &args, const unsigned long pos, std::string const &member) noexcept
{
  return args.is_array() && args.size() > pos && ObjectMemberIsInteger(args[pos], member);
}
inline bool ArrayPosObjectMemberIsString(WsArguments const &args, const unsigned long pos, std::string const &member) noexcept
{
  return args.is_array() && args.size() > pos && ObjectMemberIsString(args[pos], member);
}
inline bool ArrayPosObjectMemberIsArray(WsArguments const &args, const unsigned long pos, std::string const &member) noexcept
{
  return args.is_array() && args.size() > pos && ObjectMemberIsArray(args[pos], member);
}
inline bool getBoolOption(jsoncons::ojson const &json, const std::string &option, bool defaultValue = false) noexcept
{
  if (jsoncons::ObjectMemberIsBool(json, option)) return json[option].as_bool();
  return defaultValue;
}
inline bool isEmptyArray(jsoncons::ojson const &json) { return json.is_array() && json.size() == 0; }
inline bool isEmptyObject(jsoncons::ojson const &json) { return json.is_object() && json.size() == 0; }
inline bool isArrayOfString(jsoncons::ojson const &json)
{
  if (json.is_array())
  {
    auto isValid = true;
    for (std::size_t i = 0; i < json.size(); ++i)
    {
      if (json[i].is_string()) continue;
      else
      {
        isValid = false;
        break;
      }
    }
    if (isValid) return true;
  }
  return false;
}
inline std::string getStringKey(jsoncons::ojson const &json, std::string const &key) noexcept
{
  if (jsoncons::ObjectMemberIsString(json, key)) return json[key].as_string();
  return "";
}
inline std::tuple<std::string, std::string, bool> getFirstStringOption(jsoncons::ojson const &options, jsoncons::ojson const &json)
{
  bool required{false};
  for (auto const &option : options.array_range())
  {
    if (option.is_array())
    {
      if (jsoncons::ArrayPosIsString(option, 0) && json.contains(option[0].as_string_view()))
      {
        required = required || false;
        return {option[0].as_string(), json[option[0].as_string_view()].as_string(), required};
      }
    }
    else if (option.is_string())
    {
      required = required || true;
      if (json.contains(option.as_string_view())) { return {option.as_string(), json[option.as_string_view()].as_string(), required}; }
    }
  }
  return {"", "", required};
}
inline std::string to_string(jsoncons::ojson const &json) noexcept { return json.to_string(); }
};  // namespace jsoncons
