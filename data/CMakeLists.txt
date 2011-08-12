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
    string(TOUPPER ${SCHEMA_N} SCHEMA_NAME) #fedex_plus always uses upper case for file names
    
    project( sdai_${SCHEMA_NAME} )
    message( STATUS "Generating code for ${SCHEMA_NAME}.")
    set( SCHEMA_DIR ${CMAKE_BINARY_DIR}/${SCHEMA_NAME} )

    include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${SCL_SOURCE_DIR}/src/cldai
                         ${SCL_SOURCE_DIR}/src/cleditor ${SCL_SOURCE_DIR}/src/clutils
                         ${SCHEMA_DIR} ${SCL_SOURCE_DIR}/src/clstepcore )
    add_definitions( -DHAVE_CONFIG_H )

    #the names of the files that will be generated
    set( FEDEX_OUT ${SCHEMA_DIR}/compstructs.cc ${SCHEMA_DIR}/Sdaiclasses.h 
                   ${SCHEMA_DIR}/schema.cc ${SCHEMA_DIR}/Sdai${SCHEMA_NAME}.cc
                   ${SCHEMA_DIR}/schema.h ${SCHEMA_DIR}/Sdai${SCHEMA_NAME}.h
                   ${SCHEMA_DIR}/SdaiAll.cc ${SCHEMA_DIR}/Sdai${SCHEMA_NAME}.init.cc )

    add_custom_command( OUTPUT ${SCHEMA_DIR}
                        COMMAND cmake ARGS -E make_directory ${SCHEMA_DIR} 
                        COMMENT "Creating ${SCHEMA_DIR} for schema ${SCHEMA_NAME}")
    add_custom_command( OUTPUT ${FEDEX_OUT}
                        COMMAND fedex_plus ARGS ${SCHEMA_FILE}
                        DEPENDS ${SCHEMA_FILE} ${SCHEMA_DIR}
                        WORKING_DIRECTORY ${SCHEMA_DIR}
                        COMMENT "Running fedex_plus for ${SCHEMA_NAME}..."
                        VERBATIM )
    add_library( ${PROJECT_NAME} SHARED ${FEDEX_OUT} )
    target_link_libraries(${PROJECT_NAME} stepdai stepcore express stepeditor )
    
    if( NOT DEFINED ${DISABLE_P21READ} )
        add_executable( "p21read_${PROJECT_NAME}" ${SCL_SOURCE_DIR}/src/test/p21read/p21read.cc )
        target_link_libraries( p21read_${PROJECT_NAME} ${PROJECT_NAME} )
    endif()
ENDFUNCTION(BUILD_A_SCHEMA)

if(NOT BUILD_SCHEMAS STREQUAL "" )
  foreach( ap ${BUILD_SCHEMAS} )
    BUILD_A_SCHEMA( ${ap} )
  endforeach()
endif()
