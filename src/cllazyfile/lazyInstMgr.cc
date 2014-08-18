#include "lazyTypes.h"
#include "lazyInstMgr.h"
#include "Registry.h"
#include "SdaiSchemaInit.h"
#include "instMgrHelper.h"

lazyInstMgr::lazyInstMgr() {
    _headerRegistry = new Registry( HeaderSchemaInit );
    _mainRegistry = 0;
    _instanceTypes = new instanceTypes_t( 255 ); //NOTE arbitrary max of 255 chars for a type name
    _lazyInstanceCount = 0;
    _loadedInstanceCount = 0;
    _longestTypeNameLen = 0;
    _errors = new ErrorDescriptor();
    _ima = new instMgrAdapter( this );
}

lazyInstMgr::~lazyInstMgr() {
    delete _headerRegistry;
    delete _errors;
    delete _ima;
    //loop over files, sections, instances; delete header instances
    lazyFileReaderVec_t::iterator fit = _files.begin();
    for( ; fit != _files.end(); ++fit ) {
        if( *fit ) {
            delete *fit;
        }
    }
    dataSectionReaderVec_t::iterator sit = _dataSections.begin();
    for( ; sit != _dataSections.end(); ++sit ) {
        if( *sit ) {
            delete *sit;
        }
    }
    _instancesLoaded.clear();
    _instanceStreamPos.clear();

    void * last = _instanceTypes->end().key;
    void * current = _instanceTypes->begin().key;
    std::string str;
    while( current != last ) {
        str = (char *)current;
        str.clear();
        current = _instanceTypes->next().key;
    }
    str.clear();

    delete _instanceTypes;
}

sectionID lazyInstMgr::reserveDataSection() {
    mtxLock( dataSectionsMtx );
    sectionID sid = _dataSections.size();
    _dataSections.push_back( NULL );
    mtxUnlock( dataSectionsMtx );
    return sid;
}

void lazyInstMgr::registerDataSection( lazyDataSectionReader * sreader, sectionID sid ) {
    assert( _dataSections[sid] == NULL );
    mtxLock( dataSectionsMtx );
    _dataSections[sid] = sreader;
    mtxUnlock( dataSectionsMtx );
}

void lazyInstMgr::addLazyInstance( namedLazyInstance inst ) {
    assert( inst.loc.begin > 0 && inst.loc.instance > 0 && inst.loc.section >= 0 );

    mtxLock( instanceTypesMtx );
    _lazyInstanceCount++;
    int len = strlen( inst.name );
    if( len > _longestTypeNameLen ) {
        _longestTypeNameLen = len;
        _longestTypeName = inst.name;
    }
    _instanceTypes->insert( inst.name, inst.loc.instance );
    mtxUnlock( instanceTypesMtx );

    /* store 16 bits of section id and 48 of instance offset into one 64-bit int
    * TODO: check and warn if anything is lost (in calling code?)
    * does 32bit need anything special?
    */
    positionAndSection ps = inst.loc.section;
    ps <<= 48;
    ps |= ( inst.loc.begin & 0xFFFFFFFFFFFFULL );

    mtxLock( instanceStreamPosMtx );
    _instanceStreamPos.insert( inst.loc.instance, ps );
    mtxUnlock( instanceStreamPosMtx );

    if( inst.refs ) {
        if( inst.refs->size() > 0 ) {
            //forward refs
            mtxLock( fwdRefsMtx );
            _fwdInstanceRefs.insert( inst.loc.instance, *inst.refs );
            mtxUnlock( fwdRefsMtx );

            instanceRefs::iterator it = inst.refs->begin();
            for( ; it != inst.refs->end(); ++it ) {
                //reverse refs
                mtxLock( revRefsMtx );
                _revInstanceRefs.insert( *it, inst.loc.instance );
                mtxUnlock( revRefsMtx );
            }
        } else {
            delete inst.refs;
        }
    }
}

unsigned long lazyInstMgr::getNumTypes() const {
    unsigned long n = 0 ;
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
    return n ;
}

void lazyInstMgr::openFile( std::string fname ) {
    _files.push_back( new lazyFileReader( fname, this, _files.size() ) );
}

SDAI_Application_instance * lazyInstMgr::loadInstance( instanceID id ) {
    assert( _mainRegistry && "Main registry has not been initialized. Do so with initRegistry() or setRegistry()." );
    SDAI_Application_instance * inst = 0;
    positionAndSection ps;
    sectionID sid;

    mtxLock( instanceStreamPosMtx );

    mtxLock( loadInstanceMtx );
    inst = _instancesLoaded.find( id );
    mtxUnlock( loadInstanceMtx );

    instanceStreamPos_t::cvector * cv;
    if( !inst && 0 != ( cv = _instanceStreamPos.find( id ) ) ) {
        //FIXME _instanceStreamPos.find( id ) can return nonzero for nonexistent key?!
        switch( cv->size() ) {
            case 0:
                std::cerr << "Instance #" << id << " not found in any section." << std::endl;
                break;
            case 1:
                long int off;
                ps = cv->at( 0 );
                off = ps & 0xFFFFFFFFFFFFULL;
                sid = ps >> 48;
                assert( _dataSections.size() > sid );
                inst = _dataSections[sid]->getRealInstance( _mainRegistry, off, id );
                break;
            default:
                std::cerr << "Instance #" << id << " exists in multiple sections. This is not yet supported." << std::endl;
                break;
        }
        if( ( inst ) && ( inst != & NilSTEPentity ) ) {
            mtxLock( loadInstanceMtx );
            _instancesLoaded.insert( id, inst );
            _loadedInstanceCount++;
            mtxUnlock( loadInstanceMtx );
        } else {
            std::cerr << "Error loading instance #" << id << "." << std::endl;
        }
    } else if( !inst ) {
        std::cerr << "Instance #" << id << " not found in any section." << std::endl;
    }

    mtxUnlock( instanceStreamPosMtx );
    return inst;
}

instanceSet * lazyInstMgr::instanceDependenciesHelper( instanceID id, instanceRefs_t * _fwdRefs ) {
    instanceSet * checkedDependencies = new instanceSet();
    instanceRefs dependencies; //Acts as queue for checking duplicated dependency

    instanceRefs_t::cvector * _fwdRefsVec = _fwdRefs->find( id );
    //Initially populating direct dependencies of id into the queue
    if( _fwdRefsVec != 0 ) {
        dependencies.insert( dependencies.end(), _fwdRefsVec->begin(), _fwdRefsVec->end() );
    }

    size_t curPos = 0;
    while( curPos < dependencies.size() ) {

        bool isNewElement = ( checkedDependencies->insert( dependencies.at( curPos ) ) ).second;
        if( isNewElement ) {
            _fwdRefsVec = _fwdRefs->find( dependencies.at( curPos ) );

            if( _fwdRefsVec != 0 ) {
                dependencies.insert( dependencies.end(), _fwdRefsVec->begin(), _fwdRefsVec->end() );
            }
        }

        curPos++;
    }

    return checkedDependencies;
}

instanceSet * lazyInstMgr::instanceDependencies( instanceID id ) {
   return instanceDependenciesHelper( id, getFwdRefs() );
}

void lazyInstMgr::openFileSafely( std::string fname ) {
    filesMtx.lock();
    size_t size = _files.size();
    _files.push_back( NULL ); //place Holder
    filesMtx.unlock();

    lazyFileReader * fileReader = new lazyFileReader( fname, this, size );

    filesMtx.lock();
    _files[size] = fileReader;
    filesMtx.unlock();
}

instanceRefs_t * lazyInstMgr::getFwdRefsSafely() {

    fwdRefsMtx.lock();
    instanceRefs_t * myFwdRefsCopy = new instanceRefs_t( _fwdInstanceRefs );
    fwdRefsMtx.unlock();

    return myFwdRefsCopy;
}

instanceRefs_t * lazyInstMgr::getRevRefsSafely() {

    revRefsMtx.lock();
    instanceRefs_t * myRevRefsCopy = new instanceRefs_t( _revInstanceRefs ); 
    revRefsMtx.unlock();

    return myRevRefsCopy;
}

instanceTypes_t::cvector * lazyInstMgr::getInstancesSafely( std::string type ) {

    instanceTypesMtx.lock();
    instanceTypes_t::cvector * typeInstances = _instanceTypes->find( type.c_str() );
    instanceTypesMtx.unlock();

    return typeInstances;
}

unsigned int lazyInstMgr::countInstancesSafely( std::string type ) {
    instanceTypes_t::cvector * v = getInstancesSafely( type );
    if( !v ) {
        return 0;
    }
    return v->size();
}

SDAI_Application_instance * lazyInstMgr::loadInstanceSafely( instanceID id ) {
    assert( _mainRegistry && "Main registry has not been initialized. Do so with initRegistry() or setRegistry()." );
    SDAI_Application_instance * sdaiInst;

    mtxLock( loadInstanceMtx );
    sdaiInst = _instancesLoaded.find( id );
    mtxUnlock( loadInstanceMtx );

    if( !sdaiInst ) {
        sdaiInst = loadInstance( id );
    }

    return sdaiInst;
}

instanceSet * lazyInstMgr::instanceDependenciesSafely( instanceID id ) {
   return instanceDependenciesHelper( id, getFwdRefsSafely() );
}

