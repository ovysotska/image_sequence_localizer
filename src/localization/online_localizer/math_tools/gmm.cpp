/** Copyright (c) 2026 Olga Vysotska, RSL, ETH Zurich 
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/
#include "online_localizer/math_tools/gmm.h"
#include "online_localizer/math_tools/math_tools.h"

#include "glog/logging.h"

#include <array>
#include <cmath>
#include <numeric>
#include <vector>

namespace localization::online_localizer {

namespace {
const int kGmmIterationNumber = 100;
const double kModelChangeTolerance = 1e-08;
} // namespace

bool valuesAreFromUnitSegment(const std::vector<double> &values) {
  for (const double &value : values) {
    if (value < 0 - std::numeric_limits<double>::epsilon() ||
        value > 1.0 + std::numeric_limits<double>::epsilon()) {
      LOG(WARNING) << "Value is outside the unit segment " << value;
      return false;
    }
  }
  return true;
}

void GaussianModel::update(const std::vector<double> &values,
                           const std::vector<double> &associations) {

  double associationProbabilitySum =
      std::reduce(associations.begin(), associations.end());
  int valuesNum = values.size();

  // update mu
  mu_ = std::inner_product(associations.begin(), associations.end(),
                           values.begin(), 0.0) /
        associationProbabilitySum;

  // update sigma
  sigma_ = 0;
  for (int i = 0; i < valuesNum; ++i) {
    sigma_ += associations[i] * (values[i] - mu_) * (values[i] - mu_);
  }
  sigma_ /= associationProbabilitySum;
  if (std::fabs(sigma_) < 1e-08) {
    sigma_ = 0;
  }
  sigma_ = std::sqrt(sigma_);

  // update weight
  weight_ = associationProbabilitySum / valuesNum;
}

double GaussianModel::computeAssociationProbability(double value) const {
  return weight_ * getNormalPdfValueAtX(value, mu_, sigma_);
}

std::vector<double> GaussianModel::computeModelAssociations(
    const std::vector<double> &values) const {
  std::vector<double> associations;
  associations.reserve(values.size());
  for (int i = 0; i < values.size(); ++i) {
    associations.push_back(computeAssociationProbability(values[i]));
  }
  return associations;
}

double GaussianModel::computeDifferenceTo(const GaussianModel &other) const {
  // This could be a more elaborate comparison.
  return std::abs(mu_ - other.mu_);
}

bool GaussianModel::isDegenerate() const {
  if (std::isnan(mu_) || std::isnan(sigma_)) {
    return true;
  }
  return false;
}

void GaussianMixtureModels::initializeForUnitValues(
    const std::vector<double> &values) {
  int valuesNum = values.size();
  CHECK(valuesNum >= 2) << "Need at least 2 samples to compute GMM.";
  DCHECK(valuesAreFromUnitSegment(values))
      << "Values are outside the unit segment.";
  // This initialization only holds, because we assume all the values to be in
  // [0,1]. So we assume values to already represent the probability to belong
  // to class "Matching".
  std::vector<double> matchingAssociations = values;
  std::vector<double> nonMatchingAssociations;
  nonMatchingAssociations.reserve(valuesNum);
  for (int i = 0; i < valuesNum; i++) {
    nonMatchingAssociations.emplace_back(1 - values[i]);
  }
  matchingModel_.update(values, matchingAssociations);
  nonMatchingModel_.update(values, nonMatchingAssociations);
}

void normalizeAssociationProbabilities(
    std::vector<double> &matchingAssociations,
    std::vector<double> &nonMatchingAssociations) {
  CHECK(matchingAssociations.size() == nonMatchingAssociations.size())
      << "Associations of of different sizes.";

  for (int i = 0; i < matchingAssociations.size(); ++i) {
    double probabilitySum =
        matchingAssociations[i] + nonMatchingAssociations[i];
    CHECK(probabilitySum > 1e-08)
        << "Critical value of probability sum " << probabilitySum;
    matchingAssociations[i] /= probabilitySum;
    nonMatchingAssociations[i] /= probabilitySum;
  }
}

void GaussianMixtureModels::fitTwoModels(const std::vector<double> &values) {

  // Probably can be omitted by the power of math.
  this->initializeForUnitValues(values);

  GaussianModel previousMatching = matchingModel_;
  GaussianModel previousNonMatching = nonMatchingModel_;

  for (int iter = 0; iter < kGmmIterationNumber; ++iter) {
    // E-step
    std::vector<double> matchingAssociations =
        matchingModel_.computeModelAssociations(values);
    std::vector<double> nonMatchingAssociations =
        nonMatchingModel_.computeModelAssociations(values);
    normalizeAssociationProbabilities(matchingAssociations,
                                      nonMatchingAssociations);

    // M-step
    matchingModel_.update(values, matchingAssociations);
    nonMatchingModel_.update(values, nonMatchingAssociations);

    if (matchingModel_.computeDifferenceTo(previousMatching) <
            kModelChangeTolerance ||
        nonMatchingModel_.computeDifferenceTo(previousNonMatching) <
            kModelChangeTolerance) {
      break;
    } else {
      previousMatching = matchingModel_;
      previousNonMatching = nonMatchingModel_;
    }
  }
}

double computeModelsSeparationThreshold(const GaussianModel &matching,
                                        const GaussianModel &nonMatching) {
  // Computes a mean value between the ends of the 1-sigma confidence interval.
  double boundaryClass0Right = nonMatching.mu() + nonMatching.sigma();
  double boundaryClass0Left = nonMatching.mu() - nonMatching.sigma();
  double boundaryClass1Left = matching.mu() - matching.sigma();
  double boundaryClass1Right = matching.mu() + matching.sigma();
  if (boundaryClass0Right < boundaryClass1Left ||
      (boundaryClass0Left < boundaryClass1Left &&
       boundaryClass1Left < boundaryClass0Right)) {
    return (boundaryClass1Left + boundaryClass0Right) / 2;
  }
  return (boundaryClass0Left + boundaryClass1Right) / 2;
}

bool xBetweenValues(double x, double valueLeft, double valueRight) {
  CHECK(valueLeft <= valueRight) << "Left value > right value";
  return (valueLeft <= x && x <= valueRight);
}

std::optional<double>
computeDecisionBoundary(const GaussianModel &matching,
                        const GaussianModel &nonMatching) {
  // This function compute the boundary between two Gaussian distribution
  // The boundary is the point x that has equal probability to belong to both
  // classes Please check derivation_boundary_estimation.md for details.
  double mu_1 = matching.mu();
  double sigma_1 = matching.sigma();
  double pi_1 = matching.weight();

  double mu_2 = nonMatching.mu();
  double sigma_2 = nonMatching.sigma();
  double pi_2 = nonMatching.weight();

  if (std::abs(mu_1 - mu_2) < 1e-08) {
    // means are the same -> no separation boundary
    LOG(WARNING) << "Means are too close" << mu_1 << ", " << mu_2;
    return {};
  }

  // if variances are equal
  if (std::abs(sigma_1 - sigma_2) < 1e-03 || std::abs(sigma_1) < 1e-08 ||
      std::abs(sigma_2) < 1e-08) {
    // having a linear equation now.
    return (2.0 * (sigma_1 * sigma_1) * std::log(pi_2 / pi_1) -
            (mu_2 * mu_2 - mu_1 * mu_1)) /
           (2.0 * (mu_1 - mu_2));
  }

  double A = 1. / (2 * sigma_2 * sigma_2) - 1. / (2 * sigma_1 * sigma_1);
  double B = mu_1 / (sigma_1 * sigma_1) - mu_2 / (sigma_2 * sigma_2);
  double C = (mu_2 * mu_2) / (2 * sigma_2 * sigma_2) -
             (mu_1 * mu_1) / (2 * sigma_1 * sigma_1) -
             std::log((pi_2 * sigma_1) / (sigma_2 * pi_1));

  double discriminant = B * B - 4 * A * C;
  if (discriminant > 0) {
    double solution_1 = (-B + std::sqrt(discriminant)) / (2 * A);
    double solution_2 = (-B - std::sqrt(discriminant)) / (2 * A);

    if (mu_1 <= mu_2) {
      if (xBetweenValues(solution_1, mu_1, mu_2)) {
        return solution_1;
      } else if (xBetweenValues(solution_2, mu_1, mu_2)) {
        return solution_2;
      }
      LOG(WARNING)
          << " None of the solution of quadratic equation is between the mu_1: "
          << mu_1 << " and mu_2: " << mu_2;
      LOG(INFO) << "Solution 1: " << solution_1;
      LOG(INFO) << "Solution 2: " << solution_2;
      return {};
    }
    if (mu_2 < mu_1) {
      if (xBetweenValues(solution_1, mu_2, mu_1)) {
        return solution_1;
      } else if (xBetweenValues(solution_2, mu_2, mu_1)) {
        return solution_2;
      }
      LOG(WARNING)
          << " None of the solution of quadratic equation is between the mu_2: "
          << mu_2 << " and mu_1: " << mu_1;
      LOG(INFO) << "Solution 1: " << solution_1;
      LOG(INFO) << "Solution 2: " << solution_2;
      return {};
    }
  } else if (std::abs(discriminant) < 1e-08) {
    return -B / (2 * A);
  }

  LOG(FATAL) << "Quadratic equation does not have real solutions";
}

std::optional<std::tuple<double, GaussianMixtureModels>>
estimateSeparationThreshold(const std::vector<double> &values,
                            bool distance_based) {
  GaussianMixtureModels models;
  models.fitTwoModels(values);
  if (models.matchingModel().isDegenerate() ||
      models.nonMatchingModel().isDegenerate()) {
    LOG(WARNING) << "One of the models is degenerate";
    return {};
  }
  if (distance_based) {
    return std::tuple(computeModelsSeparationThreshold(
                          models.matchingModel(), models.nonMatchingModel()),
                      models);
  }

  std::optional<double> decision_boundary = computeDecisionBoundary(
      models.matchingModel(), models.nonMatchingModel());

  if (decision_boundary) {
    return std::tuple(decision_boundary.value(), models);
  }
  return {};
}
} // namespace localization::online_localizer