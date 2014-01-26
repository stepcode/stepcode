# The Express parser uses the tools Perplex, RE2C and Lemon to generate code
# from higher level inputs.  Depending on available tools and options, the
# SC build can either re-generate code as part of the build, or use cached
# files that are ready for compilation.
#
# SC_GENERATE_LEXER_PARSER is the "high level" control a user sets to determine
# how the SC build will interact (or not) with these tools.  AUTO (the
# default) means it will search for the necessary tools, and use them only if
# everything is found.  If not, it will fall back to the cached versions.  If
# this option is set to ON and the necessary tools are not found, the
# configure step will fail.  If it is set to OFF, SC will not even try to use
# the generators and will instead use the cached sources.
if(NOT SC_GENERATE_LEXER_PARSER)
  set(SC_GENERATE_LEXER_PARSER "AUTO" CACHE STRING "Use Perplex, RE2C and Lemon to generate C source code.")
else(NOT SC_GENERATE_LEXER_PARSER)
  string(TOUPPER "${SC_GENERATE_LEXER_PARSER}" SC_GENERATE_LEXER_PARSER)
endif(NOT SC_GENERATE_LEXER_PARSER)
set_property(CACHE SC_GENERATE_LEXER_PARSER PROPERTY STRINGS AUTO ON OFF)
if (NOT "${SC_GENERATE_LEXER_PARSER}" STREQUAL "AUTO" AND NOT "${SC_GENERATE_LEXER_PARSER}" STREQUAL "ON" AND NOT "${SC_GENERATE_LEXER_PARSER}" STREQUAL "OFF")
    message(WARNING "Unknown value ${SC_GENERATE_LEXER_PARSER} supplied for BRLCAD_WORD_SIZE - defaulting to AUTO")
    message(WARNING "Valid options are AUTO, ON and OFF")
    set(SC_GENERATE_LEXER_PARSER "AUTO" CACHE STRING "Use Perplex, RE2C and Lemon to generate C source code.")
endif (NOT "${SC_GENERATE_LEXER_PARSER}" STREQUAL "AUTO" AND NOT "${SC_GENERATE_LEXER_PARSER}" STREQUAL "ON" AND NOT "${SC_GENERATE_LEXER_PARSER}" STREQUAL "OFF")

# If the generators have not been turned off, we need to check for them
if(NOT "${SC_GENERATE_LEXER_PARSER}" STREQUAL "OFF")
    find_package(LEMON)
    find_package(RE2C)
    find_package(PERPLEX)
    if(LEMON_EXECUTABLE AND LEMON_TEMPLATE AND PERPLEX_EXECUTABLE AND PERPLEX_TEMPLATE AND RE2C_EXECUTABLE)
        # Templates may be anywhere - make sure we have a stable path if a relative
        # path was specified at CMake time
        get_filename_component(lemon_template_fpath "${LEMON_TEMPLATE}" ABSOLUTE)
        if(NOT "${lemon_template_fpath}" STREQUAL "${LEMON_TEMPLATE}")
            get_filename_component(LEMON_TEMPLATE "${CMAKE_BINARY_DIR}/${LEMON_TEMPLATE}" ABSOLUTE)
        endif(NOT "${lemon_template_fpath}" STREQUAL "${LEMON_TEMPLATE}")
        get_filename_component(perplex_template_fpath "${PERPLEX_TEMPLATE}" ABSOLUTE)
        if(NOT "${perplex_template_fpath}" STREQUAL "${PERPLEX_TEMPLATE}")
            get_filename_component(PERPLEX_TEMPLATE "${CMAKE_BINARY_DIR}/${PERPLEX_TEMPLATE}" ABSOLUTE)
        endif(NOT "${perplex_template_fpath}" STREQUAL "${PERPLEX_TEMPLATE}")

        if(NOT COMMAND LEMON_TARGET)
            include(${SC_CMAKE_DIR}/LEMON_Util.cmake)
        endif(NOT COMMAND LEMON_TARGET)
        if(NOT COMMAND PERPLEX_TARGET)
            include(${SC_CMAKE_DIR}/PERPLEX_Util.cmake)
        endif(NOT COMMAND PERPLEX_TARGET)
        set(SC_GENERATE_LP_SOURCES 1)
    else(LEMON_EXECUTABLE AND LEMON_TEMPLATE AND PERPLEX_EXECUTABLE AND PERPLEX_TEMPLATE AND RE2C_EXECUTABLE)
        if("${SC_GENERATE_LEXER_PARSER}" STREQUAL "ON")
            message(FATAL_ERROR "\nSC_GENERATE_LEXER_PARSER set to ON, but one or more components of the Perplex/RE2C/Lemon toolchain were not found.\n")
        else("${SC_GENERATE_LEXER_PARSER}" STREQUAL "ON")
            set(SC_GENERATE_LP_SOURCES 0)
        endif("${SC_GENERATE_LEXER_PARSER}" STREQUAL "ON")
    endif(LEMON_EXECUTABLE AND LEMON_TEMPLATE AND PERPLEX_EXECUTABLE AND PERPLEX_TEMPLATE AND RE2C_EXECUTABLE)
else(NOT "${SC_GENERATE_LEXER_PARSER}" STREQUAL "OFF")
    set(SC_GENERATE_LP_SOURCES 0)
endif(NOT "${SC_GENERATE_LEXER_PARSER}" STREQUAL "OFF")
