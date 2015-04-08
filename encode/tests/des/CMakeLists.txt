cmake_minimum_required(VERSION 2.8)

set(DES_INPUT des.input.large.txt)

add_custom_target(${DES_INPUT} ALL
                  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${DES_INPUT} ${CMAKE_CURRENT_BINARY_DIR}
                  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${DES_INPUT})

include(../buildtest.cmake)

BUILD_TEST_CASE(des)
BUILD_TEST_CONFIG(des "-e ${CMAKE_CURRENT_BINARY_DIR}/${DES_INPUT} /dev/null 809" 100)

