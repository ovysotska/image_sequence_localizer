add_library(math_tools math_tools.cpp)
target_link_libraries(math_tools
    cxx_flags
    glog::glog
)

add_library(gmm gmm.cpp)
target_link_libraries(gmm
    math_tools
    cxx_flags
    glog::glog
)

add_library(statistical_test statistical_test.cpp constants.h)
target_link_libraries(statistical_test
    math_tools
    cxx_flags
    glog::glog
)
