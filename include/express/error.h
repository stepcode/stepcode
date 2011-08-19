#ifndef ERROR_H
#define ERROR_H

/* $Id: error.h,v 1.8 1997/01/21 19:16:55 dar Exp $ */

/************************************************************************
** Module:  Error
** Description: This module implements the ERROR abstraction.
************************************************************************/

/*
 * This work was supported by the United States Government, and is
 * not subject to copyright.
 *
 * $Log: error.h,v $
 * Revision 1.8  1997/01/21 19:16:55  dar
 * made C++ compatible
 * ,.
 *
 * Revision 1.7  1993/10/15  18:49:23  libes
 * CADDETC certified
 *
 * Revision 1.5  1993/02/22  21:44:34  libes
 * ANSI compat fixes
 *
 * Revision 1.4  1992/08/18  17:15:40  libes
 * rm'd extraneous error messages
 *
 * Revision 1.3  1992/06/08  18:07:35  libes
 * prettied up interface to print_objects_when_running
 */

#include "basic.h"  /* get basic definitions */
#include "setjmp.h"

/*************/
/* constants */
/*************/

#define ERROR_none      (Error)NULL
#define ERROR_MAX       100

/*****************/
/* packages used */
/*****************/

#include "memory.h"
#include "symbol.h"

/************/
/* typedefs */
/************/

typedef enum {
    SEVERITY_WARNING    = 0,
    SEVERITY_ERROR  = 1,
    SEVERITY_EXIT   = 2,
    SEVERITY_DUMP   = 3,
    SEVERITY_MAX    = 4
} Severity;

/***************************/
/* hidden type definitions */
/***************************/

typedef struct Error_ {
    bool enabled;
    Severity    severity;
    char  * message;
} * Error;

typedef struct Error_Warning_ {
    char  * name;
    struct Linked_List_ * errors;
} * Error_Warning;

/****************/
/* modules used */
/****************/

/********************/
/* global variables */
/********************/

#ifdef ERROR_C
#include "defstart.h"
#else
#include "decstart.h"
#endif /* ERROR_C */

GLOBAL bool  __ERROR_buffer_errors       INITIALLY( false );
GLOBAL char * current_filename           INITIALLY( "stdin" );

/* flag to remember whether non-warning errors have occurred */
GLOBAL bool  ERRORoccurred           INITIALLY( false );


GLOBAL Error    experrc             INITIALLY( ERROR_none );
GLOBAL Error    ERROR_subordinate_failed    INITIALLY( ERROR_none );
GLOBAL Error    ERROR_syntax_expecting      INITIALLY( ERROR_none );

/* all of these are 1 if true, 0 if false switches */
/* for debugging fedex */
GLOBAL int  ERRORdebugging          INITIALLY( 0 );
/* for debugging malloc during resolution */
GLOBAL int  malloc_debug_resolve        INITIALLY( 0 );
/* for debugging yacc/lex */
GLOBAL int  debug               INITIALLY( 0 );

GLOBAL struct Linked_List_ * ERRORwarnings;
GLOBAL struct freelist_head ERROR_OPT_fl;

GLOBAL void ( *ERRORusage_function ) PROTO( ( void ) );

#include "de_end.h"

/******************************/
/* macro function definitions */
/******************************/

#define ERROR_OPT_new() (struct Error_Warning_ *)MEM_new(&ERROR_OPT_fl)
#define ERROR_OPT_destroy(x)    MEM_destroy(&ERROR_OPT_fl,(Freelist *)(Generic)x)


/********************/
/* Inline functions */
/********************/

#if supports_inline_functions || defined(ERROR_C)

static_inline
void
ERRORdisable( Error error ) {
    if( error != ERROR_none ) {
        error->enabled = false;
    }
}

static_inline
void
ERRORenable( Error error ) {
    if( error != ERROR_none ) {
        error->enabled = true;
    }
}

static_inline
bool
ERRORis_enabled( Error error ) {
    return error->enabled;
}

static_inline
void
ERRORbuffer_messages( bool flag ) {
    extern void ERROR_start_message_buffer( void ),
           ERROR_flush_message_buffer( void );

    __ERROR_buffer_errors = flag;
    if( __ERROR_buffer_errors ) {
        ERROR_start_message_buffer();
    } else {
        ERROR_flush_message_buffer();
    }
}

static_inline
void
ERRORflush_messages( void ) {
    extern void ERROR_start_message_buffer( void ),
           ERROR_flush_message_buffer( void );

    if( __ERROR_buffer_errors ) {
        ERROR_flush_message_buffer();
        ERROR_start_message_buffer();
    }
}

#endif /*supports_inline_functions || defined(ERROR_C)*/

/***********************/
/* function prototypes */
/***********************/

extern void ERRORinitialize PROTO( ( void ) );
extern void ERRORinitialize_after_LIST PROTO( ( void ) );
extern void ERRORnospace PROTO( ( void ) );
extern void ERRORabort PROTO( ( int ) );
extern Error    ERRORcreate PROTO( ( char *, Severity ) );
extern void ERRORreport PROTO( ( Error, ... ) );
/*SUPPRESS 652*/  /* 1.? */
/*SUPPRESS 842*/  /* 4.0.2 */
struct Symbol_; /* mention Symbol to avoid warning on following line */
extern void ERRORreport_with_symbol PROTO( ( Error, struct Symbol_ *, ... ) );
extern void ERRORreport_with_line PROTO( ( Error, int, ... ) );
extern void ERRORbuffer_messages PROTO( ( bool ) );
extern void ERRORflush_messages PROTO( ( void ) );

extern void ERROR_start_message_buffer PROTO( ( void ) );
extern void ERROR_flush_message_buffer PROTO( ( void ) );

extern void ERRORcreate_warning PROTO( ( char *, Error ) );
extern void ERRORset_warning PROTO( ( char *, int ) );
extern void ERRORset_all_warnings PROTO( ( int ) );
extern void ERRORsafe PROTO( ( jmp_buf env ) );
extern void ERRORunsafe PROTO( ( void ) );

#if deprecated
extern void ERRORdisable PROTO( ( Error ) );
extern void ERRORenable PROTO( ( Error ) );
extern Boolean  ERRORis_enabled PROTO( ( Error ) );
#endif

#endif /* ERROR_H */
