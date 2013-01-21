#include "lazyInstMgr.h"
#include <sc_benchmark.h>
#include "SdaiSchemaInit.h"
#include "scl_memmgr.h"

void fileInfo( lazyInstMgr& mgr, fileID id ) {
    instancesLoaded_t * headerInsts = mgr.getHeaderInstances( id );
    SDAI_Application_instance * hdrInst;
#ifdef HAVE_JUDY
    hdrInst = headerInsts->find( 3 );
#else //HAVE_JUDY
    instancesLoaded_t::iterator it =  headerInsts.find( 3 );
    hdrInst = it->second;
#endif //HAVE_JUDY
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
        instanceID ex;
#ifdef HAVE_JUDY
        ex = ( * mgr.getInstances( type ) )[ 0 ];
#else //HAVE_JUDY
        instanceTypeMMap_range range = mgr.getInstances( type );
        ex = range.first->second; //this is the last based upon multimap hash value, not based on file order
#endif //HAVE_JUDY
        std::cout << " -- example: #" << ex;
    }
    std::cout << std::endl;
    return;
}

#ifdef HAVE_JUDY
instanceID printRefs1( instanceRefs_t * refs, const char * desc ) {
    instanceID id = 0;
    instanceRefs_t::cpair p = refs->begin();
    instanceRefs_t::cvector * v = p.value;
    if( !v ) {
        std::cout << "No " << desc << " references" << std::endl;
    } else {
        instanceRefs_t::cvector::const_iterator it( v->begin() ), end( v->end() );
        std::cout << "Example of " << desc << " references - Instance #" << p.key << " refers to " << v->size() << " other instances: ";
        for( ; it != end; it++ ) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
        id = p.key;
    }
    return id;
}
#else //HAVE_JUDY
instanceID printRefs1( instanceRefMap_range r, const char * desc ) {
    instanceRefMap_t::const_iterator it;
    if( r.first == r.second ) {
        std::cout << "No " << desc << " references" << std::endl;
    } else {
        it = r.first;
        for( ; it != r.second; it++ ) {
            std::cout << "Example of " << desc << " references - Instance #" << it->first << " makes use of ";
            auto frit = it->second->begin();
            for( ; frit != it->second->end(); frit++ ) {
                std::cout << *frit << " ";
            }
            std::cout << std::endl;
            break;     //comment this out to loop through all
        }
    }
}
#endif //HAVE_JUDY

instanceID printRefs( lazyInstMgr & mgr ) {
    instanceID id;
    std::cout << "\nReferences\n==============\n";
    id = printRefs1( mgr.getFwdRefs(), "forward" );
    printRefs1( mgr.getRevRefs(), "reverse" );
    std::cout << std::endl;
    return id;
}

int main (int argc, char ** argv ) {
    instanceID instWithRef;
    if( argc != 2 ) {
        std::cerr << "Expected one argument, given " << argc << ". Exiting." << std::endl;
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
//     std::cout << "Total types: " << mgr->getNumTypes() << std::endl;

    instWithRef = printRefs( *mgr );

//     std::cout << "Number of data section instances fully loaded: " << mgr->countInstances() << std::endl;
    std::cout << "Loading #" << instWithRef;
    SDAI_Application_instance* inst = mgr->loadInstance( instWithRef );
    std::cout << " which is of type " << inst->EntityName() << std::endl;
//     std::cout << "Number of instances loaded now: " << mgr->countInstances() << std::endl;

    stats.out();
    stats.reset( "p21 lazy load: freeing memory\n=============================\n" );
    delete mgr;
    //stats will print from its destructor
}