# Copyright (c) 2011, Thomas Paviot (tpaviot@gmail.com)
# All rights reserved.

# This file is part of the StepClassLibrary (SCL).
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#   Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
#
#   Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
#   Neither the name of the <ORGANIZATION> nor the names of its contributors may
#   be used to endorse or promote products derived from this software without
#   specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import sys
import BaseType

def type_is_defined(type_str):
    """ Look for the type definition in the global scope from the type string.
    @TODO: find a better implementation than testing all modules!
    """
    modules = sys.modules
    for module in modules.values():
        if (module is not None) and (not '__' in module.__name__):
            module_variables = vars(module)
            if module_variables.has_key(type_str):
                typ = module_variables[type_str]
                return True, vars(module)[type_str]
    return False,None

class EnumerationId(object):
    """ An enumeration data type has as its domain an ordered set of names. The names represent
    values of the enumeration data type. These names are designated by enumeration_ids and are
    referred to as enumeration items.
    
    Python implementation: before being defined, enums Id of the related enumeration must be
    defined
    ahead = EnumerationId()
    behind = EnumerationId()
    """
    pass

class ENUMERATION(object):
    """ EXPRESS definition : An ENUMERATION data type has as its domain an ordered set of names. The names represent
    values of the enumeration data type.
    
    Python implementation:
    An enumeration is initialized from strings defining the types.
    For instance, some EXPRESS definition:
    TYPE ahead_or_behind = ENUMERATION OF
      (ahead,
       behind);
    END_TYPE; -- ahead_or_behind
    
    is implemented in python with the line:
    ahead_of_behind = ENUMERATION('ahead','behind')
    
    The ENUMERATION definition takes python strings because of the resolution ordre
    that could be an issue.
    
    When getting the authorized types but the ENUMERATION, python looks for the object
    definition from the globals() dictionary.
    
    ahead = EnumerationId()
    behind = EnumerationId()
    ahead_of_behind = ENUMERATION(ahead,behind)
    """
    def __init__(self,*kargs):
        # first defining the scope
        passed_types = list(kargs)
        # first check that all arguments are enums id
        for passed_type in passed_types:
            if not isinstance(passed_type, EnumerationId):
                raise TypeError("%s is not an enumeration identifier."%passed_type)
        self._enum_ids = list(kargs)

    def get_allowed_enum_id(self):
        return self._enum_ids
        
class SELECT(object):
    """ A select data type has as its domain the union of the domains of the named data types in
    its select list. The select data type is a generalization of each of the named data types in its
    select list.
    """
    def __init__(self,*kargs,**args):
        # first defining the scope
        if args.has_key('scope'):
            self._scope = args['scope']
        else:
            self._scope = None
        # create the types from the list of arguments
        self._base_types = []
        for types in list(kargs):
            new_type = BaseType.Type(types,self._scope)
            self._base_types.append(new_type)
 
    def get_allowed_types(self):
        _auth_types = []
        for types in self._base_types:
            _auth_types.append(types.get_type())
        return _auth_types

    def get_allowed_basic_types(self):
        ''' if a select contains some subselect, goes down through the different
        sublayers untill there is no more '''
        b = []
        _auth_types = self.get_allowed_types()
        for _auth_type in _auth_types:
            if isinstance(_auth_type,SELECT) or isinstance(_auth_type,ENUMERATION):
                h = _auth_type.get_allowed_types()
                b.extend(h)
            else:
                b = _auth_types
        return b
        
if __name__=='__main__':
    class line:
        pass
    class point:
        pass
    import sys
    scp = sys.modules[__name__]
    a = ENUMERATION('line','point',scope = scp)
    print a.get_allowed_types()
    