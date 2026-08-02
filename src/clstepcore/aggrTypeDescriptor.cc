#include "clstepcore/aggrTypeDescriptor.h"

#include "clstepcore/STEPaggregate.h"
#include "clstepcore/enumTypeDescriptor.h"
#include "clstepcore/selectTypeDescriptor.h"

namespace {

class DescriptorEnumAggregate : public EnumAggregate {
    const EnumTypeDescriptor * _enumType;

public:
    explicit DescriptorEnumAggregate( const EnumTypeDescriptor * enumType )
        : _enumType( enumType ) {
    }

    virtual SingleLinkNode * NewNode() {
        return new EnumNode( _enumType ? _enumType->CreateEnum() : 0 );
    }
};

}

STEPaggregate * AggrTypeDescriptor::CreateAggregate() const {
    if( CreateNewAggr ) {
        return CreateNewAggr();
    }

    const TypeDescriptor * element = ReferentType();
    const TypeDescriptor * concrete =
        element ? element->NonRefTypeDescriptor() : 0;
    const PrimitiveType type = element ? element->NonRefType() : UNKNOWN_TYPE;
    switch( type ) {
        case sdaiINSTANCE:
            return new EntityAggregate;
        case sdaiSELECT:
            return new SelectAggregate(
                dynamic_cast<const SelectTypeDescriptor *>( concrete ) );
        case sdaiENUMERATION:
            return new DescriptorEnumAggregate(
                dynamic_cast<const EnumTypeDescriptor *>( concrete ) );
        case sdaiBOOLEAN:
            return new BOOLEANS;
        case sdaiLOGICAL:
            return new LOGICALS;
        case sdaiINTEGER:
            return new IntAggregate;
        case sdaiREAL:
        case sdaiNUMBER:
            return new RealAggregate;
        case sdaiSTRING:
            return new StringAggregate;
        case sdaiBINARY:
            return new BinaryAggregate;
        default:
            return new GenericAggregate;
    }
}

void AggrTypeDescriptor::AssignAggrCreator( AggregateCreator f ) {
    CreateNewAggr = f;
}

AggrTypeDescriptor::AggrTypeDescriptor( ) :
    _bound1( -1 ), _bound2( -1 ), _uniqueElements( "UNKNOWN_TYPE" ),
    _aggrDomainType( 0 ), CreateNewAggr( 0 ),
    _bound1_type( bound_unset ), _bound2_type( bound_unset ),
    _bound1_callback( 0 ), _bound2_callback( 0 ) {
}

AggrTypeDescriptor::AggrTypeDescriptor( SDAI_Integer  b1,
                                        SDAI_Integer  b2,
                                        Logical uniqElem,
                                        TypeDescriptor * aggrDomType )
: _bound1( b1 ), _bound2( b2 ), _uniqueElements( uniqElem ),
  _aggrDomainType( aggrDomType ), CreateNewAggr( 0 ),
  _bound1_type( bound_constant ), _bound2_type( bound_constant ),
  _bound1_callback( 0 ), _bound2_callback( 0 ) {
}

AggrTypeDescriptor::~AggrTypeDescriptor() {
}
