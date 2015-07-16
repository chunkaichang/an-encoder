cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(LENGTH 10)
set(REPETITIONS 10)
BUILD_TEST_CASE(bubblesort.random bubblesort.random.veri)
BUILD_TEST_CONFIG(bubblesort.random bubblesort.random.veri "" ${CMAKE_CURRENT_SOURCE_DIR}/bubblesort.random.veri.cfg)

set(LENGTH ${LENGTH_PERF})
set(REPETITIONS 100)
BUILD_TEST_CASE(bubblesort.random bubblesort.random.perf)
BUILD_TEST_CONFIG(bubblesort.random bubblesort.random.perf "" ${CMAKE_CURRENT_SOURCE_DIR}/bubblesort.random.perf.cfg)

set(LENGTH 10)
set(REPETITIONS 1)
BUILD_TEST_CASE(bubblesort.random bubblesort.random.cover)
BUILD_TEST_CONFIG(bubblesort.random bubblesort.random.cover "" ${CMAKE_CURRENT_SOURCE_DIR}/bubblesort.random.cover.cfg)

