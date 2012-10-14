#include "lazyInstMgr.h"
#include "Registry.h"
#include "SdaiSchemaInit.h"

lazyInstMgr::lazyInstMgr() {
    _headerRegistry = new Registry( HeaderSchemaInit );
    _lazyInstanceCount = 0;
    _longestTypeNameLen = 0;
    _errors = new ErrorDescriptor();
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
    _lazyInstanceCount++;
    if( inst.name->length() > _longestTypeNameLen ) {
        _longestTypeNameLen = inst.name->length();
        _longestTypeName = *inst.name;
    }
    _instanceStreamPosMMap.insert( instanceStreamPosMMap_pair( inst.loc.instance, inst.loc ) );
    _instanceTypeMMap.insert( instanceTypeMMap_pair( *inst.name, inst.loc.instance ) );
    delete inst.name;
}

unsigned long lazyInstMgr::getNumTypes() /*const*/ {
    // http://www.daniweb.com/software-development/cpp/threads/384836/multimap-and-counting-number-of-keys#post1657899
    unsigned long n = 0 ;
    instanceTypeMMap_t::iterator iter;//( _instanceTypeMMap.begin() );
    for( iter=_instanceTypeMMap.begin(); iter != _instanceTypeMMap.end(); iter = _instanceTypeMMap.equal_range( iter->first ).second  ) {
        ++n;
    }
    return n ;
}

void lazyInstMgr::openFile( std::string fname ) {
    //  lazyFileReader adds itself to the file list - good idea or bad?
    /*_files.push_back( */new lazyFileReader( fname, this ) /*)*/;
}
