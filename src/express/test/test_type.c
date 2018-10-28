#include <assert.h>

#include "express/type.h"

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

int tag_count;

/*
 * mock functions
 */

DEFINE_FFF_GLOBALS

FAKE_VALUE_FUNC(int, EXPRESS_fail, Express)


void setup() {
    RESET_FAKE(EXPRESS_fail)
    TYPEinitialize();
}

int test_type_create_user_defined_tag() {
    Schema scope;
    Type t, g, chk;
    Symbol e, *func_id, *tag_id;
    
    scope = SCHEMAcreate();
    
    e = (Symbol) {.name = "func1", .type = OBJ_FUNCTION,
                  .ref_tok = T_FUNCTION_REF, .data = ALGcreate(OBJ_FUNCTION)};
    func_id = HASHsearch(scope->symbol_table, e, HASH_INSERT);

    e = (Symbol) {.name = "item1", .type = OBJ_TAG,
                  .ref_tok = T_RULE_LABEL_REF};
    tag_id = HASHsearch(scope->symbol_table, e, HASH_INSERT);
    
    g = TYPEcreate(generic_);
    t = TYPEcreate_nostab(tag_id, func_id->data, OBJ_TAG);
    
    chk = TYPEcreate_user_defined_tag(g, func_id->data, tag_id);
    
    assert(chk == t);
    
    return 0;
}

struct test_def tests[] = {
    {"type_create_user_defined_tag", test_type_create_user_defined_tag},
    {NULL}
};
