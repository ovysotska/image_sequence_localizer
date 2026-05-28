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
#include "math_tools.h"
#include "glog/logging.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace localization::online_localizer {

namespace {
constexpr inline double kInvSqrt2Pi = 0.3989422804014327;
} // namespace

std::vector<double> computeCumulativeSum(const std::vector<double> &array) {

  std::vector<double> cumSum;
  double sum = 0;
  cumSum.reserve(array.size());
  for (auto number : array) {
    sum += number;
    cumSum.emplace_back(sum);
  }
  return cumSum;
}

double estimateMean(const std::vector<double> &values) {
  CHECK(!values.empty()) << "Error: Cannot calculate mean for an empty vector.";
  double sum = std::accumulate(values.begin(), values.end(), 0.0);
  double mean = sum / values.size();

  return mean;
}

double estimateVariance(const std::vector<double> &values) {
  CHECK(values.size() >= 2)
      << "Error: Variance calculation requires at least two elements.";
  double mean = estimateMean(values);
  double sumSquaredDifferences = std::accumulate(
      values.begin(), values.end(), 0.0, [mean](double acc, double value) {
        return acc + (value - mean) * (value - mean);
      });

  double variance = sumSquaredDifferences / (values.size());

  return variance;
}

// Assumes x and y are sorted.
double interpolate1D(const std::vector<double> &x, const std::vector<double> &y,
                     double point) {

  CHECK(x.size() == y.size()) << "x and y vectors must have the same size";

  LOG_IF(WARNING, point < x.front() || point > x.back())
      << "point " << point << " is outside range [" << x.front() << ","
      << x.back() << "]";
  point = std::max(x.front(), point);
  point = std::min(x.back(), point);

  // Find the index i such that point < x[i]
  auto upper = std::upper_bound(x.begin(), x.end(), point);
  int i = 0;
  if (upper != x.begin()) {
    i = std::distance(x.begin(), upper) - 1;
  }

  // Perform linear interpolation
  double interpolatedValue =
      y[i] + ((y[i + 1] - y[i]) / (x[i + 1] - x[i])) * (point - x[i]);
  return interpolatedValue;
}

double getNormalPdfValueAtX(double x, double mu, double sigma) {
  if (std::abs(sigma) < 1e-08) {
    LOG(WARNING) << "Sigma is too close to 0, sigma= " << sigma;
    sigma = 1e-08;
  }
  double tmp = (x - mu) / sigma;
  return kInvSqrt2Pi / sigma * std::exp(-0.5 * tmp * tmp);
}

ValueEstimate kalmanFilterUpdate(ValueEstimate previousEstimate,
                                 ValueEstimate measurement) {
  CHECK(previousEstimate.uncertainty > 0) << "Previous uncertainty <= 0.";
  CHECK(measurement.uncertainty > 0) << "Measuremerent uncertainty <= 0.";
  // TODO(olga, when visualization is ready). Add process noise to avoid too
  // certain estimates.
  previousEstimate.uncertainty += 0.01;
  double kalmanGain = previousEstimate.uncertainty /
                      (previousEstimate.uncertainty + measurement.uncertainty);

  ValueEstimate updatedEstimate;
  updatedEstimate.value =
      previousEstimate.value +
      kalmanGain * (measurement.value - previousEstimate.value);

  updatedEstimate.uncertainty = (1 - kalmanGain) * previousEstimate.uncertainty;

  return updatedEstimate;
}

} // namespace localization::online_localizer