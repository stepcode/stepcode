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

import re

INSTANCE_DEFINITION_RE = re.compile("#(\d+)[^\S\n]?=[^\S\n]?(.*?)\((.*)\)[^\S\n]?;[\\r]?$")

def map_string_to_num(stri):
    """ Take a string, check wether it is an integer, a float or not
    """
    if ('.' in stri) or ('E' in stri): #it's definitely a float
        return REAL(stri)
    else:
        return INTEGER(stri)

class Model:
    """
    A model contains a list of instances
    """
    def __init__(self,name):
        self._name = name
        # a dict of instances
        # each time an instance is added to the model, count is incremented
        self._instances = {}
        self._number_of_instances = 0
        
    def add_instance(self, instance):
        '''
        Adds an instance to the model
        '''
        self._number_of_instances += 1
        self._instances[self._number_of_instances-1] = instance

    def print_instances(self):
        '''
        Dump instances to stdout
        '''
        for idx in range(self._number_of_instances):
            "=========="
            print "Instance #%i"%(idx+1)
            print self._instances[idx]
        
class Part21Loader:
    """
    Loads all instances definition of a Part21 file into memory.
    Two dicts are created:
    self._instance_definition : stores attibutes, key is the instance integer id
    self._number_of_ancestors : stores the number of ancestors of entity id. This enables
    to define the order of instances creation.
    """
    def __init__(self, filename):
        self._filename = filename
        # the schema
        self._schema = ""
        # the dict self._instances contain instance definition
        self._instances_definition = {}
        # this dict contains lists of 0 ancestors, 1 ancestor, etc.
        # initializes this dict
        self._number_of_ancestors = {}
        for i in range(2000):
            self._number_of_ancestors[i]=[]
        print "Parsing file %s..."%filename,
        self.parse_file()
        print "done."
        # reduce number_of_ancestors dict
        for item in self._number_of_ancestors.keys():
            if len(self._number_of_ancestors[item])==0:
                del self._number_of_ancestors[item]
    
    def get_schema(self):
        return self._schema
        
    def get_number_of_instances(self):
        return len(self._instances_definition.keys())
        
    def parse_file(self):
        fp = open(self._filename)
        for line in fp:
            # parse line
            match_instance_definition = INSTANCE_DEFINITION_RE.search(line)  # id,name,attrs
            if match_instance_definition:
                instance_id, entity_name, entity_attrs = match_instance_definition.groups()
                instance_int_id = int(instance_id)
                # find number of ancestors
                number_of_ancestors = entity_attrs.count('#')
                # fill number of ancestors dict
                self._number_of_ancestors[number_of_ancestors].append(instance_int_id)
                self._instances_definition[instance_int_id] = [entity_name,entity_attrs]
            else: #does not match with entity instance definition, parse the header
                if line.startswith('FILE_SCHEMA'):
                    #identify the schema name
                    self._schema = line.split("'")[1].split("'")[0].split(" ")[0].lower()
        fp.close()
    
class Part21Population(object):
    def __init__(self, part21_loader):
        """ Take a part21_loader a tries to create entities
        """
        self._part21_loader = part21_loader
        self._aggregate_scope = []
        self._aggr_scope = False
    
    def create_entity_instances(self):
        """ Starts entity instances creation
        """
        for number_of_ancestor in self._part21_loader._number_of_ancestors.keys():
            for entity_definition_id in self._part21_loader._number_of_ancestors[number_of_ancestor]:
                self.create_entity_instance(entity_definition_id)
    
    def create_entity_instance(self, instance_id):
        instance_definition = self._part21_loader._instances_definition[instance_id]
        # first find class name
        class_name = instance_definition[0].lower()
        object_ = globals()[class_name]
        # then attributes
        #print object_.__doc__
        instance_attributes = instance_definition[1]
        #print instance_attributes
        # find attributes
        attributes = instance_attributes.split(",")
        instance_attributes = []
        #print attributes
        for attr in attributes:
            if attr[0]=="(":#new aggregate_scope
                #print "new aggregate scope"
                self._aggr_scope = True
                at = self.map_express_to_python(attr[1:])
                #self._aggregate_scope.append(at)
            elif attr[-1]==")":
                #print "end aggregate scope"
                at = self.map_express_to_python(attr[:-1])
                self._aggregate_scope.append(at)
                self._aggr_scope = False
            else:
                at = self.map_express_to_python(attr)
            if self._aggr_scope:
                self._aggregate_scope.append(at)
            if len(self._aggregate_scope)>0 and not self._aggr_scope:
                instance_attributes.append(self._aggregate_scope)
                self._aggregate_scope = []
            elif len(self._aggregate_scope)>0 and self._aggr_scope:
                pass
            else:
                instance_attributes.append(at)
        a = object_(*instance_attributes)
        

    def map_express_to_python(self,attr):
        """ Map EXPRESS to python"""
        if attr in ["$","''"]: #optional argument
            return None
        elif attr.startswith('#'): #entity_id
            return attr[1:]
        else:
            return map_string_to_num(attr)

if __name__ == "__main__":
    import time
    import sys
    sys.path.append("..")
    from config_control_design import *
    t1 = time.time()
    print "Loading file into memory"
    #file = Part21Loader("as1-oc-214.stp")
    #file = Part21Loader("as1-tu-203.stp")
    #file = Part21Loader("HAYON.stp")
    p21loader = Part21Loader("as1.stp")
    t2 = time.time()
    print "Loading schema took: %s s \n" % ((t2-t1))
    t1 = time.time()
    print "Creating instances"
    p21population = Part21Population(p21loader)
    p21population.create_entity_instances()
    t2 = time.time()
    print "Creating instances took: %s s \n" % ((t2-t1))