cmake_minimum_required(VERSION 2.8)

if ("${CMAKE_BUILD_TYPE}" MATCHES "DEBUG" OR
    "${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  set(CLANG_EMIT_LLVM_OPTS -O0 -g)
  set(CLANG_LINK_OPTS -O0 -g)
  set(ENCODE_OPTS --no-inline --no-opts)
else()
  set(CLANG_EMIT_LLVM_OPTS -O0)
  set(CLANG_LINK_OPTS -O2)
  set(ENCODE_OPTS)
endif()

macro(SET_BUILD_VARS TEST_NAME)
  set(MAIN_MODULE_SRC  "${TEST_NAME}.main.c")
  set(ENC_MODULE_SRC   "${TEST_NAME}.enc.c")

  set(MAIN_MODULE_BC  "${TEST_NAME}.main.bc")
  set(ENC_MODULE_BC   "${TEST_NAME}.enc.bc")

  set(MAIN_MODULE_ENC_BC  "${TEST_NAME}.main.enc.bc")
  set(MAIN_MODULE_ENC_2_BC  "${TEST_NAME}.main.enc.2.bc")
  set(ENC_MODULE_ENC_BC   "${TEST_NAME}.enc.enc.bc")

  set(MAIN_TARGET "${TEST_NAME}")
  set(ENC_TARGET  "${TEST_NAME}.enc")
endmacro(SET_BUILD_VARS)


function(BUILD_TEST_CASE TEST_NAME)
  SET_BUILD_VARS(${TEST_NAME})

  # Build unencoded reference binary:
  add_custom_command(OUTPUT ${MAIN_MODULE_BC}
                     COMMAND clang -c -emit-llvm ${CLANG_EMIT_LLVM_OPTS} -mno-sse 
                             -o ${MAIN_MODULE_BC}
                             ${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_MODULE_SRC}
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_MODULE_SRC})

  add_custom_command(OUTPUT ${ENC_MODULE_BC}
                     COMMAND clang -c -emit-llvm ${CLANG_EMIT_LLVM_OPTS} -mno-sse
                             -o ${ENC_MODULE_BC}
                             ${CMAKE_CURRENT_SOURCE_DIR}/${ENC_MODULE_SRC}
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${ENC_MODULE_SRC})

  add_custom_target(${MAIN_TARGET} ALL
                    COMMAND clang ${CLANG_LINK_OPTS}
                            -o ${MAIN_TARGET} ${MAIN_MODULE_BC} ${ENC_MODULE_BC}
                            ${CMAKE_CURRENT_SOURCE_DIR}/../mylibs/mycheck.c
                            ${CMAKE_CURRENT_SOURCE_DIR}/../mylibs/mycyc.c
                    DEPENDS ${MAIN_MODULE_BC} ${ENC_MODULE_BC}
                            ${CMAKE_CURRENT_SOURCE_DIR}/../mylibs/mycheck.c
                            ${CMAKE_CURRENT_SOURCE_DIR}/../mylibs/mycyc.c)

  # Build encoded binary:
  add_custom_command(OUTPUT ${MAIN_MODULE_ENC_BC}
                     COMMAND clang -c -emit-llvm ${CLANG_EMIT_LLVM_OPTS} -mno-sse -DENCODE
                             -o ${MAIN_MODULE_ENC_BC}
                             ${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_MODULE_SRC}
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_MODULE_SRC})

  add_custom_command(OUTPUT ${MAIN_MODULE_ENC_2_BC}
                     COMMAND ${ENCODE_BIN_DIR}/encode -expand-only
                             -o ${MAIN_MODULE_ENC_2_BC} ${MAIN_MODULE_ENC_BC}
                     DEPENDS encode ${MAIN_MODULE_ENC_BC})

  add_custom_command(OUTPUT ${ENC_MODULE_ENC_BC}
                     COMMAND ${ENCODE_BIN_DIR}/encode ${ENCODE_OPTS}
                             -o ${ENC_MODULE_ENC_BC}
                             ${ENC_MODULE_BC}
                     DEPENDS encode ${ENC_MODULE_BC})

  add_custom_target(${ENC_TARGET} ALL
                    COMMAND clang ${CLANG_LINK_OPTS}
                            -o ${ENC_TARGET}
                            ${MAIN_MODULE_ENC_2_BC} ${ENC_MODULE_ENC_BC}
                            ${CMAKE_CURRENT_SOURCE_DIR}/../mylibs/mycheck.c
                            ${CMAKE_CURRENT_SOURCE_DIR}/../mylibs/mycyc.c
                    DEPENDS ${MAIN_MODULE_ENC_2_BC} ${ENC_MODULE_ENC_BC}
                            ${CMAKE_CURRENT_SOURCE_DIR}/../mylibs/mycheck.c
                            ${CMAKE_CURRENT_SOURCE_DIR}/../mylibs/mycyc.c)
endfunction(BUILD_TEST_CASE)


function(BUILD_TEST_CONFIG TEST_NAME BINARY_NAME ARGS RUNS)
  SET_BUILD_VARS(${BINARY_NAME})

  set(MAIN_BINARY ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_TARGET})
  set(ENC_BINARY ${CMAKE_CURRENT_BINARY_DIR}/${ENC_TARGET})

  add_custom_target(${TEST_NAME}.cfg ALL
                    COMMAND echo "plain:   ${MAIN_BINARY} ${ARGS}" > ${TEST_NAME}.cfg &&
                            echo "encoded: ${ENC_BINARY} ${ARGS}" >> ${TEST_NAME}.cfg &&
                            echo "run: ${RUNS}" >> ${TEST_NAME}.cfg)
endfunction(BUILD_TEST_CONFIG)
