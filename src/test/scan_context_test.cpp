/* By O. Vysotska in 2023 */

#include "features/scan_context.h"

#include <Eigen/Dense>

#include "gtest/gtest.h"
#include <Eigen/src/Core/Matrix.h>
#include <filesystem>

namespace test {

TEST(ComputeDistanceBetweenGrids, validInput) {
  Eigen::MatrixXd lhs(2, 3);
  lhs << 1, 0, 0, 1, 1, 0;
  Eigen::MatrixXd rhs(2, 3);
  rhs << 1, 1, 1, 1, 0, 0;

  EXPECT_NEAR(localization::features::computeDistanceBetweenGrids(lhs, rhs),
              1.0 / 3, 1e-02);
}

TEST(ComputeDistanceBetweenGrids, invalidInput) {
  Eigen::MatrixXd rhs(2, 3);
  rhs << 1, 1, 1, 1, 0, 0;

  EXPECT_DEATH(localization::features::computeDistanceBetweenGrids({}, rhs),
               "Invalid grid size.");

  Eigen::MatrixXd lhs(1, 3);
  lhs << 1, 1, 1;

  EXPECT_DEATH(localization::features::computeDistanceBetweenGrids(lhs, rhs),
               "The number of rows is not the same.");

  Eigen::MatrixXd lhs_wrong(2, 2);
  lhs_wrong << 1, 1, 1, 1;

  EXPECT_DEATH(
      localization::features::computeDistanceBetweenGrids(lhs_wrong, rhs),
      "The number of columns is not the same.");
}

TEST(ComputeRingKey, validInput) {
  Eigen::MatrixXd grid(3, 4);
  grid << 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0;
  const Eigen::VectorXd ringKey = localization::features::computeRingKey(grid);
  ASSERT_EQ(ringKey.size(), 3);
  EXPECT_NEAR(ringKey[0], 0.5, 1e-02);
  EXPECT_NEAR(ringKey[1], 0.25, 1e-02);
  EXPECT_NEAR(ringKey[2], 0.0, 1e-02);
}

// TODO(olga): Add tests for
// distanceBetweenRingKeys

} // namespace test