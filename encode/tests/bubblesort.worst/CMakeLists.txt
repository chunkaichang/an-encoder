cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(LENGTH 10)
set(REPETITIONS 10)
BUILD_TEST_CASE(bubblesort.worst bubblesort.worst.veri)
BUILD_TEST_CONFIG(bubblesort.worst bubblesort.worst.veri "" ${CMAKE_CURRENT_SOURCE_DIR}/bubblesort.worst.veri.cfg)

set(LENGTH ${LENGTH_PERF})
set(REPETITIONS 100)
BUILD_TEST_CASE(bubblesort.worst bubblesort.worst.perf)
BUILD_TEST_CONFIG(bubblesort.worst bubblesort.worst.perf "" ${CMAKE_CURRENT_SOURCE_DIR}/bubblesort.worst.perf.cfg)

set(LENGTH 10)
set(REPETITIONS 1)
BUILD_TEST_CASE(bubblesort.worst bubblesort.worst.cover)
BUILD_TEST_CONFIG(bubblesort.worst bubblesort.worst.cover "" ${CMAKE_CURRENT_SOURCE_DIR}/bubblesort.worst.cover.cfg)

