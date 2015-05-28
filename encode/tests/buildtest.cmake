cmake_minimum_required(VERSION 2.8)

if ("${CMAKE_BUILD_TYPE}" MATCHES "DEBUG" OR
    "${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  set(CLANG_EMIT_LLVM_OPTS -O0)
  set(CLANG_LINK_OPTS -O0)
  set(ENCODE_OPTS -no-inlining -no-opts)
  set(DEBUG_OPTS -g)
else()
  set(CLANG_EMIT_LLVM_OPTS -O2)
  set(CLANG_LINK_OPTS -O2)
  set(ENCODE_OPTS)
  set(DEBUG_OPTS)
endif()

macro(SET_BUILD_VARS TEST_NAME TARGET_NAME)
  set(MAIN_MODULE_SRC  "${TEST_NAME}.main.c")
  set(ENC_MODULE_SRC   "${TEST_NAME}.enc.c")

  set(MAIN_MODULE_BC  "${TARGET_NAME}.main.bc")
  set(ENC_MODULE_BC   "${TARGET_NAME}.enc.bc")

  set(MAIN_MODULE_ENC_BC  "${TARGET_NAME}.main.enc.bc")
  set(MAIN_MODULE_ENC_2_BC  "${TARGET_NAME}.main.enc.2.bc")
  set(ENC_MODULE_ENC_BC   "${TARGET_NAME}.enc.enc.bc")

  set(MAIN_TARGET "${TARGET_NAME}.plain")
  set(ENC_TARGET  "${TARGET_NAME}.encoded")
endmacro(SET_BUILD_VARS)


function(BUILD_TEST_CASE TEST_NAME TARGET_NAME)
  SET_BUILD_VARS(${TEST_NAME} ${TARGET_NAME})

  if(LENGTH)
    set(PP_DEFS -DLENGTH=${LENGTH})
  endif()
  if(REPETITIONS)
    set(PP_DEFS ${PP_DEFS} -DREPETITIONS=${REPETITIONS})
  endif()

  # Build unencoded reference binary:
  add_custom_command(OUTPUT ${MAIN_MODULE_BC}
		     # pass option "-O2" to ensure that functions from 'mycyc.h'
		     # are inlined:
                     COMMAND ${CLANG} -c -emit-llvm ${DEBUG_OPTS} -O2 
                             ${PP_DEFS} -o ${MAIN_MODULE_BC}
                             ${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_MODULE_SRC}
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_MODULE_SRC})

  add_custom_command(OUTPUT ${ENC_MODULE_BC}
                     COMMAND ${CLANG} -c -emit-llvm ${DEBUG_OPTS}
			     ${CLANG_EMIT_LLVM_OPTS} -mno-sse
                             ${PP_DEFS} -o ${ENC_MODULE_BC}
                             ${CMAKE_CURRENT_SOURCE_DIR}/${ENC_MODULE_SRC}
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${ENC_MODULE_SRC})

  add_custom_target(${MAIN_TARGET} ALL
                    COMMAND ${CLANG} ${CLANG_LINK_OPTS}
                            -o ${MAIN_TARGET} ${MAIN_MODULE_BC} ${ENC_MODULE_BC}
                            ../mylibs/mycheck.bc
                            ../mylibs/mycyc.bc
                    DEPENDS ${MAIN_MODULE_BC} ${ENC_MODULE_BC}
                            mycheck.bc
                            mycyc.bc)

  # Build encoded binary:
  add_custom_command(OUTPUT ${MAIN_MODULE_ENC_BC}
		     # pass option "-O2" to ensure that functions from 'mycyc.h'
		     # are inlined:
                     COMMAND ${CLANG} -c -emit-llvm ${DEBUG_OPTS} -O2
			     -DENCODE
                             ${PP_DEFS} -o ${MAIN_MODULE_ENC_BC}
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
                    COMMAND ${CLANG} ${DEBUG_OPTS} ${CLANG_LINK_OPTS}
                            -o ${ENC_TARGET}
                            ${MAIN_MODULE_ENC_2_BC} ${ENC_MODULE_ENC_BC}
                            ../mylibs/mycheck.bc
                            ../mylibs/mycyc.bc
                    DEPENDS ${MAIN_MODULE_ENC_2_BC} ${ENC_MODULE_ENC_BC}
                            mycheck.bc
                            mycyc.bc)
endfunction(BUILD_TEST_CASE)


function(BUILD_TEST_CONFIG TEST_NAME TARGET_NAME ARGS PRECONFIG)
  SET_BUILD_VARS(${TEST_NAME} ${TARGET_NAME})

  set(PLAIN_BINARY ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_TARGET})
  set(ENCODED_BINARY ${CMAKE_CURRENT_BINARY_DIR}/${ENC_TARGET})
  configure_file(${PRECONFIG} ${TARGET_NAME}.cfg)

  unset(PLAIN_BINARY)
  unset(ENCODED_BINARY)
endfunction(BUILD_TEST_CONFIG)
