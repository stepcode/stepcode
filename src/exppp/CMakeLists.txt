set(LIBEXPPP_SOURCES
    exppp.c
)

# SET(EXPPP_SOURCES
#     ../express/fedex.c
#     exppp-main.c
# )

include_directories(
    ${SCL_SOURCE_DIR}/include
    ${SCL_SOURCE_DIR}/include/exppp
    ${SCL_SOURCE_DIR}/src/base
)


if( MSVC OR BORLAND)
    add_definitions( -D__STDC__ )
endif()

SCL_ADDLIB(libexppp "${LIBEXPPP_SOURCES}" "express base")
if(SCL_BUILD_SHARED_LIBS)
    set_target_properties(libexppp PROPERTIES PREFIX "")
endif(SCL_BUILD_SHARED_LIBS)

#SCL_ADDEXEC(exppp "${EXPPP_SOURCES}" "libexppp express")

