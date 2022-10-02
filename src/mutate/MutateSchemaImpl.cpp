#include "mutate/MutateSchemaImpl.hpp"
#include <trantor/utils/Logger.h>
#include <magic_enum.hpp>
#include "mutate/Field.hpp"
#include "actor_system/Routes.hpp"
#include "db/Session.hpp"
#include <absl/strings/str_format.h>
#include "pystring.hpp"
#include "utils/json_functions.hpp"
#include "velocypack/Iterator.h"
#include "mutate/MutateSchema.hpp"
#include "utils/ErrorConstants.hpp"
#include "table/TableSchema.hpp"
#include "table/TableSchemaImpl.hpp"
namespace
{
bool getSecondBoolOrTrue(VPackSlice s)
{
  if (s.isArray() && s.length() > 1 && s[1].isBool()) { return s[1].getBool(); }
  return true;
}
}  // namespace
namespace ok
{
namespace mutate_schema
{
namespace impl
{
std::tuple<ErrorCode, Collection> getCollection(VPackSlice const &schemaJson) noexcept
{
  if (schemaJson.isObject() && schemaJson.hasKey("c")) { return {ok::ErrorCode::ERROR_NO_ERROR, schemaJson.get("c").copyString()}; }
  else
  {
    LOG_DEBUG << "collection must be in schema: ";
    return {ok::ErrorCode::SCHEMA_ERROR, ""};
  }
}
void makePrefixPositionQueryInsertTree(VPackSlice const &obj, Field const &f, jsoncons::ojson const &args, Relation const &tree, Collection const &collection, MutateQueryParts &queryParts) noexcept
{
  std::string parent;
  if (jsoncons::ObjectMemberIsString(args, "parent")) { parent = args["parent"].as_string(); }
  constexpr absl::string_view query_format = R"aql(
let %s_serial = FIRST(
FOR u IN 1..1 %s "%s" %s
  COLLECT
  AGGREGATE maxNo = MAX(TO_NUMBER(u.%s))
  RETURN {%s: (maxNo || 0.0) + 1}
 )
)aql";
  auto direction = tree.direction == TRI_edge_direction_e::TRI_EDGE_IN ? "INBOUND" : "OUTBOUND";
  auto parentId = collection + "/" + parent;
  auto query = absl::StrFormat(query_format, f.selector, direction, parentId, tree.edge, f.selector, f.selector);
  queryParts.prefix.push_back(query);
  queryParts.mergeQueryVarToData.push_back(f.selector + "_serial");
}
void makePrefixPositionQueryInsert(VPackSlice const &obj, Field const &f, MutateQueryParts &queryParts) noexcept
{
  constexpr absl::string_view query_format = R"aql(
let %s_serial = FIRST(FOR u IN @@insCollection
  COLLECT
  AGGREGATE maxNo = MAX(TO_NUMBER(u.%s))
  RETURN {%s: (maxNo || 0.0) + 1}
  )
)aql";
  auto query = absl::StrFormat(query_format, f.selector, f.selector, f.selector);
  queryParts.prefix.push_back(query);
  queryParts.mergeQueryVarToData.push_back(f.selector + "_serial");
}
bool isRelation(Field const &f) noexcept
{
  if (f.type == FieldType::internal_true_edge || f.type == FieldType::multi_select || f.type == FieldType::multi_select_hidden || f.type == FieldType::multi_select_bool_properties) { return true; }
  else
  {
    return false;
  }
}
bool isEmptyStringOrNullArg(WsArguments const &formData, const int c) noexcept { return ((jsoncons::ArrayPosIsString(formData, c) && formData[c].empty()) || formData[c].is_null()); }
bool isSkipDefaultKeys(VPackSlice const &schemaJson) noexcept { return schemaJson.isObject() && schemaJson.hasKey("p") && schemaJson.get("p").isObject() && schemaJson.get("p").hasKey("noKey"); }
bool isFieldSkip(VPackSlice const &obj, Field const &f, bool isUpdate, DocumentKey const &memberKey) noexcept
{
  bool isSkip = false;
  if (obj.hasKey("skip"))
  {
    auto skipJson = obj["skip"];
    if (skipJson.isBool())
    {
      if (skipJson.getBool()) { return true; }
    }
    else if (skipJson.isObject())
    {
      for (auto const &it : arangodb::velocypack::ObjectIterator(skipJson))
      {
        auto key = it.key.copyString();
        if (key == "memberOnly")
        {
          if (it.value.isBoolean())
          {
            if (it.value.getBoolean())
            {
              if (memberKey.empty()) isSkip = true;
              break;
            }
          }
        }
        std::cout << "key: " << it.key.copyString() << ", value: " << &it.value << std::endl;
      }
    }
    else if (skipJson.isArray())
    {
      for (auto const &vit : arangodb::velocypack::ArrayIterator(skipJson))
      {
        if (vit.isArray())
        {
          if (vit.length() > 0)
          {
            if (vit[0].isString())
            {
              auto func = vit[0].copyString();
              if (func == "update")
              {
                if (isUpdate)
                {
                  isSkip = true;
                  break;
                }
              }
            }
          }
        }
      }
    }
  }
  return isSkip;
}
bool isFieldSkipUpdate(VPackSlice const &obj, Field const &f, DocumentKey const &memberKey) noexcept
{
  if (f.type == FieldType::inserted) { return true; }
  return isFieldSkip(obj, f, true, memberKey);
}
auto getKeyField()
{
  Field fieldKey;
  fieldKey.label = "Key";
  fieldKey.id = "_key";
  fieldKey.selector = "_key";
  fieldKey.defaultValue = "";
  fieldKey.type = FieldType::text;
  return fieldKey;
}
auto getRevField()
{
  Field fieldRev;
  fieldRev.label = "Rev";
  fieldRev.id = "_rev";
  fieldRev.selector = "_rev";
  fieldRev.defaultValue = "";
  fieldRev.type = FieldType::text;
  return fieldRev;
}
void addDefaultKeys(Fields &fields, FieldOrder &allIds, VPackSlice const &schemaJson) noexcept
{
  if (!isSkipDefaultKeys(schemaJson))
  {
    auto fieldKey = getKeyField();
    fields.insert({fieldKey.id, fieldKey});
    allIds.push_back("_key");
    auto fieldRev = getRevField();
    fields.insert({fieldRev.id, fieldRev});
    allIds.push_back("_rev");
  }
}
void addDefaultKeysInsert(Fields &fields, FieldOrder &allIds, VPackSlice const &schemaJson) noexcept
{
  if (!isSkipDefaultKeys(schemaJson))
  {
    auto fieldKey = getKeyField();
    fields.insert({fieldKey.id, fieldKey});
    allIds.push_back("_key");
  }
}
namespace process_field
{
std::tuple<ErrorCode, Selector> selector(VPackSlice const &obj) noexcept
{
  if (obj.hasKey("s"))
  {
    auto selector = obj.get("s").copyString();
    if (!selector.empty()) { return {ok::ErrorCode::ERROR_NO_ERROR, selector}; }
  }
  if (obj.hasKey("r")) { return {ok::ErrorCode::ERROR_NO_ERROR, ""}; }
  LOG_DEBUG << "selector key (or relation key) must exist: " << obj.toString();
  return {ok::ErrorCode::SCHEMA_ERROR, ""};
}
std::tuple<ErrorCode, FieldType> type(VPackSlice const &obj) noexcept
{
  if (obj.hasKey("t"))
  {
    auto type = obj.get("t").copyString();
    if (auto type_magic_enum = magic_enum::enum_cast<FieldType>(type); type_magic_enum.has_value()) { return {ok::ErrorCode::ERROR_NO_ERROR, type_magic_enum.value()}; }
    else
    {
      LOG_DEBUG << "Value not Exist for type: " << type;
      return {ok::ErrorCode::SCHEMA_ERROR, FieldType::text};
    }
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, FieldType::text};
}
void visible(VPackSlice const &obj, Field &f) noexcept
{
  if (obj.hasKey("visibleIf"))
  {
    if (obj["visibleIf"].isArray())
    {
      for (auto const &vit : arangodb::velocypack::ArrayIterator(obj["visibleIf"]))
      {
        // if(vit.isString()){
        //
        //}
        if (vit.isArray())
        {
          if (vit.length() > 0)
          {
            if (vit[0].isString())
            {
              if (auto func = vit[0].copyString(); func == "isGlobal")
              {
                // todo fix this..
                /*if (database == "global")
                {
                  f.isVisible = getSecondBoolOrTrue(vit);
                  break;
                }*/
              }
              else if (func == "false")
              {
                f.isVisible = false;
                break;
              }
            }
          }
        }
      }
    }
  }
  // Not working on js:
  // if (f.type == FieldType::save_time) {
  //  f.isVisible = false;
  //}
}
void props(VPackSlice const &obj, Field &f) noexcept
{
  if (obj.hasKey("p"))
  {  // props
    f.props = jsoncons::ojson::parse(obj.get("p").toJson());
    // LOG_DEBUG << f.props.to_string();
    if (jsoncons::ObjectMemberIsString(f.props, "l"))
    {
      f.label = f.props["l"].as_string_view();
      f.props.erase("l");
    }
    if (jsoncons::ObjectMemberIsBool(f.props, "ds"))
    {
      f.disabled = f.props["ds"].as_bool();
      f.props.erase("ds");
    }
    if (jsoncons::ObjectMemberIsBool(f.props, "r"))
    {
      f.required = f.props["r"].as_bool();
      f.props.erase("r");
    }
  }
}
void default_(VPackSlice const &obj, Field &f) noexcept
{
  if (f.props.contains("df"))
  {  // default
    bool set_default = true;
    if (f.props["df"].is_array())
    {
      for (auto &vit : f.props["df"].array_range())
      {
        // if(vit.isString()){
        //
        //}
        if (vit.is_array())
        {
          if (vit.size() > 0)
          {
            if (vit[0].is_string())
            {
              if (auto func = vit[0].as_string(); func == "isGlobal")
              {
                // todo fix this...
                /*if (database == "global")
                {
                  // f.default_value =
                  // getSecondBoolOrTrue(vit);
                  set_default = false;
                  break;
                }*/
              }
              else if (func == "false")
              {
                f.defaultValue = false;
                set_default = false;
                break;
              }
              else if (func == "database")
              {
                // todo fix this
                /*f.defaultValue = database;
                set_default = false;
                break;
                */
              }
              else if (func == "memberkey")
              {
                // todo fix this
                /*f.defaultValue = memberKey;
                set_default = false;
                break;*/
              }
            }
          }
        }
      }
    }
    if (set_default)
    {
      f.defaultValue = f.props.at("df");  // hack
      f.props.erase("df");
    }
    else
    {
      f.props.erase("df");
    }
  }
  else
  {
    jsoncons::ojson type;
    switch (f.type)
    {
      case FieldType::checkbox:
        f.defaultValue = false;
        f.defaultValue = false;
        break;
      case FieldType::number:
      case FieldType::date:
      case FieldType::datetime_local:
      case FieldType::month:
      case FieldType::flatPicker:
      case FieldType::inserted:
      case FieldType::updated: f.defaultValue = 0; break;
      case FieldType::jsonEditor:
      {
        jsoncons::ojson j;
        f.defaultValue = j;
        break;
      }
      case FieldType::multi_select:
      case FieldType::multi_select_hidden:
      case FieldType::text_array:
      case FieldType::multi_select_bool_properties: f.defaultValue = jsoncons::ojson::array(); break;
      case FieldType::codemirror:
      case FieldType::text: f.defaultValue = ""; break;
      case FieldType::file:
      {
        f.defaultValue = jsoncons::ojson::null();  // null or []
        break;
      }
      default:
      {
        if (obj.hasKey("r"))
        {  // relation
          f.defaultValue = jsoncons::ojson::array();
        }
        else
        {
          f.defaultValue = "";
        }
        break;
      }
    }
  }
}
ErrorCode event(VPackSlice const &obj, Field &f) noexcept
{
  // todo fix this...
  /*if (database == "global")
  {
    if (f.props.contains("ge"))
    {
      if (auto evt = f.props["ge"]; evt.is_array())
      {
        for (unsigned int i = 0; i < evt.size(); i++)
        {
          if (jsoncons::ArrayPosIsString(evt[i], 0))
          {
            if (auto response_code = magic_enum::enum_cast<ok::smart_actor::connection::EventType>(evt[i][0].as_string());
                response_code.has_value())
            {
              evt[i][0] = (int)response_code.value();
            }
            else
            {
              LOG_DEBUG << " has no code!";
              return ok::ErrorCode::SCHEMA_ERROR;
            }
          }
          if (evt[i].is_array())
          {
            for (unsigned int j = 1; j < evt[i].size(); j++)
            {
              if (jsoncons::ArrayPosIsString(evt[i], j))
              {
                if (auto response_code = magic_enum::enum_cast<ok::smart_actor::connection::Route>(evt[i][j].as_string());
                    response_code.has_value())
                {
                  evt[i][j] = (int)response_code.value();
                }
                else
                {
                  LOG_DEBUG << " Route has no code!";
                  // throw
                  // std::runtime_error("MISMATCH");
                }
              }
            }
          }
        }
      }
      f.props["e"] = jsoncons::ojson(f.props["ge"]);
      f.props.erase("ge");
    }
  }
  else*/
  return ok::ErrorCode::ERROR_NO_ERROR;
}
ErrorCode relation(VPackSlice const &obj,
                   Field &f,
                   RelationQueryMap &relationQueries,
                   ok::smart_actor::connection::MutateEventType const &eventType,
                   Collection const &collection,
                   jsoncons::ojson const &args,
                   DocumentKey const &memberKey) noexcept
{
  if (obj.hasKey("r"))
  {
    if (obj.hasKey(std::vector<std::string>({"r", "e"})))
    {
      f.relation.edge = obj["r"]["e"].copyString();
      if (obj.hasKey(std::vector<std::string>({"r", "from"}))) { f.relation.from = obj["r"]["from"].copyString(); }
      if (obj.hasKey(std::vector<std::string>({"r", "to"}))) { f.relation.to = obj["r"]["to"].copyString(); }
      if (obj.hasKey(std::vector<std::string>({"r", "di"})))
      {
        if (auto di = obj["r"]["di"].copyString(); di == "in") { f.relation.direction = TRI_EDGE_IN; }
        else if (di == "out")
        {
          f.relation.direction = TRI_EDGE_OUT;
        }
      }
      if (f.relation.direction == TRI_EDGE_OUT)
      {
        if (f.relation.to.empty())
        {
          LOG_DEBUG << "relation to key must defined when direction is out";
          return ok::ErrorCode::SCHEMA_ERROR;
        }
        if (f.relation.from.empty()) { f.relation.from = collection; }
      }
      else if (f.relation.direction == TRI_EDGE_IN)
      {
        if (f.relation.from.empty())
        {
          LOG_DEBUG << "relation from key must defined when direction is in";
          return ok::ErrorCode::SCHEMA_ERROR;
        }
        if (f.relation.to.empty()) { f.relation.to = collection; }
      }
      // type = t
      // default = df
      // direction = di
      // global_event = ge
      // user_event = ue
      // information = i
      if (obj.hasKey(std::vector<std::string>({"r", "ge"}))) { f.relation.globalKey = obj["r"]["ge"].copyString(); }
      if (obj.hasKey(std::vector<std::string>({"r", "ue"}))) { f.relation.userKey = obj["r"]["ue"].copyString(); }
      else
      {
        f.relation.userKey = "";
      }
      if (obj.hasKey(std::vector<std::string>({"r", "config"})))
      {
        if (obj["r"]["config"].isString()) f.relation.getEdgeOppositeVertex = jsoncons::ojson::parse("\"" + obj["r"]["config"].toString() + "\"");
        else
          f.relation.getEdgeOppositeVertex = jsoncons::ojson::parse(obj["r"]["config"].toString());
      }
      if (obj.hasKey(std::vector<std::string>({"r", "i"}))) { f.relation.information = obj["r"]["i"].getBool(); }
      if (!f.relation.information)
      {
        switch (eventType)
        {
          case ok::smart_actor::connection::MutateEventType::insert:
            if (!jsoncons::isEmptyObject(f.relation.getEdgeOppositeVertex))
              if (auto [er, option, configValue] = ok::table_schema::impl::getFirstConfigValueFromArgs(f.relation.getEdgeOppositeVertex, args, memberKey); ok::isEr(er)) { return er; }
            if (auto er = relation_query_generate::insert(obj, f, relationQueries); ok::isEr(er)) { return er; }
            break;
          case ok::smart_actor::connection::MutateEventType::update:
            if (!jsoncons::isEmptyObject(f.relation.getEdgeOppositeVertex))
              if (auto [er, option, configValue] = ok::table_schema::impl::getFirstConfigValueFromArgs(f.relation.getEdgeOppositeVertex, args, memberKey); ok::isEr(er)) { return er; }
            if (auto er = relation_query_generate::update(obj, f, relationQueries); ok::isEr(er)) { return er; }
            break;
          case ok::smart_actor::connection::MutateEventType::delete_:
            if (auto er = relation_query_generate::delete_(obj, f, relationQueries); ok::isEr(er)) { return er; }
            break;
          case ok::smart_actor::connection::MutateEventType::changePosition:
            if (auto er = relation_query_generate::position(obj, f, relationQueries); ok::isEr(er)) { return er; }
            break;
          case ok::smart_actor::connection::MutateEventType::batchUpdate:
            if (!jsoncons::isEmptyObject(f.relation.getEdgeOppositeVertex))
              if (auto [er, option, configValue] = ok::table_schema::impl::getFirstConfigValueFromArgs(f.relation.getEdgeOppositeVertex, args, memberKey); ok::isEr(er)) { return er; }
            if (auto er = relation_query_generate::batchUpdate(obj, f, relationQueries); ok::isEr(er)) { return er; }
            break;
          case ok::smart_actor::connection::MutateEventType::batchDelete:
            if (auto er = relation_query_generate::batchDelete(obj, f, relationQueries); ok::isEr(er)) { return er; }
            break;
          default: break;
        }
      }
    }
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
}  // namespace process_field
namespace relation_query_generate
{
ErrorCode insert(VPackSlice const &obj, Field const &f, RelationQueryMap &relationQueries) noexcept
{
  switch (f.type)
  {
    case FieldType::internal_true_edge:
    case FieldType::multi_select:
    case FieldType::multi_select_hidden:
    case FieldType::multi_select_bool_properties:
    {
      absl::ParsedFormat<'s', 's', 's', 's', 's', 's', 's'> y_out_internal(
          R"aql(
LET %1$s = (for i_ in i
    FOR m in @%1$s
        INSERT {_%4$s: CONCAT("%2$s/", %6$s), _%5$s: CONCAT("%3$s/", %7$s)} INTO %1$s)
)aql");
      absl::ParsedFormat<'s', 's', 's', 's', 's', 's', 's'> y_out_multi(
          R"aql(
LET %1$s = (for i_ in i
    FOR m in @%1$s
        INSERT {_%4$s: CONCAT("%2$s/", %6$s), _%5$s: CONCAT("%3$s/", %7$s)} INTO %1$s)
)aql");
      absl::ParsedFormat<'s', 's', 's', 's', 's', 's', 's'> y_out_bool_prop(
          R"aql(
LET %1$s = (for i_ in i
    FOR m in @%1$s
        INSERT {_%4$s: CONCAT("%2$s/", %6$s), _%5$s: CONCAT("%3$s/", %7$s), properties: m[1]} INTO %1$s)
)aql");
      std::string s4, s5, s6, s7;
      if (f.relation.direction == TRI_EDGE_OUT)
      {
        s4 = "from";
        s5 = "to";
        s6 = "i_";
        if (f.type == FieldType::multi_select_bool_properties) { s7 = "m[0]"; }
        else
        {
          s7 = "m";
        }
      }
      else if (f.relation.direction == TRI_EDGE_IN)
      {
        // Think again why same here works?
        s4 = "from";
        s5 = "to";
        if (f.type == FieldType::multi_select_bool_properties) { s6 = "m[0]"; }
        else
        {
          s6 = "m";
        }
        s7 = "i_";
      }
      else
      {
        LOG_DEBUG << "direction must be valid";
        return ok::ErrorCode::SCHEMA_ERROR;
      }
      auto fmt = [&]()
      {
        if (f.type == FieldType::internal_true_edge) { return y_out_internal; }
        else if (f.type == FieldType::select || f.type == FieldType::multi_select || f.type == FieldType::multi_select_hidden)
        {
          return y_out_multi;
        }
        else if (f.type == FieldType::multi_select_bool_properties)
        {
          return y_out_bool_prop;
        }
        else
        {
          return y_out_internal;
        }
      };
      if (f.relation.direction == TRI_EDGE_OUT || f.relation.direction == TRI_EDGE_IN)
      {
        std::string deepQuery = absl::StrFormat(fmt(), f.relation.edge, f.relation.from, f.relation.to, s4, s5, s6, s7);
        relationQueries.insert({f.relation.edge, {deepQuery, "", f.relation.globalKey, f.relation.userKey}});
      }
      else
      {
        LOG_DEBUG << "direction must be in/out";
        return ok::ErrorCode::SCHEMA_ERROR;
      }
      break;
    }
    default: break;
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
ErrorCode update(VPackSlice const &obj, Field const &f, RelationQueryMap &relationQueries) noexcept
{
  switch (f.type)
  {
      // Todo add internal_true_edge case when
      // needed
    case FieldType::multi_select:
    case FieldType::multi_select_hidden:
    case FieldType::multi_select_bool_properties:
    {
      // https://www.arangodb.com/docs/3.8/aql/operations-upsert.html
      absl::ParsedFormat<'s', 's', 's', 's', 's', 's', 's'> y_out_multi(
          R"aql(
    LET deleted_%1$s = (FOR k_ in k
        LET exiting_%1$s = (for e in %1$s
            FILTER e._%4$s == CONCAT("%2$s/", k_)
            RETURN e)
        LET existing_%3$s_keys = (FOR a in exiting_%1$s RETURN SPLIT(a._%5$s, "/")[1])

        LET deleted_%3$s = MINUS(existing_%3$s_keys, @%1$s)
        //LET new_and_existing_%3$s = MINUS(@%1$s, deleted_%3$s)
        LET new_and_existing_%3$s = MINUS(@%1$s, existing_%3$s_keys) // only new keys, because its no meaning to update edge with {}, it just update _rev
        LET deleted_%1$s = (FOR e in exiting_%1$s
            FILTER  !POSITION( @%1$s, SPLIT(e._%5$s, "/")[1] )
            RETURN e._key
        )

        LET u = (FOR n in new_and_existing_%3$s
            UPSERT {_%4$s: CONCAT("%2$s/", %6$s), _%5$s: CONCAT("%3$s/", %7$s)}
                INSERT {_%4$s: CONCAT("%2$s/", %6$s), _%5$s: CONCAT("%3$s/", %7$s)}
                UPDATE {}
                IN %1$s
        )

        RETURN deleted_%1$s
    )
)aql");
      absl::ParsedFormat<'s', 's', 's', 's', 's', 's', 's'> y_out_bool_prop(
          R"aql(
    LET deleted_%1$s = (FOR k_ in k
        LET %1$s_new = @%1$s[*][0]
        LET exiting_%1$s = (for e in %1$s
            FILTER e._%4$s == CONCAT("%2$s/", k_)
            RETURN e)

        LET deleted_%3$s = MINUS(exiting_%1$s[*]._%5$s, %1$s_new)
        LET new_and_existing_%3$s = MINUS(%1$s_new, deleted_%3$s)
        LET deleted_%1$s = (FOR e in exiting_%1$s
            FILTER  !POSITION( %1$s_new, SPLIT(e._%5$s, "/")[1] )
            RETURN e._key
        )

        LET u = (FOR n in new_and_existing_%3$s
            LET p = POSITION( %1$s_new, n, true )
            UPSERT {_%4$s: CONCAT("%2$s/", k_), _%5$s: CONCAT("%3$s/", n)}
                INSERT {_%4$s: CONCAT("%2$s/", %6$s), _%5$s: CONCAT("%3$s/", %7$s), properties: @%1$s[p][1]}
                UPDATE {_%4$s: CONCAT("%2$s/", %6$s), _%5$s: CONCAT("%3$s/", %7$s), properties: @%1$s[p][1]}
                IN %1$s OPTIONS { keepNull: false }
        )

        RETURN deleted_%1$s
    )
)aql");
      std::string s4, s5, s6, s7;
      if (f.relation.direction == TRI_EDGE_OUT)
      {
        s4 = "from";
        s5 = "to";
        s6 = "k_";
        if (f.type == FieldType::multi_select_bool_properties) { s7 = "n"; }
        else
        {
          s7 = "n";
        }
      }
      else if (f.relation.direction == TRI_EDGE_IN)
      {
        // Think again why same here works?
        s4 = "from";
        s5 = "to";
        if (f.type == FieldType::multi_select_bool_properties) { s6 = "n"; }
        else
        {
          s6 = "n";
        }
        s7 = "k_";
      }
      else
      {
        LOG_DEBUG << "direction must be valid";
        return ok::ErrorCode::SCHEMA_ERROR;
      }
      auto fmt = [&]()
      {
        if (f.type == FieldType::internal_true_edge)
        {
          // return y_out_internal;
          return y_out_multi;
        }
        if (f.type == FieldType::select || f.type == FieldType::multi_select) { return y_out_multi; }
        else if (f.type == FieldType::multi_select_bool_properties)
        {
          return y_out_bool_prop;
        }
        else
        {
          // return y_out_internal;
          return y_out_multi;
        }
      };
      // group_member = $1
      // group = $2
      if (f.relation.direction == TRI_EDGE_OUT || f.relation.direction == TRI_EDGE_IN)
      {
        auto deepQuery = absl::StrFormat(fmt(), f.relation.edge, f.relation.from, f.relation.to, s4, s5, s6, s7);
        relationQueries.insert({f.relation.edge, {deepQuery, "FIRST(deleted_" + f.relation.edge + ")", f.relation.globalKey, f.relation.userKey}});
      }
      else
      {
        LOG_DEBUG << "direction must be in/out";
        return ok::ErrorCode::SCHEMA_ERROR;
      }
      break;
    }
    default: break;
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
ErrorCode delete_(VPackSlice const &obj, Field const &f, RelationQueryMap &relationQueries) noexcept
{
  constexpr auto y =
      R"aql(
    LET deleted_%1$s = (FOR k_ in k
        LET exiting_%1$s = (for e in %1$s
            FILTER e._%3$s == CONCAT("%2$s/", k_)
            RETURN e)
        RETURN exiting_%1$s
    )
    LET d_0_%1$s = UNIQUE(FLATTEN(deleted_%1$s))
    LET d_1_%1$s = (FOR d in d_0_%1$s
        REMOVE d IN %1$s )
    LET d_2_%1$s = (FOR d in d_0_%1$s
        RETURN d._key)
)aql";
  // group_member = $1
  // group = $2
  std::string coll, s4, s5, s6, s7;
  if (f.relation.direction == TRI_EDGE_OUT)
  {
    s4 = "from";
    s5 = "to";
    s6 = "k_";
    if (f.type == FieldType::multi_select_bool_properties) { s7 = "n"; }
    else
    {
      s7 = "n";
    }
    coll = f.relation.from;
  }
  else if (f.relation.direction == TRI_EDGE_IN)
  {
    s4 = "to";
    s5 = "from";
    if (f.type == FieldType::multi_select_bool_properties) { s6 = "n"; }
    else
    {
      s6 = "n";
    }
    s7 = "k_";
    coll = f.relation.to;
  }
  else
  {
    LOG_DEBUG << "direction must be valid";
    return ok::ErrorCode::SCHEMA_ERROR;
  }
  auto deepQuery = absl::StrFormat(y, f.relation.edge, coll, s4);
  relationQueries.insert({f.relation.edge, {deepQuery, "d_2_" + f.relation.edge, f.relation.globalKey, f.relation.userKey}});
  return ok::ErrorCode::ERROR_NO_ERROR;
}
ErrorCode position(VPackSlice const &obj, Field const &f, RelationQueryMap &relationQueries) noexcept { return ok::ErrorCode::ERROR_NO_ERROR; }
ErrorCode batchUpdate(VPackSlice const &obj, Field const &f, RelationQueryMap &relationQueries) noexcept { return ok::ErrorCode::ERROR_NO_ERROR; }
ErrorCode batchDelete(VPackSlice const &obj, Field const &f, RelationQueryMap &relationQueries) noexcept
{
  constexpr auto y =
      R"aql(
    LET deleted_%1$s = (FOR k_ in k
        LET exiting_%1$s = (for e in %1$s
            FILTER e._%3$s == CONCAT("%2$s/", k_)
            RETURN e)
        RETURN exiting_%1$s
    )
    LET d_0_%1$s = UNIQUE(FLATTEN(deleted_%1$s))
    LET d_1_%1$s = (FOR d in d_0_%1$s
        REMOVE d IN %1$s )
    LET d_2_%1$s = (FOR d in d_0_%1$s
        RETURN d._key)
)aql";
  // group_member = $1
  // group = $2
  std::string coll, s4, s5, s6, s7;
  if (f.relation.direction == TRI_EDGE_OUT)
  {
    s4 = "from";
    s5 = "to";
    s6 = "k_";
    if (f.type == FieldType::multi_select_bool_properties) { s7 = "n"; }
    else
    {
      s7 = "n";
    }
    coll = f.relation.from;
  }
  else if (f.relation.direction == TRI_EDGE_IN)
  {
    s4 = "to";
    s5 = "from";
    if (f.type == FieldType::multi_select_bool_properties) { s6 = "n"; }
    else
    {
      s6 = "n";
    }
    s7 = "k_";
    coll = f.relation.to;
  }
  else
  {
    LOG_DEBUG << "direction must be valid";
    return ok::ErrorCode::SCHEMA_ERROR;
  }
  auto deepQuery = absl::StrFormat(y, f.relation.edge, coll, s4);
  relationQueries.insert({f.relation.edge, {deepQuery, "d_2_" + f.relation.edge, f.relation.globalKey, f.relation.userKey}});
  return ok::ErrorCode::ERROR_NO_ERROR;
}
}  // namespace relation_query_generate
}  // namespace impl
}  // namespace mutate_schema
}  // namespace ok
