set(UTILS_HDRS
  dirobj.h
  errordesc.h
  gennodearray.h
  gennode.h
  gennodelist.h
  sc_hash.h
  Str.h
  )

install(FILES ${UTILS_HDRS}
  DESTINATION ${INCLUDE_DIR}/stepcode/clutils)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

