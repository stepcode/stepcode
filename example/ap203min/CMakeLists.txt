PROJECT( AP203Minimum )
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Set STEPCODE_ROOT_DIR to point to the root of the STEPcode source tree.

# Use STEPcode as library, but build from this build process.
set( SC_IS_SUBBUILD TRUE )

# STEPcode build type, DEBUG or RELEASE
set( SC_BUILD_TYPE DEBUG )

# Path to STEPcode
set( STEPCODE_BUILD_DIR ${CMAKE_BINARY_DIR}/sc CACHE PATH "STEPcode build subdir" )

# STEP schema to build and link against.
set( SCHEMA ap203 )
set( SC_BUILD_SCHEMAS ${SCHEMA} CACHE INTERNAL "Schema, internal, immutable" FORCE )

# Parse out schema name and library.
file( GLOB SCHEMA_FILE "${STEPCODE_ROOT_DIR}/data/${SCHEMA}/*.exp" )
get_filename_component( SCHEMA_SN ${SCHEMA_FILE} NAME )
string( REGEX REPLACE "\(.*\).[Ee][Xx][Pp]" "\\1" SCHEMA_LINK_NAME ${SCHEMA_SN} )
set( STEPCODE_LIBRARIES base stepcore stepeditor stepdai steputils sdai_${SCHEMA_LINK_NAME} )

# Add STEPCode project to CMake build.
add_subdirectory( ${STEPCODE_ROOT_DIR} "${CMAKE_CURRENT_BINARY_DIR}/sc" EXCLUDE_FROM_ALL )

# Set up STEPcode include directories.
set( STEPCODE_INCLUDE_DIR
${STEPCODE_ROOT_DIR}/src/base
${STEPCODE_ROOT_DIR}/src/clstepcore
${STEPCODE_ROOT_DIR}/src/cldai
${STEPCODE_ROOT_DIR}/src/clutils
${STEPCODE_ROOT_DIR}/src/cleditor
${STEPCODE_BUILD_DIR}/include
${STEPCODE_ROOT_DIR}/include
${CMAKE_BINARY_DIR}
)

# Reset sane values for build destination.  Otherwise, everything ends up in ${CMAKE_BINARY_DIR}/sc
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

include_directories(
${STEPCODE_INCLUDE_DIR}
)

# Application sources and headers
set( SRCS
ap203min.cpp
)

set( HDRS
)

add_executable( ${PROJECT_NAME} ${SRCS} ${HDRS} )

target_link_libraries( ${PROJECT_NAME} ${STEPCODE_LIBRARIES} )

set_target_properties( ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-I${STEPCODE_BUILD_DIR}/data/${SCHEMA_LINK_NAME}" )
