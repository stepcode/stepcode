
#ifndef _STEPattributeList_h
#define _STEPattributeList_h 1

/*
* NIST STEP Core Class Library
* clstepcore/STEPattributeList.h
* April 1997
* K. C. Morris
* David Sauder

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

class STEPattribute;
#include <SingleLinkList.h>

class STEPattributeList;

class AttrListNode :  public SingleLinkNode 
{
  friend class STEPattributeList;

  protected:
    STEPattribute *attr;

  public:
    AttrListNode(STEPattribute *a);
    virtual ~AttrListNode();

};

class STEPattributeList : public SingleLinkList
{
  public:
    STEPattributeList();
    virtual ~STEPattributeList();

    STEPattribute& operator [] (int n);
    int list_length();
    void push(STEPattribute *a);
};

/*************************************************************//**
**      \file STEPattributeList.h
**      This file defines the type STEPattributeList -- a list  
**      of pointers to STEPattribute objects.  The nodes on the 
**      list point to STEPattributes.  
**                                                              
**      The file is dependent on the file "STEPattribute.h"     
**      which contains the definition of STEPattribute.         
**                                                              
**      1/15/91  kcm                                            
*****************************************************************/

#endif
