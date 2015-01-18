
/*
* NIST STEP Core Class Library
* clstepcore/ExpDict.cc
* April 1997
* K. C. Morris
* David Sauder

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

#include <sc_cf.h>

#include <memory.h>
#include <math.h>
#include <stdio.h>

#include <ExpDict.h>
#include <STEPaggregate.h>
#include <Registry.h>
#include "sc_memmgr.h"
#include <SubSuperIterators.h>


///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

EnumAggregate * create_EnumAggregate() {
    return new EnumAggregate;
}

GenericAggregate * create_GenericAggregate() {
    return new GenericAggregate;
}

EntityAggregate * create_EntityAggregate() {
    return new EntityAggregate;
}

SelectAggregate * create_SelectAggregate() {
    return new SelectAggregate;
}

StringAggregate * create_StringAggregate() {
    return new StringAggregate;
}

BinaryAggregate * create_BinaryAggregate() {
    return new BinaryAggregate;
}

RealAggregate * create_RealAggregate() {
    return new RealAggregate;
}

IntAggregate * create_IntAggregate() {
    return new IntAggregate;
}



///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////




/** FIXME
 * #ifdef NOT_YET
    ///////////////////////////////////////////////////////////////////////////////
    // EnumerationTypeDescriptor functions
    ///////////////////////////////////////////////////////////////////////////////
    EnumerationTypeDescriptor::EnumerationTypeDescriptor( ) {
        _elements = new StringAggregate;
    }
 * #endif
 */

///////////////////////////////////////////////////////////////////////////////
// SelectTypeDescriptor functions
///////////////////////////////////////////////////////////////////////////////

SDAI_Select * SelectTypeDescriptor::CreateSelect() {
    if( CreateNewSelect ) {
        return CreateNewSelect();
    } else {
        return 0;
    }
}

const TypeDescriptor * SelectTypeDescriptor::IsA( const TypeDescriptor * other ) const {
    return TypeDescriptor::IsA( other );
}

/**
 * returns the td among the choices of tds describing elements of this select
 * type but only at this unexpanded level. The td ultimately describing the
 * type may be an element of a td for a select that is returned.
 */
const TypeDescriptor * SelectTypeDescriptor::CanBe( const TypeDescriptor * other ) const {
    if( this == other ) {
        return other;
    }

    TypeDescItr elements( GetElements() ) ;
    const TypeDescriptor * td = elements.NextTypeDesc();
    while( td )  {
        if( td -> CanBe( other ) ) {
            return td;
        }
        td = elements.NextTypeDesc();
    }
    return 0;
}

/**
 * returns the td among the choices of tds describing elements of this select
 * type but only at this unexpanded level. The td ultimately describing the
 * type may be an element of a td for a select that is returned.
 */
const TypeDescriptor * SelectTypeDescriptor::CanBe( const char * other ) const {
    TypeDescItr elements( GetElements() ) ;
    const TypeDescriptor * td = 0;

    // see if other is the select
    if( !StrCmpIns( _name, other ) ) {
        return this;
    }

    // see if other is one of the elements
    while( ( td = elements.NextTypeDesc() ) ) {
        if( td -> CanBe( other ) ) {
            return td;
        }
    }
    return 0;
}

/**
 * A modified CanBe, used to determine if "other", a string we have just read,
 * is a possible type-choice of this.  (I.e., our select "CanBeSet" to this
 * choice.)  This deals with the following issue, based on the Tech Corrigendum
 * to Part 21:  Say our select ("selP") has an item which is itself a select
 * ("selQ").  Say it has another select item which is a redefinition of another
 * select ("TYPE selR = selS;").  According to the T.C., if selP is set to one
 * of the members of selQ, "selQ(...)" may not appear in the instantiation.
 * If, however, selP is set to a member of selR, "selR(...)" must appear first.
 * The code below checks if "other" = one of our possible choices.  If one of
 * our choices is a select like selQ, we recurse to see if other matches a
 * member of selQ (and don't look for "selQ").  If we have a choice like selR,
 * we check if other = "selR", but do not look at selR's members.  This func-
 * tion also takes into account schNm, the name of the current schema.  If
 * schNm does not = the schema in which this type was declared, it's possible
 * that it should be referred to with a different name.  This would be the case
 * if schNm = a schema which USEs or REFERENCEs this and renames it (e.g., "USE
 * from XX (A as B)").
 */
const TypeDescriptor * SelectTypeDescriptor::CanBeSet( const char * other, const char * schNm ) const {
    TypeDescItr elements( GetElements() ) ;
    const TypeDescriptor * td = elements.NextTypeDesc();

    while( td ) {
        if( td->Type() == REFERENCE_TYPE && td->NonRefType() == sdaiSELECT ) {
            // Just look at this level, don't look at my items (see intro).
            if( td->CurrName( other, schNm ) ) {
                return td;
            }
        } else if( td->CanBeSet( other, schNm ) ) {
            return td;
        }
        td = elements.NextTypeDesc();
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// AggrTypeDescriptor functions
///////////////////////////////////////////////////////////////////////////////

STEPaggregate * AggrTypeDescriptor::CreateAggregate() {
    if( CreateNewAggr ) {
        return CreateNewAggr();
    } else {
        return 0;
    }
}

void AggrTypeDescriptor::AssignAggrCreator( AggregateCreator f ) {
    CreateNewAggr = f;
}

AggrTypeDescriptor::AggrTypeDescriptor( ) :
    _uniqueElements( "UNKNOWN_TYPE" ) {
    _bound1 = -1;
    _bound2 = -1;
    _aggrDomainType = 0;
}

AggrTypeDescriptor::AggrTypeDescriptor( SDAI_Integer  b1,
                                        SDAI_Integer  b2,
                                        Logical uniqElem,
                                        TypeDescriptor * aggrDomType )
    : _bound1( b1 ), _bound2( b2 ), _uniqueElements( uniqElem ) {
    _aggrDomainType = aggrDomType;
}

AggrTypeDescriptor::~AggrTypeDescriptor() {
}
