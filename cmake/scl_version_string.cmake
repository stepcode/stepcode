#use git for a pretty commit id
#uses 'git describe --tags', so tags are required in the repo
#create a tag with 'git tag <name>' and 'git push --tags'

# http://stackoverflow.com/questions/3780667
# http://www.cmake.org/pipermail/cmake/2009-February/027014.html

#scl_version_string.h defines scl_version() which returns a pretty commit description and a build timestamp.
if( EXISTS ${SOURCE_DIR}/.git )
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags RESULT_VARIABLE res_var OUTPUT_VARIABLE GIT_COM_ID )
        if( NOT ${res_var} EQUAL 0 )
            set( GIT_COMMIT_ID "unknown (no tags?)")
            message( WARNING "Git failed (invalid repo, or no tags). Build will not contain git revision info." )
        endif()
        string( REPLACE "\n" "" GIT_COMMIT_ID ${GIT_COM_ID} )
    else(GIT_FOUND)
        set( GIT_COMMIT_ID "unknown (git not found!)")
        message( WARNING "Git not found. Build will not contain git revision info." )
    endif(GIT_FOUND)
else()
    set( GIT_COMMIT_ID "unknown (not a repository!)")
    message( WARNING "Git failed (.git not found). Build will not contain git revision info." )
endif()

set( res_var 1 )
IF (WIN32)
    EXECUTE_PROCESS(COMMAND "date" "/T" RESULT_VARIABLE res_var OUTPUT_VARIABLE TIMESTAMP )
ELSEIF(UNIX)
    EXECUTE_PROCESS(COMMAND "date" RESULT_VARIABLE res_var OUTPUT_VARIABLE TIMESTAMP )
ELSE()
    MESSAGE(WARNING "Unknown platform: date not included in version string")
ENDIF()
if( NOT ${res_var} EQUAL 0 )
    SET( TIMESTAMP "000000" )
else()
    string( REPLACE "\n" "" TIMESTAMP ${TIMESTAMP} )
endif()

set( vstring "//scl_version_string.h - written by cmake. Changes will be lost!\n"
             "#ifndef SCL_VERSION_STRING\n"
             "#define SCL_VERSION_STRING\n\n"
             "/*\n** Returns a string like \"test-1-g5e1fb47, built on <timestamp>\", where test is the\n"
             "** name of the last tagged git revision, 1 is the number of commits since that tag,\n"
             "** 'g' is unknown, 5e1fb47 is the first 7 chars of the git sha1 commit id, and\n"
             "** <timestamp> is the output of your system's 'date' command.\n*/\n\n"
             "const char* scl_version() {\n"
             "    return \"git commit id ${GIT_COMMIT_ID}, built on ${TIMESTAMP}\"\;\n"
             "}\n\n"
             "#endif\n"
   )

file(WRITE scl_version_string.h.txt ${vstring} )
# copy the file to the final header only if the version changes
# reduces needless rebuilds
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        scl_version_string.h.txt ${BINARY_DIR}/include/scl_version_string.h)
message("-- scl_version_string.h is up-to-date.")
