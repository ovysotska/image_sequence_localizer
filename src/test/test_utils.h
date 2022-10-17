#ifndef TEST_TEST_UTILS_H_
#define TEST_TEST_UTILS_H_

#include "database/cost_matrix_database.h"
#include "database/online_database.h"
#include "localization_protos.pb.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace test {

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

  const std::filesystem::path filename = dir / name.c_str();
  std::fstream output(filename,
                      std::ios::out | std::ios::trunc | std::ios::binary);

  ASSERT_TRUE(feature_proto.SerializeToOstream(&output));
  output.close();
}

inline std::filesystem::path createDataForOnlineDatabase() {
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
inline void clearDataForOnlineDatabase(const std::filesystem::path &path) {
  std::filesystem::remove_all(path);
}
} // namespace test

#endif // TEST_TEST_UTILS_H_