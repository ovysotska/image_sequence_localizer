add_library(online_localizer 
	online_localizer.cpp 
	path_element.cpp
)
target_link_libraries(online_localizer
	cxx_flags
	successor_manager
	node
	timer
	protos
)