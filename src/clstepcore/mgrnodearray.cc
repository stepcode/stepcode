
/*
* NIST STEP Editor Class Library
* cleditor/mgrnodearray.cc
* April 1997
* David Sauder
* K. C. Morris

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

/* $Id: mgrnodearray.cc,v 3.0.1.3 1997/11/05 22:11:38 sauderd DP3.1 $ */

/*
 * MgrNodeArray - dynamic array object of MgrNodes.
 * the array part of this is copied from Unidraws UArray - dynamic array object
 * Copyright (c) 1990 Stanford University
 */

///////////////////////////////////////////////////////////////////////////////
//  debug_level >= 2 => tells when a command is chosen
//  debug_level >= 3 => prints values within functions:
//     e.g. 1) entity type list when read
//      2) entity instance list when read
///////////////////////////////////////////////////////////////////////////////
static int debug_level = 1;
// if debug_level is greater than this number then
// function names get printed out when executed
static int PrintFunctionTrace = 2;
// if debug_level is greater than this number then
// values within functions get printed out
//static int PrintValues = 3;

#include <mgrnodearray.h>
//#include <STEPentity.h>
#include <sdai.h>

#include <string.h> // to get bcopy() - ANSI
#include "sc_memmgr.h"

//////////////////////////////////////////////////////////////////////////////
// class MgrNodeArray member functions
//////////////////////////////////////////////////////////////////////////////

MgrNodeArray::MgrNodeArray( int defaultSize )
    : GenNodeArray( defaultSize ) {
}

void MgrNodeArray::AssignIndexAddress( int index ) {
//    if(debug_level >= PrintFunctionTrace)
//  cout << "MgrNodeArray::AssignIndexAddress()\n";
    mtx.lock();
    ( ( MgrNode * )_buf[index] )->ArrayIndex( index );
    mtx.unlock();
}

MgrNodeArray::~MgrNodeArray() {
    if( debug_level >= PrintFunctionTrace ) {
        cout << "MgrNodeArray::~MgrNodeArray()\n";
    }
    DeleteEntries();
}

/*****************************************************************************/

void MgrNodeArray::ClearEntries() {
    if( debug_level >= PrintFunctionTrace ) {
        cout << "MgrNodeArray::ClearEntries()\n";
    }
    mtx.lock();
    int i;
    for( i = 0 ; i < _count; i++ ) {
        _buf[i] = 0;
    }
    _count = 0;
    mtx.unlock();
}

/*****************************************************************************/

void MgrNodeArray::DeleteEntries() {
    if( debug_level >= PrintFunctionTrace ) {
        cout << "MgrNodeArray::DeleteEntries()\n";
    }
    mtx.lock();
    int i;
    for( i = 0 ; i < _count; i++ ) {
        delete( ( MgrNode * )_buf[i] );
    }
    _count = 0;
    mtx.unlock();
}

/*****************************************************************************/

int MgrNodeArray::Insert( GenericNode * gn, int index ) {
    if( debug_level >= PrintFunctionTrace ) {
        cout << "MgrNodeArray::Insert()\n";
    }
    mtx.lock();
    int AssignedIndex = GenNodeArray::Insert( gn, index );
    int i;
    for( i = AssignedIndex ; i < _count; i++ ) {
        ( ( MgrNode * )_buf[i] )->ArrayIndex( i );
    }
    mtx.unlock();
    return AssignedIndex;
}

/*****************************************************************************/

int MgrNodeArray::Insert( GenericNode * gn )     {
    mtx.lock();
    int index = Insert( gn, _count );
    mtx.unlock();
    return index;
}

/*****************************************************************************/

void MgrNodeArray::Remove( int index ) {
    if( debug_level >= PrintFunctionTrace ) {
        cout << "MgrNodeArray::Remove()\n";
    }
    mtx.lock();
    if( 0 <= index && index < _count ) {
        GenNodeArray::Remove( index );
        int i;
        for( i = index; i < _count; i++ ) {
            ( ( MgrNode * )_buf[i] )->ArrayIndex( i );
        }
    }
    mtx.unlock();
}

/*****************************************************************************/

int MgrNodeArray::MgrNodeIndex( int fileId ) {
    if( debug_level >= PrintFunctionTrace ) {
        cout << "MgrNodeArray::MgrNodeIndex()\n";
    }
    mtx.lock();
    int i;
    for( i = 0; i < _count; ++i ) {
        if( ( ( MgrNode * )_buf[i] )->GetApplication_instance()->GetFileId() == fileId ) {
            mtx.unlock();
            return i;
        }
    }
    mtx.unlock();
    return -1;
}

//////////////////////////////////////////////////////////////////////////////
// class MgrNodeArraySorted member functions
//////////////////////////////////////////////////////////////////////////////

MgrNodeArraySorted::MgrNodeArraySorted( int defaultSize )
    : GenNodeArray( defaultSize ) {
}

int MgrNodeArraySorted::Insert( GenericNode * gn ) {
//    if(debug_level >= PrintFunctionTrace)
//  cout << "MgrNodeArraySorted::Insert()\n";

    // since gn is really a MgrNode
    mtx.lock();
    int fileId = ( ( MgrNode * )gn )->GetApplication_instance()->GetFileId();

    int index = FindInsertPosition( fileId );
    index = GenNodeArray::Insert( gn, index );
    mtx.unlock();

    return index;
}

int MgrNodeArraySorted::Index( GenericNode * gn ) {
//    if(debug_level >= PrintFunctionTrace)
//  cout << "MgrNodeArraySorted::Index()\n";
    // since gn is really a MgrNode
    mtx.lock();
    int index = MgrNodeIndex( ( ( MgrNode * )gn )->GetFileId() );
    mtx.unlock();
    return index;
}

int MgrNodeArraySorted::Index( GenericNode ** gn ) {
//    if(debug_level >= PrintFunctionTrace)
//  cout << "MgrNodeArraySorted::Index()\n";
    // since gn is really a MgrNode
    mtx.lock();
    int index = MgrNodeIndex( ( ( MgrNode * )( *gn ) )->GetFileId() );
    mtx.unlock();
    return index;
}

void MgrNodeArraySorted::ClearEntries() {
    if( debug_level >= PrintFunctionTrace ) {
        cout << "MgrNodeArraySorted::ClearEntries()\n";
    }
    mtx.lock();
    int i;
    for( i = 0 ; i < _count; i++ ) {
        _buf[i] = 0;
    }
    _count = 0;
    mtx.unlock();
}

/*****************************************************************************/

void MgrNodeArraySorted::DeleteEntries() {
    if( debug_level >= PrintFunctionTrace ) {
        cout << "MgrNodeArraySorted::DeleteEntries()\n";
    }
    int i;
    mtx.lock();
    for( i = 0 ; i < _count; i++ ) {
        delete( ( MgrNode * )_buf[i] );
    }
    _count = 0;
    mtx.unlock();
}

/*****************************************************************************/

// the reason this is written this way is because most of the
// time the file id will be higher than any seen so far and
// thus the insert position will be at the end
int MgrNodeArraySorted::FindInsertPosition( const int fileId ) {
    if( debug_level >= PrintFunctionTrace ) {
        cout << "MgrNodeArraySorted::FindInsertPosition()\n";
    }
    int i;
    int curFileId;

    mtx.lock();
    for( i = _count - 1; i >= 0; --i ) {
        curFileId = ( ( MgrNode * )_buf[i] )->GetApplication_instance()->GetFileId();
        if( curFileId < fileId /*|| curFileId == fileId*/ ) {
            mtx.unlock();
            return i + 1;
        }
    }
    mtx.unlock();
    return 0;
}

/*****************************************************************************/

int MgrNodeArraySorted::MgrNodeIndex( int fileId ) {
// this function assumes that _buf[0] to _buf[_count] ALL point to MgrNodes
// that are sorted by fileId

    if( debug_level >= PrintFunctionTrace ) {
        cout << "MgrNodeArraySorted::MgrNodeIndex()\n";
    }
    int low = 0;
    int high = _count - 1;
    int mid = 0;
    int found = 0;
    int curFileId;

    mtx.lock();
    while( !found && ( low <= high ) ) {
        mid = ( low + high ) / 2;
        curFileId = ( ( MgrNode * )_buf[mid] )->GetApplication_instance()->GetFileId();
        if( curFileId == fileId ) {
            found = 1;
        } else if( curFileId < fileId ) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    if( !found ) {
        mid = -1;
    }
    mtx.unlock();
    return mid;
}
