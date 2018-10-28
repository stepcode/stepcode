#include <assert.h>

#include "express/expr.h"

/* core */
#include "express/hash.h"
#include "express/linklist.h"
#include "expparse.h"

/* non-core */
#include "express/dict.h"
#include "express/variable.h"

#include "driver.h"
#include "fff.h"

/*
 * mock globals
 */

char * EXPRESSprogram_name;
int yylineno;
int __SCOPE_search_id;

/*
 * mock functions
 */

DEFINE_FFF_GLOBALS

FAKE_VALUE_FUNC(int, EXPRESS_fail, Express)
FAKE_VALUE_FUNC(Variable, ENTITYfind_inherited_attribute, struct Scope_ *, char *, struct Symbol_ **)
FAKE_VALUE_FUNC(Variable, ENTITYresolve_attr_ref, Entity, Symbol *, Symbol *)
FAKE_VALUE_FUNC(struct Scope_ *, ENTITYfind_inherited_entity, struct Scope_ *, char *, int)
FAKE_VOID_FUNC(EXP_resolve, Expression, Scope, Type)

void setup() {
    EXPinitialize();
    
    RESET_FAKE(EXPRESS_fail);
    RESET_FAKE(ENTITYfind_inherited_attribute);
    RESET_FAKE(ENTITYresolve_attr_ref);
    RESET_FAKE(ENTITYfind_inherited_entity);
    RESET_FAKE(EXP_resolve);
}

void EXP_resolve_type_handler(Expression exp, Scope cxt, Type typ) {
    (void) typ;
    Symbol *ep;
    
    ep = HASHsearch(cxt->symbol_table, exp->symbol, HASH_FIND);
    if (!ep)
        return;
    
    exp->type = ep->data;
    exp->return_type = ep->data;
    exp->symbol.resolved = RESOLVED;
}

int test_resolve_select_enum_member() {
    Schema scope;
    Symbol e;
    Symbol *e_type_id, *enum_id_inner, *enum_id_outer, *s_type_id;
    Type enum_typ, select_typ, chk_typ;
    TypeBody tb;
    Expression expr, op1, op2, exp_enum_id;

    /*
     * boilerplate: create objects, populate symbol tables
     */
    scope = SCHEMAcreate();

    e = (Symbol) {.name = "sel1", .type = OBJ_TYPE, .ref_tok = T_TYPE_REF};
    s_type_id = HASHsearch(scope->symbol_table, e, HASH_INSERT);
    
    /* TODO: check use of OBJ_ENUM / OBJ_TYPE */
    e = (Symbol) {.name ="enum1", .type = OBJ_TYPE, .ref_tok = T_TYPE_REF};
    e_type_id = HASHsearch(scope->symbol_table, e, HASH_INSERT);
    enum_typ = TYPEcreate_name(e_type_id);
    enum_typ->symbol_table = HASHcreate();
    enum_typ->symbol = e;
    e_type_id->data = enum_typ;
    
    /* TODO: check use of OBJ_ENUM / OBJ_TYPE */
    /* TODO: OBJ_ENUM / OBJ_EXPRESSION are used interchangeably, this is confusing. */
    e = (Symbol) {.name = "val1", .type = OBJ_ENUM, .ref_tok = T_ENUMERATION_REF};
    enum_id_outer = HASHsearch(scope->enum_table, e, HASH_INSERT);
    enum_id_inner = HASHsearch(enum_typ->symbol_table, e, HASH_INSERT);
    exp_enum_id = EXPcreate(enum_typ);
    exp_enum_id->symbol = e;
    exp_enum_id->u.integer = 1;
    enum_id_outer->data = exp_enum_id;
    enum_id_inner->data = exp_enum_id;
    
    tb = TYPEBODYcreate(enumeration_);
    tb->list = LISTcreate();
    LISTadd_last(tb->list, enum_id_inner);
    enum_typ->u.type->body = tb;
    
    select_typ = TYPEcreate_name(s_type_id);
    s_type_id->data = select_typ;
    tb = TYPEBODYcreate(select_);
    tb->list = LISTcreate();
    LISTadd_last(tb->list, enum_typ);
    select_typ->u.type->body = tb;
    
    op1 = EXPcreate_from_symbol(Type_Identifier, s_type_id);
    op2 = EXPcreate_from_symbol(Type_Identifier, enum_id_inner);
    expr = BIN_EXPcreate(OP_DOT, op1, op2);    

    /*
     * test: sel_ref '.' enum_id
     * expectation: enum_typ
     */
    EXP_resolve_fake.custom_fake = EXP_resolve_type_handler;
    
    chk_typ = EXPresolve_op_dot(expr, scope);

    assert(EXP_resolve_fake.call_count == 1);
    assert(expr->e.op1->type == select_typ);
    assert(chk_typ == enum_typ);
    
    /* in case of error SIGABRT will be raised (and non-zero returned) */
    
    return 0;
}

/* TODO: remove DICTlookup after eliminating DICT_type */
void EXP_resolve_entity_handler(Expression exp, Scope cxt, Type unused) {
    (void) unused;
    Symbol *ep;
    Entity ent;
    
    ep = HASHsearch(cxt->symbol_table, exp->symbol, HASH_FIND);
    if (!ep)
        return;
    
    ent = ep->data;
    exp->type = ent->u.entity->type;
    exp->return_type = ent->u.entity->type;
    exp->symbol.resolved = RESOLVED;
}

Variable ENTITY_resolve_attr_handler(Entity ent, Symbol *grp_ref, Symbol *attr_ref) {
    (void) grp_ref;
    Symbol *ep;
    
    ep = HASHsearch(ent->symbol_table, *attr_ref, HASH_FIND);
    if (!ep)
        return NULL;

    return ep->data;   
}

int test_resolve_entity_attribute() {
    Schema scope;
    Symbol e, *e_type_id, *attr_id;
    Entity ent;
    Type attr_typ, chk_typ;
    TypeBody tb;
    Expression exp_attr, expr, op1, op2;
    Variable var_attr;
    Linked_List explicit_attr_list;

    /*
     * boilerplate: create objects, populate symbol tables
     */
    scope = SCHEMAcreate();

    e = (Symbol) {.name = "entity1", .type = OBJ_ENTITY, .ref_tok = T_ENTITY_REF};
    e_type_id = HASHsearch(scope->symbol_table, e, HASH_INSERT);
    ent = ENTITYcreate(e_type_id);
    e_type_id->data = ent;
    
    e = (Symbol) {.name = "attr1", .type = OBJ_VARIABLE, .ref_tok = T_SIMPLE_REF};
    attr_id = HASHsearch(ent->symbol_table, e, HASH_INSERT);
    exp_attr = EXPcreate_from_symbol(Type_Attribute, attr_id);    
    tb = TYPEBODYcreate(number_);
    attr_typ = TYPEcreate_from_body_anonymously(tb);
    attr_typ->superscope = ent;
    var_attr = VARcreate(exp_attr, attr_typ);
    var_attr->flags.attribute = 1;
    attr_id->data = var_attr;

    explicit_attr_list = LISTcreate();
    LISTadd_last(explicit_attr_list, var_attr);    
    LISTadd_last(ent->u.entity->attributes, explicit_attr_list);

    op1 = EXPcreate_from_symbol(Type_Identifier, e_type_id);
    op2 = EXPcreate_from_symbol(Type_Attribute, attr_id);
    expr = BIN_EXPcreate(OP_DOT, op1, op2);
    
    /*
     * test: entity_ref '.' attribute_id
     * expectation: attr_typ
     */
    EXP_resolve_fake.custom_fake = EXP_resolve_entity_handler;
    ENTITYresolve_attr_ref_fake.custom_fake = ENTITY_resolve_attr_handler;
    
    chk_typ = EXPresolve_op_dot(expr, scope);

    assert(EXP_resolve_fake.call_count == 1);
    assert(expr->e.op1->type == ent->u.entity->type);
    assert(chk_typ == attr_typ);
    
    /* in case of error SIGABRT will be raised (and non-zero returned) */
    
    return 0;
}

struct test_def tests[] = {
    {"resolve_select_enum_member", test_resolve_select_enum_member},
    {"resolve_entity_attribute", test_resolve_entity_attribute},
    {NULL}
};
