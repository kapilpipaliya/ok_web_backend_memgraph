#pragma once
#include "alias.hpp"
#include "Relation.hpp"
#include <cstdint>
#include <magic_enum.hpp>
namespace ok
{
namespace mutate_schema
{
enum class FieldType : uint8_t
{
  uninitialized = 0,
  button = 1,
  checkbox,
  checkboxes,
  color,
  date,
  datetime_local,
  email,
  file,
  image,
  month,
  number,
  password,
  radio,
  range,
  reset,
  search,
  submit,
  tel,
  text,
  time,
  url,
  week,
  textarea,
  select,
  jsonEditor,
  internal_true_edge,
  multi_select,
  multi_select_hidden,
  text_array,
  multi_select_bool_properties,
  flatPicker,
  WYSIWYG,
  serial,
  codemirror,
  save_time,
  inserted,
  updated,
  dropzone,
  dateRange,
  emoji,
  mindMap,
  mapCountries,
  uuid,
  fieldSet,
  tab,
  subdomain,
  chatInput
};
inline std::string to_string(FieldType const &ft) noexcept
{
  auto t = magic_enum::enum_name(ft);
  return std::string{t};
}
inline std::ostream &operator<<(std::ostream &out, FieldType const &ft)
{
  out << to_string(ft);
  return out;
}
struct Field
{
  FieldType type{FieldType::text};
  bool required{false};
  bool disabled{false};
  bool isVisible{true};
  bool backEndOnly{false};
  bool ignoreMssingFieldWithValue{false};
  ID id;
  Selector selector;
  Label label;
  Description description;
  jsoncons::ojson defaultValue;
  jsoncons::ojson props;
  Relation relation;
  ErrorMsg error;
  std::string valuePath;
  jsoncons::ojson value;
  jsoncons::ojson config;
  std::string group;
  // placeholder
  // related
  std::string rules;
  // extra
  // options
};
inline std::string to_string(Field const &field) noexcept
{
  std::string ret;
  ret += "Field Type: " + to_string(field.type) + "\n";
  ret += "Required: " + std::string(field.required ? "True" : "False") + "\n";
  ret += "Disabled: " + std::string(field.disabled ? "True" : "False") + "\n";
  ret += "IsVisible: " + std::string(field.isVisible ? "True" : "False") + "\n";
  ret += "BackEndOnly: " + std::string(field.backEndOnly ? "True" : "False") + "\n";
  ret += "IgnoreMssingFieldWithValue: " + std::string(field.ignoreMssingFieldWithValue ? "True" : "False") + "\n";
  ret += "ID: " + field.id + "\n";
  ret += "Selector: " + field.selector + "\n";
  ret += "Label: " + field.label + "\n";
  ret += "Description: " + field.description + "\n";
  ret += "DefaultValue: " + field.defaultValue.as_string() + "\n";
  ret += "Props: " + field.props.as_string() + "\n";
  ret += "Relation: " + to_string(field.relation) + "\n";
  ret += "Error: " + field.error + "\n";
  ret += "ValuePath: " + field.valuePath + "\n";
  ret += "Value: " + field.value.as_string() + "\n";
  ret += "Config: " + field.config.as_string() + "\n";
  ret += "Group: " + field.group + "\n";
  ret += "Rules:" + field.rules + "\n";
  return ret;
}
}  // namespace mutate_schema
}  // namespace ok
