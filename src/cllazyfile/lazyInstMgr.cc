#include <unordered_map>
#include "lazyTypes.h"
#include "lazyInstMgr.h"
#include "Registry.h"
#include "SdaiSchemaInit.h"

lazyInstMgr::lazyInstMgr() {
    _headerRegistry = new Registry( HeaderSchemaInit );
    _instanceTypes = new instanceTypes_t(255); //NOTE arbitrary max of 255 chars for a type name
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
#ifdef HAVE_JUDY
    _instancesLoaded.clear();
    _instanceStreamPos.clear();
#else //HAVE_JUDY
    int i = 0;
    instancesLoaded_t::iterator it = _instancesLoaded.begin();
    for( ; it != _instancesLoaded.end(); it++ ) {
        delete it->second;
        i++;
    }
#endif //HAVE_JUDY
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
#ifdef HAVE_JUDY
    _instanceTypes->insert( inst.name, inst.loc.instance );
    /* store 16 bits of section id and 48 of instance offset into one 64-bit int
    * TODO: check and warn if anything is lost (in calling code?)
    * does 32bit need anything special?
    */
    positionAndSection ps = inst.loc.section;
    ps <<= 48;
    ps |= ( inst.loc.begin & 0xFFFFFFFFFFFF );
    _instanceStreamPos.insert( inst.loc.instance, ps );

    if( inst.refs ) {
        if( inst.refs->size() > 0 ) {
            //forward refs
            _fwdInstanceRefs.insert( inst.loc.instance, *inst.refs );
            auto it = inst.refs->cbegin();
            for( ; it != inst.refs->cend(); it++ ) {
                //reverse refs
                _revInstanceRefs.insert( *it, inst.loc.instance );
            }
        } else {
            delete inst.refs;
        }
    }
#else // HAVE_JUDY
    _instanceTypes->insert( instanceTypes_pair( inst.name, inst.loc.instance ) );
    _instanceStreamPos.insert( instanceStreamPos_pair( inst.loc.instance, inst.loc ) );
    if( inst.refs->size() > 0 ) {
        //forward refs
        _fwdInstanceRefs.insert( instanceRefs_pair( inst.loc.instance, inst.refs ) );
        auto it = inst.refs->cbegin();
        for( ; it != inst.refs->cend(); it++ ) {
            //reverse refs
            if( _revInstanceRefs.find( *it ) == _revInstanceRefs.end() ) {
                _revInstanceRefs.insert( instanceRefs_pair( *it, new std::set< instanceID > ) );
            }
            _revInstanceRefs[ *it ]->insert( inst.loc.instance );
        }
    } else {
        delete inst.refs;
    }
#endif // HAVE_JUDY
}

unsigned long lazyInstMgr::getNumTypes() const {
    unsigned long n = 0 ;
#ifdef HAVE_JUDY
    instanceTypes_t::cpair curr, end;
    end = _instanceTypes->end();
    curr = _instanceTypes->begin();
    if( curr.value != 0 ) {
        n = 1;
        while( curr.value != end.value ) {
            n++;
            curr = _instanceTypes->next();
        }
    }
#else // HAVE_JUDY
    // http://www.daniweb.com/software-development/cpp/threads/384836/multimap-and-counting-number-of-keys#post1657899
    auto iter = _instanceTypes->cbegin();
    for( ; iter != _instanceTypes->cend(); iter = _instanceTypes->equal_range( iter->first ).second  ) {
        ++n;
    }
#endif // HAVE_JUDY
    return n ;
}

void lazyInstMgr::openFile( std::string fname ) {
    _files.push_back( new lazyFileReader( fname, this, _files.size() ) );
}
