#ifndef SDAIBINARY_H
#define SDAIBINARY_H 1

#include <scl_export.h>
#include <string>

/*
* NIST STEP Core Class Library
* clstepcore/sdaiBinary.h
* April 1997
* KC Morris

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

class SCL_DAI_EXPORT SDAI_Binary : public std::string
{
  public:

    //constructor(s) & destructor    
    SDAI_Binary (const char * str = 0, int max =0) 
      : std::string (str,max) { }

    SDAI_Binary (const std::string& s)   : std::string (s) { }


    ~SDAI_Binary ()  {  }

    //  operators
    SDAI_Binary& operator= (const char* s);

    // format for STEP
    const char * asStr () const  {  return c_str();  }
    void STEPwrite (ostream& out =cout)  const;
    const char * STEPwrite (std::string &s) const;

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
