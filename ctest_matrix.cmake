# run_ctest.cmake
# `ctest -S run_ctest.cmake`

set( CTEST_SOURCE_DIRECTORY . )
set( CTEST_BINARY_DIRECTORY build_matrix )
set( CTEST_CMAKE_GENERATOR "Unix Makefiles" )
set( CTEST_MEMORYCHECK_COMMAND /usr/bin/valgrind )
set( CTEST_INITIAL_CACHE "
SITE:STRING=${CTEST_SITE}
BUILDNAME:STRING=${CTEST_BUILD_NAME}
ENABLE_TESTING:BOOL=ON
CMAKE_BUILD_TYPE:STRING=Debug
")

# find number of processors, for faster builds
# from http://www.kitware.com/blog/home/post/63
if(NOT DEFINED PROCESSOR_COUNT)
  # Unknown:
  set(PROCESSOR_COUNT 0)

  # Linux:
  set(cpuinfo_file "/proc/cpuinfo")
  if(EXISTS "${cpuinfo_file}")
    file(STRINGS "${cpuinfo_file}" procs REGEX "^processor.: [0-9]+$")
    list(LENGTH procs PROCESSOR_COUNT)
  endif()

  # Mac:
  if(APPLE)
    find_program(cmd_sys_pro "system_profiler")
    if(cmd_sys_pro)
      execute_process(COMMAND ${cmd_sys_pro} OUTPUT_VARIABLE info)
      string(REGEX REPLACE "^.*Total Number Of Cores: ([0-9]+).*$" "\\1"
        PROCESSOR_COUNT "${info}")
    endif()
  endif()

  # Windows:
  if(WIN32)
    set(PROCESSOR_COUNT "$ENV{NUMBER_OF_PROCESSORS}")
  endif()
endif()

set(CTEST_BUILD_FLAGS "-j${PROCESSOR_COUNT}")


ctest_start(matrix)
ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})
ctest_configure( BUILD "${CTEST_BINARY_DIRECTORY}" OPTIONS -DENABLE_TESTING=ON )
ctest_build( BUILD "${CTEST_BINARY_DIRECTORY}" )
message("running tests")
ctest_test( BUILD "${CTEST_BINARY_DIRECTORY}" PARALLEL_LEVEL ${PROCESSOR_COUNT}
                        INCLUDE_LABEL "cpp_schema_....*" )

message("running python script")
execute_process( COMMAND python ../misc/wiki-scripts/update-matrix.py
                 WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY} )
