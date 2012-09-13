#ifndef LAZYTYPES_H
#define LAZYTYPES_H

#include <map>
#include <iostream>
#include "sdaiApplication_instance.h"

typedef long instanceID;

//FIXME need to include sectionReader* in below type
// typedef std::pair< std::streampos, std::streampos > streamRange;

// typedefs for instanceRefMMap, instanceTypeMMap, instancesLoaded, instanceStreamPosMap
typedef std::multimap< instanceID, instanceID > instanceRefMMap_t;
typedef std::pair< instanceID, instanceID > instanceRefMMap_pair;
typedef std::pair< instanceRefMMap_t::iterator, instanceRefMMap_t::iterator > instanceRefMMap_range;
typedef std::multimap< std::string, instanceID > instanceTypeMMap_t;
typedef std::pair< std::string, instanceID > instanceTypeMMap_pair;
typedef std::pair< instanceTypeMMap_t::iterator, instanceTypeMMap_t::iterator > instanceTypeMMap_range;
typedef std::map< instanceID, SDAI_Application_instance * > instancesLoaded_t;
typedef std::pair< instanceID, SDAI_Application_instance * > instancesLoaded_pair;
typedef std::map< instanceID, streamRange > instanceStreamPosMap_t;
typedef std::pair< instanceID, streamRange > instanceStreamPosMap_pair;

// typedef std::map< instanceID, lazyInstance * > instancesLazy_t;
// typedef std::pair< instanceID, lazyInstance * > instancesLazy_pair;




#endif //LAZYTYPES_H