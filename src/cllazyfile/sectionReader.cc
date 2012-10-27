#define SCL_CORE_EXPORT
#define SCL_DAI_EXPORT

#include <algorithm>
#include <set>
#include <assert.h>

#include "Registry.h"
#include "sdaiApplication_instance.h"
#include "read_func.h"

#include "sectionReader.h"
#include "lazyFileReader.h"
#include "lazyInstMgr.h"
#include "lazyTypes.h"

sectionReader::sectionReader( lazyFileReader * parent, std::ifstream & file, std::streampos start, sectionID sid ):
                                _lazyFile( parent ), _file( file ), _sectionStart( start ), _sectionID( sid ) {
    _fileID = _lazyFile->ID();
}


std::streampos sectionReader::findNormalString( const std::string& str, bool semicolon ) {
    std::streampos found = -1, startPos = _file.tellg(), nextTry = startPos;
    int i = 0, l = str.length();
    char c;

    //i is reset every time a character doesn't match; if i == l, this means that we've found the entire string
    while( i < l || semicolon ) {
        skipWS();
        c = _file.get();
        if( ( i == l ) && ( semicolon ) ) {
            if( c == ';' ) {
                break;
            } else {
                i = 0;
                _file.seekg( nextTry );
                continue;
            }
        }
        if(  c == '\'' ) {
            //push past string
            _file.unget();
            ToExpressStr( _file, _lazyFile->getInstMgr()->getErrorDesc() );
        }
        if( ( c == '/' ) && ( _file.peek() == '*' ) ) {
            //push past comment
            findNormalString( "*/" );
        }
        if( str[i] == c ) {
            i++;
            if( i == 1 ) {
                nextTry = _file.tellg();
            }
        } else {
            if( !_file.good() ) {
                break;
            }
            if( i >= 1 ) {
                _file.seekg( nextTry );
            }
            i = 0;
        }
    }
    if( i == l ) {
            found = _file.tellg();
    }
    if( _file.is_open() && _file.good() ) {
        return found;
    } else {
        return -1;
    }
}


//NOTE different behavior than const char * GetKeyword( istream & in, const char * delims, ErrorDescriptor & err ) in read_func.cc
const char * sectionReader::getDelimitedKeyword( const char * delimiters ) {
    static std::string str;
    char c;
    str.assign( 0, 0 ); //clear() frees the memory
    str.reserve(100);
    skipWS();
    while( c = _file.get(), _file.good() ) {
        if( c == '-' || c == '_' || isupper( c ) || isdigit( c ) ||
            ( c == '!' && str.length() == 0 ) ) {
            str.append( 1, c );
        } else if( ( c == '/' ) && ( _file.peek() == '*' ) && ( str.length() == 0 ) ) {
            //push past comment
            findNormalString( "*/" );
            skipWS();
            continue;
        } else {
            _file.putback( c );
            break;
        }
    }
    c = _file.peek();
    if( !strchr( delimiters, c ) ) {
        std::cerr << __PRETTY_FUNCTION__ << ": missing delimiter. Found " << c << ", expected one of " << delimiters << " at end of keyword " << str << ". File offset: " << _file.tellg() << std::endl;
        abort();
    }
    return str.c_str();
}

/// search forward in the file for the end of the instance. Start position should
/// be the opening parenthesis; otherwise, it is likely to fail.
///NOTE *must* check return value!
std::streampos sectionReader::seekInstanceEnd( std::set< instanceID > * refs ) {
    char c;
    int parenDepth = 0;
    while( c = _file.get(), _file.good() ) {
        switch( c ) {
            case '(':
                parenDepth++;
                break;
            case '/':
                if( _file.peek() == '*' ) {
                    findNormalString( "*/" );
                } else {
                    return -1;
                }
                break;
            case '\'':
                _file.unget();
                ToExpressStr( _file, _lazyFile->getInstMgr()->getErrorDesc() );
                break;
            case '=':
                return -1;
            case '#':
                skipWS();
                if( isdigit( _file.peek() ) ) {
                    if( refs != 0 ) {
                        instanceID n;
                        _file >> n;
                        refs->insert( n );
                    }
                } else {
                    return -1;
                }
                break;
            case ')':
                if( --parenDepth == 0 ) {
                    skipWS();
                    if( _file.get() == ';' ) {
                        return _file.tellg();
                    } else {
                        _file.unget();
                    }
                }
            default:
                break;
        }
    }
    return -1;
    //NOTE - old way: return findNormalString( ")", true );
    // old memory consumption: 673728kb; User CPU time: 35480ms; System CPU time: 17710ms (with 266MB catia-ferrari-sharknose.stp)
    // new memory: 673340kb; User CPU time: 29890ms; System CPU time: 11650ms
}

void sectionReader::locateAllInstances() {
    namedLazyInstance inst;
    while( inst = nextInstance(), ( _file.good() ) && ( inst.loc.begin > 0 ) ) {
        _lazyFile->getInstMgr()->addLazyInstance( inst );
    }
}

instanceID sectionReader::readInstanceNumber() {
    std::streampos start, end;
    char c;
    int digits = 0;
    instanceID id = -1;

    //find instance number ("# nnnn ="), where ' ' is any whitespace found by isspace()
    skipWS();
    c = _file.get();
    if( ( c == '/' ) && ( _file.peek() == '*' ) ) {
        findNormalString( "*/" );
    } else {
        _file.unget();
    }
    skipWS();
    c = _file.get();
    if( c != '#' ) {
        return -1;
    }
    skipWS();
    start = _file.tellg();
    do {
        c = _file.get();
        if( isdigit( c ) ) {
            digits++;
        } else {
            _file.unget();
            break;
        }
    } while( _file.good() );
    skipWS();

    if( _file.good() && ( digits > 0 ) && ( _file.get() == '=' ) ) {
        end = _file.tellg();
        _file.seekg( start );
        _file >> id;
        _file.seekg ( end );
    }
    return id;
}

//TODO: most of the rest of readdata1, all of readdata2
SDAI_Application_instance * sectionReader::getRealInstance( const Registry * reg, const lazyInstanceLoc* lazy, const std::string & typeName, const std::string & schName, bool header ) {
    char c;
    std::string comment;
    Severity sev;
    SDAI_Application_instance * inst = 0;

    _file.seekg( lazy->begin );
    skipWS();
    ReadTokenSeparator( _file, &comment );
    if( !header ) {
        findNormalString( "=" );
    }
    skipWS();
    c = _file.get();
    switch( c ) {
        case '&':
            std::cerr << "Can't handle scope instances. Skipping #" << lazy->instance << std::endl;
            // sev = CreateScopeInstances( in, &scopelist );
            break;
        case '(':
            std::cerr << "Can't handle complex instances. Skipping #" << lazy->instance << std::endl;
            //CreateSubSuperInstance( in, fileid, result );
            break;
        case '!':
            std::cerr << "Can't handle user-defined instances. Skipping #" << lazy->instance << std::endl;
        default:
            inst = reg->ObjCreate( typeName.c_str(), schName.c_str() );
            break;
    }
    inst->StepFileId( lazy->instance );
    if( !comment.empty() ) {
        inst->AddP21Comment( comment );
    }
    findNormalString( "(" );
    _file.unget();
    sev = inst->STEPread( lazy->instance, 0, /*&instances()*/ 0, _file, schName.c_str(), true, false );
    return inst;
}

