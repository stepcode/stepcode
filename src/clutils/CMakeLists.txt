
set(LIBSTEPUTILS_SRCS
  Str.cc
  dirobj.cc
  gennode.cc
  gennodelist.cc
  gennodearray.cc
  sc_hash.cc
  errordesc.cc
  )

set(SC_CLUTILS_HDRS
  dirobj.h
  errordesc.h
  gennodearray.h
  gennode.h
  gennodelist.h
  sc_hash.h
  Str.h
  )

include_directories(
  ${SC_SOURCE_DIR}/src/base
  ${SC_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

SC_ADDLIB(steputils "${LIBSTEPUTILS_SRCS}" "base")

if(MINGW OR MSVC OR BORLAND)
  target_link_libraries(steputils shlwapi.lib)
endif()

install(FILES ${SC_CLUTILS_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/stepcode/clutils)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

