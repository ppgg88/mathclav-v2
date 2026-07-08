cmake_minimum_required(VERSION 3.24)
project(MathClav VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MATHCLAV_BUILD_TESTS "Build the core unit test suite" ON)
option(MATHCLAV_BUILD_APP "Build the Qt6 GUI application (requires Qt6)" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(Warnings)

enable_testing()

add_subdirectory(core)

if(MATHCLAV_BUILD_APP)
    add_subdirectory(app)
endif()
