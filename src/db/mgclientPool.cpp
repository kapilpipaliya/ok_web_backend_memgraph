#include "db/mgclientPool.hpp"

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
int getIdFromRelationshipResponse(
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
}  // namespace ok::db
