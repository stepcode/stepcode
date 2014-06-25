
/*
* NIST STEP Core Class Library
* clstepcore/SingleLinkList.cc
* April 1997
* David Sauder
* KC Morris

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

#include <SingleLinkList.h>
#include "sc_memmgr.h"
#include <assert.h>

void SingleLinkList::DeleteFollowingNodes( SingleLinkNode * item ) {
    mtxP->lock();
    if( head ) {
        SingleLinkNode * trailer = 0;
        SingleLinkNode * leader = head;
        while( leader ) {
            if( leader == item )  {
                while( leader ) {
                    if( trailer ) {
                        trailer->next = leader->next;
                    } else if( leader == head ) {
                        head = leader->next;
                        trailer = head;
                    }
                    if( leader == tail ) {
                        tail = trailer;
                    }
                    delete leader;
                    leader = trailer->next;
                }
            } else {
                if( trailer ) {
                    trailer = trailer->NextNode();
                } else {
                    trailer = leader;
                }
                leader = leader->NextNode();
            }
        }
    }
    mtxP->unlock();
}

void SingleLinkList::AppendNode( SingleLinkNode * item ) {
    mtxP->lock();
    assert( item->owner == 0 && "Item which is to be appended is already inside a list" );
    if( head )  {
        tail -> next = item;
        tail = item;
    } else {
        head = tail = item;
    }
    item->owner = this;
    mtxP->unlock();
}

void SingleLinkList::DeleteNode( SingleLinkNode * item ) {
    mtxP->lock();
    assert( item->owner == this && "Item which is to be deleted is not inside this list" );
    if( head ) {
        SingleLinkNode * trailer = 0;
        SingleLinkNode * leader = head;
        while( leader ) {
            if( leader == item )  {
                if( trailer ) {
                    trailer->next = leader->next;
                }
                leader = leader->next;
                if( item == head ) {
                    head = item->next;
                }
                if( item == tail ) {
                    tail = trailer;
                }
                delete item;
            } else {
                if( trailer ) {
                    trailer = trailer->NextNode();
                } else {
                    trailer = leader;
                }
                leader = leader->NextNode();
            }
        }
    }
    mtxP->unlock();
}
