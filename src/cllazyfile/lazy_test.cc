#include "lazyInstMgr.h"
#include <sc_benchmark.h>
#include "SdaiSchemaInit.h"
#include "scl_memmgr.h"

void fileInfo( lazyInstMgr& mgr, fileID id ) {
    instancesLoaded_t headerInsts = mgr.getHeaderInstances( 0 );
    instancesLoaded_t::iterator it =  headerInsts.find( 3 );
    SDAI_Application_instance * hdrInst = it->second;
    if( ( hdrInst != 0 ) && ( hdrInst->STEPfile_id == 3 ) ) {
        SdaiFile_schema * fs = dynamic_cast< SdaiFile_schema * >( hdrInst );
        if( fs ) {
            StringAggregate_ptr p = fs->schema_identifiers_();
            StringNode * sn = (StringNode *) p->GetHead();
            std::cout << "Schema(s): ";
            while( sn ) {
                std::cout << sn->value.c_str() << " ";
                sn = (StringNode *) sn->NextNode();
            }
            std::cout << std::endl;
        }
    }
}
void countTypeInstances( lazyInstMgr & mgr, std::string type ) {
    int count = mgr.countInstances( type );
    std::cout << type << " instances: " << count;
    if( count ) {
        instanceTypeMMap_range range = mgr.getInstances( type );
        instanceID ex = range.first->second;
        std::cout << " -- example: #" << ex; //this is the last based upon multimap hash value, not based on file order
    }
    std::cout << std::endl;
    return;
}

void printRefs( lazyInstMgr & mgr ) {
    std::cout << "\nReferences\n==============\n";
    instanceRefMap_range r = mgr.getFwdRefs();
    instanceRefMap_t::const_iterator it;
    if( r.first == r.second ) {
        std::cout << "No forward references" << std::endl;
    } else {
        it = r.first;
        for( ; it != r.second; it++ ) {
            std::cout << "Example of forward references - Instance #" << it->first << " makes use of ";
            auto frit = it->second->begin();
            for( ; frit != it->second->end(); frit++ ) {
                std::cout << *frit << " ";
            }
            std::cout << std::endl;
            break;     //comment this out to loop through all
        }
    }
    r = mgr.getRevRefs();
    if( r.first == r.second ) {
        std::cout << "No reverse references" << std::endl;
    } else {
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
}

int main (int argc, char ** argv ) {
    if( argc != 2 ) {
        std::cerr << "Expected one argument, given " << argc <<". Exiting." << std::endl;
        exit( EXIT_FAILURE );
    }
    lazyInstMgr * mgr = new lazyInstMgr;
    benchmark stats( "p21 lazy load: scanning the file\n================================\n" );
    mgr->openFile( argv[1] );
    stats.out();

    stats.reset( "p21 lazy load: gathering statistics\n===================================\n" );
    fileInfo( *mgr, 0 );

    std::cout << "Total instances: " << mgr->countInstances() << std::endl;
    countTypeInstances( *mgr, "CARTESIAN_POINT" );
    countTypeInstances( *mgr, "POSITIVE_LENGTH_MEASURE" );
    countTypeInstances( *mgr, "VERTEX_POINT" );

    //complex instances
    std::cout << "Complex";
    countTypeInstances( *mgr, "" );

    std::cout << "Longest type name: " << mgr->getLongestTypeName() << std::endl;
    std::cout << "Total types: " << mgr->getNumTypes() << std::endl;

    printRefs( *mgr );
    stats.out();
    stats.reset( "p21 lazy load: freeing memory\n=============================\n" );
    delete mgr;
    //stats will print from its destructor
}