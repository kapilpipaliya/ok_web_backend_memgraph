#include "db/mgclientPool.hpp"
#include "mgclient.hpp"
namespace ok::db
{
int getIdFromResponse(std::vector<std::vector<mg::Value>> const &response)
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
std::vector<std::string> getIdsFromResponse(
    std::vector<std::vector<mg::Value>> const &response)
{
    std::vector<std::string> collections{};
    for (auto &row : response)
    {
        for (auto &matchPart : row)
        {
            if (matchPart.type() == mg::Value::Type::Node)
            {
                auto id = matchPart.ValueNode().properties().find("id");
                auto [key, value] = *id;
                collections.emplace_back(value.ValueString());
            }
        }
    }
    return collections;
}
int getIdFromRelationshipResponse(
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
mgCall(std::string const &query, ok::db::MGParams &p)
{
    auto client = mg::Client::Connect(mg::Client::Params{});
    if (!client)
    {
        return {"Failed to connect MG Server", {}};
    }
    if (!client->Execute(query, p.asConstMap()))
    {
        auto error = std::string{};
        error += "Failed to execute query!" + query + " " +
                 mg_session_error(client->session_);
        // TODO: report error to db, reason, status code
        // everything;
        return {error, {}};
    }
    try
    {
        const auto maybeResult = client->FetchAll();
        return {"", maybeResult};
    }
    catch (mg::ClientException e)
    {
        // TODO: report error to db, reason, status code
        // everything;
        client->RollbackTransaction();

        auto error =
            std::string{
                "Cant replace because internal "
                "server error."} +
            e.what();
        return {error, {}};
    }
}
}  // namespace ok::db
