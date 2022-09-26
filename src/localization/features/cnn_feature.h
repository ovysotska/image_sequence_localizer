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

#ifndef SRC_FEATURES_CNNFEATURE_H_
#define SRC_FEATURES_CNNFEATURE_H_
#include "features/ifeature.h"
#include <string>
#include <vector>

/**
 * @brief      Class for cnn feature. By default binarizable with Mid
 * binarization
 */
class CnnFeature : public iFeature {
public:

  CnnFeature(const std::string &filename);

  // Computes the cosine distance between two vectors.
  double computeSimilarityScore(const iFeature &rhs) const override;
  /**
   * @brief      weight/cost is an inverse of a score.
   *
   * @param[in]  score  The score
   *
   * @return    weight/cost. If cost is near to 0, returns the
   * std::numeric_limits<double>::max()
   */
  double score2cost(double score) const override;

  std::vector<double> dim;
  using iFeature::bits;

protected:
  void binarize();
};

#endif // SRC_FEATURES_CNNFEATURE_H_
