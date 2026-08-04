#include "cllazyfile/lazyTypes.h"
#include "cllazyfile/lazyInstMgr.h"
#include "clstepcore/Registry.h"
#include "clstepcore/SubSuperIterators.h"
#include "cleditor/SdaiSchemaInit.h"
#include "cllazyfile/instMgrHelper.h"
#include "lazyRefs.h"

#include "clstepcore/sdaiApplication_instance.h"

#include <algorithm>
#include <climits>
#include <sstream>

lazyInstMgr::lazyInstMgr() {
    _headerRegistry = new Registry( HeaderSchemaInit );
    _instanceTypes = new instanceTypes_t( 255 ); //NOTE arbitrary max of 255 chars for a type name
    _lazyInstanceCount = 0;
    _loadedInstanceCount = 0;
    _cacheHighWater = 0;
    _cacheHits = 0;
    _cacheMisses = 0;
    _materializations = 0;
    _evictions = 0;
    _activeBatches = 0;
    _residentSourceBytes = 0;
    _sourceBytesHighWater = 0;
    _batchLoadDepth = 0;
    _cancelled = false;
    _longestTypeNameLen = 0;
    _mainRegistry = 0;
    _ownsMainRegistry = false;
    _errors = new ErrorDescriptor();
    _ima = new instMgrAdapter( this );
}

lazyInstMgr::~lazyInstMgr() {
    // Keep registries and the adapter alive until their instances are gone.
    _instancesLoaded.clear( true );
    lazyFileReaderVec_t::iterator fit = _files.begin();
    for( ; fit != _files.end(); ++fit ) {
        delete *fit;
    }
    dataSectionReaderVec_t::iterator sit = _dataSections.begin();
    for( ; sit != _dataSections.end(); ++sit ) {
        delete *sit;
    }
    _instanceStreamPos.clear();
    delete _instanceTypes;
    delete _ima;
    if( _ownsMainRegistry ) delete _mainRegistry;
    delete _headerRegistry;
    delete _errors;
}

sectionID lazyInstMgr::registerDataSection( lazyDataSectionReader * sreader ) {
    _dataSections.push_back( sreader );
    return _dataSections.size() - 1;
}

void lazyInstMgr::addLazyInstance( namedLazyInstance inst ) {
    _lazyInstanceCount++;
    assert( inst.loc.begin > 0 && inst.loc.instance > 0 );
    const bool duplicate = _instanceStreamPos.find( inst.loc.instance ) != 0;
    int len = strlen( inst.name );
    if( len > _longestTypeNameLen ) {
        _longestTypeNameLen = len;
        _longestTypeName = inst.name;
    }
    _instanceTypes->insert( inst.name, inst.loc.instance );
    if( inst.componentTypes ) {
        if( !duplicate && !inst.componentTypes->empty() ) {
            _instanceComponentTypes[inst.loc.instance] = *inst.componentTypes;
        }
        std::vector<std::string>::const_iterator type = inst.componentTypes->begin();
        for( ; type != inst.componentTypes->end(); ++type ) {
            _instanceTypes->insert( type->c_str(), inst.loc.instance );
            if( static_cast<int>( type->size() ) > _longestTypeNameLen ) {
                _longestTypeNameLen = type->size();
                _longestTypeName = *type;
            }
        }
        delete inst.componentTypes;
    }
    if( !duplicate ) _allInstances.push_back( inst.loc.instance );
    instancePosition pos;
    pos.begin = inst.loc.begin;
    pos.section = inst.loc.section;
    if( duplicate ) {
        LazyDiagnostic diagnostic;
        diagnostic.severity = LAZY_DIAGNOSTIC_WARNING;
        diagnostic.entity = inst.loc.instance;
        diagnostic.type = inst.name;
        diagnostic.offset = inst.loc.begin;
        diagnostic.message = "duplicate instance identifier";
        emitDiagnostic( diagnostic );
    }
    _instanceStreamPos.insert( inst.loc.instance, pos );
    if( !duplicate && inst.loc.end >= inst.loc.begin ) {
        _instanceSourceBytes[inst.loc.instance] = inst.loc.end - inst.loc.begin;
    }

    if( inst.refs ) {
        if( inst.refs->size() > 0 ) {
            //forward refs
            _fwdInstanceRefs.insert( inst.loc.instance, *inst.refs );
            instanceRefs::iterator it = inst.refs->begin();
            for( ; it != inst.refs->end(); ++it ) {
                //reverse refs
                _revInstanceRefs.insert( *it, inst.loc.instance );
            }
        }
        delete inst.refs;
    }
}

const std::vector<std::string> & lazyInstMgr::componentTypes( instanceID id ) const {
    static const std::vector<std::string> empty;
    std::map<instanceID, std::vector<std::string> >::const_iterator found =
        _instanceComponentTypes.find( id );
    return found == _instanceComponentTypes.end() ? empty : found->second;
}

LazyInstanceIdView lazyInstMgr::instancesByType( std::string type, bool caseSensitive ) {
    if( !caseSensitive ) {
        std::string::iterator it = type.begin();
        for( ; it != type.end(); ++it ) {
            *it = static_cast<char>( toupper( static_cast<unsigned char>( *it ) ) );
        }
    }
    return LazyInstanceIdView( _instanceTypes->find( type.c_str() ) );
}

LazyInstanceIdView lazyInstMgr::forwardReferences( instanceID id ) {
    return LazyInstanceIdView( _fwdInstanceRefs.find( id ) );
}

LazyInstanceIdView lazyInstMgr::reverseReferences( instanceID id ) {
    return LazyInstanceIdView( _revInstanceRefs.find( id ) );
}

std::string lazyInstMgr::sourceRecord( instanceID id ) {
    instanceStreamPos_t::cvector * positions = _instanceStreamPos.find( id );
    std::map<instanceID, uint64_t>::const_iterator bytes = _instanceSourceBytes.find( id );
    if( !positions || positions->size() != 1 || bytes == _instanceSourceBytes.end() ||
            bytes->second == 0 ) {
        return std::string();
    }
    const instancePosition & position = positions->front();
    if( position.section >= _dataSections.size() || !_dataSections[position.section] ) {
        return std::string();
    }
    return _dataSections[position.section]->sourceRecord( position.begin, bytes->second );
}

void lazyInstMgr::observeScan( fileID file, lazyFileOffset offset, lazyFileOffset fileSize ) {
    if( _cancelled ) return;
    if( _cancellationCallback && _cancellationCallback() ) {
        _cancelled = true;
        LazyDiagnostic diagnostic;
        diagnostic.severity = LAZY_DIAGNOSTIC_INFO;
        diagnostic.offset = offset;
        diagnostic.message = "scan cancelled";
        emitDiagnostic( diagnostic );
        return;
    }
    if( _progressCallback && ( _lazyInstanceCount == 1 || ( _lazyInstanceCount % 4096 ) == 0 || offset >= fileSize ) ) {
        LazyScanProgress progress;
        progress.file = file;
        progress.offset = offset;
        progress.fileSize = fileSize;
        progress.instancesScanned = _lazyInstanceCount;
        _progressCallback( progress );
    }
}

void lazyInstMgr::emitDiagnostic( LazyDiagnostic diagnostic ) {
    std::ostringstream key;
    key << diagnostic.type << '#' << diagnostic.entity << ':' << diagnostic.attribute << ':' << diagnostic.message;
    uint64_t & count = _diagnosticCounts[key.str()];
    ++count;
    diagnostic.occurrences = count;
    if( _diagnosticCallback && count == 1 ) {
        _diagnosticCallback( diagnostic );
    }
}

uint64_t lazyInstMgr::diagnosticCount( const std::string & key ) const {
    std::map<std::string, uint64_t>::const_iterator found = _diagnosticCounts.find( key );
    return found == _diagnosticCounts.end() ? 0 : found->second;
}

LazyCacheStatistics lazyInstMgr::cacheStatistics() const {
    LazyCacheStatistics stats;
    stats.instancesScanned = _lazyInstanceCount;
    stats.instancesLoaded = _loadedInstanceCount;
    stats.instancesPinned = _pinCounts.size();
    stats.cacheHighWater = _cacheHighWater;
    stats.cacheHits = _cacheHits;
    stats.cacheMisses = _cacheMisses;
    stats.materializations = _materializations;
    stats.evictions = _evictions;
    stats.activeBatches = _activeBatches;
    stats.dataSections = _dataSections.size();
    stats.residentSourceBytes = _residentSourceBytes;
    stats.sourceBytesHighWater = _sourceBytesHighWater;
    stats.cancelled = _cancelled;
    return stats;
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

bool lazyInstMgr::openFile( std::string fname ) {
    //don't want to hold a lock for the entire time we're reading the file.
    //create a place in the vector and remember its location, then free lock
    ///FIXME begin atomic op
    size_t i = _files.size();
    _files.push_back( (lazyFileReader * ) 0 );
    ///FIXME end atomic op
    lazyFileReader * lfr = new lazyFileReader( fname, this, i );
    if( !lfr->valid() ) {
        delete lfr;
        _files.pop_back();
        return false;
    }
    _files[i] = lfr;
    validateReferences();
    /// TODO resolve inverse attr references
    //between instances, or eDesc --> inst????
    return true;
}

void lazyInstMgr::validateReferences() {
    instanceRefs_t::cpair current = _fwdInstanceRefs.begin();
    while( current.value ) {
        instanceRefs_t::cvector::const_iterator ref = current.value->begin();
        for( ; ref != current.value->end(); ++ref ) {
            if( !_instanceStreamPos.find( *ref ) ) {
                LazyDiagnostic diagnostic;
                diagnostic.severity = LAZY_DIAGNOSTIC_ERROR;
                diagnostic.entity = current.key;
                const char * type = typeFromFile( current.key );
                if( type ) diagnostic.type = type;
                instanceStreamPos_t::cvector * positions = _instanceStreamPos.find( current.key );
                if( positions && !positions->empty() ) diagnostic.offset = positions->front().begin;
                std::ostringstream message;
                message << "reference to missing instance #" << *ref;
                diagnostic.message = message.str();
                emitDiagnostic( diagnostic );
            }
        }
        current = _fwdInstanceRefs.next();
    }
}

SDAI_Application_instance * lazyInstMgr::loadInstance( instanceID id, bool reSeek, bool promoteCached ) {
    if( _batchLoadDepth && ( _cancelled ||
            ( _cancellationCallback && _cancellationCallback() ) ) ) {
        _cancelled = true;
        return 0;
    }
    assert( _mainRegistry && "Main registry has not been initialized. Do so with initRegistry() or setRegistry()." );
    std::streampos oldPos;
    instancePosition pos = instancePosition();
    SDAI_Application_instance * inst = _instancesLoaded.find( id );
    if( inst ) {
        ++_cacheHits;
        if( promoteCached && _batchLoadDepth == 0 ) _permanentlyLoadedInstances.insert( id );
        return inst;
    }
    ++_cacheMisses;
    if( id > static_cast<instanceID>( INT_MAX ) ) {
        LazyDiagnostic diagnostic;
        diagnostic.severity = LAZY_DIAGNOSTIC_ERROR;
        diagnostic.entity = id;
        diagnostic.message = "instance ID exceeds the current SDAI materialization limit";
        emitDiagnostic( diagnostic );
        return 0;
    }
    const std::pair<std::set<instanceID>::iterator, bool> loading_result = _instancesLoading.insert( id );
    if( !loading_result.second ) {
        LazyDiagnostic diagnostic;
        diagnostic.severity = LAZY_DIAGNOSTIC_ERROR;
        diagnostic.entity = id;
        const char * type = typeFromFile( id );
        if( type ) diagnostic.type = type;
        instanceStreamPos_t::cvector * positions = _instanceStreamPos.find( id );
        if( positions && !positions->empty() ) {
            diagnostic.offset = positions->front().begin;
        }
        diagnostic.message = "cyclic dependency encountered while materializing instance";
        emitDiagnostic( diagnostic );
        return 0;
    }
    /* sectionReader materializes referenced instances recursively.  Keep an
     * explicit in-progress set because an instance is not cacheable until its
     * STEPread() has completed. */
    class LoadingGuard {
      public:
        LoadingGuard( std::set<instanceID> & loading, instanceID id )
            : _loading( loading ), _id( id ), _active( true ) {}
        ~LoadingGuard() { release(); }
        void release() {
            if( _active ) {
                _loading.erase( _id );
                _active = false;
            }
        }
      private:
        std::set<instanceID> & _loading;
        instanceID _id;
        bool _active;
    } loading_guard( _instancesLoading, id );
    instanceStreamPos_t::cvector * cv;
    if( 0 != ( cv = _instanceStreamPos.find( id ) ) ) {
        switch( cv->size() ) {
            case 0:
                if( !_diagnosticCallback ) {
                    std::cerr << "Instance #" << id
                        << " not found in any section." << std::endl;
                }
                break;
            case 1:
                pos = cv->at( 0 );
                assert( _dataSections.size() > pos.section );
                if( reSeek ) {
                    oldPos = _dataSections[pos.section]->tellg();
                }
                inst = _dataSections[pos.section]->getRealInstance( _mainRegistry, pos.begin, id );
                if( reSeek ) {
                    _dataSections[pos.section]->seekg( oldPos );
                }
                /* A recursive reference load may observe cancellation after
                 * this instance began materializing.  Never cache that
                 * partially resolved SDAI object. */
                if( _batchLoadDepth && ( _cancelled ||
                        ( _cancellationCallback && _cancellationCallback() ) ) ) {
                    _cancelled = true;
                    if( !isNilSTEPentity( inst ) ) delete inst;
                    inst = 0;
                }
                break;
            default:
                if( !_diagnosticCallback ) {
                    std::cerr << "Instance #" << id
                        << " exists in multiple sections. This is not yet supported."
                        << std::endl;
                }
                {
                    LazyDiagnostic diagnostic;
                    diagnostic.severity = LAZY_DIAGNOSTIC_ERROR;
                    diagnostic.entity = id;
                    diagnostic.message = "instance identifier occurs in multiple DATA sections";
                    emitDiagnostic( diagnostic );
                }
                break;
        }
        /* Explicit attributes are now complete.  Inverse discovery may
         * safely revisit this instance without tripping the materialization
         * cycle detector. */
        loading_guard.release();
        if( !isNilSTEPentity( inst ) ) {
            _instancesLoaded.insert( id, inst );
            _loadedInstanceCount++;
            ++_materializations;
            _cacheHighWater = std::max<uint64_t>( _cacheHighWater, _loadedInstanceCount );
            std::map<instanceID, uint64_t>::const_iterator source_size = _instanceSourceBytes.find( id );
            if( source_size != _instanceSourceBytes.end() ) {
                _residentSourceBytes += source_size->second;
                _sourceBytesHighWater = std::max<uint64_t>( _sourceBytesHighWater,
                    _residentSourceBytes );
            }
            if( _batchLoadDepth ) {
                _batchOwnedInstances.insert( id );
            } else {
                _permanentlyLoadedInstances.insert( id );
                lazyRefs lr( this, inst );
                lazyRefs::referentInstances_t insts = lr.result();
                resolveDeferredInverses();
            }
        } else {
            if( !_diagnosticCallback ) {
                std::cerr << "Error loading instance #" << id << "."
                    << std::endl;
            }
            LazyDiagnostic diagnostic;
            diagnostic.severity = LAZY_DIAGNOSTIC_ERROR;
            diagnostic.entity = id;
            diagnostic.offset = pos.begin;
            diagnostic.message = "SDAI instance materialization failed";
            emitDiagnostic( diagnostic );
        }
    } else {
        if( !_diagnosticCallback ) {
            std::cerr << "Instance #" << id
                << " not found in any section." << std::endl;
        }
        LazyDiagnostic diagnostic;
        diagnostic.severity = LAZY_DIAGNOSTIC_ERROR;
        diagnostic.entity = id;
        diagnostic.message = "instance not found in any DATA section";
        emitDiagnostic( diagnostic );
    }
    return inst;
}

void lazyInstMgr::resolveDeferredInverses() {
    if( !_instancesLoading.empty() ) return;

    while( !_deferredInverseInstances.empty() ) {
        std::set<instanceID> deferred;
        deferred.swap( _deferredInverseInstances );
        std::set<instanceID>::const_iterator id = deferred.begin();
        for( ; id != deferred.end(); ++id ) {
            SDAI_Application_instance * inst = cachedInstance( *id );
            if( inst ) {
                lazyRefs refs( this, inst );
            }
        }
    }
}

SDAI_Application_instance * lazyInstMgr::cachedInstance( instanceID id ) {
    return _instancesLoaded.find( id );
}

std::vector<instanceID> lazyInstMgr::dependencyClosure( const std::vector<instanceID> & roots ) {
    std::set<instanceID> closure( roots.begin(), roots.end() );
    std::vector<instanceID> queue( roots.begin(), roots.end() );
    size_t current = 0;
    while( current < queue.size() ) {
        if( _cancelled || ( _cancellationCallback && _cancellationCallback() ) ) {
            _cancelled = true;
            break;
        }
        instanceRefs_t::cvector * refs = _fwdInstanceRefs.find( queue[current++] );
        if( !refs ) continue;
        instanceRefs_t::cvector::const_iterator ref = refs->begin();
        for( ; ref != refs->end(); ++ref ) {
            /* validateReferences() reports nonexistent targets when the file
             * is opened.  They cannot be pinned or materialized, and adding
             * them to a batch only produces a second, less useful error (or
             * an integer-limit error for a malformed maximum-width ID). */
            if( !_instanceStreamPos.find( *ref ) ) continue;
            if( closure.insert( *ref ).second ) queue.push_back( *ref );
        }
    }
    return std::vector<instanceID>( closure.begin(), closure.end() );
}

LazyInstanceBatch lazyInstMgr::loadBatch( instanceID root ) {
    std::vector<instanceID> roots( 1, root );
    return loadBatch( roots );
}

LazyInstanceBatch lazyInstMgr::loadBatch( const std::vector<instanceID> & roots ) {
    std::vector<instanceID> closure = dependencyClosure( roots );
    std::vector<instanceID>::const_iterator id = closure.begin();
    for( ; id != closure.end(); ++id ) {
        if( _cancelled || ( _cancellationCallback && _cancellationCallback() ) ) {
            _cancelled = true;
            closure.erase( id, closure.end() );
            break;
        }
        ++_pinCounts[*id];
    }
    ++_activeBatches;
    ++_batchLoadDepth;
    for( id = closure.begin(); id != closure.end(); ++id ) {
        if( _cancelled || ( _cancellationCallback && _cancellationCallback() ) ) {
            _cancelled = true;
            break;
        }
        if( !loadInstance( *id, true ) ) {
            LazyDiagnostic diagnostic;
            diagnostic.severity = LAZY_DIAGNOSTIC_ERROR;
            diagnostic.entity = *id;
            diagnostic.message = "dependency batch could not materialize instance";
            emitDiagnostic( diagnostic );
        }
    }
    --_batchLoadDepth;
    return LazyInstanceBatch( this, roots, closure );
}

void lazyInstMgr::releaseBatch( const std::vector<instanceID> & instances ) {
    std::vector<instanceID> evict;
    std::vector<instanceID>::const_iterator id = instances.begin();
    for( ; id != instances.end(); ++id ) {
        std::map<instanceID, size_t>::iterator pin = _pinCounts.find( *id );
        if( pin == _pinCounts.end() ) continue;
        if( --pin->second == 0 ) {
            _pinCounts.erase( pin );
            if( _batchOwnedInstances.count( *id ) && !_permanentlyLoadedInstances.count( *id ) ) {
                evict.push_back( *id );
            }
        }
    }
    std::vector<SDAI_Application_instance *> deleted;
    for( id = evict.begin(); id != evict.end(); ++id ) {
        SDAI_Application_instance * inst = _instancesLoaded.find( *id );
        if( !inst ) continue;
        _instancesLoaded.removeEntry( *id );
        _batchOwnedInstances.erase( *id );
        deleted.push_back( inst );
        --_loadedInstanceCount;
        std::map<instanceID, uint64_t>::const_iterator source_size = _instanceSourceBytes.find( *id );
        if( source_size != _instanceSourceBytes.end() ) {
            _residentSourceBytes = source_size->second <= _residentSourceBytes ?
                _residentSourceBytes - source_size->second : 0;
        }
        ++_evictions;
    }
    std::vector<SDAI_Application_instance *>::iterator inst = deleted.begin();
    for( ; inst != deleted.end(); ++inst ) delete *inst;
    if( _activeBatches ) --_activeBatches;
}


instanceSet * lazyInstMgr::instanceDependencies( instanceID id ) {
    instanceSet * checkedDependencies = new instanceSet();
    instanceRefs dependencies; //Acts as queue for checking duplicated dependency

    instanceRefs_t * _fwdRefs = getFwdRefs();
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
