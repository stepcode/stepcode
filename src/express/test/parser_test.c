#include <stdlib.h>

#include "lexsupport.h"

int main(int argc, char *argv[])
{
    int tok;
    struct YYSTYPE yylval;
    struct YYSTATE *pState;
    void *pParser, *pScanner;
    FILE *fp;
    
    if (argc < 2)
        yyerror("no input files!", 0);

    fp = fopen(argv[1], "r");
    if (!fp)
        yyerror("failed to open input!", 0);

    pScanner = yylexAlloc();
    pParser = yyparseAlloc(malloc);
    pState = yystateAlloc();
    
    yylexInit(pScanner, pState, fp);
    
    // yyparseTrace(stderr, "dbg: ");
    while ((tok = yylex(pScanner, &yylval))) {
        yyparse(pParser, tok, &yylval, pState);
    }
    yyparse(pParser, 0, &yylval, pState);
    
	exit(0);
}
