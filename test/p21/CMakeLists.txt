CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#test part 21 files
#currently, the only tests are for the schema name

set( schema_name_tests_head "
ISO-10303-21;
HEADER;
FILE_DESCRIPTION(('SCL test file'),'2;1');
FILE_NAME('test.stp','2011-11-29T',('mp'),(''),'0','1','2');
")
set( schema_name_tests_tail "
ENDSEC;
DATA;
#1=DRAUGHTING_PRE_DEFINED_COLOUR('green');
#2=COLOUR_RGB('',1.1E-2,1.2E-2,1.E0);
ENDSEC;
END-ISO-10303-21;
")

set( schema_name_good     "FILE_SCHEMA(('AUTOMOTIVE_DESIGN'));" )
set( schema_name_good_asn "FILE_SCHEMA(('AUTOMOTIVE_DESIGN { 1 0 10303 214 1 1 1 1 }'));" )
set( schema_name_mismatch "FILE_SCHEMA(('DOES_NOT_MATCH'));" )

set( p21read_ap214 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/p21read_sdai_AP214E3_2010" )

file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/exch_file_good.p21      "${schema_name_tests_head}${schema_name_good}${schema_name_tests_tail}" )
file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/exch_file_good_asn.p21  "${schema_name_tests_head}${schema_name_good_asn}${schema_name_tests_tail}" )
file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/exch_file_mismatch.p21  "${schema_name_tests_head}${schema_name_mismatch}${schema_name_tests_tail}" )

add_test( test_good_schema_name      ${p21read_ap214}    ${CMAKE_CURRENT_BINARY_DIR}/exch_file_good.p21 )
add_test( test_good_schema_name_asn  ${p21read_ap214}    ${CMAKE_CURRENT_BINARY_DIR}/exch_file_good_asn.p21 )
add_test( test_mismatch_schema_name  ${p21read_ap214}    ${CMAKE_CURRENT_BINARY_DIR}/exch_file_mismatch.p21 )
add_test( test_ignore_schema_name    ${p21read_ap214} -i ${CMAKE_CURRENT_BINARY_DIR}/exch_file_mismatch.p21 )

#not sure how multiple schema names should be handled for testing, by p21read, or by the schema and stepeditor libs
#add_test( multi_schema )
#rotator_asm:
# FILE_SCHEMA(('CONFIG_CONTROL_DESIGN', 'GEOMETRIC_VALIDATION_PROPERTIES_MIM','SHAPE_APPEARANCE_LAYER_MIM'));

set_tests_properties( test_good_schema_name test_good_schema_name_asn test_mismatch_schema_name test_ignore_schema_name
                      PROPERTIES DEPENDS build_cpp_sdai_AP214E3_2010 LABELS exchange_file )
set_tests_properties( test_mismatch_schema_name PROPERTIES WILL_FAIL TRUE )
