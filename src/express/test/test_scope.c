#include <assert.h>

#include "express/scope.h"

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
int __SCOPE_search_id;

int ENTITY_MARK;

Dictionary EXPRESSbuiltins;


/*
 * mock functions
 */

DEFINE_FFF_GLOBALS

FAKE_VALUE_FUNC(int, EXPRESS_fail, Express)


void setup() {
    RESET_FAKE(EXPRESS_fail);
}

int test_something() {
    /* SCOPE_find() */
    
    return 1;
}

struct test_def tests[] = {
    {"resolve_select_enum_member", test_something},
    {"resolve_entity_attribute", test_something},
    {NULL}
};
