cmake_minimum_required(VERSION 3.1)
project(LibreTuner)


# Find the QtWidgets library
find_package(Qt5 COMPONENTS Widgets Charts PrintSupport DataVisualization REQUIRED)
find_package(Threads REQUIRED)

option(Test "Build all tests." OFF)

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Build yaml-cpp tests")

add_subdirectory(lib/QHexView)
add_subdirectory(lib/lua)
#add_subdirectory(lib/sol2)
#add_subdirectory(lib/catch)

add_subdirectory(lib/LibLibreTuner)

add_subdirectory(src)
# add_subdirectory(test)
