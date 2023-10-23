find_package(OpenCV REQUIRED)

add_executable(online_localizer_lsh online_localizer_lsh.cpp)
target_link_libraries(online_localizer_lsh
    glog::glog
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
    online_localizer
    cost_matrix_database
    successor_manager
    config_parser
    default_relocalizer
    ${OpenCV_LIBS} 
)

add_executable(kdtree_retrieval kdtree_retrieval.cpp)
target_link_libraries(kdtree_retrieval
    glog::glog
    online_localizer
    cost_matrix_database
    config_parser
    lsh_cv_hashing
    ${OpenCV_LIBS} 
)

