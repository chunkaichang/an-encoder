#! /bin/bash

FOLDER=$1
TESTNAME=$2
GREP_FILTER=$3

if [ ! -z "$GREP_FILTER" ]; then
  NUMBERS=$(grep retcode $FOLDER/$TESTNAME.cover.encoded.log | \
            grep -v None | grep -v INVALID | grep -v TXT | grep $GREP_FILTER | \
            sed 's/.*no. //' | sed s'/:.*$//')
else
  NUMBERS=$(grep retcode $FOLDER/$TESTNAME.cover.encoded.log | \
            grep -v None | grep -v INVALID | grep -v TXT | sed 's/.*no. //' | sed s'/:.*$//')
fi

for N in $NUMBERS; do
  IP=$(grep "IP = 0x" $FOLDER/$TESTNAME.cover.encoded.fi.$N.stderr | \
       sed 's/.*IP = 0x//' | sed 's/,.*$//')
  echo $N: IP = $IP
done
