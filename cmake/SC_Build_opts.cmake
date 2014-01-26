# testing and compilation options, build output dirs, install dirs, etc
# included by root CMakeLists

IF( NOT DEFINED INCLUDE_INSTALL_DIR )
  SET( INCLUDE_INSTALL_DIR include )
ENDIF( NOT DEFINED INCLUDE_INSTALL_DIR )

IF( NOT DEFINED LIB_INSTALL_DIR )
  SET( LIB_INSTALL_DIR lib )
ENDIF( NOT DEFINED LIB_INSTALL_DIR )

IF( NOT DEFINED BIN_INSTALL_DIR )
  SET( BIN_INSTALL_DIR bin )
ENDIF( NOT DEFINED BIN_INSTALL_DIR )

IF( NOT DEFINED SC_BUILD_TYPE )
  SET( SC_BUILD_TYPE "Debug" CACHE STRING "Build type" ) # By default set debug build
ENDIF( NOT DEFINED SC_BUILD_TYPE )
IF(NOT SC_IS_SUBBUILD)
  SET(CMAKE_BUILD_TYPE ${SC_BUILD_TYPE} CACHE INTERNAL "Build type, immutable" FORCE )
ELSE(NOT SC_IS_SUBBUILD)
  SET(CMAKE_BUILD_TYPE ${SC_BUILD_TYPE} )
ENDIF(NOT SC_IS_SUBBUILD)

# Define helper macro OPTION_WITH_DEFAULT
MACRO( OPTION_WITH_DEFAULT OPTION_NAME OPTION_STRING OPTION_DEFAULT )
  IF( NOT DEFINED ${OPTION_NAME} )
    SET( ${OPTION_NAME} ${OPTION_DEFAULT} )
  ENDIF( NOT DEFINED ${OPTION_NAME} )
  OPTION( ${OPTION_NAME} "${OPTION_STRING}" ${${OPTION_NAME}} )
ENDMACRO( OPTION_WITH_DEFAULT OPTION_NAME OPTION_STRING OPTION_DEFAULT )

# build shared libs by default
OPTION_WITH_DEFAULT(SC_BUILD_SHARED_LIBS "Build shared libs" ON )

# don't build static libs by default
OPTION_WITH_DEFAULT(SC_BUILD_STATIC_LIBS "Build static libs" OFF)

OPTION_WITH_DEFAULT(SC_PYTHON_GENERATOR "Compile exp2python" ON)
OPTION_WITH_DEFAULT(SC_CPP_GENERATOR "Compile exp2cxx" ON)

OPTION_WITH_DEFAULT(SC_MEMMGR_ENABLE_CHECKS "Enable sc_memmgr's memory leak detection" OFF)
OPTION_WITH_DEFAULT(SC_TRACE_FPRINTF "Enable extra comments in generated code so the code's source in exp2cxx may be located" OFF)

OPTION(SC_BUILD_EXPRESS_ONLY "Only build express parser." OFF)
MARK_AS_ADVANCED(SC_BUILD_EXPRESS_ONLY)

#---------------------------------------------------------------------
# Coverage option
OPTION_WITH_DEFAULT( SC_ENABLE_COVERAGE "Enable code coverage test" OFF )
IF(SC_ENABLE_COVERAGE)
  SET(SC_ENABLE_TESTING ON CACHE BOOL "Testing enabled by coverage option" FORCE)
  # build static libs, better coverage report
  SET(SC_BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE )
  SET(SC_BUILD_STATIC_LIBS ON CACHE BOOL "Build static libs" FORCE )
  SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fprofile-arcs -ftest-coverage" CACHE STRING "Extra compile flags required by code coverage" FORCE)
  SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fprofile-arcs -ftest-coverage" CACHE STRING "Extra compile flags required by code coverage" FORCE)
  SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "-fprofile-arcs -ftest-coverage" CACHE STRING "Extra linker flags required by code coverage" FORCE)
  SET(SC_BUILD_TYPE "Debug" CACHE STRING "Build type required by testing framework" FORCE)
  SET( SC_PYTHON_GENERATOR OFF ) #won't build with static libs
ENDIF(SC_ENABLE_COVERAGE)

#---------------------------------------------------------------------
# Testing option
OPTION_WITH_DEFAULT( SC_ENABLE_TESTING "Enable unittesting framework" OFF )
IF(SC_ENABLE_TESTING)
  if( NOT DEFINED SC_BUILD_SCHEMAS )
    set( SC_BUILD_SCHEMAS "ALL" ) #test all schemas, unless otherwise specified
  endif()
  INCLUDE(CTest)
  ENABLE_TESTING()
ENDIF(SC_ENABLE_TESTING)

#---------------------------------------------------------------------
# The following logic is what allows binaries to run successfully in
# the build directory AND install directory.  Thanks to plplot for
# identifying the necessity of setting CMAKE_INSTALL_NAME_DIR on OSX.
# Documentation of these options is available at
# http://www.cmake.org/Wiki/CMake_RPATH_handling

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# the RPATH/INSTALL_NAME_DIR to be used when installing
if (NOT APPLE)
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:\$ORIGIN/../lib")
endif(NOT APPLE)
# On OSX, we need to set INSTALL_NAME_DIR instead of RPATH
# http://www.cmake.org/cmake/help/cmake-2-8-docs.html#variable:CMAKE_INSTALL_NAME_DIR
set(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH which point to
# directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# When this is a subbuild, assume that the parent project controls all of the following
#======================================================================================
IF( NOT SC_IS_SUBBUILD )

  # Output directories. In a separate file so it can be used by the schema scanner CMake as well.
  include( ${SC_CMAKE_DIR}/SC_Outdirs.cmake )

  #-----------------------------------------------------------------------------
  # Configure install locations. Only do this if CMAKE_INSTALL_PREFIX hasn't
  # been set already, to try and allow parent builds (if any) some control.
  #
  # Need a good Debug location for Windows.
  IF(NOT WIN32)
    IF (${CMAKE_BUILD_TYPE} MATCHES "Debug")
      SET(SC_INSTALL_PREFIX "${SC_SOURCE_DIR}/../sc-install")
    ELSE()
      SET(SC_INSTALL_PREFIX "/usr/local")
    ENDIF()
  ENDIF(NOT WIN32)
  SET( SC_INSTALL_PREFIX ${SC_INSTALL_PREFIX} CACHE
    PATH "Install prefix prepended to target to create install location" )
  SET( CMAKE_INSTALL_PREFIX ${SC_INSTALL_PREFIX} CACHE INTERNAL "Prefix prepended to install directories if target destination is not absolute, immutable" FORCE )

  #-----------------------------------------------------------------------------
  # SC Packaging
  # $make package
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "STEPcode")
  SET(CPACK_SET_DESTDIR "ON")
  SET(CPACK_PACKAGE_VERSION_MAJOR ${SC_VERSION_MAJOR})
  SET(CPACK_PACKAGE_VERSION_MINOR ${SC_VERSION_MINOR})
  SET(CPACK_PACKAGE_NAME SC )
  SET(CPACK_PACKAGE_CONTACT "SC Developers <scl-dev@googlegroups.com>")
  INCLUDE(CPack)

  #-----------------------------------------------------------------------------
  # Uninstall target
  # From http://www.cmake.org/Wiki/CMake_FAQ#Can_I_do_.22make_uninstall.22_with_CMake.3F
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

endif(NOT SC_IS_SUBBUILD)
