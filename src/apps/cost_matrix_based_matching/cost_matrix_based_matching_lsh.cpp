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

#include <iostream>
#include <memory>
#include <string>

#include "database/cost_matrix_database.h"
#include "database/idatabase.h"
#include "database/list_dir.h"
#include "database/online_database.h"
#include "features/cnn_feature.h"
#include "features/ifeature.h"
#include "online_localizer/ilocvisualizer.h"
#include "online_localizer/online_localizer.h"
#include "relocalizers/lsh_cv_hashing.h"
#include "successor_manager/successor_manager.h"
#include "tools/config_parser/config_parser.h"

#include <glog/logging.h>


std::vector<std::unique_ptr<iFeature>> loadFeatures(const std::string &path2folder) {
  LOG(INFO) << "Loading the features to hash with LSH.";
  std::vector<std::string> featureNames = listProtoDir(path2folder, ".Feature");
  std::vector<std::unique_ptr<iFeature>> features;

  for (size_t i = 0; i < featureNames.size(); ++i) {
    std::unique_ptr<iFeature> feature = std::make_unique<CnnFeature>(featureNames[i]);
    features.push_back(std::move(feature));
    fprintf(stderr, ".");
  }
  fprintf(stderr, "\n");
  LOG(INFO) << "Features were loaded and binarized";
  return features;
}

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;
  LOG(INFO) << "===== Online place recognition cost matrix based LSH ====\n";

  if (argc < 2) {
    printf("[ERROR] Not enough input parameters.\n");
    printf("Proper usage: ./cost_matrix_based_matching_lsh config_file.yaml\n");
    exit(0);
  }

  std::string config_file = argv[1];
  ConfigParser parser;
  parser.parseYaml(config_file);
  parser.print();

  std::unique_ptr<OnlineDatabase> database =
      std::make_unique<CostMatrixDatabase>(
          /*costMatrixFile=*/parser.costMatrix,
          /*queryFeaturesDir=*/parser.path2qu,
          /*refFeaturesDir=*/parser.path2ref, /*type=*/FeatureType::Cnn_Feature,
          /*bufferSize=*/parser.bufferSize);

  // initialize Relocalizer
  auto relocalizerPtr = LshCvHashing::Ptr(new LshCvHashing);
  relocalizerPtr->setParams(1, 12, 2);
  relocalizerPtr->setDatabase(database.get());
  relocalizerPtr->train(loadFeatures(parser.path2ref));

  // initialize SuccessorManager
  auto successorManagerPtr = SuccessorManager::Ptr(new SuccessorManager);
  successorManagerPtr->setFanOut(parser.fanOut);
  successorManagerPtr->setDatabase(database.get());
  successorManagerPtr->setRelocalizer(relocalizerPtr);

  // create localizer and run it
  OnlineLocalizer localizer;
  localizer.setQuerySize(parser.querySize);
  localizer.setSuccessorManager(successorManagerPtr);
  localizer.setExpansionRate(parser.expansionRate);
  localizer.setNonMatchingCost(parser.nonMatchCost);
  localizer.run();

  localizer.printPath(parser.matchingResult);
  printf("Done.\n");
  return 0;
}
