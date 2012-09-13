#ifndef LAZYINSTMGR_H
#define LAZYINSTMGR_H

#include <map>
#include <string>
// #include "lazyInstance.h"
#include "lazyDataSectionReader.h"
#include "lazyFileReader.h"
// #include <ExpDict.h>



// typedefs for instanceRefMMap, instanceTypeMMap, instancesLoaded
typedef std::multimap< instanceID, instanceID > instanceRefMMap_t;
typedef std::pair< instanceID, instanceID > instanceRefMMap_pair;
typedef std::pair< instanceRefMMap_t::iterator, instanceRefMMap_t::iterator > instanceRefMMap_range;
typedef std::multimap< std::string, instanceID > instanceTypeMMap_t;
typedef std::pair< std::string, instanceID > instanceTypeMMap_pair;
typedef std::pair< instanceTypeMMap_t::iterator, instanceTypeMMap_t::iterator > instanceTypeMMap_range;
typedef std::map< instanceID, SDAI_Application_instance * > instancesLoaded_t;
typedef std::pair< instanceID, SDAI_Application_instance * > instancesLoaded_pair;
// typedef std::map< instanceID, lazyInstance * > instancesLazy_t;
// typedef std::pair< instanceID, lazyInstance * > instancesLazy_pair;


class lazyInstMgr {
protected:
    /** multimap from instance number to instances that refer to it
     * \sa instanceRefMMap_pair
     * \sa instanceRefMMap_range
     */
    instanceRefMMap_t  instanceRefMMap;

    /** multimap from instance type to instance number
     * \sa instanceTypeMMap_pair
     * \sa instanceTypeMMap_range
     */
    instanceTypeMMap_t instanceTypeMMap;

    /** map from instance number to instance pointer (loaded instances only)
     * \sa instancesLoaded_pair
     */
    instancesLoaded_t instancesLoaded;

    /** map from instance number to instance pointer (instances not loaded only)
     * \sa instancesLoaded
     * \sa instancesLoaded_pair
     */
    instancesLazy_t instancesLazy;
public:
    void addSchema( void (*initFn) ());
    void addFile( std::string fname );
    // what about the registry?

//     addInstance(lazyInstance l, lazyFileReader* r);                ///< only used by lazy file reader functions
    void addLazyInstance(streamRange range, lazyDataSectionReader* r );
    void addDataSection(lazyDataSectionReader* d, lazyFileReader* f);   ///< only used by lazy file reader functions
};

#endif //LAZYINSTMGR_H