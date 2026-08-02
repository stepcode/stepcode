cmake_minimum_required(VERSION 3.12)

set(output "${CMAKE_CURRENT_BINARY_DIR}/stable_output")
file(REMOVE_RECURSE "${output}")
file(MAKE_DIRECTORY "${output}")

execute_process(COMMAND "${EXE}" --api-version 2 "${INFILE}"
  WORKING_DIRECTORY "${output}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "initial stable-output generation failed")
endif()
if(NOT EXISTS "${output}/.exp2cxx-manifest" OR
   NOT EXISTS "${output}/entity/SdaiGlue.cc" OR
   NOT EXISTS "${output}/type/SdaiPermanent_attachment.cc")
  message(FATAL_ERROR "initial generated file manifest is incomplete")
endif()

file(WRITE "${output}/unrelated.txt" "preserve me\n")
file(TIMESTAMP "${output}/SdaiAll.cc" source_timestamp UTC)
file(TIMESTAMP "${output}/.exp2cxx-manifest" manifest_timestamp UTC)
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
execute_process(COMMAND "${EXE}" --api-version 2 "${INFILE}"
  WORKING_DIRECTORY "${output}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "no-op stable-output generation failed")
endif()
file(TIMESTAMP "${output}/SdaiAll.cc" repeated_source_timestamp UTC)
file(TIMESTAMP "${output}/.exp2cxx-manifest"
  repeated_manifest_timestamp UTC)
if(NOT source_timestamp STREQUAL repeated_source_timestamp OR
   NOT manifest_timestamp STREQUAL repeated_manifest_timestamp)
  message(FATAL_ERROR "no-op generation changed output timestamps")
endif()
file(GLOB_RECURSE temporary_files "${output}/*.exp2cxx-tmp-*")
if(temporary_files)
  message(FATAL_ERROR "generation left temporary files behind")
endif()

execute_process(COMMAND "${EXE}" --late-bound "${INFILE}"
  WORKING_DIRECTORY "${output}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "late-bound stale cleanup generation failed")
endif()
if(EXISTS "${output}/entity/SdaiGlue.cc" OR EXISTS "${output}/entity" OR
   EXISTS "${output}/type/SdaiPermanent_attachment.cc" OR
   EXISTS "${output}/type" OR
   EXISTS "${output}/SdaiTEST_SELECT_DATA_TYPE_unity_entities_0001.cc")
  message(FATAL_ERROR "manifest cleanup retained stale generated output")
endif()
if(NOT EXISTS "${output}/unrelated.txt")
  message(FATAL_ERROR "manifest cleanup removed an unrelated file")
endif()

file(READ "${output}/SdaiTEST_SELECT_DATA_TYPENames.h" names_header)
if(names_header MATCHES "This line printed at")
  message(FATAL_ERROR "generated output retained a generator source path")
endif()

set(bounds_output "${CMAKE_CURRENT_BINARY_DIR}/stable_bounds_output")
file(REMOVE_RECURSE "${bounds_output}")
file(MAKE_DIRECTORY "${bounds_output}")
execute_process(COMMAND "${EXE}" --late-bound "${BOUND_INFILE}"
  WORKING_DIRECTORY "${bounds_output}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "initial expression-bound generation failed")
endif()
file(READ "${bounds_output}/SdaiAll.cc" bounds_source)
if(NOT bounds_source MATCHES "bound_funcall")
  message(FATAL_ERROR "aggregate expression bound was emitted as a constant")
endif()
file(TIMESTAMP "${bounds_output}/SdaiAll.cc" bounds_timestamp UTC)
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
execute_process(COMMAND "${EXE}" --late-bound "${BOUND_INFILE}"
  WORKING_DIRECTORY "${bounds_output}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "repeated expression-bound generation failed")
endif()
file(TIMESTAMP "${bounds_output}/SdaiAll.cc"
  repeated_bounds_timestamp UTC)
if(NOT bounds_timestamp STREQUAL repeated_bounds_timestamp)
  message(FATAL_ERROR "expression bounds made generated output unstable")
endif()
