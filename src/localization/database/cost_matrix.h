/* By O. Vysotska in 2023 */

#ifndef SRC_DATABASE_COST_MATRIX_H_
#define SRC_DATABASE_COST_MATRIX_H_

#include "features/feature_factory.h"

#include <string>
#include <vector>

namespace localization::database {

class CostMatrix {
public:
  using Matrix = std::vector<std::vector<double>>;

  explicit CostMatrix(const std::string &costMatrixFile);
  CostMatrix(const std::string &queryFeaturesDir,
             const std::string &refFeaturesDir,
             const features::FeatureType &type);
  explicit CostMatrix(const Matrix &costs);

  void loadFromTxt(const std::string &filename, int rows, int cols);

  // TODO(olga): This should be removed once ifeature doesn't have explicit
  // simlarity score requirement.
  void inverseCosts(bool inverse = true) { inverseCosts_ = inverse; }

  void loadFromProto(const std::string &filename);
  void storeToProto(const std::string &protoFilename) const;
  const Matrix &getCosts() const { return costs_; }

  double at(int row, int col) const;
  // Computes 1/value.
  double getInverseCost(int row, int col) const;

  int rows() const { return rows_; }
  int cols() const { return cols_; }

private:
  Matrix costs_;
  int rows_ = 0;
  int cols_ = 0;
  bool inverseCosts_ = true;
};
} // namespace localization::database

#endif // SRC_DATABASE_COST_MATRIX_H_