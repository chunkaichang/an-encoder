cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(LENGTH 100)
set(REPETITIONS 10)
BUILD_TEST_CASE(quicksort.random-binary quicksort.random-binary.veri)
BUILD_TEST_CONFIG(quicksort.random-binary quicksort.random-binary.veri "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.random-binary.veri.cfg)

set(LENGTH ${LENGTH_PERF})
set(REPETITIONS 100)
BUILD_TEST_CASE(quicksort.random-binary quicksort.random-binary.perf)
BUILD_TEST_CONFIG(quicksort.random-binary quicksort.random-binary.perf "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.random-binary.perf.cfg)

set(LENGTH 10)
set(REPETITIONS 1)
BUILD_TEST_CASE(quicksort.random-binary quicksort.random-binary.cover)
BUILD_TEST_CONFIG(quicksort.random-binary quicksort.random-binary.cover "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.random-binary.cover.cfg)

unset(LENGTH)
unset(REPETITIONS)
