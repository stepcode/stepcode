/*
* NIST STEP Core Class Library
* cleditor/STEPfile.cc
* February, 1994
* Peter Carr
* K. C. Morris
* David Sauder

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

/********************************************************************
 TODO LIST:
 - ReadHeader doesn't merge the information from more than one instance
   of the same entity type name. i.e. it doesn't merge info from multiple files
 - ReadWorkingFile does not handle references to entities which
   aren't in the working session file. for other *incomplete* instances,
   it prints error messages to cerr which may not need to be printed. It
   doesn't keep track of the incomplete instances, it just puts them on the
   list according to the symbol which precedes them in the working session
   file
**********************************************************************/
#include <iostream>
#include <iterator>
#include <algorithm>

#include <STEPfile.h>
#include <sdai.h>
#include <STEPcomplex.h>
#include <STEPattribute.h>
#include <SdaiHeaderSchema.h>

// STEPundefined contains
// void PushPastString (istream& in, std::string &s, ErrorDescriptor *err)
#include <STEPundefined.h>

#include "scl_memmgr.h"

/***************************
function:     SetFileName
returns:      (const std::string) The new file name for the class.
parameters:   (const std::string) The file name to be set.
description:
   This function sets the _fileName member variable to an
   absolute path name to a directory on the file system. If newName
   cannot be resolved to a file, then an empty string is returned.
side effects: STEPfile::_fileName value may change.
***************************/
const std::string STEPfile::SetFileName( const std::string newName ) {
    //  if a newName is not given or is the same as the old, use the old name
    if( ( newName.empty() ) || ( newName == _fileName ) ) {
        return FileName();
    }

    _fileName = DirObj::Normalize( newName );
    return _fileName;
}

/***************************
function:     ReadHeader
returns:      Severity
parameters:   (istream&) The input stream from which the file is read.
description:
   This function reads in the header section of an exchange file. It
   parses the header section, popluates the _headerInstances, and
   returns an error descriptor.
   It expects to find the "HEADER;" symbol at the beginning of the
   istream.
side effects: The function  gobbles all characters up to and including the
   next "ENDSEC;" from in.
   The STEPfile::_headerInstances may change.
***************************/

Severity
STEPfile::ReadHeader( istream & in ) {
    std::string cmtStr;

    InstMgr * im = new InstMgr;
    SDAI_Application_instance * obj;
#if defined(__MSVC__) || defined(__BORLAND__)
    Severity objsev = SEVERITY_NULL;
#else
    __attribute__( ( unused ) ) Severity objsev = SEVERITY_NULL;
#endif

    int endsec = 0;
    int userDefined = 0;
    int fileid;
    std::string keywd;
    char c = '\0';
    char buf [BUFSIZ];

    std::string strbuf;

    ReadTokenSeparator( in );

    // Read and gobble all 'junk' up to "HEADER;"
    if( !FindHeaderSection( in ) ) {
        delete im;
        return SEVERITY_INPUT_ERROR;
    }

    //read the header instances
    while( !endsec ) {
        ReadTokenSeparator( in, &cmtStr );
        if( in.eof() ) {
            _error.AppendToDetailMsg( "End of file reached in reading header section.\n" );
            _error.GreaterSeverity( SEVERITY_EXIT );
            delete im;
            return SEVERITY_EXIT;
        }

        //check for user defined instances
        //if it is userDefined, the '!' does not get put back on the istream
        in.get( c );
        if( c == '!' ) {
            userDefined = 1;
        } else          {
            in.putback( c );
        }

        //get the entity keyword
        keywd = GetKeyword( in, ";( /\\", _error );
        ReadTokenSeparator( in, &cmtStr );

        //check for "ENDSEC"
        if( !strncmp( const_cast<char *>( keywd.c_str() ), "ENDSEC", 7 ) ) {
            //get the token delimiter
            in.get( c ); //should be ';'
            endsec = 1;
            break; //from while-loop
        } else {
            //create and read the header instance
            // SDAI_Application_instance::STEPread now reads the opening parenthesis

            //create header instance
            buf[0] = '\0';
            if( _fileType == VERSION_OLD ) {
                _error.AppendToDetailMsg( "N279 header detected. Files this old are no longer supported.\n" );
                _error.GreaterSeverity( SEVERITY_EXIT );
                delete im;
                return SEVERITY_EXIT;
            } else {
                strncpy( buf, const_cast<char *>( keywd.c_str() ), BUFSIZ );
            }

            if( userDefined ) {
                //create user defined header instance
                // BUG: user defined entities are ignored
                //obj = _headerUserDefined->ObjCreate (buf);
                //objsev = AppendEntityErrorMsg( &(obj->Error()) );

                SkipInstance( in, strbuf );
                cerr << "User defined entity in header section " <<
                     "is ignored.\n\tdata lost: !" <<  buf << strbuf << "\n";
                _error.GreaterSeverity( SEVERITY_WARNING );
                break; //from while loop
            } else { //not userDefined
                obj = _headerRegistry->ObjCreate( buf );
            }

            //read header instance
            if( !obj || ( obj == ENTITY_NULL ) ) {
                ++_errorCount;
                SkipInstance( in, strbuf );
                cerr << "Unable to create header section entity: \'" <<
                     keywd << "\'.\n\tdata lost: " << strbuf << "\n";
                _error.GreaterSeverity( SEVERITY_WARNING );
            } else { //not ENTITY_NULL
                //read the header instance

                //check obj's Error Descriptor
                objsev = AppendEntityErrorMsg( &( obj->Error() ) );

                //set file_id to reflect the appropriate Header Section Entity
                fileid = HeaderId( keywd );

                //read the values from the istream
                objsev = obj->STEPread( fileid, 0, ( InstMgr * )0, in, NULL, true, _strict );
                if( !cmtStr.empty() ) {
                    obj->PrependP21Comment( cmtStr );
                }

                in >> ws;
                c = in.peek(); // check for semicolon or keyword 'ENDSEC'
                if( c != 'E' ) {
                    in >> c;    // read the semicolon
                }

                //check to see if object was successfully read
                AppendEntityErrorMsg( &( obj->Error() ) );
                //append to header instance manager
                im->Append( obj, completeSE );
            }
        }
        cmtStr.clear();
    }

    HeaderVerifyInstances( im );
    HeaderMergeInstances( im ); // handles delete for im

    return _error.severity();
}

/***************************
Verify the instances read from the header section of an exchange file.
If some required instances aren't present, then create them,
and populate them with default values.
The required instances are:
  #1 = FILE_DESCRIPTION
  #2 = FILE_NAME
  #3 = FILE_SCHEMA
***************************/
Severity
STEPfile::HeaderVerifyInstances( InstMgr * im ) {
    int err = 0;
    int fileid;
    SDAI_Application_instance * obj;

    //check File_Name
    fileid = HeaderId( "File_Name" );
    if( !( im->FindFileId( fileid ) ) ) {
        ++err;
        cerr << "FILE_NAME instance not found in header section\n";
        // create a File_Name entity and assign default values
        obj = HeaderDefaultFileName();
        im->Append( obj, completeSE );
    }

    //check File_Description
    fileid = HeaderId( "File_Description" );
    if( !( im->FindFileId( fileid ) ) ) {
        ++err;
        cerr << "FILE_DESCRIPTION instance not found in header section\n";
        // create a File_Description entity and assign default values
        obj = HeaderDefaultFileDescription();
        im->Append( obj, completeSE );
    }

    //check File_Schema
    fileid = HeaderId( "File_Schema" );
    if( !( im->FindFileId( fileid ) ) ) {
        ++err;
        cerr << "FILE_SCHEMA instance not found in header section\n";
        // create a File_Schema entity and read in default values
        obj = HeaderDefaultFileSchema();
        im->Append( obj, completeSE );
    }

    if( !err ) {
        return SEVERITY_NULL;
    }
    _error.AppendToUserMsg( "Missing required entity in header section.\n" );
    _error.GreaterSeverity( SEVERITY_WARNING );
    return SEVERITY_WARNING;
}

SDAI_Application_instance *
STEPfile::HeaderDefaultFileName() {
    SdaiFile_name * fn = new SdaiFile_name;
    StringAggregate_ptr tmp = new StringAggregate;

    fn->name_( "" );
    fn->time_stamp_( "" );
    tmp->StrToVal( "", &_error,
                   fn->attributes[2].
                   aDesc -> DomainType(),
                   _headerInstances );
    fn->author_( tmp );

    tmp->StrToVal( "", &_error,
                   fn->attributes[3].
                   aDesc -> DomainType(),
                   _headerInstances );
    fn->organization_( tmp );

    fn->preprocessor_version_( "" );
    fn->originating_system_( "" );
    fn->authorization_( "" );

    fn->STEPfile_id = HeaderId( "File_Name" );

    return fn;
}

SDAI_Application_instance *
STEPfile::HeaderDefaultFileDescription() {
    SdaiFile_description * fd = new SdaiFile_description;

    fd->implementation_level_( "" );

    fd->STEPfile_id = HeaderId( "File_Description" );

    return fd;
}

SDAI_Application_instance *
STEPfile::HeaderDefaultFileSchema() {
    SdaiFile_schema * fs = new SdaiFile_schema;
    StringAggregate_ptr tmp = new StringAggregate;

    tmp->StrToVal( "", &_error,
                   fs->attributes[0].
                   aDesc -> DomainType(),
                   _headerInstances );
    fs->schema_identifiers_( tmp );

    fs->STEPfile_id = HeaderId( "File_Schema" );

    return fs;
}



/***************************
This function has an effect when more than one file
is being read into a working session.

This function manages space allocation for the Instance
Manager for the header instances. If the instances in im are
copied onto the instances of _headerInstances, then im is
deleted. Otherwise no space is deleted.

Append the values of the given instance manager onto the _headerInstances.
If the _headerInstances contain no instances, then copy the instances
from im onto the _headerInstances.
This only works for an instance manager which contains the following
header section entites. The file id numbers are important.

  #1 = FILE_DESCRIPTION
  #2 = FILE_NAME
  #3 = FILE_SCHEMA
***************************/
void
STEPfile::HeaderMergeInstances( InstMgr * im ) {
    SDAI_Application_instance * se = 0;
    SDAI_Application_instance * from = 0;

    int idnum;

    //check for _headerInstances
    if( !_headerInstances ) {
        _headerInstances = im;
        return;
    }

    if( _headerInstances->InstanceCount() < 4 ) {
        delete _headerInstances;
        _headerInstances = im;
        return;
    }

    //checking for _headerInstances::FILE_NAME
    idnum = HeaderId( "File_Name" );
    if( ( se = _headerInstances->GetApplication_instance( _headerInstances->FindFileId( idnum ) ) ) ) {
        from = im->GetApplication_instance( im->FindFileId( idnum ) );

        // name:
        // time_stamp: keep the newer time_stamp
        // author: append the list of authors
        // organization: append the organization list
        // preprocessor_version:
        // originating_system:
        // authorization:
    } else { // No current File_Name instance
        from = im->GetApplication_instance( im->FindFileId( idnum ) );
        _headerInstances->Append( from, completeSE );
    }

    //checking for _headerInstances::FILE_DESCRIPTION
    idnum = HeaderId( "File_Description" );
    if( ( se = _headerInstances->GetApplication_instance( _headerInstances->FindFileId( idnum ) ) ) ) {
        from = im->GetApplication_instance( im->FindFileId( idnum ) );

        //description
        //implementation_level
    } else {
        from = im->GetApplication_instance( im->FindFileId( idnum ) );
        _headerInstances->Append( from, completeSE );
    }

    //checking for _headerInstances::FILE_SCHEMA
    idnum = HeaderId( "File_Schema" );
    if( ( se = _headerInstances->GetApplication_instance( _headerInstances->FindFileId( idnum ) ) ) ) {
        from = im->GetApplication_instance( im->FindFileId( idnum ) );

        //description
        //implementation_level
    } else {
        from = im->GetApplication_instance( im->FindFileId( idnum ) );
        _headerInstances->Append( from, completeSE );
    }

    delete im;
    return;
}


stateEnum
STEPfile::EntityWfState( char c ) {
    switch( c ) {
        case wsSaveComplete:
            return completeSE;
//  break;

        case wsSaveIncomplete:
            return incompleteSE;
//  break;

        case wsDelete:
            return deleteSE;
//  break;

        case wsNew:
            return newSE;
//  break;

        default:
            return noStateSE;
    }
}

/***************************
***************************/
//  PASS 1:  create instances
//  starts at the data section

int
STEPfile::ReadData1( istream & in ) {
    int endsec = 0;
    _entsNotCreated = 0;

    _errorCount = 0;  // reset error count
    _warningCount = 0;  // reset error count

    char c;
    int instance_count = 0;
    char buf[BUFSIZ];
    buf[0] = '\0';
    std::string tmpbuf;

    SDAI_Application_instance * obj = ENTITY_NULL;
    stateEnum inst_state = noStateSE; // used if reading working file

    ErrorDescriptor e;

    //  PASS 1:  create instances
    endsec = FoundEndSecKywd( in, _error );
    while( in.good() && !endsec ) {
        e.ClearErrorMsg();
        ReadTokenSeparator( in ); // also skips white space
        in >> c;

        if( _fileType == WORKING_SESSION ) {
            if( strchr( "CIND", c ) ) { // if there is a valid char
                inst_state = EntityWfState( c );
                ReadTokenSeparator( in );
                in >> c;    // read the ENTITY_NAME_DELIM
            } else {
                e.AppendToDetailMsg( "Invalid editing state character: " );
                e.AppendToDetailMsg( c );
                e.AppendToDetailMsg( "\nAssigning editing state to be INCOMPLETE\n" );
                e.GreaterSeverity( SEVERITY_WARNING );
                inst_state = incompleteSE;
            }
        }

        if( c != ENTITY_NAME_DELIM ) {
            in.putback( c );
            while( c != ENTITY_NAME_DELIM && in.good() &&
                    !( endsec = FoundEndSecKywd( in, _error ) ) ) {
                tmpbuf.clear();
                FindStartOfInstance( in, tmpbuf );
                cout << "ERROR: trying to recover from invalid data. skipping: "
                     << tmpbuf << endl;
                in >> c;
                ReadTokenSeparator( in );
            }
        }

        if( !endsec ) {
            obj = ENTITY_NULL;
            if( ( _fileType == WORKING_SESSION ) && ( inst_state == deleteSE ) ) {
                SkipInstance( in, tmpbuf );
            } else {
                obj =  CreateInstance( in, cout );
            }

            if( obj != ENTITY_NULL ) {
                if( obj->Error().severity() < SEVERITY_WARNING ) {
                    ++_errorCount;
                } else if( obj->Error().severity() < SEVERITY_NULL ) {
                    ++_warningCount;
                }
                obj->Error().ClearErrorMsg();

                if( _fileType == WORKING_SESSION ) {
                    instances().Append( obj, inst_state );
                } else {
                    instances().Append( obj, newSE );
                }

                ++instance_count;
            } else {
                ++_entsNotCreated;
                //old
                ++_errorCount;
            }

            if( _entsNotCreated > _maxErrorCount ) {
                _error.AppendToUserMsg( "Warning: Too Many Errors in File. Read function aborted.\n" );
                cerr << Error().UserMsg();
                cerr << Error().DetailMsg();
                Error().ClearErrorMsg();
                Error().severity( SEVERITY_EXIT );
                return instance_count;
            }

            endsec = FoundEndSecKywd( in, _error );

        }
    } // end while loop

    if( _entsNotCreated ) {
        // new
        sprintf( buf,
                 "STEPfile Reading File: Unable to create %d instances.\n\tIn first pass through DATA section. Check for invalid entity types.\n",
                 _entsNotCreated );
        // old
        /*
            sprintf(buf,
            "STEPfile Reading File: Unable to create %d instances.\n\tIn first pass through DATA section. Check for invalid entity types.\n",
                _errorCount);
        */
        _error.AppendToUserMsg( buf );
        _error.GreaterSeverity( SEVERITY_WARNING );
    }
    if( !in.good() ) {
        _error.AppendToUserMsg( "Error in input file.\n" );
    }

    return instance_count;
}

#if bzpoibujqoiwejsdlfj

int
STEPfile::ReadData1( istream & in ) {
    int endsec = 0;
    _entsNotCreated = 0;

    _errorCount = 0;  // reset error count
    _warningCount = 0;  // reset error count

    char c;
    int instance_count = 0;
    char buf[BUFSIZ];
    buf[0] = '\0';
    std::string tmpbuf;

    SDAI_Application_instance * obj;
    stateEnum inst_state; // used if reading working file

    ReadTokenSeparator( in ); // also skips white space
    c = in.peek();
///    in >> c;

    if( _fileType == WORKING_SESSION ) {
        if( strchr( "CIND", c ) ) { // if there is a valid editing state char
            in >> c; // read the editing state char from the stream
            inst_state = EntityWfState( c );
        } else {
            cout << "Invalid editing state character: " << c << endl;
            cout << "Assigning editing state to be INCOMPLETE\n";
            inst_state = incompleteSE;
        }
        ReadTokenSeparator( in );
        c = in.peek();  // read the ENTITY_NAME_DELIM
        // else the character should be ENTITY_NAME_DELIM
    }
    while( c != ENTITY_NAME_DELIM && in.good() ) {
        tmpbuf.set_null();
        SkipInstance( in, tmpbuf );
        cout << "ERROR: trying to recover from invalid data. skipping: "
             << tmpbuf << endl;
        ReadTokenSeparator( in );
    }

    //  PASS 1:  create instances
    while( c == ENTITY_NAME_DELIM && in.good() && !endsec ) {
        if( ( _fileType == WORKING_SESSION ) && ( inst_state == deleteSE ) ) {
            SkipInstance( in, tmpbuf );
        } else {
            obj =  CreateInstance( in, cout );
        }

        if( obj != ENTITY_NULL ) {
            if( obj->Error().severity() < SEVERITY_WARNING ) {
                ++_errorCount;
            } else if( obj->Error().severity() < SEVERITY_NULL ) {
                ++_warningCount;
            }
            obj->Error().ClearErrorMsg();

            if( _fileType == WORKING_SESSION ) {
                instances().Append( obj, inst_state );
            } else {
                instances().Append( obj, newSE );
            }

            ++instance_count;
        } else {
            ++_entsNotCreated;
            //old
            ++_errorCount;
        }

        if( _entsNotCreated > _maxErrorCount ) {
            _error.AppendToUserMsg( "Warning: Too Many Errors in File. Read function aborted.\n" );
            cerr << Error().UserMsg();
            cerr << Error().DetailMsg();
            Error().ClearErrorMsg();
            Error().severity( SEVERITY_EXIT );
            return instance_count;
        }

        ReadTokenSeparator( in );
        in >> c;

        if( _fileType == WORKING_SESSION ) {
            inst_state = noStateSE;
            if( strchr( "CIND", c ) ) { // if there is a valid char
                inst_state = EntityWfState( c );
                ReadTokenSeparator( in );
                in >> c;        // read the ENTITY_NAME_DELIM
            } else if( c == 'E' ) {
                in.putback( c );
                if( FoundEndSecKywd( in, _error ) ) {
                    endsec = 1;
                }
            }
            if( !endsec && ( c != ENTITY_NAME_DELIM ) ) {
                cout << "Invalid editing state character: " << c << endl;
                cout << "Assigning editing state to be INCOMPLETE\n";
                ReadTokenSeparator( in );
                in >> c;        // read the ENTITY_NAME_DELIM
            }
            // else the character should be ENTITY_NAME_DELIM
        }

        if( !endsec && ( c != ENTITY_NAME_DELIM ) ) {
            in.putback( c );
            while( c != ENTITY_NAME_DELIM && in.good() &&
                    !FoundEndSecKywd( in, _error ) ) {
                tmpbuf.set_null();
                SkipInstance( in, tmpbuf );
                cout << "ERROR: trying to recover from invalid data. skipping: "
                     << tmpbuf << endl;
                ReadTokenSeparator( in );
            }
            if( c == ENTITY_NAME_DELIM ) {
                in >> c;
            }
        }

    } // end while loop

    if( _entsNotCreated ) {
        // new
        sprintf( buf,
                 "STEPfile Reading File: Unable to create %d instances.\n\tIn first pass through DATA section. Check for invalid entity types.\n",
                 _entsNotCreated );
        // old
        /*
            sprintf(buf,
            "STEPfile Reading File: Unable to create %d instances.\n\tIn first pass through DATA section. Check for invalid entity types.\n",
                _errorCount);
        */
        _error.AppendToUserMsg( buf );
        _error.GreaterSeverity( SEVERITY_WARNING );
    }
    if( !in.good() ) {
        _error.AppendToUserMsg( "Error in input file.\n" );
    }

    return instance_count;
}
#endif

int
STEPfile::ReadWorkingData1( istream & in ) {
    return ReadData1( in );

#ifdef junk
    _errorCount = 0;  // reset error count
    _warningCount = 0;  // reset error count

    char c;
    int instance_count = 0;
    char errbuf[BUFSIZ];
    std::string tmpbuf;

    //  PASS 1:  create instances
    SDAI_Application_instance * obj;
    stateEnum inst_state;

    int endsec = 0;
    while( !endsec ) {
        //check for end of file
        if( in.eof() ) {
            _error.AppendToUserMsg(
                "Error: in ReadWorkingData1. end of file reached.\n" );
            return instance_count;
        }

        //check error count
        if( _errorCount > _maxErrorCount ) {
            cerr <<
                 "Warning: Too Many Errors in File. Read function aborted.\n";
            cerr << Error().UserMsg();
            cerr << Error().DetailMsg();
            Error().ClearErrorMsg();
            Error().severity( SEVERITY_EXIT );
            return instance_count;
        }

        inst_state = noStateSE;
        ReadTokenSeparator( in );
        in.get( c );

        inst_state = EntityWfState( c );
        if( c == 'E' ) {
            // "ENDSEC;" expected
            in.putback( c );
            endsec = 1;
            if( _errorCount ) {
                sprintf( errbuf, "%d error(s) in second pass of DATA section.\n", _errorCount );
                _error.AppendToUserMsg( errbuf );
            }
            return instance_count;
        }

        in >> ws;
        in >> c;
        if( c != ENTITY_NAME_DELIM ) {
            //      error
        }

// fixing above here
        switch( c ) {
            case wsSaveComplete:
                in >> c;
                if( c == ENTITY_NAME_DELIM ) {
                    inst_state = completeSE;
                }
                break;

            case wsSaveIncomplete:
                in >> c;
                if( c == ENTITY_NAME_DELIM ) {
                    inst_state = incompleteSE;
                }
                break;

            case wsDelete:
                in >> c;
                if( c == ENTITY_NAME_DELIM ) {
                    inst_state = deleteSE;
                }
                break;

            case wsNew:
                in >> c;
                if( c == ENTITY_NAME_DELIM ) {
                    inst_state = newSE;
                }
                break;

            case 'E':
                // "ENDSEC;" expected
                in.putback( c );
                endsec = 1;
                if( _errorCount ) {
                    sprintf( errbuf, "%d error(s) in second pass of DATA section.\n", _errorCount );
                    _error.AppendToUserMsg( errbuf );
                }

                return instance_count;

            default:
                cerr << "Error: in ReadWorkingData1. Unexpected input. \'" <<
                     c << "\'\n";
                endsec = 1;
                return instance_count;
        }

        switch( inst_state ) {
            case  noStateSE:
                SkipInstance( in, tmpbuf );
                cerr << "Error: in ReadWorkingData1.\n\tunexpected character was: " << c << "\n\tattempt to recover, data lost: " << tmpbuf << "\n";
                break;

            case  deleteSE:
                // things marked for deletion are deleted
                SkipInstance( in, tmpbuf );
                break;

            default:  // everything else is created
                obj = CreateInstance( in, cout );
                if( obj != ENTITY_NULL ) {
                    if( obj->Error().severity() < SEVERITY_WARNING ) {
                        ++_errorCount;
                    } else if( obj->Error().severity() < SEVERITY_NULL ) {
                        ++_warningCount;
                    }
                    instances().Append( obj, inst_state );
                    ++instance_count;
                } else {
                    ++_errorCount;
                }

                break;
        }
    }

    return instance_count;
#endif
}

/******************************************************************
 ** Procedure:  ReadData2
 ** Parameters:  istream
 ** Returns:  number of valid instances read
 ** Description:  reads in the data portion of the instances
 **               in an exchange file
 ******************************************************************/
int
STEPfile::ReadData2( istream & in, bool useTechCor ) {
    _entsInvalid = 0;
    _entsIncomplete = 0;
    _entsWarning = 0;

    int total_instances = 0;
    int valid_insts = 0;    // used for exchange file only
    stateEnum inst_state = noStateSE; // used if reading working file

    _errorCount = 0;  // reset error count
    _warningCount = 0;  // reset error count

    char c;
    char buf[BUFSIZ];
    buf[0] = '\0';
    std::string tmpbuf;

    SDAI_Application_instance * obj = ENTITY_NULL;
    std::string cmtStr;

//    ReadTokenSeparator(in, &cmtStr);

    int endsec = FoundEndSecKywd( in, _error );

    //  PASS 2:  read instances
    while( in.good() && !endsec ) {
        ReadTokenSeparator( in, &cmtStr );
        in >> c;

        if( _fileType == WORKING_SESSION ) {
            if( strchr( "CIND", c ) ) { // if there is a valid char
                inst_state = EntityWfState( c );
                ReadTokenSeparator( in, &cmtStr );
                in >> c;    // read the ENTITY_NAME_DELIM
            }
            /*
                    // don't need this error msg for the 2nd pass (it was done on 1st)
                    else
                    {
                    cout << "Invalid editing state character: " << c << endl;
                    cout << "Assigning editing state to be INCOMPLETE\n";
                    inst_state = incompleteSE;
                    }
            */
        }

        if( c != ENTITY_NAME_DELIM ) {
            in.putback( c );
            while( c != ENTITY_NAME_DELIM && in.good() &&
                    !( endsec = FoundEndSecKywd( in, _error ) ) ) {

                tmpbuf.clear();
                FindStartOfInstance( in, tmpbuf );
                cout << "ERROR: trying to recover from invalid data. skipping: "
                     << tmpbuf << endl;
                in >> c;
                ReadTokenSeparator( in, &cmtStr );
            }
        }


        if( !endsec ) {
            obj = ENTITY_NULL;
            if( ( _fileType == WORKING_SESSION ) && ( inst_state == deleteSE ) ) {
                SkipInstance( in, tmpbuf );
            } else {
                obj =  ReadInstance( in, cout, cmtStr, useTechCor );
            }

            cmtStr.clear();
            if( obj != ENTITY_NULL ) {
                if( obj->Error().severity() < SEVERITY_INCOMPLETE ) {
                    ++_entsInvalid;
                    // old
                    ++_errorCount;
                } else if( obj->Error().severity() == SEVERITY_INCOMPLETE ) {
                    ++_entsIncomplete;
                    ++_entsInvalid;
                } else if( obj->Error().severity() == SEVERITY_USERMSG ) {
                    ++_entsWarning;
                } else { // i.e. if severity == SEVERITY_NULL
                    ++valid_insts;
                }

                obj->Error().ClearErrorMsg();

                ++total_instances;
            } else {
                ++_entsInvalid;
                // old
                ++_errorCount;
            }

            if( _entsInvalid > _maxErrorCount ) {
                _error.AppendToUserMsg( "Warning: Too Many Errors in File. Read function aborted.\n" );
                cerr << Error().UserMsg();
                cerr << Error().DetailMsg();
                Error().ClearErrorMsg();
                Error().severity( SEVERITY_EXIT );
                return valid_insts;
            }

            endsec = FoundEndSecKywd( in, _error );
        }
    } // end while loop

    if( _entsInvalid ) {
        sprintf( buf,
                 "%s \n\tTotal instances: %d \n\tInvalid instances: %d \n\tIncomplete instances (includes invalid instances): %d \n\t%s: %d.\n",
                 "Second pass complete - instance summary:", total_instances,
                 _entsInvalid, _entsIncomplete, "Warnings",
                 _entsWarning );
        cout << buf << endl;
        _error.AppendToUserMsg( buf );
        _error.AppendToDetailMsg( buf );
        _error.GreaterSeverity( SEVERITY_WARNING );
    }
    if( !in.good() ) {
        _error.AppendToUserMsg( "Error in input file.\n" );
    }

//    if( in.good() )
//  in.putback(c);
    return valid_insts;
}

#if xpqosdblkxfnvkbybbbbb
int
STEPfile::ReadData2( istream & in ) {
    _entsInvalid = 0;
    _entsIncomplete = 0;
    _entsWarning = 0;

    int total_instances = 0;
    int valid_insts = 0;    // used for exchange file only
    stateEnum inst_state; // used if reading working file

    _errorCount = 0;  // reset error count
    _warningCount = 0;  // reset error count

    char c;
    char buf[BUFSIZ];
    buf[0] = '\0';
    std::string tmpbuf;

    SDAI_Application_instance * obj;
    std::string cmtStr;

    ReadTokenSeparator( in, &cmtStr );
    in >> c;

    if( _fileType == WORKING_SESSION ) {
        if( strchr( "SIND", c ) ) { // if there is a valid char
            inst_state = EntityWfState( c );
            ReadTokenSeparator( in, &cmtStr );
            in >> c;        // read the ENTITY_NAME_DELIM
        } else if( c != ENTITY_NAME_DELIM ) {
            cout << "Invalid editing state character: " << c << endl;
            cout << "Assigning editing state to be INCOMPLETE\n";
            ReadTokenSeparator( in, &cmtStr );
            in >> c;        // read the ENTITY_NAME_DELIM
        }
        // else the character should be ENTITY_NAME_DELIM
    }

    //  PASS 2:  read instances
    while( c == ENTITY_NAME_DELIM && in.good() ) {
        if( ( _fileType == WORKING_SESSION ) && ( inst_state == deleteSE ) ) {
            SkipInstance( in, tmpbuf );
        } else {
            obj =  ReadInstance( in, cout, cmtStr );
        }

        cmtStr.set_null();
        if( obj != ENTITY_NULL ) {
            if( obj->Error().severity() < SEVERITY_INCOMPLETE ) {
                ++_entsInvalid;
                // old
                ++_errorCount;
            } else if( obj->Error().severity() == SEVERITY_INCOMPLETE ) {
                ++_entsIncomplete;
                ++_entsInvalid;
            } else if( obj->Error().severity() == SEVERITY_USERMSG ) {
                ++_entsWarning;
            } else { // i.e. if severity == SEVERITY_NULL
                ++valid_insts;
            }

            obj->Error().ClearErrorMsg();

            ++total_instances;
        } else {
            ++_entsInvalid;
            // old
            ++_errorCount;
        }

        if( _entsInvalid > _maxErrorCount ) {
            _error.AppendToUserMsg( "Warning: Too Many Errors in File. Read function aborted.\n" );
            cerr << Error().UserMsg();
            cerr << Error().DetailMsg();
            Error().ClearErrorMsg();
            Error().severity( SEVERITY_EXIT );
            return valid_insts;
        }

        ReadTokenSeparator( in, &cmtStr );
        in >> c;

        if( _fileType == WORKING_SESSION ) {
            inst_state = noStateSE;
            if( strchr( "SIND", c ) ) { // if there is a valid char
                inst_state = EntityWfState( c );
                ReadTokenSeparator( in, &cmtStr );
                in >> c;        // read the ENTITY_NAME_DELIM
            } else if( c != ENTITY_NAME_DELIM ) {
                cout << "Invalid editing state character: " << c << endl;
                cout << "Assigning editing state to be INCOMPLETE\n";
                ReadTokenSeparator( in, &cmtStr );
                in >> c;        // read the ENTITY_NAME_DELIM
            }
            // else the character should be ENTITY_NAME_DELIM
        }
        while( c != ENTITY_NAME_DELIM && in.good() ) {
            tmpbuf.set_null();
            SkipInstance( in, tmpbuf );
            cout << "ERROR: trying to recover from invalid data. skipping: "
                 << tmpbuf << endl;
            ReadTokenSeparator( in );
        }

    } // end while loop

    if( _entsInvalid ) {
        sprintf( buf,
                 "STEPfile %s %d total instances, %d invalid, %d incomplete, %d %s.\n",
                 "Reading File 2nd pass instance summary:", total_instances,
                 _entsInvalid, _entsIncomplete, _entsWarning,
                 "issued warnings" );
//  cout << buf << endl;
        _error.AppendToUserMsg( buf );
        _error.AppendToDetailMsg( buf );
        _error.GreaterSeverity( SEVERITY_WARNING );
    }
    if( !in.good() ) {
        _error.AppendToUserMsg( "Error in input file.\n" );
    }

//    if( in.good() )
//  in.putback(c);
    return valid_insts;
}

#endif
#ifdef junk
// old version 8/23/94
int
STEPfile::ReadData2( istream & in ) {
    _errorCount = 0;  // reset error count
    _warningCount = 0;  // reset error count

    char c;
    std::string tmpbuf;

    int valid_insts = 0;    // used for exchange file only

    SDAI_Application_instance * obj = ENTITY_NULL;

    ReadTokenSeparator( in );

    in >> c ;

    //  PASS 2:  read data
    while( c == ENTITY_NAME_DELIM && in.good() ) {
        obj =  ReadInstance( in, cout );
        if( _errorCount > _maxErrorCount ) {
            _error.AppendToUserMsg( "Warning: Too Many Errors in File. Read function aborted.\n" );
            cerr << Error().UserMsg();
            cerr << Error().DetailMsg();
            Error().ClearErrorMsg();
            Error().severity( SEVERITY_EXIT );
            return valid_insts;
        }

        if( obj == ENTITY_NULL ) {
            ++_warningCount;    // the object doesn\'t exist
        } else if( obj->Error().severity() != SEVERITY_NULL )
            // there was a problem in the instance
        {
            ++_errorCount;
        } else { //  the instance is ok
            ++valid_insts;
        }

        ReadTokenSeparator( in );
        in.get( c );
    }
    in.putback( c );
    return valid_insts;
}
#endif

int
STEPfile::ReadWorkingData2( istream & in, bool useTechCor ) {
    return ReadData2( in, useTechCor );
#ifdef junk
    _errorCount = 0;  // reset error count
    _warningCount = 0;  // reset error count

    char c;
    std::string tmpbuf;
    int total_instances = 0;

    SDAI_Application_instance * obj = ENTITY_NULL;

    //  PASS 2:  read data
    int endsec = 0;
    while( !endsec ) {
        //check error count
        if( _errorCount > _maxErrorCount ) {
            cerr <<
                 "Warning: Too Many Errors in File. Read function aborted.\n";
            cerr << Error().UserMsg();
            cerr << Error().DetailMsg();
            Error().ClearErrorMsg();
            Error().severity( SEVERITY_EXIT );
            return total_instances;
        }

        ReadTokenSeparator( in );
        in.get( c );

        switch( c ) {
            case wsSaveComplete:
            case wsSaveIncomplete:
            case wsNew:
                ReadTokenSeparator( in );
                //read in the '#'
                in.get( c );

                obj = ReadInstance( in, cout );
                if( obj != ENTITY_NULL ) {
                    if( obj->Error().severity() < SEVERITY_WARNING ) {
                        ++_errorCount;
                    } else if( obj->Error().severity() < SEVERITY_NULL ) {
                        ++_warningCount;
                    }
                    ++total_instances;
                } else {
                    ++_errorCount;
                }
                break;

            case wsDelete:
                // things marked for deletion are deleted
                SkipInstance( in, tmpbuf );
                break;

            case 'E':
                //should be  "ENDSEC;"
                endsec = 1;
                in.putback( c );
                return total_instances;

            default:
                tmpbuf.Append( "Error: in ReadWorkingData2.\n\tUnexpected character in input: \'" );
                tmpbuf.Append( c );
                tmpbuf.Append( "\'.\n" );
                _error.AppendToUserMsg( tmpbuf );
                return total_instances;
        }
    }
    return total_instances;
#endif
}

/* Looks for the word DATA followed by optional whitespace
 * followed by a semicolon.  When it is looking for the word
 * DATA it skips over strings and comments.
 */

int
STEPfile::FindDataSection( istream & in ) {
    ErrorDescriptor errs;
    SDAI_String tmp;
    std::string s; // used if need to read a comment
    char c;

    while( in.good() ) {
        in >> c;
        if( in.eof() ) {
            _error.AppendToUserMsg( "Can't find \"DATA;\" section." );
            return 0; //ERROR_WARNING
        }
        switch( c ) {
            case 'D':  //  look for string "DATA;" with optional whitespace after "DATA"
                c = in.peek(); // look at next char (for 'A')
                // only peek since it may be 'D' again a we need to start over
                if( c == 'A' ) {
                    in.get( c ); // read char 'A'
                    c = in.peek();  // look for 'T'
                    if( c == 'T' ) {
                        in.get( c );    // read 'T'
                        c = in.peek();  // look for 'A'
                        if( c == 'A' ) {
                            in.get( c ); // read 'A'
                            in >> ws; // may want to skip comments or print control directives?
                            c = in.peek();  // look for semicolon
                            if( c == ';' ) {
                                in.get( c ); // read the semicolon
                                return 1; // success
                            }
                        }
                    }
                }
                break;

            case '\'':  // get past the string
                in.putback( c );
                tmp.STEPread( in, &errs );
                break;

            case '/': // read p21 file comment
                in.putback( c ); // looks like a comment
                ReadComment( in, s );
                break;

            case '\0':  // problem in input ?
                return 0;

            default:
                break;
        }
    }
    return 0;
}

int
STEPfile::FindHeaderSection( istream & in ) {
    char buf[BUFSIZ];
    char * b = buf;

    *b = '\0';

    ReadTokenSeparator( in );
    //  find the header section
    while( !( b = strstr( buf, "HEADER" ) ) ) {
        if( in.eof() ) {
            _error.AppendToUserMsg(
                "Error: Unable to find HEADER section. File not read.\n" );
            _error.GreaterSeverity( SEVERITY_INPUT_ERROR );
            return 0;
        }
        in.getline( buf, BUFSIZ, ';' ); // reads but does not store the ;
    }
    return 1;
}
//dasdelete

//dasdelete
/***************************
 description:
    This function creates an instance based on the KEYWORD
    read from the istream.
    It expects an ENTITY_NAME (int) from the first set of
    characters on the istream. If the (int) is not found,
    ENTITY_NULL is returned.

  side effects:
    The function leaves the istream set to the end of the stream
    of characters representing the ENTITY_INSTANCE. It attempts to
    recover on errors by reading up to and including the next ';'.

an ENTITY_INSTANCE consists of:
   '#'(int)'=' [SCOPE] SIMPLE_RECORD ';' ||
   '#'(int)'=' [SCOPE] SUBSUPER_RECORD ';'
The '#' is read from the istream before CreateInstance is called.
***************************/
SDAI_Application_instance *
STEPfile::CreateInstance( istream & in, ostream & out ) {
    std::string tmpbuf;
    std::string objnm;

    char c;
    std::string schnm;

    int fileid = -1;
    SDAI_Application_instance_ptr * scopelist = 0;

    SDAI_Application_instance * obj;
    ErrorDescriptor result;
    // Sent down to CreateSubSuperInstance() to receive error info

    ReadTokenSeparator( in );

    in >> fileid; // read instance id
    fileid = IncrementFileId( fileid );
    if( instances().FindFileId( fileid ) ) {
        SkipInstance( in, tmpbuf );
        out <<  "ERROR: instance #" << fileid
            << " already exists.\n\tData lost: " << tmpbuf << endl;
        return ENTITY_NULL;
    }

    ReadTokenSeparator( in );
    in.get( c ); // read equal sign
    if( c != '=' ) {
        // ERROR: '=' expected
        SkipInstance( in, tmpbuf );
        out << "ERROR: instance #" << fileid
            << " \'=\' expected.\n\tData lost: " << tmpbuf << endl;
        return ENTITY_NULL;
    }

    ReadTokenSeparator( in );
    c = in.peek(); // peek at the next character on the istream

    //check for optional "&SCOPE" construct
    if( c == '&' ) { // TODO check this out
        Severity s = CreateScopeInstances( in, &scopelist );
        if( s < SEVERITY_WARNING ) {
            return ENTITY_NULL;
        }
        ReadTokenSeparator( in );
        c = in.peek(); // peek at next char on istream again
    }

    //check for subtype/supertype record
    //DAS todo
    if( c == '(' ) {
        //  TODO:  implement complex inheritance

        obj = CreateSubSuperInstance( in, fileid, result );
        if( obj == ENTITY_NULL ) {
            SkipInstance( in, tmpbuf );
            out << "ERROR: instance #" << fileid
                << " Illegal complex entity.\n"
                << result.UserMsg() << ".\n\n";
            return ENTITY_NULL;
        }
    } else { // not a complex entity
        // check for User Defined Entity
        int userDefined = 0;
        if( c == '!' ) {
            userDefined = 1;
            in.get( c );
        }

        ReadStdKeyword( in, objnm, 1 ); // read the type name
        if( !in.good() ) {
            out << "ERROR: instance #" << fileid
                << " Unexpected file problem in "
                << "STEPfile::CreateInstance.\n";
        }

        //create the instance using the Registry object
        if( userDefined ) {
            SkipInstance( in, tmpbuf );
            out << "WARNING: instance #" << fileid
                << " User Defined Entity in DATA section ignored.\n"
                << "\tData lost: \'!" << objnm << "\': " << tmpbuf
                << endl;
            return ENTITY_NULL;
        } else {
            schnm = schemaName();
            obj = reg().ObjCreate( objnm.c_str(), schnm.c_str() );
            if( obj == ENTITY_NULL ) {
                // This will be the case if objnm does not exist in the reg.
                result.UserMsg( "Unknown ENTITY type" );
            } else if( obj->Error().severity() <= SEVERITY_WARNING ) {
                // Common causes of error is that obj is an abstract supertype
                // or that it can only be instantiated using external mapping.
                // If neither are the case, create a generic message.
                if( !obj->Error().UserMsg().empty() ) {
                    result.UserMsg( obj->Error().UserMsg() );
                } else {
                    result.UserMsg( "Could not create ENTITY" );
                }
                // Delete obj so that below we'll know that an error occur:
                delete obj;
                obj = ENTITY_NULL;
            }
        }
    }

    if( obj == ENTITY_NULL ) {
        SkipInstance( in, tmpbuf );
        out << "ERROR: instance #" << fileid << " \'" << objnm
            << "\': " << result.UserMsg()
            << ".\n\tData lost: " << tmpbuf << "\n\n";
        return ENTITY_NULL;
    }
    obj -> STEPfile_id = fileid;

    //  scan values
    SkipInstance( in, tmpbuf );

    ReadTokenSeparator( in );
    return obj;
}


/**************************************************
 description:
    This function reads the SCOPE list for an entity instance,
    creates each instance on the scope list, and appending each
    instance to the instance manager.
 side-effects:
    It first searches for "&SCOPE" and reads all characters
    from the istream up to and including "ENDSCOPE"
 returns: ErrorDescriptor
    >= SEVERITY_WARNING: the istream was read up to and including the
                      "ENDSCOPE" and the export list /#1, ... #N/
    <  SEVERITY_WARNING: the istream was read up to and including the next ";"
    < SEVERITY_BUG: fatal
**************************************************/
Severity
STEPfile::CreateScopeInstances( istream & in, SDAI_Application_instance_ptr ** scopelist ) {
    Severity rval = SEVERITY_NULL;
    SDAI_Application_instance * se;
    std::string tmpbuf;
    char c;
    int exportid;
    SDAI_Application_instance_ptr inscope [BUFSIZ];
    int i = 0;
    std::string keywd;

    keywd = GetKeyword( in, " \n\t/\\#;", _error );
    if( strncmp( const_cast<char *>( keywd.c_str() ), "&SCOPE", 6 ) ) {
        //ERROR: "&SCOPE" expected
        //TODO: should attempt to recover by reading through ENDSCOPE
        //currently recovery is attempted by reading through next ";"
        SkipInstance( in, tmpbuf );
        cerr << "ERROR: " << "\'&SCOPE\' expected." <<
             "\n\tdata lost: " << tmpbuf << "\n";
        return SEVERITY_INPUT_ERROR;
    }

    ReadTokenSeparator( in );

    in.get( c );
    while( c == '#' ) {
        se = CreateInstance( in, cout );
        if( se != ENTITY_NULL ) {
            //TODO:  apply scope information to se
            //  Add se to scopelist
            if( i < BUFSIZ ) {
                inscope [i] = se;
                ++i;
            }
            //append the se to the instance manager
            instances().Append( se, newSE );
        } else {
            //ERROR: instance in SCOPE not created
            rval = SEVERITY_WARNING;
            SkipInstance( in, tmpbuf );
            cerr << "instance in SCOPE not created.\n\tdata lost: "
                 << tmpbuf << "\n";
            ++_errorCount;
        }

        ReadTokenSeparator( in );
        in.get( c );
    }
    in.putback( c );
    *scopelist = new SDAI_Application_instance_ptr [i];
    while( i > 0 ) {
        *scopelist [i - 1] = inscope [i];
        i--;
    }

    //check for "ENDSCOPE"
    keywd = GetKeyword( in, " \t\n/\\#;", _error );
    if( strncmp( const_cast<char *>( keywd.c_str() ), "ENDSCOPE", 8 ) ) {
        //ERROR: "ENDSCOPE" expected
        SkipInstance( in, tmpbuf );
        cerr << "ERROR: " << "\'ENDSCOPE\' expected."
             << "\n\tdata lost: " << tmpbuf << "\n";
        ++_errorCount;
        return SEVERITY_INPUT_ERROR;
    }

    //check for export list
    ReadTokenSeparator( in );
    in.get( c );
    in.putback( c );
    if( c == '/' ) {
        //read export list
        in.get( c );
        c = ',';
        while( c == ',' ) {
            ReadTokenSeparator( in );
            in.get( c );
            if( c != '#' )  {  } //ERROR
            in >> exportid;
            //TODO: nothing is done with the idnums on the export list
            ReadTokenSeparator( in );
            in.get( c );
        }
        if( c != '/' ) {
            //ERROR: '/' expected while reading export list
            SkipInstance( in, tmpbuf );
            cerr << "ERROR:  \'/\' expected in export list.\n\tdata lost: " << tmpbuf << "\n";
            ++_errorCount;
            rval =  SEVERITY_INPUT_ERROR;
        }
        ReadTokenSeparator( in );
    }
    return rval;
}

SDAI_Application_instance *
STEPfile::CreateSubSuperInstance( istream & in, int fileid, ErrorDescriptor & e ) {
    std::string tmpstr;
    SDAI_Application_instance * obj = ENTITY_NULL;

    char c;
    std::string schnm;

    std::string buf; // used to hold the simple record that is read
    ErrorDescriptor err; // used to catch error msgs

    const int enaSize = 64;
    std::string * entNmArr[enaSize]; // array of entity type names
    int enaIndex = 0;

    in >> ws;
    in.get( c ); // read the open paren
    c = in.peek(); // see if you have closed paren (ending the record)
    while( in.good() && ( c != ')' ) && ( enaIndex < enaSize ) ) {
        entNmArr[enaIndex] = new std::string( "" );
        ReadStdKeyword( in, *( entNmArr[enaIndex] ), 1 ); // read the type name
        if( entNmArr[enaIndex]->empty() ) {
            delete entNmArr[enaIndex];
            entNmArr[enaIndex] = 0;
        } else {
            SkipSimpleRecord( in, buf, &err );
            buf.clear();
            enaIndex++;
        }
        in >> ws;
        c = in.peek(); // see if you have closed paren (ending the record)
        // If someone separates the entities with commas (or some other
        // garbage or a comment) this will keep the read function from
        // infinite looping.  If the entity name starts with a digit it is
        // incorrect and will be invalid.
        while( in.good() && ( c != ')' ) && !isalpha( c ) ) {
            in >> c; // skip the invalid char
            c = in.peek(); // see if you have closed paren (ending the record)
        }
    }
    entNmArr[enaIndex] = 0;
    schnm = schemaName();

    obj = new STEPcomplex( &_reg, ( const std::string ** )entNmArr, fileid, schnm.c_str() );

    if( obj->Error().severity() <= SEVERITY_WARNING ) {
        // If obj is not legal, record its error info and delete it:
        e.severity( obj->Error().severity() );
        e.UserMsg( obj->Error().UserMsg() );
        delete obj;
        obj = ENTITY_NULL;
    }

    enaIndex = 0;
    while ( entNmArr[enaIndex] != 0) {
        delete entNmArr[enaIndex];
        enaIndex ++;
    }

    return obj;
}

/**************************************************
 description:
    This function reads the SCOPE list for an entity instance,
    and reads the values for each instance from the istream.
 side-effects:
    It first searches for "&SCOPE" and reads all characters
    from the istream up to and including "ENDSCOPE"
**************************************************/
Severity
STEPfile::ReadScopeInstances( istream & in ) {
    Severity rval = SEVERITY_NULL;
    SDAI_Application_instance * se;
    std::string tmpbuf;
    char c;
    int exportid;
    std::string keywd;
    std::string cmtStr;

    keywd = GetKeyword( in, " \n\t/\\#;", _error );
    if( strncmp( const_cast<char *>( keywd.c_str() ), "&SCOPE", 6 ) ) {
        //ERROR: "&SCOPE" expected
        SkipInstance( in, tmpbuf );
        cerr << "\'&SCOPE\' expected.\n\tdata lost: " << tmpbuf << "\n";
        ++_errorCount;
        return SEVERITY_WARNING;
    }

    ReadTokenSeparator( in );

    in.get( c );
    while( c == '#' ) {
        se = ReadInstance( in, cout, cmtStr );
        if( se != ENTITY_NULL ) {
            //apply scope information to se
            //TODO: not yet implemented
        } else {
            //ERROR: unable to read instance in SCOPE
            rval = SEVERITY_WARNING;
        }

        ReadTokenSeparator( in );
        in.get( c );
    }
    in.putback( c );

    //check for "ENDSCOPE"
    keywd = GetKeyword( in, " \t\n/\\#;", _error );
    if( strncmp( const_cast<char *>( keywd.c_str() ), "ENDSCOPE", 8 ) ) {
        //ERROR: "ENDSCOPE" expected
        SkipInstance( in, tmpbuf );
        cerr << " \'ENDSCOPE\' expected.\n\tdata lost: " << tmpbuf << "\n";
        ++_errorCount;
        return SEVERITY_WARNING;
    }

    //check for export list
    ReadTokenSeparator( in );
    in.get( c );
    in.putback( c );
    if( c == '/' ) {
        //read through export list
        in.get( c );
        c = ',';
        while( c == ',' ) {
            ReadTokenSeparator( in );
            in.get( c );
            in >> exportid;
            ReadTokenSeparator( in );
            in.get( c );
        }
        if( c != '/' ) {
            //ERROR: '/' expected while reading export list
            SkipInstance( in, tmpbuf );
            cerr << " \'/\' expected while reading export list.\n\tdata lost: "
                 << tmpbuf << "\n";
            ++_errorCount;
            rval =  SEVERITY_WARNING;
        }
        ReadTokenSeparator( in );
    }
    return rval;
}



#ifdef junk
//FIXME: do we keep this?
void ReadEntityError( char c, int i, istream & in ) {
    char errStr[BUFSIZ];
    errStr[0] = '\0';

    if( ( i >= 0 ) && ( i < attributes.list_length() ) ) { // i is an attribute
        sprintf( errStr, "  invalid data for type \'%s\'\n",
                 attributes[i].TypeName() );
        _error.AppendToDetailMsg( errStr );
    } else {
        sprintf( errStr, "  No more attributes were expected.\n" );
        _error.AppendToDetailMsg( errStr );
    }

    std::string tmp;
    STEPwrite( tmp ); // STEPwrite writes to a static buffer inside function
    sprintf( errStr,
             "  The invalid instance to this point looks like :\n%s\n",
             tmp.c_str() );
    _error.AppendToDetailMsg( errStr );

    _error.AppendToDetailMsg( "  data lost looking for end of entity:" );

    //  scan over the rest of the instance and echo it

    in.clear();
    int foundEnd = 0;
    tmp = "";

    // Search until a close paren is found followed by (skipping optional
    // whitespace) a semicolon
    while( in.good() && !foundEnd ) {
        while( in.good() && ( c != ')' ) ) {
            in.get( c );
            tmp.Append( c );
        }
        if( in.good() && ( c == ')' ) ) {
            in >> ws; // skip whitespace
            in.get( c );
            tmp.Append( c );
            if( c == ';' ) {
                foundEnd = 1;
            }
        }
    }
    _error.AppendToDetailMsg( tmp.c_str() );
    sprintf( errStr, "\nfinished reading #%d\n", STEPfile_id );
    _error.AppendToDetailMsg( errStr );
    return;
}
#endif

/*****************************************************
 description:
 This function populates a SDAI_Application_instance with the values read from
 the istream.

 This function must keeps track of error messages encountered when
 reading the SDAI_Application_instance. It passes SDAI_Application_instance error information onto
 the STEPfile ErrorDescriptor.
*****************************************************/
SDAI_Application_instance *
STEPfile::ReadInstance( istream & in, ostream & out, std::string & cmtStr,
                        bool useTechCor ) {
    Severity sev = SEVERITY_NULL;

    std::string tmpbuf;
    char errbuf[BUFSIZ];
    errbuf[0] = '\0';
    std::string currSch;
    std::string objnm;

    char c;
    int fileid;
    SDAI_Application_instance * obj = ENTITY_NULL;
    int idIncrNum = FileIdIncr();

    ReadComment( in, cmtStr );

    in >> fileid;
    fileid = IncrementFileId( fileid );

    //  check to see that instance was created on PASS 1
    MgrNode * node = instances().FindFileId( fileid );
    if( ( !node ) || ( ( obj =  node -> GetApplication_instance() ) == ENTITY_NULL ) ) {
        SkipInstance( in, tmpbuf );
        // Changed the 2nd pass error message to report Part 21 User
        // Defined Entities. STEPfile still includes them in the error count
        // which is not valid.
        // Check to see if an User Defined Entity has been found.
        const char * ude = tmpbuf.c_str();
        while( *ude && ( *ude != '=' ) ) {
            ude++;
        }
        if( *ude == '=' ) {
            ude++;
        }
        while( *ude && isspace( *ude ) ) {
            ude++;
        }
        if( *ude == '!' ) {
            out << "\nWARNING: #" << fileid <<
                " - Ignoring User Defined Entity.\n\tData lost: "
                << tmpbuf << endl;
        } else
            out << "\nERROR: in 2nd pass, instance #" << fileid
                << " not found.\n\tData lost: " << tmpbuf << endl;
        return ENTITY_NULL;
    } else if( ( _fileType != WORKING_SESSION ) && ( node->CurrState() != newSE ) ) {
        SkipInstance( in, tmpbuf );
        out << "\nERROR: in 2nd pass, instance #" << fileid
            << " already exists - ignoring duplicate.\n\tData lost: "
            << tmpbuf << endl;
        return ENTITY_NULL;
    }

    ReadTokenSeparator( in, &cmtStr );

    in.get( c );
    if( c != '=' ) {
        //ERROR: '=' expected
        SkipInstance( in, tmpbuf );
        out << "ERROR: instance #" << fileid
            << " \'=\' expected.\n\tData lost: " << tmpbuf << endl;
        return ENTITY_NULL;
    }

    ReadTokenSeparator( in, &cmtStr );

    //peek at the next character on the istream
    c = in.peek();

    //check for optional "&SCOPE" construct
    if( c == '&' ) {
        ReadScopeInstances( in );
        ReadTokenSeparator( in, &cmtStr );
        in.get( c );
        in.putback( c );
    }

    currSch = schemaName();

    //check for subtype/supertype record
    if( c == '(' ) {
        // TODO
        sev = obj->STEPread( fileid, idIncrNum, &instances(), in, currSch.c_str(),
                             useTechCor, _strict );

        ReadTokenSeparator( in, &cmtStr );

        if( !cmtStr.empty() ) {
            obj->AddP21Comment( cmtStr );
        }

        c = in.peek(); // check for semicolon or keyword 'ENDSEC'
        if( c != 'E' ) {
            in >> c;    // read the semicolon
        }
    } else {
        ReadTokenSeparator( in, &cmtStr );
        c = in.peek();

        // check for User Defined Entity
        // DAS - I checked this out. It doesn't get into this code for user
        // defined entities because a ude isn't created.
        int userDefined = 0;
        if( c == '!' ) {
            userDefined = 1;
            in.get( c );
        }

        ReadStdKeyword( in, objnm, 1 ); // read the type name
        if( !in.good() ) {
            out << "ERROR: instance #" << fileid
                << " Unexpected file problem in "
                << "STEPfile::ReadInstance." << endl;;
        }
        ReadTokenSeparator( in, &cmtStr );

        //  read values
        if( userDefined ) {
            SkipInstance( in, tmpbuf );
            out << "WARNING: #" << fileid <<
                ". Ignoring User defined entity." << endl << "    data lost: !"
                << objnm << tmpbuf << endl;
            ++_warningCount;
            return ENTITY_NULL;
        }

        // NOTE: this function is called for all FileTypes
        // (WORKING_SESSION included)

        sev = obj->STEPread( fileid, idIncrNum, &instances(), in, currSch.c_str(),
                             useTechCor, _strict );

        ReadTokenSeparator( in, &cmtStr );

        if( !cmtStr.empty() ) {
            obj->AddP21Comment( cmtStr );
        }

        c = in.peek(); // check for semicolon or keyword 'ENDSEC'
        if( c != 'E' ) {
            in >> c;    // read the semicolon
        }

        AppendEntityErrorMsg( &( obj->Error() ) );
    }

    //set the node's state,
    //and set the STEPfile:_error (based on the type of file being read)
    switch( sev ) {
        case SEVERITY_NULL:
        case SEVERITY_USERMSG:
            if( _fileType != WORKING_SESSION ) {
                node->ChangeState( completeSE );
            }
            break;

        case SEVERITY_WARNING:
        case SEVERITY_INPUT_ERROR:
        case SEVERITY_BUG:

        case SEVERITY_INCOMPLETE:
            if( _fileType == VERSION_CURRENT ) {
                cerr << "ERROR in EXCHANGE FILE: incomplete instance #"
                     << obj -> STEPfile_id << ".\n";
                if( _fileType != WORKING_SESSION ) {
                    node->ChangeState( incompleteSE );
                }
            } else {
                if( node->CurrState() == completeSE ) {
                    sprintf( errbuf, "WARNING in WORKING FILE: changing instance #%d state from completeSE to incompleteSE.\n", fileid );
                    _error.AppendToUserMsg( errbuf );
                    if( _fileType != WORKING_SESSION ) {
                        node->ChangeState( incompleteSE );
                    }
                }
            }
            break;

        case SEVERITY_EXIT:
        case SEVERITY_DUMP:
        case SEVERITY_MAX:
            if( _fileType != WORKING_SESSION ) {
                node->ChangeState( noStateSE );
            }
            break;

        default:
            break;
    }

    // check ErrorDesc severity and set the state for MgrNode *node
    // according to completeSE or incompleteSE
    // watch how you set it based on whether you are reading an
    // exchange or working file.

    return obj;

}



/******************************************************
This function uses the C library function system to issue
a shell command which checks for the existence of the
file name and creates a backup file if the file exists.
The system command takes a string, which must be a valid
sh command. The command is:
   if (test -f filename) then mv filename filename.bak; fi

BUG: doesn't check to see if the backup command works.
     the results of the system call are not used by the
     by this function
******************************************************/
void
STEPfile::MakeBackupFile() {
    std::string bckup = FileName();
    bckup.append( ".bak" );

    std::fstream f( FileName().c_str(), std::fstream::in | std::fstream::binary );
    f << std::noskipws;
    std::istream_iterator<unsigned char> begin( f );
    std::istream_iterator<unsigned char> end;

    std::fstream f2( bckup.c_str(), std::fstream::out | std::fstream::trunc | std::fstream::binary );
    std::ostream_iterator<char> begin2( f2 );

    copy( begin, end, begin2 );

    _error.AppendToDetailMsg( "Making backup file: " );
    _error.AppendToDetailMsg( bckup.c_str() );
    _error.AppendToDetailMsg( "\n" );
}


/***************************
***************************/
Severity
STEPfile::WriteExchangeFile( ostream & out, int validate, int clearError,
                             int writeComments ) {
    Severity rval = SEVERITY_NULL;
    SetFileType( VERSION_CURRENT );
    if( clearError ) {
        _error.ClearErrorMsg();
    }

    if( validate ) {
        rval = instances().VerifyInstances( _error );
        _error.GreaterSeverity( rval );
        if( rval < SEVERITY_USERMSG ) {
            _error.AppendToUserMsg( "Unable to verify instances. File not written. Try saving as working session file." );
            _error.GreaterSeverity( SEVERITY_INCOMPLETE );
            return rval;
        }
    }

    out << FILE_DELIM << "\n";
    WriteHeader( out );
    WriteData( out, writeComments );
    out << END_FILE_DELIM << "\n";
    return rval;
}

/***************************
***************************/
Severity
STEPfile::WriteExchangeFile( const std::string filename, int validate, int clearError,
                             int writeComments ) {
    Severity rval = SEVERITY_NULL;

    if( clearError ) {
        _error.ClearErrorMsg();
    }

    if( validate ) {
        rval = instances().VerifyInstances( _error );
        _error.GreaterSeverity( rval );
        if( rval < SEVERITY_USERMSG ) {
            _error.AppendToUserMsg( "Unable to verify instances. File wasn't opened. Try saving as working session file.\n" );
            _error.GreaterSeverity( SEVERITY_INCOMPLETE );
            return rval;
        }
    }

    ostream * out =  OpenOutputFile( filename );
    if( _error.severity() < SEVERITY_WARNING ) {
        return _error.severity();
    }
    rval = WriteExchangeFile( *out, 0, 0, writeComments );
    CloseOutputFile( out );
    return rval;
}


/***************************
***************************/
Severity
STEPfile::WriteValuePairsFile( ostream & out, int validate, int clearError,
                               int writeComments, int mixedCase ) {
    Severity rval = SEVERITY_NULL;
    SetFileType( VERSION_CURRENT );
    if( clearError ) {
        _error.ClearErrorMsg();
    }

    if( validate ) {
        rval = instances().VerifyInstances( _error );
        _error.GreaterSeverity( rval );
        if( rval < SEVERITY_USERMSG ) {
            _error.AppendToUserMsg( "Unable to verify instances. File not written. Try saving as working session file." );
            _error.GreaterSeverity( SEVERITY_INCOMPLETE );
            return rval;
        }
    }

    WriteValuePairsData( out, writeComments, mixedCase );
    return rval;
}

/***************************
This function returns an integer value for
the file id for a header section entity,
based on a given entity name.

This function may change the value of the
STEPfile member variable: _headerId

The header section entities must be numbered in the following manner:

#1=FILE_DESCRIPTION
#2=FILE_NAME
#3=FILE_SCHEMA
***************************/
int STEPfile::HeaderId( const std::string name ) {
    std::string tmp = name;

    std::transform( tmp.begin(), tmp.end(), tmp.begin(), ::toupper );

    if( tmp == "FILE_DESCRIPTION" ) {
        return 1;
    }
    if( tmp == "FILE_NAME" ) {
        return 2;
    }
    if( tmp == "FILE_SCHEMA" ) {
        return 3;
    }
    return ++_headerId;
}

/***************************
***************************/
void
STEPfile::WriteHeader( ostream & out ) {
    out << "HEADER;\n";

    WriteHeaderInstanceFileDescription( out );
    WriteHeaderInstanceFileName( out );
    WriteHeaderInstanceFileSchema( out );

    // Write the rest of the header instances
    SDAI_Application_instance * se;
    int n = _headerInstances->InstanceCount();
    for( int i = 0; i < n; ++i ) {
        se = _headerInstances->GetMgrNode( i ) ->GetApplication_instance();
        if( !(
                    ( se->StepFileId() == HeaderId( "File_Name" ) ) ||
                    ( se->StepFileId() == HeaderId( "File_Description" ) ) ||
                    ( se->StepFileId() == HeaderId( "File_Schema" ) )
                ) )
            WriteHeaderInstance(
                _headerInstances->GetMgrNode( i )->GetApplication_instance(), out );
    }
    out << "ENDSEC;\n";
}

/***************************
***************************/
void
STEPfile::WriteHeaderInstance( SDAI_Application_instance * obj, ostream & out ) {
    std::string tmp;
    if( !obj->P21Comment().empty() ) {
        out << obj->P21Comment();
    }
    out << StrToUpper( obj->EntityName(), tmp ) << "(";
    int n = obj->attributes.list_length();
    for( int i = 0; i < n; ++i ) {
        ( obj->attributes[i] ).STEPwrite( out );
        if( i < n - 1 ) {
            out << ",";
        }
    }
    out << ");\n";
}

/***************************
***************************/
void
STEPfile::WriteHeaderInstanceFileName( ostream & out ) {
// Get the FileName instance from _headerInstances
    SDAI_Application_instance * se = 0;
    se = _headerInstances->GetApplication_instance( "File_Name" );
    if( se == ENTITY_NULL ) {
        se = ( SDAI_Application_instance * )HeaderDefaultFileName();
    }

//set some of the attribute values at time of output
    SdaiFile_name * fn = ( SdaiFile_name * )se;

    /* I'm not sure this is a good idea that Peter did but I'll leave around - DAS
        // write time_stamp (as specified in ISO Standard 8601)
        // output the current system time to the file, using the following format:
        // example: '1994-04-12T15:27:46'
        // for Calendar Date, 12 April 1994, 27 minute 46 seconds past 15 hours
    */
    time_t t = time( NULL );
    struct tm * timeptr = localtime( &t );
    char time_buf[26];
    strftime( time_buf, 26, "%Y-%m-%dT%H:%M:%S", timeptr );
    fn->time_stamp_( time_buf );

//output the values to the file
    WriteHeaderInstance( se, out );
}

void STEPfile::WriteHeaderInstanceFileDescription( ostream & out ) {
// Get the FileDescription instance from _headerInstances
    SDAI_Application_instance * se = 0;
    se = _headerInstances->GetApplication_instance( "File_Description" );
    if( se == ENTITY_NULL ) {
        se = ( SDAI_Application_instance * )HeaderDefaultFileDescription();
    }

    WriteHeaderInstance( se, out );
}

void STEPfile::WriteHeaderInstanceFileSchema( ostream & out ) {
    SDAI_Application_instance * se = 0;

    // Get the FileName instance from _headerInstances
    se = _headerInstances->GetApplication_instance( "File_Schema" );
    if( se == ENTITY_NULL ) {
        se = ( SDAI_Application_instance * ) HeaderDefaultFileSchema();
    }
    WriteHeaderInstance( se, out );
}


/***************************
***************************/

void STEPfile::WriteData( ostream & out, int writeComments ) {
    std::string currSch = schemaName();
    out << "DATA;\n";

    int n = instances().InstanceCount();
    for( int i = 0; i < n; ++i ) {
        instances().GetMgrNode( i )->GetApplication_instance()->STEPwrite( out, currSch.c_str(), writeComments );
    }

    out << "ENDSEC;\n";
}

/***************************
***************************/

void STEPfile::WriteValuePairsData( ostream & out, int writeComments, int mixedCase ) {
    std::string currSch = schemaName();
    int n = instances().InstanceCount();
    for( int i = 0; i < n; ++i ) {
        instances().GetMgrNode( i )->GetApplication_instance()->WriteValuePairs( out, currSch.c_str(), writeComments, mixedCase );
    }
}

Severity STEPfile::AppendFile( istream * in, bool useTechCor ) {
    Severity rval = SEVERITY_NULL;
    char errbuf[BUFSIZ];

    SetFileIdIncrement();
    int total_insts = 0,  valid_insts = 0;

    ReadTokenSeparator( *in );
    std::string keywd = GetKeyword( *in, "; #", _error );
    // get the delimiter off the istream
    char c;
    in->get( c );

    if( !strncmp( const_cast<char *>( keywd.c_str() ), "ISO-10303-21",
                  strlen( const_cast<char *>( keywd.c_str() ) ) ) ) {
        SetFileType( VERSION_CURRENT );
    } else if( !strncmp( const_cast<char *>( keywd.c_str() ), "STEP_WORKING_SESSION",
                         strlen( const_cast<char *>( keywd.c_str() ) ) ) ) {
        if( _fileType != WORKING_SESSION ) {
            _error.AppendToUserMsg(
                "Warning: Reading in file as Working Session file.\n" );
            _error.GreaterSeverity( SEVERITY_WARNING );
        }
        SetFileType( WORKING_SESSION );
    }

    else {
        sprintf( errbuf,
                 "Faulty input at beginning of file. \"ISO-10303-21;\" or"
                 " \"STEP_WORKING_SESSION;\" expected. File not read: %s\n",
                 ( ( FileName().compare( "-" ) == 0 ) ? "standard input" : FileName().c_str() ) );
        _error.AppendToUserMsg( errbuf );
        _error.GreaterSeverity( SEVERITY_INPUT_ERROR );
        return SEVERITY_INPUT_ERROR;
    }

    cout << "Reading Data from " << ( ( FileName().compare( "-" ) == 0 ) ? "standard input" : FileName().c_str() ) << "...\n";

    //  Read header
    rval = ReadHeader( *in );
    cout << "\nHEADER read:";
    if( rval < SEVERITY_WARNING ) {
        sprintf( errbuf,
                 "Error: non-recoverable error in reading header section. "
                 "There were %d errors encountered. Rest of file is ignored.\n",
                 _errorCount );
        _error.AppendToUserMsg( errbuf );
        return rval;
    } else if( rval != SEVERITY_NULL ) {
        sprintf( errbuf, "  %d  ERRORS\t  %d  WARNINGS\n\n",
                 _errorCount, _warningCount );
        cout << errbuf;
    } else {
        cout << endl;
    }

    if( !FindDataSection( *in ) ) {
        _error.AppendToUserMsg( "Error: Unable to find DATA section delimiter. Data section not read. Rest of file ignored.\n" );
        return SEVERITY_INPUT_ERROR;
    }

    //  PASS 1
    _errorCount = 0;
    total_insts = ReadData1( *in );

    cout << "\nFIRST PASS complete:  " << total_insts
         << " instances created.\n";
    sprintf( errbuf,
             "  %d  ERRORS\t  %d  WARNINGS\n\n",
             _errorCount, _warningCount );
    cout << errbuf;

    //  PASS 2
    //  This would be nicer if you didn't actually have to close the
    //  file but could just reposition the pointer back to the
    //  beginning of the data section.  It looks like you can do this
    //  with the GNU File class, but that class doesn't have the
    //  operator >> overloaded which is used to do the rest of the
    //  parsing.  SO we are using istreams and this works, but could
    //  be better.

    // reset the error count so you're not counting things twice:
    _errorCount = 0;
    istream * in2;
    if( !( ( in2 = OpenInputFile() ) && ( in2 -> good() ) ) ) {
        //  if the stream is not readable, there's an error
        _error.AppendToUserMsg( "Cannot open file for 2nd pass -- No data read.\n" );
        CloseInputFile( in2 );
        return SEVERITY_INPUT_ERROR;
    }
    if( !FindDataSection( *in2 ) ) {
        _error.AppendToUserMsg( "Error: Unable to find DATA section delimiter in second pass. \nData section not read. Rest of file ignored.\n" );
        CloseInputFile( in2 );
        return  SEVERITY_INPUT_ERROR;
    }

    switch( _fileType ) {
        case VERSION_CURRENT:
        case VERSION_UNKNOWN:
        case WORKING_SESSION:
            valid_insts = ReadData2( *in2, useTechCor );
            break;
        default:
            cerr << "STEPfile::AppendFile: type " << _fileType << " not handled by switch statement. " << __FILE__ << ":" <<  __LINE__ << endl;
            abort();

    }

    //check for "ENDSEC;"
    ReadTokenSeparator( *in2 );
    if( total_insts != valid_insts ) {
        sprintf( errbuf, "%d invalid instances in file: %s\n",
                 total_insts - valid_insts, ( ( FileName().compare( "-" ) == 0 ) ? "standard input" : FileName().c_str() ) );
        _error.AppendToUserMsg( errbuf );
        CloseInputFile( in2 );
        return _error.GreaterSeverity( SEVERITY_WARNING );
    }

    cout << "\nSECOND PASS complete:  " << valid_insts
         << " instances valid.\n";
    sprintf( errbuf,
             "  %d  ERRORS\t  %d  WARNINGS\n\n",
             _errorCount, _warningCount );
    _error.AppendToUserMsg( errbuf );
    cout << errbuf;


    //check for "ENDSTEP;" || "END-ISO-10303-21;"

    if( in2 -> good() ) {
        ReadTokenSeparator( *in2 );
        keywd = GetKeyword( *in2, ";", _error );
        //yank the ";" from the istream
        //if (';' == in2->peek()) in2->get();
        char c;
        in2->get( c );
        if( c == ';' ) {
            ;
        } else {
            //FIXME shouldn't *something* be done based upon the value of c? MAP, 9/2011
        }
    }

    if( ( strncmp( const_cast<char *>( keywd.c_str() ),
                   END_FILE_DELIM,
                   strlen( const_cast<char *>( keywd.c_str() ) ) ) || !( in2 -> good() ) ) ) {
        _error.AppendToUserMsg( END_FILE_DELIM );
        _error.AppendToUserMsg( " missing at end of file.\n" );
        CloseInputFile( in2 );
        return _error.GreaterSeverity( SEVERITY_WARNING );
    }
    CloseInputFile( in2 );
    cout << "Finished reading file.\n\n";
    return SEVERITY_NULL;
}




/******************************************************/
Severity STEPfile::WriteWorkingFile( ostream & out, int clearError, int writeComments ) {
    SetFileType( WORKING_SESSION );
    if( clearError ) {
        _error.ClearErrorMsg();
    }

    if( instances().VerifyInstances( _error ) < SEVERITY_INCOMPLETE ) {
        _error.AppendToUserMsg( "WARNING: some invalid instances written to working session file. Data may have been lost." );
        _error.GreaterSeverity( SEVERITY_INCOMPLETE );
    }

    out << FILE_DELIM << "\n";
    WriteHeader( out );

    WriteWorkingData( out, writeComments );
    out << END_FILE_DELIM << "\n";
    SetFileType();

    return _error.severity();
}

/******************************************************/
Severity STEPfile::WriteWorkingFile( const std::string filename, int clearError,
                                     int writeComments ) {
    if( clearError ) {
        _error.ClearErrorMsg();
    }
    ostream * out =  OpenOutputFile( filename );
    if( _error.severity() < SEVERITY_WARNING ) {
        return _error.severity();
    }
    Severity rval = WriteWorkingFile( *out, 0, writeComments );
    CloseOutputFile( out );

    return rval;
}


/***************************
***************************/
void STEPfile::WriteWorkingData( ostream & out, int writeComments ) {
    std::string currSch = schemaName();
    out << "DATA;\n";
    int n = instances().InstanceCount();

    for( int i = 0; i < n; ++i ) {
        switch( instances().GetMgrNode( i )->CurrState() ) {
            case deleteSE:
                out << wsDelete;
                instances().GetMgrNode( i )->GetApplication_instance()->
                STEPwrite( out, currSch.c_str(), writeComments );
                break;
            case completeSE:
                out << wsSaveComplete;
                instances().GetMgrNode( i )->GetApplication_instance()->
                STEPwrite( out, currSch.c_str(), writeComments );
                break;
            case incompleteSE:
                out << wsSaveIncomplete;
                instances().GetMgrNode( i )->GetApplication_instance()->
                STEPwrite( out, currSch.c_str(), writeComments );
                break;
            case newSE:
                out << wsNew;
                instances().GetMgrNode( i )->GetApplication_instance()->
                STEPwrite( out, currSch.c_str(), writeComments );
                break;
            case noStateSE:
                _error.AppendToUserMsg( "no state information for this node\n" );
                break;
        }
    }
    out << "ENDSEC;\n";
}

/**************************************************
 description:
    This function sends messages to 'cerr'
    based on the values in the given ErrorDescriptor (e), which is
    supposed to be from a SDAI_Application_instance object, being manipulated by one
    of the STEPfile member functions.

    The given error descriptor's messages are then cleared.

    The STEPfile's error descriptor is set no lower than SEVERITY_WARNING.

**************************************************/
Severity
STEPfile::AppendEntityErrorMsg( ErrorDescriptor * e ) {
    ErrorDescriptor * ed = e;

    Severity sev = ed->severity();

    if( ( sev < SEVERITY_MAX ) || ( sev > SEVERITY_NULL ) ) {
        //ERROR: something wrong with ErrorDescriptor
        //_error.AppendToDetailMsg("Error: in AppendEntityErrorMsg(ErrorDesriptor& e). Incomplete ErrorDescriptor, unable to report error message in SDAI_Application_instance.\n");
        _error.GreaterSeverity( SEVERITY_WARNING );
        return SEVERITY_BUG;
    }

    switch( sev ) {
        case SEVERITY_NULL:
            return SEVERITY_NULL;

        default: {
            cerr << e->DetailMsg();
            e->ClearErrorMsg();

            if( sev < SEVERITY_USERMSG )   {
                ++_errorCount;
            }
            if( sev < SEVERITY_WARNING )   {
                sev = SEVERITY_WARNING;
            }

            _error.GreaterSeverity( sev );
            return sev;
        }
    }
}
