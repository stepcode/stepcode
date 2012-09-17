#ifndef LAZYINSTMGR_H
#define LAZYINSTMGR_H

#include <map>
#include <string>
// #include "lazyInstance.h"
#include "lazyDataSectionReader.h"
#include "lazyFileReader.h"
// #include <ExpDict.h>
#include "lazyTypes.h"

class Registry;

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
    instanceStreamPosMMap_t _instanceStreamPosMMap;

    dataSectionReaderVec_t _dataSections;

    lazyFileReaderVec_t _files;

    Registry * _headerRegistry;


public:
    lazyInstMgr();
    void addSchema( void (*initFn) () ); //?
    void openFile( std::string fname ) {
        _files.push_back( new lazyFileReader( fname, this ) );
    }
    // what about the registry?

//     addInstance(lazyInstance l, lazyFileReader* r);                ///< only used by lazy file reader functions
    void addLazyInstance( namedLazyInstance inst ) {
        instanceStreamPosMMap_pair pos( inst.loc.instance, inst.loc );
        _instanceStreamPosMMap.insert( pos );
        instanceTypeMMap_pair type( *inst.name, inst.loc.instance );
        _instanceTypeMMap.insert( type );
        delete inst.name;
    }
    void addDataSection( lazyDataSectionReader* d, lazyFileReader* f );   ///< only used by lazy file reader functions


    instanceRefMMap_range getReferentInstances( instanceID id ) {
        return _instanceRefMMap.equal_range(id);
    }

    instanceTypeMMap_range getInstances( std::string type ) {
        return _instanceTypeMMap.equal_range( type );
    }

    Registry * getHeaderRegistry() {
        return _headerRegistry;
    }

    sectionID getSectionID( sectionReader * sreader );
    fileID getFileID( lazyFileReader * freader );

    // TODO void useDataSection( sectionID id ); ///< tell instMgr to use instances from this section
    // TODO support references from one file to another
};

#endif //LAZYINSTMGR_H