#ifndef LAZYTYPES_H
#define LAZYTYPES_H

#include <set>
#include <map>
#include <unordered_map>
#include <iostream>
#include <vector>

class SDAI_Application_instance;
class lazyDataSectionReader;
class lazyFileReader;

enum fileTypeEnum { Part21, Part28 };
// enum loadingEnum { immediate, lazy };

typedef int instanceID;  ///< the number assigned to an instance in the file
typedef int sectionID;   ///< the index of a sectionReader in a sectionReaderVec_t
typedef int fileID;      ///< the index of a lazyFileReader in a lazyFileReaderVec_t

typedef std::set< instanceID > * instanceRefs;

//TODO: create a "unique instance id" from the sectionID and instanceID, and use it everywhere?

/** This struct contains all the information necessary to locate an instance. It is primarily
 * for instances that are present in a file but not memory. It could be useful in other
 * situations, so the information should be kept up-to-date.
 */
typedef struct {
    long begin;
    instanceID instance;
    sectionID section;
    /* bool modified; */ /* this will be useful when writing instances - if an instance is
    unmodified, simply copy it from the input file to the output file */
} lazyInstanceLoc;

/// used when populating the instance type map \sa lazyInstMgr::_instanceTypeMMap
typedef struct {
    lazyInstanceLoc loc;
    const char * name;
    instanceRefs refs;
} namedLazyInstance;

// instanceRefMap - map between an instanceID and instances that refer to it
typedef std::unordered_map< instanceID, instanceRefs > instanceRefMap_t;
typedef std::pair< instanceID, instanceRefs > instanceRefMap_pair;
typedef std::pair< instanceRefMap_t::const_iterator, instanceRefMap_t::const_iterator > instanceRefMap_range;

// instanceTypeMMap - multimap from instance type to instanceID's
typedef std::unordered_multimap< std::string, instanceID > instanceTypeMMap_t;
typedef std::pair< std::string, instanceID > instanceTypeMMap_pair;
typedef std::pair< instanceTypeMMap_t::const_iterator, instanceTypeMMap_t::const_iterator > instanceTypeMMap_range;

// instancesLoaded - fully created instances
typedef std::map< instanceID, SDAI_Application_instance * > instancesLoaded_t;
typedef std::pair< instanceID, SDAI_Application_instance * > instancesLoaded_pair;

// instanceStreamPosMMap - map instance id to a streampos and data section
// use multimap because there could be multiple instances with the same ID
typedef std::unordered_multimap< instanceID, lazyInstanceLoc > instanceStreamPosMMap_t;
typedef std::pair< instanceID, lazyInstanceLoc > instanceStreamPosMMap_pair;
typedef std::pair< instanceStreamPosMMap_t::const_iterator, instanceStreamPosMMap_t::const_iterator > instanceStreamPosMMap_range;


// data sections
typedef std::vector< lazyDataSectionReader * > dataSectionReaderVec_t;

// files
typedef std::vector< lazyFileReader * > lazyFileReaderVec_t;

// type for performing actions on multiple instances
// NOTE not useful? typedef std::vector< lazyInstance > lazyInstanceVec_t;

#endif //LAZYTYPES_H
