cmake_minimum_required(VERSION 2.8)

function(COPY_SCRIPT SCRIPT)
  add_custom_target(${SCRIPT} ALL
                    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT}
                            ${ENCODE_BIN_DIR}
                    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT})
endfunction(COPY_SCRIPT)

COPY_SCRIPT(testcase.py)
COPY_SCRIPT(runner.py)
COPY_SCRIPT(coverage.py)
