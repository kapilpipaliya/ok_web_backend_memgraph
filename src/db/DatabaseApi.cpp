#include "db/DatabaseApi.hpp"
#include <velocypack/Parser.h>
#include "db/fuerteclientPool.hpp"
#include <velocypack/Builder.h>
#include <velocypack/velocypack-aliases.h>
#include <trantor/utils/Logger.h>
namespace
{
std::string catchError = "Exception caught when trying to connect database";
}
ApiResult Api::Database::PostDatabase::request(std::string const &name)
{
  try
  {
    VPackBuilder builder;
    builder.openObject();
    builder.add("name", VPackValue(name));
    builder.close();
    auto response = ok::db::fuerte_conns.requestDataRaw("_system", restVerb, url, builder.slice(), {});
    switch (response->statusCode())
    {
      case Api::Database::PostDatabase::Response201: return {ok::ErrorCode::ERROR_NO_ERROR, std::move(response)};
      case Api::Database::PostDatabase::Response400:
      case Api::Database::PostDatabase::Response403:
      case Api::Database::PostDatabase::Response409:
      default:
        LOG_ERROR << response->statusCode();
        LOG_ERROR << "Error Creating database";
        if (responses.contains(response->statusCode())) { LOG_ERROR << responses.at(response->statusCode()); }
        else
        {
          LOG_ERROR << "Response code not defined in map!: " << response->statusCode();
        }
        return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, std::move(response)};
    }
  }
  catch (...)
  {
    LOG_ERROR << catchError;
    return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, nullptr};
  }
}
ApiResult Api::Collection::PostCollection::request(std::string const &databaseName, std::string const &name, Api::Collection::PostCollection::type type)
{
  try
  {
    VPackBuilder builder;
    builder.openObject();
    builder.add("name", VPackValue(name));
    builder.add("type", VPackValue(type));
    builder.close();
    auto response = ok::db::fuerte_conns.requestDataRaw(databaseName, restVerb, url, builder.slice(), {});
    switch (response->statusCode())
    {
      case Api::Collection::PostCollection::Response200: return {ok::ErrorCode::ERROR_NO_ERROR, std::move(response)};
      case Api::Collection::PostCollection::Response400:
      case Api::Collection::PostCollection::Response404:
      default:
      {
        LOG_ERROR << response->statusCode();
        LOG_ERROR << "Error Creating collection";
        if (responses.contains(response->statusCode())) { LOG_ERROR << responses.at(response->statusCode()); }
        else
        {
          LOG_ERROR << "Response code not defined in map!";
        }
        return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, std::move(response)};
      }
    }
  }
  catch (...)
  {
    LOG_ERROR << catchError;
    return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, nullptr};
  }
}
ApiResult Api::Document::PostCollection::request(std::string const &databaseName, std::string const &collectionName, const arangodb::velocypack::Slice bindVarsObject)
{
  try
  {
    auto path = "/_api/document/" + std::string(collectionName) + "?returnNew=true&returnOld=true";
    auto response = ok::db::fuerte_conns.requestDataRaw(databaseName, restVerb, path, bindVarsObject, {});
    switch (response->statusCode())
    {
      case Api::Document::PostCollection::Response201:
      case Api::Document::PostCollection::Response202: return {ok::ErrorCode::ERROR_NO_ERROR, std::move(response)};
      case Api::Document::PostCollection::Response400:
      case Api::Document::PostCollection::Response404:
      case Api::Document::PostCollection::Response409:
      default:
        LOG_ERROR << response->statusCode();
        LOG_ERROR << responses.find(response->statusCode())->second;
        LOG_ERROR << "Error Inserting Document";
        if (responses.contains(response->statusCode())) { LOG_ERROR << responses.at(response->statusCode()); }
        else
        {
          LOG_ERROR << "Response code not defined in map!";
        }
        return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, std::move(response)};
    }
  }
  catch (...)
  {
    LOG_ERROR << catchError;
    return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, nullptr};
  }
}
ApiResult Api::Index::PostIndexPersistent::request(std::string const &databaseName, std::string const &collectionName, std::vector<std::string> const &fieldNames)
{
  try
  {
    std::string path = "/_api/index?collection=" + std::string(collectionName);
    VPackBuilder builder;
    builder.openObject();
    builder.add("type", VPackValue("persistent"));
    builder.add("unique", VPackValue(true));
    {
      VPackBuilder array_builder;
      array_builder(VPackValue(VPackValueType::Array));
      for (auto &fieldName : fieldNames) array_builder.add(VPackValue(fieldName));
      array_builder.close();
      builder.add("fields", array_builder.slice());
    }
    builder.close();
    auto response = ok::db::fuerte_conns.requestDataRaw(databaseName, restVerb, path, builder.slice(), {});
    switch (response->statusCode())
    {
      case Response201: return {ok::ErrorCode::ERROR_NO_ERROR, std::move(response)};
      case Response400:
      case Response404:
      default:
        LOG_ERROR << response->statusCode();
        LOG_ERROR << "Error Creating Index";
        if (responses.contains(response->statusCode())) { LOG_ERROR << responses.at(response->statusCode()); }
        else
        {
          LOG_ERROR << "Response code not defined in map!";
        }
        return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, std::move(response)};
    }
  }
  catch (...)
  {
    LOG_ERROR << catchError;
    return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, nullptr};
  }
}
ApiResult Api::AQL::PostQuery::request(std::string const &databaseName, std::string const &body)
{
  // This endpoint is for query validation only. To actually query the database,
  // see /api/cursor.
  // fix this: this not accept query in json
  try
  {
    VPackBuilder builder;
    builder.openObject();
    builder.add("query", VPackValue(body));
    builder.close();
    auto response = ok::db::fuerte_conns.requestDataRaw(databaseName, restVerb, url, builder.slice(), {});
    if (response->statusCode() < 300)
    {
      // resp_code.value();
      return {ok::ErrorCode::ERROR_NO_ERROR, std::move(response)};
    }
    else
    {
      LOG_ERROR << response->statusCode();
      LOG_ERROR << "Response code not defined in map!";
      return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, std::move(response)};
    }
  }
  catch (...)
  {
    LOG_ERROR << catchError;
    return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, nullptr};
  }
}
ApiResult Api::Cursor::PostCursor::request(std::string const &databaseName, const std::string &body, const arangodb::velocypack::Slice bindVarsObject)
{
  try
  {
    VPackBuilder builder;
    builder.openObject();
    builder.add("query", VPackValue(body));
    builder.add("count", VPackValue(true));
    if (!bindVarsObject.isNone()) { builder.add("bindVars", bindVarsObject); }
    {
      builder.add("options", VPackValue(VPackValueType::Object));
      builder.add("fullCount", VPackValue(true));
      builder.close();
    }
    builder.close();
    // std::unordered_map<std::string, std::string> headerFields;
    // headerFields.insert(std::make_pair("fullCount", "true"));
    auto response = ok::db::fuerte_conns.requestDataRaw(databaseName, restVerb, url, builder.slice(), {});
    switch (response->statusCode())
    {
      case Api::Cursor::PostCursor::Response201: return {ok::ErrorCode::ERROR_NO_ERROR, std::move(response)};
      case Api::Cursor::PostCursor::Response400:
      case Api::Cursor::PostCursor::Response404:
      case Api::Cursor::PostCursor::Response405:
      default:
        LOG_ERROR << response->statusCode();
        LOG_ERROR << "Error on Post Cursor";
        if (responses.contains(response->statusCode())) { LOG_ERROR << responses.at(response->statusCode()); }
        else
        {
          LOG_ERROR << "Response code not defined in map!";
        }
        return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, std::move(response)};
    }
  }
  catch (...)
  {
    LOG_ERROR << catchError;
    return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, nullptr};
  }
}
ApiResult Api::Database::GetDatabase::request(std::string const &databaseName)
{
  try
  {
    VPackBuilder builder;
    auto response = ok::db::fuerte_conns.requestDataRaw(databaseName, restVerb, url, builder.slice(), {});
    if (response->statusCode() < 300) { return {ok::ErrorCode::ERROR_NO_ERROR, std::move(response)}; }
    else
    {
      LOG_ERROR << response->statusCode();
      LOG_ERROR << "Response code not defined in map!";
      return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, std::move(response)};
    }
  }
  catch (...)
  {
    LOG_ERROR << catchError;
    return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, nullptr};
  }
}
ApiResult Api::Document::GetDocumentHandle::request(std::string const &databaseName, const std::string &collectionName, std::string const &key)
{
  try
  {
    std::string path = "/_api/document/" + std::string(collectionName) + "/" + key;
    auto response = ok::db::fuerte_conns.requestDataRaw(databaseName, restVerb, path, {}, {});
    switch (response->statusCode())
    {
      case Response200: return {ok::ErrorCode::ERROR_NO_ERROR, std::move(response)};
      case Response304:
      case Response404:
      case Response412:
      default:
        LOG_ERROR << response->statusCode();
        LOG_ERROR << "Error Getting document";
        if (responses.contains(response->statusCode())) { LOG_ERROR << responses.at(response->statusCode()); }
        else
        {
          LOG_ERROR << "Response code not defined in map!";
        }
        return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, std::move(response)};
    }
  }
  catch (...)
  {
    LOG_ERROR << catchError;
    return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, nullptr};
  }
}
ApiResult Api::Document::DeleteDocumentHandle::request(std::string const &databaseName, const std::string &collectionName, std::string const &key)
{
  try
  {
    std::string path = "/_api/document/" + std::string(collectionName) + "/" + key;
    VPackBuilder builder;
    builder.openObject();
    builder.add("silent", VPackValue(false));
    builder.close();
    auto response = ok::db::fuerte_conns.requestDataRaw(databaseName, restVerb, path, builder.slice(), {});
    switch (response->statusCode())
    {
      case Api::Document::DeleteDocumentHandle::Response200:
      case Api::Document::DeleteDocumentHandle::Response202: return {ok::ErrorCode::ERROR_NO_ERROR, std::move(response)};
      case Api::Document::DeleteDocumentHandle::Response404:
      case Api::Document::DeleteDocumentHandle::Response412:
      default:
        LOG_ERROR << response->statusCode();
        LOG_ERROR << "Error Removing document";
        if (responses.contains(response->statusCode())) { LOG_ERROR << responses.at(response->statusCode()); }
        else
        {
          LOG_ERROR << "Response code not defined in map!";
        }
        return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, std::move(response)};
    }
  }
  catch (...)
  {
    LOG_ERROR << catchError;
    return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, nullptr};
  }
}
// clang-format on
ApiResult Api::Database::DeleteDatabaseName::request(std::string const &name)
{
  try
  {
    std::string path = "/_api/database/" + std::string(name);
    auto response = ok::db::fuerte_conns.requestDataRaw("_system", restVerb, path, {}, {});
    if (response->statusCode() < 300) { return {ok::ErrorCode::ERROR_NO_ERROR, std::move(response)}; }
    else
    {
      LOG_ERROR << response->statusCode();
      LOG_ERROR << "Response code not defined in map!";
      return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, std::move(response)};
    }
  }
  catch (...)
  {
    LOG_ERROR << catchError;
    return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, nullptr};
  }
}
ApiResult Api::Administration::GetVersion::request()
{
  try
  {
    std::string path = url;
    path += "?details=true";
    auto response = ok::db::fuerte_conns.requestDataRaw("", restVerb, path, {}, {});
    if (response->statusCode() < 300) { return {ok::ErrorCode::ERROR_NO_ERROR, std::move(response)}; }
    else
    {
      LOG_ERROR << response->statusCode();
      LOG_ERROR << "Response code not defined in map!";
      return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, std::move(response)};
    }
  }
  catch (...)
  {
    LOG_ERROR << catchError;
    return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, nullptr};
  }
  // response = std::move(response);
}
