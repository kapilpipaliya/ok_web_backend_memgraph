#pragma once
#include <trantor/net/EventLoopThreadPool.h>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>
#include "mgclient.hpp"
namespace ok::db
{
class MemGraphClientPool
{
public:
  MemGraphClientPool();
  ~MemGraphClientPool() noexcept;
  void initialize(size_t const connNum) noexcept;
  size_t readyConnsSize();
  using DbConnectionPtr = std::shared_ptr<mg::Client>;
  std::vector<std::vector<mg::Value> > requestDataRaw(std::string const &database, std::string const &body); // mg::ConstMap const &params

private:
  std::string connectionInfo_;
  size_t connectionsNumber_;
  std::mutex connectionsMutex_;
  std::unordered_set<DbConnectionPtr> connections_;
  std::unordered_set<DbConnectionPtr> readyConnections_;
  std::unordered_set<DbConnectionPtr> busyConnections_;
  DbConnectionPtr newConnection();
};
inline MemGraphClientPool memgraph_conns{};
void initializeMemGraphPool(int count);
}  // namespace ok::db
