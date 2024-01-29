#pragma once
#include "jsoncons/json.hpp"
#include "alias.hpp"
#include "mgclient.hpp"
#include "db/Session.hpp"
#include "db/mgclientPool.hpp"

namespace ok::db::get
{
void fetchFromMGAndSaveToArray(const std::string& query,
                               ok::db::MGParams &p,
                               jsoncons::ojson &nodes,
                               jsoncons::ojson &relationships,
                               std::map<std::string, std::unordered_set<int64_t>> &labelIdMap,
                               std::unique_ptr<mg::Client> &connPtr, int mgPort);
void getNodeAndEdgeObj(const jsoncons::ojson &nodes,
                       const jsoncons::ojson &relationships,
                       std::map<std::string, std::unordered_set<int64_t>> &vertexLabelIdMap,
                       jsoncons::ojson &nodeObj,
                       jsoncons::ojson &relationshipObj);
jsoncons::ojson getInitialData(
    const WsArguments &args,
    std::unique_ptr<mg::Client> &mgClient,
    const ok::smart_actor::connection::Session &session);
jsoncons::ojson getCollectionData(
    const WsArguments &args,
    std::unique_ptr<mg::Client> &mgClient,
    const ok::smart_actor::connection::Session &session);
bool reconnect(std::unique_ptr<mg::Client> &connPtr, int mgPort);
}
