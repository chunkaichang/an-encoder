cmake_minimum_required(VERSION 2.8)

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

set(ENCODE_BIN_DIR ${CMAKE_BINARY_DIR}/bin)

set(MAJORITY_VOTING OFF CACHE BOOL "")
set(VERBOSE_VOTING OFF CACHE BOOL "")


set(CHECK_BEFORE ON CACHE BOOL "")
set(CHECK_AFTER  OFF CACHE BOOL "")
set(CHECK_BB     OFF CACHE BOOL "")
set(CHECK_FUN    OFF CACHE BOOL "")

set(ACCU_ARGS OFF CACHE BOOL "")
set(ACCU_RES  OFF CACHE BOOL "")

add_subdirectory(src)
add_subdirectory(runtime)
add_subdirectory(tests)
add_subdirectory(scripts)
