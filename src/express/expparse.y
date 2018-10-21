%start_symbol express_file

%name yyparse 

%extra_argument { struct YYSTATE *pState }
%token_type { struct YYSTYPE * }
%stack_size 0

/* type definitions */
%type case_action { Case_Item }
%type case_otherwise { Case_Item }

%type case_action_list { Linked_List }
%type case_label_list { Linked_List }

%type expression { Expression }

/************** OLD type definitions ***************/
/*
%type entity_body            { struct entity_body }

%type aggregate_init_element        { Expression }
%type aggregate_initializer        { Expression }
%type assignable            { Expression }
%type attribute_decl            { Expression }
%type by_expression            { Expression }
%type constant                { Expression }

%type function_call            { Expression }
%type general_ref            { Expression }
%type group_ref                { Expression }
%type identifier            { Expression }
%type initializer            { Expression }
%type interval                { Expression }
%type literal                { Expression }
%type local_initializer            { Expression }
%type precision_spec            { Expression }
%type query_expression            { Expression }
%type query_start            { Expression }
%type simple_expression            { Expression }
%type unary_expression            { Expression }
%type supertype_expression        { Expression }
%type until_control            { Expression }
%type while_control            { Expression }

%type function_header            { Integer }
%type fh_lineno                { Integer }
%type rule_header            { Integer }
%type rh_start                { Integer }
%type rh_get_line            { Integer }
%type procedure_header            { Integer }
%type ph_get_line            { Integer }

%type action_body            { Linked_List }
%type actual_parameters            { Linked_List }
%type aggregate_init_body        { Linked_List }
%type explicit_attr_list        { Linked_List }
%type case_block            { Linked_List }
%type case_labels            { Linked_List }
%type where_clause_list            { Linked_List }
%type derive_decl            { Linked_List }
%type explicit_attribute        { Linked_List }
%type expression_list            { Linked_List }
%type formal_parameter            { Linked_List }
%type formal_parameter_list        { Linked_List }
%type formal_parameter_rep        { Linked_List }
%type id_list                { Linked_List }
%type defined_type_list            { Linked_List }
%type nested_id_list            { Linked_List }
%type statement_rep            { Linked_List }
%type subtype_decl            { Linked_List }
%type where_rule            { Linked_List }
%type where_rule_OPT            { Linked_List }
%type supertype_expression_list        { Linked_List }
%type labelled_attrib_list_list        { Linked_List }
%type labelled_attrib_list        { Linked_List }
%type inverse_attr_list            { Linked_List }

%type inverse_clause            { Linked_List }
%type attribute_decl_list        { Linked_List }
%type derived_attribute_rep        { Linked_List }
%type unique_clause            { Linked_List }
%type rule_formal_parameter_list    { Linked_List }
%type qualified_attr_list        { Linked_List }

%type rel_op            { Op_Code }

%type optional_or_unique    { struct type_flags }
%type optional_fixed        { struct type_flags }
%type optional            { struct type_flags }
%type var            { struct type_flags }
%type unique            { struct type_flags }

%type qualified_attr        { Expression }

%type qualifier            { struct qualifier }

%type alias_statement        { Statement }
%type assignment_statement    { Statement }
%type case_statement        { Statement }
%type compound_statement    { Statement }
%type escape_statement        { Statement }
%type if_statement        { Statement }
%type proc_call_statement    { Statement }
%type repeat_statement        { Statement }
%type return_statement        { Statement }
%type skip_statement        { Statement }
%type statement            { Statement }

%type subsuper_decl        { struct subsuper_decl }

%type supertype_decl        { struct subtypes }
%type supertype_factor        { struct subtypes }

%type function_id        { Symbol* }
%type procedure_id        { Symbol* }

%type attribute_type        { Type }
%type defined_type        { Type }
%type parameter_type        { Type }
%type generic_type        { Type }

%type basic_type        { TypeBody }
%type select_type        { TypeBody }
%type aggregate_type        { TypeBody }
%type aggregation_type        { TypeBody }
%type array_type        { TypeBody }
%type bag_type            { TypeBody }
%type conformant_aggregation    { TypeBody }
%type list_type            { TypeBody }
%type set_type            { TypeBody }

%type set_or_bag_of_entity    { struct type_either }
%type type            { struct type_either }

%type cardinality_op        { struct upper_lower }
%type bound_spec        { struct upper_lower }

%type inverse_attr        { Variable }
%type derived_attribute        { Variable }
%type rule_formal_parameter    { Variable }

%type where_clause        { Where }
*/
/************ END OLD type definitions *************/

/* tokens not in the grammar */
%nonassoc T_INVALID T_DOCROOT T_RULE_REF T_RULE_LABEL_REF .

/* precedence */
%left T_EQ T_NEQ T_LTEQ T_GTEQ T_LT T_GT T_INST_EQ T_INST_NEQ T_IN T_LIKE .
%left T_PLUS T_MINUS T_OR T_XOR T_ANDOR .
%left T_TIMES T_RDIV T_IDIV T_MOD T_AND T_CONCAT .
%right T_EXP .
%right UNARY_OP .
%left T_LBRKT T_RBRKT T_LPAREN T_RPAREN T_ONEOF .
%nonassoc T_DOT T_BACKSLASH .

%include {

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "bstrlib.h"
#include "lexsupport.h"

Linked_List PARSEnew_schemas;

}

schema_id ::= T_SIMPLE_ID .
rule_id ::= T_SIMPLE_ID .
subtype_constraint_id ::= T_SIMPLE_ID .
parameter_id ::= T_SIMPLE_ID .
variable_id ::= T_SIMPLE_ID .
attribute_id ::= T_SIMPLE_ID .
enumeration_id ::= T_SIMPLE_ID .

type_label_id ::= T_SIMPLE_ID .
rule_label_id ::= T_SIMPLE_ID .

schema_ref ::= T_SCHEMA_REF .
function_ref ::= T_FUNCTION_REF .
procedure_ref ::= T_PROCEDURE_REF .
type_ref ::= T_TYPE_REF .
entity_ref ::= T_ENTITY_REF .

parameter_ref ::= T_PARAMETER_REF .
variable_ref ::= T_VARIABLE_REF .
constant_ref ::= T_CONSTANT_REF .
enumeration_ref ::= T_ENUMERATION_REF.

attribute_ref ::= T_SIMPLE_REF .

abstract_supertype ::= T_ABSTRACT T_SUPERTYPE T_SEMICOLON .
abstract_supertype ::= .

abstract_entity_declaration ::= T_ABSTRACT .
abstract_supertype_declaration ::= T_ABSTRACT T_SUPERTYPE .
abstract_supertype_declaration ::= T_ABSTRACT T_SUPERTYPE subtype_constraint .

    /*
     * to avoid the parser choking, we define actual_parameter_list like this
     * you'll find the brackets where actual_parameter_list is consumed
     * TODO: is this still true - tbc / rework
     */
actual_parameter_list ::= actual_parameter_list T_COMMA parameter .
actual_parameter_list ::= parameter .

add_like_op ::= T_PLUS .
add_like_op ::= T_MINUS .
add_like_op ::= T_OR .
add_like_op ::= T_XOR .

element_list ::= element_list T_COMMA element .
element_list ::= element .
aggregate_initializer ::= T_LBRKT element_list T_RBRKT .
aggregate_initializer ::= T_LBRKT T_RBRKT .

aggregate_source ::= simple_expression .
aggregate_type ::= T_AGGREGATE T_OF parameter_type .
aggregate_type ::= T_AGGREGATE T_COLON type_label T_OF parameter_type .

aggregation_types ::= array_type .
aggregation_types ::= bag_type .
aggregation_types ::= list_type .
aggregation_types ::= set_type .

algorithm_head ::= declaration_list constant_decl local_decl .

declaration_list ::= declaration_list declaration .
declaration_list ::= .

stmt_list ::= stmt_list stmt .
stmt_list ::= stmt .

qualifier_list ::= qualifier_list qualifier .
qualifier_list ::= .

alias_stmt ::= T_ALIAS variable_id T_FOR general_ref qualifier_list T_SEMICOLON stmt_list T_END_ALIAS T_SEMICOLON .

array_type ::= T_ARRAY bound_spec T_OF T_OPTIONAL T_UNIQUE instantiable_type .
array_type ::= T_ARRAY bound_spec T_OF T_OPTIONAL instantiable_type .
array_type ::= T_ARRAY bound_spec T_OF instantiable_type .

assignment_stmt ::= general_ref qualifier_list T_ASSIGN expression T_SEMICOLON .
attribute_decl ::= attribute_id .
attribute_decl ::= redeclared_attribute .
attribute_qualifier ::= T_DOT attribute_ref .

bag_type ::= T_BAG bound_spec T_OF instantiable_type .
bag_type ::= T_BAG T_OF instantiable_type .

binary_type ::= T_BINARY width_spec .
binary_type ::= T_BINARY .
boolean_type ::= T_BOOLEAN .
bound_1 ::= numeric_expression .
bound_2 ::= numeric_expression .
bound_spec ::= T_LBRKT bound_1 T_COLON bound_2 T_RBRKT .

built_in_constant ::= T_CONST_E .
built_in_constant ::= T_PI .
built_in_constant ::= T_SELF .
built_in_constant ::= T_QMARK .

built_in_function ::= T_ABS .
built_in_function ::= T_ACOS .
built_in_function ::= T_ASIN .
built_in_function ::= T_ATAN .
built_in_function ::= T_BLENGTH .
built_in_function ::= T_COS .
built_in_function ::= T_EXISTS .

built_in_function ::= T_EXP .
built_in_function ::= T_FORMAT .
built_in_function ::= T_HIBOUND .
built_in_function ::= T_HIINDEX .
built_in_function ::= T_LENGTH .
built_in_function ::= T_LOBOUND .
built_in_function ::= T_LOINDEX .

built_in_function ::= T_LOG .
built_in_function ::= T_LOG2 .
built_in_function ::= T_LOG10 .
built_in_function ::= T_NVL .
built_in_function ::= T_ODD .
built_in_function ::= T_ROLESOF .
built_in_function ::= T_SIN .
built_in_function ::= T_SIZEOF .

built_in_function ::= T_SQRT .
built_in_function ::= T_TAN .
built_in_function ::= T_TYPEOF .
built_in_function ::= T_USEDIN .
built_in_function ::= T_VALUE .
built_in_function ::= T_VALUE_IN .
built_in_function ::= T_VALUE_UNIQUE .

built_in_procedure ::= T_INSERT .
built_in_procedure ::= T_REMOVE .

case_action ::= case_label_list T_COLON stmt .

case_otherwise ::= T_OTHERWISE T_COLON stmt .
case_otherwise ::= .
    
case_label_list ::= case_label_list T_COMMA case_label .
case_label_list ::= case_label .

case_label ::= expression .

case_action_list ::= case_action_list case_action .
case_action_list ::= .

case_stmt ::= T_CASE selector T_OF case_action_list case_otherwise T_END_CASE T_SEMICOLON .

compound_stmt ::= T_BEGIN stmt_list T_END T_SEMICOLON .

concrete_types ::= aggregation_types .
concrete_types ::= simple_types .
concrete_types ::= type_ref .

constant_body ::= T_SIMPLE_ID T_COLON instantiable_type T_ASSIGN expression T_SEMICOLON .

constant_decl ::= T_CONSTANT constant_body_list T_END_CONSTANT T_SEMICOLON .
constant_decl ::= .

constant_body_list ::= constant_body_list constant_body .
constant_body_list ::= constant_body .

constant_factor ::= built_in_constant .
constant_factor ::= constant_ref .

constructed_types ::= enumeration_type .
constructed_types ::= select_type .

declaration ::= entity_decl .
declaration ::= function_decl .
declaration ::= procedure_decl .
declaration ::= subtype_constraint_decl .
declaration ::= type_decl .

derived_attr ::= attribute_decl T_COLON parameter_type T_ASSIGN expression T_SEMICOLON .

derive_clause ::= T_DERIVE derived_attr_list .
derive_clause ::= .

derived_attr_list ::= derived_attr_list derived_attr .
derived_attr_list ::= derived_attr .

domain_rule ::= rule_label_id T_COLON expression .
domain_rule ::= expression .

element ::= expression T_COLON repetition .
element ::= expression .

entity_body ::= explicit_attr_list derive_clause inverse_clause unique_clause where_clause .

explicit_attr_list ::= explicit_attr_list explicit_attr .
explicit_attr_list ::= .

entity_constructor ::= entity_ref T_LPAREN expression_list T_RPAREN .
entity_constructor ::= entity_ref T_LPAREN T_RPAREN .

expression_list ::= expression_list T_COMMA expression .
expression_list ::= expression .

entity_decl ::= entity_head entity_body T_END_ENTITY T_SEMICOLON .
entity_head ::= T_ENTITY T_SIMPLE_ID subsuper T_SEMICOLON .
enumeration_extension ::= T_BASED_ON type_ref T_WITH enumeration_items .
enumeration_extension ::= T_BASED_ON type_ref .

enumeration_items ::= T_LPAREN enumeration_id_list T_RPAREN .
enumeration_id_list ::= enumeration_id_list T_COMMA enumeration_id .
enumeration_id_list ::= enumeration_id .

enumeration_reference ::= type_ref T_DOT enumeration_ref .
enumeration_reference ::= enumeration_ref .

enumeration_type ::= T_EXTENSIBLE T_ENUMERATION T_OF enumeration_items .
enumeration_type ::= T_EXTENSIBLE T_ENUMERATION enumeration_extension .
enumeration_type ::= T_EXTENSIBLE T_ENUMERATION .
enumeration_type ::= T_ENUMERATION T_OF enumeration_items .
enumeration_type ::= T_ENUMERATION enumeration_extension .
enumeration_type ::= T_ENUMERATION .

escape_stmt ::= T_ESCAPE T_SEMICOLON .

explicit_attr ::= attribute_decl_list T_COLON T_OPTIONAL parameter_type T_SEMICOLON .
explicit_attr ::= attribute_decl_list T_COLON parameter_type T_SEMICOLON .

attribute_decl_list ::= attribute_decl_list T_COMMA attribute_decl .
attribute_decl_list ::= attribute_decl .

expression ::= simple_expression rel_op_extended simple_expression .
expression ::= simple_expression .

factor ::= simple_factor T_EXP simple_factor .
factor ::= simple_factor .

formal_parameter ::= parameter_id_list T_COLON parameter_type .

parameter_id_list ::= parameter_id_list T_COMMA parameter_id .
parameter_id_list ::= parameter_id .

function_call ::= built_in_function T_LPAREN actual_parameter_list T_RPAREN .
function_call ::= built_in_function .
function_call ::= function_ref T_LPAREN actual_parameter_list T_RPAREN .
function_call ::= function_ref .

function_decl ::= function_head algorithm_head stmt_list T_END_FUNCTION T_SEMICOLON .
function_head ::= T_FUNCTION T_SIMPLE_ID T_LPAREN formal_parameter_list T_RPAREN T_COLON parameter_type T_SEMICOLON .
function_head ::= T_FUNCTION T_SIMPLE_ID T_COLON parameter_type T_SEMICOLON .

formal_parameter_list ::= formal_parameter_list T_SEMICOLON formal_parameter .
formal_parameter_list ::= formal_parameter .

generalized_types ::= aggregate_type .
generalized_types ::= general_aggregation_types .
generalized_types ::= generic_entity_type .
generalized_types ::= generic_type .

general_aggregation_types ::= general_array_type .
general_aggregation_types ::= general_bag_type .
general_aggregation_types ::= general_list_type .
general_aggregation_types ::= general_set_type .

general_array_type ::= T_ARRAY bound_spec T_OF T_OPTIONAL T_UNIQUE parameter_type .
general_array_type ::= T_ARRAY bound_spec T_OF T_OPTIONAL parameter_type .
general_array_type ::= T_ARRAY bound_spec T_OF T_UNIQUE parameter_type .
general_array_type ::= T_ARRAY bound_spec T_OF parameter_type .
general_array_type ::= T_ARRAY T_OF T_OPTIONAL T_UNIQUE parameter_type .
general_array_type ::= T_ARRAY T_OF T_OPTIONAL parameter_type .
general_array_type ::= T_ARRAY T_OF T_UNIQUE parameter_type .
general_array_type ::= T_ARRAY T_OF parameter_type .

general_bag_type ::= T_BAG bound_spec T_OF parameter_type .
general_bag_type ::= T_BAG T_OF parameter_type .

general_list_type ::= T_LIST bound_spec T_OF T_UNIQUE parameter_type .
general_list_type ::= T_LIST bound_spec T_OF parameter_type .
general_list_type ::= T_LIST T_OF T_UNIQUE parameter_type .
general_list_type ::= T_LIST T_OF parameter_type .

general_ref ::= parameter_ref .
general_ref ::= variable_ref .

general_set_type ::= T_SET bound_spec T_OF parameter_type .
general_set_type ::= T_SET T_OF parameter_type .

generic_entity_type ::= T_GENERIC_ENTITY T_COLON type_label .
generic_entity_type ::= T_GENERIC_ENTITY .
generic_type ::= T_GENERIC T_COLON type_label .
generic_type ::= T_GENERIC .
group_qualifier ::= T_BACKSLASH entity_ref .
if_stmt ::= T_IF logical_expression T_THEN stmt_list T_ELSE stmt_list T_END_IF T_SEMICOLON .
if_stmt ::= T_IF logical_expression T_THEN stmt_list T_END_IF T_SEMICOLON .
increment ::= numeric_expression .
increment_control ::= variable_id T_ASSIGN bound_1 T_TO bound_2 T_BY increment .
increment_control ::= variable_id T_ASSIGN bound_1 T_TO bound_2 .
increment_control ::= .
index ::= numeric_expression .
index_1 ::= index .
index_2 ::= index .
index_qualifier ::= T_LBRKT index_1 T_COLON index_2 T_RBRKT .
index_qualifier ::= T_LBRKT index_1 T_RBRKT .

instantiable_type ::= concrete_types .
instantiable_type ::= entity_ref .

integer_type ::= T_INTEGER .

interface_specification ::= reference_clause .
interface_specification ::= use_clause .

interval ::= T_LCURL interval_low interval_op interval_item interval_op interval_high T_RCURL .
interval_high ::= simple_expression .
interval_item ::= simple_expression .
interval_low ::= simple_expression .
interval_op ::= T_LT .
interval_op ::= T_LTEQ .

inverse_attr ::= attribute_decl T_COLON T_SET bound_spec T_OF entity_ref T_FOR entity_ref T_DOT attribute_ref T_SEMICOLON .
inverse_attr ::= attribute_decl T_COLON T_SET T_OF entity_ref T_FOR entity_ref T_DOT attribute_ref T_SEMICOLON .
inverse_attr ::= attribute_decl T_COLON T_BAG bound_spec T_OF entity_ref T_FOR entity_ref T_DOT attribute_ref T_SEMICOLON .
inverse_attr ::= attribute_decl T_COLON T_BAG T_OF entity_ref T_FOR entity_ref T_DOT attribute_ref T_SEMICOLON .
inverse_attr ::= attribute_decl T_COLON entity_ref T_FOR entity_ref T_DOT attribute_ref T_SEMICOLON .
inverse_attr ::= attribute_decl T_COLON T_SET bound_spec T_OF entity_ref T_FOR attribute_ref T_SEMICOLON .
inverse_attr ::= attribute_decl T_COLON T_SET T_OF entity_ref T_FOR attribute_ref T_SEMICOLON .
inverse_attr ::= attribute_decl T_COLON T_BAG bound_spec T_OF entity_ref T_FOR attribute_ref T_SEMICOLON .
inverse_attr ::= attribute_decl T_COLON T_BAG T_OF entity_ref T_FOR attribute_ref T_SEMICOLON .
inverse_attr ::= attribute_decl T_COLON entity_ref T_FOR attribute_ref T_SEMICOLON .

inverse_clause ::= T_INVERSE inverse_attr_list .
inverse_clause ::= .

inverse_attr_list ::= inverse_attr_list inverse_attr .
inverse_attr_list ::= inverse_attr .

list_type ::= T_LIST bound_spec T_OF T_UNIQUE instantiable_type .
list_type ::= T_LIST bound_spec T_OF instantiable_type .
list_type ::= T_LIST T_OF T_UNIQUE instantiable_type .
list_type ::= T_LIST T_OF instantiable_type .

literal ::= T_BINARY_LITERAL .
literal ::= logical_literal .
literal ::= real_literal .
literal ::= string_literal .

real_literal ::= T_INTEGER_LITERAL .
real_literal ::= T_REAL_LITERAL .

local_decl ::= T_LOCAL local_variable_list T_END_LOCAL T_SEMICOLON .
local_decl ::= .

local_variable_list ::= local_variable_list local_variable .
local_variable_list ::= local_variable .

local_variable ::= variable_id_list T_COLON parameter_type T_ASSIGN expression T_SEMICOLON .
local_variable ::= variable_id_list T_COLON parameter_type T_SEMICOLON .

variable_id_list ::= variable_id_list T_COMMA variable_id .
variable_id_list ::= variable_id .

logical_expression ::= expression .

logical_literal ::= T_FALSE .
logical_literal ::= T_TRUE .
logical_literal ::= T_UNKNOWN .
logical_type ::= T_LOGICAL .

multiplication_like_op ::= T_TIMES .
multiplication_like_op ::= T_RDIV .
multiplication_like_op ::= T_IDIV .
multiplication_like_op ::= T_MOD .
multiplication_like_op ::= T_AND .
multiplication_like_op ::= T_CONCAT .

named_types ::= entity_ref .
named_types ::= type_ref .

named_type_or_rename ::= named_types T_AS T_SIMPLE_ID .
named_type_or_rename ::= named_types .

null_stmt ::= T_SEMICOLON .
number_type ::= T_NUMBER .
numeric_expression ::= simple_expression .

one_of ::= T_ONEOF T_LPAREN supertype_expression_list T_RPAREN .
supertype_expression_list ::= supertype_expression_list T_COMMA supertype_expression .
supertype_expression_list ::= supertype_expression .

parameter ::= expression .

parameter_type ::= generalized_types .
parameter_type ::= named_types .
parameter_type ::= simple_types .

population ::= entity_ref .
precision_spec ::= numeric_expression .
primary ::= literal .
    /* TODO: if qualifier_list is empty then the returned expression can be simplified? */
primary ::= qualifiable_factor qualifier_list .

procedure_call_stmt ::= built_in_procedure T_LPAREN actual_parameter_list T_RPAREN T_SEMICOLON .
procedure_call_stmt ::= built_in_procedure T_SEMICOLON .
procedure_call_stmt ::= procedure_ref T_LPAREN actual_parameter_list T_RPAREN T_SEMICOLON .
procedure_call_stmt ::= procedure_ref T_SEMICOLON .

procedure_decl ::= procedure_head algorithm_head stmt_list T_END_PROCEDURE T_SEMICOLON .
procedure_decl ::= procedure_head algorithm_head T_END_PROCEDURE T_SEMICOLON .

procedure_head ::= T_PROCEDURE T_SIMPLE_ID T_LPAREN varopt_formal_parameter_list T_RPAREN T_SEMICOLON .
procedure_head ::= T_PROCEDURE T_SIMPLE_ID T_SEMICOLON .

varopt_formal_parameter_list ::= varopt_formal_parameter_list T_SEMICOLON T_VAR formal_parameter .
varopt_formal_parameter_list ::= varopt_formal_parameter_list T_SEMICOLON formal_parameter .
varopt_formal_parameter_list ::= T_VAR formal_parameter .
varopt_formal_parameter_list ::= formal_parameter .

qualifiable_factor ::= attribute_ref .
qualifiable_factor ::= constant_factor .
qualifiable_factor ::= function_call .
qualifiable_factor ::= general_ref .
qualifiable_factor ::= population .

qualified_attribute ::= T_SELF group_qualifier attribute_qualifier .

qualifier ::= attribute_qualifier .
qualifier ::= group_qualifier .
qualifier ::= index_qualifier .

query_expression ::= T_QUERY T_LPAREN variable_id T_ALL_IN aggregate_source T_PIPE logical_expression T_RPAREN .
real_type ::= T_REAL T_LPAREN precision_spec T_RPAREN .
real_type ::= T_REAL .
redeclared_attribute ::= qualified_attribute T_RENAMED attribute_id .
redeclared_attribute ::= qualified_attribute .

referenced_attribute ::= attribute_ref .
referenced_attribute ::= qualified_attribute .

reference_clause ::= T_REFERENCE T_FROM schema_ref T_LPAREN resource_or_rename_list T_RPAREN T_SEMICOLON .
reference_clause ::= T_REFERENCE T_FROM schema_ref T_SEMICOLON .
resource_or_rename_list ::= resource_or_rename_list T_COMMA resource_or_rename .
resource_or_rename_list ::= resource_or_rename .

rel_op ::= T_LT .
rel_op ::= T_GT .
rel_op ::= T_LTEQ .
rel_op ::= T_GTEQ .
rel_op ::= T_NEQ .
rel_op ::= T_EQ .
rel_op ::= T_INST_NEQ .
rel_op ::= T_INST_EQ .

rel_op_extended ::= rel_op .
rel_op_extended ::= T_IN .
rel_op_extended ::= T_LIKE .

rename_id ::= T_SIMPLE_ID .

repeat_control ::= increment_control while_control until_control .
repeat_stmt ::= T_REPEAT repeat_control T_SEMICOLON stmt_list T_END_REPEAT T_SEMICOLON .

repetition ::= numeric_expression .

resource_or_rename ::= resource_ref T_AS rename_id .
resource_or_rename ::= resource_ref .

resource_ref ::= constant_ref .
resource_ref ::= entity_ref .
resource_ref ::= function_ref .
resource_ref ::= procedure_ref .
resource_ref ::= type_ref .

return_stmt ::= T_RETURN T_LPAREN expression T_RPAREN T_SEMICOLON .
return_stmt ::= T_RETURN T_SEMICOLON .

rule_decl ::= rule_head algorithm_head stmt_list where_clause T_END_RULE  T_SEMICOLON .
rule_decl ::= rule_head algorithm_head where_clause T_END_RULE T_SEMICOLON .

rule_head ::= T_RULE rule_id T_FOR T_LPAREN entity_ref_list T_RPAREN T_SEMICOLON .
entity_ref_list ::= entity_ref_list T_COMMA entity_ref .
entity_ref_list ::= entity_ref .

schema_body ::= interface_specification_list constant_decl declaration_or_rule_decl_list .

interface_specification_list ::= interface_specification_list interface_specification .
interface_specification_list ::= .

declaration_or_rule_decl ::= declaration .
declaration_or_rule_decl ::= rule_decl .

declaration_or_rule_decl_list ::= declaration_or_rule_decl_list declaration_or_rule_decl .
declaration_or_rule_decl_list ::= .

schema_decl ::= T_SCHEMA schema_id schema_version_id T_SEMICOLON schema_body T_END_SCHEMA T_SEMICOLON .
schema_decl ::= T_SCHEMA schema_id T_SEMICOLON schema_body T_END_SCHEMA T_SEMICOLON .

schema_version_id ::= string_literal .
selector ::= expression .

select_extension ::= T_BASED_ON type_ref T_WITH select_list .
select_extension ::= T_BASED_ON type_ref .

select_list ::= T_LPAREN named_types_list T_RPAREN .
named_types_list ::= named_types_list T_COMMA named_types .
named_types_list ::= named_types .

select_type ::= T_EXTENSIBLE T_GENERIC_ENTITY T_SELECT select_list .
select_type ::= T_EXTENSIBLE T_SELECT select_list .
select_type ::= T_SELECT select_list .
select_type ::= T_EXTENSIBLE T_GENERIC_ENTITY T_SELECT select_extension .
select_type ::= T_EXTENSIBLE T_SELECT select_extension .
select_type ::= T_SELECT select_extension .
select_type ::= T_SELECT .

set_type ::= T_SET bound_spec T_OF instantiable_type .
set_type ::= T_SET T_OF instantiable_type .

simple_expression ::= term_list .
term_list ::= term_list add_like_op term .
term_list ::= term .

simple_factor ::= aggregate_initializer .
simple_factor ::= entity_constructor .
simple_factor ::= enumeration_reference .
simple_factor ::= interval .
simple_factor ::= query_expression .

simple_factor ::= T_LPAREN expression T_RPAREN .
simple_factor ::= primary .

    /* UNARY_OP */
simple_factor ::= T_PLUS T_LPAREN expression T_RPAREN . [UNARY_OP]
simple_factor ::= T_PLUS primary . [UNARY_OP]
simple_factor ::= T_MINUS T_LPAREN expression T_RPAREN . [UNARY_OP]
simple_factor ::= T_MINUS primary . [UNARY_OP]
simple_factor ::= T_NOT T_LPAREN expression T_RPAREN . [UNARY_OP]
simple_factor ::= T_NOT primary . [UNARY_OP]

simple_types ::= binary_type .
simple_types ::= boolean_type .
simple_types ::= integer_type .
simple_types ::= logical_type .
simple_types ::= number_type .
simple_types ::= real_type .
simple_types ::= string_type .

skip_stmt ::= T_SKIP T_SEMICOLON .

stmt ::= alias_stmt .
stmt ::= assignment_stmt .
stmt ::= case_stmt .
stmt ::= compound_stmt .
stmt ::= escape_stmt .
stmt ::= if_stmt .
stmt ::= null_stmt .

stmt ::= procedure_call_stmt .
stmt ::= repeat_stmt .
stmt ::= return_stmt .
stmt ::= skip_stmt .

string_literal ::= T_STRING_LITERAL .
string_literal ::= T_ENCODED_STRING_LITERAL .

string_type ::= T_STRING width_spec .
string_type ::= T_STRING .

subsuper ::= supertype_constraint subtype_declaration .
subsuper ::= supertype_constraint .
subsuper ::= subtype_declaration .
subsuper ::= .

subtype_constraint ::= T_OF T_LPAREN supertype_expression T_RPAREN .

subtype_constraint_body ::= abstract_supertype total_over supertype_expression T_SEMICOLON .
subtype_constraint_body ::= abstract_supertype total_over .

subtype_constraint_decl ::= subtype_constraint_head subtype_constraint_body T_END_SUBTYPE_CONSTRAINT T_SEMICOLON .
subtype_constraint_head ::= T_SUBTYPE_CONSTRAINT subtype_constraint_id T_FOR entity_ref T_SEMICOLON .
subtype_declaration ::= T_SUBTYPE T_OF T_LPAREN entity_ref_list T_RPAREN .

supertype_constraint ::= abstract_entity_declaration .
supertype_constraint ::= abstract_supertype_declaration .
supertype_constraint ::= supertype_rule .

supertype_expression ::= supertype_factor_list .
supertype_factor_list ::= supertype_factor_list T_ANDOR supertype_factor .
supertype_factor_list ::= supertype_factor .

supertype_factor ::= supertype_term_list .
supertype_term_list ::= supertype_term_list T_AND supertype_term .
supertype_term_list ::= supertype_term .

supertype_rule ::= T_SUPERTYPE subtype_constraint .

supertype_term ::= entity_ref .
supertype_term ::= one_of .
supertype_term ::= T_LPAREN supertype_expression T_RPAREN .

term ::= factor_list .

factor_list ::= factor_list multiplication_like_op factor .
factor_list ::= factor .

total_over ::= T_TOTAL_OVER T_LPAREN entity_ref_list T_RPAREN T_SEMICOLON .
total_over ::= .

type_head ::= T_TYPE T_SIMPLE_ID T_EQ .
type_decl ::= type_head underlying_type T_SEMICOLON where_clause T_END_TYPE T_SEMICOLON .
type_label ::= type_label_id .

underlying_type ::= concrete_types .
underlying_type ::= constructed_types .

unique_clause ::= T_UNIQUE unique_rule_list .
unique_clause ::= .
unique_rule_list ::= unique_rule_list unique_rule T_SEMICOLON .
unique_rule_list ::= unique_rule T_SEMICOLON .

unique_rule ::= rule_label_id T_COLON referenced_attribute_list .
unique_rule ::= referenced_attribute_list .
referenced_attribute_list ::= referenced_attribute_list T_COMMA referenced_attribute .
referenced_attribute_list ::= referenced_attribute .

until_control ::= T_UNTIL logical_expression .
until_control ::= .

use_clause ::= T_USE T_FROM schema_ref T_LPAREN named_type_or_rename_list T_RPAREN T_SEMICOLON .
use_clause ::= T_USE T_FROM schema_ref T_SEMICOLON .
named_type_or_rename_list ::= named_type_or_rename_list T_COMMA named_type_or_rename .
named_type_or_rename_list ::= named_type_or_rename .

where_clause ::= T_WHERE domain_rule_list .
where_clause ::= .
domain_rule_list ::= domain_rule T_SEMICOLON .
domain_rule_list ::= domain_rule_list domain_rule T_SEMICOLON .

while_control ::= T_WHILE logical_expression .
while_control ::= .
width ::= numeric_expression .
width_spec ::= T_LPAREN width T_RPAREN T_FIXED .
width_spec ::= T_LPAREN width T_RPAREN .

express_file ::= schema_decl_list .

schema_decl_list ::= schema_decl_list schema_decl .
schema_decl_list ::= schema_decl .

%syntax_error { 
    yyerror("syntax error on token type: '%s', val: '%s'", pState->lineno, yyTokenName[yymajor], yyminor->cstr);
}

