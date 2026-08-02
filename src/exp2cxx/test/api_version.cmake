cmake_minimum_required(VERSION 3.12)

set(v1_default "${CMAKE_CURRENT_BINARY_DIR}/api_v1_default")
set(v1_explicit "${CMAKE_CURRENT_BINARY_DIR}/api_v1_explicit")
set(v2 "${CMAKE_CURRENT_BINARY_DIR}/api_v2")
set(late "${CMAKE_CURRENT_BINARY_DIR}/api_v2_late")
set(late_compat "${CMAKE_CURRENT_BINARY_DIR}/api_v2_late_compat")
file(REMOVE_RECURSE "${v1_default}" "${v1_explicit}" "${v2}" "${late}"
  "${late_compat}")
file(MAKE_DIRECTORY "${v1_default}" "${v1_explicit}" "${v2}" "${late}"
  "${late_compat}")

execute_process(COMMAND "${EXE}" "${INFILE}"
  WORKING_DIRECTORY "${v1_default}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "default API v1 generation failed")
endif()

execute_process(COMMAND "${EXE}" --api-version=1 "${INFILE}"
  WORKING_DIRECTORY "${v1_explicit}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "explicit API v1 generation failed")
endif()

file(GLOB_RECURSE v1_files RELATIVE "${v1_default}" "${v1_default}/*")
foreach(relative ${v1_files})
  if(NOT IS_DIRECTORY "${v1_default}/${relative}")
    if(NOT EXISTS "${v1_explicit}/${relative}")
      message(FATAL_ERROR "explicit API v1 omitted ${relative}")
    endif()
    file(READ "${v1_default}/${relative}" default_text)
    file(READ "${v1_explicit}/${relative}" explicit_text)
    if(NOT default_text STREQUAL explicit_text)
      message(FATAL_ERROR "default and explicit API v1 differ in ${relative}")
    endif()
  endif()
endforeach()

execute_process(COMMAND "${EXE}" --api-version 2 "${INFILE}"
  WORKING_DIRECTORY "${v2}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "API v2 generation failed")
endif()

file(READ "${v2}/type/SdaiPermanent_attachment.h" v2_header)
file(READ "${v2}/type/SdaiPermanent_attachment.cc" v2_source)
if(NOT v2_header MATCHES "SELECT TYPE SdaiPermanent_attachment \\(API v2\\)")
  message(FATAL_ERROR "API v2 marker missing from generated SELECT")
endif()
if(NOT v2_header MATCHES "using SDAI_Select::operator=")
  message(FATAL_ERROR "API v2 SELECT does not expose descriptor-driven assignment")
endif()
if(v2_source MATCHES "STEPread_content|CurrentUnderlyingType")
  message(FATAL_ERROR "API v2 SELECT still contains repeated runtime dispatch")
endif()

file(READ "${v2}/SdaiTEST_SELECT_DATA_TYPE.h" v2_schema_header)
file(READ "${v2}/entity/SdaiGlue.h" v2_entity_header)
file(READ "${v2}/entity/SdaiGlue.cc" v2_entity_source)
file(READ "${v2}/SdaiTEST_SELECT_DATA_TYPE_unity_entities_0001.cc" v2_entity_chunk)
file(READ "${v2}/SdaiAll.cc" v2_all)
if(v2_schema_header MATCHES "#include \"entity/")
  message(FATAL_ERROR "API v2 schema header still includes every entity definition")
endif()
if(NOT v2_entity_header MATCHES "#include \"schema.h\"")
  message(FATAL_ERROR "API v2 entity header is not independently includable")
endif()
if(v2_entity_header MATCHES "inline SdaiGlue \\* create_SdaiGlue")
  message(FATAL_ERROR "API v2 entity creator body still expands in headers")
endif()
if(NOT v2_entity_source MATCHES "SdaiGlue \\* create_SdaiGlue\\(\\)")
  message(FATAL_ERROR "API v2 entity creator was not moved to its source file")
endif()
if(v2_entity_chunk MATCHES "#include \"SdaiTEST_SELECT_DATA_TYPE_unity_entities.h\"")
  message(FATAL_ERROR "API v2 unity chunk still loads the all-entity header")
endif()
if(NOT v2_all MATCHES "InitializeSchemas" OR
   NOT v2_all MATCHES "InitializeEntityDescriptors" OR
   NOT v2_all MATCHES "InitializeTypeDescriptors")
  message(FATAL_ERROR "API v2 schema, entity, and type descriptors are not table-driven")
endif()
if(NOT v2_entity_source MATCHES "AttributeInitRecord" OR
   NOT v2_entity_source MATCHES "InitializeEntityMetadata")
  message(FATAL_ERROR "API v2 entity metadata is not table-driven")
endif()

file(GLOB v1_types "${v1_default}/type/*")
file(GLOB v2_types "${v2}/type/*")
set(v1_size 0)
set(v2_size 0)
foreach(path ${v1_types})
  file(SIZE "${path}" size)
  math(EXPR v1_size "${v1_size} + ${size}")
endforeach()
foreach(path ${v2_types})
  file(SIZE "${path}" size)
  math(EXPR v2_size "${v2_size} + ${size}")
endforeach()
math(EXPR doubled_v2_size "${v2_size} * 2")
if(NOT doubled_v2_size LESS v1_size)
  message(FATAL_ERROR "API v2 SELECT output was not reduced by at least 50%")
endif()

execute_process(COMMAND "${EXE}" --late-bound "${INFILE}"
  WORKING_DIRECTORY "${late}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "late-bound API v2 generation failed")
endif()

file(READ "${late}/Sdaiclasses.h" late_classes)
file(READ "${late}/SdaiTEST_SELECT_DATA_TYPE.h" late_schema_header)
file(READ "${late}/SdaiTEST_SELECT_DATA_TYPE.cc" late_schema_source)
file(READ "${late}/SdaiTEST_SELECT_DATA_TYPE_unity_entities_0001.cc"
  late_entity_source)
file(READ "${late}/SdaiAll.cc" late_all)
file(READ "${late}/compstructs.cc" late_complex)
if(late_classes MATCHES "typedef SDAI_Application_instance SdaiGlue")
  message(FATAL_ERROR "late-bound output retained compatibility aliases by default")
endif()
if(EXISTS "${late}/entity/SdaiGlue.h" OR EXISTS "${late}/entity/SdaiGlue.cc")
  message(FATAL_ERROR "late-bound output still emits entity leaf files")
endif()
if(late_entity_source MATCHES "SdaiGlue::")
  message(FATAL_ERROR "late-bound output still defines early-bound entity methods")
endif()
if(NOT late_entity_source MATCHES "AttributeInitRecord")
  message(FATAL_ERROR "late-bound entity metadata is not table-driven")
endif()
if(NOT late_schema_header MATCHES "enum class EntityId" OR
   NOT late_schema_header MATCHES "enum class TypeId" OR
   NOT late_schema_header MATCHES "enum class AttributeId" OR
   NOT late_schema_header MATCHES "SchemaModule schemaModule")
  message(FATAL_ERROR "late-bound schema module API is missing")
endif()
if(NOT late_schema_source MATCHES "SchemaModuleImage moduleImage" OR
   NOT late_schema_source MATCHES "InitializeFromImage" OR
   late_schema_source MATCHES "SchemaModuleSlot")
  message(FATAL_ERROR "late-bound schema module image is not packed")
endif()
if(NOT late_all MATCHES "InitializeSchemas" OR
   NOT late_all MATCHES "InitializeSchemaModuleImages" OR
   NOT late_all MATCHES "InitializeEntityDescriptors" OR
   NOT late_all MATCHES "InitializeTypeDescriptors" OR
   NOT late_all MATCHES "e_glue, \"Glue\".*LFalse, LFalse, 0")
  message(FATAL_ERROR "late-bound descriptor records are missing or have an early-bound creator")
endif()
if(NOT late_all MATCHES "InitializeGlobalRules" OR
   NOT late_all MATCHES "InitializeFunctions" OR
   NOT late_all MATCHES "InitializeProcedures")
  message(FATAL_ERROR "late-bound schema text metadata is not table-driven")
endif()
if(NOT late_complex MATCHES "ComplexNodeInitRecord" OR
   NOT late_complex MATCHES "InitializeComplexSupport")
  message(FATAL_ERROR "late-bound complex metadata is not table-driven")
endif()

execute_process(COMMAND "${EXE}" --late-bound --compat-names "${INFILE}"
  WORKING_DIRECTORY "${late_compat}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "late-bound compatibility generation failed")
endif()
file(READ "${late_compat}/Sdaiclasses.h" late_compat_classes)
if(NOT late_compat_classes MATCHES
   "typedef SDAI_Application_instance SdaiGlue")
  message(FATAL_ERROR "--compat-names did not restore entity aliases")
endif()

execute_process(COMMAND "${EXE}" --late-bound --api-version 1 "${INFILE}"
  WORKING_DIRECTORY "${late}" RESULT_VARIABLE result
  OUTPUT_QUIET ERROR_QUIET)
if(result EQUAL 0)
  message(FATAL_ERROR "late-bound output accepted incompatible API version 1")
endif()

execute_process(COMMAND "${EXE}" --api-version=3 "${INFILE}"
  WORKING_DIRECTORY "${v2}" RESULT_VARIABLE result
  OUTPUT_QUIET ERROR_QUIET)
if(result EQUAL 0)
  message(FATAL_ERROR "invalid API version was accepted")
endif()
