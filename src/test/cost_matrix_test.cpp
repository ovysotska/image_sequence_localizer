// #include "database/online_database.h"
#include "database/cost_matrix.h"
#include "localization_protos.pb.h"
#include "test_utils.h"

#include "gtest/gtest.h"
#include <filesystem>

class CostMatrixTest : public ::testing::Test {
public:
  void SetUp() {
    tmp_dir = localization::test::createDataForOnlineDatabase();
    std::filesystem::path featureDir = tmp_dir;
    image_sequence_localizer::CostMatrix cost_matrix =
        localization::test::computeCostMatrix(featureDir, featureDir);
    costMatrixFile = tmp_dir / "test.CostMatrix.pb";
    std::fstream out(costMatrixFile,
                     std::ios::out | std::ios::trunc | std::ios::binary);
    cost_matrix.SerializeToOstream(&out);
    out.close();
  }

  std::filesystem::path tmp_dir = "";
  std::string costMatrixFile = "";
};

TEST_F(CostMatrixTest, ConstructFromProto) {
  auto costMatrix = CostMatrix(costMatrixFile);
  EXPECT_EQ(costMatrix.rows(), 4);
  EXPECT_EQ(costMatrix.cols(), 4);
}

TEST_F(CostMatrixTest, FailedToConstruct) {
  ASSERT_DEATH(CostMatrix(""), "Cost matrix file is not set");
}

TEST_F(CostMatrixTest, at) {
  auto costMatrix = CostMatrix(costMatrixFile);
  Matrix expectedMatrix = {{1, 0.922225, 0.285714, 0.99449},
                           {0.922225, 1, 0.634029, 0.922876},
                           {0.285714, 0.634029, 1, 0.298347},
                           {0.99449, 0.922876, 0.298347, 1}};
  EXPECT_EQ(costMatrix.rows(), expectedMatrix.size());
  EXPECT_EQ(costMatrix.cols(), expectedMatrix[0].size());

  for (int r = 0; r < costMatrix.rows(); ++r) {
    for (int c = 0; c < costMatrix.cols(); ++c) {
      EXPECT_NEAR(costMatrix.at(r, c), expectedMatrix[r][c], 1e-06);
    }
  }
  ASSERT_DEATH(costMatrix.at(-1, 0), "Row outside range -1");
  ASSERT_DEATH(costMatrix.at(4, 0), "Row outside range 4");
  ASSERT_DEATH(costMatrix.at(0, -1), "Col outside range -1");
  ASSERT_DEATH(costMatrix.at(0, 4), "Col outside range 4");
}

TEST_F(CostMatrixTest, getInverseCost) {
  auto costMatrix = CostMatrix(costMatrixFile);
  EXPECT_DOUBLE_EQ(costMatrix.getInverseCost(0, 0), 1);
  EXPECT_NEAR(costMatrix.getInverseCost(1, 0), 1 / 0.922225, 1e-06);
}