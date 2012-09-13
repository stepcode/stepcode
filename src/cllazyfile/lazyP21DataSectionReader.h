#ifndef LAZYP21DATASECTIONREADER_H
#define LAZYP21DATASECTIONREADER_H

#include "lazyDataSectionReader.h"
#include "lazyFileReader.h"

class lazyP21DataSectionReader: public lazyDataSectionReader {
protected:
public:
    lazyP21DataSectionReader(std::ifstream* f/*, lazyFileReader* p */): lazyDataSectionReader( f/*, p */) {
    }
};

#endif //LAZYP21DATASECTIONREADER_H