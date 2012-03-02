# Copyright (c) 2011-2012, Thomas Paviot (tpaviot@gmail.com)
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
from TypeChecker import check_type
import BaseType

class Algorithm(object):
    """
    An abstract class, superclass for FUNCTION and PROCEDURE
    """
    pass

class FUNCTION(Algorithm):
    """
    EXPRESS definition:
    ===================
    A function is an algorithm which operates on parameters and that produces a single resultant
    value of a specic data type. An invocation of a function (see 12.8) in an expression evaluates
    to the resultant value at the point of invocation.
    A function shall be terminated by the execution of a return statement. The value of the
    expression, associated with the return statement, denes the result produced by the function
    call.
    Syntax:
    208 function_decl = function_head [ algorithm_head ] stmt { stmt } END_FUNCTION ';' .
    209 function_head = FUNCTION function_id [ '(' formal_parameter
    { ';' formal_parameter } ')' ] ':' parameter_type ';' .
    206 formal_parameter = parameter_id { ',' parameter_id } ':' parameter_type .
    253 parameter_type = generalized_types | named_types | simple_types .
    163 algorithm_head = { declaration } [ constant_decl ] [ local_decl ] .
    189 declaration = entity_decl | function_decl | procedure_decl | type_decl .
    Rules and restrictions:
    a) A function shall specify a return statement in each of the possible paths a process
    may take when that function is invoked.
    b) Each return statement within the scope of the function shall specify an expression
    which evaluates to the value to be returned to the point of invocation.
    c) The expression specied in each return statement shall be assignment compatible
    with the declared return type of the function.
    d) Functions do not have side-eects. Since the formal parameters to a function shall
    not be specied to be var, changes to these parameters within the function are not re
    ected
    to the point of invocation.
    e) Functions may modify local variables or parameters which are declared in an outer
    scope, i.e., if the current function is declared within the algorithm_head of a function,
    procedure or rule.
    """
    def __init__(self,function_name,params,return_type,function_definition):
        """
        param_names: a list that contains param_names as strings
        param_types: a list the contains param_types
        return type: a return type
        function_body: a string with the EXPRESS function definition
        """
        self._function_name = function_name
        self._parameters = params
        self._return_type = return_type
        self._function_definition = function_definition
        
    def __call__(self, *parameters):
        if len(parameters)!=len(self._parameters):
            raise TypeError("%s() takes exactly %i argument (%i given)"%(self._function_name,len(self._parameters),len(parameters)))
        exec self._function_python_ready

class PROCEDURE(Algorithm):
    """
    EXPRESS definition:
    ==================
    A procedure is an algorithm that receives parameters from the point of invocation and operates
    on them in some manner to produce the desired end state. Changes to the parameters within a
    procedure are only re
    ected to the point of invocation when the formal parameter is preceded
    by the var keyword.
    Syntax:
    258 procedure_decl = procedure_head [ algorithm_head ] { stmt } END_PROCEDURE ';' .
    259 procedure_head = PROCEDURE procedure_id [ '(' [ VAR ] formal_parameter
    { ';' [ VAR ] formal_parameter } ')' ] ';' .
    206 formal_parameter = parameter_id { ',' parameter_id } ':' parameter_type .
    253 parameter_type = generalized_types | named_types | simple_types .
    163 algorithm_head = { declaration } [ constant_decl ] [ local_decl ] .
    189 declaration = entity_decl | function_decl | procedure_decl | type_decl .
    Rules and restrictions:
    Procedures may modify local variables or parameters which are declared in an outer scope, i.e.,
    if the current procedure is declared within the algorithm_head of a function, procedure or
    rule.
    """
    pass

add = FUNCTION( function_name = 'add',params = [['r1', REAL],['r2', REAL],],return_type = REAL, function_definition = '''

FUNCTION add(
             r1: REAL;
             r2: REAL
    ): REAL;

  LOCAL
    result : REAL;
  END_LOCAL;
  result := r1 + r2;
  RETURN(result);

END_FUNCTION; -- add

    ''')
