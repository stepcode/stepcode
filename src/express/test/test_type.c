#include <assert.h>

#include "express/type.h"

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

int tag_count;

/*
 * mock functions
 */

DEFINE_FFF_GLOBALS

FAKE_VALUE_FUNC(int, EXPRESS_fail, Express)


void setup() {
    RESET_FAKE(EXPRESS_fail)
}

int test_something() {
    /* TYPEcreate_user_defined_tag() */
    
    return 1;
}

struct test_def tests[] = {
    {"resolve_select_enum_member", test_something},
    {"resolve_entity_attribute", test_something},
    {NULL}
};
