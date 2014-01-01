
#include "sc_mkdir.h"

#ifdef _WIN32
#  include <direct.h>
#else
#  include <sys/stat.h>
#  include <sys/types.h>
#endif /* _WIN32 */


int sc_mkdir( const char * path ) {
    #ifdef _WIN32
    return mkdir( path );
    #else
    return mkdir( path, 0777 );
    #endif /* _WIN32 */
}
