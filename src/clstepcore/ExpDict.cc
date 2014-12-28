
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

Explicit_item_id__set::Explicit_item_id__set( int defaultSize ) {
    _bufsize = defaultSize;
    _buf = new Explicit_item_id_ptr[_bufsize];
    _count = 0;
}

Explicit_item_id__set::~Explicit_item_id__set() {
    delete[] _buf;
}

void Explicit_item_id__set::Check( int index ) {
    Explicit_item_id_ptr * newbuf;

    if( index >= _bufsize ) {
        _bufsize = ( index + 1 ) * 2;
        newbuf = new Explicit_item_id_ptr[_bufsize];
        memmove( newbuf, _buf, _count * sizeof( Explicit_item_id_ptr ) );
        delete[] _buf;
        _buf = newbuf;
    }
}

void Explicit_item_id__set::Insert( Explicit_item_id_ptr v, int index ) {
    Explicit_item_id_ptr * spot;
    index = ( index < 0 ) ? _count : index;

    if( index < _count ) {
        Check( _count + 1 );
        spot = &_buf[index];
        memmove( spot + 1, spot, ( _count - index )*sizeof( Explicit_item_id_ptr ) );

    } else {
        Check( index );
        spot = &_buf[index];
    }
    *spot = v;
    ++_count;
}

void Explicit_item_id__set::Append( Explicit_item_id_ptr v ) {
    int index = _count;
    Explicit_item_id_ptr * spot;

    if( index < _count ) {
        Check( _count + 1 );
        spot = &_buf[index];
        memmove( spot + 1, spot, ( _count - index )*sizeof( Explicit_item_id_ptr ) );

    } else {
        Check( index );
        spot = &_buf[index];
    }
    *spot = v;
    ++_count;
}

void Explicit_item_id__set::Remove( int index ) {
    if( 0 <= index && index < _count ) {
        --_count;
        Explicit_item_id_ptr * spot = &_buf[index];
        memmove( spot, spot + 1, ( _count - index )*sizeof( Explicit_item_id_ptr ) );
    }
}

int Explicit_item_id__set::Index( Explicit_item_id_ptr v ) {
    for( int i = 0; i < _count; ++i ) {
        if( _buf[i] == v ) {
            return i;
        }
    }
    return -1;
}

Explicit_item_id_ptr & Explicit_item_id__set::operator[]( int index ) {
    Check( index );
    _count = ( ( _count > index + 1 ) ? _count : ( index + 1 ) );
    return _buf[index];
}

int Explicit_item_id__set::Count() {
    return _count;
}

void Explicit_item_id__set::Clear() {
    _count = 0;
}

///////////////////////////////////////////////////////////////////////////////

Implicit_item_id__set::Implicit_item_id__set( int defaultSize ) {
    _bufsize = defaultSize;
    _buf = new Implicit_item_id_ptr[_bufsize];
    _count = 0;
}

Implicit_item_id__set::~Implicit_item_id__set() {
    delete[] _buf;
}

void Implicit_item_id__set::Check( int index ) {
    Implicit_item_id_ptr * newbuf;

    if( index >= _bufsize ) {
        _bufsize = ( index + 1 ) * 2;
        newbuf = new Implicit_item_id_ptr[_bufsize];
        memmove( newbuf, _buf, _count * sizeof( Implicit_item_id_ptr ) );
        delete[]_buf;
        _buf = newbuf;
    }
}

void Implicit_item_id__set::Insert( Implicit_item_id_ptr v, int index ) {
    Implicit_item_id_ptr * spot;
    index = ( index < 0 ) ? _count : index;

    if( index < _count ) {
        Check( _count + 1 );
        spot = &_buf[index];
        memmove( spot + 1, spot, ( _count - index )*sizeof( Implicit_item_id_ptr ) );

    } else {
        Check( index );
        spot = &_buf[index];
    }
    *spot = v;
    ++_count;
}

void Implicit_item_id__set::Append( Implicit_item_id_ptr v ) {
    int index = _count;
    Implicit_item_id_ptr * spot;

    if( index < _count ) {
        Check( _count + 1 );
        spot = &_buf[index];
        memmove( spot + 1, spot, ( _count - index )*sizeof( Implicit_item_id_ptr ) );

    } else {
        Check( index );
        spot = &_buf[index];
    }
    *spot = v;
    ++_count;
}

void Implicit_item_id__set::Remove( int index ) {
    if( 0 <= index && index < _count ) {
        --_count;
        Implicit_item_id_ptr * spot = &_buf[index];
        memmove( spot, spot + 1, ( _count - index )*sizeof( Implicit_item_id_ptr ) );
    }
}

int Implicit_item_id__set::Index( Implicit_item_id_ptr v ) {
    for( int i = 0; i < _count; ++i ) {
        if( _buf[i] == v ) {
            return i;
        }
    }
    return -1;
}

Implicit_item_id_ptr & Implicit_item_id__set::operator[]( int index ) {
    Check( index );
    _count = ( ( _count > index + 1 ) ? _count : ( index + 1 ) );
    return _buf[index];
}

int Implicit_item_id__set::Count() {
    return _count;
}

void Implicit_item_id__set::Clear() {
    _count = 0;
}

///////////////////////////////////////////////////////////////////////////////

Interface_spec__set::Interface_spec__set( int defaultSize ) {
    _bufsize = defaultSize;
    _buf = new Interface_spec_ptr[_bufsize];
    _count = 0;
}

Interface_spec__set::~Interface_spec__set() {
    delete[] _buf;
}

void Interface_spec__set::Check( int index ) {
    Interface_spec_ptr * newbuf;

    if( index >= _bufsize ) {
        _bufsize = ( index + 1 ) * 2;
        newbuf = new Interface_spec_ptr[_bufsize];
        memmove( newbuf, _buf, _count * sizeof( Interface_spec_ptr ) );
        delete[] _buf;
        _buf = newbuf;
    }
}

void Interface_spec__set::Insert( Interface_spec_ptr v, int index ) {
    Interface_spec_ptr * spot;
    index = ( index < 0 ) ? _count : index;

    if( index < _count ) {
        Check( _count + 1 );
        spot = &_buf[index];
        memmove( spot + 1, spot, ( _count - index )*sizeof( Interface_spec_ptr ) );

    } else {
        Check( index );
        spot = &_buf[index];
    }
    *spot = v;
    ++_count;
}

void Interface_spec__set::Append( Interface_spec_ptr v ) {
    int index = _count;
    Interface_spec_ptr * spot;

    if( index < _count ) {
        Check( _count + 1 );
        spot = &_buf[index];
        memmove( spot + 1, spot, ( _count - index )*sizeof( Interface_spec_ptr ) );

    } else {
        Check( index );
        spot = &_buf[index];
    }
    *spot = v;
    ++_count;
}

void Interface_spec__set::Remove( int index ) {
    if( 0 <= index && index < _count ) {
        --_count;
        Interface_spec_ptr * spot = &_buf[index];
        memmove( spot, spot + 1, ( _count - index )*sizeof( Interface_spec_ptr ) );
    }
}

int Interface_spec__set::Index( Interface_spec_ptr v ) {
    for( int i = 0; i < _count; ++i ) {
        if( _buf[i] == v ) {
            return i;
        }
    }
    return -1;
}

Interface_spec_ptr & Interface_spec__set::operator[]( int index ) {
    Check( index );
    _count = ( ( _count > index + 1 ) ? _count : ( index + 1 ) );
    return _buf[index];
}

int Interface_spec__set::Count() {
    return _count;
}

void Interface_spec__set::Clear() {
    _count = 0;
}

///////////////////////////////////////////////////////////////////////////////

Interface_spec::Interface_spec()
    : _explicit_items( new Explicit_item_id__set ),
      _implicit_items( 0 ), _all_objects( 0 ) {
}

/// not tested
Interface_spec::Interface_spec( Interface_spec & is ): Dictionary_instance() {
    _explicit_items = new Explicit_item_id__set;
    int count = is._explicit_items->Count();
    int i;
    for( i = 0; i < count; i++ ) {
        ( *_explicit_items )[i] =
            ( *( is._explicit_items ) )[i];
    }
    _current_schema_id = is._current_schema_id;
    _foreign_schema_id = is._foreign_schema_id;
    _all_objects = is._all_objects;
    _implicit_items = 0;
}

Interface_spec::Interface_spec( const char * cur_sch_id,
                                const char * foreign_sch_id, int all_objects )
    : _current_schema_id( cur_sch_id ), _explicit_items( new Explicit_item_id__set ),
      _implicit_items( 0 ), _foreign_schema_id( foreign_sch_id ),
      _all_objects( all_objects ) {
}

Interface_spec::~Interface_spec() {
    delete _explicit_items;
    delete _implicit_items;
}

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

Type_or_rule::Type_or_rule() {
    std::cerr << "WARNING - Type_or_rule class doesn't seem to be complete - it has no members!" << std::endl;
}

Type_or_rule::Type_or_rule( const Type_or_rule & tor ): Dictionary_instance() {
    ( void ) tor; //TODO once this class has some members, we'll actually have something to copy
}

Type_or_rule::~Type_or_rule() {
}


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

Global_rule::Global_rule()
    : _entities( 0 ), _where_rules( 0 ), _parent_schema( 0 ) {
}

Global_rule::Global_rule( const char * n, Schema_ptr parent_sch, const std::string & rt )
    : _name( n ), _entities( 0 ), _where_rules( 0 ), _parent_schema( parent_sch ),
      _rule_text( rt ) {
}

/// not fully implemented
Global_rule::Global_rule( Global_rule & gr ): Dictionary_instance() {
    _name = gr._name;
    _parent_schema = gr._parent_schema;
}

Global_rule::~Global_rule() {
    delete _entities;
    delete _where_rules;
}

void Global_rule::entities_( const Entity__set_var & e ) {
    if( _entities ) {
        if( _entities->EntryCount() > 0 ) {
            std::cerr << "In " << __FILE__ << ", Global_rule::entities_(): overwriting non-empty entity set!" << std::endl;
        }
        delete _entities;
    }
    _entities = e;
}

void Global_rule::where_rules_( const Where_rule__list_var & wrl ) {
    if( _where_rules ) {
        if( _where_rules->Count() > 0 ) {
            std::cerr << "In " << __FILE__ << ", Global_rule::where_rules_(): overwriting non-empty rule set!" << std::endl;
        }
        delete _where_rules;
    }
    _where_rules = wrl;
}

///////////////////////////////////////////////////////////////////////////////

Global_rule__set::Global_rule__set( int defaultSize ) {
    _bufsize = defaultSize;
    _buf = new Global_rule_ptr[_bufsize];
    _count = 0;
}

Global_rule__set::~Global_rule__set() {
    Clear();
    delete[] _buf;
}

void Global_rule__set::Check( int index ) {
    Global_rule_ptr * newbuf;

    if( index >= _bufsize ) {
        _bufsize = ( index + 1 ) * 2;
        newbuf = new Global_rule_ptr[_bufsize];
        memmove( newbuf, _buf, _count * sizeof( Global_rule_ptr ) );
        delete[] _buf;
        _buf = newbuf;
    }
}

void Global_rule__set::Insert( Global_rule_ptr v, int index ) {
    Global_rule_ptr * spot;
    index = ( index < 0 ) ? _count : index;

    if( index < _count ) {
        Check( _count + 1 );
        spot = &_buf[index];
        memmove( spot + 1, spot, ( _count - index )*sizeof( Global_rule_ptr ) );

    } else {
        Check( index );
        spot = &_buf[index];
    }
    *spot = v;
    ++_count;
}

void Global_rule__set::Append( Global_rule_ptr v ) {
    int index = _count;
    Global_rule_ptr * spot;

    if( index < _count ) {
        Check( _count + 1 );
        spot = &_buf[index];
        memmove( spot + 1, spot, ( _count - index )*sizeof( Global_rule_ptr ) );

    } else {
        Check( index );
        spot = &_buf[index];
    }
    *spot = v;
    ++_count;
}

void Global_rule__set::Remove( int index ) {
    if( 0 <= index && index < _count ) {
        --_count;
        Global_rule_ptr * spot = &_buf[index];
        memmove( spot, spot + 1, ( _count - index )*sizeof( Global_rule_ptr ) );
    }
}

int Global_rule__set::Index( Global_rule_ptr v ) {
    for( int i = 0; i < _count; ++i ) {
        if( _buf[i] == v ) {
            return i;
        }
    }
    return -1;
}

Global_rule_ptr & Global_rule__set::operator[]( int index ) {
    Check( index );
    _count = ( ( _count > index + 1 ) ? _count : ( index + 1 ) );
    return _buf[index];
}

int Global_rule__set::Count() {
    return _count;
}

void Global_rule__set::Clear() {
    for( int i = 0; i < _count; i ++ ) {
        delete _buf[i];
    }
    _count = 0;
}



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
