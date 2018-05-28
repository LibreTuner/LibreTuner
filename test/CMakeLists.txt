cmake_minimum_required(VERSION 2.8)

project(LibreTuner)

add_executable(LibreTuner_test isotp.cpp)
target_link_libraries(LibreTuner_test gtest gtest_main LibreTuner_lib)
