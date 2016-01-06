cmake_minimum_required(VERSION 2.8)

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

set(ENCODE_BIN_DIR ${CMAKE_BINARY_DIR}/bin)

set(CODE_VALUE_A "58659" CACHE STRING "")

set(MAJORITY_VOTING OFF CACHE BOOL "")
set(VERBOSE_VOTING  OFF CACHE BOOL "")

set(LOG_ACCU OFF CACHE BOOL "")

set(USE_CXX11_ABI "0" CACHE STRING "")

add_subdirectory(src)
add_subdirectory(runtime)
add_subdirectory(tests)
add_subdirectory(scripts)
