cmake_minimum_required(VERSION 2.8)

if ("${CMAKE_BUILD_TYPE}" MATCHES "DEBUG" OR
    "${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  set(CLANG_EMIT_LLVM_OPTS -O0 -g)
else()
  set(CLANG_EMIT_LLVM_OPTS -O0)
endif()

if (MAJORITY_VOTING)
  set(VOTING -DMAJORITY_VOTING)
endif()
if (VERBOSE_VOTING)
  set(VOTING ${VOTING} -DVERBOSE_VOTING)
endif()

# Compile utility libraries:
add_custom_target(mycheck.bc ALL
                  COMMAND ${CLANG} ${VOTING} -c -emit-llvm
                          ${CLANG_EMIT_LLVM_OPTS} -o mycheck.bc
                          ${CMAKE_CURRENT_SOURCE_DIR}/mycheck.c
                  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mycheck.c)

add_custom_target(mycyc.bc ALL
                  COMMAND ${CLANG} ${VOTING} -c -emit-llvm
                          ${CLANG_EMIT_LLVM_OPTS} -o mycyc.bc
                          ${CMAKE_CURRENT_SOURCE_DIR}/mycyc.c
                  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mycyc.c)

