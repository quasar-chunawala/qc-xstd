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

set(CORE_SOURCES "")

set(CORE_HEADERS
    "${XSTD_CORE_ROOT}/concurrency/threadsafe_queue.h"
    "${XSTD_CORE_ROOT}/concurrency/threadsafe_stack.h"
    "${XSTD_CORE_ROOT}/containers/forward_list.h"
    "${XSTD_CORE_ROOT}/containers/vector.h"
    "${XSTD_CORE_ROOT}/containers/soa_vector.h"
    "${XSTD_CORE_ROOT}/lock_free/spsc_queue.h"
    "${XSTD_CORE_ROOT}/smart_pointers/shared_ptr.h"
    "${XSTD_CORE_ROOT}/smart_pointers/unique_ptr.h"
    "${XSTD_CORE_ROOT}/string/string.h"
    "${XSTD_CORE_ROOT}/tagged_union/poly_array.h"
    "${XSTD_CORE_ROOT}/tagged_union/utilities.h"
    "${XSTD_CORE_ROOT}/tagged_union/utils.h"
    "${XSTD_CORE_ROOT}/tagged_union/variant.h"
    "${XSTD_CORE_ROOT}/tagged_union/visit.h"
    "${XSTD_CORE_ROOT}/utilities/exception_guard.h"
)

# Reflections are supported on GCC 16 and above. 
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND "${CMAKE_CXX_COMPILER_VERSION}" VERSION_GREATER_EQUAL 16)
    set(CPP26_REFLECTIONS_SUPPORTED 1)
    list(APPEND CORE_HEADERS "${XSTD_CORE_ROOT}/containers/soa_vector.h")
endif()

# libxstd is a header-only library
add_library(libxstd INTERFACE)
add_library(xstd::xstd ALIAS libxstd)

target_sources(libxstd
    INTERFACE 
        FILE_SET HEADERS
        BASE_DIRS "${XSTD_ROOT}"
        FILES ${CORE_HEADERS}
)


target_compile_features(libxstd INTERFACE cxx_std_${CMAKE_CXX_STANDARD})
if(MSVC)
    target_compile_options(libxstd INTERFACE /W4 /permissive-)
else()
    target_compile_options(libxstd INTERFACE -Wall -Wextra -Wpedantic)
endif()

if(CPP26_REFLECTIONS_SUPPORTED)
    target_compile_options(libxstd INTERFACE -freflection)
endif()