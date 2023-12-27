/* By O. Vysotska in 2023 */

#include "database/cost_matrix.h"
#include "database/online_database.h"
#include "features/feature_factory.h"
#include "online_localizer/online_localizer.h"
#include "relocalizers/scan_context_relocalizer.h"
#include "successor_manager/successor_manager.h"
#include "tools/config_parser/config_parser.h"

#include <glog/logging.h>

#include <filesystem>
#include <memory>

namespace loc = localization;

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;
  LOG(INFO) << "===== Matching ScanContext sequences ====\n";

  if (argc < 2) {
    LOG(ERROR) << "Not enough input parameters.";
    LOG(INFO) << "Proper usage: ./scan_context_matching "
                 "config_file.yaml";
    exit(0);
  }

  std::string config_file = argv[1];
  ConfigParser parser;
  parser.parseYaml(config_file);
  parser.print();

  const auto database = std::make_unique<loc::database::OnlineDatabase>(
      parser.path2qu, parser.path2ref, loc::features::FeatureType::Scan_Context,
      parser.bufferSize, parser.costMatrix, false);

  const auto relocalizer =
      std::make_unique<loc::relocalizers::ScanContextRelocalizer>(
          parser.path2ref, database.get());

  const auto successorManager =
      std::make_unique<loc::successor_manager::SuccessorManager>(
          database.get(), relocalizer.get(), parser.fanOut);
  loc::online_localizer::OnlineLocalizer localizer{
      successorManager.get(), parser.expansionRate, parser.nonMatchCost};
  const loc::online_localizer::Matches imageMatches =
      localizer.findMatchesTill(parser.querySize);
  loc::online_localizer::storeMatchesAsProto(imageMatches,
                                             parser.matchingResult);

  LOG(INFO) << "Done.";
  return 0;
}