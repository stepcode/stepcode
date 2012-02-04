#ifndef EXPRESS_H
#define EXPRESS_H

/*
 * Express package manager.
 *
 * This software was developed by U.S. Government employees as part of
 * their official duties and is not subject to copyright.
 *
 * $Log: express.h,v $
 * Revision 1.13  1997/01/21 19:16:42  dar
 * made C++ compatible
 *
 * Revision 1.12  1995/04/05  15:09:49  clark
 * CADDETC preval
 *
 * Revision 1.11  1994/11/10  19:20:03  clark
 * Update to IS
 *
 * Revision 1.10  1994/05/11  19:51:05  libes
 * numerous fixes
 *
 * Revision 1.9  1993/10/15  18:48:24  libes
 * CADDETC certified
 *
 * Revision 1.7  1993/02/16  03:19:56  libes
 * added unwriteable error
 *
 * Revision 1.6  1993/01/19  22:44:17  libes
 * *** empty log message ***
 *
 * Revision 1.5  1992/09/16  18:20:10  libes
 * changed user-visible names
 *
 * Revision 1.4  1992/08/18  17:12:41  libes
 * rm'd extraneous error messages
 *
 * Revision 1.3  1992/06/08  18:06:24  libes
 * prettied up interface to print_objects_when_running
 *
 */

/*************/
/* constants */
/*************/

#define EXPRESS_NULL    (struct Scope_ *)0

/*****************/
/* packages used */
/*****************/

#include "expbasic.h"   /* get basic definitions */
#include <string.h>
#include "scope.h"
#include "type.h"
#include "variable.h"
#include "expr.h"
#include "entity.h"
#include "caseitem.h"
#include "stmt.h"
#include "alg.h"
#include "schema.h"
#include "lexact.h"
#include "dict.h"

/************/
/* typedefs */
/************/

typedef struct Scope_ * Express;

/****************/
/* modules used */
/****************/

/***************************/
/* hidden type definitions */
/***************************/

struct Express_ {
    FILE * file;
    char * filename;
    char * basename; /**< name of file but without directory or .exp suffix */
};

/********************/
/* global variables */
/********************/

#ifdef EXPRESS_C
# define GLOBAL
# define INITIALLY(value) = value
#else
# define GLOBAL extern
# define INITIALLY(value)
#endif /*EXPRESS_C*/

#ifdef YYDEBUG
extern SCL_EXPRESS_EXPORT int yydebug;
extern SCL_EXPRESS_EXPORT int yydbg_upper_limit;
extern SCL_EXPRESS_EXPORT int yydbg_lower_limit;
extern SCL_EXPRESS_EXPORT int yydbg_verbose;
#endif

GLOBAL SCL_EXPRESS_EXPORT Linked_List EXPRESS_path;
GLOBAL SCL_EXPRESS_EXPORT int EXPRESSpass;

GLOBAL SCL_EXPRESS_EXPORT void ( *EXPRESSinit_args ) PROTO( ( int, char ** ) )    INITIALLY( 0 );
GLOBAL SCL_EXPRESS_EXPORT void ( *EXPRESSinit_parse ) PROTO( ( void ) )      INITIALLY( 0 );
GLOBAL SCL_EXPRESS_EXPORT int ( *EXPRESSfail ) PROTO( ( Express ) )     INITIALLY( 0 );
GLOBAL SCL_EXPRESS_EXPORT int ( *EXPRESSsucceed ) PROTO( ( Express ) )      INITIALLY( 0 );
GLOBAL SCL_EXPRESS_EXPORT void ( *EXPRESSbackend ) PROTO( ( Express ) )      INITIALLY( 0 );
GLOBAL SCL_EXPRESS_EXPORT char * EXPRESSprogram_name;
extern char   EXPRESSgetopt_options[256];  /**< initialized elsewhere */
GLOBAL SCL_EXPRESS_EXPORT int ( *EXPRESSgetopt ) PROTO( ( int, char * ) )    INITIALLY( 0 );
GLOBAL SCL_EXPRESS_EXPORT bool    EXPRESSignore_duplicate_schemas       INITIALLY( false );

GLOBAL SCL_EXPRESS_EXPORT Dictionary EXPRESSbuiltins;  /**< procedures/functions */

GLOBAL SCL_EXPRESS_EXPORT Error ERROR_bail_out     INITIALLY( ERROR_none );
GLOBAL SCL_EXPRESS_EXPORT Error ERROR_syntax       INITIALLY( ERROR_none );
GLOBAL SCL_EXPRESS_EXPORT Error ERROR_unlabelled_param_type INITIALLY( ERROR_none );
GLOBAL SCL_EXPRESS_EXPORT Error ERROR_file_unreadable;
GLOBAL SCL_EXPRESS_EXPORT Error ERROR_file_unwriteable;
GLOBAL SCL_EXPRESS_EXPORT Error ERROR_warn_unsupported_lang_feat;

GLOBAL SCL_EXPRESS_EXPORT struct Scope_ * FUNC_NVL;
GLOBAL SCL_EXPRESS_EXPORT struct Scope_ * FUNC_USEDIN;

GLOBAL SCL_EXPRESS_EXPORT char * KW_ABS         INITIALLY( "ABS" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_ABSTRACT    INITIALLY( "ABSTRACT" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_ACOS        INITIALLY( "ACOS" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_AGGREGATE   INITIALLY( "AGGREGATE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_ALIAS       INITIALLY( "ALIAS" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_AND         INITIALLY( "AND" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_ANDOR       INITIALLY( "ANDOR" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_ARRAY       INITIALLY( "ARRAY" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_AS          INITIALLY( "AS" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_ASIN        INITIALLY( "ASIN" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_ATAN        INITIALLY( "ATAN" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_BAG         INITIALLY( "BAG" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_BEGIN       INITIALLY( "BEGIN" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_BINARY      INITIALLY( "BINARY" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_BLENGTH     INITIALLY( "BLENGTH" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_BOOLEAN     INITIALLY( "BOOLEAN" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_BY          INITIALLY( "BY" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_CASE        INITIALLY( "CASE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_CONST_E     INITIALLY( "CONST_E" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_CONSTANT    INITIALLY( "CONSTANT" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_CONTEXT     INITIALLY( "CONTEXT" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_COS         INITIALLY( "COS" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_DERIVE      INITIALLY( "DERIVE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_DIV         INITIALLY( "DIV" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_ELSE        INITIALLY( "ELSE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END         INITIALLY( "END" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END_ALIAS   INITIALLY( "END_ALIAS" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END_CASE    INITIALLY( "END_CASE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END_CONSTANT INITIALLY( "END_CONSTANT" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END_CONTEXT INITIALLY( "END_CONTEXT" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END_ENTITY  INITIALLY( "END_ENTITY" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END_FUNCTION INITIALLY( "END_FUNCTION" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END_IF      INITIALLY( "END_IF" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END_LOCAL   INITIALLY( "END_LOCAL" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END_MODEL   INITIALLY( "END_MODEL" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END_PROCEDURE INITIALLY( "END_PROCEDURE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END_REPEAT  INITIALLY( "END_REPEAT" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END_RULE    INITIALLY( "END_RULE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END_SCHEMA  INITIALLY( "END_SCHEMA" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_END_TYPE    INITIALLY( "END_TYPE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_ENTITY      INITIALLY( "ENTITY" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_ENUMERATION INITIALLY( "ENUMERATION" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_ESCAPE      INITIALLY( "ESCAPE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_EXISTS      INITIALLY( "EXISTS" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_EXP         INITIALLY( "EXP" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_FALSE       INITIALLY( "FALSE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_FIXED       INITIALLY( "FIXED" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_FOR         INITIALLY( "FOR" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_FORMAT      INITIALLY( "FORMAT" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_FROM        INITIALLY( "FROM" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_FUNCTION    INITIALLY( "FUNCTION" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_GENERIC     INITIALLY( "GENERIC" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_HIBOUND     INITIALLY( "HIBOUND" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_HIINDEX     INITIALLY( "HIINDEX" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_IF          INITIALLY( "IF" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_IN          INITIALLY( "IN" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_INCLUDE     INITIALLY( "INCLUDE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_INSERT      INITIALLY( "INSERT" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_INTEGER     INITIALLY( "INTEGER" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_INVERSE     INITIALLY( "INVERSE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_LENGTH      INITIALLY( "LENGTH" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_LIKE        INITIALLY( "LIKE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_LIST        INITIALLY( "LIST" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_LOBOUND     INITIALLY( "LOBOUND" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_LOCAL       INITIALLY( "LOCAL" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_LOG         INITIALLY( "LOG" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_LOG10       INITIALLY( "LOG10" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_LOG2        INITIALLY( "LOG2" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_LOGICAL     INITIALLY( "LOGICAL" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_LOINDEX     INITIALLY( "LOINDEX" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_MOD         INITIALLY( "MOD" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_MODEL       INITIALLY( "MODEL" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_NOT         INITIALLY( "NOT" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_NUMBER      INITIALLY( "NUMBER" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_NVL         INITIALLY( "NVL" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_ODD         INITIALLY( "ODD" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_OF          INITIALLY( "OF" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_ONEOF       INITIALLY( "ONEOF" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_OPTIONAL    INITIALLY( "OPTIONAL" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_OR          INITIALLY( "OR" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_OTHERWISE   INITIALLY( "OTHERWISE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_PI          INITIALLY( "PI" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_PROCEDURE   INITIALLY( "PROCEDURE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_QUERY       INITIALLY( "QUERY" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_REAL        INITIALLY( "REAL" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_REFERENCE   INITIALLY( "REFERENCE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_REMOVE      INITIALLY( "REMOVE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_REPEAT      INITIALLY( "REPEAT" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_RETURN      INITIALLY( "RETURN" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_ROLESOF     INITIALLY( "ROLESOF" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_RULE        INITIALLY( "RULE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_SCHEMA      INITIALLY( "SCHEMA" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_SELECT      INITIALLY( "SELECT" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_SELF        INITIALLY( "SELF" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_SET         INITIALLY( "SET" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_SIN         INITIALLY( "SIN" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_SIZEOF      INITIALLY( "SIZEOF" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_SKIP        INITIALLY( "SKIP" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_SQRT        INITIALLY( "SQRT" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_STRING      INITIALLY( "STRING" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_SUBTYPE     INITIALLY( "SUBTYPE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_SUPERTYPE   INITIALLY( "SUPERTYPE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_TAN         INITIALLY( "TAN" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_THEN        INITIALLY( "THEN" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_TO          INITIALLY( "TO" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_TRUE        INITIALLY( "TRUE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_TYPE        INITIALLY( "TYPE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_TYPEOF      INITIALLY( "TYPEOF" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_UNIQUE      INITIALLY( "UNIQUE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_UNKNOWN     INITIALLY( "UNKNOWN" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_UNTIL       INITIALLY( "UNTIL" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_USE         INITIALLY( "USE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_USEDIN      INITIALLY( "USEDIN" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_VALUE       INITIALLY( "VALUE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_VALUE_IN    INITIALLY( "VALUE_IN" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_VALUE_UNIQUE INITIALLY( "VALUE_UNIQUE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_VAR         INITIALLY( "VAR" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_WHERE       INITIALLY( "WHERE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_WHILE       INITIALLY( "WHILE" );
GLOBAL SCL_EXPRESS_EXPORT char * KW_XOR         INITIALLY( "XOR" );

#undef GLOBAL
#undef INITIALLY

/******************************/
/* macro function definitions */
/******************************/

#define EXPRESSget_basename(e)      ((e)->u.express->basename)
#define EXPRESSget_filename(e)      ((e)->u.express->filename)
#define EXPRESSput_basename(e,n)    ((e)->u.express->basename = (n))
#define EXPRESSput_filename(e,n)    ((e)->u.express->filename = (n))

/***********************/
/* function prototypes */
/***********************/

extern SCL_EXPRESS_EXPORT Express  EXPRESScreate PROTO( ( void ) );
extern SCL_EXPRESS_EXPORT void     EXPRESSparse PROTO( ( Express, FILE *, char * ) );
extern SCL_EXPRESS_EXPORT void     EXPRESSinitialize PROTO( ( void ) );
extern SCL_EXPRESS_EXPORT void     EXPRESSresolve PROTO( ( Express ) );
extern SCL_EXPRESS_EXPORT char  *  EXPRESSversion PROTO( ( void ) );
extern SCL_EXPRESS_EXPORT int      EXPRESS_fail PROTO( ( Express ) );
extern SCL_EXPRESS_EXPORT int      EXPRESS_succeed PROTO( ( Express ) );
extern void     EXPRESSinit_init PROTO( ( void ) );
extern SCL_EXPRESS_EXPORT void     build_complex( Express );

#endif /*EXPRESS_H*/
