
#include <assert.h>

#include "express/resolve.h"

/* core */
#include "express/hash.h"
#include "express/linklist.h"

/* non-core */
#include "express/type.h"
#include "expparse.h"

#include "driver.h"
#include "fff.h"

/*
 * mock globals
 */

char * EXPRESSprogram_name;
int EXPRESSpass;
int yylineno;
int print_objects_while_running;

/*
 * mock functions
 */

DEFINE_FFF_GLOBALS

FAKE_VOID_FUNC(ENTITYresolve_supertypes, Entity)
FAKE_VOID_FUNC(ENTITYresolve_subtypes, Schema)
FAKE_VOID_FUNC(TYPE_resolve, Type *)
FAKE_VOID_FUNC(TYPEresolve_expressions, Type, Scope)
FAKE_VOID_FUNC(EXP_resolve, Expression, Scope, Type)
FAKE_VOID_FUNC(STMTlist_resolve, Linked_List, Scope)
FAKE_VOID_FUNC(ENTITYresolve_expressions, Entity)

FAKE_VALUE_FUNC(int, WHEREresolve, Linked_List, Scope, int)
FAKE_VALUE_FUNC(int, EXPRESS_fail, Express)

void setup() {
    RESET_FAKE(ENTITYresolve_supertypes);
    RESET_FAKE(ENTITYresolve_subtypes);
    RESET_FAKE(TYPE_resolve);
    RESET_FAKE(TYPEresolve_expressions);
    RESET_FAKE(EXP_resolve);
    RESET_FAKE(STMTlist_resolve);
    RESET_FAKE(ENTITYresolve_expressions);
    RESET_FAKE(WHEREresolve);
    RESET_FAKE(EXPRESS_fail);
}

int test_scope_resolve_expr_stmt() {
    Schema scope;
    Type sel, ent_base;
    Entity ent;
    Symbol e, *ent_id, *sel_id, *ent_ref;
    
    scope = SCHEMAcreate();
    
    e = (Symbol) {.name = "ent", .type = OBJ_ENTITY, .ref_tok = T_ENTITY_REF};
    ent_id = HASHsearch(scope->symbol_table, e, HASH_INSERT);
    ent = ENTITYcreate(ent_id);
    ent->superscope = scope;
    ent_id->data = ent;
    
    e = (Symbol) {.name = "sel_typ", .type = OBJ_TYPE, .ref_tok = T_TYPE_REF};
    sel_id = HASHsearch(scope->symbol_table, e, HASH_INSERT);
    sel = TYPEcreate(select_);
    sel->symbol = *sel_id;
    sel->u.type->body->list = LISTcreate();
    sel->superscope = scope;
    sel_id->data = sel;
    
    ent_ref = SYMBOLcreate("ent", OBJ_ENTITY, T_ENTITY_REF, 1, "test_5");
    ent_base = TYPEcreate_name(ent_ref);
    ent_base->superscope = sel;
    LISTadd_last(sel->u.type->body->list, ent_base);
    
    SCOPEresolve_expressions_statements(scope);
    
    assert(ENTITYresolve_expressions_fake.call_count == 1);
    assert(TYPEresolve_expressions_fake.call_count == 1);
    
    return 0;
}

int test_scope_resolve_subsupers() {
    Schema scope;
    Type sel, ent_base;
    Entity ent;
    Symbol e, *ent_id, *sel_id, *ent_ref;
    
    scope = SCHEMAcreate();
    
    e = (Symbol) {.name = "ent", .type = OBJ_ENTITY, .ref_tok = T_ENTITY_REF};
    ent_id = HASHsearch(scope->symbol_table, e, HASH_INSERT);
    ent = ENTITYcreate(ent_id);
    ent->superscope = scope;
    ent_id->data = ent;
    
    e = (Symbol) {.name = "sel_typ", .type = OBJ_TYPE, .ref_tok = T_TYPE_REF};
    sel_id = HASHsearch(scope->symbol_table, e, HASH_INSERT);
    sel = TYPEcreate(select_);
    sel->symbol = *sel_id;
    sel->u.type->body->list = LISTcreate();
    sel->superscope = scope;
    sel_id->data = sel;
    
    ent_ref = SYMBOLcreate("ent", OBJ_ENTITY, T_ENTITY_REF, 1, "test_6");
    ent_base = TYPEcreate_name(ent_id);
    ent_base->superscope = sel;    
    LISTadd_last(sel->u.type->body->list, ent_base);

    SCOPEresolve_subsupers(scope);
    
    assert(TYPE_resolve_fake.call_count == 1);
    assert(ENTITYresolve_supertypes_fake.call_count == 1);
    assert(ENTITYresolve_subtypes_fake.call_count == 1);
    
    return 0;
}


struct test_def tests[] = {
    {"scope_resolve_expr_stmt", test_scope_resolve_expr_stmt},
    {"scope_resolve_subsupers", test_scope_resolve_subsupers},
    {NULL}
};
