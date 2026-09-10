#include <assert.h>
#include <set>
#include "cllazyfile/lazyP21DataSectionReader.h"
#include "cllazyfile/lazyInstMgr.h"

void lazyP21DataSectionReader::findSectionStart() {
    std::streampos keywordEnd = findNormalString( "DATA" );
    if( keywordEnd == std::streampos( -1 ) ) {
        _sectionStart = -1;
        return;
    }
    _file.seekg( keywordEnd );
    skipWS();
    int depth = 0;
    int current = _file.get();
    if( current == ';' ) {
        _sectionStart = _file.tellg();
        return;
    }
    if( current != '(' ) {
        _sectionStart = -1;
        return;
    }
    depth = 1;
    while( depth && _file.good() ) {
        current = _file.get();
        if( current == '\'' ) {
            _file.seekg( _file.tellg() - std::streampos( 1 ) );
            GetLiteralStr( _file, _lazyFile->getInstMgr()->getErrorDesc() );
        } else if( current == '/' && _file.peek() == '*' ) {
            findNormalString( "*/" );
        } else if( current == '(' ) {
            ++depth;
        } else if( current == ')' ) {
            --depth;
        }
    }
    skipWS();
    if( depth == 0 && _file.get() == ';' ) {
        _sectionStart = _file.tellg();
    } else {
        _sectionStart = -1;
    }
}

lazyP21DataSectionReader::lazyP21DataSectionReader( lazyFileReader * parent, std::ifstream & file,
        std::streampos start, sectionID sid ):
    lazyDataSectionReader( parent, file, start, sid ) {
    findSectionStart();
    if( _sectionStart == std::streampos( -1 ) ) {
        _error = true;
        return;
    }
    namedLazyInstance nl;
    while( nl = nextInstance(), ( ( nl.loc.begin > 0 ) && ( nl.name != 0 ) ) ) {
        parent->getInstMgr()->addLazyInstance( nl );
        parent->getInstMgr()->observeScan( parent->ID(),
            static_cast<lazyFileOffset>( static_cast<std::streamoff>( _file.tellg() ) ), parent->fileSize() );
        if( parent->getInstMgr()->cancelled() ) return;
    }

    if(  sectionReader::_error->severity() <= SEVERITY_WARNING ) {
        sectionReader::_error->PrintContents( std::cerr );
        if(  sectionReader::_error->severity() <= SEVERITY_INPUT_ERROR ) {
            _error = true;
            return;        
        }
    }
        
    if( !_file.good() ) {
        _error = true;
        return;
    }
    if( nl.loc.instance == 0 ) {
        //check for ENDSEC;
        skipWS();
        std::streampos pos = _file.tellg();
        if( _file.get() == 'E' && _file.get() == 'N' && _file.get() == 'D'
                && _file.get() == 'S' && _file.get() == 'E' && _file.get() == 'C'
                && ( skipWS(), _file.get() == ';' ) ) {
            _sectionEnd = _file.tellg();
        } else {
            _file.seekg( pos );
            char found[26] = { '\0' };
            _file.read( found, 25 );
            std::cerr << "expected 'ENDSEC;', found " << found << std::endl;
            _error = true;
        }
    }
}

// part of readdata1
//if this changes, probably need to change sectionReader::getType()
const namedLazyInstance lazyP21DataSectionReader::nextInstance() {
    std::streampos end = -1;
    namedLazyInstance i;

    i.name = 0;
    i.refs = 0;
    i.componentTypes = 0;
    i.loc.section = 0;
    i.loc.end = 0;
    std::streampos start = _file.tellg();
    i.loc.begin = start == std::streampos( -1 ) ? 0 :
        static_cast<lazyFileOffset>( static_cast<std::streamoff>( start ) );
    i.loc.instance = readInstanceNumber();
    if( ( _file.good() ) && ( i.loc.instance > 0 ) ) {
        if( !skipTokenSeparators() ) {
            _file.setstate( std::ios::failbit );
        }
        if( _file.good() && _file.peek() == '&' && !indexScope() ) {
            _file.setstate( std::ios::failbit );
        }
        if( _file.good() && !skipTokenSeparators() ) {
            _file.setstate( std::ios::failbit );
        }
        i.loc.section = _sectionID;
        if( _file.good() ) i.name = getDelimitedKeyword( ";( /\\" );
        if( _file.good() ) {
            if( i.name[0] == '\0' ) i.componentTypes = new std::vector<std::string>;
            end = seekInstanceEnd( & i.refs, i.componentTypes );
            if( end != std::streampos( -1 ) ) {
                i.loc.end = static_cast<lazyFileOffset>( static_cast<std::streamoff>( end ) );
            }
        }
    }
    if( ( i.loc.instance == 0 ) || ( !_file.good() ) || ( end == ( std::streampos ) - 1 ) ) {
        //invalid instance, so clear everything
        _file.seekg( i.loc.begin );
        i.loc.begin = 0;
        if( i.refs ) {
            delete i.refs;
            i.refs = 0;
        }
        if( i.componentTypes ) {
            delete i.componentTypes;
            i.componentTypes = 0;
        }
        i.name = 0;
    }
    return i;
}
bool lazyP21DataSectionReader::indexScope() {
    if( !skipTokenSeparators() || _file.get() != '&' ) return false;
    const std::string scopeKeyword = getDelimitedKeyword( "#/(;\\" );
    if( scopeKeyword != "SCOPE" ) return false;

    bool haveInstance = false;
    while( skipTokenSeparators() && _file.peek() == '#' ) {
        namedLazyInstance nested = nextInstance();
        if( nested.loc.begin == 0 || !nested.name ) return false;
        _lazyFile->getInstMgr()->addLazyInstance( nested );
        haveInstance = true;
    }
    if( !haveInstance || !skipTokenSeparators() ) return false;

    const std::string endKeyword = getDelimitedKeyword( "/#(;\\" );
    if( endKeyword != "ENDSCOPE" ) return false;
    return skipScopeExportList();
}

