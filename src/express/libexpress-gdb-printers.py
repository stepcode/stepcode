
#https://www.rethinkdb.com/blog/make-debugging-easier-with-custom-pretty-printers/

class Variable:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        t=Type(self.val['type']).to_string()
        s=Symbol(self.val['name']['symbol']).to_string()
        return t + ' ' + s

class Symbol:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        if not self.val:
            return ""
        fn = os.path.basename(get_char_star(self.val['filename']))
        res='  '
        if self.val['resolved']==0:
            res='??'
        name=get_char_star(self.val['name'])
        #myvar ??[file.exp:123] (where ?? indicates not resolved)
        v="{} {}[{}:{}]".format(name,res,fn,self.val['line'])
        return v

def get_char_star(val):
    '''make possibly-null strings print sanely'''
    if not val:
        return ''
    try:
        return str(val.string())
    except:
        return ''

class Type:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        if self.val['type'] != ord('t'):
            return str(self.val['type'])
        t = str(self.val['u']['type']['body']['type'])
        return t[:len(t)-1].upper()

import gdb.printing
import os.path

def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("STEPcode - libexpress")
    #pp.add_printer('Variable', '^Variable$', Variable)
    pp.add_printer('Symbol', '\*?Symbol', Symbol)
    pp.add_printer('Type', '^Type$', Type)
    return pp

gdb.printing.register_pretty_printer( gdb.current_objfile(), build_pretty_printer())
