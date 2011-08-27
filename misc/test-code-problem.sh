#!/bin/sh

if [ "x$1" = "x" ]; then
  f="AP214E3_2010.exp"  #this is the initial file name
else
  f=$1
fi

#must be the full path to fedex_plus, as the script will be run from a different dir
$FEDEX_PLUS -n $f >/dev/null 2>&1

# The line below contains a string or pattern to be matched. 
# In this case, it occurs in several files - for speed, only check the smallest of them
grep -q "representation_relationship_with_transformation_      transformation_operator" SdaiAUTOMOTIVE_DESIGN.init.cc &> /dev/null