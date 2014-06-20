#
# STEP Part 21 Parser
#
# Copyright (c) 2011, Thomas Paviot (tpaviot@gmail.com)
# Copyright (c) 2014, Christopher HORLER (cshorler@googlemail.com)
#
# All rights reserved.
#
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
import ply.lex as lex
import ply.yacc as yacc

####################################################################################################
# Common Code for Lexer / Parser
####################################################################################################
class Base:
    tokens = ('INTEGER', 'REAL', 'USER_DEFINED_KEYWORD', 'STANDARD_KEYWORD', 'STRING', 'BINARY',
              'ENTITY_INSTANCE_NAME', 'ENUMERATION', 'PART21_END', 'PART21_START', 'HEADER_SEC',
              'ENDSEC', 'DATA_SEC')

####################################################################################################
# Lexer 
####################################################################################################
class Lexer(Base):
    def __init__(self, debug=0, optimize=0):
        self.lexer = lex.lex(module=self, debug=debug, optimize=optimize)
        self.entity_keywords = []

    def input(self, s):
        self.lexer.input(s)

    def token(self):
        try:
            return next(self.lexer)
        except StopIteration:
            return None

    def register_entities(self, entities):
        self.entity_keywords.extend(entities)
    
    # Comment (ignored)
    def t_COMMENT(self, t):
        r'/\*(.|\n)*?\*/'
        t.lexer.lineno += t.value.count('\n')
    
    def t_PART21_START(self, t):
        r'ISO-10303-21;'
        return t

    def t_PART21_END(self, t):
        r'END-ISO-10303-21;'
        return t

    def t_HEADER_SEC(self, t):
        r'HEADER;'
        return t

    def t_ENDSEC(self, t):
        r'ENDSEC;'
        return t

    # Keywords
    # TODO: provide a hook for entity validation i.e. t.value in list_of_entities_for_AP###
    def t_STANDARD_KEYWORD(self, t):
        r'(?:!|)[A-Z_][0-9A-Z_]*'
        if t.value == 'DATA':
            t.type = 'DATA_SEC'
        elif t.value.startswith('!'):
            t.type = 'USER_DEFINED_KEYWORD'
        elif t.value in self.entity_keywords:
            t.type = t.value
        return t

    def t_newline(self, t):
        r'\n+'
        t.lexer.lineno += len(t.value)

    # Simple Data Types
    t_REAL = r'[+-]*[0-9][0-9]*\.[0-9]*(?:E[+-]*[0-9][0-9]*)?'
    t_INTEGER = r'[+-]*[0-9][0-9]*'
    t_STRING = r"'(?:[][!\"*$%&.#+,\-()?/:;<=>@{}|^`~0-9a-zA-Z_\\ ]|'')*'"
    t_BINARY = r'"[0-3][0-9A-F]*"'
    t_ENTITY_INSTANCE_NAME = r'\#[0-9]+'
    t_ENUMERATION = r'\.[A-Z_][A-Z0-9_]*\.'

    # Punctuation
    literals = '()=;,*$'

    # TODO: is it okay to ignore \n?
    t_ignore  = ' \t'

####################################################################################################
# Simple Model
####################################################################################################
class P21File:
    def __init__(self, header, *sections):
        self.header = header
        self.sections = list(*sections)

class P21Header:
    def __init__(self, file_description, file_name, file_schema):
        self.file_description = file_description
        self.file_name = file_name
        self.file_schema = file_schema
        self.extra_headers = []

class HeaderEntity:
    def __init__(self, type_name, *params):
        self.type_name = type_name
        self.params = list(*params) if params else []

class Section:
    def __init__(self, entities):
        self.entities = entities

class SimpleEntity:
    def __init__(self, ref, type_name, *params):
        self.ref = ref
        self.type_name = type_name
        self.params = list(*params) if params else []

class ComplexEntity:
    def __init__(self, ref, *params):
        self.ref = ref
        self.params = list(*params) if params else []

class TypedParameter:
    def __init__(self, type_name, *params):
        self.type_name = type_name
        self.params = list(*params) if params else None

####################################################################################################
# Parser
####################################################################################################
class Parser(Base):
    def __init__(self, lexer=None):
        if lexer is None:
            lexer = Lexer()
        self.lexer = lexer
        self.parser = yacc.yacc(module=self)

    def parse(self, p21_data, **kwargs):
        self.lexer.input(p21_data)
        self.refs = {}
        result = self.parser.parse(lexer=self.lexer, **kwargs)
        return result

    def p_exchange_file(self, p):
        """exchange_file : PART21_START header_section data_section_list PART21_END"""
        p[0] = P21File(p[2], p[3])

    # TODO: Specialise the first 3 header entities
    def p_header_section(self, p):
        """header_section : HEADER_SEC header_entity header_entity header_entity ENDSEC"""
        p[0] = P21Header(p[2], p[3], p[4])

    def p_header_section_with_entity_list(self, p):
        """header_section : HEADER_SEC header_entity header_entity header_entity header_entity_list ENDSEC"""
        p[0] = P21Header(p[2], p[3], p[4])
        p[0].extra_headers.extend(p[5])

    def p_header_entity(self, p):
        """header_entity : keyword '(' parameter_list ')' ';'"""
        p[0] = HeaderEntity(p[1], p[3])

    def p_check_entity_instance_name(self, p):
        """check_entity_instance_name : ENTITY_INSTANCE_NAME"""
        if p[1] in self.refs:
            print('ERROR:  Line {0}, duplicate entity instance name: {1}'.format(p.lineno(1), p[1]), file=sys.stderr)
            raise SyntaxError
        else:
            self.refs[p[1]] = None
            p[0] = p[1]

    def p_simple_entity_instance(self, p):
        """simple_entity_instance : check_entity_instance_name '=' simple_record ';'"""
        p[0] = SimpleEntity(p[1], *p[3])

    def p_complex_entity_instance(self, p):
        """complex_entity_instance : check_entity_instance_name '=' subsuper_record ';'"""
        p[0] = ComplexEntity(p[1], p[3])

    def p_subsuper_record(self, p):
        """subsuper_record : '(' simple_record_list ')'"""
        p[0] = [TypedParameter(*x) for x in p[2]]

    def p_data_section_list(self, p):
        """data_section_list : data_section_list data_section
                             | data_section"""
        try: p[0] = p[1] + [p[2],]
        except IndexError: p[0] = [p[1],]

    def p_header_entity_list(self, p):
        """header_entity_list : header_entity_list header_entity
                              | header_entity"""
        try: p[0] = p[1] + [p[2],]
        except IndexError: p[0] = [p[1],]

    def p_parameter_list(self, p):
        """parameter_list : parameter_list ',' parameter
                          | parameter"""
        try: p[0] = p[1] + [p[3],]
        except IndexError: p[0] = [p[1],]

    def p_keyword(self, p):
        """keyword : USER_DEFINED_KEYWORD
                   | STANDARD_KEYWORD"""
        p[0] = p[1]

    def p_parameter_simple(self, p):
        """parameter : STRING
                     | INTEGER
                     | REAL
                     | ENTITY_INSTANCE_NAME
                     | ENUMERATION
                     | BINARY
                     | '*'
                     | '$'
                     | typed_parameter
                     | list_parameter"""
        p[0] = p[1]

    def p_list_parameter(self, p):
        """list_parameter : '(' parameter_list ')'"""
        p[0] = p[2]

    def p_typed_parameter(self, p):
        """typed_parameter : keyword '(' parameter ')'"""
        p[0] = TypedParameter(p[1], p[3])

    def p_parameter_empty_list(self, p):
        """parameter : '(' ')'"""
        p[0] = []

    def p_data_start(self, p):
        """data_start : DATA_SEC '(' parameter_list ')' ';'"""
        pass

    def p_data_start_empty(self, p):
        """data_start : DATA_SEC '(' ')' ';'
                      | DATA_SEC ';'"""
        pass

    def p_data_section(self, p):
        """data_section : data_start entity_instance_list ENDSEC""" 
        p[0] = Section(p[2])

    def p_entity_instance_list(self, p):
        """entity_instance_list : entity_instance_list entity_instance
                                | empty"""
        try: p[0] = p[1] + [p[2],]
        except IndexError: pass # p[2] doesn't exist, p[1] is None
        except TypeError: p[0] = [p[2],] # p[1] is None, p[2] is valid

    def p_entity_instance(self, p):
        """entity_instance : simple_entity_instance
                           | complex_entity_instance"""
        p[0] = p[1]

        
    def p_simple_record_empty(self, p):
        """simple_record : keyword '(' ')'"""
        p[0] = (p[1], [])

    def p_simple_record_with_params(self, p):
        """simple_record : keyword '(' parameter_list ')'"""
        p[0] = (p[1], p[3])

    def p_simple_record_list(self, p):
        """simple_record_list : simple_record_list simple_record
                              | simple_record"""
        try: p[0] = p[1] + [p[2],]
        except IndexError: p[0] = [p[1],]
   
    def p_empty(self, p):
        """empty :"""
        pass

    def p_error(self, p):
        raise SyntaxError


def test():
    import logging
    logger = logging.getLogger()
    logger.setLevel(logging.DEBUG)

    s = open('io1-tu-203.stp', 'r').read()
    parser = Parser()

    #r = parser.parse(s, debug=logger)
    r = parser.parse(s)
    return r

if __name__ == '__main__':
    test()

