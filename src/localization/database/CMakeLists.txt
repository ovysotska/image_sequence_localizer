add_library(list_dir list_dir.cpp)
target_link_libraries(list_dir 	
    cxx_flags
    glog::glog
)

add_library(similarity_matrix similarity_matrix.cpp)
target_link_libraries(similarity_matrix
    feature_factory
    list_dir
    protos
    glog::glog
)

add_library(online_database online_database.cpp)
target_link_libraries(online_database
    timer 
    list_dir
    feature_buffer
    feature_factory
    glog::glog
    similarity_matrix
)

add_library(similarity_matrix_database similarity_matrix_database.cpp)
target_link_libraries(similarity_matrix_database
    timer 
    list_dir
    feature_buffer
    feature_factory
    glog::glog
    similarity_matrix
)

