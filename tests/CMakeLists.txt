cmake_minimum_required(VERSION 3.3...4.0)

include(architecture)
include(check_compiler)
set(arch "")
xstd_get_target(arch)
message(STATUS "Architecture : ${arch}")

set(compiler "")
set(version "")
xstd_check_compiler(compiler version)
message(STATUS "Compiler: ${compiler}")
message(STATUS "Version: ${version}")

if(${compiler} STREQUAL "GCC" AND ${version} VERSION_GREATER_EQUAL 16)
  set(XSTD_CXX_STANDARD 26)
else()
  set(XSTD_CXX_STANDARD 23)
endif()

set(CMAKE_CXX_STANDARD ${XSTD_CXX_STANDARD})              # C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Error, if the compiler doesn't support it

include(FetchContent)

fetchcontent_declare(googletest
GIT_REPOSITORY
        https://github.com/google/googletest.git
    GIT_TAG main
)

fetchcontent_makeavailable(googletest)


message("XSTD_TESTS_ROOT : ${XSTD_TESTS_ROOT}")

set(XSTD_TESTS_HEADERS
    ""
)

set(XSTD_TESTS_SOURCES
    "${XSTD_TESTS_ROOT}/forward_list_test/forward_list_test.cpp"
    "${XSTD_TESTS_ROOT}/shared_ptr_test/shared_ptr_test.cpp"
    "${XSTD_TESTS_ROOT}/spsc_queue_test/spsc_queue_test.cpp"
    "${XSTD_TESTS_ROOT}/string_test/string_test.cpp"
    "${XSTD_TESTS_ROOT}/threadsafe_queue_test/threadsafe_queue_test.cpp"
    "${XSTD_TESTS_ROOT}/threadsafe_stack_test/threadsafe_stack_test.cpp"
    "${XSTD_TESTS_ROOT}/unique_ptr_test/unique_ptr_test.cpp"
    "${XSTD_TESTS_ROOT}/variant_test/variant_test.cpp"
    "${XSTD_TESTS_ROOT}/vector_test/vector_test.cpp"
)

add_executable(xstd_tests)

target_sources(xstd_tests
    PRIVATE
        ${XSTD_TESTS_SOURCES}
        ${XSTD_TESTS_HEADERS}
)

target_link_libraries(xstd_tests
    PRIVATE
        xstd::xstd
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(xstd_tests)

