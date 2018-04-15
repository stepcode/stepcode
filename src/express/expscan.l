#include <stdio.h>
#include <stdlib.h>

#include "expparser.h"
#include "lexsupport.h"

/*!types:re2c*/

void yy_push_state(int new_state, struct exp_scanner *scanner);
int yy_exch_state(int new_state, struct exp_scanner *scanner);
void yy_pop_state(struct exp_scanner *scanner);
int yy_top_state(struct exp_scanner *scanner);

void yyreset(struct exp_scanner *s, int new_mode);

int scope_alloc(struct scopeList *stack, const char *typ, int tok);
void scope_push(struct YYSTATE *pState, int idx, struct exp_scanner *scanner);
int scope_pop(struct YYSTATE *state, struct exp_scanner *scanner);
int scope_find(struct YYSTATE *state, int itype, const char *sname, struct exp_scanner *scanner);
int resolve_symbol(struct YYSTATE *state, const char *sym, struct exp_scanner *scanner);

void add_symbol(struct scope_def *scope, const char *sym, int tok);

void brkt_push(struct intList *nesting);
int brkt_pop(struct intList *nesting);

void lexdebug(int state, unsigned char symbol);

#define IBUFSZ 128

int yylex(void *pScanner, struct YYSTYPE *plval, struct YYSTATE *pState)
{
    int scanner_modes[] = {yycP1, yycP2, yycP3};
    size_t midx;
    unsigned char ibuffer[IBUFSZ];
    unsigned char *id;
    /*!stags:re2c format = 'unsigned char *@@;'; */
    int *brkt_nesting;
    struct scope_def *sp, *scope;
    int idx, tok, ibufchk = 0;
    struct exp_scanner *yyscanner = pScanner;
    
    plval->scope_idx = -1;
    
    /*!rules:re2c

    re2c:yyfill:enable = 0;
    re2c:cond:divider = '/****************** @@ *****************/';         

    digit = [0-9] ;
    xdigit = digit | [A-Fa-f] ;
    letter = [A-Za-z] ;
    not_paren_star_special = [!"#$%&+,-./:;<=>?@[\\\]^_`{|}~'] ;
    
    WS = [ \t\r\n] ;
    BREAK = WS | ";" ;

    NOT_PAREN_STAR = letter | digit | not_paren_star_special ;
    SPECIAL = not_paren_star_special | [()*] ;
    
    SIMPLE_ID = letter ( letter | digit | "_" )* ;        
        
    <P1> "(*" {
        memset(yyscanner->tok, ' ', yyscanner->cur - yyscanner->tok);
        yy_push_state(yycCOMMENT, yyscanner);
        continue;
    }
        
    <P1> "--" ( NOT_PAREN_STAR | [ \r\t)(*] )* / "\n" {
        memset(yyscanner->tok, ' ', yyscanner->cur - yyscanner->tok);
        continue;
    }

    <P1> "--" { yyerror("invalid characters in tail remark! '%.*s'", pState->lineno, 20, yyscanner->cur); }

    <P1> "'" ("''" | NOT_PAREN_STAR \ ['] | [)(*] | WS )* "'" { continue; }

    <P1> "'" { yyerror("invalid string literal! '%.*s'", pState->lineno, 20, yyscanner->cur); }

    <P1> ( NOT_PAREN_STAR \ ['-] )+ { continue; }

    <P1> [)(*'-] { continue; }

    <COMMENT> "(*" {
        memset(yyscanner->tok, ' ', yyscanner->cur - yyscanner->tok);
        yy_push_state(yycCOMMENT, yyscanner);
        continue;
    }
    <COMMENT> "*"+ ")" {
        memset(yyscanner->tok, ' ', yyscanner->cur - yyscanner->tok);
        yy_pop_state(yyscanner);
        continue;
    }
    <COMMENT> "*"+ {
        memset(yyscanner->tok, ' ', yyscanner->cur - yyscanner->tok);
        continue;
    }
    <COMMENT> NOT_PAREN_STAR+ {
        memset(yyscanner->tok, ' ', yyscanner->cur - yyscanner->tok);
        continue;
    }
    <COMMENT> [)(] {
        memset(yyscanner->tok, ' ', yyscanner->cur - yyscanner->tok);
        continue;
    }

    
    <P2> 'SCHEMA' WS+ @id SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - id, id);
        add_symbol(pState->scope_stack->entry + yyscanner->scope_top, ibuffer, T_SCHEMA_REF);
    
        idx = scope_alloc(pState->scope_stack, "SCHEMA", T_SCHEMA);
        scope_push(pState, idx, yyscanner);
        sp = pState->scope_stack->entry + idx;
        sp->sname = strndup(id, yyscanner->cur - id);
        
        continue;
    }
    <P2> 'FUNCTION' WS+ @id SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - id, id);
        add_symbol(pState->scope_stack->entry + yyscanner->scope_top, ibuffer, T_FUNCTION_REF);

        idx = scope_alloc(pState->scope_stack, "FUNCTION", T_FUNCTION);
        scope_push(pState, idx, yyscanner);
        sp = pState->scope_stack->entry + idx;
        sp->sname = strndup(id, yyscanner->cur - id);

        continue;
    }
    <P2> 'PROCEDURE' WS+ @id SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - id, id);
        add_symbol(pState->scope_stack->entry + yyscanner->scope_top, ibuffer, T_PROCEDURE_REF);
        
        idx = scope_alloc(pState->scope_stack, "PROCEDURE", T_PROCEDURE);
        scope_push(pState, idx, yyscanner);
        sp = pState->scope_stack->entry + idx;
        sp->sname = strndup(id, yyscanner->cur - id);
        
        continue;
    }
    <P2> 'RULE' WS+ @id SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - id, id);
        add_symbol(pState->scope_stack->entry + yyscanner->scope_top, ibuffer, T_RULE_REF);
        
        idx = scope_alloc(pState->scope_stack, "RULE", T_RULE);
        scope_push(pState, idx, yyscanner);
        sp = pState->scope_stack->entry + idx;
        sp->sname = strndup(id, yyscanner->cur - id);
        
        continue;
    }
    <P2> 'ENTITY' WS+ @id SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - id, id);
        add_symbol(pState->scope_stack->entry + yyscanner->scope_top, ibuffer, T_ENTITY_REF);
        
        idx = scope_alloc(pState->scope_stack, "ENTITY", T_ENTITY);
        scope_push(pState, idx, yyscanner);
        sp = pState->scope_stack->entry + idx;
        sp->sname = strndup(id, yyscanner->cur - id);
        
        continue;
    }
    <P2> 'TYPE' WS+ @id SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - id, id);
        add_symbol(pState->scope_stack->entry + yyscanner->scope_top, ibuffer, T_TYPE_REF);
        
        idx = scope_alloc(pState->scope_stack, "TYPE", T_TYPE);
        scope_push(pState, idx, yyscanner);
        sp = pState->scope_stack->entry + idx;
        sp->sname = strndup(id, yyscanner->cur - id);
        
        continue;
    }

    <P2> 'END_' ('SCHEMA'|'FUNCTION'|'PROCEDURE'|'RULE'|'ENTITY'|'TYPE') / BREAK {
        scope_pop(pState, yyscanner);
        continue;
    }

    <P2> "'" ("''" | NOT_PAREN_STAR \ ['] | [)(*] | WS )* "'" { continue; }
    <P2> SIMPLE_ID { continue; }
    <P2> * { continue; }


    <TYPE> "=" { return T_EQ; }
    <TYPE> 'ENUMERATION' / WS {
        yy_push_state(yycENUM, yyscanner);
        return T_ENUMERATION;
    }
    <TYPE> 'EXTENSIBLE' / WS { return T_EXTENSIBLE; }
    <TYPE> 'WHERE' / WS {
        yyscanner->id_ref_typ = T_RULE_LABEL_REF;
        yy_push_state(yycLABEL_OPT, yyscanner);
        return T_WHERE;
    }
    <TYPE> ";" / WS* 'WHERE' { return T_SEMICOLON; }
    <TYPE> ";" {
        yyscanner->id_ref_typ = T_RULE_LABEL_REF;
        yy_push_state(yycLABEL_OPT, yyscanner);
        return T_SEMICOLON;
    }
    <TYPE> ";" / WS* 'END_TYPE' { return T_SEMICOLON; }
    <TYPE> 'END_TYPE' / BREAK {
        yy_pop_state(yyscanner);
        scope_pop(pState, yyscanner);
        return T_END_TYPE;
    }
    <TYPE> "" { yy_push_state(yycSTMT, yyscanner); continue; }

    <ENUM> "(" { yy_push_state(yycENUM_ID_LIST, yyscanner); return T_LPAREN; }
    <ENUM> ")" { yy_pop_state(yyscanner); return T_RPAREN; }
    <ENUM> "" :=> P3

    <ENTITY> 'DERIVE' / WS {
        yyscanner->in_explicit_attr = false;
        yyscanner->in_inverse_clause = false;
        yyscanner->in_rules_clause = false;
        yyscanner->id_ref_typ = T_SIMPLE_REF;
        yy_push_state(yycID_LIST, yyscanner);
        return T_DERIVE;
    }
    <ENTITY> 'INVERSE' / WS {
        yyscanner->in_explicit_attr = false;
        yyscanner->in_inverse_clause = true;
        yyscanner->in_rules_clause = false;
        yyscanner->id_ref_typ = T_SIMPLE_REF;
        yy_push_state(yycID_LIST, yyscanner);
        return T_INVERSE;
    }
    <ENTITY> 'UNIQUE' / WS {
        yyscanner->in_explicit_attr = false;
        yyscanner->in_inverse_clause = false;
        yyscanner->in_rules_clause = true;
        yyscanner->id_ref_typ = T_RULE_LABEL_REF;
        yy_push_state(yycLABEL_OPT, yyscanner);
        return T_UNIQUE;
    }
    <ENTITY> 'WHERE' / WS {
        yyscanner->in_explicit_attr = false;
        yyscanner->in_inverse_clause = false;
        yyscanner->in_rules_clause = true;
        yyscanner->id_ref_typ = T_RULE_LABEL_REF;
        yy_push_state(yycLABEL_OPT, yyscanner);
        return T_WHERE;
    }
    <ENTITY> ":" / [^=<] {
        if (yyscanner->in_inverse_clause || yyscanner->in_explicit_attr) {
            yyscanner->scope_aux = scope_pop(pState, yyscanner);
        }
        if (!yyscanner->in_inverse_clause) {
            yy_push_state(yycSTMT, yyscanner);
        } else {
            yy_push_state(yycINV_FOR_STMT, yyscanner);
        }
        return T_COLON;
    }
    <ENTITY> ";" / WS* ('DERIVE'|'INVERSE'|'UNIQUE'|'WHERE'|'END_ENTITY') BREAK {
        if (yyscanner->scope_aux) {
            scope_push(pState, yyscanner->scope_aux, yyscanner);
            yyscanner->scope_aux = 0;
        }
        return T_SEMICOLON;
    }
    <ENTITY> ";" {
        if (yyscanner->scope_aux) {
            scope_push(pState, yyscanner->scope_aux, yyscanner);
            yyscanner->scope_aux = 0;
        }
        if (!yyscanner->in_rules_clause) {
            yy_push_state(yycID_LIST, yyscanner);        
        } else {
            yy_push_state(yycLABEL_OPT, yyscanner);
        }
        return T_SEMICOLON;
    }
    <ENTITY> 'END_ENTITY' / BREAK {
        scope_pop(pState, yyscanner);
        yy_pop_state(yyscanner);
        return T_END_ENTITY;
    }
    <ENTITY> "" :=> P3

    <ARG_LIST> [(;] {
        /* TODO: bracket_count */
        yyscanner->id_ref_typ = T_PARAMETER_REF;
        yy_push_state(yycID_LIST, yyscanner);
        return ( *yyscanner->tok == '(' ) ? T_LPAREN : T_SEMICOLON;
    }
    <ARG_LIST> ")" {
        /* TODO: bracket_count */
        yy_pop_state(yyscanner);
        return T_RPAREN;
    }
    <ARG_LIST> 'VAR' / WS {
        yyscanner->id_ref_typ = T_PARAMETER_REF;
        yy_push_state(yycID_LIST, yyscanner);
        return T_VAR;
    }
    <ARG_LIST> "" :=> P3

    <RULE> 'WHERE'/ WS {
        yyscanner->in_rules_clause = true;
        yyscanner->id_ref_typ = T_RULE_LABEL_REF;
        yy_push_state(yycLABEL_OPT, yyscanner);
        return T_WHERE;
    }
    <RULE> ";" {
        if (!yyscanner->in_rules_clause) {
            /* TODO: is this branch ever followed? */
            yyscanner->id_ref_typ = T_SIMPLE_REF;
            yy_push_state(yycSTMT, yyscanner);
        } else {
            yyscanner->id_ref_typ = T_RULE_LABEL_REF;
            yy_push_state(yycLABEL_OPT, yyscanner);
        }
        return T_SEMICOLON;
    }
    <RULE> ";" / WS* 'WHERE' WS { return T_SEMICOLON; }
    <RULE> ";" / WS* 'END_RULE' { return T_SEMICOLON; }
    <RULE> 'END_RULE' / BREAK {
        yyscanner->in_rules_clause = false;
        scope_pop(pState, yyscanner);
        return T_END_RULE;
    }
    <RULE> "" { yy_push_state(yycSTMT, yyscanner); continue; }

    <QUERY> ")" {
        brkt_nesting = yyscanner->brkt_nesting->entry + yyscanner->brkt_nesting->qty;
        if (!--*brkt_nesting) {
            brkt_pop(yyscanner->brkt_nesting);
            scope_pop(pState, yyscanner);
            yy_pop_state(yyscanner);
        }
        return T_RPAREN;
    }
    <QUERY> "(" {
        brkt_nesting = yyscanner->brkt_nesting->entry + yyscanner->brkt_nesting->qty;
        ++*brkt_nesting;
        return T_LPAREN;
    }
    <QUERY> "" :=> P3

    <REDECL> 'RENAMED' / WS {
        yyscanner->id_ref_typ = T_SIMPLE_REF;
        yy_push_state(yycID_CUR_SCOPE, yyscanner);
        return T_RENAMED;
    }
    <REDECL> [,:] { --yyscanner->cur; yy_pop_state(yyscanner); continue; }
    <REDECL> "" :=> P3

    <STMT> ";" { --yyscanner->cur; yy_pop_state(yyscanner); continue; }
    <STMT> "" :=> P3

    <INV_FOR_STMT> 'FOR' / WS { yy_push_state(yycINV_FOR, yyscanner); return T_FOR; }
    <INV_FOR_STMT> ";" { --yyscanner->cur; yy_pop_state(yyscanner); continue; }
    <INV_FOR_STMT> "" :=> P3

    <P3> "%" [01]+ { return T_BINARY_LITERAL; }

    <P3> digit+ { return T_INTEGER_LITERAL; }

    <P3> digit+ "." digit* ([eE] [+-]? digit+)? { return T_REAL_LITERAL; }

    <P3> "'" ("''" | NOT_PAREN_STAR \ ['] | [)(*] | WS )* "'" { return T_STRING_LITERAL; }

    <P3> "\"" ( xdigit{4} )+ "\"" { return T_ENCODED_STRING_LITERAL; }
    <P3> "\"" { yyerror("invalid encoded string literal! '%.*s'", pState->lineno, 20, yyscanner->cur); }

    <P3> 'SCHEMA'/ WS {
        yyscanner->scope_ref_typ = T_SCHEMA;
        yy_push_state(yycSEARCH_SCOPE, yyscanner);
        return T_SCHEMA;
    }
    <P3> 'END_SCHEMA' / BREAK {
        scope_pop(pState, yyscanner);
        return T_END_SCHEMA;
    }

    <P3> 'TYPE' / WS {
        yyscanner->scope_ref_typ = T_TYPE;
        yy_push_state(yycTYPE, yyscanner);
        yy_push_state(yycSEARCH_SCOPE, yyscanner);
        return T_TYPE;
    }

    <P3> 'ENTITY' / WS {
        yyscanner->scope_ref_typ = T_ENTITY;
        yyscanner->id_ref_typ = T_SIMPLE_REF;
        yyscanner->in_inverse_clause = false;
        yyscanner->in_explicit_attr = true;
        yyscanner->in_rules_clause = false;
        yyscanner->scope_aux = 0;
        
        yy_push_state(yycENTITY, yyscanner);
        yy_push_state(yycSTMT, yyscanner);
        yy_push_state(yycSEARCH_SCOPE, yyscanner);
        
        return T_ENTITY;
    }

    <P3> 'FUNCTION' / WS* SIMPLE_ID WS* "(" {
        yyscanner->scope_ref_typ = T_FUNCTION;
        yy_push_state(yycARG_LIST, yyscanner);
        yy_push_state(yycSEARCH_SCOPE, yyscanner);
        return T_FUNCTION;
    }
    <P3> 'FUNCTION' / WS {
        yyscanner->scope_ref_typ = T_FUNCTION;
        yy_push_state(yycSEARCH_SCOPE, yyscanner);
        return T_FUNCTION;
    }
    <P3> 'END_FUNCTION' / BREAK {
        scope_pop(pState, yyscanner);
        return T_END_FUNCTION;
    }

    <P3> 'PROCEDURE' / WS* SIMPLE_ID WS* "(" {
        yyscanner->scope_ref_typ = T_PROCEDURE;
        yy_push_state(yycARG_LIST, yyscanner);
        yy_push_state(yycSEARCH_SCOPE, yyscanner);
        return T_PROCEDURE;
    }
    <P3> 'PROCEDURE' / WS {
        yyscanner->scope_ref_typ = T_PROCEDURE;
        yy_push_state(yycSEARCH_SCOPE, yyscanner);
        return T_PROCEDURE;
    }
    <P3> 'END_PROCEDURE' / BREAK {
        scope_pop(pState, yyscanner);
        return T_END_PROCEDURE;
    }

    <P3> 'RULE' / WS {
        yyscanner->scope_ref_typ = T_RULE;
        yy_push_state(yycRULE, yyscanner);
        yy_push_state(yycSTMT, yyscanner);
        yy_push_state(yycSEARCH_SCOPE, yyscanner);
        return T_RULE;
    }

    <P3> 'ALIAS' / WS {
        idx = scope_alloc(pState->scope_stack, "ALIAS", T_ALIAS);
        scope = pState->scope_stack->entry + idx;

        ibufchk = snprintf(ibuffer, sizeof(ibuffer), "QANON_%08lX", yyscanner->anon_scope_cnt);
        scope->sname = strdup(ibuffer);
        scope_push(pState, idx, yyscanner);
        yyscanner->anon_scope_cnt++;
        
        yyscanner->id_ref_typ = T_VARIABLE_REF;
        yy_push_state(yycID_CUR_SCOPE, yyscanner);
        return T_ALIAS;
    }
    <P3> 'END_ALIAS' / BREAK {
        scope_pop(pState, yyscanner);
        return T_END_ALIAS;
    }

    <P3> 'SUBTYPE_CONSTRAINT' / WS {
        /* _ref not supported, as only used in remarks */
        yyscanner->id_ref_typ = T_INVALID;
        
        yy_push_state(yycSUBTYPE_CONS, yyscanner);
        yy_push_state(yycID_CUR_SCOPE, yyscanner);
        return T_SUBTYPE_CONSTRAINT;
    }

    <P3> 'END_SUBTYPE_CONSTRAINT' / BREAK {
        scope_pop(pState, yyscanner);
        return T_END_SUBTYPE_CONSTRAINT;
    }

    <P3> 'REPEAT' / WS+ SIMPLE_ID WS* ":=" {
        idx = scope_alloc(pState->scope_stack, "REPEAT", T_REPEAT);
        scope = pState->scope_stack->entry + idx;

        ibufchk = snprintf(ibuffer, sizeof(ibuffer), "REP_ANON_%08lX", yyscanner->anon_scope_cnt);
        scope->sname = strdup(ibuffer);
        scope_push(pState, idx, yyscanner);
        yyscanner->anon_scope_cnt++;
        
        yyscanner->id_ref_typ = T_VARIABLE_REF;
        yy_push_state(yycID_CUR_SCOPE, yyscanner);
        return T_REPEAT;
    }
    <P3> 'REPEAT' / WS {
        idx = scope_alloc(pState->scope_stack, "REPEAT", T_REPEAT);
        scope = pState->scope_stack->entry + idx;

        ibufchk = snprintf(ibuffer, sizeof(ibuffer), "REP_ANON_%08lX", yyscanner->anon_scope_cnt);
        scope->sname = strdup(ibuffer);
        scope_push(pState, idx, yyscanner);
        yyscanner->anon_scope_cnt++;
        return T_REPEAT;
    }
    <P3> 'END_REPEAT' / BREAK {
        scope_pop(pState, yyscanner);
        return T_END_REPEAT;
    }

    <P3> 'QUERY' / WS* "(" {
        /* TODO: clean up this mess! */
        idx = scope_alloc(pState->scope_stack, "QUERY", T_QUERY);
        scope = pState->scope_stack->entry + idx;

        ibufchk = snprintf(ibuffer, sizeof(ibuffer), "QRY_ANON_%08lX", yyscanner->anon_scope_cnt);
        scope->sname = strdup(ibuffer);
        scope_push(pState, idx, yyscanner);

        brkt_push(yyscanner->brkt_nesting);
        brkt_nesting = yyscanner->brkt_nesting->entry + yyscanner->brkt_nesting->qty;
        ++*brkt_nesting;
        
        yyscanner->anon_scope_cnt++;
        
        yyscanner->id_ref_typ = T_VARIABLE_REF;
        yy_push_state(yycQUERY, yyscanner);
        yy_push_state(yycID_CUR_SCOPE, yyscanner);
        return T_QUERY;
    }

    <P3> 'CONSTANT' / WS {
        yy_push_state(yycCONSTANT, yyscanner);
        return T_CONSTANT;
    }

    <P3> 'LOCAL' / WS {
        yyscanner->id_ref_typ = T_VARIABLE_REF;
        yy_push_state(yycLOCAL, yyscanner);
        yy_push_state(yycID_LIST, yyscanner);
        return T_LOCAL;
    }

        /* recognise type_ref '.' */
    <P3> SIMPLE_ID / WS* "." WS* SIMPLE_ID WS* [^.] {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - yyscanner->tok, yyscanner->tok);
        tok = resolve_symbol(pState, ibuffer, yyscanner);
        if (tok == T_TYPE_REF)
            yy_push_state(yycENUM_REF, yyscanner);
        return tok;
    }

        /* recognise group_qualifier */
    <P3> "\\" / WS* SIMPLE_ID {
        yy_push_state(yycENTITY_REF, yyscanner);
        return T_BACKSLASH;
    }

        /* return only T_SIMPLE_ID for all occurences of type_label */
    <P3> 'GENERIC_ENTITY' / WS* ":" [^=<] {
        yy_push_state(yycID_CUR_SCOPE, yyscanner);
        return T_GENERIC_ENTITY;
    }

    <P3> 'AGGREGATE' / WS* ":" [^=<] {
        yy_push_state(yycID_CUR_SCOPE, yyscanner);
        return T_AGGREGATE;
    }

    <P3> 'GENERIC' / WS* ":" [^=<] {
        yy_push_state(yycID_CUR_SCOPE, yyscanner);
        return T_GENERIC;
    }

    <P3> "." / WS* SIMPLE_ID {
        yy_push_state(yycDOT_ATTR, yyscanner);
        return T_DOT;
    }

    <P3> SIMPLE_ID {
        int i;
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - yyscanner->tok, yyscanner->tok);
        for (i=0; reserved_keywords[i].kw != NULL; i++) {
            if (!strcasecmp(ibuffer, reserved_keywords[i].kw)) {
                return reserved_keywords[i].token;
            }
        }
        
        tok = resolve_symbol(pState, ibuffer, yyscanner);
        plval->cstr = strdup(ibuffer);
        return tok;
    }

        /* special characters */
    <P3> "+" { return T_PLUS; }
    <P3> "-" { return T_MINUS; }
    <P3> "*" { return T_TIMES; }
    <P3> "/" { return T_RDIV; }
    <P3> "**" { return T_EXP; }

    <P3> "=" { return T_EQ; }
    <P3> "<>" { return T_NEQ; }
    <P3> ">" { return T_GT; }
    <P3> "<" { return T_LT; }
    <P3> ">=" { return T_GTEQ; }
    <P3> "<=" { return T_LTEQ; }

    <P3> "(" { return T_LPAREN; }
    <P3> ")" { return T_RPAREN; }
    <P3> "{" { return T_LCURL; }
    <P3> "}" { return T_RCURL; }
    <P3> "[" { return T_LBRKT; }
    <P3> "]" { return T_RBRKT; }

    <P3> "," { return T_COMMA; }
    <P3> ":" { return T_COLON; }
    <P3> ";" { return T_SEMICOLON; }

    <P3> "\\" { return T_BACKSLASH; }
    <P3> "." { return T_DOT; }
    <P3> "<*" { return T_ALL_IN; }
    <P3> "|" { return T_PIPE; }
    <P3> ":=" { return T_ASSIGN; }
    <P3> "||" { return T_CONCAT; }
    <P3> ":=:" { return T_INST_EQ; }
    <P3> ":<>:" { return T_INST_NEQ; }

    <P3> "?" { return T_QMARK; }

    <P3> [!"#$%&@^_`'] { return T_INVALID; }
    
    <*> ( WS \ [\n] )+ { continue; }
    <*> [\n] { pState->lineno++; continue; }

    <SEARCH_SCOPE> SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - yyscanner->tok, yyscanner->tok);
        idx = scope_find(pState, yyscanner->scope_ref_typ, ibuffer, yyscanner);
        scope_push(pState, idx, yyscanner);
        yy_pop_state(yyscanner);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_ID;
    }

    <ID_CUR_SCOPE> SIMPLE_ID {
        struct scope_def *sp = pState->scope_stack->entry + yyscanner->scope_top;
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - yyscanner->tok, yyscanner->tok);
        add_symbol(sp, ibuffer, yyscanner->id_ref_typ);
        yy_pop_state(yyscanner);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_ID;
    }
    <ID_CUR_SCOPE> [(:] { return ( *yyscanner->tok == '(' ) ? T_LPAREN : T_COLON; }

    <ID_LIST> SIMPLE_ID / WS* [,:)] {
        struct scope_def *sp = pState->scope_stack->entry + yyscanner->scope_top;
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - yyscanner->tok, yyscanner->tok);
        add_symbol(sp, ibuffer, yyscanner->id_ref_typ);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_ID;
    }
    <ID_LIST> 'SELF' / WS* "\\" { yy_push_state(yycREDECL, yyscanner); return T_SELF; }
    <ID_LIST> "," { return T_COMMA; }
    <ID_LIST> "" { yy_pop_state(yyscanner); continue; }
    
    <ENUM_ID_LIST> SIMPLE_ID / WS* [,)] {
        struct scope_def *sp = pState->scope_stack->entry + yyscanner->scope_top;
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - yyscanner->tok, yyscanner->tok);
        add_symbol(sp, ibuffer, T_ENUMERATION_REF);
        add_symbol(pState->scope_stack->entry + sp->parent, ibuffer, T_ENUMERATION_REF);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_ID;
    }
    <ENUM_ID_LIST> "," { return T_COMMA; }
    <ENUM_ID_LIST> ")" { --yyscanner->cur; yy_pop_state(yyscanner); continue; }

    <LABEL_OPT> SIMPLE_ID / WS* ":" {
        struct scope_def *sp = pState->scope_stack->entry + yyscanner->scope_top;
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - yyscanner->tok, yyscanner->tok);
        add_symbol(sp, ibuffer, yyscanner->id_ref_typ);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_ID;
    }
    <LABEL_OPT> "" { yy_exch_state(yycSTMT, yyscanner); continue; }

    <ENUM_REF> "." { return T_DOT; }        
    <ENUM_REF> SIMPLE_ID {
        yy_pop_state(yyscanner);
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - yyscanner->tok, yyscanner->tok);
        plval->cstr = strdup(ibuffer);
        return T_ENUMERATION_REF;
    }

    <ENTITY_REF> SIMPLE_ID {
        yy_pop_state(yyscanner);
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - yyscanner->tok, yyscanner->tok);
        plval->cstr = strdup(ibuffer);
        return T_ENTITY_REF;
    }

    <DOT_ATTR> SIMPLE_ID {
        yy_pop_state(yyscanner);
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - yyscanner->tok, yyscanner->tok);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_REF;
    }

    <CONSTANT> ":" { yy_push_state(yycSTMT, yyscanner); return T_COLON; }
    <CONSTANT> ";" { return T_SEMICOLON; }
    <CONSTANT> ";" / WS* 'END_CONSTANT' { return T_SEMICOLON; }
    <CONSTANT> 'END_CONSTANT' / BREAK {
        yy_pop_state(yyscanner);
        return T_END_CONSTANT;
    }
    <CONSTANT> SIMPLE_ID / WS* ":" {
        struct scope_def *sp = pState->scope_stack->entry + yyscanner->scope_top;
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - yyscanner->tok, yyscanner->tok);
        add_symbol(sp, ibuffer, T_CONSTANT_REF);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_ID;
    }

    <LOCAL> ":" { yy_push_state(yycSTMT, yyscanner); return T_COLON; }
    <LOCAL> ";" {
        yyscanner->id_ref_typ = T_VARIABLE_REF;
        yy_push_state(yycID_LIST, yyscanner);
        return T_SEMICOLON;
    }
    <LOCAL> ";" / WS* 'END_LOCAL' { return T_SEMICOLON; }
    <LOCAL> 'END_LOCAL' / BREAK {
        yy_pop_state(yyscanner);
        return T_END_LOCAL;
    }

    <INV_FOR> SIMPLE_ID / WS* "." {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - yyscanner->tok, yyscanner->tok);
        plval->cstr = strdup(ibuffer);
        return T_ENTITY_REF;
    }
    <INV_FOR> SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - yyscanner->tok, yyscanner->tok);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_REF;
    }
    <INV_FOR> ";" { --yyscanner->cur; yy_pop_state(yyscanner); continue; }

    <SUBTYPE_CONS> 'FOR' / WS { return T_FOR; }        
    <SUBTYPE_CONS> SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", yyscanner->cur - yyscanner->tok, yyscanner->tok);
        idx = scope_find(pState, T_ENTITY, ibuffer, yyscanner);
        scope_push(pState, idx, yyscanner);
        plval->cstr = strdup(ibuffer);
        return T_ENTITY_REF;
    }
    <SUBTYPE_CONS> ";" { yy_pop_state(yyscanner); return T_SEMICOLON; }

    <P1,P3,COMMENT,SEARCH_SCOPE,ID_CUR_SCOPE,ENUM_ID_LIST,ENUM_REF,
     ENTITY_REF,DOT_ATTR,CONSTANT,LOCAL,INV_FOR,SUBTYPE_CONS> * {
        yyerror("unregonised input: '%.*s'", pState->lineno, 20, yyscanner->tok);
    }
    
    */

    switch(yyscanner->mode) {
        case yycP3:
            goto lbl_p3;
        case yycP1:
            goto lbl_prescan;
        case yycP2:
            yyerror("internal error - scanner should not be able to reach mode P2 here!", pState->lineno);
        default:
            yyerror("invalid scanning mode!", pState->lineno);
    }
    
lbl_prescan:
    for (midx=0; midx < sizeof(scanner_modes); midx++ ) {
        yyreset(yyscanner, scanner_modes[midx]);
        pState->lineno = 1;

lbl_p3:
        while (yyscanner->cur < yyscanner->lim) {
            if (ibufchk >= IBUFSZ)
                yyerror("id buffer overflow - undefined behaviour!", pState->lineno);
            
            yyscanner->tok = yyscanner->cur;
        
            /*!use:re2c
                re2c:define:YYCTYPE = 'unsigned char';
                re2c:define:YYDEBUG = 'lexdebug' ;
                re2c:define:YYGETCONDITION = 'yy_top_state(yyscanner)' ;
                re2c:define:YYGETCONDITION:naked = 1 ;
                re2c:define:YYSETCONDITION = '(void)(@@);' ;
                re2c:define:YYSETCONDITION:naked = 1 ;
                re2c:define:YYCURSOR = 'yyscanner->cur' ;
                re2c:define:YYLIMIT = 'yyscanner->lim' ;
                re2c:define:YYMARKER = 'yyscanner->mrk' ;
                re2c:define:YYCTXMARKER = 'yyscanner->ctx' ;
            */        
        }
        
        if (yyscanner->mode == yycP3)
            break;
    }
    
    return 0;
}

void yyreset(struct exp_scanner *s, int new_mode) {
    s->cur = s->tok = s->mrk = s->ctx = s->buffer->data;
    memset(s->cond_stack->entry, 0, sizeof(int) * s->cond_stack->mlen);
    s->cond_top = 0;
    s->cond_stack->entry[0] = new_mode;
    s->mode = new_mode;
}

void scope_push(struct YYSTATE *pState, int idx, struct exp_scanner *scanner) {
    struct scopeList *stack = pState->scope_stack;
    struct scope_def *scope = stack->entry + idx;
    scope->parent = scanner->scope_top;
    if (idx < 0) {
        yyerror("+++scope_push() internal error, pushing nonsense!", pState->lineno);
    }
    scanner->scope_top = idx;
}

int scope_pop(struct YYSTATE *pState, struct exp_scanner *scanner) {
    struct scopeList *stack = pState->scope_stack;
    int idx = scanner->scope_top;
    struct scope_def *scope = stack->entry + idx;
    if (scope->parent < 0) {
        yyerror("+++scope_pop() internal error, popping of the stack", 0);
    }
    scanner->scope_top = scope->parent;
    return idx;
}

int scope_find(struct YYSTATE *pState, int itype, const char *sname, struct exp_scanner *scanner) {
    struct scopeList *stack = pState->scope_stack;
    struct scope_def *s;
    int i, idx = -1, pidx = scanner->scope_top;
    
    /* need to match parent, name and type */
    for (i = 0; i < stack->qty; i++) {
        s = stack->entry + i;
        if (s->parent == pidx && s->itype == itype && !strcasecmp(s->sname, sname)) {
            idx = i;
            break;
        }
    }
    
    if (idx < 0)
        yyerror("+++ scope_find failed, internal error!\n", 0);
    
    return idx;
}


int scope_alloc(struct scopeList *stack, const char *typ, int tok) {
    struct scope_def *sp;
    const char *nx;
    char *np;
    int chk;

    /* TODO: error checking */
    chk = scopeListAllocMin(stack, stack->qty+1);
    if (chk != BSTR_OK)
        goto err_out;

    sp = stack->entry + stack->qty;
    sp->sname = NULL;
    sp->stype = strdup(typ);
    for (nx = typ, np = sp->stype; *nx; nx++, np++)
        *np = toupper((unsigned char) *nx);
    sp->itype = tok;
    sp->symbols = bstrListCreate();
    sp->tokens = intListCreate();
    sp->parent = 0xDEADBEEF;
    
    if (!sp->symbols || !sp->tokens)
        goto err_out;
    return stack->qty++;
    
err_out:
    yyerror("scope_alloc failure!", 0);
}


void add_symbol(struct scope_def *scope, const char *sym, int tok) {
    struct bstrList *sl = scope->symbols;
    struct intList *il = scope->tokens;
    /* TODO: check for error */
    bstrListAllocMin(sl, sl->qty+1);
    intListAllocMin(il, il->qty+1);
    sl->entry[sl->qty++] = bfromcstr(sym);
    il->entry[il->qty++] = tok;
}

int resolve_symbol(struct YYSTATE *pState, const char *sym, struct exp_scanner *scanner) {
    struct scopeList *stack = pState->scope_stack;
    struct bstrList *sl;
    struct intList *il;
    int i, cnt = 0, itype = T_SIMPLE_REF;
    struct scope_def *s = stack->entry + scanner->scope_top;
    
    while (s->itype != T_DOCROOT && !cnt) {
        il = s->tokens;
        sl = s->symbols;
        for (i = 0; i < sl->qty; i++) {
            if (!strcasecmp(sl->entry[i]->data, sym) && il->entry[i] != T_ENUMERATION_REF) {
                itype = il->entry[i];
                cnt++;
            }
        }
        s = stack->entry + s->parent;
    }

    if (cnt > 1) {
        yyerror("conflicting token types for id '%s'", 0, sym);
    } else if (cnt > 0) {
        return itype;
    }

    /* fallback - check for T_ENUMERATION_REF */
    s = stack->entry + scanner->scope_top;
    while (s->itype != T_DOCROOT) {
        il = s->tokens;
        sl = s->symbols;
        for (i = 0; i < sl->qty; i++) {
            if (!strcasecmp(sl->entry[i]->data, sym)) {
                itype = il->entry[i];
                cnt++;
            }
        }
        s = stack->entry + s->parent;
    }
    
    if (cnt > 1) {
        yyerror("conflicting token types for id '%s'", 0, sym);
    } 
    
    return itype;
}

void brkt_push(struct intList *nesting) {
    int *brkt_count;
    nesting->qty++;
    brkt_count = nesting->entry + nesting->qty;
    *brkt_count = 0;
}

int brkt_pop(struct intList *nesting) {
    nesting->qty--;
    return 0;
}


void yy_push_state(int new_state, struct exp_scanner *scanner) {
    int *p;
    size_t sz;
    
    if (scanner->cond_top + 1 < scanner->cond_stack->mlen) {
        scanner->cond_stack->entry[++scanner->cond_top] = new_state;        
    } else {
        sz = scanner->cond_stack->mlen + 10;
        p = realloc(scanner->cond_stack->entry, sz * sizeof(int));
        if (!p)
            yyerror("condition stack overflow!", 0);
        scanner->cond_stack->entry = p;
        scanner->cond_stack->mlen = sz;
        scanner->cond_stack->entry[++scanner->cond_top] = new_state;
    }
}

int yy_exch_state(int new_state, struct exp_scanner *scanner) {
    int old_state;
    
    if (scanner->cond_top >= scanner->cond_stack->mlen)
        yyerror("fatal, lexer condition stack is invalid!", 0);
        
    old_state = scanner->cond_stack->entry[scanner->cond_top];
    scanner->cond_stack->entry[scanner->cond_top] = new_state;        
    return old_state;
}

void yy_pop_state(struct exp_scanner *scanner) {
    if (!scanner->cond_top)
        yyerror("fatal, lexer trying invalid condition stack pop!", 0);
    
    scanner->cond_top--;
    /* scanner->cond_stack->entry[scanner->cond_top--] = 0; */
}

int yy_top_state(struct exp_scanner *scanner) {
    return scanner->cond_stack->entry[scanner->cond_top];
}


#if 0
int main(int argc, char **argv)
{
    FILE *fp;
    YYSTYPE yylval;
    YYLTYPE yylloc;
    struct exp_scanner *scanner;
    struct scopeList *scope_stack;
    
    if (argc < 2) {
        fprintf(stderr, "no input files\n");
        return 1;
    }

    fp = fopen(argv[1], "r");
    if (!fp)
        yyerror("failed to open input!", 0);

    yylex_init(&scanner, fp);

    scope_stack = scopeListCreate();
    scanner->scope_top = scope_alloc(pState->scope_stack, "DOCROOT", T_DOCROOT);
    scope_stack->entry[scanner->scope_top].parent = -1;        
    
    while(lex(&yylval, &yylloc, scanner, scope_stack) != 0)
        ;
    
    fclose(fp);
}
#endif

void lexdebug(int state, unsigned char symbol) {
    if (state)
        fprintf(stderr, "DBG(%i): %c\n", (state), (symbol));
}
