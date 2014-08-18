
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
  sc_mutex.h
  sc_thread.h
  )

include_directories(
  ${SC_SOURCE_DIR}/src/base
  ${SC_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

SC_ADDLIB(steputils "${LIBSTEPUTILS_SRCS};${SC_CLUTILS_HDRS}" "base")
if(HAVE_STD_THREAD)
  list(APPEND CMAKE_CXX_FLAGS "-pthread -std=c++0x")
  set_target_properties(steputils PROPERTIES COMPILE_FLAGS "-pthread -std=c++0x -DHAVE_STD_THREAD" )
endif(HAVE_STD_THREAD)

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

