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
    unsigned int i, qty;
    Hash_Table tbl;
    Symbol *ep;
    Symbol e;
    Hash_Iterator it;

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
    qty = testdata->qty;

    tbl = HASHcreate();
    
    /* HASHsearch INSERT */
    for (i = 0; i < testdata->qty; i++) {
        id = testdata->entry[i];
        if (id->slen > 0 && btrimws(id) == BSTR_ERR)
            error("btrimws failed?!");
        
        if (!id->slen) {
            qty--;
            continue;
        }
        
        ep = HASHsearch(tbl, (Symbol) {.name = id->data, .data = id}, HASH_INSERT);
        if (!ep)
            error("HASHsearch HASH_INSERT failed!");
    }
    
    /* HASHsearch FIND */
    for (i = 0; i < testdata->qty; i++) {
        id = testdata->entry[i];
    
        if (!id->slen)
            continue;
        
        ep = HASHsearch(tbl, (Symbol) {.name = id->data, .data = id}, HASH_FIND);
        if (!ep)
            error("HASHsearch HASH_FIND failed!");
    }
    
    /* HASHdo */
    i = 0;
    HASHdo_init(tbl, &it, OBJ_ANY);
    while (HASHdo(&it))
        i++;
    
    if (i != qty)
        error("HASHdo failed! expected: %d, got: %d", qty, i);
    
    if (qty != 6512)
        error("HASHdelete tests require a specific test input file, and build parameters");
    
    /* expected collisions
     * [change_action,
     *  device_terminal_map,
     *  fea_column_normalised_orthotropic_symmetric_tensor4_3d,
     *  fill_area_style_tile_symbol_with_style,
     *  ifcorthogonalcomplement,
     *  time_interval_with_bounds] 
     * 
     */
    e = (Symbol) {.name = "fea_column_normalised_orthotropic_symmetric_tensor4_3d"};
    HASHdelete(tbl, &e);
    
    i = 0;
    HASHdo_init(tbl, &it, OBJ_ANY);
    while (HASHdo(&it))
        i++;
    
    qty--;
    if (i != qty)
        error("HASHdelete failed (#, #, x, #, #, #, #), expected %d, got %d", qty, i);
    
    e = (Symbol) {.name = "ifcorthogonalcomplement"};
    HASHdelete(tbl, &e);

    i = 0;
    HASHdo_init(tbl, &it, OBJ_ANY);
    while (HASHdo(&it))
        i++;
    
    qty--;
    if (i != qty)
        error("HASHdelete failed (#, #, _, #, #, x, #), expected %d, got %d", qty, i);
    
    /*
     * expected collisions
     * [evaluation_product_definition,
     *  ifcfurniture,
     *  ifcreloverridesproperties,
     *  language,
     *  physical_state_domain]
     */
    e = (Symbol) {.name = "ifcfurniture"};
    HASHdelete(tbl, &e);

    i = 0;
    HASHdo_init(tbl, &it, OBJ_ANY);
    while (HASHdo(&it))
        i++;
    
    qty--;
    if (i != qty)
        error("HASHdelete failed (#, x, #, #, #), expected %d, got %d", qty, i);
    
    e = (Symbol) {.name = "physical_state_domain"};
    HASHdelete(tbl, &e);

    i = 0;
    HASHdo_init(tbl, &it, OBJ_ANY);
    while (HASHdo(&it))
        i++;
    
    qty--;
    if (i != qty)
        error("HASHdelete failed (#, _, #, #, x), expected %d, got %d", qty, i);
    
    /*
     * [action_resource_relationship,
     *  annotation_occurrence_relationship,
     *  erroneous_data,
     *  ifclinearvelocitymeasure]
     */
    e = (Symbol) {.name = "action_resource_relationship"};
    HASHdelete(tbl, &e);

    i = 0;
    HASHdo_init(tbl, &it, OBJ_ANY);
    while (HASHdo(&it))
        i++;
    
    qty--;
    if (i != qty)
        error("HASHdelete failed (x, #, #, #), expected %d, got %d", qty, i);
    
    /*
     * [choose_representation_context_identifier,
     *  dependent_instantiable_text_style]
     */
    e = (Symbol) {.name = "choose_representation_context_identifier"};
    HASHdelete(tbl, &e);

    i = 0;
    HASHdo_init(tbl, &it, OBJ_ANY);
    while (HASHdo(&it))
        i++;
    
    qty--;
    if (i != qty)
        error("HASHdelete failed (x, #), expected %d, got %d", qty, i);
    
    e = (Symbol) {.name = "dependent_instantiable_text_style"};
    HASHdelete(tbl, &e);

    i = 0;
    HASHdo_init(tbl, &it, OBJ_ANY);
    while (HASHdo(&it))
        i++;
    
    qty--;
    if (i != qty)
        error("HASHdelete failed (_, x), expected %d, got %d", qty, i);

    /*
     * [connector_based_interconnect_definition]
     */
    e = (Symbol) {.name = "connector_based_interconnect_definition"};
    HASHdelete(tbl, &e);

    i = 0;
    HASHdo_init(tbl, &it, OBJ_ANY);
    while (HASHdo(&it))
        i++;
    
    qty--;
    if (i != qty)
        error("HASHdelete failed (x,), expected %d, got %d", qty, i);

    e = (Symbol) {.name = "connector_based_interconnect_definition"};
    HASHdelete(tbl, &e);

    i = 0;
    HASHdo_init(tbl, &it, OBJ_ANY);
    while (HASHdo(&it))
        i++;
    
    if (i != qty)
        error("HASHdelete failed (_,), expected %d, got %d", qty, i);
    
    HASHdestroy(tbl);
    bstrListDestroy(testdata);
    bdestroy(buf);

    exit(0);
}
