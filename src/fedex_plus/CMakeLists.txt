
set(fedex_plus_SOURCES
    classes_misc.c
    ${SCL_SOURCE_DIR}/src/express/fedex.c
    fedex_main.c 
    classes_wrapper.cc 
    classes.c 
    selects.c 
    multpass.c 
    collect.cc 
    complexlist.cc 
    entlist.cc 
    multlist.cc 
    orlist.cc 
    entnode.cc 
    expressbuild.cc 
    non-ors.cc 
    match-ors.cc 
    trynext.cc 
    write.cc 
    print.cc
)

include_directories(
    ${SCL_SOURCE_DIR}/include
    ${SCL_SOURCE_DIR}/include/exppp
    ${SCL_SOURCE_DIR}/include/express
)

IF(MSVC OR BORLAND)
set(fedex_plus_EXTRA_SOURCES
    xgetopt.cc
)
add_definitions( -DSCL_EXPRESS_DLL_IMPORTS )
add_definitions( -DSCL_EXPPP_DLL_IMPORTS )
ENDIF()

IF ("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  add_definitions( -DYYDEBUG ) #-Ddebugging )
ENDIF()

SCL_ADDEXEC(fedex_plus "${fedex_plus_SOURCES} ${fedex_plus_EXTRA_SOURCES}" "libexppp express")

add_dependencies( fedex_plus version_string )