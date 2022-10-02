#ifndef EXPDICT_H
#define EXPDICT_H

/**
 * \file clstepcore/ExpDict.h
 * NIST STEP Core Class Library
 * April, 1997
 * K. C. Morris
 * David Sauder
 *
 * Development of this software was funded by the United States Government,
 * and is not subject to copyright.
 *
 * As with Expdict.cc, this file has been split into many smaller files - see includes.
 */

#include <sc_export.h>
#include "clstepcore/sdai.h"

#include <vector>
#include <string>
#include <assert.h>

#include "clstepcore/SingleLinkList.h"

#include "clstepcore/baseType.h"
#include "clstepcore/dictdefs.h"
#include "clutils/Str.h"

// each of these contains linked list, list node, iterator
#include "clstepcore/attrDescriptorList.h"
#include "inverseAttributeList.h"
#include "typeDescriptorList.h"
#include "entityDescriptorList.h"


#include "typeDescriptor.h"
#include "entityDescriptor.h"
#include "enumTypeDescriptor.h"
#include "clstepcore/attrDescriptor.h"
#include "clstepcore/derivedAttribute.h"
#include "inverseAttribute.h"

#include "clstepcore/create_Aggr.h"
#include "clstepcore/dictionaryInstance.h"

#include "uniquenessRule.h"
#include "whereRule.h"

#include "interfacedItem.h"
#include "explicitItemId.h"
#include "implicitItemId.h"
#include "interfaceSpec.h"
#include "typeOrRuleVar.h"
#include "globalRule.h"
#include "clstepcore/dictSchema.h"
#include "schRename.h"

#include "clstepcore/aggrTypeDescriptor.h"
#include "selectTypeDescriptor.h"
#include "stringTypeDescriptor.h"
#include "realTypeDescriptor.h"

#endif
