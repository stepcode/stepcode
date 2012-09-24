#ifndef LAZYP21DATASECTIONREADER_H
#define LAZYP21DATASECTIONREADER_H

#include "lazyDataSectionReader.h"
#include "lazyFileReader.h"

class lazyP21DataSectionReader: public lazyDataSectionReader {
protected:
public:
    lazyP21DataSectionReader( lazyFileReader * parent, std::ifstream & file, std::streampos start ): lazyDataSectionReader( parent, file, start ) {
    }

    void findSectionStart() {
        findString( "DATA", true );
    }

};

#endif //LAZYP21DATASECTIONREADER_H