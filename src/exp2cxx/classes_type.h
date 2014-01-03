#ifndef CLASSES_TYPE_H
#define CLASSES_TYPE_H

Type TYPEget_ancestor( Type );

const char * TypeName( Type t );
const char * TypeDescriptorName( Type );
char * TypeDescription( const Type t );
const char * AccessType( Type t );
const char * TYPEget_ctype( const Type t );

void TYPEPrint( const Type type, FILES *files, Schema schema );
void TYPEprint_descriptions( const Type, FILES *, Schema );
void TYPEprint_definition( Type, FILES *, Schema );
void TYPEprint_typedefs( Type, FILE * );
void TYPEprint_new( const Type, FILE *, Schema );
void TYPEprint_init( const Type type, FILE * file, Schema schema );

void TYPEenum_inc_print( const Type type, FILE * inc );
void TYPEenum_lib_print( const Type type, FILE * f );

void TYPEselect_print( Type, FILES *, Schema );
void TYPEselect_init_print( const Type type, FILE* f );
void TYPEselect_inc_print( const Type type, FILE * f );
void TYPEselect_lib_print( const Type type, FILE * f );

void AGGRprint_init( FILE * file, const Type t, const char * var_name, const char * aggr_name );

void print_typechain( FILE *file, const Type t, char * buf, Schema schema, const char * type_name );

#endif
