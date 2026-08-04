#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "clstepcore/ExpDict.h"
#include "clstepcore/sdaiSelect.h"

static bool expect( bool condition, const char * message ) {
    if( !condition ) {
        std::cerr << message << std::endl;
    }
    return condition;
}

int main() {
    TypeDescriptor integer_type( "integer_choice", sdaiINTEGER, 0, "INTEGER" );
    TypeDescriptor string_type( "string_choice", sdaiSTRING, 0, "STRING" );
    SelectTypeDescriptor choice_type(
        ~( sdaiINTEGER | sdaiSTRING ), "choice", sdaiSELECT, 0,
        "SELECT (integer_choice, string_choice)" );
    choice_type.Elements().AddNode( &integer_type );
    choice_type.Elements().AddNode( &string_type );

    bool pass = true;
    SDAI_Select value( &choice_type );
    pass &= expect( value.SetInteger( &integer_type, 42 ),
                    "descriptor SELECT rejected an integer choice" );
    pass &= expect( value.IntegerValue() && *value.IntegerValue() == 42,
                    "descriptor SELECT did not retain its integer value" );

    std::ostringstream integer_output;
    value.STEPwrite_content( integer_output );
    pass &= expect( integer_output.str() == "42",
                    "descriptor SELECT wrote an incorrect integer" );

    SDAI_Select copy( value );
    pass &= expect( copy.IntegerValue() && *copy.IntegerValue() == 42,
                    "descriptor SELECT copy lost its integer value" );

    ErrorDescriptor error;
    pass &= expect( value.StrToVal( "17", "integer_choice", &error ) == SEVERITY_NULL,
                    "descriptor SELECT could not parse an integer" );
    pass &= expect( value.IntegerValue() && *value.IntegerValue() == 17,
                    "descriptor SELECT parsed the wrong integer" );

    SDAI_String text( "descriptor driven" );
    pass &= expect( value.SetString( &string_type, text ),
                    "descriptor SELECT rejected a string choice" );
    pass &= expect( value.StringValue() &&
                    std::string( value.StringValue()->c_str() ) == "descriptor driven",
                    "descriptor SELECT did not retain its string value" );

    std::ostringstream string_output;
    value.STEPwrite_content( string_output );
    pass &= expect( string_output.str() == "descriptor driven",
                    "descriptor SELECT wrote an incorrect string" );

    EntityDescriptor entity_type( "entity_choice", 0, LFalse, LFalse );
    SelectTypeDescriptor entity_choice_type(
        ~sdaiINSTANCE, "entity_choice_select", sdaiSELECT, 0,
        "SELECT (entity_choice)" );
    entity_choice_type.Elements().AddNode( &entity_type );
    SDAI_Application_instance entity( 99 );
    entity.eDesc = &entity_type;
    SDAI_Select entity_value( &entity_choice_type );
    pass &= expect( entity_value.SetEntity( &entity ),
                    "descriptor SELECT rejected an entity choice" );
    pass &= expect( entity_value.EntityValue() == &entity,
                    "descriptor SELECT did not retain its entity value" );

    std::ostringstream entity_output;
    entity_value.STEPwrite_content( entity_output );
    pass &= expect( entity_output.str() == "#99",
                    "descriptor SELECT wrote an incorrect entity reference" );

    value.set_null();
    pass &= expect( value.is_null(), "descriptor SELECT did not clear its value" );

    return pass ? EXIT_SUCCESS : EXIT_FAILURE;
}
