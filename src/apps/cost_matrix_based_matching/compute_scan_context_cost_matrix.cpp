/* By O. Vysotska in 2023 */

#include "database/cost_matrix.h"
#include "database/cost_matrix_database.h"
#include "features/feature_factory.h"
#include "tools/config_parser/config_parser.h"

#include <glog/logging.h>

#include <memory>

int main(int argc, char *argv[]) {
  // TODO(olga) Add gflags support.
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;
  LOG(INFO) << "===== Compute Cost Matrix for Scan Context ====\n";

  if (argc < 4) {
    LOG(ERROR) << "Not enough input parameters.";
    LOG(INFO) << "Proper usage: ./compute_scan_context_cost_matrix "
                 "queryFeaturesDir referenceFeaturesDir outputFilename";
    exit(0);
  }

  std::string queryFeaturesDir = argv[1];
  std::string refFeaturesDir = argv[2];
  std::string outputFilename = argv[3];

  const localization::database::CostMatrix costMatrix(
      queryFeaturesDir, refFeaturesDir, localization::features::Scan_Context);
  costMatrix.storeToProto(outputFilename);

  LOG(INFO) << "Cost matrix is saved to" << outputFilename;
  return 0;
}