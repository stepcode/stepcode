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
#include "Registry.h"

enum Mode { 
    ENTITY = 0, 
    SCHEMA = 1, 
    TYPE   = 2
};
const std::vector< std::string > modeNames { "ENTITY", "SCHEMA", "TYPE" };

/// Will be populated in the main function
std::vector< std::string > names;

/// Needed for constructing a Registry
void dummyInit( Registry & reg ) {
    ( void )reg;
}

/// Adds the elements at the specified indexes from the Registry
void addElements( Registry * reg, Mode mode, int llimit, int ulimit, int stride ) {
    int i;
    for( i = llimit; i < ulimit; i+=stride ) {
        switch( mode ) {
            case ENTITY:
                reg->AddEntity( *( new EntityDescriptor( names[i].c_str(), ( Schema * ) NULL, LTrue, LFalse ) ) );
                break;

            case SCHEMA:
                reg->AddSchema( *( new Schema( names[i].c_str() ) ) );
                break;

            case TYPE:
                reg->AddType( *( new TypeDescriptor( names[i].c_str(), UNKNOWN_TYPE, ( Schema * )0, ( char * )0 ) ) );
                break;
        }
    }
}

/// Removes the elements at the specified indexes from the Registry
void removeElements( Registry * reg, Mode mode, int llimit, int ulimit, int stride ) {
    int i;
    for( i = llimit; i < ulimit; i+=stride ) {
        switch( mode ) {
            case ENTITY:
                reg->RemoveEntity( names[i].c_str() );
                break;

            case SCHEMA:
                reg->RemoveSchema( names[i].c_str() );
                break;

            case TYPE:
                reg->RemoveType( names[i].c_str() );
                break;
        }
    }
}

/// compares the elements of the two registry. i.e. either an element should be in both the registries or none of them
bool haveSameElements( Registry * reg1, Registry * reg2 ) {
    int i, size = names.size();
    for( i = 0; i < size; i++ ) {
        if( ( reg1->FindEntity( names[i].c_str() ) != NULL ) && 
            ( reg2->FindEntity( names[i].c_str() ) == NULL ) ) {
            std::cout << std::endl << modeNames[ENTITY] << " " << names[i] << " expected but not present" << std::endl; 
            return false;
        }
        if( ( reg1->FindEntity( names[i].c_str() ) == NULL ) && 
            ( reg2->FindEntity( names[i].c_str() ) != NULL ) ) {
            std::cout << std::endl << modeNames[ENTITY] << " " << names[i] << " not expected but present" << std::endl; 
            return false;
        }

        if( ( reg1->FindSchema( names[i].c_str() ) != NULL ) && 
            ( reg2->FindSchema( names[i].c_str() ) == NULL ) ) {
            std::cout << std::endl << modeNames[SCHEMA] << " " << names[i] << " expected but not present" << std::endl; 
            return false;
        }
        if( ( reg1->FindSchema( names[i].c_str() ) == NULL ) && 
            ( reg2->FindSchema( names[i].c_str() ) != NULL ) ) {
            std::cout << std::endl << modeNames[SCHEMA] << " " << names[i] << " not expected but present" << std::endl; 
            return false;
        }

        if( ( reg1->FindType( names[i].c_str() ) != NULL ) && 
            ( reg2->FindType( names[i].c_str() ) == NULL ) ) {
            std::cout << std::endl << modeNames[TYPE] << " " << names[i] << " expected but not present" << std::endl; 
            return false;
        }
        if( ( reg1->FindType( names[i].c_str() ) == NULL ) && 
            ( reg2->FindType( names[i].c_str() ) != NULL ) ) {
            std::cout << std::endl << modeNames[TYPE] << " " << names[i] << " not expected but present" << std::endl; 
            return false;
        }
    }

    return true;
}

/// compares the given registries and reports any disparity
bool compareRegistry( Registry * reg1, Registry * reg2 ) {
    if( reg1->GetEntityCnt() != reg2->GetEntityCnt() ) {
        std::cout << std::endl << "\tentityCnt not same: " << reg1->GetEntityCnt() << " and " << reg2->GetEntityCnt() << std::endl; 
        return false;
    }

    if( reg1->GetFullEntCnt() != reg2->GetFullEntCnt() ) {
        std::cout << std::endl << "\tall_ents_Cnt not same: " << reg1->GetFullEntCnt() << " and " << reg2->GetFullEntCnt() << std::endl; 
        return false;
    }

    if( !haveSameElements( reg1, reg2 ) ) {
        return false;
    }

    return true;
}

/// For a particular mode adds elements to one Registry serially and to another Registry parallely. 
///  It then compares the two Registry and reports any disparity.
void checkRegistryAddOnlyThreadSafety( Mode mode )  {
    int size = names.size();

    Registry * regExpected = new Registry( dummyInit ); 
    addElements( regExpected, mode, 0, size, 2 ); // Add even indexed elements
    addElements( regExpected, mode, 1, size, 2 ); // Add odd indexed elements

    std::cout << "Checking thread safety of Registry " << modeNames[( int )mode] << " Add Operations..." ;

    int i, iterations = 10;
    for( i = 0 ; i < iterations; i++ ) {
        Registry * regActual = new Registry( dummyInit );

        std::thread first( addElements, regActual, mode, 0, size, 2 );
        std::thread second( addElements, regActual, mode, 1, size, 2 );
        
        first.join();
        second.join();

        bool success = compareRegistry( regExpected, regActual ); 
        delete regActual;
        
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

    delete regExpected;
}

/// For a particular mode removes elements to one Registry serially and to another Registry parallely. 
///  It then compares the two Registry and reports any disparity.
void checkRegistryRemoveOnlyThreadSafety( Mode mode )  {
    int size = names.size();

    Registry * regExpected = new Registry( dummyInit ); 
    addElements( regExpected, mode, 0, size, 1 ); // Add all elements
    removeElements( regExpected, mode, 0, size, 3 ); // remove one-third elements
    removeElements( regExpected, mode, 1, size, 3 ); // remove another third elements

    std::cout << "Checking thread safety of Registry " << modeNames[( int )mode] << " Remove Operations..." ;

    int i, iterations = 10;
    for( i = 0 ; i < iterations; i++ ) {
        Registry * regActual = new Registry( dummyInit );

        addElements( regActual, mode, 0, size, 1 );
        std::thread first( removeElements, regActual, mode, 0, size, 3 );
        std::thread second( removeElements, regActual, mode, 1, size, 3 );
        
        first.join();
        second.join();

        bool success = compareRegistry( regExpected, regActual ); 
        delete regActual;
        
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

    delete regExpected;
}

/// For a particular mode adds and removes elements to one Registry serially and to ther Registry parallely. 
///  It then compares the two Registry and reports any disparity.
void checkRegistryAddRemoveThreadSafety( Mode mode )  {
    int size = names.size();

    Registry * regExpected = new Registry( dummyInit ); 
    addElements( regExpected, mode, 0, size, 2 ); // Add even indexed elements
    addElements( regExpected, mode, 1, size, 2 ); // Add odd indexed elements
    removeElements( regExpected, mode, 0, size, 2 ); // remove even indexed elements

    std::cout << "Checking thread safety of Registry " << modeNames[( int )mode] << " Add-Remove Operations..." ;

    int i, iterations = 10;
    for( i = 0 ; i < iterations; i++ ) {
        Registry * regActual = new Registry( dummyInit );

        addElements( regActual, mode, 0, size, 2 );
        std::thread first( addElements, regActual, mode, 1, size, 2 );
        std::thread second( removeElements, regActual, mode, 0, size, 2 );
        
        first.join();
        second.join();

        bool success = compareRegistry( regExpected, regActual ); 
        delete regActual;
        
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

    delete regExpected;
}

void checkRegistryEntityAddOnlyThreadSafety() {
    checkRegistryAddOnlyThreadSafety( ENTITY );
}

void checkRegistrySchemaAddOnlyThreadSafety() {
    checkRegistryAddOnlyThreadSafety( SCHEMA );
}

void checkRegistryTypeAddOnlyThreadSafety() {
    checkRegistryAddOnlyThreadSafety( TYPE );
}

void checkRegistryEntityRemoveOnlyThreadSafety() {
    checkRegistryRemoveOnlyThreadSafety( ENTITY );
}

void checkRegistrySchemaRemoveOnlyThreadSafety() {
    checkRegistryRemoveOnlyThreadSafety( SCHEMA );
}

void checkRegistryTypeRemoveOnlyThreadSafety() {
    checkRegistryRemoveOnlyThreadSafety( TYPE );
}

void checkRegistryEntityAddRemoveThreadSafety() {
    checkRegistryAddRemoveThreadSafety( ENTITY );
}

void checkRegistrySchemaAddRemoveThreadSafety() {
    checkRegistryAddRemoveThreadSafety( SCHEMA );
}

void checkRegistryTypeAddRemoveThreadSafety() {
    checkRegistryAddRemoveThreadSafety( TYPE );
}

// populates the vector names[] with string of the form [A-Z]/[a-z]/[a-z]
void populateNamesVector() {
    char ch[3];
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
                names.push_back( str );
                ch[2]++;
            }
            ch[1]++;
        }
        ch[0]++;
    }
}

int main( int , char ** ) {
    populateNamesVector();

    checkRegistryEntityAddOnlyThreadSafety();
    checkRegistryEntityRemoveOnlyThreadSafety();
    checkRegistryEntityAddRemoveThreadSafety();

    checkRegistrySchemaAddOnlyThreadSafety();
    checkRegistrySchemaRemoveOnlyThreadSafety();
    checkRegistrySchemaAddRemoveThreadSafety();

    checkRegistryTypeAddOnlyThreadSafety();
    checkRegistryTypeRemoveOnlyThreadSafety();
    checkRegistryTypeAddRemoveThreadSafety();
}
