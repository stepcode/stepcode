/*
** Fed-x parser output module for generating C++  class definitions
** December  5, 1989
** release 2 17-Feb-1992
** release 3 March 1993
** release 4 December 1993
** K. C. Morris
**
** Development of Fed-x was funded by the United States Government,
** and is not subject to copyright.

*******************************************************************
The conventions used in this binding follow the proposed specification
for the STEP Standard Data Access Interface as defined in document
N350 ( August 31, 1993 ) of ISO 10303 TC184/SC4/WG7.
*******************************************************************/

/**************************************************************//**
 * \file classes.c
***  The functions in this file generate the C++ code for ENTITY **
***  classes, TYPEs, and TypeDescriptors.                       ***
 **                                                             **/


/* this is used to add new dictionary calls */
/* #define NEWDICT */

#include <stdlib.h>
#include <assert.h>
#include "classes.h"
#include <ordered_attrs.h>

#include "./trace_fprintf.h"

int multiple_inheritance = 1;
int print_logging = 0;
int old_accessors = 0;
unsigned long exp2cxx_entity_chunk_size = 64;
unsigned long exp2cxx_type_chunk_size = 8;
int exp2cxx_api_version = 1;
int exp2cxx_late_bound = 0;

/**
 * Turn the string into a new string that will be printed the same as the
 * original string. That is, turn backslash into a quoted backslash and
 * turn \n into "\n" (i.e. 2 chars).
 *
 * Mostly replaced by format_for_std_stringout, below. This function is
 * still used in one place in ENTITYincode_print().
 */
char * format_for_stringout( char * orig_buf, char * return_buf ) {
    char * optr  = orig_buf;
    char * rptr  = return_buf;
    while( *optr ) {
        if( *optr == '\n' ) {
            *rptr = '\\';
            rptr++;
            *rptr = 'n';
        } else if( *optr == '\\' ) {
            *rptr = '\\';
            rptr++;
            *rptr = '\\';
        } else {
            *rptr = *optr;
        }
        rptr++;
        optr++;
    }
    *rptr = '\0';
    return return_buf;
}

/**
 * Like format_for_stringout above, but splits the static string up
 * into numerous small ones that are appended to a std::string named
 * 'str'. It is assumed that this string already exists and is empty.
 *
 * This version takes a file pointer and eliminates use of the temp buffer.
 */
void format_for_std_stringout( FILE * f, char * orig_buf ) {
    const char * optr  = orig_buf;
    char * s_end = "\\n\" );\n";
    char * s_begin = "    str.append( \"";
    fprintf( f, "%s", s_begin );
    while( *optr ) {
        if( *optr == '\n' ) {
            if( * ( optr + 1 ) == '\n' ) { /*  skip blank lines */
                optr++;
                continue;
            }
            fprintf( f, "%s", s_end );
            fprintf( f, "%s", s_begin );
        } else if( *optr == '\\' ) {
            fprintf( f, "\\\\" );
        } else {
            fprintf( f, "%c", *optr );
        }
        optr++;
    }
    fprintf( f, "%s", s_end );
    free( orig_buf );
}

void USEREFout( Schema schema, Dictionary refdict, Linked_List reflist, char * type, FILE * file ) {
    Dictionary dict;
    DictionaryEntry de;
    struct Rename * r;
    Linked_List list;
    char td_name[BUFSIZ+1];
    char sch_name[BUFSIZ+1];

    strncpy( sch_name, PrettyTmpName( SCHEMAget_name( schema ) ), BUFSIZ );

    LISTdo( reflist, s, Schema ) {
        fprintf( file, "        // %s FROM %s; (all objects)\n", type, s->symbol.name );
        fprintf( file, "        is = new Interface_spec(\"%s\",\"%s\");\n", sch_name, PrettyTmpName( s->symbol.name ) );
        fprintf( file, "        is->all_objects_(1);\n" );
        if( !strcmp( type, "USE" ) ) {
            fprintf( file, "        %s::schema->use_interface_list_()->Append(is);\n", SCHEMAget_name( schema ) );
        } else {
            fprintf( file, "        %s::schema->ref_interface_list_()->Append(is);\n", SCHEMAget_name( schema ) );
        }
    } LISTod

    if( !refdict ) {
        return;
    }
    dict = DICTcreate( 10 );

    /* sort each list by schema */

    /* step 1: for each entry, store it in a schema-specific list */
    DICTdo_init( refdict, &de );
    while( 0 != ( r = ( struct Rename * )DICTdo( &de ) ) ) {
        Linked_List wlist;

        wlist = ( Linked_List )DICTlookup( dict, r->schema->symbol.name );
        if( !wlist ) {
            wlist = LISTcreate();
            DICTdefine( dict, r->schema->symbol.name, wlist, NULL, OBJ_UNKNOWN );
        }
        LISTadd_last( wlist, r );
    }

    /* step 2: for each list, print out the renames */
    DICTdo_init( dict, &de );
    while( 0 != ( list = ( Linked_List )DICTdo( &de ) ) ) {
        bool first_time = true;
        LISTdo( list, re, struct Rename * ) {

            /* note: SCHEMAget_name(r->schema) equals r->schema->symbol.name) */
            if( first_time ) {
                fprintf( file, "        // %s FROM %s (selected objects)\n", type, re->schema->symbol.name );
                fprintf( file, "        is = new Interface_spec(\"%s\",\"%s\");\n", sch_name, PrettyTmpName( re->schema->symbol.name ) );
                if( !strcmp( type, "USE" ) ) {
                    fprintf( file, "        %s::schema->use_interface_list_()->Append(is);\n", SCHEMAget_name( schema ) );
                } else {
                    fprintf( file, "        %s::schema->ref_interface_list_()->Append(is);\n", SCHEMAget_name( schema ) );
                }
            }

            if( first_time ) {
                first_time = false;
            }
            if( re->type == OBJ_TYPE ) {
                snprintf( td_name, sizeof(td_name), "%s", TYPEtd_name( ( Type )re->object ) );
            } else if( re->type == OBJ_FUNCTION ) {
                snprintf( td_name, sizeof(td_name), "/* Function not implemented */ 0" );
            } else if( re->type == OBJ_PROCEDURE ) {
                snprintf( td_name, sizeof(td_name), "/* Procedure not implemented */ 0" );
            } else if( re->type == OBJ_RULE ) {
                snprintf( td_name, sizeof(td_name), "/* Rule not implemented */ 0" );
            } else if( re->type == OBJ_ENTITY ) {
                snprintf( td_name, sizeof(td_name), "%s%s%s",
                        SCOPEget_name( ( ( Entity )re->object )->superscope ),
                        ENT_PREFIX, ENTITYget_name( ( Entity )re->object ) );
            } else {
                snprintf( td_name, sizeof(td_name), "/* %c from OBJ_? in expbasic.h not implemented */ 0", re->type );
            }
            if( re->old != re->nnew ) {
                fprintf( file, "        // object %s AS %s\n", re->old->name,
                        re->nnew->name );
                if( !strcmp( type, "USE" ) ) {
                    fprintf( file, "        ui = new Used_item(\"%s\", %s, \"%s\", \"%s\");\n", re->schema->symbol.name, td_name, re->old->name, re->nnew->name );
                    fprintf( file, "        is->explicit_items_()->Append(ui);\n" );
                    fprintf( file, "        %s::schema->interface_().explicit_items_()->Append(ui);\n", SCHEMAget_name( schema ) );
                } else {
                    fprintf( file, "        ri = new Referenced_item(\"%s\", %s, \"%s\", \"%s\");\n", re->schema->symbol.name, td_name, re->old->name, re->nnew->name );
                    fprintf( file, "        is->explicit_items_()->Append(ri);\n" );
                    fprintf( file, "        %s::schema->interface_().explicit_items_()->Append(ri);\n", SCHEMAget_name( schema ) );
                }
            } else {
                fprintf( file, "        // object %s\n", re->old->name );
                if( !strcmp( type, "USE" ) ) {
                    fprintf( file, "        ui = new Used_item(\"%s\", %s, \"\", \"%s\");\n", re->schema->symbol.name, td_name, re->nnew->name );
                    fprintf( file, "        is->explicit_items_()->Append(ui);\n" );
                    fprintf( file, "        %s::schema->interface_().explicit_items_()->Append(ui);\n", SCHEMAget_name( schema ) );
                } else {
                    fprintf( file, "        ri = new Referenced_item(\"%s\", %s, \"\", \"%s\");\n", re->schema->symbol.name, td_name, re->nnew->name );
                    fprintf( file, "        is->explicit_items_()->Append(ri);\n" );
                    fprintf( file, "        %s::schema->interface_().explicit_items_()->Append(ri);\n", SCHEMAget_name( schema ) );
                }
            }
        } LISTod
    }
    HASHdestroy( dict );
}

int Handle_FedPlus_Args( int i, char * arg ) {
    if( ( ( char )i == 's' ) || ( ( char )i == 'S' ) ) {
        multiple_inheritance = 0;
    }
    if( ( ( char )i == 'a' ) || ( ( char )i == 'A' ) ) {
        old_accessors = 1;
    }
    if( ( ( char )i == 'l' ) || ( ( char )i == 'L' ) ) {
        print_logging = 1;
    }
    if( ( char )i == 'k' ) {
        char * end = 0;
        unsigned long entity_requested;
        unsigned long type_requested;
        if( !arg || !arg[0] ) {
            fprintf( stderr, "exp2cxx: chunk sizes must use N or N:N form\n" );
            return 1;
        }
        entity_requested = strtoul( arg, &end, 10 );
        if( !end || entity_requested == 0 ) {
            fprintf( stderr, "exp2cxx: chunk sizes must be positive integers\n" );
            return 1;
        }
        type_requested = entity_requested;
        if( *end == ':' ) {
            char * type_end = 0;
            type_requested = strtoul( end + 1, &type_end, 10 );
            if( !end[1] || !type_end || *type_end || type_requested == 0 ) {
                fprintf( stderr, "exp2cxx: chunk sizes must use N or N:N form\n" );
                return 1;
            }
        } else if( *end ) {
            fprintf( stderr, "exp2cxx: chunk sizes must use N or N:N form\n" );
            return 1;
        }
        exp2cxx_entity_chunk_size = entity_requested;
        exp2cxx_type_chunk_size = type_requested;
    }
    if( ( char )i == 'V' ) {
        char * end = 0;
        long requested = arg ? strtol( arg, &end, 10 ) : 0;
        if( !arg || !arg[0] || !end || *end || ( requested != 1 && requested != 2 ) ) {
            fprintf( stderr, "exp2cxx: API version must be 1 or 2\n" );
            return 1;
        }
        if( exp2cxx_late_bound && requested != 2 ) {
            fprintf( stderr, "exp2cxx: late-bound output requires API version 2\n" );
            return 1;
        }
        exp2cxx_api_version = ( int )requested;
    }
    if( ( char )i == 'T' ) {
        exp2cxx_late_bound = 1;
        exp2cxx_api_version = 2;
    }
    return 0;
}

void MODELPrintConstructorBody( Entity entity, FILES * files, Schema schema ) {
    const char * n;
    DEBUG( "Entering MODELPrintConstructorBody for %s\n", n );

    n = ENTITYget_classname( entity );

    fprintf( files->lib, "    eep = new SDAI_Entity_extent;\n" );

    fprintf( files->lib, "    eep->definition_(%s::%s%s);\n", SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );
    fprintf( files->lib, "    _folders.Append(eep);\n\n" );
}

void MODELPrint( Entity entity, FILES * files, Schema schema, int index ) {

    const char * n;
    DEBUG( "Entering MODELPrint for %s\n", n );

    n = ENTITYget_classname( entity );
    fprintf( files->lib, "\n%s__set_var SdaiModel_contents_%s::%s_get_extents() {\n", n, SCHEMAget_name( schema ), n );
    fprintf( files->lib, "\n    return (%s__set_var)((_folders.retrieve(%d))->instances_());\n}\n", n, index );
    DEBUG( "DONE MODELPrint\n" )    ;
}

void MODELprint_new( Entity entity, FILES * files ) {
    const char * n;

    n = ENTITYget_classname( entity );
    fprintf( files->inc, "\n    %s__set_var %s_get_extents();\n", n, n );
}
