#ifndef LAZYTYPES_H
#define LAZYTYPES_H

#include <map>
#include <iostream>
// #include "sdaiApplication_instance.h"

class SDAI_Application_instance;
class sectionReader;
class lazyFileReader;

typedef long instanceID; ///< the number assigned to an instance in the file
typedef int sectionID; ///< the index of a sectionReader in a sectionReaderVec_t

/** This struct contains all the information necessary to locate an instance. It is primarily
 * for instances that are present in a file but not memory. It could be useful in other
 * situations, so the information should be kept up-to-date.
 */
typedef struct {
    std::streampos begin, end;
    sectionID section;
    /* bool modified; */ /* this will be useful when writing instances - if an instance is
    unmodified, simply copy it from the input file to the output file */
} lazyInstance;

// instanceRefMMap - multimap between an instanceID and instances that refer to it
typedef std::multimap< instanceID, instanceID > instanceRefMMap_t;
typedef std::pair< instanceID, instanceID > instanceRefMMap_pair;
typedef std::pair< instanceRefMMap_t::iterator, instanceRefMMap_t::iterator > instanceRefMMap_range;

// instanceTypeMMap - multimap from instance type to instanceID's
typedef std::multimap< std::string, instanceID > instanceTypeMMap_t;
typedef std::pair< std::string, instanceID > instanceTypeMMap_pair;
typedef std::pair< instanceTypeMMap_t::iterator, instanceTypeMMap_t::iterator > instanceTypeMMap_range;

// instancesLoaded - fully created instances
typedef std::map< instanceID, SDAI_Application_instance * > instancesLoaded_t;
typedef std::pair< instanceID, SDAI_Application_instance * > instancesLoaded_pair;

// instanceStreamPosMap - map instance id to a range in a particular file
typedef std::map< instanceID, lazyInstance > instanceStreamPosMap_t;
typedef std::pair< instanceID, lazyInstance > instanceStreamPosMap_pair;

// data sections
typedef std::vector< sectionReader * > dataSectionReaderVec_t;

typedef std::vector< lazyFileReader * > lazyFileReaderVec_t;

#endif //LAZYTYPES_H
