/** \file inverse_attr3.cc
 * Oct 2013
 * Test inverse attributes; uses a tiny schema similar to a subset of IFC2x3
 *
 * This test originally used STEPfile, which didn't work. Fixing STEPfile would have been very difficult, it uses lazyInstMgr now.
 */
#include <sc_cf.h>
extern void SchemaInit( class Registry & );
#include <lazyInstMgr.h>
#include <lazyRefs.h>
#include <sdai.h>
#include <STEPattribute.h>
#include <ExpDict.h>
#include <Registry.h>
#include <errordesc.h>
#include <algorithm>
#include <string>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sc_getopt.h>
#include "schema.h"

int main( int argc, char * argv[] ) {
    if( argc != 2 ) {
        cerr << "Wrong number of args!" << endl;
        exit( EXIT_FAILURE );
    }
    lazyInstMgr lim;
    lim.initRegistry( SchemaInit );

    lim.openFile( argv[1] );

//find attributes
    instanceTypes_t::cvector * insts = lim.getInstances( "window" );
    if( !insts || insts->empty() ) {
        cout << "No window instances found!" << endl;
        exit( EXIT_FAILURE );
    }
    SdaiWindow * instance = dynamic_cast< SdaiWindow * >( lim.loadInstance( insts->at( 0 ) ) );
    if( !instance ) {
        cout << "Problem loading instance" << endl;
        exit( EXIT_FAILURE );
    }
    cout << "instance #" << instance->StepFileId() << endl;

    /* The inverse could be set with
     *    const Inverse_attribute * ia =...;
     *    const EntityDescriptor * inv_ed = reg.FindEntity( ia->inverted_entity_id_() );
     *    instance->isdefinedby_(inv_ed);
     */
    EntityAggregate * aggr = instance->isdefinedby_(); //should be filled in when the file is loaded? not sure how to do it using STEPfile...
    if( aggr ) {
        cout << aggr->EntryCount() << endl;
    } else {
        cout << "inverse attr is not defined" << endl;
        exit( EXIT_FAILURE );
    }
    exit( EXIT_SUCCESS );
}

