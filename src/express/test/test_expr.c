#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "express/expr.h"

/* core */
#include "express/alloc.h"
#include "express/memory.h"
#include "express/error.h"
#include "express/hash.h"
#include "express/linklist.h"

/* non-core */
#include "express/dict.h"

#include "fff.h"

DEFINE_FFF_GLOBALS

/*
 * missing header definition
 */
Type EXPresolve_op_dot( Expression expr, Scope scope );

/*
 * mock globals
 */

char * EXPRESSprogram_name;
int yylineno;
int __SCOPE_search_id;

Type Type_Unknown;
Type Type_Identifier;
Type Type_Real;
Type Type_Integer;
Type Type_Dont_Care;
Type Type_Bad;
Type Type_Runtime;
Type Type_Logical;
Type Type_Generic;
Type Type_Binary;
Type Type_Entity;
Type Type_Aggregate;
Type Type_Expression;
Type Type_Query;
Type Type_Attribute;

Error ERROR_warn_unsupported_lang_feat;
Error WARNING_case_skip_label;
Error ERROR_undefined_attribute;

/*
 * mock functions
 */

FAKE_VALUE_FUNC(int, EXPRESS_fail, Express)
FAKE_VALUE_FUNC(Variable, ENTITYfind_inherited_attribute, struct Scope_ *, char *, struct Symbol_ **)
FAKE_VALUE_FUNC(Variable, ENTITYresolve_attr_ref, Entity, Symbol *, Symbol *)
FAKE_VALUE_FUNC(struct Scope_ *, ENTITYfind_inherited_entity, struct Scope_ *, char *, int)
FAKE_VALUE_FUNC(Variable, VARcreate, Expression, Type)
FAKE_VOID_FUNC(EXP_resolve, Expression, Scope, Type)

void setup() {
    Type_Identifier = TYPEcreate(identifier_);    
}

/*
 * mock resolve operation
 */ 
void EXP_resolve_handler(Expression exp, Scope cxt, Type typ) {
    (void) typ;
    Type res_typ = DICTlookup(cxt->symbol_table, exp->symbol.name); 
    exp->type = res_typ;
    exp->return_type = res_typ;
    exp->symbol.resolved = RESOLVED;
}

int test_resolve_op_dot() {
    Schema scope;
    Symbol *e_type_id, *enum_id, *s_type_id;
    Type enum_typ, select_typ, chk_typ;
    TypeBody tb;
    Expression expr, op1, op2, exp_enum_id;

    /*
     * boilerplate: create objects, populate symbol tables
     */
    scope = SCHEMAcreate();

    s_type_id = SYMBOLcreate("sel1", 1, "test1");
    e_type_id = SYMBOLcreate("enum1", 1, "test1");
    enum_id = SYMBOLcreate("val1", 1, "test1");
    
    enum_typ = TYPEcreate_name(e_type_id);
    enum_typ->symbol_table = DICTcreate(50);
    
    exp_enum_id = EXPcreate(enum_typ);
    exp_enum_id->symbol = *enum_id;
    exp_enum_id->u.integer = 1;
    
    tb = TYPEBODYcreate(enumeration_);
    tb->list = LISTcreate();
    LISTadd_last(tb->list, enum_id);
    enum_typ->u.type->body = tb;
    
    DICT_define(scope->symbol_table, e_type_id->name, enum_typ, &enum_typ->symbol, OBJ_TYPE);
    
    /* TODO: OBJ_ENUM / OBJ_EXPRESSION are used interchangeably, this is confusing. */
    DICT_define(scope->enum_table, exp_enum_id->symbol.name, exp_enum_id, &exp_enum_id->symbol, OBJ_EXPRESSION);
    DICT_define(enum_typ->symbol_table, enum_id->name, exp_enum_id, enum_id, OBJ_EXPRESSION);
    
    select_typ = TYPEcreate_name(s_type_id);
    tb = TYPEBODYcreate(select_);
    tb->list = LISTcreate();
    LISTadd_last(tb->list, enum_typ);
    select_typ->u.type->body = tb;
    DICT_define(scope->symbol_table, s_type_id->name, select_typ, &select_typ->symbol, OBJ_TYPE);
    
    op1 = EXPcreate_from_symbol(Type_Identifier, s_type_id);
    op2 = EXPcreate_from_symbol(Type_Identifier, enum_id);
    expr = BIN_EXPcreate(OP_DOT, op1, op2);    

    /*
     * test: sel_ref '.' enum_id
     * expectation: enum_typ
     */
    EXP_resolve_fake.custom_fake = EXP_resolve_handler;
    
    chk_typ = EXPresolve_op_dot(expr, scope);

    assert(EXP_resolve_fake.call_count == 1);
    assert(expr->e.op1->type == select_typ);
    assert(chk_typ == enum_typ);
    
    /* in case of error SIGABRT will be raised (and non-zero returned) */
    
    return 0;
}

struct test_def {
    const char *name;
    int (*testfunc) (void);
};

int main(int argc, char *argv[]) {
    int status;
 
    struct test_def tests[] = {
        {"resolve_op_dot", test_resolve_op_dot}
    };
    
    /* enable libexpress allocator */
    MEMinit();
    
    argc--;
    status = 0;
    if (argc) {
        int test_counter = argc;
        
        /* selected tests */
        for (int i=1; i <= argc; i++) {
            for (unsigned int j=0; j < (sizeof tests / sizeof tests[0]); j++) {
                const char *test_name = tests[j].name;
                int (*test_ptr) (void) = tests[j].testfunc;
                
                if (!strcmp(argv[i], test_name)) {
                    test_counter--;
                    setup();
                    status |= test_ptr();
                }
            }
        }
        
        if (test_counter)
            fprintf(stderr, "WARNING: some tests not found...\n");
    } else {
        /* all tests */
        for (unsigned int j=0; j < (sizeof tests / sizeof tests[0]); j++) {
            int (*test_ptr) (void) = tests[j].testfunc;
            status |= test_ptr();
        }
    }

    return status;
}

/* TODO: additional test for entity_ attribute, variable */
