#include "successor_manager/successor_manager.h"
#include "gtest/gtest.h"

TEST(SuccessorManager, create) {
  ASSERT_DEATH(SuccessorManager(nullptr, nullptr, /*fanOut=*/0),
               "Database is not set.");
}
TEST(SuperTestName, testName) { EXPECT_TRUE(true); }