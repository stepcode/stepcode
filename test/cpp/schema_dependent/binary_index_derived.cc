
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

//FIXME incomplete
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
        STEPattributeList &sal = ent->attributes;
        STEPattribute sa;
//         AttrDescItr adi( ent->attributes );
//         while 0 != (
        for( int i = 0; i < cnt; i++ ) {
            sa = sal[i];
            if( ( sa.aDesc->Derived() == LTrue ) && ( sa.aDesc->BaseType() == sdaiBINARY ) ) {
                cout << "derived: " << sa.ptr.b->asStr() << endl;
            } else {
                cout << "not derived: " << sa.ptr.b->asStr() << endl;
            }
        }
        MgrNode* mnode = instance_list.FindFileId( ent->StepFileId() );
        search_index = instance_list.GetIndex( mnode ) + 1;
    }
}
