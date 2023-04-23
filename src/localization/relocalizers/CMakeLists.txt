add_library(lsh_cv_hashing lsh_cv_hashing.cpp)
target_link_libraries(lsh_cv_hashing 
    timer
    online_database
    ${OpenCV_LIBS}
    cxx_flags
    glog::glog
)

add_library(default_relocalizer default_relocalizer.cpp)
target_link_libraries(default_relocalizer
    cxx_flags
    glog::glog
)

find_package (Eigen3 3.3 REQUIRED NO_MODULE)

add_library(scan_context_relocalizer scan_context_relocalizer.cpp)
target_link_libraries(scan_context_relocalizer 
    online_database
    scan_context
    Eigen3::Eigen
    ${OpenCV_LIBS}
    cxx_flags
    glog::glog
)
