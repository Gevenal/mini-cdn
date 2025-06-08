cmake_minimum_required(VERSION 3.18)
project(mini_cdn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)

# ----------------------------------------------------------------------------
# 1. Cache library
# ----------------------------------------------------------------------------
add_library(cache
    src/LruCache.cpp
    src/ThreadPool.cpp
)
target_include_directories(cache PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# ----------------------------------------------------------------------------
# 2. Main executable
# ----------------------------------------------------------------------------
add_executable(mini_cdn
    src/main.cpp
    src/EchoProxy.cpp
    src/SocketUtils.cpp
    src/HttpProxy.cpp
    src/HttpParser.cpp     
    src/Resolver.cpp 
)
target_link_libraries(mini_cdn PRIVATE cache)

# ----------------------------------------------------------------------------
# 3. GoogleTest
# ----------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ----------------------------------------------------------------------------
# 4. Test: LruCache
# ----------------------------------------------------------------------------
add_executable(test_lru_cache
    tests/test_lru_cache.cpp
)
target_include_directories(test_lru_cache PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(test_lru_cache PRIVATE cache gtest_main)
add_test(NAME LruCacheTests COMMAND test_lru_cache)

# ----------------------------------------------------------------------------
# 5. Test: HttpParser
# ----------------------------------------------------------------------------
add_executable(test_http_parser
    tests/test_http_parser.cpp
    src/HttpParser.cpp
)
target_include_directories(test_http_parser PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(test_http_parser PRIVATE cache gtest_main)
add_test(NAME HttpParserTests COMMAND test_http_parser)

# ----------------------------------------------------------------------------
# 5. Test: Resolver
# ----------------------------------------------------------------------------

add_executable(test_resolver
    tests/test_resolver.cpp
    src/Resolver.cpp        
)
target_include_directories(test_resolver PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(test_resolver PRIVATE gtest_main)
add_test(NAME ResolverTests COMMAND test_resolver)

# ----------------------------------------------------------------------------
# 7. Test: ThreadPool
# ----------------------------------------------------------------------------
add_executable(test_thread_pool
    tests/test_thread_pool.cpp
)
target_include_directories(test_thread_pool PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(test_thread_pool PRIVATE cache gtest_main)
add_test(NAME ThreadPoolTests COMMAND test_thread_pool)
