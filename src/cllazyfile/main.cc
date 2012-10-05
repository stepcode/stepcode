#include "lazyInstMgr.h"
#include <sc_benchmark.h>

int main (int argc, char ** argv ) {
    benchmark stats( "p21 lazy load test" );

    std::string n;
    if( argc == 2 ) {
        n = argv[1];
    } else {
        n = "somefile.stp";
    }
    lazyInstMgr mgr;
    mgr.openFile( n );
    instanceTypeMMap_range r,s;
    r = mgr.getInstances("POSITIVE_LENGTH_MEASURE");
    s = mgr.getInstances("VERTEX_POINT");
    //count those

    stats.stop();
    stats.out();
}