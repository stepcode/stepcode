
/** \file schemaScanner.cc
 * This file, along with part of libexpress, are compiled (at configure time)
 * into a static executable. This executable is a schema scanner that is used
 * by CMake to determine what files exp2cxx will create. Otherwise, we'd need
 * to use a few huge files - there is no other way to tell CMake what the
 * generated files will be called.
 */

extern "C" {
#  include "expparse.h"
#  include "expscan.h"
#  include "express/scope.h"
#  include "genCxxFilenames.h"
#  include "sc_mkdir.h"

#  include <string.h>

#  if defined( _WIN32 ) || defined ( __WIN32__ )
#    include <direct.h>
#    define getcwd _getcwd
#  else
#    include <unistd.h>
#  endif
}

#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>

int multiple_inheritance = 0;

using std::string;
using std::stringstream;
using std::endl;
using std::ofstream;
using std::cerr;
using std::cout;

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

/** write a CMakeLists.txt file for the schema; print its directory to stdout for CMake's add_subdirectory() command */
void writeLists( const char * schema_name, stringstream & eh, stringstream & ei, int ecount,
                                           stringstream & th, stringstream & ti, int tcount ) {
    if( mkDirIfNone( schema_name ) < 0 ) {
        cerr << "Error creating directory " << schema_name << " at " << __FILE__ << ":" << __LINE__;
        perror( 0 );
        exit( EXIT_FAILURE );
    }
    size_t nameLen = strlen( schema_name );
    string schema_upper( nameLen, char() );
    for( size_t i = 0; i < nameLen; ++i) {
        schema_upper[i] = toupper(schema_name[i]);
    }
    string cmListsPath = schema_name;
    cmListsPath += "/CMakeLists.txt";
    ofstream cmLists;
    cmLists.open( cmListsPath.c_str() );
    if( !cmLists.good() ) {
        cerr << "error opening file " << cmListsPath << " - exiting." << endl;
        exit( EXIT_FAILURE );
    }
    cmLists << "# -----  GENERATED FILE  -----" << endl;
    cmLists << "# -----   Do not edit!   -----" << endl;
    cmLists << "# schema contains " << ecount << " entities, " << tcount << " types" << endl << endl;
    cmLists << "# list headers so they can be installed - entity, type, misc" << endl << endl;

    cmLists << "set( " << schema_name << "_entity_hdrs" << endl;
    cmLists << eh.str();
    cmLists << "   )" << endl << endl;

    cmLists << "set( " << schema_name << "_type_hdrs" << endl;
    cmLists << th.str();
    cmLists << "   )" << endl << endl;

    cmLists << "set( " << schema_name << "_misc_hdrs" << endl;
    cmLists << "     Sdaiclasses.h   schema.h" << endl;
    cmLists << "     Sdai" << schema_upper << "Helpers.h" << endl;
    cmLists << "     Sdai" << schema_upper << "Names.h" << endl;
    cmLists << "     Sdai" << schema_upper << ".h" << endl;
    cmLists << "   )" << endl << endl;

    cmLists << "# install all headers" << endl;
    cmLists << "install( FILES \"${" << schema_name << "_entity_hdrs} ${" << schema_name;
    cmLists << "_type_hdrs} ${" << schema_name << "_misc_hdrs}\"" << endl;
    cmLists << "         DESTINATION \"include/schemas/" << schema_name << "\" )" << endl << endl;

    cmLists << "# implementation files - 3 lists" << endl << endl;

    cmLists << "set( " << schema_name << "_entity_impls" << endl;
    cmLists << ei.str();
    cmLists << "   )" << endl << endl;

    cmLists << "set( " << schema_name << "_type_impls" << endl;
    cmLists << ti.str();
    cmLists << "   )" << endl << endl;

    cmLists << "set( " << schema_name << "_misc_impls" << endl;
    cmLists << "     SdaiAll.cc    compstructs.cc    schema.cc" << endl;
    cmLists << "     Sdai" << schema_upper << ".cc" << endl;
    cmLists << "     Sdai" << schema_upper << ".init.cc   )" << endl << endl;

    cmLists << "# targets, logic, etc are within a set of macros shared by all schemas" << endl;
    cmLists << "include( ${SC_CMAKE_DIR}/cxxSchemaMacros.cmake )" << endl;

    cmLists << "SCHEMA_TARGETS( \"" << input_filename << "\" \"" << schema_name << "\"" << endl;
    cmLists << "                \"${" << schema_name << "_entity_impls}" << endl;
    cmLists << "                 ${" << schema_name << "_type_impls}" << endl;
    cmLists << "                 ${" << schema_name << "_misc_impls}\" )" << endl;

    cmLists.close();

    char pwd[BUFSIZ] = {0};
    if( getcwd( pwd, BUFSIZ ) ) {
        cout << pwd << "/" << schema_name << endl;
    } else {
        cerr << "Error encountered by getcwd() for " << schema_name << " - exiting. Error was ";
        perror( 0 );
        exit( EXIT_FAILURE );
    }
}

void printSchemaFilenames( Schema sch ){
    const int numColumns = 2;
    const int colWidth = 75;
    const char * tab = "     ";

    stringstream typeHeaders, typeImpls, entityHeaders, entityImpls;
    typeHeaders << tab;
    typeImpls << tab;
    entityHeaders << tab;
    entityImpls << tab;

    int ecount = 0, tcount = 0;

    DictionaryEntry de;
    Generic x;
    filenames_t fn;
    DICTdo_init( sch->symbol_table, &de );
    while( 0 != ( x = DICTdo( &de ) ) ) {
        switch( DICT_type ) {
            case OBJ_ENTITY:
                fn = getEntityFilenames( ( Entity ) x );
                entityHeaders << std::left << std::setw( colWidth ) << fn.header << " ";
                entityImpls << std::left << std::setw( colWidth ) << fn.impl << " ";
                ++ecount;
                if( ( ecount % numColumns ) == 0 ) {
                    // columns
                    entityHeaders << endl << tab;
                    entityImpls << endl << tab;
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
                typeHeaders << std::left << std::setw( colWidth ) << fn.header << " ";
                typeImpls << std::left << std::setw( colWidth ) << fn.impl << " ";
                ++tcount;
                if( ( tcount % numColumns ) == 0 ) {
                    typeHeaders << endl << tab;
                    typeImpls << endl << tab;
                }
                break;
            }
            /* case OBJ_FUNCTION:
             *            case OBJ_PROCEDURE:
             *            case OBJ_RULE: */
            default:
                /* ignore everything else */
                /* TODO: if DEBUG is defined, print the names of these to stderr */
                break;
        }
    }
    //write the CMakeLists.txt
    writeLists( sch->symbol.name, entityHeaders, entityImpls, ecount, typeHeaders, typeImpls, tcount );
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
