/* By O. Vysotska in 2023 */

#include "database/similarity_matrix_database.h"
#include "database/idatabase.h"
#include "online_localizer/online_localizer.h"
#include "online_localizer/path_element.h"
#include "relocalizers/default_relocalizer.h"
#include "successor_manager/successor_manager.h"
#include "tools/config_parser/config_parser.h"

#include <glog/logging.h>

namespace loc = localization;

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;
  LOG(INFO)
      << "===== Online place recognition cost matrix based NO HASHING ====\n";

  if (argc < 2) {
    LOG(ERROR) << "Not enough input parameters.";
    LOG(INFO) << "Proper usage: ./cost_matrix_based_matching_no_hashing "
                 "config_file.yaml";
    exit(0);
  }

  std::string config_file = argv[1];
  ConfigParser parser;
  parser.parseYaml(config_file);
  parser.print();

  const auto database =
      std::make_unique<loc::database::SimilarityMatrixDatabase>(parser.similarityMatrix);

  const auto relocalizer =
      std::make_unique<loc::relocalizers::DefaultRelocalizer>(
          parser.fanOut, database->refSize());

  const auto successorManager =
      std::make_unique<loc::successor_manager::SuccessorManager>(
          database.get(), relocalizer.get(), parser.fanOut);
  loc::online_localizer::OnlineLocalizer localizer{
      successorManager.get(), parser.expansionRate, parser.matchingThreshold};
  const loc::online_localizer::Matches imageMatches =
      localizer.findMatchesTill(parser.querySize);
  loc::online_localizer::storeMatchesAsProto(imageMatches,
                                             parser.matchingResult);

  LOG(INFO) << "Done.";
  return 0;
}