PROJECT( SC_SUBPROJECT_SCHEMA_SCANNER C )
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.7 )

if( NOT ( "${CALLED_FROM}" STREQUAL "STEPCODE_CMAKELISTS" AND DEFINED SC_ROOT AND DEFINED SC_BUILDDIR AND DEFINED OUTDIR ) )
  message(" ${CALLED_FROM}  ${SC_ROOT} ${SC_BUILDDIR} ${OUTDIR} ")
  message( FATAL_ERROR "This is not an independent project; it must be built during the STEPcode configuration stage." )
endif()

# SC_ROOT: SC root dir
# SC_BUILDDIR: SC build dir, so generated headers can be found
# OUTDIR: location to place binary

set( RUNTIME_OUTPUT_DIRECTORY ${OUTDIR} )
set( CMAKE_BUILD_TYPE Debug )

set( schema_scanner_src
    ${SC_ROOT}/src/exp2cxx/genCxxFilenames.c
    ${SC_ROOT}/src/exp2cxx/class_strings.c
    ${SC_ROOT}/src/express/generated/expparse.c
    ${SC_ROOT}/src/express/generated/expscan.c
    ${SC_ROOT}/src/express/alg.c
    ${SC_ROOT}/src/express/caseitem.c
    ${SC_ROOT}/src/express/dict.c
    ${SC_ROOT}/src/express/entity.c
    ${SC_ROOT}/src/express/error.c
    ${SC_ROOT}/src/express/expr.c
    ${SC_ROOT}/src/express/express.c
    ${SC_ROOT}/src/express/hash.c
    ${SC_ROOT}/src/express/lexact.c
    ${SC_ROOT}/src/express/linklist.c
    ${SC_ROOT}/src/express/memory.c
    ${SC_ROOT}/src/express/object.c
    ${SC_ROOT}/src/express/info.c
    ${SC_ROOT}/src/express/resolve.c
    ${SC_ROOT}/src/express/schema.c
    ${SC_ROOT}/src/express/scope.c
    ${SC_ROOT}/src/express/stmt.c
    ${SC_ROOT}/src/express/symbol.c
    ${SC_ROOT}/src/express/type.c
    ${SC_ROOT}/src/express/variable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/schemaScanner.c
    )

include_directories(
    ${SC_ROOT}/include/
    ${SC_ROOT}/src/express/
    ${SC_ROOT}/src/express/generated
    ${SC_ROOT}/src/base
    ${SC_ROOT}/src/exp2cxx
    ${SC_BUILDDIR}/include
)

if(MSVC)
  add_definitions( -D__MSVC__ -D__WIN32__ )
  # Disable warning for preferred usage of secure functions (example strcpy should be strcpy_s, ...)
  add_definitions( -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS )
elseif(BORLAND)
  add_definitions( -D__BORLAND__ -D__WIN32__ )
else()
  add_definitions( -pedantic -W -Wall -Wundef -Wfloat-equal -Wshadow -Winline -Wno-long-long )
endif()

add_executable( schema_scanner ${schema_scanner_src} )
