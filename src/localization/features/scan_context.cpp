#include "scan_context.h"
#include "localization_protos.pb.h"

#include <Eigen/src/Core/Matrix.h>
#include <algorithm>
#include <glog/logging.h>

#include <algorithm>
#include <fstream>
#include <limits>
#include <string>

namespace localization::features {

double cosine_distance(const Eigen::VectorXd &lhs, const Eigen::VectorXd &rhs) {
  if (std::abs(lhs.norm()) < 1e-09 || std::abs(rhs.norm()) < 1e-09) {
    return 0;
  }
  return 1 - lhs.dot(rhs) / (lhs.norm() * rhs.norm());
}

double computeDistanceBetweenGrids(const Eigen::MatrixXd &lhs,
                                   const Eigen::MatrixXd &rhs) {
  CHECK(lhs.cols() > 0) << "Invalid grid size.";
  CHECK(lhs.cols() == rhs.cols()) << "The number of columns is not the same.";
  CHECK(lhs.rows() == rhs.rows()) << "The number of rows is not the same.";

  Eigen::VectorXd columnErrors(lhs.cols());
  for (int c = 0; c < lhs.cols(); ++c) {
    columnErrors[c] = cosine_distance(lhs.col(c), rhs.col(c));
  }
  return columnErrors.sum() / lhs.cols();
}

double distanceBetweenRingKeys(const std::vector<Eigen::VectorXd> &rhs,
                               const std::vector<Eigen::VectorXd> &lhs) {
  double minDistance = std::numeric_limits<double>::max();
  for (int rightRingKey = 0; rightRingKey < rhs.size(); ++rightRingKey) {
    for (int leftRingKey = 0; leftRingKey < lhs.size(); ++leftRingKey) {
      double dist = cosine_distance(rhs[rightRingKey], lhs[leftRingKey]);
      if (dist < minDistance) {
        minDistance = dist;
      }
    }
  }
  return minDistance;
}

Eigen::VectorXd computeRingKey(const Eigen::MatrixXd &grid) {
  Eigen::VectorXd ringKey{grid.rows()};
  for (int r = 0; r < grid.rows(); ++r) {
    Eigen::VectorXd row = grid.row(r);
    ringKey[r] = (row.array() > 0).count() / static_cast<double>(grid.cols());
  }
  return ringKey;
}

ScanContext::ScanContext(const std::string &filename) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  image_sequence_localizer::ScanContext scanContextProto;
  std::fstream input(filename, std::ios::in | std::ios::binary);
  if (!scanContextProto.ParseFromIstream(&input)) {
    LOG(FATAL) << "Failed to parse feature_proto file: " << filename;
  }

  for (int shift = 0; shift < scanContextProto.grids_size(); ++shift) {
    const int rows = scanContextProto.grids(shift).rows();
    const int cols = scanContextProto.grids(shift).cols();

    Eigen::MatrixXd grid(rows, cols);
    std::vector<double> row;
    for (int idx = 0; idx < scanContextProto.grids(shift).values_size();
         ++idx) {
      int r = idx / cols;
      int c = idx - r * cols;
      grid(r, c) = scanContextProto.grids(shift).values(idx);
    }
    grids.push_back(grid);
  }
  type = "ScanContext";
}

double ScanContext::computeSimilarityScore(const iFeature &rhs) const {
  CHECK(this->type == rhs.type) << "Features are not the same type";
  const ScanContext &otherFeature = static_cast<const ScanContext &>(rhs);
  std::vector<double> distances;
  distances.reserve(this->grids.size() * otherFeature.grids.size());

  for (const Eigen::MatrixXd &grid : this->grids) {
    for (const Eigen::MatrixXd &otherGrid : otherFeature.grids) {
      // TODO(olga): Rotation invariance can go here. fliping matrix by columns.
      distances.push_back(computeDistanceBetweenGrids(grid, otherGrid));
    }
  }
  return *std::min_element(distances.begin(), distances.end());
}

double ScanContext::score2cost(double score) const { return score; }

std::vector<Eigen::VectorXd> ScanContext::computeRingKeys() const {
  std::vector<Eigen::VectorXd> ringKeys;
  for (int shift = 0; shift < grids.size(); ++shift) {
    ringKeys.push_back(computeRingKey(grids[shift]));
  }
  return ringKeys;
}

} // namespace localization::features
