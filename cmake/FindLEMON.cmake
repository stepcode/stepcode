#
# - Find lemon executable and provides macros to generate custom build rules
# The module defines the following variables
#
#  LEMON_EXECUTABLE - path to the lemon program
#  LEMON_TEMPLATE - location of the lemon template file

#=============================================================================
#                 F I N D L E M O N . C M A K E
#
# Originally based off of FindBISON.cmake from Kitware's CMake distribution
#
# Copyright (c) 2010-2016 United States Government as represented by
#                the U.S. Army Research Laboratory.
# Copyright 2009 Kitware, Inc.
# Copyright 2006 Tristan Carel
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * The names of the authors may not be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

find_program(LEMON_EXECUTABLE lemon DOC "path to the lemon executable")
mark_as_advanced(LEMON_EXECUTABLE)

if (LEMON_EXECUTABLE)
    get_filename_component(lemon_dir ${LEMON_EXECUTABLE} DIRECTORY)
    find_file(LEMON_TEMPLATE lempar.c 
              PATHS "${lemon_dir}" "/usr/share/lemon"
              NO_DEFAULT_PATH)
    mark_as_advanced(LEMON_TEMPLATE)
    
    if(NOT LEMON_TEMPLATE)
        message(SEND_ERROR "failed to find lemon template (lempar.c)")
    endif()

# Define the macro
#  LEMON_TARGET(<Name> <LemonInput> <LemonSource> <LemonHeader>
#		[<ArgString>])
# which will create a custom rule to generate a parser. <LemonInput> is
# the path to a lemon file. <LemonSource> is the desired name for the
# generated source file. <LemonHeader> is the desired name for the
# generated header which contains the token list. Anything in the optional
# <ArgString> parameter is appended to the lemon command line.
#
#  ====================================================================
#  Example:
#
#   find_package(LEMON)
#   LEMON_TARGET(MyParser parser.y parser.c parser.h)
#   add_executable(Foo main.cpp ${LEMON_MyParser_OUTPUTS})
#  ====================================================================

include(CMakeParseArguments)

if(NOT COMMAND LEMON_TARGET)
  macro(LEMON_TARGET Name LemonInput LemonOutput)
    set(LT_ARGS DEFINES_FILE COMPILE_FLAGS)
    CMAKE_PARSE_ARGUMENTS(LEMON_TARGET_ARG "" "${LT_ARGS}" "" ${ARGN})

    if(NOT "${LEMON_TARGET_ARG_COMPILE_FLAGS}" STREQUAL "")
        set(LEMON_EXECUTABLE_opts "${LEMON_TARGET_ARG_COMPILE_FLAGS}")
        separate_arguments(LEMON_EXECUTABLE_opts)
    else()
        set(LEMON_EXECUTABLE_opts "")
    endif()

    # check for LemonOutput
    get_filename_component(_basename ${LemonOutput} NAME_WE)
    get_filename_component(_out_src_file ${LemonOutput} NAME)
    get_filename_component(_out_dir ${LemonOutput} PATH)
    if(NOT "${_out_dir}" STREQUAL "")
        message(WARNING "Full path specified for LemonOutput - should be filename only")
    endif()
    
    # set report file
    set(_out_rpt_file "${_basename}.out")
    
    # check for DEFINES_FILE
    if ("${LEMON_TARGET_ARG_DEFINES_FILE}" STREQUAL "")
        set(_out_hdr_file "${_basename}.h")
    else()
        get_filename_component(_out_dir ${LEMON_TARGET_ARG_DEFINES_FILE} PATH)
        get_filename_component(_out_hdr_file ${LEMON_TARGET_ARG_DEFINES_FILE} NAME)
        if(NOT "${_out_dir}" STREQUAL "")
            message(WARNING "Full path specified for DEFINES_FILE - should be filename only")
        endif()
    endif()
    
    # input file
    get_filename_component(_in_y_path ${LemonInput} ABSOLUTE)
    get_filename_component(_in_y_file ${LemonInput} NAME)

    # TODO: is this really necessary?
    add_custom_command(
        OUTPUT ${_in_y_file}
        COMMAND ${CMAKE_COMMAND} -E copy ${_in_y_path} ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    # execute lemon
    add_custom_command(
        OUTPUT ${_out_src_file}
        COMMAND ${LEMON_EXECUTABLE} -T${LEMON_TEMPLATE} ${LEMON_EXECUTABLE_opts} ${_in_y_file}
        DEPENDS ${_in_y_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "[LEMON][${Name}] Building parser with ${LEMON_EXECUTABLE}"
    )
    
    # rename the header
    add_custom_command(
        OUTPUT ${_out_hdr_file}
        COMMAND ${CMAKE_COMMAND} ARGS -E rename ${_basename}.h ${_out_hdr_file}
        DEPENDS ${_out_src_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    # set the return values
    set(LEMON_${Name}_DEFINED TRUE)
    set(LEMON_${Name}_OUTPUT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${_out_hdr_file}")
    set(LEMON_${Name}_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    set(LEMON_${Name}_OUTPUTS "${_out_src_file}")

    # make sure we clean up generated output and copied input
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${LEMON_${Name}_OUTPUTS}")
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/${_in_y_file}")

  endmacro(LEMON_TARGET)
endif(NOT COMMAND LEMON_TARGET)

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LEMON REQUIRED_VARS LEMON_EXECUTABLE LEMON_TEMPLATE)

#============================================================
# FindLEMON.cmake ends here

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8
