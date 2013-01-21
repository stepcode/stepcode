#ifndef HEADERSECTIONREADER_H
#define HEADERSECTIONREADER_H

#include <iostream>
#include <fstream>

#include "judyL2Array.h"
#include "lazyFileReader.h"
#include "sectionReader.h"
#include "lazyTypes.h"
#include "scl_memmgr.h"


///differs from the lazyDataSectionReader in that all instances are always loaded
class headerSectionReader: public sectionReader {
protected:
    instancesLoaded_t * _headerInstances;

    /// must derive from this class
    headerSectionReader( lazyFileReader * parent, std::ifstream & file, std::streampos start, sectionID sid ):
                sectionReader( parent, file, start, sid ) {
        _headerInstances = new instancesLoaded_t;
    }
public:
    instancesLoaded_t * getInstances() const {
        return _headerInstances;
    }

    ~headerSectionReader() {
#ifdef HAVE_JUDY
        _headerInstances->clear();
#else //HAVE_JUDY
        int i = 0;
        instancesLoaded_t::iterator it = _headerInstances.begin();
        for( ; it != _headerInstances.end(); it++ ) {
            delete it->second;
            i++;
        }
        std::cerr << "deleted " << i << " header instances" << std::endl;
#endif //HAVE_JUDY
    }
};

#endif  //HEADERSECTIONREADER_H