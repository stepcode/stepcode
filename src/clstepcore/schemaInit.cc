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

#include <cstring>
#include <limits>
#include <vector>

namespace {

SchemaLoadResult loadError( SchemaLoadError error, uint32_t record = 0 ) {
    SchemaLoadResult result = { error, record };
    return result;
}

template <class Record>
const Record * imageRecords( const SchemaModuleImage & image,
                             uint32_t offset, uint32_t count,
                             SchemaLoadResult & result ) {
    if( offset > image.byteSize ||
            count > ( image.byteSize - offset ) / sizeof( Record ) ||
            offset % alignof( Record ) ) {
        result = loadError( SchemaLoad_InvalidOffset );
        return 0;
    }
    return reinterpret_cast<const Record *>(
        reinterpret_cast<const unsigned char *>( &image ) + offset );
}

const char * imageStrings( const SchemaModuleImage & image,
                           SchemaLoadResult & result ) {
    if( image.stringOffset > image.byteSize ||
            image.stringBytes > image.byteSize - image.stringOffset ) {
        result = loadError( SchemaLoad_InvalidOffset );
        return 0;
    }
    return reinterpret_cast<const char *>( &image ) + image.stringOffset;
}

template <class Record>
const Record * packedComplexRecords( const PackedComplexImage & image,
                                     uint32_t offset, uint32_t count,
                                     SchemaLoadResult & result ) {
    if( offset > image.byteSize ||
            count > ( image.byteSize - offset ) / sizeof( Record ) ||
            offset % alignof( Record ) ) {
        result = loadError( SchemaLoad_InvalidOffset );
        return 0;
    }
    return reinterpret_cast<const Record *>(
        reinterpret_cast<const unsigned char *>( &image ) + offset );
}

bool validateComplexNode( uint32_t index, const PackedComplexNode * nodes,
                          uint32_t nodeCount,
                          std::vector<unsigned char> & state ) {
    if( index >= nodeCount || state[index] == 1 ) {
        return false;
    }
    if( state[index] == 2 ) {
        return true;
    }
    state[index] = 1;
    const uint32_t noNode = UINT32_MAX;
    if( nodes[index].firstChild != noNode &&
            !validateComplexNode( nodes[index].firstChild, nodes,
                                  nodeCount, state ) ) {
        return false;
    }
    if( nodes[index].nextSibling != noNode &&
            !validateComplexNode( nodes[index].nextSibling, nodes,
                                  nodeCount, state ) ) {
        return false;
    }
    state[index] = 2;
    return true;
}

bool validString( const char * strings, uint32_t bytes, uint32_t offset ) {
    return offset < bytes &&
        std::memchr( strings + offset, 0, bytes - offset ) != 0;
}

const TypeDescriptor * builtinDescriptor( uint32_t primitive ) {
    switch( static_cast<PrimitiveType>( primitive ) ) {
        case sdaiINTEGER:
            return t_sdaiINTEGER;
        case sdaiREAL:
            return t_sdaiREAL;
        case sdaiNUMBER:
            return t_sdaiNUMBER;
        case sdaiSTRING:
            return t_sdaiSTRING;
        case sdaiBINARY:
            return t_sdaiBINARY;
        case sdaiBOOLEAN:
            return t_sdaiBOOLEAN;
        case sdaiLOGICAL:
            return t_sdaiLOGICAL;
        default:
            return 0;
    }
}

bool validReference( const SchemaImageDescriptorRef & reference,
                     const SchemaImageSchemaRecord * schemas,
                     uint32_t schemaCount, uint32_t aggregateCount ) {
    switch( reference.kind ) {
        case SchemaImageRef_None:
            return true;
        case SchemaImageRef_Entity:
            return reference.schema < schemaCount &&
                reference.index < schemas[reference.schema].entityCount;
        case SchemaImageRef_Type:
            return reference.schema < schemaCount &&
                reference.index < schemas[reference.schema].typeCount;
        case SchemaImageRef_Builtin:
            return builtinDescriptor( reference.index ) != 0;
        case SchemaImageRef_Aggregate:
            return reference.index < aggregateCount;
        default:
            return false;
    }
}

const TypeDescriptor * resolveReference(
    const SchemaImageDescriptorRef & reference,
    const SchemaImageSchemaRecord * schemas,
    const std::vector<EntityDescriptor *> & entities,
    const std::vector<TypeDescriptor *> & types,
    const std::vector<AggrTypeDescriptor *> & aggregates ) {
    switch( reference.kind ) {
        case SchemaImageRef_Entity:
            return entities[schemas[reference.schema].firstEntity +
                            reference.index];
        case SchemaImageRef_Type:
            return types[schemas[reference.schema].firstType +
                         reference.index];
        case SchemaImageRef_Builtin:
            return builtinDescriptor( reference.index );
        case SchemaImageRef_Aggregate:
            return aggregates[reference.index];
        case SchemaImageRef_None:
        default:
            return 0;
    }
}

TypeDescriptor * createTypeDescriptor(
    const SchemaImageTypeRecord & record, const char * name,
    const char * description, Schema * schema,
    const SchemaImageTypeBinding * binding ) {
    const TypeDescriptorInitKind kind =
        static_cast<TypeDescriptorInitKind>( record.descriptorKind );
    TypeDescriptor * result = 0;
    switch( kind ) {
        case TypeDescriptorInit_Enum:
            result = new EnumTypeDescriptor(
                name, static_cast<PrimitiveType>( record.fundamentalType ),
                schema, description, binding ? binding->enumCreator : 0 );
            break;
        case TypeDescriptorInit_Select:
            result = new SelectTypeDescriptor(
                record.uniqueElements, name,
                static_cast<PrimitiveType>( record.fundamentalType ), schema,
                description, binding ? binding->selectCreator : 0 );
            break;
        case TypeDescriptorInit_Aggregate:
            result = new AggrTypeDescriptor(
                name, static_cast<PrimitiveType>( record.fundamentalType ),
                schema, description, binding ? binding->aggregateCreator : 0 );
            break;
        case TypeDescriptorInit_Array:
            result = new ArrayTypeDescriptor(
                name, static_cast<PrimitiveType>( record.fundamentalType ),
                schema, description, binding ? binding->aggregateCreator : 0 );
            break;
        case TypeDescriptorInit_List:
            result = new ListTypeDescriptor(
                name, static_cast<PrimitiveType>( record.fundamentalType ),
                schema, description, binding ? binding->aggregateCreator : 0 );
            break;
        case TypeDescriptorInit_Set:
            result = new SetTypeDescriptor(
                name, static_cast<PrimitiveType>( record.fundamentalType ),
                schema, description, binding ? binding->aggregateCreator : 0 );
            break;
        case TypeDescriptorInit_Bag:
            result = new BagTypeDescriptor(
                name, static_cast<PrimitiveType>( record.fundamentalType ),
                schema, description, binding ? binding->aggregateCreator : 0 );
            break;
        case TypeDescriptorInit_Base:
        default:
            result = new TypeDescriptor(
                name, static_cast<PrimitiveType>( record.fundamentalType ),
                schema, description );
            break;
    }
    if( binding && binding->slot ) {
        switch( kind ) {
            case TypeDescriptorInit_Enum:
                *static_cast<EnumTypeDescriptor **>( binding->slot ) =
                    static_cast<EnumTypeDescriptor *>( result );
                break;
            case TypeDescriptorInit_Select:
                *static_cast<SelectTypeDescriptor **>( binding->slot ) =
                    static_cast<SelectTypeDescriptor *>( result );
                break;
            case TypeDescriptorInit_Aggregate:
                *static_cast<AggrTypeDescriptor **>( binding->slot ) =
                    static_cast<AggrTypeDescriptor *>( result );
                break;
            case TypeDescriptorInit_Array:
                *static_cast<ArrayTypeDescriptor **>( binding->slot ) =
                    static_cast<ArrayTypeDescriptor *>( result );
                break;
            case TypeDescriptorInit_List:
                *static_cast<ListTypeDescriptor **>( binding->slot ) =
                    static_cast<ListTypeDescriptor *>( result );
                break;
            case TypeDescriptorInit_Set:
                *static_cast<SetTypeDescriptor **>( binding->slot ) =
                    static_cast<SetTypeDescriptor *>( result );
                break;
            case TypeDescriptorInit_Bag:
                *static_cast<BagTypeDescriptor **>( binding->slot ) =
                    static_cast<BagTypeDescriptor *>( result );
                break;
            case TypeDescriptorInit_Base:
            default:
                *static_cast<TypeDescriptor **>( binding->slot ) = result;
                break;
        }
    }
    return result;
}

AggrTypeDescriptor * createAggregateDescriptor(
    const SchemaImageAggregateRecord & record, const char * description,
    Schema * schema ) {
    const PrimitiveType type =
        static_cast<PrimitiveType>( record.fundamentalType );
    AggrTypeDescriptor * result = 0;
    switch( type ) {
        case ARRAY_TYPE:
            result = new ArrayTypeDescriptor;
            break;
        case LIST_TYPE:
            result = new ListTypeDescriptor;
            break;
        case SET_TYPE:
            result = new SetTypeDescriptor;
            break;
        case BAG_TYPE:
            result = new BagTypeDescriptor;
            break;
        case sdaiAGGR:
        default:
            result = new AggrTypeDescriptor;
            break;
    }
    result->FundamentalType( type );
    result->Description( description );
    result->OriginatingSchema( schema );
    if( record.bound1Type == bound_constant ) {
        result->SetBound1( record.bound1 );
    } else if( record.bound1Type == bound_funcall ) {
        result->SetBound1FromExpressFuncall( "" );
    }
    if( record.bound2Type == bound_constant ) {
        result->SetBound2( record.bound2 );
    } else if( record.bound2Type == bound_funcall ) {
        result->SetBound2FromExpressFuncall( "" );
    }
    result->UniqueElements(
        record.uniqueElements ? LTrue : LFalse );
    ArrayTypeDescriptor * array = dynamic_cast<ArrayTypeDescriptor *>( result );
    if( array ) {
        array->OptionalElements(
            record.optionalElements ? LTrue : LFalse );
    }
    return result;
}

}

const char * SchemaLoadResult::Message() const {
    switch( error ) {
        case SchemaLoad_Ok:
            return "schema image loaded";
        case SchemaLoad_UnsupportedVersion:
            return "unsupported schema image version";
        case SchemaLoad_TruncatedImage:
            return "truncated schema image";
        case SchemaLoad_InvalidOffset:
            return "invalid schema image offset";
        case SchemaLoad_InvalidString:
            return "invalid schema image string";
        case SchemaLoad_InvalidReference:
            return "invalid schema descriptor reference";
        case SchemaLoad_CountMismatch:
            return "schema image count mismatch";
        case SchemaLoad_MissingBinding:
            return "missing generated schema binding";
        case SchemaLoad_ModuleFailure:
            return "unable to initialize schema module";
        default:
            return "unknown schema image error";
    }
}

SchemaLoadResult InitializeSchemaFromImage(
    Registry & registry, const SchemaModuleImage & image,
    const SchemaImageSchemaBinding * schemaBindings,
    size_t schemaBindingCount,
    const SchemaImageTypeBinding * typeBindings,
    size_t typeBindingCount, SchemaLoadContext & context ) {
    SchemaLoadResult result = loadError( SchemaLoad_Ok );
    if( image.version != SchemaModuleImageVersion_2 ) {
        return loadError( SchemaLoad_UnsupportedVersion );
    }
    if( image.byteSize < sizeof( SchemaModuleImage ) ) {
        return loadError( SchemaLoad_TruncatedImage );
    }
    if( schemaBindingCount != image.schemaCount || !schemaBindings ) {
        return loadError( SchemaLoad_CountMismatch );
    }
    if( typeBindingCount && typeBindingCount != image.typeCount ) {
        return loadError( SchemaLoad_CountMismatch );
    }
    if( typeBindingCount && !typeBindings ) {
        return loadError( SchemaLoad_MissingBinding );
    }

    const SchemaImageSchemaRecord * schemaRecords =
        imageRecords<SchemaImageSchemaRecord>(
            image, image.schemaOffset, image.schemaCount, result );
    const SchemaImageEntityRecord * entityRecords =
        imageRecords<SchemaImageEntityRecord>(
            image, image.entityOffset, image.entityCount, result );
    const SchemaImageTypeRecord * typeRecords =
        imageRecords<SchemaImageTypeRecord>(
            image, image.typeOffset, image.typeCount, result );
    const SchemaImageAttributeRecord * attributeRecords =
        imageRecords<SchemaImageAttributeRecord>(
            image, image.attributeOffset, image.attributeCount, result );
    const SchemaImageAggregateRecord * aggregateRecords =
        imageRecords<SchemaImageAggregateRecord>(
            image, image.aggregateOffset, image.aggregateCount, result );
    const SchemaImageDescriptorRef * references =
        imageRecords<SchemaImageDescriptorRef>(
            image, image.referenceOffset, image.referenceCount, result );
    const SchemaImageRuleRecord * rules = imageRecords<SchemaImageRuleRecord>(
        image, image.ruleOffset, image.ruleCount, result );
    const SchemaImageSchemaTextRecord * schemaTexts =
        imageRecords<SchemaImageSchemaTextRecord>(
            image, image.schemaTextOffset, image.schemaTextCount, result );
    const uint32_t * enumElements = imageRecords<uint32_t>(
        image, image.enumElementOffset, image.enumElementCount, result );
    const char * strings = imageStrings( image, result );
    if( !result.Succeeded() ) {
        return result;
    }

    uint32_t nextEntity = 0;
    uint32_t nextType = 0;
    for( uint32_t i = 0; i < image.schemaCount; ++i ) {
        const SchemaImageSchemaRecord & record = schemaRecords[i];
        if( !schemaBindings[i].schemaSlot || !schemaBindings[i].module ) {
            return loadError( SchemaLoad_MissingBinding, i );
        }
        if( !validString( strings, image.stringBytes, record.name ) ||
                record.firstEntity != nextEntity ||
                record.entityCount > image.entityCount - record.firstEntity ||
                record.firstType != nextType ||
                record.typeCount > image.typeCount - record.firstType ) {
            return loadError( SchemaLoad_InvalidOffset, i );
        }
        nextEntity += record.entityCount;
        nextType += record.typeCount;
    }
    if( nextEntity != image.entityCount || nextType != image.typeCount ) {
        return loadError( SchemaLoad_CountMismatch );
    }
    for( uint32_t i = 0; i < image.referenceCount; ++i ) {
        if( !validReference( references[i], schemaRecords,
                             image.schemaCount, image.aggregateCount ) ) {
            return loadError( SchemaLoad_InvalidReference, i );
        }
    }
    uint32_t nextAttribute = 0;
    for( uint32_t i = 0; i < image.entityCount; ++i ) {
        const SchemaImageEntityRecord & record = entityRecords[i];
        if( record.schema >= image.schemaCount ||
                i < schemaRecords[record.schema].firstEntity ||
                i >= schemaRecords[record.schema].firstEntity +
                    schemaRecords[record.schema].entityCount ||
                !validString( strings, image.stringBytes, record.name ) ||
                !validString( strings, image.stringBytes,
                              record.supertypeStatement ) ||
                record.firstSupertype > image.referenceCount ||
                record.supertypeCount >
                    image.referenceCount - record.firstSupertype ||
                record.firstAttribute != nextAttribute ||
                record.attributeCount >
                    image.attributeCount - record.firstAttribute ||
                record.firstWhereRule > image.ruleCount ||
                record.whereRuleCount >
                    image.ruleCount - record.firstWhereRule ||
                record.firstUniqueRule > image.ruleCount ||
                record.uniqueRuleCount >
                    image.ruleCount - record.firstUniqueRule ) {
            return loadError( SchemaLoad_InvalidOffset, i );
        }
        for( uint32_t j = 0; j < record.supertypeCount; ++j ) {
            if( references[record.firstSupertype + j].kind !=
                    SchemaImageRef_Entity ) {
                return loadError( SchemaLoad_InvalidReference, i );
            }
        }
        nextAttribute += record.attributeCount;
    }
    if( nextAttribute != image.attributeCount ) {
        return loadError( SchemaLoad_CountMismatch );
    }
    for( uint32_t i = 0; i < image.typeCount; ++i ) {
        const SchemaImageTypeRecord & record = typeRecords[i];
        if( typeBindingCount && !typeBindings[i].slot ) {
            return loadError( SchemaLoad_MissingBinding, i );
        }
        if( record.schema >= image.schemaCount ||
                i < schemaRecords[record.schema].firstType ||
                i >= schemaRecords[record.schema].firstType +
                    schemaRecords[record.schema].typeCount ||
                record.descriptorKind > TypeDescriptorInit_Bag ||
                !validString( strings, image.stringBytes, record.name ) ||
                !validString( strings, image.stringBytes,
                              record.description ) ||
                !validReference( record.referent, schemaRecords,
                                 image.schemaCount, image.aggregateCount ) ||
                ( record.aggregate != UINT32_MAX &&
                  record.aggregate >= image.aggregateCount ) ||
                record.firstSelectElement > image.referenceCount ||
                record.selectElementCount >
                    image.referenceCount - record.firstSelectElement ||
                record.firstWhereRule > image.ruleCount ||
                record.whereRuleCount >
                    image.ruleCount - record.firstWhereRule ||
                record.firstEnumElement > image.enumElementCount ||
                record.enumElementCount >
                    image.enumElementCount - record.firstEnumElement ) {
            return loadError( SchemaLoad_InvalidReference, i );
        }
    }
    for( uint32_t i = 0; i < image.attributeCount; ++i ) {
        const SchemaImageAttributeRecord & record = attributeRecords[i];
        if( record.attrType > AttrType_Redefining ||
                !validString( strings, image.stringBytes, record.name ) ||
                !validString( strings, image.stringBytes, record.initializer ) ||
                !validString( strings, image.stringBytes,
                              record.invertedAttribute ) ||
                !validString( strings, image.stringBytes,
                              record.invertedEntity ) ||
                !validReference( record.domain, schemaRecords,
                                 image.schemaCount, image.aggregateCount ) ) {
            return loadError( SchemaLoad_InvalidReference, i );
        }
    }
    for( uint32_t i = 0; i < image.aggregateCount; ++i ) {
        const SchemaImageAggregateRecord & record = aggregateRecords[i];
        if( record.schema >= image.schemaCount ||
                record.bound1Type > bound_funcall ||
                record.bound2Type > bound_funcall ||
                !validString( strings, image.stringBytes,
                              record.description ) ||
                !validString( strings, image.stringBytes,
                              record.bound1Text ) ||
                !validString( strings, image.stringBytes,
                              record.bound2Text ) ||
                !validReference( record.referent, schemaRecords,
                                 image.schemaCount, image.aggregateCount ) ) {
            return loadError( SchemaLoad_InvalidReference, i );
        }
    }
    for( uint32_t i = 0; i < image.ruleCount; ++i ) {
        if( !validString( strings, image.stringBytes, rules[i].text ) ) {
            return loadError( SchemaLoad_InvalidString, i );
        }
    }
    for( uint32_t i = 0; i < image.schemaTextCount; ++i ) {
        if( schemaTexts[i].schema >= image.schemaCount ||
                schemaTexts[i].kind > SchemaImageText_Procedure ||
                !validString( strings, image.stringBytes,
                              schemaTexts[i].name ) ||
                !validString( strings, image.stringBytes,
                              schemaTexts[i].text ) ) {
            return loadError( SchemaLoad_InvalidString, i );
        }
    }
    for( uint32_t i = 0; i < image.enumElementCount; ++i ) {
        if( !validString( strings, image.stringBytes, enumElements[i] ) ) {
            return loadError( SchemaLoad_InvalidString, i );
        }
    }

    std::vector<Schema *> schemas( image.schemaCount, 0 );
    for( uint32_t i = 0; i < image.schemaCount; ++i ) {
        schemas[i] = new Schema( strings + schemaRecords[i].name );
        schemas[i]->AssignModelContentsCreator(
            schemaBindings[i].modelContentsCreator );
        *schemaBindings[i].schemaSlot = schemas[i];
        registry.AddSchema( *schemas[i] );
        context.Begin( *schemas[i] );
    }

    std::vector<EntityDescriptor *> entities( image.entityCount, 0 );
    for( uint32_t i = 0; i < image.entityCount; ++i ) {
        const SchemaImageEntityRecord & record = entityRecords[i];
        Schema * schema = schemas[record.schema];
        entities[i] = new EntityDescriptor(
            strings + record.name, schema,
            record.flags & SchemaImageEntity_Abstract ? LTrue : LFalse,
            record.flags & SchemaImageEntity_ExternalMapping ? LTrue : LFalse,
            0 );
        schema->AddEntity( entities[i] );
        context.RecordEntity( *schema, entities[i] );
    }

    std::vector<TypeDescriptor *> types( image.typeCount, 0 );
    for( uint32_t i = 0; i < image.typeCount; ++i ) {
        const SchemaImageTypeRecord & record = typeRecords[i];
        const SchemaImageTypeBinding * binding =
            typeBindingCount ? typeBindings + i : 0;
        types[i] = createTypeDescriptor(
            record, strings + record.name, strings + record.description,
            schemas[record.schema], binding );
        context.RecordType( *schemas[record.schema], types[i] );
    }

    std::vector<AggrTypeDescriptor *> aggregates( image.aggregateCount, 0 );
    for( uint32_t i = 0; i < image.aggregateCount; ++i ) {
        const SchemaImageAggregateRecord & record = aggregateRecords[i];
        aggregates[i] = createAggregateDescriptor(
            record, strings + record.description, schemas[record.schema] );
        schemas[record.schema]->AddUnnamedType( aggregates[i] );
    }
    for( uint32_t i = 0; i < image.aggregateCount; ++i ) {
        const SchemaImageAggregateRecord & record = aggregateRecords[i];
        aggregates[i]->ReferentType( resolveReference(
            record.referent, schemaRecords, entities, types, aggregates ) );
        if( record.bound1Type == bound_funcall ) {
            aggregates[i]->SetBound1FromExpressFuncall(
                strings + record.bound1Text );
        }
        if( record.bound2Type == bound_funcall ) {
            aggregates[i]->SetBound2FromExpressFuncall(
                strings + record.bound2Text );
        }
    }

    for( uint32_t i = 0; i < image.typeCount; ++i ) {
        const SchemaImageTypeRecord & record = typeRecords[i];
        TypeDescriptor * type = types[i];
        type->ReferentType( resolveReference(
            record.referent, schemaRecords, entities, types, aggregates ) );
        SelectTypeDescriptor * select =
            dynamic_cast<SelectTypeDescriptor *>( type );
        if( select ) {
            for( uint32_t j = 0; j < record.selectElementCount; ++j ) {
                const TypeDescriptor * element = resolveReference(
                    references[record.firstSelectElement + j], schemaRecords,
                    entities, types, aggregates );
                select->Elements().AddNode(
                    const_cast<TypeDescriptor *>( element ) );
            }
        }
        AggrTypeDescriptor * aggregate =
            dynamic_cast<AggrTypeDescriptor *>( type );
        if( aggregate && record.aggregate != UINT32_MAX ) {
            const SchemaImageAggregateRecord & packed =
                aggregateRecords[record.aggregate];
            aggregate->ReferentType( resolveReference(
                packed.referent, schemaRecords, entities, types,
                aggregates ) );
            if( packed.bound1Type == bound_constant ) {
                aggregate->SetBound1( packed.bound1 );
            } else if( packed.bound1Type == bound_funcall ) {
                aggregate->SetBound1FromExpressFuncall(
                    strings + packed.bound1Text );
            }
            if( packed.bound2Type == bound_constant ) {
                aggregate->SetBound2( packed.bound2 );
            } else if( packed.bound2Type == bound_funcall ) {
                aggregate->SetBound2FromExpressFuncall(
                    strings + packed.bound2Text );
            }
            aggregate->UniqueElements(
                packed.uniqueElements ? LTrue : LFalse );
            ArrayTypeDescriptor * array =
                dynamic_cast<ArrayTypeDescriptor *>( aggregate );
            if( array ) {
                array->OptionalElements(
                    packed.optionalElements ? LTrue : LFalse );
            }
        }
        if( record.whereRuleCount ) {
            type->_where_rules = new Where_rule__list;
            for( uint32_t j = 0; j < record.whereRuleCount; ++j ) {
                type->_where_rules->Append( new Where_rule(
                    strings + rules[record.firstWhereRule + j].text ) );
            }
        }
        schemas[record.schema]->AddType( type );
        registry.AddType( *type );
    }

    for( uint32_t i = 0; i < image.entityCount; ++i ) {
        const SchemaImageEntityRecord & record = entityRecords[i];
        EntityDescriptor * entity = entities[i];
        Schema * schema = schemas[record.schema];
        if( record.supertypeStatement ) {
            entity->AddSupertype_Stmt( strings + record.supertypeStatement );
        }
        for( uint32_t j = 0; j < record.supertypeCount; ++j ) {
            EntityDescriptor * supertype = const_cast<EntityDescriptor *>(
                dynamic_cast<const EntityDescriptor *>( resolveReference(
                    references[record.firstSupertype + j], schemaRecords,
                    entities, types, aggregates ) ) );
            entity->AddSupertype( supertype );
            supertype->AddSubtype( entity );
        }
        bool hasInverse = false;
        for( uint32_t j = 0; j < record.attributeCount; ++j ) {
            const SchemaImageAttributeRecord & attribute =
                attributeRecords[record.firstAttribute + j];
            const TypeDescriptor * domain = resolveReference(
                attribute.domain, schemaRecords, entities, types,
                aggregates );
            AttrDescriptor * descriptor = 0;
            if( attribute.attrType == AttrType_Inverse ) {
                Inverse_attribute * inverse = new Inverse_attribute(
                    strings + attribute.name,
                    const_cast<TypeDescriptor *>( domain ),
                    attribute.optional ? LTrue : LFalse,
                    attribute.unique ? LTrue : LFalse, *entity,
                    strings + attribute.invertedAttribute );
                inverse->inverted_entity_id_(
                    strings + attribute.invertedEntity );
                entity->AddInverseAttr( inverse );
                descriptor = inverse;
                hasInverse = true;
            } else if( attribute.attrType == AttrType_Deriving ) {
                Derived_attribute * derived = new Derived_attribute(
                    strings + attribute.name, domain,
                    attribute.optional ? LTrue : LFalse,
                    attribute.unique ? LTrue : LFalse,
                    static_cast<AttrType_Enum>( attribute.attrType ),
                    *entity );
                if( attribute.initializer ) {
                    derived->initializer_( strings + attribute.initializer );
                }
                entity->AddExplicitAttr( derived );
                descriptor = derived;
            } else {
                descriptor = new AttrDescriptor(
                    strings + attribute.name, domain,
                    attribute.optional ? LTrue : LFalse,
                    attribute.unique ? LTrue : LFalse,
                    static_cast<AttrType_Enum>( attribute.attrType ),
                    *entity );
                entity->AddExplicitAttr( descriptor );
            }
            context.RecordAttribute( *schema, descriptor );
        }
        if( record.whereRuleCount ) {
            entity->_where_rules = new Where_rule__list;
            for( uint32_t j = 0; j < record.whereRuleCount; ++j ) {
                entity->_where_rules->Append( new Where_rule(
                    strings + rules[record.firstWhereRule + j].text ) );
            }
        }
        if( record.uniqueRuleCount ) {
            entity->_uniqueness_rules = new Uniqueness_rule__set;
            for( uint32_t j = 0; j < record.uniqueRuleCount; ++j ) {
                entity->_uniqueness_rules->Append( new Uniqueness_rule(
                    strings + rules[record.firstUniqueRule + j].text ) );
            }
        }
        registry.AddEntity( *entity );
        if( hasInverse ) {
            schema->AddEntityWInverse( entity );
        }
    }

    for( uint32_t i = 0; i < image.schemaTextCount; ++i ) {
        const SchemaImageSchemaTextRecord & text = schemaTexts[i];
        Schema * schema = schemas[text.schema];
        switch( text.kind ) {
            case SchemaImageText_GlobalRule:
                schema->AddGlobal_rule( new Global_rule(
                    strings + text.name, schema, strings + text.text ) );
                break;
            case SchemaImageText_Function:
                schema->AddFunction( strings + text.text );
                break;
            case SchemaImageText_Procedure:
                schema->AddProcedure( strings + text.text );
                break;
            default:
                break;
        }
    }

    for( uint32_t i = 0; i < image.schemaCount; ++i ) {
        if( !schemaBindings[i].module->InitializeFromContext(
                *schemas[i], context, image.fingerprint ) ) {
            return loadError( SchemaLoad_ModuleFailure, i );
        }
    }
    return result;
}

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

ComplexCollect * InitializePackedComplexSupport(
    const PackedComplexImage & image, SchemaLoadResult * loadResult ) {
    SchemaLoadResult result = loadError( SchemaLoad_Ok );
    if( image.version != PackedComplexImageVersion_1 ) {
        result = loadError( SchemaLoad_UnsupportedVersion );
    } else if( image.byteSize < sizeof( PackedComplexImage ) ) {
        result = loadError( SchemaLoad_TruncatedImage );
    }

    const PackedComplexNode * nodes = 0;
    const PackedComplexList * lists = 0;
    const char * strings = 0;
    if( result.Succeeded() ) {
        nodes = packedComplexRecords<PackedComplexNode>(
            image, image.nodeOffset, image.nodeCount, result );
        lists = packedComplexRecords<PackedComplexList>(
            image, image.listOffset, image.listCount, result );
        if( image.stringOffset > image.byteSize ||
                image.stringBytes > image.byteSize - image.stringOffset ) {
            result = loadError( SchemaLoad_InvalidOffset );
        } else {
            strings = reinterpret_cast<const char *>( &image ) +
                image.stringOffset;
        }
    }

    const uint32_t noNode = UINT32_MAX;
    for( uint32_t i = 0; result.Succeeded() && i < image.nodeCount; ++i ) {
        const PackedComplexNode & node = nodes[i];
        if( node.kind > ComplexNodeInit_AndOr ||
                ( node.firstChild != noNode &&
                  node.firstChild >= image.nodeCount ) ||
                ( node.nextSibling != noNode &&
                  node.nextSibling >= image.nodeCount ) ||
                ( node.kind == ComplexNodeInit_Simple &&
                    ( node.firstChild != noNode ||
                      !validString( strings, image.stringBytes, node.name ) ) ) ) {
            result = loadError( SchemaLoad_InvalidReference, i );
        }
    }
    std::vector<unsigned char> state( image.nodeCount, 0 );
    for( uint32_t i = 0; result.Succeeded() && i < image.listCount; ++i ) {
        if( lists[i].rootNode >= image.nodeCount ||
                nodes[lists[i].rootNode].kind != ComplexNodeInit_And ||
                !validateComplexNode( lists[i].rootNode, nodes,
                                      image.nodeCount, state ) ) {
            result = loadError( SchemaLoad_InvalidReference, i );
        }
    }
    for( uint32_t i = 0; result.Succeeded() && i < image.nodeCount; ++i ) {
        if( state[i] != 2 ) {
            result = loadError( SchemaLoad_InvalidReference, i );
        }
    }

    if( loadResult ) {
        *loadResult = result;
    }
    if( !result.Succeeded() || !image.listCount ) {
        return 0;
    }

    std::vector<ComplexNodeInitRecord> unpackedNodes( image.nodeCount );
    for( uint32_t i = 0; i < image.nodeCount; ++i ) {
        unpackedNodes[i].kind =
            static_cast<ComplexNodeInitKind>( nodes[i].kind );
        unpackedNodes[i].name = nodes[i].kind == ComplexNodeInit_Simple ?
            strings + nodes[i].name : 0;
        unpackedNodes[i].firstChild = nodes[i].firstChild == noNode ?
            static_cast<size_t>( -1 ) : nodes[i].firstChild;
        unpackedNodes[i].nextSibling = nodes[i].nextSibling == noNode ?
            static_cast<size_t>( -1 ) : nodes[i].nextSibling;
    }
    std::vector<ComplexListInitRecord> unpackedLists( image.listCount );
    for( uint32_t i = 0; i < image.listCount; ++i ) {
        unpackedLists[i].rootNode = lists[i].rootNode;
    }
    return InitializeComplexSupport(
        &unpackedNodes[0], unpackedNodes.size(),
        &unpackedLists[0], unpackedLists.size() );
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
