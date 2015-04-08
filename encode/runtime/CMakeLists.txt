cmake_minimum_required(VERSION 2.8)

# Compile with optimizations turned off since we do not want the global
# Variable 'A' in "anlib.c" to be optimized away at this stage:
set(CLANG_OPTS -O0)
if ("${CMAKE_BUILD_TYPE}" MATCHES "DEBUG" OR
    "${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  set(CLANG_OPTS ${CLANG_OPTS} -g)
endif()


add_custom_target(anlib.bc ALL
                   COMMAND clang ${CLANG_OPTS} -c -emit-llvm -o anlib.bc
                          ${CMAKE_CURRENT_SOURCE_DIR}/anlib.c
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/anlib.c
                   WORKING_DIRECTORY ${ENCODE_BIN_DIR})
