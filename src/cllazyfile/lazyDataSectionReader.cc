#include "cllazyfile/lazyDataSectionReader.h"
#include "cllazyfile/lazyFileReader.h"
#include "cllazyfile/lazyInstMgr.h"
#include <iostream>

lazyDataSectionReader::lazyDataSectionReader( lazyFileReader * parent, std::ifstream & file,
        std::streampos start, sectionID sid ):
    sectionReader( parent, file, start, sid ) {
    _sectionIdentifier = ""; //FIXME retain the data section identifier (2002 revision of Part 21)
    _error = false;
    _completelyLoaded = false;
}
