#ifndef TEST_TEST_UTILS_H_
#define TEST_TEST_UTILS_H_

#include "database/list_dir.h"
#include "features/cnn_feature.h"
#include "features/feature_factory.h"
#include "localization_protos.pb.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace test {

constexpr auto kTestEpsilon = 1e-06;

namespace fs = std::filesystem;

inline image_sequence_localizer::Feature
createFeatureProto(const std::vector<double> &values) {
  image_sequence_localizer::Feature feature_proto;
  feature_proto.set_type("debug");
  feature_proto.set_size(values.size());
  for (double value : values) {
    feature_proto.add_values(value);
  }
  return feature_proto;
}

inline void
createFeatureFile(const std::filesystem::path &dir, const std::string &name,
                  const image_sequence_localizer::Feature &feature_proto) {

  const std::filesystem::path filename = dir / name;
  std::fstream output(filename,
                      std::ios::out | std::ios::trunc | std::ios::binary);

  ASSERT_TRUE(feature_proto.SerializeToOstream(&output));
  output.close();
}

inline std::filesystem::path createFeatures() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  std::filesystem::path tmp_dir =
      std::filesystem::temp_directory_path() / "features";
  std::filesystem::create_directories(tmp_dir);
  createFeatureFile(tmp_dir, "feature_0.Feature.pb",
                    createFeatureProto({0, 1, 2, 3}));
  createFeatureFile(tmp_dir, "feature_1.Feature.pb",
                    createFeatureProto({3, 4, 5, 6}));
  createFeatureFile(tmp_dir, "feature_2.Feature.pb",
                    createFeatureProto({3, 2, 1, 0}));
  createFeatureFile(tmp_dir, "feature_3.Feature.pb",
                    createFeatureProto({0, 1, 2.5, 3}));
  return tmp_dir;
}
inline void clearDataUnderPath(const std::filesystem::path &path) {
  std::filesystem::remove_all(path);
}

const std::vector<std::vector<double>> kCostMatrix = {
    {1, 0.922225, 0.285714, 0.99449},
    {0.922225, 1, 0.634029, 0.922876},
    {0.285714, 0.634029, 1, 0.298347},
    {0.99449, 0.922876, 0.298347, 1}};

inline image_sequence_localizer::CostMatrix
computeCostMatrixProto(const fs::path &queryDir, const fs::path &refDir) {

  const std::vector<std::string> queryFiles =
      localization::database::listProtoDir(queryDir, ".Feature");
  const std::vector<std::string> refFiles =
      localization::database::listProtoDir(refDir, ".Feature");
  image_sequence_localizer::CostMatrix cost_matrix;

  for (const auto &query : queryFiles) {
    const auto queryFeature = localization::features::CnnFeature(query);
    for (const auto &ref : refFiles) {
      cost_matrix.add_values(queryFeature.computeSimilarityScore(
          localization::features::CnnFeature(ref)));
    }
  }

  cost_matrix.set_cols(refFiles.size());
  cost_matrix.set_rows(queryFiles.size());
  return cost_matrix;
}

} // namespace test

#endif // TEST_TEST_UTILS_H_