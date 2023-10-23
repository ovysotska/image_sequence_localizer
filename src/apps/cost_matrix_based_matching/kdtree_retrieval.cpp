// Created by O. Vysotska in 2023

#include "database/idatabase.h"
#include "database/list_dir.h"
#include "database/online_database.h"
#include "features/cnn_feature.h"
#include "features/ifeature.h"

#include "online_localizer/online_localizer.h"
#include "online_localizer/path_element.h"
#include "relocalizers/lsh_cv_hashing.h"
#include "successor_manager/node.h"
#include "tools/config_parser/config_parser.h"

#include <glog/logging.h>

#include <iostream>
#include <memory>
#include <string>

namespace loc = localization;

std::vector<std::unique_ptr<loc::features::iFeature>>
loadFeatures(const std::string &path2folder) {
  LOG(INFO) << "Loading the features to hash with LSH.";
  std::vector<std::string> featureNames =
      loc::database::listProtoDir(path2folder, ".Feature");
  std::vector<std::unique_ptr<loc::features::iFeature>> features;

  for (size_t i = 0; i < featureNames.size(); ++i) {
    features.emplace_back(
        std::make_unique<loc::features::CnnFeature>(featureNames[i]));
    fprintf(stderr, ".");
  }
  fprintf(stderr, "\n");
  LOG(INFO) << "Features were loaded and binarized";
  return features;
}

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;
  LOG(INFO) << "===== Pure KDtree retrieval ====\n";

  if (argc < 2) {
    printf("[ERROR] Not enough input parameters.\n");
    printf("Proper usage: ./kdtree_retrieval config_file.yaml\n");
    exit(0);
  }

  std::string config_file = argv[1];
  ConfigParser parser;
  parser.parseYaml(config_file);
  parser.print();

  const auto database = std::make_unique<loc::database::OnlineDatabase>(
      /*queryFeaturesDir=*/parser.path2qu,
      /*refFeaturesDir=*/parser.path2ref,
      /*type=*/loc::features::FeatureType::Cnn_Feature,
      /*bufferSize=*/parser.bufferSize,
      /*costMatrixFile=*/parser.costMatrix);

  auto retriever = std::make_unique<loc::relocalizers::LshCvHashing>(
      /*onlineDatabase=*/database.get(),
      /*tableNum=*/1,
      /*keySize=*/12,
      /*multiProbeLevel=*/2);
  retriever->train(loadFeatures(parser.path2ref));

  std::vector<PathElement> imageMatches;
  for (int queryId = 0; queryId < parser.querySize; queryId++) {
    std::vector<int> candidates = retriever->getCandidates(queryId);
    if (candidates.size() > 0) {
      imageMatches.emplace_back(queryId, candidates[0], NodeState::REAL);
    }
  }

  loc::online_localizer::storeMatchesAsProto(imageMatches,
                                             parser.matchingResult);

  printf("Done.\n");
  return 0;
}
