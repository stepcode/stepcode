#ifndef _STEPFILE_H
#define	_STEPFILE_H

/*
* NIST STEP Core Class Library
* cleditor/STEPfile.h
* April 1997
* Peter Carr
* K. C. Morris
* David Sauder

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

#include <instmgr.h>
#include <Registry.h>
#include <fstream>
#include <dirobj.h>
#include <errordesc.h>
#include <time.h>

#include <read_func.h>

//error reporting level
#define READ_COMPLETE    10
#define READ_INCOMPLETE  20

enum  FileTypeCode {
    TYPE_UNKNOWN	= -2,
    VERSION_OLD		= -1,
    VERSION_UNKNOWN	=  0,
    VERSION_CURRENT	=  1,
    WORKING_SESSION	=  2,
    OLD_WORKING_SESSION =  3
  };

class STEPfile
{
  protected:
    //data members

    InstMgr&  _instances;
    Registry& _reg;

    InstMgr & instances ()  { return _instances; }
    Registry & reg () { return _reg; }
    int _fileIdIncr;   //Increment value to be added to FileId Numbers on input

//header information
    InstMgr*  _headerInstances;
    Registry *_headerRegistry;
    
    int _headerId;     //STEPfile_id given to SCLP23(Application_instance) from header section

//file information
    DirObj* _currentDir;
    std::string _fileName;

//error information
    ErrorDescriptor _error;

    // new errors
    int _entsNotCreated; // num entities not created in first pass
    int _entsInvalid;    // num entities that had invalid attr values
    int _entsIncomplete; // num entities that had missing attr values
                         // (includes entities that had invalid values
                         // for required attrs)
    int _entsWarning;    // num entities that may have had problems
                         // with attrs - reported as an attr user msg

    // old errors
    int _errorCount;
    int _warningCount;

    int _maxErrorCount;

  protected:
    
//file type information
    FileTypeCode _fileType;
    char ENTITY_NAME_DELIM;
    char* FILE_DELIM;    
    char* END_FILE_DELIM;
    
//public member functions
  public:

//public access to member variables
//header information
    InstMgr* HeaderInstances() { return _headerInstances; }
    const Registry *HeaderRegistry() { return _headerRegistry; }
// to create header instances
    SCLP23(Application_instance)* HeaderDefaultFileName();	
    SCLP23(Application_instance)* HeaderDefaultFileDescription();	
    SCLP23(Application_instance)* HeaderDefaultFileSchema();	

//file information
    const std::string FileName() const { return _fileName; }
    const std::string SetFileName (const std::string name = "");
    const std::string TruncFileName (const std::string name) const;

//error information
    ErrorDescriptor& Error() /* const */  { return _error;        }
    int ErrorCount() const  { return _errorCount;   }
    int WarningCount() const { return _warningCount; }
    Severity AppendEntityErrorMsg (ErrorDescriptor *e);	
    
//version information
    FileTypeCode FileType() const   { return _fileType; }
    void FileType (FileTypeCode ft) { _fileType = ft; }	
    int SetFileType (FileTypeCode ft = VERSION_CURRENT);
    
//Reading and Writing 
    Severity ReadExchangeFile (const std::string filename = "", int useTechCor =1);
    Severity AppendExchangeFile (const std::string filename = "", int useTechCor =1);

    Severity ReadWorkingFile (const std::string filename = "", int useTechCor =1);
    Severity AppendWorkingFile (const std::string filename = "", int useTechCor =1);

    Severity AppendFile (istream* in, int useTechCor =1) ;

    Severity WriteExchangeFile (ostream& out, int validate =1,
				int clearError = 1, int writeComments = 1);
    Severity WriteExchangeFile (const std::string filename = "", int validate =1,
				int clearError = 1, int writeComments = 1);
    Severity WriteValuePairsFile(ostream& out, int validate =1, 
				 int clearError =1, 
				 int writeComments = 1, int mixedCase = 1);

    Severity WriteWorkingFile (ostream& out, int clearError = 1, 
			       int writeComments = 1);
    Severity WriteWorkingFile (const std::string filename = "", int clearError = 1,
			       int writeComments = 1);

    stateEnum EntityWfState(char c);
    
    void Renumber ();

//constructors
    STEPfile (Registry& r, InstMgr& i, const std::string filename = "");
    virtual ~STEPfile();

  protected:    
//member functions
    char *schemaName( char * ); // returns and copies out schema name from
                                // header instances
//called by ReadExchangeFile
    istream* OpenInputFile (const std::string filename = "");
    void CloseInputFile(istream* in);
    
    Severity ReadHeader(istream& in);

    Severity HeaderVerifyInstances(InstMgr* im);
    void HeaderMergeInstances(InstMgr* im);
   
    int HeaderId (int increment =1);
    int HeaderId (const std::string name);

    int ReadData1 (istream& in); // first pass to create instances
	// second pass to read instances
    int ReadData2 (istream& in, int useTechCor =1);

// obsolete
    int ReadWorkingData1 (istream& in);
    int ReadWorkingData2 (istream& in, int useTechCor =1);

    void ReadRestOfFile(istream& in);

	// create instance - used by ReadData1()
    SCLP23(Application_instance) *  CreateInstance(istream& in, ostream& out);
	// create complex instance - used by CreateInstance()
    SCLP23(Application_instance) * CreateSubSuperInstance(istream& in, int fileid,
					ErrorDescriptor &);

	// read the instance - used by ReadData2()
    SCLP23(Application_instance) * ReadInstance(istream& in, ostream& out, 
					std::string &cmtStr, int useTechCor =1);

  //  reading scopes are still incomplete
  //  these functions are stubs
    Severity CreateScopeInstances(istream& in, SCLP23(Application_instance_ptr) ** scopelist);
    Severity ReadScopeInstances(istream& in);
//    Severity ReadSubSuperInstance(istream& in);

    int FindDataSection (istream& in);
    int FindHeaderSection (istream& in);

// writing working session files
    void WriteWorkingData(ostream& out, int writeComments = 1);

//called by WriteExchangeFile
    ofstream* OpenOutputFile(const std::string filename = "");
    void CloseOutputFile(ostream* out);

    void WriteHeader (ostream& out);
    void WriteHeaderInstance (SCLP23(Application_instance) *obj, ostream& out);
    void WriteHeaderInstanceFileName (ostream& out);
    void WriteHeaderInstanceFileDescription (ostream& out);
    void WriteHeaderInstanceFileSchema (ostream& out);

    void WriteData (ostream& out, int writeComments = 1);
    void WriteValuePairsData(ostream& out, int writeComments = 1, 
			     int mixedCase = 1);
    
    int IncrementFileId (int fileid);
    int FileIdIncr() { return _fileIdIncr; }
    void SetFileIdIncrement ();
    void MakeBackupFile();

//    void ReadWhiteSpace(istream& in);
};

//inline functions

#endif  /*  _STEPFILE_H  */
