#! /bin/bash

if [ -z "$ENCODE_DIR"]
then
  ENCODE_DIR=/media/norman/scratch/Projects/encode
fi

if [ -z "$LLVM_BUILD_DIR"]
then
  LLVM_BUILD_DIR=/media/norman/scratch/Projects/llvm-git-3.5.1/build
fi
LLVM_CMAKE_DIR=${LLVM_BUILD_DIR}/share/llvm/cmake/
LLVM_BIN_DIR=${LLVM_BUILD_DIR}/bin

# For configuring cmake:
export LLVM_DIR=$LLVM_CMAKE_DIR
cmake -DCMAKE_BUILD_TYPE=DEBUG ${ENCODE_DIR}
make -j4

mkdir -p encode
mkdir -p encode/runtime

LIBS="anlib.c rdtsc.c"
for file in ${LIBS}
do
  LL=${file}".ll"
  BC=${file}".bc"

  ${LLVM_BIN_DIR}/clang -c -emit-llvm -g -O0 \
      -o ${PWD}/encode/runtime/$BC \
      ${ENCODE_DIR}/encode/runtime/$file

  ${LLVM_BIN_DIR}/llvm-dis \
      -o ${PWD}/encode/runtime/$LL \
      ${PWD}/encode/runtime/$BC
done

# copy the 'apply-an.sh' script from the source tree into the build
# tree (rooted at ${PWD}):
cp ${ENCODE_DIR}/encode/bin/apply-an.sh ${PWD}/encode/apply-an.sh

# copy the tests over into the build tree (rooted at ${PWD}):
mkdir -p encode/tests
ENCODE_TESTS_DIR=${ENCODE_DIR}/encode/tests
for file in $(ls ${ENCODE_TESTS_DIR})
do
  cp ${ENCODE_TESTS_DIR}/$file ${PWD}/encode/tests/$file
done

# cp -r ${ENCODE_DIR}/encode/tests/*.{c,cpp,h,hpp} ${PWD}/encode/tests
