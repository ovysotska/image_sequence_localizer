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

#include "database/cost_matrix_database.h"

#include <fstream>
#include <limits>

#include "database/online_database.h"
#include "localization_protos.pb.h"

#include <glog/logging.h>

CostMatrixDatabase::CostMatrixDatabase(const std::string &costMatrixFile,
                                       const std::string &queryFeaturesDir,
                                       const std::string &refFeaturesDir,
                                       const FeatureType &type, int bufferSize)
    : OnlineDatabase{queryFeaturesDir, refFeaturesDir, type, bufferSize} {
  loadFromProto(costMatrixFile);
}

void CostMatrixDatabase::loadFromTxt(const std::string &filename, int rows,
                                     int cols) {
  std::ifstream in(filename.c_str());
  LOG_IF(FATAL, !in) << "The file cannot be opened " << filename;

  LOG(INFO) << "The matrix has " << rows << " rows and " << cols << "cols";
  for (int r = 0; r < rows; ++r) {
    std::vector<double> row(cols);
    for (int c = 0; c < cols; ++c) {
      float value;
      in >> value;
      row[c] = value;
    }
    costs_.push_back(row);
  }
  LOG(INFO) << "Matrix was read";
  in.close();
}

void CostMatrixDatabase::setCosts(const Matrix &costs, int rows, int cols) {
  rows_ = rows;
  cols_ = cols;
  costs_ = costs;
}

double CostMatrixDatabase::getCost(int quId, int refId) {
  if (quId >= rows_ || quId < 0) {
    printf("[ERROR][CostMatrixDatabase] Invalid query index %d\n", quId);
    return -1;
  }
  if (refId >= cols_ || refId < 0) {
    printf("[ERROR][CostMatrixDatabase] Invalid query index %d\n", refId);
    return -1;
  }
  double value = costs_[quId][refId];
  if (value < 1e-09) {
    return std::numeric_limits<double>::max();
  }
  return 1. / value;
}

void CostMatrixDatabase::loadFromProto(const std::string &filename) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  image_sequence_localizer::CostMatrix cost_matrix_proto;
  std::fstream input(filename, std::ios::in | std::ios::binary);

  if (!cost_matrix_proto.ParseFromIstream(&input)) {
    LOG(FATAL) << "Failed to parse cost_matrix file: " << filename;
  }
  std::vector<double> row;
  for (int idx = 0; idx < cost_matrix_proto.values_size(); ++idx) {
    row.push_back(cost_matrix_proto.values(idx));
    if (row.size() == cost_matrix_proto.cols()) {
      costs_.push_back(row);
      row.clear();
    }
  }
  cols_ = cost_matrix_proto.cols();
  rows_ = cost_matrix_proto.rows();
  LOG(INFO) << "Read cost matrix with " << rows_ << " rows and " << cols_
            << " cols.";
}
