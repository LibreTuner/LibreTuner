cmake_minimum_required(VERSION 2.8.11)
project(LibreTune)


# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Threads REQUIRED)


add_subdirectory(src)

