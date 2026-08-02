#include "clstepcore/Registry.h"
#include "clstepcore/STEPattribute.h"
#include "clstepcore/derivedAttribute.h"
#include "clstepcore/complexSupport.h"
#include "clstepcore/schemaInit.h"
#include "clstepcore/schemaModule.h"

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
        SchemaModuleImageVersion_1, 2, 2, 3
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
