
/** \file schemaScanner.c
 * This file, along with part of libexpress, are compiled (at configure time)
 * into a static executable. This executable is a schema scanner that is used
 * by CMake to determine what files exp2cxx will create. Otherwise, we'd need
 * to use a few huge files - there is no other way to tell CMake what the
 * generated files will be called.
 */

extern "C" {
#include "expparse.h"
#include "expscan.h"
#include "express/scope.h"
#include "genCxxFilenames.h"
#include "sc_mkdir.h"

#include <string.h>
}

#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>

int multiple_inheritance = 0;

using std::string;
using std::stringstream;
using std::endl;

bool isBuiltin( const Type t ) {
    switch( TYPEget_body( t )->type ) {
        case integer_:
        case real_:
        case string_:
        case binary_:
        case boolean_:
        case number_:
        case logical_:
            return true;
        case aggregate_:
        case bag_:
        case set_:
        case list_:
        case array_:
            return true;
            /* this probably always evaluates to true - ought to check */
/*
            if( TYPEget_body( t )->base ) {
                return isBuiltin( TYPEget_body( t )->base );
            }
            return false;
*/
        default:
            break;
    }
    return false;
}

void writeLists( const char * schema_name, stringstream & eh, stringstream & ei, stringstream & th, stringstream & ti ) {
    sc_mkdir( schema_name );
    string cmListsPath = schema_name;
    cmListsPath += "/CMakeLists.txt";
    std::ofstream cmLists;
    cmLists.open( cmListsPath.c_str() );

    cmLists << "# -----  GENERATED FILE  -----" << endl;
    cmLists << "# -----   Do not edit!   -----" << endl << endl;
    cmLists << "PROJECT( sdai_" << schema_name << " )" << endl << endl;
    cmLists << "# headers to be installed - 3 lists - entity, type, misc" << endl << endl;

    cmLists << "set( " << schema_name << "_entity_hdrs" << endl;
    cmLists << eh;
    cmLists << "   )" << endl << endl;

    cmLists << "set( " << schema_name << "_type_hdrs" << endl;
    cmLists << th;
    cmLists << "   )" << endl << endl;

    cmLists << "set( " << schema_name << "_misc_hdrs" << endl;
    cmLists << " TODO name the headers here "; //TODO
    cmLists << "   )" << endl << endl;

    cmLists << "# install all headers" << endl;
    cmLists << "install( FILES \"${" << schema_name << "_entity_hdrs} ${" << schema_name;
    cmLists << "_type_hdrs} ${" << schema_name << "_misc_hdrs}\"" << endl;
    cmLists << "         DESTINATION \"schemas/" << schema_name << "\" )" << endl << endl;

    cmLists << "# implementation files - 3 lists" << endl << endl;

    cmLists << "set( " << schema_name << "_entity_impls" << endl;
    cmLists << ei;
    cmLists << "   )" << endl << endl;

    cmLists << "set( " << schema_name << "_type_impls" << endl;
    cmLists << ti;
    cmLists << "   )" << endl << endl;

    cmLists << "set( " << schema_name << "_misc_impls" << endl;
    cmLists << " TODO name the files here "; //TODO
    cmLists << "   )" << endl << endl;

    cmLists << "# targets, logic, etc are within a set of macros shared by all schemas" << endl;
    cmLists << "include( \"cmake/schemaMacros.cmake\" )" << endl;
    cmLists << "BUILD_SCHEMA( \"" << schema_name << "\" \"${" << schema_name << "_entity_impls} ";
    cmLists << "${" << schema_name << "_type_impls} ${" << schema_name << "_misc_impls}\" )" << endl;


    //TODO assemble absolute path to schema dir and print it to stdout, so CMake can use it with add_subdirectory()
}

void printSchemaFilenames( Schema sch ){
    stringstream typeHeaders, typeImpls, entityHeaders, entityImpls;
    int ecount = 0, tcount = 0;

    DictionaryEntry de;
    Generic x;
    filenames_t fn;
    DICTdo_init( sch->symbol_table, &de );
    while( 0 != ( x = DICTdo( &de ) ) ) {
        switch( DICT_type ) {
            case OBJ_ENTITY:
                fn = getEntityFilenames( ( Entity ) x );
                entityHeaders << std::setw( 30 ) << fn.header;
                entityImpls << std::setw( 30 ) << fn.impl;
                ++ecount;
                if( ( ecount % 4 ) == 0 ) {
                    // 4 columns
                    entityHeaders << endl;
                    entityImpls << endl;
                }
                break;
            case OBJ_TYPE: {
                Type t = ( Type ) x;
                if( TYPEis_enumeration( t ) && ( TYPEget_head( t ) ) ) {
                    /* t is a renamed enum type, for which exp2cxx
                     * will print a typedef in an existing file */
                    break;
                }
                if( isBuiltin( t ) ) {
                    /* skip builtin types */
                    break;
                }
                fn = getTypeFilenames( t );
                typeHeaders << std::setw( 30 ) << fn.header;
                typeImpls << std::setw( 30 ) << fn.impl;
                ++tcount;
                if( ( tcount % 4 ) == 0 ) {
                    typeHeaders << endl;
                    typeImpls << endl;
                }
                break;
            }
            /* case OBJ_FUNCTION:
            case OBJ_PROCEDURE:
            case OBJ_RULE: */
            default:
                /* ignore everything else */
                /* TODO: if DEBUG is defined, print the names of these to stderr */
                break;
        }
    }
    //write the CMakeLists.txt
    writeLists( sch->symbol.name, entityHeaders, entityImpls, typeHeaders, typeImpls );
}

int main( int argc, char ** argv ) {
    /* TODO init globals! */

    Schema schema;
    DictionaryEntry de;
    /* copied from fedex.c */
    Express model;
    if( ( argc != 2 ) || ( strlen( argv[1] ) < 1 ) ) {
        fprintf( stderr, "\nUsage: %s file.exp\nOutput: a CMakeLists.txt to build the schema,", argv[0] );
        fprintf( stderr, " containing file names for entities, types, etc\n" );
        fprintf( stderr, "also prints (to stdout) the absolute path to the directory CMakeLists.txt was created in\n" );
        exit( EXIT_FAILURE );
    }
    EXPRESSprogram_name = argv[0];
    input_filename = argv[1];

    EXPRESSinitialize();

    model = EXPRESScreate();
    EXPRESSparse( model, ( FILE * )0, input_filename );
    if( ERRORoccurred ) {
        EXPRESSdestroy( model );
        exit( EXIT_FAILURE );
    }
    EXPRESSresolve( model );
    if( ERRORoccurred ) {
        int result = EXPRESS_fail( model );
        EXPRESScleanup();
        EXPRESSdestroy( model );
        exit( result );
    }

    DICTdo_type_init( model->symbol_table, &de, OBJ_SCHEMA );
    while( 0 != ( schema = ( Schema )DICTdo( &de ) ) ) {
        printSchemaFilenames( schema );
    }

    EXPRESSdestroy( model );
    exit( EXIT_SUCCESS );
}
