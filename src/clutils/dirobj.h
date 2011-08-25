
#ifndef dirobj_h
#define dirobj_h

/*
* NIST Utils Class Library
* clutils/dirobj.h
* April 1997
* David Sauder
* K. C. Morris

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

/*
 * DirObj - This object contains a list of files in a directory.
 *		Can be used for interpreting a tilde, checking paths etc.
 *	based on InterViews FBDirectory 
 *		Copyright (c) 1987, 1988, 1989 Stanford University from 
 *		/depot/iv/src/libInterViews/filebrowser.c <= notice
 *	I commented it, changed variable and function names, added a
 *	few things, and made it so we can actually use it.
 *		David Sauder
 */

///////////////////////////////////////////////////////////////////////////////
//
// These header files are a pain between compilers etc ... and 
// I agree they are in a mess.  You will have to CHECK them yourself
// depending on what you are using -- it works the way it is with g++
// but you need -I/depot/gnu/lib/g++-include
//
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>


#ifdef HAVE_UNISTD_H
// needed at least for getuid()
#include <unistd.h>
#endif

#ifdef HAVE_SYSENT_H
#include <sysent.h>
#endif

// sys/stat.h is apparently needed for ObjectCenter and Sun C++ ?
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <string.h>
#include <sys/param.h>

#include <scldir.h>

typedef unsigned boolean;

#ifndef nil
#define nil 0
#endif

/*****************************************************************************/

class DirObj {       
public:
    DirObj(const char* dirName);
    virtual ~DirObj();

    boolean LoadDirectory(const char*);
    const char* Normalize(const char*);
    const char* ValidDirectories(const char*);

    int Index(const char*);
    const char* File(int index);
	// check for file in the currently loaded directory
    boolean FileExists(const char* file) { return Index(file) ? 1 : 0;};
    int Count();

    boolean IsADirectory(const char*);
private:
    const char* ElimDot(const char*);
    const char* ElimDotDot(const char*);
    const char* InterpSlashSlash(const char*);
    const char* RealPath(const char*);

    boolean Reset(const char*);
    void ClearFileList();
    void CheckIndex(int index);
    void InsertFile(const char*, int index);
    void AppendFile(const char*);
    void RemoveFile(int index);
    virtual int Position(const char*);
private:
    char** fileList;
    int fileCount;
    int fileListSize;
};

//////////////////////////////// Count() //////////////////////////////////////
//
// Return the number of files in the loaded directory.
//
///////////////////////////////////////////////////////////////////////////////

inline int DirObj::Count () { return fileCount; }

//////////////////////////////// AppendFile() /////////////////////////////////
//
// Insert a new file into the fileList.
//
///////////////////////////////////////////////////////////////////////////////

inline void DirObj::AppendFile (const char* s) { InsertFile(s, fileCount); }

//////////////////////////////// File() ///////////////////////////////////////
//
// Return the file at the given index (starting at 0) in the fileList
//
///////////////////////////////////////////////////////////////////////////////

inline const char* DirObj::File (int index) { 
    return (0 <= index && index < fileCount) ? fileList[index] : nil;
}

//////////////////////////////// strdup() /////////////////////////////////////
//
// Duplicate a string.
//
///////////////////////////////////////////////////////////////////////////////

//inline char* strdup (const char* s) {
//    char* dup = new char[strlen(s) + 1];
//    strcpy(dup, s);
//    return dup;
//}

//////////////////////////////// DotSlash() ///////////////////////////////////
//
// Return 1 if the first char in 'path' is '.' and second char is
// '/' or '\0'; otherwise return 0
//
///////////////////////////////////////////////////////////////////////////////

inline boolean DotSlash (const char* path) {
    return 
        path[0] != '\0' && path[0] == '.' &&
        (path[1] == '/' || path[1] == '\0');
}

//////////////////////////////// DotDotSlash() ////////////////////////////////
//
// Return 1 if the first char in 'path' is '.', the second char is '.',
// and the third char is '/' or '\0'; otherwise return 0
//
///////////////////////////////////////////////////////////////////////////////

inline boolean DotDotSlash (const char* path) {
    return 
        path[0] != '\0' && path[1] != '\0' &&
        path[0] == '.' && path[1] == '.' &&
        (path[2] == '/' || path[2] == '\0');
}

#endif
