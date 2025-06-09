# CMake generated Testfile for 
# Source directory: /Users/letmefeel/Documents/project/computer networking/Mini-CDN Proxy/mini-cdn
# Build directory: /Users/letmefeel/Documents/project/computer networking/Mini-CDN Proxy/mini-cdn/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(LruCacheTests "/Users/letmefeel/Documents/project/computer networking/Mini-CDN Proxy/mini-cdn/build/test_lru_cache")
set_tests_properties(LruCacheTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/letmefeel/Documents/project/computer networking/Mini-CDN Proxy/mini-cdn/CMakeLists.txt;61;add_test;/Users/letmefeel/Documents/project/computer networking/Mini-CDN Proxy/mini-cdn/CMakeLists.txt;0;")
add_test(HttpParserTests "/Users/letmefeel/Documents/project/computer networking/Mini-CDN Proxy/mini-cdn/build/test_http_parser")
set_tests_properties(HttpParserTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/letmefeel/Documents/project/computer networking/Mini-CDN Proxy/mini-cdn/CMakeLists.txt;72;add_test;/Users/letmefeel/Documents/project/computer networking/Mini-CDN Proxy/mini-cdn/CMakeLists.txt;0;")
add_test(ResolverTests "/Users/letmefeel/Documents/project/computer networking/Mini-CDN Proxy/mini-cdn/build/test_resolver")
set_tests_properties(ResolverTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/letmefeel/Documents/project/computer networking/Mini-CDN Proxy/mini-cdn/CMakeLists.txt;84;add_test;/Users/letmefeel/Documents/project/computer networking/Mini-CDN Proxy/mini-cdn/CMakeLists.txt;0;")
add_test(ThreadPoolTests "/Users/letmefeel/Documents/project/computer networking/Mini-CDN Proxy/mini-cdn/build/test_thread_pool")
set_tests_properties(ThreadPoolTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/letmefeel/Documents/project/computer networking/Mini-CDN Proxy/mini-cdn/CMakeLists.txt;94;add_test;/Users/letmefeel/Documents/project/computer networking/Mini-CDN Proxy/mini-cdn/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
