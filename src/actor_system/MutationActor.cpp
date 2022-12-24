#include "actor_system/MutationActor.hpp"
#include <magic_enum.hpp>
#include "db/mgclientPool.hpp"
#include "third_party/mgclient/src/mgvalue.h"
#include "utils/BatchArrayMessage.hpp"
#include "utils/json_functions.hpp"
#include "utils/mg_helper.hpp"
#include "jsoncons/json_encoder2.hpp"
namespace ok::smart_actor
{
namespace supervisor
{
std::basic_string<char> to_string2(jsoncons::ojson const &json)
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
// note: mutation action can save all txns and revert back to any point in time!
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
    return {
        [=](create_atom,
            VertexId const &memberKey,
            WsArguments const &args,
            ws_connector_actor_int connectionActor) {
            auto conn = ok::db::memgraph_conns.getDBConnection();
            std::map<int, int> clientToServerVertexIdMap;
            std::map<int, int> clientToServerEdgeIdMap;

            if (!conn)
            {
                std::cerr << "Failed to connect!\n";
                return;
            }
            auto resultTxn = jsoncons::ojson::array();

            for (auto const &txns : args.array_range())
            {
                if (txns.is_array() && txns.size() != 4)
                    continue;
                for (auto const &vertexOldIdToNewIdEntry :
                     txns[1].array_range())
                {
                    if (vertexOldIdToNewIdEntry.is_array() &&
                        vertexOldIdToNewIdEntry.size() != 2)
                        continue;
                    clientToServerVertexIdMap.emplace(
                        vertexOldIdToNewIdEntry[0].as<int>(),
                        vertexOldIdToNewIdEntry[1].as<int>());
                }
                for (auto const &edgeOldIdToNewIdEntry : txns[2].array_range())
                {
                    if (edgeOldIdToNewIdEntry.is_array() &&
                        edgeOldIdToNewIdEntry.size() != 2)
                        continue;
                    clientToServerEdgeIdMap.emplace(
                        edgeOldIdToNewIdEntry[0].as<int>(),
                        edgeOldIdToNewIdEntry[1].as<int>());
                }

                bool isTransactionSuccss{true};
                conn->BeginTransaction();
                auto txnResult = jsoncons::ojson::array();
                for (auto const &mutationObject : txns[3].array_range())
                {
                    if (jsoncons::ObjectMemberIsObject(mutationObject,
                                                       "insert"))
                    {
                        std::string query{"CREATE (n"};
                        if (jsoncons::ObjectMemberIsArray(
                                mutationObject["insert"], "labels"))
                        {
                            std::string labels;
                            for (auto const &label :
                                 mutationObject["insert"]["labels"]
                                     .array_range())
                            {
                                if (label.is_string())
                                {
                                    labels.append(":");
                                    labels.append(label.as_string());
                                }
                            }
                            query.append(labels);
                            query.append(to_string2(
                                mutationObject["insert"]["properties"]));
                            query.append(") ");
                            query.append(" RETURN n;");
                        }
                        LOG_DEBUG << query;
                        if (!conn->Execute(query))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(conn->session_);
                            //        return 1;
                            // Todo Think about exit points
                        }
                        const auto maybeResult = conn->FetchAll();
                        if (maybeResult)
                        {
                            auto oldId =
                                mutationObject["insert"]["id"].as<int>();
                            auto id = ok::db::memgraph_conns.getIdFromResponse(
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
                    if (jsoncons::ObjectMemberIsObject(mutationObject,
                                                       "replace"))
                    {
                        // https://memgraph.com/docs/cypher-manual/clauses/set#7-replace-all-properties-using-map
                        //                    MATCH (c:Country {name:
                        //                    'Germany'}) SET c = {name:
                        //                    'Germany', population: '85000000'}
                        //                    RETURN c;
                        // MATCH (u) WHERE ID(u) = $id RETURN u
                        int id = mutationObject["replace"]["id"].as<int64_t>();
                        if (clientToServerVertexIdMap.contains(id))
                            id = clientToServerVertexIdMap[id];
                        ok::db::MGParams p2{{"id", mg_value_make_integer(id)}};
                        std::string query{
                            "MATCH (n) WHERE ID(n) = $id SET n = "};
                        query.append(to_string2(
                            mutationObject["replace"]["properties"]));
                        query.append(" RETURN n;");

                        LOG_DEBUG << query;
                        if (!conn->Execute(query, p2.asConstMap()))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(conn->session_);
                            //        return 1;
                            // Todo Think about exit points
                        }
                        const auto maybeResult = conn->FetchAll();
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

                        LOG_DEBUG << query;
                        if (!conn->Execute(query, p2.asConstMap()))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(conn->session_);
                            //        return 1;
                            // Todo Think about exit points
                        }
                        try
                        {
                            const auto maybeResult = conn->FetchAll();
                        }
                        catch (mg::ClientException e)
                        {
                            jsoncons::ojson insertResult;
                            jsoncons::ojson result;
                            result["error"] = true;
                            result["message"] = e.what();
                            result["deleteVertex"] = result;
                            txnResult.push_back(insertResult);
                            conn->RollbackTransaction();
                            isTransactionSuccss = false;
                            break;
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
                        query.append(to_string2(
                            mutationObject["insertEdge"]["properties"]));
                        query.append("]->(n2) return r;");
                        LOG_DEBUG << query;
                        if (!conn->Execute(query, p2.asConstMap()))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(conn->session_);
                            //        return 1;
                            // Todo Think about exit points
                        }
                        const auto maybeResult = conn->FetchAll();
                        if (maybeResult)
                        {
                            auto oldEdgeId =
                                mutationObject["insertEdge"]["id"].as<int>();
                            auto id = ok::db::memgraph_conns
                                          .getIdFromRelationshipResponse(
                                              *maybeResult);
                            clientToServerEdgeIdMap.emplace(
                                std::pair{oldEdgeId, id});
                            auto insertEdgeResult = jsoncons::ojson::array();
                            insertEdgeResult.push_back(oldEdgeId);
                            insertEdgeResult.push_back(id);
                            txnResult.push_back(insertEdgeResult);
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

                        ok::db::MGParams p2{
                            {"idstart", mg_value_make_integer(start)},
                            {"idend", mg_value_make_integer(end)},
                            {"type",
                             mg_value_make_string(
                                 mutationObject["replaceEdge"]["type"]
                                     .as_cstring())}};
                        std::string query{
                            "match (n1) where id(n1)=$idstart  match (n2) "
                            "where "
                            "id(n2)=$idend merge (n1)-[r:$type "};
                        query.append(to_string2(
                            mutationObject["replaceEdge"]["properties"]));
                        query.append(" ]-> (n2) return r;");
                        LOG_DEBUG << query;
                        if (!conn->Execute(query, p2.asConstMap()))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(conn->session_);
                            //        return 1;
                            // Todo Think about exit points
                        }
                        const auto maybeResult = conn->FetchAll();
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
                            {"type",
                             mg_value_make_string(
                                 mutationObject["replaceEdgeStart"]["type"]
                                     .as_cstring())}};
                        std::string query{
                            "match ()-[r:$type]->()"
                            "WHERE id(r) = $edgeid"
                            "match (s) where id(s) = $newstart"
                            "match (e) where id(e) = id(endNode(r))"
                            "merge (s)-[t:$type]-> (e)"
                            "set t = properties(r)"
                            "delete r"
                            "return t"};
                        LOG_DEBUG << query;
                        if (!conn->Execute(query, p2.asConstMap()))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(conn->session_);
                            //        return 1;
                            // Todo Think about exit points
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
                        ok::db::MGParams p2{
                            {"edgeid", mg_value_make_integer(id)},
                            {"newend", mg_value_make_integer(end)},
                            {"type",
                             mg_value_make_string(
                                 mutationObject["replaceEdgeEnd"]["type"]
                                     .as_cstring())}};
                        std::string query{
                            "match ()-[r:$type]->()"
                            "WHERE id(r) = $edgeid"
                            "match (s) where id(s) = id(startNode(r))"
                            "match (e) where id(e) = $newend"
                            "merge (s)-[t:$type]-> (e)"
                            "set t = properties(r)"
                            "delete r"
                            "return t"};
                        LOG_DEBUG << query;
                        if (!conn->Execute(query, p2.asConstMap()))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(conn->session_);
                            //        return 1;
                            // Todo Think about exit points
                        }
                    }
                    if (jsoncons::ObjectMemberIsObject(mutationObject,
                                                       "deleteEdge"))
                    {
                        int id =
                            mutationObject["deleteEdge"]["id"].as<int64_t>();
                        if (clientToServerEdgeIdMap.contains(id))
                            id = clientToServerEdgeIdMap[id];
                        ok::db::MGParams p2{
                            {"id", mg_value_make_integer(id)},
                            {"type",
                             mg_value_make_string(
                                 mutationObject["deleteEdge"]["type"]
                                     .as_cstring())}};
                        std::string query{
                            "match (n1)-[r:$type]->(n2) where id(r) = $id "
                            "delete "
                            "r;"};
                        LOG_DEBUG << query;
                        if (!conn->Execute(query, p2.asConstMap()))
                        {
                            std::cerr << "Failed to execute query!" << query
                                      << " "
                                      << mg_session_error(conn->session_);
                            //        return 1;
                            // Todo Think about exit points
                        }
                        const auto maybeResult = conn->FetchAll();
                    }
                }
                auto o = jsoncons::ojson::array();
                o.push_back(txns[0]);
                o.push_back(txnResult);
                resultTxn.push_back(o);
                if (isTransactionSuccss)
                    conn->CommitTransaction();
                else
                {
                    ok::db::memgraph_conns.deleteBadDBConnection(conn);
                    ok::db::memgraph_conns.newConnection();
                    conn = ok::db::memgraph_conns.getDBConnection();
                }
            }
            //                    if (!conn->Execute(body))
            //                    {
            //                        std::cerr << "Failed to execute
            //                        query!";
            //                        //        return 1;
            //                    }
            //                    std::vector<std::vector<mg::Value>>
            //                    resp; while (const auto maybeResult =
            //                    conn->FetchOne())
            //                    {
            //                        resp.emplace_back(std::move(*maybeResult));
            //                    }
            ok::db::memgraph_conns.freeDBConnection(conn);
            //                return resp;

            // if (!argsSchemaCheck(args)) return false;
            /*if (!self->state.wsActorArgs.contains(connectionActor))
            {
                self->state.wsActorArgs.emplace(
                    std::make_pair(connectionActor, args));
            }
            jsoncons::ojson nodes = jsoncons::ojson::array();
            jsoncons::ojson relationships = jsoncons::ojson::array();
            auto response1 =
                ok::db::memgraph_conns.request("MATCH (n) RETURN n;");
            auto response2 =
                ok::db::memgraph_conns.request("MATCH ()-[r]->() RETURN
            r;"); for (auto &row : response1) for (auto &matchPart : row) if
            (matchPart.type() == mg::Value::Type::Node) nodes.push_back(
                            convertNodeToJson(matchPart.ValueNode()));
            for (auto &row : response2)
                for (auto &matchPart : row)
                    if (matchPart.type() == mg::Value::Type::Relationship)
                        relationships.push_back(convertRelationshipToJson(
                            matchPart.ValueRelationship()));
            jsoncons::ojson result;
            result["vertexes"] = nodes;
            result["edges"] = relationships;
            auto msg = ok::smart_actor::connection::wsMessageBase();
            ok::smart_actor::connection::addEventAndJson(msg, event,
            result); self->send(connectionActor, caf::forward_atom_v,
            msg);*/
            auto responseResult = ok::smart_actor::connection::wsMessageBase();
            auto event = jsoncons::ojson::array();
            event.push_back("post");
            event.push_back("mutate");
            event.push_back(0);
            ok::smart_actor::connection::addEventAndJson(responseResult,
                                                         event,
                                                         resultTxn);
            self->send(connectionActor, caf::forward_atom_v, responseResult);
        },
        [=](shutdown_atom) { self->unbecome(); }};
}
}  // namespace supervisor
}  // namespace ok::smart_actor
