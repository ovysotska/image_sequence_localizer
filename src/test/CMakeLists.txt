set(TESTNAME test_${PROJECT_NAME})

add_executable(${TESTNAME} 
    database_test.cpp
    feature_buffer_test.cpp
)
target_link_libraries(${TESTNAME} 
    cnn_feature
    feature_buffer
    cost_matrix_database
    protos
    gtest 
    gtest_main
    cxx_flags
)

add_executable( ${TESTNAME}_successor_manager
    successor_manager_test.cpp
)
target_link_libraries(${TESTNAME}_successor_manager
    gtest
    gtest_main
    cxx_flags
)


gtest_discover_tests(${TESTNAME})
