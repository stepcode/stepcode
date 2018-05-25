#ifndef __LEXSUPPORT_H
#define __LEXSUPPORT_H

#include <stdlib.h>
#include <stdbool.h>

#include "bstrlib.h"

#include "express/expbasic.h"
#include "express/linklist.h"

#include "token_type.h"

#define yyerror_helper(fmt, lineno, ...) \
    do { fprintf(stderr, "error, line %i: " fmt "\n", (lineno), __VA_ARGS__); exit(1); } while (0)
#define yyerror(...) yyerror_helper(__VA_ARGS__, 0)

struct intList {
    int qty, mlen;
    int *entry;
};

struct exp_scanner {
    /* lexing pass / debug flag */
    int mode;
    bool debug;
    /* start condition stack */
    struct intList *cond_stack;
    int cond_top;
    /* pointers into the buffer and location */
    unsigned char *cur, *lim, *tok, *ctx, *mrk;
    int lineno;
    /* buffer and input reference */
    bstring buffer;
    const char *filename;    
    /* flags for rules based scope */
    bool in_explicit_attr, in_rules_clause, in_inverse_clause;
    /* lexical scopes */
    int scope_aux, scope_top;
    /* token ref type */
    int id_ref_typ;
    /* scope type bits */
    int scope_typ;
    /* list of counters for brackets within each scope */
    struct intList *brkt_nesting;
    /* counter for anonymous scope names */
    unsigned int anon_scope_cnt;
    /* shared state - lexical scopes */
    struct YYSTATE *state;
};

struct scope_def {
    int parent;
    
    Symbol symbol;
    int type;
    
    Hash_Table symbol_table;
    ClientData clientData;
    
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

/* TODO: migrate definitions here? */

struct YYSTYPE;

struct YYSTATE {
    int lineno; /* TODO: remove when parser rebuilds automatically */
    struct scopeList *scope_stack;
};

struct kw_token_def {
    const char *kw;
    int token;
};

extern struct kw_token_def reserved_keywords[];

/* lemon doesn't provide a header */
void *yyparseAlloc(void * (*mallocptr)(size_t));
void yyparseFree(void *, void (*freeptr)(void *));
void yyparseInit(void *);
void yyparse(void *, int, struct YYSTYPE *, struct YYSTATE *);
void yyparseTrace(FILE *, char *);

struct exp_scanner *yylexAlloc();
void yylexFree(struct exp_scanner *);
int yylexInit(struct exp_scanner *, struct YYSTATE *, FILE *);

int yylex(struct exp_scanner *, struct YYSTYPE *);
void yylexTrace(struct exp_scanner *, bool);
void yylexReset(struct exp_scanner *s, int new_mode);

struct YYSTATE *yystateAlloc();
void yystateFree();

struct intList * intListCreate (void);
int intListDestroy (struct intList * il);
int intListAllocMin (struct intList * il, int msz);

struct scopeList * scopeListCreate (void);
int scopeListDestroy (struct scopeList * sl);
int scopeListAllocMin (struct scopeList * sl, int msz);

int scope_alloc(struct scopeList *stack, const char *typ, int tok);
void scope_push(struct YYSTATE *pState, int idx, struct exp_scanner *scanner);
int scope_pop(struct YYSTATE *state, struct exp_scanner *scanner);
int scope_find(struct YYSTATE *state, int itype, const char *sname, struct exp_scanner *scanner);
int resolve_symbol(struct YYSTATE *state, const char *sym, struct exp_scanner *scanner);
void add_symbol(struct scope_def *scope, const char *id, int tok, struct exp_scanner *pScanner);



#endif /* __LEXSUPPORT_H */
