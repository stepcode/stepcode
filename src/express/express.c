

/************************************************************************
** Express package manager.
************************************************************************/

/*
 * This code was developed with the support of the United States Government,
 * and is not subject to copyright.
 *
 * $Log: express.c,v $
 * Revision 1.20  1997/01/21 19:53:20  dar
 * made C++ compatible
 *
 * Revision 1.18  1995/06/08 22:59:59  clark
 * bug fixes
 *
 * Revision 1.17  1995/05/17  14:25:56  libes
 * Fixed bug in EXPRESS_PATH handling.  Improved debugging diagnostics.
 *
 * Revision 1.16  1995/04/05  13:55:40  clark
 * CADDETC preval
 *
 * Revision 1.15  1995/03/09  18:42:21  clark
 * various fixes for caddetc - before interface clause changes
 *
 * Revision 1.14  1995/02/09  18:00:49  clark
 * update version string
 *
 * Revision 1.13  1994/11/10  19:20:03  clark
 * Update to IS
 *
 * Revision 1.12  1994/05/11  19:51:24  libes
 * numerous fixes
 *
 * Revision 1.11  1993/10/15  18:48:48  libes
 * CADDETC certified
 *
 * Revision 1.9  1993/02/22  21:45:06  libes
 * fix pass print bug
 *
 * Revision 1.8  1993/02/16  03:19:56  libes
 * added unwriteable error
 *
 * Revision 1.7  1993/01/19  22:44:17  libes
 * *** empty log message ***
 *
 * Revision 1.6  1992/09/16  18:19:14  libes
 * fixed bug in EXPRESS_PATH searching
 *
 * Revision 1.5  1992/08/27  23:38:35  libes
 * removed redundant call to initialize much of EXPRESS
 * rewrote schema-rename-connect to use fifo instead of dictionary
 * fixed smashing of file names by new schema files
 *
 * Revision 1.4  1992/08/18  17:13:43  libes
 * rm'd extraneous error messages
 *
 * Revision 1.3  1992/06/08  18:06:57  libes
 * prettied up interface to print_objects_when_running
 *
 * Revision 1.2  1992/05/31  08:35:51  libes
 * multiple files
 *
 * Revision 1.1  1992/05/28  03:55:04  libes
 * Initial revision
 *
 */

#include <scl_cf.h>
#define EXPRESS_C
#include "conf.h"
#include "express/basic.h"
#include <ctype.h>
#include <stdlib.h>
#include <setjmp.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "express/express.h"
#include "express/resolve.h"
#include "stack.h"
#include "express/scope.h"

extern FILE * yyin;
extern Express yyexpresult;

static Error ERROR_ref_nonexistent;
static Error ERROR_tilde_expansion_failed;
static Error ERROR_schema_not_in_own_schema_file;

extern Linked_List PARSEnew_schemas;
void SCOPEinitialize( void );

static Express PARSERrun PROTO( ( char *, FILE * ) );

char * EXPRESSversion( void ) {
    return( "Express Language, IS (N65), October 24, 1994" );
}

SCL_EXPRESS_EXPORT int skip_exp_pause = false;
void exp_pause() {
    if( !skip_exp_pause ) {
        #ifdef __WIN32__
        getchar();
        abort();
        #else
        pause();
        #endif
    }
}

int
EXPRESS_fail( Express model ) {
    ERRORflush_messages();

    if( EXPRESSfail ) {
        return( ( *EXPRESSfail )( model ) );
    }

    fprintf( stderr, "Errors in input\n" );
    return 1;
}

int
EXPRESS_succeed( Express model ) {
    if( EXPRESSsucceed ) {
        return( ( *EXPRESSsucceed )( model ) );
    }

    fprintf( stderr, "No errors in input\n" );
    return 0;
}

Symbol *
EXPRESS_get_symbol( Generic e ) {
    return( &( ( Express )e )->symbol );
}

Express
EXPRESScreate() {
    Express model = SCOPEcreate( OBJ_EXPRESS );
    model->u.express = ( struct Express_ * )calloc( 1, sizeof( struct Express_ ) );
    return model;
}

#define MAX_SCHEMA_FILENAME_SIZE    256
typedef struct Dir {
    char full[MAX_SCHEMA_FILENAME_SIZE];
    char * leaf;
} Dir;

static void
EXPRESS_PATHinit() {
    char * p;
    Dir * dir;
    int done = 0;

    EXPRESS_path = LISTcreate();
    p = getenv( "EXPRESS_PATH" );
    if( !p ) {
        /* if no EXPRESS_PATH, search current directory anyway */
        dir = ( Dir * )malloc( sizeof( Dir ) );
        dir->leaf = dir->full;
        LISTadd( EXPRESS_path, ( Generic )dir );
    } else {
        while( !done ) {
            char * start;   /* start of current dir */
            int length; /* length of dir */
            char * slash;   /* last slash in dir */
            char save;  /* place to character from where we */
            /* temporarily null terminate */

            /* get next directory */
            while( isspace( *p ) ) {
                p++;
            }
            if( *p == '\0' ) {
                break;
            }
            start = p;

            /* find the end of the directory */
            while( *p != '\0' && !isspace( *p ) ) {
                p++;
            }
            save = *p;
            if( *p == 0 ) {
                done = 1;
            } else {
                *p = '\0';
            }
            p++;    /* leave p after terminating null */

            dir = ( Dir * )malloc( sizeof( Dir ) );

            /* if it's just ".", make it as if it was */
            /* just "" to make error messages cleaner */
            if( streq( ".", start ) ) {
                dir->leaf = dir->full;
                LISTadd( EXPRESS_path, ( Generic )dir );
                *( p - 1 ) = save; /* put char back where */
                /* temp null was */
                continue;
            }

            //removed tilde logic; below line is all that's left
            length = ( p - 1 ) - start;

            /* if slash present at end, don't add another */
            slash = strrchr( start, '/' );
            if( slash && ( slash[1] == '\0' ) ) {
                strcpy( dir->full, start );
                dir->leaf = dir->full + length;
            } else {
                sprintf( dir->full, "%s/", start );
                dir->leaf = dir->full + length + 1;
            }
            LISTadd( EXPRESS_path, ( Generic )dir );

            *( p - 1 ) = save; /* put char back where temp null was */
        }
    }
}

void
PASSinitialize() {
    /* inform object system about bit representation */
    /* for handling pass diagnostics */

    OBJcreate( OBJ_PASS, UNK_get_symbol, "pass", OBJ_PASS_BITS );
}

/*
** Procedure:   EXPRESSinitialize
** Parameters:  -- none --
** Returns: void
** Description: Initialize the Express package.
*/

void
EXPRESSinitialize( void ) {
    Function
    func_abs,   func_acos,  func_asin,  func_atan,
                func_blength,
                func_cos,   func_exists,    func_exp,   func_format,
                func_hibound,   func_hiindex,   func_length,    func_lobound,
                func_log,   func_log10, func_log2,  func_loindex,
                func_odd,   func_rolesof,   func_sin,   func_sizeof,
                func_sqrt,  func_tan,   func_typeof,
                func_value, func_value_in,  func_value_unique;
    Procedure
    proc_insert,    proc_remove;

    _MEMinitialize();
    ERRORinitialize();
    OBJinitialize();

    HASHinitialize();   /* comes first - used by just about everything else! */
    DICTinitialize();
    LISTinitialize();   /* ditto */
    ERRORinitialize_after_LIST();
    STACKinitialize();
    PASSinitialize();

    RESOLVEinitialize();

    SYMBOLinitialize();

    SCOPEinitialize();
    TYPEinitialize();   /* cannot come before SCOPEinitialize */
    VARinitialize();

    ALGinitialize();
    ENTITYinitialize();
    SCHEMAinitialize();

    CASE_ITinitialize();
    EXPinitialize();
    /*    LOOP_CTLinitialize();*/
    STMTinitialize();

    SCANinitialize();

    EXPRESSbuiltins = DICTcreate( 35 );
#define funcdef(x,y,c,r) \
            x = ALGcreate(OBJ_FUNCTION);\
            x->symbol.name = y;\
            x->u.func->pcount = c; \
            x->u.func->return_type = r; \
            x->u.func->builtin = true; \
            resolved_all(x); \
            DICTdefine(EXPRESSbuiltins,y,(Generic)x,0,OBJ_FUNCTION);
#define procdef(x,y,c)  x = ALGcreate(OBJ_PROCEDURE);\
            x->symbol.name = y;\
            x->u.proc->pcount = c; \
            x->u.proc->builtin = true; \
            resolved_all(x); \
            DICTdefine(EXPRESSbuiltins,y,(Generic)x,0,OBJ_PROCEDURE);
    /* third arg is # of parameters */

    /* eventually everything should be data-driven, but for now */
    /* uppercase def's are global to allow passing necessary information */
    /* into resolver */
    procdef( proc_insert, KW_INSERT, 3 );
    procdef( proc_remove, KW_REMOVE, 2 );

    funcdef( func_abs,    KW_ABS,    1, Type_Number );
    funcdef( func_acos,   KW_ACOS,   1, Type_Real );
    funcdef( func_asin,   KW_ASIN,   1, Type_Real );
    funcdef( func_atan,   KW_ATAN,   2, Type_Real );
    funcdef( func_blength, KW_BLENGTH, 1, Type_Integer );
    funcdef( func_cos,    KW_COS,    1, Type_Real );
    funcdef( func_exists, KW_EXISTS, 1, Type_Boolean );
    funcdef( func_exp,    KW_EXP,    1, Type_Real );
    funcdef( func_format, KW_FORMAT, 2, Type_String );
    funcdef( func_hibound, KW_HIBOUND, 1, Type_Integer );
    funcdef( func_hiindex, KW_HIINDEX, 1, Type_Integer );
    funcdef( func_length, KW_LENGTH, 1, Type_Integer );
    funcdef( func_lobound, KW_LOBOUND, 1, Type_Integer );
    funcdef( func_log,    KW_LOG,    1, Type_Real );
    funcdef( func_log10,  KW_LOG10,  1, Type_Real );
    funcdef( func_log2,   KW_LOG2,   1, Type_Real );
    funcdef( func_loindex, KW_LOINDEX, 1, Type_Integer );
    funcdef( FUNC_NVL,    KW_NVL,    2, Type_Generic );
    funcdef( func_odd,    KW_ODD,    1, Type_Logical );
    funcdef( func_rolesof, KW_ROLESOF, 1, Type_Set_Of_String );
    funcdef( func_sin,    KW_SIN,    1, Type_Real );
    funcdef( func_sizeof, KW_SIZEOF, 1, Type_Integer );
    funcdef( func_sqrt,   KW_SQRT,   1, Type_Real );
    funcdef( func_tan,    KW_TAN,    1, Type_Real );
    funcdef( func_typeof, KW_TYPEOF, 1, Type_Set_Of_String );
    funcdef( FUNC_USEDIN,    KW_USEDIN,  2, Type_Bag_Of_Generic );
    funcdef( func_value, KW_VALUE,   1, Type_Number );
    funcdef( func_value_in,  KW_VALUE_IN,    2, Type_Logical );
    funcdef( func_value_unique, KW_VALUE_UNIQUE, 1, Type_Logical );

    ERROR_bail_out =
        ERRORcreate( "Aborting due to internal error(s)", SEVERITY_DUMP );
    ERROR_syntax =
        ERRORcreate( "%s in %s %s", SEVERITY_EXIT );
    /* i.e., "syntax error in procedure foo" */
    ERROR_ref_nonexistent = ERRORcreate(
                                "USE/REF of non-existent object (%s in schema %s)", SEVERITY_ERROR );
    ERROR_tilde_expansion_failed = ERRORcreate(
                                       "Tilde expansion for %s failed in EXPRESS_PATH environment variable", SEVERITY_ERROR );
    ERROR_schema_not_in_own_schema_file = ERRORcreate(
            "Schema %s was not found in its own schema file (%s)", SEVERITY_ERROR );
    ERROR_unlabelled_param_type = ERRORcreate(
                                      "Return type or local variable requires type label in `%s'", SEVERITY_ERROR );
    ERROR_file_unreadable = ERRORcreate(
                                "Could not read file %s: %s", SEVERITY_ERROR );
    ERROR_file_unwriteable = ERRORcreate(
                                 "Could not write file %s: %s", SEVERITY_ERROR );
    OBJcreate( OBJ_EXPRESS, EXPRESS_get_symbol, "express file", OBJ_UNUSED_BITS );

    ERRORcreate_warning( "unknown_subtype", ERROR_unknown_subtype );

    EXPRESS_PATHinit(); /* note, must follow defn of errors it needs! */
}

/*
** Procedure:   EXPRESSparse
** Parameters:  FILE* file  - Express source file to parse
** Returns: Express     - resulting Working Form model
** Description: Parse an Express source file into the Working Form.
*/
void
EXPRESSparse( Express model, FILE * fp, char * filename ) {
    yyexpresult = model;

    if( !fp ) {
        fp = fopen( filename, "r" );
    }
    if( !fp ) {
        /* go down path looking for file */
        LISTdo( EXPRESS_path, dir, Dir * )
        sprintf( dir->leaf, "%s", filename );
        if( 0 != ( fp = fopen( dir->full, "r" ) ) ) {
            filename = dir->full;
            break;
        }
        LISTod
    }

    if( !fp ) {
        ERRORreport( ERROR_file_unreadable, filename, strerror( errno ) );
        return;
    }

    if( filename ) {
        char * dot   = strrchr( filename, '.' );
        char * slash = strrchr( filename, '/' );

        /* get beginning of basename */
        char * start = slash ? ( slash + 1 ) : filename;

        int length = strlen( start );

        /* drop .exp suffix if present */
        if( dot && streq( dot, ".exp" ) ) {
            length -= 4;
        }

        model->u.express->basename = ( char * )malloc( length + 1 );
        memcpy( model->u.express->basename, filename, length );
        model->u.express->basename[length] = '\0';

        /* get new copy of filename to avoid being smashed */
        /* by subsequent lookups on EXPRESS_path */
        model->u.express->filename = SCANstrdup( filename );
        filename = model->u.express->filename;
    }

    PARSEnew_schemas = LISTcreate();
    PARSERrun( filename, model->u.express->file = fp );
}

static
/* start parsing a new schema file */
Express
PARSERrun( char * filename, FILE * fp ) {
    extern int yyparse();
    extern void SCAN_lex_init PROTO( ( char *, FILE * ) );

    if( print_objects_while_running & OBJ_PASS_BITS ) {
        fprintf( stdout, "parse (pass %d)\n", EXPRESSpass );
    }

    if( print_objects_while_running & OBJ_SCHEMA_BITS ) {
        fprintf( stdout, "parse: %s (schema file)\n", filename );
    }

    yyin = fp;
    SCAN_lex_init( filename, fp );
    if( yyparse() != 0 ) {
        ERRORreport( ERROR_bail_out );
        /* free model and model->u.express */
        return 0;
    }
    EXPRESSpass = 1;
    return yyexpresult;
}

static void RENAMEresolve( Rename * r, Schema s );

/* find the final object to which a rename points */
/* i.e., follow chain of USEs or REFs */
/* sets DICT_type */
static
Generic
SCOPEfind_for_rename( Scope schema, char * name, enum rename_type type ) {
    Generic result;
    Rename * rename;

    /* object can only appear in top level symbol table */
    /* OR in another rename clause */

    result = DICTlookup( schema->symbol_table, name );
    if( result ) {
        return result;
    }

    /* Occurs in a fully USE'd schema? */
    LISTdo( schema->u.schema->use_schemas, schema, Schema )
    /* follow chain'd USEs */
    result = SCOPEfind_for_rename( schema, name, use );
    if( result ) {
        return( result );
    }
    LISTod;

    /* Occurs in a partially USE'd schema? */
    rename = ( Rename * )DICTlookup( schema->u.schema->usedict, name );
    if( rename ) {
        RENAMEresolve( rename, schema );
        DICT_type = rename->type;
        return( rename->object );
    }

    LISTdo( schema->u.schema->uselist, r, Rename * )
    if( streq( ( r->nnew ? r->nnew : r->old )->name, name ) ) {
        RENAMEresolve( r, schema );
        DICT_type = r->type;
        return( r->object );
    }
    LISTod;

    /* we are searching for an object to be interfaced from this schema. */
    /* therefore, we *never* want to look at things which are REFERENCE'd */
    /* *into* the current schema.  -snc */
#if 0
    if( type == ref ) {
        return( 0 );
    }

    /*NOTREACHED*/
    /* Occurs in a fully REF'd schema? */
    LISTdo( schema->u.schema->ref_schemas, schema, Schema )
    result = SCOPEfind_for_rename( schema, name, ref );
    if( result ) {
        return result;
    } else {
        continue;    /* try another schema */
    }
    LISTod;

    /* Occurs in a partially REF'd schema? */
    rename = ( Rename * )DICTlookup( schema->u.schema->refdict, name );
    if( rename ) {
        RENAMEresolve( rename );
        DICT_type = rename->type;
        return( rename->object );
    }
#endif

    return 0;
}

static void
RENAMEresolve( Rename * r, Schema s ) {
    Generic remote;

    /*   if (is_resolved_rename_raw(r->old)) return;*/
    if( r->object ) {
        return;
    }

    if( is_resolve_failed_raw( r->old ) ) {
        return;
    }

    if( is_resolve_in_progress_raw( r->old ) ) {
        ERRORreport_with_symbol( ERROR_circular_reference,
                                 r->old, r->old->name );
        resolve_failed_raw( r->old );
        return;
    }
    resolve_in_progress_raw( r->old );

    remote = SCOPEfind_for_rename( r->schema, r->old->name, r->rename_type );
    if( remote == 0 ) {
        ERRORreport_with_symbol( ERROR_ref_nonexistent, r->old,
                                 r->old->name, r->schema->symbol.name );
        resolve_failed_raw( r->old );
    } else {
        r->object = remote;
        r->type = DICT_type;
        switch( r->rename_type ) {
            case use:
                SCHEMAdefine_use( s, r );
                break;
            case ref:
                SCHEMAdefine_reference( s, r );
                break;
        }
        /*  resolve_rename_raw(r->old);*/
    }
    resolve_not_in_progress_raw( r->old );
}

#ifdef using_enum_items_is_a_pain
static void
RENAMEresolve_enum( Type t, Schema s ) {
    Dictionary      d = TYPEget_enum_tags( t );
    DictionaryEntry de;
    Expression      x;

    DICTdo_type_init( t->symbol_table, &de, OBJ_EXPRESSION );
    while( 0 != ( x = ( Expression )DICTdo( &de ) ) ) {
        /*      SCHEMAadd_use(s, v*/
        /*          raw(x->symbol.name);*/
    }
}
#endif

Schema
EXPRESSfind_schema( Dictionary modeldict, char * name ) {
    Schema s;
    FILE * fp;
    char * src, *dest;
    char lower[MAX_SCHEMA_FILENAME_SIZE];   /* avoid lowerizing original */

    if( print_objects_while_running & OBJ_SCHEMA_BITS ) {
        fprintf( stdout, "pass %d: %s (schema reference)\n",
                 EXPRESSpass, name );
    }

    s = ( Schema )DICTlookup( modeldict, name );
    if( s ) {
#if 0
        if( print_objects_while_running & OBJ_SCHEMA_BITS ) {
            fprintf( stdout, "pass %d: (found schema %s in model already)\n" );
        }
#endif
        return s;
    }

    dest = lower;
    for( src = name; *src; src++ ) {
        *dest++ = tolower( *src );
    }
    *dest = '\0';

    /* go down path looking for file */
    LISTdo( EXPRESS_path, dir, Dir * )
    sprintf( dir->leaf, "%s.exp", lower );
    if( print_objects_while_running & OBJ_SCHEMA_BITS ) {
        fprintf( stdout, "pass %d: %s (schema file?)\n",
                 EXPRESSpass, dir->full );
    }
    fp = fopen( dir->full, "r" );
    if( fp ) {
        Express express;

        if( print_objects_while_running & OBJ_SCHEMA_BITS ) {
            fprintf( stdout, "pass %d: %s (schema file found)\n",
                     EXPRESSpass, dir->full );
        }

        express = PARSERrun( SCANstrdup( dir->full ), fp );
        if( express ) {
            s = ( Schema )DICTlookup( modeldict, name );
        }
        if( s ) {
            return s;
        }
        ERRORreport( ERROR_schema_not_in_own_schema_file,
                     name, dir->full );
        return 0;
    } else {
        if( print_objects_while_running & OBJ_SCHEMA_BITS ) {
            fprintf( stdout, "pass %d: %s (schema file not found), errno = %d\n", EXPRESSpass, dir->full, errno );
        }
    }
    LISTod
    return 0;
}


/* make the initial connections from one schema to another */
/* dictated by USE/REF clauses that use dictionaries, i.e., */
/* because of partial schema references */

static void
connect_lists( Dictionary modeldict, Schema schema, Linked_List list ) {
    Rename * r;

    /* translate symbols to schemas */
    LISTdo_links( list, l )
    r = ( Rename * )l->data;
    r->schema = EXPRESSfind_schema( modeldict, r->schema_sym->name );
    if( !r->schema ) {
        ERRORreport_with_symbol( ERROR_undefined_schema,
                                 r->schema_sym,
                                 r->schema_sym->name );
        resolve_failed_raw( r->old );
        resolve_failed( schema );
    }
    LISTod
}
/* same as above, except for full schemas */
static void
connect_schema_lists( Dictionary modeldict, Schema schema, Linked_List schema_list ) {
    Symbol * sym;
    Schema ref_schema;

    /* translate symbols to schemas */
    LISTdo_links( schema_list, list )
    sym = ( Symbol * )list->data;
    ref_schema = EXPRESSfind_schema( modeldict, sym->name );
    if( !ref_schema ) {
        ERRORreport_with_symbol( ERROR_undefined_schema, sym, sym->name );
        resolve_failed( schema );
        list->data = 0;
    } else {
        list->data = ( Generic )ref_schema;
    }
    LISTod
}


/*
** Procedure:   EXPRESSresolve
** Parameters:  Express model   - Working Form model to resolve
** Returns: void
** Description: Perform symbol resolution on a loosely-coupled WF.
*/

void
EXPRESSresolve( Express model ) {
    /* resolve multiple schemas.  Schemas will be resolved here or when */
    /* they are first encountered by a use/reference clause, whichever */
    /* comes first - DEL */

    Schema schema;
    DictionaryEntry de; /* imaginative, eh? */

    jmp_buf env;
    if( setjmp( env ) ) {
        return;
    }
    ERRORsafe( env );

    EXPRESSpass++;
    if( print_objects_while_running & OBJ_PASS_BITS ) {
        fprintf( stdout, "pass %d: resolving schema references\n", EXPRESSpass );
    }

    /* connect the real schemas to all the rename clauses */

    /* we may be adding new schemas (to dictionary) as we traverse it, */
    /* so to avoid confusing dictionary, use a list as a fifo.  I.e., */
    /* add news schemas to end, drop old ones off the front as we */
    /* process them. */

    LISTdo( PARSEnew_schemas, schema, Schema )
    if( print_objects_while_running & OBJ_SCHEMA_BITS ) {
        fprintf( stdout, "pass %d: %s (schema)\n",
                 EXPRESSpass, schema->symbol.name );
    }

    if( schema->u.schema->uselist )
        connect_lists( model->symbol_table,
                       schema, schema->u.schema->uselist );
    if( schema->u.schema->reflist )
        connect_lists( model->symbol_table,
                       schema, schema->u.schema->reflist );

    connect_schema_lists( model->symbol_table,
                          schema, schema->u.schema->use_schemas );
    connect_schema_lists( model->symbol_table,
                          schema, schema->u.schema->ref_schemas );
    LISTod;

    LISTfree( PARSEnew_schemas );
    PARSEnew_schemas = 0;   /* just in case */

    EXPRESSpass++;
    if( print_objects_while_running & OBJ_PASS_BITS ) {
        fprintf( stdout, "pass %d: resolving objects references to other schemas\n", EXPRESSpass );
    }

    /* connect the object in each rename clause to the real object */
    DICTdo_type_init( model->symbol_table, &de, OBJ_SCHEMA );
    while( 0 != ( schema = ( Schema )DICTdo( &de ) ) ) {
        if( is_not_resolvable( schema ) ) {
            continue;
        }

        if( print_objects_while_running & OBJ_SCHEMA_BITS ) {
            fprintf( stdout, "pass %d: %s (schema)\n",
                     EXPRESSpass, schema->symbol.name );
        }

        /* do USE's first because they take precedence */
        if( schema->u.schema->uselist ) {
            LISTdo( schema->u.schema->uselist, r, Rename * )
            RENAMEresolve( r, schema );
            LISTod;
            LISTfree( schema->u.schema->uselist );
            schema->u.schema->uselist = 0;
        }
        if( schema->u.schema->reflist ) {
            LISTdo( schema->u.schema->reflist, r, Rename * )
            RENAMEresolve( r, schema );
            LISTod;
            LISTfree( schema->u.schema->reflist );
            schema->u.schema->reflist = 0;
        }
    }

    /* resolve sub- and supertype references.  also resolve all */
    /* defined types */
    EXPRESSpass++;
    if( print_objects_while_running & OBJ_PASS_BITS ) {
        fprintf( stdout, "pass %d: resolving sub and supertypes\n", EXPRESSpass );
    }

    DICTdo_type_init( model->symbol_table, &de, OBJ_SCHEMA );
    while( 0 != ( schema = ( Schema )DICTdo( &de ) ) ) {
        if( is_not_resolvable( schema ) ) {
            continue;
        }
        SCOPEresolve_subsupers( schema );
    }
    if( ERRORoccurred ) {
        ERRORunsafe();
    }

    /* resolve types */
    EXPRESSpass++;
    if( print_objects_while_running & OBJ_PASS_BITS ) {
        fprintf( stdout, "pass %d: resolving types\n", EXPRESSpass );
    }

    SCOPEresolve_types( model );
    if( ERRORoccurred ) {
        ERRORunsafe();
    }

#ifdef using_enum_items_is_a_pain
    /* resolve USE'd enumerations */
    /* doesn't really deserve its own pass, but needs to come after */
    /* type resolution */
    EXPRESSpass++;
    if( print_objects_while_running & OBJ_PASS_BITS ) {
        fprintf( stdout, "pass %d: resolving implied USE's\n", EXPRESSpass );
    }

    DICTdo_type_init( model->symbol_table, &de, OBJ_SCHEMA );
    while( 0 != ( schema = ( Schema )DICTdo( &de ) ) ) {
        if( is_not_resolvable( schema ) ) {
            continue;
        }

        if( print_objects_while_running & OBJ_SCHEMA_BITS ) {
            fprintf( stdout, "pass %d: %s (schema)\n",
                     EXPRESSpass, schema->symbol.name );
        }

        if( schema->u.schema->usedict ) {
            DICTdo_init( schema->u.schema->usedict, &fg )
            while( 0 != ( r = ( Rename )DICTdo( &fg ) ) ) {
                if( ( r->type = OBJ_TYPE ) && ( ( Type )r->object )->body &&
                        TYPEis_enumeration( ( Type )r->object ) ) {
                    RENAMEresolve_enum( ( Type )r->object, schema );
                }
            }
        }
    }
    if( ERRORoccurred ) {
        ERRORunsafe();
    }
#endif

    EXPRESSpass++;
    if( print_objects_while_running & OBJ_PASS_BITS ) {
        fprintf( stdout, "pass %d: resolving expressions and statements\n", EXPRESSpass );
    }

    SCOPEresolve_expressions_statements( model );
    if( ERRORoccurred ) {
        ERRORunsafe();
    }

    /* mark everything resolved if possible */
    DICTdo_init( model->symbol_table, &de );
    while( 0 != ( schema = ( Schema )DICTdo( &de ) ) ) {
        if( is_resolvable( schema ) ) {
            resolved_all( schema );
        }
    }
}

#if 0
void
EXPRESSdump_schema( Schema schema ) {
    Linked_List list, list2, ref;
    Dictionary dict;
    DictionaryEntry de;

    printf( "SCHEMA %s\n", SCHEMAget_name( schema ) );
    list = SCOPEget_types( schema );
    printf( "  Types:\n" );
    LISTdo( list, s, Symbol * )
    printf( "    %s\n", SYMBOLget_name( s ) );
    LISTod;
    list = SCOPEget_entities( schema );
    printf( "  Entities:\n" );
    LISTdo( list, s, Symbol )
    printf( "    %s\n", SYMBOLget_name( s ) );
    LISTod;

    /* N14 Dump USE and REFERENCE lists */
    list = SCOPEget_uses( schema );
    printf( "  Use:\n" );
    LISTdo( list, use, Linked_List )
    printf( "   Schema: %s\n", SYMBOLget_name( LISTget_first( use ) ) );
    list2 = LISTget_second( use );
    LISTdo( list2, use_exp, Expression );
    printf( "   %s AS %s\n",
            SYMBOLget_name( BIN_EXPget_first_operand( use_exp ) ),
            SYMBOLget_name( BIN_EXPget_second_operand( use_exp ) ) );
    LISTod;
    LISTod;

    dict = SCOPEget_references( schema );
    printf( "  Reference:\n" );
    DICTdo_init( dict, &de );
    while( ref = ( Linked_List )DICTdo( &de ) ) {
        printf( "    %s\n", SYMBOLget_name( LISTget_first( ref ) ) );
        list2 = LISTget_second( ref );
        LISTdo( list2, ref_exp, Expression );
        printf( "   %s AS %s\n",
                SYMBOLget_name( BIN_EXPget_first_operand( ref_exp ) ),
                SYMBOLget_name( BIN_EXPget_second_operand( ref_exp ) ) );
        LISTod;
    }

    printf( "END_SCHEMA\n" );

    /* N14 Nested schemas obsolete
        list = SCOPEget_schemata(schema);
        LISTdo(list, s, Schema)
        EXPRESSdump_schema(s);
        LISTod; */

}

/*
** Procedure:   EXPRESSdump_model
** Parameters:  Express model   - Express model to dump
** Returns: void
** Description: Dumps an Express model to stderr.
*/

/*ARGUSED*/
void
EXPRESSdump_model( Express model ) {
    /* should make dump_model and dump_modelS! - DEL */
    /*    EXPRESSdump_schema(model->schema);*/
}

#endif /*0*/

