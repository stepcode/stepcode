#ifndef STEPSTRING_H
#define STEPSTRING_H  1

/*
* NIST STEP Core Class Library
* clstepcore/sdaiString.h
* April 1997
* KC Morris

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

#include <scl_export.h>

/* 
* In MSVC2010 std::string::npos is no longer exported from it's libraries.
* This might be a bug but following code make sure the member is known
* and fixes the linker issue for this symbol.
*/
#if defined(__MSVC__) && _MSC_VER == 1600 /* MSVC2010 version */
const std::string::size_type std::string::npos = size_t(-1); 
#endif

class SCL_DAI_EXPORT SDAI_String : public std::string {
    public:

        //constructor(s) & destructor
        SDAI_String( const char * str = 0, int max = 0 ) : std::string( str, max ) { }
        SDAI_String( const std::string & s )   : std::string( s ) { }
        SDAI_String( const SDAI_String & s )  : std::string( s ) { }
        ~SDAI_String() { }

//  operators
        SDAI_String & operator= ( const char * s );

        // format for STEP
        const char * asStr( std::string & s ) const {
            return s.c_str();
        }
        void STEPwrite( ostream & out = cout )  const;
        void STEPwrite( std::string & s ) const;

        Severity StrToVal( const char * s );
        Severity STEPread( istream & in, ErrorDescriptor * err );
        Severity STEPread( const char * s, ErrorDescriptor * err );
};

#endif
