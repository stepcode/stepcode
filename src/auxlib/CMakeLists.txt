
set(SC_AUXLIB_SOURCES bstrlib.c)

set(SC_AUXLIB_HDRS bstrlib.h)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

if (${SC_MEMMGR_ENABLE_CHECKS})
  add_definitions(-DSC_MEMMGR_ENABLE_CHECKS)
endif()

SC_ADDLIB(auxlib "${SC_AUXLIB_SOURCES}" "")

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

