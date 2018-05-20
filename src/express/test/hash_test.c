#include <stdio.h>
#include <stdlib.h>

#include "bstrlib.h"
#include "express/hash.h"

#define error_helper(fmt, ...) \
    do { fprintf(stderr, "error, line %i: " fmt "\n", __LINE__, __VA_ARGS__); exit(1); } while (0)
#define error(...) error_helper(__VA_ARGS__, 0)

int main(int argc, char *argv[]) {
    FILE *fp;
    bstring buf, id;
    struct bstrList *testdata;
    unsigned int i;
    Hash_Table tbl;
    Hash_Entry *ep;

    if (argc < 2)
        error("no input files");

    fp = fopen(argv[1], "r");
    if (!fp)
        error("failed to open input!");
    
    buf = bread((bNread) fread, fp);
    if (!buf)
        error("failed to read input!");

    if (fclose(fp))
        error("failed to correctly close input file!");
    
    testdata = bsplit(buf, '\n');

    tbl = HASHcreate();
    
    for (i = 0; i < testdata->qty; i++) {
        id = testdata->entry[i];
        if (btrimws(id) == BSTR_ERR)
            error("error trimming string!");
        
        ep = HASHsearch(tbl, (Hash_Entry) {.key = id->data, .data = id}, HASH_INSERT);
        if (!ep)
            error("error, hsearch HASH_INSERT failed!");
    }
    
    for (i = 0; i < testdata->qty; i++) {
        id = testdata->entry[i];
        if (btrimws(id) == BSTR_ERR)
            error("error trimming string!");
        
        ep = HASHsearch(tbl, (Hash_Entry) {.key = id->data, .data = id}, HASH_FIND);
        if (!ep)
            error("error, hsearch HASH_FIND failed!");
    }
    
    
    HASHdestroy(tbl);
    bstrListDestroy(testdata);
    bdestroy(buf);

    exit(0);
}
