#
# Verify that an executable does not dynamically link anything except
# operating system and compiler runtime libraries.
#
# Usage:
#   cmake -DEXE=<executable> -DTOOL=<otool|ldd|dumpbin> -P CheckStaticDeps.cmake
#
# The dependency tool's full output is printed as a diagnostic.  The check
# fails if any dependency is not a system library, cannot be resolved, or
# looks like a STEPCode library.
#
cmake_minimum_required( VERSION 3.12 )

if( NOT EXE OR NOT EXISTS "${EXE}" )
	message( FATAL_ERROR "Executable not found: '${EXE}'" )
endif()
if( NOT TOOL )
	message( FATAL_ERROR "No dependency tool (otool, ldd or dumpbin) was found." )
endif()

get_filename_component( _tool_name "${TOOL}" NAME_WE )
string( TOLOWER "${_tool_name}" _tool_name )

if( _tool_name STREQUAL "otool" )
	set( _args -L "${EXE}" )
elseif( _tool_name STREQUAL "ldd" )
	set( _args "${EXE}" )
elseif( _tool_name STREQUAL "dumpbin" )
	set( _args /dependents "${EXE}" )
else()
	message( FATAL_ERROR "Unsupported dependency tool: ${TOOL}" )
endif()

execute_process( COMMAND "${TOOL}" ${_args}
	OUTPUT_VARIABLE _out
	ERROR_VARIABLE _err
	RESULT_VARIABLE _res )

list( JOIN _args " " _args_str )
message( "${TOOL} ${_args_str}\n${_out}${_err}" )

if( NOT _res EQUAL 0 )
	message( FATAL_ERROR "${TOOL} failed with exit code ${_res}" )
endif()

# Collect (name, resolved path) for each dependency.
set( _deps )
set( _bad )
string( REPLACE "\r" "" _out "${_out}" )
string( REPLACE "\n" ";" _lines "${_out}" )

if( _tool_name STREQUAL "otool" )
	# First line is the executable itself; dependencies are indented.
	foreach( _line ${_lines} )
		if( _line MATCHES "^[ \t]+([^ \t].*) \\(compatibility version" )
			set( _path "${CMAKE_MATCH_1}" )
			list( APPEND _deps "${_path}" )
			if( NOT _path MATCHES "^/usr/lib/" AND NOT _path MATCHES "^/System/Library/" )
				list( APPEND _bad "${_path}" )
			endif()
		endif()
	endforeach()

elseif( _tool_name STREQUAL "ldd" )
	foreach( _line ${_lines} )
		string( STRIP "${_line}" _line )
		if( _line STREQUAL "" OR _line MATCHES "statically linked" )
			continue()
		endif()
		list( APPEND _deps "${_line}" )
		if( _line MATCHES "not found" )
			list( APPEND _bad "${_line}" )
		elseif( _line MATCHES "=> (/[^ ]+)" )
			set( _path "${CMAKE_MATCH_1}" )
		elseif( _line MATCHES "^(/[^ ]+)" )
			set( _path "${CMAKE_MATCH_1}" )
		else()
			# Virtual DSO such as linux-vdso.so.1 has no file.
			continue()
		endif()
		if( NOT _path MATCHES "^/(usr/)?lib(32|64|x32)?/" )
			list( APPEND _bad "${_line}" )
		endif()
	endforeach()

elseif( _tool_name STREQUAL "dumpbin" )
	set( _in_deps FALSE )
	foreach( _line ${_lines} )
		string( STRIP "${_line}" _line )
		if( _line MATCHES "has the following dependencies" )
			set( _in_deps TRUE )
			continue()
		endif()
		if( NOT _in_deps OR _line STREQUAL "" )
			continue()
		endif()
		if( _line MATCHES "^Summary" OR _line MATCHES "delay load" )
			break()
		endif()
		list( APPEND _deps "${_line}" )
		# System DLLs (including the MSVC and UCRT runtimes) live in System32.
		file( TO_CMAKE_PATH "$ENV{SystemRoot}/System32/${_line}" _sys )
		if( NOT EXISTS "${_sys}" AND NOT _line MATCHES "^(api|ext)-ms-" )
			list( APPEND _bad "${_line}" )
		endif()
	endforeach()
endif()

if( NOT _deps )
	message( FATAL_ERROR "Could not parse any dependencies from ${TOOL} output." )
endif()

# Catch STEPCode libraries even if they were somehow installed system-wide.
foreach( _dep ${_deps} )
	get_filename_component( _name "${_dep}" NAME )
	if( _name MATCHES "(step(core|dai|utils|editor|lazyfile)|sdai_|express|exppp)" )
		list( APPEND _bad "${_dep}" )
	endif()
endforeach()

list( LENGTH _deps _ndeps )
if( _bad )
	list( REMOVE_DUPLICATES _bad )
	string( REPLACE ";" "\n  " _bad "${_bad}" )
	message( FATAL_ERROR "Non-system dynamic dependencies found:\n  ${_bad}" )
endif()

message( STATUS "OK: all ${_ndeps} dynamic dependencies are system libraries." )
