#include "db/mgclientPool.hpp"
#include <trantor/utils/Logger.h>
#include "mgclient.hpp"
namespace ok::db
{
int getIdFromMGResponse(std::vector<std::vector<mg::Value>> const &response)
{
    int userId = -1;
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
std::vector<std::string> getNamesFromMGResponse(
    std::vector<std::vector<mg::Value>> const &response)
{
    std::vector<std::string> collections{};
    for (auto &row : response)
    {
        for (auto &matchPart : row)
        {
            if (matchPart.type() == mg::Value::Type::Node)
            {
                auto properties = matchPart.ValueNode().properties();

                auto id = properties.find("name");
                if (id == properties.end()) {
                    LOG_WARN << "Cant Find Name For: " << matchPart.ValueNode().id().AsInt();
                } else {
                    auto [key, value] = *id;
                    collections.emplace_back(value.ValueString());
                }
            }
        }
    }
    return collections;
}
int getIdFromMGRelationshipResponse(
    std::vector<std::vector<mg::Value>> const &response)
{
    int userId = -1;
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
std::pair<std::string, std::optional<std::vector<std::vector<mg::Value>>>>
mgCall(std::string const &query, ok::db::MGParams &p, int mgPort)
{
    auto mgClientParams = mg::Client::Params{};
    mgClientParams.port = mgPort;
    auto ngClient = mg::Client::Connect(mgClientParams);
    if (!ngClient)
    {
        return {"Failed to connect MG Server", {}};
    }
    if (!ngClient->Execute(query, p.asConstMap()))
    {
        auto error = std::string{};
        error += "Failed to execute query!" + query + " " +
                 mg_session_error(ngClient->session_);
        // TODO: report error to db, reason, status code
        // everything;
        return {error, {}};
    }
    try
    {
        const auto maybeResult = ngClient->FetchAll();
        return {"", maybeResult};
    }
    catch (mg::ClientException e)
    {
        // TODO: report error to db, reason, status code
        // everything;
        ngClient->RollbackTransaction();

        auto error =
            std::string{
                "Cant replace because internal "
                "server error."} +
            e.what();
        return {error, {}};
    }
}
}  // namespace ok::db
