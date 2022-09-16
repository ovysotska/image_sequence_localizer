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

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace fs = std::filesystem;
namespace localizer = image_sequence_localizer;

TEST(MatchMap, getCost) {
  MatchMap matchMap;
  matchMap.addMatchCost(1, 2, 3.0);
  double cost = matchMap.getMatchCost(1, 2);
  EXPECT_NEAR(cost, 3.0, 1e-09);
  cost = matchMap.getMatchCost(1, 3);
  EXPECT_NEAR(cost, -1.0, 1e-09);
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

    if (!feature_proto.SerializeToOstream(&output)) {
      std::cerr << "Failed to write feature proto." << std::endl;
      return;
    }
  }
  void SetUp() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    tmp_dir = fs::temp_directory_path();
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
  fs::path tmp_dir = "";
};

TEST_F(OnlineDatabaseTest, refSize) {
  OnlineDatabase database;
  database.setRefFeaturesFolder(tmp_dir);
  EXPECT_EQ(4, database.refSize());
}

// This should test that proper features get pulled by quID, refID rather than
// actual similarity score TEST_F(OnlineDatabaseTest, getCost) { OnlineDatabase
// database; double OnlineDatabase::computeMatchCost(int quId, int refId)
}

// Everything should be done through the cost nmatrxi
// TEST_F(CostMatrixdatabaseTest, refSize)
// TEST_F(CostMatrixdatabaseTest, loadFromProto)
// TEST_F(CostMatrixdatabaseTest, getCosts)
