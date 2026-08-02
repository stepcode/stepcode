#ifndef LAZYP21DATASECTIONREADER_H
#define LAZYP21DATASECTIONREADER_H

#include "cllazyfile/lazyDataSectionReader.h"
#include "cllazyfile/lazyFileReader.h"
#include "sc_export.h"

class SC_LAZYFILE_EXPORT lazyP21DataSectionReader: public lazyDataSectionReader {
    protected:
        /** Index the instances in an Edition 1 SCOPE and leave the stream at
         * the owning entity's record. */
        bool indexScope();
    public:
        lazyP21DataSectionReader( lazyFileReader * parent, std::ifstream & file, std::streampos start, sectionID sid );

        void findSectionStart();
        /** gets information (start, end, name, etc) about the next
         * instance in the file and returns it in a namedLazyInstance
         * \sa p21HeaderSectionReader::nextInstance()
         */
        const namedLazyInstance nextInstance();

};

#endif //LAZYP21DATASECTIONREADER_H
