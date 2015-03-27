#! /bin/bash

if [ -z "$ENCODE_BINARY_DIR" ]
then
  ENCODE_BINARY_DIR=/media/norman/scratch/Projects/build/encode.2/encode/src
fi

if [ -z "$ENCODE_RUNTIME_DIR" ]
then
  ENCODE_RUNTIME_DIR=/media/norman/scratch/Projects/build/encode.2/encode/runtime
fi

if [ -z "$LLVM_BUILD_DIR" ]
then
  LLVM_BUILD_DIR=/media/norman/scratch/Projects/llvm-git-3.5.1/build
fi
LLVM_BIN_DIR=${LLVM_BUILD_DIR}/bin

CLANG_OPTS="-O0 -g"
ENCODE_OPTS=""

MAIN_INPUT=$1
ENC_INPUT=$2

MAIN_OBJ=${MAIN_INPUT}".o"
MAIN_ENC_BC=${MAIN_INPUT}".enc.bc"

ENC_BC=${ENC_INPUT}".bc"
ENC_RESULT_BC=${ENC_INPUT}".enc.bc"

ENC_LL=${ENC_INPUT}".ll"
ENC_RESULT_LL=${ENC_INPUT}".enc.ll"

OUT=${MAIN_INPUT}".plain"
ENC_OUT=${MAIN_INPUT}".enc"

if [ ! -z "$VERBOSE" ]
then
  V="-v"
fi


${LLVM_BIN_DIR}/clang ${V} -c -O0 ${CLANG_OPTS} -o ${MAIN_OBJ} ${MAIN_INPUT}

${LLVM_BIN_DIR}/clang ${V} -c -emit-llvm ${CLANG_OPTS} -DENCODE -o ${MAIN_ENC_BC} ${MAIN_INPUT}
${ENCODE_BINARY_DIR}/encode --expand-only ${MAIN_ENC_BC} -o ${MAIN_ENC_BC}


${LLVM_BIN_DIR}/clang ${V} -c -emit-llvm -g -O0 -mno-sse ${CLANG_OPTS} -mno-sse -o ${ENC_BC} ${ENC_INPUT}
${ENCODE_BINARY_DIR}/encode ${ENCODE_OPTS} ${ENC_BC} -o ${ENC_RESULT_BC}

${LLVM_BIN_DIR}/llvm-dis -o ${ENC_LL} ${ENC_BC}
${LLVM_BIN_DIR}/llvm-dis -o ${ENC_RESULT_LL} ${ENC_RESULT_BC}

${LLVM_BIN_DIR}/clang ${V} ${CLANG_OPTS} -o ${OUT}     ${ENC_BC}        ${MAIN_OBJ} ../mylibs/mycheck.c ../mylibs/mycyc.c
${LLVM_BIN_DIR}/clang ${V} ${CLANG_OPTS} -o ${ENC_OUT} ${ENC_RESULT_BC} ${MAIN_ENC_BC} ../mylibs/mycheck.c ../mylibs/mycyc.c
