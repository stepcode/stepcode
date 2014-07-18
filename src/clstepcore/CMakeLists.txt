
set(LIBSTEPCORE_SRCS
  sdaiApplication_instance.cc
  STEPcomplex.cc
  STEPattribute.cc
  STEPattribute.inline.cc
  sdai.cc
  sdaiSelect.cc
  STEPaggregate.cc
  STEPundefined.cc
  STEPattributeList.cc
  SingleLinkList.cc
  SingleLinkList.inline.cc
  Registry.inline.cc
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
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${SC_SOURCE_DIR}/src/base
  ${SC_SOURCE_DIR}/src/cldai
  ${SC_SOURCE_DIR}/src/cleditor
  ${SC_SOURCE_DIR}/src/clutils
  )

SC_ADDLIB(stepcore "${LIBSTEPCORE_SRCS}" "steputils;stepdai;base")
if(HAVE_STD_THREAD)
  list(APPEND CMAKE_CXX_FLAGS "-pthread -std=c++0x")
  set_target_properties(stepcore PROPERTIES COMPILE_FLAGS "-pthread -std=c++0x -DHAVE_STD_THREAD" )

  SC_ADDEXEC(InstMgr_thread_safety_test "InstMgr_thread_safety_test.cc" "stepcore" )
  set_target_properties(InstMgr_thread_safety_test PROPERTIES COMPILE_FLAGS "-pthread -std=c++0x -DHAVE_STD_THREAD")
  target_link_libraries(InstMgr_thread_safety_test "pthread" )

  SC_ADDEXEC(Registry_thread_safety_test "Registry_thread_safety_test.cc" "stepcore" )
  set_target_properties(Registry_thread_safety_test PROPERTIES COMPILE_FLAGS "-pthread -std=c++0x -DHAVE_STD_THREAD")
  target_link_libraries(Registry_thread_safety_test "pthread" )

  SC_ADDEXEC(sdaiApplication_instance_thread_safety_test "sdaiApplication_instance_thread_safety_test.cc" "stepcore" )
  set_target_properties(sdaiApplication_instance_thread_safety_test PROPERTIES COMPILE_FLAGS "-pthread -std=c++0x -DHAVE_STD_THREAD")
  target_link_libraries(sdaiApplication_instance_thread_safety_test "pthread" )
endif(HAVE_STD_THREAD)

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

