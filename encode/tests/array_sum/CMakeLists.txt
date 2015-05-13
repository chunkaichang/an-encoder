cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(LENGTH 100)
set(REPETITIONS 10)
BUILD_TEST_CASE(array_sum array_sum.veri)
BUILD_TEST_CONFIG(array_sum array_sum.veri "" ${CMAKE_CURRENT_SOURCE_DIR}/array_sum.veri.cfg)

set(LENGTH ${LENGTH_PERF})
set(REPETITIONS 100)
BUILD_TEST_CASE(array_sum array_sum.perf)
BUILD_TEST_CONFIG(array_sum array_sum.perf "" ${CMAKE_CURRENT_SOURCE_DIR}/array_sum.perf.cfg)

set(LENGTH 10)
set(REPETITIONS 1)
BUILD_TEST_CASE(array_sum array_sum.cover)
BUILD_TEST_CONFIG(array_sum array_sum.cover "" ${CMAKE_CURRENT_SOURCE_DIR}/array_sum.cover.cfg)
