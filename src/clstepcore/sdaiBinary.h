#ifndef SDAIBINARY_H
#define	SDAIBINARY_H 1

/*
* NIST STEP Core Class Library
* clstepcore/sdaiBinary.h
* April 1997
* KC Morris

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

/* $Id: sdaiBinary.h,v */

/*
#ifdef __OSTORE__
#include <ostore/ostore.hh>    // Required to access ObjectStore Class Library
#endif

#ifdef __O3DB__
#include <OpenOODB.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <strstream.h>

class ErrorDescriptor;
#include <scl_string.h>
#include <errordesc.h>

#ifndef BINARY_DELIM
#define BINARY_DELIM '\"'
#endif
*/

class SCLP23_NAME(Binary) : public SCLstring
{
  public:

    //constructor(s) & destructor    
    SCLP23_NAME(Binary) (const char * str = 0, int max =0) 
      : SCLstring (str,max) { }

//Josh L, 3/28/95
//    SCLP23_NAME(Binary) (SCLstring& s)   : SCLstring (s) { }
//    SCLP23_NAME(Binary) (SCLP23_NAME(Binary)& s)  : SCLstring (s) { }
    SCLP23_NAME(Binary) (const SCLstring& s)   : SCLstring (s) { }


    ~SCLP23_NAME(Binary) ()  {  }

    //  operators
    SCLP23_NAME(Binary)& operator= (const char* s);

    // format for STEP
    const char * asStr () const  {  return chars ();  }
    void STEPwrite (ostream& out =cout)  const;
    const char * STEPwrite (SCLstring &s) const;

    Severity StrToVal (const char *s, ErrorDescriptor *err);
    Severity STEPread (istream& in, ErrorDescriptor *err);
    Severity STEPread (const char *s, ErrorDescriptor *err);

    Severity BinaryValidLevel (const char *value, ErrorDescriptor *err,
			       int optional, char *tokenList,
			       int needDelims = 0, int clearError = 1);
    Severity BinaryValidLevel (istream &in, ErrorDescriptor *err, 
			       int optional, char *tokenList,
			       int needDelims = 0, int clearError = 1);

 protected:
  Severity ReadBinary(istream& in, ErrorDescriptor *err, int AssignVal = 1,
		      int needDelims = 1);
};

#endif
