
#include "database/cost_matrix_database.h"
#include "database/idatabase.h"
#include "online_localizer/online_localizer.h"
#include "successor_manager/successor_manager.h"

#include "relocalizers/default_relocalizer.h"
#include "tools/config_parser/config_parser.h"
#include <glog/logging.h>

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;
  LOG(INFO)
      << "===== Online place recognition cost matrix based NO HASHING ====\n";

  if (argc < 2) {
    printf("[ERROR] Not enough input parameters.\n");
    printf("Proper usage: ./cost_matrix_based_matching_no_hashing "
           "config_file.yaml\n");
    exit(0);
  }

  std::string config_file = argv[1];
  ConfigParser parser;
  parser.parseYaml(config_file);
  parser.print();

  auto database = std::make_unique<CostMatrixDatabase>(
      /*costMatrixFile=*/parser.costMatrix);

  // initialize Relocalizer.
  auto relocalizer =
      std::make_unique<DefaultRelocalizer>(parser.fanOut, database->refSize());

  std::unique_ptr<SuccessorManager> successorManager =
      std::make_unique<SuccessorManager>(database.get(), relocalizer.get(),
                                         parser.fanOut);
  online_localizer::OnlineLocalizer localizer{
      successorManager.get(), parser.expansionRate, parser.nonMatchCost};
  const online_localizer::Matches imageMatches =
      localizer.findMatchesTill(parser.querySize);
  online_localizer::storeMatchesAsProto(imageMatches, parser.matchingResult);

  printf("Done.\n");
  return 0;
}