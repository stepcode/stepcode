
/** \file STEPinvAttrList.cc
 * derived from STEPattributeList.cc
 */

#include <STEPinvAttrList.h>
#include <ExpDict.h>
#include "sc_memmgr.h"

invAttrListNode::invAttrListNode( Inverse_attribute * a, setter_t s, getter_t g ): attr( a ), set( s ), get( g )  {}

invAttrListNode::~invAttrListNode() {}

STEPinvAttrList::STEPinvAttrList() {}

STEPinvAttrList::~STEPinvAttrList() {}

invAttrListNode * STEPinvAttrList::operator []( int n ) {
    int x = 0;
    invAttrListNode * a = ( invAttrListNode * )head;
    int cnt =  EntryCount();
    if( n < cnt ) {
        while( a && ( x < n ) ) {
            a = ( invAttrListNode * )( a->next );
            x++;
        }
    }
    if( !a ) {
        cerr << "\nERROR in STEP Core library:  " << __FILE__ <<  ":"
        << __LINE__ << "\n" << _POC_ << "\n\n";
    }
    return a;
}

int STEPinvAttrList::list_length() {
    return EntryCount();
}

void STEPinvAttrList::push( Inverse_attribute * a, setter_t s, getter_t g ) {
    invAttrListNode * an = ( invAttrListNode * )head;

    // if the attribute already exists in the list, don't push it
    while( an ) {
        if( a == ( an -> attr ) ) {
            return;
        }
        an = ( invAttrListNode * )( an->next );
    }
    invAttrListNode * ialn = new invAttrListNode( a, s, g );
    AppendNode( ialn );
}
