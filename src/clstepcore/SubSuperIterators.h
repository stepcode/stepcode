#ifndef SUB_SUPER_ITERATORS
#define SUB_SUPER_ITERATORS

#include "ExpDict.h"
#include "ExpDict.h"
#include <queue>
#include <assert.h>
#include <sc_mutex.h>

/** abstract base class for recursive breadth-first input iterators of EntityDescriptor/EntityDescLinkNode
 * NOTE: due to pure virtual functions being necessary for initialization, derived class constructor must call reset(t)
 */
class recursiveEntDescripIterator {
    protected:
        const EntityDescriptor * startEntity;
        typedef struct {
            const EntityDescriptor * ed;
            unsigned int depth; ///< for debugging; records how many lists had to be traversed to find the current node
        } queue_pair;

        std::deque< queue_pair > q;
        unsigned int position; ///< primarily used in comparisons between iterators
        sc_recursive_mutex * dqMtxP; ///< protects 'q' and 'position'

        ///add contents of a linked list to q
        void addLinkedList( const queue_pair qp ) {
            EntityDescLinkNode * a = listHead( qp.ed );
            queue_pair tmp;
            tmp.depth = qp.depth + 1;
            while( a != 0 ) {
                tmp.ed = nodeContent( a );
                dqMtxP->lock( );
                q.push_back( tmp );
                dqMtxP->unlock( );
                a = ( EntityDescLinkNode * ) a->NextNode( );
            }
        }
        virtual EntityDescLinkNode * listHead( const EntityDescriptor * t ) const = 0;     ///< returns the head of something inheriting SingleLinkList
        virtual EntityDescriptor * nodeContent( const EntityDescLinkNode * n ) const = 0;  ///< returns the content of a SingleLinkNode


    public:
        recursiveEntDescripIterator( const EntityDescriptor * t = 0 ): startEntity( t ), position( 0 ), dqMtxP( new sc_recursive_mutex( ) ) {
            //NOTE due to pure virtual functions, derived class constructor *must* call reset(t)
        }
        recursiveEntDescripIterator( ): startEntity( 0 ), position( 0 ), dqMtxP( new sc_recursive_mutex( ) ) {
        }
        ~recursiveEntDescripIterator( ) {
            delete dqMtxP;
        }

        void reset( const EntityDescriptor * t = 0 ) {
            dqMtxP->lock( );
            position = 0;
            q.clear( );
            if( t ) {
                startEntity = t;
            }
            if( startEntity ) {
                queue_pair p;
                p.depth = 0;
                p.ed = startEntity;
                addLinkedList( p );
            }
            dqMtxP->unlock( );
        }

        const EntityDescriptor * next( ) {
            const EntityDescriptor * ed = 0;
            dqMtxP->lock( );
            if( !q.empty( ) ) {
                position++;
                queue_pair qp = q.front( );
                q.pop_front( );
                addLinkedList( qp );
                ed = qp.ed;
            }
            dqMtxP->unlock( );
            return ed;
        }

        const EntityDescriptor * current( ) const {
            const EntityDescriptor * ed = 0;
            dqMtxP->lock();
            if( !q.empty( ) ) {
                //q.front has to be protected by a mutex otherwise it calling front on an empty queue might lead to an undefined behavior
                ed = ( q.front( ).ed );
            }
            dqMtxP->unlock();
            return ed;
        }

        bool hasNext( ) const {
            dqMtxP->lock();
            bool has_next = ( ( ( q.size( ) > 1 ) && ( q[1].ed != 0 ) ) //there is another EntityDescriptor in q
                            || ( nodeContent( listHead( q[0].ed ) ) != 0 ) ); //or, the only one in the queue has a non-empty list
            dqMtxP->unlock();
            return has_next;
        }

        bool empty( ) const {
            return q.empty( );
        }

        unsigned int pos( ) const {
            return position;
        }

        unsigned int depth( ) const {
            return q[0].depth;
        }

        const EntityDescriptor * operator *( ) const {
            return current( );
        }

        const EntityDescriptor * operator ->( ) const {
            return current( );
        }

        /// two iterators are not considered equal unless the startEntity pointers match and the positions match
        bool operator ==( const recursiveEntDescripIterator & b ) const {
            dqMtxP->lock();
            bool eq = ( ( startEntity == b.startEntity ) && ( position == b.position ) );
            dqMtxP->unlock();
            return eq;
        }

        bool operator !=( const recursiveEntDescripIterator & b ) const {
            return( !( operator ==( b ) ) );
        }

        /// for inequality operators, return a Logical; LUnknown means that the startEntity pointers do not match
        Logical operator >( const recursiveEntDescripIterator & b ) const {
            Logical logical;
            dqMtxP->lock();
            if( startEntity != b.startEntity ) {
                logical = LUnknown;
            }
            else if( position > b.position ) {
                logical = LTrue;
            } else {
                logical = LFalse;
            }
            dqMtxP->unlock();
            return logical;
        }

        Logical operator <( const recursiveEntDescripIterator & b ) const {
            Logical logical;
            dqMtxP->lock();
            if( startEntity != b.startEntity ) {
                logical = LUnknown;
            }
            else if( position < b.position ) {
                logical = LTrue;
            } else {
                logical = LFalse;
            }
            dqMtxP->unlock();
            return logical;
        }

        Logical operator >=( const recursiveEntDescripIterator & b ) const {
            Logical logical;
            dqMtxP->lock();
            if( startEntity != b.startEntity ) {
                logical = LUnknown;
            }
            else if( position >= b.position ) {
                logical = LTrue;
            } else {
                logical = LFalse;
            }
            dqMtxP->unlock();
            return logical;
        }

        Logical operator <=( const recursiveEntDescripIterator & b ) const {
            Logical logical;
            dqMtxP->lock();
            if( startEntity != b.startEntity ) {
                logical = LUnknown;
            }
            else if( position <= b.position ) {
                logical = LTrue;
            } else {
                logical = LFalse;
            }
            dqMtxP->unlock();
            return logical;
        }

        const EntityDescriptor * operator ++( ) {
            return next( );
        }

        const EntityDescriptor * operator ++( int ) {
            dqMtxP->lock();
            const EntityDescriptor * c = current( );
            next( );
            dqMtxP->unlock();
            return c;
        }
};

/** Recursive breadth-first input iterator for supertypes
 * \sa subtypesIterator
 */
class supertypesIterator : public recursiveEntDescripIterator {
    protected:
        EntityDescLinkNode * listHead( const EntityDescriptor * t ) const {     ///< returns the head of an EntityDescriptorList
            EntityDescLinkNode * edln = 0;
            dqMtxP->lock();
            if( t ) {
                edln = ( EntityDescLinkNode * ) t->Supertypes().GetHead();
            }
            dqMtxP->unlock();
            return edln;
        }
        EntityDescriptor * nodeContent( const EntityDescLinkNode * n ) const {  ///< returns the content of a EntityDescLinkNode
            EntityDescriptor * ed = 0;
            dqMtxP->lock();
            if( n ) {
                ed = n->EntityDesc();
            }
            dqMtxP->unlock();
            return ed;
        }
    public:
        supertypesIterator( const EntityDescriptor * t = 0 ): recursiveEntDescripIterator( t ) {
            reset( t );
        }
};

/** Recursive breadth-first input iterator for subtypes
 * \sa supertypesIterator
 */
class subtypesIterator: public recursiveEntDescripIterator {
    protected:
        EntityDescLinkNode * listHead( const EntityDescriptor * t ) const {     ///< returns the head of an EntityDescriptorList
            EntityDescLinkNode * edln = 0;
            dqMtxP->lock();
            if( t ) {
                edln = ( EntityDescLinkNode * ) t->Subtypes().GetHead();
            }
            dqMtxP->unlock();
            return edln;
        }
        EntityDescriptor * nodeContent( const EntityDescLinkNode * n ) const {  ///< returns the content of a EntityDescLinkNode
            EntityDescriptor * ed = 0;
            dqMtxP->lock();
            if( n ) {
                ed = n->EntityDesc();
            }
            dqMtxP->unlock();
            return ed;
        }
    public:
        subtypesIterator( const EntityDescriptor * t = 0 ): recursiveEntDescripIterator( t ) {
            reset( t );
        }
};

#endif //SUB_SUPER_ITERATORS
