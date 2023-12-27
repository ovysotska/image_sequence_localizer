#include "relocalizers/scan_context_relocalizer.h"
#include "database/list_dir.h"
#include "features/feature_factory.h"
#include "features/scan_context.h"

#include <glog/logging.h>
#include <opencv2/core/eigen.hpp>

#include <algorithm>
#include <opencv2/flann/miniflann.hpp>
#include <set>

namespace localization::relocalizers {

constexpr int kMaxCandidateNum = 5;

ScanContextRelocalizer::ScanContextRelocalizer(
    const std::string &features_dir, database::OnlineDatabase *database) {
  CHECK(!features_dir.empty()) << "Feature directory is not set";
  CHECK(database) << "Online database is not set";

  database_ = database;

  std::vector<std::string> featureFiles =
      database::listProtoDir(features_dir, ".Feature");

  LOG(INFO) << "I have found " << featureFiles.size() << " feature files";

  ringKeys_.reserve(featureFiles.size());
  for (const auto &file : featureFiles) {
    ringKeys_.push_back(features::ScanContext(file).computeRingKeys());
  }

  // indexParam_ = new cv::flann::LshIndexParams(25, 20, 2);
  indexParam_ = new cv::flann::KDTreeIndexParams(25);
  matcherPtr_ =
      cv::Ptr<cv::FlannBasedMatcher>(new cv::FlannBasedMatcher(indexParam_));

  LOG(INFO) << "Starting adding features for hashing";
  cv::Mat features;
  // Turn ringKeys to CV::Mat objects
  for (const auto &featureRingKeys : ringKeys_) {
    for (const auto &shift : featureRingKeys) {
      cv::Mat shiftRingKeyMat;
      eigen2cv(shift, shiftRingKeyMat);
      shiftRingKeyMat.convertTo(shiftRingKeyMat, CV_32F);
      features.push_back(shiftRingKeyMat.t());
    }
  }
  matcherPtr_->add(features);

  LOG(INFO) << "Started training...";
  matcherPtr_->train();
  LOG(INFO) << "Finished traning.";
}

std::vector<int> ScanContextRelocalizer::getBruteForceCandidates(
    const features::ScanContext &queryScanContext) const {
  std::vector<std::pair<int, double>> distancesPerFeature;
  distancesPerFeature.reserve(ringKeys_.size());
  for (int refId = 0; refId < ringKeys_.size(); ++refId) {
    const auto dist = features::distanceBetweenRingKeys(
        queryScanContext.computeRingKeys(), ringKeys_[refId]);
    distancesPerFeature.push_back({refId, dist});
  }
  std::sort(distancesPerFeature.begin(), distancesPerFeature.end(),
            [](const auto &left, const auto &right) {
              return left.second < right.second;
            });

  const int kNumOfCandidates = 5;
  std::vector<int> candidates;
  for (int k = 0;
       k < std::min(kNumOfCandidates, (int)distancesPerFeature.size()); ++k) {
    candidates.push_back(distancesPerFeature[k].first);
  }
  return candidates;
}

std::vector<int> ScanContextRelocalizer::getCandidates(int quId) {
  const features::iFeature &queryFeature = database_->getQueryFeature(quId);
  CHECK(queryFeature.type == "ScanContext") << "Not a ScanContext feature";
  const features::ScanContext &queryScanContext =
      static_cast<const features::ScanContext &>(queryFeature);
  const auto queryRingKeys = queryScanContext.computeRingKeys();

  // TODO (olga): This can be done without the for loop.
  std::set<int> matchedIds;
  for (const auto &shift : queryRingKeys) {
    std::vector<std::vector<cv::DMatch>> matches;
    cv::Mat shiftRingKeyMat;
    eigen2cv(shift, shiftRingKeyMat);
    shiftRingKeyMat.convertTo(shiftRingKeyMat, CV_32F);
    matcherPtr_->knnMatch(shiftRingKeyMat.t(), matches, kMaxCandidateNum);

    for (int k = 0; k < matches.size(); ++k) {
      for (const auto &match : matches[k]) {
        matchedIds.insert(
            match.trainIdx /
            queryRingKeys
                .size()); // This only considers that the there are the same
                          // number of shifts in the trained dataset
      }
    }
  }
  return {matchedIds.begin(), matchedIds.end()};
}
} // namespace localization::relocalizers