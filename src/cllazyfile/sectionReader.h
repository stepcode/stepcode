#ifndef SECTIONREADER_H
#define SECTIONREADER_H

// #include "lazyFileReader.h"
// #include "sdaiApplication_instance.h"
#include <iostream>
#include "lazyTypes.h"

class SDAI_Application_instance;
class lazyFileReader;

class sectionReader {
protected:
//     std::ifstream* file; //look this up in parent
    std::streampos _sectionStart,  ///< the start of this section as reported by tellg()
                   _sectionEnd,    ///< the end of this section as reported by tellg()
                   _currentPos;    ///< current position (while scanning file and creating lazyInstance's)
    unsigned long /*loadedInstances,*/ _totalInstances;
    std::ifstream* _file;

    lazyFileReader* _parent; //?
    sectionReader( lazyFileReader* parent, std::ifstream * file, std::streampos start );
public:
    SDAI_Application_instance * getRealInstance( lazyInstance * inst );
    //TODO add functions to find section begin/end (pure virtual), and to find instance begin/end
//     virtual std::streampos findSectionEnd() = 0;

    std::streampos startPos() const {
        return _sectionStart;
    }
    std::streampos endPos() const {
        return _sectionEnd;
    }
    const lazyInstance nextInstance() {
        lazyInstance i;
        i.begin = _currentPos;
        // i.section =
        // i.file =
        //TODO find end
        //         i.end = ...
        return i;
    }

};

#endif //SECTIONREADER_H
