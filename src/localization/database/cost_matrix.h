#ifndef SRC_DATABASE_COST_MATRIX_H_
#define SRC_DATABASE_COST_MATRIX_H_

#include "features/feature_factory.h"

#include <string>
#include <vector>

using Matrix = std::vector<std::vector<double>>;

/**
 * @brief  Stores costs as matrix.
 */
class CostMatrix {
public:
  CostMatrix(const std::string &costMatrixFile);
  CostMatrix(const std::string &queryFeaturesDir,
             const std::string &refFeaturesDir, const FeatureType &type);
  CostMatrix(const Matrix &costs);

  /**
   * @brief      Loads from txt. Expects specific format: Pure matrix values.
   *
   * @param[in]  filename  The filename
   * @param[in]  rows      The rows
   * @param[in]  cols      The cols
   */
  void loadFromTxt(const std::string &filename, int rows, int cols);

  void loadFromProto(const std::string &filename);
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
};

#endif // SRC_DATABASE_COST_MATRIX_H_