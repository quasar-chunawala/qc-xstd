cmake_minimum_required(VERSION 3.3...4.0)

project(qc_xstd VERSION 1.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(set_xstd_dirs)

# #################################################################
# Global Settings
# #################################################################
set(CMAKE_CXX_STANDARD 26) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_CLANG_TIDY "" OFF)

# #################################################################
# Project components
# #################################################################
enable_testing()

add_subdirectory(core)
add_subdirectory(tests)