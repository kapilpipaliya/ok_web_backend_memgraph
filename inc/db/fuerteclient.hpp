#pragma once
#include <velocypack/StringRef.h>
#include <fuerte/connection.h>
#include <fuerte/loop.h>
#include <fuerte/types.h>
// ClientFeature.h
constexpr static double const DEFAULT_REQUEST_TIMEOUT = 1200.0;
constexpr static double const DEFAULT_CONNECTION_TIMEOUT = 5.0;
class FuerteClient
{
  FuerteClient(FuerteClient const&) = delete;
  FuerteClient& operator=(FuerteClient const&) = delete;
public:
  explicit FuerteClient(std::string endpoint, std::string username, std::string password, std::string databaseName);
  ~FuerteClient();
public:
  void setInterrupted(bool interrupted);
  bool isConnected() const;
  // clang-format off
  void connect();
  void reconnect();

  double timeout() const { return _requestTimeout.count(); }

  void timeout(double value) {
    _requestTimeout = std::chrono::duration<double>(value);
  }

  std::string const& databaseName() const { return _databaseName; }
  void setDatabaseName(std::string const& value) { _databaseName = value; }
  void setForceJson(bool value) { _forceJson = value; };
  std::string username() const { return _builder.user(); }
  std::string password() const { return _builder.password(); }
  int lastHttpReturnCode() const { return _lastHttpReturnCode; }
  std::string lastErrorMessage() const { return _lastErrorMessage; }
  std::string const& version() const { return _version; }
  std::string const& mode() const { return _mode; }
  std::string const& role() const { return _role; }
  std::string endpointSpecification() const;

  // application_features::ApplicationServer& server();

  std::unique_ptr<arangodb::fuerte::v1::Response> getData( const std::string& location, std::unordered_map<std::string, std::string> const& headerFields);
  std::unique_ptr<arangodb::fuerte::v1::Response> headData( const std::string& location, std::unordered_map<std::string, std::string> const& headerFields);
  std::unique_ptr<arangodb::fuerte::v1::Response> deleteData( const std::string& location, const arangodb::velocypack::Slice& body, std::unordered_map<std::string, std::string> const& headerFields);
  std::unique_ptr<arangodb::fuerte::v1::Response> optionsData( const std::string& location, const arangodb::velocypack::Slice& body, std::unordered_map<std::string, std::string> const& headerFields);
  std::unique_ptr<arangodb::fuerte::v1::Response> postData( const std::string& location, const arangodb::velocypack::Slice& body, std::unordered_map<std::string, std::string> const& headerFields);
  std::unique_ptr<arangodb::fuerte::v1::Response> putData( const std::string& location, const arangodb::velocypack::Slice& body, std::unordered_map<std::string, std::string> const& headerFields);
  std::unique_ptr<arangodb::fuerte::v1::Response> patchData( const std::string& location, const arangodb::velocypack::Slice& body, std::unordered_map<std::string, std::string> const& headerFields);
  void initServer();

  std::unique_ptr<arangodb::fuerte::v1::Response> requestDataRaw( arangodb::fuerte::RestVerb verb, const std::string& location, const arangodb::velocypack::Slice& body, std::unordered_map<std::string, std::string> const& headerFields);
 private:
  std::shared_ptr<arangodb::fuerte::v1::Connection> createConnection();
  std::shared_ptr<arangodb::fuerte::v1::Connection> acquireConnection();
  // clang-format on
  //    v8::Local<v8::Value> handleResult(
  //                          std::unique_ptr<arangodb::fuerte::Response>
  //                          response, arangodb::fuerte::Error ec);
  /// @brief shuts down the connection _connection and resets the pointer
  /// to a nullptr
  void shutdownConnection();
  void setCustomError(int httpCode, std::string const& msg)
  {
    _setCustomError = true;
    _lastHttpReturnCode = httpCode;
    _lastErrorMessage = msg;
  }
private:
  // application_features::ApplicationServer& _server;
  std::string _databaseName;
  std::chrono::duration<double> _requestTimeout;
  mutable std::recursive_mutex _lock;
  int _lastHttpReturnCode;
  std::string _lastErrorMessage;
  std::string _version;
  std::string _mode;
  std::string _role;
  arangodb::fuerte::v1::EventLoopService _loop;
  arangodb::fuerte::v1::ConnectionBuilder _builder;
  std::shared_ptr<arangodb::fuerte::Connection> _connection;
  arangodb::velocypack::Options _vpackOptions;
  bool _forceJson;
  bool _setCustomError;
  std::string _endpoint;
  std::string _username;
  std::string _password;
};
