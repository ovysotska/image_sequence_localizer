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

#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;
namespace localizer = image_sequence_localizer;

TEST(OnlineDatabase, NoFeatureFiles) {
  fs::path empty_tmp_dir = fs::temp_directory_path();
  fs::create_directories(empty_tmp_dir);
  ASSERT_DEATH(OnlineDatabase(empty_tmp_dir, empty_tmp_dir,
                              FeatureType::Cnn_Feature, 10),
               "Query features are not set.");
}

class OnlineDatabaseTest : public ::testing::Test {
protected:
  localizer::Feature createFeatureProto(const std::vector<double> &values) {
    localizer::Feature feature_proto;
    feature_proto.set_type("debug");
    feature_proto.set_size(values.size());
    for (double value : values) {
      feature_proto.add_values(value);
    }
    return feature_proto;
  }
  void createFeatureFile(
      const fs::path &dir, const std::string &name,
      const image_sequence_localizer::Feature &feature_proto) const {

    const fs::path filename = dir / name.c_str();
    std::fstream output(filename,
                        std::ios::out | std::ios::trunc | std::ios::binary);

    ASSERT_TRUE(feature_proto.SerializeToOstream(&output));
    output.close();
  }

  std::string createCostMatrixProto() {
    image_sequence_localizer::CostMatrix cost_matrix;
    for (double value : {1, 2, 3, 4, 5, 6}) {
      cost_matrix.add_values(value);
    }
    cost_matrix.set_cols(3);
    cost_matrix.set_rows(2);

    std::string cost_matrix_name =
        (tmp_dir / "test_cost_matrix.CostMatrix.pb").string();

    std::fstream out(cost_matrix_name,
                     std::ios::out | std::ios::trunc | std::ios::binary);
    cost_matrix.SerializeToOstream(&out);
    out.close();
    return cost_matrix_name;
  }

  void SetUp() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    tmp_dir = fs::temp_directory_path() / "features";
    fs::create_directories(tmp_dir);
    createFeatureFile(tmp_dir, "feature_0.Feature.pb",
                      createFeatureProto({0, 1, 2, 3}));
    createFeatureFile(tmp_dir, "feature_1.Feature.pb",
                      createFeatureProto({3, 4, 5, 6}));
    createFeatureFile(tmp_dir, "feature_2.Feature.pb",
                      createFeatureProto({3, 2, 1, 0}));
    createFeatureFile(tmp_dir, "feature_3.Feature.pb",
                      createFeatureProto({0, 1, 2.5, 3}));
  }
  void TearDown() { std::filesystem::remove_all(tmp_dir); }
  fs::path tmp_dir = "";
};

TEST_F(OnlineDatabaseTest, refSize) {
  OnlineDatabase database(/*queryFeaturesDir=*/tmp_dir,
                          /*refFeaturesDir=*/tmp_dir,
                          /*type=*/FeatureType::Cnn_Feature,
                          /*bufferSize=*/10);
  EXPECT_EQ(4, database.refSize());
}

TEST_F(OnlineDatabaseTest, InvalidConstructorParams) {
  ASSERT_DEATH(OnlineDatabase(
                   /*queryFeaturesDir=*/"",
                   /*refFeaturesDir=*/"",
                   /*type=*/FeatureType::Cnn_Feature,
                   /*bufferSize=*/10),
               "Feature directory does not exist.");

  ASSERT_DEATH(OnlineDatabase(tmp_dir, "", FeatureType::Cnn_Feature, 10),
               "Feature directory does not exist.");
  ASSERT_DEATH(OnlineDatabase(tmp_dir, tmp_dir, FeatureType::Cnn_Feature, -1),
               "Invalid featureBuffer size.");
}

TEST_F(OnlineDatabaseTest, NoCostMatrixFile) {

  ASSERT_DEATH(CostMatrixDatabase(tmp_dir, tmp_dir, tmp_dir,
                                  FeatureType::Cnn_Feature, 10),
               "Failed to parse cost_matrix file: ");
}

TEST_F(OnlineDatabaseTest, CostMatrixDatabaseConstructor) {
  std::string cost_matrix_name = createCostMatrixProto();
  CostMatrixDatabase database(cost_matrix_name,
                              /*queryFeaturesDir=*/tmp_dir,
                              /*refFeaturesDir=*/tmp_dir,
                              /*type=*/FeatureType::Cnn_Feature,
                              /*bufferSize=*/10);
  EXPECT_EQ(database.getCost(0, 0), 1);
  EXPECT_DOUBLE_EQ(database.getCost(0, 2), 1. / 3);
  EXPECT_DOUBLE_EQ(database.getCost(1, 0), 1. / 4);
  EXPECT_DOUBLE_EQ(database.getCost(1, 2), 1. / 6);
}

TEST_F(OnlineDatabaseTest, CostMatrixDatabaseRefSize) {
  std::string cost_matrix_name = createCostMatrixProto();
  CostMatrixDatabase database(cost_matrix_name,
                              /*queryFeaturesDir=*/tmp_dir,
                              /*refFeaturesDir=*/tmp_dir,
                              /*type=*/FeatureType::Cnn_Feature,
                              /*bufferSize=*/10);
  EXPECT_EQ(database.refSize(), 3);
  database.overrideCosts(
      {
          {1, 2, 3, 8},
          {4, 5, 6, 7},
      },
      2, 4);
  EXPECT_EQ(database.refSize(), 4);
}

TEST_F(OnlineDatabaseTest, CostMatrixDatabaseGetCost) {
  std::string cost_matrix_name = createCostMatrixProto();
  CostMatrixDatabase database(cost_matrix_name,
                              /*queryFeaturesDir=*/tmp_dir,
                              /*refFeaturesDir=*/tmp_dir,
                              /*type=*/FeatureType::Cnn_Feature,
                              /*bufferSize=*/10);
  EXPECT_EQ(database.getCost(0, 0), 1);
  EXPECT_DOUBLE_EQ(database.getCost(0, 2), 1. / 3);
  EXPECT_DOUBLE_EQ(database.getCost(1, 0), 1. / 4);
  EXPECT_DOUBLE_EQ(database.getCost(1, 2), 1. / 6);
  ASSERT_DEATH(database.getCost(-1, 0), "Invalid query index -1");
  ASSERT_DEATH(database.getCost(0, -1), "Invalid reference index -1");
  ASSERT_DEATH(database.getCost(3, 0), "Invalid query index 3");
  ASSERT_DEATH(database.getCost(0, 4), "Invalid reference index 4");
}