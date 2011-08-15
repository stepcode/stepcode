#!/bin/sh

#rundelta.sh

#from stackoverflow
DIR="$( cd "$( dirname "$0" )" && pwd )"

function usage {
    echo "Usage for $0:"
    echo "$0 path/to/fedex_plus test-script newline-level file.exp"
    echo "where path/to/fedex_plus is the path to the fedex_plus binary, test-script"
    echo "is an executable script that tests pass/fail and returns 0 on success,"
    echo "newline-level affects delta's granularity. Start with 1, increase to 2,"
    echo "then 3, repeat, repeat, repeat."
    echo "file.exp is the schema to start with. Result is named minimal.exp"
    echo "Uses delta, available at http://delta.tigris.org/"
    echo
    echo "example: ../../misc/rundelta.sh ../bin/fedex_plus ../../misc/test-fedex-error.sh 1 ../../data/ap203e2/ap203e2_mim_lf.exp"
    exit
}

if [ -e $2 ]; then
    script=$2
elif [ -e $DIR/$2 ]; then
    script="$DIR/$2"
else
    echo "Cannot find script $2."
    usage
fi

if  [ "$3" == "" ]; then
    echo "Bad value for n: $3."
    usage
elif [ "$3" -gt "3" ]; then
    echo "Bad value for n: $3."
    usage
elif [ "$3" -lt "1" ]; then
    echo "Bad value for n: $3."
    usage
fi
        
if [ ! -e $4 ]; then
    echo "Cannot find schema, arg 4: $4."
    usage
fi

if  [ -x delta ]; then
    delta="delta"
elif [ -x "$DIR/delta/delta" ]; then
    delta="$DIR/delta/delta"
else
    echo "Cannot find delta."
    usage
fi

FEDEX_PLUS="$( cd "$( dirname "$1" )" && pwd )/fedex_plus"
if  [ -x $FEDEX_PLUS ]; then
    export FEDEX_PLUS
else
    echo "fedex_plus ($1) cannot be executed"
    usage
fi

$DIR/delta-newlines.sh $4 $3 >temp.exp
$delta -test=$2 -suffix=.exp -cp_minimal=minimal.exp temp.exp