set(CORE_HDRS
  aggrTypeDescriptor.h
  attrDescriptor.h
  attrDescriptorList.h
  baseType.h
  complexSupport.h
  create_Aggr.h
  derivedAttribute.h
  dictSchema.h
  dictdefs.h
  dictionaryInstance.h
  dispnode.h
  dispnodelist.h
# entityDescriptor.h
# entityDescriptorList.h
# enumTypeDescriptor.h
# ExpDict.h
# explicitItemId.h
# globalRule.h
# implicitItemId.h
# instmgr.h
# interfaceSpec.h
# interfacedItem.h
# inverseAttribute.h
# inverseAttributeList.h
# mgrnode.h
# mgrnodearray.h
# mgrnodelist.h
# needFunc.h
# read_func.h
# realTypeDescriptor.h
# Registry.h
# schRename.h
# sdai.h
# sdaiApplication_instance.h
# sdaiSelect.h
# selectTypeDescriptor.h
# SingleLinkList.h
# STEPaggregate.h
# STEPaggrBinary.h
# STEPaggrEntity.h
# STEPaggrEnum.h
# STEPaggrGeneric.h
# STEPaggrInt.h
# STEPaggrReal.h
# STEPaggrSelect.h
# STEPaggrString.h
# STEPattribute.h
# STEPattributeList.h
# STEPcomplex.h
# STEPinvAttrList.h
# STEPundefined.h
# stringTypeDescriptor.h
# SubSuperIterators.h
# typeDescriptor.h
# typeDescriptorList.h
# typeOrRuleVar.h
# uniquenessRule.h
# whereRule.h
)

install(FILES ${CORE_HDRS}
  DESTINATION ${INCLUDE_DIR}/stepcode/core)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

