
/*
* NIST STEP Core Class Library
* clstepcore/STEPattributeList.cc
* April 1997
* K. C. Morris
* David Sauder

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

#include <STEPattributeList.h>
#include <STEPattribute.h>
#include "sc_memmgr.h"

AttrListNode::AttrListNode( STEPattribute * a ) {
    attr = a;
}

AttrListNode::~AttrListNode() {
}


STEPattributeList::STEPattributeList() {
}

STEPattributeList::~STEPattributeList() {
}

STEPattribute & STEPattributeList::operator []( int n ) {
    int x = 0;
    mtxP->lock();
    AttrListNode * a = ( AttrListNode * )head;
    int cnt =  EntryCount();
    if( n < cnt ) {
        while( a && ( x < n ) ) {
            a = ( AttrListNode * )( a->next );
            x++;
        }
    }
    if( a ) {
        STEPattribute * attr = a->attr;
        mtxP->unlock();
        return *( attr );
    }

    // else
    cerr << "\nERROR in STEP Core library:  " << __FILE__ <<  ":"
         << __LINE__ << "\n" << _POC_ << "\n\n";
    mtxP->unlock();
    return *( STEPattribute * ) 0;
}

int STEPattributeList::list_length() {
    return EntryCount();
}

void STEPattributeList::push( STEPattribute * a ) {
    mtxP->lock();
    AttrListNode * a2 = ( AttrListNode * )head;

    // if the attribute already exists in the list, don't push it
    while( a2 ) {
        if( *a == *( a2 -> attr ) ) {
            mtxP->unlock();
            return;
        }
        a2 = ( AttrListNode * )( a2->next );
    }
    a->incrRefCount();
    AttrListNode * saln = new AttrListNode( a );
    AppendNode( saln );
    mtxP->unlock();
}
