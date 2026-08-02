#include <cstring>
#include <limits>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "clstepcore/entityDescriptor.h"
#include "clstepcore/Registry.h"
#include "clstepcore/attrDescriptor.h"
#include "clstepcore/inverseAttribute.h"
#include "clstepcore/SubSuperIterators.h"
#include "clstepcore/STEPattribute.h"
#include "clstepcore/sdaiApplication_instance.h"

struct EntityAttributeLayoutEntry {
    explicit EntityAttributeLayoutEntry(
        const AttrDescriptor * attributeDescriptor )
        : descriptor( attributeDescriptor ), derived( false ),
          redefining( std::numeric_limits<size_t>::max() ) {
    }

    const AttrDescriptor * descriptor;
    bool derived;
    size_t redefining;
};

class EntityAttributeLayout {
public:
    EntityAttributeLayout() : hasRedefinitions( false ) {
    }

    std::vector<EntityAttributeLayoutEntry> attributes;
    bool hasRedefinitions;
};

namespace {

const size_t noAttribute = std::numeric_limits<size_t>::max();

class EntityAttributeLayoutCache {
    typedef std::unordered_map<const EntityDescriptor *,
            EntityAttributeLayout *> LayoutMap;

    LayoutMap _layouts;
    std::mutex _mutex;

public:
    ~EntityAttributeLayoutCache() {
        for( LayoutMap::iterator i = _layouts.begin();
             i != _layouts.end(); ++i ) {
            delete i->second;
        }
    }

    EntityAttributeLayout * Find( const EntityDescriptor * entity ) {
        std::lock_guard<std::mutex> lock( _mutex );
        LayoutMap::iterator i = _layouts.find( entity );
        return i == _layouts.end() ? 0 : i->second;
    }

    void Insert( const EntityDescriptor * entity,
                 EntityAttributeLayout * layout ) {
        std::lock_guard<std::mutex> lock( _mutex );
        LayoutMap::iterator i = _layouts.find( entity );
        if( i == _layouts.end() ) {
            _layouts[entity] = layout;
        } else {
            delete layout;
        }
    }

    void Erase( const EntityDescriptor * entity ) {
        std::lock_guard<std::mutex> lock( _mutex );
        LayoutMap::iterator i = _layouts.find( entity );
        if( i != _layouts.end() ) {
            delete i->second;
            _layouts.erase( i );
        }
    }
};

EntityAttributeLayoutCache & layoutCache() {
    static EntityAttributeLayoutCache cache;
    return cache;
}

size_t findAttribute( const EntityAttributeLayout & layout,
                      const char * name, const char * owner ) {
    for( size_t i = 0; i < layout.attributes.size(); ++i ) {
        const AttrDescriptor * candidate = layout.attributes[i].descriptor;
        if( strcmp( name, candidate->Name() ) == 0 &&
            ( !owner || candidate->Owner().IsA( owner ) ) ) {
            return i;
        }
    }
    return noAttribute;
}

void appendLayout( const EntityDescriptor * ed, EntityAttributeLayout & layout,
                   std::set<const EntityDescriptor *> & visited ) {
    if( !ed || !visited.insert( ed ).second ) {
        return;
    }

    EntityDescItr supers( ed->Supertypes() );
    const EntityDescriptor * super = 0;
    while( ( super = supers.NextEntityDesc() ) ) {
        appendLayout( super, layout, visited );
    }

    AttrDescItr attrs( ed->ExplicitAttr() );
    const AttrDescriptor * ad = 0;
    while( ( ad = attrs.NextAttrDesc() ) ) {
        const char * separator = strrchr( ad->Name(), '.' );
        const char * simpleName = separator ? separator + 1 : ad->Name();
        if( ad->AttrType() == AttrType_Deriving ) {
            const char * ownerName = ed->Name();
            std::string qualifiedOwner;
            if( separator ) {
                qualifiedOwner.assign( ad->Name(), separator - ad->Name() );
                ownerName = qualifiedOwner.c_str();
            }
            const size_t target =
                findAttribute( layout, simpleName, ownerName );
            if( target != noAttribute ) {
                layout.attributes[target].derived = true;
            }
            continue;
        }
        if( ad->AttrType() == AttrType_Inverse ) {
            continue;
        }

        layout.attributes.push_back( EntityAttributeLayoutEntry( ad ) );
        if( ad->AttrType() == AttrType_Redefining ) {
            const size_t target = findAttribute( layout, simpleName, 0 );
            if( target != noAttribute ) {
                layout.attributes[target].redefining =
                    layout.attributes.size() - 1;
                layout.hasRedefinitions = true;
            }
        }
    }
}

class LateBoundEntity : public SDAI_Application_instance {
public:
    explicit LateBoundEntity( const EntityDescriptor * ed ) {
        eDesc = ed;
    }
};

}

EntityDescriptor::EntityDescriptor( )
    : _abstractEntity( LUnknown ), _extMapping( LUnknown ),
      _uniqueness_rules( ( Uniqueness_rule__set_var )0 ), NewSTEPentity( 0 ) {
}

EntityDescriptor::EntityDescriptor( const char * name, // i.e. char *
                                    Schema * origSchema,
                                    Logical abstractEntity, // F U or T
                                    Logical extMapping,
                                    Creator f
                                  )
    : TypeDescriptor( name, ENTITY_TYPE, origSchema, name ),
      _abstractEntity( abstractEntity ), _extMapping( extMapping ),
      _uniqueness_rules( ( Uniqueness_rule__set_var )0 ), NewSTEPentity( f ) {
}

EntityDescriptor::~EntityDescriptor() {
    layoutCache().Erase( this );
    delete _uniqueness_rules;
}

SDAI_Application_instance * EntityDescriptor::CreateEntity() const {
    if( NewSTEPentity ) {
        return NewSTEPentity();
    }

    PrepareLateBoundLayout();
    EntityAttributeLayout * layout = layoutCache().Find( this );
    LateBoundEntity * entity = new LateBoundEntity( this );
    std::vector<STEPattribute *> attributes;
    if( layout->hasRedefinitions ) {
        attributes.reserve( layout->attributes.size() );
    }
    for( size_t i = 0; i < layout->attributes.size(); ++i ) {
        const EntityAttributeLayoutEntry & entry = layout->attributes[i];
        STEPattribute * attribute = new STEPattribute( *entry.descriptor );
        attribute->set_null();
        if( entry.derived ) {
            attribute->Derive();
        }
        entity->attributes.push( attribute );
        if( layout->hasRedefinitions ) {
            attributes.push_back( attribute );
        }
    }
    if( layout->hasRedefinitions ) {
        for( size_t i = 0; i < layout->attributes.size(); ++i ) {
            const size_t redefining = layout->attributes[i].redefining;
            if( redefining != noAttribute ) {
                attributes[i]->RedefiningAttr( attributes[redefining] );
            }
        }
    }
    return entity;
}

void EntityDescriptor::PrepareLateBoundLayout() const {
    if( layoutCache().Find( this ) ) {
        return;
    }
    EntityAttributeLayout * layout = new EntityAttributeLayout;
    std::set<const EntityDescriptor *> visited;
    appendLayout( this, *layout, visited );
    layoutCache().Insert( this, layout );
}

void EntityDescriptor::InvalidateLateBoundLayout() const {
    layoutCache().Erase( this );

    EntityDescItr subtypes( _subtypes );
    const EntityDescriptor * subtype = 0;
    while( ( subtype = subtypes.NextEntityDesc() ) ) {
        subtype->InvalidateLateBoundLayout();
    }
}

// initialize one inverse attr; used in InitIAttrs, below
void initIAttr( Inverse_attribute * ia, Registry & reg, const char * schNm, const char * name ) {
    const AttrDescriptor * ad;
    const char * aid = ia->inverted_attr_id_();
    const char * eid = ia->inverted_entity_id_();
    const EntityDescriptor * e = reg.FindEntity( eid, schNm );
    AttrDescItr adl( e->ExplicitAttr() );
    while( 0 != ( ad = adl.NextAttrDesc() ) ) {
        if( !strcmp( aid, ad->Name() ) ) {
            ia->inverted_attr_( ad );
            return;
        }
    }
    supertypesIterator sit( e );
    for( ; !sit.empty(); ++sit ) {
        AttrDescItr adi( sit.current()->ExplicitAttr() );
        while( 0 != ( ad = adi.NextAttrDesc() ) ) {
            if( !strcmp( aid, ad->Name() ) ) {
                ia->inverted_attr_( ad );
                return;
            }
        }
    }
    std::cerr << "Inverse attr " << ia->Name() << " for " << name << ": cannot find AttrDescriptor " << aid << " for entity " << eid << "." << std::endl;
    //FIXME should we abort? or is there a sensible recovery path?
    abort();
}

/** initialize inverse attrs
 * call once per eDesc (once per EXPRESS entity type)
 * must be called _after_ init_Sdai* functions for any ia->inverted_entity_id_'s
 *
 */
void EntityDescriptor::InitIAttrs( Registry & reg, const char * schNm ) {
    InverseAItr iai( &( InverseAttr() ) );
    Inverse_attribute * ia;
    while( 0 != ( ia = iai.NextInverse_attribute() ) ) {
        initIAttr( ia, reg, schNm, _name );
    }
}

const char * EntityDescriptor::GenerateExpress( std::string & buf ) const {
    std::string sstr;
    int count;
    int i;
    int all_comments = 1;

    buf = "ENTITY ";
    buf.append( StrToLower( Name(), sstr ) );

    if( strlen( _supertype_stmt.c_str() ) > 0 ) {
        buf.append( "\n  " );
    }
    buf.append( _supertype_stmt );

    const EntityDescriptor * ed = 0;

    EntityDescItr edi_super( _supertypes );
    edi_super.ResetItr();
    ed = edi_super.NextEntityDesc();
    int supertypes = 0;
    if( ed ) {
        buf.append( "\n  SUBTYPE OF (" );
        buf.append( StrToLower( ed->Name(), sstr ) );
        supertypes = 1;
    }
    ed = edi_super.NextEntityDesc();
    while( ed ) {
        buf.append( ",\n\t\t" );
        buf.append( StrToLower( ed->Name(), sstr ) );
        ed = edi_super.NextEntityDesc();
    }
    if( supertypes ) {
        buf.append( ")" );
    }

    buf.append( ";\n" );

    AttrDescItr adi( _explicitAttr );

    adi.ResetItr();
    const AttrDescriptor * ad = adi.NextAttrDesc();

    while( ad ) {
        if( ad->AttrType() == AttrType_Explicit ) {
            buf.append( "    " );
            buf.append( ad->GenerateExpress( sstr ) );
        }
        ad = adi.NextAttrDesc();
    }

    adi.ResetItr();
    ad = adi.NextAttrDesc();

    count = 1;
    while( ad ) {
        if( ad->AttrType() == AttrType_Deriving ) {
            if( count == 1 ) {
                buf.append( "  DERIVE\n" );
            }
            buf.append( "    " );
            buf.append( ad->GenerateExpress( sstr ) );
            count++;
        }
        ad = adi.NextAttrDesc();
    }
    /////////

    InverseAItr iai( &_inverseAttr );

    iai.ResetItr();
    const Inverse_attribute * ia = iai.NextInverse_attribute();

    if( ia ) {
        buf.append( "  INVERSE\n" );
    }

    while( ia ) {
        buf.append( "    " );
        buf.append( ia->GenerateExpress( sstr ) );
        ia = iai.NextInverse_attribute();
    }
    ///////////////
    // count is # of UNIQUE rules
    if( _uniqueness_rules != 0 ) {
        count = _uniqueness_rules->Count();
        for( i = 0; i < count; i++ ) { // print out each UNIQUE rule
            if( !( *( _uniqueness_rules ) )[i]->_label.size() ) {
                all_comments = 0;
            }
        }

        if( all_comments ) {
            buf.append( "  (* UNIQUE *)\n" );
        } else {
            buf.append( "  UNIQUE\n" );
        }
        for( i = 0; i < count; i++ ) { // print out each UNIQUE rule
            if( !( *( _uniqueness_rules ) )[i]->_comment.empty() ) {
                buf.append( "    " );
                buf.append( ( *( _uniqueness_rules ) )[i]->comment_() );
                buf.append( "\n" );
            }
            if( ( *( _uniqueness_rules ) )[i]->_label.size() ) {
                buf.append( "    " );
                buf.append( ( *( _uniqueness_rules ) )[i]->label_() );
                buf.append( "\n" );
            }
        }
    }

    ///////////////
    // count is # of WHERE rules
    if( _where_rules != 0 ) {
        all_comments = 1;
        count = _where_rules->Count();
        for( i = 0; i < count; i++ ) { // print out each UNIQUE rule
            if( !( *( _where_rules ) )[i]->_label.size() ) {
                all_comments = 0;
            }
        }

        if( !all_comments ) {
            buf.append( "  WHERE\n" );
        } else {
            buf.append( "  (* WHERE *)\n" );
        }
        for( i = 0; i < count; i++ ) { // print out each WHERE rule
            if( !( *( _where_rules ) )[i]->_comment.empty() ) {
                buf.append( "    " );
                buf.append( ( *( _where_rules ) )[i]->comment_() );
                buf.append( "\n" );
            }
            if( ( *( _where_rules ) )[i]->_label.size() ) {
                buf.append( "    " );
                buf.append( ( *( _where_rules ) )[i]->label_() );
                buf.append( "\n" );
            }
        }
    }

    buf.append( "END_ENTITY;\n" );

    return const_cast<char *>( buf.c_str() );
}

const char * EntityDescriptor::QualifiedName( std::string & s ) const {
    s.clear();
    EntityDescItr edi( _supertypes );

    int count = 1;
    const EntityDescriptor * ed = edi.NextEntityDesc();
    while( ed ) {
        if( count > 1 ) {
            s.append( "&" );
        }
        s.append( ed->Name() );
        count++;
        ed = edi.NextEntityDesc();
    }
    if( count > 1 ) {
        s.append( "&" );
    }
    s.append( Name() );
    return const_cast<char *>( s.c_str() );
}

const TypeDescriptor * EntityDescriptor::IsA( const TypeDescriptor * td ) const {
    if( td -> NonRefType() == ENTITY_TYPE ) {
        return IsA( ( EntityDescriptor * ) td );
    } else {
        return 0;
    }
}

const EntityDescriptor * EntityDescriptor::IsA( const EntityDescriptor * other )  const {
    const EntityDescriptor * found = 0;
    const EntityDescLinkNode * link = ( const EntityDescLinkNode * )( GetSupertypes().GetHead() );

    if( this == other ) {
        return other;
    } else {
        while( link && ! found )  {
            found = link -> EntityDesc() -> IsA( other );
            link = ( EntityDescLinkNode * ) link -> NextNode();
        }
    }
    return found;
}
