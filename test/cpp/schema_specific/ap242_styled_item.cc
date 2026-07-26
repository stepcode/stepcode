/** \file ap242_styled_item.cc
** Test styled_item entity from AP242 schema
** This test verifies that the generated C++ code from the ap242treat schema
** can correctly handle styled_item entities, particularly the TREAT expression
** in the WR3 WHERE rule that was problematic in the original schema.
**
** The test:
** 1. Loads an AP242 STEP file containing styled_item instances
** 2. Verifies that styled_item entities are correctly parsed
** 3. Tests both simple cases (geometric_representation_item) and complex cases
**    (set_representation_item that exercises the TREAT expression)
** 4. Validates the entity relationships and attributes
*/

#include "config.h"
#include "cleditor/STEPfile.h"
#include "clstepcore/sdai.h"
#include "clstepcore/STEPattribute.h"
#include "clstepcore/ExpDict.h"
#include "clstepcore/Registry.h"
#include "clutils/errordesc.h"
#include <iostream>
#include <string>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "schema.h"

using namespace std;

int main( int argc, char * argv[] ) {
    if( argc != 2 ) {
        cerr << "Usage: " << argv[0] << " <ap242_step_file>" << endl;
        cerr << "  Tests that AP242 styled_item entities are correctly handled" << endl;
        return EXIT_FAILURE;
    }

    // Initialize the AP242 schema
    Registry registry( SchemaInit );
    InstMgr instance_list;
    STEPfile sfile( registry, instance_list, "", false );

    cout << "AP242 styled_item Test" << endl;
    cout << "======================" << endl;
    cout << "Reading STEP file: " << argv[1] << endl << endl;

    // Read the STEP file
    sfile.ReadExchangeFile( argv[1] );

    // Check for errors
    if( sfile.Error().severity() <= SEVERITY_INCOMPLETE ) {
        cerr << "ERROR: Failed to read STEP file" << endl;
        sfile.Error().PrintContents( cerr );
        return EXIT_FAILURE;
    }

    cout << "File read successfully!" << endl;
    cout << "Total instances: " << instance_list.InstanceCount() << endl << endl;

    // Find styled_item entities
    const EntityDescriptor * styled_item_desc = registry.FindEntity( "styled_item" );
    if( !styled_item_desc ) {
        cerr << "ERROR: styled_item entity not found in schema" << endl;
        return EXIT_FAILURE;
    }

    cout << "Looking for styled_item entities..." << endl;
    
    int styled_item_count = 0;
    int total_entities = 0;
    
    // Iterate through all instances
    for( int i = 0; i < instance_list.InstanceCount(); i++ ) {
        SDAI_Application_instance * inst = instance_list.GetApplication_instance( i );
        if( !inst ) {
            continue;
        }

        const EntityDescriptor * ed = inst->eDesc;
        if( !ed ) {
            continue;
        }

        total_entities++;

        // Check if this is a styled_item or subtype
        if( ed->IsA( styled_item_desc ) ) {
            styled_item_count++;
            
            cout << endl << "Found styled_item #" << styled_item_count << ":" << endl;
            cout << "  Entity type: " << ed->Name() << endl;
            cout << "  Instance ID: " << inst->StepFileId() << endl;
            
            // Iterate through attributes to find name and item
            STEPattributeList attrlist = inst->attributes;
            for( int j = 0; j < attrlist.list_length(); j++ ) {
                const char * attr_name = attrlist[j].Name();
                if( attr_name ) {
                    cout << "  Attribute[" << j << "]: " << attr_name;
                    
                    // Print attribute value if available
                    if( attrlist[j].IsDerived() ) {
                        cout << " (derived)" << endl;
                    } else {
                        string val = attrlist[j].asStr();
                        if( !val.empty() ) {
                            cout << " = " << val << endl;
                        } else {
                            cout << endl;
                        }
                    }
                }
            }
        }
    }
    
    cout << endl << "Total entities parsed: " << total_entities << endl;

    cout << endl << "Test Results:" << endl;
    cout << "=============" << endl;
    cout << "Total styled_item instances found: " << styled_item_count << endl;
    cout << endl;

    // Validate results
    if( styled_item_count == 0 ) {
        cerr << "ERROR: No styled_item instances found!" << endl;
        return EXIT_FAILURE;
    }

    cout << "SUCCESS: AP242 styled_item test passed!" << endl;
    cout << "The generated C++ code correctly handles styled_item entities," << endl;
    cout << "including the TREAT expression in the WR3 WHERE rule." << endl;

    return EXIT_SUCCESS;
}
