#ifndef HEADERSECTIONREADER_H
#define HEADERSECTIONREADER_H

#include <iostream>
#include <fstream>
#include "lazyFileReader.h"
#include "sectionReader.h"
#include "lazyTypes.h"

///differs from the lazyDataSectionReader in that all instances are always loaded
class headerSectionReader: public sectionReader {
protected:
    instancesLoaded_t _headerInstances;

    /// must derive from this class
    headerSectionReader( lazyFileReader * parent, std::ifstream & file, std::streampos start, sectionID sid ):
                sectionReader( parent, file, start, sid ) {
    }
public:
    instancesLoaded_t getInstances() {
        return _headerInstances;
    }

    ~headerSectionReader() {
        int i = 0;
        instancesLoaded_t::iterator it = _headerInstances.begin();
        for( ; it != _headerInstances.end(); it++ ) {
            delete it->second;
            i++;
        }
        std::cerr << "deleted " << i << " header instances" << std::endl;
    }
};

#endif  //HEADERSECTIONREADER_H