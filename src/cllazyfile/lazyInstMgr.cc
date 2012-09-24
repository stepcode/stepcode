#include "lazyInstMgr.h"
#include "Registry.h"
#include "SdaiSchemaInit.h"

lazyInstMgr::lazyInstMgr() {
    _headerRegistry = new Registry( HeaderSchemaInit );
}

sectionID lazyInstMgr::registerDataSection( lazyDataSectionReader * sreader ) {
    _dataSections.push_back( sreader );
    return _dataSections.size() - 1;
}

fileID lazyInstMgr::registerLazyFile( lazyFileReader * freader ) {
    _files.push_back( freader );
    return _files.size() - 1;
}

void lazyInstMgr::addLazyInstance( namedLazyInstance inst ) {
    _instanceStreamPosMMap.insert( instanceStreamPosMMap_pair( inst.loc.instance, inst.loc ) );
    _instanceTypeMMap.insert( instanceTypeMMap_pair( *inst.name, inst.loc.instance ) );
    delete inst.name;
}
