/* By O. Vysotska in 2023 */

#include "database/similarity_matrix.h"
#include "localization_protos.pb.h"
#include "test_utils.h"

#include "gtest/gtest.h"
#include <filesystem>

namespace test {

class SimilarityMatrixTest : public ::testing::Test {
public:
  void SetUp() {

    tmp_dir = std::filesystem::temp_directory_path() / "SimilarityMatrixTest";
    std::filesystem::create_directories(tmp_dir);
    image_sequence_localizer::CostMatrix costMatrixProto;
    for (int r = 0; r < 2; ++r) {
      for (int c = 0; c < 3; ++c) {
        costMatrixProto.add_values(similarityMatrixValues[r][c]);
      }
    }

    costMatrixProto.set_cols(3);
    costMatrixProto.set_rows(2);
    std::string testName =
        ::testing::UnitTest::GetInstance()->current_test_info()->name();

    similarityMatrixFile = tmp_dir / (testName + "_test.CostMatrix.pb");
    std::cout << "Saving to" << similarityMatrixFile;
    std::fstream out(similarityMatrixFile,
                     std::ios::out | std::ios::trunc | std::ios::binary);
    costMatrixProto.SerializeToOstream(&out);
    out.close();
  }

  void TearDown() {
    std::filesystem::remove(similarityMatrixFile);
    std::filesystem::remove(tmp_dir);
    tmp_dir = "";
  }

  std::filesystem::path tmp_dir = "";
  std::string similarityMatrixFile = "";
  const std::vector<std::vector<double>> similarityMatrixValues = {{1, 2, 3},
                                                             {4, 5, 6}};
};

TEST_F(SimilarityMatrixTest, ConstructFromProto) {
  auto similarityMatrix = localization::database::SimilarityMatrix(similarityMatrixFile);
  EXPECT_EQ(similarityMatrix.rows(), this->similarityMatrixValues.size());
  EXPECT_EQ(similarityMatrix.cols(), this->similarityMatrixValues[0].size());
}

TEST_F(SimilarityMatrixTest, FailedToConstruct) {
  ASSERT_DEATH(localization::database::SimilarityMatrix(""),
               "Similarity matrix file is not set");
}

TEST_F(SimilarityMatrixTest, at) {
  auto similarityMatrix = localization::database::SimilarityMatrix(similarityMatrixFile);
  localization::database::SimilarityMatrix::Matrix expectedMatrix =
      this->similarityMatrixValues;
  EXPECT_EQ(similarityMatrix.rows(), expectedMatrix.size());
  EXPECT_EQ(similarityMatrix.cols(), expectedMatrix[0].size());

  for (int r = 0; r < similarityMatrix.rows(); ++r) {
    for (int c = 0; c < similarityMatrix.cols(); ++c) {
      EXPECT_NEAR(similarityMatrix.at(r, c), expectedMatrix[r][c], 1e-06);
    }
  }
  ASSERT_DEATH(similarityMatrix.at(-1, 0), "Row outside range -1");
  ASSERT_DEATH(similarityMatrix.at(4, 0), "Row outside range 4");
  ASSERT_DEATH(similarityMatrix.at(0, -1), "Col outside range -1");
  ASSERT_DEATH(similarityMatrix.at(0, 4), "Col outside range 4");
}

TEST_F(SimilarityMatrixTest, getCost) {
  auto similarityMatrix = localization::database::SimilarityMatrix(similarityMatrixFile);
  EXPECT_DOUBLE_EQ(similarityMatrix.getCost(0, 0), 1);
  EXPECT_NEAR(similarityMatrix.getCost(1, 0), 1 / this->similarityMatrixValues[1][0],
              1e-06);
}

TEST(CostMatrixComputation, createCostMatrixFromFeatures) {
  const fs::path tmp_dir = test::createFeatures();

  auto similarityMatrix = localization::database::SimilarityMatrix(
      tmp_dir, tmp_dir, localization::features::Cnn_Feature);

  for (int r = 0; r < similarityMatrix.rows(); ++r) {
    for (int c = 0; c < similarityMatrix.cols(); ++c) {
      EXPECT_NEAR(similarityMatrix.at(r, c), kSimilarityMatrix[r][c], kTestEpsilon);
    }
  }
}
} // namespace test