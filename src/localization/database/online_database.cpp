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

#include "database/online_database.h"
#include "database/list_dir.h"
#include "features/feature_buffer.h"
#include "tools/timer/timer.h"
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include <glog/logging.h>

using std::string;
using std::vector;

/**
 * @brief      Gets the match cost.
 *
 * @param[in]  quId   The qu identifier
 * @param[in]  refId  The reference identifier
 *
 * @return     The match cost. return -1 if cost is not found
 */
double MatchMap::getMatchCost(int quId, int refId) {
  auto row_iter = _matches.find(quId);
  if (row_iter != _matches.end()) {
    // found
    auto el_iter = row_iter->second.find(refId);
    if (el_iter != row_iter->second.end()) {
      return el_iter->second;
    }
  }
  return -1.0;
}

double OnlineDatabase::getCost(int quId, int refId) {
  double cost = _matchMap.getMatchCost(quId, refId);
  if (cost > -1.0) {
    // cost was found
    return cost;
  }
  // printf(
  //     "[DEBUG][OnlineDatabase] Matching costs for features %d - %d will be "
  //     "computed.\n",
  //     quId, refId);
  cost = computeMatchCost(quId, refId);
  _matchMap.addMatchCost(quId, refId, cost);
  return cost;
}

OnlineDatabase::OnlineDatabase(const std::string &queryFeaturesDir,
                               const std::string &refFeaturesDir,
                               const FeatureType &type, int bufferSize) {
  quFeaturesNames_ = listProtoDir(queryFeaturesDir, ".Feature");
  refFeaturesNames_ = listProtoDir(refFeaturesDir, ".Feature");
  refBuffer_ = std::make_unique<FeatureBuffer>(bufferSize);
  queryBuffer_ = std::make_unique<FeatureBuffer>(bufferSize);
  featureType_ = type;

  LOG_IF(FATAL, quFeaturesNames_.empty()) << "Query features are not set.";
  LOG_IF(FATAL, refFeaturesNames_.empty()) << "Reference features are not set.";
}

// use for tests / visualization only
const MatchMap &OnlineDatabase::getMatchMap() const { return _matchMap; }

double OnlineDatabase::computeMatchCost(int quId, int refId) {
  if (quId < 0 || quId >= (int)quFeaturesNames_.size()) {
    LOG(FATAL) << "Query feature " << quId << " is out of range";
  }
  if (refId < 0 || refId >= (int)refFeaturesNames_.size()) {
    LOG(FATAL) << "Reference feature " << refId << " is out of range";
  }

  iFeature::ConstPtr quFeaturePtr = nullptr;
  iFeature::ConstPtr refFeaturePtr = nullptr;
  if (queryBuffer_->inBuffer(quId)) {
    quFeaturePtr = queryBuffer_->getFeature(quId);
  } else {
    // We cannot directly set const pointers, so set them through a proxy.
    // TODO(olga) Some madness is happening here.
    auto tempFeaturePtr = createFeature(featureType_, quFeaturesNames_[quId]);
    quFeaturePtr = tempFeaturePtr;
    queryBuffer_->addFeature(quId, quFeaturePtr);
  }

  if (refBuffer_->inBuffer(refId)) {
    refFeaturePtr = refBuffer_->getFeature(refId);
  } else {
    auto tempFeaturePtr = createFeature(featureType_, refFeaturesNames_[refId]);
    refFeaturePtr = tempFeaturePtr;
    refBuffer_->addFeature(refId, refFeaturePtr);
  }

  double score = quFeaturePtr->computeSimilarityScore(refFeaturePtr);
  return quFeaturePtr->score2cost(score);
}

std::string OnlineDatabase::getQuFeatureName(int id) const {
  if (id < 0 || id >= (int)quFeaturesNames_.size()) {
    printf("[WARNING][OnlineDatabase] No such feature exists\n");
    return "";
  }
  return quFeaturesNames_[id];
}

std::string OnlineDatabase::getRefFeatureName(int id) const {
  if (id < 0 || id >= (int)refFeaturesNames_.size()) {
    printf("[WARNING][OnlineDatabase] No such feature exists\n");
    return "";
  }
  return refFeaturesNames_[id];
}

iFeature::ConstPtr OnlineDatabase::getQueryFeature(int quId) {
  iFeature::ConstPtr quFeaturePtr = nullptr;
  if (queryBuffer_->inBuffer(quId)) {
    quFeaturePtr = queryBuffer_->getFeature(quId);
  } else {
    // We cannot directly set const pointers, so set them through a proxy.
    auto tempFeaturePtr = createFeature(featureType_, quFeaturesNames_[quId]);
    quFeaturePtr = tempFeaturePtr;
    queryBuffer_->addFeature(quId, quFeaturePtr);
  }
  return quFeaturePtr;
}
