# Copyright (c) 2011, Thomas Paviot (tpaviot@gmail.com)
# All rights reserved.

# This file is part StepClassLibrary (SCL).
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

from SCL.SimpleDataTypes import *
from SCL.TypeChecker import *
from SCL.ConstructedDataTypes import *

import unittest

#
# Simple data types
#
class TestINTEGER(unittest.TestCase):
    '''
    INTEGER test
    '''
    def test_create_from_int(self):
        a = INTEGER(1)
        self.assertEqual(a,1)
        b = REAL(0)
        self.assertEqual(b,0)
        
    def test_create_from_string(self):
        a = REAL("10")
        self.assertEqual(a,10)
        b = REAL("0")
        self.assertEqual(b,0)
    
    def test_type(self):
        a = INTEGER(5)
        self.assertTrue(type(a) == INTEGER)
    
    def test_INTEGER_ops(self):
        a = INTEGER(2)
        b = INTEGER(3)
        c = a*b
        self.assertEqual(c,6)
        self.assertTrue(type(c) == int)
    
    def test_create_from_string_exception(self):
        '''
        INT cannot be constructed from an ascii string
        '''
        try:
            INTEGER("c")
        except ValueError:
            pass
        except e:
            self.fail('Unexpected exception thrown:', e)
        else:
            self.fail('ExpectedException not thrown')

class TestREAL(unittest.TestCase):
    '''
    REAL test
    '''
    def test_create_from_float(self):
        a = REAL(1.5)
        self.assertEqual(a,1.5)
        b = REAL(0)
        self.assertEqual(b,0)
        
    def test_create_from_string(self):
        a = REAL("1.5")
        self.assertEqual(a,1.5)
        b = REAL("0")
        self.assertEqual(b,0)
    
    def test_type(self):
        a = REAL(5)
        self.assertTrue(type(a) == REAL)
    
    def test_REAL_ops(self):
        a = REAL(1.5)
        b = REAL(2)
        c = a*b
        self.assertEqual(c,3)
        self.assertTrue(type(c) == float)
    
    def test_REAL_INTEGER_ops(self):
        a = REAL(5.5)
        b = INTEGER(3)
        self.assertEqual(a+b,8.5)
        self.assertTrue(type(a+b) == float)
        
    def test_create_from_string_exception(self):
        '''
        REAL cannot be constructed from an ascii string
        '''
        try:
            REAL("c")
        except ValueError:
            pass
        except e:
            self.fail('Unexpected exception thrown:', e)
        else:
            self.fail('ExpectedException not thrown')

class TestBOOLEAN(unittest.TestCase):
    '''
    BOOLEAN test
    '''
    def test_create_from_bool(self):
        a = BOOLEAN(True)
        self.assertTrue(a)
        b = BOOLEAN(False)
        self.assertFalse(b)

#
# TypeChecker
#
class TestTypeChecker(unittest.TestCase):
    def test_match_type(self):
        class P:
            pass
        p = P()
        match_type = check_type(p,P) #should return True
        self.assertTrue(match_type)
    
    def test_type_dontmatch(self):
        class P:
            pass
        p = P()
        try:
            check_type(3,P)
        except TypeError:
            pass
        except a:
            self.fail('Unexpected exception thrown:', e)
        else:
            self.fail('ExpectedException not thrown')

    def test_check_enum_type(self):
        enum = ENUMERATION(["my","string"])
        print check_type("my",enum)

unittest.main()

