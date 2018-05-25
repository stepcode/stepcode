#include <stdio.h>
#include <stdlib.h>

#include "sc_memmgr.h"

#include "expparse.h"
#include "lexsupport.h"

/*!types:re2c*/

void yy_push_state(int new_state, struct exp_scanner *scanner);
int yy_exch_state(int new_state, struct exp_scanner *scanner);
void yy_pop_state(struct exp_scanner *scanner);
int yy_top_state(struct exp_scanner *scanner);

void brkt_push(struct intList *nesting);
int brkt_pop(struct intList *nesting);

void lexdebug(int state, unsigned char symbol);

#define IBUFSZ 128

int yylex(struct exp_scanner *pScanner, struct YYSTYPE *plval)
{
    int scanner_modes[] = {yycP1, yycP2, yycP3};
    size_t midx;
    unsigned char ibuffer[IBUFSZ];
    unsigned char *id;
    /*!stags:re2c format = 'unsigned char *@@;'; */
    int *brkt_nesting;
    struct scope_def *sp, *scope;
    int idx, tok, ibufchk = 0;
    struct YYSTATE *pState = pScanner->state;
    
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
        memset(pScanner->tok, ' ', pScanner->cur - pScanner->tok);
        yy_push_state(yycCOMMENT, pScanner);
        continue;
    }
        
    <P1> "--" ( NOT_PAREN_STAR | [ \r\t)(*] )* / "\n" {
        memset(pScanner->tok, ' ', pScanner->cur - pScanner->tok);
        continue;
    }

    <P1> "--" { yyerror("invalid characters in tail remark! '%.*s'", pScanner->lineno, 20, pScanner->cur); }

    <P1> "'" ("''" | NOT_PAREN_STAR \ ['] | [)(*] | WS )* "'" { continue; }

    <P1> "'" { yyerror("invalid string literal! '%.*s'", pScanner->lineno, 20, pScanner->cur); }

    <P1> ( NOT_PAREN_STAR \ ['-] )+ { continue; }

    <P1> [)(*'-] { continue; }

    <COMMENT> "(*" {
        memset(pScanner->tok, ' ', pScanner->cur - pScanner->tok);
        yy_push_state(yycCOMMENT, pScanner);
        continue;
    }
    <COMMENT> "*"+ ")" {
        memset(pScanner->tok, ' ', pScanner->cur - pScanner->tok);
        yy_pop_state(pScanner);
        continue;
    }
    <COMMENT> "*"+ {
        memset(pScanner->tok, ' ', pScanner->cur - pScanner->tok);
        continue;
    }
    <COMMENT> NOT_PAREN_STAR+ {
        memset(pScanner->tok, ' ', pScanner->cur - pScanner->tok);
        continue;
    }
    <COMMENT> [)(] {
        memset(pScanner->tok, ' ', pScanner->cur - pScanner->tok);
        continue;
    }

    
    <P2> 'SCHEMA' WS+ @id SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - id, id);
        add_symbol(pState->scope_stack->entry + pScanner->scope_top, ibuffer, T_SCHEMA_REF, pScanner);
    
        idx = scope_alloc(pState->scope_stack, "SCHEMA", OBJ_SCHEMA_BITS);
        scope_push(pState, idx, pScanner);
        sp = pState->scope_stack->entry + idx;
        sp->symbol.name = strndup(id, pScanner->cur - id);
        
        continue;
    }
    <P2> 'FUNCTION' WS+ @id SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - id, id);
        add_symbol(pState->scope_stack->entry + pScanner->scope_top, ibuffer, T_FUNCTION_REF, pScanner);

        idx = scope_alloc(pState->scope_stack, "FUNCTION", OBJ_FUNCTION_BITS);
        scope_push(pState, idx, pScanner);
        sp = pState->scope_stack->entry + idx;
        sp->symbol.name = strndup(id, pScanner->cur - id);

        continue;
    }
    <P2> 'PROCEDURE' WS+ @id SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - id, id);
        add_symbol(pState->scope_stack->entry + pScanner->scope_top, ibuffer, T_PROCEDURE_REF, pScanner);
        
        idx = scope_alloc(pState->scope_stack, "PROCEDURE", OBJ_PROCEDURE_BITS);
        scope_push(pState, idx, pScanner);
        sp = pState->scope_stack->entry + idx;
        sp->symbol.name = strndup(id, pScanner->cur - id);
        
        continue;
    }
    <P2> 'RULE' WS+ @id SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - id, id);
        add_symbol(pState->scope_stack->entry + pScanner->scope_top, ibuffer, T_RULE_REF, pScanner);
        
        idx = scope_alloc(pState->scope_stack, "RULE", OBJ_RULE_BITS);
        scope_push(pState, idx, pScanner);
        sp = pState->scope_stack->entry + idx;
        sp->symbol.name = strndup(id, pScanner->cur - id);
        
        continue;
    }
    <P2> 'ENTITY' WS+ @id SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - id, id);
        add_symbol(pState->scope_stack->entry + pScanner->scope_top, ibuffer, T_ENTITY_REF, pScanner);
        
        idx = scope_alloc(pState->scope_stack, "ENTITY", OBJ_ENTITY_BITS);
        scope_push(pState, idx, pScanner);
        sp = pState->scope_stack->entry + idx;
        sp->symbol.name = strndup(id, pScanner->cur - id);
        
        continue;
    }
    <P2> 'TYPE' WS+ @id SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - id, id);
        add_symbol(pState->scope_stack->entry + pScanner->scope_top, ibuffer, T_TYPE_REF, pScanner);
        
        idx = scope_alloc(pState->scope_stack, "TYPE", OBJ_TYPE_BITS);
        scope_push(pState, idx, pScanner);
        sp = pState->scope_stack->entry + idx;
        sp->symbol.name = strndup(id, pScanner->cur - id);
        
        continue;
    }

    <P2> 'END_' ('SCHEMA'|'FUNCTION'|'PROCEDURE'|'RULE'|'ENTITY'|'TYPE') / BREAK {
        scope_pop(pState, pScanner);
        continue;
    }

    <P2> "'" ("''" | NOT_PAREN_STAR \ ['] | [)(*] | WS )* "'" { continue; }
    <P2> SIMPLE_ID { continue; }
    <P2> * { continue; }


    <TYPE> "=" { return T_EQ; }
    <TYPE> 'ENUMERATION' / WS {
        yy_push_state(yycENUM, pScanner);
        return T_ENUMERATION;
    }
    <TYPE> 'EXTENSIBLE' / WS { return T_EXTENSIBLE; }
    <TYPE> 'WHERE' / WS {
        pScanner->id_ref_typ = T_RULE_LABEL_REF;
        yy_push_state(yycLABEL_OPT, pScanner);
        return T_WHERE;
    }
    <TYPE> ";" / WS* 'WHERE' { return T_SEMICOLON; }
    <TYPE> ";" {
        pScanner->id_ref_typ = T_RULE_LABEL_REF;
        yy_push_state(yycLABEL_OPT, pScanner);
        return T_SEMICOLON;
    }
    <TYPE> ";" / WS* 'END_TYPE' { return T_SEMICOLON; }
    <TYPE> 'END_TYPE' / BREAK {
        yy_pop_state(pScanner);
        scope_pop(pState, pScanner);
        return T_END_TYPE;
    }
    <TYPE> "" { yy_push_state(yycSTMT, pScanner); continue; }

    <ENUM> "(" { yy_push_state(yycENUM_ID_LIST, pScanner); return T_LPAREN; }
    <ENUM> ")" { yy_pop_state(pScanner); return T_RPAREN; }
    <ENUM> "" :=> P3

    <ENTITY> 'DERIVE' / WS {
        pScanner->in_explicit_attr = false;
        pScanner->in_inverse_clause = false;
        pScanner->in_rules_clause = false;
        pScanner->id_ref_typ = T_SIMPLE_REF;
        yy_push_state(yycID_LIST, pScanner);
        return T_DERIVE;
    }
    <ENTITY> 'INVERSE' / WS {
        pScanner->in_explicit_attr = false;
        pScanner->in_inverse_clause = true;
        pScanner->in_rules_clause = false;
        pScanner->id_ref_typ = T_SIMPLE_REF;
        yy_push_state(yycID_LIST, pScanner);
        return T_INVERSE;
    }
    <ENTITY> 'UNIQUE' / WS {
        pScanner->in_explicit_attr = false;
        pScanner->in_inverse_clause = false;
        pScanner->in_rules_clause = true;
        pScanner->id_ref_typ = T_RULE_LABEL_REF;
        yy_push_state(yycLABEL_OPT, pScanner);
        return T_UNIQUE;
    }
    <ENTITY> 'WHERE' / WS {
        pScanner->in_explicit_attr = false;
        pScanner->in_inverse_clause = false;
        pScanner->in_rules_clause = true;
        pScanner->id_ref_typ = T_RULE_LABEL_REF;
        yy_push_state(yycLABEL_OPT, pScanner);
        return T_WHERE;
    }
    <ENTITY> ":" / [^=<] {
        if (pScanner->in_inverse_clause || pScanner->in_explicit_attr) {
            pScanner->scope_aux = scope_pop(pState, pScanner);
        }
        if (!pScanner->in_inverse_clause) {
            yy_push_state(yycSTMT, pScanner);
        } else {
            yy_push_state(yycINV_FOR_STMT, pScanner);
        }
        return T_COLON;
    }
    <ENTITY> ";" / WS* ('DERIVE'|'INVERSE'|'UNIQUE'|'WHERE'|'END_ENTITY') BREAK {
        if (pScanner->scope_aux) {
            scope_push(pState, pScanner->scope_aux, pScanner);
            pScanner->scope_aux = 0;
        }
        return T_SEMICOLON;
    }
    <ENTITY> ";" {
        if (pScanner->scope_aux) {
            scope_push(pState, pScanner->scope_aux, pScanner);
            pScanner->scope_aux = 0;
        }
        if (!pScanner->in_rules_clause) {
            yy_push_state(yycID_LIST, pScanner);        
        } else {
            yy_push_state(yycLABEL_OPT, pScanner);
        }
        return T_SEMICOLON;
    }
    <ENTITY> 'END_ENTITY' / BREAK {
        scope_pop(pState, pScanner);
        yy_pop_state(pScanner);
        return T_END_ENTITY;
    }
    <ENTITY> "" :=> P3

    <ARG_LIST> [(;] {
        /* TODO: bracket_count */
        pScanner->id_ref_typ = T_PARAMETER_REF;
        yy_push_state(yycID_LIST, pScanner);
        return ( *pScanner->tok == '(' ) ? T_LPAREN : T_SEMICOLON;
    }
    <ARG_LIST> ")" {
        /* TODO: bracket_count */
        yy_pop_state(pScanner);
        return T_RPAREN;
    }
    <ARG_LIST> 'VAR' / WS {
        pScanner->id_ref_typ = T_PARAMETER_REF;
        yy_push_state(yycID_LIST, pScanner);
        return T_VAR;
    }
    <ARG_LIST> "" :=> P3

    <RULE> 'WHERE'/ WS {
        pScanner->in_rules_clause = true;
        pScanner->id_ref_typ = T_RULE_LABEL_REF;
        yy_push_state(yycLABEL_OPT, pScanner);
        return T_WHERE;
    }
    <RULE> ";" {
        if (!pScanner->in_rules_clause) {
            /* TODO: is this branch ever followed? */
            pScanner->id_ref_typ = T_SIMPLE_REF;
            yy_push_state(yycSTMT, pScanner);
        } else {
            pScanner->id_ref_typ = T_RULE_LABEL_REF;
            yy_push_state(yycLABEL_OPT, pScanner);
        }
        return T_SEMICOLON;
    }
    <RULE> ";" / WS* 'WHERE' WS { return T_SEMICOLON; }
    <RULE> ";" / WS* 'END_RULE' { return T_SEMICOLON; }
    <RULE> 'END_RULE' / BREAK {
        pScanner->in_rules_clause = false;
        scope_pop(pState, pScanner);
        return T_END_RULE;
    }
    <RULE> "" { yy_push_state(yycSTMT, pScanner); continue; }

    <QUERY> ")" {
        brkt_nesting = pScanner->brkt_nesting->entry + pScanner->brkt_nesting->qty;
        if (!--*brkt_nesting) {
            brkt_pop(pScanner->brkt_nesting);
            scope_pop(pState, pScanner);
            yy_pop_state(pScanner);
        }
        return T_RPAREN;
    }
    <QUERY> "(" {
        brkt_nesting = pScanner->brkt_nesting->entry + pScanner->brkt_nesting->qty;
        ++*brkt_nesting;
        return T_LPAREN;
    }
    <QUERY> "" :=> P3

    <REDECL> 'RENAMED' / WS {
        pScanner->id_ref_typ = T_SIMPLE_REF;
        yy_push_state(yycID_CUR_SCOPE, pScanner);
        return T_RENAMED;
    }
    <REDECL> [,:] { --pScanner->cur; yy_pop_state(pScanner); continue; }
    <REDECL> "" :=> P3

    <STMT> ";" { --pScanner->cur; yy_pop_state(pScanner); continue; }
    <STMT> "" :=> P3

    <INV_FOR_STMT> 'FOR' / WS { yy_push_state(yycINV_FOR, pScanner); return T_FOR; }
    <INV_FOR_STMT> ";" { --pScanner->cur; yy_pop_state(pScanner); continue; }
    <INV_FOR_STMT> "" :=> P3

    <P3> "%" [01]+ { return T_BINARY_LITERAL; }

    <P3> digit+ { return T_INTEGER_LITERAL; }

    <P3> digit+ "." digit* ([eE] [+-]? digit+)? { return T_REAL_LITERAL; }

    <P3> "'" ("''" | NOT_PAREN_STAR \ ['] | [)(*] | WS )* "'" { return T_STRING_LITERAL; }

    <P3> "\"" ( xdigit{4} )+ "\"" { return T_ENCODED_STRING_LITERAL; }
    <P3> "\"" { yyerror("invalid encoded string literal! '%.*s'", pScanner->lineno, 20, pScanner->cur); }

    <P3> 'SCHEMA'/ WS {
        pScanner->scope_typ = OBJ_SCHEMA_BITS;
        yy_push_state(yycSEARCH_SCOPE, pScanner);
        return T_SCHEMA;
    }
    <P3> 'END_SCHEMA' / BREAK {
        scope_pop(pState, pScanner);
        return T_END_SCHEMA;
    }

    <P3> 'TYPE' / WS {
        pScanner->scope_typ = OBJ_TYPE_BITS;
        yy_push_state(yycTYPE, pScanner);
        yy_push_state(yycSEARCH_SCOPE, pScanner);
        return T_TYPE;
    }

    <P3> 'ENTITY' / WS {
        pScanner->scope_typ = OBJ_ENTITY_BITS;
        pScanner->id_ref_typ = T_SIMPLE_REF;
        pScanner->in_inverse_clause = false;
        pScanner->in_explicit_attr = true;
        pScanner->in_rules_clause = false;
        pScanner->scope_aux = 0;
        
        yy_push_state(yycENTITY, pScanner);
        yy_push_state(yycSTMT, pScanner);
        yy_push_state(yycSEARCH_SCOPE, pScanner);
        
        return T_ENTITY;
    }

    <P3> 'FUNCTION' / WS* SIMPLE_ID WS* "(" {
        pScanner->scope_typ = OBJ_FUNCTION_BITS;
        yy_push_state(yycARG_LIST, pScanner);
        yy_push_state(yycSEARCH_SCOPE, pScanner);
        return T_FUNCTION;
    }
    <P3> 'FUNCTION' / WS {
        pScanner->scope_typ = OBJ_FUNCTION_BITS;
        yy_push_state(yycSEARCH_SCOPE, pScanner);
        return T_FUNCTION;
    }
    <P3> 'END_FUNCTION' / BREAK {
        scope_pop(pState, pScanner);
        return T_END_FUNCTION;
    }

    <P3> 'PROCEDURE' / WS* SIMPLE_ID WS* "(" {
        pScanner->scope_typ = OBJ_PROCEDURE_BITS;
        yy_push_state(yycARG_LIST, pScanner);
        yy_push_state(yycSEARCH_SCOPE, pScanner);
        return T_PROCEDURE;
    }
    <P3> 'PROCEDURE' / WS {
        pScanner->scope_typ = OBJ_PROCEDURE_BITS;
        yy_push_state(yycSEARCH_SCOPE, pScanner);
        return T_PROCEDURE;
    }
    <P3> 'END_PROCEDURE' / BREAK {
        scope_pop(pState, pScanner);
        return T_END_PROCEDURE;
    }

    <P3> 'RULE' / WS {
        pScanner->scope_typ = OBJ_RULE_BITS;
        yy_push_state(yycRULE, pScanner);
        yy_push_state(yycSTMT, pScanner);
        yy_push_state(yycSEARCH_SCOPE, pScanner);
        return T_RULE;
    }

    <P3> 'ALIAS' / WS {
        idx = scope_alloc(pState->scope_stack, "ALIAS", OBJ_ALIAS_BITS);
        scope = pState->scope_stack->entry + idx;

        ibufchk = snprintf(ibuffer, sizeof(ibuffer), "QANON_%08lX", pScanner->anon_scope_cnt);
        scope->symbol.name = strdup(ibuffer);
        scope_push(pState, idx, pScanner);
        pScanner->anon_scope_cnt++;
        
        pScanner->id_ref_typ = T_VARIABLE_REF;
        yy_push_state(yycID_CUR_SCOPE, pScanner);
        return T_ALIAS;
    }
    <P3> 'END_ALIAS' / BREAK {
        scope_pop(pState, pScanner);
        return T_END_ALIAS;
    }

    <P3> 'SUBTYPE_CONSTRAINT' / WS {
        /* _ref not supported, as only used in remarks */
        pScanner->id_ref_typ = T_INVALID;
        
        yy_push_state(yycSUBTYPE_CONS, pScanner);
        yy_push_state(yycID_CUR_SCOPE, pScanner);
        return T_SUBTYPE_CONSTRAINT;
    }

    <P3> 'END_SUBTYPE_CONSTRAINT' / BREAK {
        /* TODO: not clear, rework? - the Entity scope is pushed in <SUBTYPE_CONS> */
        scope_pop(pState, pScanner);
        return T_END_SUBTYPE_CONSTRAINT;
    }

    <P3> 'REPEAT' / WS+ SIMPLE_ID WS* ":=" {
        idx = scope_alloc(pState->scope_stack, "REPEAT", T_REPEAT);
        scope = pState->scope_stack->entry + idx;

        ibufchk = snprintf(ibuffer, sizeof(ibuffer), "REP_ANON_%08lX", pScanner->anon_scope_cnt);
        scope->symbol.name = strdup(ibuffer);
        scope_push(pState, idx, pScanner);
        pScanner->anon_scope_cnt++;
        
        pScanner->id_ref_typ = T_VARIABLE_REF;
        yy_push_state(yycID_CUR_SCOPE, pScanner);
        return T_REPEAT;
    }
    <P3> 'REPEAT' / WS {
        idx = scope_alloc(pState->scope_stack, "REPEAT", T_REPEAT);
        scope = pState->scope_stack->entry + idx;

        ibufchk = snprintf(ibuffer, sizeof(ibuffer), "REP_ANON_%08lX", pScanner->anon_scope_cnt);
        scope->symbol.name = strdup(ibuffer);
        scope_push(pState, idx, pScanner);
        pScanner->anon_scope_cnt++;
        return T_REPEAT;
    }
    <P3> 'END_REPEAT' / BREAK {
        scope_pop(pState, pScanner);
        return T_END_REPEAT;
    }

    <P3> 'QUERY' / WS* "(" {
        /* TODO: clean up this mess! */
        idx = scope_alloc(pState->scope_stack, "QUERY", T_QUERY);
        scope = pState->scope_stack->entry + idx;

        ibufchk = snprintf(ibuffer, sizeof(ibuffer), "QRY_ANON_%08lX", pScanner->anon_scope_cnt);
        scope->symbol.name = strdup(ibuffer);
        scope_push(pState, idx, pScanner);

        brkt_push(pScanner->brkt_nesting);
        brkt_nesting = pScanner->brkt_nesting->entry + pScanner->brkt_nesting->qty;
        ++*brkt_nesting;
        
        pScanner->anon_scope_cnt++;
        
        pScanner->id_ref_typ = T_VARIABLE_REF;
        yy_push_state(yycQUERY, pScanner);
        yy_push_state(yycID_CUR_SCOPE, pScanner);
        return T_QUERY;
    }

    <P3> 'CONSTANT' / WS {
        yy_push_state(yycCONSTANT, pScanner);
        return T_CONSTANT;
    }

    <P3> 'LOCAL' / WS {
        pScanner->id_ref_typ = T_VARIABLE_REF;
        yy_push_state(yycLOCAL, pScanner);
        yy_push_state(yycID_LIST, pScanner);
        return T_LOCAL;
    }

        /* recognise type_ref '.' */
    <P3> SIMPLE_ID / WS* "." WS* SIMPLE_ID WS* [^.] {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - pScanner->tok, pScanner->tok);
        tok = resolve_symbol(pState, ibuffer, pScanner);
        if (tok == T_TYPE_REF)
            yy_push_state(yycENUM_REF, pScanner);
        return tok;
    }

        /* recognise group_qualifier */
    <P3> "\\" / WS* SIMPLE_ID {
        yy_push_state(yycENTITY_REF, pScanner);
        return T_BACKSLASH;
    }

        /* return only T_SIMPLE_ID for all occurences of type_label */
    <P3> 'GENERIC_ENTITY' / WS* ":" [^=<] {
        yy_push_state(yycID_CUR_SCOPE, pScanner);
        return T_GENERIC_ENTITY;
    }

    <P3> 'AGGREGATE' / WS* ":" [^=<] {
        yy_push_state(yycID_CUR_SCOPE, pScanner);
        return T_AGGREGATE;
    }

    <P3> 'GENERIC' / WS* ":" [^=<] {
        yy_push_state(yycID_CUR_SCOPE, pScanner);
        return T_GENERIC;
    }

    <P3> "." / WS* SIMPLE_ID {
        yy_push_state(yycDOT_ATTR, pScanner);
        return T_DOT;
    }

    <P3> SIMPLE_ID {
        int i;
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - pScanner->tok, pScanner->tok);
        for (i=0; reserved_keywords[i].kw != NULL; i++) {
            if (!strcasecmp(ibuffer, reserved_keywords[i].kw)) {
                return reserved_keywords[i].token;
            }
        }
        
        tok = resolve_symbol(pState, ibuffer, pScanner);
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
    <*> [\n] { pScanner->lineno++; continue; }

    <SEARCH_SCOPE> SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - pScanner->tok, pScanner->tok);
        idx = scope_find(pState, pScanner->scope_typ, ibuffer, pScanner);
        scope_push(pState, idx, pScanner);
        yy_pop_state(pScanner);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_ID;
    }

    <ID_CUR_SCOPE> SIMPLE_ID {
        struct scope_def *sp = pState->scope_stack->entry + pScanner->scope_top;
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - pScanner->tok, pScanner->tok);
        add_symbol(sp, ibuffer, pScanner->id_ref_typ, pScanner);
        yy_pop_state(pScanner);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_ID;
    }
    <ID_CUR_SCOPE> [(:] { return ( *pScanner->tok == '(' ) ? T_LPAREN : T_COLON; }

    <ID_LIST> SIMPLE_ID / WS* [,:)] {
        struct scope_def *sp = pState->scope_stack->entry + pScanner->scope_top;
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - pScanner->tok, pScanner->tok);
        add_symbol(sp, ibuffer, pScanner->id_ref_typ, pScanner);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_ID;
    }
    <ID_LIST> 'SELF' / WS* "\\" { yy_push_state(yycREDECL, pScanner); return T_SELF; }
    <ID_LIST> "," { return T_COMMA; }
    <ID_LIST> "" { yy_pop_state(pScanner); continue; }
    
    <ENUM_ID_LIST> SIMPLE_ID / WS* [,)] {
        struct scope_def *sp = pState->scope_stack->entry + pScanner->scope_top;
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - pScanner->tok, pScanner->tok);
        add_symbol(sp, ibuffer, T_ENUMERATION_REF, pScanner);
        add_symbol(pState->scope_stack->entry + sp->parent, ibuffer, T_ENUMERATION_REF, pScanner);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_ID;
    }
    <ENUM_ID_LIST> "," { return T_COMMA; }
    <ENUM_ID_LIST> ")" { --pScanner->cur; yy_pop_state(pScanner); continue; }

    <LABEL_OPT> SIMPLE_ID / WS* ":" {
        struct scope_def *sp = pState->scope_stack->entry + pScanner->scope_top;
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - pScanner->tok, pScanner->tok);
        add_symbol(sp, ibuffer, pScanner->id_ref_typ, pScanner);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_ID;
    }
    <LABEL_OPT> "" { yy_exch_state(yycSTMT, pScanner); continue; }

    <ENUM_REF> "." { return T_DOT; }        
    <ENUM_REF> SIMPLE_ID {
        yy_pop_state(pScanner);
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - pScanner->tok, pScanner->tok);
        plval->cstr = strdup(ibuffer);
        return T_ENUMERATION_REF;
    }

    <ENTITY_REF> SIMPLE_ID {
        yy_pop_state(pScanner);
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - pScanner->tok, pScanner->tok);
        plval->cstr = strdup(ibuffer);
        return T_ENTITY_REF;
    }

    <DOT_ATTR> SIMPLE_ID {
        yy_pop_state(pScanner);
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - pScanner->tok, pScanner->tok);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_REF;
    }

    <CONSTANT> ":" { yy_push_state(yycSTMT, pScanner); return T_COLON; }
    <CONSTANT> ";" { return T_SEMICOLON; }
    <CONSTANT> ";" / WS* 'END_CONSTANT' { return T_SEMICOLON; }
    <CONSTANT> 'END_CONSTANT' / BREAK {
        yy_pop_state(pScanner);
        return T_END_CONSTANT;
    }
    <CONSTANT> SIMPLE_ID / WS* ":" {
        struct scope_def *sp = pState->scope_stack->entry + pScanner->scope_top;
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - pScanner->tok, pScanner->tok);
        add_symbol(sp, ibuffer, T_CONSTANT_REF, pScanner);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_ID;
    }

    <LOCAL> ":" { yy_push_state(yycSTMT, pScanner); return T_COLON; }
    <LOCAL> ";" {
        pScanner->id_ref_typ = T_VARIABLE_REF;
        yy_push_state(yycID_LIST, pScanner);
        return T_SEMICOLON;
    }
    <LOCAL> ";" / WS* 'END_LOCAL' { return T_SEMICOLON; }
    <LOCAL> 'END_LOCAL' / BREAK {
        yy_pop_state(pScanner);
        return T_END_LOCAL;
    }

    <INV_FOR> SIMPLE_ID / WS* "." {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - pScanner->tok, pScanner->tok);
        plval->cstr = strdup(ibuffer);
        return T_ENTITY_REF;
    }
    <INV_FOR> SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - pScanner->tok, pScanner->tok);
        plval->cstr = strdup(ibuffer);
        return T_SIMPLE_REF;
    }
    <INV_FOR> ";" { --pScanner->cur; yy_pop_state(pScanner); continue; }

    <SUBTYPE_CONS> 'FOR' / WS { return T_FOR; }        
    <SUBTYPE_CONS> SIMPLE_ID {
        ibufchk = snprintf(ibuffer, IBUFSZ, "%.*s", pScanner->cur - pScanner->tok, pScanner->tok);
        idx = scope_find(pState, OBJ_ENTITY_BITS, ibuffer, pScanner);
        scope_push(pState, idx, pScanner);
        plval->cstr = strdup(ibuffer);
        return T_ENTITY_REF;
    }
    <SUBTYPE_CONS> ";" { yy_pop_state(pScanner); return T_SEMICOLON; }

    <P1,P3,COMMENT,SEARCH_SCOPE,ID_CUR_SCOPE,ENUM_ID_LIST,ENUM_REF,
     ENTITY_REF,DOT_ATTR,CONSTANT,LOCAL,INV_FOR,SUBTYPE_CONS> * {
        yyerror("unregonised input: '%.*s'", pScanner->lineno, 20, pScanner->tok);
    }
    
    */

    switch(pScanner->mode) {
        case yycP3:
            goto lbl_p3;
        case yycP1:
            goto lbl_prescan;
        case yycP2:
            yyerror("internal error - scanner should not be able to reach mode P2 here!", pScanner->lineno);
        default:
            yyerror("invalid scanning mode!", pScanner->lineno);
    }
    
lbl_prescan:
    for (midx=0; midx < sizeof(scanner_modes); midx++ ) {
        yylexReset(pScanner, scanner_modes[midx]);
        pScanner->lineno = 1;

lbl_p3:
        while (pScanner->cur < pScanner->lim) {
            if (ibufchk >= IBUFSZ)
                yyerror("id buffer overflow - undefined behaviour!", pScanner->lineno);
            
            pScanner->tok = pScanner->cur;
        
            /*!use:re2c
                re2c:define:YYCTYPE = 'unsigned char';
                re2c:define:YYDEBUG = 'if (pScanner->debug) lexdebug' ;
                re2c:define:YYGETCONDITION = 'yy_top_state(pScanner)' ;
                re2c:define:YYGETCONDITION:naked = 1 ;
                re2c:define:YYSETCONDITION = '(void)(@@);' ;
                re2c:define:YYSETCONDITION:naked = 1 ;
                re2c:define:YYCURSOR = 'pScanner->cur' ;
                re2c:define:YYLIMIT = 'pScanner->lim' ;
                re2c:define:YYMARKER = 'pScanner->mrk' ;
                re2c:define:YYCTXMARKER = 'pScanner->ctx' ;
            */        
        }
        
        if (pScanner->mode == yycP3)
            break;
    }
    
    return 0;
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
        p = sc_realloc(scanner->cond_stack->entry, sz * sizeof(int));
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
}

int yy_top_state(struct exp_scanner *scanner) {
    return scanner->cond_stack->entry[scanner->cond_top];
}

void lexdebug(int state, unsigned char symbol) {
    fprintf(stderr, "LEXSTATE(%i): %c\n", state, symbol);
}
