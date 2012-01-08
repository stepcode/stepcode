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

from SimpleDataTypes import *

class BaseAggregate(object):
    """ A class that define common properties to ARRAY, LIST, SET and BAG.
    """
    def __init__( self ,  bound1 , bound2 , base_type ):
        # init base list with an empty list
        list.__init__(self,[])
        # check that bound1<bound2
        if (bound1!=None and bound2!=None):
            if bound1>bound2:
                raise AssertionError("bound1 shall be less than or equal to bound2")
        self._bound1 = bound1
        self._bound2 = bound2
        self._base_type = base_type
        print base_type
    
    def __getitem__(self, index):
        if index<self._bound1:
            raise IndexError("ARRAY index out of bound (lower bound is %i, passed %i)"%(self._bound1,index))
        elif(self._bound2!=None and index>self._bound2):
            raise IndexError("ARRAY index out of bound (upper bound is %i, passed %i)"%(self._bound2,index))
        else:
            return list.__getitem__(self,index)
    
    def __setitem__(self,index,value):
        if index<self._bound1:
            raise IndexError("ARRAY index out of bound (lower bound is %i, passed %i)"%(self._bound1,index))
        elif (self._bound2!=None and index>self._bound2):
            raise IndexError("ARRAY index out of bound (upper bound is %i, passed %i)"%(self._bound2,index))
        elif not isinstance(value,self._base_type):
            raise TypeError("%s type expected, passed %s."%(self._base_type, type(value)))
        else:
            # first find the length of the list, and extend it if ever
            # the index is 
            list.__setitem__(self,index,value)

class ARRAY(list, BaseAggregate):
    """An array data type has as its domain indexed, fixed-size collections of like elements. The lower
    and upper bounds, which are integer-valued expressions, define the range of index values, and
    thus the size of each array collection.
    An array data type definition may optionally specify
    that an array value cannot contain duplicate elements.
    It may also specify that an array value
    need not contain an element at every index position.
    """
    def __init__( self ,  bound1 , bound2 , base_type ):
        BaseAggregate.__init__( self ,  bound1 , bound2 , base_type )
 
class LIST(list, BaseAggregate):
    """A list data type has as its domain sequences of like elements. The optional lower and upper
    bounds, which are integer-valued expressions, dfine the minimum and maximum number of
    elements that can be held in the collection defined by a list data type.
    A list data type
    definition may optionally specify that a list value cannot contain duplicate elements.
    """
    def __init__( self ,  bound1 , bound2 , base_type ):
         BaseAggregate.__init__( self ,  bound1 , bound2 , base_type )
    
class BAG(tuple, BaseAggregate):
    """A bag data type has as its domain unordered collections of like elements. The optional lower
    and upper bounds, which are integer-valued expressions, dene the minimum and maximum
    number of elements that can be held in the collection dened by a bag data type.
    """
    def __init__( self ,  bound1 , bound2 , base_type ):
         BaseAggregate.__init__( self ,  bound1 , bound2 , base_type )

class SET(set, BaseAggregate):
    """A set data type has as its domain unordered collections of like elements. The set data type is
    a specialization of the bag data type. The optional lower and upper bounds, which are integer-
    valued expressions, dene the minimum and maximum number of elements that can be held in
    the collection dened by a set data type. The collection dened by set data type shall not
    contain two or more elements which are instance equal.
    """
    def __init__( self ,  bound1 , bound2 , base_type ):
         BaseAggregate.__init__( self ,  bound1 , bound2 , base_type )

if __name__=='__main__':
    # test ARRAY
    a = ARRAY(1,3,REAL)
    a[1] = REAL(3.)
    a[2] = REAL(-1.5)
    a[3] = REAL(4.)
    print a
    
    
