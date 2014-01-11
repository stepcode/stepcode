
#include this file
#

# SC_ROOT: SC root dir
# SC_BUILDDIR: SC build dir, so generated headers can be found
# SCANNER_SRC_DIR: dir this file is in
# SCANNER_OUT_DIR: location to place binary

# debugging only! ######################################################################
set(SC_BUILDDIR ../../build)
set(SC_ROOT ../..)
set(SCANNER_SRC_DIR .)
set(SCANNER_OUT_DIR ../../build/scanner)

# no way to copy config data from this project to sub project?

#mkdir OUTDIR
#cd OUTDIR; cmake ${SCANNER_SRC_DIR} -DSC_ROOT=${SC_ROOT} -DSC_BUILDDIR=${SC_BUILDDIR} -DOUTDIR=${SCANNER_OUT_DIR} -DCALLED_FROM=STEPCODE_CMAKELISTS
# cmake --build
#test exe

#add_custom_target(schema_scanner)

execute_process( COMMAND ${CMAKE_COMMAND} ${SCANNER_SRC_DIR} -DSC_ROOT=${SC_ROOT} -DSC_BUILDDIR=${SC_BUILDDIR}
                            -DOUTDIR=${SCANNER_OUT_DIR} -DCALLED_FROM=STEPCODE_CMAKELISTS
                 COMMAND ${CMAKE_COMMAND} --build ${SCANNER_OUT_DIR} --config Debug --clean-first
                 WORKING_DIRECTORY ${SCANNER_OUT_DIR}
                 TIMEOUT 30 # should take far less than 30s
                 #OUTPUT_QUIET
                 RESULT_VARIABLE _ss_build_stat
               )
# replace with if...message FATAL_ERROR ...
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
  set( ${RES_VAR} ${out} )
ENDMACRO( LIST_SCHEMA_FILES SCHEMA RES_VAR )
