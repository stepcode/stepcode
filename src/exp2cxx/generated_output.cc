#include "generated_output.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#  include <direct.h>
#  include <process.h>
#  include <windows.h>
#  define sc_getpid _getpid
#  define sc_rmdir _rmdir
#else
#  include <unistd.h>
#  define sc_getpid getpid
#  define sc_rmdir rmdir
#endif

namespace {

const char manifestName[] = ".exp2cxx-manifest";
const char manifestHeader[] = "exp2cxx generated output manifest v1";

struct OutputRecord {
    std::string temporary;
};

class OutputManager {
    std::map<std::string, OutputRecord> _outputs;
    std::map<FILE *, std::string> _openFiles;
    unsigned long _sequence;
    bool _active;
    bool _failed;

    static bool safePath( const std::string & path ) {
        if( path.empty() || path[0] == '/' || path[0] == '\\' ||
                path.find( ':' ) != std::string::npos ) {
            return false;
        }
        size_t start = 0;
        while( start <= path.size() ) {
            const size_t end = path.find_first_of( "/\\", start );
            const std::string part = path.substr(
                start, end == std::string::npos ? end : end - start );
            if( part.empty() || part == "." || part == ".." ) {
                return false;
            }
            if( end == std::string::npos ) {
                break;
            }
            start = end + 1;
        }
        return true;
    }

    std::string temporaryName( const std::string & target ) {
        std::ostringstream name;
        name << target << ".exp2cxx-tmp-" << sc_getpid() << '-'
             << ++_sequence;
        return name.str();
    }

    static bool copyFile( const std::string & source,
                          const std::string & destination ) {
        std::ifstream input( source.c_str(), std::ios::binary );
        std::ofstream output( destination.c_str(), std::ios::binary );
        if( !output ) {
            return false;
        }
        if( input ) {
            output << input.rdbuf();
        }
        return output.good();
    }

    static bool sameContents( const std::string & first,
                              const std::string & second ) {
        std::ifstream left( first.c_str(), std::ios::binary );
        std::ifstream right( second.c_str(), std::ios::binary );
        if( !left || !right ) {
            return false;
        }
        char leftBuffer[65536];
        char rightBuffer[65536];
        do {
            left.read( leftBuffer, sizeof( leftBuffer ) );
            right.read( rightBuffer, sizeof( rightBuffer ) );
            const std::streamsize leftCount = left.gcount();
            const std::streamsize rightCount = right.gcount();
            if( leftCount != rightCount ||
                    memcmp( leftBuffer, rightBuffer,
                            static_cast<size_t>( leftCount ) ) ) {
                return false;
            }
        } while( left );
        return right.peek() == std::ifstream::traits_type::eof();
    }

    static bool replaceFile( const std::string & temporary,
                             const std::string & target ) {
#ifdef _WIN32
        return MoveFileExA( temporary.c_str(), target.c_str(),
                            MOVEFILE_REPLACE_EXISTING |
                            MOVEFILE_WRITE_THROUGH ) != 0;
#else
        return rename( temporary.c_str(), target.c_str() ) == 0;
#endif
    }

    static std::set<std::string> readManifest() {
        std::set<std::string> paths;
        std::ifstream input( manifestName );
        std::string path;
        if( !std::getline( input, path ) || path != manifestHeader ) {
            return paths;
        }
        while( std::getline( input, path ) ) {
            if( safePath( path ) ) {
                paths.insert( path );
            }
        }
        return paths;
    }

    static bool writeStableManifest( const std::string & contents ) {
        std::ostringstream temporary;
        temporary << manifestName << ".exp2cxx-tmp-" << sc_getpid();
        const std::string temporaryName = temporary.str();
        {
            std::ofstream output( temporaryName.c_str(), std::ios::binary );
            output.write( contents.data(),
                          static_cast<std::streamsize>( contents.size() ) );
            if( !output.good() ) {
                remove( temporaryName.c_str() );
                return false;
            }
        }
        if( sameContents( temporaryName, manifestName ) ) {
            remove( temporaryName.c_str() );
            return true;
        }
        if( !replaceFile( temporaryName, manifestName ) ) {
            remove( temporaryName.c_str() );
            return false;
        }
        return true;
    }

    static void recordParentDirectories(
        const std::string & path, std::set<std::string> & directories ) {
        size_t slash = path.find_last_of( "/\\" );
        while( slash != std::string::npos ) {
            const std::string parent = path.substr( 0, slash );
            if( safePath( parent ) ) {
                directories.insert( parent );
            }
            slash = parent.find_last_of( "/\\" );
        }
    }

public:
    OutputManager() : _sequence( 0 ), _active( false ), _failed( false ) {
    }

    ~OutputManager() {
        for( std::map<std::string, OutputRecord>::const_iterator i =
                _outputs.begin(); i != _outputs.end(); ++i ) {
            remove( i->second.temporary.c_str() );
        }
    }

    void Begin() {
        for( std::map<std::string, OutputRecord>::const_iterator i =
                _outputs.begin(); i != _outputs.end(); ++i ) {
            remove( i->second.temporary.c_str() );
        }
        _outputs.clear();
        _openFiles.clear();
        _sequence = 0;
        _active = true;
        _failed = false;
    }

    FILE * Open( const char * filename, bool append ) {
        const std::string target = filename ? filename : "";
        if( !_active || !safePath( target ) ) {
            errno = EINVAL;
            _failed = true;
            return 0;
        }
        std::map<std::string, OutputRecord>::iterator found =
            _outputs.find( target );
        if( append ) {
            if( found == _outputs.end() ) {
                OutputRecord record;
                record.temporary = temporaryName( target );
                if( !copyFile( target, record.temporary ) ) {
                    _failed = true;
                    return 0;
                }
                found = _outputs.insert(
                    std::make_pair( target, record ) ).first;
            }
        } else {
            if( found != _outputs.end() ) {
                remove( found->second.temporary.c_str() );
                _outputs.erase( found );
            }
            OutputRecord record;
            record.temporary = temporaryName( target );
            found = _outputs.insert( std::make_pair( target, record ) ).first;
        }
        FILE * output = fopen( found->second.temporary.c_str(),
                              append ? "a" : "w" );
        if( !output ) {
            _failed = true;
            if( !append ) {
                _outputs.erase( found );
            }
            return 0;
        }
        _openFiles[output] = target;
        return output;
    }

    void Close( FILE * output ) {
        if( !output ) {
            return;
        }
        _openFiles.erase( output );
        const int flushResult = fflush( output );
        const int streamError = ferror( output );
        const int closeResult = fclose( output );
        if( flushResult || streamError || closeResult ) {
            _failed = true;
        }
    }

    int Write( const char * filename, const char * data, size_t size ) {
        FILE * output = Open( filename, false );
        if( !output ) {
            return 0;
        }
        const bool written = fwrite( data, 1, size, output ) == size;
        if( !written ) {
            _failed = true;
        }
        Close( output );
        return written ? 1 : 0;
    }

    int Finish() {
        if( !_active || _failed || !_openFiles.empty() ) {
            return 0;
        }
        const std::set<std::string> previous = readManifest();
        std::ostringstream manifest;
        manifest << manifestHeader << '\n';
        for( std::map<std::string, OutputRecord>::iterator i =
                _outputs.begin(); i != _outputs.end(); ++i ) {
            manifest << i->first << '\n';
            if( sameContents( i->second.temporary, i->first ) ) {
                remove( i->second.temporary.c_str() );
            } else if( !replaceFile( i->second.temporary, i->first ) ) {
                return 0;
            }
            i->second.temporary.clear();
        }
        std::set<std::string> directories;
        for( std::set<std::string>::const_iterator i = previous.begin();
                i != previous.end(); ++i ) {
            if( _outputs.find( *i ) == _outputs.end() ) {
                if( remove( i->c_str() ) != 0 && errno != ENOENT ) {
                    return 0;
                }
                recordParentDirectories( *i, directories );
            }
        }
        std::vector<std::string> orderedDirectories(
            directories.begin(), directories.end() );
        std::sort( orderedDirectories.begin(), orderedDirectories.end(),
            []( const std::string & left, const std::string & right ) {
                return left.size() > right.size();
            } );
        for( size_t i = 0; i < orderedDirectories.size(); ++i ) {
            sc_rmdir( orderedDirectories[i].c_str() );
        }
        if( !writeStableManifest( manifest.str() ) ) {
            return 0;
        }
        _outputs.clear();
        _active = false;
        return 1;
    }
};

OutputManager & manager() {
    static OutputManager outputManager;
    return outputManager;
}

}

extern "C" void GENERATEDbegin( void ) {
    manager().Begin();
}

extern "C" FILE * GENERATEDopen( const char * filename ) {
    return manager().Open( filename, false );
}

extern "C" FILE * GENERATEDappend( const char * filename ) {
    return manager().Open( filename, true );
}

extern "C" void GENERATEDclose( FILE * file ) {
    manager().Close( file );
}

extern "C" int GENERATEDwrite(
    const char * filename, const char * data, size_t size ) {
    return manager().Write( filename, data, size );
}

extern "C" int GENERATEDfinish( void ) {
    return manager().Finish();
}
