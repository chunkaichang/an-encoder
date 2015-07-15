cmake_minimum_required(VERSION 2.8)


if ("${CMAKE_BUILD_TYPE}" MATCHES "DEBUG" OR
    "${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  set(CLANG_EMIT_LLVM_OPTS -O0)
  set(ENCODE_OPTS ${ENCODE_OPTS} -no-inlining -no-opts)
  set(DEBUG_OPTS -g)
else()
  set(CLANG_EMIT_LLVM_OPTS -O2)
  set(ENCODE_OPTS ${ENCODE_OPTS})
  set(DEBUG_OPTS)
endif()

macro(LLVM_DIS INPUT OUTPUT OUTPUT_NAME)
  add_custom_target(${OUTPUT_NAME} ALL
                     COMMAND ${LLVM_DIS} ${INPUT}
                      -o ${OUTPUT}
                     DEPENDS ${INPUT})
endmacro(LLVM_DIS)

function(BUILD_PROFILE_TEST SOURCE_NAME PROFILE_NAME INFIX)
  set(BC_NAME "${SOURCE_NAME}.${INFIX}.bc")
  set(LL_NAME "${SOURCE_NAME}.${INFIX}.ll")
  set(ENC_BC_NAME "${SOURCE_NAME}.${INFIX}.enc.bc")
  set(ENC_LL_NAME "${SOURCE_NAME}.${INFIX}.enc.ll")

  set(ENCODE_OPTS -p ${CMAKE_CURRENT_SOURCE_DIR}/${PROFILE_NAME} ${ENCODE_OPTS})

  add_custom_command(OUTPUT ${BC_NAME}
                     COMMAND ${CLANG} -c -emit-llvm ${DEBUG_OPTS}
			     ${CLANG_EMIT_LLVM_OPTS} -mno-sse
                             ${PP_DEFS} -o ${BC_NAME}
                             ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_NAME}
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_NAME})
  LLVM_DIS(${BC_NAME}
           ${LL_NAME}
           ${LL_NAME})

  add_custom_target(${ENC_BC_NAME} ALL
                     COMMAND ${ENCODE_BIN_DIR}/encode ${ENCODE_OPTS}
                             -o ${ENC_BC_NAME}
                             ${BC_NAME}
                     DEPENDS encode ${BC_NAME})

  LLVM_DIS(${ENC_BC_NAME}
           ${ENC_LL_NAME}
           ${ENC_LL_NAME})

endfunction(BUILD_PROFILE_TEST)

