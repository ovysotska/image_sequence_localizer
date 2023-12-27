/* By O. Vysotska in 2023 */

#include "cost_matrix.h"
#include "database/list_dir.h"
#include "features/feature_factory.h"
#include "localization_protos.pb.h"

#include <glog/logging.h>

#include <fstream>

namespace localization::database {

namespace {
constexpr auto kEpsilon = 1e-09;
} // namespace

CostMatrix::CostMatrix(const std::string &costMatrixFile) {
  CHECK(!costMatrixFile.empty()) << "Cost matrix file is not set";
  loadFromProto(costMatrixFile);
}

CostMatrix::CostMatrix(const std::string &queryFeaturesDir,
                       const std::string &refFeaturesDir,
                       const features::FeatureType &type) {
  const std::vector<std::string> queryFeaturesFiles =
      listProtoDir(queryFeaturesDir, ".Feature");
  const std::vector<std::string> refFeaturesFiles =
      listProtoDir(refFeaturesDir, ".Feature");

  LOG(INFO) << "Query features size " << queryFeaturesFiles.size();
  LOG(INFO) << "Reference features size " << refFeaturesDir.size();
  LOG(INFO) << "Computing cost matrix. This make take some time...";

  costs_.reserve(queryFeaturesFiles.size());
  for (int fileIdx = 0; fileIdx < queryFeaturesFiles.size(); ++fileIdx) {
    auto queryFeature = createFeature(type, queryFeaturesFiles[fileIdx]);
    std::vector<double> row;
    row.reserve(refFeaturesFiles.size());
    for (const auto &refFile : refFeaturesFiles) {
      const auto refFeature = createFeature(type, refFile);
      row.push_back(queryFeature->computeSimilarityScore(*refFeature));
    }
    LOG(INFO) << "Computed row values for query image " << fileIdx;
    costs_.push_back(row);
  }
  rows_ = costs_.size();
  if (costs_.size() > 0) {
    cols_ = costs_[0].size();
  }
}

CostMatrix::CostMatrix(const Matrix &costs) {
  costs_ = costs;
  rows_ = costs.size();
  cols_ = rows_ > 0 ? costs[0].size() : 0;
}

void CostMatrix::loadFromTxt(const std::string &filename, int rows, int cols) {
  std::ifstream in(filename);
  LOG_IF(FATAL, !in) << "The file cannot be opened " << filename;

  LOG(INFO) << "The matrix has " << rows << " rows and " << cols << "cols";
  for (int r = 0; r < rows; ++r) {
    std::vector<double> row(cols);
    for (int c = 0; c < cols; ++c) {
      float value;
      in >> row[c];
    }
    costs_.push_back(row);
  }
  LOG(INFO) << "Matrix was read";
  in.close();
  rows_ = rows;
  cols_ = cols;
}

double CostMatrix::at(int row, int col) const {
  CHECK(row >= 0 && row < rows_) << "Row outside range " << row;
  CHECK(col >= 0 && col < cols_) << "Col outside range " << col;
  if (inverseCosts_) {
    return getInverseCost(row, col);
  }
  return costs_[row][col];
}

double CostMatrix::getInverseCost(int row, int col) const {
  const double value = costs_[row][col];
  if (std::abs(value) < kEpsilon) {
    return std::numeric_limits<double>::max();
  }
  if (value < 0) {
    LOG(WARNING) << "The cost value for row:" << row << " and col:" << col
                 << " is < 0: " << value
                 << ". This should not be like this. I will make a positive "
                    "value of it for now. But please check your values";
  }
  return 1. / std::abs(value);
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

void CostMatrix::storeToProto(const std::string &protoFilename) const {
  image_sequence_localizer::CostMatrix costMatrixProto;
  costMatrixProto.set_cols(cols_);
  costMatrixProto.set_rows(rows_);
  for (int r = 0; r < rows_; ++r) {
    for (int c = 0; c < cols_; ++c) {
      costMatrixProto.add_values(costs_[r][c]);
    }
  }

  std::fstream out(protoFilename,
                   std::ios::out | std::ios::trunc | std::ios::binary);
  if (!costMatrixProto.SerializeToOstream(&out)) {
    LOG(ERROR) << "Couldn't open the file " << protoFilename;
    LOG(ERROR) << "The path is NOT saved.";
    return;
  }
  out.close();
  LOG(INFO) << "The cost matrix was written to " << protoFilename;
}
} // namespace localization::database
