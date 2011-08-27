#!/bin/sh

if [ "x$1" = "x" ]; then
  f="temp.exp"
else
  f=$1
fi

if  [ ! -x $FEDEX_PLUS ]; then
    echo "fedex_plus ($1) cannot be executed"
fi

#must be the full path to fedex_plus, as the script will be run from a different dir
#grep ERROR | head -n1 ensures that the error remains the first error
$FEDEX_PLUS -n $f 2>&1 >/dev/null | grep WARNING | head -n1 |\
    grep -q "WARNING: Possibly ambiguous implicit downcast (vector?)" &> /dev/null
