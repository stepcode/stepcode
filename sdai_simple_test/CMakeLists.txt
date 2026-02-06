# -----  GENERATED FILE  -----
# -----   Do not edit!   -----

# schema name: simple_test
# (short name: sdai_simple_test)
# 1 entities, 0 types

# targets, logic, etc are within a set of macros shared by all schemas
include(${SC_CMAKE_DIR}/SC_CXX_schema_macros.cmake)
set(sdai_simple_test_file_count 12)

PROJECT(sdai_simple_test)
# list headers so they can be installed - entity, type, misc
set(sdai_simple_test_entity_hdrs
     entity/SdaiItem_a.h                                                            )

set(sdai_simple_test_type_hdrs
        )

set(sdai_simple_test_misc_hdrs
     Sdaiclasses.h   schema.h
     SdaiSIMPLE_TESTNames.h
     SdaiSIMPLE_TEST.h
   )

# install all headers
set(all_headers ${sdai_simple_test_entity_hdrs} ${sdai_simple_test_type_hdrs} ${sdai_simple_test_misc_hdrs})
foreach( header_file ${all_headers} )
  set(curr_dir)
  get_filename_component(curr_dir ${header_file} PATH)
  get_filename_component(curr_name ${header_file} NAME)
  if (curr_dir)
    install( FILES ${header_file} DESTINATION "include/schemas/sdai_simple_test/${curr_dir}" )
  else (curr_dir)
    install( FILES ${header_file} DESTINATION "include/schemas/sdai_simple_test" )
  endif (curr_dir)
endforeach()
# implementation files - 3 lists

# unity build: #include small .cc files to reduce the number
# of translation units that must be compiled
if(SC_UNITY_BUILD)
  # turns off include statements within type and entity .cc's - the unity T.U.'s include a unity header
  add_definitions( -DSC_SDAI_UNITY_BUILD)
  set(sdai_simple_test_entity_impls SdaiSIMPLE_TEST_unity_entities.cc)
  set(sdai_simple_test_type_impls SdaiSIMPLE_TEST_unity_types.cc)
else(SC_UNITY_BUILD)
  set(sdai_simple_test_entity_impls
     entity/SdaiItem_a.cc                                                           )

  set(sdai_simple_test_type_impls
        )
endif(SC_UNITY_BUILD)

set( sdai_simple_test_misc_impls
     SdaiAll.cc    compstructs.cc    schema.cc
     SdaiSIMPLE_TEST.cc
     SdaiSIMPLE_TEST.init.cc   )

set(schema_target_files ${sdai_simple_test_entity_impls} ${sdai_simple_test_type_impls} ${sdai_simple_test_misc_impls})
SCHEMA_TARGETS("test/unitary_schemas/simple_test.exp" "simple_test"
                 "${schema_target_files}")
