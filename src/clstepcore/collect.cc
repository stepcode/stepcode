/*************************************************************************//**
 * collect.cc     \class ComplexCollect                                      *
 *                                                                           *
 * Description: ComplexCollect is the container structure for all ofthe com- *
 *              plex entity information in a schema.  It contains a list of  *
 *              ComplexList structures each of which corresponds to one set  *
 *              of subtype/supertype information about the schema. This file *
 *              contains many of the ComplexCollect member functions.        *
 *                                                                           *
 * Created by:  David Rosenfeld                                              *
 * Date:        11/14/96                                                     *
 *****************************************************************************/

#include "complexSupport.h"
#include "sc_memmgr.h"

/**
 * Inserts a new ComplexList to our list.  The ComplexLists are ordered by
 * supertype name.  Increments count.
 */
void ComplexCollect::insert( ComplexList * c ) {
    mtx.lock();
    ComplexList * prev = NULL, *cl = clists;

    if( !cl ) {
        // checking if cl is NULL.
        c->mtx.lock();
        c->next = NULL;
        clists = c;
        c->mtx.unlock();
    } else {
        // hand over hand locking.
        // Our aim will be to lock the element before the insert slot
        cl->mtx.lock();
        while( *cl < *c ) {
            if( prev ) {
                // Except for the first iteration will execute everytime
                prev->mtx.unlock();
            }

            prev = cl;
            cl = cl->next;
            if( !cl ) {
                // If cl has become null exit. This means we have reached
                // the end of the list. Only the last element has been locked
                break;
            }
            cl->mtx.lock();
        }

        // At this point EITHER we have locked the first element (cl locked,
        // prev = NULL) OR we have locked the last element i.e (prev locked,
        // cl == NULL) or we have locked both cl & prev. The element will be
        // inserted between prev & cl

        if( prev == NULL ) {
            // I.e., c belongs before the first cl so the above loop was never
            // entered.
            clists = c;
        } else {
            prev->next = c;
        }

        c->mtx.lock();
        c->next = cl;
        c->mtx.unlock();

        if( prev ) {
            prev->mtx.unlock();
        }

        if( cl ) {
            cl->mtx.unlock();
        }
    }
    count++;
    mtx.unlock();
}

/**
 * Removes the ComplexList whose supertype name = supername.  "Removing"
 * deletes the list and removes it from this, but does not delete its
 * subtype structure.  This is done when a subtype of other supertypes
 * was temporarily stored in the Collect so that its supertypes would
 * be able to find it, and now that all its supers have accessed it, we
 * remove it from the Collect.
 */
void ComplexCollect::remove( ComplexList * c ) {
    mtx.lock();
    ComplexList * cl = clists, *prev = NULL;

    if( cl ) {
        cl->mtx.lock();
        // Our strategy will be to lock the element which is to be deleted
        // and the element before that
        while( *cl < *c ) {
            // Below if is required to deal with the first iteration.
            if( prev ) {
                prev->mtx.unlock();
            }
            prev = cl;
            cl = cl->next;

            if( !cl ) {
                // We arrived at the end of the list
                prev->mtx.unlock();
                break;
            }
            cl->mtx.lock();
        }
    }

    // At this stage we have no ComplexList locks if we have transversed
    // theentire clists without finding c (including the case when clists
    // is NULL). In case the first ComplexList matches (i.e. prev = NULL)
    // the only lock acquired will be cl. In rest of the cases (where a
    // ComplexList bigger or equal to is encountered in an iteration the
    // locks held would be cl and prev.

    if( cl == NULL || cl != c ) {
        // Just in case c isn't in the list.
        if( cl != c ) {
            // Below if condition avoids the special case in which the
            // first ComplexList element is bigger then c
            if( prev ) {
                prev->mtx.unlock();
            }

            cl->mtx.unlock();
        }
        mtx.unlock();
        return;
    }

    if( prev == NULL ) {
        // c is the first thing in clists (so prev while loop never entered)
        clists = c->next;
    } else {
        prev->next = cl->next;
        prev->mtx.unlock();
    }

    cl->next = NULL;
    cl->mtx.unlock();// cannot do it after cl->remove as it invokes the destructor
    cl->remove();
    count--;
    mtx.unlock();
}

/**
 * Searches for and returns the ComplexList whose supertype name = name.
 */
ComplexList * ComplexCollect::find( char * name ) {
    ComplexList * cl = clists, *prev = NULL, *retval = NULL;

    if( cl ) {
        cl->mtx.lock();
        while( *cl < name ) {
            prev = cl;
            cl = cl->next;
            if( !cl ) {
                // We have reached the end without success
                prev->mtx.unlock();
                return NULL;
            }

            cl->mtx.lock();
            prev->mtx.unlock();
        }

        // At this point we only hold the lock for cl
        retval = ( *cl == name ) ? cl : NULL;
        cl->mtx.unlock();
    }
    return retval;
}

/**
 * Determines if the parent schema supports the instantiation of a complex
 * type consisting of the the entities named in ents.  Does so by attempt-
 * ing to match ents against the ComplexLists in clists.  If one of the
 * nodes of ents has multSupers set to true (it has >1 supertype), it
 * should be included in >1 CList.  A more complicated algorithm is applied
 * to match it, as described in the commenting.
 */
bool ComplexCollect::supports( EntNode * ents ) const {
    ents->sharedMtxP->lock();
    EntNode * node = ents, *nextnode;
    AndList * alist = 0;
    ComplexList * clist = clists, *cl = NULL, *current, *prev;
    bool retval = false;
    EntList * elist, *next;

    // Loop through the nodes of ents.  If 1+ of them have >1 supertype, build
    // a combo-CList to handle it.
    while( node ) {
        if( node->multSuprs() ) {
            // Temporarily slice out node from its list (so that CList->
            // contains() will work properly below):
            nextnode = node->next;
            node->next = NULL;
            if( !cl ) {
                // We may have created cl already in an earlier pass.
                alist = new AndList;
                cl = new ComplexList( alist );
            }
            current = clists;
            if( current ) {
                current->mtx.lock();
                while( current ) {
                    if( current->contains( node ) ) {
                        // Must add current CList to new CList.  First check if we
                        // added current already (while testing an earlier node).
                        if( ! cl->toplevel( current->supertype() ) ) {
                            // Below line adds current to cl.  "current->head->
                            // childList" points to the EntLists directly under the
                            // top-level AND.  We'll add that list right under the
                            // new AND we created at cl's top level.
                            alist->appendList( current->head->childList );
                        }
                    }
                    prev = current;
                    current = current->next;
                    if( current ) {
                        current->mtx.lock();
                    }
                    prev->mtx.unlock();
                }
            }
            node->next = nextnode;
        }
        node = node->next;
    }

    // Now figure out if we match ents or not.  Done differently depending on
    // if we had a sub of >1 supers (and built cl as a combo).
    if( !cl ) {
        // If we never built up cl in the above loop, there were no entities
        // which had mult supers.  Simply go through each CList separately:
        if( clist != NULL ) {
            clist->mtx.lock();
            while( clist != NULL ) {
                if( clist->matches( ents ) ) {
                    clist->mtx.unlock();
                    retval = true;
                    break;
                }
                prev = clist;
                clist = clist->next;
                if( clist ) {
                    clist->mtx.lock();
                }
                prev->mtx.unlock();
            }
            // retval will be false if the loop went through whole list without match
        }
    } else {
        // Use cl to test that the conditions of all supertypes are met:
        cl->multSupers = true;
        cl->buildList();
        retval = cl->matches( ents );

        // We have our return value.  Now get rid of cl:
        // Unlink all the EntLists (gotten from other CLists) which were joined
        // to make cl:
        elist = cl->head->childList;
        while( elist ) {
            // No mutexes are used here since cl has been created and build locally
            elist->prev = NULL;
            elist = elist->next;
            next = elist->next;
            elist->next = NULL;
            elist = next;
        }
        cl->head->childList = NULL;
        // Separate the childList from head.  We don't want to delete any of the
        // sublists when we delete cl - they still belong to other sublists.
        delete cl;
    }
    ents->sharedMtxP->unlock();
    return retval;
}
