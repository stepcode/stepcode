
#include "expparse.h"
#include "expscan.h"

#include "express/scope.h"
#include "genCxxFilenames.h"

int multiple_inheritance = 0;

void printSchemaFilenames( Schema sch ){
    DictionaryEntry de;
    Generic x;
    filenames_t fn;
    printf( "%s:", sch->symbol.name );
    DICTdo_init( sch->symbol_table, &de );
    while( 0 != ( x = DICTdo( &de ) ) ) {
        switch( DICT_type ) {
            case OBJ_ENTITY:
                fn = getTypeFilenames( ( Entity )x );
                printf( " %s %s", fn.impl, fn.header );
                break;
            case OBJ_TYPE:
                fn = getEntityFilenames( ( Type )x );
                printf( " %s %s", fn.impl, fn.header );
                break;
            /* case OBJ_FUNCTION:
            case OBJ_PROCEDURE:
            case OBJ_RULE: */
            default:
                /* ignored everything else */
                break;
        }
    }
    printf( "\n" );
}

int main( int argc, char ** argv ) {
    /* TODO init globals! */

    Schema schema;
    DictionaryEntry de;
    /* copied from fedex.c */
    Express model;
    if( ( argc != 2 ) || ( strlen( argv[1] ) < 1 ) ) {
        fprintf( stderr, "\nUsage: %s file.exp\nOutput: one line per schema as follows,", argv[0] );
        fprintf( stderr, " containing file names for entities and types\n" );
        fprintf( stderr, "schema_name: entity_name.h entity_name.cc type_name.h type_name.cc ...\n" );
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
    /*EXPRESSresolve( model );
    if( ERRORoccurred ) {
        result = EXPRESS_fail( model );
        EXPRESScleanup();
        EXPRESSdestroy( model );
        exit( EXIT_FAILURE );
    }
    */
    /* don't resolve... just look for schemas, then through schemas for types/entities
       parser adds schemas to     PARSEnew_schemas (??) */

    DICTdo_type_init( model->symbol_table, &de, OBJ_SCHEMA );
    while( 0 != ( schema = ( Schema )DICTdo( &de ) ) ) {
        printSchemaFilenames( schema );
    }

    EXPRESSdestroy( model );
    exit( EXIT_SUCCESS );
}
