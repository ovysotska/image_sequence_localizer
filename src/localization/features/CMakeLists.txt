add_library(cnn_feature cnn_feature.cpp)
target_link_libraries(cnn_feature 
    PUBLIC 
    protos
    glog::glog
)

find_package (Eigen3 3.3 REQUIRED NO_MODULE)

add_library(scan_context scan_context.cpp)
target_link_libraries(scan_context 
    glog::glog
    cxx_flags
    Eigen3::Eigen
)

add_library(feature_factory feature_factory.cpp)
target_link_libraries(feature_factory 
    PUBLIC 
    cnn_feature
    scan_context 
    glog::glog

)

add_library(feature_buffer feature_buffer.cpp)
target_link_libraries(feature_buffer glog::glog cxx_flags)


 
