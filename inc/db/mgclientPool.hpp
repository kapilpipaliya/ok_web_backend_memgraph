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
// currently only supports strings
struct MGParams
{
    mg_map *extra;
    MGParams(std::initializer_list<std::pair<const char *, mg_value *>> params)
    {
        extra = mg_map_make_empty(params.size());
        if (!extra)
        {
           // return nullptr;
        }
        for (auto const &p : params)
            mg_map_insert_unsafe(extra, p.first, p.second);
    }
    auto asConstMap()
    {
        return mg::ConstMap{extra};
    }
    ~MGParams()
    {
        mg_map_destroy(extra);
    }
};
class MemGraphClientPool
{
public:
  MemGraphClientPool();
  ~MemGraphClientPool() noexcept;
  void initialize(size_t const connNum) noexcept;
  size_t readyConnsSize();
  using DbConnectionPtr = std::shared_ptr<mg::Client>;
  std::vector<std::vector<mg::Value> > request(std::string const &body);
  std::vector<std::vector<mg::Value> > request(std::string const &body,  mg::ConstMap const &params);
  int getIdFromResponse(std::vector<std::vector<mg::Value> > const &response);
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
