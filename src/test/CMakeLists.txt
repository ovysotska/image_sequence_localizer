set(TESTNAME test_${PROJECT_NAME})

add_executable(${TESTNAME} 
    feature_buffer_test.cpp
)
target_link_libraries(${TESTNAME} 
    cnn_feature
    feature_buffer
    protos
    ${OpenCV_LIBS}
    gtest 
    gtest_main
)

gtest_discover_tests(${TESTNAME})
