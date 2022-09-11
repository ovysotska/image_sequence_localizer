add_library(cnn_feature cnn_feature.cpp)

add_library(feature_factory feature_factory.cpp)
target_link_libraries( feature_factory 
	cnn_feature
)

add_library(feature_buffer feature_buffer.cpp)