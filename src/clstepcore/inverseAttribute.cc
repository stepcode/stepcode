#include "inverseAttribute.h"
#include <string>

const char * Inverse_attribute::AttrExprDefStr( std::string & s ) const {
    std::string buf;

    s = Name();
    s.append( " : " );
    if( _optional.asInt() == LTrue ) {
        s.append( "OPTIONAL " );
    }
    if( DomainType() ) {
        s.append( DomainType()->AttrTypeName( buf ) );
    }
    s.append( " FOR " );
    s.append( _inverted_attr_id );
    return const_cast<char *>( s.c_str() );
}
