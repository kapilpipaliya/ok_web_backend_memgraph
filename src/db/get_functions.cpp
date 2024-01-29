#include "db/get_functions.hpp"
#include "utils/time_functions.hpp"
#include "lib/mg_helper.hpp"
#include "utils/BatchArrayMessage.hpp"
#include "third_party/mgclient/src/mgvalue.h"
#include <magic_enum.hpp>
#include "actor_system/SyncActor.hpp"
#include "lib/json_functions.hpp"
#include "pystring.hpp"

namespace ok::db::get
{
jsoncons::ojson getJsonFromPart(mg::Value const &matchPart) {
    if (matchPart.type() == mg::Value::Type::Node)
    {
           return convertNodeToJson(matchPart.ValueNode());
    } else     if (matchPart.type() == mg::Value::Type::Relationship){
        return  convertRelationshipToJson(
            matchPart.ValueRelationship());
    } else if  (matchPart.type() == mg::Value::Type::List) {
        jsoncons::ojson list = jsoncons::ojson::array();
        for (auto item: matchPart.ValueList()) {
            if (item.type() == mg::Value::Type::Node)
            {
                list.push_back(
                    convertNodeToJson(item.ValueNode()));
            } else     if (item.type() == mg::Value::Type::Relationship)
            {
                list.push_back(convertRelationshipToJson(
                    item.ValueRelationship()));
            }
        }
        return list;
    }
    return {};
}
void fetchFromMGAndSaveToArray(const std::string& query,
                   ok::db::MGParams &p,
                   jsoncons::ojson &nodes,
                   jsoncons::ojson &relationships,
                               std::map<std::string, std::unordered_set<int64_t>> &labelIdMap,
                   std::unique_ptr<mg::Client> &connPtr, int mgPort)
{
    if (!connPtr->Execute(query, p.asConstMap()))
    {
        LOG_ERROR << "Failed to execute query!" << query << " "
                  << mg_session_error(connPtr->session_);
        reconnect(connPtr, mgPort);
        fetchFromMGAndSaveToArray(query, p, nodes, relationships, labelIdMap, connPtr, mgPort);
    }
    try
    {
        const auto maybeResult = connPtr->FetchAll();
        if (maybeResult)
            for (auto &row : *maybeResult)
                for (auto &matchPart : row)
                    if (matchPart.type() == mg::Value::Type::Node)
                    {
                        auto id = matchPart.ValueNode().id().AsInt();
                        for (auto const &label: matchPart.ValueNode().labels()) {
                            if (labelIdMap.contains(std::string{label})) {
                                labelIdMap[std::string{label}].insert(id);
                            } else {
                                labelIdMap[std::string{label}] = std::unordered_set{id};
                            }
                        }

                        nodes.push_back(convertNodeToJson(matchPart.ValueNode()));
                    } else if (matchPart.type() == mg::Value::Type::Relationship) {
                        relationships.push_back(convertRelationshipToJson(
                            matchPart.ValueRelationship()));
                    } else if  (matchPart.type() == mg::Value::Type::List) {
                        for (auto item: matchPart.ValueList()) {
                            if (item.type() == mg::Value::Type::Node)
                            {
                                auto id = item.ValueNode().id().AsInt();
                                for (auto const &label: item.ValueNode().labels()) {
                                    if (labelIdMap.contains(std::string{label})) {
                                        labelIdMap[std::string{label}].emplace(id);
                                    } else {
                                        labelIdMap[std::string{label}] = std::unordered_set{id};
                                    }
                                }
                                nodes.push_back(
                                    convertNodeToJson(item.ValueNode()));
                            } else     if (item.type() == mg::Value::Type::Relationship)
                            {
                                relationships.push_back(convertRelationshipToJson(
                                    item.ValueRelationship()));
                            }
                        }
                    }
    }
    catch (mg::ClientException e)
    {
        jsoncons::ojson result;
        result["error"] = true;
        result["message"] = e.what();
        LOG_ERROR << e.what();
        reconnect(connPtr, mgPort);
        // only refetch if needed.
        // fetchFromMGAndSaveToArray(query, nodes, mgClient);
    }
}
void getNodeAndEdgeObj(const jsoncons::ojson &nodes,
                       const jsoncons::ojson &relationships,
                       std::map<std::string, std::unordered_set<int64_t>> &vertexLabelIdMap,
                       jsoncons::ojson &nodeObj,
                       jsoncons::ojson &relationshipObj)
{
    for (auto const &n : nodes.array_range())
    {
        nodeObj[n["id"].as_string()] = n;
        for (auto const &l : n["L"].array_range())
        {
            if (!vertexLabelIdMap.contains(l.as_string()))
            {
                vertexLabelIdMap[l.as_string()] = std::unordered_set{n["id"].as<int64_t>()};
            }
            else
            {
                vertexLabelIdMap[l.as_string()].emplace(n["id"].as<int64_t>());
            }
        }
    }
    for (auto const &r : relationships.array_range())
    {
        relationshipObj[r["id"].as_string()] = r;
    }
}
jsoncons::ojson getAJson(const std::string& query,
                         ok::db::MGParams &p,
                         std::unique_ptr<mg::Client> &mgClient, int mgPort) {
    if (!mgClient->Execute(query, p.asConstMap()))
    {
        LOG_ERROR << "Failed to execute query!" << query << " "
                  << mg_session_error(mgClient->session_);
    }
    std::string resultQuery;
    try
    {
        const auto maybeResult = mgClient->FetchAll();
        if (maybeResult)
            for (auto &row : *maybeResult) {
                for (auto &matchPart : row) {
                    return getJsonFromPart(matchPart);
                }
            }
    }
    catch (mg::ClientException e)
    {
        jsoncons::ojson result;
        result["error"] = true;
        result["message"] = e.what();
        LOG_ERROR << e.what();
        return result;
    }
    return {};
}

jsoncons::ojson getInitialData(
    const WsArguments &args,
    std::unique_ptr<mg::Client> &mgClient,
    const ok::smart_actor::connection::Session &session)
{
    jsoncons::ojson nodes = jsoncons::ojson::array();
    jsoncons::ojson relationships = jsoncons::ojson::array();
    std::map<std::string, std::unordered_set<int64_t>> labelIdMap{};
    // 1. get all coll nodes.
    // 2. if no memberKey send all public nodes and relationships.
    // 3. if memberKey send data that user is allowed to see.
    // 4. if super admin send all data.
    bool superAdmin{false};
    if (superAdmin)
    {
        ok::db::MGParams p{};
        fetchFromMGAndSaveToArray("MATCH (n) RETURN n;", p, nodes,relationships, labelIdMap,mgClient, session.mg_port);
        ok::db::MGParams p1{};
        fetchFromMGAndSaveToArray("MATCH ()-[r]->() RETURN r;",
                              p1, nodes,
                              relationships, labelIdMap,
                              mgClient, session.mg_port);
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

            fetchFromMGAndSaveToArray(
                "MATCH (n) WHERE NOT all(l IN LABELS(n) WHERE l IN $labels) RETURN n;",
                p,
                nodes,relationships, labelIdMap,
                mgClient, session.mg_port);

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

            fetchFromMGAndSaveToArray(
                "MATCH  ()-[r]->() WHERE NOT type(r) in $types RETURN r;",
                p1,nodes,
                relationships, labelIdMap,
                mgClient, session.mg_port);

            // send View Components:
            // get view compnents
            //                    ok::db::MGParams p0{};
            //                    fetchFromMGAndSaveToArray("MATCH (ca:CompCategory {name: 'View'})-[]->(n:Comp) return DISTINCT n;",
            //                                  p0,
            //                                  nodes,
            //                                  mgClient);

            // get view compnents properties
            ok::db::MGParams p01{};
            fetchFromMGAndSaveToArray(
                "MATCH (ca:CompCategory {name: 'View'})-[]->(:Comp)-[]->(n) return DISTINCT n;",
                p01,
                nodes,relationships, labelIdMap,
                mgClient, session.mg_port);

            ok::db::MGParams p02{};
            fetchFromMGAndSaveToArray(
                "MATCH (ca:CompCategory {name: 'View'})-[]->(:Comp)-[r]->() return DISTINCT r;",
                p02,nodes,
                relationships, labelIdMap,
                mgClient, session.mg_port);
            ok::db::MGParams p03{};
            fetchFromMGAndSaveToArray(
                "MATCH (ca:CompCategory {name: 'View'})-[]->(:Comp)-[]->()-[r]->() return DISTINCT r;",
                p03,nodes,
                relationships, labelIdMap,
                mgClient, session.mg_port);
            ok::db::MGParams p04{};
            fetchFromMGAndSaveToArray(
                "MATCH (ca:CompCategory {name: 'View'})-[]->(:Comp)-[]->()<-[r]-() return DISTINCT r;",
                p04,nodes,
                relationships, labelIdMap,
                mgClient, session.mg_port);
        }
        else
        {
            ok::db::MGParams p{};
            fetchFromMGAndSaveToArray("MATCH (n) RETURN n;", p, nodes, relationships, labelIdMap, mgClient, session.mg_port);
            ok::db::MGParams p1{};
            fetchFromMGAndSaveToArray("MATCH ()-[r]->() RETURN r;",
                                  p1,nodes,
                                  relationships, labelIdMap,
                                  mgClient, session.mg_port);
        }
    }
    else
    {
        // get all public nodes:
        ok::db::MGParams p2{};
        auto [error, response] = ok::db::mgCall(
            getAllNodesWithALabel("Coll", "WHERE n.public = true"), p2, session.mg_port);
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
            auto getNamesFromResponse =
                ok::db::getNamesFromMGResponse(*response);
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
            fetchFromMGAndSaveToArray(
                "MATCH (n) WHERE all(l IN LABELS(n) WHERE l IN $labels) "
                "RETURN n;",
                p1,
                nodes,relationships, labelIdMap,
                mgClient, session.mg_port);

            fetchFromMGAndSaveToArray(
                "MATCH (n)-[r]->(m) "
                "WHERE all(l IN LABELS(n) WHERE l IN $labels) AND "
                "all(l IN LABELS(m) WHERE l IN $labels) "
                "RETURN r;",
                p1,nodes,
                relationships, labelIdMap,
                mgClient, session.mg_port);
        }
    }

    jsoncons::ojson result;
    result["timestemp"] = ok::utils::time::getEpochMilliseconds();

    std::map<std::string, std::unordered_set<int64_t>> vertexLabelIdMap;
    jsoncons::ojson nodeObj;
    jsoncons::ojson relationshipObj;

    getNodeAndEdgeObj(
    nodes, relationships, vertexLabelIdMap, nodeObj, relationshipObj);

    result["vertexes"] = nodeObj;
    result["vertexLabelIdMap"] = vertexLabelIdMap;
    result["edges"] = relationshipObj;
    return result;
}


jsoncons::ojson getCollectionData(
    const WsArguments &args,
    std::unique_ptr<mg::Client> &mgClient,
    const ok::smart_actor::connection::Session &session)
{
    if(!jsoncons::ObjectMemberIsString(args, "collection")) {
        // todo
        return {};
    }
    auto collection = args["collection"].as_string();
    jsoncons::ojson nodes = jsoncons::ojson::array();
    jsoncons::ojson relationships = jsoncons::ojson::array();
    std::map<std::string, std::unordered_set<int64_t>> labelIdMap{};
    // 1. get coll.
    // 2. get body level attr and data type.
    // 3. make query and get result data


        std::vector<int> componentIds;
        ok::db::MGParams pDataType{};
        fetchFromMGAndSaveToArray("MATCH (n:DataType)\n"
        "OPTIONAL MATCH (n)-[r:DataTypeDefault]-(m:Comp)\n"
        "return n + collect(r) + collect(m) as n", pDataType, nodes, relationships, labelIdMap, mgClient, session.mg_port);


        ok::db::MGParams pComp{};
        fetchFromMGAndSaveToArray("MATCH (p:Comp)\n"
            "OPTIONAL MATCH (p)<-[r0:CompCategoryComp]-(o0:CompCategory)\n"
            "OPTIONAL MATCH (p)<-[r1:CompChild]-(o1:Comp)\n"
            "OPTIONAL MATCH (p)<-[r2:DataTypeComp]-(o2:DataType)\n"
            "OPTIONAL MATCH (o2)-[r3:DataTypeDefault]->(o3:Comp)\n"
            "OPTIONAL MATCH (p)-[r4:CompProp]->(o4:CompProp)\n"
            "OPTIONAL MATCH (o4)-[r5]->(o5)\n"
            "OPTIONAL MATCH (p)-[r6:CompEvent]->(o6:CompEvent)\n"
            "OPTIONAL MATCH (o6)-[r7]->(o7)\n"
            "WITH p + collect(r0) + collect(o0) + collect(r1) + collect(o1) + collect(r2) + collect(o2) + collect(r3) + collect(o3) + collect(r4) + collect(o4) + collect(r5) + collect(o5) + collect(r6) + collect(o6) + collect(r7) + collect(o7) as list\n"
            "UNWIND list as listElement\n"
            "return collect(distinct listElement) as r", pComp, nodes, relationships, labelIdMap, mgClient, session.mg_port);

        ok::db::MGParams p{{"name", mg_value_make_string(collection.c_str())}};
        fetchFromMGAndSaveToArray("MATCH (n:Coll {name: $name})\n"
            "OPTIONAL MATCH (n)<-[r0:CollCategoryColl]-(o0:CollCategory)\n"
            "OPTIONAL MATCH (n)<-[r1:DashboardColl]-(o1:Dashboard)\n"
            "OPTIONAL MATCH (n)-[r2:CollPk]->(o2:Attr)\n"
            "OPTIONAL MATCH (n)-[r3:CollNameKey]-(o3:Attr)\n"
            "OPTIONAL MATCH path = (n)-[r4:Attr*]->(o4:Attr)\n"
            // TODO: add props
            // TODO: add roles
            "WITH n  + collect(r0) + collect(o0) + collect(r1) + collect(o1) + collect(r2) + collect(o2) + collect(r3) + collect(o3) + relationships(path) + nodes(path) as list\n"
            "UNWIND list as listElement\n"
            "return collect(distinct listElement) as r\n", p, nodes, relationships, labelIdMap, mgClient, session.mg_port);


            std::vector<std::string> labelVec{"=>", "->", "<=", "<-", "List"};
            mg_list *typesList = mg_list_make_empty(labelVec.size());
            for (auto const &v : labelVec)
            {
                mg_list_append(typesList, mg_value_make_string(v.c_str()));
            }
            ok::db::MGParams p2{{"name", mg_value_make_string(collection.c_str())}, {"types", mg_value_make_list(typesList)}};
            std::string query{"MATCH (n:Coll {name: $name})-[m:Attr]-(a:Attr)-[d:AttrDataType]->(t)  WHERE t.name in $types RETURN a, t"};
            if (!mgClient->Execute(query, p2.asConstMap()))
            {
                LOG_ERROR << "Failed to execute query!" << query << " "
                          << mg_session_error(mgClient->session_);
            }

            std::string resultQuery{"MATCH (p:"};
            resultQuery += collection;
            resultQuery += ")\n";
            resultQuery += "WITH p\n";
            std::vector<std::string> rowVariabls{{"p"}};
            std::vector<std::string> lastVariabls{};

            try
            {
                const auto maybeResult = mgClient->FetchAll();
                int index = 0;
                if (maybeResult)
                    for (auto &row : *maybeResult) {
                        auto attr = (row[0]);
                        auto datatype = (row[1]);
                        std::string r = "r";
                        r += std::to_string(index);
                        std::string o = "o";
                        o += std::to_string(index);
                        resultQuery += "OPTIONAL MATCH (p)-[";
                        resultQuery += r;
                        resultQuery += ":";
                        resultQuery += collection;
                        resultQuery += pystring::capitalize(std::string{row[0].ValueNode().properties()["name"].ValueString()});
                        resultQuery += "]->(";
                        resultQuery += o;
                        resultQuery += ":";
                        resultQuery += collection;
                        resultQuery += pystring::capitalize(std::string{row[0].ValueNode().properties()["name"].ValueString()});
                        resultQuery += ")\n";
                        lastVariabls = std::vector<std::string>{r, o};

                        resultQuery += "WITH ";
                        for (const auto& row: rowVariabls) {
                            resultQuery += row;
                            resultQuery += ", ";
                        }
                        for (const auto& row: lastVariabls) {
                            resultQuery += "collect(";
                            resultQuery += row;
                            resultQuery += ") as c";
                            resultQuery += row;
                            resultQuery += ", ";
                        }
                        resultQuery.pop_back();
                        resultQuery.pop_back();
                        resultQuery += "\n";
                        rowVariabls.emplace_back("c" + r);
                        rowVariabls.emplace_back("c" + o);
                        index++;
                    }
                resultQuery += "WITH ";
                for (const auto& row: rowVariabls) {
                    resultQuery += row;
                    resultQuery += " + ";
                }
                resultQuery.pop_back();
                resultQuery.pop_back();
                resultQuery += " as list\n";
                resultQuery += "UNWIND list as listElement\n";
                resultQuery += "RETURN COLLECT(DISTINCT listElement) as r\n";
                LOG_ERROR <<resultQuery;

            }
            catch (mg::ClientException e)
            {
                jsoncons::ojson result;
                result["error"] = true;
                result["message"] = e.what();
                LOG_ERROR << e.what();
            }

            ok::db::MGParams p3{};
            fetchFromMGAndSaveToArray(resultQuery, p3, nodes, relationships, labelIdMap, mgClient, session.mg_port);

    jsoncons::ojson result;
    result["timestemp"] = ok::utils::time::getEpochMilliseconds();
    
    std::map<std::string, std::unordered_set<int64_t>> vertexLabelIdMap;
    jsoncons::ojson nodeObj;
    jsoncons::ojson relationshipObj;
    getNodeAndEdgeObj(
        nodes, relationships, vertexLabelIdMap, nodeObj, relationshipObj);

    result["vertexes"] = nodeObj;
    result["vertexLabelIdMap"] = vertexLabelIdMap;
    result["edges"] = relationshipObj;
    return result;
}
bool reconnect(
    std::unique_ptr<mg::Client> &connPtr, int mgPort)
{
    connPtr->Finalize();
    mg::Client::Params params;
    params.host = "localhost";
    params.port = mgPort;
    params.use_ssl = false;
    auto mgClient = mg::Client::Connect(params);
    connPtr = std::move(mgClient);
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
}

// namespace ok::db::get