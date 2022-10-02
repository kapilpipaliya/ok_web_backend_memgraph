#pragma once
#include <unordered_set>
#include "TableSchema.hpp"
namespace ok
{
enum class ErrorCode;
namespace table_schema
{
jsoncons::ojson getJsonHeaderData(Columns const &columns, const ColumnOrder &allIds, Sort const &sort, jsoncons::ojson const &tableProps) noexcept;
std::tuple<ErrorCode> setFilterConfig(jsoncons::ojson const &args, Columns const &columns, TableWhere &where) noexcept;
std::tuple<ErrorCode> setSortConfig(jsoncons::ojson const &args, Columns const &columns, Sort &sort) noexcept;
std::tuple<ErrorCode> setPaginationConfig(jsoncons::ojson const &args, Pagination &pagination) noexcept;
std::tuple<ErrorCode, AQLQuery> setDefaultWhereArray(std::unordered_set<DocumentKey> const &filterKey, AQLQuery &as) noexcept;
std::tuple<ErrorCode, AQLQuery> buildQuery(AQLQuery &selector,
                                           Columns const &columns,
                                           jsoncons::ojson const &args,
                                           mutate_schema::Relation const &tree,
                                           std::map<std::string, std::pair<AQLQuery, AQLQuery> > &wrapperFilters,
                                           TableWhere const &where_,
                                           Sort sort_,
                                           Pagination pagination_,
                                           AQLQuery const &as,
                                           Collection const &collection,
                                           Edge const &edge,
                                           AQLQuery const &filterTop,
                                           AQLQuery const &filterBottom,
                                           AQLQuery const &defaultWhere) noexcept;
std::tuple<ErrorCode, AQLQuery> buildWherePart(Columns const &columns, AQLQuery const &as, TableWhere const &where, AQLQuery const &defaultWhere) noexcept;
namespace impl
{
AQLQuery buildOrderByPart(Columns const &columns, AQLQuery const &as, Sort const &sort) noexcept;
AQLQuery buildPaginationPart(Pagination const &pagination) noexcept;
AQLQuery buildTableObjectProjection(Columns const &columns, AQLQuery const &as) noexcept;
inline float convert_float(std::string const &s, std::size_t pos)
{
  // return boost::lexical_cast<int>(s.data() + pos, n);
  // https://www.fluentcpp.com/2018/07/24/how-to-convert-a-string-to-an-int-in-c/
  // address of an integer to store the number of characters processed
  std::size_t *numericp = nullptr;
  try
  {
    return std::stof(s.substr(pos), numericp);
  }
  catch (std::invalid_argument const &ia)
  {
    return 0;
  }
  catch (std::out_of_range const &oor)
  {
    return 0;
  }
}
}  // namespace impl
}  // namespace table_schema
}  // namespace ok
