#include "db/auth_fns.hpp"
#include "db/mgclientPool.hpp"
#include "third_party/mgclient/src/mgvalue.h"
#include "jwt/jwt.hpp"
#include "utils/html_functions.hpp"
#include "utils/json_functions.hpp"
#include "utils/mg_helper.hpp"
#include "utils/time_functions.hpp"

int ok::db::auth::getMemberKeyFromJwt(std::string const &jwtEncodedCookie)
{
    int memberKey{0};
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
std::tuple<std::string, int> ok::db::auth::registerFn(jsoncons::ojson const &o)
{
    if (jsoncons::ObjectMemberIsObject(o, "body"))
    {
        if (!jsoncons::ObjectMemberIsString(o["body"], "email"))
            return {"Email Adress must be provided", 0};
        if (!jsoncons::ObjectMemberIsString(o["body"], "password"))
            return {"Password must be provided", 0};
    }
    else
        return {"argument should be passed in body", 0};
    auto email = o["body"]["email"].as_string();
    auto password = o["body"]["password"].as_string();
    ok::db::MGParams p{{"email", mg_value_make_string(email.c_str())}};
    auto response = ok::db::memgraph_conns.request(
        "MATCH (u:User {email: $email}) RETURN u", p.asConstMap());
    if (response.size() > 0)
        return {"Email Adress already exist", 0};

    ok::db::MGParams p2{{"email", mg_value_make_string(email.c_str())},
                        {"password", mg_value_make_string(password.c_str())},
                        {"createdAt",
                         mg_value_make_integer(
                             utils::time::getEpochMilliseconds())}};
    auto response2 = ok::db::memgraph_conns.request(
        "CREATE (c:User {email: $email, password: $password, createdAt: "
        "$createdAt}) "
        "return c;",
        p2.asConstMap());
    return {"", ok::db::memgraph_conns.getIdFromResponse(response2)};
}

std::tuple<std::string, int> ok::db::auth::login(const jsoncons::ojson &o)
{
    if (jsoncons::ObjectMemberIsObject(o, "body"))
    {
        if (!jsoncons::ObjectMemberIsString(o["body"], "email"))
            return {"Email Adress must be provided", 0};
        if (!jsoncons::ObjectMemberIsString(o["body"], "password"))
            return {"Password must be provided", 0};
    }
    else
        return {"argument should be passed in body", 0};
    auto email = o["body"]["email"].as_string();
    auto password = o["body"]["password"].as_string();
    ok::db::MGParams p{{"email", mg_value_make_string(email.c_str())},
                       {"password", mg_value_make_string(password.c_str())}};
    auto response = ok::db::memgraph_conns.request(
        "MATCH (u:User {email: $email, password: $password}) return u;",
        p.asConstMap());
    return {"", ok::db::memgraph_conns.getIdFromResponse(response)};
}

// wip
std::tuple<std::string, std::string> ok::db::auth::change_password(
    VertexId const &memberKey,
    jsoncons::ojson const &o)
{
    if (jsoncons::ObjectMemberIsObject(o, "body"))
    {
        if (!jsoncons::ObjectMemberIsString(o["body"], "email"))
            return {"email Adress must be provided", 0};
        if (!jsoncons::ObjectMemberIsString(o["body"], "old_password"))
            return {"old Password must be provided", 0};
        if (!jsoncons::ObjectMemberIsString(o["body"], "new_password"))
            return {"pew Password must be provided", 0};
    }
    else
        return {"argument should be passed in body", 0};
    auto old_password = o["body"]["old_password"].as_string();
    auto new_password = o["body"]["new_password"].as_string();
    if (old_password == new_password)
        return {"old password and new password cannot be same", ""};

    // get email from memberKey
    // todo password check using memberkey not from email
    auto email = o["body"]["email"].as_string();

    ok::db::MGParams p{{"email", mg_value_make_string(email.c_str())},
                       {"password",
                        mg_value_make_string(old_password.c_str())}};
    auto response = ok::db::memgraph_conns.request(
        "MATCH (u:User {email: $email, password: $password}) return u;",
        p.asConstMap());
    if (ok::db::memgraph_conns.getIdFromResponse(response) == 0)
        return {"Old password is not correct", ""};

    ok::db::MGParams p2{{"email", mg_value_make_string(email.c_str())},
                        {"password",
                         mg_value_make_string(old_password.c_str())},
                        {"newPassword",
                         mg_value_make_string(new_password.c_str())}};
    auto response2 = ok::db::memgraph_conns.request(
        "MATCH (u:User {email: $email, password: $password}) SET "
        "u.pass = $newPassword return u;",
        p2.asConstMap());
    auto userId = ok::db::memgraph_conns.getIdFromResponse(response);
    if (userId == 0)
        return {"cant set new password", ""};
    else
        return {"", "Success"};
}

std::tuple<std::string, jsoncons::ojson> ok::db::auth::user(int const memberKey)
{
    if (!memberKey)
        return {"Not Logged In", ""};

    ok::db::MGParams p{{"id", mg_value_make_integer(memberKey)}};
    auto response =
        ok::db::memgraph_conns.request("MATCH (u) WHERE ID(u) = $id RETURN u",
                                       p.asConstMap());
    if (response.size() > 0)
        return {"", convertNodeToJson(response[0][0].ValueNode())};
    else
        return {"user not exist", ""};
}
