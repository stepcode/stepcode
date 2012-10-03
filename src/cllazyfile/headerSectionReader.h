#ifndef HEADERSECTIONREADER_H
#define HEADERSECTIONREADER_H

#include <iostream>
#include <fstream>
#include "lazyFileReader.h"
#include "sectionReader.h"
#include "lazyTypes.h"

// #include "SdaiSchemaInit.h"
//differs from the lazyDataSectionReader in that all instances are always loaded
class headerSectionReader: public sectionReader {
protected:
//     std::ios::streampos headerEnd;
//     lazyFileReader* parent;
    instancesLoaded_t _headerInstances;

    /// must derive from this class
    headerSectionReader( lazyFileReader * parent, std::ifstream & file, std::streampos start ): sectionReader( parent, file, start ) {
    }
public:
//     void findSectionStart();
    // what functions?
    // one to get all contents, or to get next?
    // callback... probably
};

#endif  //HEADERSECTIONREADER_H