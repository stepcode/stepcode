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
import logging
import ply.lex as lex
import ply.yacc as yacc

logger = logging.getLogger(__name__)
logger.addHandler(logging.NullHandler())

####################################################################################################
# Common Code for Lexer / Parser
####################################################################################################
class Base:
    tokens = ['INTEGER', 'REAL', 'USER_DEFINED_KEYWORD', 'STANDARD_KEYWORD', 'STRING', 'BINARY',
              'ENTITY_INSTANCE_NAME', 'ENUMERATION', 'PART21_END', 'PART21_START', 'HEADER_SEC',
              'ENDSEC', 'DATA_SEC']

####################################################################################################
# Lexer 
####################################################################################################
class Lexer(Base):
    states = (('compatibility', 'inclusive'),)

    def __init__(self, debug=0, optimize=0, compatibility_mode=False, header_limit=1024, extra_tokens=None):
        if extra_tokens: self.tokens += extra_tokens
        self.entity_mapping = {}
        self.compatibility_mode = compatibility_mode
        self.header_limit = header_limit
        self.lexer = lex.lex(module=self, debug=debug, debuglog=logger, optimize=optimize,
                             errorlog=logger)

    def __getattr__(self, name):
        if name == 'lineno':
            return self.lexer.lineno
        elif name == 'lexpos':
            return self.lexer.lexpos
        else:
            raise AttributeError

    def input(self, s):
        startidx = s.find('ISO-10303-21;', 0, self.header_limit)
        # TODO: It would be better to raise a suitable exception and
        #       let the application decide how to handle it.
        if startidx == -1:
            sys.exit('Aborting... ISO-10303-21; header not found')
        self.lexer.input(s[startidx:])
        self.lexer.lineno += s[0:startidx].count('\n')

        if self.compatibility_mode:
            self.lexer.begin('compatibility')
        else:
            self.lexer.begin('INITIAL')
 
    def token(self):
        try:
            return next(self.lexer)
        except StopIteration:
            return None

    def register_entities(self, entities):
        if isinstance(entities, list):
            entities = {k: k for k in entities}

        self.entity_mapping.update(entities)
    
    # Comment (ignored)
    def t_ANY_COMMENT(self, t):
        r'/\*(.|\n)*?\*/'
        t.lexer.lineno += t.value.count('\n')
    
    def t_ANY_PART21_START(self, t):
        r'ISO-10303-21;'
        return t

    def t_ANY_PART21_END(self, t):
        r'END-ISO-10303-21;'
        return t

    def t_ANY_HEADER_SEC(self, t):
        r'HEADER;'
        return t

    def t_ANY_ENDSEC(self, t):
        r'ENDSEC;'
        return t

    # Keywords
    def t_compatibility_STANDARD_KEYWORD(self, t):
        r'(?:!|)[A-Za-z_][0-9A-Za-z_]*'
        t.value = t.value.upper()
        if t.value == 'DATA':
            t.type = 'DATA_SEC'
        elif t.value in self.entity_mapping:
            t.type = self.entity_mapping[t.value]
        elif t.value.startswith('!'):
            t.type = 'USER_DEFINED_KEYWORD'
        return t

    def t_ANY_STANDARD_KEYWORD(self, t):
        r'(?:!|)[A-Z_][0-9A-Z_]*'
        if t.value == 'DATA':
            t.type = 'DATA_SEC'
        elif t.value in self.entity_mapping:
            t.type = self.entity_mapping[t.value]
        elif t.value.startswith('!'):
            t.type = 'USER_DEFINED_KEYWORD'
        return t

    def t_ANY_newline(self, t):
        r'\n+'
        t.lexer.lineno += len(t.value)

    # Simple Data Types
    t_ANY_REAL = r'[+-]*[0-9][0-9]*\.[0-9]*(?:E[+-]*[0-9][0-9]*)?'
    t_ANY_INTEGER = r'[+-]*[0-9][0-9]*'
    t_ANY_STRING = r"'(?:[][!\"*$%&.#+,\-()?/:;<=>@{}|^`~0-9a-zA-Z_\\ ]|'')*'"
    t_ANY_BINARY = r'"[0-3][0-9A-F]*"'
    t_ANY_ENTITY_INSTANCE_NAME = r'\#[0-9]+'
    t_ANY_ENUMERATION = r'\.[A-Z_][A-Z0-9_]*\.'

    # Punctuation
    literals = '()=;,*$'

    t_ANY_ignore  = ' \t'
            

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
    start = 'exchange_file'

    def __init__(self, lexer=None, debug=0):
        self.parser = yacc.yacc(module=self, debug=debug, debuglog=logger, errorlog=logger)

        if lexer is None:
            lexer = Lexer()
        self.lexer = lexer

    def parse(self, p21_data, **kwargs):
        self.lexer.input(p21_data)
        self.refs = {}
        self.in_p21_exchange_structure = False

        if 'debug' in kwargs:
            result = self.parser.parse(lexer=self.lexer, debug=logger,
                                       **{ k: kwargs[k] for k in kwargs if k != 'debug'})
        else:
            result = self.parser.parse(lexer=self.lexer, **kwargs)
        return result

    def p_exchange_file(self, p):
        """exchange_file : p21_start header_section data_section_list p21_end"""
        p[0] = P21File(p[2], p[3])

    def p_p21_start(self, p):
        """p21_start : PART21_START"""
        if self.in_p21_exchange_structure:
            raise SyntaxError
        self.in_p21_exchange_structure = True
        p[0] = p[1]

    def p_p21_end(self, p):
        """p21_end : PART21_END"""
        if not self.in_p21_exchange_structure:
            raise SyntaxError
        self.in_p21_exchange_structure = False
        p[0] = p[1]

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
            logger.error('Line %i, duplicate entity instance name: %s', p.lineno(1), p[1])
            sys.exit('Aborting...')
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

def test_debug():
    logging.basicConfig()
    logger.setLevel(logging.DEBUG)

    s = open('io1-tu-203.stp', 'r').read()
    parser = Parser()

    try:
        r = parser.parse(s, debug=1)
    except SystemExit:
        pass

    return (parser, r)

def test():
    logging.basicConfig()
    logger.setLevel(logging.ERROR)

    s = open('io1-tu-203.stp', 'r').read()
    parser = Parser()

    try:
        r = parser.parse(s)
    except SystemExit:
        pass

    return (parser, r)


if __name__ == '__main__':
    test()

