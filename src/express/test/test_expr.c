#include "express/expr.h"

/* core */
#include "express/memory.h"
#include "express/error.h"
#include "express/hash.h"
#include "express/linklist.h"

/* non-core */
#include "express/dict.h"
#include "express/object.h"
#include "express/info.h"

/*
 * missing header definition
 */
Type EXPresolve_op_dot( Expression expr, Scope scope );

/*
 * TODO: keep interface, but refactor for test
 */
void EXPRESSinitialize( void ) {}

/*
 * TODO: move to error.c
 */
int EXPRESS_fail( Express model ) { return 1; }

/*
 * TODO: move to memory.c
 */
Type TYPEcreate( enum type_enum type ) { return NULL; }
Type TYPEcreate_name( Symbol * symbol ) { return NULL; }
TypeBody TYPEBODYcreate( enum type_enum type ) { return NULL; }

Schema SCHEMAcreate( void ) { return NULL; }
Symbol *SYMBOLcreate( char * name, int line, const char * filename ) { return NULL; }

/*
 * mock globals
 * TODO: what needs initialising?
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
 * TODO: mock functions
 */
Variable ENTITYfind_inherited_attribute( struct Scope_ *entity, char * name, struct Symbol_ ** down_sym ) { return NULL; }

void EXP_resolve( Expression expr, Scope scope, Type typecheck ) {}

Variable ENTITYresolve_attr_ref( Entity e, Symbol * grp_ref, Symbol * attr_ref ) { return NULL; }

struct Scope_ * ENTITYfind_inherited_entity( struct Scope_ *entity, char * name, int down ) { return NULL; }

Scope SCOPEcreate_tiny( char type ) { return NULL; }

Variable VARcreate( Expression name, Type type ) { return NULL; }


int main(int argc, char *argv[]) {
    Schema scope;
    Symbol *e_type_id, *enum_id, *s_type_id;
    Type enum_typ, select_typ;
    TypeBody tb;
    Expression expr, op1, op2, exp_enum_id;

    /*
     * setup
     */
    EXPRESSinitialize();
    Type_Identifier = TYPEcreate(identifier_);

    /* TODO: SCHEMAcreate should do this */
    scope = SCHEMAcreate();
    if (!scope->symbol_table)
        scope->symbol_table = DICTcreate(50);
    if (!scope->enum_table)
        scope->enum_table = DICTcreate(50);
    
    /*
     * initial code to faciliate code path under test (use of DICT_type)
     */
    s_type_id = SYMBOLcreate("sel1", 1, "test1");
    e_type_id = SYMBOLcreate("enum1", 1, "test1");
    enum_id = SYMBOLcreate("val1", 1, "test1");
    
    enum_typ = TYPEcreate_name(e_type_id);
    
    exp_enum_id = EXPcreate(enum_typ);
    exp_enum_id->symbol = *enum_id;
    exp_enum_id->u.integer = 1;
    
    tb = TYPEBODYcreate(enumeration_);
    tb->list = LISTcreate();
    LISTadd_last(tb->list, enum_id);
    enum_typ->u.type->body = tb;
    
    DICT_define(scope->symbol_table, e_type_id->name, enum_typ, &enum_typ->symbol, OBJ_TYPE);
    
    /*
     * TODO: someone did half a job with OBJ_ENUM / OBJ_EXPRESSION
     * now it's confusing when reading the source
     */
    DICT_define(scope->enum_table, exp_enum_id->symbol.name, exp_enum_id, &exp_enum_id->symbol, OBJ_EXPRESSION);
    
    if (!enum_typ->symbol_table)
        enum_typ->symbol_table = DICTcreate(50);
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
     * test: enum_ref '.' enum_id
     */
    EXPresolve_op_dot(expr, scope);
    
    /* in case of error will exit via abort() */
    
    return 0;
}

/* TODO: additional test for entity_ attribute, variable */
