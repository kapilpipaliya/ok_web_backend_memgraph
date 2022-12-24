#include <gtest/gtest.h>
#include "db/mgclientPool.hpp"
#include "db/auth_fns.hpp"

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions)
{
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}

TEST(RegistrationTest, BasicAssertions)
{
    ok::db::MGParams p{{"email", mg_value_make_string("kapilp")}};
    auto response =
        ok::db::memgraph_conns.request("MATCH (u {email: $email}) DELETE u",
                                       p.asConstMap());
    jsoncons::ojson registerArgs = jsoncons::ojson::parse(R"(
    {
        "body": {
            "email": "kapilp",
            "password": "pass"
        }
    }
    )");
    auto [er, userId] = ok::db::auth::registerFn(registerArgs);
    EXPECT_NE(userId, 0);

    auto [er2, userId2] = ok::db::auth::login(registerArgs);
    EXPECT_EQ(userId, userId);

    auto [er3, user] = ok::db::auth::user(userId);
    EXPECT_EQ(userId, user["id"].as<int>());
}

int main(int argc, char **argv)
{
    ok::db::initializeMemGraphPool(8);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
