cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(LENGTH 1024)
set(REPETITIONS 10)
BUILD_TEST_CASE(ptrarith ptrarith.veri)
BUILD_TEST_CONFIG(ptrarith ptrarith.veri "" ${CMAKE_CURRENT_SOURCE_DIR}/ptrarith.veri.cfg)

set(LENGTH ${LENGTH_PERF})
set(REPETITIONS 100)
BUILD_TEST_CASE(ptrarith ptrarith.perf)
BUILD_TEST_CONFIG(ptrarith ptrarith.perf "" ${CMAKE_CURRENT_SOURCE_DIR}/ptrarith.perf.cfg)

set(LENGTH 32)
set(REPETITIONS 1)
BUILD_TEST_CASE(ptrarith ptrarith.cover)
BUILD_TEST_CONFIG(ptrarith ptrarith.cover "" ${CMAKE_CURRENT_SOURCE_DIR}/ptrarith.cover.cfg)

