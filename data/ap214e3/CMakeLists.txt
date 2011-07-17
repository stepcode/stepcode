#CMakeLists.txt - this is a generic file that should work for any AP without modification.

#SCHEMA_FILE is the name of an express file.
#SCHEMA_NAME is a string that will be prepended to the names of created files.

if( NOT SCHEMA_FILE STREQUAL "" )
  file(GLOB SCHEMA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/*.exp ) #should only be one file
endif()
if( NOT EXISTS ${SCHEMA_FILE} )
  message(FATAL_ERROR "Expected one express file. Found '${SCHEMA_FILE}' instead.")
endif()

# read the schema name from a line like 'SCHEMA AUTOMOTIVE_DESIGN;'
file(STRINGS ${SCHEMA_FILE} SCHEMA_STATEMENT LIMIT_COUNT 1 REGEX "SCHEMA .*")
string(REGEX REPLACE "^SCHEMA \(.*\)\;$" "\\1" SCHEMA_NAME ${SCHEMA_STATEMENT} )
project( sdai_${SCHEMA_NAME} )
message( STATUS "Generating code for ${SCHEMA_NAME}.")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${SCL_SOURCE_DIR}/src/cldai
                     ${SCL_SOURCE_DIR}/src/cleditor ${SCL_SOURCE_DIR}/src/clutils )

add_definitions( -DHAVE_CONFIG_H )

set( SCHEMA_DIR ${CMAKE_BINARY_DIR}/${SCHEMA_NAME} )
include_directories( ${SCHEMA_DIR} ${SCL_SOURCE_DIR}/src/clstepcore )

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
target_link_libraries(${PROJECT_NAME} stepdai stepcore express stepeditor ) #double-check that these are necessary

if( ${ENABLE_P21READ} )
  set( p21read_SOURCES ${SCL_SOURCE_DIR}/src/test/p21read/p21read.cc)
  add_executable( "p21read_${PROJECT_NAME}" ${p21read_SOURCES} )
  target_link_libraries( p21read_${PROJECT_NAME} ${PROJECT_NAME} )
endif()
