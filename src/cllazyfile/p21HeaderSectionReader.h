#ifndef P21HEADERSECTIONREADER_H
#define P21HEADERSECTIONREADER_H

#include "headerSectionReader.h"


class p21HeaderSectionReader: public headerSectionReader {
public:
    p21HeaderSectionReader( lazyFileReader * parent, std::ifstream * file, std::streampos start ): headerSectionReader( parent, file, start ) {
    }
//     std::streampos findSectionEnd() {
//
//     }
};

#endif //P21HEADERSECTIONREADER_H