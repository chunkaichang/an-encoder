#! /bin/bash

FOLDER=$1
TESTNAME=$2
GREP_FILTER=$3

NUMBERS=$(grep retcode $FOLDER/$TESTNAME.cover.encoded.log | \
          grep -v None | grep -v INVALID | sed 's/.*no. //' | sed s'/:.*$//')

for N in $NUMBERS
do
  #echo grep "IP = $GREP_FILTER" $FOLDER/$TESTNAME.cover.encoded.fi.$N.stderr
       
  IP=$(grep "IP = $GREP_FILTER" $FOLDER/$TESTNAME.cover.encoded.fi.$N.stderr | \
       sed 's/.*IP = 0x//' | sed 's/,.*$//')
  if [ ! -z "$IP" ]; then
    #echo $N: IP = $IP
    grep "no. $N:" $FOLDER/$TESTNAME.cover.encoded.log | grep retcode | grep -v None
  fi
done
