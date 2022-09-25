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

#ifndef SRC_DATABASE_COST_MATRIX_DATABASE_H_
#define SRC_DATABASE_COST_MATRIX_DATABASE_H_

#include "database/online_database.h"
#include <string>

typedef std::vector<std::vector<double>> Matrix;

/**
 * @brief      Class for cost matrix database. Stores costs as matrix.
 */
class CostMatrixDatabase : public OnlineDatabase {
public:
  CostMatrixDatabase(const std::string &costMatrixFile,
                     const std::string &queryFeaturesDir,
                     const std::string &refFeaturesDir, const FeatureType &type,
                     int bufferSize);

  int refSize() override { return cols_; }
  /** gets the original cost and transforms it 1/cost **/
  double getCost(int quId, int refId) override;

  /**
   * @brief      Loads from txt. Expects specific format: Pure matrix values.
   *
   * @param[in]  filename  The filename
   * @param[in]  rows      The rows
   * @param[in]  cols      The cols
   */
  void loadFromTxt(const std::string &filename, int rows, int cols);

  void loadFromProto(const std::string &filename);

  void overrideCosts(const Matrix &costs, int rows, int cols);
  const Matrix &getCosts() const { return costs_; }

private:
  Matrix costs_;
  int rows_ = 0;
  int cols_ = 0;
};

#endif // SRC_DATABASE_COST_MATRIX_DATABASE_H_
