
/*
* NIST STEP Core Class Library
* clstepcore/STEPattribute.inline.cc
* April 1997
* K. C. Morris
* David Sauder

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

#include <STEPattribute.h>
#include <sdai.h>
#include <ExpDict.h>

///  This is needed so that STEPattribute's can be passed as references to inline functions
STEPattribute::STEPattribute( const STEPattribute & a )
    : _derive( 0 ), _redefAttr( 0 ), aDesc( a.aDesc ) {}

///  INTEGER
STEPattribute::STEPattribute( const class AttrDescriptor & d, SCLP23( Integer ) *p )
    : _derive( 0 ), _redefAttr( 0 ), aDesc( &d )  {
    ptr.i = p;
}

///  BINARY
STEPattribute::STEPattribute( const class AttrDescriptor & d, SCLP23( Binary ) *p )
    : _derive( 0 ), _redefAttr( 0 ), aDesc( &d ) {
    ptr.b = p;
}

///  STRING
STEPattribute::STEPattribute( const class AttrDescriptor & d, SCLP23( String ) *p )
    : _derive( 0 ),  _redefAttr( 0 ), aDesc( &d ) {
    ptr.S = p;
}

///  REAL & NUMBER
STEPattribute::STEPattribute( const class AttrDescriptor & d, SCLP23( Real ) *p )
    : _derive( 0 ), _redefAttr( 0 ), aDesc( &d ) {
    ptr.r = p;
}

///  ENTITY
STEPattribute::STEPattribute( const class AttrDescriptor & d,
                              SCLP23( Application_instance )* *p )
    : _derive( 0 ),  _redefAttr( 0 ), aDesc( &d ) {
    ptr.c = p;
}

///  AGGREGATE
STEPattribute::STEPattribute( const class AttrDescriptor & d, STEPaggregate * p )
    : _derive( 0 ),  _redefAttr( 0 ), aDesc( &d ) {
    ptr.a =  p;
}

///  ENUMERATION  and Logical
STEPattribute::STEPattribute( const class AttrDescriptor & d, SCLP23( Enum ) *p )
    : _derive( 0 ),  _redefAttr( 0 ), aDesc( &d ) {
    ptr.e = p;
}

///  SELECT
STEPattribute::STEPattribute( const class AttrDescriptor & d,
                              class SCLP23( Select ) *p )
    : _derive( 0 ),  _redefAttr( 0 ), aDesc( &d )  {
    ptr.sh = p;
}

///  UNDEFINED
STEPattribute::STEPattribute( const class AttrDescriptor & d, SCLundefined * p )
    : _derive( 0 ), _redefAttr( 0 ), aDesc( &d )   {
    ptr.u = p;
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

const BASE_TYPE STEPattribute::Type() const {
    if( _redefAttr )  {
        return _redefAttr->Type();
    }
    return aDesc->Type();
}

const BASE_TYPE STEPattribute::NonRefType() const {
    if( _redefAttr )  {
        return _redefAttr->NonRefType();
    }
    return aDesc->NonRefType();
}

const BASE_TYPE STEPattribute::BaseType() const {
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

int STEPattribute::Nullable() const {
    if( _redefAttr )  {
        return _redefAttr->Nullable();
    }
    return ( aDesc->Optionality().asInt() == LTrue );
}
