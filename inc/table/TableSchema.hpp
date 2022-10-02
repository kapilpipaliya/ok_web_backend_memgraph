#pragma once
#include "Column.hpp"
#include "mutate/Relation.hpp"
#include "utils/json_functions.hpp"
#include "parallel_hashmap/phmap.h"
namespace ok
{
enum class ErrorCode;
namespace table_schema
{
struct Filter
{
  std::string selector;
  std::string prefix;
  std::string op;
  std::string val1;
  std::string val2;
};
using Columns = phmap::parallel_flat_hash_map<ID, Column>;
using WhereSmallPart = std::pair<ID, Filter>;
using WherePart = std::vector<WhereSmallPart>;
using TableWhere = std::vector<WherePart>;
using Sort = std::vector<SortedColumn>;
using WrapperFilters = std::map<std::string, std::pair<AQLQuery, AQLQuery>>;
using ColumnOrder = std::vector<ID>;
// \todo make exact filter working(==). (speed up) query time.
std::tuple<ErrorCode, Collection, mutate_schema::Relation, Columns, ColumnOrder, WrapperFilters, Sort, AQLQuery, AQLQuery, bool, jsoncons::ojson> initialize(
    jsoncons::ojson const &args, VPackSlice const &schemaJson,  Database const &database, DocumentKey const &memberKey, bool isPermission, bool doProcessFilters = true);
SchemaKey getKeyFromWhere(TableWhere &where) noexcept;
jsoncons::ojson getPaginationArray(const Pagination &pagination) noexcept;
bool getUserTreeOption(mutate_schema::Relation const &tree, jsoncons::ojson const &args) noexcept;
inline bool isTreeRoot(ok::mutate_schema::Relation const &tree, jsoncons::ojson const &args) noexcept { return !tree.edge.empty() && jsoncons::getStringKey(args, "parent").empty(); }
int getColumnIndex(Columns const &columns, const ID &id);
std::string to_string(Filter const &filter) noexcept;
}  // namespace table_schema
}  // namespace ok
