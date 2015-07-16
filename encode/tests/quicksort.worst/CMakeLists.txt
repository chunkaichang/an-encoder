cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(LENGTH 100)
set(REPETITIONS 10)
BUILD_TEST_CASE(quicksort.worst quicksort.worst.veri)
BUILD_TEST_CONFIG(quicksort.worst quicksort.worst.veri "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.worst.veri.cfg)

set(LENGTH ${LENGTH_PERF})
set(REPETITIONS 100)
BUILD_TEST_CASE(quicksort.worst quicksort.worst.perf)
BUILD_TEST_CONFIG(quicksort.worst quicksort.worst.perf "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.worst.perf.cfg)

set(LENGTH 10)
set(REPETITIONS 1)
BUILD_TEST_CASE(quicksort.worst quicksort.worst.cover)
BUILD_TEST_CONFIG(quicksort.worst quicksort.worst.cover "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.worst.cover.cfg)

unset(LENGTH)
unset(REPETITIONS)
