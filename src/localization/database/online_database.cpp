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
#include <fstream>
#include <limits>
#include <memory>
#include <string>

#include <glog/logging.h>

using std::string;

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

double OnlineDatabase::computeMatchingCost(int quId, int refId) {
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

double OnlineDatabase::getCost(int quId, int refId) {
  // Check if the cost was computed before.
  auto rowIter = costs_.find(quId);
  if (rowIter != costs_.end()) {
    auto elementIter = rowIter->second.find(refId);
    if (elementIter != rowIter->second.end()) {
      return elementIter->second;
    }
  }
  double cost = computeMatchingCost(quId, refId);
  costs_[quId][refId] = cost;
  return cost;
}

std::string OnlineDatabase::getQuFeatureName(int id) const {
  if (id < 0 || id >= (int)quFeaturesNames_.size()) {
    LOG(WARNING) << "No query feature with id " << id << " exists.";
    return "";
  }
  return quFeaturesNames_[id];
}

std::string OnlineDatabase::getRefFeatureName(int id) const {
  if (id < 0 || id >= (int)refFeaturesNames_.size()) {
    LOG(WARNING) << "No reference feature with id " << id << " exists.";
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
