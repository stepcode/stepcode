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

    /** map from instance number to beginning and end positions and the data section
     * \sa instanceStreamPosMap_pair
     */
    instanceStreamPosMMap_t _instanceStreamPosMMap;

    dataSectionReaderVec_t _dataSections;

    lazyFileReaderVec_t _files;

    Registry * _headerRegistry, * _mainRegistry;


public:
    lazyInstMgr();
    void addSchema( void (*initFn) () ); //?
    void openFile( std::string fname ) {
        //  lazyFileReader adds itself to the file list - good idea or bad?
        /*_files.push_back( */new lazyFileReader( fname, this ) /*)*/;
    }

    void addLazyInstance( namedLazyInstance inst );
    void addDataSection( lazyDataSectionReader* d, lazyFileReader* f );   ///< only used by lazy file reader functions


    instanceRefMMap_range getReferentInstances( instanceID id ) {
        return _instanceRefMMap.equal_range(id);
    }

    instanceTypeMMap_range getInstances( std::string type ) {
        return _instanceTypeMMap.equal_range( type );
    }

    const Registry * getHeaderRegistry() const {
        return _headerRegistry;
    }


    sectionID registerDataSection( lazyDataSectionReader * sreader );
    fileID registerLazyFile( lazyFileReader * freader );

    /* TODO impliment these
     *    void normalizeInstanceIds();
     *    void eliminateDuplicates();
     *    void useDataSection( sectionID id ); ///< tell instMgr to use instances from this section
     */
    // TODO support references from one file to another
    // TODO registry

};

#endif //LAZYINSTMGR_H