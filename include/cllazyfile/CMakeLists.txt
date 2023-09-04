set(LAZY_HDRS
  headerSectionReader.h
  lazyFileReader.h
  lazyP21DataSectionReader.h
  p21HeaderSectionReader.h
  lazyDataSectionReader.h
  lazyInstMgr.h
  lazyTypes.h
  sectionReader.h
  instMgrHelper.h
  judy.h
  judyL2Array.h
  judyLArray.h
  judyS2Array.h
  judySArray.h
  )

install(FILES ${LAZY_HDRS}
  DESTINATION ${INCLUDE_DIR}/stepcode/cllazyfile)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

