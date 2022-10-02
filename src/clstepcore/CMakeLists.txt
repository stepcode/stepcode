set(CORE_SRCS
  aggrTypeDescriptor.cc
  attrDescriptor.cc
  attrDescriptorList.cc
  collect.cc
  complexlist.cc
  create_Aggr.cc
  derivedAttribute.cc
  dictSchema.cc
  dispnode.cc
  dispnodelist.cc
  entityDescriptor.cc
  entityDescriptorList.cc
  entlist.cc
  entnode.cc
  enumTypeDescriptor.cc
  explicitItemId.cc
  globalRule.cc
  implicitItemId.cc
  instmgr.cc
  interfaceSpec.cc
  interfacedItem.cc
  inverseAttribute.cc
  inverseAttributeList.cc
  match-ors.cc
  mgrnode.cc
  mgrnodearray.cc
  mgrnodelist.cc
  multlist.cc
  needFunc.cc
  non-ors.cc
  orlist.cc
  print.cc
  read_func.cc
  Registry.cc
  schRename.cc
  sdai.cc
  sdaiApplication_instance.cc
  sdaiSelect.cc
  selectTypeDescriptor.cc
  SingleLinkList.cc
  STEPaggregate.cc
  STEPaggrBinary.cc
  STEPaggrEntity.cc
  STEPaggrEnum.cc
  STEPaggrGeneric.cc
  STEPaggrInt.cc
  STEPaggrReal.cc
  STEPaggrSelect.cc
  STEPaggrString.cc
  STEPattribute.cc
  STEPattributeList.cc
  STEPcomplex.cc
  STEPinvAttrList.cc
  STEPundefined.cc
  trynext.cc
  typeDescriptor.cc
  typeDescriptorList.cc
  typeOrRuleVar.cc
  uniquenessRule.cc
  whereRule.cc
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include/stepcode
  )

set(_libdeps steputils stepdai)

if(BUILD_SHARED_LIBS)
  SC_ADDLIB(stepcore SHARED SOURCES ${CORE_SRCS} LINK_LIBRARIES ${_libdeps})
  if(WIN32)
    target_compile_definitions(stepcore PRIVATE SC_CORE_DLL_EXPORTS)
  endif()
endif()

if(BUILD_STATIC_LIBS)
  SC_ADDLIB(stepcore-static STATIC SOURCES ${CORE_SRCS} LINK_LIBRARIES $<JOIN:${_libdeps},-static>-static)
endif()

if(SC_ENABLE_TESTING)
  add_subdirectory(test)
endif(SC_ENABLE_TESTING)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

