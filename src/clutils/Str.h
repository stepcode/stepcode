
#ifndef STR_H
#define STR_H

/*
* NIST Utils Class Library
* clutils/Str.h
* April 1997
* K. C. Morris
* David Sauder

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

#include <scl_export.h>
#include <ctype.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errordesc.h>

SCL_UTILS_EXPORT char         ToLower( const char c );
SCL_UTILS_EXPORT char         ToUpper( const char c );
SCL_UTILS_EXPORT char    *    StrToLower( const char *, char * );
SCL_UTILS_EXPORT const char * StrToLower( const char * word, std::string & s );
SCL_UTILS_EXPORT const char * StrToUpper( const char * word, std::string & s );
SCL_UTILS_EXPORT const char * StrToConstant( const char * word, std::string & s );
SCL_UTILS_EXPORT int          StrCmpIns( const char *str1, const char *str2 );
SCL_UTILS_EXPORT const char * PrettyTmpName( const char * oldname );
SCL_UTILS_EXPORT char    *    PrettyNewName( const char * oldname );
SCL_UTILS_EXPORT char    *    EntityClassName( char * oldname );

SCL_UTILS_EXPORT std::string  ToExpressStr( istream & in, ErrorDescriptor * err );

extern SCL_UTILS_EXPORT Severity CheckRemainingInput
( istream & in, ErrorDescriptor * err,
  const char * typeName, // used in error message
  const char * tokenList ); // e.g. ",)"


#endif
