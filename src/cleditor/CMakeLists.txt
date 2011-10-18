
set(LIBSTEPEDITOR_SRCS
    STEPfile.cc 
    STEPfile.inline.cc 
    cmdmgr.cc 
    dispnode.cc 
    dispnodelist.cc 
    instmgr.cc 
    mgrnode.cc 
    mgrnodearray.cc 
    mgrnodelist.cc 
    needFunc.cc 
    SdaiHEADER_SECTION_SCHEMA.cc
    SdaiHEADER_SECTION_SCHEMA.All.cc
    SdaiHEADER_SECTION_SCHEMA.init.cc
    SdaiHEADER_SECTION_SCHEMA.schema.cc
)

SET(LIBSTEPEDITOR_PRIVATE_HDRS
    STEPfile.h 
    cmdmgr.h 
    dispnode.h 
    dispnodelist.h 
    editordefines.h 
    instmgr.h 
    mgrnode.h 
    mgrnodearray.h 
    mgrnodelist.h 
    needFunc.h 
    SdaiHEADER_SECTION_SCHEMA.h
    SdaiHEADER_SECTION_SCHEMA.classes.h
    SdaiHEADER_SECTION_SCHEMA.schema.h
    seeinfodefault.h
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SCL_SOURCE_DIR}/src/cldai
    ${SCL_SOURCE_DIR}/src/clstepcore
    ${SCL_SOURCE_DIR}/src/clutils
)

add_definitions(
    -DHAVE_CONFIG_H
)

SCL_ADDLIB(stepeditor "${LIBSTEPEDITOR_SRCS}" stepcore)
