#ifndef EXPDICT_H
#define EXPDICT_H

/*
* NIST STEP Core Class Library
* clstepcore/ExpDict.h
* April, 1997
* K. C. Morris
* David Sauder

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

#include <sc_export.h>
#include <sdai.h>

#include <vector>
#include <string>
#include <assert.h>



enum AggrBoundTypeEnum {
    bound_unset = 0,
    bound_constant,
    bound_runtime,
    bound_funcall
};

#include <SingleLinkList.h>

#include <baseType.h>
#include <dictdefs.h>
#include <Str.h>

// each of these contains linked list, list node, iterator
#include "attrDescriptorList.h"
#include "inverseAttributeList.h"
#include "typeDescriptorList.h"
#include "entityDescriptorList.h"

#include "typeDescriptor.h"
#include "entityDescriptor.h"
#include "enumTypeDescriptor.h"

#include "attrDescriptor.h"
#include "derivedAttribute.h"
#include "inverseAttribute.h"

#include "aggrCreatorTDs.h"


// defined and created in Registry.inline.cc
extern SC_CORE_EXPORT const TypeDescriptor  * t_sdaiINTEGER;
extern SC_CORE_EXPORT const TypeDescriptor  * t_sdaiREAL;
extern SC_CORE_EXPORT const TypeDescriptor  * t_sdaiNUMBER;
extern SC_CORE_EXPORT const TypeDescriptor  * t_sdaiSTRING;
extern SC_CORE_EXPORT const TypeDescriptor  * t_sdaiBINARY;
extern SC_CORE_EXPORT const TypeDescriptor  * t_sdaiBOOLEAN;
extern SC_CORE_EXPORT const TypeDescriptor  * t_sdaiLOGICAL;

#include "dictionaryInstance.h"

#include "uniquenessRule.h"
#include "whereRule.h"
#include "interfacedItem.h"

#include "explicitItemId.h"

#include "implicitItemId.h"


class SC_CORE_EXPORT Used_item : public Explicit_item_id {
    public:
        Used_item() {}
        Used_item( const char * foreign_schema, TypeDescriptor * ld,
                   const char * oi, const char * ni )
            : Explicit_item_id( foreign_schema, ld, oi, ni ) {}
        virtual ~Used_item();

        const char * EXPRESS_type() {
            return "USE";
        }
};

typedef Used_item * Used_item_ptr;

class SC_CORE_EXPORT Referenced_item : public Explicit_item_id {
    public:
        Referenced_item() {}
        Referenced_item( const char * foreign_schema, TypeDescriptor * ld,
                         const char * oi, const char * ni )
            : Explicit_item_id( foreign_schema, ld, oi, ni ) {}
        virtual ~Referenced_item();

        const char * EXPRESS_type() {
            return "REFERENCE";
        }
};

typedef Referenced_item * Referenced_item_ptr;


#include "interfaceSpec.h"

#include "typeOrRuleVar.h"
#include "globalRule.h"

#include "dictSchema.h"

#include "schRename.h"



/**
 * \class AggrTypeDescriptor
 * I think we decided on a simplistic representation of aggr. types for now?
 * i.e. just have one AggrTypeDesc for Array of [list of] [set of] someType
 * the inherited variable _referentType will point to the TypeDesc for someType
 * So I don't believe this class was necessary.  If we were to retain
 * info for each of the [aggr of]'s in the example above then there would be
 * one of these for each [aggr of] above and they would be strung
 * together by the _aggrDomainType variables.  If you can make this
 * work then go for it.
 */
class SC_CORE_EXPORT AggrTypeDescriptor  :    public TypeDescriptor  {

    protected:

        SDAI_Integer  _bound1, _bound2;
        SDAI_LOGICAL _uniqueElements;
        TypeDescriptor * _aggrDomainType;
        AggregateCreator CreateNewAggr;

        AggrBoundTypeEnum _bound1_type, _bound2_type;
        boundCallbackFn _bound1_callback, _bound2_callback;
        std::string _bound1_str, _bound2_str;

    public:

        void AssignAggrCreator( AggregateCreator f = 0 );

        STEPaggregate * CreateAggregate();

        AggrTypeDescriptor( );
        AggrTypeDescriptor( SDAI_Integer b1, SDAI_Integer b2,
                            Logical uniqElem,
                            TypeDescriptor * aggrDomType );
        AggrTypeDescriptor( const char * nm, PrimitiveType ft,
                            Schema * origSchema, const char * d,
                            AggregateCreator f = 0 )
            : TypeDescriptor( nm, ft, origSchema, d ), _bound1( 0 ), _bound2( 0 ), _uniqueElements( 0 ), _aggrDomainType( NULL ), CreateNewAggr( f ) { }
        virtual ~AggrTypeDescriptor();


        /// find bound type
        AggrBoundTypeEnum Bound1Type() const {
            return _bound1_type;
        }
        /// get a constant bound
        SDAI_Integer Bound1( ) const {
            assert( _bound1_type == bound_constant );
            return _bound1;
        }
        /// get a runtime bound using an object's 'this' pointer
        SDAI_Integer Bound1Runtime( SDAI_Application_instance * this_ptr ) const {
            assert( this_ptr && ( _bound1_type == bound_runtime ) );
            return _bound1_callback( this_ptr ) ;
        }
        /// get a bound's EXPRESS function call string
        std::string Bound1Funcall() const {
            return _bound1_str;
        }
        /// set bound to a constant
        void SetBound1( SDAI_Integer  b1 )   {
            _bound1 = b1;
            _bound1_type = bound_constant;
        }
        ///set bound's callback fn. only for bounds dependent on an attribute
        void SetBound1FromMemberAccessor( boundCallbackFn callback ) {
            _bound1_callback = callback;
            _bound1_type = bound_runtime;
        }
        ///set bound from express function call. currently, this only stores the function call as a string.
        void SetBound1FromExpressFuncall( std::string s ) {
            _bound1_str = s;
            _bound1_type = bound_funcall;
        }

        /// find bound type
        AggrBoundTypeEnum Bound2Type() const {
            return _bound2_type;
        }
        /// get a constant bound
        SDAI_Integer Bound2( ) const {
            assert( _bound2_type == bound_constant );
            return _bound2;
        }
        /// get a runtime bound using an object's 'this' pointer
        SDAI_Integer Bound2Runtime( SDAI_Application_instance * this_ptr ) const {
            assert( this_ptr && ( _bound2_type == bound_runtime ) );
            return _bound2_callback( this_ptr ) ;
        }
        /// get a bound's EXPRESS function call string
        std::string Bound2Funcall() const {
            return _bound2_str;
        }
        /// set bound to a constant
        void SetBound2( SDAI_Integer  b2 )   {
            _bound2 = b2;
            _bound2_type = bound_constant;
        }
        ///set bound's callback fn
        void SetBound2FromMemberAccessor( boundCallbackFn callback ) {
            _bound2_callback = callback;
            _bound2_type = bound_runtime;
        }
        ///set bound from express function call. currently, this only stores the function call as a string.
        void SetBound2FromExpressFuncall( std::string s ) {
            _bound2_str = s;
            _bound2_type = bound_funcall;
        }

        SDAI_LOGICAL & UniqueElements() {
            return _uniqueElements;
        }
        void UniqueElements( SDAI_LOGICAL & ue ) {
            _uniqueElements.put( ue.asInt() );
        }
        void UniqueElements( Logical ue )     {
            _uniqueElements.put( ue );
        }
        void UniqueElements( const char * ue ) {
            _uniqueElements.put( ue );
        }

        class TypeDescriptor * AggrDomainType()    {
                return _aggrDomainType;
        }
        void AggrDomainType( TypeDescriptor * adt ) {
            _aggrDomainType = adt;
        }
};

///////////////////////////////////////////////////////////////////////////////
// ArrayTypeDescriptor
///////////////////////////////////////////////////////////////////////////////
class SC_CORE_EXPORT ArrayTypeDescriptor  :    public AggrTypeDescriptor  {

    protected:
        SDAI_LOGICAL  _optionalElements;
    public:

        ArrayTypeDescriptor( ) : _optionalElements( "UNKNOWN_TYPE" ) { }
        ArrayTypeDescriptor( Logical optElem ) : _optionalElements( optElem )
        { }
        ArrayTypeDescriptor( const char * nm, PrimitiveType ft,
                             Schema * origSchema, const char * d,
                             AggregateCreator f = 0 )
            : AggrTypeDescriptor( nm, ft, origSchema, d, f ),
              _optionalElements( "UNKNOWN_TYPE" )
        { }

        virtual ~ArrayTypeDescriptor() {}


        SDAI_LOGICAL & OptionalElements() {
            return _optionalElements;
        }
        void OptionalElements( SDAI_LOGICAL & oe ) {
            _optionalElements.put( oe.asInt() );
        }
        void OptionalElements( Logical oe )     {
            _optionalElements.put( oe );
        }
        void OptionalElements( const char * oe ) {
            _optionalElements.put( oe );
        }
};

class SC_CORE_EXPORT ListTypeDescriptor  :    public AggrTypeDescriptor  {

    protected:
    public:
        ListTypeDescriptor( ) { }
        ListTypeDescriptor( const char * nm, PrimitiveType ft,
                            Schema * origSchema, const char * d,
                            AggregateCreator f = 0 )
            : AggrTypeDescriptor( nm, ft, origSchema, d, f ) { }
        virtual ~ListTypeDescriptor() { }

};

class SC_CORE_EXPORT SetTypeDescriptor  :    public AggrTypeDescriptor  {

    protected:
    public:

        SetTypeDescriptor( ) { }
        SetTypeDescriptor( const char * nm, PrimitiveType ft,
                           Schema * origSchema, const char * d,
                           AggregateCreator f = 0 )
            : AggrTypeDescriptor( nm, ft, origSchema, d, f ) { }
        virtual ~SetTypeDescriptor() { }

};

class SC_CORE_EXPORT BagTypeDescriptor  :    public AggrTypeDescriptor  {

    protected:
    public:

        BagTypeDescriptor( ) { }
        BagTypeDescriptor( const char * nm, PrimitiveType ft,
                           Schema * origSchema, const char * d,
                           AggregateCreator f = 0 )
            : AggrTypeDescriptor( nm, ft, origSchema, d, f ) { }
        virtual ~BagTypeDescriptor() { }

};

typedef SDAI_Select * ( * SelectCreator )();

class SC_CORE_EXPORT SelectTypeDescriptor  :    public TypeDescriptor  {

    protected:
        TypeDescriptorList _elements;    //  of  TYPE_DESCRIPTOR
        int _unique_elements;

    public:

        SelectCreator CreateNewSelect;

        void AssignSelectCreator( SelectCreator f = 0 ) {
            CreateNewSelect = f;
        }

        SDAI_Select * CreateSelect();

        SelectTypeDescriptor( int b, const char * nm, PrimitiveType ft,
                              Schema * origSchema,
                              const char * d, SelectCreator f = 0 )
            : TypeDescriptor( nm, ft, origSchema, d ),
              _unique_elements( b ), CreateNewSelect( f )
        { }
        virtual ~SelectTypeDescriptor() { }

        TypeDescriptorList & Elements() {
            return _elements;
        }
        const TypeDescriptorList & GetElements() const {
            return _elements;
        }
        int UniqueElements() const {
            return _unique_elements;
        }
        virtual const TypeDescriptor * IsA( const TypeDescriptor * ) const;
        virtual const TypeDescriptor * IsA( const char * n ) const {
            return TypeDescriptor::IsA( n );
        }
        virtual const TypeDescriptor * CanBe( const TypeDescriptor * ) const;
        virtual const TypeDescriptor * CanBe( const char * n ) const;
        virtual const TypeDescriptor * CanBeSet( const char *, const char * )
        const;
};

class SC_CORE_EXPORT StringTypeDescriptor  :    public TypeDescriptor  {

    protected:
        SDAI_Integer   _width;  //  OPTIONAL
        SDAI_LOGICAL  _fixedSize;
    public:

        StringTypeDescriptor( ) : _fixedSize( "UNKNOWN_TYPE" ) {
            _width = 0;
        }
        virtual ~StringTypeDescriptor() { }


        SDAI_Integer Width() {
            return _width;
        }
        void Width( SDAI_Integer w ) {
            _width = w;
        }

        SDAI_LOGICAL & FixedSize() {
            return _fixedSize;
        }
        void FixedSize( SDAI_LOGICAL fs ) {
            _fixedSize.put( fs.asInt() );
        }
        void FixedSize( Logical fs ) {
            _fixedSize.put( fs );
        }
};

class SC_CORE_EXPORT RealTypeDescriptor  :    public TypeDescriptor  {

    protected:
        SDAI_Integer _precisionSpec;  //  OPTIONAL
    public:

        RealTypeDescriptor( ) {
            _precisionSpec = 0;
        }
        virtual ~RealTypeDescriptor() { }

        SDAI_Integer PrecisionSpec() {
            return _precisionSpec;
        }
        void PrecisionSpec( SDAI_Integer ps ) {
            _precisionSpec = ps;
        }
};


#endif
