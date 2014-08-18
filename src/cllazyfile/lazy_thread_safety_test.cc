#include "lazyTypes.h"
#include "lazyInstMgr.h"
#include "SdaiSchemaInit.h"
#include "instMgrHelper.h"

// Registry will be needed for checkLazyLoadingSafety & checkLazyLoadingSafetyWithAdapter checks.
#ifndef NO_REGISTRY
# include "schema.h"
#endif //NO_REGISTRY

#include <algorithm>

#ifdef HAVE_STD_THREAD
# include <thread>
#else
# error Need std::thread for this test!
#endif

/// It copies the keys of the _refs judy structure provided to it into the vector realRefs
void prepareRealRefs ( instanceRefs_t * _refs, instanceRefs &realRefs ) {

    instanceID last = _refs->end().key;
    instanceID current = _refs->begin().key;

    while( current != last ) {                
        realRefs.push_back( current );
        current = _refs->next().key;
    }

    realRefs.push_back( last );
}

/// It copies the keys of the _refs judy structure provided to it into the vector realRefs
void prepareRealRefsFromStreamPos ( instanceStreamPos_t * _spos, instanceRefs &realRefs ) {

    instanceID last = _spos->end().key;
    instanceID current = _spos->begin().key;

    while( current != last ) {
        realRefs.push_back( current );
        current = _spos->next().key;
    }

    realRefs.push_back( last );
}


/// Used by an individual thread to iterate over the keys of the _fwdRefs / _revRefs multiple times. For each iteration it expects the order of the keys to be the same as dictated by realRefs.
void iterateOverRefs ( lazyInstMgr *mgr, instanceRefs &realRefs, bool forward, bool * success ) {
    const int iterations = 1000;
    int i, k, instances = realRefs.size();
    instanceID current;
    instanceRefs_t * _refs;
 
    for( k = 0; k < iterations; k++ ) {
        _refs = forward ? mgr->getFwdRefsSafely() : mgr->getRevRefsSafely();

        current = _refs->begin().key;
        for( i = 0; i < instances; i++ ) {

            if( current != realRefs[i] ) {
                break;
            }

            current = _refs->next().key;
        }

        *success = *success && ( i == instances );
    }
}

/// Checks the thread safety of _fwdRefs (_revRefs) when the forward value provided to it is true (false).
void checkRefsSafety( char * fileName, bool forward ) {
    instanceRefs realRefs; 
    lazyInstMgr * mgr = new lazyInstMgr;
    mgr->openFile( fileName );

    if( forward ) {
        std::cout << "Checking thread safety while iterating over forward references..." ;
        prepareRealRefs( mgr->getFwdRefs(), realRefs );
    } else {
        std::cout << "Checking thread safety while iterating over backward references..." ;
        prepareRealRefs( mgr->getRevRefs(), realRefs );
    }

    bool success[2] = { true, true };
    std::thread first( iterateOverRefs, mgr, realRefs, forward, &success[0] );
    std::thread second( iterateOverRefs, mgr, realRefs, forward, &success[1] );

    first.join();
    second.join();

    if( success[0] && success[1] ) {
        std::cout << "..PASS!" << std::endl;
    } else {
        std::cout << "...FAIL!" << std::endl;

        if( !success[0] ) {
            std::cout << "\tThread 0 could not iterate properly" << std::endl;
        }

        if( !success[1] ) {
            std::cout << "\tThread 1 could not iterate properly" << std::endl;
        }
    }
    
    std::cout << std::endl;
    delete mgr;
}

/// Checks thread safety of getFwdRefs();
void checkFwdRefsSafety( char * fileName ) {
    checkRefsSafety( fileName, true );
}

/// Checks thread safety of getFwdRefs();
void checkRevRefsSafety( char * fileName ) {
    checkRefsSafety( fileName, false );
}

/// A vector of some common types which are present in most step files. The types persent in this vector are used to check for thread safety.
const std::vector< std::string > commonTypes { "PRODUCT_DEFINITION", "CARTESIAN_POINT", "NODE", "CLASSIFICATION_ROLE" };

/// Prepares a vector containing pointers to list of instances of the above types.
void prepareSameTypeInstances( lazyInstMgr *mgr, std::vector< const instanceRefs * > &sameTypeInstances ) {

    for( int i = 0; i < ( int )commonTypes.size(); i++ ) {
        sameTypeInstances.push_back( mgr->getInstances( commonTypes[i] ) );
    }
}

/// compares the list of both vectors. Returns true when the lists are same.
bool compareTypeLists( const instanceRefs * v1, const instanceRefs * v2 ) {

    if( v1 == NULL || v1->empty() ) {
        //return true if both lists are NULL or EMPTY. false if only one is NULL or EMPTY
        return ( v2 == NULL || v2->empty() );
    } else {
        //An O(1) operation since if the first element is same so will be the rest. (std::vectors are concurrent read safe)
        return ( v1->at( 0 ) == v2->at( 0 ) );
    }
}

/// compares the original instances lists with the thread's own view of instances list.
void iterateTypeLists( lazyInstMgr * mgr, std::vector< const instanceRefs * > &sameTypeInstances, bool * success ) {
    const int iterations = 100000;
    const instanceRefs * refs;
    int i, k, instances = commonTypes.size();
    for( k = 0; k < iterations; k++ ) {

        for( i = 0; i < instances; i++ ) {

            refs = mgr->getInstancesSafely( commonTypes[i] );

            if( !compareTypeLists( refs, sameTypeInstances[i] ) ) {
                break;
            }
        }

        *success = *success && ( i == instances );
    }
}

/// checks the thread safety of getInstances();
void checkTypeInstancesSafety( char * fileName ) {
    lazyInstMgr * mgr = new lazyInstMgr;
    mgr->openFile( fileName );

    std::cout << "Checking thread safety while getting insances of a common types..." ;

    std::vector< const instanceRefs * > sameTypeInstances;
    prepareSameTypeInstances( mgr, sameTypeInstances );

    bool success[2] = { true, true };
    std::thread first( iterateTypeLists, mgr, sameTypeInstances, &success[0] );
    std::thread second( iterateTypeLists, mgr, sameTypeInstances, &success[1] );

    first.join();
    second.join();

    if( success[0] && success[1] ) {
        std::cout << "..PASS!" << std::endl;
    } else {
        std::cout << "...FAIL!" << std::endl;

        if( !success[0] ) {
            std::cout << "\tThread 0 could not get insances properly" << std::endl;
        }

        if( !success[1] ) {
            std::cout << "\tThread 1 could not get insances properly" << std::endl;
        }
    }

    std::cout << "\t\tNumber of instances of different types: ";
    for( int i = 0; i < ( int )commonTypes.size(); i++ ) {
        std::cout << ( sameTypeInstances[i] == NULL ? 0 : sameTypeInstances[i]->size() ) << " ";
    }

    std::cout << std::endl << std::endl;
    delete mgr;
}

/// load instances found in _refs into the instancesLoaded. After doing this once, it iterates over the _refs and reports any changes in loaded value. 
/// Either directly calls loadInstance or uses an adapter, (determined by the value of useAdapter)
void loadInstancesFromList( lazyInstMgr * mgr, instanceRefs * _refs, instancesLoaded_t * myInstances, bool useAdapter, bool * success) {
    const int instances = _refs->size();
    SDAI_Application_instance * sdaiInstance;
    int i;
    // Initial insertion into myInstances
    for( i = 0; i < instances; i++ ) {
        if( useAdapter ) {
            sdaiInstance = mgr->getAdapter()->FindFileIdSafely( _refs->at( i ) )->GetSTEPentitySafely();
        } else {
            sdaiInstance = mgr->loadInstanceSafely( _refs->at( i ) );
        }

        myInstances->insert( _refs->at( i ), sdaiInstance );
    }

    // For each instance comparing the new pointer with the original pointer
    for( i = 0; i < instances; i++ ) {
        if( useAdapter ) {
            sdaiInstance = mgr->getAdapter()->FindFileIdSafely( _refs->at( i ) )->GetSTEPentitySafely();
        } else {
            sdaiInstance = mgr->loadInstanceSafely( _refs->at( i ) );
        }

        if( myInstances->find( _refs->at( i ) ) != sdaiInstance ) {
			//the old value has been overwritten. An object lazy-loaded twice. Not Good!!!
            *success = false;
        }
    }
}

//compares the instances present in loadedOnT1 and loadedOnT2. If both have an instance belonging to a particular instanceID then the instances should be same
bool compareLoadedInstances( instanceRefs * toBeLoadedOnT1, instancesLoaded_t * loadedOnT1, instanceRefs * toBeLoadedOnT2, instancesLoaded_t * loadedOnT2 ) {
    SDAI_Application_instance * sdaiInstance;

	//Iterate over instanceID's in toBeLoadedOnT1
    int i, instances = toBeLoadedOnT1->size();
    for( i = 0; i < instances; i++ ) {
        sdaiInstance = loadedOnT2->find( toBeLoadedOnT1->at( i ) );
        if( sdaiInstance != NULL ) {
            if( sdaiInstance != loadedOnT1->find( toBeLoadedOnT1->at( i ) ) ) {
                return false;
            }
        }
    }       
   
	//Iterate over instanceID's in toBeLoadedOnT2
    instances = toBeLoadedOnT2->size();
    for( i = 0; i < instances; i++ ) {
        sdaiInstance = loadedOnT1->find( toBeLoadedOnT2->at( i ) );
        if( sdaiInstance != NULL ) {
            if( sdaiInstance != loadedOnT2->find( toBeLoadedOnT2->at( i ) ) ) {
                return false;
            }
        }
    }       
     
    return true;
}

#ifndef NO_REGISTRY
//checks thread safety of loadInstance. (Also of instMgrAdapter if useAdapter is TRUE)
void checkLazyLoadingSafety( char * fileName, bool useAdapter=false ) {

    instanceRefs instancesToBeLoadedFwd, instancesToBeLoadedRev;
    instanceRefs * toBeLoadedOnT1, * toBeLoadedOnT2;
    instancesLoaded_t loadedOnT1, loadedOnT2;

    lazyInstMgr * mgr = new lazyInstMgr;
    mgr->initRegistry( SchemaInit );
    mgr->openFile( fileName );

    std::cout << "Checking thread safety in Lazy Loading ("<< ( useAdapter ? "with" : "without" ) << " Adapter)...";

    prepareRealRefs( mgr->getFwdRefs(), instancesToBeLoadedFwd );
    prepareRealRefs( mgr->getRevRefs(), instancesToBeLoadedRev );

    bool intraThreadSuccess[2] = { true, true };
    bool interThreadSuccess = true;
    const int iterations = 50;
    for( int i = 0; i < 2 * iterations; i++ ) {
        // First set iterations both threads will try to load from same list. 
        toBeLoadedOnT1 = &instancesToBeLoadedFwd;
        toBeLoadedOnT2 = i < iterations ? &instancesToBeLoadedFwd : &instancesToBeLoadedRev;

        std::thread first( loadInstancesFromList, mgr, toBeLoadedOnT1, &loadedOnT1, useAdapter, &intraThreadSuccess[0] );
        std::thread second( loadInstancesFromList, mgr, toBeLoadedOnT2, &loadedOnT2, useAdapter, &intraThreadSuccess[1] );

        first.join();
        second.join();
        
        interThreadSuccess &= compareLoadedInstances( toBeLoadedOnT1, &loadedOnT1, toBeLoadedOnT2, &loadedOnT2 );
        mgr->unloadAllInstances();

        loadedOnT1.clear();
        loadedOnT2.clear();
    } 

    if( intraThreadSuccess[0] && intraThreadSuccess[1] && interThreadSuccess ) {
        std::cout << "..PASS!" << std::endl;
    } else {
        std::cout << "...FAIL!" << std::endl;

        if( !intraThreadSuccess[0] ) {
            std::cout << "\tThread 0: Difference in instances loaded within 1st and 2nd iterations" << std::endl;
        }

        if( !intraThreadSuccess[1] ) {
            std::cout << "\tThread 1: Difference in instances loaded within 1st and 2nd iterations" << std::endl;
        }

        if( !interThreadSuccess ) {
            std::cout << "\tDifference in instances loaded by the 2 threads" << std::endl;
        }
    }

    std::cout << std::endl;
    delete mgr;
}

//checks thread safety of lazyloading along with that of instMgrAdapter
void checkLazyLoadingSafetyWithAdapter( char * fileName ) {
    checkLazyLoadingSafety( fileName, true );
}
#endif //NO_REGISTRY

/// enum elements represent the status used checking thread safety of openFile
enum lazyInstMgrStatus {
    FWD_REFS,
    REV_REFS,
    TYPES,
    TOTAL_COUNT,
    COUNT_DATA_SECTIONS,
    LONGEST_TYPE,
    STREAM_POS,
    OK
};

std::string getLazyInstMgrStatusString( lazyInstMgrStatus lIMStatus ) {
    switch( lIMStatus ) {
        case FWD_REFS: return "FWD_REFS";
        case REV_REFS: return "REV_REFS";
        case TYPES: return "TYPES";
        case TOTAL_COUNT: return "TOTAL_COUNT";
        case COUNT_DATA_SECTIONS: return "COUNT_DATA_SECTIONS";
        case LONGEST_TYPE: return "LONGEST_TYPE";
        case STREAM_POS: return "STREAM_POS";
        case OK: return "OK";
        default: return "Unknown Type";
    }
}

/// compares the given instanceRefs_t.
bool compareRefs( instanceRefs_t * _refs1, instanceRefs_t * _refs2 ) {
    instanceRefs refs1, refs2;
    prepareRealRefs( _refs1, refs1 );
    prepareRealRefs( _refs2, refs2 );

    //both vectors should have same number of elements
    if( refs1.size() != refs2.size() ) {
        return false;
    }

    for( size_t i = 0; i < refs1.size(); i++ ) {
        const instanceRefs * deps1 = _refs1->find( refs1[i] );
        const instanceRefs * deps2 = _refs2->find( refs1[i] );

        //Just compare the size of both (value) vectors is sufficient
        if( deps2 == NULL || deps1->size() != deps2->size() ) {
            return false;
        }
    }

    return true;
}

/// compares the given instance vectors. Returns true if both have same elements (irrespective of their order)
bool compareInstanceRefs( const instanceRefs * refs1, const instanceRefs * refs2 ) {
    if( refs1 == NULL ) {
        return ( refs2 == NULL ) || ( refs2->size() == 0 );
    }

    if( refs2 == NULL ) {
        return ( refs1 == NULL ) || ( refs1->size() == 0 );
    }

    if( refs1->size() != refs2->size() ) {
        return false;
    }

    for( size_t i = 0; i < refs1->size(); i++ )
    {
        if( std::find( refs2->begin(), refs2->end(), refs1->at( i ) ) == refs2->end() ) {
            return false;
        }
    }

    return true;
}

/// compares _instanceTypes data structure of both managers.
bool compareTypes( lazyInstMgr * mgr1, lazyInstMgr * mgr2 ) {
    int i, types = commonTypes.size();
    for( i = 0; i < types; i++ ) {
        const instanceRefs * refs1 = mgr1->getInstances( commonTypes[i] );
        const instanceRefs * refs2 = mgr2->getInstances( commonTypes[i] );
        if( !compareInstanceRefs( refs1, refs2 ) ) {
            return false;
        }
    }

    return true;
}

/// compares the given positionAndSection vectors. Returns true if both have same elements (irrespective of their order)
bool comparePosVectors( const std::vector< positionAndSection > * pos1, const std::vector< positionAndSection > * pos2 ) {
    if( pos1->size() != pos2->size() ) {
        return false;
    }

    unsigned i, j;
    for( i = 0; i < pos1->size(); i++ ) {
        bool result = false;
        for( j = 0; j < pos2->size(); j++ ) {
            positionAndSection ps1, ps2;
            ps1 = pos1->at( i );
            ps2 = pos2->at( j );

            //only checking the offset as the sectionID may be different
            if( ( ps1 & 0xFFFFFFFFFFFF ) == ( ps2 & 0xFFFFFFFFFFFF ) ) {
                result = true;
                break;
            }
        }

        if( !result ) {
            return false;
        }
    }

    return true;
}

/// For every instance compares its corresponding stream position vector in both lazyInstMgr. Returns false in case of any disprenency
bool compareStreamPos( lazyInstMgr * mgr1, lazyInstMgr * mgr2 ) {

    instanceRefs refs1, refs2;
    instanceStreamPos_t * _pos1 = mgr1->getInstanceStreamPos();
    instanceStreamPos_t * _pos2 = mgr2->getInstanceStreamPos();

    prepareRealRefsFromStreamPos( _pos1, refs1 );
    prepareRealRefsFromStreamPos( _pos2, refs2 );

    // number of instances in both managers must be equal
    if( refs1.size() != refs2.size() ) {
        return false;
    }

    const std::vector< positionAndSection > * pos1, * pos2;

    unsigned int i;
    for( i = 0; i < refs1.size(); i++ ) {
        pos1 = _pos1->find( refs1[i] );
        pos2 = _pos2->find( refs1[i] );
        //checking pos1 and pos2 for similarity
        if( !comparePosVectors( pos1, pos2 ) ) {
            return false;
        }
    }

    return true;
}

/// compares various data structures of both lazyInstMgr's and returns OK if both are similar, else returns the point of difference.
lazyInstMgrStatus compareLazyInstMgr( lazyInstMgr * mgr1, lazyInstMgr * mgr2 ) {

    if( !compareRefs( mgr1->getFwdRefs(), mgr2->getFwdRefs() ) ) {
        return FWD_REFS;
    }

    if( !compareRefs( mgr1->getRevRefs(), mgr2->getRevRefs() ) ) {
        return REV_REFS;
    }

    // compares getType and getNumTypes
    if( !compareTypes( mgr1, mgr2 ) ) {
        return TYPES;
    }

    if( mgr1->totalInstanceCount() != mgr2->totalInstanceCount() ) {
        return TOTAL_COUNT;
    }

    if( mgr1->countDataSections() != mgr2->countDataSections() ) {
        return COUNT_DATA_SECTIONS;
    }

    if( mgr1->getLongestTypeName().length() != mgr2->getLongestTypeName().length() ) {
        return LONGEST_TYPE;
    }

    if( !compareStreamPos( mgr1, mgr2 ) ) {
        return STREAM_POS;
    }

    return OK;
}

/// used by different threads to open a step file
void openFile( lazyInstMgr * mgr, char * fileName ) {
    mgr->openFileSafely( fileName );
}

/// checks the thread safety by opening multiple files in parallel
void checkOpenFileSafety( char * file1, char * file2 ) {
    lazyInstMgr * e_mgr = new lazyInstMgr; //expected lazyInstMgr

    std::cout << "Checking thread safety while opening multiple files in parallel...";

    e_mgr->openFile( file1 );
    e_mgr->openFile( file2 );

    lazyInstMgrStatus compareResult = OK;
    const int iterations = 100;
    for( int i = 0; i < iterations; i++ ) {
        lazyInstMgr * a_mgr = new lazyInstMgr; //actual lazyInstMgr

        std::thread first( openFile, a_mgr, file1 );
        std::thread second( openFile, a_mgr, file2 );

        first.join();
        second.join();

        compareResult = compareLazyInstMgr( e_mgr, a_mgr );

        delete a_mgr;

        if( compareResult != OK ) {
            break;
        }
    }

    if( compareResult == OK ) {
        std::cout << "..PASS!" << std::endl;
    } else {
        std::cout << "...FAIL!" << std::endl;
        std::cout << "\tDisperency in " << getLazyInstMgrStatusString( compareResult ) << " structure" << std::endl;
    }

    std::cout << std::endl;
    delete e_mgr;
}


/// These tests were run on stepcode/data/cd209/ATS1-out.stp && stepcode/data/cd209/ATS10-out.stp (Bigger files may take a longer time)
int main( int argc, char ** argv ) {
    if( argc != 3 ) {
        std::cerr << "Expected two argument, given " << argc - 1 << ". Exiting." << std::endl;
        exit( EXIT_FAILURE );
    }

    checkFwdRefsSafety( argv[1] );
    checkRevRefsSafety( argv[1] );

    checkTypeInstancesSafety( argv[1] );

#ifndef NO_REGISTRY
    checkLazyLoadingSafety( argv[1] );
    checkLazyLoadingSafetyWithAdapter( argv[1] );
#endif //NO_REGISTRY

    checkOpenFileSafety( argv[1], argv[2] );
}

