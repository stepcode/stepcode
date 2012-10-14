#ifndef LAZYTYPES_H
#define LAZYTYPES_H

#include <map>
#include <unordered_map>
#include <iostream>
#include <vector>
// #include "sdaiApplication_instance.h"

class SDAI_Application_instance;
class sectionReader;
class lazyFileReader;

enum fileTypeEnum { Part21, Part28 };
// enum loadingEnum { immediate, lazy };

typedef int instanceID; ///< the number assigned to an instance in the file
typedef int sectionID;   ///< the index of a sectionReader in a sectionReaderVec_t
typedef int fileID;      ///< the index of a lazyFileReader in a lazyFileReaderVec_t

/** This struct contains all the information necessary to locate an instance. It is primarily
 * for instances that are present in a file but not memory. It could be useful in other
 * situations, so the information should be kept up-to-date.
 */
typedef struct {
    long begin/*, end*/;
    instanceID instance;
    sectionID section;
//     fileID file;
    /* bool modified; */ /* this will be useful when writing instances - if an instance is
    unmodified, simply copy it from the input file to the output file */
} lazyInstanceLoc;

/// used when populating the instance type map \sa lazyInstMgr::_instanceTypeMMap
typedef struct {
    lazyInstanceLoc loc;
    const char * name;
} namedLazyInstance;

// instanceRefMMap - multimap between an instanceID and instances that refer to it
typedef std::unordered_multimap< instanceID, instanceID > instanceRefMMap_t;
typedef std::pair< instanceID, instanceID > instanceRefMMap_pair;
typedef std::pair< instanceRefMMap_t::iterator, instanceRefMMap_t::iterator > instanceRefMMap_range;

// instanceTypeMMap - multimap from instance type to instanceID's
typedef std::unordered_multimap< std::string, instanceID > instanceTypeMMap_t;
typedef std::pair< std::string, instanceID > instanceTypeMMap_pair;
typedef std::pair< instanceTypeMMap_t::iterator, instanceTypeMMap_t::iterator > instanceTypeMMap_range;

// instancesLoaded - fully created instances
typedef std::map< instanceID, SDAI_Application_instance * > instancesLoaded_t;
typedef std::pair< instanceID, SDAI_Application_instance * > instancesLoaded_pair;

// instanceStreamPosMMap - map instance id to a range in a particular data section
// use multimap because there could be multiple instances with the same ID
typedef std::unordered_multimap< instanceID, lazyInstanceLoc > instanceStreamPosMMap_t;
typedef std::pair< instanceID, lazyInstanceLoc > instanceStreamPosMMap_pair;
typedef std::pair< instanceStreamPosMMap_t::iterator, instanceStreamPosMMap_t::iterator > instanceStreamPosMMap_range;


// data sections
typedef std::vector< sectionReader * > dataSectionReaderVec_t;

// files
typedef std::vector< lazyFileReader * > lazyFileReaderVec_t;

// type for performing actions on multiple instances
// NOTE not useful? typedef std::vector< lazyInstance > lazyInstanceVec_t;

#endif //LAZYTYPES_H
