#ifndef P21HEADERSECTIONREADER_H
#define P21HEADERSECTIONREADER_H

#include "headerSectionReader.h"

class p21HeaderSectionReader: public headerSectionReader {
public:
    p21HeaderSectionReader(std::ifstream* f/*, lazyFileReader* p*/ ): headerSectionReader( f/*, p */) {
    }
};

#endif //P21HEADERSECTIONREADER_H