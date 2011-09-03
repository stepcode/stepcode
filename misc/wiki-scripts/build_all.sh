#!/bin/sh

out_dir="wiki-dir"
matrix_file="$out_dir/Schema-build-matrix.md"
mk="make -j4"

#count warnings and errors, append to $matrix_file
function count_we {
    echo "<tr><td><b>$1</b></td>" >>$matrix_file
    echo "<td><font color="blue">0 warnings</font></td>" >>$matrix_file
    echo "<td><font color="red">0 errors</font></td><td>" >>$matrix_file
    if [ -s $2 ]; then
        echo "<td><font color="blue">`grep -ci warning $2` warnings</font></td>" >>$matrix_file
        echo "<td><font color="red">`grep -ci error $2` errors</font></td><td>" >>$matrix_file
        echo -n "<a href=" >>$matrix_file
        echo $2 | sed -e "s/^\(fedex.*\)\.txt\s*$/\1.md/;" >>$matrix_file
        echo ">full text</a>" >>$matrix_file
    else
        if [ -f $2 ]; then
            rm $2
        fi
    fi
    echo "</td></tr>" >>$matrix_file
}

#make fedex warnings and errors pretty
function fedex_details {
    (
        i="$out_dir/fedex_$1_stderr.txt"
        w=`sed -ne 's|^.*:\([0-9]\+\): WARNING: \(.*\)$|<tr><td>\1</td><td>\2</td></tr>|p;' $i`
        e=`sed -ne 's|^.*:\([0-9]\+\): --ERROR: \(.*\)$|<tr><td>\1</td><td>\2</td></tr>|p;' $i`
        if [ "x$w" != "x" ]; then
            echo "<table width=100%><tr><td><font color="blue">fedex_plus warnings</font></td></tr><tr><td><table  width=100%>"
            echo "<tr><th>Line</th><th>Message</th>$w</table></td></tr></table>"
        fi
        if [ "x$e" != "x" ]; then
            echo "<table width=100%><tr><td><font color="red">fedex_plus errors</font></td></tr><tr><td><table  width=100%>"
            echo "<tr><th>Line</th><th>Message</th>$e</table></td></tr></table>"
        fi
        o=`sed -e 's|^.*:[0-9]\+: WARNING: .*$||g;' -e 's|^.*:[0-9]\+: --ERROR: .*$||g;' $i`
        if [ "x$e" != "x" ]; then
            echo "<br><h3>Other text in file</h3>"
            echo "<pre>"
            echo $o
            echo "</pre>"
        fi
    ) >$out_dir/fedex_$1_stderr.md
}

function build_one {
    #set $i to the schema name, all caps (to match fedex_plus output)
    i=`sed -ne '0,/^\s*SCHEMA/s/^.*SCHEMA\s\+\(.*\);.*$/\1/p;' $1|tr a-z A-Z`

    echo "Running fedex_plus for $i..."
    make -f data/CMakeFiles/sdai_$i.dir/build.make $i/compstructs.cc >/dev/null  2>"$out_dir/fedex_"$i"_stderr.txt" >"$out_dir/fedex_"$i"_stdout.txt" && \
    $mk sdai_$i >/dev/null 2>"$out_dir/compile_libsdai_"$i"_stderr.txt" && \
    $mk p21read_sdai_$i >/dev/null 2>"$out_dir/compile_p21read_sdai_"$i"_stderr.txt"

    #todo: test p21read_sdai_$i

    #create a table for this schema in the markdown file
    echo "<tr><td><table width=100%><tr><th>Schema $i</th></tr><tr><td>" >>$matrix_file
    j=`echo $1|sed -e "s|^.*/\(.*\.exp\)$|\1|;"`  #j is the name of the express file
    echo "$j</td></tr><tr><td><table border=1>" >>$matrix_file
    count_we "fedex_plus (generate c++)" $out_dir/fedex_"$i"_stderr.txt
    count_we "gcc (compile lib)" $out_dir/compile_libsdai_"$i"_stderr.txt
    count_we "gcc (compile p21read)" $out_dir/compile_p21read_sdai_"$i"_stderr.txt
    echo "</table></td></tr>" >>$matrix_file
    echo "</table></td></tr>" >>$matrix_file
    
    if [ -s $out_dir/fedex_"$i"_stderr.txt ]; then
        fedex_details $i
    fi
}

schemas="../data/example/example.exp;../data/ap227/ap227.exp;../data/ap214e3/AP214E3_2010.exp;../data/select.exp;../data/ap203e2/ap203e2_mim_lf.exp;../data/ap203/203wseds.exp;../data/ifc2x3/IFC2X3_TC1.exp;../data/pdmnet.exp;../data/ifc2x4/IFC2X4_RC2.exp;/opt/step/notingit/part409cdts_wg3n2617mim_lf.exp;/opt/step/notingit/ap242_managed_model_based_3d_engineering_mim_lf.exp;/opt/step/notingit/AP224E3_wg3n1941.exp;/opt/step/notingit/AP238E2_aim_lf_20100903.exp"

#schemas="../data/example/example.exp;../data/ap203/203wseds.exp;../data/ap203e2/ap203e2_mim_lf.exp"

function build {
     cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC_LIBS=OFF -DBUILD_SCHEMAS=$schemas

    # build all parts of scl that are necessary for schema libs to build
    echo Building SCL...
    $mk stepeditor | grep "^Linking"
    $mk fedex_plus stepdai | grep "^Linking"

    for h in `echo $schemas | sed -e 's/;/\n/g;'`
    do
        build_one $h
    done
}

mkdir -p build_all && cd build_all && rm -rf * && mkdir $out_dir
echo "## `date`" >$matrix_file
git log --decorate=short -n1 |\
    head -n1 |\
    sed -e 's|^.*commit \([a-z0-9]\+\) .*$|### commit [\1](http://github.com/mpictor/StepClassLibrary/commit/\1)|;' >>$matrix_file
echo "<table border=1>" >>$matrix_file

build

echo "</table>" >>$matrix_file
