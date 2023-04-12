add_library(node node.cpp)
target_link_libraries(node
cxx_flags
)

add_library(successor_manager successor_manager.cpp)
target_link_libraries(successor_manager
    node 
    cxx_flags
    glog::glog
)
