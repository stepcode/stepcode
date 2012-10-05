#ifndef SECTIONREADER_H
#define SECTIONREADER_H

// #include "lazyFileReader.h"
// #include "sdaiApplication_instance.h"
#include <fstream>
#include "lazyTypes.h"

class SDAI_Application_instance;
class lazyFileReader;
class ErrorDescriptor;

class sectionReader {
protected:
    //protected data members
    lazyFileReader * _lazyFile;
    std::ifstream & _file;

    std::streampos _sectionStart,  ///< the start of this section as reported by tellg()
                   _sectionEnd;    ///< the end of this section as reported by tellg()
    unsigned long /*loadedInstances,*/ _totalInstances;

    ErrorDescriptor * _error;
    sectionID _sectionID;
    fileID _fileID;

    // protected member functions

    sectionReader( lazyFileReader * parent, std::ifstream & file, std::streampos start );

    /** Find next occurence of str.
     * \param semicolon if true, 'str' must be followed by a semicolon, possibly preceded by whitespace.
     * \param currentPos if true, seekg() to currentPos when done. Otherwise, file pos in the returned value.
     * \returns the position of the end of the found string
     */
    std::streampos findString( const std::string& str, bool semicolon = false, bool resetPos = false );

    /** Get a keyword ending with one of delimiters.
     */
    std::string * getDelimitedKeyword( const char * delimiters );

    /** Seek to the end of the current instance */
    std::streampos seekInstanceEnd();
public:
    SDAI_Application_instance * getRealInstance( lazyInstanceLoc * inst );
    sectionID ID() const {
        return _sectionID;
    }

    virtual void findSectionStart() = 0;

    void findSectionEnd() {
        _sectionEnd = findString( "ENDSEC", true );
    }

    std::streampos startPos() const {
        return _sectionStart;
    }
    std::streampos endPos() const {
        return _sectionEnd;
    }
    void locateAllInstances(); /**< find instances in section, and add lazyInstance's to lazyInstMgr */
    virtual const namedLazyInstance nextInstance() = 0;
    instanceID readInstanceNumber();
};

#endif //SECTIONREADER_H
