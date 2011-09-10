#!/bin/sh

#run from the scl root dir
#creates files for upload to the scl wiki. some are in markdown format, others are raw text

out_dir="wiki"
matrix_file="$out_dir/Schema-build-matrix.md"
mk="make -j4"

#count warnings and errors, append to $matrix_file. creates hypertext links to stderr,stdout txt
# $1 is the name of the row, $2 is the first part of the filename
function count_we {
    e="$2"_stderr.txt
    o="$2"_stdout.txt
    if [ -s $e ]; then
        echo "<tr><td><b>$1</b></td>" >>$matrix_file
        echo "<td><font color="red">`grep -ci error $e` errors</font></td>" >>$matrix_file
        echo "<td><font color="blue">`grep -ci warning $e` warnings</font></td>" >>$matrix_file
        if [ -s $e ]; then
            echo -n "<td><a href=" >>$matrix_file
            echo $e | sed -e "s|$out_dir/||;" -e "s/^\(.*fedex.*\)\.txt\s*$/\1.md/;" >>$matrix_file
            echo ">stderr text</a></td>" >>$matrix_file
        else
            echo "<td>   ----   </td>" >>$matrix_file
        fi
        echo "</td>" >>$matrix_file
    else #empty
        echo "<tr><td><b>$1</b></td>" >>$matrix_file
        echo "<td><font color="red">0 errors</font></td>" >>$matrix_file
        echo "<td><font color="blue">0 warnings</font></td>" >>$matrix_file
        echo "<td>   ----   </td>" >>$matrix_file
        if [ -f $e ]; then
            rm $e
        fi
    fi
    if [ -s $o ]; then
        echo -n "<td><a href=" >>$matrix_file
        echo $o | sed -e "s|$out_dir/||;" >>$matrix_file
        echo ">stdout text</a></td>" >>$matrix_file
    else #empty
        echo "<td>   ----   </td>" >>$matrix_file
        if [ -f $o ]; then
            rm $o
        fi
    fi
    echo "</td></tr>" >>$matrix_file
}

#make fedex warnings and errors pretty
function fedex_details {
    (
        i="$out_dir/status/fedex_$1_stderr.txt"
        w=`sed -ne 's|^.*:\([0-9]\+\): WARNING: \(.*\)$|<tr><td>\1</td><td>\2</td></tr>|p;' $i`
        e=`sed -ne 's|^.*:\([0-9]\+\): --ERROR: \(.*\)$|<tr><td>\1</td><td>\2</td></tr>|p;' $i`
        if [ "x$w" != "x" ]; then
            echo "<table width=100%><tr><td><font color="blue">fedex_plus warnings</font></td></tr><tr><td><table  width=100%>"
            echo "<tr><th>Line</th><th>Message</th>$w</table></td></tr></table>"
        fi
        if [ "x$e" != "x" ]; then
            echo "<table width=100%><tr><td><font color="red">fedex_plus errors</font></td></tr><tr><td><table  width=100%>"
            echo "<tr><th>Line</th><th>Message</th>$e</tr></table></td></tr></table>"
        fi
        o=`grep -ve ": WARNING:" -e ": --ERROR:" -e ^$ $i`
        if [ "x$o" != "x" ]; then
            echo "<br><h3>Other text in file</h3>"
            echo "<pre>"
            sed -e 's|^.*:[0-9]\+: WARNING: .*$||g;' -e 's|^.*:[0-9]\+: --ERROR: .*$||g;' -e 's|$|\n|;' -e 's/\t/    /;' $i|grep -v ^$
            echo "</pre>"
        fi
    ) >$out_dir/status/fedex_$1_stderr.md
    rm $out_dir/status/fedex_$1_stderr.txt
}

function build_one {
    #set $i to the schema name, all caps (to match fedex_plus output)
    i=`sed -ne '0,/^\s*SCHEMA/s/^.*SCHEMA\s\+\(.*\);.*$/\1/p;' $1|tr a-z A-Z`

    echo "Running fedex_plus and gcc for $i..."
    make -f data/CMakeFiles/sdai_$i.dir/build.make $i/compstructs.cc  2>"$out_dir/status/fedex_"$i"_stderr.txt" >"$out_dir/status/fedex_"$i"_stdout.txt" && \
    $mk sdai_$i >/dev/null 2>"$out_dir/status/compile_libsdai_"$i"_stderr.txt" && \
    $mk p21read_sdai_$i >/dev/null 2>"$out_dir/status/compile_p21read_sdai_"$i"_stderr.txt"

    #todo: test p21read_sdai_$i

    #create a table for this schema in the markdown file
    echo "<tr><td><table width=100%><tr><th>Schema $i</th></tr><tr><td>" >>$matrix_file
    j=`echo $1|sed -e "s|^.*/\(.*\.exp\)$|\1|;"`  # strip path from .exp file name
    echo "$j</td></tr><tr><td><table border=1>" >>$matrix_file
    count_we "fedex_plus (generate c++)" $out_dir/status/fedex_$i
    count_we "gcc (compile lib)" $out_dir/status/compile_libsdai_$i
    count_we "gcc (compile p21read)" $out_dir/status/compile_p21read_sdai_$i
    echo "</table></td></tr>" >>$matrix_file
    echo "</table></td></tr>" >>$matrix_file
    
    if [ -s $out_dir/status/fedex_"$i"_stderr.txt ]; then
        fedex_details $i
    fi
}

schemas="../data/example/example.exp;../data/ap227/ap227.exp;../data/ap214e3/AP214E3_2010.exp;../data/select.exp;../data/ap203e2/ap203e2_mim_lf.exp;../data/ap203/203wseds.exp;../data/ifc2x3/IFC2X3_TC1.exp;../data/pdmnet.exp;../data/ifc2x4/IFC2X4_RC2.exp;/opt/step/notingit/part409cdts_wg3n2617mim_lf.exp;/opt/step/notingit/ap242_managed_model_based_3d_engineering_mim_lf.exp;/opt/step/notingit/AP224E3_wg3n1941.exp;/opt/step/notingit/AP238E2_aim_lf_20100903.exp"

function build {
    echo "Running cmake..."
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC_LIBS=OFF -DBUILD_SCHEMAS=$schemas >/dev/null

    # build all parts of scl that are necessary for schema libs to build
    echo "Building SCL:"
    $mk stepeditor | grep "^Linking"
    $mk fedex_plus stepdai | grep "^Linking"

    for h in `echo $schemas | sed -e 's/;/\n/g;'`
    do
        build_one $h
    done
}

mkdir -p build_all && cd build_all && rm -rf * && mkdir -p $out_dir/status
echo "## `date`" >$matrix_file
git log --decorate=short -n1 |\
    head -n1 |\
    sed -e 's|^.*commit \([a-z0-9]\+\) .*$|### Current as of commit [\1](http://github.com/mpictor/StepClassLibrary/commit/\1)|;' >>$matrix_file
echo "<table border=1>" >>$matrix_file

build

echo "</table>" >>$matrix_file
echo "Done!"
echo "Copy everything in build_all/$out_dir/ to the wiki"
