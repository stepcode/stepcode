#include <assert.h>

#include "p21HeaderSectionReader.h"


void p21HeaderSectionReader::findSectionStart() {
    _sectionStart = findString( "HEADER", true );
    assert( _file.is_open() && _file.good() );
}

p21HeaderSectionReader::p21HeaderSectionReader( lazyFileReader * parent, std::ifstream & file, std::streampos start ):
                                                    headerSectionReader( parent, file, start ) {
    findSectionStart();
    findSectionEnd();
    _file.seekg( _sectionStart );
    namedLazyInstance nl;
    while( nl = nextInstance( true ), ( nl.loc.end != nl.loc.begin ) ) {
        _headerInstances.insert( instancesLoaded_pair( nl.loc.instance, getRealInstance( &nl.loc ) ) );
    }
//     _headerInstances.insert();
//     SDAI_Application_instance * getRealInstance( lazyInstanceLoc * inst );
//     inst = nextInstance( true );
    std::cerr << "p21hsr ctor" << std::endl;
}
