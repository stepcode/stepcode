#include "genCxxFilenames.h"

/** \file genCxxFilenames.c
 * functions shared by exp2cxx and the schema
 * scanner which determines, at configuration time,
 * the names of the files exp2cxx will generate
 */

/* these buffers are shared amongst (and potentially overwritten by) all functions in this file */
char header[ BUFSIZ ] = {0};
char impl[ BUFSIZ ] = {0};

/* struct containing pointers to above buffers. pointers are 'const char *' */
filenames_t fnames;


filenames_t getEntityFilenames( Entity e ) {
    snprintf( header, BUFSIZ-1, "%s.h", e->symbol.name );
    snprintf( impl, BUFSIZ-1, "%s.cc", e->symbol.name );
    fnames.header = header;
    fnames.impl = impl;
    return fnames;
}
filenames_t getTypeFilenames( Type t ) {
    snprintf( header, BUFSIZ-1, "%s.h", t->symbol.name );
    snprintf( impl, BUFSIZ-1, "%s.cc", t->symbol.name );
    fnames.header = header;
    fnames.impl = impl;
    return fnames;
}
