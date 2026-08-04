#include "clstepcore/enumTypeDescriptor.h"

#include <cctype>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

typedef std::vector<std::string> EnumElements;
typedef std::unordered_map<const EnumTypeDescriptor *, EnumElements>
    EnumElementMap;

EnumElementMap & enumElementMap() {
    static EnumElementMap elements;
    return elements;
}

std::mutex & enumElementMutex() {
    static std::mutex mutex;
    return mutex;
}

EnumElements parseEnumDescription( const char * description ) {
    EnumElements result;
    if( !description ) {
        return result;
    }
    const char * begin = strchr( description, '(' );
    const char * end = strrchr( description, ')' );
    if( !begin || !end || begin >= end ) {
        return result;
    }
    ++begin;
    while( begin < end ) {
        while( begin < end &&
                ( std::isspace( static_cast<unsigned char>( *begin ) ) ||
                  *begin == ',' ) ) {
            ++begin;
        }
        const char * itemEnd = begin;
        while( itemEnd < end && *itemEnd != ',' ) {
            ++itemEnd;
        }
        const char * trimmed = itemEnd;
        while( trimmed > begin &&
                std::isspace( static_cast<unsigned char>( trimmed[-1] ) ) ) {
            --trimmed;
        }
        if( trimmed > begin ) {
            std::string item( begin, trimmed );
            for( size_t i = 0; i < item.size(); ++i ) {
                item[i] = static_cast<char>( std::toupper(
                    static_cast<unsigned char>( item[i] ) ) );
            }
            result.push_back( item );
        }
        begin = itemEnd + ( itemEnd < end ? 1 : 0 );
    }
    return result;
}

EnumElements descriptorElements( const EnumTypeDescriptor & descriptor ) {
    std::lock_guard<std::mutex> lock( enumElementMutex() );
    EnumElementMap::const_iterator i = enumElementMap().find( &descriptor );
    if( i != enumElementMap().end() ) {
        return i->second;
    }
    return parseEnumDescription( descriptor.Description() );
}

class DescriptorEnum : public SDAI_Enum {
    std::string _name;
    EnumElements _elements;

public:
    explicit DescriptorEnum( const EnumTypeDescriptor & descriptor )
        : _name( descriptor.Name() ),
          _elements( descriptorElements( descriptor ) ) {
        nullify();
    }

    virtual int no_elements() const {
        return static_cast<int>( _elements.size() );
    }

    virtual const char * Name() const {
        return _name.c_str();
    }

    virtual const char * element_at( int index ) const {
        return index >= 0 && static_cast<size_t>( index ) < _elements.size() ?
            _elements[index].c_str() : "UNSET";
    }
};

}

/*
 * why have EnumTypeDescriptor + EnumerationTypeDescriptor ???
 * this was in ExpDict.cc before splitting it up
 */
#ifdef NOT_YET
EnumerationTypeDescriptor::EnumerationTypeDescriptor( ) {
    _elements = new StringAggregate;
}
#endif

EnumTypeDescriptor::EnumTypeDescriptor( const char * nm, PrimitiveType ft,
                                        Schema * origSchema,
                                        const char * d, EnumCreator f )
    : TypeDescriptor( nm, ft, origSchema, d ), CreateNewEnum( f ) {
}

SDAI_Enum * EnumTypeDescriptor::CreateEnum() const {
    if( CreateNewEnum ) {
        return CreateNewEnum();
    }
    if( NonRefType() == sdaiBOOLEAN ) {
        return new SDAI_BOOLEAN;
    }
    if( NonRefType() == sdaiLOGICAL ) {
        return new SDAI_LOGICAL;
    }
    return new DescriptorEnum( *this );
}

EnumTypeDescriptor::~EnumTypeDescriptor() {
    std::lock_guard<std::mutex> lock( enumElementMutex() );
    enumElementMap().erase( this );
}

void RegisterEnumDescriptorElements(
    const EnumTypeDescriptor & descriptor,
    const char * const * elements, size_t count ) {
    EnumElements values;
    values.reserve( count );
    for( size_t i = 0; i < count; ++i ) {
        values.push_back( elements[i] ? elements[i] : "" );
    }
    std::lock_guard<std::mutex> lock( enumElementMutex() );
    enumElementMap()[&descriptor].swap( values );
}

const char * EnumTypeDescriptor::GenerateExpress( std::string & buf ) const {
    char tmp[BUFSIZ+1];
    buf = "TYPE ";
    buf.append( StrToLower( Name(), tmp ) );
    buf.append( " = ENUMERATION OF \n  (" );
    const char * desc = Description();
    const char * ptr = &( desc[16] );

    while( *ptr != '\0' ) {
        if( *ptr == ',' ) {
            buf.append( ",\n  " );
        } else if( isupper( *ptr ) ) {
            buf += ( char )tolower( *ptr );
        } else {
            buf += *ptr;
        }
        ptr++;
    }
    buf.append( ";\n" );
    ///////////////
    // count is # of WHERE rules
    if( _where_rules != 0 ) {
        int all_comments = 1;
        int count = _where_rules->Count();
        for( int i = 0; i < count; i++ ) { // print out each UNIQUE rule
            if( !( *( _where_rules ) )[i]->_label.size() ) {
                all_comments = 0;
            }
        }

        if( all_comments ) {
            buf.append( "  (* WHERE *)\n" );
        } else {
            buf.append( "  WHERE\n" );
        }

        for( int i = 0; i < count; i++ ) { // print out each WHERE rule
            if( !( *( _where_rules ) )[i]->_comment.empty() ) {
                buf.append( "    " );
                buf.append( ( *( _where_rules ) )[i]->comment_() );
            }
            if( ( *( _where_rules ) )[i]->_label.size() ) {
                buf.append( "    " );
                buf.append( ( *( _where_rules ) )[i]->label_() );
            }
        }
    }

    buf.append( "END_TYPE;\n" );
    return const_cast<char *>( buf.c_str() );
}
