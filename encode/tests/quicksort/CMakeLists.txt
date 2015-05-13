cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(LENGTH 100)
set(REPETITIONS 10)
BUILD_TEST_CASE(quicksort quicksort.veri)
BUILD_TEST_CONFIG(quicksort quicksort.veri "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.veri.cfg)

set(LENGTH ${LENGTH_PERF})
set(REPETITIONS 100)
BUILD_TEST_CASE(quicksort quicksort.perf)
BUILD_TEST_CONFIG(quicksort quicksort.perf "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.perf.cfg)

set(LENGTH 10)
set(REPETITIONS 1)
BUILD_TEST_CASE(quicksort quicksort.cover)
BUILD_TEST_CONFIG(quicksort quicksort.cover "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.cover.cfg)

unset(LENGTH)
unset(REPETITIONS)
