#include "actor_system/MutationActor.hpp"
#include <magic_enum.hpp>
#include "db/mgclientPool.hpp"
#include "third_party/mgclient/src/mgvalue.h"
#include "utils/BatchArrayMessage.hpp"
#include "utils/json_functions.hpp"
#include "jsoncons/json_encoder2.hpp"
namespace ok::smart_actor
{
namespace supervisor
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
    if (!node.contains("labels"))
    {
        return "node must contain labels";
    }
    if (!node["labels"].is_array() && node["labels"].size() > 1)
    {
        return "node labels must be an array and should contain at least one "
               "item.";
    }
    if (!node["labels"].is_array() && node["labels"].size() > 1)
    {
        return "node labels must be an array and should contain at least one "
               "item.";
    }
    for (auto const &label : node["labels"].array_range())
    {
        if (!label.is_string())
        {
            return "node label must be a string";
        }
    }
    if (!node.contains("properties") || !node["properties"].is_object())
    {
        return "node must have properties key and it must be an object";
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
    if (!edge.contains("type") || !edge["type"].is_string())
    {
        return "edge must contain type and it must be a string.";
    }
    if (!edge.contains("start") || !edge["start"].is_int64())
    {
        return "edge must contain start and it must be a integer.";
    }
    if (!edge.contains("end") || !edge["end"].is_int64())
    {
        return "edge must contain end and it must be a integer.";
    }
    if (!edge.contains("properties") || !edge["properties"].is_object())
    {
        return "edge must have properties key and it must be an object";
    }
    if (!edge.contains("id"))
    {
        return "edge must have id key";
    }
    return "";
}

// note: mutation action can save all txns and revert back to any point in time!
// TODO: save all txnIds that are done on mutationActor, so it cant be done
// twice.
// TODO: mutation actor update sync actor.
mutation_actor_int::behavior_type MutationActor(MutationActorPointer self)
{
    self->set_error_handler([](caf::error &err) {
        LOG_DEBUG << "Mutation Actor Error :";
        LOG_DEBUG << ok::smart_actor::supervisor::getReasonString(err);
    });
    self->set_down_handler([](caf::scheduled_actor *act,
                              caf::down_msg &msg) noexcept {
        LOG_DEBUG << "Monitored Actor Error Down Error :" << act->name();
        LOG_DEBUG << ok::smart_actor::supervisor::getReasonString(msg.reason);
    });
    // If self exception error occur: server freeze.
    self->set_exception_handler(
        [](caf::scheduled_actor *,
           std::exception_ptr &eptr) noexcept -> caf::error {
            try
            {
                if (eptr)
                {
                    std::rethrow_exception(eptr);
                }
            }
            catch (std::exception const &e)
            {
                LOG_DEBUG << "Muration Actor Exception Error : " << e.what();
            }
            return caf::make_error(
                caf::pec::success);  // self will not resume actor.
        });
    self->set_default_handler(
        [](caf::scheduled_actor *ptr,
           caf::message &x) noexcept -> caf::skippable_result {
            LOG_DEBUG << "unexpected message, I will Quit";
            LOG_DEBUG << "*** unexpected message [id: " << ptr->id()
                      << ", name: " << ptr->name()
                      << "]: " << caf::deep_to_string(x);
            return caf::message{};
        });
    // NOTE: [r:$type] in query doesnt work replace $type with string manually
    return {
        [=](create_atom,
            VertexId const &memberKey,
            WsArguments const &args,
            ws_connector_actor_int connectionActor) {
            std::map<int, int> clientToServerVertexIdMap;
            std::map<int, int> clientToServerEdgeIdMap;
            auto responseResult = ok::smart_actor::connection::wsMessageBase();
            auto event = jsoncons::ojson::array();
            event.push_back("post");
            event.push_back("mutate");
            event.push_back(0);

            // create new connection:
            // on transaction error, we need to create a new connection.
            // its better to always create a new connection because after some
            // inactivity connection is lost too.
            auto client = mg::Client::Connect(self->state.params);
            if (!client)
            {
                LOG_DEBUG << "Failed to connect MG Server";
                return self->send(connectionActor,
                                  caf::forward_atom_v,
                                  ok::smart_actor::connection::addFailure(
                                      responseResult,
                                      event,
                                      "Invalid argument shape, the argument to "
                                      "sync(mutate) must be an array"));
            }
            self->state.connPtr = std::move(client);

            if (!args.is_array())
            {
                return self->send(connectionActor,
                                  caf::forward_atom_v,
                                  ok::smart_actor::connection::addFailure(
                                      responseResult,
                                      event,
                                      "Invalid argument shape, the argument to "
                                      "sync(mutate) must be an array"));
            }
            for (auto const &txns : args.array_range())
            {
                if (!txns.is_array())
                {
                    return self->send(connectionActor,
                                      caf::forward_atom_v,
                                      ok::smart_actor::connection::addFailure(
                                          responseResult,
                                          event,
                                          "Invalid argument shape, the inner "
                                          "argument to sync(mutate) must "
                                          "be an array"));
                }
                if (txns.size() != 4)
                {
                    return self->send(
                        connectionActor,
                        caf::forward_atom_v,
                        ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            "Invalid argument shape, the inner argument to "
                            "sync(mutate) must "
                            "be an array and it should contain 4 elements. "
                            "first-txnId, second-vertexIdMap, third-EdgeIdMap, "
                            "fourth-The transactions."));
                }
                if ((!txns[vertexIdMap].is_array() ||
                     !txns[edgeIdMap].is_array()))
                {
                    return self->send(
                        connectionActor,
                        caf::forward_atom_v,
                        ok::smart_actor::connection::addFailure(
                            responseResult,
                            event,
                            "Invalid argument shape, the inner argument to "
                            "sync(mutate) must "
                            "be an array and it should contain 4 elements. "
                            "first-txnId, second-vertexIdMap, third-EdgeIdMap, "
                            "fourth-The transactions. vertexIdMap and "
                            "EdgeIdMap each item is [num, num] tuple"));
                }
                for (auto const &idmap : txns[vertexIdMap].array_range())
                {
                    if ((idmap.size() != 2) ||
                        (!idmap[0].is_int64() || !idmap[1].is_int64()))
                    {
                        return self->send(
                            connectionActor,
                            caf::forward_atom_v,
                            ok::smart_actor::connection::addFailure(
                                responseResult,
                                event,
                                "Invalid argument shape, vertexIdMap and "
                                "EdgeIdMap each item is [num, num] tuple"));
                    }
                }
                for (auto const &idmap : txns[edgeIdMap].array_range())
                {
                    if ((idmap.size() != 2) ||
                        (!idmap[0].is_int64() || !idmap[1].is_int64()))
                    {
                        return self->send(
                            connectionActor,
                            caf::forward_atom_v,
                            ok::smart_actor::connection::addFailure(
                                responseResult,
                                event,
                                "Invalid argument shape, vertexIdMap and "
                                "EdgeIdMap each item is [num, num] tuple"));
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
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    key + " shape must be an object"));
                        }
                        if (mutationObject.contains(key))
                        {
                            auto const err =
                                isProperNodeFormat(mutationObject[key]);
                            if (!err.empty())
                            {
                                return self->send(
                                    connectionActor,
                                    caf::forward_atom_v,
                                    ok::smart_actor::connection::addFailure(
                                        responseResult,
                                        event,
                                        key + " " + err));
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
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    key + " shape must be an object"));
                        }
                        if (mutationObject.contains(key))
                        {
                            auto const err =
                                isProperEdgeFormat(mutationObject[key]);
                            if (!err.empty())
                            {
                                return self->send(
                                    connectionActor,
                                    caf::forward_atom_v,
                                    ok::smart_actor::connection::addFailure(
                                        responseResult,
                                        event,
                                        key + " " + err));
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

                self->state.connPtr->BeginTransaction();
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
                             mutationObject["insert"]["labels"].array_range())
                        {
                            labels.append(":");
                            labels.append(label.as_string());
                        }
                        query.append(labels);
                        query.append(jsonToMemGraphQueryObject(
                            mutationObject["insert"]["properties"]));
                        query.append(") ");
                        query.append(" RETURN n;");

                        if (!self->state.connPtr->Execute(query))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(
                                             self->state.connPtr->session_);
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    "Cant insert because internal server "
                                    "error."));
                        }
                        try
                        {
                            const auto maybeResult =
                                self->state.connPtr->FetchAll();
                            if (maybeResult)
                            {
                                auto oldId =
                                    mutationObject["insert"]["id"].as<int>();
                                auto id =
                                    ok::db::memgraph_conns.getIdFromResponse(
                                        *maybeResult);
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
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    std::string{"Cant insert because internal "
                                                "server error."} +
                                        e.what()));
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
                            mutationObject["replace"]["properties"]));
                        query.append(" RETURN n;");

                        if (!self->state.connPtr->Execute(query,
                                                          p2.asConstMap()))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(
                                             self->state.connPtr->session_);
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    "Cant insert because internal server "
                                    "error."));
                        }
                        try
                        {
                            const auto maybeResult =
                                self->state.connPtr->FetchAll();
                            auto result = jsoncons::ojson{};
                            result["error"] = false;
                            txnResult.push_back(result);
                        }
                        catch (mg::ClientException e)
                        {
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    std::string{"Cant replace because internal "
                                                "server error."} +
                                        e.what()));
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
                            mutationObject["merge"]["properties"]));
                        query.append(" RETURN n;");

                        if (!self->state.connPtr->Execute(query,
                                                          p2.asConstMap()))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(
                                             self->state.connPtr->session_);
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    "Cant merge because internal server "
                                    "error."));
                        }
                        try
                        {
                            const auto maybeResult =
                                self->state.connPtr->FetchAll();
                            auto result = jsoncons::ojson{};
                            result["error"] = false;
                            txnResult.push_back(result);
                        }
                        catch (mg::ClientException e)
                        {
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    std::string{"Cant replace because internal "
                                                "server error."} +
                                        e.what()));
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

                        if (!self->state.connPtr->Execute(query,
                                                          p2.asConstMap()))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(
                                             self->state.connPtr->session_);
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    "Cant insert because internal server "
                                    "error."));
                        }
                        try
                        {
                            const auto maybeResult =
                                self->state.connPtr->FetchAll();
                        }
                        catch (mg::ClientException e)
                        {
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    std::string{"Cant delete edge because "
                                                "internal server error."} +
                                        e.what()));
                        }
                    }
                    if (jsoncons::ObjectMemberIsObject(mutationObject,
                                                       "insertEdge"))
                    {
                        auto start =
                            mutationObject["insertEdge"]["start"].as<int64_t>();
                        if (clientToServerVertexIdMap.contains(start))
                            start = clientToServerVertexIdMap[start];
                        auto end =
                            mutationObject["insertEdge"]["end"].as<int64_t>();
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
                            mutationObject["insertEdge"]["type"].as_string());
                        query.append(jsonToMemGraphQueryObject(
                            mutationObject["insertEdge"]["properties"]));
                        query.append("]->(n2) return r;");

                        if (!self->state.connPtr->Execute(query,
                                                          p2.asConstMap()))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(
                                             self->state.connPtr->session_);
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    "Cant insert because internal server "
                                    "error."));
                        }
                        try
                        {
                            const auto maybeResult =
                                self->state.connPtr->FetchAll();
                            if (maybeResult)
                            {
                                auto oldEdgeId =
                                    mutationObject["insertEdge"]["id"]
                                        .as<int>();
                                auto id = ok::db::memgraph_conns
                                              .getIdFromRelationshipResponse(
                                                  *maybeResult);
                                clientToServerEdgeIdMap.emplace(
                                    std::pair{oldEdgeId, id});
                                auto insertEdgeResult =
                                    jsoncons::ojson::array();
                                insertEdgeResult.push_back(oldEdgeId);
                                insertEdgeResult.push_back(id);
                                txnResult.push_back(insertEdgeResult);
                            }
                        }
                        catch (mg::ClientException e)
                        {
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    std::string{"Cant insert edge because "
                                                "internal server error."} +
                                        e.what()));
                        }
                    }
                    if (jsoncons::ObjectMemberIsObject(mutationObject,
                                                       "replaceEdge"))
                    {
                        int id =
                            mutationObject["replaceEdge"]["id"].as<int64_t>();
                        if (clientToServerEdgeIdMap.contains(id))
                            id = clientToServerEdgeIdMap[id];
                        auto start = mutationObject["replaceEdge"]["start"]
                                         .as<int64_t>();
                        if (clientToServerVertexIdMap.contains(start))
                            start = clientToServerVertexIdMap[start];
                        auto end =
                            mutationObject["replaceEdge"]["end"].as<int64_t>();
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
                            mutationObject["replaceEdge"]["type"].as_string());
                        query.append("]-> (n2) where id(r) = $id set r = ");
                        query.append(jsonToMemGraphQueryObject(
                            mutationObject["replaceEdge"]["properties"]));
                        query.append(" return r;");

                        if (!self->state.connPtr->Execute(query,
                                                          p2.asConstMap()))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(
                                             self->state.connPtr->session_);
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    "Cant replace edge properties because "
                                    "internal server error."));
                        }
                        try
                        {
                            const auto maybeResult =
                                self->state.connPtr->FetchAll();
                        }
                        catch (mg::ClientException e)
                        {
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    std::string{"Cant insert because internal "
                                                "server error."} +
                                        e.what()));
                        }
                    }
                    if (jsoncons::ObjectMemberIsObject(mutationObject,
                                                       "replaceEdgeStart"))
                    {
                        int id = mutationObject["replaceEdgeStart"]["id"]
                                     .as<int64_t>();
                        if (clientToServerEdgeIdMap.contains(id))
                            id = clientToServerEdgeIdMap[id];
                        auto start = mutationObject["replaceEdgeStart"]["start"]
                                         .as<int64_t>();
                        if (clientToServerVertexIdMap.contains(start))
                            start = clientToServerVertexIdMap[start];

                        ok::db::MGParams p2{
                            {"edgeid", mg_value_make_integer(id)},
                            {"newstart", mg_value_make_integer(start)},
                        };
                        std::string query{"match ()-[r:"};
                        query.append(mutationObject["replaceEdgeStart"]["type"]
                                         .as_string());
                        query.append(
                            "]->() "
                            "WHERE id(r) = $edgeid "
                            "match (s) where id(s) = $newstart "
                            "match (e) where id(e) = id(endNode(r)) "
                            "merge (s)-[t:");
                        query.append(mutationObject["replaceEdgeStart"]["type"]
                                         .as_string());
                        query.append(
                            "]-> (e)"
                            "set t = properties(r)"
                            "delete r"
                            "return t");

                        if (!self->state.connPtr->Execute(query,
                                                          p2.asConstMap()))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(
                                             self->state.connPtr->session_);
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    "Cant insert because internal server "
                                    "error."));
                        }
                        try
                        {
                            const auto maybeResult =
                                self->state.connPtr->FetchAll();
                        }
                        catch (mg::ClientException e)
                        {
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    std::string{
                                        "Cant replace edge start because "
                                        "internal server error."} +
                                        e.what()));
                        }
                    }
                    if (jsoncons::ObjectMemberIsObject(mutationObject,
                                                       "replaceEdgeEnd"))
                    {
                        int id = mutationObject["replaceEdgeEnd"]["id"]
                                     .as<int64_t>();
                        if (clientToServerEdgeIdMap.contains(id))
                            id = clientToServerEdgeIdMap[id];
                        auto end = mutationObject["replaceEdgeEnd"]["end"]
                                       .as<int64_t>();
                        if (clientToServerVertexIdMap.contains(end))
                            end = clientToServerVertexIdMap[end];
                        ok::db::MGParams p2{{"edgeid",
                                             mg_value_make_integer(id)},
                                            {"newend",
                                             mg_value_make_integer(end)}};
                        std::string query{"match ()-[r:"};
                        auto type = mutationObject["replaceEdgeEnd"]["type"]
                                        .as_string();
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
                        if (!self->state.connPtr->Execute(query,
                                                          p2.asConstMap()))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(
                                             self->state.connPtr->session_);
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    "Cant replace edge end because internal "
                                    "server error."));
                        }
                        try
                        {
                            // this is required to execute query:
                            const auto maybeResult =
                                self->state.connPtr->FetchAll();
                        }
                        catch (mg::ClientException e)
                        {
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    std::string{"Cant replace edge end because "
                                                "internal server error."} +
                                        e.what()));
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
                            mutationObject["deleteEdge"]["start"].as<int64_t>();
                        if (clientToServerVertexIdMap.contains(start))
                            start = clientToServerVertexIdMap[start];
                        auto end =
                            mutationObject["deleteEdge"]["end"].as<int64_t>();
                        if (clientToServerVertexIdMap.contains(end))
                            end = clientToServerVertexIdMap[end];

                        ok::db::MGParams p2{{"idstart",
                                             mg_value_make_integer(start)},
                                            {"idend",
                                             mg_value_make_integer(end)},
                                            {"id", mg_value_make_integer(id)}};
                        std::string query{"match (n1)-[r:"};
                        query.append(
                            mutationObject["deleteEdge"]["type"].as_string());
                        query.append(
                            "]->(n2) where id(n1) = $idstart and id(n2) = "
                            "$idend and id(r) = $id "
                            "delete r;");

                        if (!self->state.connPtr->Execute(query,
                                                          p2.asConstMap()))
                        {
                            LOG_DEBUG << "Failed to execute query!" << query
                                      << " "
                                      << mutationObject["deleteEdge"]["type"]
                                             .as_cstring()
                                      << mg_session_error(
                                             self->state.connPtr->session_);
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    "Cant delete edge because internal server "
                                    "error."));
                        }
                        try
                        {
                            const auto maybeResult =
                                self->state.connPtr->FetchAll();
                        }
                        catch (mg::ClientException e)
                        {
                            // TODO: report error to db, reason, status code
                            // everything;
                            self->state.connPtr->RollbackTransaction();
                            return self->send(
                                connectionActor,
                                caf::forward_atom_v,
                                ok::smart_actor::connection::addFailure(
                                    responseResult,
                                    event,
                                    std::string{"Cant delete edge because "
                                                "internal server error."} +
                                        e.what()));
                        }
                    }
                }
                auto o = jsoncons::ojson::array();
                o.push_back(txns[0]);
                o.push_back(txnResult);
                resultTxn.push_back(o);

                self->state.connPtr->CommitTransaction();
                self->state.connPtr->Finalize();
            }

            ok::smart_actor::connection::addEventAndJson(responseResult,
                                                         event,
                                                         resultTxn);
            self->send(connectionActor, caf::forward_atom_v, responseResult);
        },
        [=](shutdown_atom) { self->unbecome(); }};
}
}  // namespace supervisor
}  // namespace ok::smart_actor
