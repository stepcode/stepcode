
/*
* NIST STEP Core Class Library
* clstepcore/sdaiString.cc
* April 1997
* KC Morris

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

#include <sdai.h>
#include <sstream>

SDAI_String::SDAI_String( const char * str, int max ) {
    content = std::string( str, max );
}

SDAI_String::SDAI_String( const std::string & s ) {
    content = std::string( s );
}

SDAI_String::SDAI_String( const SDAI_String & s ) {
    content = std::string( s.c_str() );
}

SDAI_String::~SDAI_String( void ) {
}

SDAI_String & SDAI_String::operator= ( const char * s ) {
    content = std::string( s );
    return *this;
}

bool SDAI_String::operator== ( const char * s ) const {
    return ( content == s );
}

void SDAI_String::clear( void ) {
    content.clear();
}

bool SDAI_String::empty( void ) const {
    return content.empty();
}

const char * SDAI_String::c_str( void ) const {
    return content.c_str();
}


void SDAI_String::STEPwrite( ostream & out ) const {
    out << c_str();
}

void SDAI_String::STEPwrite( std::string & s ) const {
    s += c_str();
}

Severity SDAI_String::StrToVal( const char * s ) {
    operator= ( s );
    if( ! strcmp( c_str(),  s ) ) {
        return SEVERITY_NULL ;
    } else {
        return SEVERITY_INPUT_ERROR;
    }
}

/**
 * STEPread reads a string in exchange file format
 * starting with a single quote
 */
Severity SDAI_String::STEPread( istream & in, ErrorDescriptor * err ) {
    clear();  // clear the old string
    // remember the current format state to restore the previous settings
    ios_base::fmtflags flags = in.flags();
    in.unsetf( ios::skipws );

    // extract the string from the inputstream
    string s = ToExpressStr( in, err );
    content += s;

    // retrieve current severity
    Severity sev = err -> severity();

    // Not missing closing quote on string value
    if( sev != SEVERITY_INPUT_ERROR && s.compare( "" ) != 0 ) {
        sev = SEVERITY_NULL;
    }

    // There was no quote
    if( !( sev == SEVERITY_INPUT_ERROR || sev == SEVERITY_NULL ) ) {
        in.flags( flags ); // set the format state back to previous settings
        clear();
        err -> GreaterSeverity( SEVERITY_INCOMPLETE );
        sev = SEVERITY_INCOMPLETE;
    }
    return sev;
}

/**
 * \copydoc STEPread( istream & in, ErrorDescriptor * err )
 */
Severity SDAI_String::STEPread( const char * s, ErrorDescriptor * err ) {
    istringstream in( ( char * )s );
    return STEPread( in, err );
}
