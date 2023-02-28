// #include "backward.hpp"
#include <drogon/drogon.h>
#include "actor_system/CAF.hpp"
#include "utils/os_functions.hpp"
#include "Api.hpp"
#include "actor_system/WSEventMap.hpp"
#include "db/mgclientPool.hpp"
#include <numeric>  // std::accumulate
#include "utils/mg_helper.hpp"
namespace
{
void loadDrogonConfig(const std::string& file)
{
    drogon::app().loadConfigFile(file);
    std::vector<std::string> paths;
    paths.push_back(drogon::app().getDocumentRoot());
    ok::utils::file::assertDirectoryExist(paths);
    ok::smart_actor::connection::impl::addAuthRoutes();
    ok::smart_actor::connection::impl::addSyncRoutes();
    ok::smart_actor::connection::impl::addMutateRoutes();
}
void runActorFramework()
{
    // ok::db::seed::newUserDatabase();
    // ok::smart_actor::connection::fetch();
    ok::smart_actor::supervisor::initialiseMainActor();
    caf::anon_send(ok::smart_actor::supervisor::mainActor,
                   spawn_and_monitor_atom_v);
}
void runDrogon(int port)
{
    ok::api::registerAuthApi();
    ok::api::registerApi();
    ok::api::registerRegexApi();
    drogon::app().addListener("0.0.0.0", port).run();
}
// jsoncons::ojson memGrapMapToJson(std::vector<std::vector<mg::Value>>&
// response) {

//}

void memGraphTest()
{
    //    auto response =
    //        ok::db::memgraph_conns.request("MATCH (n)-[r]->(m) RETURN
    //        n,r,m;");
    //    for (auto& row : response)
    //    {
    //        for (auto& matchPart : row)
    //        {
    //            switch (matchPart.type())
    //            {
    //                case mg::Value::Type::Null:
    //                    LOG_DEBUG << "null";
    //                    break;
    //                case mg::Value::Type::Bool:
    //                    LOG_DEBUG << matchPart.ValueBool();
    //                    break;
    //                case mg::Value::Type::Int:
    //                    LOG_DEBUG << matchPart.ValueInt();
    //                    break;
    //                case mg::Value::Type::Double:
    //                    LOG_DEBUG << matchPart.ValueDouble();
    //                    break;
    //                case mg::Value::Type::String:
    //                    LOG_DEBUG << matchPart.ValueString();
    //                    break;
    //                case mg::Value::Type::List:
    //                    LOG_DEBUG << "List";
    //                    break;
    //                case mg::Value::Type::Map:
    //                    LOG_DEBUG << "Map";
    //                    break;
    //                case mg::Value::Type::Node:
    //                {
    //                    const auto json =
    //                        convertNodeToJson(std::move(matchPart.ValueNode()));
    //                    LOG_DEBUG << json.to_string();
    //                    break;
    //                }
    //                case mg::Value::Type::Relationship:
    //                {
    //                    const auto json = convertRelationshipToJson(
    //                        std::move(matchPart.ValueRelationship()));
    //                    LOG_DEBUG << json.to_string();
    //                    break;
    //                }
    //                case mg::Value::Type::UnboundRelationship:
    //                    LOG_DEBUG << "UnboundRelationship";
    //                    break;
    //                case mg::Value::Type::Path:
    //                    LOG_DEBUG << "Path";
    //                    break;
    //                case mg::Value::Type::Date:
    //                    LOG_DEBUG << "Date";
    //                    break;
    //                case mg::Value::Type::Time:
    //                    LOG_DEBUG << "Time";
    //                    break;
    //                case mg::Value::Type::LocalTime:
    //                    LOG_DEBUG << "LocalTime";
    //                    break;
    //                case mg::Value::Type::DateTime:
    //                    LOG_DEBUG << "DateTime";
    //                    break;
    //                case mg::Value::Type::DateTimeZoneId:
    //                    LOG_DEBUG << "DateTimeZoneId";
    //                    break;
    //                case mg::Value::Type::LocalDateTime:
    //                    LOG_DEBUG << "LocalDateTime";
    //                    break;
    //                case mg::Value::Type::Duration:
    //                    LOG_DEBUG << "Duration";
    //                    break;
    //                case mg::Value::Type::Point2d:
    //                    LOG_DEBUG << "Point2d";
    //                    break;
    //                case mg::Value::Type::Point3d:
    //                    LOG_DEBUG << "Point3d";
    //                    break;
    //            }
    //        }
    //    }
}
void setupConstrains()
{
    //    auto response = ok::db::memgraph_conns.request(
    //        "CREATE CONSTRAINT ON (u:User) ASSERT u.email IS UNIQUE");
}
}  // namespace
int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        LOG_DEBUG << "Arguments should be > 2, first is config file and"
                     " second is mg port third is server port that is exposed";
        exit(1);
    }
    global_var::mg_port = std::stoi(std::string{argv[2]});
    int port = std::stoi(std::string{argv[3]});
    loadDrogonConfig(argv[1]);
    // ok::smart_actor::connection::setGlobalVariables();
    //  memGraphTest();
    setupConstrains();
    runActorFramework();
    runDrogon(port);
}
