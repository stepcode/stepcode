#ifndef LAZYDATASECTIONREADER_H
#define LAZYDATASECTIONREADER_H

#include <map>
#include <iostream>
// #include "fileInstance.h"
// #include "sdaiApplication_instance.h"
// #include "lazyFileReader.h"
// #include "lazyInstance.h"
#include "sectionReader.h"
// class lazyFileReader;
// class lazyInstance;
// class SDAI_Application_instance;

//typedefs for instanceStreamPosMap
typedef std::map< instanceID, streamRange > instanceStreamPosMap_t;
typedef std::pair< instanceID, streamRange > instanceStreamPosMap_pair;

/** base class for data section readers
 * \sa lazyP21DataSectionReader
 * \sa lazyP28DataSectionReader
 */
class lazyDataSectionReader: public sectionReader {
protected:
    /** map from instance number to beginning and end positions in the stream
     * \sa instanceStreamPosMap_pair
     */
    instanceStreamPosMap_t instanceStreamPosMap;
    bool error, completelyLoaded;
    std::ifstream* file;
//     lazyFileReader* parent;

    std::streampos sectionStart,  ///< the start of this section as reported by tellg()
                   sectionEnd,    ///< the end of this section as reported by tellg()
                   currentPos;    ///< current position (while scanning file and creating lazyInstance's)
    unsigned long /*loadedInstances,*/ totalInstances;

    /// only makes sense to call the ctor from derived class ctors
    lazyDataSectionReader(std::ifstream* f/*, lazyFileReader* p*/ ): file(f)/*, parent(p)*/ {
    }
public:
    bool success() {
        return !error;
    }
    std::ios::streampos startPos() const {
        return sectionStart;
    }
    std::ios::streampos endPos() const {
        return sectionEnd;
    }
    const streamRange nextInstance() {
        streamRange r;
        r.first = currentPos;
        //TODO find end
//         r.second = ...
        return r;
    }
    SDAI_Application_instance* getRealInstance( std::streampos start, std::streampos end );
};

#endif //LAZYDATASECTIONREADER_H