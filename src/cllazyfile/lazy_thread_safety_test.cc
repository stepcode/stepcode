#include "lazyTypes.h"
#include "lazyInstMgr.h"
#include "SdaiSchemaInit.h"
#include "instMgrHelper.h"

#include "schema.h"

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
void loadInstancesFromList( lazyInstMgr * mgr, instanceRefs * _refs, instancesLoaded_t * myInstances, bool * success) {
    const int instances = _refs->size();
    SDAI_Application_instance * sdaiInstance;
    int i;
    // Initial insertion into myInstances
    for( i = 0; i < instances; i++ ) {
        sdaiInstance = mgr->loadInstanceSafely( _refs->at( i ) );
        myInstances->insert( _refs->at( i ), sdaiInstance );
    }

    // For each instance comparing the new pointer with the original pointer
    for( i = 0; i < instances; i++ ) {
        sdaiInstance = mgr->loadInstanceSafely( _refs->at( i ) );

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

//checks thread safety of loadInstance.
void checkLazyLoadingSafety( char * fileName ) {

    instanceRefs instancesToBeLoadedFwd, instancesToBeLoadedRev;
    instanceRefs * toBeLoadedOnT1, * toBeLoadedOnT2;
    instancesLoaded_t loadedOnT1, loadedOnT2;

    lazyInstMgr * mgr = new lazyInstMgr;
    mgr->initRegistry( SchemaInit );
    mgr->openFile( fileName );

    std::cout << "Checking thread safety in Lazy Loading...";

    prepareRealRefs( mgr->getFwdRefs(), instancesToBeLoadedFwd );
    prepareRealRefs( mgr->getRevRefs(), instancesToBeLoadedRev );

    bool intraThreadSuccess[2] = { true, true };
    bool interThreadSuccess = true;
    const int iterations = 50;
    for( int i = 0; i < 2 * iterations; i++ ) {
        // First set iterations both threads will try to load from same list. 
        toBeLoadedOnT1 = &instancesToBeLoadedFwd;
        toBeLoadedOnT2 = i < iterations ? &instancesToBeLoadedFwd : &instancesToBeLoadedRev;

        std::thread first( loadInstancesFromList, mgr, toBeLoadedOnT1, &loadedOnT1, &intraThreadSuccess[0] );
        std::thread second( loadInstancesFromList, mgr, toBeLoadedOnT2, &loadedOnT2, &intraThreadSuccess[1] );

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

/// These tests were run on stepcode/data/cd209/ATS1-out.stp (Bigger files may take a longer time)
int main( int argc, char ** argv ) {
    if( argc != 2 ) {
        std::cerr << "Expected one argument, given " << argc - 1 << ". Exiting." << std::endl;
        exit( EXIT_FAILURE );
    }

    checkFwdRefsSafety( argv[1] );
    checkRevRefsSafety( argv[1] );

    checkTypeInstancesSafety( argv[1] );

    checkLazyLoadingSafety( argv[1] );
}

