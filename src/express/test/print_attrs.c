/* derived from symlink.c */
/* prints names of attrs for an entity defined in an EXPRESS file */
/* symlink.c author: Don Libes, NIST, 20-Mar-1993 */

#include <stdlib.h>
#include <sys/param.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "express/express.h"
#include <express/scope.h>
#include <express/variable.h>
#include <assert.h>

char * entityName, _buf[512] = { 0 };

void my_usage() {
    EXPRESSusage( 0 );
    printf( "\t----\n\t-a <entity>: print attrs for <entity>\n" );
    exit( 2 );
}

void print_attrs( Express model ) {
    DictionaryEntry de;
    Schema s;
    Entity e;
    Linked_List attrs;

    if( !entityName ) {
        my_usage();
    }

    printf( "File: %s\n  ", model->u.express->filename );

    DICTdo_init( model->symbol_table, &de );
    while( 0 != ( s = DICTdo( &de ) ) ) {
        printf( "Schema %s\n", s->symbol.name );
        e = DICTlookup( s->symbol_table, entityName );
        if( e ) {
            printf( "    Entity %s\n", e->symbol.name );
            attrs = ENTITYget_all_attributes( e ); // FIXME write this out, avoid using schema name for types. what happens with entities? same?
            LISTdo( attrs, attr, Variable ) {
                const char * source, * scope;
                //there doesn't seem to be a way to get the owning entity of an attr that is a type/entity itself
                //set attr.name.type.superscope???
                if( ( attr->type->superscope->type == 's' ) || ( attr->type->superscope->type == '!' )) { //schema or file
                    source = attr->type->symbol.name;
                    scope = "  TYPE/ENTITY  ";
                } else {
                    source = attr->type->superscope->symbol.name;
                    scope = "    attr of    ";
                }
                printf( "     %s%s, * %p is %s %s\n", ( attr->initializer ? "*" : " " ), attr->name->symbol.name, ( void * ) attr, scope, source );
                if( attr->initializer ) {
                    assert( attr->initializer->e.op1 );
                    printf( "%s", attr->initializer->e.op1->symbol.name );
                }
            } LISTod
        } else {
            printf( "\tnot found.\n" );
        }
        LISTfree( attrs );
    }
    printf( "\n" );
    exit( 0 );
}

int attr_arg( int i, char * arg ) {
    const char * src = arg;
    int count = 0;
    if( ( char )i == 'a' ) {
        entityName = _buf;
        while( *src ) {
            _buf[count] = tolower( *src );
            src++;
            count++;
            if( count == 511 ) {
                break;
            }
        }
        if( count == 0 ) {
            entityName = 0;
        }
    } else if( !entityName ) {
        //if libexpress comes across an unrecognized arg that isn't '-a' and if the entityName isn't set
        return 1; // print usage and exit
    }
    return 0;
}

void EXPRESSinit_init() {
    entityName = 0;
    EXPRESSbackend = print_attrs;
    ERRORusage_function = my_usage;
    strcat( EXPRESSgetopt_options, "a:" );
    EXPRESSgetopt = attr_arg;
}
