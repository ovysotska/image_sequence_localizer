set(TESTNAME test_${PROJECT_NAME})

add_executable(${TESTNAME} dummy_test.cpp dummy_test_2.cpp)
target_link_libraries(${TESTNAME} gtest gtest_main)

gtest_discover_tests(${TESTNAME})
