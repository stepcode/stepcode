#!/bin/sh
#Run rundelta.sh 502 times, with varying granularity

function runone {
    ../../misc/rundelta.sh ../bin/fedex_plus ../../misc/test-fedex-warn-vector.sh $1 $2
}

function runit {
    runone 1 $1
    for i in seq 100; do
        runone 2 minimal.exp
        runone 3 minimal.exp
        runone 3 minimal.exp
        runone 3 minimal.exp
        runone 1 minimal.exp
    done
    runone 2 minimal.exp
}

runit $1 2>&1 |sed -ne 's/.* \([0-9]\+.exp\sSUCCESS, lines: [0-9]\+ \**\).*/\1/pg;'
