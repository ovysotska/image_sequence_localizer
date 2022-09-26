add_library(list_dir list_dir.cpp)
target_link_libraries(list_dir 	
	cxx_flags
	glog::glog
)

add_library(online_database online_database.cpp)
target_link_libraries(online_database
	timer 
    list_dir
	feature_buffer
    feature_factory
	glog::glog
)
	
add_library(cost_matrix_database cost_matrix_database.cpp)
target_link_libraries(cost_matrix_database 
	online_database
	protos
	glog::glog
)
