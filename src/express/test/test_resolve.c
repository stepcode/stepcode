#include <assert.h>

#include "express/resolve.h"

/* core */
#include "express/hash.h"
#include "express/linklist.h"

/* non-core */
#include "express/type.h"

#include "driver.h"
#include "fff.h"

/*
 * mock globals
 */

char * EXPRESSprogram_name;
int yylineno;
int __SCOPE_search_id;
int EXPRESSpass;
struct Scope_ * FUNC_NVL;
struct Scope_ * FUNC_USEDIN;

Type Type_Unknown;
Type Type_Dont_Care;
Type Type_Bad;
Type Type_Runtime;
Type Type_Attribute;
Type Type_Self;
Type Type_Bag_Of_Generic;
Type Type_Funcall;

struct EXPop_entry EXPop_table[OP_LAST];

int tag_count;

/*
 * mock functions
 */

DEFINE_FFF_GLOBALS

FAKE_VALUE_FUNC(Generic, SCOPEfind, Scope, char *, int)
FAKE_VALUE_FUNC(Variable, VARfind, Scope, char *, int)
FAKE_VALUE_FUNC(char *, VARget_simple_name, Variable)
FAKE_VALUE_FUNC(struct Scope_ *, ENTITYfind_inherited_entity, struct Scope_ *, char *, int)
FAKE_VALUE_FUNC(Variable, ENTITYresolve_attr_ref, Entity, Symbol *, Symbol *)
FAKE_VALUE_FUNC(Variable, ENTITYget_named_attribute, Entity, char *)
FAKE_VALUE_FUNC(int, ENTITYdeclares_variable, Entity, Variable)
FAKE_VALUE_FUNC(int, EXPRESS_fail, Express)

void setup() {
    RESET_FAKE(SCOPEfind);
    RESET_FAKE(VARfind);
    RESET_FAKE(VARget_simple_name);
    RESET_FAKE(ENTITYfind_inherited_entity);
    RESET_FAKE(ENTITYresolve_attr_ref);
    RESET_FAKE(ENTITYget_named_attribute);
    RESET_FAKE(ENTITYdeclares_variable);
    RESET_FAKE(EXPRESS_fail);    
}

int test_something() {

/*
    EXP_resolve();
    
    ENTITYresolve_subtype_expression();
    
    TYPE_resolve(typ);

    STMTresolve();
    
    SCOPEresolve_types();
    
    SCOPEresolve_subsupers();
    
    ENTITYresolve_supertypes();
    
    SCOPEresolve_expressions_statements();*/
  
    return 1;
}

struct test_def tests[] = {
    {"resolve_select_enum_member", test_something},
    {"resolve_entity_attribute", test_something},
    {NULL}
};

