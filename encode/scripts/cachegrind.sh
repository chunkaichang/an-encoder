#! /bin/bash

DEBUG="Release"
SOURCE_PATH="."
LENGTH=100

while getopts "ds:l:p:e:n:" opt; do
	case $opt in
		d) DEBUG="Debug"
			;;
		s) SOURCE_PATH=$OPTARG
			;;
		l) LENGTH=$OPTARG
			;;
		p) PLAIN_TEST=$OPTARG
			;;
		e) ENCODED_TEST=$OPTARG
			;; 
		n) NAME=$OPTARG
			;;
	esac
done

cmake -DLENGTH_PERF=${LENGTH} -DCMAKE_BUILD_TYPE=${DEBUG} -L ${SOURCE_PATH}
make -j4
valgrind --tool=cachegrind \
	--cachegrind-out-file=cachegrind.${NAME}.plain.${DEBUG}.${LENGTH}.out \
	${PLAIN_TEST}
valgrind --tool=cachegrind \
	--cachegrind-out-file=cachegrind.${NAME}.encoded.${DEBUG}.${LENGTH}.out \
	${ENCODED_TEST}
