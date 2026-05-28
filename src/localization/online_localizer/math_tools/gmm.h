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
#ifndef SRC_ONLINE_LOCALIZER_MATH_TOOLS_GMM_H_
#define SRC_ONLINE_LOCALIZER_MATH_TOOLS_GMM_H_

#include <memory>
#include <optional>
#include <ostream>
#include <vector>

namespace localization::online_localizer {

class GaussianModel {
public:
  GaussianModel(double mu, double sigma, double weight)
      : mu_(mu), sigma_(sigma), weight_(weight) {}
  double computeAssociationProbability(double value) const;
  std::vector<double>
  computeModelAssociations(const std::vector<double> &values) const;
  bool isDegenerate() const;
  void update(const std::vector<double> &values,
              const std::vector<double> &associations);
  double computeDifferenceTo(const GaussianModel &other) const;
  const double &mu() const { return mu_; }
  const double &sigma() const { return sigma_; }
  const double &weight() const { return weight_; }
  friend std::ostream &operator<<(std::ostream &stream,
                                  const GaussianModel &model) {
    stream << "mu " << model.mu_ << " sigma " << model.sigma_ << " weight "
           << model.weight_;
    return stream;
  }

private:
  double mu_ = 0.0;
  double sigma_ = 1.0;
  double weight_ = 0.0;
};

class GaussianMixtureModels {
public:
  void fitTwoModels(const std::vector<double> &values);
  const GaussianModel &matchingModel() const { return matchingModel_; }
  const GaussianModel &nonMatchingModel() const { return nonMatchingModel_; }

  void initializeForUnitValues(const std::vector<double> &values);

private:
  GaussianModel matchingModel_ = {1.0, 1.0, 0.5};
  GaussianModel nonMatchingModel_ = {0.0, 1.0, 0.5};
};

double computeModelsSeparationThreshold(const GaussianModel &matching,
                                        const GaussianModel &nonMatching);

bool xBetweenValues(double x, double valueLeft, double valueRight);
std::optional<double> computeDecisionBoundary(const GaussianModel &matching,
                                              const GaussianModel &nonMatching);

void normalizeAssociationProbabilities(
    std::vector<double> &matchingAssociations,
    std::vector<double> &nonMatchingAssociations);

std::optional<std::tuple<double, GaussianMixtureModels>>
estimateSeparationThreshold(const std::vector<double> &values,
                            bool distance_based = true);

} // namespace localization::online_localizer

#endif