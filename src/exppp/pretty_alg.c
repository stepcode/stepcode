/** \file pretty_alg.c
 * split out of exppp.c 9/21/13
 */


#include <express/scope.h>
#include <express/linklist.h>

#include "pp.h"
#include "pretty_type.h"
#include "pretty_expr.h"
#include "pretty_scope.h"
#include "pretty_alg.h"

void ALGscope_out( Scope s, int level ) {
    SCOPEtypes_out( s, level );
    SCOPEentities_out( s, level );
    SCOPEalgs_out( s, level );

    SCOPEconsts_out( s, level );
    SCOPElocals_out( s, level );
}

/** last arg is not terminated with ; or \n */
void ALGargs_out( Linked_List args, int level ) {
    Type previoustype = 0;
    indent2 = level + exppp_continuation_indent;

    /* combine adjacent parameters that have the same type */

    LISTdo( args, v, Variable )
    if( previoustype != v->type ) {
        if( previoustype ) {
            wrap( " : " );
            TYPE_head_out( previoustype, NOLEVEL );
            raw( ";\n" );
        }
        raw( "%*s", level, "" );
        EXPR_out( VARget_name( v ), 0 );
    } else {
        raw( ", " );
        EXPR_out( VARget_name( v ), 0 );
    }
    previoustype = v->type;
    LISTod

    wrap( " : " );
    TYPE_head_out( previoustype, NOLEVEL );
}
