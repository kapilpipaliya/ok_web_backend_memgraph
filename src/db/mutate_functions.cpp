#include "db/mutate_functions.hpp"
#include "jsoncons/json_encoder2.hpp"
#include "lib/json_functions.hpp"
#include "utils/BatchArrayMessage.hpp"
#include "third_party/mgclient/src/mgvalue.h"
#include "db/mgclientPool.hpp"
#include "actor_system/MutationActor.hpp"
namespace ok::db::mutate
{
std::basic_string<char> jsonToMemGraphQueryObject(jsoncons::ojson const &json)
{
    std::string dump;
    using string_type = std::basic_string<char>;
    string_type s;
    jsoncons::basic_compact_json_encoder2<char,
                                          jsoncons::string_sink<string_type>>
        encoder(s);
    json.dump(encoder);
    return s;
}
enum MutationValue
{
    txnId = 0,
    vertexIdMap = 1,
    edgeIdMap = 2,
    transactions = 3
};

std::string isProperNodeFormat(jsoncons::ojson node)
{
    if (!node.is_object())
    {
        return "node must be an object";
    }
    if (!node.contains("L"))
    {
        return "node must contain L (labels)";
    }
    if (!node["L"].is_array() && node["L"].size() > 1)
    {
        return "node L (labels) must be an array and should contain at least "
               "one "
               "item.";
    }
    if (!node["L"].is_array() && node["L"].size() > 1)
    {
        return "node L (labels) must be an array and should contain at least "
               "one "
               "item.";
    }
    for (auto const &label : node["L"].array_range())
    {
        if (!label.is_string())
        {
            return "node label must be a string";
        }
    }
    if (!node.contains("P") || !node["P"].is_object())
    {
        return "node must have P(properties) key and it must be an object";
    }
    if (!node.contains("id"))
    {
        return "node must have id key";
    }
    return "";
}
std::string isProperEdgeFormat(jsoncons::ojson edge)
{
    if (!edge.is_object())
    {
        return "edge must be an object";
    }
    if (!edge.contains("T") || !edge["T"].is_string())
    {
        return "edge must contain T (type) and it must be a string.";
    }
    if (!edge.contains("S") || !edge["S"].is_int64())
    {
        return "edge must contain S (start) and it must be a integer.";
    }
    if (!edge.contains("E") || !edge["E"].is_int64())
    {
        return "edge must contain E (end) and it must be a integer.";
    }
    if (!edge.contains("P") || !edge["P"].is_object())
    {
        return "edge must have P (properties) key and it must be an object";
    }
    if (!edge.contains("id"))
    {
        return "edge must have id key";
    }
    return "";
}

jsoncons::ojson mutate_data(
    const WsEvent &event,
    const WsArguments &args,
    std::unique_ptr<mg::Client> &mgClient,
    const ok::smart_actor::connection::Session &session)
{
    std::map<int, int> clientToServerVertexIdMap;
    std::map<int, int> clientToServerEdgeIdMap;
    auto responseResult = ok::smart_actor::connection::wsMessageBase();

    


    if (!args.is_array())
    {
        return ok::smart_actor::connection::addFailure(
                              responseResult,
                              event,
                              "Invalid argument shape, the argument to "
                              "sync(mutate) must be an array");
    }
    for (auto const &txns : args.array_range())
    {
        if (!txns.is_array())
        {
            return ok::smart_actor::connection::addFailure(
                                  responseResult,
                                  event,
                                  "Invalid argument shape, the inner "
                                  "argument to sync(mutate) must "
                                  "be an array");
        }
        if (txns.size() != 4)
        {
            return ok::smart_actor::connection::addFailure(
                    responseResult,
                    event,
                    "Invalid argument shape, the inner argument to "
                    "sync(mutate) must "
                    "be an array and it should contain 4 elements. "
                    "first-txnId, second-vertexIdMap, third-EdgeIdMap, "
                    "fourth-The transactions.");
        }
        if ((!txns[vertexIdMap].is_array() ||
             !txns[edgeIdMap].is_array()))
        {
            return ok::smart_actor::connection::addFailure(
                    responseResult,
                    event,
                    "Invalid argument shape, the inner argument to "
                    "sync(mutate) must "
                    "be an array and it should contain 4 elements. "
                    "first-txnId, second-vertexIdMap, third-EdgeIdMap, "
                    "fourth-The transactions. vertexIdMap and "
                    "EdgeIdMap each item is [num, num] tuple");
        }
        for (auto const &idmap : txns[vertexIdMap].array_range())
        {
            if ((idmap.size() != 2) ||
                (!idmap[0].is_int64() || !idmap[1].is_int64()))
            {
                return ok::smart_actor::connection::addFailure(
                        responseResult,
                        event,
                        "Invalid argument shape, vertexIdMap and "
                        "EdgeIdMap each item is [num, num] tuple");
            }
        }
        for (auto const &idmap : txns[edgeIdMap].array_range())
        {
            if ((idmap.size() != 2) ||
                (!idmap[0].is_int64() || !idmap[1].is_int64()))
            {
                return ok::smart_actor::connection::addFailure(
                        responseResult,
                        event,
                        "Invalid argument shape, vertexIdMap and "
                        "EdgeIdMap each item is [num, num] tuple");
            }
        }
        for (auto const &mutationObject :
             txns[transactions].array_range())
        {
            for (auto const &key : std::vector<std::string>{
                     "insert", "replace", "merge", "deleteVertex"})
            {
                if (mutationObject.contains(key) &&
                    !mutationObject[key].is_object())
                {
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            key + " shape must be an object");
                }
                if (mutationObject.contains(key))
                {
                    auto const err =
                        isProperNodeFormat(mutationObject[key]);
                    if (!err.empty())
                    {
                        return ok::smart_actor::connection::addFailure(
                                responseResult,
                                event,
                                key + " " + err);
                    }
                }
            }
            for (auto const &key :
                 std::vector<std::string>{"insertEdge",
                                          "replaceEdge",
                                          "replaceEdgeStart",
                                          "replaceEdgeEnd",
                                          "deleteEdge"})
            {
                if (mutationObject.contains(key) &&
                    !mutationObject[key].is_object())
                {
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            key + " shape must be an object");
                }
                if (mutationObject.contains(key))
                {
                    auto const err =
                        isProperEdgeFormat(mutationObject[key]);
                    if (!err.empty())
                    {
                        return ok::smart_actor::connection::addFailure(
                                responseResult,
                                event,
                                key + " " + err);
                    }
                }
            }
        }
    }
    auto resultTxn = jsoncons::ojson::array();
    for (auto const &txns : args.array_range())
    {
        for (auto const &vertexOldIdToNewIdEntry :
             txns[vertexIdMap].array_range())
        {
            clientToServerVertexIdMap.emplace(
                vertexOldIdToNewIdEntry[0].as<int>(),
                vertexOldIdToNewIdEntry[1].as<int>());
        }
        for (auto const &edgeOldIdToNewIdEntry :
             txns[edgeIdMap].array_range())
        {
            clientToServerEdgeIdMap.emplace(
                edgeOldIdToNewIdEntry[0].as<int>(),
                edgeOldIdToNewIdEntry[1].as<int>());
        }

        mgClient->BeginTransaction();
        auto txnResult = jsoncons::ojson::array();
        for (auto const &mutationObject :
             txns[transactions].array_range())
        {
            if (jsoncons::ObjectMemberIsObject(mutationObject,
                                               "insert"))
            {
                std::string query{"CREATE (n"};

                std::string labels;
                for (auto const &label :
                     mutationObject["insert"]["L"].array_range())
                {
                    labels.append(":");
                    labels.append(label.as_string());
                }
                query.append(labels);
                query.append(jsonToMemGraphQueryObject(
                    mutationObject["insert"]["P"]));
                query.append(") ");
                query.append(" RETURN n;");

                if (!mgClient->Execute(query))
                {
                    std::cerr << "Failed to execute query!" << query
                              << " "
                              << mg_session_error(
                                     mgClient->session_);
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            "Cant insert because internal server "
                            "error.");
                }
                try
                {
                    const auto maybeResult =
                        mgClient->FetchAll();
                    if (maybeResult)
                    {
                        auto oldId =
                            mutationObject["insert"]["id"].as<int>();
                        auto id = ok::db::getIdFromMGResponse(*maybeResult);
                        clientToServerVertexIdMap.emplace(
                            std::pair{oldId, id});
                        jsoncons::ojson insertResult;
                        auto result = jsoncons::ojson::array();
                        result.push_back(oldId);
                        result.push_back(id);
                        insertResult["insert"] = result;
                        txnResult.push_back(insertResult);
                    }
                }
                catch (mg::ClientException e)
                {
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            std::string{"Cant insert because internal "
                                        "server error."} +
                                e.what());
                }
            }
            if (jsoncons::ObjectMemberIsObject(mutationObject,
                                               "replace"))
            {
                int id = mutationObject["replace"]["id"].as<int64_t>();
                if (clientToServerVertexIdMap.contains(id))
                    id = clientToServerVertexIdMap[id];
                ok::db::MGParams p2{{"id", mg_value_make_integer(id)}};
                std::string query{
                    "MATCH (n) WHERE ID(n) = $id SET n = "};
                query.append(jsonToMemGraphQueryObject(
                    mutationObject["replace"]["P"]));
                query.append(" RETURN n;");

                if (!mgClient->Execute(query,
                                                  p2.asConstMap()))
                {
                    std::cerr << "Failed to execute query!" << query
                              << " "
                              << mg_session_error(
                                     mgClient->session_);
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            "Cant insert because internal server "
                            "error.");
                }
                try
                {
                    const auto maybeResult =
                        mgClient->FetchAll();
                    auto result = jsoncons::ojson{};
                    result["error"] = false;
                    txnResult.push_back(result);
                }
                catch (mg::ClientException e)
                {
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            std::string{"Cant replace because internal "
                                        "server error."} +
                                e.what());
                }
            }
            if (jsoncons::ObjectMemberIsObject(mutationObject, "merge"))
            {
                int id = mutationObject["merge"]["id"].as<int64_t>();
                if (clientToServerVertexIdMap.contains(id))
                    id = clientToServerVertexIdMap[id];
                ok::db::MGParams p2{{"id", mg_value_make_integer(id)}};
                std::string query{
                    "MATCH (n) WHERE ID(n) = $id SET n += "};
                query.append(jsonToMemGraphQueryObject(
                    mutationObject["merge"]["P"]));
                query.append(" RETURN n;");

                if (!mgClient->Execute(query,
                                                  p2.asConstMap()))
                {
                    std::cerr << "Failed to execute query!" << query
                              << " "
                              << mg_session_error(
                                     mgClient->session_);
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            "Cant merge because internal server "
                            "error.");
                }
                try
                {
                    const auto maybeResult =
                        mgClient->FetchAll();
                    auto result = jsoncons::ojson{};
                    result["error"] = false;
                    txnResult.push_back(result);
                }
                catch (mg::ClientException e)
                {
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            std::string{"Cant replace because internal "
                                        "server error."} +
                                e.what());
                }
            }
            if (jsoncons::ObjectMemberIsObject(mutationObject,
                                               "deleteVertex"))
            {
                int id =
                    mutationObject["deleteVertex"]["id"].as<int64_t>();
                if (clientToServerVertexIdMap.contains(id))
                    id = clientToServerVertexIdMap[id];
                ok::db::MGParams p2{{"id", mg_value_make_integer(id)}};
                std::string query{"MATCH (n) WHERE ID(n) = $id "};
                query.append(" DELETE n;");

                if (!mgClient->Execute(query,
                                                  p2.asConstMap()))
                {
                    std::cerr << "Failed to execute query!" << query
                              << " "
                              << mg_session_error(
                                     mgClient->session_);
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            "Cant insert because internal server "
                            "error.");
                }
                try
                {
                    const auto maybeResult =
                        mgClient->FetchAll();
                }
                catch (mg::ClientException e)
                {
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            std::string{"Cant delete edge because "
                                        "internal server error."} +
                                e.what());
                }
            }
            if (jsoncons::ObjectMemberIsObject(mutationObject,
                                               "insertEdge"))
            {
                auto start =
                    mutationObject["insertEdge"]["S"].as<int64_t>();
                if (clientToServerVertexIdMap.contains(start))
                    start = clientToServerVertexIdMap[start];
                auto end =
                    mutationObject["insertEdge"]["E"].as<int64_t>();
                if (clientToServerVertexIdMap.contains(end))
                    end = clientToServerVertexIdMap[end];
                ok::db::MGParams p2{{"idstart",
                                     mg_value_make_integer(start)},
                                    {"idend",
                                     mg_value_make_integer(end)}};
                std::string query{
                    "match (n1) where id(n1)=$idstart  match (n2) "
                    "where "
                    "id(n2)=$idend merge (n1)-[r:"};
                query.append(
                    mutationObject["insertEdge"]["T"].as_string());
                query.append(jsonToMemGraphQueryObject(
                    mutationObject["insertEdge"]["P"]));
                query.append("]->(n2) return r;");

                if (!mgClient->Execute(query,
                                                  p2.asConstMap()))
                {
                    std::cerr << "Failed to execute query!" << query
                              << " "
                              << mg_session_error(
                                     mgClient->session_);
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            "Cant insert because internal server "
                            "error.");
                }
                try
                {
                    const auto maybeResult =
                        mgClient->FetchAll();
                    if (maybeResult)
                    {
                        auto oldEdgeId =
                            mutationObject["insertEdge"]["id"]
                                .as<int>();
                        auto id = ok::db::getIdFromMGRelationshipResponse(
                            *maybeResult);
                        clientToServerEdgeIdMap.emplace(
                            std::pair{oldEdgeId, id});
                        jsoncons::ojson insertEdgeResult;
                        auto result = jsoncons::ojson::array();
                        result.push_back(oldEdgeId);
                        result.push_back(id);
                        insertEdgeResult["insertEdge"] = result;
                        txnResult.push_back(insertEdgeResult);
                    }
                }
                catch (mg::ClientException e)
                {
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            std::string{"Cant insert edge because "
                                        "internal server error."} +
                                e.what());
                }
            }
            if (jsoncons::ObjectMemberIsObject(mutationObject,
                                               "replaceEdge"))
            {
                int id =
                    mutationObject["replaceEdge"]["id"].as<int64_t>();
                if (clientToServerEdgeIdMap.contains(id))
                    id = clientToServerEdgeIdMap[id];
                auto start =
                    mutationObject["replaceEdge"]["S"].as<int64_t>();
                if (clientToServerVertexIdMap.contains(start))
                    start = clientToServerVertexIdMap[start];
                auto end =
                    mutationObject["replaceEdge"]["E"].as<int64_t>();
                if (clientToServerVertexIdMap.contains(end))
                    end = clientToServerVertexIdMap[end];

                ok::db::MGParams p2{{"idstart",
                                     mg_value_make_integer(start)},
                                    {"idend",
                                     mg_value_make_integer(end)},
                                    {"id", mg_value_make_integer(id)}};
                // we can also write query like replaceEdgeStart
                std::string query{
                    "match (n1) where id(n1)=$idstart  match (n2) "
                    "where "
                    "id(n2)=$idend match (n1)-[r:"};
                query.append(
                    mutationObject["replaceEdge"]["T"].as_string());
                query.append("]-> (n2) where id(r) = $id set r = ");
                query.append(jsonToMemGraphQueryObject(
                    mutationObject["replaceEdge"]["P"]));
                query.append(" return r;");

                if (!mgClient->Execute(query,
                                                  p2.asConstMap()))
                {
                    std::cerr << "Failed to execute query!" << query
                              << " "
                              << mg_session_error(
                                     mgClient->session_);
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            "Cant replace edge P (properties) because "
                            "internal server error.");
                }
                try
                {
                    const auto maybeResult =
                        mgClient->FetchAll();
                }
                catch (mg::ClientException e)
                {
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            std::string{"Cant insert because internal "
                                        "server error."} +
                                e.what());
                }
            }
            if (jsoncons::ObjectMemberIsObject(mutationObject,
                                               "replaceEdgeStart"))
            {
                int id = mutationObject["replaceEdgeStart"]["id"]
                             .as<int64_t>();
                if (clientToServerEdgeIdMap.contains(id))
                    id = clientToServerEdgeIdMap[id];
                auto start = mutationObject["replaceEdgeStart"]["S"]
                                 .as<int64_t>();
                if (clientToServerVertexIdMap.contains(start))
                    start = clientToServerVertexIdMap[start];

                ok::db::MGParams p2{
                    {"edgeid", mg_value_make_integer(id)},
                    {"newstart", mg_value_make_integer(start)},
                };
                std::string query{"match ()-[r:"};
                query.append(mutationObject["replaceEdgeStart"]["T"]
                                 .as_string());
                query.append(
                    "]->() "
                    "WHERE id(r) = $edgeid "
                    "match (s) where id(s) = $newstart "
                    "match (e) where id(e) = id(endNode(r)) "
                    "merge (s)-[t:");
                query.append(mutationObject["replaceEdgeStart"]["T"]
                                 .as_string());
                query.append(
                    "]-> (e)"
                    "set t = properties(r)"
                    "delete r"
                    "return t");

                if (!mgClient->Execute(query,
                                                  p2.asConstMap()))
                {
                    std::cerr << "Failed to execute query!" << query
                              << " "
                              << mg_session_error(
                                     mgClient->session_);
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            "Cant insert because internal server "
                            "error.");
                }
                try
                {
                    const auto maybeResult =
                        mgClient->FetchAll();
                }
                catch (mg::ClientException e)
                {
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            std::string{
                                "Cant replace edge start because "
                                "internal server error."} +
                                e.what());
                }
            }
            if (jsoncons::ObjectMemberIsObject(mutationObject,
                                               "replaceEdgeEnd"))
            {
                int id = mutationObject["replaceEdgeEnd"]["id"]
                             .as<int64_t>();
                if (clientToServerEdgeIdMap.contains(id))
                    id = clientToServerEdgeIdMap[id];
                auto end =
                    mutationObject["replaceEdgeEnd"]["E"].as<int64_t>();
                if (clientToServerVertexIdMap.contains(end))
                    end = clientToServerVertexIdMap[end];
                ok::db::MGParams p2{{"edgeid",
                                     mg_value_make_integer(id)},
                                    {"newend",
                                     mg_value_make_integer(end)}};
                std::string query{"match ()-[r:"};
                auto type =
                    mutationObject["replaceEdgeEnd"]["T"].as_string();
                query.append(type);
                query.append(
                    "]->() "
                    "WHERE id(r) = $edgeid "
                    "match (s) where id(s) = id(startNode(r)) "
                    "match (e) where id(e) = $newend "
                    "create (s)-[t:");
                query.append(type);
                query.append(
                    "]->(e) "
                    "set t = properties(r) "
                    "delete r "
                    "return t;");
                LOG_DEBUG << query;
                if (!mgClient->Execute(query,
                                                  p2.asConstMap()))
                {
                    std::cerr << "Failed to execute query!" << query
                              << " "
                              << mg_session_error(
                                     mgClient->session_);
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            "Cant replace edge end because internal "
                            "server error.");
                }
                try
                {
                    // this is required to execute query:
                    const auto maybeResult =
                        mgClient->FetchAll();
                }
                catch (mg::ClientException e)
                {
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            std::string{"Cant replace edge end because "
                                        "internal server error."} +
                                e.what());
                }
            }
            if (jsoncons::ObjectMemberIsObject(mutationObject,
                                               "deleteEdge"))
            {
                int id =
                    mutationObject["deleteEdge"]["id"].as<int64_t>();
                if (clientToServerEdgeIdMap.contains(id))
                    id = clientToServerEdgeIdMap[id];
                auto start =
                    mutationObject["deleteEdge"]["S"].as<int64_t>();
                if (clientToServerVertexIdMap.contains(start))
                    start = clientToServerVertexIdMap[start];
                auto end =
                    mutationObject["deleteEdge"]["E"].as<int64_t>();
                if (clientToServerVertexIdMap.contains(end))
                    end = clientToServerVertexIdMap[end];

                ok::db::MGParams p2{{"idstart",
                                     mg_value_make_integer(start)},
                                    {"idend",
                                     mg_value_make_integer(end)},
                                    {"id", mg_value_make_integer(id)}};
                std::string query{"match (n1)-[r:"};
                query.append(
                    mutationObject["deleteEdge"]["T"].as_string());
                query.append(
                    "]->(n2) where id(n1) = $idstart and id(n2) = "
                    "$idend and id(r) = $id "
                    "delete r;");

                if (!mgClient->Execute(query,
                                                  p2.asConstMap()))
                {
                    LOG_ERROR << "Failed to execute query!" << query
                              << " "
                              << mutationObject["deleteEdge"]["T"]
                                     .as_cstring()
                              << mg_session_error(
                                     mgClient->session_);
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            "Cant delete edge because internal server "
                            "error.");
                }
                try
                {
                    const auto maybeResult =
                        mgClient->FetchAll();
                }
                catch (mg::ClientException e)
                {
                    // TODO: report error to db, reason, status code
                    // everything;
                    mgClient->RollbackTransaction();
                    return ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            std::string{"Cant delete edge because "
                                        "internal server error."} +
                                e.what());
                }
            }
        }
        auto o = jsoncons::ojson::array();
        o.push_back(txns[0]);
        o.push_back(txnResult);
        resultTxn.push_back(o);

        mgClient->CommitTransaction();
        mgClient->Finalize();
    }

    ok::smart_actor::connection::addEventAndJson(responseResult,
                                                 event,
                                                 resultTxn);
}
}  // namespace ok::db::get