CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
#c++ tests that depend on a particular schema

INCLUDE_DIRECTORIES( ${SCL_SOURCE_DIR}/src/cldai ${SCL_SOURCE_DIR}/src/cleditor ${SCL_SOURCE_DIR}/src/clutils
         ${SCL_SOURCE_DIR}/src/clstepcore ${SCL_SOURCE_DIR}/src/base )

# ${name} is used for the C++ file name (${name}.cc), and as the suffix for the target and test names
# ${sdai_lib} is the name of the schema lib that is used
# ${exe_args} are additional args for the test executable
# two optional args: ARGV3, ARGV4 are compile flags and libs, respectively.
FUNCTION( add_schema_dependent_test name sdai_lib exe_args )
    add_executable( tst_${name} "${name}.cc" )
    set_target_properties( tst_${name} PROPERTIES COMPILE_FLAGS "-I${CMAKE_BINARY_DIR}/${sdai_lib} ${ARGV3}" EXCLUDE_FROM_ALL ON )
    target_link_libraries( tst_${name} sdai_${sdai_lib} ${ARGV4} )
    add_test( NAME build_cpp_${name}
              WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
              COMMAND ${CMAKE_COMMAND} --build .
                                       --target tst_${name}
                                       --config $<CONFIGURATION> )
    set_tests_properties( build_cpp_${name} PROPERTIES DEPENDS build_cpp_sdai_${sdai_lib}
                          LABELS cpp_schema_specific )
    add_test( NAME test_${name}
              WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
              COMMAND $<TARGET_FILE:tst_${name}> ${exe_args} )
    set_tests_properties( test_${name} PROPERTIES DEPENDS build_cpp_${name}
                          LABELS cpp_schema_specific )
ENDFUNCTION( add_schema_dependent_test name sdai_lib exe_args )

#if you need a schema that won't be built automatically (i.e. not in scl/data), put it in here
#temporarily disabling testing ensures that the schema will be built before these tests run
SET(SCL_ENABLE_TESTING OFF)
  BUILD_A_SCHEMA( ${SCL_SOURCE_DIR}/test/unitary_schemas/array_bounds_expr.exp )
SET(SCL_ENABLE_TESTING ON)

add_schema_dependent_test( "aggregate_bound_runtime" "array_bounds_expr"
                           "${SCL_SOURCE_DIR}/test/p21/test_array_bounds.p21" )
add_test( NAME test_aggregate_bound_runtime_FAIL1
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
          COMMAND $<TARGET_FILE:tst_aggregate_bound_runtime> "${SCL_SOURCE_DIR}/test/p21/test_array_bounds_FAIL1.p21" )
set_tests_properties( test_aggregate_bound_runtime_FAIL1 PROPERTIES
                      DEPENDS test_aggregate_bound_runtime WILL_FAIL true
                      LABELS cpp_schema_specific )

if(HAVE_STD_THREAD)
# for best results, use a large file. as1-oc-214.stp is currently the largest file in the repo that scl works with.
add_schema_dependent_test( "stepfile_rw_progress" "AP214E3_2010"
                           "${SCL_SOURCE_DIR}/data/ap214e3/as1-oc-214.stp" "-pthread -std=c++0x" "pthread" )
endif(HAVE_STD_THREAD)
