cmake_minimum_required(VERSION 2.8)

add_custom_target(run_tests.py ALL
                   COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py
                           ${ENCODE_BIN_DIR}
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py)
