set(TESTNAME test_${PROJECT_NAME})

# TODO: Make this with proper folder structure
# add_library(test_utils INTERFACE test_utils.h)
# target_link_libraries(test_utils INTERFACE
#     cnn_feature
#     protos
#     cxx_flags
# )

add_executable(${TESTNAME} 
    database_test.cpp
    feature_buffer_test.cpp
    online_localizer_test.cpp
)
target_link_libraries(${TESTNAME} 
    cnn_feature
    feature_buffer
    cost_matrix_database
    successor_manager
    online_localizer
    list_dir
    protos
    gtest 
    gtest_main
    cxx_flags
)

add_executable( ${TESTNAME}_successor_manager
    successor_manager_test.cpp
)
target_link_libraries(${TESTNAME}_successor_manager
    online_database
    successor_manager
    gtest
    gtest_main
    cxx_flags
)


gtest_discover_tests(${TESTNAME})
gtest_discover_tests(${TESTNAME}_successor_manager)
