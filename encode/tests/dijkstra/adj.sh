#! /bin/bash

# Simple script for extracting matrix values from a dump of
# the 'Adjacency Matrix'.

# 1st command line argument:
# Text file containing the matrix entry. Each entry is
# expected to have the format "(00,00) 123;".
FILE=$1

# 2nd command line argument:
# Matrix row index, expected to be a two digit number
# with leading "0".
X=$2

# 3rd command line argument:
# Matrix column index, expected to be a two digit number
# with leading "0".
Y=$3

# Build the index pair pattern, in parenthesis:
PATTERN="("$X","$Y")"

# 'grep' for the index pair pattern, and trim the result
# using 'sed':
grep $PATTERN $FILE | \
  sed 's/.*'$PATTERN'//' | \
  sed 's/;.*$//'
