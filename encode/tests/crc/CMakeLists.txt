cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

BUILD_TEST_CASE(crc)

set(CRC_INPUT crc_input_small.pcm)
add_custom_target(${CRC_INPUT} ALL
                  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${CRC_INPUT} ${CMAKE_CURRENT_BINARY_DIR})

BUILD_TEST_CONFIG(crc "${CMAKE_CURRENT_BINARY_DIR}/${CRC_INPUT}" 20)
