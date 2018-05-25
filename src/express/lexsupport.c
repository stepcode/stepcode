#include <stdio.h>
#include <stdlib.h>

#include "sc_memmgr.h"

#include "lexsupport.h"
#include "expparse.h"
#include "expscan.h"

struct kw_token_def reserved_keywords[] = {
    {"ABS", T_ABS}, 
    {"ABSTRACT", T_ABSTRACT}, 
    {"ACOS", T_ACOS}, 
    {"AGGREGATE", T_AGGREGATE}, 
    {"ALIAS", T_ALIAS}, 
    {"AND", T_AND}, 
    {"ANDOR", T_ANDOR}, 
    {"ARRAY", T_ARRAY}, 
    {"AS", T_AS}, 
    {"ASIN", T_ASIN}, 
    {"ATAN", T_ATAN}, 
    {"BAG", T_BAG}, 
    {"BASED_ON", T_BASED_ON}, 
    {"BEGIN", T_BEGIN}, 
    {"BINARY", T_BINARY}, 
    {"BLENGTH", T_BLENGTH}, 
    {"BOOLEAN", T_BOOLEAN}, 
    {"BY", T_BY}, 
    {"CASE", T_CASE}, 
    {"CONSTANT", T_CONSTANT}, 
    {"CONST_E", T_CONST_E}, 
    {"COS", T_COS}, 
    {"DERIVE", T_DERIVE}, 
    {"DIV", T_IDIV}, 
    {"ELSE", T_ELSE}, 
    {"END", T_END}, 
    {"END_ALIAS", T_END_ALIAS}, 
    {"END_CASE", T_END_CASE}, 
    {"END_CONSTANT", T_END_CONSTANT}, 
    {"END_ENTITY", T_END_ENTITY}, 
    {"END_FUNCTION", T_END_FUNCTION}, 
    {"END_IF", T_END_IF}, 
    {"END_LOCAL", T_END_LOCAL}, 
    {"END_PROCEDURE", T_END_PROCEDURE}, 
    {"END_REPEAT", T_END_REPEAT}, 
    {"END_RULE", T_END_RULE}, 
    {"END_SCHEMA", T_END_SCHEMA}, 
    {"END_SUBTYPE_CONSTRAINT", T_END_SUBTYPE_CONSTRAINT}, 
    {"END_TYPE", T_END_TYPE}, 
    {"ENTITY", T_ENTITY}, 
    {"ENUMERATION", T_ENUMERATION}, 
    {"ESCAPE", T_ESCAPE}, 
    {"EXISTS", T_EXISTS}, 
    {"EXP", T_EXP}, 
    {"EXTENSIBLE", T_EXTENSIBLE}, 
    {"FALSE", T_FALSE}, 
    {"FIXED", T_FIXED}, 
    {"FOR", T_FOR}, 
    {"FORMAT", T_FORMAT}, 
    {"FROM", T_FROM}, 
    {"FUNCTION", T_FUNCTION}, 
    {"GENERIC", T_GENERIC}, 
    {"GENERIC_ENTITY", T_GENERIC_ENTITY}, 
    {"HIBOUND", T_HIBOUND}, 
    {"HIINDEX", T_HIINDEX}, 
    {"IF", T_IF}, 
    {"IN", T_IN}, 
    {"INSERT", T_INSERT}, 
    {"INTEGER", T_INTEGER}, 
    {"INVERSE", T_INVERSE}, 
    {"LENGTH", T_LENGTH}, 
    {"LIKE", T_LIKE}, 
    {"LIST", T_LIST}, 
    {"LOBOUND", T_LOBOUND}, 
    {"LOCAL", T_LOCAL}, 
    {"LOG", T_LOG}, 
    {"LOG10", T_LOG10}, 
    {"LOG2", T_LOG2}, 
    {"LOGICAL", T_LOGICAL}, 
    {"LOINDEX", T_LOINDEX}, 
    {"MOD", T_MOD}, 
    {"NOT", T_NOT}, 
    {"NUMBER", T_NUMBER}, 
    {"NVL", T_NVL}, 
    {"ODD", T_ODD}, 
    {"OF", T_OF}, 
    {"ONEOF", T_ONEOF}, 
    {"OPTIONAL", T_OPTIONAL}, 
    {"OR", T_OR}, 
    {"OTHERWISE", T_OTHERWISE}, 
    {"PI", T_PI}, 
    {"PROCEDURE", T_PROCEDURE}, 
    {"QUERY", T_QUERY}, 
    {"REAL", T_REAL}, 
    {"REFERENCE", T_REFERENCE}, 
    {"REMOVE", T_REMOVE}, 
    {"RENAMED", T_RENAMED}, 
    {"REPEAT", T_REPEAT}, 
    {"RETURN", T_RETURN}, 
    {"ROLESOF", T_ROLESOF}, 
    {"RULE", T_RULE}, 
    {"SCHEMA", T_SCHEMA}, 
    {"SELECT", T_SELECT}, 
    {"SELF", T_SELF}, 
    {"SET", T_SET}, 
    {"SIN", T_SIN}, 
    {"SIZEOF", T_SIZEOF}, 
    {"SKIP", T_SKIP}, 
    {"SQRT", T_SQRT}, 
    {"STRING", T_STRING}, 
    {"SUBTYPE", T_SUBTYPE}, 
    {"SUBTYPE_CONSTRAINT", T_SUBTYPE_CONSTRAINT}, 
    {"SUPERTYPE", T_SUPERTYPE}, 
    {"TAN", T_TAN}, 
    {"THEN", T_THEN}, 
    {"TO", T_TO}, 
    {"TOTAL_OVER", T_TOTAL_OVER}, 
    {"TRUE", T_TRUE}, 
    {"TYPE", T_TYPE}, 
    {"TYPEOF", T_TYPEOF}, 
    {"UNIQUE", T_UNIQUE}, 
    {"UNKNOWN", T_UNKNOWN}, 
    {"UNTIL", T_UNTIL}, 
    {"USE", T_USE}, 
    {"USEDIN", T_USEDIN}, 
    {"VALUE", T_VALUE}, 
    {"VALUE_IN", T_VALUE_IN}, 
    {"VALUE_UNIQUE", T_VALUE_UNIQUE}, 
    {"VAR", T_VAR}, 
    {"WHERE", T_WHERE}, 
    {"WHILE", T_WHILE}, 
    {"WITH", T_WITH}, 
    {"XOR", T_XOR},
    {NULL, 0}
};


struct intList *intListCreate (void) {
    struct intList * il = sc_malloc(sizeof(struct intList));
	if (il) {
		il->entry = sc_malloc(sizeof(int));
		if (!il->entry) {
			sc_free(il);
			il = NULL;
		} else {
            il->entry[0] = 0;
			il->qty = 0;
			il->mlen = 1;
		}
	}
	return il;
}

int intListDestroy (struct intList * il) {
    /* dummy implementation */
    return 0;
}

int intListAllocMin (struct intList * il, int msz) {
    int *l;
    size_t nsz;
	
	if (!il || msz <= 0 || !il->entry || il->qty < 0 || il->mlen <= 0 ||
	    il->qty > il->mlen) return BSTR_ERR;
	if (msz < il->qty) msz = il->qty;
	if (il->mlen == msz) return BSTR_OK;
	nsz = ((size_t) msz) * sizeof(int);
	if (nsz < (size_t) msz) return BSTR_ERR;
	l = sc_realloc(il->entry, nsz);
	if (!l) return BSTR_ERR;
	il->mlen = msz;
	il->entry = l;
	memset(il->entry + il->qty, 0, sizeof(int) * (il->mlen - il->qty));
	return BSTR_OK;
}

struct scopeList *scopeListCreate (void) {
    struct scopeList * sl = sc_malloc(sizeof(struct scopeList));
	if (sl) {
		sl->entry = sc_malloc(sizeof(struct scope_def));
		if (!sl->entry) {
			sc_free(sl);
			sl = NULL;
		} else {
			sl->qty = 0;
			sl->mlen = 1;
		}
	}
	return sl;
}

int scopeListDestroy (struct scopeList * sl) {
    /* dummy implementation */
    return 0;
}

int scopeListAllocMin (struct scopeList * sl, int msz) {
    struct scope_def *l;
    size_t nsz;
	
	if (!sl || msz <= 0 || !sl->entry || sl->qty < 0 || sl->mlen <= 0 ||
	    sl->qty > sl->mlen) return BSTR_ERR;
	if (msz < sl->qty) msz = sl->qty;
	if (sl->mlen == msz) return BSTR_OK;
	nsz = ((size_t) msz) * sizeof(struct scope_def);
	if (nsz < (size_t) msz) return BSTR_ERR;
	l = sc_realloc(sl->entry, nsz);
	if (!l) return BSTR_ERR;
	sl->mlen = msz;
	sl->entry = l;
	return BSTR_OK;
}

void yylexReset(struct exp_scanner *s, int new_mode) {
    s->cur = s->tok = s->mrk = s->ctx = s->buffer->data;
    memset(s->cond_stack->entry, 0, sizeof(int) * s->cond_stack->mlen);
    s->cond_top = 0;
    s->cond_stack->entry[0] = new_mode;
    s->mode = new_mode;
}

int yylexInit(struct exp_scanner *pScanner, struct YYSTATE *pState, FILE *fp) {
    bstring buf;
    
    buf = bread((bNread) fread, fp);
    if (!buf)
        yyerror("failed to read input!", 0);

    pScanner->state = pState;
    pScanner->scope_top = scope_alloc(pState->scope_stack, "DOCROOT", T_DOCROOT);
    pState->scope_stack->entry[pScanner->scope_top].parent = -1;

    pScanner->buffer = buf; 
    pScanner->lim = buf->data + buf->slen;
    pScanner->anon_scope_cnt = 1;

    yylexReset(pScanner, yycP1);    
    return 0;
}

struct exp_scanner *yylexAlloc() {
    int chk;
    struct exp_scanner *scanner;
    
    scanner = sc_malloc(sizeof *scanner);
    if (scanner) {
        memset(scanner, 0, sizeof *scanner);

        scanner->cond_stack = intListCreate();
        chk = intListAllocMin(scanner->cond_stack, 20);
        if (chk != BSTR_OK)
            goto err_out;
        
        scanner->brkt_nesting = intListCreate();
        chk = intListAllocMin(scanner->brkt_nesting, 50);
        if (chk != BSTR_OK)
            goto err_out;
    }
    
    return scanner;
    
err_out:
    return NULL;
}


struct YYSTATE *yystateAlloc() {
    struct YYSTATE *pState = sc_malloc(sizeof(struct YYSTATE));

    if (pState) {
        pState->scope_stack = scopeListCreate();
        pState->lineno = 1;
    }
    
    return pState;
}

int scope_alloc(struct scopeList *stack, const char *typ, int scope_type) {
    struct scope_def *sp;
    const char *nx;
    char *np;
    int chk;

    /* TODO: error checking */
    chk = scopeListAllocMin(stack, stack->qty+1);
    if (chk != BSTR_OK)
        goto err_out;

    sp = stack->entry + stack->qty;
    /* TODO: really this doesn't need a symbol - we only have the "name", which could be assigned later or here */
    sp->symbol.name = NULL;
    sp->type = scope_type;
    sp->parent = 0xDEADBEEF;
    sp->symbol_table = HASHcreate();

#ifdef YYDEBUG
    sp->symbol.stype = strdup(typ);
    for (nx = typ, np = sp->stype; *nx; nx++, np++)
        *np = toupper((unsigned char) *nx);
#endif

    return stack->qty++;
    
err_out:
    yyerror("scope_alloc failure!", 0);
}

void scope_push(struct YYSTATE *pState, int idx, struct exp_scanner *pScanner) {
    struct scopeList *stack = pState->scope_stack;
    struct scope_def *scope = stack->entry + idx;
    scope->parent = pScanner->scope_top;
    if (idx < 0) {
        yyerror("+++scope_push() internal error, pushing nonsense!", pScanner->lineno);
    }
    pScanner->scope_top = idx;
}

int scope_pop(struct YYSTATE *pState, struct exp_scanner *pScanner) {
    struct scopeList *stack = pState->scope_stack;
    int idx = pScanner->scope_top;
    struct scope_def *scope = stack->entry + idx;
    if (scope->parent < 0) {
        yyerror("+++scope_pop() internal error, popping of the stack", 0);
    }
    pScanner->scope_top = scope->parent;
    return idx;
}

int scope_find(struct YYSTATE *pState, int itype, const char *sname, struct exp_scanner *pScanner) {
    struct scopeList *stack = pState->scope_stack;
    struct scope_def *s;
    int i, idx = -1, pidx = pScanner->scope_top;
    
    /* need to match parent, name and type */
    for (i = 0; i < stack->qty; i++) {
        s = stack->entry + i;
        if (s->parent == pidx && s->type == itype && !strcasecmp(s->symbol.name, sname)) {
            idx = i;
            break;
        }
    }
    
    if (idx < 0)
        yyerror("+++ scope_find failed, internal error!\n", 0);
    
    return idx;
}

void add_symbol(struct scope_def *scope, const char *id, int ref_tok, struct exp_scanner *pScanner) {
    Symbol e, *ep;
    int type;
    
    switch (ref_tok) {
        case T_SCHEMA_REF:
            type = OBJ_SCHEMA;
            break;
        case T_FUNCTION_REF:
            type = OBJ_FUNCTION;
            break;            
        case T_PROCEDURE_REF:
            type = OBJ_PROCEDURE;
            break;
        case T_RULE_REF:
            type = OBJ_RULE;
            break;
        case T_ENTITY_REF:
            type = OBJ_ENTITY;
            break;
        case T_TYPE_REF:
            type = OBJ_TYPE;  
            break;
        case T_ENUMERATION_REF:
            type = OBJ_ENUM;
            break;
        case T_RULE_LABEL_REF:
            type = OBJ_TAG;  /* TODO: check, confirm rule labels are tags */
            break;
        case T_VARIABLE_REF:
        case T_PARAMETER_REF: /* TODO: check - parameter maps to variable? */
            type = OBJ_VARIABLE;
            break;
        default:  /* T_CONSTANT_REF, T_INVALID, T_SIMPLE_REF */
            type = OBJ_UNKNOWN; /* TODO: check for CONSTANT */
    }
    e = (Symbol) {.name = strdup(id), .type = type, .ref_tok = ref_tok,
                  .filename = pScanner->filename, .line = pScanner->lineno};
    ep = HASHsearch(scope->symbol_table, e, HASH_INSERT);
    if (!ep)
        yyerror("failed to insert symbol into symbol table", pScanner->lineno);
}

int resolve_symbol(struct YYSTATE *pState, const char *id, struct exp_scanner *pScanner) {
    Symbol *qry, *sym;
    bool is_enum;
    int cnt_enum = 0, cnt = 0, chk;
    struct scopeList *stack = pState->scope_stack;
    struct scope_def *s = stack->entry + pScanner->scope_top;
    
    /* basic resolution */
    qry = NULL;
    while (s->type != T_DOCROOT) {
        qry = HASHsearch(s->symbol_table, (Symbol) {.name = id}, HASH_FIND);
        if (qry)
            break;
        s = stack->entry + s->parent;
    }
    
    /* failure, not found */
    if (!qry)
        return T_INVALID;
        
    sym = qry;
    while (qry) {
        if (!strcasecmp(qry->name, id)) {
            cnt++;
            if (qry->type == T_ENUMERATION_REF)
                cnt_enum++;
            }
        qry = qry->next;
    }
    
    chk = cnt - cnt_enum;
    if (chk > 1 || (!chk && cnt_enum > 1)) {
        yyerror("conflicting token types for id '%s'", pScanner->lineno, id);
    }
    
    is_enum = chk > 0 ? false : true;
    qry = sym;
    while (qry) {
        if (!strcasecmp(sym->name, id)) {
            if ((!is_enum && qry->type != T_ENUMERATION_REF) ||
                (is_enum && qry->type == T_ENUMERATION_REF))
                break;
        }
        qry = qry->next;
    }
    
    return qry->type;
}


