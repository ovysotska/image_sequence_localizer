/** vpr_relocalization: a library for visual place recognition in changing
** environments with efficient relocalization step.
** Copyright (c) 2017 O. Vysotska, C. Stachniss, University of Bonn
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/

/* Updated by O. Vysotska in 2022 */

#include "database/cost_matrix_database.h"
#include "database/online_database.h"
#include "localization_protos.pb.h"
#include "test_utils.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace test {

namespace fs = std::filesystem;
namespace loc_database = localization::database;

using FeatureType = localization::features::FeatureType;

class OnlineDatabaseTest : public ::testing::Test {
protected:
  std::string createCostMatrixProto(const std::filesystem::path &dir) {
    image_sequence_localizer::CostMatrix cost_matrix;
    for (double value : {1, 2, 3, 4, 5, 6}) {
      cost_matrix.add_values(value);
    }
    cost_matrix.set_cols(3);
    cost_matrix.set_rows(2);

    std::string cost_matrix_name =
        (dir / "test_cost_matrix.CostMatrix.pb").string();

    std::fstream out(cost_matrix_name,
                     std::ios::out | std::ios::trunc | std::ios::binary);
    cost_matrix.SerializeToOstream(&out);
    out.close();
    return cost_matrix_name;
  }
  void SetUp() { tmp_dir = test::createFeatures(); }
  void TearDown() {
    test::clearDataUnderPath(tmp_dir);
    tmp_dir = "";
  }
  std::filesystem::path tmp_dir = "";
};

TEST(OnlineDatabase, NoFeatureFiles) {
  fs::path empty_tmp_dir = fs::temp_directory_path();
  fs::create_directories(empty_tmp_dir);
  ASSERT_DEATH(loc_database::OnlineDatabase(empty_tmp_dir, empty_tmp_dir,
                                            FeatureType::Cnn_Feature, 10),
               "Query features are not set.");
}

TEST_F(OnlineDatabaseTest, refSize) {
  loc_database::OnlineDatabase database(
      /*queryFeaturesDir=*/tmp_dir,
      /*refFeaturesDir=*/tmp_dir, FeatureType::Cnn_Feature,
      /*bufferSize=*/10);
  EXPECT_EQ(4, database.refSize());
}

TEST_F(OnlineDatabaseTest, InvalidConstructorParams) {
  ASSERT_DEATH(loc_database::OnlineDatabase(
                   /*queryFeaturesDir=*/"",
                   /*refFeaturesDir=*/"", FeatureType::Cnn_Feature,
                   /*bufferSize=*/10),
               "Feature directory does not exist.");

  ASSERT_DEATH(
      loc_database::OnlineDatabase(tmp_dir, "", FeatureType::Cnn_Feature, 10),
      "Feature directory does not exist.");
  ASSERT_DEATH(loc_database::OnlineDatabase(tmp_dir, tmp_dir,
                                            FeatureType::Cnn_Feature, -1),
               "Invalid featureBuffer size.");
}

TEST_F(OnlineDatabaseTest, NoCostMatrixFile) {

  ASSERT_DEATH(loc_database::OnlineDatabase(
                   tmp_dir, tmp_dir, FeatureType::Cnn_Feature, 10, tmp_dir),
               "Failed to parse cost_matrix file: ");
}

TEST_F(OnlineDatabaseTest, CostMatrixDatabaseConstructor) {
  std::string cost_matrix_name = createCostMatrixProto(tmp_dir);
  loc_database::OnlineDatabase database(/*queryFeaturesDir=*/tmp_dir,
                                        /*refFeaturesDir=*/tmp_dir,
                                        /*type=*/FeatureType::Cnn_Feature,
                                        /*bufferSize=*/10, cost_matrix_name);
  EXPECT_EQ(database.getCost(0, 0), 1);
  EXPECT_DOUBLE_EQ(database.getCost(0, 2), 1. / 3);
  EXPECT_DOUBLE_EQ(database.getCost(1, 0), 1. / 4);
  EXPECT_DOUBLE_EQ(database.getCost(1, 2), 1. / 6);
}

TEST_F(OnlineDatabaseTest, CostMatrixDatabaseGetCost) {
  std::string cost_matrix_name = createCostMatrixProto(tmp_dir);
  loc_database::OnlineDatabase database(/*queryFeaturesDir=*/tmp_dir,
                                        /*refFeaturesDir=*/tmp_dir,
                                        /*type=*/FeatureType::Cnn_Feature,
                                        /*bufferSize=*/10, cost_matrix_name);
  EXPECT_EQ(database.getCost(0, 0), 1);
  EXPECT_DOUBLE_EQ(database.getCost(0, 2), 1. / 3);
  EXPECT_DOUBLE_EQ(database.getCost(1, 0), 1. / 4);
  EXPECT_DOUBLE_EQ(database.getCost(1, 2), 1. / 6);
  ASSERT_DEATH(database.getCost(-1, 0), "Row outside range -1");
  ASSERT_DEATH(database.getCost(0, -1), "Col outside range -1");
  ASSERT_DEATH(database.getCost(3, 0), "Row outside range 3");
  ASSERT_DEATH(database.getCost(0, 4), "Col outside range 4");
}
} // namespace test
