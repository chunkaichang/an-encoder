cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(LENGTH 10)
set(REPETITIONS 10)
BUILD_TEST_CASE(bubblesort bubblesort.veri)
BUILD_TEST_CONFIG(bubblesort bubblesort.veri "" ${CMAKE_CURRENT_SOURCE_DIR}/bubblesort.veri.cfg)

set(LENGTH ${LENGTH_PERF})
set(REPETITIONS 100)
BUILD_TEST_CASE(bubblesort bubblesort.perf)
BUILD_TEST_CONFIG(bubblesort bubblesort.perf "" ${CMAKE_CURRENT_SOURCE_DIR}/bubblesort.perf.cfg)

set(LENGTH 10)
set(REPETITIONS 1)
BUILD_TEST_CASE(bubblesort bubblesort.cover)
BUILD_TEST_CONFIG(bubblesort bubblesort.cover "" ${CMAKE_CURRENT_SOURCE_DIR}/bubblesort.cover.cfg)

