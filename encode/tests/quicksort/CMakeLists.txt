cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

BUILD_TEST_CASE(quicksort)
BUILD_TEST_CONFIG(quicksort quicksort "" ${CMAKE_CURRENT_SOURCE_DIR}/quicksort.precfg)

