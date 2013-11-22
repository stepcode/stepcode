/** \file list_attributes.cc
 *
 * early-bound implementation of an attribute printer
 * list_attributes -a entity
 */
#include <sc_cf.h>
extern void SchemaInit( class Registry & );
#include "sc_version.h"
// #include <STEPfile.h>
// #include <sdai.h>
#include <STEPattribute.h>
#include <ExpDict.h>
#include <Registry.h>
#include <errordesc.h>
#include <algorithm>
#include <string>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#include <string.h>
#endif
#include <sc_getopt.h>
#include "schema.h"


int main( int argc, char * argv[] ) {
    const char * entity = "si_energy_unit"; //TODO
    if( argc == 2 ) {
        entity = argv[1];
    } else if( ( argc == 3 ) && ( 0 == strncasecmp( argv[1], "-a", 2 ) ) ) {
        entity = argv[2];
    } /*else {
        std::cerr << "Bad args. Use:" << std::endl << argv[0] << " [-aA] <entity>" << std::endl;
        std::cerr << "Prints entity attributes as STEPcode thinks they should be" << std::endl;
        exit( EXIT_FAILURE );
    }*/ //TODO

    Registry  registry( SchemaInit );
    EntityDescriptor * seu = registry.FindEntity( entity );
    AttrDescriptorList adl = seu->ExplicitAttr();

}

