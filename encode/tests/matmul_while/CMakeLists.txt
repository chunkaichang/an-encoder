cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(LENGTH 1024)
BUILD_TEST_CASE(matmul_while matmul_while.veri)
BUILD_TEST_CONFIG(matmul_while matmul_while.veri "" ${CMAKE_CURRENT_SOURCE_DIR}/matmul_while.veri.cfg)

# NOTE: This test case is only used for verification. (Perfomance and coverage analyses are
# done in the "matmul" test case.)
