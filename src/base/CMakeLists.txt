
set(SCL_BASE_SOURCES
    scl_memmgr.cc
    scl_trace_fprintf.c
)

include_directories(
	${SCL_SOURCE_DIR}/include
)

if(MSVC OR BORLAND)
add_definitions( -DSCL_BASE_DLL_EXPORTS )
endif()

SCL_ADDLIB(base "${SCL_BASE_SOURCES}" "")
