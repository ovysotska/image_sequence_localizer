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
#include "statistical_test.h"
#include "constants.h"
#include "glog/logging.h"
#include "math_tools.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

namespace localization::online_localizer {

namespace {
const int kHistogramBinsNum = 50;
const double kSignificanceValue = 0.05;
} // namespace

const EmpiricalDistribution kGaussianDistribution = createStandardGaussian();
const EmpiricalDistribution kKolmogorovPvalueDistribution =
    createKolmogorovDistribution();

EmpiricalDistribution createStandardGaussian() {
  const float kGaussianResolution = 0.001;
  const float kGaussianBoundaries = 5.0;
  std::vector<double> gaussianX(2 * kGaussianBoundaries / kGaussianResolution,
                                0.0);
  std::vector<double> gaussianPdf(2 * kGaussianBoundaries / kGaussianResolution,
                                  0.0);
  std::vector<double> gaussianCdf(2 * kGaussianBoundaries / kGaussianResolution,
                                  0.0);
  gaussianX[0] = -kGaussianBoundaries;
  gaussianPdf[0] = getNormalPdfValueAtX(gaussianX[0], 0.0, 1.0);
  for (int i = 1; i < gaussianX.size(); ++i) {
    gaussianX[i] = gaussianX[i - 1] + kGaussianResolution;
    gaussianPdf[i] = getNormalPdfValueAtX(gaussianX[i], 0.0, 1.0);
  }

  std::vector<double> cumsumValues = computeCumulativeSum(gaussianPdf);
  // We need to multiply by bin_size to turn cumsum into cdf (describes the cdf
  // of a distribution)
  for (int idx = 0; idx < cumsumValues.size(); ++idx) {
    gaussianCdf[idx] = cumsumValues[idx] * kGaussianResolution;
  }
  return EmpiricalDistribution(gaussianX, gaussianPdf, gaussianCdf);
}

double getNormalCdfAtPoint(double point, double mu, double sigma,
                           const EmpiricalDistribution &gaussianDistribution) {
  double pointNorm = (point - mu) / sigma;
  return interpolate1D(gaussianDistribution.x, gaussianDistribution.cdf,
                       pointNorm);
}

EmpiricalDistribution createKolmogorovDistribution() {
  return EmpiricalDistribution(constants::kolmogorovX,
                               constants::kolmogorovPvalue, {});
}

EmpiricalDistribution::EmpiricalDistribution(const std::vector<double> &x,
                                             const std::vector<double> &values,
                                             const std::vector<double> &cdf) {
  // Make sure that the values make sense. No further checks are provided here.
  this->x = x;
  this->values = values;
  this->cdf = cdf;
}

EmpiricalDistribution::EmpiricalDistribution(const std::vector<double> &samples,
                                             int numBins) {

  CHECK(samples.size() > 1) << "Samples should contain at least 2 values.";
  CHECK(numBins > 0) << "Num of Bins must be higher then 0.";

  computeHistogram(samples, numBins);
  computeCdf();
}

void EmpiricalDistribution::computeHistogram(const std::vector<double> &samples,
                                             int numBins) {
  double minValue = *std::min_element(samples.begin(), samples.end());
  double maxValue = *std::max_element(samples.begin(), samples.end());

  CHECK(maxValue > minValue) << "Max value should be bigger then min value";

  double binSize = (maxValue - minValue) / numBins;
  double tmpX = minValue;
  while (tmpX <= maxValue) {
    x.push_back(tmpX);
    tmpX += binSize;
  }

  values.resize(x.size(), 0.0);
  for (double value : samples) {
    int binIndex = std::floor((value - minValue) / binSize);
    if (binIndex == x.size() && std::abs(value - maxValue) < 1e-08) {
      values[binIndex - 1] += 1;
      continue;
    }
    LOG_IF(WARNING, binIndex > x.size()) << "Bin index is outside boundaries";
    values[binIndex] += 1;
  }
}

void EmpiricalDistribution::computeCdf() {
  // In theory, the cdf is the integral over the pdf (lossely speaking).
  // However, we don't compute a real PDF before, we just use histogram,
  // so just computing the cumulative sum and dividing by the last element (the
  // sum of all the values) represents the same cdf if we would have computed
  // the proper pdf and the integral over it.
  std::vector<double> cumsumValues = computeCumulativeSum(values);
  cdf.resize(values.size(), 0.0);
  for (int idx = 0; idx < cumsumValues.size(); ++idx) {
    cdf[idx] = cumsumValues[idx] / *(cumsumValues.end() - 1);
  }
}

std::tuple<bool, double, double>
patchContainsPath(const std::vector<double> &values) {
  // This function checks if the patch contains the path using
  // the Kolmogorov-Smirnov statistical test.
  // https://en.wikipedia.org/wiki/Kolmogorov%E2%80%93Smirnov_test
  // We consider that the patch contains NO path is the values form a unimodal
  // distribution. If the observed distribution diverges from the unimodal
  // Gaussian distribution, then there exists a path in the patch.
  // The null hypothesis in the ks-test is: The patch is unimodal.

  // The idea: We compute a unimodal Gaussian parameters from the set of values.
  // Then we compare the theoretical CDF of this unimodal Gaussian with the
  // empirical CDF of the values (real, obtained values).
  // If the CDF differs "significantly" (according to the kSignificanceValue)
  // then we reject the null hypothesis. And the patch contains the path.

  const double expectedMean = estimateMean(values);
  const double expectedVariance = estimateVariance(values);
  const double expectedSigma = std::sqrt(expectedVariance);

  EmpiricalDistribution observedDistribution =
      EmpiricalDistribution(values, kHistogramBinsNum);

  std::vector<double> gaussianCdfsAtObservedPoints;
  for (const auto &observedPoint : observedDistribution.x) {
    gaussianCdfsAtObservedPoints.push_back(getNormalCdfAtPoint(
        observedPoint, expectedMean, expectedSigma, kGaussianDistribution));
  }

  // Compute D statistics. Notation from KS test.
  std::vector<double> dStatistics(gaussianCdfsAtObservedPoints.size());
  for (int pointIdx = 0; pointIdx < gaussianCdfsAtObservedPoints.size();
       ++pointIdx) {
    dStatistics[pointIdx] = observedDistribution.cdf[pointIdx] -
                            gaussianCdfsAtObservedPoints[pointIdx];
  }

  auto dMax = std::max_element(dStatistics.begin(), dStatistics.end());
  auto dMin = std::min_element(dStatistics.begin(), dStatistics.end());

  double Dn, location;
  if (std::abs(*dMax) > std::abs(*dMin)) {
    Dn = std::abs(*dMax);
    location = observedDistribution.x[dMax - dStatistics.begin()];
  } else {
    Dn = std::abs(*dMin);
    location = observedDistribution.x[dMin - dStatistics.begin()];
  }
  double Kn = std::sqrt(observedDistribution.x.size()) * Dn;
  double pvalue = interpolate1D(kKolmogorovPvalueDistribution.x,
                                kKolmogorovPvalueDistribution.values, Kn);

  // If pvalue < 0.05, we reject the null hypothesis that patch is unimodal
  // with probability of 95%
  return std::make_tuple(pvalue < kSignificanceValue, pvalue, location);
}

} // namespace localization::online_localizer