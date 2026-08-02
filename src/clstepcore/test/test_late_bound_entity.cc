#include "clstepcore/Registry.h"
#include "clstepcore/STEPaggrInt.h"
#include "clstepcore/STEPattribute.h"
#include "clstepcore/derivedAttribute.h"
#include "clstepcore/complexSupport.h"
#include "clstepcore/schemaInit.h"
#include "clstepcore/schemaModule.h"
#include "clstepcore/sdaiSelect.h"

#include <string>

namespace {

Schema * testSchema = 0;
EntityDescriptor * baseEntity = 0;
EntityDescriptor * leafEntity = 0;
AttrDescriptor * labelAttribute = 0;
AttrDescriptor * lengthAttribute = 0;
Derived_attribute * derivedLabel = 0;
AttrDescriptor * redefinedLabel = 0;
TypeDescriptor * labelType = 0;
TypeDescriptor * distanceType = 0;

Schema * packedSchema = 0;
SchemaModule packedSchemaModule;
SchemaLoadResult packedSchemaResult = { SchemaLoad_Ok, 0 };

struct PackedTestSchemaImage {
    SchemaModuleImage header;
    SchemaImageSchemaRecord schemas[1];
    SchemaImageEntityRecord entities[1];
    SchemaImageTypeRecord types[1];
    SchemaImageAttributeRecord attributes[1];
    SchemaImageRenameRecord renames[1];
    char strings[39];
};

const PackedTestSchemaImage packedTestSchemaImage = {
    { SchemaModuleImageVersion_2, 1, 1, 1,
      sizeof( PackedTestSchemaImage ), SchemaModuleImage_FullMetadata, 1, 38,
      offsetof( PackedTestSchemaImage, schemas ),
      offsetof( PackedTestSchemaImage, entities ),
      offsetof( PackedTestSchemaImage, types ),
      offsetof( PackedTestSchemaImage, attributes ),
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      offsetof( PackedTestSchemaImage, strings ), 0, 1234,
      1, offsetof( PackedTestSchemaImage, renames ) },
    { { 1, 0, 1, 0, 1 } },
    { { 0, 8, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { 0, 20, 27, sdaiREAL, TypeDescriptorInit_Base, 0,
        { SchemaImageRef_Builtin, 0, sdaiREAL }, UINT32_MAX,
        0, 0, 0, 0, 0, 0 } },
    { { 14, { SchemaImageRef_Type, 0, 0 },
        0, 0, AttrType_Explicit, 0, 0, 0 } },
    { { { SchemaImageRef_Entity, 0, 0 }, 0, 32 } },
    "\000Packed\000Thing\000value\000Length\000REAL\000Alias\000"
};

void initializePackedSchema( Registry & registry ) {
    const SchemaImageSchemaBinding schemas[] = {
        { &packedSchema, 0, &packedSchemaModule }
    };
    SchemaLoadContext context;
    packedSchemaResult = InitializeSchemaFromImage(
        registry, packedTestSchemaImage.header, schemas, 1, 0, 0, context );
}

void initializeLateSchema( Registry & registry ) {
    const SchemaInitRecord schemas[] = {
        { &testSchema, "Late_Bound_Test", 0 }
    };
    InitializeSchemas( registry, schemas, 1 );
    InitializeSchemaModuleImages( schemas, 1 );

    const EntityDescriptorInitRecord entities[] = {
        { &baseEntity, "Base", &testSchema, LFalse, LFalse, 0 },
        { &leafEntity, "Leaf", &testSchema, LFalse, LFalse, 0 }
    };
    InitializeEntityDescriptors( entities, 2 );

    const TypeDescriptorInitRecord types[] = {
        TypeDescriptorInitRecord( &labelType, "Label_Type", STRING_TYPE,
                                  &testSchema, "STRING" ),
        TypeDescriptorInitRecord( &distanceType, "Distance_Type", REAL_TYPE,
                                  &testSchema, "REAL" )
    };
    InitializeTypeDescriptors( types, 2 );
    InitializeTypeMetadata( registry, *testSchema, *distanceType, t_sdaiREAL,
                            0, 0 );
    InitializeTypeMetadata( registry, *testSchema, *labelType, t_sdaiSTRING,
                            0, 0 );

    const AttributeInitRecord baseAttributes[] = {
        AttributeInitRecord( &labelAttribute, "label", t_sdaiSTRING,
                             LFalse, LFalse, AttrType_Explicit )
    };
    InitializeEntityMetadata( registry, *baseEntity, *testSchema, 0, 0,
                              baseAttributes, 1 );

    const EntitySupertypeInitRecord supertypes[] = { { baseEntity } };
    const AttributeInitRecord leafAttributes[] = {
        AttributeInitRecord( &lengthAttribute, "length", t_sdaiREAL,
                             LFalse, LFalse, AttrType_Explicit ),
        AttributeInitRecord( &derivedLabel, "base.label", t_sdaiSTRING,
                             LFalse, LFalse, AttrType_Deriving, "'derived'" )
    };
    InitializeEntityMetadata( registry, *leafEntity, *testSchema,
                              supertypes, 1, leafAttributes, 2 );
}

}

int main() {
#define CHECK(condition) do { if( !( condition ) ) return __LINE__; } while( 0 )
    Registry registry( initializeLateSchema );
    const EntityDescriptor * entities[] = { baseEntity, leafEntity };
    const TypeDescriptor * types[] = { t_sdaiSTRING, t_sdaiREAL };
    const AttrDescriptor * attributes[] = {
        labelAttribute, lengthAttribute, derivedLabel
    };
    SchemaModule module;
    module.Initialize( *testSchema, entities, 2, types, 2, attributes, 3 );
    CHECK( module.IsInitialized() );
    CHECK( &module.GetSchema() == testSchema );
    CHECK( module.EntityCount() == 2 && module.Entity( 1 ) == leafEntity );
    CHECK( module.TypeCount() == 2 && module.Type( 0 ) == t_sdaiSTRING );
    CHECK( module.AttributeCount() == 3 &&
           module.Attribute( 2 ) == derivedLabel );
    CHECK( module.Entity( 2 ) == 0 && module.Type( 2 ) == 0 &&
           module.Attribute( 3 ) == 0 );

    const SchemaModuleImage image = {
        SchemaModuleImageVersion_1, 2, 2, 3,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0
    };
    SchemaModule packedModule;
    packedModule.InitializeFromImage( *testSchema, image );
    CHECK( packedModule.EntityCount() == 2 );
    CHECK( packedModule.Entity( 0 ) == baseEntity );
    CHECK( packedModule.Entity( 1 ) == leafEntity );
    CHECK( packedModule.TypeCount() == 2 );
    CHECK( packedModule.Type( 0 ) == labelType );
    CHECK( packedModule.Type( 1 ) == distanceType );
    CHECK( packedModule.AttributeCount() == 3 );
    CHECK( packedModule.Attribute( 0 ) == labelAttribute );
    CHECK( packedModule.Attribute( 1 ) == lengthAttribute );
    CHECK( packedModule.Attribute( 2 ) == derivedLabel );

    Registry packedRegistry( initializePackedSchema );
    CHECK( packedSchemaResult.Succeeded() );
    CHECK( packedSchemaModule.IsInitialized() );
    CHECK( packedSchemaModule.Fingerprint() == 1234 );
    CHECK( packedSchemaModule.EntityCount() == 1 );
    CHECK( packedSchemaModule.Entity( 0 )->Name() == std::string( "Thing" ) );
    CHECK( packedSchemaModule.TypeCount() == 1 );
    CHECK( packedSchemaModule.AttributeCount() == 1 );
    CHECK( std::string(
               packedSchemaModule.Entity( 0 )->Name( "Packed" ) ) ==
           "Alias" );
    SDAI_Application_instance * packedInstance =
        packedRegistry.ObjCreate( "Thing" );
    CHECK( packedInstance != 0 );
    delete packedInstance;

    SchemaModuleImage invalidImage = packedTestSchemaImage.header;
    invalidImage.byteSize = sizeof( SchemaModuleImage ) - 1;
    const SchemaImageSchemaBinding invalidBindings[] = {
        { &packedSchema, 0, &packedSchemaModule }
    };
    SchemaLoadContext invalidContext;
    SchemaLoadResult invalidResult = InitializeSchemaFromImage(
        packedRegistry, invalidImage, invalidBindings, 1, 0, 0,
        invalidContext );
    CHECK( invalidResult.error == SchemaLoad_TruncatedImage );

    const size_t noNode = static_cast<size_t>( -1 );
    const ComplexNodeInitRecord complexNodes[] = {
        { ComplexNodeInit_And, 0, 1, noNode },
        { ComplexNodeInit_Simple, "base", noNode, 2 },
        { ComplexNodeInit_Or, 0, 3, noNode },
        { ComplexNodeInit_Simple, "leaf", noNode, 4 },
        { ComplexNodeInit_Simple, "other_leaf", noNode, noNode }
    };
    const ComplexListInitRecord complexLists[] = { { 0 } };
    ComplexCollect * complex = InitializeComplexSupport(
        complexNodes, 5, complexLists, 1 );
    CHECK( complex && complex->clists && complex->clists->head );
    CHECK( complex->clists->head->childCount() == 2 );
    MultList * choice = dynamic_cast<MultList *>(
        complex->clists->head->getChild( 1 ) );
    CHECK( choice && choice->childCount() == 2 );
    delete complex;

    struct PackedTestComplexImage {
        PackedComplexImage header;
        PackedComplexNode nodes[5];
        PackedComplexList lists[1];
        char strings[23];
    };
    const PackedTestComplexImage packedComplexImage = {
        { PackedComplexImageVersion_1, sizeof( PackedTestComplexImage ),
          offsetof( PackedTestComplexImage, strings ), 22,
          offsetof( PackedTestComplexImage, nodes ), 5,
          offsetof( PackedTestComplexImage, lists ), 1 },
        {
            { ComplexNodeInit_And, 0, 1, UINT32_MAX },
            { ComplexNodeInit_Simple, 1, UINT32_MAX, 2 },
            { ComplexNodeInit_Or, 0, 3, UINT32_MAX },
            { ComplexNodeInit_Simple, 6, UINT32_MAX, 4 },
            { ComplexNodeInit_Simple, 11, UINT32_MAX, UINT32_MAX }
        },
        { { 0 } },
        "\000base\000leaf\000other_leaf\000"
    };
    SchemaLoadResult complexResult = { SchemaLoad_Ok, 0 };
    complex = InitializePackedComplexSupport(
        packedComplexImage.header, &complexResult );
    CHECK( complexResult.Succeeded() && complex );
    CHECK( complex->clists->head->childCount() == 2 );
    delete complex;

    PackedTestComplexImage cyclicComplex = packedComplexImage;
    cyclicComplex.nodes[4].nextSibling = 2;
    complex = InitializePackedComplexSupport(
        cyclicComplex.header, &complexResult );
    CHECK( !complex && complexResult.error == SchemaLoad_InvalidReference );

    EnumTypeDescriptor colorType(
        "Color", sdaiENUMERATION, testSchema,
        "ENUMERATION OF (RED, GREEN)", 0 );
    const char * colorElements[] = { "RED", "GREEN" };
    RegisterEnumDescriptorElements( colorType, colorElements, 2 );
    SDAI_Enum * color = colorType.CreateEnum();
    CHECK( color && color->no_elements() == 2 );
    CHECK( std::string( color->get_value_at( 1 ) ) == "GREEN" );
    CHECK( color->put( "RED" ) == 0 );
    delete color;

    SelectTypeDescriptor choiceType(
        ~sdaiSTRING, "Choice", sdaiSELECT, testSchema, "SELECT (STRING)", 0 );
    choiceType.Elements().AddNode(
        const_cast<TypeDescriptor *>( t_sdaiSTRING ) );
    SDAI_Select * genericChoice = choiceType.CreateSelect();
    CHECK( genericChoice &&
           genericChoice->SetString( t_sdaiSTRING, "generic" ) );
    CHECK( genericChoice->StringValue() &&
           *genericChoice->StringValue() == "generic" );
    delete genericChoice;

    ListTypeDescriptor integerList(
        "Integer_List", LIST_TYPE, testSchema, "LIST OF INTEGER", 0 );
    integerList.ReferentType(
        const_cast<TypeDescriptor *>( t_sdaiINTEGER ) );
    STEPaggregate * integers = integerList.CreateAggregate();
    CHECK( dynamic_cast<IntAggregate *>( integers ) != 0 );
    delete integers;

    SDAI_Application_instance * instance = registry.ObjCreate( "leaf" );
    CHECK( instance );
    CHECK( instance->eDesc == leafEntity );
    CHECK( instance->AttributeCount() == 2 );

    instance->ResetAttributes();
    STEPattribute * label = instance->NextAttribute();
    STEPattribute * length = instance->NextAttribute();
    CHECK( label && length );
    CHECK( label->IsDerived() );
    CHECK( length->StrToVal( "12.5", 0, 0 ) > SEVERITY_INCOMPLETE );
    CHECK( length->Real() && *length->Real() > 12.49 &&
           *length->Real() < 12.51 );

    SDAI_Application_instance * copy = instance->Replicate();
    CHECK( copy );
    CHECK( copy->AttributeCount() == 2 );
    copy->ResetAttributes();
    copy->NextAttribute();
    STEPattribute * copiedLength = copy->NextAttribute();
    CHECK( copiedLength && copiedLength->Real() );
    CHECK( *copiedLength->Real() > 12.49 && *copiedLength->Real() < 12.51 );

    delete copy;
    delete instance;

    redefinedLabel = new AttrDescriptor(
        "label", t_sdaiSTRING, LFalse, LFalse, AttrType_Redefining,
        *leafEntity );
    leafEntity->AddExplicitAttr( redefinedLabel );
    SDAI_Application_instance * narrowed = registry.ObjCreate( "leaf" );
    CHECK( narrowed && narrowed->AttributeCount() == 3 );
    narrowed->ResetAttributes();
    label = narrowed->NextAttribute();
    narrowed->NextAttribute();
    STEPattribute * narrowedLabel = narrowed->NextAttribute();
    CHECK( label && narrowedLabel );
    CHECK( label->IsDerived() );
    CHECK( label->RedefiningAttr() == narrowedLabel );
    delete narrowed;
#undef CHECK
    return 0;
}
