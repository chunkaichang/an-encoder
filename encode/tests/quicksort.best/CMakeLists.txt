cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(LENGTH 100)
set(REPETITIONS 10)
BUILD_TEST_CASE(quicksort.best quicksort.best.veri)
BUILD_TEST_CONFIG(quicksort.best quicksort.best.veri "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.best.veri.cfg)

set(LENGTH ${LENGTH_PERF})
set(REPETITIONS 100)
BUILD_TEST_CASE(quicksort.best quicksort.best.perf)
BUILD_TEST_CONFIG(quicksort.best quicksort.best.perf "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.best.perf.cfg)

set(LENGTH 10)
set(REPETITIONS 1)
BUILD_TEST_CASE(quicksort.best quicksort.best.cover)
BUILD_TEST_CONFIG(quicksort.best quicksort.best.cover "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.best.cover.cfg)

unset(LENGTH)
unset(REPETITIONS)
