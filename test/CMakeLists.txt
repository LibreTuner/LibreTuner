project(test_LibreTuner)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} Catch2::Catch2 LibreTuner_lib)
