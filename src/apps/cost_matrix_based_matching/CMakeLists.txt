find_package(OpenCV REQUIRED)

add_executable(cost_matrix_based_matching_lsh cost_matrix_based_matching_lsh.cpp)
target_link_libraries( cost_matrix_based_matching_lsh
    online_localizer
    cost_matrix_database
    successor_manager
    config_parser
    lsh_cv_hashing
    cnn_feature
    ${OpenCV_LIBS}
)
