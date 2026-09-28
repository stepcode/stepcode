#
# Build STEPCode as an ExternalProject, static libraries only.
#
# This mirrors how OpenVSP builds STEPCode.  After inclusion, the following
# variables are set for use by the consuming project:
#
#   STEPCODE_INSTALL_DIR      - install prefix of the STEPCode build
#   STEPCODE_INCLUDE_DIR      - include directories
#   STEPCODE_LIBRARIES        - full paths to static libraries, in link order
#   STEPCODE_SYSTEM_LIBRARIES - platform libraries STEPCode depends on
#   STEPCODE_DEFINITIONS      - compile definitions required by consumers
#

set( STEPCODE_INSTALL_DIR ${CMAKE_BINARY_DIR}/sc-install )

# Static libraries are named <lib>-static.  Order matters for single-pass
# linkers: dependents before their dependencies.
set( STEPCODE_LIB_NAMES
	sdai_ap203-static
	stepeditor-static
	stepcore-static
	stepdai-static
	steputils-static
)

set( STEPCODE_LIBRARIES )
foreach( _sclib ${STEPCODE_LIB_NAMES} )
	list( APPEND STEPCODE_LIBRARIES
		${STEPCODE_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${_sclib}${CMAKE_STATIC_LIBRARY_SUFFIX} )
endforeach()

set( STEPCODE_SYSTEM_LIBRARIES )
if( WIN32 )
	set( STEPCODE_SYSTEM_LIBRARIES shlwapi )
endif()

set( STEPCODE_INCLUDE_DIR
	${STEPCODE_INSTALL_DIR}/include/stepcode
	${STEPCODE_INSTALL_DIR}/include/stepcode/clstepcore
	${STEPCODE_INSTALL_DIR}/include/stepcode/cldai
	${STEPCODE_INSTALL_DIR}/include/stepcode/clutils
	${STEPCODE_INSTALL_DIR}/include/stepcode/cleditor
	${STEPCODE_INSTALL_DIR}/include/schemas/sdai_ap203
)

# Consumers of the static libraries must define SC_STATIC, otherwise the
# headers declare everything __declspec(dllimport) on Windows.
set( STEPCODE_DEFINITIONS SC_STATIC )

if( CMAKE_BUILD_TYPE )
	set( STEPCODE_BUILD_TYPE ${CMAKE_BUILD_TYPE} )
else()
	set( STEPCODE_BUILD_TYPE Release )
endif()

ExternalProject_Add( STEPCODE
	URL ${CMAKE_CURRENT_SOURCE_DIR}/../../..
	CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
		-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
		-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
		-DCMAKE_BUILD_TYPE=${STEPCODE_BUILD_TYPE}
		-DCMAKE_INSTALL_PREFIX:PATH=${STEPCODE_INSTALL_DIR}
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON
		-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
		-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
		-DSC_BUILD_SCHEMAS=ap203/ap203.exp
		-DBUILD_SHARED_LIBS=OFF
		-DBUILD_STATIC_LIBS=ON
		-DSC_PYTHON_GENERATOR=OFF
		-DSC_ENABLE_TESTING=OFF
	# Ninja must be told about every file a rule produces.
	BUILD_BYPRODUCTS ${STEPCODE_LIBRARIES}
)
