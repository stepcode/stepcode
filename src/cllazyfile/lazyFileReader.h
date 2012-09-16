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

///read an exchange file of any supported type (currently only p21)
///for use only from within lazyInstMgr
class lazyFileReader {
//     friend class lazyInstMgr;
//     friend class sectionReader;
protected:
    lazyInstMgr* _parent;
    std::vector<lazyDataSectionReader*> _lazyDataReaders;
    headerSectionReader* _header;
    std::string _fileName;
    std::ifstream* _file;
    fileTypeEnum _fileType;
    void initP21() {
        _header = new p21HeaderSectionReader( this, _file, 0 );
        lazyDataSectionReader * r;

        //FIXME rework this? check for EOF instead of "success()"?
        while( r = new lazyP21DataSectionReader( this, _file, _file->tellg() ), r->success() ) {
            _lazyDataReaders.push_back(r);
        }
        delete r; //last read attempt failed
    }
    //TODO detect file type; for now, assume all are Part 21
    void detectType() {
        _fileType = Part21;
    }
public:
    lazyFileReader( std::string fname, lazyInstMgr* i ): _fileName(fname), _parent(i) {
        _file = new std::ifstream( _fileName.c_str() );
        detectType();
        switch( _fileType ) {
            case Part21:
                initP21();
                break;
            case Part28:
                //initP28();
                //break;
            default:
                std::cerr << "Reached default case, " << __FILE__ << ":" << __LINE__ << std::endl;
                abort();
        }
    }
    const fileTypeEnum type() const {
        return _fileType;
    }
//     const lazyInstMgr* getInstMgr() const {
//         return parent;
//     }

};

#endif //LAZYFILEREADER_H

