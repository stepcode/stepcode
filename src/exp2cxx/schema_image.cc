#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

extern "C" {
#include "schema_image.h"
#include "classes.h"
#include "rules.h"
char * non_unique_types_string( const Type type );
}

namespace {

const uint32_t noIndex = UINT32_MAX;

struct StringPool {
    std::string data;
    std::unordered_map<std::string, uint32_t> offsets;

    StringPool() : data( 1, '\0' ) {
        offsets[""] = 0;
    }

    uint32_t Add( const char * value ) {
        const std::string text = value ? value : "";
        std::unordered_map<std::string, uint32_t>::const_iterator found =
            offsets.find( text );
        if( found != offsets.end() ) {
            return found->second;
        }
        const uint32_t offset = static_cast<uint32_t>( data.size() );
        data.append( text );
        data.push_back( '\0' );
        offsets[text] = offset;
        return offset;
    }
};

struct PackedRef {
    uint32_t kind;
    uint32_t schema;
    uint32_t index;
    std::string indexToken;

    PackedRef() : kind( 0 ), schema( 0 ), index( 0 ) {
    }
};

struct PackedSchema {
    uint32_t name;
    uint32_t firstEntity;
    uint32_t entityCount;
    uint32_t firstType;
    uint32_t typeCount;
};

struct PackedEntity {
    uint32_t schema;
    uint32_t name;
    uint32_t flags;
    uint32_t supertypeStatement;
    uint32_t firstSupertype;
    uint32_t supertypeCount;
    uint32_t firstAttribute;
    uint32_t attributeCount;
    uint32_t firstWhereRule;
    uint32_t whereRuleCount;
    uint32_t firstUniqueRule;
    uint32_t uniqueRuleCount;
};

struct PackedType {
    uint32_t schema;
    uint32_t name;
    uint32_t description;
    std::string fundamentalType;
    uint32_t descriptorKind;
    std::string uniqueElements;
    PackedRef referent;
    uint32_t aggregate;
    uint32_t firstSelectElement;
    uint32_t selectElementCount;
    uint32_t firstWhereRule;
    uint32_t whereRuleCount;
    uint32_t firstEnumElement;
    uint32_t enumElementCount;
};

struct PackedAttribute {
    uint32_t name;
    PackedRef domain;
    uint32_t optional;
    uint32_t unique;
    std::string attrType;
    uint32_t initializer;
    uint32_t invertedAttribute;
    uint32_t invertedEntity;
};

struct PackedAggregate {
    uint32_t schema;
    uint32_t description;
    std::string fundamentalType;
    PackedRef referent;
    std::string bound1Type;
    int32_t bound1;
    uint32_t bound1Text;
    std::string bound2Type;
    int32_t bound2;
    uint32_t bound2Text;
    uint32_t optionalElements;
    uint32_t uniqueElements;
};

struct PackedSchemaText {
    uint32_t schema;
    std::string kind;
    uint32_t name;
    uint32_t text;
};

struct RegisteredSchema {
    Schema schema;
    std::vector<Entity> entities;
    std::vector<Type> types;
};

struct RefLocation {
    uint32_t schema;
    uint32_t index;
};

class ImageBuilder {
    std::vector<RegisteredSchema> _registered;
    std::unordered_map<const void *, RefLocation> _entityLocations;
    std::unordered_map<const void *, RefLocation> _typeLocations;
    std::unordered_map<const void *, uint32_t> _schemaLocations;
    std::unordered_map<const void *, bool> _externalMappings;
    std::unordered_map<const void *, uint32_t> _aggregateLocations;

    StringPool _strings;
    std::vector<PackedSchema> _schemas;
    std::vector<PackedEntity> _entities;
    std::vector<PackedType> _types;
    std::vector<PackedAttribute> _attributes;
    std::vector<PackedAggregate> _aggregates;
    std::vector<PackedRef> _references;
    std::vector<uint32_t> _rules;
    std::vector<PackedSchemaText> _schemaTexts;
    std::vector<uint32_t> _enumElements;

    static std::string primitiveToken( const Type type, int reportRef ) {
        return FundamentalType( type, reportRef );
    }

    static std::string descriptorKind( const Type type ) {
        if( TYPEis_select( type ) ) {
            return "TypeDescriptorInit_Select";
        }
        switch( TYPEget_body( type )->type ) {
            case boolean_:
            case logical_:
            case enumeration_:
                return "TypeDescriptorInit_Enum";
            case aggregate_:
                return "TypeDescriptorInit_Aggregate";
            case array_:
                return "TypeDescriptorInit_Array";
            case list_:
                return "TypeDescriptorInit_List";
            case set_:
                return "TypeDescriptorInit_Set";
            case bag_:
                return "TypeDescriptorInit_Bag";
            default:
                return "TypeDescriptorInit_Base";
        }
    }

    static uint32_t descriptorKindValue( const Type type ) {
        const std::string kind = descriptorKind( type );
        if( kind == "TypeDescriptorInit_Enum" ) return 1;
        if( kind == "TypeDescriptorInit_Select" ) return 2;
        if( kind == "TypeDescriptorInit_Aggregate" ) return 3;
        if( kind == "TypeDescriptorInit_Array" ) return 4;
        if( kind == "TypeDescriptorInit_List" ) return 5;
        if( kind == "TypeDescriptorInit_Set" ) return 6;
        if( kind == "TypeDescriptorInit_Bag" ) return 7;
        return 0;
    }

    static bool builtinType( const Type type ) {
        switch( TYPEget_body( type )->type ) {
            case integer_:
            case real_:
            case string_:
            case binary_:
            case boolean_:
            case number_:
            case logical_:
                return !TYPEget_name( type );
            default:
                return false;
        }
    }

    uint32_t schemaFor( Schema schema ) const {
        std::unordered_map<const void *, uint32_t>::const_iterator found =
            _schemaLocations.find( schema );
        assert( found != _schemaLocations.end() );
        return found->second;
    }

    PackedRef entityReference( Entity entity ) const {
        PackedRef result;
        result.kind = 1;
        std::unordered_map<const void *, RefLocation>::const_iterator found =
            _entityLocations.find( entity );
        assert( found != _entityLocations.end() );
        result.schema = found->second.schema;
        result.index = found->second.index;
        return result;
    }

    PackedRef builtinReference( const Type type ) const {
        PackedRef result;
        result.kind = 3;
        result.indexToken = primitiveToken( type, 0 );
        return result;
    }

    void bound( Expression expression, std::string & kind, int32_t & value,
                uint32_t & text ) {
        kind = "bound_unset";
        value = 0;
        text = 0;
        if( !expression ) {
            return;
        }
        if( expression->symbol.resolved && expression->type != Type_Funcall ) {
            kind = "bound_constant";
            value = expression->u.integer;
            return;
        }
        char * expressionText = EXPRto_string( expression );
        kind = "bound_funcall";
        text = _strings.Add( expressionText );
        free( expressionText );
    }

    uint32_t addAggregate( Type type, Schema schema ) {
        std::unordered_map<const void *, uint32_t>::const_iterator found =
            _aggregateLocations.find( type );
        if( found != _aggregateLocations.end() ) {
            return found->second;
        }
        const uint32_t index = static_cast<uint32_t>( _aggregates.size() );
        _aggregateLocations[type] = index;
        PackedAggregate record = {};
        record.schema = schemaFor( schema );
        record.description = _strings.Add( TypeDescription( type ) );
        record.fundamentalType = primitiveToken( type, 1 );
        record.bound1Type = "bound_unset";
        record.bound2Type = "bound_unset";
        _aggregates.push_back( record );

        Type base = TYPEget_body( type )->base;
        _aggregates[index].referent = typeReference( base, schema );
        bound( TYPEget_body( type )->lower,
               _aggregates[index].bound1Type,
               _aggregates[index].bound1,
               _aggregates[index].bound1Text );
        bound( TYPEget_body( type )->upper,
               _aggregates[index].bound2Type,
               _aggregates[index].bound2,
               _aggregates[index].bound2Text );
        _aggregates[index].optionalElements =
            TYPEget_body( type )->flags.optional ? 1 : 0;
        _aggregates[index].uniqueElements =
            TYPEget_body( type )->flags.unique ? 1 : 0;
        return index;
    }

    PackedRef typeReference( Type type, Schema schema ) {
        PackedRef result;
        if( !type ) {
            return result;
        }
        if( TYPEis_entity( type ) ) {
            return entityReference( ENT_TYPEget_entity( type ) );
        }
        std::unordered_map<const void *, RefLocation>::const_iterator named =
            _typeLocations.find( type );
        if( named != _typeLocations.end() ) {
            result.kind = 2;
            result.schema = named->second.schema;
            result.index = named->second.index;
            return result;
        }
        if( builtinType( type ) ) {
            return builtinReference( type );
        }
        if( TYPEget_name( type ) && TYPEget_head( type ) ) {
            return typeReference( TYPEget_head( type ), schema );
        }
        switch( TYPEget_body( type )->type ) {
            case aggregate_:
            case array_:
            case list_:
            case set_:
            case bag_:
                result.kind = 4;
                result.index = addAggregate( type, schema );
                return result;
            default:
                break;
        }
        if( TYPEget_body( type )->base ) {
            return typeReference( TYPEget_body( type )->base, schema );
        }
        assert( false && "unresolved packed schema type reference" );
        return result;
    }

    uint32_t addWhereRules( Linked_List wheres ) {
        const uint32_t first = static_cast<uint32_t>( _rules.size() );
        if( !wheres ) {
            return first;
        }
        LISTdo( wheres, where, Where ) {
            char * expression = EXPRto_string( where->expr );
            std::string text;
            if( where->label ) {
                text.append( where->label->name );
                text.append( ": " );
            }
            text.push_back( '(' );
            text.append( expression );
            text.append( ");\n" );
            free( expression );
            _rules.push_back( _strings.Add( text.c_str() ) );
        } LISTod
        return first;
    }

    uint32_t addUniqueRules( Entity entity ) {
        const uint32_t first = static_cast<uint32_t>( _rules.size() );
        Linked_List unique = ENTITYget_uniqueness_list( entity );
        if( !unique ) {
            return first;
        }
        LISTdo( unique, list, Linked_List ) {
            std::string text;
            int item = 0;
            LISTdo_n( list, expression, Expression, b ) {
                ++item;
                if( item == 1 ) {
                    if( expression ) {
                        text.append( StrToUpper(
                            ( ( Symbol * )expression )->name ) );
                        text.append( " : " );
                    }
                } else {
                    char * expressionText = EXPRto_string( expression );
                    if( item > 2 ) {
                        text.append( ", " );
                    }
                    text.append( expressionText );
                    free( expressionText );
                }
            } LISTod
            _rules.push_back( _strings.Add( text.c_str() ) );
        } LISTod
        return first;
    }

    std::string supertypeStatement( Entity entity ) const {
        if( ENTITYget_abstract( entity ) ) {
            if( !entity->u.entity->subtype_expression ) {
                return "ABSTRACT SUPERTYPE";
            }
            char * expression =
                SUBTYPEto_string( entity->u.entity->subtype_expression );
            std::string result = "ABSTRACT SUPERTYPE OF ( ";
            result.append( expression );
            result.push_back( ')' );
            free( expression );
            return result;
        }
        if( entity->u.entity->subtype_expression ) {
            char * expression =
                SUBTYPEto_string( entity->u.entity->subtype_expression );
            std::string result = "SUPERTYPE OF ( ";
            result.append( expression );
            result.push_back( ')' );
            free( expression );
            return result;
        }
        return "";
    }

    void buildEntity( Entity entity, Schema schema ) {
        PackedEntity record = {};
        record.schema = schemaFor( schema );
        record.name = _strings.Add( PrettyTmpName( ENTITYget_name( entity ) ) );
        record.flags = ENTITYget_abstract( entity ) ? 1u : 0u;
        if( _externalMappings[entity] ) {
            record.flags |= 2u;
        }
        const std::string statement = supertypeStatement( entity );
        record.supertypeStatement = _strings.Add( statement.c_str() );
        record.firstSupertype = static_cast<uint32_t>( _references.size() );
        LISTdo( ENTITYget_supertypes( entity ), supertype, Entity ) {
            _references.push_back( entityReference( supertype ) );
        } LISTod
        record.supertypeCount = static_cast<uint32_t>( _references.size() ) -
            record.firstSupertype;

        record.firstAttribute = static_cast<uint32_t>( _attributes.size() );
        LISTdo( ENTITYget_attributes( entity ), variable, Variable ) {
            PackedAttribute attribute = {};
            char name[BUFSIZ + 1];
            generate_dict_attr_name( variable, name );
            attribute.name = _strings.Add( name );
            attribute.domain = typeReference( variable->type, schema );
            attribute.optional = VARget_optional( variable ) ? 1 : 0;
            attribute.unique = VARget_unique( variable ) ? 1 : 0;
            if( VARget_inverse( variable ) ) {
                attribute.attrType = "AttrType_Inverse";
                attribute.invertedAttribute = _strings.Add(
                    variable->inverse_attribute->name->symbol.name );
                const char * inverseEntity = 0;
                if( variable->type->symbol.name ) {
                    inverseEntity = variable->type->symbol.name;
                } else if( TYPEget_body( variable->type )->type == entity_ ) {
                    inverseEntity =
                        TYPEget_body( variable->type )->entity->symbol.name;
                } else {
                    inverseEntity =
                        TYPEget_body( variable->type )->base->symbol.name;
                }
                attribute.invertedEntity = _strings.Add( inverseEntity );
            } else if( VARis_derived( variable ) ) {
                attribute.attrType = "AttrType_Deriving";
                if( variable->initializer ) {
                    char * initializer = EXPRto_string( variable->initializer );
                    attribute.initializer = _strings.Add( initializer );
                    free( initializer );
                }
            } else if( VARis_type_shifter( variable ) ) {
                attribute.attrType = "AttrType_Redefining";
            } else {
                attribute.attrType = "AttrType_Explicit";
            }
            _attributes.push_back( attribute );
        } LISTod
        record.attributeCount = static_cast<uint32_t>( _attributes.size() ) -
            record.firstAttribute;
        record.firstWhereRule = addWhereRules( TYPEget_where( entity ) );
        record.whereRuleCount = static_cast<uint32_t>( _rules.size() ) -
            record.firstWhereRule;
        record.firstUniqueRule = addUniqueRules( entity );
        record.uniqueRuleCount = static_cast<uint32_t>( _rules.size() ) -
            record.firstUniqueRule;
        _entities.push_back( record );
    }

    void addEnumElements( Type type, PackedType & record ) {
        record.firstEnumElement =
            static_cast<uint32_t>( _enumElements.size() );
        if( TYPEget_body( type )->type == enumeration_ &&
                ENUM_TYPEget_items( type ) ) {
            DictionaryEntry entry;
            DICTdo_type_init( ENUM_TYPEget_items( type ), &entry, OBJ_ENUM );
            Expression expression;
            while( ( expression = ( Expression )DICTdo( &entry ) ) != 0 ) {
                _enumElements.push_back(
                    _strings.Add( StrToUpper( EXPget_name( expression ) ) ) );
            }
        }
        record.enumElementCount =
            static_cast<uint32_t>( _enumElements.size() ) -
            record.firstEnumElement;
    }

    void buildType( Type type, Schema schema ) {
        PackedType record = {};
        record.schema = schemaFor( schema );
        record.name = _strings.Add( PrettyTmpName( TYPEget_name( type ) ) );
        record.description = _strings.Add( TypeDescription( type ) );
        record.fundamentalType = primitiveToken( type, 1 );
        record.descriptorKind = descriptorKindValue( type );
        record.uniqueElements = "0";
        record.aggregate = noIndex;
        if( TYPEis_select( type ) ) {
            char * nonUnique = non_unique_types_string( type );
            record.uniqueElements = "~";
            record.uniqueElements.append( nonUnique );
            free( nonUnique );
        } else if( TYPEget_head( type ) ) {
            record.referent = typeReference( TYPEget_head( type ), schema );
        } else if( TYPEget_body( type )->base ) {
            record.referent = typeReference(
                TYPEget_body( type )->base, schema );
        }
        switch( TYPEget_body( type )->type ) {
            case aggregate_:
            case array_:
            case list_:
            case set_:
            case bag_:
                record.aggregate = addAggregate( type, schema );
                record.referent = _aggregates[record.aggregate].referent;
                break;
            default:
                break;
        }
        record.firstSelectElement =
            static_cast<uint32_t>( _references.size() );
        if( TYPEis_select( type ) ) {
            LISTdo( SEL_TYPEget_items( type ), element, Type ) {
                _references.push_back( typeReference( element, schema ) );
            } LISTod
        }
        record.selectElementCount =
            static_cast<uint32_t>( _references.size() ) -
            record.firstSelectElement;
        record.firstWhereRule = addWhereRules( TYPEget_where( type ) );
        record.whereRuleCount = static_cast<uint32_t>( _rules.size() ) -
            record.firstWhereRule;
        addEnumElements( type, record );
        _types.push_back( record );
    }

    void buildSchemaTexts( Schema schema, uint32_t schemaIndex ) {
        DictionaryEntry entry;
        DICTdo_type_init( schema->symbol_table, &entry, OBJ_RULE );
        Rule rule;
        while( ( rule = ( Rule )DICTdo( &entry ) ) != 0 ) {
            char * text = RULEto_string( rule );
            PackedSchemaText record = {
                schemaIndex, "SchemaImageText_GlobalRule",
                _strings.Add( rule->symbol.name ), _strings.Add( text )
            };
            free( text );
            _schemaTexts.push_back( record );
        }
        DICTdo_type_init( schema->symbol_table, &entry, OBJ_FUNCTION );
        Function function;
        while( ( function = ( Function )DICTdo( &entry ) ) != 0 ) {
            char * text = FUNCto_string( function );
            PackedSchemaText record = {
                schemaIndex, "SchemaImageText_Function", 0,
                _strings.Add( text )
            };
            free( text );
            _schemaTexts.push_back( record );
        }
        DICTdo_type_init( schema->symbol_table, &entry, OBJ_PROCEDURE );
        Procedure procedure;
        while( ( procedure = ( Procedure )DICTdo( &entry ) ) != 0 ) {
            char * text = PROCto_string( procedure );
            PackedSchemaText record = {
                schemaIndex, "SchemaImageText_Procedure", 0,
                _strings.Add( text )
            };
            free( text );
            _schemaTexts.push_back( record );
        }
    }

    static void writeRef( FILE * output, const PackedRef & reference ) {
        fprintf( output, "{ %u, %u, ", reference.kind, reference.schema );
        if( reference.indexToken.empty() ) {
            fprintf( output, "%u", reference.index );
        } else {
            fprintf( output, "%s", reference.indexToken.c_str() );
        }
        fprintf( output, " }" );
    }

    static size_t arraySize( size_t size ) {
        return size ? size : 1;
    }

    static uint64_t fingerprint( const std::string & data ) {
        uint64_t value = UINT64_C( 14695981039346656037 );
        for( size_t i = 0; i < data.size(); ++i ) {
            value ^= static_cast<unsigned char>( data[i] );
            value *= UINT64_C( 1099511628211 );
        }
        return value;
    }

    static void writeStringPool( FILE * output, const std::string & data ) {
        fprintf( output, "    \"" );
        size_t column = 5;
        for( size_t i = 0; i < data.size(); ++i ) {
            const unsigned char value =
                static_cast<unsigned char>( data[i] );
            char escaped[8];
            if( value == '"' || value == '\\' ) {
                snprintf( escaped, sizeof( escaped ), "\\%c", value );
            } else if( value >= 32 && value < 127 ) {
                escaped[0] = static_cast<char>( value );
                escaped[1] = '\0';
            } else {
                snprintf( escaped, sizeof( escaped ), "\\%03o", value );
            }
            const size_t length = strlen( escaped );
            if( column + length > 76 ) {
                fprintf( output, "\"\n    \"" );
                column = 5;
            }
            fprintf( output, "%s", escaped );
            column += length;
        }
        fprintf( output, "\"\n" );
    }

public:
    void Reset() {
        _registered.clear();
        _entityLocations.clear();
        _typeLocations.clear();
        _schemaLocations.clear();
        _externalMappings.clear();
        _aggregateLocations.clear();
        _strings.data.assign( 1, '\0' );
        _strings.offsets.clear();
        _strings.offsets[""] = 0;
        _schemas.clear();
        _entities.clear();
        _types.clear();
        _attributes.clear();
        _aggregates.clear();
        _references.clear();
        _rules.clear();
        _schemaTexts.clear();
        _enumElements.clear();
    }

    void RegisterSchema( Schema schema, Linked_List entities ) {
        if( _schemaLocations.find( schema ) != _schemaLocations.end() ) {
            return;
        }
        RegisteredSchema registered;
        registered.schema = schema;
        const uint32_t schemaIndex =
            static_cast<uint32_t>( _registered.size() );
        _schemaLocations[schema] = schemaIndex;
        LISTdo( entities, entity, Entity ) {
            RefLocation location = {
                schemaIndex,
                static_cast<uint32_t>( registered.entities.size() )
            };
            _entityLocations[entity] = location;
            registered.entities.push_back( entity );
        } LISTod
        DictionaryEntry entry;
        SCOPEdo_types( schema, type, entry ) {
            RefLocation location = {
                schemaIndex,
                static_cast<uint32_t>( registered.types.size() )
            };
            _typeLocations[type] = location;
            registered.types.push_back( type );
        } SCOPEod
        _registered.push_back( registered );
    }

    void SetExternalMapping( Entity entity, bool externalMapping ) {
        _externalMappings[entity] = externalMapping;
    }

    void Build() {
        uint32_t firstEntity = 0;
        uint32_t firstType = 0;
        for( uint32_t i = 0; i < _registered.size(); ++i ) {
            PackedSchema schema = {
                _strings.Add( PrettyTmpName(
                    SCHEMAget_name( _registered[i].schema ) ) ),
                firstEntity,
                static_cast<uint32_t>( _registered[i].entities.size() ),
                firstType,
                static_cast<uint32_t>( _registered[i].types.size() )
            };
            _schemas.push_back( schema );
            firstEntity += schema.entityCount;
            firstType += schema.typeCount;
        }
        for( uint32_t i = 0; i < _registered.size(); ++i ) {
            for( size_t j = 0; j < _registered[i].entities.size(); ++j ) {
                buildEntity( _registered[i].entities[j],
                             _registered[i].schema );
            }
            for( size_t j = 0; j < _registered[i].types.size(); ++j ) {
                buildType( _registered[i].types[j], _registered[i].schema );
            }
            buildSchemaTexts( _registered[i].schema, i );
        }
    }

    void Write( FILE * output ) {
        Build();
        fprintf( output,
            "struct GeneratedSchemaImage {\n"
            "    SchemaModuleImage header;\n"
            "    SchemaImageSchemaRecord schemas[%zu];\n"
            "    SchemaImageEntityRecord entities[%zu];\n"
            "    SchemaImageTypeRecord types[%zu];\n"
            "    SchemaImageAttributeRecord attributes[%zu];\n"
            "    SchemaImageAggregateRecord aggregates[%zu];\n"
            "    SchemaImageDescriptorRef references[%zu];\n"
            "    SchemaImageRuleRecord rules[%zu];\n"
            "    SchemaImageSchemaTextRecord schemaTexts[%zu];\n"
            "    uint32_t enumElements[%zu];\n"
            "    char strings[%zu];\n"
            "};\n\n",
            arraySize( _schemas.size() ), arraySize( _entities.size() ),
            arraySize( _types.size() ), arraySize( _attributes.size() ),
            arraySize( _aggregates.size() ), arraySize( _references.size() ),
            arraySize( _rules.size() ), arraySize( _schemaTexts.size() ),
            arraySize( _enumElements.size() ), _strings.data.size() + 1 );
        fprintf( output,
            "const GeneratedSchemaImage generatedSchemaImage = {\n"
            "    { SchemaModuleImageVersion_2, %zu, %zu, %zu,\n"
            "      sizeof(GeneratedSchemaImage), "
            "SchemaModuleImage_FullMetadata, %zu, %zu,\n"
            "      offsetof(GeneratedSchemaImage, schemas),\n"
            "      offsetof(GeneratedSchemaImage, entities),\n"
            "      offsetof(GeneratedSchemaImage, types),\n"
            "      offsetof(GeneratedSchemaImage, attributes),\n"
            "      %zu, offsetof(GeneratedSchemaImage, aggregates),\n"
            "      %zu, offsetof(GeneratedSchemaImage, references),\n"
            "      %zu, offsetof(GeneratedSchemaImage, rules),\n"
            "      %zu, offsetof(GeneratedSchemaImage, schemaTexts),\n"
            "      %zu, offsetof(GeneratedSchemaImage, enumElements),\n"
            "      offsetof(GeneratedSchemaImage, strings), 0,\n"
            "      UINT64_C(%" PRIu64 ") },\n",
            _entities.size(), _types.size(), _attributes.size(),
            _schemas.size(), _strings.data.size(), _aggregates.size(),
            _references.size(), _rules.size(), _schemaTexts.size(),
            _enumElements.size(), fingerprint( _strings.data ) );

        fprintf( output, "    {\n" );
        for( size_t i = 0; i < _schemas.size(); ++i ) {
            const PackedSchema & record = _schemas[i];
            fprintf( output, "        { %u, %u, %u, %u, %u },\n",
                record.name, record.firstEntity, record.entityCount,
                record.firstType, record.typeCount );
        }
        if( _schemas.empty() ) fprintf( output, "        { 0, 0, 0, 0, 0 },\n" );
        fprintf( output, "    },\n    {\n" );
        for( size_t i = 0; i < _entities.size(); ++i ) {
            const PackedEntity & record = _entities[i];
            fprintf( output,
                "        { %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u },\n",
                record.schema, record.name, record.flags,
                record.supertypeStatement, record.firstSupertype,
                record.supertypeCount, record.firstAttribute,
                record.attributeCount, record.firstWhereRule,
                record.whereRuleCount, record.firstUniqueRule,
                record.uniqueRuleCount );
        }
        if( _entities.empty() ) fprintf( output, "        {},\n" );
        fprintf( output, "    },\n    {\n" );
        for( size_t i = 0; i < _types.size(); ++i ) {
            const PackedType & record = _types[i];
            fprintf( output, "        { %u, %u, %u, %s, %u, %s, ",
                record.schema, record.name, record.description,
                record.fundamentalType.c_str(), record.descriptorKind,
                record.uniqueElements.c_str() );
            writeRef( output, record.referent );
            fprintf( output, ", %u, %u, %u, %u, %u, %u, %u },\n",
                record.aggregate, record.firstSelectElement,
                record.selectElementCount, record.firstWhereRule,
                record.whereRuleCount, record.firstEnumElement,
                record.enumElementCount );
        }
        if( _types.empty() ) fprintf( output, "        {},\n" );
        fprintf( output, "    },\n    {\n" );
        for( size_t i = 0; i < _attributes.size(); ++i ) {
            const PackedAttribute & record = _attributes[i];
            fprintf( output, "        { %u, ", record.name );
            writeRef( output, record.domain );
            fprintf( output, ", %u, %u, %s, %u, %u, %u },\n",
                record.optional, record.unique, record.attrType.c_str(),
                record.initializer, record.invertedAttribute,
                record.invertedEntity );
        }
        if( _attributes.empty() ) fprintf( output, "        {},\n" );
        fprintf( output, "    },\n    {\n" );
        for( size_t i = 0; i < _aggregates.size(); ++i ) {
            const PackedAggregate & record = _aggregates[i];
            fprintf( output, "        { %u, %u, %s, ", record.schema,
                record.description, record.fundamentalType.c_str() );
            writeRef( output, record.referent );
            fprintf( output, ", %s, %d, %u, %s, %d, %u, %u, %u },\n",
                record.bound1Type.c_str(), record.bound1, record.bound1Text,
                record.bound2Type.c_str(), record.bound2, record.bound2Text,
                record.optionalElements, record.uniqueElements );
        }
        if( _aggregates.empty() ) fprintf( output, "        {},\n" );
        fprintf( output, "    },\n    {\n" );
        for( size_t i = 0; i < _references.size(); ++i ) {
            fprintf( output, "        " );
            writeRef( output, _references[i] );
            fprintf( output, ",\n" );
        }
        if( _references.empty() ) fprintf( output, "        {},\n" );
        fprintf( output, "    },\n    {\n" );
        for( size_t i = 0; i < _rules.size(); ++i ) {
            fprintf( output, "        { %u },\n", _rules[i] );
        }
        if( _rules.empty() ) fprintf( output, "        {},\n" );
        fprintf( output, "    },\n    {\n" );
        for( size_t i = 0; i < _schemaTexts.size(); ++i ) {
            const PackedSchemaText & record = _schemaTexts[i];
            fprintf( output, "        { %u, %s, %u, %u },\n",
                record.schema, record.kind.c_str(), record.name, record.text );
        }
        if( _schemaTexts.empty() ) fprintf( output, "        {},\n" );
        fprintf( output, "    },\n    {\n" );
        for( size_t i = 0; i < _enumElements.size(); ++i ) {
            fprintf( output, "        %u,\n", _enumElements[i] );
        }
        if( _enumElements.empty() ) fprintf( output, "        0,\n" );
        fprintf( output, "    },\n" );
        writeStringPool( output, _strings.data );
        fprintf( output, "};\n\n" );
    }
};

ImageBuilder & builder() {
    static ImageBuilder imageBuilder;
    return imageBuilder;
}

}

extern "C" void SCHEMAimage_reset( void ) {
    builder().Reset();
}

extern "C" void SCHEMAimage_register_schema(
    Schema schema, Linked_List entities ) {
    builder().RegisterSchema( schema, entities );
}

extern "C" void SCHEMAimage_set_external_mapping(
    Entity entity, int externalMapping ) {
    builder().SetExternalMapping( entity, externalMapping != 0 );
}

extern "C" void SCHEMAimage_write( FILE * output ) {
    builder().Write( output );
}
