#include "instmgr.h"
#include "ExpDict.h"
#include "sdaiApplication_instance.h"
#include <baseType.h>

#ifdef HAVE_STD_THREAD
# include <thread>
#else
# error Need std::thread for this test!
#endif

const std::vector< std::string > dummyEDescNames { "A", "B", "C", "A", "B", "C", "D", "B", "A", "E", "C", "D", "B", "A", "E", "F", "F", "B", "C", "E", "F", "G", "C", "A", "G", "H", "I", "B", "A", "E", "F", "F", "C", "C", "E", "F" };

typedef std::vector< SDAI_Application_instance * > sdaiVec_t;

/// Deletes non-null elements of sdaiVec
void deleteSdaiVec( sdaiVec_t &sdaiVec ) {
    int i, size = sdaiVec.size();
    for( i = 0; i < size; i++ ) {
        if( !sdaiVec[i]) {
            delete sdaiVec[i];
        }
    } 
}

void appendInstances( InstMgr * im, sdaiVec_t &sdaiVec, int offset, int stride, int limit ) {
    SDAI_Application_instance * sai;
    for( int i = offset; i < limit; i+=stride ) {        
        sai = new SDAI_Application_instance( i+1 );
        sai->eDesc = new EntityDescriptor( dummyEDescNames[i].c_str(), ( Schema * ) NULL, LTrue, LFalse );
        sdaiVec[i] = sai;
        im->Append( sai, completeSE );
    }
}

void deleteInstances( InstMgr * im, sdaiVec_t &sdaiVec, int offset, int stride, int limit ) {
    for( int i = offset; i < limit; i+=stride ) {
        im->Delete( sdaiVec[i] );
        sdaiVec[i] = 0;
    }
}

bool compareKeywordCount ( InstMgr * mgr1, InstMgr * mgr2 ) {
    int i, size = dummyEDescNames.size();
    for( i = 0; i < size; i++ ) {
        //some comparisions may be repeated.
        const char * str = dummyEDescNames[i].c_str();
        if( mgr1->EntityKeywordCount( str ) !=  mgr2->EntityKeywordCount( str ) ) {
            std::cout << std::endl << "\tEntityKeywordCount mistmatch for " << str << ": " << mgr1->MaxFileId() << " and " << mgr2->MaxFileId() << std::endl; 
            return false;
        }
    }

    return true;
}

/// Checks for similarity in the two InstMgr
bool compareInstMgr( InstMgr * mgr1, InstMgr * mgr2 ) {
    if( mgr1->MaxFileId() != mgr2->MaxFileId() ) {
        std::cout << std::endl << "\tMaxFileId not same: " << mgr1->MaxFileId() << " and " << mgr2->MaxFileId() << std::endl; 
        return false;
    }

    if( mgr1->InstanceCount() != mgr2->InstanceCount() ) {
        std::cout << std::endl << "\tInstance count not same: " << mgr1->InstanceCount() << " and " << mgr2->InstanceCount() << std::endl; 
        return false;
    }

    if( !compareKeywordCount( mgr1, mgr2 ) ) {
        return false;
    }

    return true;
}

/// Performs addition of different instances to the same InstMgr simultaneosly.
void checkInstMgrAppendOnlyThreadSafety() {
    InstMgr * imExpected = new InstMgr( 0 );
    int size = dummyEDescNames.size();

    //simulate the work done by two threads
    sdaiVec_t sdaiVecOld( size );
    appendInstances( imExpected, sdaiVecOld, 0, 2, size );
    appendInstances( imExpected, sdaiVecOld, 1, 2, size );
    std::cout << "Checking thread safety of InstMgr in Append Operation..." ;

    int i, iterations = 100;
    for( i = 0 ; i < iterations; i++ ) {
        InstMgr * imActual = new InstMgr( 0 );
        sdaiVec_t sdaiVecNew( size );

        std::thread first( appendInstances, imActual, sdaiVecNew, 0, 2, size );
        std::thread second( appendInstances, imActual, sdaiVecNew, 1, 2, size );
        
        first.join();
        second.join();

        bool success = compareInstMgr( imExpected, imActual ); 
        delete imActual;
        deleteSdaiVec( sdaiVecNew );
        
        if( !success ) {  
            break;
        }
    }

    if( i == iterations ) {
        std::cout << "...PASS!" << std::endl;
    } else {
        std::cout << "...FAIL!" << std::endl;
    }

    std::cout << std::endl;

    delete imExpected;
    deleteSdaiVec( sdaiVecOld );
}

/// Performs deletion of different instances to the same InstMgr simultaneosly.
void checkInstMgrDeleteOnlyThreadSafety() {
    InstMgr * imExpected = new InstMgr( 0 );
    int size = dummyEDescNames.size();
    //simulate the work done by two threads

    sdaiVec_t sdaiVecOld( size );
    appendInstances( imExpected, sdaiVecOld, 0, 1, size );
    deleteInstances( imExpected, sdaiVecOld, 0, 2, size );
    deleteInstances( imExpected, sdaiVecOld, 1, 2, size );
    std::cout << "Checking thread safety of InstMgr in Delete Operation..." ;

    int i, iterations = 100;
    for( i = 0 ; i < iterations; i++ ) {
        InstMgr * imActual = new InstMgr( 0 );
        sdaiVec_t sdaiVecNew( size );

        appendInstances( imActual, sdaiVecNew, 0, 1, size ); //Preparetion
        std::thread first( deleteInstances, imActual, sdaiVecNew, 0, 2, size );
        std::thread second( deleteInstances, imActual, sdaiVecNew, 1, 2, size );

        first.join();
        second.join();

        bool success = compareInstMgr( imExpected, imActual ); 
        delete imActual;
        deleteSdaiVec( sdaiVecNew );
        
        if( !success ) {  
            break;
        }
    }

    if( i == iterations ) {
        std::cout << "...PASS!" << std::endl;
    } else {
        std::cout << "...FAIL!" << std::endl;
    }

    std::cout << std::endl;
    delete imExpected;
    deleteSdaiVec( sdaiVecOld );
}

/// Performs addition and deletion of different instances to the same InstMgr simultaneosly.
void checkInstMgrAppendDeleteThreadSafety() {
    InstMgr * imExpected = new InstMgr( 0 );
    int size = dummyEDescNames.size();
    //simulate the work done by two threads

    sdaiVec_t sdaiVecOld( size );
    appendInstances( imExpected, sdaiVecOld, 0, 2, size );
    appendInstances( imExpected, sdaiVecOld, 1, 2, size );
    deleteInstances( imExpected, sdaiVecOld, 0, 2, size );
    std::cout << "Checking thread safety of InstMgr in Append-Delete Operation..." ;

    int i, iterations = 100;
    for( i = 0 ; i < iterations; i++ ) {
        InstMgr * imActual = new InstMgr( 0 );
        sdaiVec_t sdaiVecNew( size );

        appendInstances( imActual, sdaiVecNew, 0, 2, size ); //Preparation
        std::thread first( appendInstances, imActual, sdaiVecNew, 1, 2, size );
        std::thread second( deleteInstances, imActual, sdaiVecNew, 0, 2, size );

        first.join();
        second.join();

        bool success = compareInstMgr( imExpected, imActual ); 
        delete imActual;
        deleteSdaiVec( sdaiVecNew );
        
        if( !success ) {  
            break;
        }
    }

    if( i == iterations ) {
        std::cout << "...PASS!" << std::endl;
    } else {
        std::cout << "...FAIL!" << std::endl;
    }

    std::cout << std::endl;
    delete imExpected;
    deleteSdaiVec( sdaiVecOld );
}

int main( int , char ** ) {

    assert( dummyEDescNames.size() % 4 == 0 && "dummyEDescNames should be a multiple of 4 ");

    checkInstMgrAppendOnlyThreadSafety();
    checkInstMgrDeleteOnlyThreadSafety();
    checkInstMgrAppendDeleteThreadSafety();
}

