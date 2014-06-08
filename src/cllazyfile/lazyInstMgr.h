#ifndef LAZYINSTMGR_H
#define LAZYINSTMGR_H

#include <map>
#include <string>
#include <assert.h>

#include "lazyDataSectionReader.h"
#include "lazyFileReader.h"
#include "lazyTypes.h"

#ifdef HAVE_STD_THREAD
# include <mutex>
#endif //HAVE_STD_THREAD

#include "Registry.h"
#include "sc_memmgr.h"

#include "judyLArray.h"
#include "judySArray.h"
#include "judyL2Array.h"
#include "judyS2Array.h"

class Registry;
class instMgrAdapter;

class lazyInstMgr {
    protected:
        /** map from instance number to instances that it refers to
         * \sa instanceRefs_pair
         */
        instanceRefs_t  _fwdInstanceRefs;
        /** map from instance number to instances that refer to it
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
         */
        instancesLoaded_t _instancesLoaded;

        /** map from instance number to beginning and end positions and the data section
         * \sa instanceStreamPos_pair
         * FIXME to save memory, modify judyL2Array to not use a vector until there are several
         * instances with the same instanceID. This will help elsewhere as well.
         */
        instanceStreamPos_t _instanceStreamPos;

        dataSectionReaderVec_t _dataSections;

        lazyFileReaderVec_t _files;

        Registry * _headerRegistry, * _mainRegistry;
        ErrorDescriptor * _errors;

        unsigned long _lazyInstanceCount, _loadedInstanceCount;
        int _longestTypeNameLen;
        std::string _longestTypeName;

        instMgrAdapter * _ima;

#ifdef HAVE_STD_THREAD
        /** mutexes for mutual exclusion */
        std::mutex fwdRefsMtx, revRefsMtx, instanceTypesMtx, loadInstanceMtx, instanceStreamPosMtx, dataSectionsMtx, filesMtx;
#else
        /** Dummy variables **/
        int fwdRefsMtx, revRefsMtx, instanceTypesMtx, loadInstanceMtx, instanceStreamPosMtx, dataSectionsMtx, filesMtx;
#endif //HAVE_STD_THREAD

    public:
        lazyInstMgr();
        ~lazyInstMgr();
        void addSchema( void ( *initFn )() ); //?
        void openFile( std::string fname );

        void addLazyInstance( namedLazyInstance inst );
        instMgrAdapter * getAdapter() {
            return _ima;
        }

        instanceRefs_t * getFwdRefs() {
            return & _fwdInstanceRefs;
        }

        instanceRefs_t * getRevRefs() {
            return & _revInstanceRefs;
        }

        /// returns two iterators delimiting the instances that match `type`
        instanceTypes_t::cvector * getInstances( std::string type ) { /*const*/
            return _instanceTypes->find( type.c_str() );
        }
        /// get the number of instances of a certain type
        unsigned int countInstances( std::string type ) {
            instanceTypes_t::cvector * v = _instanceTypes->find( type.c_str() );
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

        /// get the number of data sections that have been identified
        unsigned int countDataSections() {
            return _dataSections.size();
        }

        ///builds the registry using the given initFunct
        const Registry * initRegistry( CF_init initFunct ) {
            setRegistry( new Registry( initFunct ) );
            return _mainRegistry;
        }

        /// set the registry to one already initialized
        void setRegistry( Registry * reg ) {
            assert( _mainRegistry == 0 );
            _mainRegistry = reg;
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

        /// reserves a slot in _dataSections vector. Gives the slot a value null, and return the index value.
        sectionID reserveDataSection();

        /// updates _dataSections vector with the sreader value at the given index.
        void registerDataSection( lazyDataSectionReader * sreader, sectionID sid );

        fileID registerLazyFile( lazyFileReader * freader );

        ErrorDescriptor * getErrorDesc() {
            return _errors;
        }

        SDAI_Application_instance * loadInstance( instanceID id );

        /// lists all instances that one instance depends on (recursive)
        instanceSet * instanceDependencies( instanceID id );
        instanceSet * instanceDependenciesHelper( instanceID id, instanceRefs_t * _fwdRefs );

#ifdef HAVE_STD_THREAD
        /// thread safe counterpart of openFile();
        void openFileSafely( std::string fname );

        /// thread safe counterpart of getFwdRefs()
        instanceRefs_t * getFwdRefsSafely();

        /// thread safe counterpart of getRevRefs()
        instanceRefs_t * getRevRefsSafely();

        /// thread safe counterpart of getInstances()
        instanceTypes_t::cvector * getInstancesSafely( std::string type );

        /// thread safe counterpart of countInstances()
        unsigned int countInstancesSafely( std::string type );

        /// Thread safe counterpart of loadInstance( instanceID )
        SDAI_Application_instance * loadInstanceSafely( instanceID id );

        /// Thread safe counterpart of instanceDependencies( instanceID )
        instanceSet * instanceDependenciesSafely( instanceID id );

        void mtxLock( std::mutex &mtx ) {
            mtx.lock();
        }

        void mtxUnlock( std::mutex &mtx ) {
            mtx.unlock();
        }
#else
        /// dummy counterpart of mtxLock
        void mtxLock( int dummy ) {
            ( void )dummy;
        }

        /// dummy counterpart of mtxUnlock
        void mtxUnlock( int dummy ) {
            ( void )dummy;
        }
#endif //HAVE_STD_THREAD

        //unloads all instances. (For testing purpose, not thread safe)
        void unloadAllInstances() {
            _loadedInstanceCount = 0;
            _instancesLoaded.clear();
        }

        // (For testing purpose, not thread safe)
        instanceStreamPos_t * getInstanceStreamPos() {
            return &_instanceStreamPos;
        }

        // TODO implement these
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

