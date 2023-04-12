/* By O. Vysotska in 2023 */

#include "database/cost_matrix.h"
#include "localization_protos.pb.h"
#include "test_utils.h"

#include "gtest/gtest.h"
#include <filesystem>

namespace test {

class CostMatrixTest : public ::testing::Test {
public:
  void SetUp() {

    tmp_dir = std::filesystem::temp_directory_path() / "costMatrixTest";
    std::filesystem::create_directories(tmp_dir);
    image_sequence_localizer::CostMatrix costMatrixProto;
    for (int r = 0; r < 2; ++r) {
      for (int c = 0; c < 3; ++c) {
        costMatrixProto.add_values(costMatrixValues[r][c]);
      }
    }

    costMatrixProto.set_cols(3);
    costMatrixProto.set_rows(2);
    std::string testName =
        ::testing::UnitTest::GetInstance()->current_test_info()->name();

    costMatrixFile = tmp_dir / (testName + "_test.CostMatrix.pb");
    std::cout << "Saving to" << costMatrixFile;
    std::fstream out(costMatrixFile,
                     std::ios::out | std::ios::trunc | std::ios::binary);
    costMatrixProto.SerializeToOstream(&out);
    out.close();
  }

  void TearDown() {
    std::filesystem::remove(costMatrixFile);
    std::filesystem::remove(tmp_dir);
    tmp_dir = "";
  }

  std::filesystem::path tmp_dir = "";
  std::string costMatrixFile = "";
  const std::vector<std::vector<double>> costMatrixValues = {{1, 2, 3},
                                                             {4, 5, 6}};
};

TEST_F(CostMatrixTest, ConstructFromProto) {
  auto costMatrix = localization::database::CostMatrix(costMatrixFile);
  EXPECT_EQ(costMatrix.rows(), this->costMatrixValues.size());
  EXPECT_EQ(costMatrix.cols(), this->costMatrixValues[0].size());
}

TEST_F(CostMatrixTest, FailedToConstruct) {
  ASSERT_DEATH(localization::database::CostMatrix(""),
               "Cost matrix file is not set");
}

TEST_F(CostMatrixTest, at) {
  auto costMatrix = localization::database::CostMatrix(costMatrixFile);
  localization::database::CostMatrix::Matrix expectedMatrix =
      this->costMatrixValues;
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
  auto costMatrix = localization::database::CostMatrix(costMatrixFile);
  EXPECT_DOUBLE_EQ(costMatrix.getInverseCost(0, 0), 1);
  EXPECT_NEAR(costMatrix.getInverseCost(1, 0), 1 / this->costMatrixValues[1][0],
              1e-06);
}
} // namespace test