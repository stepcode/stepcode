#ifndef SUB_SUPER_ITERATORS
#define SUB_SUPER_ITERATORS

#include "ExpDict.h"
#include "ExpDict.h"
#include <queue>
#include <assert.h>

/** abstract base class for recursive breadth-first input iterators of EntityDescriptor/EntityDescLinkNode
 * NOTE: due to pure virtual functions being necessary for initialization, derived class constructor must call reset(t)
 */
class recursiveEntDescripIterator {
protected:
    const EntityDescriptor * startEntity;
    std::deque<EntityDescLinkNode *> q;
    unsigned int position; ///< primarily used in comparisons between iterators

    ///add contents of a linked list to q
    void addLinkedList( const EntityDescriptor * t ) {
        EntityDescLinkNode *a = listHead( t );
        if( a != 0 ) {
            q.push_back(a);
        }
        while( ( a != 0 ) && ( 0 != ( a = ( EntityDescLinkNode * ) a->NextNode( ) ) ) ) {
            q.push_back( a );
        }
    }
    virtual EntityDescLinkNode * listHead( const EntityDescriptor * t ) const = 0;     ///< returns the head of something inheriting SingleLinkList
    virtual EntityDescriptor * nodeContent( const EntityDescLinkNode * n ) const = 0;  ///< returns the content of a SingleLinkNode


public:
    recursiveEntDescripIterator( const EntityDescriptor * t = 0 ): startEntity( t ), position( 0 ) {
        //NOTE due to pure virtual functions, derived class constructor *must* call reset(t)
    }

    ~recursiveEntDescripIterator( ) {}

    void reset( const EntityDescriptor * t = 0 ) {
        position = 0;
        q.clear( );
        if( t ) {
            startEntity = t;
        }
        if( startEntity ) {
            addLinkedList(startEntity);
        }
    }

    const EntityDescriptor * next( ) {
        position++;
        if( q.empty( ) ) {
            return ( EntityDescriptor * ) 0;
        }
        EntityDescLinkNode * node = q.front( );
        q.pop_front( );
        if( node ) {
            EntityDescriptor * tmp =  nodeContent( node );
            addLinkedList( tmp );
            return tmp;
        } else {
            return ( EntityDescriptor * ) 0;
        }
    }

    const EntityDescriptor * current( ) const {
        if( q.empty( ) ) {
            return ( EntityDescriptor * ) 0;
        }
        return nodeContent( q.front( ) );
    }

    bool hasNext( ) const {
        return( (q.size( ) > 1 ) && ( q[1] != 0 ) );
    }

    bool empty( ) const {
        return q.empty( );
    }

    unsigned int pos( ) const {
        return position;
    }

    const EntityDescriptor * operator *( ) const {
        return current( );
    }

    const EntityDescriptor * operator ->( ) /*const*/ {
        return current( );
    }

    ///we assume that two iterators cannot be considered equal unless the startEntity pointers match and the positions match
    bool operator ==( const recursiveEntDescripIterator & b ) const {
        return( ( startEntity == b.startEntity ) && ( position == b.position ) );
    }

    bool operator !=( const recursiveEntDescripIterator & b ) const {
        return( ( startEntity != b.startEntity ) || ( position != b.position ) );
    }

    bool operator >( const recursiveEntDescripIterator & b ) const {
        assert( startEntity == b.startEntity );
        return( position > b.position );
    }

    bool operator <( const recursiveEntDescripIterator & b ) const {
        assert( startEntity == b.startEntity );
        return( position < b.position );
    }

    bool operator >=( const recursiveEntDescripIterator & b ) const {
        assert( startEntity == b.startEntity );
        return( position >= b.position );
    }

    bool operator <=( const recursiveEntDescripIterator & b ) const {
        assert( startEntity == b.startEntity );
        return( position <= b.position );
    }

    const EntityDescriptor * operator ++( ) {
        return next( );
    }

    const EntityDescriptor * operator ++( int ) {
        const EntityDescriptor * c = current( );
        next( );
        return c;
    }
};

/** Recursive breadth-first input iterator for supertypes
 * \sa subtypesIterator
 */
class supertypesIterator : public recursiveEntDescripIterator {
protected:
    EntityDescLinkNode * listHead( const EntityDescriptor * t ) const {     ///< returns the head of something inheriting SingleLinkList
        return ( EntityDescLinkNode * ) t->Supertypes().GetHead();
    }
    EntityDescriptor * nodeContent( const EntityDescLinkNode * n ) const {  ///< returns the content of a SingleLinkNode
        return n->EntityDesc();
    }
public:
    supertypesIterator( const EntityDescriptor* t = 0 ):recursiveEntDescripIterator(t) {
        reset( t );
    }
};

/** Recursive breadth-first input iterator for subtypes
 * \sa supertypesIterator
 */
class subtypesIterator: public recursiveEntDescripIterator{
protected:
    EntityDescLinkNode * listHead( const EntityDescriptor * t ) const {     ///< returns the head of something inheriting SingleLinkList
        return ( EntityDescLinkNode * ) t->Subtypes().GetHead();
    }
    EntityDescriptor * nodeContent( const EntityDescLinkNode * n ) const {  ///< returns the content of a SingleLinkNode
        return n->EntityDesc();
    }
public:
    subtypesIterator( const EntityDescriptor* t = 0 ):recursiveEntDescripIterator(t) {
        reset( t );
    }
};

#endif //SUB_SUPER_ITERATORS
