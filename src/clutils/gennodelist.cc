
/*
* NIST Utils Class Library
* clutils/gennodelist.cc
* April 1997
* David Sauder
* K. C. Morris

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

/* $Id: gennodelist.cc,v 3.0.1.2 1997/11/05 22:33:49 sauderd DP3.1 $  */

#include <gennode.h>
#include <gennodelist.h>
//#include <gennode.inline.h>
#include <gennodearray.h>
#include <sc_memmgr.h>
#include <assert.h>

// inserts after existNode
void GenNodeList::InsertAfter( GenericNode * newNode,
                               GenericNode * existNode ) {
    assert( newNode->containingList == 0 && "Node Already in another list, remove it first" );
    newNode->next = existNode->next;
    newNode->next->prev = newNode;

    newNode->prev = existNode;
    existNode->next = newNode;
    newNode->containingList = this;
}

// inserts before existNode
void GenNodeList::InsertBefore( GenericNode * newNode,
                                GenericNode * existNode ) {
    assert( newNode->containingList == 0 && "Node Already in another list, remove it first" );
    existNode->prev->next = newNode;
    newNode->prev = existNode->prev;

    newNode->next = existNode;
    existNode->prev = newNode;
    newNode->containingList = this;
}

// inserts before the head node
void GenNodeList::Append( GenericNode * node ) {
    InsertBefore( node, head );
}

void
GenNodeList::Remove( GenericNode * node ) {
    if( node != head ) {
        node->next->prev = node->prev;
        node->prev->next = node->next;
        node->next = 0;
        node->prev = 0;
        node->containingList = 0;
    }
}
void GenNodeList::ClearEntries() {
//    if(debug_level >= PrintFunctionTrace)
//  cout << "GenNodeList::ClearEntries()\n";
    GenericNode * gnPrev = head->Next();
    GenericNode * gn = gnPrev->Next();

    while( gnPrev != head ) {
        gnPrev->prev = 0;
        gnPrev->next = 0;
        gnPrev->containingList = 0;
        gnPrev = gn;
        gn = gn->Next();
    }
    head->next = head;
    head->prev = head;
}

void GenNodeList::DeleteEntries() {
//    if(debug_level >= PrintFunctionTrace)
//  cout << "GenNodeList::DeleteEntries()\n";
    GenericNode * gnPrev = head->Next();
    GenericNode * gn;
    head->next = 0;

    while( gnPrev != head ) {
        gn = gnPrev->Next();
        delete gnPrev;
        gnPrev = gn;
    }
    head->next = head;
    head->prev = head;
}
