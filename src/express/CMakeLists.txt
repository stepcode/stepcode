IF ("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  #add_definitions( -Ddebugging )
  #set( EXTRA_YACC_FLAGS "-Wall" ) #-Wall introduced somewhere between bison v2.3 and v2.4.1

  #This graph is HUGE! dot takes a very long time, so does loading it in a viewer
  #enable with -DBISON_GRAPH=TRUE
  if( BISON_GRAPH )
    message("Bison will create a graph of the scanner flow at ${SCL_BINARY_DIR}/expparse.dot")
    set( EXTRA_YACC_FLAGS "${EXTRA_YACC_FLAGS} --graph=${SCL_BINARY_DIR}/expparse.dot" )
  endif( BISON_GRAPH )
  set( YACC_FLAGS "${EXTRA_YACC_FLAGS} " )
  set( LEX_FLAGS " " ) # -d for lexer debugging
ELSE()
  set( LEX_FLAGS " " )
  set( YACC_FLAGS " " ) #FindYACC.cmake insists on 3, 5, or 7 args
ENDIF()

add_definitions( -DYYDEBUG )  #always define this. equivalent to 'bison -t'


YACC_TARGET(ExpParser expparse.y ${CMAKE_CURRENT_BINARY_DIR}/expparse.c COMPILE_FLAGS ${YACC_FLAGS})
LEX_TARGET(ExpScanner expscan.l ${CMAKE_CURRENT_BINARY_DIR}/expscan.c COMPILE_FLAGS ${LEX_FLAGS})
ADD_LEX_YACC_DEPENDENCY(ExpScanner ExpParser)

set(EXPRESS_SOURCES
    ${YACC_ExpParser_OUTPUTS}
    ${LEX_ExpScanner_OUTPUTS}
    symbol.c 
    type.c 
    variable.c 
    expr.c 
    entity.c 
    caseitem.c 
    stmt.c 
    alg.c 
    scope.c 
    schema.c 
    resolve.c 
    lexact.c 
    linklist.c 
    error.c 
    dict.c 
    hash.c 
    memory.c 
    object.c 
    inithook.c 
    express.c
)

SET(CHECK_EXPRESS_SOURCES
     fedex.c
)

SET(EXPRESS_PRIVATE_HDRS
    conf.h
    exptoks.h
    stack.h
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions( -DFLEX )

if(MSVC)
add_definitions( -DYY_NO_UNISTD_H )
add_definitions( -DSCL_EXPRESS_DLL_EXPORTS )
endif(MSVC)

SCL_ADDLIB(express "${EXPRESS_SOURCES}" "")

if(APPLE)
    set_target_properties(express PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress")
endif(APPLE)

SCL_ADDEXEC("check-express" "${CHECK_EXPRESS_SOURCES}" express)
