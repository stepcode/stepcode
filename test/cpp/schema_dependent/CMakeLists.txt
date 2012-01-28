CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
#c++ tests that depend on a particular schema

# ${name} is used for the C++ file name (${name}.cc), and as the suffix for the target and test names
# ${sdai_lib} is the name of the schema lib that is used
# ${args} are additional args for the test command
FUNCTION( add_schema_dependent_test name sdai_lib args )
    add_executable( tst_${name} "${name}.cc" )
    set_target_properties( tst_${name} PROPERTIES COMPILE_FLAGS "-I${SCL_SOURCE_DIR}/src/cldai -I${SCL_SOURCE_DIR}/src/cleditor -I${SCL_SOURCE_DIR}/src/clutils -I${CMAKE_BINARY_DIR}/${sdai_lib} -I${SCL_SOURCE_DIR}/src/clstepcore" )
    set_target_properties( tst_${name} PROPERTIES EXCLUDE_FROM_ALL ON )
    target_link_libraries( tst_${name} sdai_${sdai_lib} )
    add_test( NAME test_${name}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND tst_${name} ${args} )
    set_tests_properties( test_${name} PROPERTIES DEPENDS build_cpp_sdai_${sdai_lib} LABELS cpp )
ENDFUNCTION( add_schema_dependent_test name sdai_lib args )

add_schema_dependent_test( "binary_index_derived" "binary_index" "${SCL_SOURCE_DIR}/test/p21/test_binary_index.p21" )
add_schema_dependent_test( "stepAttrListSegfault" "binary_index" "${SCL_SOURCE_DIR}/test/p21/test_binary_index.p21" )

