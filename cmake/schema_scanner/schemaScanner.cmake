
#include this file


# SC_ROOT: SC root dir
# SC_BUILDDIR: SC build dir, so generated headers can be found
# SCANNER_SRC_DIR: dir this file is in
# SCANNER_OUT_DIR: location to place binary

set( SCANNER_SRC_DIR ${SC_CMAKE_DIR}/schema_scanner )
set( SCANNER_OUT_DIR ${SC_BINARY_DIR}/schema_scanner )

#write a cmake file for the cache. the alternative is a very long
# command line - and the command line can't have newlines in it
set( initial_scanner_cache ${SCANNER_OUT_DIR}/initial_scanner_cache.cmake )
file( WRITE ${initial_scanner_cache} "
set( SC_ROOT \"${SC_SOURCE_DIR}\" CACHE STRING \"root dir\" )
set( SC_BUILDDIR \"${SC_BINARY_DIR}\" CACHE PATH \"build dir\" )
set( OUTDIR \"${SCANNER_OUT_DIR}\" CACHE PATH \"out dir\" )
set( CALLED_FROM \"STEPCODE_CMAKELISTS\" CACHE STRING \"verification\" )
set( CMAKE_BUILD_TYPE \"Debug\" CACHE STRING \"build type\" )
set( CMAKE_C_COMPILER \"${CMAKE_C_COMPILER}\" CACHE STRING \"compiler\" )
" )

execute_process( COMMAND ${CMAKE_COMMAND} -E make_directory ${SCANNER_OUT_DIR} )
execute_process( COMMAND ${CMAKE_COMMAND} -C ${initial_scanner_cache} ${SCANNER_SRC_DIR}
                 WORKING_DIRECTORY ${SCANNER_OUT_DIR}
                 TIMEOUT 10
                 OUTPUT_QUIET
                 RESULT_VARIABLE _ss_config_stat
                )
execute_process( COMMAND ${CMAKE_COMMAND} --build ${SCANNER_OUT_DIR} --config Debug --clean-first
                 WORKING_DIRECTORY ${SCANNER_OUT_DIR}
                 TIMEOUT 30 # should take far less than 30s
                 #OUTPUT_QUIET
                 RESULT_VARIABLE _ss_build_stat
               )
# replace with if...message FATAL_ERROR ...
message( "scanner config status: ${_ss_config_stat}." )
message( "scanner build status: ${_ss_build_stat}." )

# macro LIST_SCHEMA_FILES
# SCHEMA - path to the schema
# RES_VAR - result variable, which will contain a list of
MACRO( LIST_SCHEMA_FILES SCHEMA RES_VAR )
  execute_process( COMMAND ${SCANNER_OUT_DIR}/schema_scanner ${SCHEMA}
                   RESULT_VARIABLE _ss_stat
                   OUTPUT_VARIABLE _ss_out
                   ERROR_QUIET
                 )
  #check stat, out
  message("scan stat ${_ss_stat} - output ${_ss_out}")
  set( ${RES_VAR} ${_ss_out} )
  configure_file( ${SCHEMA} ${SCANNER_OUT_DIR}/dummy ) #FIXME - does this need unique file names?
ENDMACRO( LIST_SCHEMA_FILES SCHEMA RES_VAR )
