#include <assert.h>


#include "lazyFileReader.h"
#include "lazyDataSectionReader.h"
#include "lazyInstMgr.h"

void lazyFileReader::initP21() {
    _header = new p21HeaderSectionReader( this, _file, 0 );
    lazyDataSectionReader * r;

    //FIXME rework this? check for EOF instead of "success()"?
    while( r = new lazyP21DataSectionReader( this, _file, _file.tellg() ), r->success() ) {
        _lazyDataReaders.push_back(r);
    }
    delete r; //last read attempt failed
}

lazyFileReader::lazyFileReader( std::string fname, lazyInstMgr* i ): _fileName(fname), _parent(i) {
    _fileID = _parent->registerLazyFile( this );

    _file.open( _fileName.c_str() );
    _file.imbue( std::locale::classic() );
    _file.unsetf( std::ios_base::skipws );
    assert( _file.is_open() && _file.good() );

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

