cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

function(COPY_DES_INPUT INPUT_FILENAME)
  add_custom_target(${INPUT_FILENAME} ALL
                    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILENAME} ${CMAKE_CURRENT_BINARY_DIR}
                    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILENAME})
endfunction(COPY_DES_INPUT)

# Tiny test case:
set(DES_TINY_INPUT  des.input.tiny.txt)
COPY_DES_INPUT(${DES_TINY_INPUT})

BUILD_TEST_CASE(des des.tiny.veri)
BUILD_TEST_CONFIG(des des.tiny.veri
                      "-e ${CMAKE_CURRENT_BINARY_DIR}/${DES_TINY_INPUT} /dev/null 809"
                      ${CMAKE_CURRENT_SOURCE_DIR}/des.tiny.veri.cfg)

BUILD_TEST_CASE(des des.tiny.perf)
BUILD_TEST_CONFIG(des des.tiny.perf
                      "-e ${CMAKE_CURRENT_BINARY_DIR}/${DES_TINY_INPUT} /dev/null 809"
                      ${CMAKE_CURRENT_SOURCE_DIR}/des.tiny.perf.cfg)

BUILD_TEST_CASE(des des.tiny.cover)
BUILD_TEST_CONFIG(des des.tiny.cover
                      "-e ${CMAKE_CURRENT_BINARY_DIR}/${DES_TINY_INPUT} /dev/null 809"
                      ${CMAKE_CURRENT_SOURCE_DIR}/des.tiny.cover.cfg)

# Large test case:
set(DES_LARGE_INPUT des.input.large.txt)
COPY_DES_INPUT(${DES_LARGE_INPUT})

BUILD_TEST_CASE(des des.large.veri)
BUILD_TEST_CONFIG(des des.large.veri
                      "-e ${CMAKE_CURRENT_BINARY_DIR}/${DES_LARGE_INPUT} /dev/null 809"
                      ${CMAKE_CURRENT_SOURCE_DIR}/des.large.veri.cfg)

BUILD_TEST_CASE(des des.large.perf)
BUILD_TEST_CONFIG(des des.large.perf
                      "-e ${CMAKE_CURRENT_BINARY_DIR}/${DES_LARGE_INPUT} /dev/null 809"
                      ${CMAKE_CURRENT_SOURCE_DIR}/des.large.perf.cfg)

# NOTE: Only the "tiny" test case is used for coverage.

# Huge test case:
set(DES_HUGE_INPUT des.input.huge.txt)
COPY_DES_INPUT(${DES_HUGE_INPUT})

BUILD_TEST_CASE(des des.huge.veri)
BUILD_TEST_CONFIG(des des.huge.veri
                      "-e ${CMAKE_CURRENT_BINARY_DIR}/${DES_HUGE_INPUT} /dev/null 809"
                      ${CMAKE_CURRENT_SOURCE_DIR}/des.huge.veri.cfg)

BUILD_TEST_CASE(des des.huge.perf)
BUILD_TEST_CONFIG(des des.huge.perf
                      "-e ${CMAKE_CURRENT_BINARY_DIR}/${DES_HUGE_INPUT} /dev/null 809"
                      ${CMAKE_CURRENT_SOURCE_DIR}/des.huge.perf.cfg)

# NOTE: Only the "tiny" test case is used for coverage.
