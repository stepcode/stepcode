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

class ENUMERATION(list):
    """ An ENUMERATION data type has as its domain an ordered set of names. The names represent
    values of the enumeration data type.
    """
    def __getattr__(self,attr_name):
        if attr_name in self:
            return attr_name
        else:
            raise AttributeError("This ENUMERATION has no element '%s'"%attr_name)

class SELECT(list):
    """ A select data type has as its domain the union of the domains of the named data types in
    its select list. The select data type is a generalization of each of the named data types in its
    select list.
    """
    def get_aggregated_allowed_types(self):
        """ This method returns a list of all types that are handle by this SELECT.
        A SELECT can actually be an aggregate of many SELECTs"""
        agg = []
        for allowed_type in self:
            if isinstance(allowed_type,SELECT):
                # in this case, we should recurse the select to get all subtypes
                b = allowed_type.get_aggregated_allowed_types()
                for elem in b:
                    agg.append(elem)
            else:
                agg.append(allowed_type)
        return agg
                
if __name__=='__main__':
    a = SELECT([1,2])
    b = SELECT([3,4])
    c = SELECT([a,b])
    print a.get_aggregated_allowed_types()
    print c.get_aggregated_allowed_types()
    