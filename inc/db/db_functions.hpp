#pragma once
#include "alias.hpp"
#include "Session.hpp"
#include <fuerte/connection.h>  // can't forward declare inline namespace
namespace drogon
{
class HttpFile;
}
namespace ok
{
enum class ErrorCode;
namespace db
{
void waitForFuerte();
bool isDatabaseServerConnected();
ErrorCode copyGlobalCollection(Database const &database, Collection const &collection);
ErrorCode copyGlobalCollections(Database const &database);
std::tuple<ErrorCode, Database> saveUser(std::string const &email, std::string const &password, std::string const &url);
ErrorCode isDatabaseExist(Database const &dbName);
ErrorCode createDatabase(Database const &database);
ErrorCode deleteDatabase(Database const &database);
ErrorCode createCollections(Database const &database);
ErrorCode createPermissions(Database const &database);
std::tuple<ErrorCode, DocumentKey> createMember(Database const &database, std::string const &firstName, std::string const &lastName, std::string const &email, std::string const &password, std::string const &key = "", bool isAdmin = false, bool lock = false);
std::tuple<ErrorCode, DocumentKey, Email, bool> findMember(Database const &database, std::string const &email, std::string const &password);
std::tuple<ErrorCode, DocumentKey, Email, bool> findMember(Database const &database, DocumentKey const &memberKey);
ErrorCode createMemberPermissions(Database const &database, DocumentKey const &memberKey);
ErrorCode setMemberEmailConfirmed(Database const &database, DocumentKey const &memberKey);
std::tuple<ok::ErrorCode, Database, DocumentKey> confirmMember(DocumentKey const &confirmKey, std::string const &token);
std::tuple<ErrorCode, Database> findDatabaseNameFromSubdomain(std::string const &subDomain);
void authenticateAndSaveSession(std::string const &jwtEncoded, ok::smart_actor::connection::Session &session, std::string const &subDomain);
std::tuple<ErrorCode, Database> findDatabaseFromUserEmail(std::string const &email);
ErrorCode deleteConfirmDocument(DocumentKey const &confirmKey);
bool cleanDBSession(DocumentKey const &sessionKey);
ErrorCode createLoginDbSession(ok::smart_actor::connection::Session &session);
std::tuple<ErrorCode, std::string> generateConfirmUrl(Database const &database, DocumentKey const &memberKey);
// private
ErrorCode uniqueMemberFieldCheck(Database const &database, Collection const &collection, const std::string &fieldName, const std::string &fieldValue);
void cleanExpiredCookies();
unsigned long expandCookieTime(DocumentKey const &sessionKey);
std::tuple<ErrorCode, std::unique_ptr<arangodb::fuerte::v1::Response>> getDocumentWithProjection(Database const &database,
                                                                                                 Collection const &collection,
                                                                                                 std::string const &key,
                                                                                                 std::function<std::string(std::string const &)> const projection) noexcept;
std::tuple<ErrorCode, std::unique_ptr<arangodb::fuerte::v1::Response>> getDocumentWithProjection(Database const &database,
                                                                                                 Collection const &collection,
                                                                                                 std::string const &key,
                                                                                                 std::string const &projection) noexcept;
std::tuple<ErrorCode, std::unique_ptr<arangodb::fuerte::v1::Response>> getSchema(Database const &database,
                                                                                 Collection const &collection,
                                                                                 std::string const &key,
                                                                                 std::string const &projection) noexcept;
std::tuple<ok::ErrorCode, DocumentKey> saveFileToDatabase(drogon::HttpFile const &file, std::string const &fileName, ok::smart_actor::connection::Session &session) noexcept;
namespace seed
{
void newUserDatabase();
void createUserCollectionsAndEdges();
void createGlobalCollectionsAndEdges();
void createIndex();
void createSuperUser();
}  // namespace seed
namespace chat
{
std::pair<ErrorCode, jsoncons::ojson> chat_statistics(Database const &database);
std::pair<ok::ErrorCode, std::string> getConversationState(Database const &database, DocumentKey const &conversationKey);
ok::ErrorCode deteteChatData(Database const &database, DocumentKey const &conversationKey);
}  // namespace chat
};  // namespace db
}  // namespace ok
