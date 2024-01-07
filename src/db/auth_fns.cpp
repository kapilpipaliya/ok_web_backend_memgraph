#include "db/auth_fns.hpp"
#include "db/mgclientPool.hpp"
#include "third_party/mgclient/src/mgvalue.h"
#include "jwt/jwt.hpp"
#include "utils/html_functions.hpp"
#include "lib/json_functions.hpp"
#include "lib/mg_helper.hpp"
#include "utils/time_functions.hpp"
namespace ok::db::auth
{
std::tuple<SubDomain, int>  getMemberKeyFromJwt(std::string const &jwtEncodedCookie)
{
    int memberKey{-1};
    if (jwtEncodedCookie.empty())
        return {"", memberKey};
    auto dec_obj = ok::utils::jwt_functions::decodeCookie(jwtEncodedCookie);
    if (dec_obj.payload().has_claim("memberKey"))
    {
        auto memberKey_ =
            dec_obj.payload().get_claim_value<std::string>("memberKey");
        memberKey =  std::stoi(memberKey_);
    }
    SubDomain subDomain;
    if (dec_obj.payload().has_claim("subDomain"))
    {
        subDomain =
            dec_obj.payload().get_claim_value<std::string>("subDomain");

    }
    return {subDomain, memberKey};
}

std::tuple<ErrorMsg, int> registerFn(jsoncons::ojson const &o, SubDomain const &subDomain, int mgPort)
{
    if (jsoncons::ObjectMemberIsObject(o, "body"))
    {
        if (!jsoncons::ObjectMemberIsString(o["body"], "email"))
            return {"Email Address must be provided", -1};
        if (!jsoncons::ObjectMemberIsString(o["body"], "pass"))
            return {"Password must be provided", -1};
    }
    else
        return {"argument should be passed in body", -1};
    auto email = o["body"]["email"].as_string();
    auto password = o["body"]["pass"].as_string();
    ok::db::MGParams p{{"email", mg_value_make_string(email.c_str())}};

    std::string query{"MATCH (u:User {email: $email}) RETURN u;"};
    const auto [error, maybeResult] = mgCall(query, p, mgPort);
    if (!error.empty())
    {
        return {error, {}};
    }
    if (ok::db::getIdFromMGResponse(*maybeResult) != -1)
    {
        return {"Email Address already exist", -1};
    }

    ok::db::MGParams p2{{"email", mg_value_make_string(email.c_str())},
                        {"pass", mg_value_make_string(password.c_str())},
                        {"createdAt",
                         mg_value_make_integer(
                             utils::time::getEpochMilliseconds())}};

    const auto [error2, maybeResult2] = mgCall(
        "CREATE (c:User {email: $email, pass: $pass, createdAt: "
        "$createdAt}) return c;",
        p2,
        mgPort);
    if (!error2.empty())
    {
        return {error2, -1};
    }

    auto userId = ok::db::getIdFromMGResponse(*maybeResult2);
    if (userId == -1)
        return {"cant set new password", -1};
    else {
        if (subDomain == "system") {
            // 1. fire a script to create a db
        }
        return {"", ok::db::getIdFromMGResponse(*maybeResult2)};
    }
}

std::tuple<ErrorMsg, int> login(const jsoncons::ojson &o, int mgPort)
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
    const auto [error, maybeResult] = mgCall(query, p, mgPort);
    if (!error.empty())
    {
        return {error, -1};
    }

    return {"", ok::db::getIdFromMGResponse(*maybeResult)};
}

std::tuple<int, jsoncons::ojson> loginJwt(std::string const &jwtEncoded)
{
    auto [subDomain, memberKey] = db::auth::getMemberKeyFromJwt(jwtEncoded);
    auto mgPort = ok::db::auth::getSubDomainMGPort(subDomain);
    if (subDomain.empty() || memberKey == -1)
    {
        return {-1, jsoncons::ojson::null()};
    }
    else
    {
        auto [error, member] = user(memberKey, mgPort);
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
std::tuple<ErrorMsg, std::string> change_password(VertexId const &memberKey, int mgPort,
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
    const auto [error, maybeResult] = mgCall(query, p, mgPort);
    if (!error.empty())
    {
        return {error, ""};
    }
    if (ok::db::getIdFromMGResponse(*maybeResult) == -1)
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
        p2,
        mgPort);
    if (!error2.empty())
    {
        return {error2, ""};
    }

    auto userId = ok::db::getIdFromMGResponse(*maybeResult2);
    if (userId == -1)
        return {"cant set new password", ""};
    else
        return {"", "Success"};
}
namespace
{
jsoncons::ojson getRolesOfUser(int const &memberKey, int mgPort)
{
    if (!memberKey)
        return jsoncons::ojson::null();

    ok::db::MGParams p{{"id", mg_value_make_integer(memberKey)}};

    std::string query{
        "MATCH (c:User)-[r:UserRole]->(n)  WHERE ID(c) = $id return COLLECT(DISTINCT id(n));"};
    const auto [error, maybeResult] = mgCall(query, p, mgPort);
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
std::tuple<ErrorMsg, jsoncons::ojson> user(int const memberKey, int mgPort)
{
    if (!memberKey)
        return {"Not Logged In", jsoncons::ojson::null()};

    ok::db::MGParams p{{"id", mg_value_make_integer(memberKey)}};

    // TODO: can make generic function for this:
    std::string query{"MATCH (u) WHERE ID(u) = $id RETURN u;"};
    const auto [error, maybeResult] = mgCall(query, p, mgPort);
    if (!error.empty())
    {
        return {error, jsoncons::ojson::null()};
    }
    if (ok::db::getIdFromMGResponse(*maybeResult) == -1)
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
            j["P"]["roles"] = getRolesOfUser(memberKey, mgPort);
            return {"", j};
        }
        else
            return {"user not exist", jsoncons::ojson::null()};
    }
    return {"user not exist", jsoncons::ojson::null()};
}
int getSubDomainMGPort(std::string const &subDomain)
{
    if (subDomain.empty()) {
        return -1;
    }
    // make a mg call to get port
    std::string query{"MATCH  (n {subDomain: '" + subDomain + "'}) RETURN n"};
    
    mg::Client::Params params;
    params.host = "localhost";
    params.port = 1102;
    params.use_ssl = false;
    
    auto mgClient = mg::Client::Connect(params);
    if (!mgClient)
    {
        LOG_ERROR << "Failed to connect MG Server.";
        return -1;
    }
    
    if (!mgClient->Execute(query))
    {
        LOG_ERROR << "Failed to execute query!" << query << " "
                  << mg_session_error(mgClient->session_);
        return -1;
    }
    try
    {
        const auto maybeResult = mgClient->FetchAll();
        if (maybeResult)
            for (auto &row : *maybeResult)
                for (auto &matchPart : row)
                    if (matchPart.type() == mg::Value::Type::Node) {
                        auto port = matchPart.ValueNode().properties()["a"];
                        if (port.type() == mg::Value::Type::Int) {
                            return port.ValueInt();
                        }
                        return -1;
                    }
    }
    catch (mg::ClientException e)
    {
        LOG_ERROR << e.what();
        return -1;
    }
}
}
