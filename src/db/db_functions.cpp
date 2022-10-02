#include "db/db_functions.hpp"
#include "db/DatabaseApi.hpp"
#include <trantor/utils/Logger.h>
#include "utils/time_functions.hpp"
#include "db/fuerteclient.hpp"
#include <velocypack/Iterator.h>
#include <absl/strings/str_format.h>
#include <velocypack/Collection.h>
#include "jwt/jwt.hpp"
#include "utils/html_functions.hpp"
#include "utils/ErrorConstants.hpp"
#include "utils/GlobalSettings.hpp"
namespace ok
{
namespace db
{
void waitForFuerte()
{
  constexpr auto domain = "vst://localhost:8529";
  constexpr auto userName = "root";
  constexpr auto password = "";
  while (true)
  {
    auto connPtr = std::make_shared<FuerteClient>(domain, userName, password, "");
    connPtr->connect();
    if (connPtr->isConnected())
    {
      ok::utils::time::waitSeconds(5);  // sleep to pass maintenance mode.
      break;
    }
    LOG_DEBUG << "waiting for database..";
  }
}
bool isDatabaseServerConnected()
{
  constexpr auto domain = "vst://localhost:8529";
  constexpr auto userName = "root";
  constexpr auto password = "";
  FuerteClient f{domain, userName, password, "_system"};
  f.connect();
  if (f.isConnected()) { return true; }
  else
  {
    LOG_DEBUG << "cant connect to database server while status check";
    return false;
  }
}
ok::ErrorCode copyGlobalCollection(Database const &database, Collection const &collection)
{
  constexpr auto getData = "FOR d in @@collection RETURN d";
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("@collection", arangodb::velocypack::Value(collection));
  bindVars.close();
  auto [erDb, response] = Api::Cursor::PostCursor::request("global", getData, bindVars.slice());
  if (isEr(erDb)) { return ok::ErrorCode::BAD_QUERY; }
  auto slice = response->slices().front();
  constexpr auto insertQuery = "FOR d in @insertData INSERT d in @@collection";
  arangodb::velocypack::Builder bindVars2;
  bindVars2.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars2.add("insertData", slice.get("result"));
  bindVars2.add("@collection", arangodb::velocypack::Value(collection));
  bindVars2.close();
  auto [erDb2, response2] = Api::Cursor::PostCursor::request(database, insertQuery, bindVars2.slice());
  if (isEr(erDb2)) { return ok::ErrorCode::BAD_QUERY; }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
ok::ErrorCode copyGlobalCollections(Database const &database)
{
  if (auto [er, myResp] = getSchema("global", "schema", "seed", ".form"); ok::isEr(er)) { return er; }
  else
  {
    auto slice = myResp->slices().front().get("result");
    if (slice.length() == 1)
    {
      auto copyCollectionsArray = slice[0].get("copy_global_on_user_registration");
      for (auto const &it : arangodb::velocypack::ArrayIterator(copyCollectionsArray))
      {
        if (auto er2 = copyGlobalCollection(database, it.copyString()); ok::isEr(er)) { return er2; }
      }
    }
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
std::tuple<ok::ErrorCode, Database> saveUser(std::string const &email, std::string const &password, std::string const &url)
{
  // if I want to make https://discordapp.com/  all users and members must be
  // saved to redis cache too.
  auto query = R"aql(FOR u IN user
  COLLECT
  AGGREGATE maxNo = MAX(TO_NUMBER(u.no))
  RETURN {
    maxNo: maxNo || 0.0
  })aql";
  auto [erDb, response] = Api::Cursor::PostCursor::request("user", query, {});
  if (isEr(erDb)) { return {ok::ErrorCode::BAD_QUERY, ""}; }
  auto slice = response->slices().front();
  auto result = slice.get("result");
  auto db_number = 0;
  if (result.length() == 1 && result[0].get("maxNo").isDouble()) { db_number = result[0].get("maxNo").getDouble() + 1; }
  else
  {
    return {ok::ErrorCode::BAD_QUERY, ""};
  }
  auto database = "user_" + std::to_string(db_number);
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("_key", arangodb::velocypack::Value(std::to_string(db_number)));
  bindVars.add("no", arangodb::velocypack::Value(db_number));
  bindVars.add("database", arangodb::velocypack::Value(database));
  bindVars.add("email", arangodb::velocypack::Value(email));
  bindVars.add("pass", arangodb::velocypack::Value(password));
  bindVars.add("url", arangodb::velocypack::Value(url));
  bindVars.add("createdAt", arangodb::velocypack::Value(utils::time::getEpochMilliseconds()));
  bindVars.close();
  auto [erDb2, response2] = Api::Document::PostCollection::request("user", "user", bindVars.slice());
  if (isEr(erDb2)) { return {ok::ErrorCode::BAD_QUERY, ""}; }
  // auto slice = pc.response->slices().front();
  // userKey = slice.get("_key").copyString();
  return {ok::ErrorCode::ERROR_NO_ERROR, database};
  // required super_actor and parent actor to spawn.
  /*request(super_actor, caf::infinite, caf::get_atom_v, user_list_actor)
      .then([=, this](table_actor_int org_actor) {
        request(org_actor, caf::infinite, caf::send_all_atom_v, event_key,
                filterKey, result_key);
      });
  unbecome();*/
}
ok::ErrorCode isDatabaseExist(Database const &dbName)
{
  auto [erDb, response] = Api::Database::GetDatabase::request("_system");
  if (isEr(erDb)) { return ok::ErrorCode::BAD_QUERY; }
  auto slice = response->slices().front();
  auto result = slice.get("result");  // ["_system",..]
  if (auto is_member = arangodb::velocypack::Collection::contains(result, [&dbName](VPackSlice const &s, arangodb::velocypack::ValueLength index) { return s.copyString() == dbName; }); !is_member)
  {
    return ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE;
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
ok::ErrorCode createDatabase(Database const &database)
{
  auto [erDb, response] = Api::Database::PostDatabase::request(database);
  if (isEr(erDb)) { return ok::ErrorCode::BAD_QUERY; }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
ok::ErrorCode deleteDatabase(Database const &database)
{
  auto [erDb, response] = Api::Database::DeleteDatabaseName::request(database);
  if (isEr(erDb)) { return erDb; }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
ok::ErrorCode createCollections(Database const &database)
{
  auto createCol = [&](std::string const &collectionName, Api::Collection::PostCollection::type type)
  {
    auto [erDb, response] = Api::Collection::PostCollection::request(database, collectionName, type);
    if (isEr(erDb))
    {
      std::string err = "critical error. cant create collections: ";
      err += collectionName;
    }
  };
  auto createIndex = [&](std::string const &collectionName, std::vector<std::string> &fieldNames)
  {
    auto [erDb, response] = Api::Index::PostIndexPersistent::request(database, collectionName, fieldNames);
    if (isEr(erDb))
    {
      std::string err = "critical error. cant create Indexe: ";
      err += collectionName;
    }
  };
  if (auto [er, myResp] = getSchema("global", "schema", "seed", ".form"); ok::isEr(er))
  {
    // std::string err = "Empty result while getting collection and edges";
    return ok::ErrorCode::BAD_QUERY;
  }
  else
  {
    auto slice = myResp->slices().front().get("result");
    if (slice.length() == 1)
    {
      auto object = slice[0];
      auto collections = object.get("collections");
      for (auto const &it : arangodb::velocypack::ArrayIterator(collections)) { createCol(it.copyString(), Api::Collection::PostCollection::document_collection); }
      auto edgesArray = object.get("edges");
      for (auto const &it : arangodb::velocypack::ArrayIterator(edgesArray)) { createCol(it.copyString(), Api::Collection::PostCollection::edge_collection); }
      auto IndexesArray = object.get("uniqueIndexes");
      for (auto const &it : arangodb::velocypack::ArrayIterator(IndexesArray))
      {
        std::vector<std::string> fieldNames;
        for (auto const &it : arangodb::velocypack::ArrayIterator(it["fields"])) { fieldNames.push_back(it.copyString()); }
        createIndex(it["collection"].copyString(), fieldNames);
      }
      return ok::ErrorCode::ERROR_NO_ERROR;
    }
  }
  return ok::ErrorCode::BAD_QUERY;  // "many values not possible";
}
ok::ErrorCode createPermissions(Database const &database)
{
  constexpr absl::string_view y = R"aql(
LET groups = [
  {"_key": "admin", id: "admin", name: "Admin", "lock": true},
  {"_key": "guest", id: "guest", name: "Guest", "lock": true}
]
let gps = (FOR g in groups INSERT g INTO group RETURN NEW._id)

LET group_has_roles = [
  {_from: "group/admin", _to: "role/admin", "lock": true},
  {_from: "group/guest", _to: "role/guest", "lock": true}
]
LET has_role = (FOR r in group_has_roles INSERT r INTO has_role RETURN NEW._id)

LET has_members = [
  {_from: "group/admin", _to: "member/1", "lock": true},
  {_from: "group/guest", _to: "member/guest", "lock": true}
]
LET has_member = (FOR m in has_members INSERT m INTO has_member RETURN NEW._id)
RETURN true
)aql";
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, y.data(), {});
  // auto slice = resp->slices().front();
  if (isEr(erDb)) { return ok::ErrorCode::BAD_QUERY; }
  return ok::ErrorCode::ERROR_NO_ERROR;
  /*  constexpr absl::string_view y = R"(
  {
    "organization": {
      "permissions": [
        %s
      ],
      "children": {
        "project": {
          "permissions": [
            %s
          ],
          "children": {
            "work_package": {
              "permissions": [
                %s
              ]
            }
          }
        }
      }
    }
  }
  )";
    std::string org_permissions = std::to_string(1);
    std::string pro_permissions = std::to_string(1);
    std::string work_permissions = std::to_string(1);
    auto super_admin =
        absl::StrFormat(y, org_permissions, pro_permissions, work_permissions);

    Api::Document::PostCollection a;
a.request(
        f, permissions, super_admin);

    if (Api::Document::PostCollection::is_success()) {
      auto slice = response->slices().front();
      session->memberKey =
  g->removequote(slice.get("_key").toString()); } else {
      ErrorCode = std::to_string(response->statusCode()) +
                          "Error inserting permission document";
    }*/
}
std::tuple<ok::ErrorCode, DocumentKey> createMember(
    Database const &database, std::string const &firstName, std::string const &lastName, std::string const &email, std::string const &password, std::string const &key, bool isAdmin, bool lock)
{
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  if (!key.empty()) { bindVars.add("_key", arangodb::velocypack::Value(key)); }
  bindVars.add("firstName", arangodb::velocypack::Value(firstName));
  bindVars.add("lastName", arangodb::velocypack::Value(lastName));
  bindVars.add("email", arangodb::velocypack::Value(email));
  bindVars.add("pass", arangodb::velocypack::Value(password));
  bindVars.add("isAdmin", arangodb::velocypack::Value(isAdmin));
  bindVars.add("emailVerified", arangodb::velocypack::Value(false));
  bindVars.add("lock", arangodb::velocypack::Value(lock));
  bindVars.add("createdAt", arangodb::velocypack::Value(utils::time::getEpochMilliseconds()));
  bindVars.close();
  auto [erDb, response] = Api::Document::PostCollection::request(database, "member", bindVars.slice());
  if (isEr(erDb)) { return {ok::ErrorCode::BAD_QUERY, ""}; }
  auto slice = response->slices().front();
  auto memberKey = slice.get("_key").copyString();
  return {ok::ErrorCode::ERROR_NO_ERROR, memberKey};
}
ok::ErrorCode createMemberPermissions(Database const &database, DocumentKey const &memberKey)
{
  constexpr absl::string_view y = R"aql(
LET has_member = (INSERT {_from: "group/guest", _to: "member/%s","lock": true} INTO has_member RETURN true)
RETURN true
)aql";
  auto perm_query = absl::StrFormat(y, memberKey);
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, perm_query.data(), {});
  if (isEr(erDb)) { return ok::ErrorCode::BAD_QUERY; }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
std::tuple<ok::ErrorCode, Database> findDatabaseNameFromSubdomain(std::string const &subDomain)
{
  auto query = R"aql(FOR doc IN @@collection
  FILTER doc.`url` == @value
  RETURN doc)aql";
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("@collection", arangodb::velocypack::Value("user"));
  bindVars.add("value", arangodb::velocypack::Value(subDomain));
  bindVars.close();
  auto [erDb, response] = Api::Cursor::PostCursor::request("user", query, bindVars.slice());
  if (isEr(erDb)) { return {ok::ErrorCode::BAD_QUERY, ""}; }
  auto slice = response->slices().front();
  auto result = slice.get("result");
  if (result.length() == 0) { return {ok::ErrorCode::ERROR_HTTP_NOT_FOUND, ""}; }
  else
  {
    return {ok::ErrorCode::ERROR_NO_ERROR, result[0].get("database").copyString()};
  }
}
std::tuple<ok::ErrorCode, Database> findDatabaseFromUserEmail(std::string const &email)
{
  auto query = R"aql(FOR doc IN @@collection
  FILTER doc.`email` == @value
  RETURN doc)aql";
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("@collection", arangodb::velocypack::Value("user"));
  bindVars.add("value", arangodb::velocypack::Value(email));
  bindVars.close();
  auto [erDb, response] = Api::Cursor::PostCursor::request("user", query, bindVars.slice());
  if (isEr(erDb)) { return {ok::ErrorCode::BAD_QUERY, ""}; }
  auto slice = response->slices().front();
  auto result = slice.get("result");
  if (result.length() == 0) { return {ok::ErrorCode::ERROR_HTTP_NOT_FOUND, ""}; }
  else
  {
    return {ok::ErrorCode::ERROR_NO_ERROR, result[0].get("database").copyString()};
  }
}
std::tuple<ok::ErrorCode, DocumentKey, Email, bool> findMember(Database const &database, std::string const &email, std::string const &password)
{
  auto query = R"aql(FOR doc IN @@collection
  FILTER doc.`email` == @email && doc.pass == @pass
  RETURN doc)aql";
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("@collection", arangodb::velocypack::Value("member"));
  bindVars.add("email", arangodb::velocypack::Value(email));
  bindVars.add("pass", arangodb::velocypack::Value(password));
  bindVars.close();
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  if (isEr(erDb)) { return {ok::ErrorCode::BAD_QUERY, "", "", false}; }
  auto slice = response->slices().front();
  auto result = slice.get("result");
  if (result.length() == 0)
  {
    // return {"Email or password is invalid.", {}};
    return {ok::ErrorCode::PASSWORD_NOT_MATCH, "", "", false};
  }
  else
  {
    auto _key = result[0].get("_key").copyString();
    auto emailVerified = result[0].get("emailVerified").getBool();
    auto email2 = result[0].get("email").copyString();
    return {ok::ErrorCode::ERROR_NO_ERROR, _key, email2, emailVerified};
  }
}
std::tuple<ok::ErrorCode, DocumentKey, Email, bool> findMember(Database const &database, DocumentKey const &memberKey)
{
  auto query = R"aql(FOR doc IN @@collection
  FILTER doc.`_key` == @key
  RETURN doc)aql";
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("@collection", arangodb::velocypack::Value("member"));
  bindVars.add("key", arangodb::velocypack::Value(memberKey));
  bindVars.close();
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  if (isEr(erDb))
  {
    if (response)
    {
      auto slice = response->slices().front();
      LOG_FATAL << "Error: " << slice.toJson();
    }
    LOG_FATAL << "Error: " << errno_string(erDb) << " | executed query: " << query << " | database: " << database << "| bindVars: " << bindVars.toString();
    return {ok::ErrorCode::BAD_QUERY, "", "", false};
  }
  auto slice = response->slices().front();
  if (auto result = slice.get("result"); result.length() == 0)
  {
    // return {"Email or password is invalid.", {}};
    return {ok::ErrorCode::PASSWORD_NOT_MATCH, "", "", false};
  }
  else
  {
    auto _key = result[0].get("_key").copyString();
    auto emailVerified = result[0].get("emailVerified").getBool();
    auto email = result[0].get("email").copyString();
    return {ok::ErrorCode::ERROR_NO_ERROR, _key, email, emailVerified};
  }
}
ok::ErrorCode deleteConfirmDocument(DocumentKey const &confirmKey)
{
  std::string err;
  auto [erDb, response] = Api::Document::DeleteDocumentHandle::request("user", "confirm", confirmKey);
  if (isEr(erDb)) { return ok::ErrorCode::BAD_QUERY; }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
bool cleanDBSession(DocumentKey const &sessionKey)
{
  if (!sessionKey.empty())
  {
    auto [erDb, response] = Api::Document::DeleteDocumentHandle::request("user", "session", sessionKey);
    if (isEr(erDb)) { LOG_DEBUG << "error in deleting document"; }
    else
    {
      return true;
    }
  }
  return false;
}
ok::ErrorCode createLoginDbSession(ok::smart_actor::connection::Session &session)
{
  std::string err;
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  // bindVars.add("_key",arangodb::velocypack::Value(drogon::utils::getUuid()));
  bindVars.add("server", arangodb::velocypack::Value(session.database));
  bindVars.add("member_key", arangodb::velocypack::Value(session.memberKey));
  auto createdAt = utils::time::getEpochMilliseconds();
  bindVars.add("createdAt", arangodb::velocypack::Value(createdAt));
  bindVars.close();
  auto [erDb, response] = Api::Document::PostCollection::request("user", "session", bindVars.slice());
  if (isEr(erDb))
  {
    LOG_DEBUG << "Error when creating a session";
    return ok::ErrorCode::BAD_QUERY;
  }
  auto slice = response->slices().front();
  session.sessionKey = slice.get("_key").copyString();
  session.sessionEpoch = createdAt;
  return ok::ErrorCode::ERROR_NO_ERROR;
}
ok::ErrorCode setMemberEmailConfirmed(Database const &database, DocumentKey const &memberKey)
{
  std::string err;
  auto query = R"aql(UPDATE @key WITH { emailVerified: true } IN member)aql";
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("key", arangodb::velocypack::Value(memberKey));
  bindVars.close();
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  if (isEr(erDb))
  {
    err += "cant modify member";
    return ok::ErrorCode::ERROR_HTTP_SERVER_ERROR;
  }
  else
  {
    return ok::ErrorCode::ERROR_NO_ERROR;
  }
}
std::tuple<ok::ErrorCode, std::string> generateConfirmUrl(Database const &database, DocumentKey const &memberKey)
{
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("server", arangodb::velocypack::Value(database));
  bindVars.add("member_key", arangodb::velocypack::Value(memberKey));
  auto createdAt = utils::time::getEpochMilliseconds();
  bindVars.add("createdAt", arangodb::velocypack::Value(createdAt));
  bindVars.close();
  auto [erDb, response] = Api::Document::PostCollection::request("user", "confirm", bindVars.slice());
  if (isEr(erDb))
  {
    LOG_DEBUG << "Error when creating confirm document";
    return {ok::ErrorCode::BAD_QUERY, ""};
  }
  auto slice = response->slices().front();
  auto confirmKey = slice.get("_key").copyString();
  // auto wsConfirmUrl =
  // ok::smart_actor::connection::serverUrl + std::string("/account/confirm?key=") + key + "&token=" + std::to_string(createdAt);
  auto confirmUrl = ok::smart_actor::connection::serverUrl + std::string("/confirm/") + confirmKey + "/" + std::to_string(createdAt);
  return {ok::ErrorCode::ERROR_NO_ERROR, confirmUrl};
}
ok::ErrorCode uniqueMemberFieldCheck(Database const &database, Collection const &collection, const std::string &fieldName, const std::string &fieldValue)
{
  constexpr absl::string_view query_format = R"aql(FOR doc IN %s
  FILTER doc.`%s` == @value
  RETURN doc)aql";
  auto query = absl::StrFormat(query_format, collection, fieldName);
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("value", arangodb::velocypack::Value(fieldValue));
  bindVars.close();
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  if (isEr(erDb)) { return ok::ErrorCode::BAD_QUERY; }
  auto slice = response->slices().front();
  if (auto result = slice.get("result"); result.length() == 0) { return ok::ErrorCode::ERROR_NO_ERROR; }
  // return fieldName + " already exists. please choose other email.";
  return ok::ErrorCode::UNIQUE_ERROR;
}
void cleanExpiredCookies()
{
  constexpr auto query =
      R"aql(let yesterday = DATE_TIMESTAMP(DATE_SUBTRACT(DATE_NOW(), 1, "day"))
                  for d in session
                      FILTER d.createdAt < yesterday
                      REMOVE d IN session
                      return true)aql";
  auto [erDb, response] = Api::Cursor::PostCursor::request("user", query, {});
  if (isEr(erDb)) { return; }
  auto slice = response->slices().front();
  if (auto result = slice.get("result"); result.length() > 0) { LOG_DEBUG << result.length() << " sessions cleared"; }
}
void authenticateAndSaveSession(std::string const &jwtEncoded, ok::smart_actor::connection::Session &session, std::string const &subDomain)
{
  std::string sessionKey;
  auto dec_obj = ok::utils::jwt_functions::decodeCookie(jwtEncoded);
  if (dec_obj.payload().has_claim("sessionKey")) { sessionKey = dec_obj.payload().get_claim_value<std::string>("sessionKey"); }
  if (!sessionKey.empty())
  {
    auto query = R"aql(FOR doc IN session
                      FILTER doc.`_key` == @key
                      RETURN doc)aql";
    arangodb::velocypack::Builder bindVars;
    bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
    bindVars.add("key", arangodb::velocypack::Value(sessionKey));
    bindVars.close();
    auto [erDb, response] = Api::Cursor::PostCursor::request("user", query, bindVars.slice());
    if (isEr(erDb)) { LOG_DEBUG << "error in query. session not found"; }
    else
    {
      auto slice = response->slices().front();
      auto result = slice.get("result");
      if (result.length() == 0) { return; }
      {
        auto session_epoch = result[0].get("createdAt").getInt();
        constexpr unsigned long milliSeconds = 24 * 60 * 60 * 1000;
        if (auto now_epoch = utils::time::getEpochMilliseconds(); now_epoch < session_epoch + milliSeconds)
        {
          session.database = result[0].get("server").copyString();
          if (session.database.empty())
          {
            LOG_DEBUG << "There is error on saved session. server must not be empty";
            return;
          }
          session.sessionKey = sessionKey;
          session.sessionEpoch = session_epoch;
          session.memberKey = result[0].get("member_key").copyString();
          if (constexpr unsigned long one_hour = 1 * 60 * 60 * 1000; session_epoch + one_hour > now_epoch)
          {
            auto newSessionEpoch = expandCookieTime(session.sessionKey);
            if (newSessionEpoch > 0) { session.sessionEpoch = newSessionEpoch; }
          }
        }
        else
        {
          // TODO: when logging out every pages should be redirect
          // to login page.
        }
      }
    }
  }
  else
  {
    if (auto [er2, database] = findDatabaseNameFromSubdomain(subDomain); isEr(er2)) { LOG_DEBUG << "This should not happen"; }
    else
      session.database = std::move(database);
  }
}
unsigned long expandCookieTime(DocumentKey const &sessionKey)
{
  constexpr absl::string_view update_ = R"aql(
    UPDATE "%s" WITH { createdAt: %u } IN session
)aql";
  auto createdAt = utils::time::getEpochMilliseconds();
  auto query = absl::StrFormat(update_, sessionKey, createdAt);
  auto [erDb, response] = Api::Cursor::PostCursor::request("user", query, {});
  if (isEr(erDb))
  {
    auto slice = response->slices().front();
    LOG_DEBUG << "Error updating session: " << slice.toJson() << " | executed query: " << query << " | database is: "
              << "user"
              << " | collection is: "
              << "session";
  }
  else
  {
    return createdAt;
  }
  return 0;
}
std::tuple<ok::ErrorCode, Database, DocumentKey> confirmMember(DocumentKey const &confirmKey, std::string const &token)
{
  // ErrorCode ErrorCode;
  if (!confirmKey.empty() && !token.empty())
  {
    auto createdAt = std::stoul(token);
    auto createdAt_ = utils::time::getEpochMilliseconds() + (60 * 60 * 24 * 1000);
    if (bool is_expired = createdAt >= createdAt_; is_expired)
    {
      // ErrorCode = "Error: Verification Link Expired.";
      return {ok::ErrorCode::EXPIRED, "", ""};
    }
    Database dbName;
    unsigned long dbToken = 0;
    DocumentKey memberKey;
    auto [erDb, response] = Api::Document::GetDocumentHandle::request("user", "confirm", confirmKey);
    if (isEr(erDb))
    {
      if (response)
      {
        auto slice = response->slices().front();
        LOG_FATAL << "Error: " << slice.toJson();
      }
      LOG_FATAL << "Error: " << errno_string(erDb) << "error getting confirm document: " << confirmKey;
      return {ok::ErrorCode::ERROR_FAILED, "", ""};
    }
    else
    {
      auto slice = response->slices().front();
      // we get object:
      dbToken = slice.get("createdAt").getUInt();
      memberKey = slice.get("member_key").copyString();
      dbName = slice.get("server").copyString();
    }
    if (dbToken == 0)
    {
      // ErrorCode = "Error: Confirmation link is removed. try to re-send confirmation link";
      return {ok::ErrorCode::EXPIRED, "", ""};
    }
    if (dbName.empty())
    {
      // ErrorCode = "Error: Database not exist";
      return {ok::ErrorCode::CANNOT_CONNECT_TO_DATABASE, "", ""};
    }
    if (dbToken == createdAt)
    {
      if (auto conf_err = setMemberEmailConfirmed(dbName, memberKey); ok::isEr(conf_err))
      {
        // ErrorCode = "Error: can't verify member. please try to re-send confirmation. " + conf_err;
        return {ok::ErrorCode::EXPIRED, "", ""};
      }
      deleteConfirmDocument(confirmKey);
      // todo: send Email: "Thank you. Your Email is successfully confirmed.";
      return {ok::ErrorCode::ERROR_NO_ERROR, dbName, memberKey};
    }
  }
  return {ok::ErrorCode::ERROR_BAD_PARAMETER, "", ""};
}
std::tuple<ok::ErrorCode, std::unique_ptr<arangodb::fuerte::Response> > runQuery(Database const &database, AQLQuery const &schemaQuery)
{
  auto [erDb, resp] = Api::Cursor::PostCursor::request(database, schemaQuery, {});
  if (isEr(erDb))  // if (slice.get("error").getBool()) {
  {
    auto slice = resp->slices().front();
    LOG_DEBUG << "Error: " << slice.toJson() << " | executed query: " << schemaQuery << " | database is: " << database;
    return {ok::ErrorCode::BAD_QUERY, std::move(resp)};
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, std::move(resp)};
}
std::tuple<ok::ErrorCode, std::unique_ptr<arangodb::fuerte::Response> > getDocumentWithProjection(Database const &database,
                                                                                                  Collection const &collection,
                                                                                                  std::string const &key,
                                                                                                  std::function<std::string(std::string const &)> const projection) noexcept
{
  std::string schemaQuery = "FOR o IN " + collection + " FILTER o._key == '" + key + "' RETURN " + projection("o");
  return runQuery(database, schemaQuery);
}
std::tuple<ok::ErrorCode, std::unique_ptr<arangodb::fuerte::Response> > getDocumentWithProjection(Database const &database,
                                                                                                  Collection const &collection,
                                                                                                  std::string const &key,
                                                                                                  std::string const &projection) noexcept
{
  std::string schemaQuery = "FOR o IN " + collection + " FILTER o._key == '" + key + "' RETURN o" + projection;
  return runQuery(database, schemaQuery);
}
std::tuple<ok::ErrorCode, std::unique_ptr<arangodb::fuerte::Response> > getSchema(Database const &database,
                                                                                  Collection const &collection,
                                                                                  std::string const &key,
                                                                                  std::string const &projection) noexcept
{
  std::string schemaQuery = "FOR o IN " + collection + " FILTER o._key == '" + key + "' RETURN o" + projection;
  auto [ok, resp] = runQuery(database, schemaQuery);
  if (ok::isEr(ok)) return {ok, std::move(resp)};
  auto slice = resp->slices().front();
  if (slice.hasKey("result") && slice.get("result").isArray())
  {
    if (slice.get("result").length() < 1)
    {
      LOG_DEBUG << "Error: Result is null" << slice.toJson() << " | executed query: " << schemaQuery << " | database is: " << database;
      return {ok::ErrorCode::BAD_QUERY, std::move(resp)};
    }
    else if (slice.get("result")[0].isNull())
    {
      LOG_DEBUG << "Error: Result is null" << slice.toJson() << " | executed query: " << schemaQuery << " | database is: " << database;
      return {ok::ErrorCode::BAD_QUERY, std::move(resp)};
    }
    return {ok::ErrorCode::ERROR_NO_ERROR, std::move(resp)};
    // return slice.get("result");
  }
  else
  {
    LOG_DEBUG << "Error: Result has no Result Key" << slice.toJson() << " | executed query: " << schemaQuery << " | database is: " << database;
    return {ok::ErrorCode::ERROR_NO_ERROR, std::move(resp)};
  }
}
std::string fileTypeToString(drogon::FileType const &fileType)
{
  switch (fileType)
  {
    case drogon::FT_UNKNOWN: return "unknown";
    case drogon::FT_CUSTOM: return "custom";
    case drogon::FT_DOCUMENT: return "document";
    case drogon::FT_ARCHIVE: return "archive";
    case drogon::FT_AUDIO: return "audio";
    case drogon::FT_MEDIA: return "media";
    case drogon::FT_IMAGE: return "image";
  }
}
std::tuple<ok::ErrorCode, DocumentKey> saveFileToDatabase(drogon::HttpFile const &file, std::string const &fileName, ok::smart_actor::connection::Session &session) noexcept
{
  constexpr auto query = "insert @data in @@collection return NEW._key";
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("@collection", arangodb::velocypack::Value("upload"));
  bindVars.add("data", arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("name", arangodb::velocypack::Value(fileName));
  bindVars.add("type", arangodb::velocypack::Value(fileTypeToString(file.getFileType())));
  bindVars.add("md5", arangodb::velocypack::Value(file.getMd5()));
  bindVars.close();
  bindVars.close();
  auto [erDb, response] = Api::Cursor::PostCursor::request(session.database, query, bindVars.slice());
  if (isEr(erDb))
  {
    LOG_DEBUG << "error saving upload to collection. query: " << query << " bindVars: " << bindVars.toString();
    return {ok::ErrorCode::BAD_QUERY, ""};
  }
  auto slice = response->slices().front();
  auto result = slice.get("result");
  if (result.length() == 0)
  {
    LOG_DEBUG << "error saving upload to collection. query: " << query << " result: " << slice.toString();
    return {ok::ErrorCode::BAD_QUERY, ""};
  }
  return {ok::ErrorCode::ERROR_NO_ERROR, result[0].copyString()};
}
namespace seed
{
void newUserDatabase()
{
  auto [erDb, response] = Api::Database::PostDatabase::request("user");
  if (isEr(erDb))
  {
    return;  // already seeded
  }
  createUserCollectionsAndEdges();
  createIndex();
  createSuperUser();
}
void createUserCollectionsAndEdges() {}
void createGlobalCollectionsAndEdges()
{
  // create collections
  {
    auto create = [&](std::string const &name, Api::Collection::PostCollection::type type) { auto [erDb, response] = Api::Collection::PostCollection::request("global", name, type); };
    if (auto [er, myResp] = getSchema("global", "schema", "seed", ".form"); ok::isEr(er)) { LOG_DEBUG << "no collections/edge"; }
    else
    {
      auto slice = myResp->slices().front().get("result");
      if (slice.length() == 1)
      {
        auto object = slice[0];
        auto collections = object.get("global_collections");
        for (auto const &it : arangodb::velocypack::ArrayIterator(collections)) { create(it.copyString(), Api::Collection::PostCollection::document_collection); }
        auto edgesArray = object.get("global_edges");
        for (auto const &it : arangodb::velocypack::ArrayIterator(edgesArray)) { create(it.copyString(), Api::Collection::PostCollection::edge_collection); }
      }
    }
  }
}
void createIndex() { auto [erDb, response] = Api::Index::PostIndexPersistent::request("user", "user", {"email"}); }
void createSuperUser()
{
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("_key", arangodb::velocypack::Value("1"));
  bindVars.add("no", arangodb::velocypack::Value(1));
  bindVars.add("email", arangodb::velocypack::Value("sadmin"));
  bindVars.add("pass", arangodb::velocypack::Value("123"));
  bindVars.add("type", arangodb::velocypack::Value("super_admin"));
  auto createdAt = utils::time::getEpochMilliseconds();
  bindVars.add("createdAt", arangodb::velocypack::Value(createdAt));
  bindVars.close();
  auto [erDb, response] = Api::Document::PostCollection::request("user", "user", bindVars.slice());
}
}  // namespace seed
namespace chat
{
std::pair<ok::ErrorCode, jsoncons::ojson> chat_statistics(Database const &database)
{
  auto query = R"aql(let conversations_count = FIRST(FOR u IN conversation COLLECT WITH COUNT INTO length RETURN length)
let has_conversations_count = FIRST(FOR u IN has_conversation COLLECT WITH COUNT INTO length RETURN length)
let participants_count = FIRST(FOR u IN participant COLLECT WITH COUNT INTO length RETURN length)
let has_participants_count = FIRST(FOR u IN has_participant COLLECT WITH COUNT INTO length RETURN length)
let messages_count = FIRST(FOR u IN message COLLECT WITH COUNT INTO length RETURN length)
let has_messages_count = FIRST(FOR u IN has_message COLLECT WITH COUNT INTO length RETURN length)
let real = FIRST(FOR u IN conversation
        COLLECT AGGREGATE
            real_messages_count = SUM(FIRST(FOR v IN 1..1 OUTBOUND u has_message COLLECT WITH COUNT INTO real_messages_count RETURN real_messages_count)),
            real_participants_count = SUM(FIRST(FOR v IN 1..1 OUTBOUND u has_participant COLLECT WITH COUNT INTO real_participants_count return real_participants_count))
    RETURN {real_messages_count, real_participants_count})

let t1 = conversations_count == has_conversations_count
let t2 = participants_count == has_participants_count
let t3 = messages_count == has_messages_count
let t4 = messages_count == real.real_messages_count
let t5 = participants_count == real.real_participants_count

return {conversations_count, has_conversations_count, participants_count, has_participants_count, messages_count, has_messages_count, t1, t2, t3, t4, t5}
)aql";
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.close();
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  if (isEr(erDb))
  {
    if (response)
    {
      auto slice = response->slices().front();
      LOG_FATAL << "Error: " << slice.toJson();
    }
    LOG_FATAL << "Error: " << errno_string(erDb) << " | executed query: " << query << " | database: " << database << "| bindVars: " << bindVars.toString();
    return {ok::ErrorCode::BAD_QUERY, {}};
  }
  auto slice = response->slices().front();
  return {ok::ErrorCode::ERROR_NO_ERROR, jsoncons::ojson::parse(slice.toJson())};
}
std::pair<ok::ErrorCode, std::string> getConversationState(Database const &database, DocumentKey const &conversationKey)
{
  auto query = R"aql(FOR t in conversation
  FILTER t._key == @conversationKey
  RETURN t.state
  )aql";
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("conversationKey", arangodb::velocypack::Value(conversationKey));
  bindVars.close();
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  if (isEr(erDb))
  {
    if (response)
    {
      auto slice = response->slices().front();
      LOG_FATAL << "Error: " << slice.toJson();
    }
    LOG_FATAL << "Error: " << errno_string(erDb) << " | executed query: " << query << " | database: " << database << "| bindVars: " << bindVars.toString();
    return {ok::ErrorCode::BAD_QUERY, ""};
  }
  auto slice = response->slices().front();
  if (auto result = slice.get("result"); result.length() == 1)
  {
    auto state = result[0].copyString();
    return {ok::ErrorCode::ERROR_NO_ERROR, state};
  }
  return {ok::ErrorCode::ERROR_HTTP_NOT_FOUND, ""};
}
ok::ErrorCode deteteChatData(Database const &database, DocumentKey const &conversationKey)
{
  auto query = R"aql(let messags_deleted = (for u in has_message
  FILTER u._from == CONCAT("conversation/", @conversationKey)
  REMOVE {_key: SPLIT(u._to, "/")[1] } in message
  REMOVE u in has_message
  return u)

let participants_deleted = (for u in has_participant
  FILTER u._from == concat("conversation/", @conversationKey)
  REMOVE {_key: SPLIT(u._to, "/")[1] } in participant
  REMOVE u in has_participant
  return u)

RETURN {messags_deleted, participants_deleted}
  )aql";
  arangodb::velocypack::Builder bindVars;
  bindVars.add(arangodb::velocypack::Value(arangodb::velocypack::ValueType::Object));
  bindVars.add("conversationKey", arangodb::velocypack::Value(conversationKey));
  bindVars.close();
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, query, bindVars.slice());
  if (isEr(erDb))
  {
    if (response)
    {
      auto slice = response->slices().front();
      LOG_FATAL << "Error: " << slice.toJson();
    }
    LOG_FATAL << "Error: " << errno_string(erDb) << " | executed query: " << query << " | database: " << database << "| bindVars: " << bindVars.toString();
    return ok::ErrorCode::BAD_QUERY;
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
}  // namespace chat
}  // namespace db
}  // namespace ok
