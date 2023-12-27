#ifndef LOCALIZATION_RELOCALIZERS_SCAN_CONTEXT_RELOCALIZER_H_
#define LOCALIZATION_RELOCALIZERS_SCAN_CONTEXT_RELOCALIZER_H_

#include "database/online_database.h"
#include "features/scan_context.h"
#include "relocalizers/irelocalizer.h"

#include "opencv2/features2d/features2d.hpp"
#include <Eigen/Dense>

#include <string>
#include <vector>

namespace localization::relocalizers {

class ScanContextRelocalizer : public iRelocalizer {
public:
  ScanContextRelocalizer(const std::string &features_dir,
                         database::OnlineDatabase *database);
  std::vector<int> getCandidates(int quId) override;

private:
  std::vector<int>
  getBruteForceCandidates(const features::ScanContext &queryScanContext) const;
  std::vector<std::vector<Eigen::VectorXd>> ringKeys_;
  database::OnlineDatabase *database_ = nullptr;
  cv::Ptr<cv::FlannBasedMatcher> matcherPtr_;
  cv::Ptr<cv::flann::IndexParams> indexParam_;
};
} // namespace localization::relocalizers
#endif // LOCALIZATION_RELOCALIZERS_SCAN_CONTEXT_RELOCALIZER_H_