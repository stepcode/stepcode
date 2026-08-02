#include "clstepcore/schemaInit.h"

#include "clstepcore/Registry.h"
#include "clstepcore/derivedAttribute.h"
#include "clstepcore/inverseAttribute.h"
#include "clstepcore/selectTypeDescriptor.h"

TypeDescriptorInitRecord::TypeDescriptorInitRecord(
    TypeDescriptor ** descriptorSlot, const char * typeName,
    PrimitiveType type, Schema ** schema, const char * typeDescription )
    : slot( descriptorSlot ), kind( TypeDescriptorInit_Base ), name( typeName ),
      fundamentalType( type ), schemaSlot( schema ), description( typeDescription ),
      uniqueElements( 0 ), selectCreator( 0 ), enumCreator( 0 ),
      aggregateCreator( 0 ) {
}

TypeDescriptorInitRecord::TypeDescriptorInitRecord(
    EnumTypeDescriptor ** descriptorSlot, const char * typeName,
    PrimitiveType type, Schema ** schema, const char * typeDescription,
    EnumCreator creator )
    : slot( descriptorSlot ), kind( TypeDescriptorInit_Enum ), name( typeName ),
      fundamentalType( type ), schemaSlot( schema ), description( typeDescription ),
      uniqueElements( 0 ), selectCreator( 0 ), enumCreator( creator ),
      aggregateCreator( 0 ) {
}

TypeDescriptorInitRecord::TypeDescriptorInitRecord(
    SelectTypeDescriptor ** descriptorSlot, int unique, const char * typeName,
    PrimitiveType type, Schema ** schema, const char * typeDescription,
    SelectCreator creator )
    : slot( descriptorSlot ), kind( TypeDescriptorInit_Select ), name( typeName ),
      fundamentalType( type ), schemaSlot( schema ), description( typeDescription ),
      uniqueElements( unique ), selectCreator( creator ), enumCreator( 0 ),
      aggregateCreator( 0 ) {
}

#define AGGREGATE_RECORD_CONSTRUCTOR( descriptorType, descriptorKind ) \
TypeDescriptorInitRecord::TypeDescriptorInitRecord( \
    descriptorType ** descriptorSlot, const char * typeName, \
    PrimitiveType type, Schema ** schema, const char * typeDescription, \
    AggregateCreator creator ) \
    : slot( descriptorSlot ), kind( descriptorKind ), name( typeName ), \
      fundamentalType( type ), schemaSlot( schema ), description( typeDescription ), \
      uniqueElements( 0 ), selectCreator( 0 ), enumCreator( 0 ), \
      aggregateCreator( creator ) { \
}

AGGREGATE_RECORD_CONSTRUCTOR( AggrTypeDescriptor, TypeDescriptorInit_Aggregate )
AGGREGATE_RECORD_CONSTRUCTOR( ArrayTypeDescriptor, TypeDescriptorInit_Array )
AGGREGATE_RECORD_CONSTRUCTOR( ListTypeDescriptor, TypeDescriptorInit_List )
AGGREGATE_RECORD_CONSTRUCTOR( SetTypeDescriptor, TypeDescriptorInit_Set )
AGGREGATE_RECORD_CONSTRUCTOR( BagTypeDescriptor, TypeDescriptorInit_Bag )

#undef AGGREGATE_RECORD_CONSTRUCTOR

AttributeInitRecord::AttributeInitRecord(
    AttrDescriptor ** slot, const char * attrName, const TypeDescriptor * domainType,
    Logical isOptional, Logical isUnique, AttrType_Enum type,
    const char * init )
    : attributeSlot( slot ), derivedSlot( 0 ), inverseSlot( 0 ),
      name( attrName ), domain( domainType ), optional( isOptional ),
      unique( isUnique ), attrType( type ), initializer( init ),
      invertedAttribute( 0 ), invertedEntity( 0 ) {
}

AttributeInitRecord::AttributeInitRecord(
    Derived_attribute ** slot, const char * attrName,
    const TypeDescriptor * domainType, Logical isOptional, Logical isUnique,
    AttrType_Enum type, const char * init )
    : attributeSlot( 0 ), derivedSlot( slot ), inverseSlot( 0 ),
      name( attrName ), domain( domainType ), optional( isOptional ),
      unique( isUnique ), attrType( type ), initializer( init ),
      invertedAttribute( 0 ), invertedEntity( 0 ) {
}

AttributeInitRecord::AttributeInitRecord(
    Inverse_attribute ** slot, const char * attrName,
    const TypeDescriptor * domainType, Logical isOptional, Logical isUnique,
    const char * invertedAttr, const char * invertedEnt )
    : attributeSlot( 0 ), derivedSlot( 0 ), inverseSlot( slot ),
      name( attrName ), domain( domainType ), optional( isOptional ),
      unique( isUnique ), attrType( AttrType_Inverse ), initializer( 0 ),
      invertedAttribute( invertedAttr ), invertedEntity( invertedEnt ) {
}

void InitializeSchemas( Registry & reg, const SchemaInitRecord * records,
                        size_t count ) {
    for( size_t i = 0; i < count; ++i ) {
        Schema * schema = new Schema( records[i].name );
        schema->AssignModelContentsCreator( records[i].modelContentsCreator );
        *records[i].slot = schema;
        reg.AddSchema( *schema );
    }
}

void InitializeEntityDescriptors( const EntityDescriptorInitRecord * records,
                                  size_t count ) {
    for( size_t i = 0; i < count; ++i ) {
        const EntityDescriptorInitRecord & record = records[i];
        EntityDescriptor * entity = new EntityDescriptor(
            record.name, *record.schemaSlot, record.abstractEntity,
            record.externalMapping, record.creator );
        *record.slot = entity;
        ( *record.schemaSlot )->AddEntity( entity );
    }
}

void InitializeTypeDescriptors( const TypeDescriptorInitRecord * records,
                                size_t count ) {
    for( size_t i = 0; i < count; ++i ) {
        const TypeDescriptorInitRecord & record = records[i];
        Schema * schema = *record.schemaSlot;
        switch( record.kind ) {
            case TypeDescriptorInit_Enum:
                *static_cast<EnumTypeDescriptor **>( record.slot ) =
                    new EnumTypeDescriptor( record.name, record.fundamentalType,
                                            schema, record.description,
                                            record.enumCreator );
                break;
            case TypeDescriptorInit_Select:
                *static_cast<SelectTypeDescriptor **>( record.slot ) =
                    new SelectTypeDescriptor( record.uniqueElements, record.name,
                                              record.fundamentalType, schema,
                                              record.description,
                                              record.selectCreator );
                break;
            case TypeDescriptorInit_Aggregate:
                *static_cast<AggrTypeDescriptor **>( record.slot ) =
                    new AggrTypeDescriptor( record.name, record.fundamentalType,
                                            schema, record.description,
                                            record.aggregateCreator );
                break;
            case TypeDescriptorInit_Array:
                *static_cast<ArrayTypeDescriptor **>( record.slot ) =
                    new ArrayTypeDescriptor( record.name, record.fundamentalType,
                                             schema, record.description,
                                             record.aggregateCreator );
                break;
            case TypeDescriptorInit_List:
                *static_cast<ListTypeDescriptor **>( record.slot ) =
                    new ListTypeDescriptor( record.name, record.fundamentalType,
                                            schema, record.description,
                                            record.aggregateCreator );
                break;
            case TypeDescriptorInit_Set:
                *static_cast<SetTypeDescriptor **>( record.slot ) =
                    new SetTypeDescriptor( record.name, record.fundamentalType,
                                           schema, record.description,
                                           record.aggregateCreator );
                break;
            case TypeDescriptorInit_Bag:
                *static_cast<BagTypeDescriptor **>( record.slot ) =
                    new BagTypeDescriptor( record.name, record.fundamentalType,
                                           schema, record.description,
                                           record.aggregateCreator );
                break;
            case TypeDescriptorInit_Base:
            default:
                *static_cast<TypeDescriptor **>( record.slot ) =
                    new TypeDescriptor( record.name, record.fundamentalType,
                                        schema, record.description );
                break;
        }
    }
}

void InitializeTypeMetadata(
    Registry & reg, Schema & schema, TypeDescriptor & type,
    const TypeDescriptor * referent,
    const TypeDescriptor * const * selectElements, size_t selectElementCount ) {
    if( referent ) {
        type.ReferentType( referent );
    }
    SelectTypeDescriptor * select = dynamic_cast<SelectTypeDescriptor *>( &type );
    if( select ) {
        for( size_t i = 0; i < selectElementCount; ++i ) {
            select->Elements().AddNode(
                const_cast<TypeDescriptor *>( selectElements[i] ) );
        }
    }
    schema.AddType( &type );
    reg.AddType( type );
}

void InitializeEntityMetadata(
    Registry & reg, EntityDescriptor & entity, Schema & schema,
    const EntitySupertypeInitRecord * supertypes, size_t supertypeCount,
    const AttributeInitRecord * attributes, size_t attributeCount ) {
    size_t i = 0;
    for( ; i < supertypeCount; ++i ) {
        EntityDescriptor * supertype = supertypes[i].supertype;
        entity.AddSupertype( supertype );
        supertype->AddSubtype( &entity );
    }

    bool hasInverse = false;
    for( i = 0; i < attributeCount; ++i ) {
        const AttributeInitRecord & record = attributes[i];
        if( record.inverseSlot ) {
            Inverse_attribute * inverse = new Inverse_attribute(
                record.name, const_cast<TypeDescriptor *>( record.domain ),
                record.optional, record.unique,
                entity, record.invertedAttribute );
            inverse->inverted_entity_id_( record.invertedEntity );
            *record.inverseSlot = inverse;
            entity.AddInverseAttr( inverse );
            hasInverse = true;
        } else if( record.derivedSlot ) {
            Derived_attribute * derived = new Derived_attribute(
                record.name, record.domain, record.optional, record.unique,
                record.attrType, entity );
            derived->initializer_( record.initializer );
            *record.derivedSlot = derived;
            entity.AddExplicitAttr( derived );
        } else {
            AttrDescriptor * attribute = new AttrDescriptor(
                record.name, record.domain, record.optional, record.unique,
                record.attrType, entity );
            *record.attributeSlot = attribute;
            entity.AddExplicitAttr( attribute );
        }
    }

    reg.AddEntity( entity );
    if( hasInverse ) {
        schema.AddEntityWInverse( &entity );
    }
}
