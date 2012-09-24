
set( clLazyFile_SRCS lazyDataSectionReader.cc lazyFileReader.cc lazyInstMgr.cc p21HeaderSectionReader.cc sectionReader.cc )

set( clLazyFile_HDRS headerSectionReader.h lazyFileReader.h lazyP21DataSectionReader.h p21HeaderSectionReader.h
lazyDataSectionReader.h lazyInstMgr.h lazyTypes.h sectionReader.h )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SCL_SOURCE_DIR}/src/cleditor
    ${SCL_SOURCE_DIR}/src/cldai
    ${SCL_SOURCE_DIR}/src/clstepcore
    ${SCL_SOURCE_DIR}/src/clutils
)


SCL_ADDLIB(steplazyfile "${clLazyFile_SRCS} ${clLazyFile_HDRS}" "stepcore stepdai steputils base")
