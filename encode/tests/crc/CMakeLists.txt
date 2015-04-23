cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(CRC_INPUT crc_input_small.pcm)
add_custom_target(${CRC_INPUT} ALL
                  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${CRC_INPUT} ${CMAKE_CURRENT_BINARY_DIR})

BUILD_TEST_CASE(crc crc.veri)
BUILD_TEST_CONFIG(crc crc.veri
                      ${CMAKE_CURRENT_BINARY_DIR}/${CRC_INPUT}
                      ${CMAKE_CURRENT_SOURCE_DIR}/crc.veri.cfg)

BUILD_TEST_CASE(crc crc.perf)
BUILD_TEST_CONFIG(crc crc.perf
                      ${CMAKE_CURRENT_BINARY_DIR}/${CRC_INPUT}
                      ${CMAKE_CURRENT_SOURCE_DIR}/crc.perf.cfg)

BUILD_TEST_CASE(crc crc.cover)
BUILD_TEST_CONFIG(crc crc.cover
                      ${CMAKE_CURRENT_BINARY_DIR}/${CRC_INPUT}
                      ${CMAKE_CURRENT_SOURCE_DIR}/crc.cover.cfg)
