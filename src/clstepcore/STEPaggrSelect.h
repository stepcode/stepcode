#ifndef STEPAGGRSELECT_H
#define STEPAGGRSELECT_H

#include "STEPaggregate.h"
#include <sc_export.h>

/** \file STEPaggrSelect.h
 * classes SelectAggregate, SelectNode
 */


/**
 * * \class SelectAggregate
 ** This is a minimal represention for a collection of SDAI_Select
 */
class SC_CORE_EXPORT SelectAggregate  :  public STEPaggregate {
public:
    virtual Severity ReadValue( std::istream & in, ErrorDescriptor * err,
                                const TypeDescriptor * elem_type,
                                InstMgrBase * insts, int addFileId = 0,
                                int assignVal = 1, int ExchangeFileFormat = 1,
                                const char * currSch = 0 );

    virtual SingleLinkNode * NewNode();
    virtual STEPaggregate & ShallowCopy( const STEPaggregate & );

    SelectAggregate();
    virtual ~SelectAggregate();
};
typedef        SelectAggregate  *   SelectAggregateH;
typedef        SelectAggregate  *   SelectAggregate_ptr;
typedef  const SelectAggregate  *   SelectAggregate_ptr_c;
typedef        SelectAggregate_ptr  SelectAggregate_var;


/**
 * * \class SelectNode
 ** This is a minimal representions for node in lists of SDAI_Select
 */
class SC_CORE_EXPORT SelectNode  : public STEPnode {
public:
    SDAI_Select  * node;
    //  INPUT
    virtual Severity StrToVal( const char * s, ErrorDescriptor * err,
                               const TypeDescriptor * elem_type,
                               InstMgrBase * insts, int addFileId = 0 );
    virtual Severity StrToVal( std::istream & in, ErrorDescriptor * err,
                               const TypeDescriptor * elem_type,
                               InstMgrBase * insts, int addFileId = 0,
                               const char * currSch = 0 );

    virtual Severity STEPread( const char * s, ErrorDescriptor * err,
                               const TypeDescriptor * elem_type,
                               InstMgrBase * insts, int addFileId = 0 );
    virtual Severity STEPread( std::istream & in, ErrorDescriptor * err,
                               const TypeDescriptor * elem_type,
                               InstMgrBase * insts, int addFileId = 0,
                               const char * currSch = 0 );
    //  OUTPUT
    virtual const char * asStr( std::string & s );
    virtual const char * STEPwrite( std::string & s, const char * = 0 );
    virtual void    STEPwrite( std::ostream & out = std::cout );

    //  CONSTRUCTORS
    SelectNode( SDAI_Select  * s );
    SelectNode();
    ~SelectNode();

    virtual SingleLinkNode   *  NewNode();

    // Calling these functions is an error.
    Severity StrToVal( const char * s, ErrorDescriptor * err ) {
        std::cerr << "Internal error:  " << __FILE__ <<  __LINE__
        << "\n" << _POC_ "\n";
        return StrToVal( s, err, 0, 0, 0 );
    }
    Severity StrToVal( std::istream & in, ErrorDescriptor * err ) {
        std::cerr << "Internal error:  " << __FILE__ <<  __LINE__
        << "\n" << _POC_ "\n";
        return StrToVal( in, err, 0, 0, 0 );
    }

    Severity STEPread( const char * s, ErrorDescriptor * err ) {
        std::cerr << "Internal error:  " << __FILE__ <<  __LINE__
        << "\n" << _POC_ "\n";
        return STEPread( s, err, 0, 0, 0 );
    }
    Severity STEPread( std::istream & in, ErrorDescriptor * err ) {
        std::cerr << "Internal error:  " << __FILE__ <<  __LINE__
        << "\n" << _POC_ "\n";
        return STEPread( in, err, 0, 0, 0 );
    }
};


#endif //STEPAGGRSELECT_H
