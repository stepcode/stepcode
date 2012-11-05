#include <unordered_map>
#include "lazyTypes.h"
#include "lazyInstMgr.h"
#include "Registry.h"
#include "SdaiSchemaInit.h"

lazyInstMgr::lazyInstMgr() {
    _headerRegistry = new Registry( HeaderSchemaInit );
    _lazyInstanceCount = 0;
    _longestTypeNameLen = 0;
    _errors = new ErrorDescriptor();
}

lazyInstMgr::~lazyInstMgr() {
    delete _headerRegistry;
    delete _errors;
    //loop over files, sections, instances; delete header instances
    lazyFileReaderVec_t::iterator fit = _files.begin();
    for( ; fit != _files.end(); fit++ ) {
        delete *fit;
    }
    dataSectionReaderVec_t::iterator sit = _dataSections.begin();
    for( ; sit != _dataSections.end(); sit++ ) {
        delete *sit;
    }
    int i = 0;
    instancesLoaded_t::iterator it = _instancesLoaded.begin();
    for( ; it != _instancesLoaded.end(); it++ ) {
        delete it->second;
        i++;
    }
}

sectionID lazyInstMgr::registerDataSection( lazyDataSectionReader * sreader ) {
    _dataSections.push_back( sreader );
    return _dataSections.size() - 1;
}

void lazyInstMgr::addLazyInstance( namedLazyInstance inst ) {
    _lazyInstanceCount++;
    assert( inst.loc.begin > 0 && inst.loc.instance >= 0 && inst.loc.section >= 0 );
    int len = strlen( inst.name );
    if( len > _longestTypeNameLen ) {
        _longestTypeNameLen = len;
        _longestTypeName = inst.name;
    }
    _instanceStreamPosMMap.insert( instanceStreamPosMMap_pair( inst.loc.instance, inst.loc ) );
    _instanceTypeMMap.insert( instanceTypeMMap_pair( inst.name, inst.loc.instance ) );

    if( inst.refs->size() > 0 ) {
        //forward refs
        _fwdInstanceRefsMap.insert( instanceRefMap_pair( inst.loc.instance, inst.refs ) );
        auto it = inst.refs->cbegin();
        for( ; it != inst.refs->cend(); it++ ) {
            //reverse refs
            if( _revInstanceRefsMap.find( *it ) == _revInstanceRefsMap.end() ) {
                _revInstanceRefsMap.insert( instanceRefMap_pair( *it, new std::set< instanceID > ) );
            }
            _revInstanceRefsMap[ *it ]->insert( inst.loc.instance );
        }
    } else {
        delete inst.refs;
    }
}

unsigned long lazyInstMgr::getNumTypes() const {
    // http://www.daniweb.com/software-development/cpp/threads/384836/multimap-and-counting-number-of-keys#post1657899
    unsigned long n = 0 ;
    auto iter = _instanceTypeMMap.cbegin();
    for( ; iter != _instanceTypeMMap.cend(); iter = _instanceTypeMMap.equal_range( iter->first ).second  ) {
        ++n;
    }
    return n ;
}

void lazyInstMgr::openFile( std::string fname ) {
    _files.push_back( new lazyFileReader( fname, this, _files.size() ) );
}
