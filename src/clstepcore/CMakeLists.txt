
set(LIBSTEPCORE_SRCS
  sdaiApplication_instance.cc
  STEPcomplex.cc
  STEPaggregate.cc
  STEPattribute.cc
  STEPattribute.inline.cc
  STEPattributeList.cc
  STEPinvAttrList.cc
  STEPundefined.cc
  sdai.cc
  sdaiSelect.cc
  SingleLinkList.cc
  Registry.cc
  ExpDict.cc
  ExpDict.inline.cc
  read_func.cc
  collect.cc
  complexlist.cc
  entlist.cc
  multlist.cc
  orlist.cc
  entnode.cc
  non-ors.cc
  match-ors.cc
  trynext.cc
  print.cc
  instmgr.cc
  mgrnode.cc
  mgrnodearray.cc
  mgrnodelist.cc
  dispnode.cc
  dispnodelist.cc
  needFunc.cc
  attrDescriptorList.cc
  entityDescriptorList.cc
  inverseAttributeList.cc
  typeDescriptorList.cc
  attrDescriptor.cc
  derivedAttribute.cc
  entityDescriptor.cc
  enumTypeDescriptor.cc
  inverseAttribute.cc
  typeDescriptor.cc
  schRename.cc
  dictSchema.cc
  whereRule.cc
  uniquenessRule.cc
  interfacedItem.cc
  explicitItemId.cc
  implicitItemId.cc
  interfaceSpec.cc
  typeOrRuleVar.cc
  globalRule.cc
)

set(SC_CLSTEPCORE_HDRS
  baseType.h
  complexSupport.h
  dictdefs.h
  ExpDict.h
  read_func.h
  Registry.h
  sdaiApplication_instance.h
  sdai.h
  sdaiSelect.h
  SingleLinkList.h
  STEPaggregate.h
  STEPattribute.h
  STEPattributeList.h
  STEPinvAttrList.h
  STEPcomplex.h
  STEPundefined.h
  instmgr.h
  mgrnode.h
  mgrnodearray.h
  mgrnodelist.h
  dispnode.h
  dispnodelist.h
  needFunc.h
  SubSuperIterators.h
  attrDescriptorList.h
  entityDescriptorList.h
  inverseAttributeList.h
  typeDescriptorList.h
  attrDescriptor.h
  derivedAttribute.h
  entityDescriptor.h
  enumTypeDescriptor.h
  inverseAttribute.h
  typeDescriptor.h
  schRename.h
  dictSchema.h
  whereRule.h
  uniquenessRule.h
  dictionaryInstance.h
  typeOrRuleVar.h
  aggrCreatorTDs.h
  explicitItemId.h
  globalRule.h
  implicitItemId.h
  interfaceSpec.h
  interfacedItem.h
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${SC_SOURCE_DIR}/src/base
  ${SC_SOURCE_DIR}/src/cldai
  ${SC_SOURCE_DIR}/src/cleditor
  ${SC_SOURCE_DIR}/src/clutils
  )

SC_ADDLIB(stepcore "${LIBSTEPCORE_SRCS}" "steputils;stepdai;base")

install(FILES ${SC_CLSTEPCORE_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/stepcode/clstepcore)

if(SC_ENABLE_TESTING)
  add_subdirectory(test)
endif(SC_ENABLE_TESTING)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

