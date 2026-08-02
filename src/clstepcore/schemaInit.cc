#include "clstepcore/schemaInit.h"

#include "clstepcore/Registry.h"
#include "clstepcore/complexSupport.h"
#include "clstepcore/derivedAttribute.h"
#include "clstepcore/globalRule.h"
#include "clstepcore/inverseAttribute.h"
#include "clstepcore/selectTypeDescriptor.h"
#include "clstepcore/schemaModule.h"
#include "clstepcore/uniquenessRule.h"
#include "clstepcore/whereRule.h"

#include <vector>

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

void InitializeSchemaModuleImages( const SchemaInitRecord * records,
                                   size_t count ) {
    for( size_t i = 0; i < count; ++i ) {
        BeginSchemaModuleImage( **records[i].slot );
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
        RecordSchemaModuleEntity( **record.schemaSlot, entity );
    }
}

void InitializeTypeDescriptors( const TypeDescriptorInitRecord * records,
                                size_t count ) {
    for( size_t i = 0; i < count; ++i ) {
        const TypeDescriptorInitRecord & record = records[i];
        Schema * schema = *record.schemaSlot;
        TypeDescriptor * type = 0;
        switch( record.kind ) {
            case TypeDescriptorInit_Enum: {
                EnumTypeDescriptor * descriptor = new EnumTypeDescriptor(
                    record.name, record.fundamentalType, schema,
                    record.description, record.enumCreator );
                *static_cast<EnumTypeDescriptor **>( record.slot ) = descriptor;
                type = descriptor;
                break;
            }
            case TypeDescriptorInit_Select: {
                SelectTypeDescriptor * descriptor = new SelectTypeDescriptor(
                    record.uniqueElements, record.name, record.fundamentalType,
                    schema, record.description, record.selectCreator );
                *static_cast<SelectTypeDescriptor **>( record.slot ) = descriptor;
                type = descriptor;
                break;
            }
            case TypeDescriptorInit_Aggregate: {
                AggrTypeDescriptor * descriptor = new AggrTypeDescriptor(
                    record.name, record.fundamentalType, schema,
                    record.description, record.aggregateCreator );
                *static_cast<AggrTypeDescriptor **>( record.slot ) = descriptor;
                type = descriptor;
                break;
            }
            case TypeDescriptorInit_Array: {
                ArrayTypeDescriptor * descriptor = new ArrayTypeDescriptor(
                    record.name, record.fundamentalType, schema,
                    record.description, record.aggregateCreator );
                *static_cast<ArrayTypeDescriptor **>( record.slot ) = descriptor;
                type = descriptor;
                break;
            }
            case TypeDescriptorInit_List: {
                ListTypeDescriptor * descriptor = new ListTypeDescriptor(
                    record.name, record.fundamentalType, schema,
                    record.description, record.aggregateCreator );
                *static_cast<ListTypeDescriptor **>( record.slot ) = descriptor;
                type = descriptor;
                break;
            }
            case TypeDescriptorInit_Set: {
                SetTypeDescriptor * descriptor = new SetTypeDescriptor(
                    record.name, record.fundamentalType, schema,
                    record.description, record.aggregateCreator );
                *static_cast<SetTypeDescriptor **>( record.slot ) = descriptor;
                type = descriptor;
                break;
            }
            case TypeDescriptorInit_Bag: {
                BagTypeDescriptor * descriptor = new BagTypeDescriptor(
                    record.name, record.fundamentalType, schema,
                    record.description, record.aggregateCreator );
                *static_cast<BagTypeDescriptor **>( record.slot ) = descriptor;
                type = descriptor;
                break;
            }
            case TypeDescriptorInit_Base:
            default:
                type = new TypeDescriptor(
                    record.name, record.fundamentalType, schema,
                    record.description );
                *static_cast<TypeDescriptor **>( record.slot ) = type;
                break;
        }
        RecordSchemaModuleType( *schema, type );
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
            RecordSchemaModuleAttribute( schema, inverse );
            hasInverse = true;
        } else if( record.derivedSlot ) {
            Derived_attribute * derived = new Derived_attribute(
                record.name, record.domain, record.optional, record.unique,
                record.attrType, entity );
            derived->initializer_( record.initializer );
            *record.derivedSlot = derived;
            entity.AddExplicitAttr( derived );
            RecordSchemaModuleAttribute( schema, derived );
        } else {
            AttrDescriptor * attribute = new AttrDescriptor(
                record.name, record.domain, record.optional, record.unique,
                record.attrType, entity );
            *record.attributeSlot = attribute;
            entity.AddExplicitAttr( attribute );
            RecordSchemaModuleAttribute( schema, attribute );
        }
    }

    reg.AddEntity( entity );
    if( hasInverse ) {
        schema.AddEntityWInverse( &entity );
    }
}

void InitializeWhereRules( TypeDescriptor & owner,
                           const char * const * rules, size_t count ) {
    if( !count ) {
        return;
    }
    owner._where_rules = new Where_rule__list;
    for( size_t i = 0; i < count; ++i ) {
        owner._where_rules->Append( new Where_rule( rules[i] ) );
    }
}

void InitializeUniquenessRules( EntityDescriptor & owner,
                                const char * const * rules, size_t count ) {
    if( !count ) {
        return;
    }
    owner._uniqueness_rules = new Uniqueness_rule__set;
    for( size_t i = 0; i < count; ++i ) {
        owner._uniqueness_rules->Append( new Uniqueness_rule( rules[i] ) );
    }
}

void InitializeGlobalRules( Schema & schema,
                            const GlobalRuleInitRecord * rules,
                            size_t count ) {
    for( size_t i = 0; i < count; ++i ) {
        schema.AddGlobal_rule(
            new Global_rule( rules[i].name, &schema, rules[i].text ) );
    }
}

void InitializeFunctions( Schema & schema,
                          const char * const * functions, size_t count ) {
    for( size_t i = 0; i < count; ++i ) {
        schema.AddFunction( functions[i] );
    }
}

void InitializeProcedures( Schema & schema,
                           const char * const * procedures, size_t count ) {
    for( size_t i = 0; i < count; ++i ) {
        schema.AddProcedure( procedures[i] );
    }
}

ComplexCollect * InitializeComplexSupport(
    const ComplexNodeInitRecord * nodes, size_t nodeCount,
    const ComplexListInitRecord * lists, size_t listCount ) {
    if( !listCount ) {
        return 0;
    }

    const size_t noNode = static_cast<size_t>( -1 );
    std::vector<EntList *> runtimeNodes( nodeCount, 0 );
    for( size_t i = 0; i < nodeCount; ++i ) {
        switch( nodes[i].kind ) {
            case ComplexNodeInit_And:
                runtimeNodes[i] = new AndList;
                break;
            case ComplexNodeInit_Or:
                runtimeNodes[i] = new OrList;
                break;
            case ComplexNodeInit_AndOr:
                runtimeNodes[i] = new AndOrList;
                break;
            case ComplexNodeInit_Simple:
            default:
                runtimeNodes[i] = new SimpleList( nodes[i].name );
                break;
        }
    }

    for( size_t i = 0; i < nodeCount; ++i ) {
        if( nodes[i].nextSibling != noNode ) {
            runtimeNodes[i]->next = runtimeNodes[nodes[i].nextSibling];
            runtimeNodes[nodes[i].nextSibling]->prev = runtimeNodes[i];
        }
    }
    /* Sibling chains must be complete before appendList counts them. */
    for( size_t i = 0; i < nodeCount; ++i ) {
        if( nodes[i].firstChild != noNode ) {
            MultList * parent = dynamic_cast<MultList *>( runtimeNodes[i] );
            parent->appendList( runtimeNodes[nodes[i].firstChild] );
        }
    }

    ComplexCollect * result = new ComplexCollect;
    for( size_t i = 0; i < listCount; ++i ) {
        AndList * root = dynamic_cast<AndList *>( runtimeNodes[lists[i].rootNode] );
        ComplexList * list = new ComplexList( root );
        list->buildList();
        root->setLevel( 0 );
        result->insert( list );
    }
    return result;
}
