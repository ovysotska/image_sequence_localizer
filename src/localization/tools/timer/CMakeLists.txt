
add_library(timer timer.cpp timer.h)
target_link_libraries(timer
    cxx_flags
    glog::glog
)