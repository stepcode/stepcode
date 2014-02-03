
# set compile definitions for dll exports on windows
MACRO(DEFINE_DLL_EXPORTS libname)
  if(MSVC OR BORLAND)
    if(${libname} MATCHES "sdai_.*")
      set(export "SC_SCHEMA_DLL_EXPORTS")
    else()
      STRING(REGEX REPLACE "lib" "" shortname "${libname}")
      STRING(REGEX REPLACE "step" "" LOWERCORE "${shortname}")
      STRING(TOUPPER ${LOWERCORE} UPPER_CORE)
      set(export "SC_${UPPER_CORE}_DLL_EXPORTS")
    endif()
    get_target_property(defs ${libname} COMPILE_DEFINITIONS)
    if(defs) #if no properties, ${defs} will be defs-NOTFOUND which CMake interprets as false
      set(defs "${defs};${export}")
    else(defs)
      set(defs "${export}")
    endif(defs)
    set_target_properties(${libname} PROPERTIES COMPILE_DEFINITIONS "${defs}")
  endif(MSVC OR BORLAND)
endmacro(DEFINE_DLL_EXPORTS libname)

# set compile definitions for dll imports on windows
MACRO(DEFINE_DLL_IMPORTS tgt libs)
  if(MSVC OR BORLAND)
    get_target_property(defs ${tgt} COMPILE_DEFINITIONS)
    if(NOT defs) #if no properties, ${defs} will be defs-NOTFOUND which CMake interprets as false
      set(defs "")
    endif(NOT defs)
    foreach(lib ${libs})
      STRING(REGEX REPLACE "lib" "" shortname "${lib}")
      STRING(REGEX REPLACE "step" "" LOWERCORE "${shortname}")
      STRING(TOUPPER ${LOWERCORE} UPPER_CORE)
      list(APPEND defs "SC_${UPPER_CORE}_DLL_IMPORTS")
    endforeach(lib ${libs})
    if(DEFINED defs)
      if(defs)
        set_target_properties(${tgt} PROPERTIES COMPILE_DEFINITIONS "${defs}")
      endif(defs)
    endif(DEFINED defs)
  endif(MSVC OR BORLAND)
endmacro(DEFINE_DLL_IMPORTS tgt libs)

#EXCLUDE_OR_INSTALL(target destination ARGV3)
# installs ${target} in ${destination} unless testing is enabled AND ${arg_3} == "TESTABLE",
# in which case the EXCLUDE_FROM_ALL property is set for testing.
# EXCLUDE_FROM_ALL cannot be set on targets that are to be installed,
# so either test the target or install it - but not both
MACRO(EXCLUDE_OR_INSTALL target dest arg_3)
  if(NOT ((SC_ENABLE_TESTING) AND ("${arg_3}" STREQUAL "TESTABLE")))
    INSTALL(TARGETS ${target} DESTINATION ${dest})
  else(NOT ((SC_ENABLE_TESTING) AND ("${arg_3}" STREQUAL "TESTABLE")))
    set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL ON)
  endif(NOT ((SC_ENABLE_TESTING) AND ("${arg_3}" STREQUAL "TESTABLE")))
endmacro(EXCLUDE_OR_INSTALL target dest arg_3)

#SC_ADDEXEC(execname "source files" "linked libs" ["TESTABLE"] ["MSVC flag" ...])
# optional 4th argument of "TESTABLE", passed to EXCLUDE_OR_INSTALL macro
# optional args can also be used by MSVC-specific code, but it looks like these two uses
# will not conflict because the MSVC args must contain "STRICT"
macro(SC_ADDEXEC execname srcslist libslist)
  if(SC_BUILD_SHARED_LIBS)
    add_executable(${execname} ${srcslist})
    target_link_libraries(${execname} ${libslist})
    DEFINE_DLL_IMPORTS(${execname} "${libslist}")  #add import definitions for all libs that the executable is linked to
    EXCLUDE_OR_INSTALL(${execname} "bin" "${ARGV3}")
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
  endif(SC_BUILD_SHARED_LIBS)
  if(SC_BUILD_STATIC_LIBS)
    if(NOT SC_BUILD_SHARED_LIBS)
      set(staticexecname "${execname}")
    else()
      set(staticexecname "${execname}-static")
    endif(NOT SC_BUILD_SHARED_LIBS)
    add_executable(${staticexecname} ${srcslist})
    target_link_libraries(${staticexecname} ${libslist})
    EXCLUDE_OR_INSTALL(${staticexecname} "bin" "${ARGV3}")
    # Enable extra compiler flags if local executables and/or global options dictate
    set(LOCAL_COMPILE_FLAGS "")
    foreach(extraarg ${ARGN})
      if(${extraarg} MATCHES "STRICT" AND SC-ENABLE_STRICT)
        set(LOCAL_COMPILE_FLAGS "${LOCAL_COMPILE_FLAGS} ${STRICT_FLAGS}")
      endif(${extraarg} MATCHES "STRICT" AND SC-ENABLE_STRICT)
    endforeach(extraarg ${ARGN})
    if(LOCAL_COMPILE_FLAGS)
      set_target_properties(${staticexecname} PROPERTIES COMPILE_FLAGS ${LOCAL_COMPILE_FLAGS})
    endif(LOCAL_COMPILE_FLAGS)
  endif(SC_BUILD_STATIC_LIBS)
endmacro(SC_ADDEXEC execname srcslist libslist)

#SC_ADDLIB(libname "source files" "linked libs" ["TESTABLE"] ["MSVC flag" ...])
# optional 4th argument of "TESTABLE", passed to EXCLUDE_OR_INSTALL macro
# optional args can also be used by MSVC-specific code, but it looks like these two uses
# will not conflict because the MSVC args must contain "STRICT"
macro(SC_ADDLIB libname srcslist libslist)
  STRING(REGEX REPLACE "-framework;" "-framework " libslist "${libslist1}")
  if(SC_BUILD_SHARED_LIBS)
    add_library(${libname} SHARED ${srcslist})
    DEFINE_DLL_EXPORTS(${libname})
    if(NOT "${libs}" MATCHES "NONE")
      target_link_libraries(${libname} ${libslist})
      DEFINE_DLL_IMPORTS(${libname} "${libslist}")
    endif(NOT "${libs}" MATCHES "NONE")
    set_target_properties(${libname} PROPERTIES VERSION ${SC_ABI_VERSION} SOVERSION ${SC_ABI_SOVERSION})
    EXCLUDE_OR_INSTALL(${libname} "lib" "${ARGV3}")
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
    add_library(${staticlibname} STATIC ${srcslist})
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
    EXCLUDE_OR_INSTALL(${staticlibname} "lib" "${ARGV3}")
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

