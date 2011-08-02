#ifndef	STEPUNDEFINED_H
#define	STEPUNDEFINED_H

/*
* NIST STEP Core Class Library
* clstepcore/STEPundefined.h
* April 1997
* KC Morris
* David Sauder

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

/* $Id: STEPundefined.h,v 3.0.1.2 1997/11/05 21:59:28 sauderd DP3.1 $ */

#ifdef __O3DB__
#include <OpenOODB.h>
#endif

#include <errordesc.h>
#include <string>
#include <read_func.h>

class SCLundefined  {
  protected:
    std::string val;
    
  public:
//	INPUT
    virtual Severity StrToVal(const char *s, ErrorDescriptor *err);
    virtual Severity StrToVal(istream &in, ErrorDescriptor *err);

    virtual Severity STEPread(const char *s, ErrorDescriptor *err);
    virtual Severity STEPread(istream &in, ErrorDescriptor *err);

//	OUTPUT
    virtual const char *asStr(std::string& s) const;
    virtual const char *STEPwrite(std::string& s);
    virtual void 	STEPwrite (ostream& out =cout);

    int set_null ();
    int is_null ();
    SCLundefined& operator= (const SCLundefined&); 
    SCLundefined& operator= (const char *str); 
    SCLundefined ();
    virtual ~SCLundefined ();
}
;

#endif
