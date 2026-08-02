set(FEDEX_COMMON_SRCS
  classes_misc.c
  ../express/fedex.c
  )

set(exp2cxx_SOURCES
  ${FEDEX_COMMON_SRCS}
  trace_fprintf.c
  fedex_main.c
  classes_wrapper.cc
  schema_image.cc
  classes.c
  classes_attribute.c
  classes_entity.c
  classes_type.c
  class_strings.c
  selects.c
  multpass.c
  rules.c
  collect.cc
  complexlist.cc
  entlist.cc
  multlist.cc
  orlist.cc
  entnode.cc
  expressbuild.cc
  non-ors.cc
  match-ors.cc
  trynext.cc
  write.cc
  print.cc
  genCxxFilenames.c
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${SC_SOURCE_DIR}/include
  ${SC_SOURCE_DIR}/include/exppp
  ${SC_SOURCE_DIR}/include/express
  )

if(BUILD_SHARED_LIBS)
  SC_ADDEXEC(exp2cxx SOURCES ${exp2cxx_SOURCES} LINK_LIBRARIES libexppp express)
else()
  SC_ADDEXEC(exp2cxx SOURCES ${exp2cxx_SOURCES} LINK_LIBRARIES libexppp-static express-static)
endif()

if(SC_ENABLE_TESTING)
  add_subdirectory(test)
endif(SC_ENABLE_TESTING)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8
