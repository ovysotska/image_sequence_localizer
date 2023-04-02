#include "cost_matrix.h"
#include <fstream>

#include "localization_protos.pb.h"

#include <glog/logging.h>

void CostMatrix::loadFromTxt(const std::string &filename, int rows, int cols) {
  std::ifstream in(filename);
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

void CostMatrix::loadFromProto(const std::string &filename) {
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
