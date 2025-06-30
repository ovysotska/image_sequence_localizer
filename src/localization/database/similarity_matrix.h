/* By O. Vysotska in 2023 */

#ifndef SRC_DATABASE_SIMILARITY_MATRIX_H_
#define SRC_DATABASE_SIMILARITY_MATRIX_H_

#include "features/feature_factory.h"

#include <string>
#include <vector>

namespace localization::database {
class SimilarityMatrix {
public:
  using Matrix = std::vector<std::vector<double>>;

  explicit SimilarityMatrix(const std::string &similarityMatrixFile);
  SimilarityMatrix(const std::string &queryFeaturesDir,
             const std::string &refFeaturesDir,
             const features::FeatureType &type);
  explicit SimilarityMatrix(const Matrix &costs);

  void loadFromTxt(const std::string &filename, int rows, int cols);

  void loadFromProto(const std::string &filename);
  const Matrix &getScores() const { return scores_; }

  double at(int row, int col) const;
  // Cost is the value opposite to a score.
  // This function computes cost as inverse score cost = 1/score.
  double getCost(int row, int col) const;
  int rows() const { return rows_; }
  int cols() const { return cols_; }

private:
  Matrix scores_;
  int rows_ = 0;
  int cols_ = 0;
};
} // namespace localization::database

#endif // SRC_DATABASE_SIMILARITY_MATRIX_H_