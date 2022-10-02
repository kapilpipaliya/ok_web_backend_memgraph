#include "db/fuerteclientPool.hpp"
#include <trantor/utils/Logger.h>
#include <thread>
namespace ok::db
{
FuerteClientPool::FuerteClientPool() { LOG_DEBUG << "initializing fuerte connections"; }
void FuerteClientPool::initialize(size_t const connNum) noexcept
{
  LOG_DEBUG << "initializing " << connNum << " connections";
  connectionsNumber_ = connNum;
  assert(connNum > 0);
  {
    for (size_t i = 0; i < connectionsNumber_; ++i)
    {
      // can do on separate threads
      connections_.insert(newConnection());
    }
  }
}
size_t FuerteClientPool::readyConnsSize() { return readyConnections_.size(); }
FuerteClientPool::~FuerteClientPool() noexcept
{
  std::lock_guard<std::mutex> lock(connectionsMutex_);
  for (auto const &conn : connections_) { conn->setInterrupted(true); }
  connections_.clear();
  readyConnections_.clear();
  busyConnections_.clear();
}
std::unique_ptr<arangodb::fuerte::Response> FuerteClientPool::requestDataRaw(std::string const &database,
                                                                             arangodb::fuerte::RestVerb verb,
                                                                             std::string const &location,
                                                                             arangodb::velocypack::Slice const &body,
                                                                             std::unordered_map<std::string, std::string> const &headerFields)
{
  DbConnectionPtr conn;
  {
    std::lock_guard<std::mutex> guard(connectionsMutex_);
    if (readyConnections_.empty())
    {
      if (busyConnections_.empty())
      {
        // throw std::runtime_error("No connection to database server");
        LOG_FATAL << "No connection to database server";
        exit(1);
        // return;
      }
      // LOG_TRACE << "Push query to buffer";
      // I think this should not happen.
      LOG_FATAL << "no ready connections. Please Increase ready connections pool";
    }
    else
    {
      // move ready connection -> busy connection
      auto iter = std::begin(readyConnections_);
      busyConnections_.insert(*iter);
      conn = *iter;
      readyConnections_.erase(iter);
    }
  }
  if (conn)
  {
    conn->setDatabaseName(database);
    auto resp = conn->requestDataRaw(verb, location, body, headerFields);
    std::lock_guard<std::mutex> guard(connectionsMutex_);
    busyConnections_.erase(conn);
    readyConnections_.insert(conn);
    return resp;
  }
  LOG_FATAL << "This Should Not happened.";  // Todo fix this
  return nullptr;
}
FuerteClientPool::DbConnectionPtr FuerteClientPool::newConnection()
{
  DbConnectionPtr connPtr;
  constexpr auto domain = "vst://localhost:8529";
  constexpr auto userName = "root";
  constexpr auto password = "";
  connPtr = std::make_shared<FuerteClient>(domain, userName, password, "");
  connPtr->connect();
  if (connPtr->isConnected())
  {
    LOG_TRACE << "connected!";
    std::lock_guard<std::mutex> guard(connectionsMutex_);
    readyConnections_.insert(connPtr);
  }
  return connPtr;
  // it set callback for close, ok and idle.
  // on close : erase from 3 maps. and create new connection
  // on ok : busyConnections insert
  // on idle : busyConnections remove, readyConnections insert
}
void initializeFuertePool(int count)
{
  ok::db::fuerte_conns.initialize(count);
  if (ok::db::fuerte_conns.readyConnsSize() <= 0)
  {
    LOG_DEBUG << "cant connect to database. exiting application";
    exit(0);
  }
}
}  // namespace ok::db
