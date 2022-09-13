find_package(Protobuf REQUIRED)

# TODO: is opencv really required?
find_package( OpenCV REQUIRED )

## libyaml-cpp
find_package (yaml-cpp REQUIRED)

add_library(cxx_flags INTERFACE)
target_compile_features(cxx_flags INTERFACE cxx_std_17)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS localization_protos.proto)

set(PROTOS_FOLDER ${CMAKE_SOURCE_DIR}/src/python/protos)
file(MAKE_DIRECTORY ${PROTOS_FOLDER})
protobuf_generate_python(PROTO_PY localization_protos.proto PROTOC_OUT_DIR ${PROTOS_FOLDER})

add_library(protos ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(protos ${Protobuf_LIBRARIES})

include_directories(localization)
add_subdirectory(localization)
add_subdirectory(apps)

# # Generate the python proto files
add_custom_target(proto_python ALL DEPENDS ${PROTO_PY})

if(PACKAGE_TESTS)
    add_subdirectory(test)
endif()