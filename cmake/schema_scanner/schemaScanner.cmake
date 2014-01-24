
# this file should be included from data/CMakeLists.txt
#
# at configure time, this builds a small program
# which parses express schemas to determine what files
# exp2cxx will create for that schema.
#
# The LIST_SCHEMA_FILES macro is to be used to run this
# program. It will set variables for schema name(s),
# headers, and implementation files.

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
set( CMAKE_CXX_COMPILER \"${CMAKE_CXX_COMPILER}\" CACHE STRING \"compiler\" )
" )

execute_process( COMMAND ${CMAKE_COMMAND} -E make_directory ${SC_BINARY_DIR}/schemas )
execute_process( COMMAND ${CMAKE_COMMAND} -E make_directory ${SCANNER_OUT_DIR} )
execute_process( COMMAND ${CMAKE_COMMAND} -C ${initial_scanner_cache} ${SCANNER_SRC_DIR}
                 WORKING_DIRECTORY ${SCANNER_OUT_DIR}
                 TIMEOUT 10
                 OUTPUT_VARIABLE _ss_config_out
                 RESULT_VARIABLE _ss_config_stat
                 ERROR_VARIABLE _ss_config_err
                )
if( NOT ${_ss_config_stat} STREQUAL "0" )
  message( FATAL_ERROR "Scanner config status: ${_ss_config_stat}. stdout:\n${_ss_config_out}\nstderr:\n${_ss_config_err}" )
endif( NOT ${_ss_config_stat} STREQUAL "0" )
execute_process( COMMAND ${CMAKE_COMMAND} --build ${SCANNER_OUT_DIR} --config Debug --clean-first
                 WORKING_DIRECTORY ${SCANNER_OUT_DIR}
                 TIMEOUT 30 # should take far less than 30s
                 OUTPUT_VARIABLE _ss_build_out
                 RESULT_VARIABLE _ss_build_stat
                 ERROR_VARIABLE _ss_build_err
               )
if( NOT ${_ss_build_stat} STREQUAL "0" )
  message( FATAL_ERROR "Scanner build status: ${_ss_build_stat}. stdout:\n${_ss_build_out}\nstderr:\n${_ss_build_err}" )
endif( NOT ${_ss_build_stat} STREQUAL "0" )

# not sure if it makes sense to install this or not...
install( PROGRAMS ${SCANNER_OUT_DIR}/schema_scanner DESTINATION ${BIN_INSTALL_DIR} )

# macro SCHEMA_CMLIST
# runs the schema scanner on one express file, creating a CMakeLists.txt file for each schema found. Those files are added via add_subdirectory().
#
# SCHEMA_FILE - path to the schema
# TODO should we have a result variable to return schema name(s) found?
MACRO( SCHEMA_CMLIST SCHEMA_FILE )
  execute_process( COMMAND ${SCANNER_OUT_DIR}/schema_scanner ${SCHEMA_FILE}
                   WORKING_DIRECTORY ${SC_BINARY_DIR}/schemas
                   RESULT_VARIABLE _ss_stat
                   OUTPUT_VARIABLE _ss_out
                   ERROR_VARIABLE _ss_err
                 )
  if( NOT "${_ss_stat}" STREQUAL "0" )
    #check size of output, put in file if large?
    message( FATAL_ERROR "Schema scan exited with error code ${_ss_stat}. stdout:\n${_ss_out}\nstderr:\n${_ss_err}" )
  endif( NOT "${_ss_stat}" STREQUAL "0" )
  # scanner output format: each line contains an absolute path. each path is a dir containing a CMakeLists for one schema
  # there will be usually be a single line of output, but it is not illegal for multiple schemas to exist in one .exp file
  string( STRIP "${_ss_out}" _ss_stripped )
  string( REGEX REPLACE "\\\n" ";" _list ${_ss_stripped} )
  foreach( _dir ${_list} )
#    message( "${SCHEMA_FILE}: ${_dir}" )
    add_subdirectory( ${_dir} ${_dir} ) #specify source and binary dirs as the same
  endforeach( _dir ${_ss_out} )
  configure_file( ${SCHEMA_FILE} ${SCANNER_OUT_DIR}/${_schema} ) #if multiple schemas in one file, _schema is the last one printed.
ENDMACRO( SCHEMA_CMLIST SCHEMA_FILE )
