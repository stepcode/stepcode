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
#include "lazyTypes.h"

/** base class for data section readers
 * \sa lazyP21DataSectionReader
 * \sa lazyP28DataSectionReader
 */
class lazyDataSectionReader: public sectionReader {
protected:
    bool _error, _completelyLoaded;
//     lazyFileReader* parent;
    std::string _sectionIdentifier;

    /// only makes sense to call the ctor from derived class ctors
    lazyDataSectionReader( lazyFileReader * parent, std::ifstream & file, std::streampos start, sectionID sid );
public:
    virtual ~lazyDataSectionReader() {}
    bool success() {
        return !_error;
    }
//     SDAI_Application_instance* getRealInstance( std::streampos start, std::streampos end );
};

#endif //LAZYDATASECTIONREADER_H