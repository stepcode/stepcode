#include <assert.h>

#include "express/scope.h"

/* core */
#include "express/hash.h"
#include "express/linklist.h"
#include "expparse.h"

/* non-core */
#include "driver.h"
#include "fff.h"

/*
 * mock globals
 */

char * EXPRESSprogram_name;
int yylineno;
int __SCOPE_search_id;

int ENTITY_MARK;

Hash_Table EXPRESSbuiltins;


/*
 * mock functions
 */

DEFINE_FFF_GLOBALS

FAKE_VALUE_FUNC(int, EXPRESS_fail, Express)


void setup() {
    RESET_FAKE(EXPRESS_fail);
}

int test_scope_find() {
    Schema scope;
    Type sel, ent_base, chk_sel;
    Entity ent, chk_ent;
    Symbol e, *ent_id, *sel_id;
    
    scope = SCHEMAcreate();
    
    e = (Symbol) {.name = "ent", .type = OBJ_ENTITY, .ref_tok = T_ENTITY_REF};
    ent_id = HASHsearch(scope->symbol_table, e, HASH_INSERT);
    ent = ent_id->data = ENTITYcreate(ent_id);
    ent->superscope = scope;

    ent_base = TYPEcreate_name(ent_id);
    ent_base->superscope = scope;
    
    e = (Symbol) {.name = "sel_typ", .type = OBJ_TYPE,
                  .ref_tok = T_TYPE_REF, .data = TYPEcreate(select_)};
    sel_id = HASHsearch(scope->symbol_table, e, HASH_INSERT);
    sel = sel_id->data;
    sel->u.type->body->list = LISTcreate();
    sel->superscope = scope;
    LISTadd_last(sel->u.type->body->list, ent_base);
        
    scope->search_id = -1;
    chk_sel = SCOPE_find(scope, "sel_typ", SCOPE_FIND_ENTITY | SCOPE_FIND_TYPE);
    
    scope->search_id = -1;
    chk_ent = SCOPE_find(scope, "ent", SCOPE_FIND_ENTITY | SCOPE_FIND_TYPE);
    
    assert(chk_sel == sel);
    assert(chk_ent == ent);
    
    return 0;
}

struct test_def tests[] = {
    {"scope_find", test_scope_find},
    {NULL}
};
