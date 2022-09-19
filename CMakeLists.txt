cmake_minimum_required(VERSION 3.10)

project(image_sequence_localizer)

include(GoogleTest)
include(cmake/UpdateSubmodules.cmake)


option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    add_subdirectory(src/external/googletest)
endif()

# Options for glog
option(WITH_GFLAGS "Use gflags" OFF)
option(WITH_GTEST "Use Google Test" OFF)
option(BUILD_TESTING "Use Build testing" OFF)
add_subdirectory(src/external/glog)
add_subdirectory(src)

