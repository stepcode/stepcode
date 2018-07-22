#include <assert.h>

#include "express/schema.h"

/* core */
#include "express/hash.h"
#include "express/linklist.h"

/* non-core */

#include "driver.h"
#include "fff.h"

/*
 * mock globals
 */
char * EXPRESSprogram_name;
int yylineno;

int ENTITY_MARK;


/*
 * mock functions
 */

DEFINE_FFF_GLOBALS

FAKE_VALUE_FUNC(int, EXPRESS_fail, Express)
FAKE_VOID_FUNC(SCOPE_get_entities, Scope, Linked_List)
FAKE_VALUE_FUNC(Variable, ENTITYfind_inherited_attribute, struct Scope_ *, char *, struct Symbol_ **)

void setup() {
    RESET_FAKE(EXPRESS_fail)
    RESET_FAKE(SCOPE_get_entities)
    RESET_FAKE(ENTITYfind_inherited_attribute)
}

int test_something() {
    /* SCHEMAdefine_reference();

    SCHEMAdefine_use();

    SCHEMA_get_entities_ref();

    VARfind();*/
    
    return 1;
}

struct test_def tests[] = {
    {"resolve_select_enum_member", test_something},
    {"resolve_entity_attribute", test_something},
    {NULL}
};
