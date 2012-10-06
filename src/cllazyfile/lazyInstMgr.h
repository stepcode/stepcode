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
    ErrorDescriptor * _errors;

    unsigned long _lazyInstanceCount;
    int _longestTypeNameLen;
    std::string _longestTypeName;


public:
    lazyInstMgr();
    void addSchema( void (*initFn) () ); //?
    void openFile( std::string fname );

    void addLazyInstance( namedLazyInstance inst );

    /// FIXME don't return something that can be modified; also, template references will cause problems on windows
    instanceRefMMap_range getReferentInstances( instanceID id ) {
        return _instanceRefMMap.equal_range(id);
    }

    /// FIXME don't return something that can be modified; also, template references will cause problems on windows
    instanceTypeMMap_range getInstances( std::string type ) {
        return _instanceTypeMMap.equal_range( type );
    }

    const Registry * getHeaderRegistry() const {
        return _headerRegistry;
    }

    /// get the number of instances that have been found in the open files.
    unsigned long getInstanceCount() const {
        return _lazyInstanceCount;
    }

    /// get the longest type name
    const std::string & getLongestTypeName() const {
        return _longestTypeName;
    }

    /// get the number of types of instances.
    unsigned long getNumTypes();

    sectionID registerDataSection( lazyDataSectionReader * sreader );
    fileID registerLazyFile( lazyFileReader * freader );

    ErrorDescriptor * getErrorDesc() {
        return _errors;
    }

    /* TODO impliment these
     *    void normalizeInstanceIds();
     *    void eliminateDuplicates();
     *    void useDataSection( sectionID id ); ///< tell instMgr to use instances from this section
     */
    // TODO support references from one file to another
    // TODO registry

};

#endif //LAZYINSTMGR_H