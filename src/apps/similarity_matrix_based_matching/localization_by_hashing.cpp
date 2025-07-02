// Created by O.Vysotska in 2023

#include "database/idatabase.h"
#include "database/list_dir.h"
#include "database/online_database.h"
#include "features/cnn_feature.h"
#include "features/ifeature.h"
#include "online_localizer/path_element.h"
#include "relocalizers/lsh_cv_hashing.h"
#include "tools/config_parser/config_parser.h"

#include <glog/logging.h>

#include <iostream>
#include <limits>
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
  LOG(INFO) << "===== Place recognition using pure Hashing algorithm ====\n";

  if (argc < 2) {
    printf("[ERROR] Not enough input parameters.\n");
    printf("Proper usage: ./localization_by_hashing config_file.yaml\n");
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
      /*similarityMatrixFile=*/parser.similarityMatrix);

  auto relocalizer = std::make_unique<loc::relocalizers::LshCvHashing>(
      /*onlineDatabase=*/database.get(),
      /*tableNum=*/1,
      /*keySize=*/12,
      /*multiProbeLevel=*/2);
  relocalizer->train(loadFeatures(parser.path2ref));

  loc::online_localizer::Matches matches;
  for (int queryId = 0; queryId < parser.querySize; ++queryId) {
    loc::online_localizer::PathElement pathElement;
    std::vector<int> candidates = relocalizer->getCandidates(queryId);
    double minCost = std::numeric_limits<double>::max();
    int refId = -1;
    for (int candidateId : candidates) {
      double cost = database->getCost(queryId, candidateId);
      if (cost < minCost) {
        minCost = cost;
        refId = candidateId;
      }
    }
    if (refId == -1) {
      // no candidates found
      matches.emplace_back(queryId, 0,
                           loc::online_localizer::NodeState::HIDDEN);
    } else {
      matches.emplace_back(queryId, refId,
                           loc::online_localizer::NodeState::REAL);
    }
  }
  loc::online_localizer::storeMatchesAsProto(matches, parser.matchingResult);

  LOG(INFO) << "Done.";
  return 0;
}
