
#ifndef _STEPinvAttrList_h
#define _STEPinvAttrList_h 1

/** \file STEPinvAttrList.h
 * derived from STEPattributeList.h
 *
 * Similar to Inverse_attributeList, but this list also contains pointers to the setter and getter.
 *
 * \sa Inverse_attributeList
 */

class Inverse_attribute;

#include <sc_export.h>
#include <SingleLinkList.h>

class STEPinvAttrList;
class EntityAggregate;
class SDAI_Application_instance;
typedef void ( *setter_t )( SDAI_Application_instance*, EntityAggregate * );
typedef EntityAggregate * ( *getter_t )( const SDAI_Application_instance * );

/// Similar to Inverse_attributeList, but this list also contains pointers to the setter and getter.
class SC_CORE_EXPORT invAttrListNode :  public SingleLinkNode {
        friend class STEPinvAttrList;
    protected:
        Inverse_attribute * attr;
        setter_t set;
        getter_t get;
    public:
        invAttrListNode( Inverse_attribute * a, setter_t s, getter_t g );
        virtual ~invAttrListNode();
        setter_t setter() {
            return set;
        }
        getter_t getter() {
            return get;
        }
        Inverse_attribute * inverseADesc() {
            return attr;
        }
};

/// Similar to Inverse_attributeList, but this list also contains pointers to the setter and getter.
class SC_CORE_EXPORT STEPinvAttrList : public SingleLinkList {
    public:
        STEPinvAttrList();
        virtual ~STEPinvAttrList();
        invAttrListNode * operator []( int n );
        int list_length();
        void push( Inverse_attribute * a, setter_t s, getter_t g );
};


#endif
