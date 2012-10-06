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
    while( nl = nextInstance(), ( nl.loc.end != nl.loc.begin ) ) {
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
    i.loc.file = _fileID;
    _file >> std::ws;
    i.name = getDelimitedKeyword(";( /\\");

    //TODO figure out the instance number
    if( 0 == i.name->compare( "FILE_DESCRIPTION" ) ) {
        i.loc.instance = 1;
    } else if( 0 == i.name->compare( "FILE_NAME" ) ) {
        i.loc.instance = 2;
    } else if( 0 == i.name->compare( "FILE_SCHEMA" ) ) {
        i.loc.instance = 3;
    } else {
        i.loc.instance = nextFreeInstance++;
    }

    i.loc.end = seekInstanceEnd();

    if( i.loc.end >= _sectionEnd ) {
        //invalid instance, so clear everything
        i.loc.end = i.loc.begin;
        delete i.name;
        i.name = 0;
    }
    return i;
}