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


#include "cnn_feature.h"
#include "features/ifeature.h"
#include "localization_protos.pb.h"

#include <math.h>
#include <algorithm>
#include <fstream>
#include <limits>
#include <numeric>

#include <glog/logging.h>


CnnFeature::CnnFeature(const std::string &filename) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  image_sequence_localizer::Feature feature_proto;
  std::fstream input(filename, std::ios::in | std::ios::binary);
  if (!feature_proto.ParseFromIstream(&input)) {
    LOG(FATAL) << "Failed to parse feature_proto file: " << filename;
  }

  for (int idx = 0; idx < feature_proto.values_size(); ++idx) {
    dimensions.push_back(feature_proto.values(idx));
  }
  binarize();
  type = "CnnFeature";
}

void CnnFeature::binarize() {
  bits.clear();
  int des_max = 255, des_min = 0; // des_ - desired params
  auto min_maxEl = std::minmax_element(dimensions.begin(), dimensions.end());

  double d_min, d_max;
  d_min = *min_maxEl.first;
  d_max = *min_maxEl.second;
  double tmp = (des_max - des_min) / (d_max - d_min);

  bits.resize(dimensions.size(), 0);

  int thresh = des_max / 2;
  for (int f = 0; f < dimensions.size(); ++f) {
    int d_int = (dimensions[f] - d_min) * tmp + des_min;
    bool value = d_int < thresh ? 0 : 1;
    bits[f] = value;
  }
}

double CnnFeature::computeSimilarityScore(const iFeature &rhs) const {
  CHECK(this->type == rhs.type) <<  "Features are not the same type";
  double norm_qr =
      sqrt(std::inner_product(dimensions.begin(), dimensions.end(), dimensions.begin(), 0.0L));
  double norm_db =
      sqrt(std::inner_product(rhs.dimensions.begin(), rhs.dimensions.end(),
                              rhs.dimensions.begin(), 0.0L));
  double prod_qr_db = std::inner_product(
      rhs.dimensions.begin(), rhs.dimensions.end(), dimensions.begin(), 0.0L);
  double cos_dist = prod_qr_db / (norm_qr * norm_db);
  return cos_dist;
}

double CnnFeature::score2cost(double score) const {
  double cost;
  if (score < 1e-09) {
    cost = std::numeric_limits<double>::max();
    printf("[INFO] The cost of comparing two images is suspiciously small.\n");
  } else {
    cost = 1. / score;
  }
  return cost;
}
