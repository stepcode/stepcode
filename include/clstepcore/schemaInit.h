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

SC_CORE_EXPORT void InitializeSchemas( Registry & reg,
                                       const SchemaInitRecord * records,
                                       size_t count );
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
