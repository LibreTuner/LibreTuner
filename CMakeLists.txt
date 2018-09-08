cmake_minimum_required(VERSION 2.8.11)
project(LibreTuner)


# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5DataVisualization CONFIG REQUIRED)
find_package(Qt5Charts CONFIG REQUIRED)
find_package(Threads REQUIRED)

option(Test "Build all tests." OFF)

add_subdirectory(lib/GSL)
add_subdirectory(lib/cparse)
add_subdirectory(src)

#if (Test)
  #add_subdirectory(lib/googletest)
  #enable_testing()
  #include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
  #add_subdirectory(test)
#endif()
