set(LIBEXPPP_SOURCES
    exppp.c
)

# SET(EXPPP_SOURCES
#     ${SCL_SOURCE_DIR}/src/express/fedex.c
#     exppp-main.c
# )

include_directories(
    ${SCL_SOURCE_DIR}/include
    ${SCL_SOURCE_DIR}/include/exppp
)

if(MSVC OR BORLAND)
add_definitions( -DSCL_EXPPP_DLL_EXPORTS )
add_definitions( -DSCL_EXPRESS_DLL_IMPORTS )
endif()

SCL_ADDLIB(libexppp "${LIBEXPPP_SOURCES}" express)
set_target_properties(libexppp PROPERTIES PREFIX "")

#SCL_ADDEXEC(exppp "${EXPPP_SOURCES}" "libexppp express")

if(APPLE)
    set_target_properties(libexppp PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress")
endif(APPLE)
