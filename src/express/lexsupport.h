#ifndef __LEXSUPPORT_H
#define __LEXSUPPORT_H

#include <stdbool.h>
#include "bstrlib.h"

#include "dict.h"

#define yyerror_helper(fmt, lineno, ...) \
    do { fprintf(stderr, "error, line %i: " fmt "\n", (lineno), __VA_ARGS__); exit(1); } while (0)
#define yyerror(...) yyerror_helper(__VA_ARGS__, 0)

struct intList {
    int qty, mlen;
    int *entry;
};

struct exp_scanner {
    /* lexing pass, top level start condition */
    int mode;
    /* start condition stack */
    struct intList *cond_stack;
    int cond_top;
    /* pointers into the buffer */
    unsigned char *cur, *lim, *tok, *ctx, *mrk;
    /* bstrlib buffer */
    bstring buffer;
    /* flags for rules based scope */
    bool in_explicit_attr, in_rules_clause, in_inverse_clause;
    /* lexical scopes */
    int scope_aux, scope_top;
    /* token type and scope debugging */
    int id_ref_typ, scope_ref_typ;
    /* list of counters for brackets within each scope */
    struct intList *brkt_nesting;
    /* counter for anonymous scope names */
    unsigned int anon_scope_cnt;
};

struct scope_def {
    int parent;
    
    /* TODO: absort into symbol */
    char *sname;
    char *stype;
    int itype;
    
    /* TODO: absorb into symbol_table */
    struct bstrList *symbols;
    struct intList *tokens;
    
    /* migration to existing API */
    Symbol symbol;
    Dictionary symbol_table;
    ClientData clientData;
    
    /* DISCUSS: C11 adds anonymous unions - IMO ideal in a structure like this */
    union {
        struct Procedure_ * proc;
        struct Function_ * func;
        struct Rule_ * rule;
        struct Entity_ * entity;
        struct Schema_ * schema;
        struct Express_ * express;
        struct Increment_ * incr;
        struct TypeHead_ * type;
        /* TBD: struct Query_ * query; */
    } u;
};

struct scopeList {
    int qty, mlen;
    struct scope_def *entry;
};

struct YYSTYPE {
    int scope_idx;
    union {
        char *cstr;
    };
};

struct YYSTATE {
    int lineno;
    struct scopeList *scope_stack;
};

struct kw_token_def {
    const char *kw;
    int token;
};

extern struct kw_token_def reserved_keywords[];

void *yylexAlloc();
int yylex(void *pScanner, struct YYSTYPE *plval, struct YYSTATE *pState);
int yylexInit(void *pScanner, struct YYSTATE *pState, FILE *fp);

struct YYSTATE *yystateAlloc();

struct intList * intListCreate (void);
int intListDestroy (struct intList * il);
int intListAllocMin (struct intList * il, int msz);

struct scopeList * scopeListCreate (void);
int scopeListDestroy (struct scopeList * sl);
int scopeListAllocMin (struct scopeList * sl, int msz);

#endif /* __LEXSUPPORT_H */
