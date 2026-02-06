# -----  GENERATED FILE  -----
# -----   Do not edit!   -----

# schema name: test_select_data_type
# (short name: sdai_test_treat_real)
# 5 entities, 3 types

# targets, logic, etc are within a set of macros shared by all schemas
include(${SC_CMAKE_DIR}/SC_CXX_schema_macros.cmake)
set(sdai_test_treat_real_file_count 26)

PROJECT(sdai_test_treat_real)
# list headers so they can be installed - entity, type, misc
set(sdai_test_treat_real_entity_hdrs
     entity/SdaiWeld.h                                                           entity/SdaiGlue.h                                                           
     entity/SdaiWall_mounting.h                                                  entity/SdaiScrew.h                                                          
     entity/SdaiNail.h                                                              )

set(sdai_test_treat_real_type_hdrs
     type/SdaiPermanent_attachment.h                                             type/SdaiTemporary_attachment.h                                             
     type/SdaiAttachment_method.h                                                   )

set(sdai_test_treat_real_misc_hdrs
     Sdaiclasses.h   schema.h
     SdaiTEST_SELECT_DATA_TYPENames.h
     SdaiTEST_SELECT_DATA_TYPE.h
   )

# install all headers
set(all_headers ${sdai_test_treat_real_entity_hdrs} ${sdai_test_treat_real_type_hdrs} ${sdai_test_treat_real_misc_hdrs})
foreach( header_file ${all_headers} )
  set(curr_dir)
  get_filename_component(curr_dir ${header_file} PATH)
  get_filename_component(curr_name ${header_file} NAME)
  if (curr_dir)
    install( FILES ${header_file} DESTINATION "include/schemas/sdai_test_treat_real/${curr_dir}" )
  else (curr_dir)
    install( FILES ${header_file} DESTINATION "include/schemas/sdai_test_treat_real" )
  endif (curr_dir)
endforeach()
# implementation files - 3 lists

# unity build: #include small .cc files to reduce the number
# of translation units that must be compiled
if(SC_UNITY_BUILD)
  # turns off include statements within type and entity .cc's - the unity T.U.'s include a unity header
  add_definitions( -DSC_SDAI_UNITY_BUILD)
  set(sdai_test_treat_real_entity_impls SdaiTEST_SELECT_DATA_TYPE_unity_entities.cc)
  set(sdai_test_treat_real_type_impls SdaiTEST_SELECT_DATA_TYPE_unity_types.cc)
else(SC_UNITY_BUILD)
  set(sdai_test_treat_real_entity_impls
     entity/SdaiWeld.cc                                                          entity/SdaiGlue.cc                                                          
     entity/SdaiWall_mounting.cc                                                 entity/SdaiScrew.cc                                                         
     entity/SdaiNail.cc                                                             )

  set(sdai_test_treat_real_type_impls
     type/SdaiPermanent_attachment.cc                                            type/SdaiTemporary_attachment.cc                                            
     type/SdaiAttachment_method.cc                                                  )
endif(SC_UNITY_BUILD)

set( sdai_test_treat_real_misc_impls
     SdaiAll.cc    compstructs.cc    schema.cc
     SdaiTEST_SELECT_DATA_TYPE.cc
     SdaiTEST_SELECT_DATA_TYPE.init.cc   )

set(schema_target_files ${sdai_test_treat_real_entity_impls} ${sdai_test_treat_real_type_impls} ${sdai_test_treat_real_misc_impls})
SCHEMA_TARGETS("test/unitary_schemas/test_treat_real.exp" "test_select_data_type"
                 "${schema_target_files}")
