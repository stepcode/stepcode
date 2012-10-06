#include "lazyInstMgr.h"
#include <sc_benchmark.h>

void countTypeInstances( lazyInstMgr & mgr, std::string type ) {
    instanceTypeMMap_range range = mgr.getInstances( type );
    int count = 0;
    for( ; range.first != range.second; range.first++ ) {
        count++;
    }
    std::cerr << type <<" instances: " << count << std::endl;
    return;
}

int main (int argc, char ** argv ) {
    if( argc != 2 ) {
        std::cerr << "Expected one argument, given " << argc <<". Exiting." << std::endl;
    }
    benchmark stats( "p21 lazy load test" );

    lazyInstMgr mgr;
    mgr.openFile( argv[1] );

    std::cerr << "Total instances: " << mgr.getInstanceCount() << std::endl;
    countTypeInstances( mgr, "CARTESIAN_POINT" );
    countTypeInstances( mgr, "POSITIVE_LENGTH_MEASURE" );
    countTypeInstances( mgr, "VERTEX_POINT" );
    std::cerr << "Longest type name: " << mgr.getLongestTypeName() << std::endl;
    std::cerr << "Total types: " << mgr.getNumTypes() << std::endl;

    stats.stop();
    stats.out();
}