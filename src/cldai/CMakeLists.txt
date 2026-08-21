set(DAI_SRCS
  sdaiApplication_instance_set.cc
  sdaiBinary.cc
  sdaiDaObject.cc
  sdaiEntity_extent.cc
  sdaiEntity_extent_set.cc
  sdaiEnum.cc
  sdaiModel_contents.cc
  sdaiModel_contents_list.cc
  sdaiObject.cc
  sdaiSession_instance.cc
  sdaiString.cc
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include/stepcode
  )

set(_libdeps steputils)

if(BUILD_SHARED_LIBS)
  SC_ADDLIB(stepdai SHARED SOURCES ${DAI_SRCS} LINK_LIBRARIES ${_libdeps})
  if(WIN32)
    target_compile_definitions(stepdai PRIVATE SC_DAI_DLL_EXPORTS)
  endif()
endif()

if(BUILD_STATIC_LIBS)
  # Spell the static dependency names out rather than composing them with a
  # generator expression. Since 0b950dd1 SC_ADDLIB publishes LINK_LIBRARIES as
  # PUBLIC, so anything given here lands in INTERFACE_LINK_LIBRARIES and is
  # re-evaluated in each consumer - where a nested $<JOIN:...> comes back out
  # mangled and reaches the linker verbatim.
  set(_static_libdeps)
  foreach(_libdep ${_libdeps})
    list(APPEND _static_libdeps ${_libdep}-static)
  endforeach()
  SC_ADDLIB(stepdai-static STATIC SOURCES ${DAI_SRCS} LINK_LIBRARIES ${_static_libdeps})
endif()

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

