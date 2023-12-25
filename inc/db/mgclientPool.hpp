#pragma once
#include <optional>
#include "mgclient-value.hpp"
namespace ok::db
{
// currently only supports strings
struct MGParams
{
    mg_map *extra;
    MGParams(std::initializer_list<std::pair<const char *, mg_value *>> params)
    {
        extra = mg_map_make_empty(params.size());
        if (!extra)
        {
           // return nullptr;
        }
        for (auto const &p : params)
            mg_map_insert_unsafe(extra, p.first, p.second);
    }
    auto asConstMap()
    {
        return mg::ConstMap{extra};
    }
    ~MGParams()
    {
        mg_map_destroy(extra);
    }
};
int getIdFromMGResponse(std::vector<std::vector<mg::Value> > const &response);
std::vector<std::string> getNamesFromMGResponse(std::vector<std::vector<mg::Value> > const &response);
int getIdFromMGRelationshipResponse(std::vector<std::vector<mg::Value> > const &response);
std::pair<std::string, std::optional<std::vector<std::vector<mg::Value>>>>
mgCall(std::string const &query, ok::db::MGParams &p);
}  // namespace ok::db
