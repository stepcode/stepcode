
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

# macro LIST_SCHEMA_FILES
# lists the files created for individual entities or types in the schema,
# but not the schema-level files (i.e. SdaiSCHEMA_NAME.cc)
#
# SCHEMA_FILE - path to the schema
# OUT_PATH_PREFIX - prefixed to the name of each file; easier while building lists than later
# SCHEMA_NAME_RES - result variable, set to the schema name. if multiple schemas in a file, a semicolon-separated list
# HEADERS_RES - result variable, which will contain a list of generated headers
# IMPLS_RES - result variable, which will contain a list of generated implementation files
MACRO( LIST_SCHEMA_FILES SCHEMA_FILE OUT_PATH_PREFIX SCHEMA_NAME_RES HEADERS_RES IMPLS_RES)
  execute_process( COMMAND ${SCANNER_OUT_DIR}/schema_scanner ${SCHEMA_FILE}
                   WORKING_DIRECTORY ${SC_BINARY_DIR}/schemas
                   RESULT_VARIABLE _ss_stat
                   OUTPUT_VARIABLE _ss_out
                   ERROR_VARIABLE _ss_err
                 )
  if( NOT ${_ss_stat} STREQUAL "0" )
    #check size of output, put in file if large?
    message( FATAL_ERROR "Schema scan exited with error code ${_ss_build_stat}. stdout:\n${_ss_out}\nstderr:\n${_ss_err}" )
  endif( NOT ${_ss_stat} STREQUAL "0" )
  # scanner output format
  #  :schema_name:;entity/e_name.h;entity/e_name.cc;type/t_name.h;type/t_name.cc;...;\n
  string( STRIP "${_ss_out}" _scan )
  foreach( _item ${_scan} )
    if( ${_item} MATCHES "^:.*:$" )
      #schema name(s) will be wrapped in colons
      string(REGEX REPLACE "^:(.*):$" "\\1" _schema ${_item})
      list( APPEND ${SCHEMA_NAME_RES} ${_schema} )
    elseif( ${_item} MATCHES ".*\\.h$" )
      # header
      list( APPEND ${HEADERS_RES} ${OUT_PATH_PREFIX}/${_item} )
    elseif( ${_item} MATCHES ".*\\.cc$" )
      # implementation
      list( APPEND ${IMPLS_RES} ${OUT_PATH_PREFIX}/${_item} )
    else()
      # unknown?!
      if( NOT _item STREQUAL "" )
        message( FATAL_ERROR "unrecognized item in schema scanner output: '${_item}'. aborting." )
      endif( NOT _item STREQUAL "" )
    endif( ${_item} MATCHES "^:.*:$" )
  endforeach( _item ${_ss_out} )
  configure_file( ${SCHEMA_FILE} ${SCANNER_OUT_DIR}/${_schema} ) #if multiple schemas in one file, _schema is the last one printed.
ENDMACRO( LIST_SCHEMA_FILES SCHEMA_FILE SCHEMA_NAME_RES HEADERS_RES IMPLS_RES)
