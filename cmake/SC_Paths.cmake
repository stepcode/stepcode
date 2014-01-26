MACRO(RELATIVE_PATH_TO_TOPLEVEL current_dir rel_path)
  string(REPLACE "${SC_SOURCE_DIR}" "" subpath "${current_dir}")
  string(REGEX REPLACE "^/" "" subpath "${subpath}")
  string(LENGTH "${subpath}" PATH_LENGTH)
  if(PATH_LENGTH GREATER 0)
    set(${rel_path} "..")
    get_filename_component(subpath "${subpath}" PATH)
    string(LENGTH "${subpath}" PATH_LENGTH)
    while(PATH_LENGTH GREATER 0)
      set(${rel_path} "${${rel_path}}/..")
      get_filename_component(subpath "${subpath}" PATH)
      string(LENGTH "${subpath}" PATH_LENGTH)
    endwhile(PATH_LENGTH GREATER 0)
  endif(PATH_LENGTH GREATER 0)
ENDMACRO(RELATIVE_PATH_TO_TOPLEVEL current_dir rel_path)

MACRO( LOCATE_SCHEMA SCHEMA_FILE )
  if( EXISTS "${CMAKE_BINARY_DIR}/${SCHEMA_FILE}" )  #is it a path relative to build dir?
    set( SCHEMA_FILE "${CMAKE_BINARY_DIR}/${SCHEMA_FILE}" )
  elseif( EXISTS "${SC_SOURCE_DIR}/data/${SCHEMA_FILE}" )  # path relative to STEPcode/data?
    set( SCHEMA_FILE "${SC_SOURCE_DIR}/data/${SCHEMA_FILE}" )
  elseif( NOT EXISTS ${SCHEMA_FILE} ) # absolute path?
    message( FATAL_ERROR "Cannot find ${CMAKE_BINARY_DIR}/${SCHEMA_FILE} or ${SC_SOURCE_DIR}/data/${SCHEMA_FILE}/*.exp or ${SCHEMA_FILE}" )
  endif()

  if( IS_DIRECTORY ${SCHEMA_FILE} ) #if it is a dir, look for one .exp file inside
    file(GLOB SCHEMA_FILE ${SCHEMA_FILE}/*.exp )
  endif()

  if( NOT EXISTS ${SCHEMA_FILE} )
    message(FATAL_ERROR "Expected one express file. Found '${SCHEMA_FILE}' instead.")
  endif()
ENDMACRO( LOCATE_SCHEMA SCHEMA_FILE )
