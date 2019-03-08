cmake_minimum_required(VERSION 2.8.11)
project(LibreTuner)


# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5DataVisualization CONFIG REQUIRED)
find_package(Qt5Charts CONFIG REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Threads REQUIRED)

option(Test "Build all tests." OFF)

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Build yaml-cpp tests")

add_subdirectory(lib/cparse)
add_subdirectory(lib/yaml)
add_subdirectory(lib/QHexView)
add_subdirectory(lib/lua)
add_subdirectory(lib/sol2)
add_subdirectory(lib/catch)


add_subdirectory(lib/LibLibreTuner)

add_library(QHexView ${QHEXVIEW_SOURCES})

target_include_directories(QHexView PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/QHexView)
target_link_libraries(QHexView Qt5::Widgets)

add_subdirectory(src)
add_subdirectory(test)
