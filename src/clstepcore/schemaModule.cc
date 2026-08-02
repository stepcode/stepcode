#include "clstepcore/schemaModule.h"

#include <cassert>
#include <unordered_map>

#include "clstepcore/attrDescriptor.h"
#include "clstepcore/aggrTypeDescriptor.h"
#include "clstepcore/derivedAttribute.h"
#include "clstepcore/dictSchema.h"
#include "clstepcore/entityDescriptor.h"
#include "clstepcore/enumTypeDescriptor.h"
#include "clstepcore/inverseAttribute.h"
#include "clstepcore/selectTypeDescriptor.h"
#include "clstepcore/typeDescriptor.h"

namespace {

struct PendingSchemaModule {
    std::vector<const EntityDescriptor *> entities;
    std::vector<const TypeDescriptor *> types;
    std::vector<const AttrDescriptor *> attributes;
};

class PendingSchemaModules {
    typedef std::unordered_map<const Schema *, PendingSchemaModule> ModuleMap;

    ModuleMap _modules;
    const Schema * _lastSchema;
    PendingSchemaModule * _lastModule;

    PendingSchemaModule * Find( Schema & schema ) {
        if( _lastSchema == &schema ) {
            return _lastModule;
        }
        ModuleMap::iterator i = _modules.find( &schema );
        _lastSchema = &schema;
        _lastModule = i == _modules.end() ? 0 : &i->second;
        return _lastModule;
    }

public:
    PendingSchemaModules() : _lastSchema( 0 ), _lastModule( 0 ) {
    }

    void Begin( Schema & schema ) {
        PendingSchemaModule & module = _modules[&schema];
        module.entities.clear();
        module.types.clear();
        module.attributes.clear();
        _lastSchema = &schema;
        _lastModule = &module;
    }

    void RecordEntity( Schema & schema, const EntityDescriptor * entity ) {
        PendingSchemaModule * module = Find( schema );
        if( module ) {
            module->entities.push_back( entity );
        }
    }

    void RecordType( Schema & schema, const TypeDescriptor * type ) {
        PendingSchemaModule * module = Find( schema );
        if( module ) {
            module->types.push_back( type );
        }
    }

    void RecordAttribute( Schema & schema, const AttrDescriptor * attribute ) {
        PendingSchemaModule * module = Find( schema );
        if( module ) {
            module->attributes.push_back( attribute );
        }
    }

    bool Consume( Schema & schema, PendingSchemaModule & module ) {
        ModuleMap::iterator i = _modules.find( &schema );
        if( i == _modules.end() ) {
            return false;
        }
        module.entities.swap( i->second.entities );
        module.types.swap( i->second.types );
        module.attributes.swap( i->second.attributes );
        if( _lastSchema == &schema ) {
            _lastSchema = 0;
            _lastModule = 0;
        }
        _modules.erase( i );
        return true;
    }
};

PendingSchemaModules & pendingSchemaModules() {
    static PendingSchemaModules modules;
    return modules;
}

const TypeDescriptor * typeFromSlot( const SchemaModuleSlot & record ) {
    switch( record.kind ) {
        case SchemaModuleSlot_EnumType:
            return *static_cast<EnumTypeDescriptor * const *>( record.slot );
        case SchemaModuleSlot_SelectType:
            return *static_cast<SelectTypeDescriptor * const *>( record.slot );
        case SchemaModuleSlot_AggregateType:
            return *static_cast<AggrTypeDescriptor * const *>( record.slot );
        case SchemaModuleSlot_ArrayType:
            return *static_cast<ArrayTypeDescriptor * const *>( record.slot );
        case SchemaModuleSlot_ListType:
            return *static_cast<ListTypeDescriptor * const *>( record.slot );
        case SchemaModuleSlot_SetType:
            return *static_cast<SetTypeDescriptor * const *>( record.slot );
        case SchemaModuleSlot_BagType:
            return *static_cast<BagTypeDescriptor * const *>( record.slot );
        case SchemaModuleSlot_Type:
        default:
            return *static_cast<TypeDescriptor * const *>( record.slot );
    }
}

const AttrDescriptor * attributeFromSlot( const SchemaModuleSlot & record ) {
    switch( record.kind ) {
        case SchemaModuleSlot_DerivedAttribute:
            return *static_cast<Derived_attribute * const *>( record.slot );
        case SchemaModuleSlot_InverseAttribute:
            return *static_cast<Inverse_attribute * const *>( record.slot );
        case SchemaModuleSlot_Attribute:
        default:
            return *static_cast<AttrDescriptor * const *>( record.slot );
    }
}

void prepareLateBoundLayouts(
    const std::vector<const EntityDescriptor *> & entities ) {
    for( size_t i = 0; i < entities.size(); ++i ) {
        if( entities[i] && !entities[i]->NewSTEPentity ) {
            entities[i]->PrepareLateBoundLayout();
        }
    }
}

}

void BeginSchemaModuleImage( Schema & schema ) {
    pendingSchemaModules().Begin( schema );
}

void RecordSchemaModuleEntity( Schema & schema,
                               const EntityDescriptor * entity ) {
    pendingSchemaModules().RecordEntity( schema, entity );
}

void RecordSchemaModuleType( Schema & schema, const TypeDescriptor * type ) {
    pendingSchemaModules().RecordType( schema, type );
}

void RecordSchemaModuleAttribute( Schema & schema,
                                  const AttrDescriptor * attribute ) {
    pendingSchemaModules().RecordAttribute( schema, attribute );
}

SchemaModule::SchemaModule() : _schema( 0 ) {
}

SchemaModule::~SchemaModule() {
}

void SchemaModule::Initialize(
    Schema & schema,
    const EntityDescriptor * const * entities, size_t entityCount,
    const TypeDescriptor * const * types, size_t typeCount,
    const AttrDescriptor * const * attributes, size_t attributeCount ) {
    _schema = &schema;
    _entities.assign( entities, entities + entityCount );
    _types.assign( types, types + typeCount );
    _attributes.assign( attributes, attributes + attributeCount );
    prepareLateBoundLayouts( _entities );
}

void SchemaModule::InitializeFromSlots(
    Schema & schema,
    const SchemaModuleSlot * entities, size_t entityCount,
    const SchemaModuleSlot * types, size_t typeCount,
    const SchemaModuleSlot * attributes, size_t attributeCount ) {
    _schema = &schema;
    _entities.clear();
    _entities.reserve( entityCount );
    for( size_t i = 0; i < entityCount; ++i ) {
        _entities.push_back(
            *static_cast<EntityDescriptor * const *>( entities[i].slot ) );
    }
    _types.clear();
    _types.reserve( typeCount );
    for( size_t i = 0; i < typeCount; ++i ) {
        _types.push_back( typeFromSlot( types[i] ) );
    }
    _attributes.clear();
    _attributes.reserve( attributeCount );
    for( size_t i = 0; i < attributeCount; ++i ) {
        _attributes.push_back( attributeFromSlot( attributes[i] ) );
    }
    prepareLateBoundLayouts( _entities );
}

void SchemaModule::InitializeFromImage( Schema & schema,
                                        const SchemaModuleImage & image ) {
    PendingSchemaModule module;
    const bool found = pendingSchemaModules().Consume( schema, module );
    const bool valid = found &&
        image.version == SchemaModuleImageVersion_1 &&
        module.entities.size() == image.entityCount &&
        module.types.size() == image.typeCount &&
        module.attributes.size() == image.attributeCount;
    assert( valid );
    if( !valid ) {
        _schema = 0;
        _entities.clear();
        _types.clear();
        _attributes.clear();
        return;
    }
    _schema = &schema;
    _entities.swap( module.entities );
    _types.swap( module.types );
    _attributes.swap( module.attributes );
    prepareLateBoundLayouts( _entities );
}

bool SchemaModule::IsInitialized() const {
    return _schema != 0;
}

const Schema & SchemaModule::GetSchema() const {
    assert( _schema );
    return *_schema;
}

const EntityDescriptor * SchemaModule::Entity( size_t id ) const {
    return id < _entities.size() ? _entities[id] : 0;
}

const TypeDescriptor * SchemaModule::Type( size_t id ) const {
    return id < _types.size() ? _types[id] : 0;
}

const AttrDescriptor * SchemaModule::Attribute( size_t id ) const {
    return id < _attributes.size() ? _attributes[id] : 0;
}

size_t SchemaModule::EntityCount() const {
    return _entities.size();
}

size_t SchemaModule::TypeCount() const {
    return _types.size();
}

size_t SchemaModule::AttributeCount() const {
    return _attributes.size();
}
