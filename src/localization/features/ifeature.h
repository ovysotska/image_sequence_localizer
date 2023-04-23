/** vpr_relocalization: a library for visual place recognition in changing
** environments with efficient relocalization step.
** Copyright (c) 2017 O. Vysotska, C. Stachniss, University of Bonn
**
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

/* Updated by O. Vysotska in 2022 */

#ifndef SRC_FEATURES_IFEATURE_H_
#define SRC_FEATURES_IFEATURE_H_

#include <memory>
#include <string>
#include <vector>

namespace localization::features {

/**
 * @brief      Interface class for features.
 */
class iFeature {
public:
  /**
   * @brief     Returns the similarity score between two feature vectors. The
   * higher the score the more similar two features should be.
   *
   * @param[in]  rhs   The right hand side
   *
   * @return     One double value as a result of comparison.
   */
  virtual double computeSimilarityScore(const iFeature &rhs) const = 0;
  /**
   * @brief      Transforms similarity into the weights/cost for the graph.

   * @param[in]  score  The score
   *
   * @return     weight/cost. If cost is near to 0 (<1e-09), returns the
   * std::numeric_limits<double>::max()
   */
  virtual double score2cost(double score) const = 0;
  virtual ~iFeature() {}
  iFeature() = default;
  iFeature(const iFeature &) = delete;
  iFeature(iFeature &&) = delete;
  iFeature &operator=(const iFeature &) = delete;
  iFeature &operator=(iFeature &&) = delete;

  std::string type = "";
  std::vector<int> bits;
  std::vector<double> dimensions;
};
/**
  \fn iFeature::score2weight
   * Intuitively, the bigger the similarity between the feature the smaller
   * should be the weight on the corresponding edges in the graph.
   * If you already have an "inverted similarity" score, this function should
  just return the same value it takes as an input.

*/
} // namespace localization::features

#endif // SRC_FEATURES_IFEATURE_H_
