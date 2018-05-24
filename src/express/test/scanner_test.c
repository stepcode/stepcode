#include "lexsupport.h"

/* stubs */
Symbol * SYMBOLcreate( char * name, int ref_tok, int line, const char * filename ) {
        return NULL;
}

int main(int argc, char **argv)
{
    FILE *fp;
    struct YYSTATE *pState;
    struct exp_scanner *pScanner;
    YYSTYPE yylval;
    
    if (argc < 2) {
        fprintf(stderr, "no input files\n");
        return 1;
    }

    fp = fopen(argv[1], "r");
    if (!fp)
        yyerror("failed to open input!", 0);

    
    pScanner = yylexAlloc();
    pState = yystateAlloc();
    yylexInit(pScanner, pState, fp);

    while(yylex(pScanner, &yylval) != 0)
        ;
    
    fclose(fp);
}
