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

#ifndef SRC_DATABASE_SIMILARITY_MATRIX_DATABASE_H_
#define SRC_DATABASE_SIMILARITY_MATRIX_DATABASE_H_

#include "database/idatabase.h"
#include "database/similarity_matrix.h"

#include <memory>
#include <string>

namespace localization::database {

class SimilarityMatrixDatabase : public iDatabase {
public:
  explicit SimilarityMatrixDatabase(const std::string &costMatrixFile);

  int refSize() override { return similarityMatrix_.cols(); }
  double getCost(int quId, int refId) override;

private:
  SimilarityMatrix similarityMatrix_;
};

} // namespace localization::database

#endif // SRC_DATABASE_SIMILARITY_MATRIX_DATABASE_H_
