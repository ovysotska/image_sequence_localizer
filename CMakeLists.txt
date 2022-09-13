cmake_minimum_required(VERSION 3.10)

project(image_sequence_localizer)

find_package(Protobuf REQUIRED)
set_target_properties(protobuf::protoc PROPERTIES IMPORTED_GLOBAL TRUE)

include(GoogleTest)
include(cmake/UpdateSubmodules.cmake)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    add_subdirectory(src/external/googletest)
endif()

add_subdirectory(src)

