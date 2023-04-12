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

#include "features/cnn_feature.h"
#include "features/feature_buffer.h"
#include "features/ifeature.h"

#include "gtest/gtest.h"

#include <iostream>
#include <string>
#include <vector>

namespace test {

namespace loc_features = localization::features;

class DummyFeature : public loc_features::iFeature {
public:
  DummyFeature(const std::vector<double> &values) { dimensions = values; }
  double computeSimilarityScore(const iFeature &rhs) const override {
    return 0.0;
  }
  double score2cost(double score) const override { return 0.0; }
};

TEST(featureBuffer, addFeature) {
  loc_features::FeatureBuffer buffer(2);

  buffer.addFeature(0,
                    std::make_unique<DummyFeature>(std::vector{1.0, 2.0, 3.0}));
  buffer.addFeature(1,
                    std::make_unique<DummyFeature>(std::vector{4.0, 5.0, 6.0}));
  EXPECT_EQ(buffer.featureMap.size(), 2);
  EXPECT_EQ(buffer.ids[0], 0);
  EXPECT_EQ(buffer.ids[1], 1);

  buffer.addFeature(3,
                    std::make_unique<DummyFeature>(std::vector{7.0, 8.0, 9.0}));
  EXPECT_EQ(buffer.featureMap.size(), 2);
  EXPECT_EQ(buffer.ids[0], 1);
  EXPECT_EQ(buffer.ids[1], 3);
}

TEST(featureBuffer, inBuffer) {
  loc_features::FeatureBuffer buffer(4);
  EXPECT_FALSE(buffer.inBuffer(4));

  buffer.addFeature(1,
                    std::make_unique<DummyFeature>(std::vector{4.0, 5.0, 6.0}));
  EXPECT_FALSE(buffer.inBuffer(4));
  EXPECT_TRUE(buffer.inBuffer(1));
}

TEST(featureBuffer, getFeature) {
  loc_features::FeatureBuffer buffer(4);
  buffer.addFeature(1,
                    std::make_unique<DummyFeature>(std::vector{4.0, 5.0, 6.0}));
  const auto &feature = buffer.getFeature(1);
  EXPECT_DOUBLE_EQ(feature.dimensions[0], 4);
  EXPECT_DOUBLE_EQ(feature.dimensions[1], 5);
  EXPECT_DOUBLE_EQ(feature.dimensions[2], 6);
}
} // namespace test
