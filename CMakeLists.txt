# build script for the Mini-CDN project
cmake_minimum_required(VERSION 3.18)
project(mini_cdn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic)

# --- Your executable ---
add_executable(mini_cdn
    src/main.cpp
)

# --- GoogleTest Setup ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(unit_tests
    tests/sample_test.cpp
)
target_link_libraries(unit_tests
    gtest_main
)
add_test(NAME all_tests COMMAND unit_tests)
