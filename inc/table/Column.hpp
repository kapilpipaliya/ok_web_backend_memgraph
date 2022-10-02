#pragma once
#include <velocypack/Slice.h>
#include <jsoncons/json.hpp>
#include "alias.hpp"
namespace ok
{
namespace table_schema
{
enum class ColumnDisplayType : uint8_t
{
  UNINITIALIZED,
  Checkbox = 1,
  Number,
  Text,
  Double,
  Date,
  DateTime,
  Url,
  Color
};
struct Column
{
  bool isVisible{true};
  ColumnDisplayType visible_type;
  ID id;
  Label label;
  Selector selector;
  std::string calculated;
  std::string prefix;
  jsoncons::ojson editableOptions;
  jsoncons::ojson props;
  double width{0};
  bool skipFrontEnd{false};
};
std::string to_string(Column const &column) noexcept;
enum SortDirection : uint8_t
{
  None,
  Ascending,
  Descending
};
struct SortedColumn
{
  bool operator==(SortedColumn const &rhs) const { return id == rhs.id && direction == rhs.direction; }
  ID id;
  SortDirection direction;
};
struct Pagination
{
  int limit{0};
  int offset{0};
  int currentPage{0};
};
}  // namespace table_schema
}  // namespace ok
