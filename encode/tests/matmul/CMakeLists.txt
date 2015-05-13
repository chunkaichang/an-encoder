cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(LENGTH 1024)
BUILD_TEST_CASE(matmul matmul.veri)
BUILD_TEST_CONFIG(matmul matmul.veri "" ${CMAKE_CURRENT_SOURCE_DIR}/matmul.veri.cfg)

set(LENGTH ${LENGTH_PERF})
BUILD_TEST_CASE(matmul matmul.perf)
BUILD_TEST_CONFIG(matmul matmul.perf "" ${CMAKE_CURRENT_SOURCE_DIR}/matmul.perf.cfg)

set(LENGTH 1024)
BUILD_TEST_CASE(matmul matmul.cover)
BUILD_TEST_CONFIG(matmul matmul.cover "" ${CMAKE_CURRENT_SOURCE_DIR}/matmul.cover.cfg)


