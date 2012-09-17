#include "lazyInstMgr.h"
#include "Registry.h"
#include "SdaiSchemaInit.h"

lazyInstMgr::lazyInstMgr() {
    //TODO init header registry
    _headerRegistry = new Registry( SdaiHEADER_SECTION_SCHEMAInit );
}

sectionID lazyInstMgr::getSectionID( sectionReader * sreader ) {
    sectionID i = _dataSections.size() - 1;
    while( i >= 0 ) {
        if( _dataSections[i] == sreader ) {
            return i;
        }
        i--;
    }
    abort();
    // return -1;
}

fileID lazyInstMgr::getFileID( lazyFileReader * freader ) {
    sectionID i = _files.size() - 1;
    while( i >= 0 ) {
        if( _files[i] == freader ) {
            return i;
        }
        i--;
    }
    abort();
    // return -1;
}
