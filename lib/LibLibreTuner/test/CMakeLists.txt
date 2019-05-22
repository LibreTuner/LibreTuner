cmake_minimum_required(VERSION 3.10)

set(SOURCES
        src/main.cpp)

add_executable(tests ${SOURCES})

target_link_libraries(tests LibLibreTuner Catch2::Catch2)
