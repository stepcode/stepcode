#ifndef CLASSES_ATTRIBUTE_H
#define CLASSES_ATTRIBUTE_H

char * generate_attribute_name( Variable a, char * out );
char * generate_attribute_func_name( Variable a, char * out );

void ATTRsign_access_methods( Variable a, FILE * file );
void ATTRprint_access_methods( CONST char * entnm, Variable a, FILE * file );

#endif
