cmake_minimum_required(VERSION 4.1)

project(meta-reflection
    LANGUAGES CXX
)

set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(tests)
