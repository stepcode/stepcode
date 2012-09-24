#include <assert.h>

#include "p21HeaderSectionReader.h"


void p21HeaderSectionReader::findSectionStart() {
    findString( "HEADER", true );
    assert( _file.is_open() && _file.good() );
}

p21HeaderSectionReader::p21HeaderSectionReader( lazyFileReader * parent, std::ifstream & file, std::streampos start ):
                                                    headerSectionReader( parent, file, start ) {
    findSectionStart();
    findSectionEnd();
    _file.seekg( _sectionStart );
    nextInstance( true );
    std::cerr << "p21hsr ctor" << std::endl;
}
