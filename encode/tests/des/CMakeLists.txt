cmake_minimum_required(VERSION 2.8)

set(DES_HUGE_INPUT des.input.huge.txt)
set(DES_LARGE_INPUT des.input.large.txt)
set(DES_TINY_INPUT  des.input.tiny.txt)

function(COPY_DES_INPUT INPUT_FILENAME)
  add_custom_target(${INPUT_FILENAME} ALL
                    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILENAME} ${CMAKE_CURRENT_BINARY_DIR}
                    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILENAME})
endfunction(COPY_DES_INPUT)

include(../buildtest.cmake)

BUILD_TEST_CASE(des)

COPY_DES_INPUT(${DES_HUGE_INPUT})
BUILD_TEST_CONFIG(des.huge  des "-e ${CMAKE_CURRENT_BINARY_DIR}/${DES_HUGE_INPUT}  /dev/null 809" 4)

COPY_DES_INPUT(${DES_LARGE_INPUT})
BUILD_TEST_CONFIG(des.large des "-e ${CMAKE_CURRENT_BINARY_DIR}/${DES_LARGE_INPUT} /dev/null 809" 10)

COPY_DES_INPUT(${DES_TINY_INPUT})
BUILD_TEST_CONFIG(des.tiny  des "-e ${CMAKE_CURRENT_BINARY_DIR}/${DES_TINY_INPUT}  /dev/null 809" 100)

