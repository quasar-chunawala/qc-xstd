cmake_minimum_required(VERSION 3.3...4.0)

set(CMAKE_CXX_STANDARD 26)              # C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Error, if the compiler doesn't support it

set(CORE_SOURCES
    "${XSTD_CORE_ROOT}/string/string.cpp"
)

set(CORE_HEADERS
    "${XSTD_CORE_ROOT}/concurrency/threadsafe_queue.h"
    "${XSTD_CORE_ROOT}/concurrency/threadsafe_stack.h"
    "${XSTD_CORE_ROOT}/containers/forward_list.h"
    "${XSTD_CORE_ROOT}/containers/vector.h"
    "${XSTD_CORE_ROOT}/lock_free/spsc_queue.h"
    "${XSTD_CORE_ROOT}/smart_pointers/shared_ptr.h"
    "${XSTD_CORE_ROOT}/smart_pointers/unique_ptr.h"
    "${XSTD_CORE_ROOT}/string/string.h"
    "${XSTD_CORE_ROOT}/tagged_union/poly_array.h"
    "${XSTD_CORE_ROOT}/tagged_union/utilities.h"
    "${XSTD_CORE_ROOT}/tagged_union/utils.h"
    "${XSTD_CORE_ROOT}/tagged_union/variant.h"
    "${XSTD_CORE_ROOT}/tagged_union/visit.h"
)

add_library(libxstd SHARED)

target_sources(libxstd
    PRIVATE 
        ${CORE_SOURCES}
        containers/soa_vector.h
        utilities/exception_guard.h
    PUBLIC 
        FILE_SET HEADERS
        BASE_DIRS "${XSTD_ROOT}"
        FILES ${CORE_HEADERS}
)


target_compile_features(libxstd PRIVATE cxx_std_26)
target_compile_options(libxstd PRIVATE -Wall -Wpedantic -freflection)
