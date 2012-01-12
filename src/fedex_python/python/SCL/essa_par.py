def process_nested_parent_str(attr_str):
    '''
    The first letter should be a parenthesis
    input string: "(1,4,(5,6),7)"
    output: tuple (1,4,(4,6),7)
    '''
    params = []
    agg_scope_level = 0
    current_param = ''
    for i,ch in enumerate(attr_str):
        if ch==',':
            params.append(current_param)
            current_param = ''
        elif ch=='(':
            agg_scope_level +=1
        elif ch==')':
            agg_scope_level = 0
        elif agg_scope_level == 0:
            current_param += ch
    return params

idx = 0
def process_nested_parent_str2(attr_str):
    '''
    The first letter should be a parenthesis
    input string: "(1,4,(5,6),7)"
    output: ['1','4',['5','6'],'7']
    '''
    global idx
    acc=0
    print 'Entering function with string %s and index %i'%(attr_str,idx)
    params = []
    current_param = ''
    for i,ch in enumerate(attr_str):
        idx += 1
        acc +=1
        if ch==',':
            params.append(current_param)
            current_param = ''
        elif ch=='(':
            nv = attr_str[idx:]
            print "params",params
            print "Str passed to the function:%s (idx=%i)"%(nv,idx)
            current_param = process_nested_parent_str2(nv)
        elif ch==')':
            params.append(current_param)
            idx -= acc+1
            return params
        else:
            current_param += ch
    params.append(current_param)
    return params
#print process_nested_parent_str2('1,2,3,4,5,6')
print process_nested_parent_str2("'A','B',('C','D'),'E'")