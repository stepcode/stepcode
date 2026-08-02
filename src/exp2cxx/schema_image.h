#ifndef EXP2CXX_SCHEMA_IMAGE_H
#define EXP2CXX_SCHEMA_IMAGE_H

#include <stdio.h>

#include "express/entity.h"
#include "express/linklist.h"
#include "express/schema.h"

#ifdef __cplusplus
extern "C" {
#endif

void SCHEMAimage_reset( void );
void SCHEMAimage_register_schema( Schema schema, Linked_List entities );
void SCHEMAimage_set_external_mapping( Entity entity, int externalMapping );
void SCHEMAimage_write( FILE * output );

#ifdef __cplusplus
}
#endif

#endif
