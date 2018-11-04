

/** **********************************************************************
** Module:  Schema \file schema.c
** This module implements the Schema abstraction, which
**  basically amounts to a named symbol table.
** Constants:
**  SCHEMA_NULL - the null schema
**
************************************************************************/

/*
 * This code was developed with the support of the United States Government,
 * and is not subject to copyright.
 *
 * $Log: schema.c,v $
 * Revision 1.13  1997/01/21 19:19:51  dar
 * made C++ compatible
 *
 * Revision 1.12  1995/06/08  22:59:59  clark
 * bug fixes
 *
 * Revision 1.11  1995/04/05  13:55:40  clark
 * CADDETC preval
 *
 * Revision 1.10  1994/11/10  19:20:03  clark
 * Update to IS
 *
 * Revision 1.9  1993/10/15  18:48:48  libes
 * CADDETC certified
 *
 * Revision 1.8  1993/02/22  21:48:53  libes
 * removed old ifdeffed code
 *
 * Revision 1.7  1993/01/19  22:16:43  libes
 * *** empty log message ***
 *
 * Revision 1.6  1992/08/27  23:42:20  libes
 * *** empty log message ***
 *
 * Revision 1.5  1992/08/18  17:13:43  libes
 * rm'd extraneous error messages
 *
 * Revision 1.4  1992/06/08  18:06:57  libes
 * prettied up interface to print_objects_when_running
 */

#include "express/expbasic.h"
#include "express/schema.h"
#include "express/object.h"
#include "express/resolve.h"

int __SCOPE_search_id = 0;

/** Initialize the Schema module. */
void SCHEMAinitialize( void ) {
}


/**  SCHEMAget_name
** \param  schema schema to examine
** \return  schema name
** Retrieve the name of a schema.
** \note This function is implemented as a macro in schema.h
*/

#if 0
/** \fn SCHEMAdump
** \param schema schema to dump
** \param file file to dump to
** Dump a schema to a file.
** \note This function is provided for debugging purposes.
*/

void
SCHEMAdump( Schema schema, FILE * file ) {
    fprintf( file, "SCHEMA %s:\n", SCHEMAget_name( schema ) );
    SCOPEdump( schema, file );
    fprintf( file, "END SCHEMA %s\n\n", SCHEMAget_name( schema ) );
}
#endif

#if 0
SYMBOLprint( Symbol * s ) {
    fprintf( stderr, "%s (r:%d #:%d f:%s)\n", s->name, s->resolved, s->line, s->filename );
}
#endif

void SCHEMAadd_reference( Schema cur_schema, Symbol * ref_schema, Symbol * old, Symbol * snnew ) {
    Rename * r = REN_new();
    r->schema_sym = ref_schema;
    r->old = old;
    r->nnew = snnew;
    r->rename_type = ref;

    if( !cur_schema->u.schema->reflist ) {
        cur_schema->u.schema->reflist = LISTcreate();
    }
    LISTadd_last( cur_schema->u.schema->reflist, r );
}

void SCHEMAadd_use( Schema cur_schema, Symbol * ref_schema, Symbol * old, Symbol * snnew ) {
    Rename * r = REN_new();
    r->schema_sym = ref_schema;
    r->old = old;
    r->nnew = snnew;
    r->rename_type = use;

    if( !cur_schema->u.schema->uselist ) {
        cur_schema->u.schema->uselist = LISTcreate();
    }
    LISTadd_last( cur_schema->u.schema->uselist, r );
}

void SCHEMAdefine_reference( Schema schema, Rename * r ) {
    Rename *old;
    Symbol e, *ep;
    Symbol *sym;
    
    if( !schema->u.schema->refdict ) {
        schema->u.schema->refdict = HASHcreate();
    } 

    sym = r->nnew ? r->nnew : r->old;

    e = (Symbol) {.name = sym->name, .data = r, .type = OBJ_RENAME};
    ep = HASHsearch(schema->u.schema->refdict, e, HASH_FIND);
    old = !ep ? NULL : ep->data;
    
    if( !ep || ep->type != OBJ_RENAME || old->object != r->object ) {
        /* TODO: evaluate difference in behaviour with DICTdefine
         * if there's already an OBJ_RENAME, then DICTdefine would replace it
         * - this will instead append another
         */
        HASHsearch(schema->u.schema->refdict, e, HASH_INSERT);
    }
}

void SCHEMAdefine_use( Schema schema, Rename * r ) {
    Rename * old;
    Symbol *ep, e;
    Symbol *sym;

    if( !schema->u.schema->usedict ) {
        schema->u.schema->usedict = HASHcreate();
    }
    
    sym = r->nnew ? r->nnew : r->old;
    e = (Symbol) {.name = sym->name, .data = r, .type = OBJ_RENAME};
    
    ep = HASHsearch(schema->u.schema->usedict, e, HASH_FIND);
    old = !ep ? NULL : ep->data;
        
    if( !ep || ep->type != OBJ_RENAME || old->object != r->object ) {
        /* TODO: evaluate difference in behaviour with DICTdefine
         * if there's already an OBJ_RENAME, then DICTdefine would replace it
         * - this will instead append another
         */
        HASHsearch(schema->u.schema->usedict, e, HASH_INSERT);
    }
}

static void SCHEMA_get_entities_use( Scope scope, Linked_List result ) {
    Hash_Iterator it;
    Symbol *ep;
    Rename * rename;

    if( scope->search_id == __SCOPE_search_id ) {
        return;
    }
    scope->search_id = __SCOPE_search_id;

    /* fully USE'd schema */
    LISTdo( scope->u.schema->use_schemas, schema, Schema )
    SCOPE_get_entities( schema, result );
    SCHEMA_get_entities_use( schema, result );
    LISTod

    /* partially USE'd schema */
    if( scope->u.schema->usedict ) {
        HASHdo_init(scope->u.schema->usedict, &it, '*');
        while ( (ep = HASHdo(&it)) ) {
            rename = ep->data;
            LISTadd_last(result, rename->object);            
        }
    }
}

/** return use'd entities */
Linked_List SCHEMAget_entities_use( Scope scope ) {
    Linked_List result = LISTcreate();

    __SCOPE_search_id++;
    ENTITY_MARK++;

    SCHEMA_get_entities_use( scope, result );
    return( result );
}

/** return ref'd entities */
void SCHEMA_get_entities_ref( Scope scope, Linked_List result ) {
    Rename * rename;
    Hash_Iterator it;
    Symbol *ep;

    if( scope->search_id == __SCOPE_search_id ) {
        return;
    }
    scope->search_id = __SCOPE_search_id;

    ENTITY_MARK++;

    /* fully REF'd schema */
    LISTdo( scope->u.schema->ref_schemas, schema, Schema )
    SCOPE_get_entities( schema, result );
    /* don't go down remote schema's ref_schemas */
    LISTod

    /* partially REF'd schema */
    HASHdo_init(scope->u.schema->refdict, &it, OBJ_ENTITY);
    while ( (ep = HASHdo(&it)) ) {
        rename = ep->data;
        LISTadd_last(result, rename->object);
    }
}

/** return ref'd entities */
Linked_List SCHEMAget_entities_ref( Scope scope ) {
    Linked_List result = LISTcreate();

    __SCOPE_search_id++;
    ENTITY_MARK++;

    SCHEMA_get_entities_ref( scope, result );
    return( result );
}

/**
 * look up an attribute reference
 * if strict false, anything can be returned, not just attributes
 */
Symbol *VARfind( Scope scope, const char * name ) {
    Symbol *ep;

    /* first look up locally */
    switch( scope->type ) {
        case OBJ_ENTITY:
            ep = ENTITYfind_inherited_attribute(scope, name, NULL);
            /* strict is checked when parsing */
            if (ep)
                return ep;
            break;
        case OBJ_INCREMENT:
        case OBJ_QUERY:
        case OBJ_ALIAS:
            ep = HASHsearch(scope->symbol_table, (Symbol) {.name = name, .type = OBJ_VARIABLE}, HASH_FIND);
            /* strict is checked when parsing */
            if (ep)
                return ep;
            
            ep = VARfind( scope->superscope, name );
            if (ep)
                return ep;
    }
    return NULL;
}
