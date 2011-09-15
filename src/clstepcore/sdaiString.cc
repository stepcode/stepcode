
/*
* NIST STEP Core Class Library
* clstepcore/sdaiString.cc
* April 1997
* KC Morris

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

/* $Id: sdaiString.cc,v 1.4 1997/11/05 21:59:17 sauderd DP3.1 $ */

#include <sdai.h>
#include <sstream>


SCLP23( String ) &
SCLP23( String )::operator= ( const char * s ) {
    std::string::operator= ( s );
    return *this;
}

void
SCLP23( String )::STEPwrite( ostream & out ) const {
    out << c_str();
}

void
SCLP23( String )::STEPwrite( std::string & s ) const {
    s += c_str();
}

Severity
SCLP23( String )::StrToVal( const char * s ) {
    operator= ( s );
    if( ! strcmp( c_str(),  s ) ) {
        return SEVERITY_NULL ;
    } else {
        return SEVERITY_INPUT_ERROR;
    }
}

//  STEPread reads a string in exchange file format
//  starting with a single quote
Severity
SCLP23( String )::STEPread( istream & in, ErrorDescriptor * err ) {
    clear();  // clear the old string
    // remember the current format state to restore the previous settings
#if defined(__GNUC__) && (__GNUC__ > 2)
    ios_base::fmtflags flags = in.flags();
#else
    ios::fmtflags flags = in.flags();
#endif
    in.unsetf( ios::skipws );

    // extract the string from the inputstream
    string s = ToExpressStr(in, err);
    operator+= (s);

    // retrieve current severity
    Severity sev = err -> severity();

    // Not missing closing quote on string value
    if (sev != SEVERITY_INPUT_ERROR && s.compare("") != 0) {
        sev = SEVERITY_NULL;
    }

    // There was no quote
    if ( !(sev == SEVERITY_INPUT_ERROR || sev == SEVERITY_NULL) ) {
            in.flags( flags ); // set the format state back to previous settings
            clear();
            err -> GreaterSeverity( SEVERITY_INCOMPLETE );
            sev = SEVERITY_INCOMPLETE;
    }
    return sev;
}

Severity
SCLP23( String )::STEPread( const char * s, ErrorDescriptor * err ) {
    istringstream in( ( char * )s );
    return STEPread( in, err );
}
