#include "lazyP21DataSectionReader.h"
#include "lazyInstMgr.h"
#include <assert.h>

lazyP21DataSectionReader::lazyP21DataSectionReader( lazyFileReader * parent, std::ifstream & file, std::streampos start ):
                                                    lazyDataSectionReader( parent, file, start ) {
    findSectionStart();
    namedLazyInstance nl;
    while( nl = nextInstance(), ( ( nl.loc.begin > 0 ) && ( nl.name != 0 ) ) ) {
        parent->getInstMgr()->addLazyInstance( nl );
    }
    if( !_file.good() ) {
        _error = true;
        return;
    }
    if( nl.loc.instance == -1 ) {
        //check for ENDSEC;
        skipWS();
        std::streampos pos = _file.tellg();
        if( _file.get() == 'E' && _file.get() == 'N' && _file.get() == 'D'
           && _file.get() == 'S' && _file.get() == 'E' && _file.get() == 'C'
           && ( skipWS(), _file.get() == ';' ) ) {
            _sectionEnd = _file.tellg();
        } else {
            _file.seekg( pos );
            char found[26];
            _file.read( found, 25 );
            std::cerr << "expected 'ENDSEC;', found " << found << std::endl;
            _error = true;
        }
    }
}

// part of readdata1
const namedLazyInstance lazyP21DataSectionReader::nextInstance() {
    namedLazyInstance i;

    //TODO detect comments

    i.loc.begin = _file.tellg();
    i.loc.instance = readInstanceNumber();
    if( ( i.loc.instance < 0 ) || ( !_file.good() ) ) {
        _file.seekg( i.loc.begin );
        i.loc.begin = -1;
        return i;
    }
    skipWS();

    i.loc.section = _sectionID;
    skipWS();
    i.name = getDelimitedKeyword(";( /\\");
    if( _file.good() ) {
        seekInstanceEnd();
    }
    if( !_file.good() ) {
        //invalid instance, so clear everything
        i.loc.begin = -1;
        delete i.name;
        i.name = 0;
    }
    return i;
}