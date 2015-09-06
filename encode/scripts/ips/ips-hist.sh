#! /bin/bash

DIRNAME=$(dirname $0)
$DIRNAME/ips.sh $1 $2 $3 | sed 's/.*IP = //' | sort | uniq -c 
