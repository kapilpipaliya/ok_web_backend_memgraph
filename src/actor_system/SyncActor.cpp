#include "actor_system/SyncActor.hpp"
#include <magic_enum.hpp>
#include "db/mgclientPool.hpp"
#include "third_party/mgclient/src/mgvalue.h"
#include "utils/BatchArrayMessage.hpp"
#include "utils/mg_helper.hpp"
#include "utils/time_functions.hpp"
namespace ok::smart_actor
{
namespace supervisor
{

void fetchNodesAPI(std::string query,
                   ok::db::MGParams &p,
                   jsoncons::ojson &nodes,
                   syncActorState::DbConnectionPtr &connPtr)
{
    if (!connPtr->Execute(query.c_str(), p.asConstMap()))
    {
        LOG_DEBUG << "Failed to execute query!" << query << " "
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
        LOG_DEBUG << e.what();
        reconnect(connPtr);
        // only refetch if needed.
        // fetchNodesAPI(query, nodes, connPtr);
    }
}

void fetchRelationshipsAPI(std::string query,
                           ok::db::MGParams &p,
                           jsoncons::ojson &relationships,
                           syncActorState::DbConnectionPtr &connPtr)
{
    if (!connPtr->Execute(query, p.asConstMap()))
    {
        LOG_DEBUG << "Failed to execute query!" << query << " "
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
        LOG_DEBUG << e.what();
        reconnect(connPtr);
    }
}

sync_actor_int::behavior_type SyncActor(SyncActorPointer self)
{
    self->set_error_handler([](caf::error &err) {
        LOG_DEBUG << "Sync Actor Error :";
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
                LOG_DEBUG << "Sync Actor Exception Error : " << e.what();
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

    // create new connection:
    auto client = mg::Client::Connect(self->state.params);
    if (!client)
    {
        LOG_DEBUG << "Failed to connect MG Server";
    }
    self->state.connPtr = std::move(client);

    return {
        [=](caf::subscribe_atom,
            WsEvent const &event,
            WsArguments const &args,
            VertexId const &memberKey,
            ws_connector_actor_int connectionActor) {
            // if (!argsSchemaCheck(args)) return false;
            if (!self->state.wsActorArgs.contains(connectionActor))
            {
                self->state.wsActorArgs.emplace(
                    std::make_pair(connectionActor, args));
            }
            jsoncons::ojson nodes = jsoncons::ojson::array();
            jsoncons::ojson relationships = jsoncons::ojson::array();

            // 1. get all coll nodes.
            // 2. if no memberKey send all public nodes and relationships.
            // 3. if memberKey send data that user is allowed to see.
            // 4. if super admin send all data.
            if (memberKey != -1)
            {
                // get all public nodes:
                ok::db::MGParams p2{};
                auto [error, response] = db::mgCall(
                    getAllNodesWithALabel("Coll", "WHERE n.public = true"), p2);
                if (!error.empty())
                {
                    LOG_DEBUG << error;
                }
                else
                {
                    std::vector<std::string> labelVec{"DataType",
                                                      "Comp",
                                                      "Attr",
                                                      "Coll"};
                    auto getIdsFromResponse = db::getIdsFromResponse(*response);
                    for (auto &id : getIdsFromResponse)
                    {
                        labelVec.emplace_back(id);
                    }

                    mg_list *labelList = mg_list_make_empty(labelVec.size());
                    for (auto const &v : labelVec)
                    {
                        mg_list_append(labelList,
                                       mg_value_make_string(v.c_str()));
                    }
                    ok::db::MGParams p1{{"ids", mg_value_make_list(labelList)}};
                    std::string labels{};
                    for (auto &id : labelVec)
                    {
                        labels += ":" + id;
                    }
                    fetchNodesAPI(
                        "MATCH (n) WHERE all(k IN LABELS(n) WHERE k IN $ids) "
                        "RETURN n;",
                        p1,
                        nodes,
                        self->state.connPtr);

                    fetchRelationshipsAPI(
                        "MATCH (n)-[r]->(m) "
                        "WHERE all(k IN LABELS(n) WHERE k IN $ids) AND "
                        "all(k IN LABELS(m) WHERE k IN $ids) "
                        "RETURN r;",
                        p1,
                        relationships,
                        self->state.connPtr);
                }
            }
            else
            {
                ok::db::MGParams p{};
                fetchNodesAPI("MATCH (n) RETURN n;",
                              p,
                              nodes,
                              self->state.connPtr);
                ok::db::MGParams p1{};
                fetchRelationshipsAPI("MATCH ()-[r]->() RETURN r;",
                                      p1,
                                      relationships,
                                      self->state.connPtr);
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
                            jsoncons::ojson(jsoncons::json_array_arg,
                                            {n["id"]});
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

            auto msg = ok::smart_actor::connection::wsMessageBase();
            ok::smart_actor::connection::addEventAndJson(msg, event, result);
            self->send(connectionActor, caf::forward_atom_v, msg);
        },
        [=](create_atom,
            std::vector<VertexId> const &vertexes,
            std::vector<EdgeId> const &edges) {
            jsoncons::ojson nodes = jsoncons::ojson::array();
            jsoncons::ojson relationships = jsoncons::ojson::array();
            mg_list *nodeList = mg_list_make_empty(vertexes.size());
            for (auto const &v : vertexes)
                mg_list_append(nodeList, mg_value_make_integer(v));
            ok::db::MGParams p1{{"ids", mg_value_make_list(nodeList)}};

            // TODO FIx this
            /*auto response1 = ok::db::memgraph_conns.request(
                "MATCH (u) WHERE ID(u) in $ids RETURN u", p1.asConstMap());
            mg_list *edgeList = mg_list_make_empty(vertexes.size());
            for (auto const &e : edges)
                mg_list_append(edgeList, mg_value_make_integer(e));
            ok::db::MGParams p2{{"ids", mg_value_make_list(edgeList)}};
            auto response2 = ok::db::memgraph_conns.request(
                "MATCH ()-[r]->() WHERE ID(u) in $ids RETURN r;",
                p2.asConstMap());
            for (auto &row : response1)
                for (auto &matchPart : row)
                    if (matchPart.type() == mg::Value::Type::Node)
                        nodes.push_back(
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
            jsoncons::ojson event =
                jsoncons::ojson::parse(R"(["get", "sync", 0])");
            ok::smart_actor::connection::addEventAndJson(msg, event, result);
            for (auto const &a : self->state.wsActorArgs)
                self->send(a.first, caf::forward_atom_v, msg);*/
        },
        [=](set_atom,
            std::vector<VertexId> const &vertexes,
            std::vector<EdgeId> const &edges) {
            jsoncons::ojson result;
            result["deleted_vertexes"] = vertexes;
            result["deleted_edges"] = edges;
            auto msg = ok::smart_actor::connection::wsMessageBase();
            jsoncons::ojson event =
                jsoncons::ojson::parse(R"(["get", "sync", 0])");
            ok::smart_actor::connection::addEventAndJson(msg, event, result);
            for (auto const &a : self->state.wsActorArgs)
                self->send(a.first, caf::forward_atom_v, msg);
        },
        [=](remove_atom,
            std::vector<VertexId> const &vertexes,
            std::vector<EdgeId> const &edges) {},
        [=](conn_exit_atom, ws_connector_actor_int ws_actor) {
            self->state.wsActorArgs.erase(ws_actor);
        },
        [=](shutdown_atom) {
            self->state.wsActorArgs.clear();
            self->unbecome();
        }};
}

void reconnect(syncActorState::DbConnectionPtr &connPtr)
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
    }
    else
    {
        LOG_DEBUG << "Failed to connect!";
        return;
    }
}

}  // namespace supervisor
}  // namespace ok::smart_actor
