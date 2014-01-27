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

MACRO( LOCATE_SCHEMA SCHEMA_FILE _res_var )
  if( EXISTS "${CMAKE_BINARY_DIR}/${SCHEMA_FILE}" )  #is it a path relative to build dir?
    set( ${_res_var} "${CMAKE_BINARY_DIR}/${SCHEMA_FILE}" )
  elseif( EXISTS "${SC_SOURCE_DIR}/data/${SCHEMA_FILE}" )  # path relative to STEPcode/data?
    set( ${_res_var} "${SC_SOURCE_DIR}/data/${SCHEMA_FILE}" )
  elseif( EXISTS ${SCHEMA_FILE} ) # already an absolute path
    set( ${_res_var} ${SCHEMA_FILE} )
  else()
    message( FATAL_ERROR "Cannot find ${CMAKE_BINARY_DIR}/${SCHEMA_FILE} or ${SC_SOURCE_DIR}/data/${SCHEMA_FILE}/*.exp or ${SCHEMA_FILE}" )
  endif()

  if( IS_DIRECTORY ${${_res_var}} ) #if it is a dir, look for one .exp file inside
    file(GLOB ${_res_var} ${${_res_var}}/*.exp )
  endif()

  if( NOT EXISTS ${${_res_var}} )
    message(FATAL_ERROR "Expected one express file. Found '${${_res_var}}' instead.")
  endif()
ENDMACRO( LOCATE_SCHEMA SCHEMA_FILE _res_var )
