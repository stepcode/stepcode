

/**
 * Module:  Scope \file scope.c
 *
 *  This module implements a hierarchical (i.e., scoped)
 *  symbol table.  The symbol table can store definitions of entities,
 *  types, algorithms, and variables, as well as containing a list
 *  of subscopes.
 *
 * Constants:
 *  SCOPE_NULL  - the null scope
 */

/*
 * This software was developed by U.S. Government employees as part of
 * their official duties and is not subject to copyright.
 *
 * $Log: scope.c,v $
 * Revision 1.9  1997/01/21 19:19:51  dar
 * made C++ compatible
 *
 * Revision 1.8  1995/06/08  22:59:59  clark
 * bug fixes
 *
 * Revision 1.7  1995/04/05  13:55:40  clark
 * CADDETC preval
 *
 * Revision 1.6  1993/10/15  18:48:48  libes
 * CADDETC certified
 *
 * Revision 1.5  1993/01/19  22:16:43  libes
 * *** empty log message ***
 *
 * Revision 1.4  1992/08/18  17:13:43  libes
 * rm'd extraneous error messages
 *
 * Revision 1.3  1992/06/08  18:06:57  libes
 * prettied up interface to print_objects_when_running
 */

#define SCOPE_C
#include "express/scope.h"
#include "express/resolve.h"

void SCOPEinitialize( void ) {
}

/**
 * \sa SCOPEget_entities()
 */
void SCOPE_get_entities( Scope scope, Linked_List result ) {
    Hash_Iterator it;
    Symbol *ep;

    HASHdo_init( scope->symbol_table, &it, OBJ_ENTITY );
    while ( (ep = HASHdo(&it)) ) {
        LISTadd_last(result, ep->data);
    }
}

/**
 * \sa SCOPEget_functions()
 */
void SCOPE_get_functions( Scope scope, Linked_List result ) {
    Hash_Iterator it;
    Symbol *ep;

    HASHdo_init( scope->symbol_table, &it, OBJ_FUNCTION );
    while ( (ep = HASHdo(&it)) ) {
        LISTadd_last(result, ep->data);
    }
}
/**
 * \sa SCOPE_get_functions()
 */
Linked_List SCOPEget_functions( Scope scope ) {
    Linked_List result = LISTcreate();
    SCOPE_get_functions( scope, result );
    return( result );
}

/**
 * \sa SCOPEget_rules()
 */
void SCOPE_get_rules( Scope scope, Linked_List result ) {
    Hash_Iterator it;
    Symbol *ep;
    
    HASHdo_init( scope->symbol_table, &it, OBJ_RULE );
    while ( (ep = HASHdo(&it)) ) {
        LISTadd_last(result, ep->data);
    }
}
/**
 * \sa SCOPE_get_rules()
 */
Linked_List SCOPEget_rules( Scope scope ) {
    Linked_List result = LISTcreate();
    SCOPE_get_rules( scope, result );
    return( result );
}


/**
 ** \param scope scope to examine
 ** \return entities defined locally
 **
 ** Retrieve a list of the entities defined locally in a scope.
 **
 ** \note This function is considerably faster than
 **  SCOPEget_entities_superclass_order(), and should be used whenever
 **  the order of the entities on the list is not important.
 */
Linked_List SCOPEget_entities( Scope scope ) {
    Linked_List result = LISTcreate();
    SCOPE_get_entities( scope, result );
    return( result );
}

/**
 * \sa SCOPEget_entities_superclass_order()
 */
void SCOPE_dfs( Hash_Table symbols, Entity root, Linked_List result ) {
    Symbol *ep;
    Entity ent;

    if( ( ENTITYget_mark( root ) != ENTITY_MARK ) ) {
        ENTITYput_mark( root, ENTITY_MARK );
        LISTdo( ENTITYget_supertypes( root ), super, Entity )
            /* if super explicitly defined in scope, recurse. */
            /* this chops out USEd and REFd entities */
            ep = HASHsearch(symbols, (Symbol) {.name = ENTITYget_name( super )}, HASH_FIND);
            if (ep) {
                ent = ep->data;
                SCOPE_dfs( symbols, ent, result);
            }
        LISTod
        LISTadd_last( result, root );
    }
}

/**
 ** \param scope scope to examine
 ** \return entities defined locally
 **
 ** Retrieve a list of the entities defined locally in a scope.
 **
 ** \note The list returned is ordered such that an entity appears before all of its subtypes.
 ** \sa SCOPEget_entities()
 */
Linked_List SCOPEget_entities_superclass_order( Scope scope ) {
    Linked_List result;
    Hash_Iterator it;
    Symbol *ep;
    
    result = LISTcreate();
    ++ENTITY_MARK;
    
    HASHdo_init(scope->symbol_table, &it, OBJ_ENTITY);
    while( (ep = HASHdo(&it)) ) {        
        SCOPE_dfs( scope->symbol_table, ep->data, result );    
    }
    
    return result;
}

/**
 * find an entity type, return without resolving it
 * note that object found is not actually checked, only because
 * caller is in a better position to describe the error with context
 */
extern Hash_Table EXPRESSbuiltins;

Symbol *SCOPEfind( Scope scope, char * name, int type_mask ) {
    Symbol *ep;

    /* TODO: check, this looks like it should be in the parser */
    __SCOPE_search_id++;
    
    ep = SCOPE_find( scope, name, type_mask );
    if ( ep )
        return ep;

    /* TODO: rework (order & check overall logic) */
    if ( type_mask & (SCOPE_FIND_FUNCTION | SCOPE_FIND_PROCEDURE) )
        ep = HASHsearch(EXPRESSbuiltins, (Symbol) {.name = name}, HASH_FIND);

    return ep;
}


/**
 * look up types, functions, etc.  anything not inherited through
 * the supertype/subtype hierarchy
 * EH???  -> lookup an object when the current scope is not a schema
 */
Symbol *SCOPE_find( Scope scope, char * name, int type_mask ) {
    Symbol *ep;
    Rename * rename;

    if ( scope->search_id == __SCOPE_search_id )
        return 0;

    scope->search_id = __SCOPE_search_id;

    /* go up the superscopes, looking for object */
    for (;; scope = scope->superscope) {
        ep = HASHsearch(scope->symbol_table, (Symbol) {.name = name}, HASH_FIND);
        if (ep && OBJtype_is_oneof( ep->type, type_mask ))
            return ep->data;

        if (scope->type == OBJ_SCHEMA)
            break;
    }

    if ( type_mask & (SCOPE_FIND_ENTITY | SCOPE_FIND_TYPE) ) {
        /* Occurs in a fully USE'd schema? */
        LISTdo( scope->u.schema->use_schemas, schema, Schema )
            /* follow chain'd USEs */
            if (!schema)
                continue;
            
            ep = SCOPE_find( schema, name, type_mask );
            if (ep) 
                return ep;
        LISTod;

        /* Occurs in a partially USE'd schema? */
        /* TODO: need to resolve the rename */
        ep = HASHsearch(scope->u.schema->usedict, (Symbol) {.name = name}, HASH_FIND);
        rename = !ep ? NULL : ep->data;
        if( rename && OBJtype_is_oneof(rename->type, type_mask) )
            return rename->object;
    }

    /* Occurs in a fully REF'd schema? */
    LISTdo( scope->u.schema->ref_schemas, schema, Schema )
        if (!schema)
            continue;

        ep = HASHsearch(schema->symbol_table, (Symbol) {.name = name}, HASH_FIND);
        if (ep && OBJtype_is_oneof( ep->type, type_mask ))
            return ep->data;
    LISTod;

    /* Occurs in a partially REF'd schema? */
    /* TODO: need to resolve the rename */
    ep = HASHsearch(scope->u.schema->refdict, (Symbol) {.name = name}, HASH_FIND);
    rename = !ep ? NULL : ep->data;
    if( rename && OBJtype_is_oneof(rename->type, type_mask) )
        return rename->object;

    return 0;
}
