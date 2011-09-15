
/*
* NIST STEP Core Class Library
* cleditor/STEPfile.inline.cc
* April 1997
* Peter Carr
* K. C. Morris
* David Sauder

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

#include <STEPfile.h>
#include <s_HEADER_SCHEMA.h>
#include <STEPaggregate.h>

#include <cstring>

extern "C" {
    double  ceil( double );
}


extern void HeaderSchemaInit( Registry & reg );

//To Be inline functions

//constructor & destructor

STEPfile::STEPfile( Registry & r, InstMgr & i, const std::string filename ) :
    _instances( i ), _reg( r ), _fileIdIncr( 0 ), _headerId( 0 ),
    _entsNotCreated( 0 ), _entsInvalid( 0 ), _entsIncomplete( 0 ),
    _entsWarning( 0 ), _errorCount( 0 ), _warningCount( 0 ),
    _maxErrorCount( 5000 )
{
    SetFileType( VERSION_CURRENT );
    SetFileIdIncrement();
    _currentDir = new DirObj( "" );
    _headerRegistry = new Registry( HeaderSchemaInit );
    _headerInstances = new InstMgr;
    if( !filename.empty() ) {
        ReadExchangeFile( filename );
    }
}

STEPfile::~STEPfile() {
    delete _currentDir;

    // remove everything from the Registry before deleting it
    _headerRegistry->DeleteContents();
    delete _headerRegistry;

    _headerInstances->DeleteInstances();
    delete _headerInstances;
}

int STEPfile::SetFileType( FileTypeCode ft ) {
    FileType( ft );

    switch( _fileType ) {
        case( VERSION_OLD ):
            ENTITY_NAME_DELIM = '@';
            FILE_DELIM = ( char * )"STEP;";
            END_FILE_DELIM = ( char * )"ENDSTEP;";
            break;
        case( VERSION_UNKNOWN ):
        case( VERSION_CURRENT ):
            ENTITY_NAME_DELIM = '#';
            FILE_DELIM = ( char * )"ISO-10303-21;";
            END_FILE_DELIM = ( char * )"END-ISO-10303-21;";
            break;
        case( WORKING_SESSION ):
            ENTITY_NAME_DELIM = '#';
            FILE_DELIM = ( char * )"STEP_WORKING_SESSION;";
            END_FILE_DELIM = ( char * )"END-STEP_WORKING_SESSION;";
            break;

        default:
            // some kind of error
            cerr << "Internal error:  " << __FILE__ <<  __LINE__
                 << "\n" << _POC_ "\n";
            return 0;
    }
    return 1;
}


/******************************************************
** remove any slashes, and anything before the slash,
** from filename
*/
const std::string STEPfile::TruncFileName( const std::string filename ) const {
#if defined(__WIN32__) && !defined(__mingw32__)
    char slash = '\\';
#else
    char slash = '/';
#endif
    size_t l = filename.find_last_of(slash);
    if( l == std::string::npos ) {
        return filename;
    } else {
        return filename.substr(l);
    }
}


/******************************************************/
Severity STEPfile::ReadExchangeFile( const std::string filename, int useTechCor ) {
    _error.ClearErrorMsg();
    _errorCount = 0;
    istream * in = OpenInputFile( filename );
    if( _error.severity() < SEVERITY_WARNING ) {
        CloseInputFile( in );
        return _error.severity();
    }

    instances().ClearInstances();
    if( _headerInstances ) {
        _headerInstances->ClearInstances();
    }
    _headerId = 5;
    Severity rval = AppendFile( in, useTechCor );
    CloseInputFile( in );
    return rval;
}

Severity STEPfile::AppendExchangeFile( const std::string filename, int useTechCor ) {
    _error.ClearErrorMsg();
    _errorCount = 0;
    istream * in = OpenInputFile( filename );
    if( _error.severity() < SEVERITY_WARNING ) {
        CloseInputFile( in );
        return _error.severity();
    }
    Severity rval = AppendFile( in, useTechCor );
    CloseInputFile( in );
    return rval;
}

/******************************************************/
Severity STEPfile::ReadWorkingFile( const std::string filename, int useTechCor ) {
    _error.ClearErrorMsg();
    _errorCount = 0;
    istream * in = OpenInputFile( filename );
    if( _error.severity() < SEVERITY_WARNING ) {
        CloseInputFile( in );
        return _error.severity();
    }

    instances().ClearInstances();
    _headerInstances->ClearInstances();
    SetFileType( WORKING_SESSION );

    Severity rval = AppendFile( in, useTechCor );
    SetFileType();
    CloseInputFile( in );
    return rval;
}


Severity STEPfile::AppendWorkingFile( const std::string filename, int useTechCor ) {
    _error.ClearErrorMsg();
    _errorCount = 0;
    istream * in = OpenInputFile( filename );
    if( _error.severity() < SEVERITY_WARNING ) {
        CloseInputFile( in );
        return _error.severity();
    }
    SetFileType( WORKING_SESSION );
    Severity rval = AppendFile( in, useTechCor );
    SetFileType();
    CloseInputFile( in );
    return rval;
}



/******************************************************/
istream * STEPfile::OpenInputFile( const std::string filename ) {
    //  if there's no filename to use, fail
    if( filename.empty() && FileName().empty() ) {
        _error.AppendToUserMsg( "Unable to open file for input. No current file name.\n" );
        _error.GreaterSeverity( SEVERITY_INPUT_ERROR );
        return( 0 );
    } else {
        if( SetFileName( filename ).empty() && ( filename.compare( "-" ) != 0 ) ) {
            char msg[BUFSIZ];
            sprintf( msg, "Unable to find file for input: \'%s\'. File not read.\n", filename.c_str() );
            _error.AppendToUserMsg( msg );
            _error.GreaterSeverity( SEVERITY_INPUT_ERROR );
            return( 0 );
        }
    }

    std::istream * in;

    if( filename.compare( "-" ) == 0 ) {
        in = &std::cin;
    } else {
        in = new ifstream( FileName().c_str() );
    }

    if( !in || !( in -> good() ) ) {
        char msg[BUFSIZ];
        sprintf( msg, "Unable to open file for input: \'%s\'. File not read.\n", filename.c_str() );
        _error.AppendToUserMsg( msg );
        _error.GreaterSeverity( SEVERITY_INPUT_ERROR );
        return ( 0 );
    }

    return in;
}

/******************************************************/
void STEPfile::CloseInputFile( istream * in ) {
    if( in && *in != std::cin ) {
        delete in;
    }
}


/******************************************************/
ofstream * STEPfile::OpenOutputFile( const std::string filename ) {
    if( filename.empty() ) {
        if( FileName().empty() ) {
            _error.AppendToUserMsg( "No current file name.\n" );
            _error.GreaterSeverity( SEVERITY_INPUT_ERROR );
        }
    } else {
        if( SetFileName( filename ).empty() ) {
            char msg[BUFSIZ];
            sprintf( msg, "can't find file: %s\nFile not written.\n", filename.c_str() );
            _error.AppendToUserMsg( msg );
            _error.GreaterSeverity( SEVERITY_INPUT_ERROR );
        }
    }

    if( _currentDir->FileExists( TruncFileName( filename ) ) ) {
        MakeBackupFile();
    }
    ofstream * out  = new ofstream( filename.c_str() );
    if( !out ) {
        _error.AppendToUserMsg( "unable to open file for output\n" );
        _error.GreaterSeverity( SEVERITY_INPUT_ERROR );
    }
    return out;
}

void STEPfile::CloseOutputFile( ostream * out ) {
    delete out;
}



/******************************************************/
int STEPfile::IncrementFileId( int fileid ) {
    return ( fileid + FileIdIncr() );
}


void STEPfile::SetFileIdIncrement() {
    if( instances().MaxFileId() < 0 ) {
        _fileIdIncr = 0;
    } else _fileIdIncr =
            ( int )( ( ceil( ( double )( ( instances().MaxFileId() + 99 ) / 1000 ) ) + 1 ) * 1000 );
    // FIXME: Is this correct? Why put an integer expression into ceil()?
}

char * STEPfile::schemaName( char * schName )
/*
 * Returns the schema name from the file schema header section (or the 1st
 * one if more than one exists).  Copies this value into schName.  If there
 * is no header section or no value for file schema, NULL is returned and
 * schName is unset.
 */
{
    p21DIS_File_schema * fs;
    std::string tmp;
    STEPnode * n;

    if( _headerInstances == NULL ) {
        return NULL;
    }
    fs = ( p21DIS_File_schema * )_headerInstances->GetApplication_instance( "File_Schema" );
    if( fs == ENTITY_NULL ) {
        return NULL;
    }

    n = ( STEPnode * )fs->schema_identifiers().GetHead();
    // (take the first one)
    if( n == NULL ) {
        return NULL;
    }
    n->STEPwrite( tmp );
    if( *tmp.c_str() == '\0' || *tmp.c_str() == '$' ) {
        return NULL;
    }
    // tmp.c_str() returns the string we want plus a beginning and ending
    // quote mark (').  We remove these below.
    strncpy( schName, tmp.c_str() + 1, BUFSIZ - 1 );
    // "+1" to remove beginning '.
    if( *( schName + strlen( schName ) - 1 ) == '\'' ) {
        // Remove trailing '.  This condition checks that it wasn't removed
        // already.  That may have happend if strncpy had truncated schName
        // (it were >= BUFSIZ).
        *( schName + strlen( schName ) - 1 ) = '\0';
    }
    return schName;
}
