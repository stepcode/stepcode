#include <assert.h>


#include "cllazyfile/lazyFileReader.h"
#include "cllazyfile/lazyDataSectionReader.h"
#include "cllazyfile/headerSectionReader.h"
#include "cllazyfile/lazyInstMgr.h"

void lazyFileReader::initP21() {
    _header = new p21HeaderSectionReader( this, _file, 0, -1 );

    for( ;; ) {
        lazyDataSectionReader * r;
        r = new lazyP21DataSectionReader( this, _file, _file.tellg(), _parent->countDataSections() );
        if( !r->success() ) {
            delete r; //last read attempt failed
            std::cerr << "Corrupted data section" << std::endl;
            break;
        }
        _parent->registerDataSection( r );

        if( _parent->cancelled() ) break;

        //check for new data section (DATA) or end of file (END-ISO-10303-21;)
        for( ;; ) {
            while( isspace( _file.peek() ) && _file.good() ) _file.ignore( 1 );
            if( _file.peek() != '/' ) break;
            std::streampos comment = _file.tellg();
            _file.get();
            if( _file.peek() != '*' ) {
                _file.seekg( comment );
                break;
            }
            _file.get();
            int previous = 0;
            int current = 0;
            while( _file.good() ) {
                current = _file.get();
                if( previous == '*' && current == '/' ) break;
                previous = current;
            }
        }
        std::streampos nextSection = _file.tellg();
        if( needKW( "END-ISO-10303-21;" ) ) {
            break;
        }
        _file.clear();
        _file.seekg( nextSection );
        if( !needKW( "DATA" ) ) {
            std::cerr << "Corrupted file - did not find new data section (\"DATA\") or end of file (\"END-ISO-10303-21;\") at offset " << _file.tellg() << std::endl;
            break;
        }
        _file.clear();
        _file.seekg( nextSection );
    }
}

bool lazyFileReader::needKW( const char * kw ) {
    std::streampos start = _file.tellg();
    const char * c = kw;
    bool found = true;
    while( *c ) {
        if( *c != _file.get() ) {
            found = false;
            break;
        }
        c++;
    }
    if( !found ) {
        _file.clear();
        _file.seekg( start );
    }
    return found;
}

instancesLoaded_t * lazyFileReader::getHeaderInstances() {
    return _header->getInstances();
}

lazyFileReader::lazyFileReader( std::string fname, lazyInstMgr * i, fileID fid ): _fileName( fname ), _parent( i ),
        _header( 0 ), _fileID( fid ), _fileSize( 0 ), _valid( false ) {
    _file.open( _fileName.c_str(), std::ios::binary );
    _file.imbue( std::locale::classic() );
    _file.unsetf( std::ios_base::skipws );
    if( !_file.is_open() || !_file.good() ) {
        LazyDiagnostic diagnostic;
        diagnostic.severity = LAZY_DIAGNOSTIC_FATAL;
        diagnostic.message = "unable to open exchange file: " + _fileName;
        _parent->emitDiagnostic( diagnostic );
        return;
    }

    _file.seekg( 0, std::ios::end );
    std::streampos end = _file.tellg();
    if( end != std::streampos( -1 ) ) {
        _fileSize = static_cast<lazyFileOffset>( static_cast<std::streamoff>( end ) );
    }
    _file.seekg( 0, std::ios::beg );

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
    _valid = true;
}

lazyFileReader::~lazyFileReader() {
    delete _header;
}
