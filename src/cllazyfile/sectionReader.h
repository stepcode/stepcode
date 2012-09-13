#ifndef SECTIONREADER_H
#define SECTIONREADER_H

// #include "lazyFileReader.h"
#include "sdaiApplication_instance.h"
#include "sdaiApplication_instance_set.h"

class sectionReader {
protected:
//     lazyFileReader* parent; //?
public:
    SDAI_Application_instance* getRealInstance( std::streampos start, std::streampos end );
    //TODO add functions to find section begin/end (pure virtual), and to find instance end

};

#endif //SECTIONREADER_H
