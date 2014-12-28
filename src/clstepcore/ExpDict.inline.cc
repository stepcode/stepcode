
/*
* NIST STEP Core Class Library
* clstepcore/ExpDict.inline.cc
* April 1997
* K. C. Morris
* David Sauder

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

#include <ExpDict.h>
#include "sc_memmgr.h"




Interfaced_item::Interfaced_item() {
}

Interfaced_item::Interfaced_item( const Interfaced_item & ii ): Dictionary_instance() {
    _foreign_schema = ii._foreign_schema;
}

Interfaced_item::Interfaced_item( const char * foreign_schema )
    : _foreign_schema( foreign_schema ) {
}

Interfaced_item::~Interfaced_item() {
}

const Express_id Interfaced_item::foreign_schema_() {
    return _foreign_schema;
}

void Interfaced_item::foreign_schema_( const Express_id & fs ) {
    _foreign_schema = fs;
}

Explicit_item_id::Explicit_item_id() {
    _local_definition = 0;
}

Explicit_item_id::Explicit_item_id( const Explicit_item_id & eii )
    : Interfaced_item( eii ) {
    _local_definition = eii._local_definition;
    _original_id = eii._original_id;
    _new_id = eii._new_id;
}

Explicit_item_id::~Explicit_item_id() {
    _local_definition = 0;
}

Used_item::~Used_item() {
}

Referenced_item::~Referenced_item() {
}

Implicit_item_id::Implicit_item_id() {
    _local_definition = 0;
}

Implicit_item_id::Implicit_item_id( Implicit_item_id & iii )
    : Interfaced_item( iii ) {
    _local_definition = iii._local_definition;
}

Implicit_item_id::~Implicit_item_id() {
    _local_definition = 0;
}
