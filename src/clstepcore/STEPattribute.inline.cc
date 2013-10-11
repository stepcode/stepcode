
/*
* NIST STEP Core Class Library
* clstepcore/STEPattribute.inline.cc
* April 1997
* K. C. Morris
* David Sauder

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

#include <assert.h>
#include <STEPattribute.h>
#include <STEPaggregate.h>
#include <sdai.h>
#include <ExpDict.h>
#include "sc_memmgr.h"

/// This is needed so that STEPattribute's can be passed as references to inline functions
/// NOTE this code only does shallow copies. It may be necessary to do more, in which case
/// the destructor and assignment operator will also need examined.
STEPattribute::STEPattribute( const STEPattribute & a )
: _derive( a._derive ), _mustDeletePtr( false ), _redefAttr( a._redefAttr ), aDesc( a.aDesc ), refCount( a.refCount ) {
    ShallowCopy( & a );

    //NOTE may need to do a deep copy for the following types since they are classes
    /*
    switch( NonRefType() ) {
        case BINARY_TYPE:

        case STRING_TYPE:

        case ENTITY_TYPE:

        case AGGREGATE_TYPE:
        case ARRAY_TYPE:      // DAS
        case BAG_TYPE:        // DAS
        case SET_TYPE:        // DAS
        case LIST_TYPE:       // DAS

        case SELECT_TYPE:

        case ENUM_TYPE:
        case BOOLEAN_TYPE:
        case LOGICAL_TYPE:

        default:
            break;
    }
    */
}

///  INTEGER
STEPattribute::STEPattribute( const class AttrDescriptor & d, SDAI_Integer * p )
    : _derive( false ), _redefAttr( 0 ), aDesc( &d ), refCount( 0 )  {
    ptr.i = p;
    assert( &d ); //ensure that the AttrDescriptor is not a null pointer
}

///  BINARY
STEPattribute::STEPattribute( const class AttrDescriptor & d, SDAI_Binary * p )
    : _derive( false ), _redefAttr( 0 ), aDesc( &d ), refCount( 0 ) {
    ptr.b = p;
    assert( &d ); //ensure that the AttrDescriptor is not a null pointer
}

///  STRING
STEPattribute::STEPattribute( const class AttrDescriptor & d, SDAI_String * p )
    : _derive( false ),  _redefAttr( 0 ), aDesc( &d ), refCount( 0 ) {
    ptr.S = p;
    assert( &d ); //ensure that the AttrDescriptor is not a null pointer
}

///  REAL & NUMBER
STEPattribute::STEPattribute( const class AttrDescriptor & d, SDAI_Real * p )
    : _derive( false ), _redefAttr( 0 ), aDesc( &d ), refCount( 0 ) {
    ptr.r = p;
    assert( &d ); //ensure that the AttrDescriptor is not a null pointer
}

///  ENTITY
STEPattribute::STEPattribute( const class AttrDescriptor & d,
                              SDAI_Application_instance * *p )
    : _derive( false ),  _redefAttr( 0 ), aDesc( &d ), refCount( 0 ) {
    ptr.c = p;
    assert( &d ); //ensure that the AttrDescriptor is not a null pointer
}

///  AGGREGATE
STEPattribute::STEPattribute( const class AttrDescriptor & d, STEPaggregate * p )
    : _derive( false ),  _redefAttr( 0 ), aDesc( &d ), refCount( 0 ) {
    ptr.a =  p;
    assert( &d ); //ensure that the AttrDescriptor is not a null pointer
}

///  ENUMERATION  and Logical
STEPattribute::STEPattribute( const class AttrDescriptor & d, SDAI_Enum * p )
    : _derive( false ),  _redefAttr( 0 ), aDesc( &d ), refCount( 0 ) {
    ptr.e = p;
    assert( &d ); //ensure that the AttrDescriptor is not a null pointer
}

///  SELECT
STEPattribute::STEPattribute( const class AttrDescriptor & d,
                              class SDAI_Select * p )
    : _derive( false ),  _redefAttr( 0 ), aDesc( &d ), refCount( 0 ) {
    ptr.sh = p;
    assert( &d ); //ensure that the AttrDescriptor is not a null pointer
}

///  UNDEFINED
STEPattribute::STEPattribute( const class AttrDescriptor & d, SCLundefined * p )
    : _derive( false ), _redefAttr( 0 ), aDesc( &d ), refCount( 0 ) {
    ptr.u = p;
    assert( &d ); //ensure that the AttrDescriptor is not a null pointer
}

/// the destructor conditionally deletes the object in ptr
STEPattribute::~STEPattribute() {
    if( _mustDeletePtr ) {
        switch( NonRefType() ) {
            case AGGREGATE_TYPE:
            case ARRAY_TYPE:      // DAS
            case BAG_TYPE:        // DAS
            case SET_TYPE:        // DAS
            case LIST_TYPE:       // DAS
                if( ptr.a ) {
                    delete ptr.a;
                    ptr.a = 0;
                }
                break;
            case BOOLEAN_TYPE:
                if( ptr.e ) {
                    delete ( SDAI_BOOLEAN * ) ptr.e;
                    ptr.e = 0;
                }
            case LOGICAL_TYPE:
                if( ptr.e ) {
                    delete ( SDAI_LOGICAL * ) ptr.e;
                    ptr.e = 0;
                }
                break;
            default:
                break;
        }
    }
}

/// name is the same even if redefined
const char * STEPattribute::Name() const {
    return aDesc->Name();
}

const char * STEPattribute::TypeName() const {
    if( _redefAttr )  {
        return _redefAttr->TypeName();
    }
    return aDesc->TypeName();
}

BASE_TYPE STEPattribute::Type() const {
    if( _redefAttr )  {
        return _redefAttr->Type();
    }
    return aDesc->Type();
}

BASE_TYPE STEPattribute::NonRefType() const {
    if( _redefAttr )  {
        return _redefAttr->NonRefType();
    } else if( aDesc ) {
        return aDesc->NonRefType();
    }
    return UNKNOWN_TYPE;
}

BASE_TYPE STEPattribute::BaseType() const {
    if( _redefAttr )  {
        return _redefAttr->BaseType();
    }
    return aDesc->BaseType();
}

const TypeDescriptor * STEPattribute::ReferentType() const {
    if( _redefAttr )  {
        return _redefAttr->ReferentType();
    }
    return aDesc->ReferentType();
}

bool STEPattribute::Nullable() const {
    if( _redefAttr )  {
        return _redefAttr->Nullable();
    }
    return ( aDesc->Optionality().asInt() == LTrue );
}
