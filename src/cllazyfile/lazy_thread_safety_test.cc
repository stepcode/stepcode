#include <thread>
#include "lazyTypes.h"
#include "lazyInstMgr.h"

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

    std::cout << std::endl;
    delete mgr;
}


int main( int argc, char ** argv ) {
    if( argc != 2 ) {
        std::cerr << "Expected one argument, given " << argc - 1 << ". Exiting." << std::endl;
        exit( EXIT_FAILURE );
    }

    checkFwdRefsSafety( argv[1] );
    checkRevRefsSafety( argv[1] );

    checkTypeInstancesSafety( argv[1] );
}

