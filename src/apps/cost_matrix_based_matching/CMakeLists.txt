find_package(OpenCV REQUIRED)

add_executable(online_localizer_lsh online_localizer_lsh.cpp)
target_link_libraries(online_localizer_lsh
    glog::glog
    path_element
    online_localizer
    cost_matrix_database
    successor_manager
    config_parser
    lsh_cv_hashing
    cnn_feature
    ${OpenCV_LIBS}
   
)

add_executable(cost_matrix_no_hashing cost_matrix_no_hashing.cpp)
target_link_libraries(cost_matrix_no_hashing
    glog::glog
    path_element
    online_localizer
    cost_matrix_database
    successor_manager
    config_parser
    default_relocalizer
    ${OpenCV_LIBS} 
)

add_executable(localization_by_hashing localization_by_hashing.cpp)
target_link_libraries(localization_by_hashing
    glog::glog
    cnn_feature
    path_element
    cost_matrix_database
    config_parser
    lsh_cv_hashing
    ${OpenCV_LIBS} 
)

add_executable(scan_context_matching scan_context_matching.cpp)
target_link_libraries(scan_context_matching
    cost_matrix 
    config_parser
    online_database
    successor_manager
    online_localizer
    scan_context_relocalizer 
    glog::glog
)

add_executable(compute_scan_context_cost_matrix compute_scan_context_cost_matrix.cpp)
target_link_libraries(compute_scan_context_cost_matrix
    cost_matrix 
    glog::glog
)

