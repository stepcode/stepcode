#
# CMakeLists.txt for AP203 Minimum
#
#  This file is released to the public domain.  Any part of this file may be
# freely copied in part or in full for any purpose.  No acknowledgment is required
# for the use of this file.
#
cmake_minimum_required(VERSION 3.12)
project(AP203Minimum)

INCLUDE( ExternalProject )

set(CMAKE_MODULE_PATH ${AP203Minimum_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Sets STEPCODE_INSTALL_DIR, STEPCODE_INCLUDE_DIR, STEPCODE_LIBRARIES,
# STEPCODE_SYSTEM_LIBRARIES and STEPCODE_DEFINITIONS.
INCLUDE( External_STEPCode )

MESSAGE( STATUS "STEPCODE_INSTALL_DIR: " ${STEPCODE_INSTALL_DIR} )

set(SRCS ../ap203min.cpp)

set(HDRS )

add_executable( ${PROJECT_NAME} ${SRCS} ${HDRS})

add_dependencies( ${PROJECT_NAME} STEPCODE )

target_include_directories( ${PROJECT_NAME} PRIVATE ${STEPCODE_INCLUDE_DIR} )

target_compile_definitions( ${PROJECT_NAME} PRIVATE ${STEPCODE_DEFINITIONS} )

target_link_libraries( ${PROJECT_NAME}
    ${STEPCODE_LIBRARIES}
    ${STEPCODE_SYSTEM_LIBRARIES}
)

#####
# Tests: run the example, and verify it has no non-system dynamic dependencies.

enable_testing()

add_test( NAME run_ap203min COMMAND ${PROJECT_NAME}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )

if( APPLE )
	find_program( STATIC_DEPS_TOOL otool )
elseif( WIN32 )
	get_filename_component( _cxx_dir "${CMAKE_CXX_COMPILER}" DIRECTORY )
	get_filename_component( _linker_dir "${CMAKE_LINKER}" DIRECTORY )
	find_program( STATIC_DEPS_TOOL dumpbin HINTS ${_cxx_dir} ${_linker_dir} )
else()
	find_program( STATIC_DEPS_TOOL ldd )
endif()

add_test( NAME static_deps_ap203min
	COMMAND ${CMAKE_COMMAND}
		-DEXE=$<TARGET_FILE:${PROJECT_NAME}>
		-DTOOL=${STATIC_DEPS_TOOL}
		-P ${AP203Minimum_SOURCE_DIR}/cmake/CheckStaticDeps.cmake )

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8
