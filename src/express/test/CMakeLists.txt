include_directories(..)

set(EXPRESS_CORE_OBJ
    $<TARGET_OBJECTS:objlib_alloc_c>
    $<TARGET_OBJECTS:objlib_memory_c>
    $<TARGET_OBJECTS:objlib_factory_c>
    $<TARGET_OBJECTS:objlib_error_c>
    $<TARGET_OBJECTS:objlib_hash_c>
    $<TARGET_OBJECTS:objlib_linklist_c>
# extra
    $<TARGET_OBJECTS:objlib_dict_c>
    $<TARGET_OBJECTS:objlib_object_c>
    $<TARGET_OBJECTS:objlib_info_c>
)

add_executable(test_expr test_expr.c $<TARGET_OBJECTS:objlib_expr_c> ${EXPRESS_CORE_OBJ})
add_test(NAME exp_resolve_select_enum_member COMMAND test_expr resolve_select_enum_member)
add_test(NAME exp_resolve_entity_attribute COMMAND test_expr resolve_entity_attribute)

add_test(NAME build_check_express
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} --build .
  --target check-express
  --config $<CONFIGURATION>
  )

add_test(NAME test_plib_parse_err
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND $<TARGET_FILE:check-express> "${CMAKE_CURRENT_SOURCE_DIR}/plib_parse_err.exp"
  )

set_tests_properties( test_plib_parse_err PROPERTIES DEPENDS "build_check_express;$<TARGET_NAME:check-express>" )
set_tests_properties( test_plib_parse_err build_check_express PROPERTIES LABELS parser )

sc_addexec(print_schemas "../fedex.c;print_schemas.c" "express;base")
sc_addexec(print_attrs "../fedex.c;print_attrs.c" "express;base")

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

