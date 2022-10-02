#include "mutate/MutateSchema.hpp"
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
#include "utils/ErrorConstants.hpp"
#include "utils/time_functions.hpp"
#include "table/TableSchema.hpp"
#include "table/TableSchemaImpl.hpp"
namespace ok
{
namespace mutate_schema
{
std::tuple<ErrorCode, Collection, Relation, Fields, FieldOrder> initialize(VPackSlice const &schemaJson, ok::smart_actor::connection::MutateEventType const &eventType) noexcept
{
  Fields fields;
  FieldOrder allIds;
  auto [er, collection] = impl::getCollection(schemaJson);
  if (ok::isEr(er)) return {er, "", {}, fields, allIds};
  auto [er2, tree] = ok::table_schema::impl::processTree(schemaJson, collection);
  if (ok::isEr(er2)) return {er2, "", tree, fields, allIds};
  if (eventType == ok::smart_actor::connection::MutateEventType::insert) impl::addDefaultKeysInsert(fields, allIds, schemaJson);
  else
    impl::addDefaultKeys(fields, allIds, schemaJson);
  // do processFormProps manually
  return {ok::ErrorCode::ERROR_NO_ERROR, collection, tree, fields, allIds};
}
std::tuple<ErrorCode, Fields, FieldOrder> processDataPart(VPackSlice const &schemaJson,
                                                          jsoncons::ojson const &args,
                                                          ok::smart_actor::connection::MutateEventType const &eventType,
                                                          Collection const &collection,
                                                          Relation const &tree,
                                                          MutateQueryParts &queryParts,
                                                          DocumentKey const &memberKey,
                                                          bool ignoreMissingFieldWithValue) noexcept
{
  std::vector<Selector> selector;
  Fields fields;
  FieldOrder allIds;
  if (args.contains("sel")) { selector = args["sel"].as<std::vector<std::string>>(); }
  if (!schemaJson.hasKey("d") && !schemaJson.get("d").isArray())
  {
    LOG_DEBUG << "schema must has 'd' key";
    return {ok::ErrorCode::ERROR_NO_ERROR, fields, allIds};
  }
  auto data = schemaJson["d"];
  for (auto const &it : arangodb::velocypack::ArrayIterator(data))
  {
    if (!it.isObject())
    {
      LOG_DEBUG << "each 'd' array element must be object";
      return {ok::ErrorCode::SCHEMA_ERROR, fields, allIds};
    }
    Field f;
    f.id = ok::table_schema::impl::process_column::id(it);
    if (auto [er, type] = impl::process_field::type(it); ok::isEr(er)) { return {er, fields, allIds}; }
    else
      f.type = type;
    impl::process_field::props(it, f);
    if (f.type == ok::mutate_schema::FieldType::fieldSet)
    {
      f.backEndOnly = true;
      fields.insert({f.id, f});
      continue;
    }
    if (f.type == ok::mutate_schema::FieldType::inserted && eventType != ok::smart_actor::connection::MutateEventType::insert) continue;
    if (auto [er, dbKey] = impl::process_field::selector(it); isEr(er)) return {er, fields, allIds};
    else
      f.selector = dbKey;
    if (f.id.empty()) f.id = f.selector;
    if (auto [er, description] = arangodb::velocypack::getString(it, "description"); !isEr(er)) f.description = description;
    if (auto [er, group] = arangodb::velocypack::getString(it, "group"); !isEr(er)) f.group = group;
    if (auto [er, valuePath] = arangodb::velocypack::getString(it, "path"); !isEr(er)) f.valuePath = valuePath;
    else
      f.valuePath = f.id;
    if (auto isKeyInSelector = std::find(std::begin(selector), std::end(selector), f.selector); !selector.empty() && isKeyInSelector == std::end(selector))
    {
      LOG_DEBUG << "skipping field by manual config: " << f.selector;
      continue;
    }
    if (!f.selector.empty())  // if relation we have empty selector.
    {
      if (fields.contains(f.id))
      {
        LOG_DEBUG << "duplicate schema field: " << f.id;
        return {ok::ErrorCode::SCHEMA_ERROR, fields, allIds};
      }
    }
    if (f.type == FieldType::serial)
    {
      if (eventType == ok::smart_actor::connection::MutateEventType::insert)
      {
        if (impl::isTreeChild(tree, args)) impl::makePrefixPositionQueryInsertTree(it, f, args, tree, collection, queryParts);
        else
          impl::makePrefixPositionQueryInsert(it, f, queryParts);
      }
    }
    if (f.label.empty()) { f.label = pystring::first_upper(pystring::replace(f.id, "_", " ")); }
    impl::process_field::visible(it, f);
    impl::process_field::default_(it, f);
    impl::process_field::event(it, f);
    if (auto er = impl::process_field::relation(it, f, queryParts.relationQueries, eventType, collection, args, memberKey); er == ok::ErrorCode::ERROR_BAD_PARAMETER && ignoreMissingFieldWithValue)
      continue;  // ERROR_BAD_PARAMETER means no opposite edge value found
    else if (ok::isEr(er))
      return {er, fields, allIds};
    if (impl::isFieldSkip(it, f, eventType == ok::smart_actor::connection::MutateEventType::update, memberKey)) f.backEndOnly = true;
    allIds.push_back(f.id);
    fields.insert({f.id, f});
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, fields, allIds};
}
/*std::tuple<ErrorCode> processExternalPart(Fields &externalFields,
                                          VPackSlice const &schemaJson,
                                          jsoncons::ojson const &args,
                                          ok::smart_actor::connection::EventType const &eventType,
                                          Collection const &collection,
                                          Relation const &tree,
                                          MutateQueryParts &queryParts) noexcept
{
  if (!schemaJson.hasKey("e") && !schemaJson.get("e").isArray())
  {
    return {ok::ErrorCode::ERROR_NO_ERROR};
  }
  auto data = schemaJson["e"];
  for (auto const &it : arangodb::velocypack::ArrayIterator(data))
  {
    if (!it.isObject())
    {
      LOG_DEBUG << "each 'e' array element must be object";
      return {ok::ErrorCode::SCHEMA_ERROR};
    }
    Field f;
     impl::process_field::Description(it, f);
     impl::process_field::Selector(it, f);

    if (!f.selector.empty())  // if relation we have empty selector.
    {
      if (auto find = std::find_if(std::begin(fields), std::end(fields), [&](Field const &i) { return i.selector == f.selector; }); find != std::end(externalFields))
      {
        LOG_DEBUG << "duplicate schema field: " << find->selector;
        return {ok::ErrorCode::SCHEMA_ERROR};
      }
    }
    if (auto [er, type] =  impl::process_field::Type(it); ok::isEr(er)) { return {er}; }
    else
    {
      f.type = type;
      if (f.type == FieldType::serial)
      {
        if (eventType == ok::smart_actor::connection::MutateEventType::insert)
        {
          if (impl::isTreeChild(tree, args)) impl::makePrefixPositionQueryInsertTree(it, f, args, tree, collection, queryParts);
          else
            impl::makePrefixPositionQueryInsert(it, f, queryParts);
        }
      }
    }
     impl::process_field::Visible(it, f);
     impl::process_field::Props(it, f);
     impl::process_field::Default(it, f);
     impl::process_field::Event(it, f);
    if (auto er =  impl::process_field::Relation(it, f, queryParts.relationQueries, eventType, collection); ok::isEr(er)) return {er};
    if (impl::isFieldSkip(it, f, eventType == ok::smart_actor::connection::MutateEventType::update, memberKey)) f.backEndOnly = true;
    externalFields.push_back(f);
  }
  return {ok::ErrorCode::ERROR_NO_ERROR};

}*/
std::tuple<ErrorCode, jsoncons::ojson> processFormProps(VPackSlice const &schemaJson, Fields const &fields, DocumentKey const &memberKey) noexcept
{
  if (schemaJson.isObject() && schemaJson.hasKey("p"))
  {
    auto options = jsoncons::ojson::parse(schemaJson["p"].toJson());
    // todo fix this.
    if (memberKey == "1")  // enable permission form.
    {
      if (options.contains("ds"))  // disabled
      {
        options.erase("ds");
      }
    }
    /*if (options.contains("l") && options["l"].is_array())
    {  // layout set selector to index
      auto l = options["l"];
      // loop array
      for (auto &j : l.array_range())
      {
        if (j.is_array())
        {
          for (auto &k : j.array_range())
          {
            // find index:
            if (k.is_string())
            {
              if (auto find = std::find_if(std::begin(fields), std::end(fields), [&](Field const &f) { return f.selector == k.as_string(); }); find != std::end(fields))
              {
                auto last_found_idx = std::distance(std::begin(fields), find);
                k = last_found_idx;
              }
              else
              {
                LOG_DEBUG << "Array element must be found in fields data: " << k.as_string() << " not found";
                k = -1;
                return {ok::ErrorCode::SCHEMA_ERROR, jsoncons::ojson::object()};
              }
            }
          }
        }
        else
        {
          LOG_DEBUG << "Array Element Must be Array";
          return {ok::ErrorCode::SCHEMA_ERROR, jsoncons::ojson::object()};
        }
      }
      // std::cout << pretty_print(l) << "\n";
    }*/
    return {ok::ErrorCode::ERROR_NO_ERROR, options};
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, jsoncons::ojson::object()};
}
std::tuple<ErrorCode, arangodb::velocypack::Builder> makeRelationBindVarsBase(Fields const &fields, WsArguments const &args, DocumentKey const &memberKey) noexcept
{
  // preRequirements: formData must be object
  auto formValue = args["value"];
  arangodb::velocypack::Builder relationBindVars;
  relationBindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  for (auto const &[id, field] : fields)
  {
    if (!field.relation.edge.empty())
    {
      if (field.id.empty())
      {
        LOG_DEBUG << "relation id must exist for field:\n " << ok::mutate_schema::to_string(field);
        return {ok::ErrorCode::SCHEMA_ERROR, {}};
      }
      if (field.backEndOnly)  //  || jsoncons::isEmptyArray(formData[field.id] meanigless to check isEmptyArray
      {
        auto [er, option, configValue] = ok::table_schema::impl::getFirstConfigValueFromArgs(field.relation.getEdgeOppositeVertex, args, memberKey);
        if (ok::isEr(er) && er != ok::ErrorCode::ERROR_FORM_EMPTY) return {er, relationBindVars};
        if (er == ok::ErrorCode::ERROR_NO_ERROR)
        {
          if (!configValue.empty())
          {
            jsoncons::ojson value{jsoncons::json_array_arg};
            value.push_back(configValue);
            auto [er2, builder] = arangodb::velocypack::parseJson(value.to_string());
            if (ok::isEr(er)) { return {er, relationBindVars}; }
            relationBindVars.add(field.relation.edge, builder->slice());  // This is limited feature!!!
          }
          else
          {
            arangodb::velocypack::Builder b;
            b.openArray();
            b.close();
            relationBindVars.add(field.relation.edge, b.slice());
          }
        }
        // if (er == ok::ErrorCode::ERROR_FORM_EMPTY) // Handle this...
      }
      else
      {
        auto [er, builder] = arangodb::velocypack::parseJson(formValue[field.id].to_string());
        if (ok::isEr(er)) { return {er, relationBindVars}; }
        relationBindVars.add(field.relation.edge, builder->slice());  // This is limited feature!!!
      }
    }
  }
  relationBindVars.close();
  return {ok::ErrorCode::ERROR_NO_ERROR, relationBindVars};
}
arangodb::velocypack::Builder makeBindVarsDelete() noexcept
{
  arangodb::velocypack::Builder formSchemaBindVars;
  formSchemaBindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  formSchemaBindVars.close();
  return formSchemaBindVars;
}
namespace table_actor
{
jsoncons::ojson getFormHeaderData(Fields &fields, FieldOrder const &allIds, const jsoncons::ojson &options, jsoncons::ojson const &valueArray) noexcept
{
  jsoncons::ojson ret = jsoncons::ojson::object();
  jsoncons::ojson columnSchemaRow;
  jsoncons::ojson defaultLayout = jsoncons::ojson::object();
  auto isValueExist = valueArray.is_array() && valueArray.size() == 1;
  if (isValueExist)
  {
    auto &value = valueArray[0];
    for (auto it = fields.begin(); it != fields.end(); it++)
    {
      if (value.has_key(it.key())) it.value().value = value[it.key()];  // we are sure value object has id key
    }
  }
  for (auto const &[id, field] : fields)
  {
    // LOG_DEBUG << field.id << "  "  << field.group << "  " << field.backEndOnly;
    if (field.type == ok::mutate_schema::FieldType::fieldSet)
    {
      jsoncons::ojson v;
      v["type"] = (int)field.type;
      v["props"] = field.props;
      v["layout"] = jsoncons::ojson::object();
      defaultLayout[id] = v;
    }
    if (field.backEndOnly) continue;
    jsoncons::ojson v;
    v["label"] = field.label;
    v["type"] = (int)field.type;
    // v["path"] = field.valuePath;
    v["isRequired"] = field.required;
    v["disabled"] = field.disabled;
    v["description"] = field.description;
    v["options"] = field.props;
    v["default"] = field.defaultValue;
    // v["initial"] = field.defaultValue;
    v["value"] = isValueExist ? field.value : field.defaultValue;
    columnSchemaRow[id] = v;
    if (!field.group.empty()) { defaultLayout[field.group]["layout"][id] = jsoncons::ojson::object(); }
    else
      defaultLayout[id] = jsoncons::ojson::object();
  }
  ret["fields"] = columnSchemaRow;
  ret["formProps"] = options;
  ret["layout"] = defaultLayout;
  return ret;
}
jsoncons::ojson getFormDefaultObject(Fields const &fields, FieldOrder const &allIds) noexcept
{
  jsoncons::ojson r;
  jsoncons::ojson res = jsoncons::ojson::array();
  jsoncons::ojson jsonDefaultRow = jsoncons::ojson::object();
  for (auto const &id : allIds)
  {
    auto field = fields.find(id);
    if (field != std::end(fields))
    {
      if (field->second.backEndOnly) continue;  // ok remove comment if ok?
      jsonDefaultRow[field->first] = field->second.defaultValue;
    }
  }
  res.push_back(jsonDefaultRow);
  r["r"]["result"] = res;
  return r;
}
std::tuple<ErrorCode, AQLQuery> makeFormFieldsObjectProjection(Fields const &fields, FieldOrder const &allIds, Collection const &collection, std::string const &as) noexcept
{
  std::string selector;
  if (!fields.empty())
  {
    // like: pystring::join
    std::vector<std::string>::size_type allIdsSize = allIds.size();
    std::vector<std::string>::size_type i;
    std::string result;
    if (allIdsSize == 0) { result = ""; }
    else
    {
      result = const_cast<Fields &>(fields)[allIds[0]].selector + ":" + as + "." + const_cast<Fields &>(fields)[allIds[0]].selector;
      if (allIdsSize != 1)
      {
        for (i = 1; i < allIdsSize; ++i)
        {
          if (const_cast<Fields &>(fields)[allIds[i]].backEndOnly) continue;  // ok remove comment if ok
          if (auto &f = const_cast<Fields &>(fields)[allIds[i]]; !f.selector.empty()) { result += "," + f.selector + ":" + as + "." + f.selector; }
          else
          {
            // bindVars not using here...
            switch (f.type)
            {
              case FieldType::internal_true_edge:
              case FieldType::multi_select:
              case FieldType::multi_select_hidden:
              {
                constexpr auto y = R"aql((FOR e in %1$s
    FILTER e._%4$s == CONCAT("%2$s/", %3$s)
    RETURN SPLIT(e._%5$s, "/")[1])
    )aql";
                std::string s4;
                std::string s5;
                if (f.relation.direction == TRI_EDGE_OUT)
                {
                  s4 = "from";
                  s5 = "to";
                }
                else if (f.relation.direction == TRI_EDGE_IN)
                {
                  s4 = "to";
                  s5 = "from";
                }
                else
                {
                  LOG_DEBUG << "direction must be valid for field with id: " << f.id << " " << ok::mutate_schema::to_string(f);
                  return {ok::ErrorCode::SCHEMA_ERROR, ""};
                }
                result += "," + f.relation.edge + ":" + absl::StrFormat(y, f.relation.edge, collection, as + "._key", s4, s5);
                break;
              }
              case FieldType::multi_select_bool_properties:
              {
                constexpr auto y = R"aql((FOR e in %1$s
    FILTER e._%4$s == CONCAT("%2$s/", %3$s)
    RETURN [SPLIT(e._%5$s, "/")[1], e.properties] )
    )aql";
                std::string s4, s5;
                if (f.relation.direction == TRI_EDGE_OUT)
                {
                  s4 = "from";
                  s5 = "to";
                }
                else if (f.relation.direction == TRI_EDGE_IN)
                {
                  s4 = "to";
                  s5 = "from";
                }
                else
                {
                  LOG_DEBUG << "direction must be valid";
                  return {ok::ErrorCode::SCHEMA_ERROR, ""};
                }
                result += "," + f.relation.edge + ":" + absl::StrFormat(y, f.relation.edge, collection, as + "._key", s4, s5);
                break;
              }
              default: break;
            }
          }
        }
      }
    }
    if (!result.empty()) { selector += "{" + result + "}"; }
  }
  else
  {
    LOG_DEBUG << "Error: Form Fields schema is empty";
    return {ok::ErrorCode::SCHEMA_ERROR, ""};
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, selector};
}
}  // namespace table_actor
namespace mutation
{
AQLQuery getDeepQueryFromResult(RelationQueryMap const &relationQueries) noexcept
{
  std::string result;
  for (auto &q : relationQueries) { result += q.second.query; }
  return result;
}
AQLQuery getProjectionFromResult(RelationQueryMap const &relationQueries) noexcept
{
  std::string result;
  if (!relationQueries.empty())
  {
    for (auto &q : relationQueries)
    {
      // result += ", FIRST(" + q.second.projection + ")";
      result += ", " + q.second.projection;
    }
  }
  return result;
}
}  // namespace mutation
namespace make_bind_vars
{
std::tuple<ErrorCode, ErrorMsg> fieldArgsValidateType(Field field, jsoncons::ojson const &arg)
{
  switch (field.type)
  {
    case FieldType::uninitialized:
    case FieldType::button:
    case FieldType::checkboxes:
    case FieldType::color:
    case FieldType::email:
    case FieldType::image:
    case FieldType::password:
    case FieldType::radio:
    case FieldType::range:
    case FieldType::reset:
    case FieldType::search:
    case FieldType::submit:
    case FieldType::tel:
    case FieldType::text:
    case FieldType::time:
    case FieldType::url:
    case FieldType::week:
    case FieldType::textarea:
    case FieldType::select:
    case FieldType::internal_true_edge:
    case FieldType::WYSIWYG:
    case FieldType::codemirror:
    case FieldType::save_time:
    case FieldType::dropzone:
    case FieldType::dateRange:
    case FieldType::emoji:
    case FieldType::mindMap:
    case FieldType::mapCountries:
    case FieldType::uuid:
    case FieldType::subdomain:
    case FieldType::chatInput:
      if (arg.is_string()) return {ok::ErrorCode::ERROR_NO_ERROR, ""};
      break;
    case FieldType::checkbox:
      if (arg.is_bool()) return {ok::ErrorCode::ERROR_NO_ERROR, ""};
      break;
    case FieldType::date:
    case FieldType::datetime_local:
    case FieldType::month:
    case FieldType::number:
    case FieldType::flatPicker:
    case FieldType::serial:
    case FieldType::inserted:
    case FieldType::updated:
      if (arg.is_number()) return {ok::ErrorCode::ERROR_NO_ERROR, ""};
      break;
    case FieldType::jsonEditor:
      if (arg.is_object() || arg.is_array()) return {ok::ErrorCode::ERROR_NO_ERROR, ""};
      break;
    case FieldType::multi_select:
    case FieldType::multi_select_hidden:
    case FieldType::text_array:
    case FieldType::multi_select_bool_properties:
      if (arg.is_array()) return {ok::ErrorCode::ERROR_NO_ERROR, ""};
      break;
    case FieldType::fieldSet:
    case FieldType::tab:
      if (arg.is_object()) return {ok::ErrorCode::ERROR_NO_ERROR, ""};
      break;
    case FieldType::file:
      if (arg.is_null() || jsoncons::isArrayOfString(arg)) return {ok::ErrorCode::ERROR_NO_ERROR, ""};
      break;
    default:
      if (arg.is_string()) return {ok::ErrorCode::ERROR_NO_ERROR, ""};
  }
  return {ok::ErrorCode::ERROR_BAD_PARAMETER, "bad type"};
}
auto validateAndSetFieldValue(ID const &id, Field &field, jsoncons::ojson const &formData)
{
  if (auto [er, msg] = fieldArgsValidateType(field, formData[field.selector]); ok::isEr(er))
  {
    LOG_DEBUG << "error: field value has invalid type. field: " << ok::mutate_schema::to_string(field) << " value: " << formData[field.selector].to_string();
    field.error = "bad type";
    return er;
  }
  else
    field.value = formData[id];
  return ok::ErrorCode::ERROR_NO_ERROR;
}
bool alwaysSetDefaultValue(Field &field)
{
  switch (field.type)
  {
    case FieldType::save_time:
    {
      field.value = utils::time::getEpochMilliseconds();
      return true;
    }
    case FieldType::inserted:  // note: on update inserted filed is skipped...
    case FieldType::updated:
    {
      field.value = utils::time::getEpochMilliseconds();
      return true;
    }
    default: return false;
  }
}
jsoncons::ojson getDefaultValue(FieldType type)
{
  switch (type)
  {
    case FieldType::uninitialized:
    case FieldType::button:
    case FieldType::checkboxes:
    case FieldType::color:
    case FieldType::email:
    case FieldType::image:
    case FieldType::password:
    case FieldType::radio:
    case FieldType::range:
    case FieldType::reset:
    case FieldType::search:
    case FieldType::submit:
    case FieldType::tel:
    case FieldType::text:
    case FieldType::time:
    case FieldType::url:
    case FieldType::week:
    case FieldType::textarea:
    case FieldType::select:
    case FieldType::WYSIWYG:
    case FieldType::codemirror:
    case FieldType::dropzone:
    case FieldType::dateRange:
    case FieldType::emoji:
    case FieldType::mindMap:
    case FieldType::mapCountries:
    case FieldType::uuid:
    case FieldType::subdomain:
    case FieldType::chatInput: return jsoncons::ojson{""};
    case FieldType::checkbox: return jsoncons::ojson{false};
    case FieldType::date:
    case FieldType::datetime_local:
    case FieldType::month:
    case FieldType::number:
    case FieldType::flatPicker:
    case FieldType::serial: return jsoncons::ojson{0};
    case FieldType::jsonEditor: return jsoncons::ojson{};
    case FieldType::multi_select:
    case FieldType::multi_select_hidden:
    case FieldType::text_array:
    case FieldType::multi_select_bool_properties: return jsoncons::ojson::array();
    case FieldType::fieldSet:
    case FieldType::tab: return jsoncons::ojson{};
    case FieldType::file: return jsoncons::ojson::null();
    case FieldType::save_time: return utils::time::getEpochMilliseconds();
    case FieldType::internal_true_edge: return jsoncons::ojson::null();
    case FieldType::inserted:  // note: on update inserted filed is skipped...
    case FieldType::updated: return utils::time::getEpochMilliseconds();
  }
}
ErrorCode setNonRelationFieldValues(Fields &fields, WsArguments const &formValue, WsArguments const &backendValue, bool ignoreMissingFieldWithValue) noexcept
{
  // PRECONDITION: only pass formData if it's an array
  ErrorCode formFieldError = ErrorCode::ERROR_NO_ERROR;
  for (auto fieldIt = fields.begin(); fieldIt != fields.end(); ++fieldIt)
  {
    if (impl::isRelation(fieldIt->second))
    {
      if (fieldIt->second.backEndOnly) continue;
      continue;
    }
    if (!fieldIt->second.backEndOnly)
    {
      if (formValue.has_key(fieldIt->second.selector))
      {
        auto er = validateAndSetFieldValue(fieldIt->first, fieldIt.value(), formValue);
        if (ok::isEr(er)) formFieldError = er;
      }
      else if (backendValue.has_key(fieldIt->second.selector))
      {
        auto er = validateAndSetFieldValue(fieldIt->first, fieldIt.value(), backendValue);
        if (ok::isEr(er)) formFieldError = er;
      }
      else if (alwaysSetDefaultValue(fieldIt.value()))
      {
      }
      else if (ignoreMissingFieldWithValue)
        fieldIt.value().ignoreMssingFieldWithValue = true;
      else
      {
        LOG_DEBUG << "error:field value not exist in arguments. field: " << ok::mutate_schema::to_string(fieldIt->second) << " values: " << formValue.to_string();
        fieldIt.value().error = "missing value";
        formFieldError = ok::ErrorCode::ERROR_BAD_PARAMETER;
      }
    }
    else
    {
      if (backendValue.has_key(fieldIt->second.selector))
      {
        auto er = validateAndSetFieldValue(fieldIt->first, fieldIt.value(), backendValue);
        if (ok::isEr(er)) formFieldError = er;
      }
      else if (alwaysSetDefaultValue(fieldIt.value()))
      {
      }
      else if (ignoreMissingFieldWithValue)
        fieldIt.value().ignoreMssingFieldWithValue = true;
      else
        fieldIt.value().value = getDefaultValue(fieldIt->second.type);
    }
  }
  if (ok::isEr(formFieldError)) { return formFieldError; }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
std::tuple<ErrorCode, jsoncons::ojson> makeFormObject(Fields &fields, FieldOrder const &allIds) noexcept
{
  jsoncons::ojson jsonResult;
  for (auto const &id : allIds)
  {
    if (impl::isRelation(fields[id])) { continue; }
    else if (fields[id].id == "_key" && ((fields[id].value.is_string() && fields[id].value.as_string().empty()) || fields[id].value.is_null()))
    {
      // nothing because if _key is "" it gives error: AQL: illegal document key (while executing)
      continue;
    }
    else
    {
      auto &field = fields[id];
      if (!field.ignoreMssingFieldWithValue) jsonResult[id] = field.value;
    }
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, jsonResult};
}
}  // namespace make_bind_vars
}  // namespace mutate_schema
}  // namespace ok
