#ifndef LAZYINSTMGR_H
#define LAZYINSTMGR_H

#include <cctype>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <assert.h>

#include "cllazyfile/lazyDataSectionReader.h"
#include "cllazyfile/lazyFileReader.h"
#include "cllazyfile/lazyTypes.h"
#include "cllazyfile/lazySupport.h"

#include "clstepcore/Registry.h"
#include "sc_export.h"

#include "judyLArray.h"
#include "judySArray.h"
#include "judyL2Array.h"
#include "judyS2Array.h"

class Registry;
class instMgrAdapter;
class lazyRefs;

class SC_LAZYFILE_EXPORT lazyInstMgr {
    protected:
        /** multimap from instance number to instances that it refers to
         * \sa instanceRefs_pair
         */
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4251 )
#endif
        instanceRefs_t  _fwdInstanceRefs;
        /** multimap from instance number to instances that refer to it - the majority of these will not be inverse references!
         * \sa instanceRefs_pair
         */
        instanceRefs_t _revInstanceRefs;

        /** multimap from instance type to instance number
         * \sa instanceType_pair
         * \sa instanceType_range
         */
        instanceTypes_t * _instanceTypes;

        /** map from instance number to instance pointer (loaded instances only)
         * \sa instancesLoaded_pair
         *
         * TODO should be multimap to allow use of instances in multiple data sections?
         * a unique instance ID (containing sectionID and instanceID) would also work
         */
        instancesLoaded_t _instancesLoaded;

        /** map from instance number to beginning and end positions and the data section
         * \sa instanceStreamPos_pair
         *
         * FIXME to save memory, modify judyL2Array to not use a vector until there are several
         * instances with the same instanceID. This will help elsewhere as well.
         */
        instanceStreamPos_t _instanceStreamPos;

        dataSectionReaderVec_t _dataSections;

        lazyFileReaderVec_t _files;

        /** All indexed DATA instance IDs in deterministic file order. */
        instanceRefs _allInstances;

        Registry * _headerRegistry, * _mainRegistry;
        bool _ownsMainRegistry;
        ErrorDescriptor * _errors;

        unsigned long _lazyInstanceCount, _loadedInstanceCount;
        uint64_t _cacheHighWater, _cacheHits, _cacheMisses, _materializations, _evictions;
        uint64_t _activeBatches;
        uint64_t _residentSourceBytes, _sourceBytesHighWater;
        int _longestTypeNameLen;
        std::string _longestTypeName;

        std::map<instanceID, size_t> _pinCounts;
        std::map<instanceID, uint64_t> _instanceSourceBytes;
        std::map<std::string, std::string> _materializationTypeAliases;
        std::set<instanceID> _batchOwnedInstances;
        std::set<instanceID> _permanentlyLoadedInstances;
        std::set<instanceID> _instancesLoading;
        std::set<instanceID> _deferredInverseInstances;
        size_t _batchLoadDepth;

        LazyProgressCallback _progressCallback;
        LazyCancellationCallback _cancellationCallback;
        LazyDiagnosticCallback _diagnosticCallback;
        std::map<std::string, uint64_t> _diagnosticCounts;
        bool _cancelled;

        instMgrAdapter * _ima;

        friend class LazyInstanceBatch;
        friend class lazyRefs;
        void releaseBatch( const std::vector<instanceID> & instances );
        void resolveDeferredInverses();
        bool isMaterializing( instanceID id ) const {
            return _instancesLoading.count( id ) != 0;
        }
        void deferInverseResolution( instanceID id ) {
            _deferredInverseInstances.insert( id );
        }
        SDAI_Application_instance * cachedInstance( instanceID id );
        std::vector<instanceID> dependencyClosure( const std::vector<instanceID> & roots );

#ifdef _MSC_VER
#pragma warning( pop )
#endif

    public:
        lazyInstMgr();
        ~lazyInstMgr();
        bool openFile( std::string fname );

        void addLazyInstance( namedLazyInstance inst );
        InstMgrBase * getAdapter() {
            return ( InstMgrBase * ) _ima;
        }

        /** Register an explicit source-keyword substitution used only when
         * materializing an SDAI object.  The lazy type index retains the
         * original Part 21 keyword.  This is intended for known,
         * attribute-compatible exporter aliases, not arbitrary recovery. */
        void setMaterializationTypeAlias( std::string source, const std::string & target ) {
            for( std::string::iterator c = source.begin(); c != source.end(); ++c ) {
                *c = static_cast<char>( toupper( static_cast<unsigned char>( *c ) ) );
            }
            _materializationTypeAliases[source] = target;
        }

        std::string materializationType( std::string source ) const {
            std::string key = source;
            for( std::string::iterator c = key.begin(); c != key.end(); ++c ) {
                *c = static_cast<char>( toupper( static_cast<unsigned char>( *c ) ) );
            }
            std::map<std::string, std::string>::const_iterator alias =
                _materializationTypeAliases.find( key );
            return alias == _materializationTypeAliases.end() ? source : alias->second;
        }

        instanceRefs_t * getFwdRefs() {
            return & _fwdInstanceRefs;
        }

        instanceRefs_t * getRevRefs() {
            return & _revInstanceRefs;
        }
        LazyInstanceIdView instancesByType( std::string type, bool caseSensitive = false );
        LazyInstanceIdView allInstances() const {
            return LazyInstanceIdView( &_allInstances );
        }
        LazyInstanceIdView forwardReferences( instanceID id );
        LazyInstanceIdView reverseReferences( instanceID id );
        /** Copy the exact indexed source record for an instance.  Returns an
         * empty string when the ID is missing or ambiguous. */
        std::string sourceRecord( instanceID id );

        /// returns a vector containing the instances that match `type`
        instanceTypes_t::cvector * getInstances( std::string type, bool caseSensitive = false ) { /*const*/
            if( !caseSensitive ) {
                std::string::iterator it = type.begin();
                for( ; it != type.end(); ++it ) {
                    *it = static_cast<char>( toupper( static_cast<unsigned char>( *it ) ) );
                }
            }
            return _instanceTypes->find( type.c_str() );
        }
        /// get the number of instances of a certain type
        unsigned int countInstances( std::string type, bool caseSensitive = false ) {
            instanceTypes_t::cvector * v = getInstances( type, caseSensitive );
            if( !v ) {
                return 0;
            }
            return v->size();
        }
        instancesLoaded_t * getHeaderInstances( fileID file ) {
            return _files[file]->getHeaderInstances();
        }

        /// get the number of instances that have been found in the open files.
        unsigned long totalInstanceCount() const {
            return _lazyInstanceCount;
        }

        /// get the number of instances that are loaded.
        unsigned long loadedInstanceCount() const {
            return _loadedInstanceCount;
        }

        LazyCacheStatistics cacheStatistics() const;

        void setProgressCallback( const LazyProgressCallback & callback ) {
            _progressCallback = callback;
        }
        void setCancellationCallback( const LazyCancellationCallback & callback ) {
            _cancellationCallback = callback;
        }
        void setDiagnosticCallback( const LazyDiagnosticCallback & callback ) {
            _diagnosticCallback = callback;
        }
        bool cancelled() const {
            return _cancelled;
        }
        void observeScan( fileID file, lazyFileOffset offset, lazyFileOffset fileSize );
        void emitDiagnostic( LazyDiagnostic diagnostic );
        uint64_t diagnosticCount( const std::string & key ) const;
        const std::map<std::string, uint64_t> & diagnosticCounts() const {
            return _diagnosticCounts;
        }
        void validateReferences();

        /// get the number of data sections that have been identified
        unsigned int countDataSections() {
            return _dataSections.size();
        }

        ///builds the registry using the given initFunct
        const Registry * initRegistry( CF_init initFunct ) {
            assert( _mainRegistry == 0 );
            _mainRegistry = new Registry( initFunct );
            _ownsMainRegistry = true;
            return _mainRegistry;
        }

        /// set the registry to one already initialized
        void setRegistry( Registry * reg ) {
            assert( _mainRegistry == 0 );
            _mainRegistry = reg;
            _ownsMainRegistry = false;
        }

        const Registry * getHeaderRegistry() const {
            return _headerRegistry;
        }
        const Registry * getMainRegistry() const {
            return _mainRegistry;
        }


        /// get the longest type name
        const std::string & getLongestTypeName() const {
            return _longestTypeName;
        }

        /// get the number of types of instances.
        unsigned long getNumTypes() const;

        sectionID registerDataSection( lazyDataSectionReader * sreader );

        ErrorDescriptor * getErrorDesc() {
            return _errors;
        }

        /** returns a pointer to an instance, loading it if necessary.
         * \param id the instance number to look for
         * \param reSeek if true, reset file position to current position when done. only necessary when loading an instance with dependencies; excessive use will cause a performance hit
         * \param promoteCached if true, a cached instance requested outside a batch is retained for legacy callers
         */
        SDAI_Application_instance * loadInstance( instanceID id, bool reSeek = false, bool promoteCached = true );

        LazyInstanceBatch loadBatch( instanceID root );
        LazyInstanceBatch loadBatch( const std::vector<instanceID> & roots );

        //list all instances that one instance depends on (recursive)
        instanceSet * instanceDependencies( instanceID id );
        bool isLoaded( instanceID id ) {
            _instancesLoaded.find( id );
            return _instancesLoaded.success();
        }

        const char * typeFromFile( instanceID id ) {
            instanceStreamPos_t::cvector * cv;
            cv = _instanceStreamPos.find( id );
            if( cv ) {
                if( cv->size() != 1 ) {
                    std::cerr << "Error at " << __FILE__ << ":" << __LINE__ << " - multiple instances (" << cv->size() << ") with one instanceID (" << id << ") not supported yet." << std::endl;
                    return 0;
                }
                instancePosition pos = cv->at( 0 );
                return _dataSections[pos.section]->getType( pos.begin );
            }
            std::cerr << "Error at " << __FILE__ << ":" << __LINE__ << " - instanceID " << id << " not found." << std::endl;
            return 0;
        }

        // TODO implement these

            // add another schema to registry
            //void addSchema( void ( *initFn )() );

            //list all instances that one instance depends on (recursive)
            //std::vector<instanceID> instanceDependencies( instanceID id ); //set is faster?

            /* * the opposite of instanceDependencies() - all instances that are *not* dependencies of one particular instance
                 same as above, but with list of instances */
            //std::vector<instanceID> notDependencies(...)

            //renumber instances so that they are numbered 1..N where N is the total number of instances
            //void normalizeInstanceIds();

            //find data that is repeated and eliminate, if possible
            //void eliminateDuplicates();

            //tell instMgr to use instances from this section
            //void useDataSection( sectionID id );

        // TODO support references from one file to another
};

#endif //LAZYINSTMGR_H
