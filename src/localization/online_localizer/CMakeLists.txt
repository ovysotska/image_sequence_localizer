add_library(path_element path_element.cpp)
target_link_libraries(path_element
	cxx_flags
	protos
	glog::glog
)

add_library(online_localizer 
	online_localizer.cpp
)
target_link_libraries(online_localizer
	cxx_flags
	path_element
	successor_manager
	node
	timer
	protos
	glog::glog
)