set(LAZY_SRCS
  lazyDataSectionReader.cc
  lazyFileReader.cc
  lazyInstMgr.cc
  lazySupport.cc
  p21HeaderSectionReader.cc
  sectionReader.cc
  lazyP21DataSectionReader.cc
  judy.c
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include/stepcode
  )



if(BUILD_SHARED_LIBS)
  set(_libdeps stepcore stepdai steputils stepeditor)
  SC_ADDLIB(steplazyfile SHARED SOURCES ${LAZY_SRCS} LINK_LIBRARIES ${_libdeps})
  if(WIN32)
    target_compile_definitions(steplazyfile PRIVATE SC_LAZYFILE_DLL_EXPORTS)
  endif()
  SC_ADDEXEC(lazy_test SOURCES "lazy_test.cc;sc_benchmark.cc" LINK_LIBRARIES steplazyfile stepeditor ${_libdeps} NO_INSTALL)
  target_compile_definitions(lazy_test PRIVATE NO_REGISTRY)
endif()

if(BUILD_STATIC_LIBS)
  set(_libdeps stepcore-static stepdai-static steputils-static stepeditor-static)
  SC_ADDLIB(steplazyfile-static STATIC SOURCES ${LAZY_SRCS} LINK_LIBRARIES ${_libdeps})
  if(NOT BUILD_SHARED_LIBS)
    SC_ADDEXEC(lazy_test SOURCES "lazy_test.cc;sc_benchmark.cc" LINK_LIBRARIES steplazyfile-static stepeditor-static ${_libdeps} NO_INSTALL)
    target_compile_definitions(lazy_test PRIVATE NO_REGISTRY)
  endif()
endif()


# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

if(SC_ENABLE_TESTING)
  if(BUILD_SHARED_LIBS)
    set(_lazy_index_libs steplazyfile stepeditor stepcore stepdai steputils)
  else()
    set(_lazy_index_libs steplazyfile-static stepeditor-static
      stepcore-static stepdai-static steputils-static)
  endif()
  SC_ADDEXEC(lazy_index_test SOURCES lazy_index_test.cc
    LINK_LIBRARIES ${_lazy_index_libs} NO_INSTALL)
  add_test(NAME lazy_index
    COMMAND lazy_index_test ${CMAKE_CURRENT_SOURCE_DIR}/test/lazy_index.stp)
endif()
