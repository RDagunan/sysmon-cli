cmake_minimum_required(VERSION 3.16)
project(SysMonCLI LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_executable(sysmon
    src/main.cpp
    src/metrics.cpp
)
target_include_directories(sysmon PRIVATE include)

add_executable(metrics_test
    tests/metrics_test.cpp
    src/metrics.cpp
)

target_include_directories(metrics_test PRIVATE include)

add_test(NAME memory_parser COMMAND metrics_test)

add_executable(cpu_test
    tests/cpu_test.cpp
    src/metrics.cpp
)
target_include_directories(cpu_test PRIVATE include)

add_test(NAME cpu_parser COMMAND cpu_test)