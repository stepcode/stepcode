/** \file scope.c
 * split out of exppp.c 9/21/13
 */

#include "pp.h"
#include "exppp.h"
#include "pretty_expr.h"
#include "pretty_rule.h"
#include "pretty_type.h"
#include "pretty_func.h"
#include "pretty_entity.h"
#include "pretty_proc.h"
#include "pretty_scope.h"

void SCOPEadd_inorder( Linked_List list, Scope s ) {
    Link k = 0;

    LISTdo_links( list, link )
    if( 0 > strcmp(
                SCOPEget_name( s ),
                SCOPEget_name( ( Type )( link->data ) ) ) ) {
        k = link;
        break;
    } LISTod

    LISTadd_before( list, k, ( Generic )s );
}

/** print the rules in a scope */
void SCOPErules_out( Scope s, int level ) {
    Rule r;
    DictionaryEntry de;

    if( exppp_alphabetize == false ) {
        DICTdo_type_init( s->symbol_table, &de, OBJ_RULE );
        while( 0 != ( r = ( Rule )DICTdo( &de ) ) ) {
            RULE_out( r, level );
        }
    } else {
        Linked_List alpha = LISTcreate();

        DICTdo_type_init( s->symbol_table, &de, OBJ_RULE );
        while( 0 != ( r = ( Rule )DICTdo( &de ) ) ) {
            SCOPEadd_inorder( alpha, r );
        }

        LISTdo( alpha, ru, Rule ) {
            RULE_out( ru, level );
        } LISTod

        LISTfree( alpha );
    }

}

/** print the functions in a scope */
void SCOPEfuncs_out( Scope s, int level ) {
    Function f;
    DictionaryEntry de;

    if( exppp_alphabetize == false ) {
        DICTdo_type_init( s->symbol_table, &de, OBJ_FUNCTION );
        while( 0 != ( f = ( Function )DICTdo( &de ) ) ) {
            FUNC_out( f, level );
        }
    } else {
        Linked_List alpha = LISTcreate();

        DICTdo_type_init( s->symbol_table, &de, OBJ_FUNCTION );
        while( 0 != ( f = ( Function )DICTdo( &de ) ) ) {
            SCOPEadd_inorder( alpha, f );
        }

        LISTdo( alpha, fun, Function ) {
            FUNC_out( fun, level );
        } LISTod

        LISTfree( alpha );
    }

}

/* print the procs in a scope */
void SCOPEprocs_out( Scope s, int level ) {
    Procedure p;
    DictionaryEntry de;

    if( exppp_alphabetize == false ) {
        DICTdo_type_init( s->symbol_table, &de, OBJ_PROCEDURE );
        while( 0 != ( p = ( Procedure )DICTdo( &de ) ) ) {
            PROC_out( p, level );
        }
    } else {
        Linked_List alpha = LISTcreate();

        DICTdo_type_init( s->symbol_table, &de, OBJ_PROCEDURE );
        while( 0 != ( p = ( Procedure )DICTdo( &de ) ) ) {
            SCOPEadd_inorder( alpha, p );
        }

        LISTdo( alpha, pr, Procedure ) {
            PROC_out( pr, level );
        } LISTod

        LISTfree( alpha );
    }

}

/* print the algorithms in a scope */
void SCOPEalgs_out( Scope s, int level ) {
    /* Supplementary Directivies 2.1.1 requires rules to be separated */
    /* might as well separate funcs and procs, too */
    SCOPErules_out( s, level );
    SCOPEfuncs_out( s, level );
    SCOPEprocs_out( s, level );
}

void SCOPEconsts_out( Scope s, int level ) {
    Variable v;
    DictionaryEntry de;
    size_t max_indent = 0, old_indent2;
    Dictionary d = s->symbol_table;

    DICTdo_type_init( d, &de, OBJ_VARIABLE );
    while( 0 != ( v = ( Variable )DICTdo( &de ) ) ) {
        if( !v->flags.constant ) {
            continue;
        }
        if( strlen( v->name->symbol.name ) > max_indent ) {
            max_indent = strlen( v->name->symbol.name );
        }
    }

    if( !max_indent ) {
        return;
    }

    first_newline();

    raw( "%*sCONSTANT\n", level, "" );

    /* if max_indent is too big, wrap() won't insert *any* newlines */
    if( ( max_indent * 2 ) > ( size_t ) exppp_linelength ) {
        max_indent /= 3;
    }
    indent2 = level + max_indent + strlen( ": " ) + exppp_continuation_indent;

    DICTdo_type_init( d, &de, OBJ_VARIABLE );
    while( 0 != ( v = ( Variable )DICTdo( &de ) ) ) {
        if( !v->flags.constant ) {
            continue;
        }

        /* print attribute name */
        raw( "%*s%-*s :", level + 2, "",
             max_indent, v->name->symbol.name );

        /* print attribute type */
        if( VARget_optional( v ) ) {
            wrap( " OPTIONAL" );
        }

        /* let type definition stick out a bit to the left */
        old_indent2 = indent2;
        if( indent2 > 4 ) {
            indent2 -= 4;
        }
        TYPE_head_out( v->type, NOLEVEL );
        indent2 = old_indent2;

        if( v->initializer ) {
            wrap( " := " );
            if( curpos > exppp_linelength * 2 / 3 ) {
                /* let '[' on first line of initializer stick out so strings are aligned */
                raw( "\n%*s", indent2 - 1, "" );
            }
            EXPR_out( v->initializer, 0 );
        }

        raw( ";\n" );
    }

    raw( "%*sEND_CONSTANT;\n", level, "" );
}

void SCOPElocals_out( Scope s, int level ) {
    Variable v;
    DictionaryEntry de;
    size_t max_indent = 0;
    Dictionary d = s->symbol_table;

    DICTdo_type_init( d, &de, OBJ_VARIABLE );
    while( 0 != ( v = ( Variable )DICTdo( &de ) ) ) {
        if( v->flags.constant ) {
            continue;
        }
        if( v->flags.parameter ) {
            continue;
        }
        if( strlen( v->name->symbol.name ) > max_indent ) {
            max_indent = strlen( v->name->symbol.name );
        }
    }

    if( !max_indent ) {
        return;
    }

    first_newline();

    raw( "%*sLOCAL\n", level, "" );
    indent2 = level + max_indent + strlen( ": " ) + exppp_continuation_indent;

    DICTdo_type_init( d, &de, OBJ_VARIABLE );
    while( 0 != ( v = ( Variable )DICTdo( &de ) ) ) {
        if( v->flags.constant ) {
            continue;
        }
        if( v->flags.parameter ) {
            continue;
        }

        /* print attribute name */
        raw( "%*s%-*s :", level + exppp_nesting_indent, "",
             max_indent, v->name->symbol.name );

        /* print attribute type */
        if( VARget_optional( v ) ) {
            wrap( " OPTIONAL" );
        }
        TYPE_head_out( v->type, NOLEVEL );

        if( v->initializer ) {
            wrap( " := " );
            EXPR_out( v->initializer, 0 );
        }

        raw( ";\n" );
    }

    raw( "%*sEND_LOCAL;\n", level, "" );
}

/** print all entities in a scope */
void SCOPEentities_out( Scope s, int level ) {
    Entity e;
    DictionaryEntry de;

    if( exppp_alphabetize == false ) {
        DICTdo_type_init( s->symbol_table, &de, OBJ_ENTITY );
        while( 0 != ( e = ( Entity )DICTdo( &de ) ) ) {
            ENTITY_out( e, level );
        }
    } else {
        Linked_List alpha = LISTcreate();

        DICTdo_type_init( s->symbol_table, &de, OBJ_ENTITY );
        while( 0 != ( e = ( Entity )DICTdo( &de ) ) ) {
            SCOPEadd_inorder( alpha, e );
        }

        LISTdo( alpha, en, Entity ) {
            ENTITY_out( en, level );
        } LISTod

        LISTfree( alpha );
    }
}

/** print all types in a scope */
void SCOPEtypes_out( Scope s, int level ) {
    DictionaryEntry de;
    Type t;

    if( exppp_alphabetize == false ) {
        DICTdo_type_init( s->symbol_table, &de, OBJ_TYPE );
        while( 0 != ( t = ( Type )DICTdo( &de ) ) ) {
            TYPE_out( t, level );
        }
    } else {
        Linked_List alpha = LISTcreate();

        DICTdo_type_init( s->symbol_table, &de, OBJ_TYPE );
        while( 0 != ( t = ( Type )DICTdo( &de ) ) ) {
            SCOPEadd_inorder( alpha, t );
        }

        LISTdo( alpha, ty, Type ) {
            TYPE_out( ty, level );
        } LISTod

        LISTfree( alpha );
    }
}