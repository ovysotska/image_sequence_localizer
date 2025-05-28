find_package(OpenCV REQUIRED)

add_executable(feature_based_localizer feature_based_localizer.cpp)
target_link_libraries(feature_based_localizer
    cxx_flags
    glog::glog
    path_element
    online_database
    online_localizer
    successor_manager
    config_parser
    lsh_cv_hashing
    cnn_feature
    ${OpenCV_LIBS}
)