#include <gtest/gtest.h>
//#include "db/mgclientPool.hpp"



// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
//    ok::db::initializeMemGraphPool(8);
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}

