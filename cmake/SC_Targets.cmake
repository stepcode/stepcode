
# set compile definitions for dll exports on windows
macro(DEFINE_DLL_EXPORTS libname)
  if(MSVC OR BORLAND)
    if(${libname} MATCHES "sdai_.*")
      set(export "SC_SCHEMA_DLL_EXPORTS")
    else()
      string(REGEX REPLACE "lib" "" shortname "${libname}")
      string(REGEX REPLACE "step" "" LOWERCORE "${shortname}")
      string(TOUPPER ${LOWERCORE} UPPER_CORE)
      set(export "SC_${UPPER_CORE}_DLL_EXPORTS")
    endif()
    set_property(TARGET ${libname} APPEND PROPERTY COMPILE_DEFINITIONS "${export}")
  endif(MSVC OR BORLAND)
endmacro(DEFINE_DLL_EXPORTS libname)

# set compile definitions for dll imports on windows
macro(DEFINE_DLL_IMPORTS tgt libs)
  if(MSVC OR BORLAND)
    set(imports "")
    foreach(lib ${libs})
      string(REGEX REPLACE "lib" "" shortname "${lib}")
      string(REGEX REPLACE "step" "" LOWERCORE "${shortname}")
      string(TOUPPER ${LOWERCORE} UPPER_CORE)
      list(APPEND imports "SC_${UPPER_CORE}_DLL_IMPORTS")
    endforeach(lib ${libs})
    set_property(TARGET ${tgt} APPEND PROPERTY COMPILE_DEFINITIONS "${imports}")
  endif(MSVC OR BORLAND)
endmacro(DEFINE_DLL_IMPORTS tgt libs)

#SC_ADDEXEC(execname "source files" "linked libs" ["TESTABLE"] ["NO_INSTALL"])
macro(SC_ADDEXEC execname srcslist libslist)

  string(TOUPPER "${execname}" EXECNAME_UPPER)
  if(${ARGC} GREATER 3)
    CMAKE_PARSE_ARGUMENTS(${EXECNAME_UPPER} "NO_INSTALL;TESTABLE" "" "" ${ARGN})
  endif(${ARGC} GREATER 3)

  add_executable(${execname} ${srcslist})
  target_link_libraries(${execname} ${libslist})
  DEFINE_DLL_IMPORTS(${execname} "${libslist}")  #add import definitions for all libs that the executable is linked to
  if(NOT ${EXECNAME_UPPER}_NO_INSTALL AND NOT ${EXECNAME_UPPER}_TESTABLE)
    install(TARGETS ${execname}
      RUNTIME DESTINATION ${BIN_DIR}
      LIBRARY DESTINATION ${LIB_DIR}
      ARCHIVE DESTINATION ${LIB_DIR}
      )
  endif(NOT ${EXECNAME_UPPER}_NO_INSTALL AND NOT ${EXECNAME_UPPER}_TESTABLE)
  if(NOT SC_ENABLE_TESTING AND ${EXECNAME_UPPER}_TESTABLE)
    set_target_properties( ${execname} PROPERTIES EXCLUDE_FROM_ALL ON )
  endif(NOT SC_ENABLE_TESTING AND ${EXECNAME_UPPER}_TESTABLE)

  # Enable extra compiler flags if local executables and/or global options dictate
  set(LOCAL_COMPILE_FLAGS "")
  foreach(extraarg ${ARGN})
    if(${extraarg} MATCHES "STRICT" AND SC-ENABLE_STRICT)
      set(LOCAL_COMPILE_FLAGS "${LOCAL_COMPILE_FLAGS} ${STRICT_FLAGS}")
    endif(${extraarg} MATCHES "STRICT" AND SC-ENABLE_STRICT)
  endforeach(extraarg ${ARGN})
  if(LOCAL_COMPILE_FLAGS)
    set_target_properties(${execname} PROPERTIES COMPILE_FLAGS ${LOCAL_COMPILE_FLAGS})
  endif(LOCAL_COMPILE_FLAGS)

endmacro(SC_ADDEXEC execname srcslist libslist)

#SC_ADDLIB(libname "source files" "linked libs" ["TESTABLE"] ["NO_INSTALL"] ["SO_SRCS ..."] ["STATIC_SRCS ..."])
macro(SC_ADDLIB libname srcslist libslist)

  string(TOUPPER "${libname}" LIBNAME_UPPER)
  if(${ARGC} GREATER 3)
    CMAKE_PARSE_ARGUMENTS(${LIBNAME_UPPER} "NO_INSTALL;TESTABLE" "" "SO_SRCS;STATIC_SRCS" ${ARGN})
  endif(${ARGC} GREATER 3)

  string(REGEX REPLACE "-framework;" "-framework " libslist "${libslist1}")
  if(SC_BUILD_SHARED_LIBS)
    add_library(${libname} SHARED ${srcslist} ${${LIBNAME_UPPER}_SO_SRCS})
    DEFINE_DLL_EXPORTS(${libname})
    if(NOT "${libs}" MATCHES "NONE")
      target_link_libraries(${libname} ${libslist})
      DEFINE_DLL_IMPORTS(${libname} "${libslist}")
    endif(NOT "${libs}" MATCHES "NONE")
    set_target_properties(${libname} PROPERTIES VERSION ${SC_ABI_VERSION} SOVERSION ${SC_ABI_SOVERSION})
    if(NOT ${LIBNAME_UPPER}_NO_INSTALL AND NOT ${LIBNAME_UPPER}_TESTABLE)
      install(TARGETS ${libname}
	RUNTIME DESTINATION ${BIN_DIR}
	LIBRARY DESTINATION ${LIB_DIR}
	ARCHIVE DESTINATION ${LIB_DIR}
	)
    endif(NOT ${LIBNAME_UPPER}_NO_INSTALL AND NOT ${LIBNAME_UPPER}_TESTABLE)
    if(NOT SC_ENABLE_TESTING AND ${LIBNAME_UPPER}_TESTABLE)
      set_target_properties( ${libname} PROPERTIES EXCLUDE_FROM_ALL ON )
    endif(NOT SC_ENABLE_TESTING AND ${LIBNAME_UPPER}_TESTABLE)
    if(APPLE)
      set_target_properties(${libname} PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress")
    endif(APPLE)
  endif(SC_BUILD_SHARED_LIBS)
  if(SC_BUILD_STATIC_LIBS)
    if(NOT SC_BUILD_SHARED_LIBS)
      set(staticlibname "${libname}")
    else()
      set(staticlibname "${libname}-static")
    endif(NOT SC_BUILD_SHARED_LIBS)
    add_library(${staticlibname} STATIC ${srcslist} ${${LIBNAME_UPPER}_STATIC_SRCS})
    DEFINE_DLL_EXPORTS(${staticlibname})
    if(NOT ${libs} MATCHES "NONE")
      target_link_libraries(${staticlibname} "${libslist}")
      DEFINE_DLL_IMPORTS(${staticlibname} ${libslist})
    endif(NOT ${libs} MATCHES "NONE")
    if(NOT WIN32)
      set_target_properties(${staticlibname} PROPERTIES OUTPUT_NAME "${libname}")
    endif(NOT WIN32)
    if(WIN32)
      # We need the lib prefix on win32, so add it even if our add_library
      # wrapper function has removed it due to the target name - see
      # http://www.cmake.org/Wiki/CMake_FAQ#How_do_I_make_my_shared_and_static_libraries_have_the_same_root_name.2C_but_different_suffixes.3F
      set_target_properties(${staticlibname} PROPERTIES PREFIX "lib")
    endif(WIN32)
    if(NOT ${LIBNAME_UPPER}_NO_INSTALL AND NOT ${LIBNAME_UPPER}_TESTABLE)
      install(TARGETS ${libname}-static
	RUNTIME DESTINATION ${BIN_DIR}
	LIBRARY DESTINATION ${LIB_DIR}
	ARCHIVE DESTINATION ${LIB_DIR}
	)
    endif(NOT ${LIBNAME_UPPER}_NO_INSTALL AND NOT ${LIBNAME_UPPER}_TESTABLE)
    if(NOT SC_ENABLE_TESTING AND ${LIBNAME_UPPER}_TESTABLE)
      set_target_properties( ${libname}-static PROPERTIES EXCLUDE_FROM_ALL ON )
    endif(NOT SC_ENABLE_TESTING AND ${LIBNAME_UPPER}_TESTABLE)
    if(APPLE)
      set_target_properties(${staticlibname} PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress")
    endif(APPLE)
  endif(SC_BUILD_STATIC_LIBS)
  # Enable extra compiler flags if local libraries and/or global options dictate
  set(LOCAL_COMPILE_FLAGS "")
  foreach(extraarg ${ARGN})
    if(${extraarg} MATCHES "STRICT" AND SC-ENABLE_STRICT)
      set(LOCAL_COMPILE_FLAGS "${LOCAL_COMPILE_FLAGS} ${STRICT_FLAGS}")
    endif(${extraarg} MATCHES "STRICT" AND SC-ENABLE_STRICT)
  endforeach(extraarg ${ARGN})
  if(LOCAL_COMPILE_FLAGS)
    if(BUILD_SHARED_LIBS)
      set_target_properties(${libname} PROPERTIES COMPILE_FLAGS ${LOCAL_COMPILE_FLAGS})
    endif(BUILD_SHARED_LIBS)
    if(BUILD_STATIC_LIBS)
      set_target_properties(${staticlibname} PROPERTIES COMPILE_FLAGS ${LOCAL_COMPILE_FLAGS})
    endif(BUILD_STATIC_LIBS)
  endif(LOCAL_COMPILE_FLAGS)
endmacro(SC_ADDLIB libname srcslist libslist)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

