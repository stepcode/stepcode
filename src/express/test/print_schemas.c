/* derived from symlink.c */
/* prints names of schemas used in an EXPRESS file */
/* symlink.c author: Don Libes, NIST, 20-Mar-1993 */

#include "sc_cf.h"
#include <stdlib.h>
#ifdef HAVE_SYS_PARAM_H
#  include <sys/param.h>
#endif
#include <stdio.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include "express/express.h"
#include "express/hash.h"

void
print_schemas( Express model ) {
    Hash_Iterator it;
    Schema s;

    printf( "File: %s\n  ", model->u.express->filename );

    HASHdo_init( model->symbol_table, &it, OBJ_ANY );
    while((s = HASHdo(&it))) {
        printf( "%s", s->symbol.name );
    }
    printf( "\n" );
    exit( 0 );
}

void EXPRESSinit_init() {
    EXPRESSbackend = print_schemas;
}

