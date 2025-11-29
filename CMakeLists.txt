cmake_minimum_required(VERSION 4.1)

project(meta-reflection
    LANGUAGES CXX
)

set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory(include)
add_subdirectory(tests)
