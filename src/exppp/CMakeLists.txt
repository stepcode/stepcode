set(LIBEXPPP_SOURCES
  exppp.c
  pretty_alg.c
  pretty_case.c
  pretty_entity.c
  pretty_expr.c
  pretty_express.c
  pretty_func.c
  pretty_loop.c
  pretty_proc.c
  pretty_ref.c
  pretty_rule.c
  pretty_schema.c
  pretty_scope.c
  pretty_stmt.c
  pretty_subtype.c
  pretty_type.c
  pretty_where.c
 )

SET(EXPPP_SOURCES
  ../express/fedex.c
  exppp-main.c
 )

include_directories(
  ${SC_SOURCE_DIR}/include
  ${SC_SOURCE_DIR}/include/exppp
  ${SC_SOURCE_DIR}/src/base
  ${SC_SOURCE_DIR}/src/express
 )

if(BORLAND)
  add_definitions(-D__STDC__)
endif()

SC_ADDLIB(libexppp "${LIBEXPPP_SOURCES}" "express;base")
set_target_properties(libexppp PROPERTIES PREFIX "")

SC_ADDEXEC(exppp "${EXPPP_SOURCES}" "libexppp;express;base")

if(SC_ENABLE_TESTING)
  add_subdirectory(test)
endif(SC_ENABLE_TESTING)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

