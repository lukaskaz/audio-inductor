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
add_dependencies(${PROJECT_NAME} liblogger)
add_dependencies(${PROJECT_NAME} libdisplays)
add_dependencies(${PROJECT_NAME} libpwm)
add_dependencies(${PROJECT_NAME} libadc)
add_dependencies(${PROJECT_NAME} libservo)
add_dependencies(${PROJECT_NAME} libledrgb)
add_dependencies(${PROJECT_NAME} libgpio)
add_dependencies(${PROJECT_NAME} libspeech)
add_dependencies(${PROJECT_NAME} cavaproject)

target_link_libraries(${PROJECT_NAME}
    graphs
    shell
    logger
    displays
    pwm
    adc
    servo
    ledrgb
    gpio
    speech
)
