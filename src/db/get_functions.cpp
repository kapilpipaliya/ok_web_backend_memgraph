#include "db/get_functions.hpp"
#include "utils/time_functions.hpp"
#include "utils/mg_helper.hpp"
#include "utils/BatchArrayMessage.hpp"
#include "third_party/mgclient/src/mgvalue.h"
#include "db/mgclientPool.hpp"
#include <magic_enum.hpp>
#include "actor_system/SyncActor.hpp"
namespace ok::db::get
{


void fetchNodesAPI(const std::string& query,
                   ok::db::MGParams &p,
                   jsoncons::ojson &nodes,
                   std::unique_ptr<mg::Client> &connPtr)
{
    if (!connPtr->Execute(query.c_str(), p.asConstMap()))
    {
        LOG_ERROR << "Failed to execute query!" << query << " "
                  << mg_session_error(connPtr->session_);
        reconnect(connPtr);
        fetchNodesAPI(query, p, nodes, connPtr);
    }
    try
    {
        const auto maybeResult = connPtr->FetchAll();
        if (maybeResult)
            for (auto &row : *maybeResult)
                for (auto &matchPart : row)
                    if (matchPart.type() == mg::Value::Type::Node)
                        nodes.push_back(
                            convertNodeToJson(matchPart.ValueNode()));
    }
    catch (mg::ClientException e)
    {
        jsoncons::ojson result;
        result["error"] = true;
        result["message"] = e.what();
        LOG_ERROR << e.what();
        reconnect(connPtr);
        // only refetch if needed.
        // fetchNodesAPI(query, nodes, mgClient);
    }
}

void fetchRelationshipsAPI(const std::string& query,
                           ok::db::MGParams &p,
                           jsoncons::ojson &relationships,
                           std::unique_ptr<mg::Client> &connPtr)
{
    if (!connPtr->Execute(query, p.asConstMap()))
    {
        LOG_ERROR << "Failed to execute query!" << query << " "
                  << mg_session_error(connPtr->session_);
        reconnect(connPtr);
        fetchRelationshipsAPI(query, p, relationships, connPtr);
    }
    try
    {
        const auto maybeResult = connPtr->FetchAll();
        if (maybeResult)
            for (auto &row : *maybeResult)
                for (auto &matchPart : row)
                    if (matchPart.type() == mg::Value::Type::Relationship)
                        relationships.push_back(convertRelationshipToJson(
                            matchPart.ValueRelationship()));
    }
    catch (mg::ClientException e)
    {
        jsoncons::ojson result;
        result["error"] = true;
        result["message"] = e.what();
        LOG_ERROR << e.what();
        reconnect(connPtr);
    }
}


jsoncons::ojson getInitialData(
    const WsArguments &args,
    std::unique_ptr<mg::Client> &mgClient,
    const ok::smart_actor::connection::Session &session)
{
    jsoncons::ojson nodes = jsoncons::ojson::array();
    jsoncons::ojson relationships = jsoncons::ojson::array();
    // 1. get all coll nodes.
    // 2. if no memberKey send all public nodes and relationships.
    // 3. if memberKey send data that user is allowed to see.
    // 4. if super admin send all data.
    bool superAdmin{false};
    if (superAdmin)
    {
        ok::db::MGParams p{};
        fetchNodesAPI("MATCH (n) RETURN n;", p, nodes, mgClient);
        ok::db::MGParams p1{};
        fetchRelationshipsAPI("MATCH ()-[r]->() RETURN r;",
                              p1,
                              relationships,
                              mgClient);
    }
    // disable auth temparary
    // else if (memberKey != -1)
    else if (true)
    {
        // temporary sending all data to logged in member:

        // get web component category id
        // MATCH (c:CompCategory) WHERE c.name = 'View' return c

        // get all web components nodes.
        // MATCH (c:Comp)-[]->(p) RETURN c, p

        // get all web components edges.
        // MATCH (c:Comp)-[cp]->()-[cp2]->() RETURN cp, cp2

        if (!args.contains("admin"))
        {
            std::vector<std::string> labelVec{"CompProp", "CompEvent"};
            mg_list *labelList = mg_list_make_empty(labelVec.size());
            for (auto const &v : labelVec)
            {
                mg_list_append(labelList, mg_value_make_string(v.c_str()));
            }
            ok::db::MGParams p{{"labels", mg_value_make_list(labelList)}};

            fetchNodesAPI(
                "MATCH (n) WHERE NOT all(l IN LABELS(n) WHERE l IN $labels) RETURN n;",
                p,
                nodes,
                mgClient);

            std::vector<std::string> labelType{"CompProp",
                                               "CompEvent",
                                               "CompPropDataType",
                                               "CompPropComp",
                                               "AttrPropName"};
            mg_list *typeList = mg_list_make_empty(labelType.size());
            for (auto const &v : labelType)
            {
                mg_list_append(typeList, mg_value_make_string(v.c_str()));
            }
            ok::db::MGParams p1{{"types", mg_value_make_list(typeList)}};

            fetchRelationshipsAPI(
                "MATCH  ()-[r]->() WHERE NOT type(r) in $types RETURN r;",
                p1,
                relationships,
                mgClient);

            // send View Components:
            // get view compnents
            //                    ok::db::MGParams p0{};
            //                    fetchNodesAPI("MATCH (ca:CompCategory {name: 'View'})-[]->(n:Comp) return DISTINCT n;",
            //                                  p0,
            //                                  nodes,
            //                                  mgClient);

            // get view compnents properties
            ok::db::MGParams p01{};
            fetchNodesAPI(
                "MATCH (ca:CompCategory {name: 'View'})-[]->(:Comp)-[]->(n) return DISTINCT n;",
                p01,
                nodes,
                mgClient);

            ok::db::MGParams p02{};
            fetchRelationshipsAPI(
                "MATCH (ca:CompCategory {name: 'View'})-[]->(:Comp)-[r]->() return DISTINCT r;",
                p02,
                relationships,
                mgClient);
            ok::db::MGParams p03{};
            fetchRelationshipsAPI(
                "MATCH (ca:CompCategory {name: 'View'})-[]->(:Comp)-[]->()-[r]->() return DISTINCT r;",
                p03,
                relationships,
                mgClient);
            ok::db::MGParams p04{};
            fetchRelationshipsAPI(
                "MATCH (ca:CompCategory {name: 'View'})-[]->(:Comp)-[]->()<-[r]-() return DISTINCT r;",
                p04,
                relationships,
                mgClient);
        }
        else
        {
            ok::db::MGParams p{};
            fetchNodesAPI("MATCH (n) RETURN n;", p, nodes, mgClient);
            ok::db::MGParams p1{};
            fetchRelationshipsAPI("MATCH ()-[r]->() RETURN r;",
                                  p1,
                                  relationships,
                                  mgClient);
        }
    }
    else
    {
        // get all public nodes:
        ok::db::MGParams p2{};
        auto [error, response] = ok::db::mgCall(
            getAllNodesWithALabel("Coll", "WHERE n.public = true"), p2);
        if (!error.empty())
        {
            LOG_ERROR << error;
        }
        else
        {
            std::vector<std::string> labelVec{"DataType",
                                              "Comp",
                                              "CompProp",
                                              "Attr",
                                              "Coll",
                                              "CollAttr",
                                              "CollAttrProp",
                                              "Nav",
                                              "NavAttr"};
            auto getNamesFromResponse = ok::db::getNamesFromResponse(*response);
            for (auto &id : getNamesFromResponse)
            {
                labelVec.emplace_back(id);
            }

            mg_list *labelList = mg_list_make_empty(labelVec.size());
            for (auto const &v : labelVec)
            {
                mg_list_append(labelList, mg_value_make_string(v.c_str()));
            }
            ok::db::MGParams p1{{"labels", mg_value_make_list(labelList)}};
            std::string labels{};
            for (auto &id : labelVec)
            {
                labels += ":" + id;
            }
            fetchNodesAPI(
                "MATCH (n) WHERE all(l IN LABELS(n) WHERE l IN $labels) "
                "RETURN n;",
                p1,
                nodes,
                mgClient);

            fetchRelationshipsAPI(
                "MATCH (n)-[r]->(m) "
                "WHERE all(l IN LABELS(n) WHERE l IN $labels) AND "
                "all(l IN LABELS(m) WHERE l IN $labels) "
                "RETURN r;",
                p1,
                relationships,
                mgClient);
        }
    }

    jsoncons::ojson result;
    result["timestemp"] = ok::utils::time::getEpochMilliseconds();

    jsoncons::ojson vertexLabelIdMap;
    jsoncons::ojson nodeObj;
    for (auto const &n : nodes.array_range())
    {
        nodeObj[n["id"].as_string()] = n;
        for (auto const &l : n["L"].array_range())
        {
            if (vertexLabelIdMap[l.as_string_view()].empty())
            {
                vertexLabelIdMap[l.as_string_view()] =
                    jsoncons::ojson(jsoncons::json_array_arg, {n["id"]});
            }
            else
            {
                vertexLabelIdMap[l.as_string_view()].push_back(n["id"]);
            }
        }
    }
    jsoncons::ojson relationshipObj;
    for (auto const &r : relationships.array_range())
    {
        relationshipObj[r["id"].as_string()] = r;
    }

    result["vertexes"] = nodeObj;
    result["vertexLabelIdMap"] = vertexLabelIdMap;
    result["edges"] = relationshipObj;
    return result;
}
bool reconnect(
    std::unique_ptr<mg::Client> &connPtr)
{
    connPtr->Finalize();
    mg::Client::Params params;
    params.host = "localhost";
    params.port = global_var::mg_port;
    params.use_ssl = false;
    auto client = mg::Client::Connect(params);
    connPtr = std::move(client);
    if (connPtr)
    {
        return true;
    }
    else
    {
        LOG_ERROR << "Failed to connect!";
        return false;
    }
}
}  // namespace ok::db::get