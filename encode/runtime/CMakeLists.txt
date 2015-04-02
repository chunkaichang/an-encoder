cmake_minimum_required(VERSION 2.8)


add_custom_target(anlib.bc ALL
                  COMMAND clang -O0 -g -c -emit-llvm -o anlib.bc
                          ${CMAKE_CURRENT_SOURCE_DIR}/anlib.c
                  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/anlib.c)

add_custom_target(rdtsc.bc ALL
                  COMMAND clang -O0 -g -c -emit-llvm -o rdtsc.bc
                          ${CMAKE_CURRENT_SOURCE_DIR}/rdtsc.c
                  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rdtsc.c)
