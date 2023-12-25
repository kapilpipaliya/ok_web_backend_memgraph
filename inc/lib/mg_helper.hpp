#pragma once
#include "jsoncons/json.hpp"
#include "mgclient.hpp"

jsoncons::ojson memGraphMapToJson(mg::ConstMap map);
jsoncons::ojson convertNodeToJson(mg::ConstNode node);
jsoncons::ojson convertRelationshipToJson(mg::ConstRelationship relationship);
jsoncons::ojson convertListToJson(mg::ConstList list);

inline std::string getAllNodesWithALabel(std::string const &label, std::string const &filter) {
    return "MATCH (n:" + label + ") "  + filter + " RETURN n;";
}
