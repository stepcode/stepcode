#ifndef READ_FUNC_H
#define	READ_FUNC_H

#include <sdai.h>

#define MAX_COMMENT_LENGTH 512

// print Error information for debugging purposes
extern void PrintErrorState(ErrorDescriptor &err);

// print istream error information for debugging purposes
extern void IStreamState(istream &in);

extern int 
ReadInteger(SCLP23(Integer) &val, istream &in, ErrorDescriptor *err, 
	    const char *tokenList);

extern int 
ReadInteger(SCLP23(Integer) &val, const char *s, ErrorDescriptor *err, 
	    const char *tokenList);

extern Severity 
IntValidLevel (const char *attrValue, ErrorDescriptor *err,
	       int clearError, int optional, char *tokenList);

extern char * 
WriteReal(SCLP23(Real) val, std::string &s);

extern void
WriteReal(SCLP23(Real) val, ostream &out);

extern int
ReadReal(SCLP23(Real) &val, istream &in, ErrorDescriptor *err, 
	 const char *tokenList);

extern int
ReadReal(SCLP23(Real) &val, const char *s, ErrorDescriptor *err, 
	 const char *tokenList);

extern Severity 
RealValidLevel (const char *attrValue, ErrorDescriptor *err,
		int clearError, int optional, char *tokenList);

extern int
ReadNumber(SCLP23(Real) &val, istream &in, ErrorDescriptor *err, 
	   const char *tokenList);

extern int
ReadNumber(SCLP23(Real) &val, const char *s, ErrorDescriptor *err, 
	   const char *tokenList);

extern Severity 
NumberValidLevel (const char *attrValue, ErrorDescriptor *err,
		  int clearError, int optional, char *tokenList);


////////////////////

extern int   QuoteInString(istream& in);

extern void 
PushPastString (istream& in, std::string &s, ErrorDescriptor *err);

extern void 
PushPastImbedAggr (istream& in, std::string &s, ErrorDescriptor *err);

extern void 
PushPastAggr1Dim(istream& in, std::string &s, ErrorDescriptor *err);

////////////////////

extern Severity 
FindStartOfInstance(istream& in, std::string&  inst);

	//  used for instances that aren\'t valid - reads to next \';\'
extern Severity 
SkipInstance (istream& in, std::string & inst);

extern const char *
SkipSimpleRecord(istream &in, std::string &buf, ErrorDescriptor *err);

 // this includes entity names
extern const char *
ReadStdKeyword(istream& in, std::string &buf, int skipInitWS = 1);

extern const char* 
GetKeyword(istream& in, const char* delims, ErrorDescriptor &err);

extern int 
FoundEndSecKywd(istream& in, ErrorDescriptor &err);

extern const char *ReadComment(std::string &ss, const char *s);

extern const char *ReadComment(istream& in, std::string &s);

extern Severity    ReadPcd(istream& in);   //print control directive

extern void        ReadTokenSeparator(istream& in, std::string *comments = 0);

#endif
