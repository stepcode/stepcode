#include <assert.h>

#include "p21HeaderSectionReader.h"


void p21HeaderSectionReader::findSectionStart() {
    _sectionStart = findNormalString( "HEADER", true );
    assert( _file.is_open() && _file.good() );
}

p21HeaderSectionReader::p21HeaderSectionReader( lazyFileReader * parent, std::ifstream & file, std::streampos start ):
                                                    headerSectionReader( parent, file, start ) {
    findSectionStart();
    findSectionEnd();
    _file.seekg( _sectionStart );
    namedLazyInstance nl;
    std::cerr << "lazy instance size: " << sizeof( nl.loc ) << std::endl;
    while( nl = nextInstance(), ( nl.loc.begin > 0 ) ) {
        _headerInstances.insert( instancesLoaded_pair( nl.loc.instance, getRealInstance( &nl.loc ) ) );
    }
    _file.seekg( _sectionEnd );
}

// part of readdata1
const namedLazyInstance p21HeaderSectionReader::nextInstance() {
    namedLazyInstance i;
    static instanceID nextFreeInstance = 4; // 1-3 are reserved

    i.loc.begin = _file.tellg();
    i.loc.section = _sectionID;
    skipWS();
    if( i.loc.begin <= 0 ) {
        i.name = 0;
    } else {
        i.name = getDelimitedKeyword(";( /\\");

        if( 0 == i.name->compare( "FILE_DESCRIPTION" ) ) {
            i.loc.instance = 1;
        } else if( 0 == i.name->compare( "FILE_NAME" ) ) {
            i.loc.instance = 2;
        } else if( 0 == i.name->compare( "FILE_SCHEMA" ) ) {
            i.loc.instance = 3;
        } else {
            i.loc.instance = nextFreeInstance++;
        }

        assert( i.name->length() > 0 );

        std::streampos end = seekInstanceEnd();
        if( (end == -1 ) || ( end >= _sectionEnd ) ) {
            //invalid instance, so clear everything
            i.loc.begin = -1;
//             delete i.name;
            i.name = 0;
        }
    }
    return i;
}