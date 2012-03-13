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

import unittest
import sys
sys.path.append('../examples/unitary_schemas')

from SCL.SCLBase import *
from SCL.SimpleDataTypes import *
from SCL.ConstructedDataTypes import *
from SCL.AggregationDataTypes import *
from SCL.TypeChecker import check_type
from SCL.Expr import *

class TestSelectDataType(unittest.TestCase):
    '''
    unitary_schemas/test_select_data_type.py
    '''
    def test_import_schema_module(self):
        import test_select_data_type
    
    def test_schema(self):
        import test_select_data_type
        my_glue = test_select_data_type.glue(STRING("comp"),STRING("solvent"))
        wm = test_select_data_type.wall_mounting(STRING("mounting"),STRING("on"),my_glue)

class TestSingleInheritance(unittest.TestCase):
    '''
    unitary_schemas/test_single_inheritance.py, generated from:
    SCHEMA test_single_inheritance;

    TYPE label = STRING;
    END_TYPE;

    TYPE length_measure = REAL;
    END_TYPE;

    TYPE point = REAL;
    END_TYPE;

    ENTITY shape
    SUPERTYPE OF (rectangle);
        item_name : label;
        number_of_sides : INTEGER;
    END_ENTITY;

    ENTITY rectangle
    SUBTYPE OF (shape);
        height : length_measure;
        width : length_measure;
    END_ENTITY;

    END_SCHEMA;
    '''
    def test_import_schema_module(self):
        import test_single_inheritance
    
    def test_schema(self):
        import test_single_inheritance
        my_base_shape = test_single_inheritance.shape(test_single_inheritance.label("spherical shape"),INTEGER(1))
        my_shape = test_single_inheritance.rectangle(test_single_inheritance.label("rect"),INTEGER(6),test_single_inheritance.length_measure(30.0),test_single_inheritance.length_measure(45.))


class TestSingleInheritanceMultiLevel(unittest.TestCase):
    '''
    unitary_schemas/test_single_inheritance_multi_level.py, generated from schema:
    SCHEMA test_single_inheritance_multi_level;

    TYPE label = STRING;
    END_TYPE;

    TYPE length_measure = REAL;
    END_TYPE;

    TYPE point = REAL;
    END_TYPE;

    ENTITY shape
    SUPERTYPE OF (subshape);
        item_name : label;
        number_of_sides : INTEGER;
    END_ENTITY;

    ENTITY subshape
    SUPERTYPE OF (rectangle)
    SUBTYPE OF (shape);
    END_ENTITY;

    ENTITY rectangle
    SUBTYPE OF (subshape);
        height : length_measure;
        width : length_measure;
    END_ENTITY;

    END_SCHEMA;
    '''
    def test_import_schema_module(self):
        import test_single_inheritance_multi_level
    
    def test_schema(self):
        import test_single_inheritance_multi_level
        my_base_shape = test_single_inheritance_multi_level.shape(STRING("spherical shape"),INTEGER(1))
        my_shape = test_single_inheritance_multi_level.rectangle(STRING("rect"),INTEGER(6), height = REAL(30.0), width = REAL(45.))


class TestEnumEntityName(unittest.TestCase):
    '''
    unitary_schemas/test_enum_entity_name.py, generated from schema:
    SCHEMA test_enum_entity_name;

    TYPE simple_datum_reference_modifier = ENUMERATION OF (
       line,
       translation);
    END_TYPE;

    ENTITY line;
    END_ENTITY;

    END_SCHEMA;
    '''
    def test_import_schema_module(self):
        import test_enum_entity_name
    
    def test_schema(self):
        import test_enum_entity_name
        check_type(test_enum_entity_name.simple_datum_reference_modifier.line,test_enum_entity_name.simple_datum_reference_modifier)
        check_type(test_enum_entity_name.translation,test_enum_entity_name.simple_datum_reference_modifier)
        # if line is passed, should raise an error since it is an entity name
        try:
            check_type(test_enum_entity_name.line,test_enum_entity_name.simple_datum_reference_modifier)      
        except TypeError:
            pass
        except e:
            self.fail('Unexpected exception thrown:', e)
        else:
            self.fail('ExpectedException not thrown')
        
        
class TestArray(unittest.TestCase):
    '''
    unitary_schemas/test_array.py, generated from schema:
    SCHEMA test_array;

    ENTITY point;
        coords : ARRAY[1:3] OF REAL;
    END_ENTITY;

    END_SCHEMA;
    '''
    def test_import_schema_module(self):
        import test_array
    
    def test_schema(self):
        import test_array
        scp = sys.modules[__name__]
        my_coords = ARRAY(1,3,REAL)
        my_point = test_array.point(my_coords)
        # following cases should raise error
        # 1. passed LIST whereas ARRAY expected
        my_coords = LIST(1,3,REAL)
        try:
            my_point = test_array.point(my_coords)      
        except TypeError:
            pass
        except e:
            self.fail('Unexpected exception thrown:', e)
        else:
            self.fail('ExpectedException not thrown')
        # 2. passed ARRAY OF INTEGER whereas ARRAY OF REAL expected
        my_coords = ARRAY(1,3,INTEGER)
        try:
            my_point = test_array.point(my_coords)      
        except TypeError:
            pass
        except e:
            self.fail('Unexpected exception thrown:', e)
        else:
            self.fail('ExpectedException not thrown')
        # 3. passed UNIQUE= True whereas False expected
        my_coords = ARRAY(1,3,REAL, UNIQUE=True)
        try:
            my_point = test_array.point(my_coords)      
        except TypeError:
            pass
        except e:
            self.fail('Unexpected exception thrown:', e)
        else:
            self.fail('ExpectedException not thrown')
        # 4. passed OPTIONAL= True whereas False expected
        my_coords = ARRAY(1,3,REAL, OPTIONAL=True)
        try:
            my_point = test_array.point(my_coords)      
        except TypeError:
            pass
        except e:
            self.fail('Unexpected exception thrown:', e)
        else:
            self.fail('ExpectedException not thrown')
                  
class TestArrayOfArrayOfReal(unittest.TestCase):
    '''
    unitary_schemas/test_array_of_array_of_real.py, generated from schema:
    SCHEMA test_array_of_array_of_simple_types;

    ENTITY transformation;
        rotation : ARRAY[1:3] OF ARRAY[1:3] OF REAL;
    END_ENTITY;

    END_SCHEMA;

    '''
    def test_import_schema_module(self):
        import test_array_of_array_of_simple_types

    def test_schema(self):
        import test_array_of_array_of_simple_types
        scp = sys.modules[__name__]
        # first build the double array my_matrix
        my_matrix = ARRAY(1,3,ARRAY(1,3,REAL))
        my_matrix[1] = ARRAY(1,3,REAL)
        my_matrix[2] = ARRAY(1,3,REAL)
        my_matrix[3] = ARRAY(1,3,REAL)
        my_matrix[1][1] = REAL(1.0)
        my_matrix[2][2] = REAL(1.0)
        my_matrix[3][3] = REAL(1.0)
        # create a new 'transformation' instance
        trsf = test_array_of_array_of_simple_types.transformation(my_matrix)

def suite():
   suite = unittest.TestSuite()
   suite.addTest(unittest.makeSuite(TestSelectDataType))
   suite.addTest(unittest.makeSuite(TestSingleInheritance))
   suite.addTest(unittest.makeSuite(TestSingleInheritanceMultiLevel))
   suite.addTest(unittest.makeSuite(TestEnumEntityName))
   return suite

if __name__ == '__main__':
    unittest.main()


