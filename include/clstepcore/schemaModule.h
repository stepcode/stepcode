#ifndef SCHEMAMODULE_H
#define SCHEMAMODULE_H

#include <stddef.h>
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

    bool IsInitialized() const;
    const Schema & GetSchema() const;
    const EntityDescriptor * Entity( size_t id ) const;
    const TypeDescriptor * Type( size_t id ) const;
    const AttrDescriptor * Attribute( size_t id ) const;
    size_t EntityCount() const;
    size_t TypeCount() const;
    size_t AttributeCount() const;
};

#endif
