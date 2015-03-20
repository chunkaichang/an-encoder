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
ENCODE_OPTS="-no-opts -no-inlining"

INPUT=$1
USERLIB=$2

INPUT_LONG=${INPUT}".long.c"
INPUT_CALLE=${INPUT_LONG}".calle.c"

BC=${INPUT}".bc"
LL=${INPUT}".ll"
CNT_BC=${INPUT}".cnt.bc"
CNT_LL=${INPUT}".cnt.ll"
CNT_OUT=${INPUT}".cnt"
ENC_BC=${INPUT}".enc.bc"
ENC_LL=${INPUT}".enc.ll"
ENC_OUT=${INPUT}".enc"

if [ ! -z "$VERBOSE" ]
then
  V="-v"
fi

${LLVM_BIN_DIR}/int64-convert ${INPUT} -- 1> ${INPUT_LONG}
${LLVM_BIN_DIR}/callexpr-convert ${INPUT_LONG} -- 1> ${INPUT_CALLE}
INPUT=${INPUT_CALLE}

${LLVM_BIN_DIR}/clang ${V} -c -emit-llvm ${CLANG_OPTS} -mno-sse -o ${BC} ${INPUT}
${LLVM_BIN_DIR}/llvm-dis -o ${LL} ${BC}

export ENCODE_RUNTIME_DIR=${ENCODE_RUNTIME_DIR}
${ENCODE_BINARY_DIR}/encode ${ENCODE_OPTS} ${BC} -o ${ENC_BC}
${ENCODE_BINARY_DIR}/encode ${ENCODE_OPTS} -count-only ${BC} -o ${CNT_BC}

${LLVM_BIN_DIR}/llvm-dis -o ${ENC_LL} ${ENC_BC}
${LLVM_BIN_DIR}/llvm-dis -o ${CNT_LL} ${CNT_BC}

if [ ! -z "$USERLIB" ]
then
  USEROBJ=${USERLIB}".o"
  ${LLVM_BIN_DIR}/clang ${V} -c ${CLANG_OPTS} -o ${USEROBJ} ${USERLIB}
fi

${LLVM_BIN_DIR}/clang ${V} ${CLANG_OPTS} -o ${ENC_OUT} ${ENC_BC} ${USEROBJ}
${LLVM_BIN_DIR}/clang ${V} ${CLANG_OPTS} -o ${CNT_OUT} ${CNT_BC} ${USEROBJ}
