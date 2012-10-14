#include "lazyInstMgr.h"
#include <sc_benchmark.h>

void countTypeInstances( lazyInstMgr & mgr, std::string type ) {
    int count = mgr.countInstances( type );
    std::cout << type <<" instances: " << count;
    if( count ) {
        instanceTypeMMap_range range = mgr.getInstances( type );
        instanceID ex = range.first->second;
        std::cout << " example: #" << ex; //this is the last based upon multimap hash value, not based on file order
    }
    std::cout << std::endl;
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

    std::cout << "Total instances: " << mgr.getInstanceCount() << std::endl;
    countTypeInstances( mgr, "CARTESIAN_POINT" );
    countTypeInstances( mgr, "POSITIVE_LENGTH_MEASURE" );
    countTypeInstances( mgr, "VERTEX_POINT" );

    //complex instances
    std::cout << "Complex";
    countTypeInstances( mgr, "" );

    std::cout << "Longest type name: " << mgr.getLongestTypeName() << std::endl;
    std::cout << "Total types: " << mgr.getNumTypes() << std::endl;
    std::cout << "Bytes read by sectionReader::findNormalString(): " << sectionReader::findStringByteCount() << std::endl;

    stats.stop();
    stats.out();
}