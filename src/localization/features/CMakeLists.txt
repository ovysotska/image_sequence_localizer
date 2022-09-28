add_library(cnn_feature cnn_feature.cpp)
target_link_libraries(cnn_feature 
    PUBLIC 
    protos
    glog::glog
)

add_library(feature_factory feature_factory.cpp)
target_link_libraries(feature_factory 
    PUBLIC 
    cnn_feature 
    glog::glog
)

add_library(feature_buffer feature_buffer.cpp)
target_link_libraries(feature_buffer glog::glog cxx_flags)
