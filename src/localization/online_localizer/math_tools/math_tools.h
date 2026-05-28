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
#ifndef SRC_ONLINE_LOCALIZER_MATH_TOOLS_H_
#define SRC_ONLINE_LOCALIZER_MATH_TOOLS_H_

#include <vector>

namespace localization::online_localizer {

double getNormalPdfValueAtX(double x, double mu, double sigma);
std::vector<double> computeCumulativeSum(const std::vector<double> &array);
double interpolate1D(const std::vector<double> &x, const std::vector<double> &y,
                     double t);

double estimateMean(const std::vector<double> &values);
double estimateVariance(const std::vector<double> &values);

struct ValueEstimate {
  double value;
  double uncertainty;
};
ValueEstimate kalmanFilterUpdate(ValueEstimate previousEstimate,
                                 ValueEstimate measurement);

} // namespace localization::online_localizer

#endif