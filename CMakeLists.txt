cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(audioinductor)

include(cmake/dependencies.cmake)
include(cmake/flags.cmake)

option(RUN_TESTS "Creates and runs unit tests for the project" OFF)

if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

include_directories(inc)
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})
add_dependencies(${PROJECT_NAME} libgraphs)
add_dependencies(${PROJECT_NAME} libshellcmd)
add_dependencies(${PROJECT_NAME} cavaproject)

target_link_libraries(${PROJECT_NAME}
    graphs
    shellcmd
    ${WIRINGPI_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

