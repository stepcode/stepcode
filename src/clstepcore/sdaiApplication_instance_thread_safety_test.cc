#include <assert.h>
#include <string>
#include <vector>
#include <iostream>

#ifdef HAVE_STD_THREAD
# include <thread>
#else
# error Need std::thread for this test!
#endif

#include "ExpDict.h"
#include "baseType.h"
#include "sdaiApplication_instance.h"

typedef std::vector<SDAI_Application_instance *> sdaiVec_t;
/// Will be populated in the createInstanceList function
std::vector< std::string > EntityNames;

// populates the vector names[] with string of the form [A-Z]/[a-z]/[a-z]
void populateNamesVector() {
    char ch[4];
    ch[3] = '\0';
    ch[0] = 'A';
    while( ch[0] <= 'Z')
    {
        ch[1] = 'a';
        while( ch[1] <= 'z')
        {
            ch[2] = '0';
            while( ch[2] <= '9')
            {
                string str( ch );
                EntityNames.push_back( str );
                ch[2]++;
            }
            ch[1]++;
        }
        ch[0]++;
    }
}

// creates a List of sdaiApplication_instance's, along with initializing its eDesc.
void createInstanceList( sdaiVec_t & sdaiVec ) {
    const int size = 1000;
    assert( size > 10 && size % 4 == 0 && "size of the sdaiVec should be greater then 10 and multiple of 4" ); // A hard reminder
    populateNamesVector();

    assert( ( int )EntityNames.size() >= size && "EntityNames vector is smaller then sdaiVec size" );
    SDAI_Application_instance * sai;
    for( int i = 0; i < size; i++ ) {
        sai = new SDAI_Application_instance( i+1 );
        sai->eDesc = new EntityDescriptor( EntityNames[i].c_str(), ( Schema * ) NULL, LTrue, LFalse );
        sdaiVec.push_back( sai );
    }
}

// deletes the sdaiApplication_instances from the given vector.
void deleteInstanceList( sdaiVec_t & sdaiVec ) {
    int size = sdaiVec.size();
    for( int i = 0; i < size; i++ ) {
        delete sdaiVec[i]->eDesc;
        delete sdaiVec[i];
    }
    sdaiVec.clear();
}

// appends the sdaiApplication_instances from a certain section of the list to sai. (upper and lower limits are inclusive)
void appendListTo( SDAI_Application_instance * sai, sdaiVec_t * sdaiVec, int llimit, int ulimit ) {
    for( int i = llimit; i <= ulimit; i++ ) {
        sai->AppendMultInstance( (*sdaiVec)[i] );
    }
}

// searches for the sdaiApplication_instances belonging to ahe certain section of the list from sai. (upper and lower limits are inclusive)
// nullAllowedIndex marks the index after which GetMiEntity is allowed to return a NULL value 
void searchForElements( SDAI_Application_instance * sai, sdaiVec_t * sdaiVec, int llimit, int ulimit, int nullAllowedIndex, bool * errorInFinding ) {
    SDAI_Application_instance * saiFound;
    for( int i = llimit; i <= ulimit; i++ ) {
        saiFound = sai->GetMiEntity( EntityNames[i].c_str() );
        if( saiFound != (*sdaiVec)[i] ) {
            if( saiFound == NULL && i >= nullAllowedIndex ) {
                continue;
            } else if( i < nullAllowedIndex ) {
                std::cout << "Disparity in the confirmed region" << std::endl;
            } else /* if( saiFound != NULL ) */ {
                std::cout << "Non Null Disparity in the unconfirmed region" << std::endl;
            }
            *errorInFinding = true;
            return;
        }
    }
    *errorInFinding = false;
}

// compares the sdaiApplication_instance's list (headed by sai) with sdaiVec and reports any disparity.
bool validate( SDAI_Application_instance * sai, sdaiVec_t & sdaiVec ) {
    int size = sdaiVec.size();
    int saiSize = 0;
    SDAI_Application_instance * next = sai;
    while( next ) {
        saiSize++;
        next = next->GetNextMiEntity();
    }

    if( saiSize != size ) {
        std::cout << "Expected size: " << size << "\tActual size: "<< saiSize << std::endl;
        return false;
    }

    for( int i = 0; i < size; i++ ) {
        if( sdaiVec[i] != sai->GetMiEntity( EntityNames[i].c_str() ) ) {
            std::cout << "Address are different" << std::endl;
            return false;
        }
    }

    return true;
}

// checks thread safety of two intertwine append operations.
bool checkAddAddInstance() {
    sdaiVec_t sdaiVec;
    std::cout << "Checking thread safety in Adding Instances to a sdaiApplication_instance ...";

    const int iterations = 10;
    int i, size;
    for( i = 0; i < iterations; i++ ) {
        createInstanceList( sdaiVec );
        size = sdaiVec.size();
        
        appendListTo( sdaiVec[0], &sdaiVec, 1, 4 ); //Initializing List before test

        std::thread first( appendListTo, sdaiVec[0], &sdaiVec, 5, size/2  );
        std::thread second( appendListTo, sdaiVec[4], &sdaiVec, ( size/2 )+1 , size-1  );
        
        first.join();
        second.join();

        if ( !validate( sdaiVec[0], sdaiVec ) ) {
            break;
        }
        deleteInstanceList( sdaiVec );
    }

    if( i == iterations ) {
        std::cout << "...PASS" << std::endl;
        return true;
    } else {
        std::cout << "...FAIL" << std::endl;
        return false;
    }
}

// checks thread safety of two intertwine append operations and an append operation with GetMiEntity operation (which may or may not return a null value)
bool checkAddAddGetInstance() {
    sdaiVec_t sdaiVec;
    std::cout << "Checking thread safety in Adding Instances to a sdaiApplication_instance ...";

    const int iterations = 10;
    int i, size;
    for( i = 0; i < iterations; i++ ) {
        createInstanceList( sdaiVec );
        size = sdaiVec.size();
        
        appendListTo( sdaiVec[0], &sdaiVec, 1, 4 ); //Initializing List before test

        // The elements added by the two threads are in the ratio 1:3
        std::thread first( appendListTo, sdaiVec[0], &sdaiVec, 5, size/4 );
        std::thread second( appendListTo, sdaiVec[4], &sdaiVec, ( size/4 )+1 , size-1  );
        
        first.join();

        bool errorInFinding;
        //search for first half of the sdaiApplication_instances. 1/4 elements will be present. rest 1/4 elements may / may not be present
        std::thread third( searchForElements, sdaiVec[0], &sdaiVec, 0, size/2, size/4, &errorInFinding );

        second.join();
        third.join();

        if ( errorInFinding || !validate( sdaiVec[0], sdaiVec ) ) {
            break;
        }
        deleteInstanceList( sdaiVec );
    }

    if( i == iterations ) {
        std::cout << "...PASS" << std::endl;
        return true;
    } else {
        std::cout << "...FAIL" << std::endl;
        return false;
    }
}

int main( int, char ** ) {
    bool pass = true;
    pass &= checkAddAddInstance();
    pass &= checkAddAddGetInstance();

    if( pass ) {
        exit( EXIT_SUCCESS );
    }
    exit( EXIT_FAILURE );
}

