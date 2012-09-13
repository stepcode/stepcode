#ifndef SECTIONREADER_H
#define SECTIONREADER_H

// #include "lazyFileReader.h"
#include "sdaiApplication_instance.h"
#include <iostream>


class sectionReader {
protected:
    std::ifstream* file;
    std::streampos sectionStart,  ///< the start of this section as reported by tellg()
    sectionEnd,    ///< the end of this section as reported by tellg()
    currentPos;    ///< current position (while scanning file and creating lazyInstance's)
    unsigned long /*loadedInstances,*/ totalInstances;

    //     lazyFileReader* parent; //?
public:
    SDAI_Application_instance* getRealInstance( std::streampos start, std::streampos end );
    //TODO add functions to find section begin/end (pure virtual), and to find instance end

    std::streampos startPos() const {
        return sectionStart;
    }
    std::streampos endPos() const {
        return sectionEnd;
    }
    const streamRange nextInstance() {
        streamRange r;
        r.first = currentPos;
        //TODO find end
        //         r.second = ...
        return r;
    }

};

#endif //SECTIONREADER_H
