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
  ok::smart_actor::connection::impl::addMyRoute();
  ok::smart_actor::connection::impl::addAccountRoute();
  ok::smart_actor::connection::impl::addSuperRoutes();
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
void memGrapToJson(std::vector<std::vector<mg::Value>>& response) {}
jsoncons::ojson convertNodeToJson(mg::Value const& node)
{
  jsoncons::ojson json;
  auto valueNode = node.ValueNode();
  json["id"] = valueNode.id().AsInt();
  auto array = jsoncons::ojson::array();
  for (auto const& label : valueNode.labels()) { array.push_back(label.data()); }
  json["labels"] = array;
  jsoncons::ojson properties;
  //  json["properties"] = valueNode.properties();
  for (const auto& [key, value] : valueNode.properties())
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
        //          properties[key] = value.ValueString();
        break;
      }
      case mg::Value::Type::Map:
      {
        break;
      }
      case mg::Value::Type::Node:
      {
        break;
      }
      case mg::Value::Type::Relationship:
      {
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
        properties[key] = value.ValueDate();
        break;
      }
      case mg::Value::Type::Time:
      {
        properties[key] = value.ValueDateTime();
        break;
      }
      case mg::Value::Type::LocalTime:
      {
        properties[key] = value.ValueLocalTime();
        break;
      }
      case mg::Value::Type::DateTime:
      {
        properties[key] = value.ValueLocalDateTime();
      }
      case mg::Value::Type::DateTimeZoneId:
      {
        properties[key] = value.ValueDateTimeZoneId();
      }
      case mg::Value::Type::LocalDateTime:
      {
        properties[key] = value.ValueLocalDateTime();
      }
      case mg::Value::Type::Duration:
      {
        properties[key] = value.ValueDuration();
      }
      case mg::Value::Type::Point2d:
      {
        properties[key] = value.ValuePoint2d();
      }
      case mg::Value::Type::Point3d:
      {
        properties[key] = value.ValuePoint3d();
      }
    }
  }
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
  caf::deep_to_string(response);
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
          //        case mg::Value::Type::List: LOG_DEBUG << e.ValueList();
          //        case mg::Value::Type::Map: LOG_DEBUG << ;
        case mg::Value::Type::List: LOG_DEBUG << "List"; break;
        case mg::Value::Type::Map: LOG_DEBUG << "Map"; break;
        case mg::Value::Type::Node:
        {
          const auto json = convertNodeToJson(matchPart);
          LOG_DEBUG << json.to_string();
          const auto node = matchPart.ValueNode();
          LOG_DEBUG << "NODE ID IS: " << node.id().AsInt();
          auto labels = node.labels();
          std::string labels_str = std::accumulate(labels.begin(), labels.end(), std::string(""), [](const std::string& acc, const std::string_view value) { return acc + ":" + std::string(value); });
          const auto props = node.properties();
          std::string props_str = std::accumulate(props.begin(),
                                                  props.end(),
                                                  std::string("{"),
                                                  [](const std::string& acc, const auto& key_value)
                                                  {
                                                    const auto& [key, value] = key_value;
                                                    std::string value_str;
                                                    if (value.type() == mg::Value::Type::Int) { value_str = std::to_string(value.ValueInt()); }
                                                    else if (value.type() == mg::Value::Type::String) { value_str = value.ValueString(); }
                                                    else if (value.type() == mg::Value::Type::Bool) { value_str = std::to_string(value.ValueBool()); }
                                                    else if (value.type() == mg::Value::Type::Double) { value_str = std::to_string(value.ValueDouble()); }
                                                    else
                                                    {
                                                      std::cerr << "Uncovered converstion from data type to a string" << std::endl;
                                                      std::exit(1);
                                                    }
                                                    return acc + " " + std::string(key) + ": " + value_str;
                                                  }) +
                                  " }";
          std::cout << labels_str << " " << props_str << std::endl;
          break;
        }
        case mg::Value::Type::Relationship: LOG_DEBUG << "Relationship"; break;
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
