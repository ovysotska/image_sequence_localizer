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

#include "lsh_cv_hashing.h"

#include "database/list_dir.h"
#include "tools/timer/timer.h"

#include <glog/logging.h>

const int kMaxCandidateNum = 5;

LshCvHashing::LshCvHashing(OnlineDatabase *database, int tableNum, int keySize,
                           int multiProbeLevel) {
  CHECK(database) << "Database is not set\n";
  database_ = database;
  indexParam_ =
      new cv::flann::LshIndexParams(tableNum, keySize, multiProbeLevel);
}

void LshCvHashing::train(
    const std::vector<std::unique_ptr<iFeature>> &features) {
  matcherPtr_ =
      cv::Ptr<cv::FlannBasedMatcher>(new cv::FlannBasedMatcher(indexParam_));
  // transform to cv::Mat array of arrays
  cv::Mat matFeatures(features.size(), features[0]->bits.size(), CV_8UC1);
  for (int f = 0; f < features.size(); ++f) {
    for (int d = 0; d < features[f]->bits.size(); ++d) {
      matFeatures.at<uchar>(f, d) = features[f]->bits[d];
    }
  }

  LOG(INFO) << "Features were converted to Mat type " << matFeatures.type();
  matcherPtr_->add(matFeatures);
  matcherPtr_->train();

  LOG(INFO) << "Training completed";
}

std::vector<int> LshCvHashing::hashFeature(const iFeature &feature) {
  std::vector<std::vector<cv::DMatch>> matches;
  cv::Mat featureCV(1, feature.bits.size(), CV_8UC1);
  for (int i = 0; i < feature.bits.size(); ++i) {
    featureCV.at<uchar>(0, i) = feature.bits[i];
  }
  Timer timer;
  timer.start();
  matcherPtr_->knnMatch(featureCV, matches, kMaxCandidateNum);
  timer.stop();
  LOG(INFO) << "Time to extract neighbours:";
  timer.print_elapsed_time(TimeExt::MicroSec);

  std::vector<int> matchedIds;
  for (int k = 0; k < matches.size(); ++k) {
    for (const auto &match : matches[k]) {
      matchedIds.push_back(match.trainIdx);
    }
  }
  return matchedIds;
}
void LshCvHashing::saveHashes() {
  cv::FileStorage fs("trained_trees.yaml", cv::FileStorage::WRITE);
  matcherPtr_->write(fs);

  cv::FileStorage fsRead("trained_trees.yaml", cv::FileStorage::READ);
  cv::FileNode node = fsRead["indexParams"];
  matcherPtr_->read(node);
}

std::vector<int> LshCvHashing::getCandidates(int quId) {
  LOG(INFO) << "Getting candidates for a query image";
  const auto &feature = database_->getQueryFeature(quId);
  Timer timer;
  timer.start();

  std::vector<int> candidates;
  candidates = hashFeature(feature);

  timer.stop();
  LOG(INFO) << "Hash retrieval time";
  timer.print_elapsed_time(TimeExt::MSec);
  LOG(INFO) << "Candidates size: " << candidates.size();
  return candidates;
}
