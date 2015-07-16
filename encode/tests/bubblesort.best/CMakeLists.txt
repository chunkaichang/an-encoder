cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(LENGTH 10)
set(REPETITIONS 10)
BUILD_TEST_CASE(bubblesort.best bubblesort.best.veri)
BUILD_TEST_CONFIG(bubblesort.best bubblesort.best.veri "" ${CMAKE_CURRENT_SOURCE_DIR}/bubblesort.best.veri.cfg)

set(LENGTH ${LENGTH_PERF})
set(REPETITIONS 100)
BUILD_TEST_CASE(bubblesort.best bubblesort.best.perf)
BUILD_TEST_CONFIG(bubblesort.best bubblesort.best.perf "" ${CMAKE_CURRENT_SOURCE_DIR}/bubblesort.best.perf.cfg)

set(LENGTH 10)
set(REPETITIONS 1)
BUILD_TEST_CASE(bubblesort.best bubblesort.best.cover)
BUILD_TEST_CONFIG(bubblesort.best bubblesort.best.cover "" ${CMAKE_CURRENT_SOURCE_DIR}/bubblesort.best.cover.cfg)

