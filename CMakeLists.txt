cmake_minimum_required(VERSION 3.3...4.0)

project(qc_xstd VERSION 1.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(set_xstd_dirs)

# #################################################################
# Global Settings
# #################################################################
include(architecture)
include(check_compiler)
set(arch "")
xstd_get_target(arch)

set(compiler "")
set(version "")
xstd_check_compiler(compiler version)

if(${compiler} STREQUAL "GCC" AND ${version} VERSION_GREATER_EQUAL 16)
  set(XSTD_CXX_STANDARD 26)
else()
  set(XSTD_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_STANDARD ${XSTD_CXX_STANDARD})              # C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)                       # Error, if the compiler doesn't support it
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_CLANG_TIDY "" OFF)

# #################################################################
# Project components
# #################################################################
enable_testing()

add_subdirectory(core)
add_subdirectory(tests)