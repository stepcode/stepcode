#ifndef HEADERSECTIONREADER_H
#define HEADERSECTIONREADER_H

#include <iostream>
#include <fstream>
#include "lazyFileReader.h"

// #include "SdaiSchemaInit.h"

class headerSectionReader: public sectionReader {
protected:
    std::ifstream* file;
    std::ios::streampos headerEnd;
//     lazyFileReader* parent;

    /// must derive from this class
    headerSectionReader(std::ifstream* f/*, lazyFileReader* p*/ ): file(f)/*, parent(p)*/ {
    }
public:
    // what functions?
    // one to get all contents, or to get next?
    // callback... probably
};

#endif  //HEADERSECTIONREADER_H