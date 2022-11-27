//#include "backward.hpp"
#include <drogon/drogon.h>
#include "actor_system/CAF.hpp"
#include "utils/os_functions.hpp"
#include "Api.hpp"
#include "actor_system/WSEventMap.hpp"
#include "db/mgclientPool.hpp"
#include <numeric>  // std::accumulate
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
  caf::anon_send(ok::smart_actor::supervisor::mainActor, spawn_and_monitor_atom_v);
}
void runDrogon()
{
  ok::api::registerApi();
  ok::api::registerRegexApi();
  drogon::app().run();
}
//jsoncons::ojson memGrapMapToJson(std::vector<std::vector<mg::Value>>& response) {

//}
jsoncons::ojson memGraphMapToJson(mg::ConstMap map);
jsoncons::ojson convertNodeToJson(mg::ConstNode node);
jsoncons::ojson convertRelationshipToJson(mg::ConstRelationship relationship);
jsoncons::ojson convertListToJson(mg::ConstList list){
    auto array = jsoncons::ojson::array();
    for (const auto &item : list) {
          switch (item.type())
          {
            case mg::Value::Type::Null: array.emplace_back(jsoncons::ojson::null()); break;
            case mg::Value::Type::Bool: array.emplace_back(item.ValueBool()); break;
            case mg::Value::Type::Int: array.emplace_back(item.ValueInt()); break;
            case mg::Value::Type::Double: array.emplace_back(item.ValueDouble()); break;
            case mg::Value::Type::String:
              array.emplace_back(item.ValueString());; break;
              break;
          case mg::Value::Type::List:{
             array.emplace_back(convertListToJson(item.ValueList())); break;
             break;
          }
            case mg::Value::Type::Map: {
                array.emplace_back(memGraphMapToJson(item.ValueMap())); break;
             break;
          }
            case mg::Value::Type::Node:
            {
              array.emplace_back(convertNodeToJson(std::move(item.ValueNode())));
              break;
            }
          case mg::Value::Type::Relationship: {
              const auto json = convertRelationshipToJson(std::move(item.ValueRelationship()));
              break;
          }
            case mg::Value::Type::UnboundRelationship: LOG_DEBUG << "UnboundRelationship"; break;
            case mg::Value::Type::Path: LOG_DEBUG << "Path"; break;
            case mg::Value::Type::Date: LOG_DEBUG << "Date"; break;
            case mg::Value::Type::Time: LOG_DEBUG << "Time"; break;
            case mg::Value::Type::LocalTime: LOG_DEBUG << "LocalTime"; break;
            case mg::Value::Type::DateTime: LOG_DEBUG << "DateTime"; break;
            case mg::Value::Type::DateTimeZoneId: LOG_DEBUG << "DateTimeZoneId"; break;
            case mg::Value::Type::LocalDateTime: LOG_DEBUG << "LocalDateTime"; break;
            case mg::Value::Type::Duration: LOG_DEBUG << "Duration"; break;
            case mg::Value::Type::Point2d: LOG_DEBUG << "Point2d"; break;
            case mg::Value::Type::Point3d: LOG_DEBUG << "Point3d"; break;
          }
    }
    return array;
}
jsoncons::ojson memGraphMapToJson(mg::ConstMap map) {
    jsoncons::ojson properties;

    for (const auto& [key, value] : map)
    {
      switch (value.type())
      {
        case mg::Value::Type::Null:
        {
          properties[key] = jsoncons::ojson::null();
          break;
        }
        case mg::Value::Type::Bool:
        {
          properties[key] = value.ValueBool();
          break;
        }
        case mg::Value::Type::Int:
        {
          properties[key] = value.ValueInt();
          break;
        }
        case mg::Value::Type::Double:
        {
          properties[key] = value.ValueDouble();
          break;
        }
        case mg::Value::Type::String:
        {
          properties[key] = value.ValueString();
          break;
        }
        case mg::Value::Type::List:
        {
          properties[key] = convertListToJson(value.ValueList());
          break;
        }
        case mg::Value::Type::Map:
        {
          properties[key] = memGraphMapToJson(std::move(value.ValueMap()));
          break;
        }
        case mg::Value::Type::Node:
        {
          properties[key] = convertNodeToJson(std::move(value.ValueNode()));
          break;
        }
        case mg::Value::Type::Relationship:
        {
          properties[key] = convertRelationshipToJson(std::move(value.ValueRelationship()));
          break;
        }
        case mg::Value::Type::UnboundRelationship:
        {
          break;
        }
        case mg::Value::Type::Path:
        {
          break;
        }
        case mg::Value::Type::Date:
        {
  //        properties[key] = value.ValueDate();
          break;
        }
        case mg::Value::Type::Time:
        {
  //        properties[key] = value.ValueDateTime();
          break;
        }
        case mg::Value::Type::LocalTime:
        {
  //        properties[key] = value.ValueLocalTime();
          break;
        }
        case mg::Value::Type::DateTime:
        {
  //        properties[key] = value.ValueLocalDateTime();
        }
        case mg::Value::Type::DateTimeZoneId:
        {
  //        properties[key] = value.ValueDateTimeZoneId();
        }
        case mg::Value::Type::LocalDateTime:
        {
  //        properties[key] = value.ValueLocalDateTime();
        }
        case mg::Value::Type::Duration:
        {
  //        properties[key] = value.ValueDuration();
        }
        case mg::Value::Type::Point2d:
        {
  //        properties[key] = value.ValuePoint2d();
        }
        case mg::Value::Type::Point3d:
        {
  //        properties[key] = value.ValuePoint3d();
        }
      }
    }
    return properties;
}
jsoncons::ojson convertNodeToJson(mg::ConstNode node)
{
  jsoncons::ojson json;
  json["id"] = node.id().AsInt();
  auto array = jsoncons::ojson::array();
  for (auto const& label : node.labels()) { array.push_back(label.data()); }
  json["labels"] = array;
  json["properties"] = memGraphMapToJson(std::move(node.properties()));
  return json;
}
jsoncons::ojson convertRelationshipToJson(mg::ConstRelationship relationship)
{
  jsoncons::ojson json;
  json["id"] = relationship.id().AsInt();
  json["from"] = relationship.from().AsInt();
  json["to"] = relationship.to().AsInt();
  json["type"] = relationship.type();
  json["properties"] = memGraphMapToJson(std::move(relationship.properties()));
  return json;
}
}  // namespace
int main(int argc, char* argv[])
{
  if (argc < 1) LOG_DEBUG << "Arguments should be > 1";
  loadDrogonConfig(argv[1]);
  // ok::smart_actor::connection::setGlobalVariables();
  ok::db::initializeMemGraphPool(8);
  auto response = ok::db::memgraph_conns.requestDataRaw("", "MATCH (n)-[r]->(m) RETURN n,r,m;");
  for (auto& row : response)
  {
    for (auto& matchPart : row)
    {
      switch (matchPart.type())
      {
        case mg::Value::Type::Null: LOG_DEBUG << "null"; break;
        case mg::Value::Type::Bool: LOG_DEBUG << matchPart.ValueBool(); break;
        case mg::Value::Type::Int: LOG_DEBUG << matchPart.ValueInt(); break;
        case mg::Value::Type::Double: LOG_DEBUG << matchPart.ValueDouble(); break;
        case mg::Value::Type::String:
          LOG_DEBUG << matchPart.ValueString();
          break;
        case mg::Value::Type::List: LOG_DEBUG << "List"; break;
        case mg::Value::Type::Map: LOG_DEBUG << "Map"; break;
        case mg::Value::Type::Node:
        {

          const auto json = convertNodeToJson(std::move(matchPart.ValueNode()));
          LOG_DEBUG << json.to_string();
          break;
        }
      case mg::Value::Type::Relationship: {
          const auto json = convertRelationshipToJson(std::move(matchPart.ValueRelationship()));
          LOG_DEBUG << json.to_string();
          break;
      }
        case mg::Value::Type::UnboundRelationship: LOG_DEBUG << "UnboundRelationship"; break;
        case mg::Value::Type::Path: LOG_DEBUG << "Path"; break;
        case mg::Value::Type::Date: LOG_DEBUG << "Date"; break;
        case mg::Value::Type::Time: LOG_DEBUG << "Time"; break;
        case mg::Value::Type::LocalTime: LOG_DEBUG << "LocalTime"; break;
        case mg::Value::Type::DateTime: LOG_DEBUG << "DateTime"; break;
        case mg::Value::Type::DateTimeZoneId: LOG_DEBUG << "DateTimeZoneId"; break;
        case mg::Value::Type::LocalDateTime: LOG_DEBUG << "LocalDateTime"; break;
        case mg::Value::Type::Duration: LOG_DEBUG << "Duration"; break;
        case mg::Value::Type::Point2d: LOG_DEBUG << "Point2d"; break;
        case mg::Value::Type::Point3d: LOG_DEBUG << "Point3d"; break;
      }
    }
  }
  runActorFramework();
  runDrogon();
}
