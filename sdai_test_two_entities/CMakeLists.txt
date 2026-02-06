# -----  GENERATED FILE  -----
# -----   Do not edit!   -----

# schema name: test_two_entities
# (short name: sdai_test_two_entities)
# 2 entities, 0 types

# targets, logic, etc are within a set of macros shared by all schemas
include(${SC_CMAKE_DIR}/SC_CXX_schema_macros.cmake)
set(sdai_test_two_entities_file_count 14)

PROJECT(sdai_test_two_entities)
# list headers so they can be installed - entity, type, misc
set(sdai_test_two_entities_entity_hdrs
     entity/SdaiScrew.h                                                          entity/SdaiNail.h                                                           
        )

set(sdai_test_two_entities_type_hdrs
        )

set(sdai_test_two_entities_misc_hdrs
     Sdaiclasses.h   schema.h
     SdaiTEST_TWO_ENTITIESNames.h
     SdaiTEST_TWO_ENTITIES.h
   )

# install all headers
set(all_headers ${sdai_test_two_entities_entity_hdrs} ${sdai_test_two_entities_type_hdrs} ${sdai_test_two_entities_misc_hdrs})
foreach( header_file ${all_headers} )
  set(curr_dir)
  get_filename_component(curr_dir ${header_file} PATH)
  get_filename_component(curr_name ${header_file} NAME)
  if (curr_dir)
    install( FILES ${header_file} DESTINATION "include/schemas/sdai_test_two_entities/${curr_dir}" )
  else (curr_dir)
    install( FILES ${header_file} DESTINATION "include/schemas/sdai_test_two_entities" )
  endif (curr_dir)
endforeach()
# implementation files - 3 lists

# unity build: #include small .cc files to reduce the number
# of translation units that must be compiled
if(SC_UNITY_BUILD)
  # turns off include statements within type and entity .cc's - the unity T.U.'s include a unity header
  add_definitions( -DSC_SDAI_UNITY_BUILD)
  set(sdai_test_two_entities_entity_impls SdaiTEST_TWO_ENTITIES_unity_entities.cc)
  set(sdai_test_two_entities_type_impls SdaiTEST_TWO_ENTITIES_unity_types.cc)
else(SC_UNITY_BUILD)
  set(sdai_test_two_entities_entity_impls
     entity/SdaiScrew.cc                                                         entity/SdaiNail.cc                                                          
        )

  set(sdai_test_two_entities_type_impls
        )
endif(SC_UNITY_BUILD)

set( sdai_test_two_entities_misc_impls
     SdaiAll.cc    compstructs.cc    schema.cc
     SdaiTEST_TWO_ENTITIES.cc
     SdaiTEST_TWO_ENTITIES.init.cc   )

set(schema_target_files ${sdai_test_two_entities_entity_impls} ${sdai_test_two_entities_type_impls} ${sdai_test_two_entities_misc_impls})
SCHEMA_TARGETS("test/unitary_schemas/test_two_entities.exp" "test_two_entities"
                 "${schema_target_files}")
