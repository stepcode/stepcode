#include "express/resolve.h"

int main(int argc, char *argv[]) {

    EXP_resolve();
    
    ENTITYresolve_subtype_expression();
    
    TYPE_resolve(typ);

    STMTresolve();
    
    SCOPEresolve_types();
    
    SCOPEresolve_subsupers();
    
    ENTITYresolve_supertypes();
    
    SCOPEresolve_expressions_statements();
    
    return 0;
}

