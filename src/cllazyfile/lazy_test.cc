#include "lazyInstMgr.h"
#include <sc_benchmark.h>

void countTypeInstances( lazyInstMgr & mgr, std::string type ) {
    instanceTypeMMap_range range = mgr.getInstances( type );
    int count = 0;
    instanceID ex = range.first->second;
    for( ; range.first != range.second; range.first++ ) {
        count++;
    }
    std::cerr << type <<" instances: " << count;
    if( count ) {
        std::cerr << " example: #" << ex; //this is the last based upon multimap hash value, not based on file order
    }
    std::cerr << std::endl;
    return;
}

int main (int argc, char ** argv ) {
    if( argc != 2 ) {
        std::cerr << "Expected one argument, given " << argc <<". Exiting." << std::endl;
        exit( EXIT_FAILURE );
    }
    benchmark stats( "p21 lazy load test" );

    lazyInstMgr mgr;
    mgr.openFile( argv[1] );

    std::cerr << "Total instances: " << mgr.getInstanceCount() << std::endl;
    countTypeInstances( mgr, "CARTESIAN_POINT" );
    countTypeInstances( mgr, "POSITIVE_LENGTH_MEASURE" );
    countTypeInstances( mgr, "VERTEX_POINT" );

    //complex instances
    std::cerr << "Complex";
    countTypeInstances( mgr, "" );

    std::cerr << "Longest type name: " << mgr.getLongestTypeName() << std::endl;
    std::cerr << "Total types: " << mgr.getNumTypes() << std::endl;
    std::cerr << "Bytes read by sectionReader::findNormalString(): " << sectionReader::findStringByteCount() << std::endl;

    stats.stop();
    stats.out();
}