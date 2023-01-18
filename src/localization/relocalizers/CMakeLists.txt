add_library(lsh_cv_hashing lsh_cv_hashing.cpp)
target_link_libraries(lsh_cv_hashing 
    timer
    online_database
    ${OpenCV_LIBS}
    cxx_flags
	glog::glog
)