
set( clLazyFile_SRCS
  lazyDataSectionReader.cc
  lazyFileReader.cc
  lazyInstMgr.cc
  p21HeaderSectionReader.cc
  sectionReader.cc
  lazyP21DataSectionReader.cc
  judy.c
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include/stepcode
  ${SC_SOURCE_DIR}/src/cleditor
  ${SC_SOURCE_DIR}/src/cldai
  ${SC_SOURCE_DIR}/src/clstepcore
  ${SC_SOURCE_DIR}/src/clutils
  )

set(_libdeps stepcore stepdai steputils stepeditor)

if(BUILD_SHARED_LIBS)
  SC_ADDLIB(steplazyfile SHARED SOURCES ${clLazyFile_SRCS} LINK_LIBRARIES ${_libdeps})
  if(WIN32)
    target_compile_definitions(steplazyfile PRIVATE SC_LAZYFILE_DLL_EXPORTS)
  endif()
endif()

if(BUILD_STATIC_LIBS)
  set(_libdeps stepcore-static stepdai-static steputils-static stepeditor-static)
  SC_ADDLIB(steplazyfile-static STATIC SOURCES ${clLazyFile_SRCS} LINK_LIBRARIES ${_libdeps})
endif()

SC_ADDEXEC(lazy_test SOURCES "lazy_test.cc;sc_benchmark.cc" LINK_LIBRARIES steplazyfile stepeditor NO_INSTALL)
target_compile_definitions(lazy_test PRIVATE NO_REGISTRY)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

