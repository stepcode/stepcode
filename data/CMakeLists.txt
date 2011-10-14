IF( INSTALL_DATA )
    SET(ap203_data
    ap203/203wseds.exp
    ap203/cube1.p21
    ap203/cube2.p21
    ap203/g_r_assembly1.p21
    ap203/gasket1.p21
    ap203/gasket2.p21
    ap203/gasket3.p21
    ap203/hex_prism1.p21
    ap203/rod_aspect1.p21
    ap203/star1.p21
    )
    INSTALL(FILES ${ap203_data} DESTINATION ${DATA_DIR}/data/ap203)

    SET(ap227_data
    ap227/ap227.exp
    ap227/mitre.p21
    ap227/mitre.step.txt
    )
    INSTALL(FILES ${ap227_data} DESTINATION ${DATA_DIR}/data/ap227)

    SET(step_example_data
    example/example.exp
    )
    INSTALL(FILES ${step_example_data} DESTINATION ${DATA_DIR}/data/example)



    SET(step_datafiles
    pdmnet.exp
    select.exp
    )
    INSTALL(FILES ${step_datafiles} DESTINATION ${DATA_DIR}/data)
ENDIF( INSTALL_DATA )

# To build one or more schemas, configure with 
# 'cmake -DBUILD_SCHEMAS="path/to/schema.exp;path/to/schema2.exp"

# This function runs fedex on one express file. The generated source goes in a dir 
# in the build dir, and it is compiled into a library. A p21read executable is 
# compiled and linked to the lib, unless DISABLE_P21READ is true. p21read is used
# to test the lib.
FUNCTION(BUILD_A_SCHEMA SCHEMA_FILE)
    if( EXISTS "${CMAKE_BINARY_DIR}/${SCHEMA_FILE}" )  #try absolute path. if that fails, must already be absolute.
        set( SCHEMA_FILE "${CMAKE_BINARY_DIR}/${SCHEMA_FILE}" )
    else()
        if( NOT EXISTS ${SCHEMA_FILE} )
            message( FATAL_ERROR "Cannot find ${CMAKE_BINARY_DIR}/${SCHEMA_FILE} or ${SCHEMA_FILE}")
        endif()
    endif()

    if( IS_DIRECTORY ${SCHEMA_FILE} ) #if it is a dir, look for one .exp file inside
        file(GLOB SCHEMA_FILE ${SCHEMA_FILE}/*.exp )
    endif()

    if( NOT EXISTS ${SCHEMA_FILE} )
        message(FATAL_ERROR "Expected one express file. Found '${SCHEMA_FILE}' instead.")
    endif()

    # read the schema name from a line like 'SCHEMA AUTOMOTIVE_DESIGN;'
    file(STRINGS ${SCHEMA_FILE} SCHEMA_STATEMENT LIMIT_COUNT 1 REGEX "SCHEMA .*")
    string(REGEX REPLACE "^SCHEMA \(.*\)\;$" "\\1" SCHEMA_N ${SCHEMA_STATEMENT} )
    string(TOUPPER ${SCHEMA_N} SCHEMA_LONG_NAME) #fedex_plus always uses upper case for file names
    get_filename_component(SCHEMA_SHORT_NAME ${SCHEMA_FILE} NAME_WE)
    
    project( sdai_${SCHEMA_SHORT_NAME} )
    message( STATUS "Generating code for ${SCHEMA_SHORT_NAME}.")
    set( SCHEMA_DIR ${CMAKE_BINARY_DIR}/${SCHEMA_SHORT_NAME} )

    #the names of the files that will be generated
    set( FEDEX_OUT ${SCHEMA_DIR}/compstructs.cc ${SCHEMA_DIR}/Sdaiclasses.h 
                   ${SCHEMA_DIR}/schema.cc ${SCHEMA_DIR}/Sdai${SCHEMA_LONG_NAME}.cc
                   ${SCHEMA_DIR}/schema.h ${SCHEMA_DIR}/Sdai${SCHEMA_LONG_NAME}.h
                   ${SCHEMA_DIR}/SdaiAll.cc ${SCHEMA_DIR}/Sdai${SCHEMA_LONG_NAME}.init.cc )

    # *cannot* use include_directories() because the includes keep piling up - if building
    # multiple schemas, each one will use the include dirs from all previous schemas. Since
    # one header (schema.h) is always named the same, this will not work. only workaround
    # seems to be set_target_properties( <target> PROPERTIES COMPILE_FLAGS <flags> )
    set( ${PROJECT_NAME}_COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR} -I${SCL_SOURCE_DIR}/src/cldai -I${SCL_SOURCE_DIR}/src/cleditor -I${SCL_SOURCE_DIR}/src/clutils -I${SCHEMA_DIR} -I${SCL_SOURCE_DIR}/src/clstepcore -DHAVE_CONFIG_H" )

    add_custom_target( generate_${SCHEMA_SHORT_NAME} SOURCES ${FEDEX_OUT} )
    add_custom_command( OUTPUT ${SCHEMA_DIR}
                        COMMAND cmake ARGS -E make_directory ${SCHEMA_DIR}
                        COMMENT "Creating ${SCHEMA_DIR} for schema ${SCHEMA_SHORT_NAME}")
    add_custom_command( OUTPUT ${FEDEX_OUT}
                        COMMAND fedex_plus ARGS ${SCHEMA_FILE}
                        DEPENDS ${SCHEMA_FILE} ${SCHEMA_DIR}
                        WORKING_DIRECTORY ${SCHEMA_DIR}
                        COMMENT "Running fedex_plus for ${SCHEMA_SHORT_NAME}..."
                        VERBATIM )
    add_library( ${PROJECT_NAME} SHARED ${FEDEX_OUT} )
    target_link_libraries(${PROJECT_NAME} stepdai stepcore express stepeditor )
    add_dependencies( ${PROJECT_NAME} generate_${SCHEMA_SHORT_NAME} )
    set_target_properties( ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS
                           ${${PROJECT_NAME}_COMPILE_FLAGS} )
    
    if( NOT DEFINED ${DISABLE_P21READ} )
        add_executable( "p21read_${PROJECT_NAME}" ${SCL_SOURCE_DIR}/src/test/p21read/p21read.cc )
        target_link_libraries( p21read_${PROJECT_NAME} ${PROJECT_NAME} )
        set_target_properties( "p21read_${PROJECT_NAME}" PROPERTIES COMPILE_FLAGS
                               ${${PROJECT_NAME}_COMPILE_FLAGS} )
    endif()

ENDFUNCTION(BUILD_A_SCHEMA)

if(NOT BUILD_SCHEMAS STREQUAL "" )
  foreach( ap ${BUILD_SCHEMAS} )
    BUILD_A_SCHEMA( ${ap} )
  endforeach()
endif()
