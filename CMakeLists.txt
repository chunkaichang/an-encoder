cmake_minimum_required(VERSION 2.8)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

set(CLANG   ${LLVM_INSTALL_PREFIX}/bin/clang)
set(CMAKE_C_COMPILER   ${CLANG})
message("Using C compiler: ${CLANG}")

set(CLANGXX ${LLVM_INSTALL_PREFIX}/bin/clang++)
set(CMAKE_CXX_COMPILER ${CLANGXX})
message("Using C++ compiler: ${CLANGXX}")

set(LINK   ${LLVM_INSTALL_PREFIX}/bin/llvm-link)

add_subdirectory(encode)
