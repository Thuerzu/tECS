cmake_minimum_required(VERSION 3.20)

add_executable(Benchmark test.cpp)

target_link_libraries(Benchmark PRIVATE tECS PUBLIC tBenchmark)

set_target_properties(Benchmark
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

target_compile_definitions(Benchmark
PRIVATE
    LOGGING_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../logs"
)

add_test(NAME BENCHMARK COMMAND Benchmark)