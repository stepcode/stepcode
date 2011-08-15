#!/bin/sh

# expands/compact express - i.e. adds/removes newlines for delta.
# output is on stdout

#use: delta-newlines.sh file.exp n
# where n is a number:
# 1 to remove the most newlines
# 3 to leave or add the most newlines

# remove end-of-line comments; won't work properly on single-line comments inside (*...*) comments but those probably don't occur in official schemas
# remove blank lines
# remove spaces before semicolons
# replace newlines and tabs (\r,\n,\t) with a space, using tr
# remove multiline comments
# replace multiple spaces with one
# newline after SCHEMA and END_SCHEMA
# remove whitespace inside parenthesis
function compact {
    sed -e 's/--.*$//g'                         \
        -e 's/^ *$//g'                          \
        -e 's/^ *//g'                           \
        -e 's/ *;/;/g' $1                      |\
    tr  '\t' ' '                               |\
    tr  '\r' ' '                               |\
    tr  '\n' ' '                               |\
    sed -e 's/(\*[^)]*\*)//g'                   \
        -e 's/ \+/ /g'                          \
        -e 's/END_SCHEMA;/END_SCHEMA;\n/'       \
        -e 's/\(SCHEMA [A-Za-z0-9_-]*;\)/\1\n/' \
        -e 's/( \+)/()/g'
}

#newline after every comma or semicolon, before and after ||, |, *, /, +, -, :, inside () and []
function expand_all {
    sed -e 's/\([,;]\)/\1\n/g;'                     \
        -e 's/||/\n||\n/g;'                         \
        -e 's/ \([*\/+-=|:]\) /\n\1\n/g;'           \
        -e 's/(\([^)]\)/\n(\n\1/g;'                 \
        -e 's/\([^(]\))/\1\n)\n/g;'                 \
        -e 's/\[\([^]]\)/[\n\1/g;'                  \
        -e 's/\([^[]\)]/\1\n]/g;' 
}

function expand_inner {
    sed -e 's/END_CASE;/END_CASE;\n/g;'     \
        -e 's/END_IF;/END_IF;\n/g;'         \
        -e 's/END_LOCAL;/END_LOCAL;\n/g;'   \
        -e 's/END_REPEAT;/END_REPEAT;\n/g;'
}

function expand_outer {
    sed -e 's/END_RULE;/END_RULE;\n/g;'         \
        -e 's/END_TYPE;/END_TYPE;\n/g;'         \
        -e 's/END_CONSTANT;/END_CONSTANT;\n/g;' \
        -e 's/END_ENTITY;/END_ENTITY;\n/g;'     \
        -e 's/END_FUNCTION;/END_FUNCTION;\n/g;'
}

if  [ "$2" == "1" ]; then
    compact $1 | expand_outer
elif [ "$2" == "2" ]; then
    compact $1 | expand_outer | expand_inner
elif [ "$2" == "3" ]; then
    compact $1 | expand_all
else
    echo "use: delta-newlines.sh file.exp n > out.exp" >&2
    echo " where n is a number, 1-3: 3 for most newlines, 1 for least." >&2
    echo " if using delta, start with 1, go up, repeat." >&2
fi;
