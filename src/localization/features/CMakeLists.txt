add_library(cnn_feature cnn_feature.cpp)
target_link_libraries(cnn_feature PUBLIC protos)

add_library(feature_factory feature_factory.cpp)
target_link_libraries(feature_factory PUBLIC cnn_feature)

add_library(feature_buffer feature_buffer.cpp)