
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

#include <ctype.h>

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <errordesc.h>


//StrCmpIns - case-insensitive string compare. Original fct (replaced Sep 2011)
//called PrettyTmpName(). Not doing so may affect sort order but that shouldn't hurt
#ifdef _MSC_VER
    #define StrCmpIns(a,b) _stricmp(a,b)
#else
    #define StrCmpIns(a,b) strcasecmp(a,b)
#endif

char         ToLower (const char c);
char         ToUpper  (const char c);
char *       StrToLower (const char *, char *);
const char * StrToLower (const char * word, std::string &s);
const char * StrToUpper (const char * word, std::string &s);
const char * StrToConstant (const char * word, std::string &s);
const char * PrettyTmpName (const char * oldname);
char *       PrettyNewName (const char * oldname);
char *       EntityClassName ( char * oldname);

std::string  ToExpressStr (istream &in, ErrorDescriptor *err);

extern Severity CheckRemainingInput
   (istream &in, ErrorDescriptor *err, 
    const char *typeName, // used in error message
    const char *tokenList); // e.g. ",)"


#endif 
