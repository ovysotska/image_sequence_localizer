#ifndef LOCALIZATION_FEATURES_SCAN_CONTEXT_H_
#define LOCALIZATION_FEATURES_SCAN_CONTEXT_H_

#include "features/ifeature.h"

#include <Eigen/Dense>

#include <vector>

namespace localization::features {

double computeDistanceBetweenGrids(const Eigen::MatrixXd &lhs,
                                   const Eigen::MatrixXd &rhs);
Eigen::VectorXd computeRingKey(const Eigen::MatrixXd &grid);

double distanceBetweenRingKeys(const std::vector<Eigen::VectorXd> &rhs,
                               const std::vector<Eigen::VectorXd> &lhs);

class ScanContext : public iFeature {
public:
  explicit ScanContext(const std::string &filename);
  double computeSimilarityScore(const iFeature &rhs) const override;
  double score2cost(double score) const override;
  std::vector<Eigen::VectorXd> computeRingKeys() const;

  std::vector<Eigen::MatrixXd> grids;
  // TODO (olga): Probably can store RingKeys
};
} // namespace localization::features

#endif // LOCALIZATION_FEATURES_SCAN_CONTEXT_H_