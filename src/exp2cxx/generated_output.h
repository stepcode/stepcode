#ifndef GENERATED_OUTPUT_H
#define GENERATED_OUTPUT_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void GENERATEDbegin( void );
FILE * GENERATEDopen( const char * filename );
FILE * GENERATEDappend( const char * filename );
void GENERATEDclose( FILE * file );
int GENERATEDwrite( const char * filename, const char * data, size_t size );
int GENERATEDfinish( void );

#ifdef __cplusplus
}
#endif

#endif
