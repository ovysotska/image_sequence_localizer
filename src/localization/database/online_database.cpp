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

#include "database/online_database.h"
#include "database/list_dir.h"
#include "features/feature_buffer.h"
#include "features/ifeature.h"

#include <glog/logging.h>

#include <fstream>
#include <limits>
#include <memory>
#include <string>

namespace {
const iFeature&
addFeatureIfNeeded(FeatureBuffer &featureBuffer,
                   const std::vector<std::string> &featureNames,
                   FeatureType type, int featureId) {
  if (featureBuffer.inBuffer(featureId)) {
    return featureBuffer.getFeature(featureId);
  }
  featureBuffer.addFeature(featureId, createFeature(type, featureNames[featureId]));
  return featureBuffer.getFeature(featureId);
}
} // namespace

OnlineDatabase::OnlineDatabase(const std::string &queryFeaturesDir,
                               const std::string &refFeaturesDir,
                               FeatureType type, int bufferSize)
    : quFeaturesNames_{listProtoDir(queryFeaturesDir, ".Feature")},
      refFeaturesNames_{listProtoDir(refFeaturesDir, ".Feature")},
      featureType_{type}, refBuffer_{std::make_unique<FeatureBuffer>(
                              bufferSize)},
      queryBuffer_{std::make_unique<FeatureBuffer>(bufferSize)} {
  LOG_IF(FATAL, quFeaturesNames_.empty()) << "Query features are not set.";
  LOG_IF(FATAL, refFeaturesNames_.empty()) << "Reference features are not set.";
}

double OnlineDatabase::computeMatchingCost(int quId, int refId) {
  CHECK(quId >= 0 && quId < (int)quFeaturesNames_.size())
      << "Query feature " << quId << " is out of range";
  CHECK(refId >= 0 && refId < (int)refFeaturesNames_.size())
      << "Reference feature " << refId << " is out of range";

  const auto &quFeature =
      addFeatureIfNeeded(*queryBuffer_, quFeaturesNames_, featureType_, quId);
  const auto &refFeature =
      addFeatureIfNeeded(*refBuffer_, refFeaturesNames_, featureType_, refId);

  return quFeature.score2cost(
      quFeature.computeSimilarityScore(refFeature));
}

double OnlineDatabase::getCost(int quId, int refId) {
  // Check if the cost was computed before.
  auto rowIter = costs_.find(quId);
  if (rowIter != costs_.end()) {
    auto elementIter = rowIter->second.find(refId);
    if (elementIter != rowIter->second.end()) {
      return elementIter->second;
    }
  }
  const double cost = computeMatchingCost(quId, refId);
  costs_[quId][refId] = cost;
  return cost;
}

const iFeature& OnlineDatabase::getQueryFeature(int quId) {
  return addFeatureIfNeeded(*queryBuffer_, quFeaturesNames_, featureType_,
                            quId);
}
