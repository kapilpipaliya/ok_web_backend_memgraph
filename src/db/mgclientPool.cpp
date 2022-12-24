#include "db/mgclientPool.hpp"
#include <trantor/utils/Logger.h>
#include <thread>
namespace ok::db
{
MemGraphClientPool::MemGraphClientPool()
{
    LOG_DEBUG << "initializing mgclient connections";
}
void MemGraphClientPool::initialize(size_t const connNum) noexcept
{
    LOG_DEBUG << "initializing " << connNum << " connections";
    connectionsNumber_ = connNum;
    assert(connNum > 0);
    {
        for (size_t i = 0; i < connectionsNumber_; ++i)
        {
            // can do on separate threads
            newConnection();
        }
    }
}
size_t MemGraphClientPool::readyConnsSize()
{
    return readyConnections_.size();
}
MemGraphClientPool::~MemGraphClientPool() noexcept
{
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    //    for (auto const &conn : connections_)
    //    {
    // Deallocate the client because mg_finalize has to be called globally.
    //    conn->reset(nullptr); // giving error
    //    }

    readyConnections_.clear();
    busyConnections_.clear();
}
MemGraphClientPool::DbConnectionPtr MemGraphClientPool::getDBConnection()
{
    DbConnectionPtr conn;
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
        LOG_FATAL << "no ready connections. Please Increase ready "
                     "connections pool";
    }
    else
    {
        // move ready connection -> busy connection
        auto iter = std::begin(readyConnections_);
        busyConnections_.insert(*iter);
        conn = *iter;
        readyConnections_.erase(iter);
    }
    return conn;
}

void MemGraphClientPool::freeDBConnection(DbConnectionPtr conn)
{
    std::lock_guard<std::mutex> guard(connectionsMutex_);
    busyConnections_.erase(conn);
    readyConnections_.insert(conn);
}
void MemGraphClientPool::deleteBadDBConnection(DbConnectionPtr conn)
{
    std::lock_guard<std::mutex> guard(connectionsMutex_);
    busyConnections_.erase(conn);
    readyConnections_.erase(conn);
    newConnection();
}
std::vector<std::vector<mg::Value>> MemGraphClientPool::request(
    std::string const &body)  //, mg::ConstMap const &params
{
    auto conn = getDBConnection();
    if (conn)
    {
        //    conn->setDatabaseName(database);
        if (!conn->Execute(body))
        {
            std::cerr << "Failed to execute query!" << body << " "
                      << mg_session_error(conn->session_) << std::endl;
            //        return 1;
        }
        std::vector<std::vector<mg::Value>> resp;
        while (const auto maybeResult = conn->FetchOne())
        {
            resp.emplace_back(std::move(*maybeResult));
        }
        freeDBConnection(conn);
        return resp;
    }
    else
    {
        std::cerr << "Failed to connect!\n";
        //        return 1;
    }
    LOG_FATAL << "This Should Not happened.";  // Todo fix this
    return {};
}
std::vector<std::vector<mg::Value>> MemGraphClientPool::request(
    std::string const &body,
    mg::ConstMap const &params)
{
    auto conn = getDBConnection();
    if (conn)
    {
        //    conn->setDatabaseName(database);
        if (!conn->Execute(body, params))
        {
            std::cerr << "Failed to execute query!" << body << " "
                      << mg_session_error(conn->session_) << std::endl;
            //        return 1;
        }
        std::vector<std::vector<mg::Value>> resp;
        while (const auto maybeResult = conn->FetchOne())
        {
            resp.emplace_back(std::move(*maybeResult));
        }
        freeDBConnection(conn);
        return resp;
    }
    else
    {
        std::cerr << "Failed to connect!\n";
        //        return 1;
    }
    LOG_FATAL << "This Should Not happened.";  // Todo fix this
    return {};
}

int MemGraphClientPool::getIdFromResponse(
    std::vector<std::vector<mg::Value>> const &response)
{
    int userId = 0;
    for (auto &row : response)
    {
        for (auto &matchPart : row)
        {
            if (matchPart.type() == mg::Value::Type::Node)
            {
                userId = matchPart.ValueNode().id().AsInt();
            }
        }
    }
    return userId;
}
int MemGraphClientPool::getIdFromRelationshipResponse(
    std::vector<std::vector<mg::Value>> const &response)
{
    int userId = 0;
    for (auto &row : response)
    {
        for (auto &matchPart : row)
        {
            if (matchPart.type() == mg::Value::Type::Relationship)
            {
                userId = matchPart.ValueRelationship().id().AsInt();
            }
        }
    }
    return userId;
}
MemGraphClientPool::DbConnectionPtr MemGraphClientPool::newConnection()
{
    DbConnectionPtr connPtr;
    mg::Client::Params params;
    params.host = "localhost";
    params.port = 7687;
    params.use_ssl = false;
    auto client = mg::Client::Connect(params);
    connPtr = std::move(client);
    if (connPtr)
    {
        LOG_TRACE << "connected!";
        std::lock_guard<std::mutex> guard(connectionsMutex_);
        readyConnections_.insert(connPtr);
    }
    else
    {
        LOG_DEBUG << "Failed to connect!";
    }
    return connPtr;
    // it set callback for close, ok and idle.
    // on close : erase from 3 maps. and create new connection
    // on ok : busyConnections insert
    // on idle : busyConnections remove, readyConnections insert
}
void initializeMemGraphPool(int count)
{
    ok::db::memgraph_conns.initialize(count);
    if (ok::db::memgraph_conns.readyConnsSize() <= 0)
    {
        LOG_DEBUG << "cant connect to database. exiting application";
        exit(0);
    }
}
}  // namespace ok::db
