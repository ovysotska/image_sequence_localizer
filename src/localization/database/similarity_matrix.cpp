/* By O. Vysotska in 2023 */

#include "similarity_matrix.h"
#include "database/list_dir.h"
#include "features/feature_factory.h"
#include "localization_protos.pb.h"

#include <glog/logging.h>

#include <fstream>

namespace localization::database {

namespace {
constexpr auto kEpsilon = 1e-09;
} // namespace

SimilarityMatrix::SimilarityMatrix(const std::string &similarityMatrixFile) {
  CHECK(!similarityMatrixFile.empty()) << "Cost matrix file is not set";
  loadFromProto(similarityMatrixFile);
}

SimilarityMatrix::SimilarityMatrix(const std::string &queryFeaturesDir,
                       const std::string &refFeaturesDir,
                       const features::FeatureType &type) {
  const std::vector<std::string> queryFeaturesFiles =
      listProtoDir(queryFeaturesDir, ".Feature");
  const std::vector<std::string> refFeaturesFiles =
      listProtoDir(refFeaturesDir, ".Feature");

  std::cerr << "Query features" << queryFeaturesFiles.size() << std::endl;
  std::cerr << "ref features" << refFeaturesFiles.size() << std::endl;

  scores_.reserve(queryFeaturesFiles.size());
  for (const auto &queryFile : queryFeaturesFiles) {
    auto queryFeature = createFeature(type, queryFile);
    std::vector<double> row;
    row.reserve(refFeaturesFiles.size());
    for (const auto &refFile : refFeaturesFiles) {
      const auto refFeature = createFeature(type, refFile);
      row.push_back(queryFeature->computeSimilarityScore(*refFeature));
    }
    scores_.push_back(row);
  }
  rows_ = scores_.size();
  if (scores_.size() > 0) {
    cols_ = scores_[0].size();
  }
}

SimilarityMatrix::SimilarityMatrix(const Matrix &scores) {
  scores_ = scores;
  rows_ = scores.size();
  cols_ = rows_ > 0 ? scores[0].size() : 0;
}

void SimilarityMatrix::loadFromTxt(const std::string &filename, int rows, int cols) {
  std::ifstream in(filename);
  LOG_IF(FATAL, !in) << "The file cannot be opened " << filename;

  LOG(INFO) << "The matrix has " << rows << " rows and " << cols << "cols";
  for (int r = 0; r < rows; ++r) {
    std::vector<double> row(cols);
    for (int c = 0; c < cols; ++c) {
      float value;
      in >> row[c];
    }
    scores_.push_back(row);
  }
  LOG(INFO) << "Matrix was read";
  in.close();
  rows_ = rows;
  cols_ = cols;
}

double SimilarityMatrix::at(int row, int col) const {
  CHECK(row >= 0 && row < rows_) << "Row outside range " << row;
  CHECK(col >= 0 && col < cols_) << "Col outside range " << col;
  return scores_[row][col];
}

double SimilarityMatrix::getCost(int row, int col) const {
  const double value = this->at(row, col);
  if (std::abs(value) < kEpsilon) {
    return std::numeric_limits<double>::max();
  }
  if (value < 0){
    LOG(WARNING) << "The score value for row:" << row  << " and col:" << col <<" is < 0: " << value<< ". This should not be like this. I will make a positive value of it for now. But please check your values";

  }
  return 1. / std::abs(value);
}

void SimilarityMatrix::loadFromProto(const std::string &filename) {
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
      scores_.push_back(row);
      row.clear();
    }
  }
  cols_ = cost_matrix_proto.cols();
  rows_ = cost_matrix_proto.rows();
  LOG(INFO) << "Read cost matrix with " << rows_ << " rows and " << cols_
            << " cols.";
}
} // namespace localization::database
