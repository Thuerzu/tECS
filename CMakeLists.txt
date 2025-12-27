cmake_minimum_required(VERSION 3.20)

project(tECS LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(tECS INTERFACE)

target_include_directories(tECS 
INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(tECS
PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

enable_testing()
add_subdirectory(test)