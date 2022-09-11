find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(features)
add_subdirectory(database)
add_subdirectory(successor_manager)
add_subdirectory(online_localizer)
# add_subdirectory(visualizer)
add_subdirectory(tools)
add_subdirectory(relocalizers)

