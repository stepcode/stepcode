#ifndef  SDAIALL_CC
#define  SDAIALL_CC
// This file was generated by fedex_plus.  You probably don't want to edit
// it since your modifications will be lost if fedex_plus is used to
// regenerate it.
/* $Id$  */
#include <schema.h>

void
InitSchemasAndEnts( Registry & reg ) {
    Interface_spec_ptr is;
    Used_item_ptr ui;
    Referenced_item_ptr ri;
    Uniqueness_rule_ptr ur;
    Where_rule_ptr wr;
    Global_rule_ptr gr;
    // Schema:  SdaiEXAMPLE_SCHEMA
    s_example_schema = new Schema( "Example_Schema" );
    s_example_schema->AssignModelContentsCreator(
        ( ModelContentsCreator ) create_SdaiModel_contents_example_schema );
    reg.AddSchema( *s_example_schema );

    //  *****  Initialize the Types
    example_schemat_color = new EnumTypeDescriptor(
        "Color",  // Name
        sdaiENUMERATION,  // FundamentalType
        s_example_schema, // Originating Schema
        "ENUMERATION of (red, green, blue, yellow, orange, white, black, brown)", // Description
        ( EnumCreator ) create_SdaiColor_var ); // Creator function
    s_example_schema->AddType( example_schemat_color );
    example_schemat_label = new TypeDescriptor(
        "Label",  // Name
        sdaiSTRING,   // FundamentalType
        s_example_schema, // Originating Schema
        "STRING" );   // Description
    s_example_schema->AddType( example_schemat_label );
    example_schemat_point = new TypeDescriptor(
        "Point",  // Name
        sdaiREAL, // FundamentalType
        s_example_schema, // Originating Schema
        "REAL" ); // Description
    s_example_schema->AddType( example_schemat_point );
    example_schemat_length_measure = new TypeDescriptor(
        "Length_Measure", // Name
        sdaiREAL, // FundamentalType
        s_example_schema, // Originating Schema
        "REAL" ); // Description
    s_example_schema->AddType( example_schemat_length_measure );

    //  *****  Initialize the Entities
    example_schemae_poly_line = new EntityDescriptor(
        "Poly_Line", s_example_schema, SCLLOG( LFalse ), SCLLOG( LFalse ),
        ( Creator ) create_SdaiPoly_line );
    s_example_schema->AddEntity( example_schemae_poly_line );
    example_schemae_shape = new EntityDescriptor(
        "Shape", s_example_schema, SCLLOG( LFalse ), SCLLOG( LFalse ),
        ( Creator ) create_SdaiShape );
    s_example_schema->AddEntity( example_schemae_shape );
    example_schemae_rectangle = new EntityDescriptor(
        "Rectangle", s_example_schema, SCLLOG( LFalse ), SCLLOG( LFalse ),
        ( Creator ) create_SdaiRectangle );
    s_example_schema->AddEntity( example_schemae_rectangle );
    example_schemae_square = new EntityDescriptor(
        "Square", s_example_schema, SCLLOG( LFalse ), SCLLOG( LFalse ),
        ( Creator ) create_SdaiSquare );
    s_example_schema->AddEntity( example_schemae_square );
    example_schemae_triangle = new EntityDescriptor(
        "Triangle", s_example_schema, SCLLOG( LFalse ), SCLLOG( LFalse ),
        ( Creator ) create_SdaiTriangle );
    s_example_schema->AddEntity( example_schemae_triangle );
    example_schemae_circle = new EntityDescriptor(
        "Circle", s_example_schema, SCLLOG( LFalse ), SCLLOG( LFalse ),
        ( Creator ) create_SdaiCircle );
    s_example_schema->AddEntity( example_schemae_circle );
    example_schemae_line = new EntityDescriptor(
        "Line", s_example_schema, SCLLOG( LFalse ), SCLLOG( LFalse ),
        ( Creator ) create_SdaiLine );
    s_example_schema->AddEntity( example_schemae_line );
    example_schemae_cartesian_point = new EntityDescriptor(
        "Cartesian_Point", s_example_schema, SCLLOG( LFalse ), SCLLOG( LFalse ),
        ( Creator ) create_SdaiCartesian_point );
    s_example_schema->AddEntity( example_schemae_cartesian_point );

    //////////////// USE statements
    //////////////// REFERENCE statements
}

#endif
