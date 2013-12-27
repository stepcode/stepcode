cmake_minimum_required( VERSION 2.8 )
project( test_exppp )

set( breakLongStr_SRCS
     test_breakLongStr.c
     ../exppp.c
     )

# this executable doesn't really check the results, just ensures no segfaults. ought to improve it...
SC_ADDEXEC( tst_breakLongStr "${breakLongStr_SRCS}" "express;base" "TESTABLE" )
add_test( NAME build_tst_breakLongStr
              WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
              COMMAND ${CMAKE_COMMAND} --build .
                                       --target tst_breakLongStr
                                       --config $<CONFIGURATION> )
add_test( test_breakLongStr ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tst_breakLongStr )
set_tests_properties( test_breakLongStr PROPERTIES DEPENDS build_tst_breakLongStr )

#could run test schemas through exppp, then check line length...
