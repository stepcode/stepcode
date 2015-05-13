# creates sc_version_string.h, which defines sc_version()
# sc_version() returns a pretty commit description and a build timestamp.

# only update the file if the git commit has changed, because whenever the file is updated files including the header must rebuild
# parallel rebuilds can result in race conditions and failures, particularly when running ctest in parallel

# http://stackoverflow.com/questions/3780667
# http://www.cmake.org/pipermail/cmake/2009-February/027014.html

set(SC_IS_SUBBUILD "@SC_IS_SUBBUILD@")

set(SC_VERSION_HEADER "${BINARY_DIR}/include/sc_version_string.h")

#---------- find commit id ------------------
#use git for a pretty commit id
#uses 'git describe --tags', so tags are required in the repo
#create a tag with 'git tag <name>' and 'git push --tags'
#if git can't be found, uses contents of SC_VERSION.txt

set(VERS_FILE ${SOURCE_DIR}/SC_VERSION.txt)
if(EXISTS ${SOURCE_DIR}/.git)
  find_package(Git QUIET)
  if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags WORKING_DIRECTORY ${SOURCE_DIR}
      RESULT_VARIABLE res_var OUTPUT_VARIABLE GIT_COMMIT_ID)
    if(NOT ${res_var} EQUAL 0)
      file(READ ${VERS_FILE} GIT_COMMIT_ID LIMIT 255)
      if(NOT SC_IS_SUBBUILD)
	message(WARNING "Git failed (probably no tags in repo). Build will contain revision info from ${VERS_FILE}.")
      endif(NOT SC_IS_SUBBUILD)
    endif()
  else(GIT_FOUND)
    file(READ ${VERS_FILE} GIT_COMMIT_ID LIMIT 255)
    if(NOT SC_IS_SUBBUILD)
      message(WARNING "Git not found. Build will contain revision info from ${VERS_FILE}.")
    endif(NOT SC_IS_SUBBUILD)
  endif(GIT_FOUND)
else()
  file(READ ${VERS_FILE} GIT_COMMIT_ID LIMIT 255)
  if(NOT SC_IS_SUBBUILD)
    message(WARNING "Git failed ('.git' not found). Build will contain revision info from ${VERS_FILE}.")
  endif(NOT SC_IS_SUBBUILD)
endif()
string(REPLACE "\n" "" GIT_COMMIT_ID ${GIT_COMMIT_ID})

#-------------- date and time ---------------
#once cmake_minimum_required is >= 2.8.11, we can use TIMESTAMP:
#string(TIMESTAMP date_time_string)

if(UNIX)
  execute_process(COMMAND date "+%d %b %Y %H:%M" OUTPUT_VARIABLE date_time_string OUTPUT_STRIP_TRAILING_WHITESPACE)
elseif(WIN32)
  execute_process(COMMAND cmd /c date /t OUTPUT_VARIABLE currentDate OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND cmd /c time /t OUTPUT_VARIABLE currentTime OUTPUT_STRIP_TRAILING_WHITESPACE)
  set (date_time_string "${currentDate} ${currentTime}")
else()
  set(date_time_string "\" __DATE__ \" \" __TIME__ \" ")
  if(NOT SC_IS_SUBBUILD)
    message(STATUS "Unknown platform - using date from preprocessor")
  endif(NOT SC_IS_SUBBUILD)
endif()

set(header_string "/* sc_version_string.h - written by cmake. Changes will be lost! */\n"
  "#ifndef SC_VERSION_STRING\n"
  "#define SC_VERSION_STRING\n\n"
  "/*\n** The git commit id looks like \"test-1-g5e1fb47\", where test is the\n"
  "** name of the last tagged git revision, 1 is the number of commits since that tag,\n"
  "** 'g' is unknown, and 5e1fb47 is the first 7 chars of the git sha1 commit id.\n"
  "** timestamp is created from date/time commands on known platforms, and uses\n"
  "** preprocessor macros elsewhere.\n*/\n\n"
  "static char sc_version[512] = {\n"
  "    \"git commit id: ${GIT_COMMIT_ID}, build timestamp ${date_time_string}\"\n"
  "}\;\n\n"
  "#endif\n"
 )

#compare the new and old commit versions, don't update the file if only the timestamp differs
if(EXISTS ${SC_VERSION_HEADER})
  file(READ ${SC_VERSION_HEADER} OLD_VER_STRING LIMIT 600) #file is ~586 bytes
  string(FIND "${OLD_VER_STRING}" "git commit id: ${GIT_COMMIT_ID}" COMMIT_MATCH )
  # ${COMMIT_MATCH} == -1 if no match
else()
  set(COMMIT_MATCH -1)
endif(EXISTS ${SC_VERSION_HEADER})

if(${COMMIT_MATCH} LESS 1 )
  file(WRITE ${SC_VERSION_HEADER} ${header_string})
endif(${COMMIT_MATCH} LESS 1)

if(NOT SC_IS_SUBBUILD)
  message("-- sc_version_string.h is up-to-date.")
endif(NOT SC_IS_SUBBUILD)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

