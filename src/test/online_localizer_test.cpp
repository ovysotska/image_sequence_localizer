
#include "database/cost_matrix_database.h"
#include "features/ifeature.h"
#include "localization_protos.pb.h"
#include "online_localizer/online_localizer.h"
#include "relocalizers/irelocalizer.h"
#include "successor_manager/successor_manager.h"
#include "test_utils.h"

#include "gtest/gtest.h"

#include <iostream>
#include <memory>
#include <string>

namespace {

class FakeRelocalizer : public iRelocalizer {
public:
  std::vector<int> getCandidates(int quId) override { return {0}; }
};

class OnlineLocalizerTest : public ::testing::Test {
public:
  void SetUp() {
    tmp_dir = localization::test::createDataForOnlineDatabase();
    std::filesystem::path featureDir = tmp_dir;
    image_sequence_localizer::CostMatrix cost_matrix =
        localization::test::computeCostMatrix(featureDir, featureDir);
    std::string costMatrixFile = tmp_dir / "test.CostMatrix.pb";
    std::fstream out(costMatrixFile,
                     std::ios::out | std::ios::trunc | std::ios::binary);
    cost_matrix.SerializeToOstream(&out);
    out.close();

    database = std::make_unique<CostMatrixDatabase>(
        costMatrixFile, featureDir, featureDir, FeatureType::Cnn_Feature, 10);
    relocalizer = std::make_unique<FakeRelocalizer>();
    successorManager = std::make_unique<SuccessorManager>(database.get(),
                                                          relocalizer.get(), 2);
    localizer = std::make_unique<online_localizer::OnlineLocalizer>(
        successorManager.get(), 1.0, 100.0);
  }
  std::unique_ptr<online_localizer::OnlineLocalizer> localizer = nullptr;
  std::unique_ptr<SuccessorManager> successorManager = nullptr;
  std::unique_ptr<iRelocalizer> relocalizer = nullptr;
  std::unique_ptr<OnlineDatabase> database = nullptr;
  std::filesystem::path tmp_dir = "";
};

TEST_F(OnlineLocalizerTest, Get) {
  online_localizer::Matches matches = localizer->findMatchesTill(4);

  // Expecting diagonal elements as path in reverse.
  for (int i = 0; i < matches.size(); ++i) {
    EXPECT_EQ(matches[i].quId, matches.size() - 1 - i);
    EXPECT_EQ(matches[i].refId, matches.size() - 1 - i);
    EXPECT_EQ(matches[i].state, NodeState::REAL);
  }
}

} // namespace
