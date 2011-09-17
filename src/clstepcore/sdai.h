#ifndef P23SDAI_H
#define	P23SDAI_H

/*
* NIST STEP Core Class Library
* clstepcore/sdai.h
* April 1997
* David Sauder
* KC Morris

* Development of this software was funded by the United States Government,
* and is not subject to copyright.

* This file is included to support the proposed SDAI/C++ Language Binding
* specified in Annex D of STEP Part 22.

* This file specifies the appropriate naming conventions and NULL
* values for the EXPRESS base types.
*/

#include <scl_cf.h>

extern const char *SCLversion;

#include <ctype.h>
#include <string>

#include <sclprefixes.h>
#include <dictdefs.h>

#include <baseType.h>
#include <Str.h>
#include <errordesc.h>

typedef std::string Express_id;

class TypeDescriptor;
class EntityDescriptor;
class SelectTypeDescriptor;
class InstMgr;

#include <STEPattributeList.h>

class STEPattributeList;
class STEPattribute;

#ifndef BINARY_DELIM
#define BINARY_DELIM '\"'
#endif

#ifndef STRING_DELIM
#define STRING_DELIM '\''
#endif

	//  STRING
#define S_STRING_NULL	""

#ifndef CC_NO_NESTED_CLASSES
struct P23_NAMESPACE {
#endif // CC_NO_NESTED_CLASSES

typedef long           SCLP23_NAME(Integer);
typedef short          SCLP23_NAME(Short);
typedef unsigned short SCLP23_NAME(UShort);
typedef unsigned long  SCLP23_NAME(ULong);
typedef double         SCLP23_NAME(Real);

// C++ from values.h DAS PORT
#ifdef CC_NO_NESTED_CLASSES
   extern const SCLP23_NAME(Integer) SCLP23_NAME(INT_NULL);
   extern const SCLP23_NAME(Real) SCLP23_NAME(REAL_NULL);
	// arbitrary choice by me for number DAS
   extern const SCLP23_NAME(Real) SCLP23_NAME(NUMBER_NULL);
#else
 #ifdef PART26
   extern const SCLP23_NAME(Integer) SCLP23_NAME(INT_NULL);
   extern const SCLP23_NAME(Real) SCLP23_NAME(REAL_NULL);
	// arbitrary choice by me for number DAS
   extern const SCLP23_NAME(Real) SCLP23_NAME(NUMBER_NULL);
 #else
   static const SCLP23_NAME(Integer) SCLP23_NAME(INT_NULL);
   static const SCLP23_NAME(Real) SCLP23_NAME(REAL_NULL);
	// arbitrary choice by me for number DAS
   static const SCLP23_NAME(Real) SCLP23_NAME(NUMBER_NULL);
 #endif
#endif

enum SCLP23_NAME(Access_type) {
  SCLP23_NAME(sdaiRO), 
  SCLP23_NAME(sdaiRW), 
  SCLP23_NAME(Access_type_unset)
};

enum SCLP23_NAME(Commit_mode) {
  SCLP23_NAME(sdaiCOMMIT), 
  SCLP23_NAME(sdaiABORT)
} ;

enum SCLP23_NAME(AttrFlag) { 
  SCLP23_NAME(sdaiSET), 
SCLP23_NAME(sdaiUNSET) 
} ;

enum SCLP23_NAME(ImplementationClass)  { 
  SCLP23_NAME(sdaiCLASS1), 
  SCLP23_NAME(sdaiCLASS2), 
  SCLP23_NAME(sdaiCLASS3), 
  SCLP23_NAME(sdaiCLASS4), 
  SCLP23_NAME(sdaiCLASS5) 
};  // conflict with part 22 EXPRESS:

enum SCLP23_NAME(Error_id) {
    //
    // error codes taken from 10303-23, Jan 28, 1997.
    // ISO TC184/SC4/WG11 N 004
    //
  SCLP23_NAME(sdaiNO_ERR)  =    0,   // No error 
  SCLP23_NAME(sdaiSS_OPN)  =   10,   // Session open
  SCLP23_NAME(sdaiSS_NAVL) =   20,   // Session not available 
  SCLP23_NAME(sdaiSS_NOPN) =   30,   // Session is not open
  SCLP23_NAME(sdaiRP_NEXS) =   40,   // Repository does not exist 
  SCLP23_NAME(sdaiRP_NAVL) =   50,   // Repository not available 
  SCLP23_NAME(sdaiRP_OPN)  =   60,   // Repository already opened 
  SCLP23_NAME(sdaiRP_NOPN) =   70,   // Repository is not open 
  SCLP23_NAME(sdaiTR_EAB)  =   80,   // Transaction ended abnormally so it no longer exists
  SCLP23_NAME(sdaiTR_EXS)  =   90,   // Transaction exists
  SCLP23_NAME(sdaiTR_NAVL) =  100,   // Transaction not available
  SCLP23_NAME(sdaiTR_RW)   =  110,   // Transaction read-write
  SCLP23_NAME(sdaiTR_NRW)  =  120,   // Transaction not read-write
  SCLP23_NAME(sdaiTR_NEXS) =  130,   // Transaction does not exist
  SCLP23_NAME(sdaiMO_NDEQ) =  140,   // SDAI-model not domain-equivalent 
  SCLP23_NAME(sdaiMO_NEXS) =  150,   // SDAI-model does not exist 
  SCLP23_NAME(sdaiMO_NVLD) =  160,   // SDAI-model invalid
  SCLP23_NAME(sdaiMO_DUP)  =  170,   // SDAI-model duplicate
  SCLP23_NAME(sdaiMX_NRW)  =  180,   // SDAI-model access not read-write
  SCLP23_NAME(sdaiMX_NDEF) =  190,   // SDAI-model access is not defined
  SCLP23_NAME(sdaiMX_RW)   =  200,   // SDAI-model access read-write
  SCLP23_NAME(sdaiMX_RO)   =  210,   // SDAI-model access read-only
  SCLP23_NAME(sdaiSD_NDEF) =  220,   // Schema definition is not defined
  SCLP23_NAME(sdaiED_NDEF) =  230,   // Entity definition not defined
  SCLP23_NAME(sdaiED_NDEQ) =  240,   // Entity definition not domain equivalent
  SCLP23_NAME(sdaiED_NVLD) =  250,   // Entity definition invalid
//  SCLP23_NAME(sdaiED_NAVL) =  250,   // Entity definition not available
  SCLP23_NAME(sdaiRU_NDEF) =  260,   // Rule not defined 
  SCLP23_NAME(sdaiEX_NSUP) =  270,   // Expression evaluation not supported 
  SCLP23_NAME(sdaiAT_NVLD) =  280,   // Attribute invalid
  SCLP23_NAME(sdaiAT_NDEF) =  290,   // Attribute not defined
  SCLP23_NAME(sdaiSI_DUP)  =  300,   // Schema instance duplicate
  SCLP23_NAME(sdaiSI_NEXS) =  310,   // Schema instance does not exist
  SCLP23_NAME(sdaiEI_NEXS) =  320,   // Entity instance does not exist 
  SCLP23_NAME(sdaiEI_NAVL) =  330,   // Entity instance not available
  SCLP23_NAME(sdaiEI_NVLD) =  340,   // Entity instance invalid
  SCLP23_NAME(sdaiEI_NEXP) =  350,   // Entity instance not exported
  SCLP23_NAME(sdaiSC_NEXS) =  360,   // Scope does not exist 
  SCLP23_NAME(sdaiSC_EXS) =  370,   // Scope exists 
  SCLP23_NAME(sdaiAI_NEXS) =  380,   // Aggregate instance does not exist 
  SCLP23_NAME(sdaiAI_NVLD) =  390,   // Aggregate instance invalid 
  SCLP23_NAME(sdaiAI_NSET) =  400,   // Aggregate instance is empty 
  SCLP23_NAME(sdaiVA_NVLD) =  410,   // Value invalid
  SCLP23_NAME(sdaiVA_NEXS) =  420,   // Value does not exist
  SCLP23_NAME(sdaiVA_NSET) =  430,   // Value not set 
  SCLP23_NAME(sdaiVT_NVLD) =  440,   // Value type invalid
  SCLP23_NAME(sdaiIR_NEXS) =  450,   // Iterator does not exist 
  SCLP23_NAME(sdaiIR_NSET) =  460,   // Current member is not defined
  SCLP23_NAME(sdaiIX_NVLD) =  470,   // Index invalid
  SCLP23_NAME(sdaiER_NSET) =  480,   // Event recording not set
  SCLP23_NAME(sdaiOP_NVLD) =  490,   // Operator invalid 
  SCLP23_NAME(sdaiFN_NAVL) =  500,   // Function not available
  SCLP23_NAME(sdaiSY_ERR)  = 1000    // Underlying system error 
};

typedef char * SCLP23_NAME(Time_stamp);
typedef char * SCLP23_NAME(Entity_name);
typedef char * SCLP23_NAME(Schema_name);

#include <sdaiString.h>

#include <sdaiBinary.h>

// define Object which I am calling sdaiObject for now - DAS
#include <sdaiObject.h>

/******************************************************************************
ENUMERATION
    These types do not use the interface specified.
    The interface used is to enumerated values.  The enumerations are
    mapped in the following way:
    *  enumeration-item-from-schema ==> enumeration item in c++ enum clause
    *  all enumeration items in c++ enum are in upper case 
    *  the value ENUM_NULL is used to represent NULL for all enumerated types 
 *****************************************************************************/

#include <sdaiEnum.h>

/******************************************************************************
BOOLEAN and LOGICAL

    Logical are implemented as an enumeration in sdaiEnum.h:

    Boolean are implemented as an enumeration in sdaiEnum.h:

******************************************************************************/

// ***note*** this file needs classes from sdaiEnum.h
// define DAObjectID and classes PID, PID_DA, PID_SDAI, DAObject, DAObject_SDAI
#include <sdaiDaObject.h>


#include <sdaiApplication_instance.h>
#include <sdaiApplication_instance_set.h>

/******************************************************************************
SELECT

    Selects are represented as subclasses of the SCLP23(Select) class in 
    sdaiSelect.h

******************************************************************************/
#include <sdaiSelect.h>

//typedef char * SCLP23_NAME(Entity_name);

class SCLP23_NAME(Model_contents);
typedef SCLP23_NAME(Model_contents) * SCLP23_NAME(Model_contents_ptr);
typedef SCLP23_NAME(Model_contents_ptr) SCLP23_NAME(Model_contents_var);

#include <sdaiModel_contents_list.h>

#include <sdaiSession_instance.h>
#include <sdaiEntity_extent.h>
#include <sdaiEntity_extent_set.h>
#include <sdaiModel_contents.h>

#ifndef CC_NO_NESTED_CLASSES
#ifndef PART26
}; // end struct P23_NAMESPACE
#endif // PART26
#endif // CC_NO_NESTED_CLASSES

#ifdef PART26

//#define SDAI_DEF_TIE_P26_Application_instance(x) DEF_TIE_P26_Application_instance(x)

#define Application_instance_i SCLP23(Application_instance)
DEF_TIE_IDL_Application_instance(Application_instance_i)
//DEF_TIE_P26_Application_instance(Application_instance_i)
//DEF_TIE_IDL_Application_instance(SCLP23(Application_instance))
#endif

	//  ENTITY
extern SCLP23(Application_instance) NilSTEPentity;
#define ENTITY_NULL	&NilSTEPentity
#define NULL_ENTITY	&NilSTEPentity
#define S_ENTITY_NULL	&NilSTEPentity


//#define STEPentity SCLP23_NAME(Application_instance);
typedef SCLP23(Application_instance) STEPentity;
typedef SCLP23(Application_instance)* STEPentity_ptr;
typedef STEPentity_ptr STEPentity_var;

typedef SCLP23(Application_instance)* STEPentityPtr;
typedef SCLP23(Application_instance)* STEPentityH;

extern SCLP23(Application_instance) *
ReadEntityRef(istream &in, ErrorDescriptor *err, const char *tokenList, 
	      InstMgr * instances, int addFileId);

#define SdaiInteger SCLP23(Integer)
#define SdaiReal SCLP23(Real)

#define STEPenumeration SCLP23(Enum)
#define SdaiSelect SCLP23(Select)
#define SdaiString SCLP23(String)
#define SdaiBinary SCLP23(Binary)

#define	S_INT_NULL    SCLP23(INT_NULL)
#define S_REAL_NULL   SCLP23(REAL_NULL)
#define S_NUMBER_NULL SCLP23(NUMBER_NULL)

/******************************************************************************
AGGREGATE TYPES

    Aggregate types are accessed generically.  (There are not seperate
    classes for the different types of aggregates.)  Aggregates are 
    implemented through the STEPaggregate class.

******************************************************************************/

inline SCLP23(BOOLEAN) *create_BOOLEAN() { return new SCLP23(BOOLEAN) ; }

inline SCLP23(LOGICAL) *create_LOGICAL() { return new SCLP23(LOGICAL) ; }

// below is outdated
typedef SCLP23(Select) * SdaiSelectH;

//#define INT_NULL MAXLONG
// C from limits.h
//#define	S_INT_NULL	LONG_MAX 

//#define S_REAL_NULL	10 ** DBL_MAX_10_EXP
//#define FLT_MIN  1E-37  - port 29-Mar-1994 kcm
// C++ from values.h DAS PORT

//#define REAL_NULL MINFLOAT
// C from float.h
//#define S_REAL_NULL FLT_MIN

//#define NUMBER_NULL MINFLOAT
// C from float.h
//#define S_NUMBER_NULL FLT_MIN

#endif
