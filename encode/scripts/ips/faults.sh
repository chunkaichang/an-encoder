#! /bin/bash

FOLDER=$1
TESTNAME=$2
FAULT=$3
GREP_FILTER=$4

NUMBERS=$(grep retcode $FOLDER/$TESTNAME.cover.encoded.log | \
          grep -v None | grep -v INVALID | grep $FAULT | sed 's/.*no. //' | sed s'/:.*$//')

for N in $NUMBERS
do
  grep "$GREP_FILTER" $FOLDER/$TESTNAME.cover.encoded.fi.$N.stderr
done
