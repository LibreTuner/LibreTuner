cmake_minimum_required(VERSION 3.1)
project(LibreTuner)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Find the QtWidgets library
find_package(Qt5 COMPONENTS Core Widgets Charts PrintSupport DataVisualization REQUIRED)
find_package(Threads REQUIRED)

#option(Test "Build all tests." OFF)

add_subdirectory(LibLibreTuner)
add_subdirectory(ui)

#add_subdirectory(lib/QHexView)
#add_subdirectory(lib/lua)
#add_subdirectory(lib/sol2)
#add_subdirectory(lib/catch)

#add_subdirectory(lib/LibLibreTuner)

#add_subdirectory(gui)
# add_subdirectory(test)


#add_executable(LibreTuner main.cpp)
#target_link_libraries(LibreTuner ${CONAN_LIBS} LibLibreTuner)
