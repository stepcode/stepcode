CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
#c++ tests that depend on a particular schema

INCLUDE_DIRECTORIES( ${SCL_SOURCE_DIR}/src/cldai ${SCL_SOURCE_DIR}/src/cleditor ${SCL_SOURCE_DIR}/src/clutils
         ${SCL_SOURCE_DIR}/src/clstepcore ${SCL_SOURCE_DIR}/src/base )

# ${name} is used for the C++ file name (${name}.cc), and as the suffix for the target and test names
# ${sdai_lib} is the name of the schema lib that is used
# ${args} are additional args for the test executable
FUNCTION( add_schema_dependent_test name sdai_lib args )
    add_executable( tst_${name} "${name}.cc" )
    set_target_properties( tst_${name} PROPERTIES COMPILE_FLAGS "-I${CMAKE_BINARY_DIR}/${sdai_lib}" )
    set_target_properties( tst_${name} PROPERTIES EXCLUDE_FROM_ALL ON )
    target_link_libraries( tst_${name} sdai_${sdai_lib} )
    add_test( NAME test_${name}
              WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
              COMMAND tst_${name} ${args} )
    set_tests_properties( test_${name} PROPERTIES DEPENDS build_cpp_sdai_${sdai_lib} LABELS cpp_schema_specific )
ENDFUNCTION( add_schema_dependent_test name sdai_lib args )

BUILD_A_SCHEMA( ${SCL_SOURCE_DIR}/test/unitary_schemas/array_bounds_expr.exp )
add_schema_dependent_test( "aggregate_bound_runtime" "array_bounds_expr"
                           "${SCL_SOURCE_DIR}/test/p21/test_array_bounds.p21" )

add_test( NAME test_aggregate_bound_runtime_FAIL1
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
          COMMAND tst_aggregate_bound_runtime "${SCL_SOURCE_DIR}/test/p21/test_array_bounds_FAIL1.p21" )
set_tests_properties( test_aggregate_bound_runtime_FAIL1 PROPERTIES
                      DEPENDS test_aggregate_bound_runtime WILL_FAIL true LABELS cpp )
