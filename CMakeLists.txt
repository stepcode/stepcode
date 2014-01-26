#    C M A K E L I S T S . T X T   F O R   S T E P C O D E
#
# This file is Copyright (c) 2010 United States Government as
# represented by the U.S. Army Research Laboratory.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following
# disclaimer in the documentation and/or other materials provided
# with the distribution.
#
# 3. The name of the author may not be used to endorse or promote
# products derived from this software without specific prior written
# permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# *******************************************************************
# ***                   SC's CMakeLists.txt                      ***
# *******************************************************************

# This file contains the top level CMakeLists.txt logic for the
# SC software package.

PROJECT(SC)

# SC version
SET(SC_VERSION_MAJOR 0)
SET(SC_VERSION_MINOR 7-dev)
SET(SC_VERSION ${SC_VERSION_MAJOR}.${SC_VERSION_MINOR})

# SC ABI version. SC_ABI_SOVERSION should be incremented
# for each release introducing API incompatibilities
SET(SC_ABI_SOVERSION 2)
SET(SC_ABI_VERSION ${SC_ABI_SOVERSION}.0.0)

# Minimum required version of CMake
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

# CMake derives much of its functionality from modules, typically
# stored in one directory - let CMake know where to find them.
SET(SC_CMAKE_DIR "${SC_SOURCE_DIR}/cmake")
if(NOT IS_SUBBUILD)
  SET(CMAKE_MODULE_PATH "${SC_CMAKE_DIR};${CMAKE_MODULE_PATH}")
else(NOT IS_SUBBUILD)
  SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${SC_CMAKE_DIR}")
endif(NOT IS_SUBBUILD)

# testing and compilation options, build output dirs, install dirs, uninstall, package creation, etc
include( ${SC_CMAKE_DIR}/SC_Build_opts.cmake )

# SC_ADDEXEC and SC_ADDLIB macros, dllimport/export, etc
include( ${SC_CMAKE_DIR}/SC_Targets.cmake )

# Macros related to paths
include( ${SC_CMAKE_DIR}/SC_Paths.cmake )

# locale stuff
include( ${SC_CMAKE_DIR}/SC_Locale.cmake )

# logic related to regenerating the lexer and parser source code
include( ${SC_CMAKE_DIR}/SC_Regenerate.cmake )

if(NOT DEFINED SC_SDAI_ADDITIONAL_EXES_SRCS )
  set( SC_SDAI_ADDITIONAL_EXES_SRCS "" CACHE STRING "Source files for additional executables to be linked with SDAI libs" )
endif(NOT DEFINED SC_SDAI_ADDITIONAL_EXES_SRCS )

if( NOT DEFINED SC_BUILD_SCHEMAS )
    set( SCHEMAS_WARNING TRUE )
    #this makes SC_BUILD_SCHEMAS show up in cmake-gui
    SET(SC_BUILD_SCHEMAS "ALL" CACHE string "Semicolon-separated list of paths to EXPRESS schemas to be built" )
endif( NOT DEFINED SC_BUILD_SCHEMAS )

# create config headers sc_cf.h and sc_version_string.h
include( ${SC_CMAKE_DIR}/SC_Config_Headers.cmake )


################

if(MSVC)
  # add_definitions( -Wall )
  add_definitions( -D__MSVC__ -D__WIN32__ )
  # Disable warning for preferred usage of secure functions (example strcpy should be strcpy_s, ...)
  add_definitions( -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS )
elseif(BORLAND)
  add_definitions( -D__BORLAND__ -D__WIN32__ )
else()
  add_definitions( -pedantic -W -Wall -Wundef -Wfloat-equal -Wshadow -Winline -Wno-long-long )
endif()

include_directories(
  ${SC_SOURCE_DIR}/include
  ${SC_BINARY_DIR}/include
  )

ADD_SUBDIRECTORY(src/base)
ADD_SUBDIRECTORY(src/express)
ADD_SUBDIRECTORY(src/exppp)
ADD_SUBDIRECTORY(src/exp2cxx)
ADD_SUBDIRECTORY(src/exp2python)
ADD_SUBDIRECTORY(src/clstepcore)
ADD_SUBDIRECTORY(src/cleditor)
ADD_SUBDIRECTORY(src/cldai)
ADD_SUBDIRECTORY(src/clutils)
if( NOT WIN32 ) # don't build cllazyfile on windows until export/import macros are in place
  ADD_SUBDIRECTORY( src/cllazyfile )
endif( NOT WIN32 )
ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(data)
IF(SC_ENABLE_TESTING)
  ADD_SUBDIRECTORY( test )
ENDIF(SC_ENABLE_TESTING)
ADD_SUBDIRECTORY(doc)

if( SCHEMAS_WARNING )
    message( "** CMake variable SC_BUILD_SCHEMAS was not set. Defaults to building ALL schemas, which will take a")
    message("     while; see http://stepcode.org/mw/index.php?title=STEPcode_CMake_variables#SC_BUILD_SCHEMAS" )
endif( SCHEMAS_WARNING )
message( ".. Don't worry about any messages above about missing headers or failed tests, as long as\n    you see '-- Configuring done' below. Headers and features vary by compiler." )

# this is for testing - 'make core' builds everything that isn't generated
add_custom_target( core )
add_dependencies( core stepdai check-express stepeditor exp2cxx )
