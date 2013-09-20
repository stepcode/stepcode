/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
#include <stdio.h>
#line 2 "expparse.y"

#include <assert.h>
#include "token_type.h"
#include "parse_data.h"

int yyerrstatus = 0;
#define yyerrok (yyerrstatus = 0)

YYSTYPE yylval;

    /*
     * YACC grammar for Express parser.
     *
     * This software was developed by U.S. Government employees as part of
     * their official duties and is not subject to copyright.
     *
     * $Log: expparse.y,v $
     * Revision 1.23  1997/11/14 17:09:04  libes
     * allow multiple group references
     *
     * Revision 1.22  1997/01/21 19:35:43  dar
     * made C++ compatible
     *
     * Revision 1.21  1995/06/08  22:59:59  clark
     * bug fixes
     *
     * Revision 1.20  1995/04/08  21:06:07  clark
     * WHERE rule resolution bug fixes, take 2
     *
     * Revision 1.19  1995/04/08  20:54:18  clark
     * WHERE rule resolution bug fixes
     *
     * Revision 1.19  1995/04/08  20:49:08  clark
     * WHERE
     *
     * Revision 1.18  1995/04/05  13:55:40  clark
     * CADDETC preval
     *
     * Revision 1.17  1995/03/09  18:43:47  clark
     * various fixes for caddetc - before interface clause changes
     *
     * Revision 1.16  1994/11/29  20:55:58  clark
     * fix inline comment bug
     *
     * Revision 1.15  1994/11/22  18:32:39  clark
     * Part 11 IS; group reference
     *
     * Revision 1.14  1994/11/10  19:20:03  clark
     * Update to IS
     *
     * Revision 1.13  1994/05/11  19:50:00  libes
     * numerous fixes
     *
     * Revision 1.12  1993/10/15  18:47:26  libes
     * CADDETC certified
     *
     * Revision 1.10  1993/03/19  20:53:57  libes
     * one more, with feeling
     *
     * Revision 1.9  1993/03/19  20:39:51  libes
     * added unique to parameter types
     *
     * Revision 1.8  1993/02/16  03:17:22  libes
     * reorg'd alg bodies to not force artificial begin/ends
     * added flag to differentiate parameters in scopes
     * rewrote query to fix scope handling
     * added support for Where type
     *
     * Revision 1.7  1993/01/19  22:44:17  libes
     * *** empty log message ***
     *
     * Revision 1.6  1992/08/27  23:36:35  libes
     * created fifo for new schemas that are parsed
     * connected entity list to create of oneof
     *
     * Revision 1.5  1992/08/18  17:11:36  libes
     * rm'd extraneous error messages
     *
     * Revision 1.4  1992/06/08  18:05:20  libes
     * prettied up interface to print_objects_when_running
     *
     * Revision 1.3  1992/05/31  23:31:13  libes
     * implemented ALIAS resolution
     *
     * Revision 1.2  1992/05/31  08:30:54  libes
     * multiple files
     *
     * Revision 1.1  1992/05/28  03:52:25  libes
     * Initial revision
     */

#include "express/symbol.h"
#include "express/linklist.h"
#include "stack.h"
#include "express/express.h"
#include "express/schema.h"
#include "express/entity.h"
#include "express/resolve.h"
#include "expscan.h"
#include <float.h>

    extern int print_objects_while_running;

    int tag_count;    /* use this to count tagged GENERIC types in the formal */
    /* argument lists.  Gross, but much easier to do it this */
    /* way then with the 'help' of yacc. */
    /* Set it to -1 to indicate that tags cannot be defined, */
    /* only used (outside of formal parameter list, i.e. for */
    /* return types).  Hey, as long as */
    /* there's a gross hack sitting around, we might as well */
    /* milk it for all it's worth!  -snc */

    Express yyexpresult;    /* hook to everything built by parser */

    Symbol *interface_schema;    /* schema of interest in use/ref clauses */
    void (*interface_func)();    /* func to attach rename clauses */

    /* record schemas found in a single parse here, allowing them to be */
    /* differentiated from other schemas parsed earlier */
    Linked_List PARSEnew_schemas;

    void SCANskip_to_end_schema(perplex_t scanner);

    int yylineno;

    bool yyeof = false;

#define MAX_SCOPE_DEPTH    20    /* max number of scopes that can be nested */

    static struct scope {
        struct Scope_ *this_;
        char type;    /* one of OBJ_XXX */
        struct scope *pscope;    /* pointer back to most recent scope */
        /* that has a printable name - for better */
        /* error messages */
    } scopes[MAX_SCOPE_DEPTH], *scope;
#define CURRENT_SCOPE (scope->this_)
#define PREVIOUS_SCOPE ((scope-1)->this_)
#define CURRENT_SCHEMA (scope->this_->u.schema)
#define CURRENT_SCOPE_NAME        (OBJget_symbol(scope->pscope->this_,scope->pscope->type)->name)
#define CURRENT_SCOPE_TYPE_PRINTABLE    (OBJget_type(scope->pscope->type))

    /* ths = new scope to enter */
    /* sym = name of scope to enter into parent.  Some scopes (i.e., increment) */
    /*       are not named, in which case sym should be 0 */
    /*     This is useful for when a diagnostic is printed, an earlier named */
    /*      scoped can be used */
    /* typ = type of scope */
#define PUSH_SCOPE(ths,sym,typ) \
    if (sym) DICTdefine(scope->this_->symbol_table,(sym)->name,(Generic)ths,sym,typ);\
    ths->superscope = scope->this_; \
    scope++;        \
    scope->type = typ;    \
    scope->pscope = (sym?scope:(scope-1)->pscope); \
    scope->this_ = ths; \
    if (sym) { \
        ths->symbol = *(sym); \
    }
#define POP_SCOPE() scope--

    /* PUSH_SCOPE_DUMMY just pushes the scope stack with nothing actually on it */
    /* Necessary for situations when a POP_SCOPE is unnecessary but inevitable */
#define PUSH_SCOPE_DUMMY() scope++

    /* normally the superscope is added by PUSH_SCOPE, but some things (types) */
    /* bother to get pushed so fix them this way */
#define SCOPEadd_super(ths) ths->superscope = scope->this_;

#define ERROR(code)    ERRORreport(code, yylineno)

void parserInitState()
{
    scope = scopes;
    /* no need to define scope->this */
    scope->this_ = yyexpresult;
    scope->pscope = scope;
    scope->type = OBJ_EXPRESS;
    yyexpresult->symbol.name = yyexpresult->u.express->filename;
    yyexpresult->symbol.filename = yyexpresult->u.express->filename;
    yyexpresult->symbol.line = 1;
}
#line 190 "expparse.c"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    ParseTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is ParseTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    ParseARG_SDECL     A static variable declaration for the %extra_argument
**    ParseARG_PDECL     A parameter declaration for the %extra_argument
**    ParseARG_STORE     Code to store %extra_argument into yypParser
**    ParseARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned short int
#define YYNOCODE 280
#define YYACTIONTYPE unsigned short int
#define ParseTOKENTYPE  YYSTYPE 
typedef union {
  int yyinit;
  ParseTOKENTYPE yy0;
  struct qualifier yy46;
  Variable yy91;
  Op_Code yy126;
  struct entity_body yy176;
  Where yy234;
  struct subsuper_decl yy242;
  struct type_flags yy252;
  struct upper_lower yy253;
  Symbol* yy275;
  Type yy297;
  Case_Item yy321;
  Statement yy332;
  Linked_List yy371;
  struct type_either yy378;
  struct subtypes yy385;
  Expression yy401;
  Qualified_Attr* yy457;
  TypeBody yy477;
  Integer yy507;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 0
#endif
#define ParseARG_SDECL  parse_data_t parseData ;
#define ParseARG_PDECL , parse_data_t parseData 
#define ParseARG_FETCH  parse_data_t parseData  = yypParser->parseData 
#define ParseARG_STORE yypParser->parseData  = parseData 
#define YYNSTATE 650
#define YYNRULE 333
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
#define YY_ACTTAB_COUNT (2659)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    77,   78,  619,   67,   68,   45,  384,   71,   69,   70,
 /*    10 */    72,  248,   79,   74,   73,   16,   42,  588,  400,  399,
 /*    20 */    75,  487,  486,  392,  368,  604,   57,   56,   89,  607,
 /*    30 */   268,  602,   60,   35,  601,  383,  599,  603,   66,  174,
 /*    40 */   598,   44,  151,  156,  555,  624,  623,  112,  111,  574,
 /*    50 */    77,   78,  226,  165,  617,  166,  527,  249,  109,  618,
 /*    60 */   306,   15,   79,  616,  107,   16,   42,   87,  626,  611,
 /*    70 */   453,  529,  157,  392,  301,  377,  613,  612,  610,  609,
 /*    80 */   608,  409,  412,  182,  413,  178,  411,  167,   66,  391,
 /*    90 */   215,  984,  117,  407,  202,  624,  623,  112,  111,  405,
 /*   100 */    77,   78,  617,  548,  617,   60,  522,  244,  539,  618,
 /*   110 */   168,  616,   79,  616,  142,   16,   42,  553,   39,  611,
 /*   120 */   400,  399,   75,  392,  301,   82,  613,  612,  610,  609,
 /*   130 */   608,  528,  471,  458,  470,  473,  869,  469,   66,  391,
 /*   140 */   350,  472,  530,  234,  638,  624,  623,  400,  348,   75,
 /*   150 */    77,   78,   73,  129,  617,  471,  474,  470,  473,  618,
 /*   160 */   469,  311,   79,  616,  472,   16,   42,  346,  525,  611,
 /*   170 */   507,  120,  333,  392,  247,  223,  613,  612,  610,  609,
 /*   180 */   608,  471,  468,  470,  473,  409,  469,  782,   66,  391,
 /*   190 */   472,  112,  111,  112,  111,  624,  623,  649,  621,  562,
 /*   200 */    77,   78,  520,  346,  617,  471,  467,  470,  473,  618,
 /*   210 */   469,  455,   79,  616,  472,   16,   42,  359,  142,  611,
 /*   220 */   869,  120,  446,  312,  449,  114,  613,  612,  610,  609,
 /*   230 */   608,  367,  365,  361,  406,  736,  118,  549,   66,  391,
 /*   240 */   526,  123,  564,   29,  168,  624,  623,  221,  152,  514,
 /*   250 */   322,  553,  318,  251,  617,  625,  514,   23,  451,  618,
 /*   260 */    67,   68,  250,  616,   71,   69,   70,   72,  119,  611,
 /*   270 */    74,   73,  134,  382,  113,  344,  613,  612,  610,  609,
 /*   280 */   608,  594,  592,  595,  169,  590,  589,  593,  596,  391,
 /*   290 */   591,   67,   68,  518,  647,   71,   69,   70,   72,  614,
 /*   300 */   564,   74,   73,  362,  782,  152,  531,   36,  198,  222,
 /*   310 */   113,  477,  390,  514,  406,  500,  499,  498,  497,  496,
 /*   320 */   495,  494,  493,  492,  491,    2,  302,  647,  574,  310,
 /*   330 */   127,  382,  163,  620,  161,  628,  245,  243,  581,  580,
 /*   340 */   242,  240,   39,  521,  315,  334,  106,  299,  152,  237,
 /*   350 */   356,  236,  630,   19,   26,  631,  514,    3,  632,  637,
 /*   360 */   636,  525,  635,  634,  647,  160,  159,  343,  217,    5,
 /*   370 */   533,  286,  500,  499,  498,  497,  496,  495,  494,  493,
 /*   380 */   492,  491,    2,   71,   69,   70,   72,  127,  533,   74,
 /*   390 */    73,  435,  152,  355,  436,  434,  529,  437,  637,  636,
 /*   400 */   514,  635,  634,   39,  432,  448,   14,  203,   12,  131,
 /*   410 */    84,   13,  103,  335,    3,  500,  499,  498,  497,  496,
 /*   420 */   495,  494,  493,  492,  491,    2,  389,  617,  337,  433,
 /*   430 */   127,  102,   43,  524,   67,   68,  616,  304,   71,   69,
 /*   440 */    70,   72,  152,   41,   74,   73,   91,  335,  525,  406,
 /*   450 */   514,   39,  586,   63,  189,   10,  349,    3,  231,  500,
 /*   460 */   499,  498,  497,  496,  495,  494,  493,  492,  491,    2,
 /*   470 */   439,   67,   68,  335,  127,   71,   69,   70,   72,  315,
 /*   480 */    90,   74,   73,  438,  454,  152,  222,  354,  316,  585,
 /*   490 */   405,  645,  184,  514,  238,  185,  362,  646,  644,  643,
 /*   500 */    39,    3,  642,  641,  640,  639,  116,  228,  202,  500,
 /*   510 */   499,  498,  497,  496,  495,  494,  493,  492,  491,    2,
 /*   520 */   523,  132,   76,  525,  127,  417,  556,  561,  181,  152,
 /*   530 */   352,   40,   39,  309,  574,  331,  208,  514,  246,  162,
 /*   540 */   173,  628,  245,  243,  581,  580,  242,  240,  556,  429,
 /*   550 */    21,    3,  500,  499,  498,  497,  496,  495,  494,  493,
 /*   560 */   492,  491,    2,  330,  308,  419,  297,  127,   39,  633,
 /*   570 */   630,  172,  171,  631,  490,  100,  632,  637,  636,  152,
 /*   580 */   635,  634,   39,  362,   39,  232,   85,  514,  568,  201,
 /*   590 */   565,   98,  567,  381,    3,  524,  500,  499,  498,  497,
 /*   600 */   496,  495,  494,  493,  492,  491,    2,  545,  380,  379,
 /*   610 */   329,  127,  471,  466,  470,  473,  170,  469,  563,  370,
 /*   620 */   298,  472,  152,  553,  489,  128,  353,  376,  629,  630,
 /*   630 */   514,  378,  631,  547,  647,  632,  637,  636,    3,  635,
 /*   640 */   634,  375,   24,  374,  214,  554,  500,  499,  498,  497,
 /*   650 */   496,  495,  494,  493,  492,  491,    2,  483,  362,   14,
 /*   660 */   203,  127,  377,  124,   13,  425,  327,  351,  230,   53,
 /*   670 */    51,   54,   47,   49,   48,   52,   55,   46,   50,   14,
 /*   680 */   203,   57,   56,  229,   13,  369,  332,   60,    3,  500,
 /*   690 */   499,  498,  497,  496,  495,  494,  493,  492,  491,    2,
 /*   700 */   471,  465,  470,  473,  127,  469,   14,  203,  227,  472,
 /*   710 */   366,   13,   27,  187,  647,  224,  516,  137,  622,  364,
 /*   720 */    53,   51,   54,   47,   49,   48,   52,   55,   46,   50,
 /*   730 */   363,    3,   57,   56,  108,  104,  360,  105,   60,  519,
 /*   740 */   357,  220,    9,   20,  482,  481,  480,  606,  115,  219,
 /*   750 */   216,  211,   32,   18,  205,  642,  641,  640,  639,  116,
 /*   760 */   206,  347,    9,   20,  482,  481,  480,   80,  871,   25,
 /*   770 */   204,  342,  464,   97,   92,  642,  641,  640,  639,  116,
 /*   780 */   200,   95,  158,  196,  336,   93,  197,  457,  331,    9,
 /*   790 */    20,  482,  481,  480,  192,  191,  133,  430,  324,  188,
 /*   800 */   422,  186,  642,  641,  640,  639,  116,  323,  331,  321,
 /*   810 */    53,   51,   54,   47,   49,   48,   52,   55,   46,   50,
 /*   820 */   183,  319,   57,   56,  471,  463,  470,  473,   60,  469,
 /*   830 */   317,  420,  179,  472,  175,  331,  176,    8,  121,   58,
 /*   840 */   445,   53,   51,   54,   47,   49,   48,   52,   55,   46,
 /*   850 */    50,  195,  325,   57,   56,   11,  647,  648,  141,   60,
 /*   860 */    39,   53,   51,   54,   47,   49,   48,   52,   55,   46,
 /*   870 */    50,  404,   61,   57,   56,  627,   67,   68,  587,   60,
 /*   880 */    71,   69,   70,   72,   21,  578,   74,   73,  600,  579,
 /*   890 */    53,   51,   54,   47,   49,   48,   52,   55,   46,   50,
 /*   900 */   582,  241,   57,   56,  471,  462,  470,  473,   60,  469,
 /*   910 */    86,  239,  571,  472,   59,  235,   83,  583,  577,   53,
 /*   920 */    51,   54,   47,   49,   48,   52,   55,   46,   50,   37,
 /*   930 */   138,   57,   56,  110,  373,  566,  233,   60,  471,  461,
 /*   940 */   470,  473,  605,  469,  546,  139,  574,  472,   38,  537,
 /*   950 */    53,   51,   54,   47,   49,   48,   52,   55,   46,   50,
 /*   960 */   367,  534,   57,   56,   28,  114,  544,   81,   60,  359,
 /*   970 */   471,  460,  470,  473,  213,  469,  449,  543,  576,  472,
 /*   980 */    53,   51,   54,   47,   49,   48,   52,   55,   46,   50,
 /*   990 */   388,  604,   57,   56,  542,  607,  278,  602,   60,  253,
 /*  1000 */   601,   27,  599,  603,  541,  251,  598,   44,  151,  156,
 /*  1010 */   136,  540,  140,  536,   53,   51,   54,   47,   49,   48,
 /*  1020 */    52,   55,   46,   50,  134,  535,   57,   56,  532,  212,
 /*  1030 */   254,  517,   60,  207,   53,   51,   54,   47,   49,   48,
 /*  1040 */    52,   55,   46,   50,  512,  511,   57,   56,  510,  509,
 /*  1050 */   508,    4,   60,  450,   53,   51,   54,   47,   49,   48,
 /*  1060 */    52,   55,   46,   50,  504,  502,   57,   56,  501,   14,
 /*  1070 */   203,  303,   60,  244,   13,  471,  459,  470,  473,    1,
 /*  1080 */   469,  210,  456,   99,  472,  488,  484,  476,   96,    6,
 /*  1090 */   452,   53,   51,   54,   47,   49,   48,   52,   55,   46,
 /*  1100 */    50,  194,  442,   57,   56,  447,  252,  444,  193,   60,
 /*  1110 */   122,  443,  441,  440,   31,  190,   53,   51,   54,   47,
 /*  1120 */    49,   48,   52,   55,   46,   50,  604,  300,   57,   56,
 /*  1130 */   607,  431,  602,  326,   60,  601,  428,  599,  603,   17,
 /*  1140 */   427,  598,   44,  152,  155,  424,  426,  423,  421,  130,
 /*  1150 */   418,  514,  479,   20,  482,  481,  480,  180,  320,  416,
 /*  1160 */   415,  177,  414,   30,  152,  642,  641,  640,  639,  116,
 /*  1170 */   604,  410,  514,  525,  607,  149,  602,   88,  403,  601,
 /*  1180 */   408,  599,  603,  285,  551,  598,   44,  151,  156,  386,
 /*  1190 */   385,  552,  550,  372,  471,  199,  470,  473,  331,  469,
 /*  1200 */   371,  647,  538,  472,  503,  338,   22,  339,  244,  101,
 /*  1210 */   500,  499,  498,  497,  496,  495,  494,  493,  492,  491,
 /*  1220 */   513,  471,  125,  470,  473,  127,  469,  340,  341,   94,
 /*  1230 */   472,  500,  499,  498,  497,  496,  495,  494,  493,  492,
 /*  1240 */   491,  485,  135,  387,  604,  524,  127,  615,  607,  278,
 /*  1250 */   602,  597,  244,  601,  516,  599,  603,   62,  505,  598,
 /*  1260 */    44,  151,  156,  560,   53,   51,   54,   47,   49,   48,
 /*  1270 */    52,   55,   46,   50,  558,  314,   57,   56,   65,  604,
 /*  1280 */    64,  559,   60,  607,  149,  602,  515,  478,  601,  475,
 /*  1290 */   599,  603,  328,  985,  598,   44,  151,  156,  604,  305,
 /*  1300 */   358,  525,  607,  276,  602,  985,  985,  601,  362,  599,
 /*  1310 */   603,  307,  985,  598,   44,  151,  156,   34,  985,    7,
 /*  1320 */   985,  985,  985,  985,  985,  985,  244,  985,  985,  218,
 /*  1330 */   617,  985,  985,  985,  575,  630,  985,  313,  631,  616,
 /*  1340 */    33,  632,  637,  636,  574,  635,  634,  985,  246,  985,
 /*  1350 */   173,  628,  245,  243,  581,  580,  242,  240,  985,  985,
 /*  1360 */   985,  244,  985,  985,  506,  985,  985,  985,  126,  985,
 /*  1370 */   164,  985,  985,  524,  985,  985,  647,  209,  985,  985,
 /*  1380 */   244,  172,  171,  557,  604,  985,  985,  985,  607,  261,
 /*  1390 */   602,  985,  985,  601,  985,  599,  603,  985,  985,  598,
 /*  1400 */    44,  151,  156,  604,  985,  985,  525,  607,  584,  602,
 /*  1410 */   985,  985,  601,  985,  599,  603,  284,  985,  598,   44,
 /*  1420 */   151,  156,  604,  985,  985,  985,  607,  266,  602,  985,
 /*  1430 */   985,  601,  985,  599,  603,  985,  362,  598,   44,  151,
 /*  1440 */   156,  604,  985,  985,  985,  607,  265,  602,  985,  985,
 /*  1450 */   601,  985,  599,  603,  985,  985,  598,   44,  151,  156,
 /*  1460 */   985,  985,  985,  985,  985,  604,  244,  985,  985,  607,
 /*  1470 */   402,  602,  985,  985,  601,  985,  599,  603,  524,  985,
 /*  1480 */   598,   44,  151,  156,  604,  244,  985,  985,  607,  401,
 /*  1490 */   602,  985,  985,  601,  985,  599,  603,  985,  985,  598,
 /*  1500 */    44,  151,  156,  985,  244,  985,  985,  985,  985,  604,
 /*  1510 */   985,  985,  985,  607,  296,  602,  985,  985,  601,  985,
 /*  1520 */   599,  603,  985,  244,  598,   44,  151,  156,  604,  985,
 /*  1530 */   985,  985,  607,  295,  602,  985,  985,  601,  985,  599,
 /*  1540 */   603,  362,  985,  598,   44,  151,  156,  244,  985,  985,
 /*  1550 */   985,  985,  985,  985,  985,  985,  604,  985,  985,  985,
 /*  1560 */   607,  294,  602,  985,  985,  601,  244,  599,  603,  985,
 /*  1570 */   985,  598,   44,  151,  156,  985,  985,  985,  604,  985,
 /*  1580 */   985,  985,  607,  293,  602,  985,  985,  601,  985,  599,
 /*  1590 */   603,  244,  985,  598,   44,  151,  156,  604,  985,  985,
 /*  1600 */   985,  607,  292,  602,  985,  985,  601,  985,  599,  603,
 /*  1610 */   244,  985,  598,   44,  151,  156,  604,  985,  985,  985,
 /*  1620 */   607,  291,  602,  985,  985,  601,  985,  599,  603,  985,
 /*  1630 */   985,  598,   44,  151,  156,  985,  985,  604,  244,  985,
 /*  1640 */   985,  607,  290,  602,  985,  985,  601,  985,  599,  603,
 /*  1650 */   985,  985,  598,   44,  151,  156,  985,  985,  985,  985,
 /*  1660 */   244,  985,  985,  985,  985,  604,  985,  985,  985,  607,
 /*  1670 */   289,  602,  985,  985,  601,  985,  599,  603,  985,  244,
 /*  1680 */   598,   44,  151,  156,  604,  985,  985,  985,  607,  288,
 /*  1690 */   602,  985,  985,  601,  985,  599,  603,  985,  244,  598,
 /*  1700 */    44,  151,  156,  985,  985,  985,  604,  985,  985,  985,
 /*  1710 */   607,  287,  602,  985,  985,  601,  985,  599,  603,  244,
 /*  1720 */   985,  598,   44,  151,  156,  604,  985,  985,  985,  607,
 /*  1730 */   277,  602,  985,  985,  601,  985,  599,  603,  985,  985,
 /*  1740 */   598,   44,  151,  156,  604,  985,  985,  244,  607,  264,
 /*  1750 */   602,  985,  985,  601,  985,  599,  603,  985,  985,  598,
 /*  1760 */    44,  151,  156,  985,  985,  604,  244,  985,  985,  607,
 /*  1770 */   263,  602,  985,  985,  601,  985,  599,  603,  985,  985,
 /*  1780 */   598,   44,  151,  156,  985,  985,  985,  985,  244,  985,
 /*  1790 */   985,  985,  985,  604,  985,  985,  985,  607,  262,  602,
 /*  1800 */   985,  985,  601,  985,  599,  603,  985,  244,  598,   44,
 /*  1810 */   151,  156,  604,  985,  985,  985,  607,  275,  602,  985,
 /*  1820 */   985,  601,  985,  599,  603,  985,  244,  598,   44,  151,
 /*  1830 */   156,  985,  985,  985,  604,  985,  985,  985,  607,  274,
 /*  1840 */   602,  985,  985,  601,  985,  599,  603,  244,  985,  598,
 /*  1850 */    44,  151,  156,  604,  985,  985,  985,  607,  260,  602,
 /*  1860 */   985,  985,  601,  985,  599,  603,  985,  985,  598,   44,
 /*  1870 */   151,  156,  604,  985,  985,  244,  607,  259,  602,  985,
 /*  1880 */   985,  601,  985,  599,  603,  985,  985,  598,   44,  151,
 /*  1890 */   156,  985,  985,  604,  244,  985,  985,  607,  273,  602,
 /*  1900 */   985,  985,  601,  985,  599,  603,  985,  985,  598,   44,
 /*  1910 */   151,  156,  985,  985,  985,  985,  244,  985,  985,  985,
 /*  1920 */   985,  604,  985,  985,  985,  607,  148,  602,  985,  985,
 /*  1930 */   601,  985,  599,  603,  985,  244,  598,   44,  151,  156,
 /*  1940 */   604,  985,  985,  985,  607,  147,  602,  985,  985,  601,
 /*  1950 */   985,  599,  603,  985,  244,  598,   44,  151,  156,  985,
 /*  1960 */   985,  985,  604,  985,  985,  985,  607,  258,  602,  985,
 /*  1970 */   985,  601,  985,  599,  603,  244,  985,  598,   44,  151,
 /*  1980 */   156,  604,  985,  985,  985,  607,  257,  602,  985,  985,
 /*  1990 */   601,  985,  599,  603,  985,  985,  598,   44,  151,  156,
 /*  2000 */   604,  985,  985,  244,  607,  256,  602,  985,  985,  601,
 /*  2010 */   985,  599,  603,  985,  985,  598,   44,  151,  156,  985,
 /*  2020 */   985,  604,  244,  985,  985,  607,  146,  602,  985,  985,
 /*  2030 */   601,  985,  599,  603,  985,  985,  598,   44,  151,  156,
 /*  2040 */   985,  985,  985,  985,  244,  985,  985,  985,  985,  604,
 /*  2050 */   985,  985,  985,  607,  272,  602,  985,  985,  601,  985,
 /*  2060 */   599,  603,  985,  244,  598,   44,  151,  156,  604,  985,
 /*  2070 */   985,  985,  607,  255,  602,  985,  985,  601,  985,  599,
 /*  2080 */   603,  985,  244,  598,   44,  151,  156,  985,  985,  985,
 /*  2090 */   604,  985,  985,  985,  607,  271,  602,  985,  985,  601,
 /*  2100 */   985,  599,  603,  244,  985,  598,   44,  151,  156,  604,
 /*  2110 */   985,  985,  985,  607,  270,  602,  985,  985,  601,  985,
 /*  2120 */   599,  603,  985,  985,  598,   44,  151,  156,  604,  985,
 /*  2130 */   985,  244,  607,  269,  602,  985,  985,  601,  985,  599,
 /*  2140 */   603,  985,  985,  598,   44,  151,  156,  985,  985,  604,
 /*  2150 */   244,  985,  985,  607,  145,  602,  985,  985,  601,  985,
 /*  2160 */   599,  603,  985,  985,  598,   44,  151,  156,  985,  985,
 /*  2170 */   985,  985,  244,  985,  985,  985,  985,  604,  305,  358,
 /*  2180 */   985,  607,  267,  602,  985,  985,  601,  985,  599,  603,
 /*  2190 */   985,  244,  598,   44,  151,  156,   34,  985,    7,  985,
 /*  2200 */   985,  985,  985,  985,  985,  985,  985,  985,  218,  617,
 /*  2210 */   244,  604,  985,  985,  985,  607,  985,  602,  616,   33,
 /*  2220 */   601,  985,  599,  603,  985,  985,  598,   44,  279,  156,
 /*  2230 */   985,  244,  985,  985,  985,  985,  985,  985,  985,  985,
 /*  2240 */   985,  985,  985,  506,  985,  985,  604,  126,  985,  164,
 /*  2250 */   607,  985,  602,  985,  985,  601,  209,  599,  603,  244,
 /*  2260 */   985,  598,   44,  398,  156,  604,  985,  985,  985,  607,
 /*  2270 */   985,  602,  985,  985,  601,  985,  599,  603,  985,  985,
 /*  2280 */   598,   44,  397,  156,  985,  985,  604,  985,  985,  985,
 /*  2290 */   607,  985,  602,  244,  985,  601,  985,  599,  603,  985,
 /*  2300 */   985,  598,   44,  396,  156,  604,  985,  985,  985,  607,
 /*  2310 */   985,  602,  985,  985,  601,  985,  599,  603,  985,  985,
 /*  2320 */   598,   44,  395,  156,  985,  985,  604,  985,  244,  985,
 /*  2330 */   607,  985,  602,  985,  985,  601,  985,  599,  603,  985,
 /*  2340 */   985,  598,   44,  394,  156,  604,  985,  244,  985,  607,
 /*  2350 */   985,  602,  985,  985,  601,  985,  599,  603,  985,  985,
 /*  2360 */   598,   44,  393,  156,  985,  985,  985,  985,  244,  985,
 /*  2370 */   985,  985,  985,  985,  604,  985,  985,  985,  607,  985,
 /*  2380 */   602,  985,  985,  601,  985,  599,  603,  244,  985,  598,
 /*  2390 */    44,  283,  156,  985,  985,  573,  630,  985,  985,  631,
 /*  2400 */   985,  985,  632,  637,  636,  604,  635,  634,  244,  607,
 /*  2410 */   985,  602,  985,  985,  601,  985,  599,  603,  985,  985,
 /*  2420 */   598,   44,  282,  156,  985,  604,  985,  244,  985,  607,
 /*  2430 */   985,  602,  985,  985,  601,  985,  599,  603,  985,  985,
 /*  2440 */   598,   44,  144,  156,  985,  985,  985,  985,  604,  985,
 /*  2450 */   985,  985,  607,  985,  602,  985,  244,  601,  985,  599,
 /*  2460 */   603,  985,  985,  598,   44,  143,  156,  985,  985,  985,
 /*  2470 */   985,  985,  985,  604,  985,  985,  985,  607,  985,  602,
 /*  2480 */   985,  985,  601,  985,  599,  603,  985,  244,  598,   44,
 /*  2490 */   150,  156,  572,  630,  985,  985,  631,  985,  985,  632,
 /*  2500 */   637,  636,  604,  635,  634,  985,  607,  244,  602,  985,
 /*  2510 */   985,  601,  985,  599,  603,  985,  985,  598,   44,  280,
 /*  2520 */   156,  985,  985,  985,  985,  985,  985,  985,  985,  985,
 /*  2530 */   244,  985,  985,  604,  985,  985,  985,  607,  985,  602,
 /*  2540 */   985,  985,  601,  985,  599,  603,  985,  985,  598,   44,
 /*  2550 */   281,  156,  985,  604,  985,  244,  985,  607,  985,  602,
 /*  2560 */   985,  985,  601,  985,  599,  603,  985,  985,  598,   44,
 /*  2570 */   604,  154,  985,  985,  607,  985,  602,  985,  985,  601,
 /*  2580 */   985,  599,  603,  985,  244,  598,   44,  985,  153,  985,
 /*  2590 */   570,  630,  985,  985,  631,  985,  985,  632,  637,  636,
 /*  2600 */   985,  635,  634,  985,  985,  985,  985,  985,  985,  985,
 /*  2610 */   985,  985,  985,  985,  985,  244,  569,  630,  985,  985,
 /*  2620 */   631,  985,  985,  632,  637,  636,  985,  635,  634,  225,
 /*  2630 */   630,  985,  985,  631,  985,  244,  632,  637,  636,  985,
 /*  2640 */   635,  634,  985,  985,  985,  985,  345,  630,  985,  985,
 /*  2650 */   631,  985,  244,  632,  637,  636,  985,  635,  634,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    11,   12,   28,   11,   12,   31,   66,   15,   16,   17,
 /*    10 */    18,   80,   23,   21,   22,   26,   27,   28,   24,   25,
 /*    20 */    26,  123,  124,   34,  125,  127,   13,   14,   30,  131,
 /*    30 */   132,  133,   19,   39,  136,   95,  138,  139,   49,   33,
 /*    40 */   142,  143,  144,  145,   34,   56,   57,   19,   20,   34,
 /*    50 */    11,   12,   30,   31,   65,   40,   28,  235,  159,   70,
 /*    60 */   162,  239,   23,   74,   27,   26,   27,   33,   29,   80,
 /*    70 */   167,   34,  169,   34,  170,   65,   87,   88,   89,   90,
 /*    80 */    91,  241,  260,  261,  262,  263,  264,   72,   49,  100,
 /*    90 */   157,  251,  252,  253,  191,   56,   57,   19,   20,  129,
 /*   100 */    11,   12,   65,  179,   65,   19,   28,  209,  129,   70,
 /*   110 */   186,   74,   23,   74,  274,   26,   27,  193,   26,   80,
 /*   120 */    24,   25,   26,   34,  170,   33,   87,   88,   89,   90,
 /*   130 */    91,   94,  212,  213,  214,  215,   27,  217,   49,  100,
 /*   140 */    51,  221,  174,  164,  165,   56,   57,   24,   25,   26,
 /*   150 */    11,   12,   22,  183,   65,  212,  213,  214,  215,   70,
 /*   160 */   217,  182,   23,   74,  221,   26,   27,  136,  136,   80,
 /*   170 */   237,  267,  268,   34,  206,  207,   87,   88,   89,   90,
 /*   180 */    91,  212,  213,  214,  215,  241,  217,   27,   49,  100,
 /*   190 */   221,   19,   20,   19,   20,   56,   57,  253,   34,  229,
 /*   200 */    11,   12,   28,  136,   65,  212,  213,  214,  215,   70,
 /*   210 */   217,   28,   23,   74,  221,   26,   27,   62,  274,   80,
 /*   220 */   111,  267,  268,   34,   69,   58,   87,   88,   89,   90,
 /*   230 */    91,  113,  114,  115,   79,    0,  178,  179,   49,  100,
 /*   240 */   208,  128,   34,   27,  186,   56,   57,  134,  128,  136,
 /*   250 */    83,  193,   85,   98,   65,   29,  136,   31,  168,   70,
 /*   260 */    11,   12,  107,   74,   15,   16,   17,   18,   60,   80,
 /*   270 */    21,   22,  117,   65,  243,  244,   87,   88,   89,   90,
 /*   280 */    91,    1,    2,    3,   31,    5,    6,    7,    8,  100,
 /*   290 */    10,   11,   12,  173,  111,   15,   16,   17,   18,   50,
 /*   300 */    34,   21,   22,  271,   27,  128,   28,   30,  140,   31,
 /*   310 */   243,  244,   27,  136,   79,  195,  196,  197,  198,  199,
 /*   320 */   200,  201,  202,  203,  204,  205,   32,  111,   34,  171,
 /*   330 */   210,   65,   38,   34,   40,   41,   42,   43,   44,   45,
 /*   340 */    46,   47,   26,   28,  109,  255,   31,  171,  128,   33,
 /*   350 */   173,  211,  212,   30,   31,  215,  136,  237,  218,  219,
 /*   360 */   220,  136,  222,  223,  111,   71,   72,   73,   77,   78,
 /*   370 */   212,  146,  195,  196,  197,  198,  199,  200,  201,  202,
 /*   380 */   203,  204,  205,   15,   16,   17,   18,  210,  212,   21,
 /*   390 */    22,  212,  128,  173,  215,  216,   34,  218,  219,  220,
 /*   400 */   136,  222,  223,   26,  225,  237,  149,  150,  151,  152,
 /*   410 */    33,  154,   30,   31,  237,  195,  196,  197,  198,  199,
 /*   420 */   200,  201,  202,  203,  204,  205,   34,   65,   30,  250,
 /*   430 */   210,   33,  101,  208,   11,   12,   74,  173,   15,   16,
 /*   440 */    17,   18,  128,   30,   21,   22,   30,   31,  136,   79,
 /*   450 */   136,   26,   29,   30,  275,  160,  161,  237,   33,  195,
 /*   460 */   196,  197,  198,  199,  200,  201,  202,  203,  204,  205,
 /*   470 */    28,   11,   12,   31,  210,   15,   16,   17,   18,  109,
 /*   480 */   265,   21,   22,   28,  167,  128,   31,  173,  273,   29,
 /*   490 */   129,  234,   28,  136,   33,   31,  271,  240,  241,  242,
 /*   500 */    26,  237,  245,  246,  247,  248,  249,   33,  191,  195,
 /*   510 */   196,  197,  198,  199,  200,  201,  202,  203,  204,  205,
 /*   520 */   208,  276,  277,  136,  210,   28,  175,  176,   31,  128,
 /*   530 */   173,   30,   26,  146,   34,  278,  148,  136,   38,   33,
 /*   540 */    40,   41,   42,   43,   44,   45,   46,   47,  175,  176,
 /*   550 */    27,  237,  195,  196,  197,  198,  199,  200,  201,  202,
 /*   560 */   203,  204,  205,   63,  177,  257,  258,  210,   26,  211,
 /*   570 */   212,   71,   72,  215,  173,   33,  218,  219,  220,  128,
 /*   580 */   222,  223,   26,  271,   26,  180,   33,  136,   95,   33,
 /*   590 */   229,   33,   66,   25,  237,  208,  195,  196,  197,  198,
 /*   600 */   199,  200,  201,  202,  203,  204,  205,  212,   34,   24,
 /*   610 */   110,  210,  212,  213,  214,  215,  186,  217,   34,  224,
 /*   620 */   185,  221,  128,  193,  173,   30,   34,   25,  211,  212,
 /*   630 */   136,   34,  215,  228,  111,  218,  219,  220,  237,  222,
 /*   640 */   223,   34,   39,   24,  256,   34,  195,  196,  197,  198,
 /*   650 */   199,  200,  201,  202,  203,  204,  205,  135,  271,  149,
 /*   660 */   150,  210,   65,   30,  154,  230,  156,  173,   33,    1,
 /*   670 */     2,    3,    4,    5,    6,    7,    8,    9,   10,  149,
 /*   680 */   150,   13,   14,   33,  154,   36,  156,   19,  237,  195,
 /*   690 */   196,  197,  198,  199,  200,  201,  202,  203,  204,  205,
 /*   700 */   212,  213,  214,  215,  210,  217,  149,  150,   34,  221,
 /*   710 */    33,  154,  120,  156,  111,   61,  194,   27,   50,  115,
 /*   720 */     1,    2,    3,    4,    5,    6,    7,    8,    9,   10,
 /*   730 */    33,  237,   13,   14,   27,   27,   33,   27,   19,   34,
 /*   740 */    34,   37,  232,  233,  234,  235,  236,   28,   36,   55,
 /*   750 */    77,  104,   39,   30,   53,  245,  246,  247,  248,  249,
 /*   760 */   104,   34,  232,  233,  234,  235,  236,   30,  111,   39,
 /*   770 */    59,   30,   34,   33,   30,  245,  246,  247,  248,  249,
 /*   780 */    33,   33,   33,   97,   34,   33,   93,   34,  278,  232,
 /*   790 */   233,  234,  235,  236,  116,   33,   27,    1,   34,   68,
 /*   800 */    27,  106,  245,  246,  247,  248,  249,   36,  278,   84,
 /*   810 */     1,    2,    3,    4,    5,    6,    7,    8,    9,   10,
 /*   820 */    34,   82,   13,   14,  212,  213,  214,  215,   19,  217,
 /*   830 */    84,   34,   34,  221,   34,  278,  108,  238,  269,   30,
 /*   840 */   270,    1,    2,    3,    4,    5,    6,    7,    8,    9,
 /*   850 */    10,  155,  153,   13,   14,  239,  111,  237,  237,   19,
 /*   860 */    26,    1,    2,    3,    4,    5,    6,    7,    8,    9,
 /*   870 */    10,  227,   27,   13,   14,  141,   11,   12,  157,   19,
 /*   880 */    15,   16,   17,   18,   27,   96,   21,   22,   28,  189,
 /*   890 */     1,    2,    3,    4,    5,    6,    7,    8,    9,   10,
 /*   900 */   141,  141,   13,   14,  212,  213,  214,  215,   19,  217,
 /*   910 */   192,  141,   95,  221,   49,  137,  192,   28,  189,    1,
 /*   920 */     2,    3,    4,    5,    6,    7,    8,    9,   10,   39,
 /*   930 */    86,   13,   14,   95,   34,  237,  181,   19,  212,  213,
 /*   940 */   214,  215,  102,  217,  228,  184,   34,  221,   30,   66,
 /*   950 */     1,    2,    3,    4,    5,    6,    7,    8,    9,   10,
 /*   960 */   113,  174,   13,   14,  118,   58,  212,  190,   19,   62,
 /*   970 */   212,  213,  214,  215,  148,  217,   69,  212,   29,  221,
 /*   980 */     1,    2,    3,    4,    5,    6,    7,    8,    9,   10,
 /*   990 */   126,  127,   13,   14,  212,  131,  132,  133,   19,   92,
 /*  1000 */   136,  120,  138,  139,  212,   98,  142,  143,  144,  145,
 /*  1010 */   254,  237,   33,  237,    1,    2,    3,    4,    5,    6,
 /*  1020 */     7,    8,    9,   10,  117,  237,   13,   14,  212,  147,
 /*  1030 */   237,  237,   19,  147,    1,    2,    3,    4,    5,    6,
 /*  1040 */     7,    8,    9,   10,  237,  237,   13,   14,  237,  237,
 /*  1050 */   237,  237,   19,   34,    1,    2,    3,    4,    5,    6,
 /*  1060 */     7,    8,    9,   10,  237,  237,   13,   14,  237,  149,
 /*  1070 */   150,  170,   19,  209,  154,  212,  213,  214,  215,  237,
 /*  1080 */   217,   28,   34,  192,  221,  237,  237,  237,  192,   76,
 /*  1090 */   237,    1,    2,    3,    4,    5,    6,    7,    8,    9,
 /*  1100 */    10,  272,   34,   13,   14,  237,  237,  237,  168,   19,
 /*  1110 */    27,  237,  237,  172,   81,   27,    1,    2,    3,    4,
 /*  1120 */     5,    6,    7,    8,    9,   10,  127,  170,   13,   14,
 /*  1130 */   131,  237,  133,  175,   19,  136,  237,  138,  139,  119,
 /*  1140 */   237,  142,  143,  128,  145,  230,   34,  237,  237,   27,
 /*  1150 */   257,  136,  232,  233,  234,  235,  236,  259,   34,  237,
 /*  1160 */   237,  259,  237,   48,  128,  245,  246,  247,  248,  249,
 /*  1170 */   127,  237,  136,  136,  131,  132,  133,  188,  227,  136,
 /*  1180 */   237,  138,  139,  146,  237,  142,  143,  144,  145,  227,
 /*  1190 */   227,  193,  237,  227,  212,  213,  214,  215,  278,  217,
 /*  1200 */   227,  111,  129,  221,  237,  227,  163,  227,  209,  188,
 /*  1210 */   195,  196,  197,  198,  199,  200,  201,  202,  203,  204,
 /*  1220 */   205,  212,  213,  214,  215,  210,  217,  227,  227,  188,
 /*  1230 */   221,  195,  196,  197,  198,  199,  200,  201,  202,  203,
 /*  1240 */   204,  205,  237,  126,  127,  208,  210,  266,  131,  132,
 /*  1250 */   133,  194,  209,  136,  194,  138,  139,  226,  130,  142,
 /*  1260 */   143,  144,  145,  237,    1,    2,    3,    4,    5,    6,
 /*  1270 */     7,    8,    9,   10,  231,  158,   13,   14,  187,  127,
 /*  1280 */   187,  237,   19,  131,  132,  133,  237,  237,  136,   67,
 /*  1290 */   138,  139,   34,  279,  142,  143,  144,  145,  127,   34,
 /*  1300 */    35,  136,  131,  132,  133,  279,  279,  136,  271,  138,
 /*  1310 */   139,  146,  279,  142,  143,  144,  145,   52,  279,   54,
 /*  1320 */   279,  279,  279,  279,  279,  279,  209,  279,  279,   64,
 /*  1330 */    65,  279,  279,  279,  211,  212,  279,  166,  215,   74,
 /*  1340 */    75,  218,  219,  220,   34,  222,  223,  279,   38,  279,
 /*  1350 */    40,   41,   42,   43,   44,   45,   46,   47,  279,  279,
 /*  1360 */   279,  209,  279,  279,   99,  279,  279,  279,  103,  279,
 /*  1370 */   105,  279,  279,  208,  279,  279,  111,  112,  279,  279,
 /*  1380 */   209,   71,   72,  231,  127,  279,  279,  279,  131,  132,
 /*  1390 */   133,  279,  279,  136,  279,  138,  139,  279,  279,  142,
 /*  1400 */   143,  144,  145,  127,  279,  279,  136,  131,  132,  133,
 /*  1410 */   279,  279,  136,  279,  138,  139,  146,  279,  142,  143,
 /*  1420 */   144,  145,  127,  279,  279,  279,  131,  132,  133,  279,
 /*  1430 */   279,  136,  279,  138,  139,  279,  271,  142,  143,  144,
 /*  1440 */   145,  127,  279,  279,  279,  131,  132,  133,  279,  279,
 /*  1450 */   136,  279,  138,  139,  279,  279,  142,  143,  144,  145,
 /*  1460 */   279,  279,  279,  279,  279,  127,  209,  279,  279,  131,
 /*  1470 */   132,  133,  279,  279,  136,  279,  138,  139,  208,  279,
 /*  1480 */   142,  143,  144,  145,  127,  209,  279,  279,  131,  132,
 /*  1490 */   133,  279,  279,  136,  279,  138,  139,  279,  279,  142,
 /*  1500 */   143,  144,  145,  279,  209,  279,  279,  279,  279,  127,
 /*  1510 */   279,  279,  279,  131,  132,  133,  279,  279,  136,  279,
 /*  1520 */   138,  139,  279,  209,  142,  143,  144,  145,  127,  279,
 /*  1530 */   279,  279,  131,  132,  133,  279,  279,  136,  279,  138,
 /*  1540 */   139,  271,  279,  142,  143,  144,  145,  209,  279,  279,
 /*  1550 */   279,  279,  279,  279,  279,  279,  127,  279,  279,  279,
 /*  1560 */   131,  132,  133,  279,  279,  136,  209,  138,  139,  279,
 /*  1570 */   279,  142,  143,  144,  145,  279,  279,  279,  127,  279,
 /*  1580 */   279,  279,  131,  132,  133,  279,  279,  136,  279,  138,
 /*  1590 */   139,  209,  279,  142,  143,  144,  145,  127,  279,  279,
 /*  1600 */   279,  131,  132,  133,  279,  279,  136,  279,  138,  139,
 /*  1610 */   209,  279,  142,  143,  144,  145,  127,  279,  279,  279,
 /*  1620 */   131,  132,  133,  279,  279,  136,  279,  138,  139,  279,
 /*  1630 */   279,  142,  143,  144,  145,  279,  279,  127,  209,  279,
 /*  1640 */   279,  131,  132,  133,  279,  279,  136,  279,  138,  139,
 /*  1650 */   279,  279,  142,  143,  144,  145,  279,  279,  279,  279,
 /*  1660 */   209,  279,  279,  279,  279,  127,  279,  279,  279,  131,
 /*  1670 */   132,  133,  279,  279,  136,  279,  138,  139,  279,  209,
 /*  1680 */   142,  143,  144,  145,  127,  279,  279,  279,  131,  132,
 /*  1690 */   133,  279,  279,  136,  279,  138,  139,  279,  209,  142,
 /*  1700 */   143,  144,  145,  279,  279,  279,  127,  279,  279,  279,
 /*  1710 */   131,  132,  133,  279,  279,  136,  279,  138,  139,  209,
 /*  1720 */   279,  142,  143,  144,  145,  127,  279,  279,  279,  131,
 /*  1730 */   132,  133,  279,  279,  136,  279,  138,  139,  279,  279,
 /*  1740 */   142,  143,  144,  145,  127,  279,  279,  209,  131,  132,
 /*  1750 */   133,  279,  279,  136,  279,  138,  139,  279,  279,  142,
 /*  1760 */   143,  144,  145,  279,  279,  127,  209,  279,  279,  131,
 /*  1770 */   132,  133,  279,  279,  136,  279,  138,  139,  279,  279,
 /*  1780 */   142,  143,  144,  145,  279,  279,  279,  279,  209,  279,
 /*  1790 */   279,  279,  279,  127,  279,  279,  279,  131,  132,  133,
 /*  1800 */   279,  279,  136,  279,  138,  139,  279,  209,  142,  143,
 /*  1810 */   144,  145,  127,  279,  279,  279,  131,  132,  133,  279,
 /*  1820 */   279,  136,  279,  138,  139,  279,  209,  142,  143,  144,
 /*  1830 */   145,  279,  279,  279,  127,  279,  279,  279,  131,  132,
 /*  1840 */   133,  279,  279,  136,  279,  138,  139,  209,  279,  142,
 /*  1850 */   143,  144,  145,  127,  279,  279,  279,  131,  132,  133,
 /*  1860 */   279,  279,  136,  279,  138,  139,  279,  279,  142,  143,
 /*  1870 */   144,  145,  127,  279,  279,  209,  131,  132,  133,  279,
 /*  1880 */   279,  136,  279,  138,  139,  279,  279,  142,  143,  144,
 /*  1890 */   145,  279,  279,  127,  209,  279,  279,  131,  132,  133,
 /*  1900 */   279,  279,  136,  279,  138,  139,  279,  279,  142,  143,
 /*  1910 */   144,  145,  279,  279,  279,  279,  209,  279,  279,  279,
 /*  1920 */   279,  127,  279,  279,  279,  131,  132,  133,  279,  279,
 /*  1930 */   136,  279,  138,  139,  279,  209,  142,  143,  144,  145,
 /*  1940 */   127,  279,  279,  279,  131,  132,  133,  279,  279,  136,
 /*  1950 */   279,  138,  139,  279,  209,  142,  143,  144,  145,  279,
 /*  1960 */   279,  279,  127,  279,  279,  279,  131,  132,  133,  279,
 /*  1970 */   279,  136,  279,  138,  139,  209,  279,  142,  143,  144,
 /*  1980 */   145,  127,  279,  279,  279,  131,  132,  133,  279,  279,
 /*  1990 */   136,  279,  138,  139,  279,  279,  142,  143,  144,  145,
 /*  2000 */   127,  279,  279,  209,  131,  132,  133,  279,  279,  136,
 /*  2010 */   279,  138,  139,  279,  279,  142,  143,  144,  145,  279,
 /*  2020 */   279,  127,  209,  279,  279,  131,  132,  133,  279,  279,
 /*  2030 */   136,  279,  138,  139,  279,  279,  142,  143,  144,  145,
 /*  2040 */   279,  279,  279,  279,  209,  279,  279,  279,  279,  127,
 /*  2050 */   279,  279,  279,  131,  132,  133,  279,  279,  136,  279,
 /*  2060 */   138,  139,  279,  209,  142,  143,  144,  145,  127,  279,
 /*  2070 */   279,  279,  131,  132,  133,  279,  279,  136,  279,  138,
 /*  2080 */   139,  279,  209,  142,  143,  144,  145,  279,  279,  279,
 /*  2090 */   127,  279,  279,  279,  131,  132,  133,  279,  279,  136,
 /*  2100 */   279,  138,  139,  209,  279,  142,  143,  144,  145,  127,
 /*  2110 */   279,  279,  279,  131,  132,  133,  279,  279,  136,  279,
 /*  2120 */   138,  139,  279,  279,  142,  143,  144,  145,  127,  279,
 /*  2130 */   279,  209,  131,  132,  133,  279,  279,  136,  279,  138,
 /*  2140 */   139,  279,  279,  142,  143,  144,  145,  279,  279,  127,
 /*  2150 */   209,  279,  279,  131,  132,  133,  279,  279,  136,  279,
 /*  2160 */   138,  139,  279,  279,  142,  143,  144,  145,  279,  279,
 /*  2170 */   279,  279,  209,  279,  279,  279,  279,  127,   34,   35,
 /*  2180 */   279,  131,  132,  133,  279,  279,  136,  279,  138,  139,
 /*  2190 */   279,  209,  142,  143,  144,  145,   52,  279,   54,  279,
 /*  2200 */   279,  279,  279,  279,  279,  279,  279,  279,   64,   65,
 /*  2210 */   209,  127,  279,  279,  279,  131,  279,  133,   74,   75,
 /*  2220 */   136,  279,  138,  139,  279,  279,  142,  143,  144,  145,
 /*  2230 */   279,  209,  279,  279,  279,  279,  279,  279,  279,  279,
 /*  2240 */   279,  279,  279,   99,  279,  279,  127,  103,  279,  105,
 /*  2250 */   131,  279,  133,  279,  279,  136,  112,  138,  139,  209,
 /*  2260 */   279,  142,  143,  144,  145,  127,  279,  279,  279,  131,
 /*  2270 */   279,  133,  279,  279,  136,  279,  138,  139,  279,  279,
 /*  2280 */   142,  143,  144,  145,  279,  279,  127,  279,  279,  279,
 /*  2290 */   131,  279,  133,  209,  279,  136,  279,  138,  139,  279,
 /*  2300 */   279,  142,  143,  144,  145,  127,  279,  279,  279,  131,
 /*  2310 */   279,  133,  279,  279,  136,  279,  138,  139,  279,  279,
 /*  2320 */   142,  143,  144,  145,  279,  279,  127,  279,  209,  279,
 /*  2330 */   131,  279,  133,  279,  279,  136,  279,  138,  139,  279,
 /*  2340 */   279,  142,  143,  144,  145,  127,  279,  209,  279,  131,
 /*  2350 */   279,  133,  279,  279,  136,  279,  138,  139,  279,  279,
 /*  2360 */   142,  143,  144,  145,  279,  279,  279,  279,  209,  279,
 /*  2370 */   279,  279,  279,  279,  127,  279,  279,  279,  131,  279,
 /*  2380 */   133,  279,  279,  136,  279,  138,  139,  209,  279,  142,
 /*  2390 */   143,  144,  145,  279,  279,  211,  212,  279,  279,  215,
 /*  2400 */   279,  279,  218,  219,  220,  127,  222,  223,  209,  131,
 /*  2410 */   279,  133,  279,  279,  136,  279,  138,  139,  279,  279,
 /*  2420 */   142,  143,  144,  145,  279,  127,  279,  209,  279,  131,
 /*  2430 */   279,  133,  279,  279,  136,  279,  138,  139,  279,  279,
 /*  2440 */   142,  143,  144,  145,  279,  279,  279,  279,  127,  279,
 /*  2450 */   279,  279,  131,  279,  133,  279,  209,  136,  279,  138,
 /*  2460 */   139,  279,  279,  142,  143,  144,  145,  279,  279,  279,
 /*  2470 */   279,  279,  279,  127,  279,  279,  279,  131,  279,  133,
 /*  2480 */   279,  279,  136,  279,  138,  139,  279,  209,  142,  143,
 /*  2490 */   144,  145,  211,  212,  279,  279,  215,  279,  279,  218,
 /*  2500 */   219,  220,  127,  222,  223,  279,  131,  209,  133,  279,
 /*  2510 */   279,  136,  279,  138,  139,  279,  279,  142,  143,  144,
 /*  2520 */   145,  279,  279,  279,  279,  279,  279,  279,  279,  279,
 /*  2530 */   209,  279,  279,  127,  279,  279,  279,  131,  279,  133,
 /*  2540 */   279,  279,  136,  279,  138,  139,  279,  279,  142,  143,
 /*  2550 */   144,  145,  279,  127,  279,  209,  279,  131,  279,  133,
 /*  2560 */   279,  279,  136,  279,  138,  139,  279,  279,  142,  143,
 /*  2570 */   127,  145,  279,  279,  131,  279,  133,  279,  279,  136,
 /*  2580 */   279,  138,  139,  279,  209,  142,  143,  279,  145,  279,
 /*  2590 */   211,  212,  279,  279,  215,  279,  279,  218,  219,  220,
 /*  2600 */   279,  222,  223,  279,  279,  279,  279,  279,  279,  279,
 /*  2610 */   279,  279,  279,  279,  279,  209,  211,  212,  279,  279,
 /*  2620 */   215,  279,  279,  218,  219,  220,  279,  222,  223,  211,
 /*  2630 */   212,  279,  279,  215,  279,  209,  218,  219,  220,  279,
 /*  2640 */   222,  223,  279,  279,  279,  279,  211,  212,  279,  279,
 /*  2650 */   215,  279,  209,  218,  219,  220,  279,  222,  223,
};
#define YY_SHIFT_USE_DFLT (-70)
#define YY_SHIFT_COUNT (406)
#define YY_SHIFT_MIN   (-69)
#define YY_SHIFT_MAX   (2144)
static const short yy_shift_ofst[] = {
 /*     0 */   370, 1265, 1265, 1265, 1265, 1265, 1265, 1265, 1265, 1265,
 /*    10 */    89,  155,  907,  907,  907,  155,   39,  189, 2144, 2144,
 /*    20 */   907,  -11,  189,  139,  139,  139,  139,  139,  139,  139,
 /*    30 */   139,  139,  139,  139,  139,  139,  139,  139,  139,  139,
 /*    40 */   139,  139,  139,  139,  139,  139,  139,  139,  139,  139,
 /*    50 */   139,  139,  139,  139,  139,  139,  139,  139,  139,  139,
 /*    60 */   139,  139,  139,  139,  139,  139,  139,  139,  139,  139,
 /*    70 */   139,  139,  139,  139,  139,  139,  500,  139,  139,  139,
 /*    80 */  1310, 1310, 1310, 1310, 1310, 1310, 1310, 1310, 1310, 1310,
 /*    90 */   167,  294,  294,  294,  294,  294,  294,  294,  294,  294,
 /*   100 */   294,  294,  294,  294,   37,   37,   37,   37,   37,  208,
 /*   110 */   597,   37,   37,  362,  362,  362,  118,  235,  597,  266,
 /*   120 */  1048, 1048, 1222,  123,   15,  603,  592,  523,   10,  266,
 /*   130 */  1124, 1112, 1020,  912, 1258, 1222, 1083,  912,  900, 1020,
 /*   140 */   -70,  -70,  -70,  280,  280, 1090, 1115, 1090, 1090, 1090,
 /*   150 */   249,  865,   -6,   96,   96,   96,   96,  183,  -60,  558,
 /*   160 */   556,  542,  -60,  506,  216,  266,  474,  425,  253,   10,
 /*   170 */   253,  377,  316,   92,  -60,  745,  745,  745, 1122,  745,
 /*   180 */   745, 1124, 1122,  745,  745, 1112,  745, 1020,  745,  745,
 /*   190 */  1048, 1088,  745,  745, 1083, 1068,  745,  745,  745,  745,
 /*   200 */   817,  817, 1048, 1019,  745,  745,  745,  745,  846,  745,
 /*   210 */   745,  745,  745,  846,  881,  745,  745,  745,  745,  745,
 /*   220 */   745,  745,  912,  847,  745,  745,  883,  745,  912,  912,
 /*   230 */   912,  912,  900,  838,  844,  745,  890,  817,  817,  789,
 /*   240 */   845,  789,  845,  845,  857,  845,  834,  745,  745,  -70,
 /*   250 */   -70,  -70,  -70,  -70,  -70, 1053, 1033, 1013,  979,  949,
 /*   260 */   918,  889,  860,  840,  719,  668,  809, 1263, 1263, 1263,
 /*   270 */  1263, 1263, 1263, 1263, 1263, 1263, 1263, 1263, 1263,  423,
 /*   280 */   460,   -8,  368,  368,  174,   78,   28,   13,   13,   13,
 /*   290 */    13,   13,   13,   13,   13,   13,   13,  497,  464,  455,
 /*   300 */   442,  416,  398,  382,  291,  109,  323,  172,  315,  172,
 /*   310 */   278,   22,  277,  -26,  226,  800,  728,  798,  746,  797,
 /*   320 */   739,  786,  725,  773,  771,  764,  695,  731,  796,  769,
 /*   330 */   762,  678,  686,  693,  744,  753,  752,  750,  749,  748,
 /*   340 */   747,  740,  738,  741,  711,  730,  737,  657,  727,  701,
 /*   350 */   723,  656,  647,  713,  673,  694,  704,  712,  706,  705,
 /*   360 */   710,  703,  708,  707,  697,  604,  690,  677,  654,  674,
 /*   370 */   649,  650,  635,  633,  611,  619,  607,  602,  595,  584,
 /*   380 */   585,  574,  568,  526,  493,  553,  461,  501,  413,  331,
 /*   390 */   392,  285,  160,  130,  130,  130,  130,  130,  130,  299,
 /*   400 */   164,   86,   86,   34,    6,   -2,  -69,
};
#define YY_REDUCE_USE_DFLT (-179)
#define YY_REDUCE_COUNT (254)
#define YY_REDUCE_MIN   (-178)
#define YY_REDUCE_MAX   (2443)
static const short yy_reduce_ofst[] = {
 /*     0 */  -160,  494,  451,  401,  357,  314,  264,  220,  177,  120,
 /*    10 */  -102,  257,  557,  530,  510,  257, 1117, 1043, 1036, 1015,
 /*    20 */   920, 1171, 1152,  864, 2050, 2022, 2001, 1982, 1963, 1941,
 /*    30 */  1922, 1894, 1873, 1854, 1835, 1813, 1794, 1766, 1745, 1726,
 /*    40 */  1707, 1685, 1666, 1638, 1617, 1598, 1579, 1557, 1538, 1510,
 /*    50 */  1489, 1470, 1451, 1429, 1401, 1382, 1357, 1338, 1314, 1295,
 /*    60 */  1276, 1257, 2406, 2375, 2346, 2321, 2298, 2278, 2247, 2218,
 /*    70 */  2199, 2178, 2159, 2138, 2119, 2084,  179, 2443, 2426,  999,
 /*    80 */  2435, 2418, 2405, 2379, 2281, 2184, 1123,  417,  358,  140,
 /*    90 */  -178, 1009,  982,  863,  758,  726,  692,  612,  488,  400,
 /*   100 */    -7,  -31,  -57,  -80,  387, 1270, 1165, 1037,  225,  -21,
 /*   110 */    58,  312,   32,   67,   31,  113,  -32,  -56,  -76,  -30,
 /*   120 */   -46,  -96,  -97,  522,  395,  168,  388,  -67,  430,  361,
 /*   130 */   308,  435,  373,  176,  245,  317,   90,  158,  405,  351,
 /*   140 */   295, -101,  215, 1093, 1091, 1050, 1128, 1049, 1044, 1026,
 /*   150 */   981, 1031, 1060, 1057, 1057, 1057, 1057, 1005, 1041, 1001,
 /*   160 */  1000,  980, 1021,  978,  967, 1073,  973,  966,  955,  998,
 /*   170 */   947,  963,  962,  951,  989,  943,  934,  925,  902,  923,
 /*   180 */   922,  893,  898,  911,  910,  915,  903,  958,  899,  894,
 /*   190 */   957,  941,  875,  874,  940,  829,  870,  869,  868,  853,
 /*   200 */   896,  891,  901,  756,  850,  849,  848,  842,  886,  831,
 /*   210 */   828,  827,  814,  882,  826,  813,  812,  811,  808,  807,
 /*   220 */   794,  793,  816,  787,  788,  776,  777,  774,  792,  782,
 /*   230 */   765,  754,  716,  761,  755,  698,  778,  724,  718,  729,
 /*   240 */   770,  700,  760,  759,  721,  734,  644,  621,  620,  616,
 /*   250 */   699,  696,  570,  569,  599,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   983,  918,  918,  918,  918,  918,  918,  918,  918,  918,
 /*    10 */   705,  899,  654,  654,  654,  898,  983,  983,  983,  983,
 /*    20 */   654,  983,  978,  983,  983,  983,  983,  983,  983,  983,
 /*    30 */   983,  983,  983,  983,  983,  983,  983,  983,  983,  983,
 /*    40 */   983,  983,  983,  983,  983,  983,  983,  983,  983,  983,
 /*    50 */   983,  983,  983,  983,  983,  983,  983,  983,  983,  983,
 /*    60 */   983,  983,  983,  983,  983,  983,  983,  983,  983,  983,
 /*    70 */   983,  983,  983,  983,  983,  983,  983,  983,  983,  983,
 /*    80 */   983,  983,  983,  983,  983,  983,  983,  983,  983,  983,
 /*    90 */   691,  983,  983,  983,  983,  983,  983,  983,  983,  983,
 /*   100 */   983,  983,  983,  983,  983,  983,  983,  983,  983,  719,
 /*   110 */   983,  983,  983,  712,  712,  983,  921,  983,  971,  983,
 /*   120 */   844,  844,  765,  948,  983,  983,  981,  983,  983,  720,
 /*   130 */   983,  983,  979,  983,  983,  765,  768,  983,  983,  979,
 /*   140 */   700,  680,  818,  983,  983,  983,  696,  983,  983,  983,
 /*   150 */   983,  739,  983,  959,  958,  957,  754,  983,  854,  983,
 /*   160 */   983,  983,  854,  983,  983,  983,  983,  983,  983,  983,
 /*   170 */   983,  983,  983,  983,  854,  983,  983,  983,  983,  815,
 /*   180 */   983,  983,  983,  812,  983,  983,  983,  983,  983,  983,
 /*   190 */   983,  983,  983,  983,  768,  983,  983,  983,  983,  983,
 /*   200 */   960,  960,  983,  983,  983,  983,  983,  983,  972,  983,
 /*   210 */   983,  983,  983,  972,  981,  983,  983,  983,  983,  983,
 /*   220 */   983,  983,  983,  922,  983,  983,  733,  983,  983,  983,
 /*   230 */   983,  983,  830,  970,  829,  983,  983,  960,  960,  859,
 /*   240 */   861,  859,  861,  861,  983,  861,  983,  983,  983,  691,
 /*   250 */   897,  868,  848,  847,  672,  983,  983,  983,  983,  983,
 /*   260 */   983,  983,  983,  983,  983,  983,  983,  841,  703,  704,
 /*   270 */   982,  973,  697,  804,  662,  664,  763,  764,  660,  983,
 /*   280 */   983,  753,  762,  761,  983,  983,  983,  752,  751,  750,
 /*   290 */   749,  748,  747,  746,  745,  744,  743,  983,  983,  983,
 /*   300 */   983,  983,  983,  983,  983,  799,  983,  933,  983,  932,
 /*   310 */   983,  983,  799,  983,  983,  983,  983,  983,  983,  983,
 /*   320 */   805,  983,  983,  983,  983,  983,  983,  983,  983,  983,
 /*   330 */   983,  983,  983,  983,  983,  983,  983,  983,  983,  983,
 /*   340 */   983,  983,  983,  793,  983,  983,  983,  873,  983,  983,
 /*   350 */   983,  983,  983,  983,  983,  983,  983,  983,  983,  983,
 /*   360 */   983,  983,  983,  983,  926,  983,  983,  983,  983,  983,
 /*   370 */   983,  983,  983,  983,  983,  983,  983,  983,  962,  983,
 /*   380 */   983,  983,  983,  856,  855,  983,  983,  661,  663,  983,
 /*   390 */   983,  983,  799,  760,  759,  758,  757,  756,  755,  983,
 /*   400 */   983,  742,  741,  983,  983,  983,  983,  737,  902,  901,
 /*   410 */   900,  819,  817,  816,  814,  813,  811,  809,  808,  807,
 /*   420 */   806,  810,  896,  895,  894,  893,  892,  891,  777,  946,
 /*   430 */   944,  943,  942,  941,  940,  939,  938,  937,  903,  852,
 /*   440 */   727,  945,  867,  866,  865,  846,  845,  843,  842,  779,
 /*   450 */   780,  781,  778,  770,  771,  769,  795,  796,  767,  666,
 /*   460 */   786,  788,  790,  792,  794,  791,  789,  787,  785,  784,
 /*   470 */   775,  774,  773,  772,  665,  766,  714,  713,  711,  655,
 /*   480 */   653,  652,  651,  947,  707,  706,  702,  701,  887,  920,
 /*   490 */   919,  917,  916,  915,  914,  913,  912,  911,  910,  909,
 /*   500 */   908,  907,  889,  888,  886,  803,  870,  864,  863,  801,
 /*   510 */   800,  728,  708,  699,  675,  676,  674,  671,  650,  726,
 /*   520 */   927,  935,  936,  931,  929,  934,  930,  928,  853,  799,
 /*   530 */   923,  925,  851,  850,  924,  725,  735,  734,  732,  731,
 /*   540 */   828,  825,  824,  823,  822,  821,  827,  826,  969,  968,
 /*   550 */   966,  967,  965,  964,  963,  962,  980,  977,  976,  975,
 /*   560 */   974,  724,  722,  730,  729,  723,  721,  858,  857,  683,
 /*   570 */   833,  961,  906,  905,  849,  832,  831,  690,  860,  689,
 /*   580 */   688,  687,  686,  862,  740,  875,  874,  776,  657,  885,
 /*   590 */   884,  883,  882,  881,  880,  879,  878,  950,  956,  955,
 /*   600 */   954,  953,  952,  951,  949,  877,  876,  840,  839,  838,
 /*   610 */   837,  836,  835,  834,  890,  820,  798,  797,  783,  656,
 /*   620 */   873,  872,  698,  710,  709,  659,  658,  685,  684,  682,
 /*   630 */   679,  678,  677,  673,  670,  669,  668,  667,  681,  718,
 /*   640 */   717,  716,  715,  695,  694,  693,  692,  904,  802,  738,
};

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  ParseARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void ParseTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "TOK_EQUAL",     "TOK_GREATER_EQUAL",  "TOK_GREATER_THAN",
  "TOK_IN",        "TOK_INST_EQUAL",  "TOK_INST_NOT_EQUAL",  "TOK_LESS_EQUAL",
  "TOK_LESS_THAN",  "TOK_LIKE",      "TOK_NOT_EQUAL",  "TOK_MINUS",   
  "TOK_PLUS",      "TOK_OR",        "TOK_XOR",       "TOK_DIV",     
  "TOK_MOD",       "TOK_REAL_DIV",  "TOK_TIMES",     "TOK_AND",     
  "TOK_ANDOR",     "TOK_CONCAT_OP",  "TOK_EXP",       "TOK_NOT",     
  "TOK_DOT",       "TOK_BACKSLASH",  "TOK_LEFT_BRACKET",  "TOK_LEFT_PAREN",
  "TOK_RIGHT_PAREN",  "TOK_RIGHT_BRACKET",  "TOK_COLON",     "TOK_COMMA",   
  "TOK_AGGREGATE",  "TOK_OF",        "TOK_IDENTIFIER",  "TOK_ALIAS",   
  "TOK_FOR",       "TOK_END_ALIAS",  "TOK_ARRAY",     "TOK_ASSIGNMENT",
  "TOK_BAG",       "TOK_BOOLEAN",   "TOK_INTEGER",   "TOK_REAL",    
  "TOK_NUMBER",    "TOK_LOGICAL",   "TOK_BINARY",    "TOK_STRING",  
  "TOK_BY",        "TOK_LEFT_CURL",  "TOK_RIGHT_CURL",  "TOK_OTHERWISE",
  "TOK_CASE",      "TOK_END_CASE",  "TOK_BEGIN",     "TOK_END",     
  "TOK_PI",        "TOK_E",         "TOK_CONSTANT",  "TOK_END_CONSTANT",
  "TOK_DERIVE",    "TOK_END_ENTITY",  "TOK_ENTITY",    "TOK_ENUMERATION",
  "TOK_ESCAPE",    "TOK_SELF",      "TOK_OPTIONAL",  "TOK_VAR",     
  "TOK_END_FUNCTION",  "TOK_FUNCTION",  "TOK_BUILTIN_FUNCTION",  "TOK_LIST",    
  "TOK_SET",       "TOK_GENERIC",   "TOK_QUESTION_MARK",  "TOK_IF",      
  "TOK_THEN",      "TOK_END_IF",    "TOK_ELSE",      "TOK_INCLUDE", 
  "TOK_STRING_LITERAL",  "TOK_TO",        "TOK_AS",        "TOK_REFERENCE",
  "TOK_FROM",      "TOK_USE",       "TOK_INVERSE",   "TOK_INTEGER_LITERAL",
  "TOK_REAL_LITERAL",  "TOK_STRING_LITERAL_ENCODED",  "TOK_LOGICAL_LITERAL",  "TOK_BINARY_LITERAL",
  "TOK_LOCAL",     "TOK_END_LOCAL",  "TOK_ONEOF",     "TOK_UNIQUE",  
  "TOK_FIXED",     "TOK_END_PROCEDURE",  "TOK_PROCEDURE",  "TOK_BUILTIN_PROCEDURE",
  "TOK_QUERY",     "TOK_ALL_IN",    "TOK_SUCH_THAT",  "TOK_REPEAT",  
  "TOK_END_REPEAT",  "TOK_RETURN",    "TOK_END_RULE",  "TOK_RULE",    
  "TOK_END_SCHEMA",  "TOK_SCHEMA",    "TOK_SELECT",    "TOK_SEMICOLON",
  "TOK_SKIP",      "TOK_SUBTYPE",   "TOK_ABSTRACT",  "TOK_SUPERTYPE",
  "TOK_END_TYPE",  "TOK_TYPE",      "TOK_UNTIL",     "TOK_WHERE",   
  "TOK_WHILE",     "error",         "statement_list",  "case_action", 
  "case_otherwise",  "entity_body",   "aggregate_init_element",  "aggregate_initializer",
  "assignable",    "attribute_decl",  "by_expression",  "constant",    
  "expression",    "function_call",  "general_ref",   "group_ref",   
  "identifier",    "initializer",   "interval",      "literal",     
  "local_initializer",  "precision_spec",  "query_expression",  "query_start", 
  "simple_expression",  "unary_expression",  "supertype_expression",  "until_control",
  "while_control",  "function_header",  "fh_lineno",     "rule_header", 
  "rh_start",      "rh_get_line",   "procedure_header",  "ph_get_line", 
  "action_body",   "actual_parameters",  "aggregate_init_body",  "explicit_attr_list",
  "case_action_list",  "case_block",    "case_labels",   "where_clause_list",
  "derive_decl",   "explicit_attribute",  "expression_list",  "formal_parameter",
  "formal_parameter_list",  "formal_parameter_rep",  "id_list",       "defined_type_list",
  "nested_id_list",  "statement_rep",  "subtype_decl",  "where_rule",  
  "where_rule_OPT",  "supertype_expression_list",  "labelled_attrib_list_list",  "labelled_attrib_list",
  "inverse_attr_list",  "inverse_clause",  "attribute_decl_list",  "derived_attribute_rep",
  "unique_clause",  "rule_formal_parameter_list",  "qualified_attr_list",  "rel_op",      
  "optional_or_unique",  "optional_fixed",  "optional",      "var",         
  "unique",        "qualified_attr",  "qualifier",     "alias_statement",
  "assignment_statement",  "case_statement",  "compound_statement",  "escape_statement",
  "if_statement",  "proc_call_statement",  "repeat_statement",  "return_statement",
  "skip_statement",  "statement",     "subsuper_decl",  "supertype_decl",
  "supertype_factor",  "function_id",   "procedure_id",  "attribute_type",
  "defined_type",  "parameter_type",  "generic_type",  "basic_type",  
  "select_type",   "aggregate_type",  "aggregation_type",  "array_type",  
  "bag_type",      "conformant_aggregation",  "list_type",     "set_type",    
  "set_or_bag_of_entity",  "type",          "cardinality_op",  "bound_spec",  
  "inverse_attr",  "derived_attribute",  "rule_formal_parameter",  "where_clause",
  "action_body_item_rep",  "action_body_item",  "declaration",   "constant_decl",
  "local_decl",    "semicolon",     "alias_push_scope",  "block_list",  
  "block_member",  "include_directive",  "rule_decl",     "constant_body",
  "constant_body_list",  "entity_decl",   "function_decl",  "procedure_decl",
  "type_decl",     "entity_header",  "enumeration_type",  "express_file",
  "schema_decl_list",  "schema_decl",   "fh_push_scope",  "fh_plist",    
  "increment_control",  "rename",        "rename_list",   "parened_rename_list",
  "reference_clause",  "reference_head",  "use_clause",    "use_head",    
  "interface_specification",  "interface_specification_list",  "right_curl",    "local_variable",
  "local_body",    "allow_generic_types",  "disallow_generic_types",  "oneof_op",    
  "ph_push_scope",  "schema_body",   "schema_header",  "type_item_body",
  "type_item",     "ti_start",      "td_start",    
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "action_body ::= action_body_item_rep statement_rep",
 /*   1 */ "action_body_item ::= declaration",
 /*   2 */ "action_body_item ::= constant_decl",
 /*   3 */ "action_body_item ::= local_decl",
 /*   4 */ "action_body_item_rep ::=",
 /*   5 */ "action_body_item_rep ::= action_body_item action_body_item_rep",
 /*   6 */ "actual_parameters ::= TOK_LEFT_PAREN expression_list TOK_RIGHT_PAREN",
 /*   7 */ "actual_parameters ::= TOK_LEFT_PAREN TOK_RIGHT_PAREN",
 /*   8 */ "aggregate_initializer ::= TOK_LEFT_BRACKET TOK_RIGHT_BRACKET",
 /*   9 */ "aggregate_initializer ::= TOK_LEFT_BRACKET aggregate_init_body TOK_RIGHT_BRACKET",
 /*  10 */ "aggregate_init_element ::= expression",
 /*  11 */ "aggregate_init_body ::= aggregate_init_element",
 /*  12 */ "aggregate_init_body ::= aggregate_init_element TOK_COLON expression",
 /*  13 */ "aggregate_init_body ::= aggregate_init_body TOK_COMMA aggregate_init_element",
 /*  14 */ "aggregate_init_body ::= aggregate_init_body TOK_COMMA aggregate_init_element TOK_COLON expression",
 /*  15 */ "aggregate_type ::= TOK_AGGREGATE TOK_OF parameter_type",
 /*  16 */ "aggregate_type ::= TOK_AGGREGATE TOK_COLON TOK_IDENTIFIER TOK_OF parameter_type",
 /*  17 */ "aggregation_type ::= array_type",
 /*  18 */ "aggregation_type ::= bag_type",
 /*  19 */ "aggregation_type ::= list_type",
 /*  20 */ "aggregation_type ::= set_type",
 /*  21 */ "alias_statement ::= TOK_ALIAS TOK_IDENTIFIER TOK_FOR general_ref semicolon alias_push_scope statement_rep TOK_END_ALIAS semicolon",
 /*  22 */ "alias_push_scope ::=",
 /*  23 */ "array_type ::= TOK_ARRAY bound_spec TOK_OF optional_or_unique attribute_type",
 /*  24 */ "assignable ::= assignable qualifier",
 /*  25 */ "assignable ::= identifier",
 /*  26 */ "assignment_statement ::= assignable TOK_ASSIGNMENT expression semicolon",
 /*  27 */ "attribute_type ::= aggregation_type",
 /*  28 */ "attribute_type ::= basic_type",
 /*  29 */ "attribute_type ::= defined_type",
 /*  30 */ "explicit_attr_list ::=",
 /*  31 */ "explicit_attr_list ::= explicit_attr_list explicit_attribute",
 /*  32 */ "bag_type ::= TOK_BAG bound_spec TOK_OF attribute_type",
 /*  33 */ "bag_type ::= TOK_BAG TOK_OF attribute_type",
 /*  34 */ "basic_type ::= TOK_BOOLEAN",
 /*  35 */ "basic_type ::= TOK_INTEGER precision_spec",
 /*  36 */ "basic_type ::= TOK_REAL precision_spec",
 /*  37 */ "basic_type ::= TOK_NUMBER",
 /*  38 */ "basic_type ::= TOK_LOGICAL",
 /*  39 */ "basic_type ::= TOK_BINARY precision_spec optional_fixed",
 /*  40 */ "basic_type ::= TOK_STRING precision_spec optional_fixed",
 /*  41 */ "block_list ::=",
 /*  42 */ "block_list ::= block_list block_member",
 /*  43 */ "block_member ::= declaration",
 /*  44 */ "block_member ::= include_directive",
 /*  45 */ "block_member ::= rule_decl",
 /*  46 */ "by_expression ::=",
 /*  47 */ "by_expression ::= TOK_BY expression",
 /*  48 */ "cardinality_op ::= TOK_LEFT_CURL expression TOK_COLON expression TOK_RIGHT_CURL",
 /*  49 */ "case_action ::= case_labels TOK_COLON statement",
 /*  50 */ "case_action_list ::=",
 /*  51 */ "case_action_list ::= case_action_list case_action",
 /*  52 */ "case_block ::= case_action_list case_otherwise",
 /*  53 */ "case_labels ::= expression",
 /*  54 */ "case_labels ::= case_labels TOK_COMMA expression",
 /*  55 */ "case_otherwise ::=",
 /*  56 */ "case_otherwise ::= TOK_OTHERWISE TOK_COLON statement",
 /*  57 */ "case_statement ::= TOK_CASE expression TOK_OF case_block TOK_END_CASE semicolon",
 /*  58 */ "compound_statement ::= TOK_BEGIN statement_rep TOK_END semicolon",
 /*  59 */ "constant ::= TOK_PI",
 /*  60 */ "constant ::= TOK_E",
 /*  61 */ "constant_body ::= identifier TOK_COLON attribute_type TOK_ASSIGNMENT expression semicolon",
 /*  62 */ "constant_body_list ::=",
 /*  63 */ "constant_body_list ::= constant_body constant_body_list",
 /*  64 */ "constant_decl ::= TOK_CONSTANT constant_body_list TOK_END_CONSTANT semicolon",
 /*  65 */ "declaration ::= entity_decl",
 /*  66 */ "declaration ::= function_decl",
 /*  67 */ "declaration ::= procedure_decl",
 /*  68 */ "declaration ::= type_decl",
 /*  69 */ "derive_decl ::=",
 /*  70 */ "derive_decl ::= TOK_DERIVE derived_attribute_rep",
 /*  71 */ "derived_attribute ::= attribute_decl TOK_COLON attribute_type initializer semicolon",
 /*  72 */ "derived_attribute_rep ::= derived_attribute",
 /*  73 */ "derived_attribute_rep ::= derived_attribute_rep derived_attribute",
 /*  74 */ "entity_body ::= explicit_attr_list derive_decl inverse_clause unique_clause where_rule_OPT",
 /*  75 */ "entity_decl ::= entity_header subsuper_decl semicolon entity_body TOK_END_ENTITY semicolon",
 /*  76 */ "entity_header ::= TOK_ENTITY TOK_IDENTIFIER",
 /*  77 */ "enumeration_type ::= TOK_ENUMERATION TOK_OF nested_id_list",
 /*  78 */ "escape_statement ::= TOK_ESCAPE semicolon",
 /*  79 */ "attribute_decl ::= TOK_IDENTIFIER",
 /*  80 */ "attribute_decl ::= TOK_SELF TOK_BACKSLASH TOK_IDENTIFIER TOK_DOT TOK_IDENTIFIER",
 /*  81 */ "attribute_decl_list ::= attribute_decl",
 /*  82 */ "attribute_decl_list ::= attribute_decl_list TOK_COMMA attribute_decl",
 /*  83 */ "optional ::=",
 /*  84 */ "optional ::= TOK_OPTIONAL",
 /*  85 */ "explicit_attribute ::= attribute_decl_list TOK_COLON optional attribute_type semicolon",
 /*  86 */ "express_file ::= schema_decl_list",
 /*  87 */ "schema_decl_list ::= schema_decl",
 /*  88 */ "schema_decl_list ::= schema_decl_list schema_decl",
 /*  89 */ "expression ::= simple_expression",
 /*  90 */ "expression ::= expression TOK_AND expression",
 /*  91 */ "expression ::= expression TOK_OR expression",
 /*  92 */ "expression ::= expression TOK_XOR expression",
 /*  93 */ "expression ::= expression TOK_LESS_THAN expression",
 /*  94 */ "expression ::= expression TOK_GREATER_THAN expression",
 /*  95 */ "expression ::= expression TOK_EQUAL expression",
 /*  96 */ "expression ::= expression TOK_LESS_EQUAL expression",
 /*  97 */ "expression ::= expression TOK_GREATER_EQUAL expression",
 /*  98 */ "expression ::= expression TOK_NOT_EQUAL expression",
 /*  99 */ "expression ::= expression TOK_INST_EQUAL expression",
 /* 100 */ "expression ::= expression TOK_INST_NOT_EQUAL expression",
 /* 101 */ "expression ::= expression TOK_IN expression",
 /* 102 */ "expression ::= expression TOK_LIKE expression",
 /* 103 */ "expression ::= simple_expression cardinality_op simple_expression",
 /* 104 */ "simple_expression ::= unary_expression",
 /* 105 */ "simple_expression ::= simple_expression TOK_CONCAT_OP simple_expression",
 /* 106 */ "simple_expression ::= simple_expression TOK_EXP simple_expression",
 /* 107 */ "simple_expression ::= simple_expression TOK_TIMES simple_expression",
 /* 108 */ "simple_expression ::= simple_expression TOK_DIV simple_expression",
 /* 109 */ "simple_expression ::= simple_expression TOK_REAL_DIV simple_expression",
 /* 110 */ "simple_expression ::= simple_expression TOK_MOD simple_expression",
 /* 111 */ "simple_expression ::= simple_expression TOK_PLUS simple_expression",
 /* 112 */ "simple_expression ::= simple_expression TOK_MINUS simple_expression",
 /* 113 */ "expression_list ::= expression",
 /* 114 */ "expression_list ::= expression_list TOK_COMMA expression",
 /* 115 */ "var ::=",
 /* 116 */ "var ::= TOK_VAR",
 /* 117 */ "formal_parameter ::= var id_list TOK_COLON parameter_type",
 /* 118 */ "formal_parameter_list ::=",
 /* 119 */ "formal_parameter_list ::= TOK_LEFT_PAREN formal_parameter_rep TOK_RIGHT_PAREN",
 /* 120 */ "formal_parameter_rep ::= formal_parameter",
 /* 121 */ "formal_parameter_rep ::= formal_parameter_rep semicolon formal_parameter",
 /* 122 */ "parameter_type ::= basic_type",
 /* 123 */ "parameter_type ::= conformant_aggregation",
 /* 124 */ "parameter_type ::= defined_type",
 /* 125 */ "parameter_type ::= generic_type",
 /* 126 */ "function_call ::= function_id actual_parameters",
 /* 127 */ "function_decl ::= function_header action_body TOK_END_FUNCTION semicolon",
 /* 128 */ "function_header ::= fh_lineno fh_push_scope fh_plist TOK_COLON parameter_type semicolon",
 /* 129 */ "fh_lineno ::= TOK_FUNCTION",
 /* 130 */ "fh_push_scope ::= TOK_IDENTIFIER",
 /* 131 */ "fh_plist ::= formal_parameter_list",
 /* 132 */ "function_id ::= TOK_IDENTIFIER",
 /* 133 */ "function_id ::= TOK_BUILTIN_FUNCTION",
 /* 134 */ "conformant_aggregation ::= aggregate_type",
 /* 135 */ "conformant_aggregation ::= TOK_ARRAY TOK_OF optional_or_unique parameter_type",
 /* 136 */ "conformant_aggregation ::= TOK_ARRAY bound_spec TOK_OF optional_or_unique parameter_type",
 /* 137 */ "conformant_aggregation ::= TOK_BAG TOK_OF parameter_type",
 /* 138 */ "conformant_aggregation ::= TOK_BAG bound_spec TOK_OF parameter_type",
 /* 139 */ "conformant_aggregation ::= TOK_LIST TOK_OF unique parameter_type",
 /* 140 */ "conformant_aggregation ::= TOK_LIST bound_spec TOK_OF unique parameter_type",
 /* 141 */ "conformant_aggregation ::= TOK_SET TOK_OF parameter_type",
 /* 142 */ "conformant_aggregation ::= TOK_SET bound_spec TOK_OF parameter_type",
 /* 143 */ "generic_type ::= TOK_GENERIC",
 /* 144 */ "generic_type ::= TOK_GENERIC TOK_COLON TOK_IDENTIFIER",
 /* 145 */ "id_list ::= TOK_IDENTIFIER",
 /* 146 */ "id_list ::= id_list TOK_COMMA TOK_IDENTIFIER",
 /* 147 */ "identifier ::= TOK_SELF",
 /* 148 */ "identifier ::= TOK_QUESTION_MARK",
 /* 149 */ "identifier ::= TOK_IDENTIFIER",
 /* 150 */ "if_statement ::= TOK_IF expression TOK_THEN statement_rep TOK_END_IF semicolon",
 /* 151 */ "if_statement ::= TOK_IF expression TOK_THEN statement_rep TOK_ELSE statement_rep TOK_END_IF semicolon",
 /* 152 */ "include_directive ::= TOK_INCLUDE TOK_STRING_LITERAL semicolon",
 /* 153 */ "increment_control ::= TOK_IDENTIFIER TOK_ASSIGNMENT expression TOK_TO expression by_expression",
 /* 154 */ "initializer ::= TOK_ASSIGNMENT expression",
 /* 155 */ "rename ::= TOK_IDENTIFIER",
 /* 156 */ "rename ::= TOK_IDENTIFIER TOK_AS TOK_IDENTIFIER",
 /* 157 */ "rename_list ::= rename",
 /* 158 */ "rename_list ::= rename_list TOK_COMMA rename",
 /* 159 */ "parened_rename_list ::= TOK_LEFT_PAREN rename_list TOK_RIGHT_PAREN",
 /* 160 */ "reference_clause ::= TOK_REFERENCE TOK_FROM TOK_IDENTIFIER semicolon",
 /* 161 */ "reference_clause ::= reference_head parened_rename_list semicolon",
 /* 162 */ "reference_head ::= TOK_REFERENCE TOK_FROM TOK_IDENTIFIER",
 /* 163 */ "use_clause ::= TOK_USE TOK_FROM TOK_IDENTIFIER semicolon",
 /* 164 */ "use_clause ::= use_head parened_rename_list semicolon",
 /* 165 */ "use_head ::= TOK_USE TOK_FROM TOK_IDENTIFIER",
 /* 166 */ "interface_specification ::= use_clause",
 /* 167 */ "interface_specification ::= reference_clause",
 /* 168 */ "interface_specification_list ::=",
 /* 169 */ "interface_specification_list ::= interface_specification_list interface_specification",
 /* 170 */ "interval ::= TOK_LEFT_CURL simple_expression rel_op simple_expression rel_op simple_expression right_curl",
 /* 171 */ "set_or_bag_of_entity ::= defined_type",
 /* 172 */ "set_or_bag_of_entity ::= TOK_SET TOK_OF defined_type",
 /* 173 */ "set_or_bag_of_entity ::= TOK_SET bound_spec TOK_OF defined_type",
 /* 174 */ "set_or_bag_of_entity ::= TOK_BAG bound_spec TOK_OF defined_type",
 /* 175 */ "set_or_bag_of_entity ::= TOK_BAG TOK_OF defined_type",
 /* 176 */ "inverse_attr_list ::= inverse_attr",
 /* 177 */ "inverse_attr_list ::= inverse_attr_list inverse_attr",
 /* 178 */ "inverse_attr ::= TOK_IDENTIFIER TOK_COLON set_or_bag_of_entity TOK_FOR TOK_IDENTIFIER semicolon",
 /* 179 */ "inverse_clause ::=",
 /* 180 */ "inverse_clause ::= TOK_INVERSE inverse_attr_list",
 /* 181 */ "bound_spec ::= TOK_LEFT_BRACKET expression TOK_COLON expression TOK_RIGHT_BRACKET",
 /* 182 */ "list_type ::= TOK_LIST bound_spec TOK_OF unique attribute_type",
 /* 183 */ "list_type ::= TOK_LIST TOK_OF unique attribute_type",
 /* 184 */ "literal ::= TOK_INTEGER_LITERAL",
 /* 185 */ "literal ::= TOK_REAL_LITERAL",
 /* 186 */ "literal ::= TOK_STRING_LITERAL",
 /* 187 */ "literal ::= TOK_STRING_LITERAL_ENCODED",
 /* 188 */ "literal ::= TOK_LOGICAL_LITERAL",
 /* 189 */ "literal ::= TOK_BINARY_LITERAL",
 /* 190 */ "literal ::= constant",
 /* 191 */ "local_initializer ::= TOK_ASSIGNMENT expression",
 /* 192 */ "local_variable ::= id_list TOK_COLON parameter_type semicolon",
 /* 193 */ "local_variable ::= id_list TOK_COLON parameter_type local_initializer semicolon",
 /* 194 */ "local_body ::=",
 /* 195 */ "local_body ::= local_variable local_body",
 /* 196 */ "local_decl ::= TOK_LOCAL allow_generic_types local_body TOK_END_LOCAL semicolon disallow_generic_types",
 /* 197 */ "allow_generic_types ::=",
 /* 198 */ "disallow_generic_types ::=",
 /* 199 */ "defined_type ::= TOK_IDENTIFIER",
 /* 200 */ "defined_type_list ::= defined_type",
 /* 201 */ "defined_type_list ::= defined_type_list TOK_COMMA defined_type",
 /* 202 */ "nested_id_list ::= TOK_LEFT_PAREN id_list TOK_RIGHT_PAREN",
 /* 203 */ "oneof_op ::= TOK_ONEOF",
 /* 204 */ "optional_or_unique ::=",
 /* 205 */ "optional_or_unique ::= TOK_OPTIONAL",
 /* 206 */ "optional_or_unique ::= TOK_UNIQUE",
 /* 207 */ "optional_or_unique ::= TOK_OPTIONAL TOK_UNIQUE",
 /* 208 */ "optional_or_unique ::= TOK_UNIQUE TOK_OPTIONAL",
 /* 209 */ "optional_fixed ::=",
 /* 210 */ "optional_fixed ::= TOK_FIXED",
 /* 211 */ "precision_spec ::=",
 /* 212 */ "precision_spec ::= TOK_LEFT_PAREN expression TOK_RIGHT_PAREN",
 /* 213 */ "proc_call_statement ::= procedure_id actual_parameters semicolon",
 /* 214 */ "proc_call_statement ::= procedure_id semicolon",
 /* 215 */ "procedure_decl ::= procedure_header action_body TOK_END_PROCEDURE semicolon",
 /* 216 */ "procedure_header ::= TOK_PROCEDURE ph_get_line ph_push_scope formal_parameter_list semicolon",
 /* 217 */ "ph_push_scope ::= TOK_IDENTIFIER",
 /* 218 */ "ph_get_line ::=",
 /* 219 */ "procedure_id ::= TOK_IDENTIFIER",
 /* 220 */ "procedure_id ::= TOK_BUILTIN_PROCEDURE",
 /* 221 */ "group_ref ::= TOK_BACKSLASH TOK_IDENTIFIER",
 /* 222 */ "qualifier ::= TOK_DOT TOK_IDENTIFIER",
 /* 223 */ "qualifier ::= TOK_BACKSLASH TOK_IDENTIFIER",
 /* 224 */ "qualifier ::= TOK_LEFT_BRACKET simple_expression TOK_RIGHT_BRACKET",
 /* 225 */ "qualifier ::= TOK_LEFT_BRACKET simple_expression TOK_COLON simple_expression TOK_RIGHT_BRACKET",
 /* 226 */ "query_expression ::= query_start expression TOK_RIGHT_PAREN",
 /* 227 */ "query_start ::= TOK_QUERY TOK_LEFT_PAREN TOK_IDENTIFIER TOK_ALL_IN expression TOK_SUCH_THAT",
 /* 228 */ "rel_op ::= TOK_LESS_THAN",
 /* 229 */ "rel_op ::= TOK_GREATER_THAN",
 /* 230 */ "rel_op ::= TOK_EQUAL",
 /* 231 */ "rel_op ::= TOK_LESS_EQUAL",
 /* 232 */ "rel_op ::= TOK_GREATER_EQUAL",
 /* 233 */ "rel_op ::= TOK_NOT_EQUAL",
 /* 234 */ "rel_op ::= TOK_INST_EQUAL",
 /* 235 */ "rel_op ::= TOK_INST_NOT_EQUAL",
 /* 236 */ "repeat_statement ::= TOK_REPEAT increment_control while_control until_control semicolon statement_rep TOK_END_REPEAT semicolon",
 /* 237 */ "repeat_statement ::= TOK_REPEAT while_control until_control semicolon statement_rep TOK_END_REPEAT semicolon",
 /* 238 */ "return_statement ::= TOK_RETURN semicolon",
 /* 239 */ "return_statement ::= TOK_RETURN TOK_LEFT_PAREN expression TOK_RIGHT_PAREN semicolon",
 /* 240 */ "right_curl ::= TOK_RIGHT_CURL",
 /* 241 */ "rule_decl ::= rule_header action_body where_rule TOK_END_RULE semicolon",
 /* 242 */ "rule_formal_parameter ::= TOK_IDENTIFIER",
 /* 243 */ "rule_formal_parameter_list ::= rule_formal_parameter",
 /* 244 */ "rule_formal_parameter_list ::= rule_formal_parameter_list TOK_COMMA rule_formal_parameter",
 /* 245 */ "rule_header ::= rh_start rule_formal_parameter_list TOK_RIGHT_PAREN semicolon",
 /* 246 */ "rh_start ::= TOK_RULE rh_get_line TOK_IDENTIFIER TOK_FOR TOK_LEFT_PAREN",
 /* 247 */ "rh_get_line ::=",
 /* 248 */ "schema_body ::= interface_specification_list block_list",
 /* 249 */ "schema_body ::= interface_specification_list constant_decl block_list",
 /* 250 */ "schema_decl ::= schema_header schema_body TOK_END_SCHEMA semicolon",
 /* 251 */ "schema_decl ::= include_directive",
 /* 252 */ "schema_header ::= TOK_SCHEMA TOK_IDENTIFIER semicolon",
 /* 253 */ "select_type ::= TOK_SELECT TOK_LEFT_PAREN defined_type_list TOK_RIGHT_PAREN",
 /* 254 */ "semicolon ::= TOK_SEMICOLON",
 /* 255 */ "set_type ::= TOK_SET bound_spec TOK_OF attribute_type",
 /* 256 */ "set_type ::= TOK_SET TOK_OF attribute_type",
 /* 257 */ "skip_statement ::= TOK_SKIP semicolon",
 /* 258 */ "statement ::= alias_statement",
 /* 259 */ "statement ::= assignment_statement",
 /* 260 */ "statement ::= case_statement",
 /* 261 */ "statement ::= compound_statement",
 /* 262 */ "statement ::= escape_statement",
 /* 263 */ "statement ::= if_statement",
 /* 264 */ "statement ::= proc_call_statement",
 /* 265 */ "statement ::= repeat_statement",
 /* 266 */ "statement ::= return_statement",
 /* 267 */ "statement ::= skip_statement",
 /* 268 */ "statement_rep ::=",
 /* 269 */ "statement_rep ::= semicolon statement_rep",
 /* 270 */ "statement_rep ::= statement statement_rep",
 /* 271 */ "subsuper_decl ::=",
 /* 272 */ "subsuper_decl ::= supertype_decl",
 /* 273 */ "subsuper_decl ::= subtype_decl",
 /* 274 */ "subsuper_decl ::= supertype_decl subtype_decl",
 /* 275 */ "subtype_decl ::= TOK_SUBTYPE TOK_OF TOK_LEFT_PAREN defined_type_list TOK_RIGHT_PAREN",
 /* 276 */ "supertype_decl ::= TOK_ABSTRACT TOK_SUPERTYPE",
 /* 277 */ "supertype_decl ::= TOK_SUPERTYPE TOK_OF TOK_LEFT_PAREN supertype_expression TOK_RIGHT_PAREN",
 /* 278 */ "supertype_decl ::= TOK_ABSTRACT TOK_SUPERTYPE TOK_OF TOK_LEFT_PAREN supertype_expression TOK_RIGHT_PAREN",
 /* 279 */ "supertype_expression ::= supertype_factor",
 /* 280 */ "supertype_expression ::= supertype_expression TOK_AND supertype_factor",
 /* 281 */ "supertype_expression ::= supertype_expression TOK_ANDOR supertype_factor",
 /* 282 */ "supertype_expression_list ::= supertype_expression",
 /* 283 */ "supertype_expression_list ::= supertype_expression_list TOK_COMMA supertype_expression",
 /* 284 */ "supertype_factor ::= identifier",
 /* 285 */ "supertype_factor ::= oneof_op TOK_LEFT_PAREN supertype_expression_list TOK_RIGHT_PAREN",
 /* 286 */ "supertype_factor ::= TOK_LEFT_PAREN supertype_expression TOK_RIGHT_PAREN",
 /* 287 */ "type ::= aggregation_type",
 /* 288 */ "type ::= basic_type",
 /* 289 */ "type ::= defined_type",
 /* 290 */ "type ::= select_type",
 /* 291 */ "type_item_body ::= enumeration_type",
 /* 292 */ "type_item_body ::= type",
 /* 293 */ "type_item ::= ti_start type_item_body semicolon",
 /* 294 */ "ti_start ::= TOK_IDENTIFIER TOK_EQUAL",
 /* 295 */ "type_decl ::= td_start TOK_END_TYPE semicolon",
 /* 296 */ "td_start ::= TOK_TYPE type_item where_rule_OPT",
 /* 297 */ "general_ref ::= assignable group_ref",
 /* 298 */ "general_ref ::= assignable",
 /* 299 */ "unary_expression ::= aggregate_initializer",
 /* 300 */ "unary_expression ::= unary_expression qualifier",
 /* 301 */ "unary_expression ::= literal",
 /* 302 */ "unary_expression ::= function_call",
 /* 303 */ "unary_expression ::= identifier",
 /* 304 */ "unary_expression ::= TOK_LEFT_PAREN expression TOK_RIGHT_PAREN",
 /* 305 */ "unary_expression ::= interval",
 /* 306 */ "unary_expression ::= query_expression",
 /* 307 */ "unary_expression ::= TOK_NOT unary_expression",
 /* 308 */ "unary_expression ::= TOK_PLUS unary_expression",
 /* 309 */ "unary_expression ::= TOK_MINUS unary_expression",
 /* 310 */ "unique ::=",
 /* 311 */ "unique ::= TOK_UNIQUE",
 /* 312 */ "qualified_attr ::= TOK_IDENTIFIER",
 /* 313 */ "qualified_attr ::= TOK_SELF TOK_BACKSLASH TOK_IDENTIFIER TOK_DOT TOK_IDENTIFIER",
 /* 314 */ "qualified_attr_list ::= qualified_attr",
 /* 315 */ "qualified_attr_list ::= qualified_attr_list TOK_COMMA qualified_attr",
 /* 316 */ "labelled_attrib_list ::= qualified_attr_list semicolon",
 /* 317 */ "labelled_attrib_list ::= TOK_IDENTIFIER TOK_COLON qualified_attr_list semicolon",
 /* 318 */ "labelled_attrib_list_list ::= labelled_attrib_list",
 /* 319 */ "labelled_attrib_list_list ::= labelled_attrib_list_list labelled_attrib_list",
 /* 320 */ "unique_clause ::=",
 /* 321 */ "unique_clause ::= TOK_UNIQUE labelled_attrib_list_list",
 /* 322 */ "until_control ::=",
 /* 323 */ "until_control ::= TOK_UNTIL expression",
 /* 324 */ "where_clause ::= expression semicolon",
 /* 325 */ "where_clause ::= TOK_IDENTIFIER TOK_COLON expression semicolon",
 /* 326 */ "where_clause_list ::= where_clause",
 /* 327 */ "where_clause_list ::= where_clause_list where_clause",
 /* 328 */ "where_rule ::= TOK_WHERE where_clause_list",
 /* 329 */ "where_rule_OPT ::=",
 /* 330 */ "where_rule_OPT ::= where_rule",
 /* 331 */ "while_control ::=",
 /* 332 */ "while_control ::= TOK_WHILE expression",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 256;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to Parse and ParseFree.
*/
void *ParseAlloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  ParseARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    case 122: /* statement_list */
{
#line 187 "expparse.y"

    if (parseData.scanner == NULL) {
    (yypminor->yy0).string = (char*)NULL;
    }

#line 1614 "expparse.c"
}
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos;

  if( pParser->yyidx<0 ) return 0;

  yytos = &pParser->yystack[pParser->yyidx];

#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor(pParser, yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from ParseAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void ParseFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
int ParseStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>YY_SHIFT_COUNT
   || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        return yy_find_shift_action(pParser, iFallback);
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
          j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
          j<YY_ACTTAB_COUNT &&
#endif
          yy_lookahead[j]==YYWILDCARD
        ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   ParseARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
#line 2480 "expparse.y"

    fprintf(stderr, "Express parser experienced stack overflow.\n");
    fprintf(stderr, "Last token had value %x\n", yypMinor->yy0.val);
#line 1803 "expparse.c"
   ParseARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 156, 2 },
  { 233, 1 },
  { 233, 1 },
  { 233, 1 },
  { 232, 0 },
  { 232, 2 },
  { 157, 3 },
  { 157, 2 },
  { 127, 2 },
  { 127, 3 },
  { 126, 1 },
  { 158, 1 },
  { 158, 3 },
  { 158, 3 },
  { 158, 5 },
  { 217, 3 },
  { 217, 5 },
  { 218, 1 },
  { 218, 1 },
  { 218, 1 },
  { 218, 1 },
  { 195, 9 },
  { 238, 0 },
  { 219, 5 },
  { 128, 2 },
  { 128, 1 },
  { 196, 4 },
  { 211, 1 },
  { 211, 1 },
  { 211, 1 },
  { 159, 0 },
  { 159, 2 },
  { 220, 4 },
  { 220, 3 },
  { 215, 1 },
  { 215, 2 },
  { 215, 2 },
  { 215, 1 },
  { 215, 1 },
  { 215, 3 },
  { 215, 3 },
  { 239, 0 },
  { 239, 2 },
  { 240, 1 },
  { 240, 1 },
  { 240, 1 },
  { 130, 0 },
  { 130, 2 },
  { 226, 5 },
  { 123, 3 },
  { 160, 0 },
  { 160, 2 },
  { 161, 2 },
  { 162, 1 },
  { 162, 3 },
  { 124, 0 },
  { 124, 3 },
  { 197, 6 },
  { 198, 4 },
  { 131, 1 },
  { 131, 1 },
  { 243, 6 },
  { 244, 0 },
  { 244, 2 },
  { 235, 4 },
  { 234, 1 },
  { 234, 1 },
  { 234, 1 },
  { 234, 1 },
  { 164, 0 },
  { 164, 2 },
  { 229, 5 },
  { 183, 1 },
  { 183, 2 },
  { 125, 5 },
  { 245, 6 },
  { 249, 2 },
  { 250, 3 },
  { 199, 2 },
  { 129, 1 },
  { 129, 5 },
  { 182, 1 },
  { 182, 3 },
  { 190, 0 },
  { 190, 1 },
  { 165, 5 },
  { 251, 1 },
  { 252, 1 },
  { 252, 2 },
  { 132, 1 },
  { 132, 3 },
  { 132, 3 },
  { 132, 3 },
  { 132, 3 },
  { 132, 3 },
  { 132, 3 },
  { 132, 3 },
  { 132, 3 },
  { 132, 3 },
  { 132, 3 },
  { 132, 3 },
  { 132, 3 },
  { 132, 3 },
  { 132, 3 },
  { 144, 1 },
  { 144, 3 },
  { 144, 3 },
  { 144, 3 },
  { 144, 3 },
  { 144, 3 },
  { 144, 3 },
  { 144, 3 },
  { 144, 3 },
  { 166, 1 },
  { 166, 3 },
  { 191, 0 },
  { 191, 1 },
  { 167, 4 },
  { 168, 0 },
  { 168, 3 },
  { 169, 1 },
  { 169, 3 },
  { 213, 1 },
  { 213, 1 },
  { 213, 1 },
  { 213, 1 },
  { 133, 2 },
  { 246, 4 },
  { 149, 6 },
  { 150, 1 },
  { 254, 1 },
  { 255, 1 },
  { 209, 1 },
  { 209, 1 },
  { 221, 1 },
  { 221, 4 },
  { 221, 5 },
  { 221, 3 },
  { 221, 4 },
  { 221, 4 },
  { 221, 5 },
  { 221, 3 },
  { 221, 4 },
  { 214, 1 },
  { 214, 3 },
  { 170, 1 },
  { 170, 3 },
  { 136, 1 },
  { 136, 1 },
  { 136, 1 },
  { 200, 6 },
  { 200, 8 },
  { 241, 3 },
  { 256, 6 },
  { 137, 2 },
  { 257, 1 },
  { 257, 3 },
  { 258, 1 },
  { 258, 3 },
  { 259, 3 },
  { 260, 4 },
  { 260, 3 },
  { 261, 3 },
  { 262, 4 },
  { 262, 3 },
  { 263, 3 },
  { 264, 1 },
  { 264, 1 },
  { 265, 0 },
  { 265, 2 },
  { 138, 7 },
  { 224, 1 },
  { 224, 3 },
  { 224, 4 },
  { 224, 4 },
  { 224, 3 },
  { 180, 1 },
  { 180, 2 },
  { 228, 6 },
  { 181, 0 },
  { 181, 2 },
  { 227, 5 },
  { 222, 5 },
  { 222, 4 },
  { 139, 1 },
  { 139, 1 },
  { 139, 1 },
  { 139, 1 },
  { 139, 1 },
  { 139, 1 },
  { 139, 1 },
  { 140, 2 },
  { 267, 4 },
  { 267, 5 },
  { 268, 0 },
  { 268, 2 },
  { 236, 6 },
  { 269, 0 },
  { 270, 0 },
  { 212, 1 },
  { 171, 1 },
  { 171, 3 },
  { 172, 3 },
  { 271, 1 },
  { 188, 0 },
  { 188, 1 },
  { 188, 1 },
  { 188, 2 },
  { 188, 2 },
  { 189, 0 },
  { 189, 1 },
  { 141, 0 },
  { 141, 3 },
  { 201, 3 },
  { 201, 2 },
  { 247, 4 },
  { 154, 5 },
  { 272, 1 },
  { 155, 0 },
  { 210, 1 },
  { 210, 1 },
  { 135, 2 },
  { 194, 2 },
  { 194, 2 },
  { 194, 3 },
  { 194, 5 },
  { 142, 3 },
  { 143, 6 },
  { 187, 1 },
  { 187, 1 },
  { 187, 1 },
  { 187, 1 },
  { 187, 1 },
  { 187, 1 },
  { 187, 1 },
  { 187, 1 },
  { 202, 8 },
  { 202, 7 },
  { 203, 2 },
  { 203, 5 },
  { 266, 1 },
  { 242, 5 },
  { 230, 1 },
  { 185, 1 },
  { 185, 3 },
  { 151, 4 },
  { 152, 5 },
  { 153, 0 },
  { 273, 2 },
  { 273, 3 },
  { 253, 4 },
  { 253, 1 },
  { 274, 3 },
  { 216, 4 },
  { 237, 1 },
  { 223, 4 },
  { 223, 3 },
  { 204, 2 },
  { 205, 1 },
  { 205, 1 },
  { 205, 1 },
  { 205, 1 },
  { 205, 1 },
  { 205, 1 },
  { 205, 1 },
  { 205, 1 },
  { 205, 1 },
  { 205, 1 },
  { 173, 0 },
  { 173, 2 },
  { 173, 2 },
  { 206, 0 },
  { 206, 1 },
  { 206, 1 },
  { 206, 2 },
  { 174, 5 },
  { 207, 2 },
  { 207, 5 },
  { 207, 6 },
  { 146, 1 },
  { 146, 3 },
  { 146, 3 },
  { 177, 1 },
  { 177, 3 },
  { 208, 1 },
  { 208, 4 },
  { 208, 3 },
  { 225, 1 },
  { 225, 1 },
  { 225, 1 },
  { 225, 1 },
  { 275, 1 },
  { 275, 1 },
  { 276, 3 },
  { 277, 2 },
  { 248, 3 },
  { 278, 3 },
  { 134, 2 },
  { 134, 1 },
  { 145, 1 },
  { 145, 2 },
  { 145, 1 },
  { 145, 1 },
  { 145, 1 },
  { 145, 3 },
  { 145, 1 },
  { 145, 1 },
  { 145, 2 },
  { 145, 2 },
  { 145, 2 },
  { 192, 0 },
  { 192, 1 },
  { 193, 1 },
  { 193, 5 },
  { 186, 1 },
  { 186, 3 },
  { 179, 2 },
  { 179, 4 },
  { 178, 1 },
  { 178, 2 },
  { 184, 0 },
  { 184, 2 },
  { 147, 0 },
  { 147, 2 },
  { 231, 2 },
  { 231, 4 },
  { 163, 1 },
  { 163, 2 },
  { 175, 2 },
  { 176, 0 },
  { 176, 1 },
  { 148, 0 },
  { 148, 2 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  ParseARG_FETCH;

  yymsp = &yypParser->yystack[yypParser->yyidx];

  if( yyruleno>=0 ) {
#ifndef NDEBUG
      if ( yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0]))) {
         if (yyTraceFILE) {
      fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
              yyRuleName[yyruleno]);
    }
   }
#endif /* NDEBUG */
  } else {
    /* invalid rule number range */
    return;
  }


  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.  
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  /*memset(&yygotominor, 0, sizeof(yygotominor));*/
  yygotominor = yyzerominor;


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0: /* action_body ::= action_body_item_rep statement_rep */
      case 70: /* derive_decl ::= TOK_DERIVE derived_attribute_rep */ yytestcase(yyruleno==70);
      case 180: /* inverse_clause ::= TOK_INVERSE inverse_attr_list */ yytestcase(yyruleno==180);
      case 269: /* statement_rep ::= semicolon statement_rep */ yytestcase(yyruleno==269);
      case 321: /* unique_clause ::= TOK_UNIQUE labelled_attrib_list_list */ yytestcase(yyruleno==321);
      case 328: /* where_rule ::= TOK_WHERE where_clause_list */ yytestcase(yyruleno==328);
      case 330: /* where_rule_OPT ::= where_rule */ yytestcase(yyruleno==330);
#line 360 "expparse.y"
{
    yygotominor.yy371 = yymsp[0].minor.yy371;
}
#line 2267 "expparse.c"
        break;
      case 1: /* action_body_item ::= declaration */
      case 2: /* action_body_item ::= constant_decl */ yytestcase(yyruleno==2);
      case 3: /* action_body_item ::= local_decl */ yytestcase(yyruleno==3);
      case 43: /* block_member ::= declaration */ yytestcase(yyruleno==43);
      case 44: /* block_member ::= include_directive */ yytestcase(yyruleno==44);
      case 45: /* block_member ::= rule_decl */ yytestcase(yyruleno==45);
      case 65: /* declaration ::= entity_decl */ yytestcase(yyruleno==65);
      case 66: /* declaration ::= function_decl */ yytestcase(yyruleno==66);
      case 67: /* declaration ::= procedure_decl */ yytestcase(yyruleno==67);
      case 68: /* declaration ::= type_decl */ yytestcase(yyruleno==68);
      case 87: /* schema_decl_list ::= schema_decl */ yytestcase(yyruleno==87);
      case 157: /* rename_list ::= rename */ yytestcase(yyruleno==157);
      case 166: /* interface_specification ::= use_clause */ yytestcase(yyruleno==166);
      case 167: /* interface_specification ::= reference_clause */ yytestcase(yyruleno==167);
      case 203: /* oneof_op ::= TOK_ONEOF */ yytestcase(yyruleno==203);
      case 251: /* schema_decl ::= include_directive */ yytestcase(yyruleno==251);
      case 291: /* type_item_body ::= enumeration_type */ yytestcase(yyruleno==291);
#line 366 "expparse.y"
{
    yygotominor.yy0 = yymsp[0].minor.yy0;
}
#line 2290 "expparse.c"
        break;
      case 5: /* action_body_item_rep ::= action_body_item action_body_item_rep */
      case 42: /* block_list ::= block_list block_member */ yytestcase(yyruleno==42);
      case 63: /* constant_body_list ::= constant_body constant_body_list */ yytestcase(yyruleno==63);
      case 88: /* schema_decl_list ::= schema_decl_list schema_decl */ yytestcase(yyruleno==88);
      case 169: /* interface_specification_list ::= interface_specification_list interface_specification */ yytestcase(yyruleno==169);
      case 195: /* local_body ::= local_variable local_body */ yytestcase(yyruleno==195);
      case 248: /* schema_body ::= interface_specification_list block_list */ yytestcase(yyruleno==248);
#line 383 "expparse.y"
{
    yygotominor.yy0 = yymsp[-1].minor.yy0;
}
#line 2303 "expparse.c"
        break;
      case 6: /* actual_parameters ::= TOK_LEFT_PAREN expression_list TOK_RIGHT_PAREN */
      case 202: /* nested_id_list ::= TOK_LEFT_PAREN id_list TOK_RIGHT_PAREN */ yytestcase(yyruleno==202);
      case 275: /* subtype_decl ::= TOK_SUBTYPE TOK_OF TOK_LEFT_PAREN defined_type_list TOK_RIGHT_PAREN */ yytestcase(yyruleno==275);
#line 400 "expparse.y"
{
    yygotominor.yy371 = yymsp[-1].minor.yy371;
}
#line 2312 "expparse.c"
        break;
      case 7: /* actual_parameters ::= TOK_LEFT_PAREN TOK_RIGHT_PAREN */
      case 320: /* unique_clause ::= */ yytestcase(yyruleno==320);
#line 404 "expparse.y"
{
    yygotominor.yy371 = 0;
}
#line 2320 "expparse.c"
        break;
      case 8: /* aggregate_initializer ::= TOK_LEFT_BRACKET TOK_RIGHT_BRACKET */
#line 410 "expparse.y"
{
    yygotominor.yy401 = EXPcreate(Type_Aggregate);
    yygotominor.yy401->u.list = LISTcreate();
}
#line 2328 "expparse.c"
        break;
      case 9: /* aggregate_initializer ::= TOK_LEFT_BRACKET aggregate_init_body TOK_RIGHT_BRACKET */
#line 416 "expparse.y"
{
    yygotominor.yy401 = EXPcreate(Type_Aggregate);
    yygotominor.yy401->u.list = yymsp[-1].minor.yy371;
}
#line 2336 "expparse.c"
        break;
      case 10: /* aggregate_init_element ::= expression */
      case 25: /* assignable ::= identifier */ yytestcase(yyruleno==25);
      case 47: /* by_expression ::= TOK_BY expression */ yytestcase(yyruleno==47);
      case 89: /* expression ::= simple_expression */ yytestcase(yyruleno==89);
      case 104: /* simple_expression ::= unary_expression */ yytestcase(yyruleno==104);
      case 154: /* initializer ::= TOK_ASSIGNMENT expression */ yytestcase(yyruleno==154);
      case 190: /* literal ::= constant */ yytestcase(yyruleno==190);
      case 191: /* local_initializer ::= TOK_ASSIGNMENT expression */ yytestcase(yyruleno==191);
      case 298: /* general_ref ::= assignable */ yytestcase(yyruleno==298);
      case 299: /* unary_expression ::= aggregate_initializer */ yytestcase(yyruleno==299);
      case 301: /* unary_expression ::= literal */ yytestcase(yyruleno==301);
      case 302: /* unary_expression ::= function_call */ yytestcase(yyruleno==302);
      case 303: /* unary_expression ::= identifier */ yytestcase(yyruleno==303);
      case 305: /* unary_expression ::= interval */ yytestcase(yyruleno==305);
      case 306: /* unary_expression ::= query_expression */ yytestcase(yyruleno==306);
      case 308: /* unary_expression ::= TOK_PLUS unary_expression */ yytestcase(yyruleno==308);
      case 323: /* until_control ::= TOK_UNTIL expression */ yytestcase(yyruleno==323);
      case 332: /* while_control ::= TOK_WHILE expression */ yytestcase(yyruleno==332);
#line 422 "expparse.y"
{
    yygotominor.yy401 = yymsp[0].minor.yy401;
}
#line 2360 "expparse.c"
        break;
      case 11: /* aggregate_init_body ::= aggregate_init_element */
      case 113: /* expression_list ::= expression */ yytestcase(yyruleno==113);
      case 282: /* supertype_expression_list ::= supertype_expression */ yytestcase(yyruleno==282);
#line 427 "expparse.y"
{
    yygotominor.yy371 = LISTcreate();
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy401);
}
#line 2370 "expparse.c"
        break;
      case 12: /* aggregate_init_body ::= aggregate_init_element TOK_COLON expression */
#line 432 "expparse.y"
{
    yygotominor.yy371 = LISTcreate();
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[-2].minor.yy401);

    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy401);

    yymsp[0].minor.yy401->type = Type_Repeat;
}
#line 2382 "expparse.c"
        break;
      case 13: /* aggregate_init_body ::= aggregate_init_body TOK_COMMA aggregate_init_element */
#line 442 "expparse.y"
{ 
    yygotominor.yy371 = yymsp[-2].minor.yy371;

    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy401);

}
#line 2392 "expparse.c"
        break;
      case 14: /* aggregate_init_body ::= aggregate_init_body TOK_COMMA aggregate_init_element TOK_COLON expression */
#line 450 "expparse.y"
{
    yygotominor.yy371 = yymsp[-4].minor.yy371;

    LISTadd_last(yygotominor.yy371, (Generic)yymsp[-2].minor.yy401);
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy401);

    yymsp[0].minor.yy401->type = Type_Repeat;
}
#line 2404 "expparse.c"
        break;
      case 15: /* aggregate_type ::= TOK_AGGREGATE TOK_OF parameter_type */
#line 460 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(aggregate_);
    yygotominor.yy477->base = yymsp[0].minor.yy297;

    if (tag_count < 0) {
        Symbol sym;
        sym.line = yylineno;
        sym.filename = current_filename;
        ERRORreport_with_symbol(ERROR_unlabelled_param_type, &sym,
        CURRENT_SCOPE_NAME);
    }
}
#line 2420 "expparse.c"
        break;
      case 16: /* aggregate_type ::= TOK_AGGREGATE TOK_COLON TOK_IDENTIFIER TOK_OF parameter_type */
#line 474 "expparse.y"
{
    Type t = TYPEcreate_user_defined_tag(yymsp[0].minor.yy297, CURRENT_SCOPE, yymsp[-2].minor.yy0.symbol);

    if (t) {
        SCOPEadd_super(t);
        yygotominor.yy477 = TYPEBODYcreate(aggregate_);
        yygotominor.yy477->tag = t;
        yygotominor.yy477->base = yymsp[0].minor.yy297;
    }
}
#line 2434 "expparse.c"
        break;
      case 17: /* aggregation_type ::= array_type */
      case 18: /* aggregation_type ::= bag_type */ yytestcase(yyruleno==18);
      case 19: /* aggregation_type ::= list_type */ yytestcase(yyruleno==19);
      case 20: /* aggregation_type ::= set_type */ yytestcase(yyruleno==20);
#line 486 "expparse.y"
{
    yygotominor.yy477 = yymsp[0].minor.yy477;
}
#line 2444 "expparse.c"
        break;
      case 21: /* alias_statement ::= TOK_ALIAS TOK_IDENTIFIER TOK_FOR general_ref semicolon alias_push_scope statement_rep TOK_END_ALIAS semicolon */
#line 505 "expparse.y"
{
    Expression e = EXPcreate_from_symbol(Type_Attribute, yymsp[-7].minor.yy0.symbol);
    Variable v = VARcreate(e, Type_Unknown);

    v->initializer = yymsp[-5].minor.yy401; 

    DICTdefine(CURRENT_SCOPE->symbol_table, yymsp[-7].minor.yy0.symbol->name, (Generic)v,
        yymsp[-7].minor.yy0.symbol, OBJ_VARIABLE);
    yygotominor.yy332 = ALIAScreate(CURRENT_SCOPE, v, yymsp[-2].minor.yy371);

    POP_SCOPE();
}
#line 2460 "expparse.c"
        break;
      case 22: /* alias_push_scope ::= */
#line 519 "expparse.y"
{
    struct Scope_ *s = SCOPEcreate_tiny(OBJ_ALIAS);
    PUSH_SCOPE(s, (Symbol *)0, OBJ_ALIAS);
}
#line 2468 "expparse.c"
        break;
      case 23: /* array_type ::= TOK_ARRAY bound_spec TOK_OF optional_or_unique attribute_type */
#line 526 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(array_);

    yygotominor.yy477->flags.optional = yymsp[-1].minor.yy252.optional;
    yygotominor.yy477->flags.unique = yymsp[-1].minor.yy252.unique;
    yygotominor.yy477->upper = yymsp[-3].minor.yy253.upper_limit;
    yygotominor.yy477->lower = yymsp[-3].minor.yy253.lower_limit;
    yygotominor.yy477->base = yymsp[0].minor.yy297;
}
#line 2481 "expparse.c"
        break;
      case 24: /* assignable ::= assignable qualifier */
      case 300: /* unary_expression ::= unary_expression qualifier */ yytestcase(yyruleno==300);
#line 538 "expparse.y"
{
    yymsp[0].minor.yy46.first->e.op1 = yymsp[-1].minor.yy401;
    yygotominor.yy401 = yymsp[0].minor.yy46.expr;
}
#line 2490 "expparse.c"
        break;
      case 26: /* assignment_statement ::= assignable TOK_ASSIGNMENT expression semicolon */
#line 549 "expparse.y"
{ 
    yygotominor.yy332 = ASSIGNcreate(yymsp[-3].minor.yy401, yymsp[-1].minor.yy401);
}
#line 2497 "expparse.c"
        break;
      case 27: /* attribute_type ::= aggregation_type */
      case 28: /* attribute_type ::= basic_type */ yytestcase(yyruleno==28);
      case 122: /* parameter_type ::= basic_type */ yytestcase(yyruleno==122);
      case 123: /* parameter_type ::= conformant_aggregation */ yytestcase(yyruleno==123);
#line 554 "expparse.y"
{
    yygotominor.yy297 = TYPEcreate_from_body_anonymously(yymsp[0].minor.yy477);
    SCOPEadd_super(yygotominor.yy297);
}
#line 2508 "expparse.c"
        break;
      case 29: /* attribute_type ::= defined_type */
      case 124: /* parameter_type ::= defined_type */ yytestcase(yyruleno==124);
      case 125: /* parameter_type ::= generic_type */ yytestcase(yyruleno==125);
#line 564 "expparse.y"
{
    yygotominor.yy297 = yymsp[0].minor.yy297;
}
#line 2517 "expparse.c"
        break;
      case 30: /* explicit_attr_list ::= */
      case 50: /* case_action_list ::= */ yytestcase(yyruleno==50);
      case 69: /* derive_decl ::= */ yytestcase(yyruleno==69);
      case 268: /* statement_rep ::= */ yytestcase(yyruleno==268);
#line 569 "expparse.y"
{
    yygotominor.yy371 = LISTcreate();
}
#line 2527 "expparse.c"
        break;
      case 31: /* explicit_attr_list ::= explicit_attr_list explicit_attribute */
#line 573 "expparse.y"
{
    yygotominor.yy371 = yymsp[-1].minor.yy371;
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy371); 
}
#line 2535 "expparse.c"
        break;
      case 32: /* bag_type ::= TOK_BAG bound_spec TOK_OF attribute_type */
      case 138: /* conformant_aggregation ::= TOK_BAG bound_spec TOK_OF parameter_type */ yytestcase(yyruleno==138);
#line 579 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(bag_);
    yygotominor.yy477->base = yymsp[0].minor.yy297;
    yygotominor.yy477->upper = yymsp[-2].minor.yy253.upper_limit;
    yygotominor.yy477->lower = yymsp[-2].minor.yy253.lower_limit;
}
#line 2546 "expparse.c"
        break;
      case 33: /* bag_type ::= TOK_BAG TOK_OF attribute_type */
#line 586 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(bag_);
    yygotominor.yy477->base = yymsp[0].minor.yy297;
}
#line 2554 "expparse.c"
        break;
      case 34: /* basic_type ::= TOK_BOOLEAN */
#line 592 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(boolean_);
}
#line 2561 "expparse.c"
        break;
      case 35: /* basic_type ::= TOK_INTEGER precision_spec */
#line 596 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(integer_);
    yygotominor.yy477->precision = yymsp[0].minor.yy401;
}
#line 2569 "expparse.c"
        break;
      case 36: /* basic_type ::= TOK_REAL precision_spec */
#line 601 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(real_);
    yygotominor.yy477->precision = yymsp[0].minor.yy401;
}
#line 2577 "expparse.c"
        break;
      case 37: /* basic_type ::= TOK_NUMBER */
#line 606 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(number_);
}
#line 2584 "expparse.c"
        break;
      case 38: /* basic_type ::= TOK_LOGICAL */
#line 610 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(logical_);
}
#line 2591 "expparse.c"
        break;
      case 39: /* basic_type ::= TOK_BINARY precision_spec optional_fixed */
#line 614 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(binary_);
    yygotominor.yy477->precision = yymsp[-1].minor.yy401;
    yygotominor.yy477->flags.fixed = yymsp[0].minor.yy252.fixed;
}
#line 2600 "expparse.c"
        break;
      case 40: /* basic_type ::= TOK_STRING precision_spec optional_fixed */
#line 620 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(string_);
    yygotominor.yy477->precision = yymsp[-1].minor.yy401;
    yygotominor.yy477->flags.fixed = yymsp[0].minor.yy252.fixed;
}
#line 2609 "expparse.c"
        break;
      case 46: /* by_expression ::= */
#line 646 "expparse.y"
{
    yygotominor.yy401 = LITERAL_ONE;
}
#line 2616 "expparse.c"
        break;
      case 48: /* cardinality_op ::= TOK_LEFT_CURL expression TOK_COLON expression TOK_RIGHT_CURL */
      case 181: /* bound_spec ::= TOK_LEFT_BRACKET expression TOK_COLON expression TOK_RIGHT_BRACKET */ yytestcase(yyruleno==181);
#line 656 "expparse.y"
{
    yygotominor.yy253.lower_limit = yymsp[-3].minor.yy401;
    yygotominor.yy253.upper_limit = yymsp[-1].minor.yy401;
}
#line 2625 "expparse.c"
        break;
      case 49: /* case_action ::= case_labels TOK_COLON statement */
#line 662 "expparse.y"
{
    yygotominor.yy321 = CASE_ITcreate(yymsp[-2].minor.yy371, yymsp[0].minor.yy332);
    SYMBOLset(yygotominor.yy321);
}
#line 2633 "expparse.c"
        break;
      case 51: /* case_action_list ::= case_action_list case_action */
#line 672 "expparse.y"
{
    yyerrok;

    yygotominor.yy371 = yymsp[-1].minor.yy371;

    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy321);
}
#line 2644 "expparse.c"
        break;
      case 52: /* case_block ::= case_action_list case_otherwise */
#line 681 "expparse.y"
{
    yygotominor.yy371 = yymsp[-1].minor.yy371;

    if (yymsp[0].minor.yy321) {
        LISTadd_last(yygotominor.yy371,
        (Generic)yymsp[0].minor.yy321);
    }
}
#line 2656 "expparse.c"
        break;
      case 53: /* case_labels ::= expression */
#line 691 "expparse.y"
{
    yygotominor.yy371 = LISTcreate();

    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy401);
}
#line 2665 "expparse.c"
        break;
      case 54: /* case_labels ::= case_labels TOK_COMMA expression */
#line 697 "expparse.y"
{
    yyerrok;

    yygotominor.yy371 = yymsp[-2].minor.yy371;
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy401);
}
#line 2675 "expparse.c"
        break;
      case 55: /* case_otherwise ::= */
#line 705 "expparse.y"
{
    yygotominor.yy321 = (Case_Item)0;
}
#line 2682 "expparse.c"
        break;
      case 56: /* case_otherwise ::= TOK_OTHERWISE TOK_COLON statement */
#line 709 "expparse.y"
{
    yygotominor.yy321 = CASE_ITcreate(LIST_NULL, yymsp[0].minor.yy332);
    SYMBOLset(yygotominor.yy321);
}
#line 2690 "expparse.c"
        break;
      case 57: /* case_statement ::= TOK_CASE expression TOK_OF case_block TOK_END_CASE semicolon */
#line 716 "expparse.y"
{
    yygotominor.yy332 = CASEcreate(yymsp[-4].minor.yy401, yymsp[-2].minor.yy371);
}
#line 2697 "expparse.c"
        break;
      case 58: /* compound_statement ::= TOK_BEGIN statement_rep TOK_END semicolon */
#line 721 "expparse.y"
{
    yygotominor.yy332 = COMP_STMTcreate(yymsp[-2].minor.yy371);
}
#line 2704 "expparse.c"
        break;
      case 59: /* constant ::= TOK_PI */
#line 726 "expparse.y"
{ 
    yygotominor.yy401 = LITERAL_PI;
}
#line 2711 "expparse.c"
        break;
      case 60: /* constant ::= TOK_E */
#line 731 "expparse.y"
{ 
    yygotominor.yy401 = LITERAL_E;
}
#line 2718 "expparse.c"
        break;
      case 61: /* constant_body ::= identifier TOK_COLON attribute_type TOK_ASSIGNMENT expression semicolon */
#line 738 "expparse.y"
{
    Variable v;

    yymsp[-5].minor.yy401->type = yymsp[-3].minor.yy297;
    v = VARcreate(yymsp[-5].minor.yy401, yymsp[-3].minor.yy297);
    v->initializer = yymsp[-1].minor.yy401;
    v->flags.constant = 1;
    DICTdefine(CURRENT_SCOPE->symbol_table, yymsp[-5].minor.yy401->symbol.name, (Generic)v,
    &yymsp[-5].minor.yy401->symbol, OBJ_VARIABLE);
}
#line 2732 "expparse.c"
        break;
      case 64: /* constant_decl ::= TOK_CONSTANT constant_body_list TOK_END_CONSTANT semicolon */
#line 757 "expparse.y"
{
    yygotominor.yy0 = yymsp[-3].minor.yy0;
}
#line 2739 "expparse.c"
        break;
      case 71: /* derived_attribute ::= attribute_decl TOK_COLON attribute_type initializer semicolon */
#line 789 "expparse.y"
{
    yygotominor.yy91 = VARcreate(yymsp[-4].minor.yy401, yymsp[-2].minor.yy297);
    yygotominor.yy91->initializer = yymsp[-1].minor.yy401;
    yygotominor.yy91->flags.attribute = true;
}
#line 2748 "expparse.c"
        break;
      case 72: /* derived_attribute_rep ::= derived_attribute */
      case 176: /* inverse_attr_list ::= inverse_attr */ yytestcase(yyruleno==176);
#line 796 "expparse.y"
{
    yygotominor.yy371 = LISTcreate();
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy91);
}
#line 2757 "expparse.c"
        break;
      case 73: /* derived_attribute_rep ::= derived_attribute_rep derived_attribute */
      case 177: /* inverse_attr_list ::= inverse_attr_list inverse_attr */ yytestcase(yyruleno==177);
#line 801 "expparse.y"
{
    yygotominor.yy371 = yymsp[-1].minor.yy371;
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy91);
}
#line 2766 "expparse.c"
        break;
      case 74: /* entity_body ::= explicit_attr_list derive_decl inverse_clause unique_clause where_rule_OPT */
#line 808 "expparse.y"
{
    yygotominor.yy176.attributes = yymsp[-4].minor.yy371;
    /* this is flattened out in entity_decl - DEL */
    LISTadd_last(yygotominor.yy176.attributes, (Generic)yymsp[-3].minor.yy371);

    if (yymsp[-2].minor.yy371 != LIST_NULL) {
    LISTadd_last(yygotominor.yy176.attributes, (Generic)yymsp[-2].minor.yy371);
    }

    yygotominor.yy176.unique = yymsp[-1].minor.yy371;
    yygotominor.yy176.where = yymsp[0].minor.yy371;
}
#line 2782 "expparse.c"
        break;
      case 75: /* entity_decl ::= entity_header subsuper_decl semicolon entity_body TOK_END_ENTITY semicolon */
#line 823 "expparse.y"
{
    CURRENT_SCOPE->u.entity->subtype_expression = yymsp[-4].minor.yy242.subtypes;
    CURRENT_SCOPE->u.entity->supertype_symbols = yymsp[-4].minor.yy242.supertypes;
    LISTdo( yymsp[-2].minor.yy176.attributes, l, Linked_List ) {
        LISTdo_n( l, a, Variable, b ) {
            ENTITYadd_attribute(CURRENT_SCOPE, a);
        } LISTod;
    } LISTod;
    CURRENT_SCOPE->u.entity->abstract = yymsp[-4].minor.yy242.abstract;
    CURRENT_SCOPE->u.entity->unique = yymsp[-2].minor.yy176.unique;
    CURRENT_SCOPE->where = yymsp[-2].minor.yy176.where;
    POP_SCOPE();
}
#line 2799 "expparse.c"
        break;
      case 76: /* entity_header ::= TOK_ENTITY TOK_IDENTIFIER */
#line 838 "expparse.y"
{
    Entity e = ENTITYcreate(yymsp[0].minor.yy0.symbol);

    if (print_objects_while_running & OBJ_ENTITY_BITS) {
    fprintf(stdout, "parse: %s (entity)\n", yymsp[0].minor.yy0.symbol->name);
    }

    PUSH_SCOPE(e, yymsp[0].minor.yy0.symbol, OBJ_ENTITY);
}
#line 2812 "expparse.c"
        break;
      case 77: /* enumeration_type ::= TOK_ENUMERATION TOK_OF nested_id_list */
#line 849 "expparse.y"
{
    int value = 0;
    Expression x;
    Symbol *tmp;
    TypeBody tb;
    tb = TYPEBODYcreate(enumeration_);
    CURRENT_SCOPE->u.type->head = 0;
    CURRENT_SCOPE->u.type->body = tb;
    tb->list = yymsp[0].minor.yy371;

    if (!CURRENT_SCOPE->symbol_table) {
        CURRENT_SCOPE->symbol_table = DICTcreate(25);
    }
    if (!PREVIOUS_SCOPE->enum_table) {
        PREVIOUS_SCOPE->enum_table = DICTcreate(25);
    }
    LISTdo_links(yymsp[0].minor.yy371, id) {
        tmp = (Symbol *)id->data;
        id->data = (Generic)(x = EXPcreate(CURRENT_SCOPE));
        x->symbol = *(tmp);
        x->u.integer = ++value;

        /* define both in enum scope and scope of */
        /* 1st visibility */
        DICT_define(CURRENT_SCOPE->symbol_table, x->symbol.name,
            (Generic)x, &x->symbol, OBJ_EXPRESSION);
        DICTdefine(PREVIOUS_SCOPE->enum_table, x->symbol.name,
            (Generic)x, &x->symbol, OBJ_EXPRESSION);
        SYMBOL_destroy(tmp);
    } LISTod;
}
#line 2847 "expparse.c"
        break;
      case 78: /* escape_statement ::= TOK_ESCAPE semicolon */
#line 882 "expparse.y"
{
    yygotominor.yy332 = STATEMENT_ESCAPE;
}
#line 2854 "expparse.c"
        break;
      case 79: /* attribute_decl ::= TOK_IDENTIFIER */
#line 887 "expparse.y"
{
    yygotominor.yy401 = EXPcreate(Type_Attribute);
    yygotominor.yy401->symbol = *yymsp[0].minor.yy0.symbol;
    SYMBOL_destroy(yymsp[0].minor.yy0.symbol);
}
#line 2863 "expparse.c"
        break;
      case 80: /* attribute_decl ::= TOK_SELF TOK_BACKSLASH TOK_IDENTIFIER TOK_DOT TOK_IDENTIFIER */
#line 894 "expparse.y"
{
    yygotominor.yy401 = EXPcreate(Type_Expression);
    yygotominor.yy401->e.op1 = EXPcreate(Type_Expression);
    yygotominor.yy401->e.op1->e.op_code = OP_GROUP;
    yygotominor.yy401->e.op1->e.op1 = EXPcreate(Type_Self);
    yygotominor.yy401->e.op1->e.op2 = EXPcreate_from_symbol(Type_Entity, yymsp[-2].minor.yy0.symbol);
    SYMBOL_destroy(yymsp[-2].minor.yy0.symbol);

    yygotominor.yy401->e.op_code = OP_DOT;
    yygotominor.yy401->e.op2 = EXPcreate_from_symbol(Type_Attribute, yymsp[0].minor.yy0.symbol);
    SYMBOL_destroy(yymsp[0].minor.yy0.symbol);
}
#line 2879 "expparse.c"
        break;
      case 81: /* attribute_decl_list ::= attribute_decl */
#line 908 "expparse.y"
{
    yygotominor.yy371 = LISTcreate();
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy401);

}
#line 2888 "expparse.c"
        break;
      case 82: /* attribute_decl_list ::= attribute_decl_list TOK_COMMA attribute_decl */
      case 114: /* expression_list ::= expression_list TOK_COMMA expression */ yytestcase(yyruleno==114);
#line 915 "expparse.y"
{
    yygotominor.yy371 = yymsp[-2].minor.yy371;
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy401);
}
#line 2897 "expparse.c"
        break;
      case 83: /* optional ::= */
#line 921 "expparse.y"
{
    yygotominor.yy252.optional = 0;
}
#line 2904 "expparse.c"
        break;
      case 84: /* optional ::= TOK_OPTIONAL */
#line 925 "expparse.y"
{
    yygotominor.yy252.optional = 1;
}
#line 2911 "expparse.c"
        break;
      case 85: /* explicit_attribute ::= attribute_decl_list TOK_COLON optional attribute_type semicolon */
#line 931 "expparse.y"
{
    Variable v;

    LISTdo_links (yymsp[-4].minor.yy371, attr)
    v = VARcreate((Expression)attr->data, yymsp[-1].minor.yy297);
    v->flags.optional = yymsp[-2].minor.yy252.optional;
    v->flags.attribute = true;
    attr->data = (Generic)v;
    LISTod;

    yygotominor.yy371 = yymsp[-4].minor.yy371;
}
#line 2927 "expparse.c"
        break;
      case 90: /* expression ::= expression TOK_AND expression */
#line 960 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_AND, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 2936 "expparse.c"
        break;
      case 91: /* expression ::= expression TOK_OR expression */
#line 966 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_OR, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 2945 "expparse.c"
        break;
      case 92: /* expression ::= expression TOK_XOR expression */
#line 972 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_XOR, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 2954 "expparse.c"
        break;
      case 93: /* expression ::= expression TOK_LESS_THAN expression */
#line 978 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_LESS_THAN, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 2963 "expparse.c"
        break;
      case 94: /* expression ::= expression TOK_GREATER_THAN expression */
#line 984 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_GREATER_THAN, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 2972 "expparse.c"
        break;
      case 95: /* expression ::= expression TOK_EQUAL expression */
#line 990 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_EQUAL, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 2981 "expparse.c"
        break;
      case 96: /* expression ::= expression TOK_LESS_EQUAL expression */
#line 996 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_LESS_EQUAL, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 2990 "expparse.c"
        break;
      case 97: /* expression ::= expression TOK_GREATER_EQUAL expression */
#line 1002 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_GREATER_EQUAL, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 2999 "expparse.c"
        break;
      case 98: /* expression ::= expression TOK_NOT_EQUAL expression */
#line 1008 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_NOT_EQUAL, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 3008 "expparse.c"
        break;
      case 99: /* expression ::= expression TOK_INST_EQUAL expression */
#line 1014 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_INST_EQUAL, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 3017 "expparse.c"
        break;
      case 100: /* expression ::= expression TOK_INST_NOT_EQUAL expression */
#line 1020 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_INST_NOT_EQUAL, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 3026 "expparse.c"
        break;
      case 101: /* expression ::= expression TOK_IN expression */
#line 1026 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_IN, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 3035 "expparse.c"
        break;
      case 102: /* expression ::= expression TOK_LIKE expression */
#line 1032 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_LIKE, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 3044 "expparse.c"
        break;
      case 103: /* expression ::= simple_expression cardinality_op simple_expression */
      case 240: /* right_curl ::= TOK_RIGHT_CURL */ yytestcase(yyruleno==240);
      case 254: /* semicolon ::= TOK_SEMICOLON */ yytestcase(yyruleno==254);
#line 1038 "expparse.y"
{
    yyerrok;
}
#line 3053 "expparse.c"
        break;
      case 105: /* simple_expression ::= simple_expression TOK_CONCAT_OP simple_expression */
#line 1048 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_CONCAT, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 3062 "expparse.c"
        break;
      case 106: /* simple_expression ::= simple_expression TOK_EXP simple_expression */
#line 1054 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_EXP, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 3071 "expparse.c"
        break;
      case 107: /* simple_expression ::= simple_expression TOK_TIMES simple_expression */
#line 1060 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_TIMES, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 3080 "expparse.c"
        break;
      case 108: /* simple_expression ::= simple_expression TOK_DIV simple_expression */
#line 1066 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_DIV, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 3089 "expparse.c"
        break;
      case 109: /* simple_expression ::= simple_expression TOK_REAL_DIV simple_expression */
#line 1072 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_REAL_DIV, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 3098 "expparse.c"
        break;
      case 110: /* simple_expression ::= simple_expression TOK_MOD simple_expression */
#line 1078 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_MOD, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 3107 "expparse.c"
        break;
      case 111: /* simple_expression ::= simple_expression TOK_PLUS simple_expression */
#line 1084 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_PLUS, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 3116 "expparse.c"
        break;
      case 112: /* simple_expression ::= simple_expression TOK_MINUS simple_expression */
#line 1090 "expparse.y"
{
    yyerrok;

    yygotominor.yy401 = BIN_EXPcreate(OP_MINUS, yymsp[-2].minor.yy401, yymsp[0].minor.yy401);
}
#line 3125 "expparse.c"
        break;
      case 115: /* var ::= */
#line 1108 "expparse.y"
{
    yygotominor.yy252.var = 1;
}
#line 3132 "expparse.c"
        break;
      case 116: /* var ::= TOK_VAR */
#line 1112 "expparse.y"
{
    yygotominor.yy252.var = 0;
}
#line 3139 "expparse.c"
        break;
      case 117: /* formal_parameter ::= var id_list TOK_COLON parameter_type */
#line 1117 "expparse.y"
{
    Symbol *tmp;
    Expression e;
    Variable v;

    yygotominor.yy371 = yymsp[-2].minor.yy371;
    LISTdo_links(yygotominor.yy371, param)
    tmp = (Symbol*)param->data;

    e = EXPcreate_from_symbol(Type_Attribute, tmp);
    v = VARcreate(e, yymsp[0].minor.yy297);
    v->flags.optional = yymsp[-3].minor.yy252.var;
    v->flags.parameter = true;
    param->data = (Generic)v;

    /* link it in to the current scope's dict */
    DICTdefine(CURRENT_SCOPE->symbol_table,
    tmp->name, (Generic)v, tmp, OBJ_VARIABLE);

    LISTod;
}
#line 3164 "expparse.c"
        break;
      case 118: /* formal_parameter_list ::= */
      case 179: /* inverse_clause ::= */ yytestcase(yyruleno==179);
      case 329: /* where_rule_OPT ::= */ yytestcase(yyruleno==329);
#line 1140 "expparse.y"
{
    yygotominor.yy371 = LIST_NULL;
}
#line 3173 "expparse.c"
        break;
      case 119: /* formal_parameter_list ::= TOK_LEFT_PAREN formal_parameter_rep TOK_RIGHT_PAREN */
#line 1145 "expparse.y"
{
    yygotominor.yy371 = yymsp[-1].minor.yy371;

}
#line 3181 "expparse.c"
        break;
      case 120: /* formal_parameter_rep ::= formal_parameter */
#line 1151 "expparse.y"
{
    yygotominor.yy371 = yymsp[0].minor.yy371;

}
#line 3189 "expparse.c"
        break;
      case 121: /* formal_parameter_rep ::= formal_parameter_rep semicolon formal_parameter */
#line 1157 "expparse.y"
{
    yygotominor.yy371 = yymsp[-2].minor.yy371;
    LISTadd_all(yygotominor.yy371, yymsp[0].minor.yy371);
}
#line 3197 "expparse.c"
        break;
      case 126: /* function_call ::= function_id actual_parameters */
#line 1182 "expparse.y"
{
    yygotominor.yy401 = EXPcreate(Type_Funcall);
    yygotominor.yy401->symbol = *yymsp[-1].minor.yy275;
    SYMBOL_destroy(yymsp[-1].minor.yy275);
    yygotominor.yy401->u.funcall.list = yymsp[0].minor.yy371;
}
#line 3207 "expparse.c"
        break;
      case 127: /* function_decl ::= function_header action_body TOK_END_FUNCTION semicolon */
#line 1191 "expparse.y"
{
    FUNCput_body(CURRENT_SCOPE, yymsp[-2].minor.yy371);
    ALGput_full_text(CURRENT_SCOPE, yymsp[-3].minor.yy507, SCANtell());
    POP_SCOPE();
}
#line 3216 "expparse.c"
        break;
      case 128: /* function_header ::= fh_lineno fh_push_scope fh_plist TOK_COLON parameter_type semicolon */
#line 1199 "expparse.y"
{ 
    Function f = CURRENT_SCOPE;

    f->u.func->return_type = yymsp[-1].minor.yy297;
    yygotominor.yy507 = yymsp[-5].minor.yy507;
}
#line 3226 "expparse.c"
        break;
      case 129: /* fh_lineno ::= TOK_FUNCTION */
      case 218: /* ph_get_line ::= */ yytestcase(yyruleno==218);
      case 247: /* rh_get_line ::= */ yytestcase(yyruleno==247);
#line 1207 "expparse.y"
{
    yygotominor.yy507 = SCANtell();
}
#line 3235 "expparse.c"
        break;
      case 130: /* fh_push_scope ::= TOK_IDENTIFIER */
#line 1212 "expparse.y"
{
    Function f = ALGcreate(OBJ_FUNCTION);
    tag_count = 0;
    if (print_objects_while_running & OBJ_FUNCTION_BITS) {
        fprintf(stdout, "parse: %s (function)\n", yymsp[0].minor.yy0.symbol->name);
    }
    PUSH_SCOPE(f, yymsp[0].minor.yy0.symbol, OBJ_FUNCTION);
}
#line 3247 "expparse.c"
        break;
      case 131: /* fh_plist ::= formal_parameter_list */
#line 1222 "expparse.y"
{
    Function f = CURRENT_SCOPE;
    f->u.func->parameters = yymsp[0].minor.yy371;
    f->u.func->pcount = LISTget_length(yymsp[0].minor.yy371);
    f->u.func->tag_count = tag_count;
    tag_count = -1;     /* done with parameters, no new tags can be defined */
}
#line 3258 "expparse.c"
        break;
      case 132: /* function_id ::= TOK_IDENTIFIER */
      case 219: /* procedure_id ::= TOK_IDENTIFIER */ yytestcase(yyruleno==219);
      case 220: /* procedure_id ::= TOK_BUILTIN_PROCEDURE */ yytestcase(yyruleno==220);
#line 1231 "expparse.y"
{
    yygotominor.yy275 = yymsp[0].minor.yy0.symbol;
}
#line 3267 "expparse.c"
        break;
      case 133: /* function_id ::= TOK_BUILTIN_FUNCTION */
#line 1235 "expparse.y"
{
    yygotominor.yy275 = yymsp[0].minor.yy0.symbol;

}
#line 3275 "expparse.c"
        break;
      case 134: /* conformant_aggregation ::= aggregate_type */
#line 1241 "expparse.y"
{
    yygotominor.yy477 = yymsp[0].minor.yy477;

}
#line 3283 "expparse.c"
        break;
      case 135: /* conformant_aggregation ::= TOK_ARRAY TOK_OF optional_or_unique parameter_type */
#line 1247 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(array_);
    yygotominor.yy477->flags.optional = yymsp[-1].minor.yy252.optional;
    yygotominor.yy477->flags.unique = yymsp[-1].minor.yy252.unique;
    yygotominor.yy477->base = yymsp[0].minor.yy297;
}
#line 3293 "expparse.c"
        break;
      case 136: /* conformant_aggregation ::= TOK_ARRAY bound_spec TOK_OF optional_or_unique parameter_type */
#line 1255 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(array_);
    yygotominor.yy477->flags.optional = yymsp[-1].minor.yy252.optional;
    yygotominor.yy477->flags.unique = yymsp[-1].minor.yy252.unique;
    yygotominor.yy477->base = yymsp[0].minor.yy297;
    yygotominor.yy477->upper = yymsp[-3].minor.yy253.upper_limit;
    yygotominor.yy477->lower = yymsp[-3].minor.yy253.lower_limit;
}
#line 3305 "expparse.c"
        break;
      case 137: /* conformant_aggregation ::= TOK_BAG TOK_OF parameter_type */
#line 1264 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(bag_);
    yygotominor.yy477->base = yymsp[0].minor.yy297;

}
#line 3314 "expparse.c"
        break;
      case 139: /* conformant_aggregation ::= TOK_LIST TOK_OF unique parameter_type */
#line 1277 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(list_);
    yygotominor.yy477->flags.unique = yymsp[-1].minor.yy252.unique;
    yygotominor.yy477->base = yymsp[0].minor.yy297;

}
#line 3324 "expparse.c"
        break;
      case 140: /* conformant_aggregation ::= TOK_LIST bound_spec TOK_OF unique parameter_type */
#line 1285 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(list_);
    yygotominor.yy477->base = yymsp[0].minor.yy297;
    yygotominor.yy477->flags.unique = yymsp[-1].minor.yy252.unique;
    yygotominor.yy477->upper = yymsp[-3].minor.yy253.upper_limit;
    yygotominor.yy477->lower = yymsp[-3].minor.yy253.lower_limit;
}
#line 3335 "expparse.c"
        break;
      case 141: /* conformant_aggregation ::= TOK_SET TOK_OF parameter_type */
      case 256: /* set_type ::= TOK_SET TOK_OF attribute_type */ yytestcase(yyruleno==256);
#line 1293 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(set_);
    yygotominor.yy477->base = yymsp[0].minor.yy297;
}
#line 3344 "expparse.c"
        break;
      case 142: /* conformant_aggregation ::= TOK_SET bound_spec TOK_OF parameter_type */
#line 1298 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(set_);
    yygotominor.yy477->base = yymsp[0].minor.yy297;
    yygotominor.yy477->upper = yymsp[-2].minor.yy253.upper_limit;
    yygotominor.yy477->lower = yymsp[-2].minor.yy253.lower_limit;
}
#line 3354 "expparse.c"
        break;
      case 143: /* generic_type ::= TOK_GENERIC */
#line 1306 "expparse.y"
{
    yygotominor.yy297 = Type_Generic;

    if (tag_count < 0) {
        Symbol sym;
        sym.line = yylineno;
        sym.filename = current_filename;
        ERRORreport_with_symbol(ERROR_unlabelled_param_type, &sym,
        CURRENT_SCOPE_NAME);
    }
}
#line 3369 "expparse.c"
        break;
      case 144: /* generic_type ::= TOK_GENERIC TOK_COLON TOK_IDENTIFIER */
#line 1318 "expparse.y"
{
    TypeBody g = TYPEBODYcreate(generic_);
    yygotominor.yy297 = TYPEcreate_from_body_anonymously(g);

    SCOPEadd_super(yygotominor.yy297);

    g->tag = TYPEcreate_user_defined_tag(yygotominor.yy297, CURRENT_SCOPE, yymsp[0].minor.yy0.symbol);
    if (g->tag) {
        SCOPEadd_super(g->tag);
    }
}
#line 3384 "expparse.c"
        break;
      case 145: /* id_list ::= TOK_IDENTIFIER */
#line 1331 "expparse.y"
{
    yygotominor.yy371 = LISTcreate();
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy0.symbol);

}
#line 3393 "expparse.c"
        break;
      case 146: /* id_list ::= id_list TOK_COMMA TOK_IDENTIFIER */
#line 1337 "expparse.y"
{
    yyerrok;

    yygotominor.yy371 = yymsp[-2].minor.yy371;
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy0.symbol);
}
#line 3403 "expparse.c"
        break;
      case 147: /* identifier ::= TOK_SELF */
#line 1345 "expparse.y"
{
    yygotominor.yy401 = EXPcreate(Type_Self);
}
#line 3410 "expparse.c"
        break;
      case 148: /* identifier ::= TOK_QUESTION_MARK */
#line 1349 "expparse.y"
{
    yygotominor.yy401 = LITERAL_INFINITY;
}
#line 3417 "expparse.c"
        break;
      case 149: /* identifier ::= TOK_IDENTIFIER */
#line 1353 "expparse.y"
{
    yygotominor.yy401 = EXPcreate(Type_Identifier);
    yygotominor.yy401->symbol = *(yymsp[0].minor.yy0.symbol);
    SYMBOL_destroy(yymsp[0].minor.yy0.symbol);
}
#line 3426 "expparse.c"
        break;
      case 150: /* if_statement ::= TOK_IF expression TOK_THEN statement_rep TOK_END_IF semicolon */
#line 1361 "expparse.y"
{
    yygotominor.yy332 = CONDcreate(yymsp[-4].minor.yy401, yymsp[-2].minor.yy371, STATEMENT_LIST_NULL);
}
#line 3433 "expparse.c"
        break;
      case 151: /* if_statement ::= TOK_IF expression TOK_THEN statement_rep TOK_ELSE statement_rep TOK_END_IF semicolon */
#line 1366 "expparse.y"
{
    yygotominor.yy332 = CONDcreate(yymsp[-6].minor.yy401, yymsp[-4].minor.yy371, yymsp[-2].minor.yy371);
}
#line 3440 "expparse.c"
        break;
      case 152: /* include_directive ::= TOK_INCLUDE TOK_STRING_LITERAL semicolon */
#line 1371 "expparse.y"
{
    SCANinclude_file(yymsp[-1].minor.yy0.string);
}
#line 3447 "expparse.c"
        break;
      case 153: /* increment_control ::= TOK_IDENTIFIER TOK_ASSIGNMENT expression TOK_TO expression by_expression */
#line 1377 "expparse.y"
{
    Increment i = INCR_CTLcreate(yymsp[-5].minor.yy0.symbol, yymsp[-3].minor.yy401, yymsp[-1].minor.yy401, yymsp[0].minor.yy401);

    /* scope doesn't really have/need a name, I suppose */
    /* naming it by the iterator variable is fine */

    PUSH_SCOPE(i, (Symbol *)0, OBJ_INCREMENT);
}
#line 3459 "expparse.c"
        break;
      case 155: /* rename ::= TOK_IDENTIFIER */
#line 1392 "expparse.y"
{
    (*interface_func)(CURRENT_SCOPE, interface_schema, yymsp[0].minor.yy0, yymsp[0].minor.yy0);
}
#line 3466 "expparse.c"
        break;
      case 156: /* rename ::= TOK_IDENTIFIER TOK_AS TOK_IDENTIFIER */
#line 1396 "expparse.y"
{
    (*interface_func)(CURRENT_SCOPE, interface_schema, yymsp[-2].minor.yy0, yymsp[0].minor.yy0);
}
#line 3473 "expparse.c"
        break;
      case 158: /* rename_list ::= rename_list TOK_COMMA rename */
      case 161: /* reference_clause ::= reference_head parened_rename_list semicolon */ yytestcase(yyruleno==161);
      case 164: /* use_clause ::= use_head parened_rename_list semicolon */ yytestcase(yyruleno==164);
      case 249: /* schema_body ::= interface_specification_list constant_decl block_list */ yytestcase(yyruleno==249);
      case 295: /* type_decl ::= td_start TOK_END_TYPE semicolon */ yytestcase(yyruleno==295);
#line 1405 "expparse.y"
{
    yygotominor.yy0 = yymsp[-2].minor.yy0;
}
#line 3484 "expparse.c"
        break;
      case 160: /* reference_clause ::= TOK_REFERENCE TOK_FROM TOK_IDENTIFIER semicolon */
#line 1412 "expparse.y"
{
    if (!CURRENT_SCHEMA->ref_schemas) {
        CURRENT_SCHEMA->ref_schemas = LISTcreate();
    }

    LISTadd_last(CURRENT_SCHEMA->ref_schemas, (Generic)yymsp[-1].minor.yy0.symbol);
}
#line 3495 "expparse.c"
        break;
      case 162: /* reference_head ::= TOK_REFERENCE TOK_FROM TOK_IDENTIFIER */
#line 1425 "expparse.y"
{
    interface_schema = yymsp[0].minor.yy0.symbol;
    interface_func = SCHEMAadd_reference;
}
#line 3503 "expparse.c"
        break;
      case 163: /* use_clause ::= TOK_USE TOK_FROM TOK_IDENTIFIER semicolon */
#line 1431 "expparse.y"
{
    if (!CURRENT_SCHEMA->use_schemas) {
        CURRENT_SCHEMA->use_schemas = LISTcreate();
    }

    LISTadd_last(CURRENT_SCHEMA->use_schemas, (Generic)yymsp[-1].minor.yy0.symbol);
}
#line 3514 "expparse.c"
        break;
      case 165: /* use_head ::= TOK_USE TOK_FROM TOK_IDENTIFIER */
#line 1444 "expparse.y"
{
    interface_schema = yymsp[0].minor.yy0.symbol;
    interface_func = SCHEMAadd_use;
}
#line 3522 "expparse.c"
        break;
      case 170: /* interval ::= TOK_LEFT_CURL simple_expression rel_op simple_expression rel_op simple_expression right_curl */
#line 1467 "expparse.y"
{
    Expression    tmp1, tmp2;

    yygotominor.yy401 = (Expression)0;
    tmp1 = BIN_EXPcreate(yymsp[-4].minor.yy126, yymsp[-5].minor.yy401, yymsp[-3].minor.yy401);
    tmp2 = BIN_EXPcreate(yymsp[-2].minor.yy126, yymsp[-3].minor.yy401, yymsp[-1].minor.yy401);
    yygotominor.yy401 = BIN_EXPcreate(OP_AND, tmp1, tmp2);
}
#line 3534 "expparse.c"
        break;
      case 171: /* set_or_bag_of_entity ::= defined_type */
      case 289: /* type ::= defined_type */ yytestcase(yyruleno==289);
#line 1479 "expparse.y"
{
    yygotominor.yy378.type = yymsp[0].minor.yy297;
    yygotominor.yy378.body = 0;
}
#line 3543 "expparse.c"
        break;
      case 172: /* set_or_bag_of_entity ::= TOK_SET TOK_OF defined_type */
#line 1484 "expparse.y"
{
    yygotominor.yy378.type = 0;
    yygotominor.yy378.body = TYPEBODYcreate(set_);
    yygotominor.yy378.body->base = yymsp[0].minor.yy297;

}
#line 3553 "expparse.c"
        break;
      case 173: /* set_or_bag_of_entity ::= TOK_SET bound_spec TOK_OF defined_type */
#line 1491 "expparse.y"
{
    yygotominor.yy378.type = 0; 
    yygotominor.yy378.body = TYPEBODYcreate(set_);
    yygotominor.yy378.body->base = yymsp[0].minor.yy297;
    yygotominor.yy378.body->upper = yymsp[-2].minor.yy253.upper_limit;
    yygotominor.yy378.body->lower = yymsp[-2].minor.yy253.lower_limit;
}
#line 3564 "expparse.c"
        break;
      case 174: /* set_or_bag_of_entity ::= TOK_BAG bound_spec TOK_OF defined_type */
#line 1499 "expparse.y"
{
    yygotominor.yy378.type = 0;
    yygotominor.yy378.body = TYPEBODYcreate(bag_);
    yygotominor.yy378.body->base = yymsp[0].minor.yy297;
    yygotominor.yy378.body->upper = yymsp[-2].minor.yy253.upper_limit;
    yygotominor.yy378.body->lower = yymsp[-2].minor.yy253.lower_limit;
}
#line 3575 "expparse.c"
        break;
      case 175: /* set_or_bag_of_entity ::= TOK_BAG TOK_OF defined_type */
#line 1507 "expparse.y"
{
    yygotominor.yy378.type = 0;
    yygotominor.yy378.body = TYPEBODYcreate(bag_);
    yygotominor.yy378.body->base = yymsp[0].minor.yy297;
}
#line 3584 "expparse.c"
        break;
      case 178: /* inverse_attr ::= TOK_IDENTIFIER TOK_COLON set_or_bag_of_entity TOK_FOR TOK_IDENTIFIER semicolon */
#line 1526 "expparse.y"
{
    Expression e = EXPcreate(Type_Attribute);

    e->symbol = *yymsp[-5].minor.yy0.symbol;
    SYMBOL_destroy(yymsp[-5].minor.yy0.symbol);

    if (yymsp[-3].minor.yy378.type) {
        yygotominor.yy91 = VARcreate(e, yymsp[-3].minor.yy378.type);
    } else {
        Type t = TYPEcreate_from_body_anonymously(yymsp[-3].minor.yy378.body);
        SCOPEadd_super(t);
        yygotominor.yy91 = VARcreate(e, t);
    }

    yygotominor.yy91->flags.attribute = true;
    yygotominor.yy91->inverse_symbol = yymsp[-1].minor.yy0.symbol;
}
#line 3605 "expparse.c"
        break;
      case 182: /* list_type ::= TOK_LIST bound_spec TOK_OF unique attribute_type */
#line 1562 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(list_);
    yygotominor.yy477->base = yymsp[0].minor.yy297;
    yygotominor.yy477->flags.unique = yymsp[-1].minor.yy252.unique;
    yygotominor.yy477->lower = yymsp[-3].minor.yy253.lower_limit;
    yygotominor.yy477->upper = yymsp[-3].minor.yy253.upper_limit;
}
#line 3616 "expparse.c"
        break;
      case 183: /* list_type ::= TOK_LIST TOK_OF unique attribute_type */
#line 1570 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(list_);
    yygotominor.yy477->base = yymsp[0].minor.yy297;
    yygotominor.yy477->flags.unique = yymsp[-1].minor.yy252.unique;
}
#line 3625 "expparse.c"
        break;
      case 184: /* literal ::= TOK_INTEGER_LITERAL */
#line 1577 "expparse.y"
{
    if (yymsp[0].minor.yy0.iVal == 0) {
        yygotominor.yy401 = LITERAL_ZERO;
    } else if (yymsp[0].minor.yy0.iVal == 1) {
    yygotominor.yy401 = LITERAL_ONE;
    } else {
    yygotominor.yy401 = EXPcreate_simple(Type_Integer);
    yygotominor.yy401->u.integer = (int)yymsp[0].minor.yy0.iVal;
    resolved_all(yygotominor.yy401);
    }
}
#line 3640 "expparse.c"
        break;
      case 185: /* literal ::= TOK_REAL_LITERAL */
#line 1589 "expparse.y"
{
    if( ( abs( yymsp[0].minor.yy0.rVal ) <= FLT_EPSILON ) && ( abs( yymsp[0].minor.yy0.rVal ) > 0 ) ) {
        Symbol sym;
        sym.line = yylineno;
        sym.filename = current_filename;
        ERRORreport_with_symbol(ERROR_warn_small_real, &sym, yymsp[0].minor.yy0.rVal );
    }
    if( abs( yymsp[0].minor.yy0.rVal ) < DBL_EPSILON ) {
        yygotominor.yy401 = LITERAL_ZERO;
    } else {
        yygotominor.yy401 = EXPcreate_simple(Type_Real);
        yygotominor.yy401->u.real = yymsp[0].minor.yy0.rVal;
        resolved_all(yygotominor.yy401);
    }
}
#line 3659 "expparse.c"
        break;
      case 186: /* literal ::= TOK_STRING_LITERAL */
#line 1605 "expparse.y"
{
    yygotominor.yy401 = EXPcreate_simple(Type_String);
    yygotominor.yy401->symbol.name = yymsp[0].minor.yy0.string;
    resolved_all(yygotominor.yy401);
}
#line 3668 "expparse.c"
        break;
      case 187: /* literal ::= TOK_STRING_LITERAL_ENCODED */
#line 1611 "expparse.y"
{
    yygotominor.yy401 = EXPcreate_simple(Type_String_Encoded);
    yygotominor.yy401->symbol.name = yymsp[0].minor.yy0.string;
    resolved_all(yygotominor.yy401);
}
#line 3677 "expparse.c"
        break;
      case 188: /* literal ::= TOK_LOGICAL_LITERAL */
#line 1617 "expparse.y"
{
    yygotominor.yy401 = EXPcreate_simple(Type_Logical);
    yygotominor.yy401->u.logical = yymsp[0].minor.yy0.logical;
    resolved_all(yygotominor.yy401);
}
#line 3686 "expparse.c"
        break;
      case 189: /* literal ::= TOK_BINARY_LITERAL */
#line 1623 "expparse.y"
{
    yygotominor.yy401 = EXPcreate_simple(Type_Binary);
    yygotominor.yy401->symbol.name = yymsp[0].minor.yy0.binary;
    resolved_all(yygotominor.yy401);
}
#line 3695 "expparse.c"
        break;
      case 192: /* local_variable ::= id_list TOK_COLON parameter_type semicolon */
#line 1639 "expparse.y"
{
    Expression e;
    Variable v;
    LISTdo(yymsp[-3].minor.yy371, sym, Symbol *)

    /* convert symbol to name-expression */

    e = EXPcreate(Type_Attribute);
    e->symbol = *sym; SYMBOL_destroy(sym);
    v = VARcreate(e, yymsp[-1].minor.yy297);
    DICTdefine(CURRENT_SCOPE->symbol_table, e->symbol.name, (Generic)v,
    &e->symbol, OBJ_VARIABLE);
    LISTod;
    LISTfree(yymsp[-3].minor.yy371);
}
#line 3714 "expparse.c"
        break;
      case 193: /* local_variable ::= id_list TOK_COLON parameter_type local_initializer semicolon */
#line 1656 "expparse.y"
{
    Expression e;
    Variable v;
    LISTdo(yymsp[-4].minor.yy371, sym, Symbol *)
    e = EXPcreate(Type_Attribute);
    e->symbol = *sym; SYMBOL_destroy(sym);
    v = VARcreate(e, yymsp[-2].minor.yy297);
    v->initializer = yymsp[-1].minor.yy401;
    DICTdefine(CURRENT_SCOPE->symbol_table, e->symbol.name, (Generic)v,
    &e->symbol, OBJ_VARIABLE);
    LISTod;
    LISTfree(yymsp[-4].minor.yy371);
}
#line 3731 "expparse.c"
        break;
      case 197: /* allow_generic_types ::= */
#line 1679 "expparse.y"
{
    tag_count = 0; /* don't signal an error if we find a generic_type */
}
#line 3738 "expparse.c"
        break;
      case 198: /* disallow_generic_types ::= */
#line 1684 "expparse.y"
{
    tag_count = -1; /* signal an error if we find a generic_type */
}
#line 3745 "expparse.c"
        break;
      case 199: /* defined_type ::= TOK_IDENTIFIER */
#line 1689 "expparse.y"
{
    yygotominor.yy297 = TYPEcreate_name(yymsp[0].minor.yy0.symbol);
    SCOPEadd_super(yygotominor.yy297);
    SYMBOL_destroy(yymsp[0].minor.yy0.symbol);
}
#line 3754 "expparse.c"
        break;
      case 200: /* defined_type_list ::= defined_type */
#line 1696 "expparse.y"
{
    yygotominor.yy371 = LISTcreate();
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy297);

}
#line 3763 "expparse.c"
        break;
      case 201: /* defined_type_list ::= defined_type_list TOK_COMMA defined_type */
#line 1702 "expparse.y"
{
    yygotominor.yy371 = yymsp[-2].minor.yy371;
    LISTadd_last(yygotominor.yy371,
    (Generic)yymsp[0].minor.yy297);
}
#line 3772 "expparse.c"
        break;
      case 204: /* optional_or_unique ::= */
#line 1719 "expparse.y"
{
    yygotominor.yy252.unique = 0;
    yygotominor.yy252.optional = 0;
}
#line 3780 "expparse.c"
        break;
      case 205: /* optional_or_unique ::= TOK_OPTIONAL */
#line 1724 "expparse.y"
{
    yygotominor.yy252.unique = 0;
    yygotominor.yy252.optional = 1;
}
#line 3788 "expparse.c"
        break;
      case 206: /* optional_or_unique ::= TOK_UNIQUE */
#line 1729 "expparse.y"
{
    yygotominor.yy252.unique = 1;
    yygotominor.yy252.optional = 0;
}
#line 3796 "expparse.c"
        break;
      case 207: /* optional_or_unique ::= TOK_OPTIONAL TOK_UNIQUE */
      case 208: /* optional_or_unique ::= TOK_UNIQUE TOK_OPTIONAL */ yytestcase(yyruleno==208);
#line 1734 "expparse.y"
{
    yygotominor.yy252.unique = 1;
    yygotominor.yy252.optional = 1;
}
#line 3805 "expparse.c"
        break;
      case 209: /* optional_fixed ::= */
#line 1745 "expparse.y"
{
    yygotominor.yy252.fixed = 0;
}
#line 3812 "expparse.c"
        break;
      case 210: /* optional_fixed ::= TOK_FIXED */
#line 1749 "expparse.y"
{
    yygotominor.yy252.fixed = 1;
}
#line 3819 "expparse.c"
        break;
      case 211: /* precision_spec ::= */
#line 1754 "expparse.y"
{
    yygotominor.yy401 = (Expression)0;
}
#line 3826 "expparse.c"
        break;
      case 212: /* precision_spec ::= TOK_LEFT_PAREN expression TOK_RIGHT_PAREN */
      case 304: /* unary_expression ::= TOK_LEFT_PAREN expression TOK_RIGHT_PAREN */ yytestcase(yyruleno==304);
#line 1758 "expparse.y"
{
    yygotominor.yy401 = yymsp[-1].minor.yy401;
}
#line 3834 "expparse.c"
        break;
      case 213: /* proc_call_statement ::= procedure_id actual_parameters semicolon */
#line 1768 "expparse.y"
{
    yygotominor.yy332 = PCALLcreate(yymsp[-1].minor.yy371);
    yygotominor.yy332->symbol = *(yymsp[-2].minor.yy275);
}
#line 3842 "expparse.c"
        break;
      case 214: /* proc_call_statement ::= procedure_id semicolon */
#line 1773 "expparse.y"
{
    yygotominor.yy332 = PCALLcreate((Linked_List)0);
    yygotominor.yy332->symbol = *(yymsp[-1].minor.yy275);
}
#line 3850 "expparse.c"
        break;
      case 215: /* procedure_decl ::= procedure_header action_body TOK_END_PROCEDURE semicolon */
#line 1780 "expparse.y"
{
    PROCput_body(CURRENT_SCOPE, yymsp[-2].minor.yy371);
    ALGput_full_text(CURRENT_SCOPE, yymsp[-3].minor.yy507, SCANtell());
    POP_SCOPE();
}
#line 3859 "expparse.c"
        break;
      case 216: /* procedure_header ::= TOK_PROCEDURE ph_get_line ph_push_scope formal_parameter_list semicolon */
#line 1788 "expparse.y"
{
    Procedure p = CURRENT_SCOPE;
    p->u.proc->parameters = yymsp[-1].minor.yy371;
    p->u.proc->pcount = LISTget_length(yymsp[-1].minor.yy371);
    p->u.proc->tag_count = tag_count;
    tag_count = -1;    /* done with parameters, no new tags can be defined */
    yygotominor.yy507 = yymsp[-3].minor.yy507;
}
#line 3871 "expparse.c"
        break;
      case 217: /* ph_push_scope ::= TOK_IDENTIFIER */
#line 1798 "expparse.y"
{
    Procedure p = ALGcreate(OBJ_PROCEDURE);
    tag_count = 0;

    if (print_objects_while_running & OBJ_PROCEDURE_BITS) {
    fprintf(stdout, "parse: %s (procedure)\n", yymsp[0].minor.yy0.symbol->name);
    }

    PUSH_SCOPE(p, yymsp[0].minor.yy0.symbol, OBJ_PROCEDURE);
}
#line 3885 "expparse.c"
        break;
      case 221: /* group_ref ::= TOK_BACKSLASH TOK_IDENTIFIER */
#line 1824 "expparse.y"
{
    yygotominor.yy401 = BIN_EXPcreate(OP_GROUP, (Expression)0, (Expression)0);
    yygotominor.yy401->e.op2 = EXPcreate(Type_Identifier);
    yygotominor.yy401->e.op2->symbol = *yymsp[0].minor.yy0.symbol;
    SYMBOL_destroy(yymsp[0].minor.yy0.symbol);
}
#line 3895 "expparse.c"
        break;
      case 222: /* qualifier ::= TOK_DOT TOK_IDENTIFIER */
#line 1832 "expparse.y"
{
    yygotominor.yy46.expr = yygotominor.yy46.first = BIN_EXPcreate(OP_DOT, (Expression)0, (Expression)0);
    yygotominor.yy46.expr->e.op2 = EXPcreate(Type_Identifier);
    yygotominor.yy46.expr->e.op2->symbol = *yymsp[0].minor.yy0.symbol;
    SYMBOL_destroy(yymsp[0].minor.yy0.symbol);
}
#line 3905 "expparse.c"
        break;
      case 223: /* qualifier ::= TOK_BACKSLASH TOK_IDENTIFIER */
#line 1839 "expparse.y"
{
    yygotominor.yy46.expr = yygotominor.yy46.first = BIN_EXPcreate(OP_GROUP, (Expression)0, (Expression)0);
    yygotominor.yy46.expr->e.op2 = EXPcreate(Type_Identifier);
    yygotominor.yy46.expr->e.op2->symbol = *yymsp[0].minor.yy0.symbol;
    SYMBOL_destroy(yymsp[0].minor.yy0.symbol);
}
#line 3915 "expparse.c"
        break;
      case 224: /* qualifier ::= TOK_LEFT_BRACKET simple_expression TOK_RIGHT_BRACKET */
#line 1848 "expparse.y"
{
    yygotominor.yy46.expr = yygotominor.yy46.first = BIN_EXPcreate(OP_ARRAY_ELEMENT, (Expression)0,
    (Expression)0);
    yygotominor.yy46.expr->e.op2 = yymsp[-1].minor.yy401;
}
#line 3924 "expparse.c"
        break;
      case 225: /* qualifier ::= TOK_LEFT_BRACKET simple_expression TOK_COLON simple_expression TOK_RIGHT_BRACKET */
#line 1857 "expparse.y"
{
    yygotominor.yy46.expr = yygotominor.yy46.first = TERN_EXPcreate(OP_SUBCOMPONENT, (Expression)0,
    (Expression)0, (Expression)0);
    yygotominor.yy46.expr->e.op2 = yymsp[-3].minor.yy401;
    yygotominor.yy46.expr->e.op3 = yymsp[-1].minor.yy401;
}
#line 3934 "expparse.c"
        break;
      case 226: /* query_expression ::= query_start expression TOK_RIGHT_PAREN */
#line 1865 "expparse.y"
{
    yygotominor.yy401 = yymsp[-2].minor.yy401;
    yygotominor.yy401->u.query->expression = yymsp[-1].minor.yy401;
    POP_SCOPE();
}
#line 3943 "expparse.c"
        break;
      case 227: /* query_start ::= TOK_QUERY TOK_LEFT_PAREN TOK_IDENTIFIER TOK_ALL_IN expression TOK_SUCH_THAT */
#line 1873 "expparse.y"
{
    yygotominor.yy401 = QUERYcreate(yymsp[-3].minor.yy0.symbol, yymsp[-1].minor.yy401);
    SYMBOL_destroy(yymsp[-3].minor.yy0.symbol);
    PUSH_SCOPE(yygotominor.yy401->u.query->scope, (Symbol *)0, OBJ_QUERY);
}
#line 3952 "expparse.c"
        break;
      case 228: /* rel_op ::= TOK_LESS_THAN */
#line 1880 "expparse.y"
{
    yygotominor.yy126 = OP_LESS_THAN;
}
#line 3959 "expparse.c"
        break;
      case 229: /* rel_op ::= TOK_GREATER_THAN */
#line 1884 "expparse.y"
{
    yygotominor.yy126 = OP_GREATER_THAN;
}
#line 3966 "expparse.c"
        break;
      case 230: /* rel_op ::= TOK_EQUAL */
#line 1888 "expparse.y"
{
    yygotominor.yy126 = OP_EQUAL;
}
#line 3973 "expparse.c"
        break;
      case 231: /* rel_op ::= TOK_LESS_EQUAL */
#line 1892 "expparse.y"
{
    yygotominor.yy126 = OP_LESS_EQUAL;
}
#line 3980 "expparse.c"
        break;
      case 232: /* rel_op ::= TOK_GREATER_EQUAL */
#line 1896 "expparse.y"
{
    yygotominor.yy126 = OP_GREATER_EQUAL;
}
#line 3987 "expparse.c"
        break;
      case 233: /* rel_op ::= TOK_NOT_EQUAL */
#line 1900 "expparse.y"
{
    yygotominor.yy126 = OP_NOT_EQUAL;
}
#line 3994 "expparse.c"
        break;
      case 234: /* rel_op ::= TOK_INST_EQUAL */
#line 1904 "expparse.y"
{
    yygotominor.yy126 = OP_INST_EQUAL;
}
#line 4001 "expparse.c"
        break;
      case 235: /* rel_op ::= TOK_INST_NOT_EQUAL */
#line 1908 "expparse.y"
{
    yygotominor.yy126 = OP_INST_NOT_EQUAL;
}
#line 4008 "expparse.c"
        break;
      case 236: /* repeat_statement ::= TOK_REPEAT increment_control while_control until_control semicolon statement_rep TOK_END_REPEAT semicolon */
#line 1916 "expparse.y"
{
    yygotominor.yy332 = LOOPcreate(CURRENT_SCOPE, yymsp[-5].minor.yy401, yymsp[-4].minor.yy401, yymsp[-2].minor.yy371);

    /* matching PUSH_SCOPE is in increment_control */
    POP_SCOPE();
}
#line 4018 "expparse.c"
        break;
      case 237: /* repeat_statement ::= TOK_REPEAT while_control until_control semicolon statement_rep TOK_END_REPEAT semicolon */
#line 1924 "expparse.y"
{
    yygotominor.yy332 = LOOPcreate((struct Scope_ *)0, yymsp[-5].minor.yy401, yymsp[-4].minor.yy401, yymsp[-2].minor.yy371);
}
#line 4025 "expparse.c"
        break;
      case 238: /* return_statement ::= TOK_RETURN semicolon */
#line 1929 "expparse.y"
{
    yygotominor.yy332 = RETcreate((Expression)0);
}
#line 4032 "expparse.c"
        break;
      case 239: /* return_statement ::= TOK_RETURN TOK_LEFT_PAREN expression TOK_RIGHT_PAREN semicolon */
#line 1934 "expparse.y"
{
    yygotominor.yy332 = RETcreate(yymsp[-2].minor.yy401);
}
#line 4039 "expparse.c"
        break;
      case 241: /* rule_decl ::= rule_header action_body where_rule TOK_END_RULE semicolon */
#line 1945 "expparse.y"
{
    RULEput_body(CURRENT_SCOPE, yymsp[-3].minor.yy371);
    RULEput_where(CURRENT_SCOPE, yymsp[-2].minor.yy371);
    ALGput_full_text(CURRENT_SCOPE, yymsp[-4].minor.yy507, SCANtell());
    POP_SCOPE();
}
#line 4049 "expparse.c"
        break;
      case 242: /* rule_formal_parameter ::= TOK_IDENTIFIER */
#line 1953 "expparse.y"
{
    Expression e;
    Type t;

    /* it's true that we know it will be an entity_ type later */
    TypeBody tb = TYPEBODYcreate(set_);
    tb->base = TYPEcreate_name(yymsp[0].minor.yy0.symbol);
    SCOPEadd_super(tb->base);
    t = TYPEcreate_from_body_anonymously(tb);
    SCOPEadd_super(t);
    e = EXPcreate_from_symbol(t, yymsp[0].minor.yy0.symbol);
    yygotominor.yy91 = VARcreate(e, t);
    yygotominor.yy91->flags.attribute = true;
    yygotominor.yy91->flags.parameter = true;

    /* link it in to the current scope's dict */
    DICTdefine(CURRENT_SCOPE->symbol_table, yymsp[0].minor.yy0.symbol->name, (Generic)yygotominor.yy91,
    yymsp[0].minor.yy0.symbol, OBJ_VARIABLE);
}
#line 4072 "expparse.c"
        break;
      case 243: /* rule_formal_parameter_list ::= rule_formal_parameter */
#line 1974 "expparse.y"
{
    yygotominor.yy371 = LISTcreate();
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy91); 
}
#line 4080 "expparse.c"
        break;
      case 244: /* rule_formal_parameter_list ::= rule_formal_parameter_list TOK_COMMA rule_formal_parameter */
#line 1980 "expparse.y"
{
    yygotominor.yy371 = yymsp[-2].minor.yy371;
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy91);
}
#line 4088 "expparse.c"
        break;
      case 245: /* rule_header ::= rh_start rule_formal_parameter_list TOK_RIGHT_PAREN semicolon */
#line 1987 "expparse.y"
{
    CURRENT_SCOPE->u.rule->parameters = yymsp[-2].minor.yy371;

    yygotominor.yy507 = yymsp[-3].minor.yy507;
}
#line 4097 "expparse.c"
        break;
      case 246: /* rh_start ::= TOK_RULE rh_get_line TOK_IDENTIFIER TOK_FOR TOK_LEFT_PAREN */
#line 1995 "expparse.y"
{
    Rule r = ALGcreate(OBJ_RULE);

    if (print_objects_while_running & OBJ_RULE_BITS) {
    fprintf(stdout, "parse: %s (rule)\n", yymsp[-2].minor.yy0.symbol->name);
    }

    PUSH_SCOPE(r, yymsp[-2].minor.yy0.symbol, OBJ_RULE);

    yygotominor.yy507 = yymsp[-3].minor.yy507;
}
#line 4112 "expparse.c"
        break;
      case 250: /* schema_decl ::= schema_header schema_body TOK_END_SCHEMA semicolon */
#line 2022 "expparse.y"
{
    POP_SCOPE();
}
#line 4119 "expparse.c"
        break;
      case 252: /* schema_header ::= TOK_SCHEMA TOK_IDENTIFIER semicolon */
#line 2031 "expparse.y"
{
    Schema schema = ( Schema ) DICTlookup(CURRENT_SCOPE->symbol_table, yymsp[-1].minor.yy0.symbol->name);

    if (print_objects_while_running & OBJ_SCHEMA_BITS) {
    fprintf(stdout, "parse: %s (schema)\n", yymsp[-1].minor.yy0.symbol->name);
    }

    if (EXPRESSignore_duplicate_schemas && schema) {
    SCANskip_to_end_schema(parseData.scanner);
    PUSH_SCOPE_DUMMY();
    } else {
    schema = SCHEMAcreate();
    LISTadd_last(PARSEnew_schemas, (Generic)schema);
    PUSH_SCOPE(schema, yymsp[-1].minor.yy0.symbol, OBJ_SCHEMA);
    }
}
#line 4139 "expparse.c"
        break;
      case 253: /* select_type ::= TOK_SELECT TOK_LEFT_PAREN defined_type_list TOK_RIGHT_PAREN */
#line 2050 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(select_);
    yygotominor.yy477->list = yymsp[-1].minor.yy371;
}
#line 4147 "expparse.c"
        break;
      case 255: /* set_type ::= TOK_SET bound_spec TOK_OF attribute_type */
#line 2061 "expparse.y"
{
    yygotominor.yy477 = TYPEBODYcreate(set_);
    yygotominor.yy477->base = yymsp[0].minor.yy297;
    yygotominor.yy477->lower = yymsp[-2].minor.yy253.lower_limit;
    yygotominor.yy477->upper = yymsp[-2].minor.yy253.upper_limit;
}
#line 4157 "expparse.c"
        break;
      case 257: /* skip_statement ::= TOK_SKIP semicolon */
#line 2074 "expparse.y"
{
    yygotominor.yy332 = STATEMENT_SKIP;
}
#line 4164 "expparse.c"
        break;
      case 258: /* statement ::= alias_statement */
      case 259: /* statement ::= assignment_statement */ yytestcase(yyruleno==259);
      case 260: /* statement ::= case_statement */ yytestcase(yyruleno==260);
      case 261: /* statement ::= compound_statement */ yytestcase(yyruleno==261);
      case 262: /* statement ::= escape_statement */ yytestcase(yyruleno==262);
      case 263: /* statement ::= if_statement */ yytestcase(yyruleno==263);
      case 264: /* statement ::= proc_call_statement */ yytestcase(yyruleno==264);
      case 265: /* statement ::= repeat_statement */ yytestcase(yyruleno==265);
      case 266: /* statement ::= return_statement */ yytestcase(yyruleno==266);
      case 267: /* statement ::= skip_statement */ yytestcase(yyruleno==267);
#line 2079 "expparse.y"
{
    yygotominor.yy332 = yymsp[0].minor.yy332;
}
#line 4180 "expparse.c"
        break;
      case 270: /* statement_rep ::= statement statement_rep */
#line 2128 "expparse.y"
{
    yygotominor.yy371 = yymsp[0].minor.yy371;
    LISTadd_first(yygotominor.yy371, (Generic)yymsp[-1].minor.yy332); 
}
#line 4188 "expparse.c"
        break;
      case 271: /* subsuper_decl ::= */
#line 2138 "expparse.y"
{
    yygotominor.yy242.subtypes = EXPRESSION_NULL;
    yygotominor.yy242.abstract = false;
    yygotominor.yy242.supertypes = LIST_NULL;
}
#line 4197 "expparse.c"
        break;
      case 272: /* subsuper_decl ::= supertype_decl */
#line 2144 "expparse.y"
{
    yygotominor.yy242.subtypes = yymsp[0].minor.yy385.subtypes;
    yygotominor.yy242.abstract = yymsp[0].minor.yy385.abstract;
    yygotominor.yy242.supertypes = LIST_NULL;
}
#line 4206 "expparse.c"
        break;
      case 273: /* subsuper_decl ::= subtype_decl */
#line 2150 "expparse.y"
{
    yygotominor.yy242.supertypes = yymsp[0].minor.yy371;
    yygotominor.yy242.abstract = false;
    yygotominor.yy242.subtypes = EXPRESSION_NULL;
}
#line 4215 "expparse.c"
        break;
      case 274: /* subsuper_decl ::= supertype_decl subtype_decl */
#line 2156 "expparse.y"
{
    yygotominor.yy242.subtypes = yymsp[-1].minor.yy385.subtypes;
    yygotominor.yy242.abstract = yymsp[-1].minor.yy385.abstract;
    yygotominor.yy242.supertypes = yymsp[0].minor.yy371;
}
#line 4224 "expparse.c"
        break;
      case 276: /* supertype_decl ::= TOK_ABSTRACT TOK_SUPERTYPE */
#line 2169 "expparse.y"
{
    yygotominor.yy385.subtypes = (Expression)0;
    yygotominor.yy385.abstract = true;
}
#line 4232 "expparse.c"
        break;
      case 277: /* supertype_decl ::= TOK_SUPERTYPE TOK_OF TOK_LEFT_PAREN supertype_expression TOK_RIGHT_PAREN */
#line 2175 "expparse.y"
{
    yygotominor.yy385.subtypes = yymsp[-1].minor.yy401;
    yygotominor.yy385.abstract = false;
}
#line 4240 "expparse.c"
        break;
      case 278: /* supertype_decl ::= TOK_ABSTRACT TOK_SUPERTYPE TOK_OF TOK_LEFT_PAREN supertype_expression TOK_RIGHT_PAREN */
#line 2181 "expparse.y"
{
    yygotominor.yy385.subtypes = yymsp[-1].minor.yy401;
    yygotominor.yy385.abstract = true;
}
#line 4248 "expparse.c"
        break;
      case 279: /* supertype_expression ::= supertype_factor */
#line 2187 "expparse.y"
{
    yygotominor.yy401 = yymsp[0].minor.yy385.subtypes;
}
#line 4255 "expparse.c"
        break;
      case 280: /* supertype_expression ::= supertype_expression TOK_AND supertype_factor */
#line 2191 "expparse.y"
{
    yygotominor.yy401 = BIN_EXPcreate(OP_AND, yymsp[-2].minor.yy401, yymsp[0].minor.yy385.subtypes);
}
#line 4262 "expparse.c"
        break;
      case 281: /* supertype_expression ::= supertype_expression TOK_ANDOR supertype_factor */
#line 2196 "expparse.y"
{
    yygotominor.yy401 = BIN_EXPcreate(OP_ANDOR, yymsp[-2].minor.yy401, yymsp[0].minor.yy385.subtypes);
}
#line 4269 "expparse.c"
        break;
      case 283: /* supertype_expression_list ::= supertype_expression_list TOK_COMMA supertype_expression */
#line 2207 "expparse.y"
{
    LISTadd_last(yymsp[-2].minor.yy371, (Generic)yymsp[0].minor.yy401);
    yygotominor.yy371 = yymsp[-2].minor.yy371;
}
#line 4277 "expparse.c"
        break;
      case 284: /* supertype_factor ::= identifier */
#line 2213 "expparse.y"
{
    yygotominor.yy385.subtypes = yymsp[0].minor.yy401;
}
#line 4284 "expparse.c"
        break;
      case 285: /* supertype_factor ::= oneof_op TOK_LEFT_PAREN supertype_expression_list TOK_RIGHT_PAREN */
#line 2218 "expparse.y"
{
    yygotominor.yy385.subtypes = EXPcreate(Type_Oneof);
    yygotominor.yy385.subtypes->u.list = yymsp[-1].minor.yy371;
}
#line 4292 "expparse.c"
        break;
      case 286: /* supertype_factor ::= TOK_LEFT_PAREN supertype_expression TOK_RIGHT_PAREN */
#line 2223 "expparse.y"
{
    yygotominor.yy385.subtypes = yymsp[-1].minor.yy401;
}
#line 4299 "expparse.c"
        break;
      case 287: /* type ::= aggregation_type */
      case 288: /* type ::= basic_type */ yytestcase(yyruleno==288);
      case 290: /* type ::= select_type */ yytestcase(yyruleno==290);
#line 2228 "expparse.y"
{
    yygotominor.yy378.type = 0;
    yygotominor.yy378.body = yymsp[0].minor.yy477;
}
#line 4309 "expparse.c"
        break;
      case 292: /* type_item_body ::= type */
#line 2253 "expparse.y"
{
    CURRENT_SCOPE->u.type->head = yymsp[0].minor.yy378.type;
    CURRENT_SCOPE->u.type->body = yymsp[0].minor.yy378.body;
}
#line 4317 "expparse.c"
        break;
      case 294: /* ti_start ::= TOK_IDENTIFIER TOK_EQUAL */
#line 2261 "expparse.y"
{
    Type t = TYPEcreate_name(yymsp[-1].minor.yy0.symbol);
    PUSH_SCOPE(t, yymsp[-1].minor.yy0.symbol, OBJ_TYPE);
}
#line 4325 "expparse.c"
        break;
      case 296: /* td_start ::= TOK_TYPE type_item where_rule_OPT */
#line 2272 "expparse.y"
{
    CURRENT_SCOPE->where = yymsp[0].minor.yy371;
    POP_SCOPE();
    yygotominor.yy0 = yymsp[-2].minor.yy0;
}
#line 4334 "expparse.c"
        break;
      case 297: /* general_ref ::= assignable group_ref */
#line 2279 "expparse.y"
{
    yymsp[0].minor.yy401->e.op1 = yymsp[-1].minor.yy401;
    yygotominor.yy401 = yymsp[0].minor.yy401;
}
#line 4342 "expparse.c"
        break;
      case 307: /* unary_expression ::= TOK_NOT unary_expression */
#line 2322 "expparse.y"
{
    yygotominor.yy401 = UN_EXPcreate(OP_NOT, yymsp[0].minor.yy401);
}
#line 4349 "expparse.c"
        break;
      case 309: /* unary_expression ::= TOK_MINUS unary_expression */
#line 2330 "expparse.y"
{
    yygotominor.yy401 = UN_EXPcreate(OP_NEGATE, yymsp[0].minor.yy401);
}
#line 4356 "expparse.c"
        break;
      case 310: /* unique ::= */
#line 2335 "expparse.y"
{
    yygotominor.yy252.unique = 0;
}
#line 4363 "expparse.c"
        break;
      case 311: /* unique ::= TOK_UNIQUE */
#line 2339 "expparse.y"
{
    yygotominor.yy252.unique = 1;
}
#line 4370 "expparse.c"
        break;
      case 312: /* qualified_attr ::= TOK_IDENTIFIER */
#line 2344 "expparse.y"
{
    yygotominor.yy457 = QUAL_ATTR_new();
    yygotominor.yy457->attribute = yymsp[0].minor.yy0.symbol;
}
#line 4378 "expparse.c"
        break;
      case 313: /* qualified_attr ::= TOK_SELF TOK_BACKSLASH TOK_IDENTIFIER TOK_DOT TOK_IDENTIFIER */
#line 2350 "expparse.y"
{
    yygotominor.yy457 = QUAL_ATTR_new();
    yygotominor.yy457->entity = yymsp[-2].minor.yy0.symbol;
    yygotominor.yy457->attribute = yymsp[0].minor.yy0.symbol;
}
#line 4387 "expparse.c"
        break;
      case 314: /* qualified_attr_list ::= qualified_attr */
#line 2357 "expparse.y"
{
    yygotominor.yy371 = LISTcreate();
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy457);
}
#line 4395 "expparse.c"
        break;
      case 315: /* qualified_attr_list ::= qualified_attr_list TOK_COMMA qualified_attr */
#line 2362 "expparse.y"
{
    yygotominor.yy371 = yymsp[-2].minor.yy371;
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy457);
}
#line 4403 "expparse.c"
        break;
      case 316: /* labelled_attrib_list ::= qualified_attr_list semicolon */
#line 2368 "expparse.y"
{
    LISTadd_first(yymsp[-1].minor.yy371, (Generic)EXPRESSION_NULL);
    yygotominor.yy371 = yymsp[-1].minor.yy371;
}
#line 4411 "expparse.c"
        break;
      case 317: /* labelled_attrib_list ::= TOK_IDENTIFIER TOK_COLON qualified_attr_list semicolon */
#line 2374 "expparse.y"
{
    LISTadd_first(yymsp[-1].minor.yy371, (Generic)yymsp[-3].minor.yy0.symbol); 
    yygotominor.yy371 = yymsp[-1].minor.yy371;
}
#line 4419 "expparse.c"
        break;
      case 318: /* labelled_attrib_list_list ::= labelled_attrib_list */
#line 2381 "expparse.y"
{
    yygotominor.yy371 = LISTcreate();
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy371);
}
#line 4427 "expparse.c"
        break;
      case 319: /* labelled_attrib_list_list ::= labelled_attrib_list_list labelled_attrib_list */
#line 2387 "expparse.y"
{
    LISTadd_last(yymsp[-1].minor.yy371, (Generic)yymsp[0].minor.yy371);
    yygotominor.yy371 = yymsp[-1].minor.yy371;
}
#line 4435 "expparse.c"
        break;
      case 322: /* until_control ::= */
      case 331: /* while_control ::= */ yytestcase(yyruleno==331);
#line 2402 "expparse.y"
{
    yygotominor.yy401 = 0;
}
#line 4443 "expparse.c"
        break;
      case 324: /* where_clause ::= expression semicolon */
#line 2411 "expparse.y"
{
    yygotominor.yy234 = WHERE_new();
    yygotominor.yy234->label = SYMBOLcreate("<unnamed>", yylineno, current_filename);
    yygotominor.yy234->expr = yymsp[-1].minor.yy401;
}
#line 4452 "expparse.c"
        break;
      case 325: /* where_clause ::= TOK_IDENTIFIER TOK_COLON expression semicolon */
#line 2417 "expparse.y"
{
    yygotominor.yy234 = WHERE_new();
    yygotominor.yy234->label = yymsp[-3].minor.yy0.symbol;
    yygotominor.yy234->expr = yymsp[-1].minor.yy401;

    if (!CURRENT_SCOPE->symbol_table) {
    CURRENT_SCOPE->symbol_table = DICTcreate(25);
    }

    DICTdefine(CURRENT_SCOPE->symbol_table, yymsp[-3].minor.yy0.symbol->name, (Generic)yygotominor.yy234,
    yymsp[-3].minor.yy0.symbol, OBJ_WHERE);
}
#line 4468 "expparse.c"
        break;
      case 326: /* where_clause_list ::= where_clause */
#line 2431 "expparse.y"
{
    yygotominor.yy371 = LISTcreate();
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy234);
}
#line 4476 "expparse.c"
        break;
      case 327: /* where_clause_list ::= where_clause_list where_clause */
#line 2436 "expparse.y"
{
    yygotominor.yy371 = yymsp[-1].minor.yy371;
    LISTadd_last(yygotominor.yy371, (Generic)yymsp[0].minor.yy234);
}
#line 4484 "expparse.c"
        break;
      default:
      /* (4) action_body_item_rep ::= */ yytestcase(yyruleno==4);
      /* (41) block_list ::= */ yytestcase(yyruleno==41);
      /* (62) constant_body_list ::= */ yytestcase(yyruleno==62);
      /* (86) express_file ::= schema_decl_list */ yytestcase(yyruleno==86);
      /* (159) parened_rename_list ::= TOK_LEFT_PAREN rename_list TOK_RIGHT_PAREN */ yytestcase(yyruleno==159);
      /* (168) interface_specification_list ::= */ yytestcase(yyruleno==168);
      /* (194) local_body ::= */ yytestcase(yyruleno==194);
      /* (196) local_decl ::= TOK_LOCAL allow_generic_types local_body TOK_END_LOCAL semicolon disallow_generic_types */ yytestcase(yyruleno==196);
      /* (293) type_item ::= ti_start type_item_body semicolon */ yytestcase(yyruleno==293);
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  ParseARG_FETCH;
#define TOKEN (yyminor.yy0)
#line 2464 "expparse.y"

    Symbol sym;

    (void) yymajor; /* quell unused param warning */
    (void) yyminor;
    yyerrstatus++;

    sym.line = yylineno;
    sym.filename = current_filename;

    ERRORreport_with_symbol(ERROR_syntax, &sym, "",
    CURRENT_SCOPE_TYPE_PRINTABLE, CURRENT_SCOPE_NAME);
#line 4568 "expparse.c"
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  ParseARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  ParseARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "ParseAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void Parse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  ParseTOKENTYPE yyminor       /* The value for the token */
  ParseARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  ParseARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact<YYNSTATE ){
      assert( !yyendofinput );  /* Impossible to shift the $ token */
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor,yyminorunion);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}
