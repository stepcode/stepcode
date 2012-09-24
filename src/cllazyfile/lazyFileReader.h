#ifndef LAZYFILEREADER_H
#define LAZYFILEREADER_H

#include <vector>
#include <string>
#include <cstdlib>

// PART 21
#include "lazyP21DataSectionReader.h"
#include "p21HeaderSectionReader.h"

/* // PART 28
 * #include "lazyP28DataSectionReader.h"
 * #include "p28HeaderSectionReader.h"
 */

class lazyInstMgr;
class Registry;
class headerSectionReader;

///read an exchange file of any supported type (currently only p21)
///for use only from within lazyInstMgr
class lazyFileReader {
//     friend class lazyInstMgr;
//     friend class sectionReader;
protected:
    std::string _fileName;
    lazyInstMgr * _parent;
    std::vector< lazyDataSectionReader * > _lazyDataReaders;
    headerSectionReader * _header;
    std::ifstream _file;
    fileTypeEnum _fileType;
    fileID _fileID;

    void initP21();    //TODO detect file type; for now, assume all are Part 21
    void detectType() {
        _fileType = Part21;
    }
public:
    fileID ID() const {
        return _fileID;
    }

    lazyFileReader( std::string fname, lazyInstMgr* i );

    fileTypeEnum type() const {
        return _fileType;
    }
    lazyInstMgr * getInstMgr() const {
        return _parent;
    }


};

#endif //LAZYFILEREADER_H

