#include "db/fuerteclient.hpp"
#include <fuerte/requests.h>
// #include <Basics/VelocyPackHelper.h>
#include <velocypack/Buffer.h>
#include <velocypack/Builder.h>
#include <velocypack/Options.h>
#include <velocypack/Parser.h>
#include <velocypack/Slice.h>
#include <velocypack/StringRef.h>
#include <velocypack/ValueType.h>
#include <cassert>
#include <iostream>
#include <tuple>
namespace VelocyPackHelper{
/// @brief returns a string value, or the default value if it is not a string
std::string getStringValue(VPackSlice slice,
                                             std::string const& defaultValue) {
  if (!slice.isString()) {
    return defaultValue;
  }
  return slice.copyString();
}

  /// @brief returns a string sub-element, or the default value if it does not
  /// exist or it is not a string
  template<typename T>
  static std::string getStringValue(VPackSlice slice, T const& name,
                                    std::string const& defaultValue) {
    if (slice.isExternal()) {
      slice = VPackSlice(reinterpret_cast<uint8_t const*>(slice.getExternal()));
    }

    if (slice.isObject()) {
      VPackSlice sub = slice.get(name);
      if (sub.isString()) {
        return sub.copyString();
      }
    }
    return defaultValue;
  }
}
FuerteClient::FuerteClient(std::string endpoint, std::string username, std::string password, std::string databaseName)
    : _endpoint(std::move(endpoint)),
      _username(std::move(username)),
      _password(std::move(password)),
      _databaseName(std::move(databaseName)),
      _requestTimeout(0),  // read message.h file, get timeout, 0 means no timeout
      _lastHttpReturnCode(0),
      _lastErrorMessage(""),
      _version("arango"),
      _mode("unknown mode"),
      _role("UNKNOWN"),
      _loop(1, "OKFuerteClientConnection"),
      _vpackOptions(arangodb::velocypack::Options::Defaults),
      _forceJson(false),
      _setCustomError(false)
{
  _vpackOptions.buildUnindexedObjects = true;
  _vpackOptions.buildUnindexedArrays = true;
  _builder.maxConnectRetries(3);
  _builder.connectRetryPause(std::chrono::milliseconds(100));
  _builder.connectTimeout(std::chrono::milliseconds(static_cast<int64_t>(DEFAULT_CONNECTION_TIMEOUT)));
  _builder.onFailure([this](arangodb::fuerte::Error err, std::string const &msg) {
    // care only about connection errors
    if (err == arangodb::fuerte::Error::CouldNotConnect || err == arangodb::fuerte::Error::VstUnauthorized || err == arangodb::fuerte::Error::ProtocolError)
    {
      std::unique_lock<std::recursive_mutex> guard(_lock, std::try_to_lock);
      if (guard && !_setCustomError)
      {
        _lastHttpReturnCode = 503;
        _lastErrorMessage = msg;
      }
      _setCustomError = false;
    }
  });
}
FuerteClient::~FuerteClient()
{
  _builder.onFailure(nullptr);  // reset callback
  shutdownConnection();
  _loop.stop();
}
std::shared_ptr<arangodb::fuerte::Connection> FuerteClient::createConnection()
{
  auto newConnection = _builder.connect(_loop);
  arangodb::fuerte::StringMap params{{"details", "true"}};
  auto req = arangodb::fuerte::createRequest(arangodb::fuerte::RestVerb::Get, "/_api/version", params);
  req->header.database = _databaseName;
  req->timeout(std::chrono::seconds(30));
  try
  {
    auto res = newConnection->sendRequest(std::move(req));
    _lastHttpReturnCode = res->statusCode();
    std::shared_ptr<VPackBuilder> parsedBody;
    VPackSlice body;
    if (res->contentType() == arangodb::fuerte::ContentType::VPack) { body = res->slice(); }
    else if (res->contentType() == arangodb::fuerte::ContentType::Json)
    {
      parsedBody = VPackParser::fromJson(reinterpret_cast<char const *>(res->payload().data()), res->payload().size());
      body = parsedBody->slice();
    }
    if (_lastHttpReturnCode >= 400)
    {
      auto const &headers = res->messageHeader().meta();
      auto it = headers.find("http/1.1");
      if (it != std::end(headers))
      {
        std::string errorMessage = (*it).second;
        if (body.isObject())
        {
          std::string const msg = VelocyPackHelper::getStringValue(body, "errorMessage", "");
          if (!msg.empty()) { errorMessage = msg; }
        }
        setCustomError(_lastHttpReturnCode, errorMessage);
        return nullptr;
      }
    }
    if (!body.isObject())
    {
      std::string msg("invalid response: '");
      msg += std::string(reinterpret_cast<char const *>(res->payload().data()), res->payload().size());
      msg += "'";
      setCustomError(503, msg);
      return nullptr;
    }
    std::lock_guard<std::recursive_mutex> guard(_lock);
    _connection = newConnection;
    // not necessary:
    /*std::string const server = arangodb::basics::VelocyPackHelper::getStringValue(body, "server", "");

    // "server" value is a string and content is "arango"
    if (server == "arango")
    {
      // look up "version" value
      _version = arangodb::basics::VelocyPackHelper::getStringValue(body, "version", "");
      VPackSlice const details = body.get("details");
      if (details.isObject())
      {
        VPackSlice const mode = details.get("mode");
        if (mode.isString())
        {
          _mode = mode.copyString();
        }
        VPackSlice role = details.get("role");
        if (role.isString())
        {
          _role = role.copyString();
        }
      }
      if (!body.hasKey("version"))
      {
        // if we don't get a version number in return, the server is
        // probably running in hardened mode
        return newConnection;
      }

      std::string const versionString =
          arangodb::basics::VelocyPackHelper::getStringValue(body, "version", "");
      std::pair<int, int> version = rest::Version::parseVersionString(versionString);
      if (version.first < 3) {
        // major version of server is too low
        //_client->disconnect();
        shutdownConnection();
        std::string msg("Server version number ('" + versionString +
                        "') is too low. Expecting 3.0 or higher");;
        setCustomError(500, msg);
        return newConnection;
      }
    }*/
    return _connection;
  }
  catch (arangodb::fuerte::Error const &e)
  {  // connection error
    std::string msg(arangodb::fuerte::to_string(e));
    setCustomError(503, msg);
    return nullptr;
  }
}
std::shared_ptr<arangodb::fuerte::Connection> FuerteClient::acquireConnection()
{
  std::lock_guard<std::recursive_mutex> guard(_lock);

  _lastErrorMessage = "";
  _lastHttpReturnCode = 0;

  if (!_connection ||
      (_connection->state() == arangodb::fuerte::Connection::State::Closed)) {
    return createConnection();
  }
  return _connection;
}
void FuerteClient::setInterrupted(bool interrupted)
{
  std::lock_guard<std::recursive_mutex> guard(_lock);
  if (interrupted && _connection != nullptr) {
    shutdownConnection();
  } else if (!interrupted && (_connection == nullptr ||
                            (_connection->state() == arangodb::fuerte::Connection::State::Closed))) {
    createConnection();
  }
}
bool FuerteClient::isConnected() const
{
  std::lock_guard<std::recursive_mutex> guard(_lock);
  if (_connection)
  {
    if (_connection->state() == arangodb::fuerte::Connection::State::Connected) { return true; }
    // the client might have automatically closed the connection,
    // as long as there was no error we can reconnect
    return _lastHttpReturnCode < 400;
  }
  return false;
}
void FuerteClient::connect()
{
  std::lock_guard<std::recursive_mutex> guard(_lock);
  /*
  TRI_ASSERT(client);
  std::lock_guard<std::recursive_mutex> guard(_lock);
  _forceJson = client->forceJson();

  _requestTimeout = std::chrono::duration<double>(client->requestTimeout());
  _databaseName = client->databaseName();
  _builder.endpoint(client->endpoint());
  // check jwtSecret first, as it is empty by default,
  // but username defaults to "root" in most configurations
  if (!client->jwtSecret().empty()) {
      _builder.jwtToken(
          fuerte::jwt::generateInternalToken(client->jwtSecret(), "arangosh"));
      _builder.authenticationType(fuerte::AuthenticationType::Jwt);
  } else if (!client->username().empty()) {
      _builder.user(client->username()).password(client->password());
      _builder.authenticationType(fuerte::AuthenticationType::Basic);
  }
  */
  _forceJson = false;
  _requestTimeout = std::chrono::duration<double>(0);
  _builder.endpoint(_endpoint);
  _builder.user(_username).password(_password);
  _builder.authenticationType(arangodb::fuerte::AuthenticationType::Basic);
  createConnection();
}
void FuerteClient::reconnect()
{
  std::lock_guard<std::recursive_mutex> guard(_lock);
  _requestTimeout = std::chrono::duration<double>(0);
  //_databaseName = _databaseName;
  _forceJson = false;
  _builder.endpoint(_endpoint);
  _builder.user(_username).password(_password);
  _builder.authenticationType(arangodb::fuerte::AuthenticationType::Basic);
  std::shared_ptr<arangodb::fuerte::Connection> oldConnection;
  _connection.swap(oldConnection);
  if (oldConnection) { oldConnection->cancel(); }
  oldConnection.reset();
  try
  {
    createConnection();
  }
  catch (...)
  {
    std::string errorMessage = "error in '" + _endpoint + "'";
    throw errorMessage;
  }
  if (isConnected() && _lastHttpReturnCode == 200)
  {  // OK
  }
  else
  {
    std::cout << "Could not connect to endpoint '" << _endpoint << "', username: '" << _username << "' - Server message: " << _lastErrorMessage;
    std::string errorMsg = "could not connect";
    if (!_lastErrorMessage.empty()) { errorMsg = _lastErrorMessage; }
    throw errorMsg;
  }
}
std::string FuerteClient::endpointSpecification() const
{
  std::lock_guard<std::recursive_mutex> guard(_lock);
  if (_connection) { return _connection->endpoint(); }
  return "";
}
////////////////////////////////////////////////////////////////////////////////
/// @brief map of connections
////////////////////////////////////////////////////////////////////////////////
static std::unordered_map<void *, FuerteClient> Connections;
////////////////////////////////////////////////////////////////////////////////
/// @brief weak reference callback for connections (call the destructor here)
////////////////////////////////////////////////////////////////////////////////
static void DestroyV8ClientConnection(FuerteClient *v8connection)
{
  assert(v8connection != nullptr);
  auto it = Connections.find(v8connection);
  if (it != std::end(Connections))
  {
    //(*it).second.Reset(); // ?
    Connections.erase(it);
  }
  delete v8connection;
}
// clang-format off
std::unique_ptr<arangodb::fuerte::Response> FuerteClient::getData( const std::string &location, const std::unordered_map<std::string, std::string> &headerFields) {
  return requestDataRaw(arangodb::fuerte::RestVerb::Get, location, {}, headerFields);
}
std::unique_ptr<arangodb::fuerte::Response> FuerteClient::headData( const std::string &location, const std::unordered_map<std::string, std::string> &headerFields) {
  return requestDataRaw(arangodb::fuerte::RestVerb::Head, location, {}, headerFields);
}
std::unique_ptr<arangodb::fuerte::Response> FuerteClient::deleteData( const std::string &location, const arangodb::velocypack::Slice &body, const std::unordered_map<std::string, std::string> &headerFields) {
  return requestDataRaw(arangodb::fuerte::RestVerb::Delete, location, body, headerFields);
}
std::unique_ptr<arangodb::fuerte::Response> FuerteClient::optionsData( const std::string &location, const arangodb::velocypack::Slice &body, const std::unordered_map<std::string, std::string> &headerFields) {
  return requestDataRaw(arangodb::fuerte::RestVerb::Options, location, body, headerFields);
}
std::unique_ptr<arangodb::fuerte::Response> FuerteClient::postData( const std::string &location, const arangodb::velocypack::Slice &body, const std::unordered_map<std::string, std::string> &headerFields) {
  return requestDataRaw(arangodb::fuerte::RestVerb::Post, location, body, headerFields);
}
std::unique_ptr<arangodb::fuerte::Response> FuerteClient::putData( const std::string &location, const arangodb::velocypack::Slice &body, const std::unordered_map<std::string, std::string> &headerFields) {
  return requestDataRaw(arangodb::fuerte::RestVerb::Put, location, body, headerFields);
}
std::unique_ptr<arangodb::fuerte::Response> FuerteClient::patchData( const std::string &location, const arangodb::velocypack::Slice &body, const std::unordered_map<std::string, std::string> &headerFields) {
    return requestDataRaw(arangodb::fuerte::RestVerb::Patch, location, body, headerFields);
}
// clang-format on
std::unique_ptr<arangodb::fuerte::Response> FuerteClient::requestDataRaw(arangodb::fuerte::RestVerb verb,
                                                                         const std::string &location,
                                                                         const arangodb::velocypack::Slice &body,
                                                                         const std::unordered_map<std::string, std::string> &headerFields)
{
  bool retry = true;
again:
  // short method:
  //  auto request = arangodb::fuerte::createRequest(
  //      arangodb::fuerte::RestVerb::Post, "/_api/cursor");
  auto req = std::make_unique<arangodb::fuerte::Request>();
  req->header.restVerb = verb;
  req->header.database = _databaseName;
  req->header.parseArangoPath(location);
  // can also do:
  // req->header.parameters = headerFields;
  for (auto &pair : headerFields) { req->header.addMeta(pair.first, pair.second); }
  req->header.contentType(arangodb::fuerte::ContentType::VPack);
  req->addVPack(std::move(body));
  if (req->header.acceptType() == arangodb::fuerte::ContentType::Unset) { req->header.acceptType(arangodb::fuerte::ContentType::VPack); }
  req->timeout(std::chrono::duration_cast<std::chrono::milliseconds>(_requestTimeout));
  std::shared_ptr<arangodb::fuerte::Connection> connection = acquireConnection();
  if (!connection || connection->state() == arangodb::fuerte::Connection::State::Closed) {
    //TRI_V8_SET_EXCEPTION_MESSAGE(TRI_ERROR_SIMPLE_CLIENT_COULD_NOT_CONNECT, "not connected");
    //return v8::Undefined(isolate);
    throw std::runtime_error("not connected");
  }
  arangodb::fuerte::Error rc = arangodb::fuerte::Error::NoError;
  std::unique_ptr<arangodb::fuerte::Response> response;
  try
  {
    response = connection->sendRequest(std::move(req));
  }
  catch (arangodb::fuerte::Error const &e)
  {
    rc = e;
    _lastErrorMessage.assign(arangodb::fuerte::to_string(e));
    _lastHttpReturnCode = 503;
  }
  if (rc == arangodb::fuerte::Error::ConnectionClosed && retry)
  {
    retry = false;
    goto again;
  }
  // complete
  _lastHttpReturnCode = response->statusCode();  // crash when rc= arangodb::fuerte::Error::Canceled error happened.
  if (_lastHttpReturnCode >= 400) {}
  else
  {
  }
  // and returns
  return response;
}
void FuerteClient::shutdownConnection()
{
  std::lock_guard<std::recursive_mutex> guard(_lock);
  if (_connection) { _connection->cancel(); }
}
