
extern void SchemaInit( class Registry & );
#include "scl_version_string.h"
#include <STEPfile.h>
#include <sdai.h>
#include <STEPattribute.h>
#include <ExpDict.h>
#include <Registry.h>
#include <errordesc.h>
#include <algorithm>
#include <string>
#include <unistd.h>

#include "SdaiTEST_BINARY_INDEX.h"

/* TODO: simplify this as much as possible
 * This segfault will probably occur with the STEPattributeList base class, SingleLinkList, as well - and without a schema lib.
 */
int main( int argc, char * argv[] ) {

    if ( argc != 2 ) {
        cerr << "Wrong number of args. Use: " << argv[0] << " file.stp" << endl;
        exit(1);
    }

    Registry  registry( SchemaInit );
    InstMgr   instance_list;
    STEPfile  sfile( registry, instance_list, "", false );

    sfile.ReadExchangeFile( argv[1] );
    sfile.Error().PrintContents(cout);

    Severity readSev = sfile.Error().severity();

    // Keeps track of the last processed ent id
    int search_index = 0;

    const EntityDescriptor* ed = registry.FindEntity("Buynary");
    AttrDescItr aditr( ed->ExplicitAttr() );
    const AttrDescriptor * attrDesc = aditr.NextAttrDesc();
    while( attrDesc != 0 ) {
        if( attrDesc->Derived() == LTrue ) {
            cout << "attr: " << attrDesc->Name() << " initializer: " << ((Derived_attribute*) attrDesc)->initializer_() << endl;
            //how to find the value of an attribute for an entity?
        }
        attrDesc = aditr.NextAttrDesc();
    }

    SdaiBuynary* ent;
    while ( ENTITY_NULL != (ent = (SdaiBuynary*) instance_list.GetApplication_instance("Buynary",search_index) ) ) {
        // Loop over the Buynarys in the file
        cout << "Ent #" << ent->StepFileId() << endl;
        SDAI_Binary b = ent->bin_();
        int cnt = ent->AttributeCount();
        cout << "bin " << b.asStr() << endl;
        cout << "count " << cnt << endl;

        STEPattributeList sal = ent->attributes; //commenting this line out prevents the segfault

        MgrNode* mnode = instance_list.FindFileId( ent->StepFileId() );
        search_index = instance_list.GetIndex( mnode ) + 1;
    }
    /* attr: lasthalf initializer: bin[5:8]
     * Ent #1
     * bin 15A
     * count 1
     * 
     * Program received signal SIGSEGV, Segmentation fault.
     * 0x000000000067f768 in ?? ()
     * (gdb) bt
     * #0  0x000000000067f768 in ?? ()
     * #1  0x00007ffff778962d in SingleLinkList::Empty (this=0x6755b8)
     *    at /opt/step/scl/src/clstepcore/SingleLinkList.inline.cc:32
     * #2  0x00007ffff77895b8 in SingleLinkList::~SingleLinkList (this=0x6755b8, __in_chrg=<optimized out>)
     *    at /opt/step/scl/src/clstepcore/SingleLinkList.inline.cc:26
     * #3  0x00007ffff7788f64 in STEPattributeList::~STEPattributeList (this=0x6755b8, __in_chrg=<optimized out>)
     *    at /opt/step/scl/src/clstepcore/STEPattributeList.cc:27
     * #4  0x00007ffff77778e0 in SDAI_Application_instance::~SDAI_Application_instance (this=0x6755a0,
     *    __in_chrg=<optimized out>) at /opt/step/scl/src/clstepcore/sdaiApplication_instance.cc:42
     * #5  0x00007ffff7bdb163 in SdaiBuynary::~SdaiBuynary (this=0x6755a0, __in_chrg=<optimized out>)
     *    at /opt/step/scl/build/binary_index/SdaiTEST_BINARY_INDEX.cc:46
     * #6  0x00007ffff7bdb1b6 in SdaiBuynary::~SdaiBuynary (this=0x6755a0, __in_chrg=<optimized out>)
     *    at /opt/step/scl/build/binary_index/SdaiTEST_BINARY_INDEX.cc:46
     * #7  0x00007ffff79c8656 in MgrNode::~MgrNode (this=0x67f790, __in_chrg=<optimized out>)
     *    at /opt/step/scl/src/cleditor/mgrnode.cc:69
     * #8  0x00007ffff79c86ee in MgrNode::~MgrNode (this=0x67f790, __in_chrg=<optimized out>)
     *    at /opt/step/scl/src/cleditor/mgrnode.cc:75
     * #9  0x00007ffff79c8efd in MgrNodeArray::DeleteEntries (this=0x637940)
     *    at /opt/step/scl/src/cleditor/mgrnodearray.cc:84
     * #10 0x00007ffff79c8d7f in MgrNodeArray::~MgrNodeArray (this=0x637940, __in_chrg=<optimized out>)
     *    at /opt/step/scl/src/cleditor/mgrnodearray.cc:60
     * #11 0x00007ffff79c8dde in MgrNodeArray::~MgrNodeArray (this=0x637940, __in_chrg=<optimized out>)
     *    at /opt/step/scl/src/cleditor/mgrnodearray.cc:61
     * #12 0x00007ffff79c7554 in InstMgr::~InstMgr (this=0x7fffffffe030, __in_chrg=<optimized out>)
     *    at /opt/step/scl/src/cleditor/instmgr.cc:58
     * #13 0x0000000000402133 in main (argc=2, argv=0x7fffffffe1f8)
     *    at /opt/step/scl/test/cpp/schema_dependent/binary_index_derived.cc:25
     */
    
}
