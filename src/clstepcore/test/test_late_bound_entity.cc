#include "clstepcore/Registry.h"
#include "clstepcore/STEPattribute.h"
#include "clstepcore/derivedAttribute.h"
#include "clstepcore/schemaInit.h"

namespace {

Schema * testSchema = 0;
EntityDescriptor * baseEntity = 0;
EntityDescriptor * leafEntity = 0;
AttrDescriptor * labelAttribute = 0;
AttrDescriptor * lengthAttribute = 0;
Derived_attribute * derivedLabel = 0;

void initializeLateSchema( Registry & registry ) {
    const SchemaInitRecord schemas[] = {
        { &testSchema, "Late_Bound_Test", 0 }
    };
    InitializeSchemas( registry, schemas, 1 );

    const EntityDescriptorInitRecord entities[] = {
        { &baseEntity, "Base", &testSchema, LFalse, LFalse, 0 },
        { &leafEntity, "Leaf", &testSchema, LFalse, LFalse, 0 }
    };
    InitializeEntityDescriptors( entities, 2 );

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
#undef CHECK
    return 0;
}
