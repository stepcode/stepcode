#ifndef CLASSES_ENTITY_H
#define CLASSES_ENTITY_H

const char * ENTITYget_classname( Entity );
Entity ENTITYget_superclass( Entity entity );
Entity ENTITYput_superclass( Entity entity );
int ENTITYhas_explicit_attributes( Entity e );
void ENTITYget_first_attribs( Entity entity, Linked_List result );

void ENTITYPrint( Entity, FILES *, Schema );
void ENTITYprint_new( Entity, FILES *, Schema, int );

#endif
