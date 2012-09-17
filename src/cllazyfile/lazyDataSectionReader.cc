#include "lazyDataSectionReader.h"
#include "lazyFileReader.h"
#include "lazyInstMgr.h"

lazyDataSectionReader::lazyDataSectionReader( lazyFileReader * parent, std::ifstream * file, std::streampos start ): sectionReader( parent, file, start ) {
    _sectionID = _lazyFile->getInstMgr()->getSectionID(this);
}
