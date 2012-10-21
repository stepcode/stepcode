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
    //nothing public
};

#endif  //HEADERSECTIONREADER_H