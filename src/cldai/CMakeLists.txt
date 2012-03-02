
set(LIBSTEPDAI_SRCS
    sdaiApplication_instance_set.cc 
    sdaiSession_instance.cc 
    sdaiObject.cc 
    sdaiDaObject.cc 
    sdaiEntity_extent.cc 
    sdaiEntity_extent_set.cc 
    sdaiModel_contents.cc 
    sdaiModel_contents_list.cc
	sdaiEnum.cc
	sdaiString.cc
	sdaiBinary.cc
)

SET(LIBSTEPDAI_PRIVATE_HDRS
    sdaiApplication_instance_set.h 
    sdaiDaObject.h 
    sdaiEntity_extent.h 
    sdaiEntity_extent_set.h 
    sdaiModel_contents.h 
    sdaiModel_contents_list.h 
    sdaiObject.h 
    sdaiSession_instance.h 
    StrUtil.hh
	sdaiEnum.h
	sdaiString.h
	sdaiBinary.h
)

IF(MSVC OR BORLAND)
add_definitions( -DSCL_DAI_DLL_EXPORTS )
add_definitions( -DSCL_UTILS_DLL_IMPORTS )
ENDIF()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SCL_SOURCE_DIR}/src/clstepcore
    ${SCL_SOURCE_DIR}/src/clutils
)

SCL_ADDLIB(stepdai "${LIBSTEPDAI_SRCS}" "steputils")

if(APPLE)
    if(SCL_BUILD_SHARED_LIBS)
        set_target_properties(stepdai PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress")
    endif(SCL_BUILD_SHARED_LIBS)
endif(APPLE)
