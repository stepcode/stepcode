/*
 * This software was developed by U.S. Government employees as part of
 * their official duties and is not subject to copyright.
 */

#include "express/express.h"
#include "express/schema.h"
#include "express/resolve.h"


/********************************new****************************************/

void SCOPEresolve_subsupers( Scope scope ) {
    Hash_Iterator it;
    Symbol *ep;
    Symbol * sym;
    Type t;

    if( print_objects_while_running & OBJ_SCOPE_BITS &
            OBJget_bits( scope->type ) ) {
        fprintf( stderr, "pass %d: %s (%s)\n", EXPRESSpass,
                 scope->symbol.name, OBJget_type( scope->type ) );
    }

    HASHdo_init( scope->symbol_table, &it, '*' );
    while( (ep = HASHdo( &it )) ) {
        switch( ep->type ) {
            case OBJ_ENTITY:
                ENTITYresolve_supertypes( ep->data );
                ENTITYresolve_subtypes( ep->data );
                break;
            case OBJ_FUNCTION:
            case OBJ_PROCEDURE:
            case OBJ_RULE:
                SCOPEresolve_subsupers( ep->data );
                break;
            case OBJ_TYPE:
                t = ( Type ) ep->data;
                TYPEresolve( &t );
                break;
            default:
                /* ignored everything else */
                break;
        }
        sym = OBJget_symbol( ep->data, ep->type );
        if( is_resolve_failed_raw( sym ) ) {
            resolve_failed( scope );
        }
    }
}

void SCOPEresolve_expressions_statements( Scope s ) {
    Hash_Iterator it;
    Symbol *ep;
    Variable v;

    if( print_objects_while_running & OBJ_SCOPE_BITS &
            OBJget_bits( s->type ) ) {
        fprintf( stderr, "pass %d: %s (%s)\n", EXPRESSpass,
                 s->symbol.name, OBJget_type( s->type ) );
    }

    HASHdo_init( s->symbol_table, &it, '*' );
    while( (ep = HASHdo( &it )) ) {
        switch( ep->type ) {
            case OBJ_SCHEMA:
                if( is_not_resolvable( ( Schema )ep->data ) ) {
                    break;
                }
                SCOPEresolve_expressions_statements( ( Scope )ep->data );
                break;
            case OBJ_ENTITY:
                ENTITYresolve_expressions( ( Entity )ep->data );
                break;
            case OBJ_FUNCTION:
                ALGresolve_expressions_statements( ( Scope )ep->data, ( ( Scope )ep->data )->u.func->body );
                break;
            case OBJ_PROCEDURE:
                ALGresolve_expressions_statements( ( Scope )ep->data, ( ( Scope )ep->data )->u.proc->body );
                break;
            case OBJ_RULE:
                ALGresolve_expressions_statements( ( Scope )ep->data, ( ( Scope )ep->data )->u.rule->body );

                WHEREresolve( RULEget_where( ( Scope )ep->data ), ( Scope )ep->data, 0 );
                break;
            case OBJ_VARIABLE:
                v = ( Variable )ep->data;
                TYPEresolve_expressions( v->type, s );
                if( v->initializer ) {
                    EXPresolve( v->initializer, s, v->type );
                }
                break;
            case OBJ_TYPE:
                TYPEresolve_expressions( ( Type )ep->data, s );
                break;
            default:
                /* ignored everything else */
                break;
        }
    }
}

void ALGresolve_expressions_statements( Scope s, Linked_List statements ) {
    int status = 0;

    if( print_objects_while_running & OBJ_ALGORITHM_BITS &
            OBJget_bits( s->type ) ) {
        fprintf( stderr, "pass %d: %s (%s)\n", EXPRESSpass,
                 s->symbol.name, OBJget_type( s->type ) );
    }

    SCOPEresolve_expressions_statements( s );
    STMTlist_resolve( statements, s );

    s->symbol.resolved = status;
}

