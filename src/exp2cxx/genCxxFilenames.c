#include "genCxxFilenames.h"
#include "class_strings.h"

/** \file genCxxFilenames.c
 * functions shared by exp2cxx and the schema
 * scanner which determines, at configuration time,
 * the names of the files exp2cxx will generate
 */

/* these buffers are shared amongst (and potentially overwritten by) all functions in this file */
char impl[ BUFSIZ ] = {0};
char header[ BUFSIZ ] = {0};

/* struct containing pointers to above buffers. pointers are 'const char *' */
filenames_t fnames = { impl, header };


filenames_t getEntityFilenames( Entity e ) {
    const char * name = ENTITYget_classname( e );
    snprintf( header, BUFSIZ-1, "entity/%s.h", name );
    snprintf( impl, BUFSIZ-1, "entity/%s.cc", name );
    return fnames;
}

filenames_t getTypeFilenames( Type t ) {
    const char * name = TYPEget_ctype( t );
    snprintf( header, BUFSIZ-1, "type/%s.h", name );
    snprintf( impl, BUFSIZ-1, "type/%s.cc", name );
    return fnames;
}
