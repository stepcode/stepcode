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

enum fileType_ { Part21, Part28 };

///read an exchange file of any supported type (currently only p21)
///for use only from within lazyInstMgr
class lazyFileReader {
    friend class lazyInstMgr;
private:
//     lazyInstMgr* parent;
    std::vector<lazyDataSectionReader*> lazyDataReaders;
    headerSectionReader* header;
    std::string fileName;
    std::ifstream* file;
    fileType_ fileType;
    void initP21() {
        header = new p21HeaderSectionReader( file/*, this*/ );
        lazyDataSectionReader * r;

        //FIXME rework this? check for EOF instead of "success()"?
        while( r = new lazyP21DataSectionReader( file/*, this*/ ), r->success() ) {
            lazyDataReaders.push_back(r);
        }
        delete r; //last read attempt failed
    }
    //TODO detect file type; for now, assume all are Part 21
    void detectType() {
        fileType = Part21;
    }
protected:
    lazyFileReader( std::string fname/*, lazyInstMgr* i*/ ): fileName(fname)/*, parent(i)*/ {
        file = new std::ifstream( fileName.c_str() );
        detectType();
        switch( fileType ) {
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
    const fileType_ type() const {
        return fileType;
    }
//     const lazyInstMgr* getInstMgr() const {
//         return parent;
//     }

};

#endif //LAZYFILEREADER_H

