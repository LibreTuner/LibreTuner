cmake_minimum_required(VERSION 3.10)
project(LibLibreTuner)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Dependencies
find_package(Threads REQUIRED)
add_subdirectory(depends/serial)

# Options
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
    #add_subdirectory(test)
endif()

# Sources
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lt)

set(ROOT_HEADERS
        ${SOURCE_DIR}/libretuner.h)

file(GLOB_RECURSE NETWORK_CAN_HEADERS ${SOURCE_DIR}/network/can/*.h)
file(GLOB_RECURSE NETWORK_ISOTP_HEADERS ${SOURCE_DIR}/network/isotp/*.h)
file(GLOB_RECURSE NETWORK_UDS_HEADERS ${SOURCE_DIR}/network/uds/*.h)
file(GLOB_RECURSE NETWORK_COMMAND_HEADERS ${SOURCE_DIR}/network/command/*.h)
file(GLOB_RECURSE OS_HEADERS ${SOURCE_DIR}/os/*.h)
file(GLOB_RECURSE J2534_HEADERS ${SOURCE_DIR}/j2534/*.h)
file(GLOB_RECURSE DOWNLOAD_HEADERS ${SOURCE_DIR}/download/*.h)
file(GLOB_RECURSE AUTH_HEADERS ${SOURCE_DIR}/auth/*.h)
file(GLOB_RECURSE SUPPORT_HEADERS ${SOURCE_DIR}/support/*.h)
file(GLOB_RECURSE FLASH_HEADERS ${SOURCE_DIR}/flash/*.h)
file(GLOB_RECURSE DIAGNOSTICS_HEADERS ${SOURCE_DIR}/diagnostics/*.h)
file(GLOB_RECURSE DEFINITION_HEADERS ${SOURCE_DIR}/definition/*.h)
file(GLOB_RECURSE ROM_HEADERS ${SOURCE_DIR}/rom/*.h)
file(GLOB_RECURSE LINK_HEADERS ${SOURCE_DIR}/link/*.h)
file(GLOB_RECURSE SESSION_HEADERS ${SOURCE_DIR}/session/*.h)
file(GLOB_RECURSE DATALOG_HEADERS ${SOURCE_DIR}/datalog/*.h)
file(GLOB_RECURSE SERIALIZE_HEADERS ${SOURCE_DIR}/serialize/*.h)
file(GLOB_RECURSE PROJECT_HEADERS ${SOURCE_DIR}/project/*.h)
file(GLOB_RECURSE BUFFER_HEADERS ${SOURCE_DIR}/buffer/*.h)

set(ROOT_SOURCES
        ${SOURCE_DIR}/context.cpp)
file(GLOB_RECURSE NETWORK_CAN_SOURCES ${SOURCE_DIR}/network/can/*.cpp)
file(GLOB_RECURSE NETWORK_ISOTP_SOURCES ${SOURCE_DIR}/network/isotp/*.cpp)
file(GLOB_RECURSE NETWORK_UDS_SOURCES ${SOURCE_DIR}/network/uds/*.cpp)
file(GLOB_RECURSE NETWORK_COMMAND_SOURCES ${SOURCE_DIR}/network/command/*.cpp)
file(GLOB_RECURSE OS_SOURCES ${SOURCE_DIR}/os/*.cpp)
file(GLOB_RECURSE J2534_SOURCES ${SOURCE_DIR}/j2534/*.cpp)
file(GLOB_RECURSE DOWNLOAD_SOURCES ${SOURCE_DIR}/download/*.cpp)
file(GLOB_RECURSE AUTH_SOURCES ${SOURCE_DIR}/auth/*.cpp)
file(GLOB_RECURSE SUPPORT_SOURCES ${SOURCE_DIR}/support/*.cpp)
file(GLOB_RECURSE FLASH_SOURCES ${SOURCE_DIR}/flash/*.cpp)
file(GLOB_RECURSE DIAGNOSTICS_SOURCES ${SOURCE_DIR}/diagnostics/*.cpp)
file(GLOB_RECURSE DEFINITION_SOURCES ${SOURCE_DIR}/definition/*.cpp)
file(GLOB_RECURSE ROM_SOURCES ${SOURCE_DIR}/rom/*.cpp)
file(GLOB_RECURSE LINK_SOURCES ${SOURCE_DIR}/link/*.cpp)
file(GLOB_RECURSE SESSION_SOURCES ${SOURCE_DIR}/session/*.cpp)
file(GLOB_RECURSE DATALOG_SOURCES ${SOURCE_DIR}/datalog/*.cpp)
file(GLOB_RECURSE SERIALIZE_SOURCES ${SOURCE_DIR}/serialize/*.cpp)
file(GLOB_RECURSE PROJECT_SOURCES ${SOURCE_DIR}/project/*.cpp)
file(GLOB_RECURSE BUFFER_SOURCES ${SOURCE_DIR}/buffer/*.cpp)

set(NETWORK_HEADERS
        ${NETWORK_CAN_HEADERS}
        ${NETWORK_ISOTP_HEADERS}
        ${NETWORK_UDS_HEADERS}
        ${NETWORK_COMMAND_HEADERS})

set(NETWORK_SOURCES
        ${NETWORK_CAN_SOURCES}
        ${NETWORK_ISOTP_SOURCES}
        ${NETWORK_UDS_SOURCES}
        ${NETWORK_COMMAND_SOURCES})

set(HEADERS
        ${ROOT_HEADERS}
        ${NETWORK_HEADERS}
        ${OS_HEADERS}
        ${J2534_HEADERS}
        ${DOWNLOAD_HEADERS}
        ${AUTH_HEADERS}
        ${SUPPORT_HEADERS}
        ${FLASH_HEADERS}
        ${DIAGNOSTICS_HEADERS}
        ${DEFINITION_HEADERS}
        ${ROM_HEADERS}
        ${LINK_HEADERS}
        ${SESSION_HEADERS}
        ${DATALOG_HEADERS}
        ${PROJECT_HEADERS}
        ${BUFFER_HEADERS})

set(SOURCES
        ${ROOT_SOURCES}
        ${NETWORK_SOURCES}
        ${OS_SOURCES}
        ${J2534_SOURCES}
        ${DOWNLOAD_SOURCES}
        ${AUTH_SOURCES}
        ${SUPPORT_SOURCES}
        ${FLASH_SOURCES}
        ${DIAGNOSTICS_SOURCES}
        ${DEFINITION_SOURCES}
        ${ROM_SOURCES}
        ${LINK_SOURCES}
        ${SESSION_SOURCES}
        ${DATALOG_SOURCES}
        ${PROJECT_SOURCES}
        ${BUFFER_SOURCES})


add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${CONAN_LIBS} Qt5::Widgets Qt5::DataVisualization Qt5::Charts Qt5::PrintSupport Serial)
target_include_directories(${PROJECT_NAME} PRIVATE ${SOURCE_DIR})
#target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/cereal/include)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Set warnings
if(MSVC)
    target_compile_options(LibLibreTuner PRIVATE /W4)
else()
    target_compile_options(LibLibreTuner PRIVATE -Wall -Wextra -pedantic -Wno-missing-field-initializers -Wno-missing-braces)
endif()
