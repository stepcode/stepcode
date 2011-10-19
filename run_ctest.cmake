# run_ctest.cmake
# `ctest -S run_ctest.cmake`


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

######################################################
###########   Set these variables   ##################
######################################################
set( CTEST_SITE "your name here")
set( CTEST_BUILD_NAME "build type, os, arch")
set( CTEST_COMPILER_NAME "cc version" )

message( FATAL_ERROR "Set the above variables and comment this line out!" )

######################################################

set( CTEST_SOURCE_DIRECTORY . )
set( CTEST_BINARY_DIRECTORY build_ctest )
set( CTEST_CMAKE_GENERATOR "Unix Makefiles" )
set( CTEST_MEMORYCHECK_COMMAND /usr/bin/valgrind )
set( CTEST_SITE "mpictor")
set( CTEST_BUILD_NAME "Debug - Debian wheezy 64-bit")
set( CTEST_COMPILER_NAME "gcc (Debian 4.6.1-4) 4.6.1" )
set( CTEST_INITIAL_CACHE "
SITE:STRING=${CTEST_SITE}
BUILDNAME:STRING=${CTEST_BUILD_NAME}
ENABLE_TESTING:BOOL=ON
CMAKE_BUILD_TYPE:STRING=Debug
")


######################################################
##### To disable a set of tests, comment out the
##### ctest_submit line immediately following the set
##### set you wish to disable. If other tests
##### depend on those tests, they will be executed
##### but not reported.
######################################################


ctest_start(Experimental)
ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})

ctest_configure( BUILD "${CTEST_BINARY_DIRECTORY}" APPEND OPTIONS -DENABLE_TESTING=ON )
ctest_submit( PARTS Configure )
ctest_build( BUILD "${CTEST_BINARY_DIRECTORY}" APPEND )
ctest_submit( PARTS Build )
# ctest_memcheck( BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res PARALLEL_LEVEL ${PROCESSOR_COUNT} )
ctest_test( BUILD "${CTEST_BINARY_DIRECTORY}" APPEND PARALLEL_LEVEL ${PROCESSOR_COUNT} INCLUDE_LABEL "schema_gen" )
ctest_submit( PARTS Test )
ctest_test( BUILD "${CTEST_BINARY_DIRECTORY}" APPEND PARALLEL_LEVEL ${PROCESSOR_COUNT} INCLUDE_LABEL "schema_build" )
ctest_submit( PARTS Test )
ctest_test( BUILD "${CTEST_BINARY_DIRECTORY}" APPEND PARALLEL_LEVEL ${PROCESSOR_COUNT} INCLUDE_LABEL "schema_rw" )
ctest_submit( PARTS Test )

# ctest_coverage( )
