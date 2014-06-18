
#ifndef gennodelist_h
#define gennodelist_h

/*
* NIST Utils Class Library
* clutils/gennodelist.h
* April 1997
* David Sauder
* K. C. Morris

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

/* $Id: gennodelist.h,v 3.0.1.2 1997/11/05 22:33:48 sauderd DP3.1 $  */

#include <sc_export.h>
#include <iostream>
#include <sc_mutex.h>

//////////////////////////////////////////////////////////////////////////////
// class GenNodeList
// this class implements a doubly linked list by default.
// If you delete this object it does not delete all of its entries,
// only its head.  If you want it to delete all of its entries as well
// as its head, you need to call DeleteEntries().
//////////////////////////////////////////////////////////////////////////////

class SC_UTILS_EXPORT GenNodeList {
    public:
        GenNodeList( GenericNode * headNode );
        virtual ~GenNodeList() {
            delete head;
        }

        GenericNode * GetHead()  {
            return head;
        }

        virtual void ClearEntries();
        virtual void DeleteEntries();
        // deletes node from its previous list & appends
        virtual void Append( GenericNode * node );
        // deletes newNode from its previous list & inserts in
        //  relation to existNode. Returns true on success. If
        //  the existNode doesn't belong to the list then simply
        //  returns false
        virtual bool InsertAfter( GenericNode * newNode, GenericNode * existNode );
        virtual bool InsertBefore( GenericNode * newNode, GenericNode * existNode );

        virtual void Remove( GenericNode * node );

    protected:
        GenericNode * head;
        sc_recursive_mutex mtx;
};

//////////////////////////////////////////////////////////////////////////////
// class GenNodeList inline functions
// these functions don't rely on any inline functions (its own or
//  other classes) that aren't in this file
//////////////////////////////////////////////////////////////////////////////

inline GenNodeList::GenNodeList( GenericNode * headNode ) {
    head = headNode;
    head->next = head;
    head->prev = head;
    head->containingList = this;
}

#endif
