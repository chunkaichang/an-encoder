cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

BUILD_TEST_CASE(matmul)
BUILD_TEST_CONFIG(matmul matmul "" 100)

