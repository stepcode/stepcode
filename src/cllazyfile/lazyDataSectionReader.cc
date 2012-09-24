#include "lazyDataSectionReader.h"
#include "lazyFileReader.h"
#include "lazyInstMgr.h"
#include <iostream>

lazyDataSectionReader::lazyDataSectionReader( lazyFileReader * parent, std::ifstream & file, std::streampos start ): sectionReader( parent, file, start ) {
    _sectionID = _lazyFile->getInstMgr()->registerDataSection( this );
    _sectionIdentifier = "";
    std::cerr << "FIXME set _sectionIdentifier" << std::endl;
    error = true;     std::cerr << "FIXME read file, set `error` correctly, ..." << std::endl;

}
