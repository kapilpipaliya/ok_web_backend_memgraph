#include "db/auth_fns.hpp"
#include "db/mgclientPool.hpp"
#include "third_party/mgclient/src/mgvalue.h"
#include "jwt/jwt.hpp"
#include "utils/BatchArrayMessage.hpp"
#include "utils/html_functions.hpp"
#include "utils/json_functions.hpp"
#include "utils/mg_helper.hpp"
#include "utils/time_functions.hpp"

int ok::db::auth::getMemberKeyFromJwt(std::string const &jwtEncodedCookie)
{
    int memberKey{-1};
    if (jwtEncodedCookie.empty())
        return memberKey;
    auto dec_obj = ok::utils::jwt_functions::decodeCookie(jwtEncodedCookie);
    if (dec_obj.payload().has_claim("memberKey"))
    {
        auto memberKey =
            dec_obj.payload().get_claim_value<std::string>("memberKey");
        return std::stoi(memberKey);
    }
    return memberKey;
}

std::tuple<ErrorMsg, int> ok::db::auth::registerFn(jsoncons::ojson const &o)
{
    if (jsoncons::ObjectMemberIsObject(o, "body"))
    {
        if (!jsoncons::ObjectMemberIsString(o["body"], "email"))
            return {"Email Adress must be provided", -1};
        if (!jsoncons::ObjectMemberIsString(o["body"], "pass"))
            return {"Password must be provided", -1};
    }
    else
        return {"argument should be passed in body", -1};
    auto email = o["body"]["email"].as_string();
    auto password = o["body"]["pass"].as_string();
    ok::db::MGParams p{{"email", mg_value_make_string(email.c_str())}};

    std::string query{"MATCH (u:User {email: $email}) RETURN u;"};
    const auto [error, maybeResult] = mgCall(query, p);
    if (!error.empty())
    {
        return {error, {}};
    }
    if (ok::db::getIdFromResponse(*maybeResult) != -1)
    {
        return {"Email Adress already exist", -1};
    }

    ok::db::MGParams p2{{"email", mg_value_make_string(email.c_str())},
                        {"pass", mg_value_make_string(password.c_str())},
                        {"createdAt",
                         mg_value_make_integer(
                             utils::time::getEpochMilliseconds())}};

    const auto [error2, maybeResult2] = mgCall(
        "CREATE (c:User {email: $email, pass: $pass, createdAt: "
        "$createdAt}) return c;",
        p2);
    if (!error2.empty())
    {
        return {error2, -1};
    }

    auto userId = ok::db::getIdFromResponse(*maybeResult2);
    if (userId == -1)
        return {"cant set new password", -1};
    else
        return {"", ok::db::getIdFromResponse(*maybeResult2)};
}

std::tuple<ErrorMsg, int> ok::db::auth::login(const jsoncons::ojson &o)
{
    if (jsoncons::ObjectMemberIsObject(o, "body"))
    {
        if (!jsoncons::ObjectMemberIsString(o["body"], "email"))
            return {"Email Adress must be provided", -1};
        if (!jsoncons::ObjectMemberIsString(o["body"], "pass"))
            return {"Password must be provided", -1};
    }
    else
        return {"argument should be passed in body", -1};

    auto email = o["body"]["email"].as_string();
    auto password = o["body"]["pass"].as_string();

    ok::db::MGParams p{{"email", mg_value_make_string(email.c_str())},
                       {"pass", mg_value_make_string(password.c_str())}};

    std::string query{"MATCH (u:User {email: $email, pass: $pass}) return u;"};
    const auto [error, maybeResult] = mgCall(query, p);
    if (!error.empty())
    {
        return {error, -1};
    }

    return {"", ok::db::getIdFromResponse(*maybeResult)};
}

std::tuple<int, jsoncons::ojson> ok::db::auth::loginJwt(
    std::string const &jwtEncoded)
{
    auto memberKey = db::auth::getMemberKeyFromJwt(jwtEncoded);
    if (memberKey == -1)
    {
        return {memberKey, jsoncons::ojson::null()};
    }
    else
    {
        auto [error, member] = ok::db::auth::user(memberKey);
        if (error.empty())
        {
            return {member["id"].as<int>(), member};
        }
        else
        {
            return {memberKey, jsoncons::ojson::null()};
        }
    }
}
// wip
std::tuple<ErrorMsg, std::string> ok::db::auth::change_password(
    VertexId const &memberKey,
    jsoncons::ojson const &o)
{
    if (jsoncons::ObjectMemberIsObject(o, "body"))
    {
        if (!jsoncons::ObjectMemberIsString(o["body"], "email"))
            return {"email Adress must be provided", ""};
        if (!jsoncons::ObjectMemberIsString(o["body"], "old_password"))
            return {"old Password must be provided", ""};
        if (!jsoncons::ObjectMemberIsString(o["body"], "new_password"))
            return {"pew Password must be provided", ""};
    }
    else
        return {"argument should be passed in body", ""};
    auto old_password = o["body"]["old_password"].as_string();
    auto new_password = o["body"]["new_password"].as_string();
    if (old_password == new_password)
        return {"old password and new password cannot be same", ""};

    // get email from memberKey
    // todo password check using memberkey not from email
    auto email = o["body"]["email"].as_string();

    ok::db::MGParams p{{"email", mg_value_make_string(email.c_str())},
                       {"pass", mg_value_make_string(old_password.c_str())}};

    std::string query{
        "MATCH (u:User {email: $email, password: $pass}) return u;"};
    const auto [error, maybeResult] = mgCall(query, p);
    if (!error.empty())
    {
        return {error, ""};
    }
    if (ok::db::getIdFromResponse(*maybeResult) == -1)
    {
        return {"Old password is not correct", ""};
    }

    ok::db::MGParams p2{{"email", mg_value_make_string(email.c_str())},
                        {"pass", mg_value_make_string(old_password.c_str())},
                        {"newPassword",
                         mg_value_make_string(new_password.c_str())}};

    const auto [error2, maybeResult2] = mgCall(
        "MATCH (u:User {email: $email, pass: $pass}) SET "
        "u.pass = $newPassword return u;",
        p2);
    if (!error2.empty())
    {
        return {error2, ""};
    }

    auto userId = ok::db::getIdFromResponse(*maybeResult2);
    if (userId == -1)
        return {"cant set new password", ""};
    else
        return {"", "Success"};
}
namespace {
jsoncons::ojson getRolesOfUser(int const &memberKey){
    if (!memberKey)
        return jsoncons::ojson::null();

    ok::db::MGParams p{{"id", mg_value_make_integer(memberKey)}};

    std::string query{"MATCH (c:User)-[r:UserRole]->(n)  WHERE ID(c) = $id return COLLECT(DISTINCT id(n));"};
    const auto [error, maybeResult] = mgCall(query, p);
    if (!error.empty())
    {
        return jsoncons::ojson::null();
    }
    if (maybeResult)
    {
        if (maybeResult.value().size() > 0)
        {
            jsoncons::ojson roleIds = jsoncons::ojson::array();
            for (auto &row : *maybeResult)
                for (auto &matchPart : row)
                    if (matchPart.type() == mg::Value::Type::List)
                        roleIds = convertListToJson(matchPart.ValueList());
            return roleIds;
        }
        else
            return jsoncons::ojson::null();
    }
    return jsoncons::ojson::null();
}
}
std::tuple<ErrorMsg, jsoncons::ojson> ok::db::auth::user(int const memberKey)
{
    if (!memberKey)
        return {"Not Logged In", jsoncons::ojson::null()};

    ok::db::MGParams p{{"id", mg_value_make_integer(memberKey)}};

    // TODO: can make generic function for this:
    std::string query{"MATCH (u) WHERE ID(u) = $id RETURN u;"};
    const auto [error, maybeResult] = mgCall(query, p);
    if (!error.empty())
    {
        return {error, jsoncons::ojson::null()};
    }
    if (ok::db::getIdFromResponse(*maybeResult) == -1)
    {
        return {std::string{"User"} + std::to_string(memberKey) + " Not Found",
                jsoncons::ojson::null()};
    }
    if (maybeResult)
    {
        if (maybeResult.value().size() > 0)
        {
            auto j = convertNodeToJson(maybeResult.value()[0][0].ValueNode());
            j["P"].erase("pass");
            j["P"]["roles"] = getRolesOfUser(memberKey);
            return {"", j};
        }
        else
            return {"user not exist", jsoncons::ojson::null()};
    }
    return {"user not exist", jsoncons::ojson::null()};
}
