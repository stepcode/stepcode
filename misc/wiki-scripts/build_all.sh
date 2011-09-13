#!/bin/sh

#run from the scl root dir
#creates files for upload to the scl wiki. some are in markdown format, others are raw text

out_dir="wiki"
matrix_file="$out_dir/Schema-build-matrix.md"
result_dir="."
mk="make -j4"

#separated by ; for cmake
schemas="../data/example/example.exp;../data/ap227/ap227.exp;../data/ap214e3/AP214E3_2010.exp;../data/select.exp;../data/ap203e2/ap203e2_mim_lf.exp;../data/ap203/203wseds.exp;../data/ifc2x3/IFC2X3_TC1.exp;../data/pdmnet.exp;../data/ifc2x4/IFC2X4_RC2.exp;/opt/step/notingit/part409cdts_wg3n2617mim_lf.exp;/opt/step/notingit/ap242_managed_model_based_3d_engineering_mim_lf.exp;/opt/step/notingit/AP224E3_wg3n1941.exp;/opt/step/notingit/AP238E2_aim_lf_20100903.exp"

#count warnings and errors, append to $matrix_file. creates hypertext links to stderr,stdout txt
# $1 is the name of the row, $2 is the path and first part of the filename
function count_we {
    e="$2"_stderr.txt
    o="$2"_stdout.txt
    (
        if [ -s $e ]; then
            echo "<tr><td><b>$1</b></td>"
            echo "<td><font color="red">`grep -ci error $e` errors</font></td>"
            echo "<td><font color="blue">`grep -ci warning $e` warnings</font></td>"
            if [ -s $e ]; then
                echo -n "<td><a href="
                echo $e | sed -e "s|$result_dir/||;" -e "s/^\(.*\)\.txt$/\1/;"
                echo ">stderr text</a></td>"
            else
                echo "<td>   ----   </td>"
            fi
            echo "</td>"
        else #empty
            echo "<tr><td><b>$1</b></td>"
            echo "<td><font color="red">0 errors</font></td>"
            echo "<td><font color="blue">0 warnings</font></td>"
            echo "<td>   ----   </td>"
        fi
        if [ -s $o ]; then
            echo -n "<td><a href="
            echo $o | sed -e "s|$result_dir/||;"
            echo ">stdout text</a></td>"
        else #empty
            echo "<td>   ----   </td>"
        fi
        echo "</td></tr>" 
    ) >>$matrix_file
}

#make fedex warnings and errors pretty
function fedex_details {
    (
        i="$result_dir/fedex_$1_stderr.txt"
        w=`sed -ne 's|^.*:\([0-9]\+\): WARNING: \(.*\)$|<tr><td>\1</td><td>\2</td></tr>|p;' $i`
        e=`sed -ne 's|^.*:\([0-9]\+\): --ERROR: \(.*\)$|<tr><td>\1</td><td>\2</td></tr>|p;' $i`
        if [ "x$w" != "x" ]; then
            echo "## fedex_plus warnings"
            echo "<table  width=100%><tr><th>Line</th><th>Message</th>$w</table>"
            echo 
        fi
        if [ "x$e" != "x" ]; then
            echo "## fedex_plus errors"
            echo "<table  width=100%><tr><th>Line</th><th>Message</th>$e</tr></table>"
            echo
        fi
        o=`grep -ve ": WARNING:" -e ": --ERROR:" -e ^$ $i`
        if [ "x$o" != "x" ]; then
            echo "## Other text from Standard Error"
            echo "<pre>"
            sed -e 's|^.*:[0-9]\+: WARNING: .*$||g;' -e 's|^.*:[0-9]\+: --ERROR: .*$||g;' -e 's|$|\n|;' -e 's/\t/    /;' $i|grep -v ^$
            echo "</pre>"
            echo
        fi
        if [ -s $result_dir/fedex_$1_stdout.txt ]
            echo "## Standard Output"
            cat $result_dir/fedex_$1_stdout.txt
            echo
        fi

        # TODO rename file, put everything for one schema in one file
    ) >$out_dir/fedex_$1_stderr.md
}

function build_one_schema {
    #set $i to the schema name, all caps (to match fedex_plus output)
    i=`sed -ne '0,/^\s*SCHEMA/s/^.*SCHEMA\s\+\(.*\);.*$/\1/p;' $1|tr a-z A-Z`

    echo "Running fedex_plus and gcc for $i..."
    make -f data/CMakeFiles/sdai_$i.dir/build.make $i/compstructs.cc  2>"$result_dir/fedex_"$i"_stderr.txt" >"$result_dir/fedex_"$i"_stdout.txt" && \
    $mk sdai_$i >/dev/null 2>"$result_dir/compile_libsdai_"$i"_stderr.txt" && \
    $mk p21read_sdai_$i >/dev/null 2>"$result_dir/compile_p21read_sdai_"$i"_stderr.txt"

    #todo: test p21read_sdai_$i
}

function build {
    echo "Running cmake..."
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC_LIBS=OFF -DBUILD_SCHEMAS=$schemas >/dev/null

    # build all parts of scl that are necessary for schema libs to build
    # when given multiple targets, cmake's makefiles don't always work well with 'make -j4' - some things get built twice, simultaneously
    echo "Building SCL:"
    $mk stepeditor | grep "^Linking"
    $mk fedex_plus stepdai | grep "^Linking"

    for h in `echo $schemas | sed -e 's/;/\n/g;'`
    do
        build_one_schema $h
    done
}

function gen_wiki {
    echo "## `date`" >$matrix_file
    git log --decorate=short -n1 |\
        head -n1 |\
        sed -e 's|^.*commit \([a-z0-9]\+\) .*$|### Current as of commit [\1](http://github.com/mpictor/StepClassLibrary/commit/\1)|;' >>$matrix_file
    echo "<table border=1>" >>$matrix_file
    for h in `echo $schemas | sed -e 's/;/\n/g;'`
    do
        i=`sed -ne '0,/^\s*SCHEMA/s/^.*SCHEMA\s\+\(.*\);.*$/\1/p;' $h | tr a-z A-Z`

        #create a table for this schema
        echo "<tr><td><table width=100%><tr><th>Schema $i</th></tr><tr><td>" >>$matrix_file
        j=`echo $1|sed -e "s|^.*/\(.*\.exp\)$|\1|;"`  # strip path from .exp file name
        echo "$j</td></tr><tr><td><table border=1>" >>$matrix_file
        count_we "fedex_plus (generate c++)" $result_dir/fedex_$i
        count_we "gcc (compile lib)" $result_dir/compile_libsdai_$i
        count_we "gcc (compile p21read)" $result_dir/compile_p21read_sdai_$i
        echo "</table></td></tr>" >>$matrix_file
        echo "</table></td></tr>" >>$matrix_file
        if [ -s $result_dir/fedex_"$i"_stderr.txt ]; then
            fedex_details $i
        fi
    done
    echo "</table>" >>$matrix_file
    echo "Done!"
    echo "Copy everything in build_all/$out_dir to the wiki"
}


if [ ! ${PWD##*/} == "build_all" ]; then
    mkdir -p build_all && cd build_all && rm -rf * && mkdir -p $out_dir && mkdir -p $result_dir
    build
    echo -n "Done building. "
fi

echo "Now creating wiki pages"
gen_wiki
