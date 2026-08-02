#ifndef SCHEMAINIT_H
#define SCHEMAINIT_H

#include <stddef.h>

#include "sc_export.h"
#include "clstepcore/attrDescriptor.h"
#include "clstepcore/aggrTypeDescriptor.h"
#include "clstepcore/dictSchema.h"
#include "clstepcore/entityDescriptor.h"
#include "clstepcore/enumTypeDescriptor.h"
#include "clstepcore/selectTypeDescriptor.h"
#include "clstepcore/schemaModule.h"

class Derived_attribute;
class Inverse_attribute;
class ComplexCollect;
class Registry;

/** Compact records used by API v2 generated schema initialization. */
struct SC_CORE_EXPORT SchemaInitRecord {
    Schema ** slot;
    const char * name;
    ModelContentsCreator modelContentsCreator;
};

struct SC_CORE_EXPORT EntityDescriptorInitRecord {
    EntityDescriptor ** slot;
    const char * name;
    Schema ** schemaSlot;
    Logical abstractEntity;
    Logical externalMapping;
    Creator creator;
};

struct SC_CORE_EXPORT EntitySupertypeInitRecord {
    EntityDescriptor * supertype;
};

enum TypeDescriptorInitKind {
    TypeDescriptorInit_Base,
    TypeDescriptorInit_Enum,
    TypeDescriptorInit_Select,
    TypeDescriptorInit_Aggregate,
    TypeDescriptorInit_Array,
    TypeDescriptorInit_List,
    TypeDescriptorInit_Set,
    TypeDescriptorInit_Bag
};

struct SC_CORE_EXPORT TypeDescriptorInitRecord {
    void * slot;
    TypeDescriptorInitKind kind;
    const char * name;
    PrimitiveType fundamentalType;
    Schema ** schemaSlot;
    const char * description;
    int uniqueElements;
    SelectCreator selectCreator;
    EnumCreator enumCreator;
    AggregateCreator aggregateCreator;

    TypeDescriptorInitRecord( TypeDescriptor ** descriptorSlot,
                              const char * typeName, PrimitiveType type,
                              Schema ** schema, const char * typeDescription );
    TypeDescriptorInitRecord( EnumTypeDescriptor ** descriptorSlot,
                              const char * typeName, PrimitiveType type,
                              Schema ** schema, const char * typeDescription,
                              EnumCreator creator );
    TypeDescriptorInitRecord( SelectTypeDescriptor ** descriptorSlot,
                              int unique, const char * typeName,
                              PrimitiveType type, Schema ** schema,
                              const char * typeDescription,
                              SelectCreator creator );
    TypeDescriptorInitRecord( AggrTypeDescriptor ** descriptorSlot,
                              const char * typeName, PrimitiveType type,
                              Schema ** schema, const char * typeDescription,
                              AggregateCreator creator );
    TypeDescriptorInitRecord( ArrayTypeDescriptor ** descriptorSlot,
                              const char * typeName, PrimitiveType type,
                              Schema ** schema, const char * typeDescription,
                              AggregateCreator creator );
    TypeDescriptorInitRecord( ListTypeDescriptor ** descriptorSlot,
                              const char * typeName, PrimitiveType type,
                              Schema ** schema, const char * typeDescription,
                              AggregateCreator creator );
    TypeDescriptorInitRecord( SetTypeDescriptor ** descriptorSlot,
                              const char * typeName, PrimitiveType type,
                              Schema ** schema, const char * typeDescription,
                              AggregateCreator creator );
    TypeDescriptorInitRecord( BagTypeDescriptor ** descriptorSlot,
                              const char * typeName, PrimitiveType type,
                              Schema ** schema, const char * typeDescription,
                              AggregateCreator creator );
};

/**
 * Attribute descriptor record.  The overloaded constructors preserve the
 * concrete type of generated descriptor slots without templates or casts in
 * generated translation units.
 */
struct SC_CORE_EXPORT AttributeInitRecord {
    AttrDescriptor ** attributeSlot;
    Derived_attribute ** derivedSlot;
    Inverse_attribute ** inverseSlot;
    const char * name;
    const TypeDescriptor * domain;
    Logical optional;
    Logical unique;
    AttrType_Enum attrType;
    const char * initializer;
    const char * invertedAttribute;
    const char * invertedEntity;

    AttributeInitRecord( AttrDescriptor ** slot, const char * attrName,
                         const TypeDescriptor * domainType, Logical isOptional,
                         Logical isUnique, AttrType_Enum type,
                         const char * init = 0 );
    AttributeInitRecord( Derived_attribute ** slot, const char * attrName,
                         const TypeDescriptor * domainType, Logical isOptional,
                         Logical isUnique, AttrType_Enum type,
                         const char * init = 0 );
    AttributeInitRecord( Inverse_attribute ** slot, const char * attrName,
                         const TypeDescriptor * domainType, Logical isOptional,
                         Logical isUnique, const char * invertedAttr,
                         const char * invertedEnt );
};

enum ComplexNodeInitKind {
    ComplexNodeInit_Simple,
    ComplexNodeInit_And,
    ComplexNodeInit_Or,
    ComplexNodeInit_AndOr
};

struct SC_CORE_EXPORT ComplexNodeInitRecord {
    ComplexNodeInitKind kind;
    const char * name;
    size_t firstChild;
    size_t nextSibling;
};

struct SC_CORE_EXPORT ComplexListInitRecord {
    size_t rootNode;
};

struct SC_CORE_EXPORT GlobalRuleInitRecord {
    const char * name;
    const char * text;
};

enum SchemaImageDescriptorRefKind {
    SchemaImageRef_None,
    SchemaImageRef_Entity,
    SchemaImageRef_Type,
    SchemaImageRef_Builtin,
    SchemaImageRef_Aggregate
};

struct SC_CORE_EXPORT SchemaImageDescriptorRef {
    uint32_t kind;
    uint32_t schema;
    uint32_t index;
};

struct SC_CORE_EXPORT SchemaImageSchemaRecord {
    uint32_t name;
    uint32_t firstEntity;
    uint32_t entityCount;
    uint32_t firstType;
    uint32_t typeCount;
};

struct SC_CORE_EXPORT SchemaImageEntityRecord {
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

enum SchemaImageEntityFlags {
    SchemaImageEntity_Abstract = 1u << 0,
    SchemaImageEntity_ExternalMapping = 1u << 1
};

struct SC_CORE_EXPORT SchemaImageTypeRecord {
    uint32_t schema;
    uint32_t name;
    uint32_t description;
    uint32_t fundamentalType;
    uint32_t descriptorKind;
    int32_t uniqueElements;
    SchemaImageDescriptorRef referent;
    uint32_t aggregate;
    uint32_t firstSelectElement;
    uint32_t selectElementCount;
    uint32_t firstWhereRule;
    uint32_t whereRuleCount;
    uint32_t firstEnumElement;
    uint32_t enumElementCount;
};

struct SC_CORE_EXPORT SchemaImageAttributeRecord {
    uint32_t name;
    SchemaImageDescriptorRef domain;
    uint32_t optional;
    uint32_t unique;
    uint32_t attrType;
    uint32_t initializer;
    uint32_t invertedAttribute;
    uint32_t invertedEntity;
};

struct SC_CORE_EXPORT SchemaImageAggregateRecord {
    uint32_t schema;
    uint32_t description;
    uint32_t fundamentalType;
    SchemaImageDescriptorRef referent;
    uint32_t bound1Type;
    int32_t bound1;
    uint32_t bound1Text;
    uint32_t bound2Type;
    int32_t bound2;
    uint32_t bound2Text;
    uint32_t optionalElements;
    uint32_t uniqueElements;
};

struct SC_CORE_EXPORT SchemaImageRuleRecord {
    uint32_t text;
};

enum SchemaImageSchemaTextKind {
    SchemaImageText_GlobalRule,
    SchemaImageText_Function,
    SchemaImageText_Procedure
};

struct SC_CORE_EXPORT SchemaImageSchemaTextRecord {
    uint32_t schema;
    uint32_t kind;
    uint32_t name;
    uint32_t text;
};

struct SC_CORE_EXPORT SchemaImageSchemaBinding {
    Schema ** schemaSlot;
    ModelContentsCreator modelContentsCreator;
    SchemaModule * module;
};

struct SC_CORE_EXPORT SchemaImageTypeBinding {
    void * slot;
    SelectCreator selectCreator;
    EnumCreator enumCreator;
    AggregateCreator aggregateCreator;
};

enum SchemaLoadError {
    SchemaLoad_Ok,
    SchemaLoad_UnsupportedVersion,
    SchemaLoad_TruncatedImage,
    SchemaLoad_InvalidOffset,
    SchemaLoad_InvalidString,
    SchemaLoad_InvalidReference,
    SchemaLoad_CountMismatch,
    SchemaLoad_MissingBinding,
    SchemaLoad_ModuleFailure
};

struct SC_CORE_EXPORT SchemaLoadResult {
    SchemaLoadError error;
    uint32_t record;

    bool Succeeded() const {
        return error == SchemaLoad_Ok;
    }
    const char * Message() const;
};

SC_CORE_EXPORT SchemaLoadResult InitializeSchemaFromImage(
    Registry & registry, const SchemaModuleImage & image,
    const SchemaImageSchemaBinding * schemaBindings,
    size_t schemaBindingCount,
    const SchemaImageTypeBinding * typeBindings,
    size_t typeBindingCount, SchemaLoadContext & context );

enum PackedComplexImageVersion {
    PackedComplexImageVersion_1 = 1
};

struct SC_CORE_EXPORT PackedComplexImage {
    uint32_t version;
    uint32_t byteSize;
    uint32_t stringOffset;
    uint32_t stringBytes;
    uint32_t nodeOffset;
    uint32_t nodeCount;
    uint32_t listOffset;
    uint32_t listCount;
};

struct SC_CORE_EXPORT PackedComplexNode {
    uint32_t kind;
    uint32_t name;
    uint32_t firstChild;
    uint32_t nextSibling;
};

struct SC_CORE_EXPORT PackedComplexList {
    uint32_t rootNode;
};

SC_CORE_EXPORT ComplexCollect * InitializePackedComplexSupport(
    const PackedComplexImage & image, SchemaLoadResult * result = 0 );

SC_CORE_EXPORT void InitializeSchemas( Registry & reg,
                                       const SchemaInitRecord * records,
                                       size_t count );
SC_CORE_EXPORT void InitializeSchemaModuleImages(
    const SchemaInitRecord * records, size_t count );
SC_CORE_EXPORT void InitializeEntityDescriptors(
    const EntityDescriptorInitRecord * records, size_t count );
SC_CORE_EXPORT void InitializeTypeDescriptors(
    const TypeDescriptorInitRecord * records, size_t count );
SC_CORE_EXPORT void InitializeTypeMetadata(
    Registry & reg, Schema & schema, TypeDescriptor & type,
    const TypeDescriptor * referent,
    const TypeDescriptor * const * selectElements, size_t selectElementCount );
SC_CORE_EXPORT void InitializeEntityMetadata(
    Registry & reg, EntityDescriptor & entity, Schema & schema,
    const EntitySupertypeInitRecord * supertypes, size_t supertypeCount,
    const AttributeInitRecord * attributes, size_t attributeCount );
SC_CORE_EXPORT void InitializeWhereRules(
    TypeDescriptor & owner, const char * const * rules, size_t count );
SC_CORE_EXPORT void InitializeUniquenessRules(
    EntityDescriptor & owner, const char * const * rules, size_t count );
SC_CORE_EXPORT void InitializeGlobalRules(
    Schema & schema, const GlobalRuleInitRecord * rules, size_t count );
SC_CORE_EXPORT void InitializeFunctions(
    Schema & schema, const char * const * functions, size_t count );
SC_CORE_EXPORT void InitializeProcedures(
    Schema & schema, const char * const * procedures, size_t count );
SC_CORE_EXPORT ComplexCollect * InitializeComplexSupport(
    const ComplexNodeInitRecord * nodes, size_t nodeCount,
    const ComplexListInitRecord * lists, size_t listCount );

#endif
