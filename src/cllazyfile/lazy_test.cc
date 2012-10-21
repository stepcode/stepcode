#include "lazyInstMgr.h"
#include <sc_benchmark.h>

void countTypeInstances( lazyInstMgr & mgr, std::string type ) {
    int count = mgr.countInstances( type );
    std::cout << type << " instances: " << count;
    if( count ) {
        instanceTypeMMap_range range = mgr.getInstances( type );
        instanceID ex = range.first->second;
        std::cout << " example: #" << ex; //this is the last based upon multimap hash value, not based on file order
    }
    std::cout << std::endl;
    return;
}

void printRefs( lazyInstMgr & mgr ) {
    instanceRefMap_range r = mgr.getFwdRefs();
    instanceRefMap_t::const_iterator it = r.first;
    for( ; it != r.second; it++ ) {
        std::cout << "Example of forward references - Instance #" << it->first << " makes use of ";
        auto frit = it->second->begin();
        for( ; frit != it->second->end(); frit++ ) {
            std::cout << *frit << " ";
        }
        std::cout << std::endl;
        break;     //comment this out to loop through all
    }

    r = mgr.getRevRefs();
    it = r.first;
    for( ; it != r.second; it++ ) {
        std::cout << "Example of reverse references - Instance #" << it->first << " is referred to by ";
        auto rrit = it->second->begin();
        for( ; rrit != it->second->end(); rrit++ ) {
            std::cout << *rrit << " ";
        }
        std::cout << std::endl;
        break;     //comment this out to loop through all
    }
}

int main (int argc, char ** argv ) {
    if( argc != 2 ) {
        std::cerr << "Expected one argument, given " << argc <<". Exiting." << std::endl;
        exit( EXIT_FAILURE );
    }
    benchmark stats( "p21 lazy load test" );

    lazyInstMgr mgr;
    mgr.openFile( argv[1] );

    std::cout << "Total instances: " << mgr.countInstances() << std::endl;
    countTypeInstances( mgr, "CARTESIAN_POINT" );
    countTypeInstances( mgr, "POSITIVE_LENGTH_MEASURE" );
    countTypeInstances( mgr, "VERTEX_POINT" );

    //complex instances
    std::cout << "Complex";
    countTypeInstances( mgr, "" );

    std::cout << "Longest type name: " << mgr.getLongestTypeName() << std::endl;
    std::cout << "Total types: " << mgr.getNumTypes() << std::endl;

    std::cout << "\nReferences\n==============" << std::endl;
    printRefs( mgr );

    stats.stop();
    stats.out();
}