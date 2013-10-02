#ifndef SC_STDBOOL_H
#define SC_STDBOOL_H

#ifdef _WIN32
    typedef int bool;
#   define false 0
#   define true 1
#else
#   include <stdbool.h>
#endif

#endif /* SC_STDBOOL_H */