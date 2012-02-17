/*
** Fed-x parser output module for generating C++  class definitions
** December  5, 1989
** release 2 17-Feb-1992
** release 3 March 1993
** release 4 December 1993
** K. C. Morris
**
** Development of Fed-x was funded by the United States Government,
** and is not subject to copyright.

*******************************************************************
The conventions used in this binding follow the proposed specification
for the STEP Standard Data Access Interface as defined in document
N350 ( August 31, 1993 ) of ISO 10303 TC184/SC4/WG7.
*******************************************************************/

/******************************************************************
***  The functions in this file generate the C++ code for ENTITY **
***  classes, TYPEs, and TypeDescriptors.                       ***
 **                             **/


/* this is used to add new dictionary calls */
/* #define NEWDICT */

#include <stdlib.h>
#include "classes.h"

int isAggregateType( const Type t );
int isAggregate( Variable a );
Variable VARis_type_shifter( Variable a );
const char * ENTITYget_CORBAname( Entity ent );
const char * GetTypeDescriptorName( Type t );

char * FundamentalType( const Type t, int report_reftypes );

int multiple_inheritance = 1;
int print_logging = 0;
int corba_binding = 0;
int old_accessors = 0;

/* several classes use attr_count for naming attr dictionary entry
   variables.  All but the last function generating code for a particular
   entity increment a copy of it for naming each attr in the entity.
   Here are the functions:
   ENTITYhead_print (Entity entity, FILE* file,Schema schema)
   LIBdescribe_entity (Entity entity, FILE* file, Schema schema)
   LIBcopy_constructor (Entity ent, FILE* file)
   LIBstructor_print (Entity entity, FILE* file, Schema schema)
   LIBstructor_print_w_args (Entity entity, FILE* file, Schema schema)
   ENTITYincode_print (Entity entity, FILE* file,Schema schema)
   DAS
 */
static int attr_count;  /* number each attr to avoid inter-entity clashes */
static int type_count;  /* number each temporary type for same reason above */

extern int any_duplicates_in_select( const Linked_List list );
extern int unique_types( const Linked_List list );
extern char * non_unique_types_string( const Type type );
static void printEnumCreateHdr( FILE *, const Type );
static void printEnumCreateBody( FILE *, const Type );
static void printEnumAggrCrHdr( FILE *, const Type );
static void printEnumAggrCrBody( FILE *, const Type );
void printAccessHookFriend( FILE *, const char * );
void printAccessHookHdr( FILE *, const char * );
int TYPEget_RefTypeVarNm( const Type t, char * buf, Schema schema );
void TypeBody_Description( TypeBody body, char * buf );

/*
Turn the string into a new string that will be printed the same as the
original string. That is, turn backslash into a quoted backslash and
turn \n into "\n" (i.e. 2 chars).
*/

char * format_for_stringout( char * orig_buf, char * return_buf ) {
    char * optr  = orig_buf;
    char * rptr  = return_buf;
    while( *optr ) {
        if( *optr == '\n' ) {
            *rptr = '\\';
            rptr++;
            *rptr = 'n';
        } else if( *optr == '\\' ) {
            *rptr = '\\';
            rptr++;
            *rptr = '\\';
        } else {
            *rptr = *optr;
        }
        rptr++;
        optr++;
    }
    *rptr = '\0';
    return return_buf;
}

void
USEREFout( Schema schema, Dictionary refdict, Linked_List reflist, char * type, FILE * file ) {
    Dictionary dict;
    DictionaryEntry de;
    struct Rename * r;
    Linked_List list;
    char td_name[BUFSIZ];
    char sch_name[BUFSIZ];

    strncpy( sch_name, PrettyTmpName( SCHEMAget_name( schema ) ), BUFSIZ );

    LISTdo( reflist, s, Schema ) {
        fprintf( file, "\t// %s FROM %s; (all objects)\n", type, s->symbol.name );
        fprintf( file, "\tis = new Interface_spec(\"%s\",\"%s\");\n", sch_name, PrettyTmpName( s->symbol.name ) );
        fprintf( file, "\tis->all_objects_(1);\n" );
        if( !strcmp( type, "USE" ) ) {
            fprintf( file, "\t%s%s->use_interface_list_()->Append(is);\n", SCHEMA_PREFIX, SCHEMAget_name( schema ) );
        } else {
            fprintf( file, "\t%s%s->ref_interface_list_()->Append(is);\n", SCHEMA_PREFIX, SCHEMAget_name( schema ) );
        }
    }
    LISTod

    if( !refdict ) {
        return;
    }
    dict = DICTcreate( 10 );

    /* sort each list by schema */

    /* step 1: for each entry, store it in a schema-specific list */
    DICTdo_init( refdict, &de );
    while( 0 != ( r = ( struct Rename * )DICTdo( &de ) ) ) {
        Linked_List list;

        list = ( Linked_List )DICTlookup( dict, r->schema->symbol.name );
        if( !list ) {
            list = LISTcreate();
            DICTdefine( dict, r->schema->symbol.name, list,
                        ( Symbol * )0, OBJ_UNKNOWN );
        }
        LISTadd( list, r );
    }

    /* step 2: for each list, print out the renames */
    DICTdo_init( dict, &de );
    while( 0 != ( list = ( Linked_List )DICTdo( &de ) ) ) {
        bool first_time = true;
        LISTdo( list, r, struct Rename * )

        /*
           Interface_spec_ptr is;
           Used_item_ptr ui;
           is = new Interface_spec(const char * cur_sch_id);
           schemadescriptor->use_interface_list_()->Append(is);
           ui = new Used_item(TypeDescriptor *ld, const char *oi, const char *ni) ;
           is->_explicit_items->Append(ui);
        */

        /* note: SCHEMAget_name(r->schema) equals r->schema->symbol.name) */
        if( first_time ) {
            fprintf( file, "\t// %s FROM %s (selected objects)\n", type, r->schema->symbol.name );
            fprintf( file, "\tis = new Interface_spec(\"%s\",\"%s\");\n", sch_name, PrettyTmpName( r->schema->symbol.name ) );
            if( !strcmp( type, "USE" ) ) {
                fprintf( file, "\t%s%s->use_interface_list_()->Append(is);\n", SCHEMA_PREFIX, SCHEMAget_name( schema ) );
            } else {
                fprintf( file, "\t%s%s->ref_interface_list_()->Append(is);\n", SCHEMA_PREFIX, SCHEMAget_name( schema ) );
            }
        }

        if( first_time ) {
            first_time = false;
        }
        if( r->type == OBJ_TYPE ) {
            sprintf( td_name, "%s", TYPEtd_name( ( Type )r->object ) );
        } else if( r->type == OBJ_FUNCTION ) {
            sprintf( td_name, "/* Function not implemented */ 0" );
        } else if( r->type == OBJ_PROCEDURE ) {
            sprintf( td_name, "/* Procedure not implemented */ 0" );
        } else if( r->type == OBJ_RULE ) {
            sprintf( td_name, "/* Rule not implemented */ 0" );
        } else if( r->type == OBJ_ENTITY ) {
            sprintf( td_name, "%s%s%s",
                     SCOPEget_name( ( ( Entity )r->object )->superscope ),
                     ENT_PREFIX, ENTITYget_name( ( Entity )r->object ) );
        } else {
            sprintf( td_name, "/* %c from OBJ_? in expbasic.h not implemented */ 0", r->type );
        }
        if( r->old != r->nnew ) {
            fprintf( file, "\t// object %s AS %s\n", r->old->name,
                     r->nnew->name );
            if( !strcmp( type, "USE" ) ) {
                fprintf( file, "\tui = new Used_item(\"%s\", %s, \"%s\", \"%s\");\n", r->schema->symbol.name, td_name, r->old->name, r->nnew->name );
                fprintf( file, "\tis->explicit_items_()->Append(ui);\n" );
                fprintf( file, "\t%s%s->interface_().explicit_items_()->Append(ui);\n", SCHEMA_PREFIX, SCHEMAget_name( schema ) );
            } else {
                fprintf( file, "\tri = new Referenced_item(\"%s\", %s, \"%s\", \"%s\");\n", r->schema->symbol.name, td_name, r->old->name, r->nnew->name );
                fprintf( file, "\tis->explicit_items_()->Append(ri);\n" );
                fprintf( file, "\t%s%s->interface_().explicit_items_()->Append(ri);\n", SCHEMA_PREFIX, SCHEMAget_name( schema ) );
            }
        } else {
            fprintf( file, "\t// object %s\n", r->old->name );
            if( !strcmp( type, "USE" ) ) {
                fprintf( file, "\tui = new Used_item(\"%s\", %s, \"\", \"%s\");\n", r->schema->symbol.name, td_name, r->nnew->name );
                fprintf( file, "\tis->explicit_items_()->Append(ui);\n" );
                fprintf( file, "\t%s%s->interface_().explicit_items_()->Append(ui);\n", SCHEMA_PREFIX, SCHEMAget_name( schema ) );
            } else {
                fprintf( file, "\tri = new Referenced_item(\"%s\", %s, \"\", \"%s\");\n", r->schema->symbol.name, td_name, r->nnew->name );
                fprintf( file, "\tis->explicit_items_()->Append(ri);\n" );
                fprintf( file, "\t%s%s->interface_().explicit_items_()->Append(ri);\n", SCHEMA_PREFIX, SCHEMAget_name( schema ) );
            }
        }
        LISTod
    }
    HASHdestroy( dict );
}

const char *
IdlEntityTypeName( Type t ) {
}


int Handle_FedPlus_Args( int i, char * arg ) {
    if( ( ( char )i == 's' ) || ( ( char )i == 'S' ) ) {
        multiple_inheritance = 0;
    }
    if( ( ( char )i == 'a' ) || ( ( char )i == 'A' ) ) {
        old_accessors = 1;
    }
    if( ( char )i == 'L' ) {
        print_logging = 1;
    }
    if( ( ( char )i == 'c' ) || ( ( char )i == 'C' ) ) {
        corba_binding = 1;
    }
    return 0;
}


bool is_python_keyword(char * word) {
    bool python_keyword = false;
    if (strcmp(word,"class")==0) python_keyword = true;
    return python_keyword;
}

/******************************************************************
 ** Procedure:  generate_attribute_name
 ** Parameters:  Variable a, an Express attribute; char *out, the C++ name
 ** Description:  converts an Express name into the corresponding C++ name
 **       see relation to generate_dict_attr_name() DAS
 ** Side Effects:
 ** Status:  complete 8/5/93
 ******************************************************************/
char *
generate_attribute_name( Variable a, char * out ) {
    char * temp, *p, *q;
    int j;
    temp = EXPRto_string( VARget_name( a ) );
    p = temp;
    if( ! strncmp( StrToLower( p ), "self\\", 5 ) ) {
        p = p + 5;
    }
    /*  copy p to out  */
    /* DAR - fixed so that '\n's removed */
    for( j = 0, q = out; j < BUFSIZ; p++ ) {
        /* copy p to out, 1 char at time.  Skip \n's and spaces, convert */
        /*  '.' to '_', and convert to lowercase. */
        if( ( *p != '\n' ) && ( *p != ' ' ) ) {
            if( *p == '.' ) {
                *q = '_';
            } else {
                *q = tolower( *p );
            }
            j++;
            q++;
        }
    }
    free(temp);
    // python generator : we should prevend an attr name to be a python reserved keyword
    if (is_python_keyword(out)) strcat(out,"_");  
    return out;
}

char *
generate_attribute_func_name( Variable a, char * out ) {
    generate_attribute_name( a, out );
    strncpy( out, CheckWord( StrToLower( out ) ), BUFSIZ );
    if( old_accessors ) {
        out[0] = toupper( out[0] );
    } else {
        out[strlen( out )] = '_';
    }
    return out;
}

/******************************************************************
 ** Procedure:  generate_dict_attr_name
 ** Parameters:  Variable a, an Express attribute; char *out, the C++ name
 ** Description:  converts an Express name into the corresponding SCL
 **       dictionary name.  The difference between this and the
 **           generate_attribute_name() function is that for derived
 **       attributes the name will have the form <parent>.<attr_name>
 **       where <parent> is the name of the parent containing the
 **       attribute being derived and <attr_name> is the name of the
 **       derived attribute. Both <parent> and <attr_name> may
 **       contain underscores but <parent> and <attr_name> will be
 **       separated by a period.  generate_attribute_name() generates
 **       the same name except <parent> and <attr_name> will be
 **       separated by an underscore since it is illegal to have a
 **       period in a variable name.  This function is used for the
 **       dictionary name (a string) and generate_attribute_name()
 **       will be used for variable and access function names.
 ** Side Effects:
 ** Status:  complete 8/5/93
 ******************************************************************/
char *
generate_dict_attr_name( Variable a, char * out ) {
    char * temp, *p, *q;
    int j;

    temp = EXPRto_string( VARget_name( a ) );
    p = temp;
    if( ! strncmp( StrToLower( p ), "self\\", 5 ) ) {
        p = p + 5;
    }
    /*  copy p to out  */
    strncpy( out, StrToLower( p ), BUFSIZ );
    /* DAR - fixed so that '\n's removed */
    for( j = 0, q = out; j < BUFSIZ; p++ ) {
        /* copy p to out, 1 char at time.  Skip \n's, and convert to lc. */
        if( *p != '\n' ) {
            *q = tolower( *p );
            j++;
            q++;
        }
    }

    free( temp );
    return out;
}

/******************************************************************
**      Entity Generation                */

/******************************************************************
 ** Procedure:  ENTITYhead_print
 ** Parameters:  const Entity entity
 **   FILE* file  --  file being written to
 ** Returns:
 ** Description:  prints the beginning of the entity class definition for the
 **               c++ code and the declaration of extern attr descriptors for
 **       the registry.  In the .h file
 ** Side Effects:  generates c++ code
 ** Status:  good 1/15/91
 **          added registry things 12-Apr-1993
 ******************************************************************/

void
ENTITYhead_print( Entity entity, FILE * file, Schema schema ) {
    char entnm [BUFSIZ];
    char attrnm [BUFSIZ];
    Linked_List list;
    int attr_count_tmp = attr_count;
    Entity super = 0;

    strncpy( entnm, ENTITYget_classname( entity ), BUFSIZ );

    /* DAS print all the attr descriptors and inverse attr descriptors for an
       entity as extern defs in the .h file. */
    LISTdo( ENTITYget_attributes( entity ), v, Variable )
    generate_attribute_name( v, attrnm );
    fprintf( file, "extern %s *%s%d%s%s;\n",
             ( VARget_inverse( v ) ? "Inverse_attribute" : ( VARis_derived( v ) ? "Derived_attribute" : "AttrDescriptor" ) ),
             ATTR_PREFIX, attr_count_tmp++,
             ( VARis_derived( v ) ? "D" : ( VARis_type_shifter( v ) ? "R" : ( VARget_inverse( v ) ? "I" : "" ) ) ),
             attrnm );

    /* **** testing the functions **** */
    /*
        if( !(VARis_derived(v) &&
          VARget_initializer(v) &&
          VARis_type_shifter(v) &&
          VARis_overrider(entity, v)) )
          fprintf(file,"// %s Attr is not derived, a type shifter, overrider, no initializer.\n",attrnm);

        if(VARis_derived (v))
          fprintf(file,"// %s Attr is derived\n",attrnm);
        if (VARget_initializer (v))
          fprintf(file,"// %s Attr has an initializer\n",attrnm);
        if(VARis_type_shifter (v))
          fprintf(file,"// %s Attr is a type shifter\n",attrnm);
        if(VARis_overrider (entity, v))
          fprintf(file,"// %s Attr is an overrider\n",attrnm);
    */
    /* ****** */

    LISTod

    fprintf( file, "\nclass %s  :  ", entnm );

    /* inherit from either supertype entity class or root class of
       all - i.e. SCLP23(Application_instance) */

    if( multiple_inheritance ) {
        list = ENTITYget_supertypes( entity );
        if( ! LISTempty( list ) ) {
            super = ( Entity )LISTpeek_first( list );
        }
    } else { /* the old way */
        super = ENTITYput_superclass( entity );
    }

    if( super ) {
        fprintf( file, "  public %s  {\n ", ENTITYget_classname( super ) );
    } else {
        fprintf( file, "  public SCLP23(Application_instance) {\n" );
    }

#if 0
    /* this code is old non-working multiple inheritance code */

    list = ENTITYget_supertypes( entity );
    if( ! LISTempty( list ) ) {
        LISTdo( list, e, Entity )
        /*  if there\'s no super class yet,
            or the super class doesn\'t have any attributes
            */
        if( ( ! super ) || ( ! ENTITYhas_explicit_attributes( super ) ) ) {
            super = e;
            ++ super_cnt;
        }  else {
            printf( "WARNING:  multiple inheritance not implemented.\n" );
            printf( "\tin ENTITY %s\n\tSUPERTYPE %s IGNORED.\n\n",
                    ENTITYget_name( entity ), ENTITYget_name( e ) );
        }
        LISTod;
        fprintf( file, "  public %s  {\n ", ENTITYget_classname( super ) );

        /*  for multiple inheritance
            LISTdo (list, e, Entity)
                sprintf (buf, "  public %s, ", ENTITYget_classname (e));
                move (buf);
            LISTod;
            sprintf (buf - 2, " {\n");
            move (buf);
            fprintf(file,buffer);
        */
    }  else {   /*  if entity has no supertypes, it's at top of hierarchy  */
        fprintf( file, "  public SCLP23(Application_instance) {\n" );
    }
#endif

}

/******************************************************************
 ** Procedure:  DataMemberPrint
 ** Parameters:  const Entity entity  --  entity being processed
 **   FILE* file  --  file being written to
 ** Returns:
 ** Description:  prints out the data members for an entity's c++ class
 **               definition
 ** Side Effects:  generates c++ code
 ** Status:  ok 1/15/91
 ******************************************************************/

void
DataMemberPrint( Entity entity, FILE * file, Schema schema ) {
}

/******************************************************************
 ** Procedure:  MemberFunctionSign
 ** Parameters:  Entity *entity --  entity being processed
 **     FILE* file  --  file being written to
 ** Returns:
 ** Description:  prints the signature for member functions
                  of an entity's class definition
 **       DAS prints the end of the entity class def and the creation
 **       function that the EntityTypeDescriptor uses.
 ** Side Effects:  prints c++ code to a file
 ** Status:  ok 1/1/5/91
 **  updated 17-Feb-1992 to print only the signature
             and not the function definitions
 ******************************************************************/

void
MemberFunctionSign( Entity entity, FILE * file ) {

}

/******************************************************************
 ** Procedure:    LIBdescribe_entity (entity, file, schema)
 ** Parameters:  Entity entity --  entity being processed
 **     FILE* file  --  file being written to
 **     Schema schema -- schema being processed
 ** Returns:
 ** Description:  declares the global pointer to the EntityDescriptor
                  representing a particular entity
 **       DAS also prints the attr descs and inverse attr descs
 **       This function creates the storage space for the externs defs
 **       that were defined in the .h file. These global vars go in
 **       the .cc file.
 ** Side Effects:  prints c++ code to a file
 ** Status:  ok 12-Apr-1993
 ******************************************************************/
char *
GetAttrTypeName(Type t) {
    char * attr_type;
    if (TYPEis_string(t))
        {
            attr_type = "STRING";
        }
    else if (TYPEis_logical(t))
        {
                attr_type = "LOGCIAL";
        }
    else if (TYPEis_boolean(t))
        {
                attr_type = "BOOLEAN";
        }
    else if (TYPEis_real(t))
        {
                attr_type = "REAL";
        }
    else if (TYPEis_integer(t))
        {
                attr_type = "INTEGER";
        }
    else
        {
            attr_type = TYPEget_name(t);
        }
    return attr_type;
}

/*
*
* A recursive function to export aggregate to python
*
*/
void
process_aggregate (FILE *file, Type t) {
    Expression lower = AGGR_TYPEget_lower_limit(t);
    char *lower_str = EXPRto_string(lower);
    Expression upper = AGGR_TYPEget_upper_limit(t);
    char *upper_str = NULL;
    if (upper == LITERAL_INFINITY) {
        upper_str = "None";
    }
    else {
        upper_str = EXPRto_string(upper);
    }
    switch(TYPEget_body( t )->type) {
          case array_:
            fprintf(file,"ARRAY");
            break;
          case bag_:
            fprintf(file,"BAG");
            break;
          case set_:
            fprintf(file,"SET");
            break;
          case list_:
            fprintf(file,"LIST");
            break;
          default:
            break;
          }
          fprintf(file,"(%s,%s,",lower_str,upper_str);
          //write base type
          Type base_type = TYPEget_base_type(t);
          if (TYPEis_aggregate(base_type)) {
              process_aggregate(file,base_type);
              fprintf(file,")"); //close parenthesis
          }
          else {
              char * array_base_type = GetAttrTypeName(TYPEget_base_type(t));
              fprintf(file,"%s)",array_base_type);
          }
    
}

void
LIBdescribe_entity( Entity entity, FILE * file, Schema schema ) {
    int attr_count_tmp = attr_count;//, attr_count_tmp2 = 0;
    char attrnm [BUFSIZ], parent_attrnm[BUFSIZ];
    char * attr_type;
    bool generate_constructor = true; //by default, generates a python constructor
    bool inheritance = false;
    Type t;
    /* class name
     need to use new-style classes for properties to work correctly
    so class must inherit from object */
    if (is_python_keyword(ENTITYget_name(entity))) {fprintf(file,"class %s_(",ENTITYget_name(entity));}
    else {fprintf(file,"class %s(",ENTITYget_name(entity));}
    
    /*
    * Look for inheritance and super classes
    */
    Linked_List list;
    list = ENTITYget_supertypes( entity );
    int num_parent = 0;
    if( ! LISTempty( list ) ) {
        inheritance = true;
        LISTdo( list, e, Entity )
        /*  if there\'s no super class yet,
            or the super class doesn\'t have any attributes
        */
        if (num_parent > 0) fprintf(file,","); //separator for parent classes names
        if (is_python_keyword(ENTITYget_name(e))) {fprintf(file,"%s_",ENTITYget_name(e));}
        else {fprintf(file,"%s",ENTITYget_name(e));}
    //fprintf( file, "\t/*  parent: %s  */\n", ENTITYget_name( e ) );
        num_parent++;
        LISTod;
    }
    else {
        //inherit from BaeEntityClass by default, in order to enable decorators
        // as well as advanced __repr__ feature
        fprintf(file,"BaseEntityClass");
    }
    fprintf(file,"):\n");
    /*
    * Write docstrings in a Sphinx compliant manner
    */
    fprintf(file,"\t'''Entity %s definition.\n",ENTITYget_name(entity));
    LISTdo(ENTITYget_attributes( entity ), v, Variable)
    generate_attribute_name( v, attrnm );
    t = VARget_type( v );
    fprintf(file,"\n\t:param %s\n",attrnm);
    fprintf(file,"\t:type %s:%s\n",attrnm, GetAttrTypeName(t));
    attr_count_tmp++;
    LISTod
    fprintf(file,"\t'''\n");
    /*
    * Before writing constructor, check if this entity has any attribute
    * other wise just a 'pass' statement is enough
    */
    attr_count_tmp = 0;//attr_count;
    int num_derived_inverse_attr = 0;
    LISTdo(ENTITYget_attributes( entity ), v, Variable)
    if (VARis_derived(v) || VARget_inverse(v)) {
        num_derived_inverse_attr++;
    }
    else {
        attr_count_tmp++;
    }  
    LISTod
    if ((attr_count_tmp == 0) && !inheritance) {
        fprintf(file,"\t# This class does not define any attribute.\n");
        fprintf(file,"\tpass\n");
        generate_constructor = false;
    }
    if (false) {}
    else { 
    /* 
    * write class constructor
    */
    if (generate_constructor) {
        fprintf(file,"\tdef __init__( self , ");
    }
    // if inheritance, first write the inherited parameters
    list = ENTITYget_supertypes( entity );
    int num_parent = 0, index_attribute = 0;
    if( ! LISTempty( list ) ) {
        LISTdo( list, e, Entity )
            //num_attributes = get_local_attribute_number(e); //number of attributes for the current superclass
            //index_attribute = 0;
            /*  search attribute names for superclass */
            LISTdo(ENTITYget_attributes( e ), v2, Variable)
                //if (index_attribute<num_attributes+1) fprintf(file," , ");
                generate_attribute_name( v2, parent_attrnm );
                fprintf(file,"%s__%s , ",ENTITYget_name(e),parent_attrnm);
                index_attribute++;
            LISTod
        num_parent++;
        LISTod;
    }
    LISTdo(ENTITYget_attributes( entity ), v, Variable)
        generate_attribute_name( v, attrnm );
        if (!VARis_derived(v) && !VARget_inverse(v)) {
            fprintf(file,"%s,",attrnm);
        }
    LISTod
    // close constructor method
    if (generate_constructor) fprintf(file," ):\n");
    /** if inheritance, first init base class **/
    list = ENTITYget_supertypes( entity );
    if( ! LISTempty( list ) ) {
        LISTdo( list, e, Entity )
        fprintf(file,"\t\t%s.__init__(self , ",ENTITYget_name(e));
        /*  search and write attribute names for superclass */
            LISTdo(ENTITYget_attributes( e ), v2, Variable)
                generate_attribute_name( v2, parent_attrnm );
                fprintf(file,"%s__%s , ",ENTITYget_name(e),parent_attrnm);
            LISTod
        fprintf(file,")\n"); //separator for parent classes names
        LISTod;
    }
    // init variables in constructor
    //attr_count_tmp = attr_count;
    LISTdo(ENTITYget_attributes( entity ), v, Variable)
    generate_attribute_name( v, attrnm );
    if (!VARis_derived(v) && !VARget_inverse(v)) fprintf(file,"\t\tself.%s = %s\n",attrnm,attrnm);
    //attr_count_tmp++;
    LISTod
    /*
    * write attributes as python properties
    */
    //attr_count_tmp = attr_count;
    LISTdo( ENTITYget_attributes( entity ), v, Variable )
    generate_attribute_name( v, attrnm );
    fprintf(file,"\n\t@apply\n");
    fprintf(file,"\tdef %s():\n",attrnm);
    // fget
    fprintf(file,"\t\tdef fget( self ):\n");
    if (!VARis_derived(v)) {
        fprintf(file,"\t\t\treturn self._%s\n",attrnm);
    }
    else {
        // expression initializer
        char * expression_string = EXPRto_string( v->initializer );
        fprintf(file,"\t\t\treturn EvalDerivedAttribute(self,'''%s''')\n",expression_string);
        free( expression_string );
    }
    // fset
    fprintf(file,"\t\tdef fset( self, value ):\n");
    t = VARget_type( v );
    attr_type = GetAttrTypeName(t);
   
    if (!VARis_derived(v) && !VARget_inverse(v)) {
        // if the argument is not optional
        if (!VARget_optional(v)) {
            fprintf(file, "\t\t# Mandatory argument\n");
            fprintf(file,"\t\t\tif value==None:\n");
            fprintf(file,"\t\t\t\traise AssertionError('Argument %s is mantatory and can not be set to None')\n",attrnm);
        }
        else {
            fprintf(file,"\t\t\tif value != None: # OPTIONAL attribute\n\t");
        }
        // check wether attr_type is aggr or explicit
        if( TYPEis_aggregate( t ) ) {
            fprintf(file,"\t\t\tcheck_type(value,");
             process_aggregate(file,t);
             fprintf(file,")\n");
         }
        else {
            fprintf(file,"\t\t\tcheck_type(value,");
            //printf(attr_type);
            //is_python_keyword(attr_type);// printf("pou");
            fprintf(file,"%s)\n",attr_type);
        }
        fprintf(file,"\t\t\tself._%s = value\n",attrnm);
    }
    // if the attribute is derived, prevent fset to attribute to be set
    else if (VARis_derived(v)){
        fprintf(file,"\t\t# DERIVED argument\n");
        fprintf(file,"\t\t\traise AssertionError('Argument %s is DERIVED. It is computed and can not be set to any value')\n",attrnm);
    }
    else if (VARget_inverse(v)) {
        fprintf(file,"\t\t# INVERSE argument\n");
        fprintf(file,"\t\t\traise AssertionError('Argument %s is INVERSE. It is computed and can not be set to any value')\n",attrnm);
    }
    fprintf(file,"\t\treturn property(**locals())\n");
    LISTod
    }
}


/******************************************************************
 ** Procedure:  ENTITYinc_print
 ** Parameters:  Entity *entity --  entity being processed
 **     FILE* file  --  file being written to
 ** Returns:
 ** Description:  drives the generation of the c++ class definition code
 ** Side Effects:  prints segment of the c++ .h file
 ** Status:  ok 1/15/91
 ******************************************************************/
/*
void
ENTITYinc_print( Entity entity, FILE * file, Schema schema ) {
    ENTITYhead_print( entity, file, schema );
    DataMemberPrint( entity, file, schema );
    MemberFunctionSign( entity, file );
}
*/

int
get_local_attribute_number( Entity entity ) {
    int i = 0;
    Linked_List local = ENTITYget_attributes( entity );
    LISTdo( local, a, Variable )
    /*  go to the child's first explicit attribute */
    if( ( ! VARget_inverse( a ) ) && ( ! VARis_derived( a ) ) ) ++i;
    LISTod;
    return i;
}

int
get_attribute_number( Entity entity ) {
    int i = 0;
    int found = 0;
    Linked_List local, complete;
    complete = ENTITYget_all_attributes( entity );
    local = ENTITYget_attributes( entity );

    LISTdo( local, a, Variable )
    /*  go to the child's first explicit attribute */
    if( ( ! VARget_inverse( a ) ) && ( ! VARis_derived( a ) ) )  {
        LISTdo( complete, p, Variable )
        /*  cycle through all the explicit attributes until the
        child's attribute is found  */
        if( !found && ( ! VARget_inverse( p ) ) && ( ! VARis_derived( p ) ) ) {
            if( p != a ) {
                ++i;
            } else {
                found = 1;
            }
        }
        LISTod;
        if( found ) {
            return i;
        } else printf( "Internal error:  %s:%d\n"
                           "Attribute %s not found. \n"
                           /* In this case, a is a Variable - so macro VARget_name (a) expands  *
                            * to an Expression. The first element of an Expression is a Symbol. *
                            * The first element of a Symbol is char * name.                     */
                           , __FILE__, __LINE__, VARget_name( a )->symbol.name );
    }

    LISTod;
    return -1;
}

/******************************************************************
 ** Procedure:  LIBstructor_print
 ** Parameters:  Entity *entity --  entity being processed
 **     FILE* file  --  file being written to
 ** Returns:
 ** Description:  prints the c++ code for entity class's
 **     constructor and destructor.  goes to .cc file
 ** Side Effects:  generates codes segment in c++ .cc file
 ** Status:  ok 1/15/91
 ** Changes: Modified generator to initialize attributes to NULL based
 **          on the NULL symbols defined in "SDAI C++ Binding for PDES,
 **          Inc. Prototyping" by Stephen Clark.
 ** Change Date: 5/22/91 CD
 ** Changes: Modified STEPattribute constructors to take fewer arguments
 **     21-Dec-1992 -kcm
 ******************************************************************/
void
LIBstructor_print( Entity entity, FILE * file, Schema schema ) {
    Linked_List attr_list;
    Type t;
    char attrnm [BUFSIZ];

    Linked_List list;
    __attribute__( ( unused ) ) Entity super = 0;
    int super_cnt = 0;
    Entity principalSuper = 0;

    const char * entnm = ENTITYget_classname( entity );
    int count = attr_count;
    int index = 0;
    int first = 1;

    /*  constructor definition  */
    fprintf( file, "%s::%s( ) \n", entnm, entnm );
    fprintf( file, "{\n" );

    /*    super = ENTITYput_superclass (entity); */

    /* ////MULTIPLE INHERITANCE//////// */

    if( multiple_inheritance ) {
        fprintf( file, "\n" );
        list = ENTITYget_supertypes( entity );
        if( ! LISTempty( list ) ) {
            LISTdo( list, e, Entity )
            /*  if there\'s no super class yet,
                or the super class doesn\'t have any attributes
            */
            fprintf( file, "\t/*  parent: %s  */\n", ENTITYget_classname( e ) );

            super = e;
            super_cnt++;
            //if( super_cnt == 1 ) {
                /* ignore the 1st parent */
            //    fprintf( file,
            //             "\t/* Ignore the first parent since it is */\n %s\n",
            //             "\t/* part of the main inheritance hierarchy */" );
            //    principalSuper = e; /* principal SUPERTYPE */
            //} else {
                fprintf( file, "    HeadEntity(this); \n" );
                fprintf( file, "#if 0 \n" );
                fprintf( file,
                         "\t/* Optionally use the following to replace the line following \n" );
                fprintf( file,
                         "\t   the endif. Use this to turn off adding attributes in \n" );
                fprintf( file,
                         "\t   diamond shaped hierarchies for each additional parent at this\n" );
                fprintf( file,
                         "\t   level. You currently must hand edit this for it to work. */\n" );
                fprintf( file, "    int attrFlags[3]; // e.g. \n" );
                fprintf( file, "    attrFlags[0] = 1; // add parents attrs\n" );
                fprintf( file,
                         "    attrFlags[1] = 1; // add parent of parents attrs\n" );
                fprintf( file,
                         "    attrFlags[2] = 0; // do not add parent of parent of parents attrs\n" );
                fprintf( file,
                         "      // In *imaginary* hierarchy turn off attrFlags[2] since it \n" );
                fprintf( file,
                         "      // would be the parent that has more than one path to it.\n" );
                fprintf( file,
                         "    AppendMultInstance(new %s(this, attrFlags)); \n",
                         ENTITYget_classname( e ) );
                fprintf( file, "#endif \n" );

                fprintf( file, "    AppendMultInstance(new %s(this)); \n",
                         ENTITYget_classname( e ) );
                /*        fprintf (file, "new %s(this);  \n", ENTITYget_classname (e));*/

                if( super_cnt == 2 ) {
                    printf( "\nMULTIPLE INHERITANCE for entity: %s\n",
                            ENTITYget_name( entity ) );
                    printf( "\tSUPERTYPE 1: %s (principal supertype)\n",
                            ENTITYget_name( principalSuper ) );
                //}
                printf( "\tSUPERTYPE %d: %s\n", super_cnt, ENTITYget_name( e ) );
                /*      printf("\tin ENTITY %s\n\tadding SUPERTYPE %s. cp\n\n",
                               ENTITYget_name (entity), ENTITYget_name (e));*/
            }
            LISTod;

        } else {    /*  if entity has no supertypes, it's at top of hierarchy  */
            fprintf( file, "\t/*  no SuperTypes */\n" );
        }
    }
    /* ////MULTIPLE INHERITANCE//////// */

    /* Next lines added for independent field - DAR */
    /*  if ( ENTITYget_supertypes(entity) || ENTITYget_abstract(entity) ) {
        // If entity has supertypes or is abstract it's not independent.
        fprintf (file, "\n    _independent = 0;\n");
        fprintf (file, "    // entity either has supertypes or is abstract\n");
		// Otherwise, keep the default value of 1.
        }
    */
    /*  attributes  */
    /*    fprintf (file, "\n\tSTEPattribute * a;\n");*/

    /* what if entity comes from other schema? */
    fprintf( file, "\n    eDesc = %s%s%s;\n",
             SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

    attr_list = ENTITYget_attributes( entity );

    LISTdo( attr_list, a, Variable )
    if( VARget_initializer( a ) == EXPRESSION_NULL ) {
        /*  include attribute if it is not derived  */
        generate_attribute_name( a, attrnm );
        t = VARget_type( a );

        /*  1.  declare the AttrDescriptor  */
        /*  this is now in the header  */
        /*      fprintf(file,"extern AttrDescriptor *%s%d%s;\n",*/
        /*          ATTR_PREFIX,count,VARget_name(a));*/

        /*  if the attribute is Explicit, make a STEPattribute  */
        /*      if (VARis_simple_explicit (a))  {*/
        if( ( ! VARget_inverse( a ) ) && ( ! VARis_derived( a ) ) )  {
            /*  1. create a new STEPattribute */

            fprintf( file, "    "
                     "%sa = new STEPattribute(*%s%d%s%s, %s &_%s);\n",
                     ( first ? "STEPattribute *" : "" ),
                     /*  first time through declare a */
                     ATTR_PREFIX, count,
                     ( VARis_type_shifter( a ) ? "R" : "" ),
                     attrnm,
                     ( TYPEis_entity( t ) ? "(SCLP23(Application_instance_ptr) *)" : "" ),
                     attrnm );
            if( first ) {
                first = 0 ;
            }
            /*  2. initialize everything to NULL (even if not optional)  */

            fprintf( file, "    a -> set_null ();\n" );

            /*  3.  put attribute on attributes list  */
            fprintf( file, "    attributes.push (a);\n" );

            /* if it is redefining another attribute make connection of
               redefined attribute to redefining attribute */
            if( VARis_type_shifter( a ) ) {
                fprintf( file, "    MakeRedefined(a, \"%s\");\n",
                         VARget_simple_name( a ) );
            }
        }
        count++;
    }

    LISTod;

    attr_list = ENTITYget_all_attributes( entity );

    LISTdo( attr_list, a, Variable )
    /*      if (VARis_overrider (entity, a)) { */
    if( VARis_derived( a ) ) {
        fprintf( file, "    MakeDerived (\"%s\");\n",
                 VARget_simple_name( a ) );
    }
    LISTod;
    fprintf( file, "}\n" );

    /*  copy constructor  */
    /*  LIBcopy_constructor (entity, file); */
    entnm = ENTITYget_classname( entity );
    fprintf( file, "%s::%s (%s& e ) \n", entnm, entnm, entnm );
    fprintf( file, "\t{  CopyAs((SCLP23(Application_instance_ptr)) &e);\t}\n" );

    /*  print destructor  */
    /*  currently empty, but should check to see if any attributes need
    to be deleted -- attributes will need reference count  */

    entnm = ENTITYget_classname( entity );
    fprintf( file, "%s::~%s () {  }\n", entnm, entnm );

    /*  Open OODB reInit function  */
    fprintf( file, "\n#ifdef __O3DB__\n" );
    fprintf( file, "void \n%s::oodb_reInit ()\n{", entnm );
    fprintf( file, "\teDesc = %s%s%s;\n",
             SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

    count = attr_count;
    attr_list = ENTITYget_attributes( entity );
    index = get_attribute_number( entity );

    LISTdo( attr_list, a, Variable )
    /*  if the attribute is Explicit, assign the Descriptor  */
    if( ( ! VARget_inverse( a ) ) && ( ! VARis_derived( a ) ) )  {
        generate_attribute_name( a, attrnm );
        /*  1. assign the Descriptor for the STEPattributes */
        fprintf( file, "\tattributes [%d].aDesc = %s%d%s%s;\n",
                 index,
                 ATTR_PREFIX, count,
                 ( VARis_type_shifter( a ) ? "R" : "" ),
                 attrnm );
    }
    index++,
          count++;
    LISTod;
    fprintf( file, "}\n"
             "#endif\n\n" );


}

/********************/
/* print the constructor that accepts a SCLP23(Application_instance) as an argument used
   when building multiply inherited entities.
*/

void
LIBstructor_print_w_args( Entity entity, FILE * file, Schema schema ) {
    Linked_List attr_list;
    Type t;
    char attrnm [BUFSIZ];

    Linked_List list;
    __attribute__( ( unused ) ) Entity super = 0;
    int super_cnt = 0;

    /* added for calling parents constructor if there is one */
    char parentnm [BUFSIZ];
    char * parent = 0;
    Entity parentEntity = 0;

    const char * entnm;
    int count = attr_count;
    int first = 1;

    if( multiple_inheritance ) {

        /* //////////// */
        list = ENTITYget_supertypes( entity );
        if( ! LISTempty( list ) ) {
            parentEntity = ( Entity )LISTpeek_first( list );
            if( parentEntity ) {
                strcpy( parentnm, ENTITYget_classname( parentEntity ) );
                parent = parentnm;
            } else {
                parent = 0;    /* no parent */
            }
        } else {
            parent = 0;    /* no parent */
        }

        /* ENTITYget_classname returns a static buffer so don't call it twice
           before it gets used - (I didn't write it) - I had to move it below
            the above use. DAS */
        entnm = ENTITYget_classname( entity );
        /*  constructor definition  */
        if( parent )
            fprintf( file, "%s::%s (SCLP23(Application_instance) *se, int *addAttrs) : %s(se, (addAttrs ? &addAttrs[1] : 0)) \n", entnm, entnm,
                     parentnm );
        else {
            fprintf( file, "%s::%s( SCLP23(Application_instance) *se, int *addAttrs)\n", entnm, entnm );
        }

        fprintf( file, "{\n" );

        /* ////MULTIPLE INHERITANCE//////// */
        /*    super = ENTITYput_superclass (entity); */

        fprintf( file, "\t/* Set this to point to the head entity. */\n" );
        fprintf( file, "    HeadEntity(se); \n" );

        fprintf( file, "\n" );
        list = ENTITYget_supertypes( entity );
        if( ! LISTempty( list ) ) {
            LISTdo( list, e, Entity )
            /*  if there\'s no super class yet,
                or the super class doesn\'t have any attributes
                */
            fprintf( file, "\t/*  parent: %s  */\n", ENTITYget_classname( e ) );

            super = e;
            super_cnt++;
            if( super_cnt == 1 ) {
                /* ignore the 1st parent */
                fprintf( file,
                         "\t/* Ignore the first parent since it is */\n %s\n",
                         "\t/* part of the main inheritance hierarchy */" );
            }  else {
                fprintf( file, "#if 0 \n" );
                fprintf( file,
                         "\t/* Optionally use the following to replace the line following \n" );
                fprintf( file,
                         "\t   the endif. Use this to turn off adding attributes in \n" );
                fprintf( file,
                         "\t   diamond shaped hierarchies for each additional parent at this\n" );
                fprintf( file,
                         "\t   level. You currently must hand edit this for it to work. */\n" );
                fprintf( file, "    int attrFlags[3]; // e.g. \n" );
                fprintf( file, "    attrFlags[0] = 1; // add parents attrs\n" );
                fprintf( file,
                         "    attrFlags[1] = 1; // add parent of parents attrs\n" );
                fprintf( file,
                         "    attrFlags[2] = 0; // do not add parent of parent of parents attrs\n" );
                fprintf( file,
                         "      // In *imaginary* hierarchy turn off attrFlags[2] since it \n" );
                fprintf( file,
                         "      // would be the parent that has more than one path to it.\n" );
                fprintf( file,
                         "    se->AppendMultInstance(new %s(se, attrFlags)); \n",
                         ENTITYget_classname( e ) );
                fprintf( file, "#endif \n" );
                fprintf( file, "    se->AppendMultInstance(new %s(se, 0)); \n",
                         ENTITYget_classname( e ) );
                /*      printf("\tin ENTITY %s\n\thandling SUPERTYPE %s cp wArgs\n\n",
                            ENTITYget_name (entity), ENTITYget_name (e));*/
            }
            LISTod;

        }  else {   /*  if entity has no supertypes, it's at top of hierarchy  */
            fprintf( file, "\t/*  no SuperTypes */\n" );
        }

        /* ////MULTIPLE INHERITANCE//////// */

        /*  attributes  */
        /*    fprintf (file, "\n    STEPattribute * a;\n");*/

        /* what if entity comes from other schema? */
        fprintf( file, "\n    eDesc = %s%s%s;\n",
                 SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

        attr_list = ENTITYget_attributes( entity );

        LISTdo( attr_list, a, Variable )
        if( VARget_initializer( a ) == EXPRESSION_NULL ) {
            /*  include attribute if it is not derived  */
            generate_attribute_name( a, attrnm );
            t = VARget_type( a );

            /*  1.  declare the AttrDescriptor  */
            /*  this is now in the header  */
            /*      fprintf(file,"extern AttrDescriptor *%s%d%s;\n",*/
            /*          ATTR_PREFIX,count,VARget_name(a));*/

            /*  if the attribute is Explicit, make a STEPattribute  */
            /*      if (VARis_simple_explicit (a))  {*/
            if( ( ! VARget_inverse( a ) ) && ( ! VARis_derived( a ) ) )  {
                /*  1. create a new STEPattribute */

                fprintf( file, "    "
                         "%sa = new STEPattribute(*%s%d%s%s, %s &_%s);\n",
                         ( first ? "STEPattribute *" : "" ),
                         /*  first time through declare a */
                         ATTR_PREFIX, count,
                         ( VARis_type_shifter( a ) ? "R" : "" ),
                         attrnm,
                         ( TYPEis_entity( t ) ? "(SCLP23(Application_instance_ptr) *)" : "" ),
                         attrnm );

                if( first ) {
                    first = 0 ;
                }
                /*  2. initialize everything to NULL (even if not optional)  */

                fprintf( file, "    a -> set_null ();\n" );

                fprintf( file,
                         "\t/* Put attribute on this class' %s\n",
                         "attributes list so the */\n\t/*access functions still work. */" );
                /*  3.  put attribute on this class' attributes list so the
                access functions still work */
                fprintf( file, "    attributes.push (a);\n" );
                fprintf( file,
                         "\t/* Put attribute on the attributes list %s\n",
                         "for the */\n\t/* main inheritance heirarchy. */" );
                /* ////MULTIPLE INHERITANCE//////// */
                /*  4.  put attribute on attributes list for the main
                inheritance heirarchy */
                fprintf( file, "    if(!addAttrs || addAttrs[0])\n" );
                fprintf( file, "        se->attributes.push (a);\n" );

                /* if it is redefining another attribute make connection of
                   redefined attribute to redefining attribute */
                if( VARis_type_shifter( a ) ) {
                    fprintf( file, "    MakeRedefined(a, \"%s\");\n",
                             VARget_simple_name( a ) );
                }
                /* ////MULTIPLE INHERITANCE//////// */
            }
            count++;
        }

        LISTod;

        attr_list = ENTITYget_all_attributes( entity );

        LISTdo( attr_list, a, Variable )
        /*      if (VARis_overrider (entity, a)) { */
        if( VARis_derived( a ) ) {
            fprintf( file, "    MakeDerived (\"%s\");\n",
                     VARget_simple_name( a ) );
        }
        LISTod;
        fprintf( file, "}\n" );
    } /* end if(multiple_inheritance) */

}
/********************/

/******************************************************************
 ** Procedure:  ENTITYlib_print
 ** Parameters:  Entity *entity --  entity being processed
 **     FILE* file  --  file being written to
 ** Returns:
 ** Description:  drives the printing of the code for the class library
 **     additional member functions can be generated by writing a routine
 **     to generate the code and calling that routine from this procedure
 ** Side Effects:  generates code segment for c++ library file
 ** Status:  ok 1/15/91
 ******************************************************************/

void
ENTITYlib_print( Entity entity, FILE * file, Schema schema ) {
    LIBdescribe_entity( entity, file, schema );
    //LIBstructor_print( entity, file, schema );
    //if( multiple_inheritance ) {
    //    LIBstructor_print_w_args( entity, file, schema );
    //}
    //LIBmemberFunctionPrint( entity, file );
}

//FIXME should return bool
/* return 1 if types are predefined by us */
int
TYPEis_builtin( const Type t ) {
    switch( TYPEget_body( t )->type ) { /* dunno if correct*/
        case integer_:
        case real_:
        case string_:
        case binary_:
        case boolean_:
        case number_:
        case logical_:
            return 1;
            break;
        default:
            break;
    }
    return 0;
}

/* go down through a type'sbase type chain,
   Make and print new TypeDescriptors for each type with no name.

   This function should only be called for types that don't have an
   associated Express name.  Currently this only includes aggregates.
   If this changes this function needs to be changed to support the type
   that changed.  This function prints TypeDescriptors for types
   without names and it will go down through the type chain until it hits
   a type that has a name.  i.e. when it hits a type with a name it stops.
   There are only two places where a type can not have a name - both
   cases are aggregate types.
   1. an aggregate created in an attr declaration
      e.g. names : ARRAY [1:3] of STRING;
   2. an aggregate that is an element of another aggregate.
      e.g. TYPE Label = STRING; END_TYPE;
           TYPE listSetOfLabel = LIST of SET of Label; END_TYPE;
      LIST of SET of Label has a name i.e. listSetOfReal
      SET of Label does not have a name and this function should be called
         to generate one.
      This function will not generate the code to handle Label.

      Type t contains the Type with no Express name that needs to have
        TypeDecriptor[s] generated for it.
      buf needs to have space declared enough to hold the name of the var
        that can be referenced to refer to the type that was created for
    Type t.
*/
void
print_typechain( FILE * f, const Type t, char * buf, Schema schema ) {
}

/******************************************************************
 ** Procedure:  ENTITYincode_print
 ** Parameters:  Entity *entity --  entity being processed
 **     FILE* file  --  file being written to
 ** Returns:
 ** Description:  generates code to enter entity in STEP registry
 **      This goes to the .init.cc file
 ** Side Effects:
 ** Status:  ok 1/15/91
 ******************************************************************/
void
ENTITYincode_print( Entity entity, FILE * file, Schema schema ) {
#define entity_name ENTITYget_name(entity)
#define schema_name SCHEMAget_name(schema)
    char attrnm [BUFSIZ];
    char dict_attrnm [BUFSIZ];
    const char * super_schema;
    char * tmp, *tmp2;

#ifdef NEWDICT
    /* DAS New SDAI Dictionary 5/95 */
    /* insert the entity into the schema descriptor */
    fprintf( file,
             "\t((SDAIAGGRH(Set,EntityH))%s%s->Entities())->Add(%s%s%s);\n",
             SCHEMA_PREFIX, schema_name, schema_name, ENT_PREFIX, entity_name );
#endif

    if( ENTITYget_abstract( entity ) ) {
        fprintf( file, "\t%s%s%s->AddSupertype_Stmt(\"",
                 schema_name, ENT_PREFIX, entity_name );
        if( entity->u.entity->subtype_expression ) {
            fprintf( file, "ABSTRACT SUPERTYPE OF (" );
            tmp = SUBTYPEto_string( entity->u.entity->subtype_expression );
            tmp2 = ( char * )malloc( sizeof( char ) * ( strlen( tmp ) + BUFSIZ ) );
            fprintf( file, "%s)\");\n", format_for_stringout( tmp, tmp2 ) );
            free( tmp );
            free( tmp2 );
        } else {
            fprintf( file, "ABSTRACT SUPERTYPE\");\n" );
        }
    } else {
        if( entity->u.entity->subtype_expression ) {
            fprintf( file, "\t%s%s%s->AddSupertype_Stmt(\"",
                     schema_name, ENT_PREFIX, entity_name );
            fprintf( file, "SUPERTYPE OF (" );
            tmp = SUBTYPEto_string( entity->u.entity->subtype_expression );
            tmp2 = ( char * )malloc( sizeof( char ) * ( strlen( tmp ) + BUFSIZ ) );
            fprintf( file, "%s)\");\n", format_for_stringout( tmp, tmp2 ) );
            free( tmp );
            free( tmp2 );
        }
    }
    /*
        if (entity->u.entity->subtype_expression) {
            tmp = SUBTYPEto_string(entity->u.entity->subtype_expression);
            tmp2 = (char*)malloc( sizeof(char) * (strlen(tmp)+BUFSIZ) );
            fprintf(file,"\t%s%s%s->AddSupertype_Stmt(\"(%s)\");\n",
                schema_name,ENT_PREFIX,entity_name,format_for_stringout(tmp,tmp2));
            free(tmp);
            free(tmp2);
        }
    */
    /*
        LISTdo(ENTITYget_subtypes(entity),sub,Entity)
            fprintf(file,"  %s%s%s->AddSubtype(%s%s%s);\n",
                schema_name,ENT_PREFIX,entity_name,
                schema_name,ENT_PREFIX,ENTITYget_name(sub));
        LISTod
    */
    LISTdo( ENTITYget_supertypes( entity ), sup, Entity )
    /*  set the owning schema of the supertype  */
    super_schema = SCHEMAget_name( ENTITYget_schema( sup ) );
    /* print the supertype list for this entity */
    fprintf( file, "	%s%s%s->AddSupertype(%s%s%s);\n",
             schema_name, ENT_PREFIX, entity_name,
             super_schema,
             ENT_PREFIX, ENTITYget_name( sup ) );

    /* add this entity to the subtype list of it's supertype    */
    fprintf( file, "	%s%s%s->AddSubtype(%s%s%s);\n",
             super_schema,
             ENT_PREFIX, ENTITYget_name( sup ),
             schema_name, ENT_PREFIX, entity_name );
    LISTod

    LISTdo( ENTITYget_attributes( entity ), v, Variable )
    generate_attribute_name( v, attrnm );
    /*  do EXPLICIT and DERIVED attributes first  */
    /*    if  ( ! VARget_inverse (v))  {*/
    /* first make sure that type descriptor exists */
    if( TYPEget_name( v->type ) ) {
        if( ( !TYPEget_head( v->type ) ) &&
                ( TYPEget_body( v->type )->type == entity_ ) ) {
            /*          fprintf(file, "\t%s%d%s%s = new %sAttrDescriptor(\"%s\",%s%s%s,%s,%s,%s,*%s%s%s);\n", */
            fprintf( file, "\t%s%d%s%s =\n\t  new %s"
                     "(\"%s\",%s%s%s,\n\t  %s,%s%s,\n\t  *%s%s%s);\n",
                     ATTR_PREFIX, attr_count,
                     ( VARis_derived( v ) ? "D" :
                       ( VARis_type_shifter( v ) ? "R" :
                         ( VARget_inverse( v ) ? "I" : "" ) ) ),
                     attrnm,

                     ( VARget_inverse( v ) ? "Inverse_attribute" : ( VARis_derived( v ) ? "Derived_attribute" : "AttrDescriptor" ) ),

                     /* attribute name param */
                     generate_dict_attr_name( v, dict_attrnm ),

                     /* following assumes we are not in a nested */
                     /* entity otherwise we should search upward */
                     /* for schema */
                     /* attribute's type  */
                     TYPEget_name(
                         TYPEget_body( v->type )->entity->superscope ),
                     ENT_PREFIX, TYPEget_name( v->type ),

                     ( VARget_optional( v ) ? "LTrue" : "LFalse" ),

                     ( VARget_unique( v ) ? "LTrue" : "LFalse" ),

                     /* Support REDEFINED */
                     ( VARget_inverse( v ) ? "" :
                       ( VARis_derived( v ) ? ", AttrType_Deriving" :
                         ( VARis_type_shifter( v ) ? ", AttrType_Redefining" : ", AttrType_Explicit" ) ) ),

                     schema_name, ENT_PREFIX, TYPEget_name( entity )
                   );
        } else {
            /* type reference */
            /*          fprintf(file,"  %s%d%s%s = new %sAttrDescriptor(\"%s\",%s%s%s,%s,%s,%s,*%s%s%s);\n",*/
            fprintf( file, "  %s%d%s%s =\n\t  new %s"
                     "(\"%s\",%s%s%s,\n\t  %s,%s%s,\n\t  *%s%s%s);\n",
                     ATTR_PREFIX, attr_count,
                     ( VARis_derived( v ) ? "D" :
                       ( VARis_type_shifter( v ) ? "R" :
                         ( VARget_inverse( v ) ? "I" : "" ) ) ),
                     attrnm,

                     ( VARget_inverse( v ) ? "Inverse_attribute" : ( VARis_derived( v ) ? "Derived_attribute" : "AttrDescriptor" ) ),

                     /* attribute name param */
                     generate_dict_attr_name( v, dict_attrnm ),

                     SCHEMAget_name( v->type->superscope ),
                     TD_PREFIX, TYPEget_name( v->type ),

                     ( VARget_optional( v ) ? "LTrue" : "LFalse" ),

                     ( VARget_unique( v ) ? "LTrue" : "LFalse" ),

                     ( VARget_inverse( v ) ? "" :
                       ( VARis_derived( v ) ? ", AttrType_Deriving" :
                         ( VARis_type_shifter( v ) ? ", AttrType_Redefining" : ", AttrType_Explicit" ) ) ),

                     schema_name, ENT_PREFIX, TYPEget_name( entity )
                   );
        }
    } else if( TYPEis_builtin( v->type ) ) {
        /*  the type wasn\'t named -- it must be built in or aggregate  */

        /*          fprintf(file,"  %s%d%s%s = new %sAttrDescriptor(\"%s\",%s%s,%s,%s,%s,*%s%s%s);\n",*/
        fprintf( file, "  %s%d%s%s =\n\t  new %s"
                 "(\"%s\",%s%s,\n\t  %s,%s%s,\n\t  *%s%s%s);\n",
                 ATTR_PREFIX, attr_count,
                 ( VARis_derived( v ) ? "D" :
                   ( VARis_type_shifter( v ) ? "R" :
                     ( VARget_inverse( v ) ? "I" : "" ) ) ),
                 attrnm,
                 ( VARget_inverse( v ) ? "Inverse_attribute" : ( VARis_derived( v ) ? "Derived_attribute" : "AttrDescriptor" ) ),
                 /* attribute name param */
                 generate_dict_attr_name( v, dict_attrnm ),
                 /* not sure about 0 here */ TD_PREFIX, FundamentalType( v->type, 0 ),
                 ( VARget_optional( v ) ? "LTrue" :
                   "LFalse" ),
                 ( VARget_unique( v ) ? "LTrue" :
                   "LFalse" ),
                 ( VARget_inverse( v ) ? "" :
                   ( VARis_derived( v ) ? ", AttrType_Deriving" :
                     ( VARis_type_shifter( v ) ?
                       ", AttrType_Redefining" :
                       ", AttrType_Explicit" ) ) ),
                 schema_name, ENT_PREFIX, TYPEget_name( entity )
               );
    } else {
        /* manufacture new one(s) on the spot */
        char typename_buf[MAX_LEN];
        print_typechain( file, v->type, typename_buf, schema );
        /*          fprintf(file,"  %s%d%s%s = new %sAttrDescriptor(\"%s\",%s,%s,%s,%s,*%s%s%s);\n",*/
        fprintf( file, "  %s%d%s%s =\n\t  new %s"
                 "(\"%s\",%s,%s,%s%s,\n\t  *%s%s%s);\n",
                 ATTR_PREFIX, attr_count,
                 ( VARis_derived( v ) ? "D" :
                   ( VARis_type_shifter( v ) ? "R" :
                     ( VARget_inverse( v ) ? "I" : "" ) ) ),
                 attrnm,
                 ( VARget_inverse( v ) ? "Inverse_attribute" : ( VARis_derived( v ) ? "Derived_attribute" : "AttrDescriptor" ) ),
                 /* attribute name param */
                 generate_dict_attr_name( v, dict_attrnm ),
                 typename_buf,
                 ( VARget_optional( v ) ? "LTrue" :
                   "LFalse" ),
                 ( VARget_unique( v ) ? "LTrue" :
                   "LFalse" ),
                 ( VARget_inverse( v ) ? "" :
                   ( VARis_derived( v ) ? ", AttrType_Deriving" :
                     ( VARis_type_shifter( v ) ?
                       ", AttrType_Redefining" :
                       ", AttrType_Explicit" ) ) ),
                 schema_name, ENT_PREFIX, TYPEget_name( entity )
               );
    }

    fprintf( file, "	%s%s%s->Add%sAttr (%s%d%s%s);\n",
             schema_name, ENT_PREFIX, TYPEget_name( entity ),
             ( VARget_inverse( v ) ? "Inverse" : "Explicit" ),
             ATTR_PREFIX, attr_count,
             ( VARis_derived( v ) ? "D" :
               ( VARis_type_shifter( v ) ? "R" :
                 ( VARget_inverse( v ) ? "I" : "" ) ) ),
             attrnm );

    if( VARis_derived( v ) && v->initializer ) {
        tmp = EXPRto_string( v->initializer );
        tmp2 = ( char * )malloc( sizeof( char ) * ( strlen( tmp ) + BUFSIZ ) );
        fprintf( file, "\t%s%d%s%s->initializer_(\"%s\");\n",
                 ATTR_PREFIX, attr_count,
                 ( VARis_derived( v ) ? "D" :
                   ( VARis_type_shifter( v ) ? "R" :
                     ( VARget_inverse( v ) ? "I" : "" ) ) ),
                 attrnm, format_for_stringout( tmp, tmp2 ) );
        free( tmp );
        free( tmp2 );
    }
    if( VARget_inverse( v ) ) {
        fprintf( file, "\t%s%d%s%s->inverted_attr_id_(\"%s\");\n",
                 ATTR_PREFIX, attr_count,
                 ( VARis_derived( v ) ? "D" :
                   ( VARis_type_shifter( v ) ? "R" :
                     ( VARget_inverse( v ) ? "I" : "" ) ) ),
                 attrnm, v->inverse_attribute->name->symbol.name );
        if( v->type->symbol.name ) {
            fprintf( file,
                     "\t%s%d%s%s->inverted_entity_id_(\"%s\");\n",
                     ATTR_PREFIX, attr_count,
                     ( VARis_derived( v ) ? "D" :
                       ( VARis_type_shifter( v ) ? "R" :
                         ( VARget_inverse( v ) ? "I" : "" ) ) ), attrnm,
                     v->type->symbol.name );
            fprintf( file, "// inverse entity 1 %s\n", v->type->symbol.name );
        } else {
            /*          fprintf(file,"// inverse entity %s",TYPE_body_out(v->type));*/
            switch( TYPEget_body( v->type )->type ) {
                case entity_:
                    fprintf( file,
                             "\t%s%d%s%s->inverted_entity_id_(\"%s\");\n",
                             ATTR_PREFIX, attr_count,
                             ( VARis_derived( v ) ? "D" :
                               ( VARis_type_shifter( v ) ? "R" :
                                 ( VARget_inverse( v ) ? "I" : "" ) ) ), attrnm,
                             TYPEget_body( v->type )->entity->symbol.name );
                    fprintf( file, "// inverse entity 2 %s\n", TYPEget_body( v->type )->entity->symbol.name );
                    break;
                case aggregate_:
                case array_:
                case bag_:
                case set_:
                case list_:
                    fprintf( file,
                             "\t%s%d%s%s->inverted_entity_id_(\"%s\");\n",
                             ATTR_PREFIX, attr_count,
                             ( VARis_derived( v ) ? "D" :
                               ( VARis_type_shifter( v ) ? "R" :
                                 ( VARget_inverse( v ) ? "I" : "" ) ) ), attrnm,
                             TYPEget_body( v->type )->base->symbol.name );
                    fprintf( file, "// inverse entity 3 %s\n", TYPEget_body( v->type )->base->symbol.name );
                    break;
                default:
                    printf( "Error in %s, line %d: type %d not handled by switch statement.", __FILE__, __LINE__, TYPEget_body( v->type )->type );
                    abort();
            }
        }
    }
attr_count++;

LISTod

fprintf( file, "\treg.AddEntity (*%s%s%s);\n",
         schema_name, ENT_PREFIX, entity_name );

#undef schema_name
}

/******************************************************************
 ** Procedure:  ENTITYPrint
 ** Parameters:  Entity *entity --  entity being processed
 **     FILE* file  --  file being written to
 ** Returns:
 ** Description:  drives the functions for printing out code in lib,
 **     include, and initialization files for a specific entity class
 ** Side Effects:  generates code in 3 files
 ** Status:  complete 1/15/91
 ******************************************************************/


void
ENTITYPrint( Entity entity, FILES * files, Schema schema ) {
    char * n = ENTITYget_name( entity );
    DEBUG( "Entering ENTITYPrint for %s\n", n );
    fprintf( files->lib, "\n####################\n # ENTITY %s #\n####################\n", n );
    ENTITYlib_print( entity, files -> lib, schema );
    DEBUG( "DONE ENTITYPrint\n" )    ;
}

void
MODELPrintConstructorBody( Entity entity, FILES * files, Schema schema
                           /*, int index*/ ) {
    const char * n;
    DEBUG( "Entering MODELPrintConstructorBody for %s\n", n );

    n = ENTITYget_classname( entity );

    fprintf( files->lib, "        eep = new SCLP23(Entity_extent);\n" );


    fprintf( files->lib, "    eep->definition_(%s%s%s);\n",
             SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );
    fprintf( files->lib, "    _folders.Append(eep);\n\n" );

    /*
      fprintf (files->lib, "    %s__set_var SdaiModel_contents_%s::%s_get_extents()\n",
           n, SCHEMAget_name(schema), n);

           fprintf(files->create,"  %s%s%s = new EntityDescriptor(\"%s\",%s%s,%s, (Creator) create_%s);\n",

            PrettyTmpName (ENTITYget_name(entity)),
            SCHEMA_PREFIX,SCHEMAget_name(schema),
            (ENTITYget_abstract(entity)?"LTrue":"LFalse"),
            ENTITYget_classname (entity)
            );


      fprintf (files->lib,
           "{\n    return (%s__set_var)((_folders.retrieve(%d))->instances_());\n}\n",
           n, index);
    */
}

void
MODELPrint( Entity entity, FILES * files, Schema schema, int index ) {

    const char * n;
    DEBUG( "Entering MODELPrint for %s\n", n );

    n = ENTITYget_classname( entity );
    fprintf( files->lib, "\n%s__set_var SdaiModel_contents_%s::%s_get_extents()\n",
             n, SCHEMAget_name( schema ), n );
    fprintf( files->lib,
             "{\n    return (%s__set_var)((_folders.retrieve(%d))->instances_());\n}\n",
             n, index );
    /*
      fprintf (files->lib,
           "{\n    return (%s__set_var)((_folders[%d])->instances_());\n}\n",
           n, index);
    */

    /* //////////////// */
    /*
      fprintf (files->inc, "\n/////////\t ENTITY %s\n\n", n);
      ENTITYinc_print (entity, files -> inc,schema);
      fprintf (files->inc, "\n/////////\t END_ENTITY %s\n\n", n);

      fprintf (files->lib, "\n/////////\t ENTITY %s\n\n", n);
      ENTITYlib_print (entity, files -> lib,schema);
      fprintf (files->lib, "\n/////////\t END_ENTITY %s\n\n", n);

      fprintf (files->init, "\n/////////\t ENTITY %s\n\n", n);
      ENTITYincode_print (entity, files -> init, schema);
      fprintf (files->init, "/////////\t END_ENTITY %s\n", n);
    */
    DEBUG( "DONE MODELPrint\n" )    ;
}

/*
getEntityDescVarName(Entity entity)
{
    SCHEMAget_name(schema),ENT_PREFIX,ENTITYget_name(entity),
}
*/

/* print in include file: class forward prototype, class typedefs, and
   extern EntityDescriptor.  `externMap' = 1 if entity must be instantiated
   with external mapping (see Part 21, sect 11.2.5.1).  */
void
ENTITYprint_new( Entity entity, FILES * files, Schema schema, int externMap ) {
    const char * n;
    Linked_List wheres;
    /*  char buf[BUFSIZ],buf2[BUFSIZ]; */
    char * whereRule, *whereRule_formatted = "";
    int whereRule_formatted_size = 0;
    char * ptr, *ptr2;
    char * uniqRule, *uniqRule_formatted;
    Linked_List uniqs;
    int i;

    fprintf( files->create, "\t%s%s%s = new EntityDescriptor(\n\t\t",
             SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );
    fprintf( files->create, "  \"%s\", %s%s, %s, ",
             PrettyTmpName( ENTITYget_name( entity ) ),
             SCHEMA_PREFIX, SCHEMAget_name( schema ),
             ( ENTITYget_abstract( entity ) ? "LTrue" :
               "LFalse" ) );
    fprintf( files->create, "%s,\n\t\t", externMap ? "LTrue" :
             "LFalse" );

    fprintf( files->create, "  (Creator) create_%s );\n",
             ENTITYget_classname( entity ) );
    /* add the entity to the Schema dictionary entry */
    fprintf( files->create, "\t%s%s->AddEntity(%s%s%s);\n", SCHEMA_PREFIX, SCHEMAget_name( schema ), SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

    wheres = TYPEget_where( entity );

    if( wheres ) {
        fprintf( files->create,
                 "\t%s%s%s->_where_rules = new Where_rule__list;\n",
                 SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

        LISTdo( wheres, w, Where )
        whereRule = EXPRto_string( w->expr );
        ptr2 = whereRule;

        if( whereRule_formatted_size == 0 ) {
            whereRule_formatted_size = 3 * BUFSIZ;
            whereRule_formatted = ( char * )malloc( sizeof( char ) * whereRule_formatted_size );
        } else if( ( strlen( whereRule ) + 300 ) > whereRule_formatted_size ) {
            free( whereRule_formatted );
            whereRule_formatted_size = strlen( whereRule ) + BUFSIZ;
            whereRule_formatted = ( char * )malloc( sizeof( char ) * whereRule_formatted_size );
        }
        whereRule_formatted[0] = '\0';
        /*
                printf("whereRule length: %d\n",strlen(whereRule));
                printf("whereRule_formatted size: %d\n",whereRule_formatted_size);
        */
        if( w->label ) {
            strcpy( whereRule_formatted, w->label->name );
            strcat( whereRule_formatted, ": (" );
            ptr = whereRule_formatted + strlen( whereRule_formatted );
            while( *ptr2 ) {
                if( *ptr2 == '\n' ) {
                    ;
                } else if( *ptr2 == '\\' ) {
                    *ptr = '\\';
                    ptr++;
                    *ptr = '\\';
                    ptr++;

                } else if( *ptr2 == '(' ) {
                    *ptr = '\\';
                    ptr++;
                    *ptr = 'n';
                    ptr++;
                    *ptr = '\\';
                    ptr++;
                    *ptr = 't';
                    ptr++;
                    *ptr = *ptr2;
                    ptr++;
                } else {
                    *ptr = *ptr2;
                    ptr++;
                }
                ptr2++;
            }
            *ptr = '\0';

            strcat( ptr, ");\\n" );
        } else {
            /* no label */
            strcpy( whereRule_formatted, "(" );
            ptr = whereRule_formatted + strlen( whereRule_formatted );

            while( *ptr2 ) {
                if( *ptr2 == '\n' ) {
                    ;
                } else if( *ptr2 == '\\' ) {
                    *ptr = '\\';
                    ptr++;
                    *ptr = '\\';
                    ptr++;

                } else if( *ptr2 == '(' ) {
                    *ptr = '\\';
                    ptr++;
                    *ptr = 'n';
                    ptr++;
                    *ptr = '\\';
                    ptr++;
                    *ptr = 't';
                    ptr++;
                    *ptr = *ptr2;
                    ptr++;
                } else {
                    *ptr = *ptr2;
                    ptr++;
                }
                ptr2++;
            }
            *ptr = '\0';
            strcat( ptr, ");\\n" );
        }
        fprintf( files->create, "\twr = new Where_rule(\"%s\");\n", whereRule_formatted );
        fprintf( files->create, "\t%s%s%s->_where_rules->Append(wr);\n",
                 SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

        free( whereRule );
        ptr2 = whereRule = 0;
        LISTod
    }

    uniqs = entity->u.entity->unique;

    if( uniqs ) {
        fprintf( files->create,
                 "\t%s%s%s->_uniqueness_rules = new Uniqueness_rule__set;\n",
                 SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

        if( whereRule_formatted_size == 0 ) {
            uniqRule_formatted = ( char * )malloc( sizeof( char ) * 2 * BUFSIZ );
            whereRule_formatted = uniqRule_formatted;
        } else {
            uniqRule_formatted = whereRule_formatted;
        }

        /*******/
        /*
        DASBUG
         * take care of qualified attribute names like SELF\entity.attrname
         * add parent entity to the uniqueness rule
         * change EntityDescriptor::generate_express() to generate the UNIQUE clause
        */
        LISTdo( uniqs, list, Linked_List )
        i = 0;
        fprintf( files->create, "\tur = new Uniqueness_rule(\"" );
        LISTdo( list, v, Variable )
        i++;
        if( i == 1 ) {
            /* print label if present */
            if( v ) {
                fprintf( files->create, "%s : ", StrToUpper( ( ( Symbol * )v )->name ) );
            }
        } else {
            if( i > 2 ) {
                fprintf( files->create, ", " );
            }
            uniqRule = EXPRto_string( v->name );
            fprintf( files->create, "%s", uniqRule );
        }
        LISTod
        fprintf( files->create, ";\\n\");\n" );
        fprintf( files->create, "\t%s%s%s->_uniqueness_rules->Append(ur);\n",
                 SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );
        LISTod
        /********/

    }

    if( whereRule_formatted_size > 0 ) {
        free( whereRule_formatted );
    }

    n = ENTITYget_classname( entity );
    fprintf( files->classes, "\nclass %s;\n", n );
    fprintf( files->classes, "typedef %s *  \t%sH;\n", n, n );
    fprintf( files->classes, "typedef %s *  \t%s_ptr;\n", n, n );
    fprintf( files->classes, "typedef %s_ptr\t%s_var;\n", n, n );

    fprintf( files->classes,
             "#define %s__set \tSCLP23(DAObject__set)\n", n );

    fprintf( files->classes,
             "#define %s__set_var \tSCLP23(DAObject__set_var)\n", n );

    fprintf( files ->classes, "extern EntityDescriptor \t*%s%s%s;\n",
             SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

}

void
MODELprint_new( Entity entity, FILES * files, Schema schema ) {
    const char * n;

    n = ENTITYget_classname( entity );
    fprintf( files->inc, "\n    %s__set_var %s_get_extents();\n", n, n );
    /*
        fprintf(files->create," %s%s%s = new EntityDescriptor(\"%s\",%s%s,%s, (Creator) create_%s);\n",
            SCHEMAget_name(schema),ENT_PREFIX,ENTITYget_name(entity),
            PrettyTmpName (ENTITYget_name(entity)),
            SCHEMA_PREFIX,SCHEMAget_name(schema),
            (ENTITYget_abstract(entity)?"LTrue":"LFalse"),
            ENTITYget_classname (entity)
            );
    */
    /*
        fprintf(files ->inc,"extern EntityDescriptor \t*%s%s%s;\n",
            SCHEMAget_name(schema),ENT_PREFIX,ENTITYget_name(entity));
    */

}

/******************************************************************
 **         TYPE GENERATION             **/


/******************************************************************
 ** Procedure:  TYPEprint_enum
 ** Parameters: const Type type - type to print
 **     FILE*      f    - file on which to print
 ** Returns:
 ** Requires:   TYPEget_class(type) == TYPE_ENUM
 ** Description:  prints code to represent an enumerated type in c++
 ** Side Effects:  prints to header file
 ** Status:  ok 1/15/91
 ** Changes: Modified to check for appropiate key words as described
 **          in "SDAI C++ Binding for PDES, Inc. Prototyping" by
 **          Stephen Clark.
 ** - Changed to match CD2 Part 23, 1/14/97 DAS
 ** Change Date: 5/22/91  CD
 ******************************************************************/
const char *
EnumCElementName( Type type, Expression expr )  {

    static char buf [BUFSIZ];
    sprintf( buf, "%s__",
             EnumName( TYPEget_name( type ) ) );
    strcat( buf, StrToLower( EXPget_name( expr ) ) );

    return buf;
}

char *
CheckEnumSymbol( char * s ) {

    static char b [BUFSIZ];
    if( strcmp( s, "sdaiTRUE" )
            && strcmp( s, "sdaiFALSE" )
            && strcmp( s, "sdaiUNKNOWN" ) ) {
        /*  if the symbol is not a reserved one */
        return ( s );

    } else {
        strcpy( b, s );
        strcat( b, "_" );
        printf( "** warning:  the enumerated value %s is already being used ", s );
        printf( " and has been changed to %s **\n", b );
        return ( b );
    }
}

void
TYPEenum_lib_print( const Type type, FILE * f ) {
    DictionaryEntry de;
    Expression expr;
    //const char * n;   /*  pointer to class name  */
    char c_enum_ele [BUFSIZ];
    if (is_python_keyword(TYPEget_name( type ))) {
        fprintf( f, "# ENUMERATION TYPE %s_\n", TYPEget_name( type ) );
        fprintf(f,"%s_ = ENUMERATION([",TYPEget_name( type ));
    }
    else {
        fprintf( f, "# ENUMERATION TYPE %s\n", TYPEget_name( type ) );
        fprintf(f,"%s = ENUMERATION([",TYPEget_name( type ));
    }
    /*  set up the dictionary info  */

    //fprintf( f, "const char * \n%s::element_at (int n) const  {\n", n );
    //fprintf( f, "  switch (n)  {\n" );
    DICTdo_type_init( ENUM_TYPEget_items( type ), &de, OBJ_ENUM );
    while( 0 != ( expr = ( Expression )DICTdo( &de ) ) ) {
        strncpy( c_enum_ele, EnumCElementName( type, expr ), BUFSIZ );
        if (is_python_keyword(EXPget_name(expr))) {
            fprintf(f,"\n\t'%s_',",EXPget_name(expr));
        }
        else {
            fprintf(f,"\n\t'%s',",EXPget_name(expr));
        }
    }
    fprintf(f,"\n\t])\n");
}


void Type_Description( const Type, char * );

/* return printable version of entire type definition */
/* return it in static buffer */
char *
TypeDescription( const Type t ) {
    static char buf[4000];

    buf[0] = '\0';

    if( TYPEget_head( t ) ) {
        Type_Description( TYPEget_head( t ), buf );
    } else {
        TypeBody_Description( TYPEget_body( t ), buf );
    }

    /* should also print out where clause here */

    return buf + 1;
}

void strcat_expr( Expression e, char * buf ) {
    if( e == LITERAL_INFINITY ) {
        strcat( buf, "?" );
    } else if( e == LITERAL_PI ) {
        strcat( buf, "PI" );
    } else if( e == LITERAL_E ) {
        strcat( buf, "E" );
    } else if( e == LITERAL_ZERO ) {
        strcat( buf, "0" );
    } else if( e == LITERAL_ONE ) {
        strcat( buf, "1" );
    } else if( TYPEget_name( e ) ) {
        strcat( buf, TYPEget_name( e ) );
    } else if( TYPEget_body( e->type )->type == integer_ ) {
        char tmpbuf[30];
        sprintf( tmpbuf, "%d", e->u.integer );
        strcat( buf, tmpbuf );
    } else {
        strcat( buf, "??" );
    }
}

/* print t's bounds to end of buf */
void
strcat_bounds( TypeBody b, char * buf ) {
    if( !b->upper ) {
        return;
    }

    strcat( buf, " [" );
    strcat_expr( b->lower, buf );
    strcat( buf, ":" );
    strcat_expr( b->upper, buf );
    strcat( buf, "]" );
}

void
TypeBody_Description( TypeBody body, char * buf ) {
    char * s;

    switch( body->type ) {
        case integer_:
            strcat( buf, " INTEGER" );
            break;
        case real_:
            strcat( buf, " REAL" );
            break;
        case string_:
            strcat( buf, " STRING" );
            break;
        case binary_:
            strcat( buf, " BINARY" );
            break;
        case boolean_:
            strcat( buf, " BOOLEAN" );
            break;
        case logical_:
            strcat( buf, " LOGICAL" );
            break;
        case number_:
            strcat( buf, " NUMBER" );
            break;
        case entity_:
            strcat( buf, " " );
            strcat( buf, PrettyTmpName( TYPEget_name( body->entity ) ) );
            break;
        case aggregate_:
        case array_:
        case bag_:
        case set_:
        case list_:
            switch( body->type ) {
                    /* ignore the aggregate bounds for now */
                case aggregate_:
                    strcat( buf, " AGGREGATE OF" );
                    break;
                case array_:
                    strcat( buf, " ARRAY" );
                    strcat_bounds( body, buf );
                    strcat( buf, " OF" );
                    if( body->flags.optional ) {
                        strcat( buf, " OPTIONAL" );
                    }
                    if( body->flags.unique ) {
                        strcat( buf, " UNIQUE" );
                    }
                    break;
                case bag_:
                    strcat( buf, " BAG" );
                    strcat_bounds( body, buf );
                    strcat( buf, " OF" );
                    break;
                case set_:
                    strcat( buf, " SET" );
                    strcat_bounds( body, buf );
                    strcat( buf, " OF" );
                    break;
                case list_:
                    strcat( buf, " LIST" );
                    strcat_bounds( body, buf );
                    strcat( buf, " OF" );
                    if( body->flags.unique ) {
                        strcat( buf, " UNIQUE" );
                    }
                    break;
                default:
                    printf( "Error in %s, line %d: type %d not handled by switch statement.", __FILE__, __LINE__, body->type );
                    abort();
            }

            Type_Description( body->base, buf );
            break;
        case enumeration_:
            strcat( buf, " ENUMERATION of (" );
            LISTdo( body->list, e, Expression )
            strcat( buf, ENUMget_name( e ) );
            strcat( buf, ", " );
            LISTod
            /* find last comma and replace with ')' */
            s = strrchr( buf, ',' );
            if( s ) {
                strcpy( s, ")" );
            }
            break;

        case select_:
            strcat( buf, " SELECT (" );
            LISTdo( body->list, t, Type )
            strcat( buf, PrettyTmpName( TYPEget_name( t ) ) );
            strcat( buf, ", " );
            LISTod
            /* find last comma and replace with ')' */
            s = strrchr( buf, ',' );
            if( s ) {
                strcpy( s, ")" );
            }
            break;
        default:
            strcat( buf, " UNKNOWN" );
    }

    if( body->precision ) {
        strcat( buf, " (" );
        strcat_expr( body->precision, buf );
        strcat( buf, ")" );
    }
    if( body->flags.fixed ) {
        strcat( buf, " FIXED" );
    }
}

void
Type_Description( const Type t, char * buf ) {
    if( TYPEget_name( t ) ) {
        strcat( buf, " " );
        strcat( buf, TYPEget_name( t ) );
        /* strcat(buf,PrettyTmpName (TYPEget_name(t)));*/
    } else {
        TypeBody_Description( TYPEget_body( t ), buf );
    }
}

/**************************************************************************
 ** Procedure:  TYPEprint_typedefs
 ** Parameters:  const Type type
 ** Returns:
 ** Description:
 **    Prints in Sdaiclasses.h typedefs, forward declarations, and externs
 **    for user-defined types.  Only a fraction of the typedefs and decla-
 **    rations are needed in Sdaiclasses.h.  Enum's and selects must actu-
 **    ally be defined before objects (such as entities) which use it can
 **    be defined.  So forward declarations will not serve any purpose.
 **    Other redefined types and aggregate types may be declared here.
 ** Side Effects:
 ** Status:  16-Mar-1993 kcm; updated 04-Feb-1997 dar
 **************************************************************************/
void
TYPEprint_typedefs( Type t, FILE * classes ) {
}

/* return 1 if it is a multidimensional aggregate at the level passed in
   otherwise return 0;  If it refers to a type that is a multidimensional
   aggregate 0 is still returned. */
int
isMultiDimAggregateType( const Type t ) {
    if( TYPEget_body( t )->base )
        if( isAggregateType( TYPEget_body( t )->base ) ) {
            return 1;
        }
    return 0;
}

/* Get the TypeDescriptor variable name that t's TypeDescriptor references (if
   possible).
   pass space in through buf, buff will be filled in with the name of the
   TypeDescriptor (TD) that needs to be referenced by the TD that is
   generated for Type t.  Return 1 if buf has been filled in with the name
   of a TD.  Return 0 if it hasn't for these reasons: Enumeration TDs don't
   reference another TD, select TDs reference several TDs and are handled
   separately, Multidimensional aggregates generate unique intermediate TD
   variables that are referenced - when these don't have an Express related
   name this function can't know about them - e.g.
   TYPE listSetAggr = LIST OF SET OF STRING;  This function won't fill in the
   name that listSetAggr's ListTypeDescriptor will reference.
   TYPE arrListSetAggr = ARRAY [1:4] of listSetAggr;  This function will
   return the name of the TD that arrlistSetAggr's ArrayTypeDescriptor should
   reference since it has an Express name associated with it.
*/
int TYPEget_RefTypeVarNm( const Type t, char * buf, Schema schema ) {

    /* It looks like TYPEget_head(t) is true when processing a type
       that refers to another type. e.g. when processing "name" in:
       TYPE name = label; ENDTYPE; TYPE label = STRING; ENDTYPE; DAS */
    if( TYPEget_head( t ) ) {
        /* this means that it is defined in an Express TYPE stmt and
           it refers to another Express TYPE stmt */
        /*  it would be a reference_ type */
        /*  a TypeDescriptor of the form <schema_name>t_<type_name_referred_to> */
        sprintf( buf, "%s%s%s",
                 SCHEMAget_name( TYPEget_head( t )->superscope ),
                 TYPEprefix( t ), TYPEget_name( TYPEget_head( t ) ) );
        return 1;
    } else {
        switch( TYPEget_body( t )->type ) {
            case integer_:
            case real_:
            case boolean_:
            case logical_:
            case string_:
            case binary_:
            case number_:
                /* one of the SCL builtin TypeDescriptors of the form
                   t_STRING_TYPE, or t_REAL_TYPE */
                sprintf( buf, "%s%s", TD_PREFIX, FundamentalType( t, 0 ) );
                return 1;
                break;

            case enumeration_: /* enums don't have a referent type */
            case select_:  /* selects are handled differently elsewhere, they
                refer to several TypeDescriptors */
                return 0;
                break;

            case entity_:
                sprintf( buf, "%s", TYPEtd_name( t ) );
                /* following assumes we are not in a nested entity */
                /* otherwise we should search upward for schema */
                /*       TYPEget_name(TYPEget_body(t)->entity->superscope),
                         ENT_PREFIX,TYPEget_name(t) ); */
                return 1;
                break;

            case aggregate_:
            case array_:
            case bag_:
            case set_:
            case list_:
                /* referent TypeDescriptor will be the one for the element unless it
                   is a multidimensional aggregate then return 0 */

                if( isMultiDimAggregateType( t ) ) {
                    if( TYPEget_name( TYPEget_body( t )->base ) ) {
                        sprintf( buf, "%s%s%s",
                                 SCHEMAget_name( TYPEget_body( t )->base->superscope ),
                                 TYPEprefix( t ), TYPEget_name( TYPEget_body( t )->base ) );
                        return 1;
                    }

                    /* if the multi aggr doesn't have a name then we are out of scope
                       of what this function can do */
                    return 0;
                } else {
                    /* for a single dimensional aggregate return TypeDescriptor
                       for element */
                    /* being an aggregate implies that base below is not 0 */

                    if( TYPEget_body( TYPEget_body( t )->base )->type == enumeration_ ||
                            TYPEget_body( TYPEget_body( t )->base )->type == select_ ) {

                        sprintf( buf, "%s", TYPEtd_name( TYPEget_body( t )->base ) );
                        return 1;
                    } else if( TYPEget_name( TYPEget_body( t )->base ) ) {
                        if( TYPEget_body( TYPEget_body( t )->base )->type == entity_ ) {
                            sprintf( buf, "%s", TYPEtd_name( TYPEget_body( t )->base ) );
                            return 1;
                        }
                        sprintf( buf, "%s%s%s",
                                 SCHEMAget_name( TYPEget_body( t )->base->superscope ),
                                 TYPEprefix( t ), TYPEget_name( TYPEget_body( t )->base ) );
                        return 1;
                    }
                    return TYPEget_RefTypeVarNm( TYPEget_body( t )->base, buf, schema );
                }
                break;
            default:
                return 0;
        }
    }
    /*  return 0; // this stmt will never be reached */
}


/*****
   print stuff for types that are declared in Express TYPE statements... i.e.
   extern descriptor declaration in .h file - MOVED BY DAR to TYPEprint_type-
       defs - in order to print all the Sdaiclasses.h stuff in fedex_plus's
       first pass through each schema.
   descriptor definition in the .cc file
   initialize it in the .init.cc file (DAR - all initialization done in fn
       TYPEprint_init() (below) which is done in fedex_plus's 1st pass only.)
*****/

void
TYPEprint_descriptions( const Type type, FILES * files, Schema schema ) {
    char tdnm [BUFSIZ],
         typename_buf [MAX_LEN],
         base [BUFSIZ],
         nm [BUFSIZ];
    Type i;

    strncpy( tdnm, TYPEtd_name( type ), BUFSIZ );

    /* define type descriptor pointer */
    /*  put extern def in header, put the real definition in .cc file  */

    /*  put extern def in header (DAR - moved to TYPEprint_typedef's -
     *  see fn header comments.)*/
    /*
        fprintf(files->inc,"extern %s \t*%s;\n",
            GetTypeDescriptorName(type), tdnm);
    */

    /*  in source - declare the real definition of the pointer */
    /*  i.e. in the .cc file                                   */
    if( TYPEis_enumeration( type ) && ( i = TYPEget_ancestor( type ) ) != NULL ) {
        /* If we're a renamed enum type, just print a few typedef's to the
        // original and some specialized create functions: */
        strncpy( base, StrToLower(EnumName( TYPEget_name( i ) )), BUFSIZ );
        strncpy( nm, StrToLower(EnumName( TYPEget_name( type ) )), BUFSIZ );
        fprintf( files->lib, "%s = %s\n",nm,base );
        return;
    }

    if( TYPEget_RefTypeVarNm( type, typename_buf, schema ) ) 
    {
        fprintf(files->lib, "%s = ",TYPEget_name(type));
        char * output = FundamentalType(type,0);
        if (!strcmp(output,"ARRAY_TYPE")) {
            process_aggregate(files->lib,type);
            fprintf(files->lib,"\n");
            }
        else {
            fprintf(files->lib,"%s\n",output);
        }
        }
    else {
        switch( TYPEget_body( type )->type ) {
            case enumeration_:
                TYPEenum_lib_print( type, files -> lib );
                break;

            case select_:
                //TYPEselect_lib_print (type, files -> lib);
                break;
            default:
                break;
        }
    }
}


static void
printEnumCreateHdr( FILE * inc, const Type type )
/*
 * Prints a bunch of lines for enumeration creation functions (i.e., "cre-
 * ate_SdaiEnum1()").  Since this is done both for an enum and for "copies"
 * of it (when "TYPE enum2 = enum1"), I placed this code in a separate fn.
 */
{

}

static void
printEnumCreateBody( FILE * lib, const Type type )
/*
 * See header comment above by printEnumCreateHdr.
 */
{
}

static void
printEnumAggrCrHdr( FILE * inc, const Type type )
/*
 * Similar to printEnumCreateHdr above for the enum aggregate.
 */
{
}

static void
printEnumAggrCrBody( FILE * lib, const Type type ) {
}

void
TYPEprint_init( const Type type, FILE * ifile, Schema schema ) {
    char tdnm [BUFSIZ];
    char typename_buf[MAX_LEN];

    strncpy( tdnm, TYPEtd_name( type ), BUFSIZ );

    if( isAggregateType( type ) ) {
        if( !TYPEget_head( type ) ) {
            if( TYPEget_body( type )->lower )
                fprintf( ifile, "\t%s->Bound1(%d);\n", tdnm,
                         TYPEget_body( type )->lower->u.integer );
            if( TYPEget_body( type )->upper )
                fprintf( ifile, "\t%s->Bound2(%d);\n", tdnm,
                         TYPEget_body( type )->upper->u.integer );
            if( TYPEget_body( type )->flags.unique ) {
                fprintf( ifile, "\t%s->UniqueElements(\"LTrue\");\n", tdnm );
            }
            /*    fprintf(ifile, "\t%s->UniqueElements(%d);\n", tdnm,
                      TYPEget_body(type)->flags.unique); */
            if( TYPEget_body( type )->flags.optional ) {
                fprintf( ifile, "\t%s->OptionalElements(\"LTrue\");\n", tdnm );
            }
            /*    fprintf(ifile, "\t%s->OptionalElements(%d);\n", tdnm,
                      TYPEget_body(type)->flags.optional);*/
        }
    }

    /* fill in the TD's values in the SchemaInit function (it is already
    declared with basic values) */

    if( TYPEget_RefTypeVarNm( type, typename_buf, schema ) ) {
        fprintf( ifile, "\t%s->ReferentType(%s);\n", tdnm, typename_buf );
    } else {
        switch( TYPEget_body( type )->type ) {
            case aggregate_: /* aggregate_ should not happen? DAS */
            case array_:
            case bag_:
            case set_:
            case list_: {

                if( isMultiDimAggregateType( type ) ) {
                    print_typechain( ifile, TYPEget_body( type )->base,
                                     typename_buf, schema );
                    fprintf( ifile, "	%s->ReferentType(%s);\n", tdnm,
                             typename_buf );
                }
                break;
            }
            default:
                break;
        }
    }

    /* DAR - moved fn call below from TYPEselect_print to here to put all init
    ** info together. */
    if( TYPEis_select( type ) ) {
        TYPEselect_init_print( type, ifile, schema );
    }
#ifdef NEWDICT
    /* DAS New SDAI Dictionary 5/95 */
    /* insert the type into the schema descriptor */
    fprintf( ifile,
             "\t((SDAIAGGRH(Set,DefinedTypeH))%s%s->Types())->Add((DefinedTypeH)%s);\n",
             SCHEMA_PREFIX, SCHEMAget_name( schema ), tdnm );
#endif
    /* insert into type dictionary */
    fprintf( ifile, "\treg.AddType (*%s);\n", tdnm );
}

/* print name, fundamental type, and description initialization function
   calls */

void
TYPEprint_nm_ft_desc( Schema schema, const Type type, FILE * f, char * endChars ) {

    fprintf( f, "\t\t  \"%s\",\t// Name\n",
             PrettyTmpName( TYPEget_name( type ) ) );
    fprintf( f, "\t\t  %s,\t// FundamentalType\n",
             FundamentalType( type, 1 ) );
    fprintf( f, "\t\t  %s%s,\t// Originating Schema\n",
             SCHEMA_PREFIX, SCHEMAget_name( schema ) );
    fprintf( f, "\t\t  \"%s\"%s\t// Description\n",
             TypeDescription( type ), endChars );
}


/* new space for a variable of type TypeDescriptor (or subtype).  This
   function is called for Types that have an Express name. */

void
TYPEprint_new( const Type type, FILE * create, Schema schema ) {
}
