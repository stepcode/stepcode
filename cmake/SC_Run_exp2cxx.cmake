# TODO if oneshot is true, don't run exp2cxx if source files exist... but how to check?
execute_process( COMMAND ${EXE} ${EXP}
                 WORKING_DIRECTORY ${DIR}
                 RESULT_VARIABLE _res
                 OUTPUT_FILE exp2cxx_stdout.txt
                 ERROR_FILE exp2cxx_stderr.txt
               )

if( NOT "${_res}" STREQUAL "0" )
    message( FATAL_ERROR "${EXE} reported an error for ${EXP}.\nsee exp2cxx_stdout.txt and exp2cxx_stderr.txt in ${DIR} for details." )
endif( NOT "${_res}" STREQUAL "0" )
