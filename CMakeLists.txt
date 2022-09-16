cmake_minimum_required(VERSION 3.10)

project(image_sequence_localizer)


include(cmake/UpdateSubmodules.cmake)
include(GoogleTest)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    add_subdirectory(src/external/googletest)
endif()

add_subdirectory(src)

