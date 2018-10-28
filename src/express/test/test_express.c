#include <assert.h>

#include "express/express.h"

/* core */
#include "express/hash.h"
#include "express/linklist.h"
#include "expparse.h"

/* non-core */
#include "express/resolve.h"
#include "express/schema.h"

#include "../token_type.h"
#include "expscan.h"
#include "lexsupport.h"
#include "expparse.h"

#include "driver.h"
#include "fff.h"

/*
 * mock globals
 */
int yylineno;
Express yyexpresult;
Linked_List PARSEnew_schemas;
int print_objects_while_running;
YYSTYPE yylval;
int yyerrstatus;

/*
 * mock functions
 */
typedef void * (*malloc_func_t) (size_t);
typedef void   (*free_func_t)   (void *);

DEFINE_FFF_GLOBALS

FAKE_VOID_FUNC0(RESOLVEinitialize)
FAKE_VOID_FUNC0(SYMBOLinitialize)
FAKE_VOID_FUNC0(SCOPEinitialize)
FAKE_VOID_FUNC0(TYPEinitialize)
FAKE_VOID_FUNC0(VARinitialize)
FAKE_VOID_FUNC0(ENTITYinitialize)
FAKE_VOID_FUNC0(SCHEMAinitialize)
FAKE_VOID_FUNC0(CASE_ITinitialize)
FAKE_VOID_FUNC0(EXPinitialize)
FAKE_VOID_FUNC0(RESOLVEcleanup)
FAKE_VOID_FUNC0(TYPEcleanup)
FAKE_VOID_FUNC0(EXPcleanup)
FAKE_VOID_FUNC(SCHEMAdefine_use, Schema, Rename *)
FAKE_VOID_FUNC(SCHEMAdefine_reference, Schema, Rename *)
FAKE_VOID_FUNC(SCOPEresolve_subsupers, Scope)
FAKE_VOID_FUNC(SCOPEresolve_types, Scope)
FAKE_VOID_FUNC(SCOPEresolve_expressions_statements, Scope)
FAKE_VOID_FUNC(yyparse, void *, int, struct YYSTYPE *, struct YYSTATE *)
FAKE_VOID_FUNC(yyparseFree, void *, free_func_t)
FAKE_VOID_FUNC(yyparseTrace, FILE *, char *)
FAKE_VALUE_FUNC(void *, yyparseAlloc, malloc_func_t)
FAKE_VALUE_FUNC0(struct YYSTATE *, yystateAlloc)
FAKE_VALUE_FUNC(int, yylex, struct exp_scanner *, struct YYSTYPE *)
FAKE_VALUE_FUNC0(struct exp_scanner *, yylexAlloc)
FAKE_VALUE_FUNC(int, yylexInit, struct exp_scanner *, struct YYSTATE *, FILE *)

void setup() {
    RESET_FAKE(RESOLVEinitialize);
    RESET_FAKE(SYMBOLinitialize);
    RESET_FAKE(SCOPEinitialize);
    RESET_FAKE(TYPEinitialize);
    RESET_FAKE(VARinitialize);
    RESET_FAKE(ENTITYinitialize);
    RESET_FAKE(SCHEMAinitialize);
    RESET_FAKE(CASE_ITinitialize);
    RESET_FAKE(EXPinitialize);
    RESET_FAKE(RESOLVEcleanup);
    RESET_FAKE(TYPEcleanup);
    RESET_FAKE(EXPcleanup);
    RESET_FAKE(SCHEMAdefine_use);
    RESET_FAKE(SCHEMAdefine_reference);
    RESET_FAKE(SCOPEresolve_subsupers);
    RESET_FAKE(SCOPEresolve_types);
    RESET_FAKE(SCOPEresolve_expressions_statements);
    RESET_FAKE(yyparse);
    RESET_FAKE(yyparseFree);
    RESET_FAKE(yyparseTrace);
    RESET_FAKE(yyparseAlloc);
    RESET_FAKE(yystateAlloc);
    RESET_FAKE(yylex);
    RESET_FAKE(yylexAlloc);
    RESET_FAKE(yylexInit);
}

int test_express_rename_resolve() {
    Scope model;
    Schema cur_schema, ref_schema;
    Rename *use_ref;
    Entity ent;
    Symbol e, *cur_schema_id, *ent_id, *ent_ref, *ref_schema_id;

    model = SCOPEcreate(OBJ_EXPRESS);
    
    cur_schema = SCHEMAcreate();
    cur_schema->u.schema->uselist = LISTcreate();    
    e = (Symbol) {.name = "cur_schema", .type = OBJ_SCHEMA, .data = cur_schema,
                  .ref_tok = T_SCHEMA_REF, .filename = "cur.exp"};
    cur_schema_id = HASHsearch(model->symbol_table, e, HASH_INSERT);
    cur_schema->symbol = *cur_schema_id;

    ref_schema = SCHEMAcreate();
    e = (Symbol) {.name = "ref_schema", .type = OBJ_SCHEMA, .data = ref_schema,
                  .ref_tok = T_SCHEMA_REF, .filename = "ref.exp"};
    ref_schema_id = HASHsearch(model->symbol_table, e, HASH_INSERT);
    ref_schema->symbol = *ref_schema_id;

    
    e = (Symbol) {.name = "line", .type = OBJ_ENTITY,
                  .ref_tok = T_ENTITY_REF, .filename = "cur.exp"};
    ent_id = HASHsearch(ref_schema->symbol_table, e, HASH_INSERT);
    ent = ENTITYcreate(ent_id);
    ent_id->data = ent;
    
    /* TODO: one of the few cases for using SYMBOLcreate(), perhaps refactor */
    ent_ref = SYMBOLcreate("line", OBJ_ENTITY, T_ENTITY_REF, 1, "ref.exp");    
    use_ref = REN_new();
    use_ref->schema_sym = ref_schema_id;
    use_ref->old = ent_ref;
    use_ref->nnew = ent_ref;
    use_ref->rename_type = use;
    use_ref->schema = ref_schema_id->data;
    LISTadd_last(cur_schema->u.schema->uselist, use_ref);
    
    RENAMEresolve(use_ref, cur_schema);
    
    assert(use_ref->type == OBJ_ENTITY);
    return 0;
}

struct test_def tests[] = {
    {"express_rename_resolve", test_express_rename_resolve},
    {NULL}
};
