#include "database/online_database.h"
#include "successor_manager/successor_manager.h"
#include "test_utils.h"

#include "gtest/gtest.h"

namespace test {

namespace fs = std::filesystem;

class SuccessorManagerTest : public ::testing::Test {
protected:
  void SetUp() { tmp_dir = test::createDataForOnlineDatabase(); }
  void TearDown() {
    test::clearDataForOnlineDatabase(tmp_dir);
    tmp_dir = "";
  }
  fs::path tmp_dir = "";
};

TEST_F(SuccessorManagerTest, create) {
  ASSERT_DEATH(localization::successor_manager::SuccessorManager(
                   nullptr, nullptr, /*fanOut=*/0),
               "Database is not set.");
  // Create test database.
  localization::database::OnlineDatabase database(
      /*queryFeaturesDir=*/tmp_dir,
      /*refFeaturesDir=*/tmp_dir,
      localization::features::FeatureType::Cnn_Feature,
      /*bufferSize=*/10);
  ASSERT_DEATH(localization::successor_manager::SuccessorManager(
                   &database, nullptr, /*fanOut=*/0),
               "Relocalizer is not set.");
}
} // namespace test