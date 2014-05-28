
set( clLazyFile_SRCS
  lazyDataSectionReader.cc
  lazyFileReader.cc
  lazyInstMgr.cc
  p21HeaderSectionReader.cc
  sectionReader.cc
  lazyP21DataSectionReader.cc
  ../base/judy/src/judy.c
  ../base/judy/src/judyLArray.h
  ../base/judy/src/judySArray.h
  )

set( SC_CLLAZYFILE_HDRS
  headerSectionReader.h
  lazyFileReader.h
  lazyP21DataSectionReader.h
  p21HeaderSectionReader.h
  lazyDataSectionReader.h
  lazyInstMgr.h
  lazyTypes.h
  sectionReader.h
  instMgrHelper.h
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${SC_SOURCE_DIR}/src/cleditor
  ${SC_SOURCE_DIR}/src/cldai
  ${SC_SOURCE_DIR}/src/clstepcore
  ${SC_SOURCE_DIR}/src/clutils
  ${SC_SOURCE_DIR}/src/base
  ${SC_SOURCE_DIR}/src/base/judy/src
  )

SC_ADDLIB(steplazyfile "${clLazyFile_SRCS};${clLazyFile_HDRS}" "stepcore;stepdai;steputils;base;")

if(HAVE_STD_THREAD)
  list(APPEND CMAKE_CXX_FLAGS "-pthread -std=c++0x")
  set_target_properties(steplazyfile PROPERTIES COMPILE_FLAGS "-pthread -std=c++0x -DHAVE_STD_THREAD" )

  #Directory frim which lazy_thread_safety_test will get its schema.h
  include_directories(
    ${CMAKE_BINARY_DIR}/schemas/sdai_cd209
  )

  SC_ADDEXEC(lazy_thread_safety_test "lazy_thread_safety_test.cc" "steplazyfile;stepeditor" )
  set_target_properties(lazy_thread_safety_test PROPERTIES COMPILE_FLAGS "-pthread -std=c++0x -DHAVE_STD_THREAD")
  target_link_libraries(lazy_thread_safety_test sdai_cd209 "pthread" )
endif(HAVE_STD_THREAD)

SC_ADDEXEC(lazy_test "lazy_test.cc" "steplazyfile;stepeditor" )
set_target_properties(lazy_test PROPERTIES COMPILE_FLAGS "-DNO_REGISTRY" )

install(FILES ${SC_CLLAZYFILE_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/stepcode/cllazyfile)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

