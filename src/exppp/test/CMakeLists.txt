project( test_exppp )

set( breakLongStr_SRCS
     test_breakLongStr.c
     ../exppp.c
     )

# this executable doesn't really check the results, just ensures no segfaults. ought to improve it...
SC_ADDEXEC( test_breakLongStr "${breakLongStr_SRCS}" "express;base" )

#could run test schemas through exppp, then check line length...
