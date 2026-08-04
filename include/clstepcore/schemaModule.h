#ifndef SCHEMAMODULE_H
#define SCHEMAMODULE_H

#include <stddef.h>
#include <stdint.h>
#include <vector>

#include "sc_export.h"

class AttrDescriptor;
class EntityDescriptor;
class Schema;
class TypeDescriptor;

enum SchemaModuleSlotKind {
    SchemaModuleSlot_Entity,
    SchemaModuleSlot_Type,
    SchemaModuleSlot_EnumType,
    SchemaModuleSlot_SelectType,
    SchemaModuleSlot_AggregateType,
    SchemaModuleSlot_ArrayType,
    SchemaModuleSlot_ListType,
    SchemaModuleSlot_SetType,
    SchemaModuleSlot_BagType,
    SchemaModuleSlot_Attribute,
    SchemaModuleSlot_DerivedAttribute,
    SchemaModuleSlot_InverseAttribute
};

struct SC_CORE_EXPORT SchemaModuleSlot {
    const void * slot;
    SchemaModuleSlotKind kind;
};

enum SchemaModuleImageVersion {
    SchemaModuleImageVersion_1 = 1,
    SchemaModuleImageVersion_2 = 2
};

enum SchemaModuleImageFlags {
    SchemaModuleImage_FullMetadata = 0,
    SchemaModuleImage_StructuralMetadata = 1u << 0
};

/**
 * Relocation-free generated header for a schema module image.
 *
 * Version 1 records only stable generated-ID counts.  Version 2 describes
 * one bounded block of offset-based schemas, descriptors, metadata, and
 * strings.  Runtime loading validates the complete block before constructing
 * descriptors or changing a Registry.
 */
struct SC_CORE_EXPORT SchemaModuleImage {
    uint32_t version;
    uint32_t entityCount;
    uint32_t typeCount;
    uint32_t attributeCount;

    /* Version 2 fields.  Version 1 initializers leave these zero. */
    uint32_t byteSize;
    uint32_t flags;
    uint32_t schemaCount;
    uint32_t stringBytes;
    uint32_t schemaOffset;
    uint32_t entityOffset;
    uint32_t typeOffset;
    uint32_t attributeOffset;
    uint32_t aggregateCount;
    uint32_t aggregateOffset;
    uint32_t referenceCount;
    uint32_t referenceOffset;
    uint32_t ruleCount;
    uint32_t ruleOffset;
    uint32_t schemaTextCount;
    uint32_t schemaTextOffset;
    uint32_t enumElementCount;
    uint32_t enumElementOffset;
    uint32_t stringOffset;
    uint32_t reserved;
    uint64_t fingerprint;
    uint32_t renameCount;
    uint32_t renameOffset;
};

class SchemaModule;

/**
 * Per-load descriptor collection used by packed API v2 schema images.
 *
 * Keeping this state explicit makes independent schema loads reentrant and
 * avoids the process-global capture map retained for version-1 images.
 */
class SC_CORE_EXPORT SchemaLoadContext {
    class Impl;
    Impl * _impl;

    SchemaLoadContext( const SchemaLoadContext & );
    SchemaLoadContext & operator=( const SchemaLoadContext & );
    friend class SchemaModule;

public:
    SchemaLoadContext();
    ~SchemaLoadContext();

    void Begin( Schema & schema );
    void RecordEntity( Schema & schema, const EntityDescriptor * entity );
    void RecordType( Schema & schema, const TypeDescriptor * type );
    void RecordAttribute( Schema & schema, const AttrDescriptor * attribute );
};

SC_CORE_EXPORT void RecordSchemaModuleEntity(
    Schema & schema, const EntityDescriptor * entity );
SC_CORE_EXPORT void RecordSchemaModuleType(
    Schema & schema, const TypeDescriptor * type );
SC_CORE_EXPORT void RecordSchemaModuleAttribute(
    Schema & schema, const AttrDescriptor * attribute );
SC_CORE_EXPORT void BeginSchemaModuleImage( Schema & schema );

/**
 * Stable API v2 view of a generated schema.
 *
 * Generated EntityId, TypeId, and AttributeId enumerations are converted to
 * indexes before calling this class.  Keeping the public lookup API separate
 * from generated descriptor globals allows the underlying representation to
 * move from C++ objects to packed schema data without another API break.
 */
class SC_CORE_EXPORT SchemaModule {
    Schema * _schema;
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4251 )
#endif
    std::vector<const EntityDescriptor *> _entities;
    std::vector<const TypeDescriptor *> _types;
    std::vector<const AttrDescriptor *> _attributes;
    uint64_t _fingerprint;
#ifdef _MSC_VER
#pragma warning( pop )
#endif

public:
    SchemaModule();
    ~SchemaModule();

    void Initialize(
        Schema & schema,
        const EntityDescriptor * const * entities, size_t entityCount,
        const TypeDescriptor * const * types, size_t typeCount,
        const AttrDescriptor * const * attributes, size_t attributeCount );
    void InitializeFromSlots(
        Schema & schema,
        const SchemaModuleSlot * entities, size_t entityCount,
        const SchemaModuleSlot * types, size_t typeCount,
        const SchemaModuleSlot * attributes, size_t attributeCount );
    void InitializeFromImage( Schema & schema,
                              const SchemaModuleImage & image );
    bool InitializeFromContext( Schema & schema, SchemaLoadContext & context,
                                uint64_t fingerprint );

    bool IsInitialized() const;
    const Schema & GetSchema() const;
    const EntityDescriptor * Entity( size_t id ) const;
    const TypeDescriptor * Type( size_t id ) const;
    const AttrDescriptor * Attribute( size_t id ) const;
    size_t EntityCount() const;
    size_t TypeCount() const;
    size_t AttributeCount() const;
    uint64_t Fingerprint() const;
};

#endif
