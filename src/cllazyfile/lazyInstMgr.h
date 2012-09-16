#ifndef LAZYINSTMGR_H
#define LAZYINSTMGR_H

#include <map>
#include <string>
// #include "lazyInstance.h"
#include "lazyDataSectionReader.h"
#include "lazyFileReader.h"
// #include <ExpDict.h>
#include "lazyTypes.h"




class lazyInstMgr {
protected:
    /** multimap from instance number to instances that refer to it
     * \sa instanceRefMMap_pair
     * \sa instanceRefMMap_range
     */
    instanceRefMMap_t  _instanceRefMMap;

    /** multimap from instance type to instance number
     * \sa instanceTypeMMap_pair
     * \sa instanceTypeMMap_range
     */
    instanceTypeMMap_t _instanceTypeMMap;

    /** map from instance number to instance pointer (loaded instances only)
     * \sa instancesLoaded_pair
     */
    instancesLoaded_t _instancesLoaded;

//     /** map from instance number to instance pointer (instances not loaded only)
//      * \sa instancesLoaded
//      * \sa instancesLoaded_pair
//      */
//     instancesLazy_t instancesLazy;

    /** map from instance number to beginning and end positions and the data section
     * \sa instanceStreamPosMap_pair
     */
    instanceStreamPosMMap_t _instanceStreamPosMap;

    dataSectionReaderVec_t _dataSections;

    lazyFileReaderVec_t _files;

public:
    void addSchema( void (*initFn) () ); //?
    void openFile( std::string fname ) {
        _files.push_back( new lazyFileReader( fname, this ) );
    }
    // what about the registry?

//     addInstance(lazyInstance l, lazyFileReader* r);                ///< only used by lazy file reader functions
//     void addLazyInstance( lazyInstance range, lazyDataSectionReader* r );
    void addDataSection( lazyDataSectionReader* d, lazyFileReader* f );   ///< only used by lazy file reader functions


    instanceRefMMap_range getReferentInstances( instanceID id ) {
        return _instanceRefMMap.equal_range(id);
    }

    instanceTypeMMap_range getInstances( std::string type ) {
        return _instanceTypeMMap.equal_range( type );
    }

    sectionID getSectionID( sectionReader * reader ) {
        sectionID i = 0, l = _dataSections.size();
        while( i < l ) {
            if( _dataSections[i] == reader ) {
                return i;
            }
        }
        abort();
//         return -1;
    }
    fileID getFileID();

    // TODO void useDataSection( sectionID id ); ///< tell instMgr to use instances from this section
    // TODO support references from one file to another
};

#endif //LAZYINSTMGR_H