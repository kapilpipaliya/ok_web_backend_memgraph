#include "lib/mg_helper.hpp"
#include "trantor/utils/Logger.h"

jsoncons::ojson convertListToJson(mg::ConstList list)
{
    auto array = jsoncons::ojson::array();
    for (const auto& item : list)
    {
        switch (item.type())
        {
            case mg::Value::Type::Null:
                array.emplace_back(jsoncons::ojson::null());
                break;
            case mg::Value::Type::Bool:
                array.emplace_back(item.ValueBool());
                break;
            case mg::Value::Type::Int:
                array.emplace_back(item.ValueInt());
                break;
            case mg::Value::Type::Double:
                array.emplace_back(item.ValueDouble());
                break;
            case mg::Value::Type::String:
                array.emplace_back(item.ValueString());
                ;
                break;
                break;
            case mg::Value::Type::List:
            {
                array.emplace_back(convertListToJson(item.ValueList()));
                break;
                break;
            }
            case mg::Value::Type::Map:
            {
                array.emplace_back(memGraphMapToJson(item.ValueMap()));
                break;
                break;
            }
            case mg::Value::Type::Node:
            {
                array.emplace_back(
                    convertNodeToJson(std::move(item.ValueNode())));
                break;
            }
            case mg::Value::Type::Relationship:
            {
                const auto json = convertRelationshipToJson(
                    std::move(item.ValueRelationship()));
                break;
            }
            case mg::Value::Type::UnboundRelationship:
                LOG_ERROR << "UnboundRelationship";
                break;
            case mg::Value::Type::Path:
                LOG_ERROR << "Path";
                break;
            case mg::Value::Type::Date:
                LOG_ERROR << "Date";
                break;
            case mg::Value::Type::Time:
                LOG_ERROR << "Time";
                break;
            case mg::Value::Type::LocalTime:
                LOG_ERROR << "LocalTime";
                break;
            case mg::Value::Type::DateTime:
                LOG_ERROR << "DateTime";
                break;
            case mg::Value::Type::DateTimeZoneId:
                LOG_ERROR << "DateTimeZoneId";
                break;
            case mg::Value::Type::LocalDateTime:
                LOG_ERROR << "LocalDateTime";
                break;
            case mg::Value::Type::Duration:
                LOG_ERROR << "Duration";
                break;
            case mg::Value::Type::Point2d:
                LOG_ERROR << "Point2d";
                break;
            case mg::Value::Type::Point3d:
                LOG_ERROR << "Point3d";
                break;
        }
    }
    return array;
}
jsoncons::ojson memGraphMapToJson(mg::ConstMap map)
{
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
                properties[key] =
                    memGraphMapToJson(std::move(value.ValueMap()));
                break;
            }
            case mg::Value::Type::Node:
            {
                properties[key] =
                    convertNodeToJson(std::move(value.ValueNode()));
                break;
            }
            case mg::Value::Type::Relationship:
            {
                properties[key] = convertRelationshipToJson(
                    std::move(value.ValueRelationship()));
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
                properties[key] = value.ValueDate().days();
                break;
            }
            case mg::Value::Type::Time:
            {
                jsoncons::ojson val;
                val["nanosec"] = value.ValueTime().nanoseconds();
                val["tzsec"] = value.ValueTime().tz_offset_seconds();
                properties[key] = val;
                break;
            }
            case mg::Value::Type::LocalTime:
            {
                properties[key] = value.ValueLocalTime().nanoseconds();
                break;
            }
            case mg::Value::Type::DateTime:
            {
                jsoncons::ojson val;
                val["nanosec"] = value.ValueDateTime().nanoseconds();
                val["tzmin"] = value.ValueDateTime().tz_offset_minutes();
                properties[key] = val;
                break;
            }
            case mg::Value::Type::DateTimeZoneId:
            {
                jsoncons::ojson val;
                val["nanosec"] = value.ValueDateTimeZoneId().nanoseconds();
                val["tz"] = value.ValueDateTimeZoneId().tzId();
                properties[key] = val;
                break;
            }
            case mg::Value::Type::LocalDateTime:
            {
                properties[key] = value.ValueLocalDateTime().nanoseconds();
                break;
            }
            case mg::Value::Type::Duration:
            {
                jsoncons::ojson val;
                properties["days"] = value.ValueDuration().days();
                properties["month"] = value.ValueDuration().months();
                properties["second"] = value.ValueDuration().seconds();
                properties["nanosec"] = value.ValueDuration().nanoseconds();
                properties[key] = val;
                break;
            }
            case mg::Value::Type::Point2d:
            {
                jsoncons::ojson val;
                val["srid"] = value.ValuePoint2d().srid();
                val["x"] = value.ValuePoint2d().x();
                val["y"] = value.ValuePoint2d().y();
                properties[key] = val;
            }
            case mg::Value::Type::Point3d:
            {
                jsoncons::ojson val;
                properties["srid"] = value.ValuePoint3d().srid();
                properties["x"] = value.ValuePoint3d().x();
                properties["y"] = value.ValuePoint3d().y();
                properties["z"] = value.ValuePoint3d().z();
                properties[key] = val;
                break;
            }
        }
    }
    return properties;
}
// mg::ConstMap jsonToMemGraph(jsoncons::ojson json)
//{
//     mg_map* extra = mg_map_make_empty(json.size());
//     for (auto const& kv : json.object_range())
//     {
//         if (kv.value().is_object())
//         {
//         }
//         else if (kv.value().is_array())
//         {
//         }
//         else if (kv.value().is_bool())
//         {
//         }
//         else if (kv.value().is_double())
//         {
//         }
//         else if (kv.value().is_null())
//         {
//         }
//         else if (kv.value().is_string())
//         {
//         }
//         else if (kv.value().is_bignum())
//         {
//         }
//     }
//     return extra;
// }
jsoncons::ojson convertNodeToJson(mg::ConstNode node)
{
    jsoncons::ojson json;
    json["id"] = node.id().AsInt();
    auto array = jsoncons::ojson::array();
    for (auto const& label : node.labels())
    {
        std::string label2;
        for (auto const& c : label)
        {
            label2 += c;
        }
        array.push_back(label2);
    }
    json["L"] = array;
    json["P"] = memGraphMapToJson(std::move(node.properties()));
    json["OUT"] = jsoncons::ojson::object();
    json["IN"] = jsoncons::ojson::object();
    return json;
}
jsoncons::ojson convertRelationshipToJson(mg::ConstRelationship relationship)
{
    jsoncons::ojson json;
    json["id"] = relationship.id().AsInt();
    json["S"] = relationship.from().AsInt();
    json["E"] = relationship.to().AsInt();
    json["T"] = relationship.type();
    json["P"] = memGraphMapToJson(std::move(relationship.properties()));
    return json;
}
