# build script for the Mini-CDN project
cmake_minimum_required(VERSION 3.18)
project(mini_cdn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic)

# ---------------------------------------------------------------------------
# 1) Cache library (must come BEFORE anything that links to it)
# ---------------------------------------------------------------------------
add_library(cache
    src/LruCache.cpp
)
target_include_directories(cache PUBLIC
    ${PROJECT_SOURCE_DIR}/include       # <- expose headers
)

# ---------------------------------------------------------------------------
# 2) Main executable — link it to the cache
# ---------------------------------------------------------------------------
add_executable(mini_cdn
    src/main.cpp
    src/EchoProxy.cpp         
    src/SocketUtils.cpp 
)
target_link_libraries(mini_cdn PRIVATE cache)

# ---------------------------------------------------------------------------
# 3) GoogleTest setup (fetch BEFORE test targets)
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP true       # silence CMP0135 warning
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ---------------------------------------------------------------------------
# 4) Unit-test executable (one target is enough)
# ---------------------------------------------------------------------------
add_executable(test_lru_cache
    tests/test_lru_cache.cpp            # make sure this file exists
)
target_link_libraries(test_lru_cache
    PRIVATE cache gtest_main
)
add_test(NAME LruCacheTests COMMAND test_lru_cache)
