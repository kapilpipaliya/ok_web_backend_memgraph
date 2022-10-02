#pragma once
#include <trantor/net/EventLoopThreadPool.h>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>
#include "fuerteclient.hpp"
namespace ok::db
{
class FuerteClientPool
{
public:
  FuerteClientPool();
  ~FuerteClientPool() noexcept;
  void initialize(size_t const connNum) noexcept;
  size_t readyConnsSize();
  using DbConnectionPtr = std::shared_ptr<FuerteClient>;
  std::unique_ptr<arangodb::fuerte::v1::Response> requestDataRaw(std::string const &database,
                                                                 arangodb::fuerte::RestVerb verb,
                                                                 std::string const &location,
                                                                 arangodb::velocypack::Slice const &body,
                                                                 std::unordered_map<std::string, std::string> const &headerFields);

private:
  std::string connectionInfo_;
  size_t connectionsNumber_;
  std::mutex connectionsMutex_;
  std::unordered_set<DbConnectionPtr> connections_;
  std::unordered_set<DbConnectionPtr> readyConnections_;
  std::unordered_set<DbConnectionPtr> busyConnections_;
  DbConnectionPtr newConnection();
};
inline FuerteClientPool fuerte_conns{};
void initializeFuertePool(int count);
}  // namespace ok::db
