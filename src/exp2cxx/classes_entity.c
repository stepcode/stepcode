/** *****************************************************************
** \file classes_entity.c
** FedEx parser output module for generating C++  class definitions
**
** Development of FedEx was funded by the United States Government,
** and is not subject to copyright.

*******************************************************************
The conventions used in this binding follow the proposed specification
for the STEP Standard Data Access Interface as defined in document
N350 ( August 31, 1993 ) of ISO 10303 TC184/SC4/WG7.
*******************************************************************/

/* this is used to add new dictionary calls */
/* #define NEWDICT */

#include <sc_memmgr.h>
#include <stdlib.h>
#include <assert.h>
#include <sc_mkdir.h>
#include "classes.h"
#include "class_strings.h"
#include "genCxxFilenames.h"
#include <ordered_attrs.h>

#include <sc_trace_fprintf.h>

extern int multiple_inheritance;
extern int old_accessors;

static int attr_count;  /**< number each attr to avoid inter-entity clashes
                            several classes use attr_count for naming attr dictionary entry
                            variables.  All but the last function generating code for a particular
                            entity increment a copy of it for naming each attr in the entity.
                            Here are the functions:
                            ENTITYhead_print (Entity entity, FILE* file,Schema schema)
                            LIBdescribe_entity (Entity entity, FILE* file, Schema schema)
                            LIBcopy_constructor (Entity ent, FILE* file)
                            LIBstructor_print (Entity entity, FILE* file, Schema schema)
                            LIBstructor_print_w_args (Entity entity, FILE* file, Schema schema)
                            ENTITYincode_print(Entity entity, FILE* file, Schema schema)
                            DAS
                        */

/******************************************************************
**      Entity Generation                */

/**
 * print entity descriptors and attrdescriptors to the namespace in files->names
 * hopefully this file can be safely included everywhere, eliminating use of 'extern'
 *
 * Nov 2011 - MAP - This function was split out of ENTITYhead_print to enable
 *                  use of a separate header with a namespace.
 */
void ENTITYnames_print( Entity entity, FILE * file ) {
    char attrnm [BUFSIZ];
    int attr_count_tmp = attr_count;

    fprintf( file, "    extern EntityDescriptor *%s%s;\n", ENT_PREFIX, ENTITYget_name( entity ) );

    /* DAS print all the attr descriptors and inverse attr descriptors for an
     *       entity as defs in the .h file. */
    LISTdo( ENTITYget_attributes( entity ), v, Variable )
    generate_attribute_name( v, attrnm );
    fprintf( file, "    extern %s *%s%d%s%s;\n",
             ( VARget_inverse( v ) ? "Inverse_attribute" : ( VARis_derived( v ) ? "Derived_attribute" : "AttrDescriptor" ) ),
             ATTR_PREFIX, attr_count_tmp++,
             ( VARis_derived( v ) ? "D" : ( VARis_type_shifter( v ) ? "R" : ( VARget_inverse( v ) ? "I" : "" ) ) ),
             attrnm );
    LISTod;
}

/**************************************************************//**
 ** Procedure:  DataMemberPrintAttr
 ** Parameters:  Entity entity  --  entity being processed
 **              Variable a -- attribute being processed
 **              FILE* file  --  file being written to
 ** Returns:
 ** Description:  prints out the current attribute for an entity's c++ class
 **               definition
 ******************************************************************/
void DataMemberPrintAttr( Entity entity, Variable a, FILE * file ) {
    char attrnm [BUFSIZ];
    const char * ctype, * etype;
    if( VARget_initializer( a ) == EXPRESSION_NULL ) {
        ctype = TYPEget_ctype( VARget_type( a ) );
        generate_attribute_name( a, attrnm );
        if( !strcmp( ctype, "SCLundefined" ) ) {
            printf( "WARNING:  in entity %s, ", ENTITYget_name( entity ) );
            printf( " the type for attribute  %s is not fully implemented\n", attrnm );
        }
        if( TYPEis_entity( VARget_type( a ) ) ) {
            fprintf( file, "        SDAI_Application_instance_ptr _%s;", attrnm );
        } else if( TYPEis_aggregate( VARget_type( a ) ) ) {
            fprintf( file, "        %s_ptr _%s;", ctype, attrnm );
        } else {
            fprintf( file, "        %s _%s;", ctype, attrnm );
        }
        if( VARget_optional( a ) ) {
            fprintf( file, "    //  OPTIONAL" );
        }
        if( isAggregate( a ) )        {
            /*  if it's a named type, comment the type  */
            if( ( etype = TYPEget_name
                          ( TYPEget_nonaggregate_base_type( VARget_type( a ) ) ) ) ) {
                fprintf( file, "          //  of  %s\n", etype );
            }
        }

        fprintf( file, "\n" );
    }
}

/**************************************************************//**
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
void LIBdescribe_entity( Entity entity, FILE * file, Schema schema ) {
    int attr_count_tmp = attr_count;
    char attrnm [BUFSIZ];

    fprintf( file, "EntityDescriptor * %s::%s%s = 0;\n", SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

    LISTdo( ENTITYget_attributes( entity ), v, Variable )
    generate_attribute_name( v, attrnm );
    fprintf( file, "%s * %s::%s%d%s%s = 0;\n",
             ( VARget_inverse( v ) ? "Inverse_attribute" : ( VARis_derived( v ) ? "Derived_attribute" : "AttrDescriptor" ) ),
             SCHEMAget_name( schema ), ATTR_PREFIX, attr_count_tmp++,
             ( VARis_derived( v ) ? "D" : ( VARis_type_shifter( v ) ? "R" : ( VARget_inverse( v ) ? "I" : "" ) ) ), attrnm );
    LISTod

    fprintf( file, "\n");
}

/**************************************************************//**
 ** Procedure:  LIBmemberFunctionPrint
 ** Parameters:  Entity *entity --  entity being processed
 **     FILE* file  --  file being written to
 ** Returns:
 ** Description:  prints the member functions for the class
                  representing an entity.  These go in the .cc file
 ** Side Effects:  prints c++ code to a file
 ** Status:  ok 17-Feb-1992
 ******************************************************************/
void LIBmemberFunctionPrint( Entity entity, Linked_List neededAttr, FILE * file ) {

    Linked_List attr_list;
    char entnm [BUFSIZ];

    strncpy( entnm, ENTITYget_classname( entity ), BUFSIZ ); /*  assign entnm */

    /*  1. put in member functions which belong to all entities */
    /*  the common function are still in the class definition 17-Feb-1992 */

    /*  2. print access functions for attributes    */
    attr_list = ENTITYget_attributes( entity );
    LISTdo( attr_list, a, Variable )
    /*  do for EXPLICIT, REDEFINED, and INVERSE attributes - but not DERIVED */
    if( ! VARis_derived( a ) )  {

        /*  retrieval  and  assignment   */
        ATTRprint_access_methods( entnm, a, file );
    }
    LISTod;
    /* //////////////// */
    if( multiple_inheritance ) {
        LISTdo( neededAttr, attr, Variable ) {
            if( ! VARis_derived( attr ) && ! VARis_overrider( entity, attr ) ) {
                ATTRprint_access_methods( entnm, attr, file );
            }
        }
        LISTod;
    }
    /* //////////////// */
    
    fprintf( file, "\n" );
}

int get_attribute_number( Entity entity ) {
    int i = 0;
    int found = 0;
    Linked_List local, complete;
    complete = ENTITYget_all_attributes( entity );
    local = ENTITYget_attributes( entity );

    LISTdo( local, a, Variable ) {
        /*  go to the child's first explicit attribute */
        if( ( ! VARget_inverse( a ) ) && ( ! VARis_derived( a ) ) )  {
                LISTdo_n( complete, p, Variable, b ) {
                /*  cycle through all the explicit attributes until the
                child's attribute is found  */
                if( !found && ( ! VARget_inverse( p ) ) && ( ! VARis_derived( p ) ) ) {
                    if( p != a ) {
                        ++i;
                    } else {
                        found = 1;
                    }
                }
            } LISTod;
            if( found ) {
                return i;
            } else printf( "Internal error:  %s:%d\n"
                            "Attribute %s not found.\n"
                            , __FILE__, __LINE__, EXPget_name( VARget_name( a ) ) );
        }
    } LISTod;
    return -1;
}

/**************************************************************//**
 ** Procedure:  ENTITYhead_print
 ** Parameters:  const Entity entity
 **   FILE* file  --  file being written to
 ** Returns:
 ** Description:  prints the beginning of the entity class definition for the
 **               c++ code and the declaration of attr descriptors for
 **       the registry.  In the .h file
 ** Side Effects:  generates c++ code
 ** Status:  good 1/15/91
 **          added registry things 12-Apr-1993
 **          remove extern keyword - MAP - Nov 2011
 **          split out stuff in namespace to ENTITYdesc_print - MAP - Nov 2011
 ******************************************************************/
void ENTITYhead_print( Entity entity, FILE * file ) {
    char entnm [BUFSIZ];
    Linked_List list;
    Entity super = 0;

    strncpy( entnm, ENTITYget_classname( entity ), BUFSIZ );
    entnm[BUFSIZ-1] = '\0';

    /* inherit from either supertype entity class or root class of
       all - i.e. SDAI_Application_instance */

    if( multiple_inheritance ) {
        list = ENTITYget_supertypes( entity );
        if( ! LISTempty( list ) ) {
            super = ( Entity )LISTpeek_first( list );
        }
    } else { /* the old way */
        super = ENTITYput_superclass( entity );
    }

    fprintf( file, "class SC_SCHEMA_EXPORT %s : ", entnm );
    if( super ) {
        fprintf( file, "public %s {\n ", ENTITYget_classname( super ) );
    } else {
        fprintf( file, "public SDAI_Application_instance {\n" );
    }
}

/**************************************************************//**
** Procedure:  DataMemberPrint
** Parameters:  const Entity entity  --  entity being processed
**   FILE* file  --  file being written to
** Returns:
** Description:  prints out the data members for an entity's c++ class
**               definition
** Side Effects:  generates c++ code
** Status:  ok 1/15/91
******************************************************************/
void DataMemberPrint( Entity entity, Linked_List neededAttr, FILE * file ) {
    Linked_List attr_list;
    char entnm [BUFSIZ];
    strncpy( entnm, ENTITYget_classname( entity ), BUFSIZ ); /*  assign entnm  */

    /*  print list of attributes in the protected access area   */
    fprintf( file, "    protected:\n" );

    attr_list = ENTITYget_attributes( entity );
    LISTdo( attr_list, attr, Variable ) {
        DataMemberPrintAttr( entity, attr, file );
    }
    LISTod;

    /*  add attributes for parent attributes not inherited through C++ inheritance. */
    if( multiple_inheritance ) {
        LISTdo( neededAttr, attr, Variable ) {
            DataMemberPrintAttr( entity, attr, file );
        }
        LISTod;
    }
}

/**************************************************************//**
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
void MemberFunctionSign( Entity entity, Linked_List neededAttr, FILE * file ) {

    Linked_List attr_list;
    static int entcode = 0;
    char entnm [BUFSIZ];

    strncpy( entnm, ENTITYget_classname( entity ), BUFSIZ ); /*  assign entnm  */
    entnm[BUFSIZ-1] = '\0';

    fprintf( file, "    public: \n" );

    /*  put in member functions which belong to all entities    */
    /*  constructors:    */
    fprintf( file, "        %s();\n", entnm );
    fprintf( file, "        %s( SDAI_Application_instance *se, bool addAttrs = true );\n", entnm );
    /*  copy constructor */
    fprintf( file, "        %s( %s & e );\n", entnm, entnm );
    /*  destructor: */
    fprintf( file, "        ~%s();\n", entnm );

    fprintf( file, "        int opcode() {\n            return %d;\n        }\n", entcode++ );

    /*  print signature of access functions for attributes      */
    attr_list = ENTITYget_attributes( entity );
    LISTdo( attr_list, a, Variable ) {
        if( VARget_initializer( a ) == EXPRESSION_NULL ) {

            /*  retrieval  and  assignment  */
            ATTRsign_access_methods( a, file );
        }
    }
    LISTod;

    /* //////////////// */
    if( multiple_inheritance ) {
        /*  add the EXPRESS inherited attributes which are non */
        /*  inherited in C++ */
        LISTdo( neededAttr, attr, Variable ) {
            if( ! VARis_derived( attr ) && ! VARis_overrider( entity, attr ) ) {
                ATTRsign_access_methods( attr, file );
            }
        }
        LISTod;

    }
    /* //////////////// */
    fprintf( file, "};\n\n" );

    /*  print creation function for class */
    fprintf( file, "inline %s * create_%s() {\n    return new %s;\n}\n\n", entnm, entnm, entnm );
}

/**************************************************************//**
 ** Procedure:  ENTITYinc_print
 ** Parameters:  Entity *entity --  entity being processed
 **     FILE* file  --  file being written to
 ** Returns:
 ** Description:  drives the generation of the c++ class definition code
 ** Side Effects:  prints segment of the c++ .h file
 ** Status:  ok 1/15/91
 ******************************************************************/
void ENTITYinc_print( Entity entity, Linked_List neededAttr, FILE * file ) {
    ENTITYhead_print( entity, file );
    DataMemberPrint( entity, neededAttr, file );
    MemberFunctionSign( entity, neededAttr, file );
}

/**************************************************************//**
 ** Procedure:  LIBcopy_constructor
 ** Parameters:
 ** Returns:
 ** Description:
 ** Side Effects:
 ** Status:  not used 17-Feb-1992
 ******************************************************************/
void LIBcopy_constructor( Entity ent, FILE * file ) {
    Linked_List attr_list;
    Class_Of_Type class;
    Type t;
    char buffer [BUFSIZ],
         attrnm[BUFSIZ],
         *b = buffer;
    int count = attr_count;

    const char * entnm = ENTITYget_classname( ent );
    const char * StrToLower( const char * word );

    /*mjm7/10/91 copy constructor definition  */
    fprintf( file, "        %s::%s(%s& e )\n", entnm, entnm, entnm );
    fprintf( file, "  {" );

    /*  attributes  */
    attr_list = ENTITYget_attributes( ent );
    LISTdo( attr_list, a, Variable )
    if( VARget_initializer( a ) == EXPRESSION_NULL ) {
        /*  include attribute if it is not derived  */
        generate_attribute_name( a, attrnm );
        t = VARget_type( a );
        class = TYPEget_type( t );

        /*  1. initialize everything to NULL (even if not optional)  */

        /*    default:  to intialize attribute to NULL  */
        sprintf( b, "        _%s = e.%s();\n", attrnm, attrnm );

        /*mjm7/11/91  case TYPE_STRING */
        if( ( class == string_ ) || ( class == binary_ ) ) {
            sprintf( b, "        _%s = strdup(e.%s());\n", attrnm, attrnm );
        }


        /*      case TYPE_ENTITY:   */
        if( class == entity_ ) {
            sprintf( b, "        _%s = e.%s();\n", attrnm, attrnm );
        }
        /* previous line modified to conform with SDAI C++ Binding for PDES, Inc. Prototyping 5/22/91 CD */

        /*    case TYPE_ENUM:   */
        if( class == enumeration_ ) {
            sprintf( b, "        _%s.put(e.%s().asInt());\n", attrnm, attrnm );
        }
        /*    case TYPE_SELECT: */
        if( class == select_ ) {
            sprintf( b, "DDDDDDD        _%s.put(e.%s().asInt());\n", attrnm, attrnm );
        }
        /*   case TYPE_BOOLEAN    */
        if( class == boolean_ ) {
            sprintf( b, "        _%s.put(e.%s().asInt());\n", attrnm, attrnm );
        }
        /* previous line modified to conform with SDAI C++ Binding for PDES, Inc. Prototyping 5/22/91 CD */

        /*   case TYPE_LOGICAL    */
        if( class == logical_ ) {
            sprintf( b, "        _%s.put(e.%s().asInt());\n", attrnm, attrnm );
        }
        /* previous line modified to conform with SDAI C++ Binding for PDES, Inc. Prototyping 5/22/91 CD */

        /*  case TYPE_ARRAY:
        case TYPE_LIST:
          case TYPE_SET:
          case TYPE_BAG:  */
        if( isAggregateType( t ) ) {
            *b = '\0';
        }

        fprintf( file, "%s", b )       ;

        fprintf( file, "         attributes.push " );

        /*  2.  put attribute on attributes list    */

        /*  default:    */

        fprintf( file, "\n        (new STEPattribute(*%s%d%s, %s &_%s));\n",
                 ATTR_PREFIX, count,
                 attrnm,
                 ( TYPEis_entity( t ) ? "(SDAI_Application_instance_ptr *)" : "" ),
                 attrnm );
        ++count;

    }
    LISTod;
    fprintf( file, " }\n" );
}

/** initialize attributes in the constructor; used for two different constructors */
void initializeAttrs( Entity e, FILE* file ) {
    const orderedAttr * oa;
    orderedAttrsInit( e );
    while( 0 != ( oa = nextAttr() ) ) {
        if( oa->deriver ) {
            fprintf( file, "    MakeDerived( \"%s\", \"%s\" );\n", oa->attr->name->symbol.name, oa->creator->symbol.name );
        }
    }
    orderedAttrsCleanup();
}

/**************************************************************//**
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
void LIBstructor_print( Entity entity, FILE * file, Schema schema ) {
    Linked_List attr_list;
    Type t;
    char attrnm [BUFSIZ];

    Linked_List list;
    Entity principalSuper = 0;

    const char * entnm = ENTITYget_classname( entity );
    int count = attr_count;
    bool first = true;

    /*  constructor definition  */

    /* parent class initializer (if any) and '{' printed below */
    fprintf( file, "%s::%s()", entnm, entnm );

    /* ////MULTIPLE INHERITANCE//////// */

    if( multiple_inheritance ) {
        int super_cnt = 0;
        list = ENTITYget_supertypes( entity );
        if( ! LISTempty( list ) ) {
            LISTdo( list, e, Entity ) {
                /*  if there's no super class yet,
                    or the super class doesn't have any attributes
                */

                super_cnt++;
                if( super_cnt == 1 ) {
                    /* ignore the 1st parent */
                    const char * parent = ENTITYget_classname( e );

                    /* parent class initializer */
                    fprintf( file, ": %s() {\n", parent );
                    fprintf( file, "        /*  parent: %s  */\n%s\n%s\n", parent,
                            "        /* Ignore the first parent since it is */",
                            "        /* part of the main inheritance hierarchy */"  );
                    principalSuper = e; /* principal SUPERTYPE */
                } else {
                    fprintf( file, "        /*  parent: %s  */\n", ENTITYget_classname( e ) );
                    fprintf( file, "    HeadEntity(this);\n" );
                    fprintf( file, "    AppendMultInstance(new %s(this));\n",
                            ENTITYget_classname( e ) );

                    if( super_cnt == 2 ) {
                        printf( "\nMULTIPLE INHERITANCE for entity: %s\n",
                                ENTITYget_name( entity ) );
                        printf( "        SUPERTYPE 1: %s (principal supertype)\n",
                                ENTITYget_name( principalSuper ) );
                    }
                    printf( "        SUPERTYPE %d: %s\n", super_cnt, ENTITYget_name( e ) );
                }
            } LISTod;

        } else {    /*  if entity has no supertypes, it's at top of hierarchy  */
            /*  no parent class constructor has been printed, so still need an opening brace */
            fprintf( file, " {\n" );
            fprintf( file, "        /*  no SuperTypes */\n" );
        }
    }

    /* what if entity comes from other schema?
     * It appears that entity.superscope.symbol.name is the schema name (but only if entity.superscope.type == 's'?)  --MAP 27Nov11
     */
    fprintf( file, "\n    eDesc = %s::%s%s;\n",
             SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

    attr_list = ENTITYget_attributes( entity );

    LISTdo( attr_list, a, Variable )
    if( VARget_initializer( a ) == EXPRESSION_NULL ) {
        /*  include attribute if it is not derived  */
        generate_attribute_name( a, attrnm );
        t = VARget_type( a );

        if( ( ! VARget_inverse( a ) ) && ( ! VARis_derived( a ) ) )  {
            /*  1. create a new STEPattribute */

            /*  if type is aggregate, the variable is a pointer and needs initialized */
            if( TYPEis_aggregate( t ) ) {
                fprintf( file, "    _%s = new %s;\n", attrnm, TYPEget_ctype( t ) );
            }
            fprintf( file, "    %sa = new STEPattribute( * %s::%s%d%s%s, %s %s_%s );\n",
                     ( first ? "STEPattribute * " : "" ), /*   first time through, declare 'a' */
                     SCHEMAget_name( schema ),
                     ATTR_PREFIX, count,
                     ( VARis_type_shifter( a ) ? "R" : "" ),
                     attrnm,
                     ( TYPEis_entity( t ) ? "( SDAI_Application_instance_ptr * )" : "" ),
                     ( TYPEis_aggregate( t ) ? "" : "& " ),
                     attrnm );
            if( first ) {
                first = false;
            }
            /*  2. initialize everything to NULL (even if not optional)  */

            fprintf( file, "    a->set_null();\n" );

            /*  3.  put attribute on attributes list  */
            fprintf( file, "    attributes.push( a );\n" );

            /* if it is redefining another attribute make connection of
               redefined attribute to redefining attribute */
            if( VARis_type_shifter( a ) ) {
                fprintf( file, "    MakeRedefined( a, \"%s\" );\n",
                         VARget_simple_name( a ) );
            }
        }
        count++;
    }

    LISTod;

    initializeAttrs( entity, file );

    fprintf( file, "}\n\n" );

    /*  copy constructor  */
    /*  LIBcopy_constructor (entity, file); */
    entnm = ENTITYget_classname( entity );
    fprintf( file, "%s::%s ( %s & e ) : ", entnm, entnm, entnm );

    /* include explicit initialization of base class */
    if( principalSuper ) {
        fprintf( file, "%s()", ENTITYget_classname( principalSuper ) );
    } else {
        fprintf( file, "SDAI_Application_instance()" );
    }

    fprintf( file, " {\n    CopyAs( ( SDAI_Application_instance_ptr ) & e );\n}\n\n" );

    /*  print destructor  */
    /*  currently empty, but should check to see if any attributes need
    to be deleted -- attributes will need reference count  */

    entnm = ENTITYget_classname( entity );
    fprintf( file, "%s::~%s() {\n", entnm, entnm );

    attr_list = ENTITYget_attributes( entity );

    LISTdo( attr_list, a, Variable )
    if( VARget_initializer( a ) == EXPRESSION_NULL ) {
        generate_attribute_name( a, attrnm );
        t = VARget_type( a );

        if( ( ! VARget_inverse( a ) ) && ( ! VARis_derived( a ) ) )  {
            if( TYPEis_aggregate( t ) ) {
                fprintf( file, "    delete _%s;\n", attrnm );
            }
        }
    }
    LISTod;

    fprintf( file, "}\n\n" );
}

/********************/
/** print the constructor that accepts a SDAI_Application_instance as an argument used
   when building multiply inherited entities.
   \sa LIBstructor_print()
*/
void LIBstructor_print_w_args( Entity entity, FILE * file, Schema schema ) {
    Linked_List attr_list;
    Type t;
    char attrnm [BUFSIZ];

    Linked_List list;
    int super_cnt = 0;

    /* added for calling parents constructor if there is one */
    char parentnm [BUFSIZ];
    char * parent = 0;

    const char * entnm;
    int count = attr_count;
    bool first = true;

    if( multiple_inheritance ) {
        Entity parentEntity = 0;
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
            fprintf( file, "%s::%s( SDAI_Application_instance * se, bool addAttrs ) : %s( se, addAttrs ) {\n", entnm, entnm, parentnm );
        else {
            fprintf( file, "%s::%s( SDAI_Application_instance * se, bool addAttrs ) {\n", entnm, entnm );
        }

        fprintf( file, "    /* Set this to point to the head entity. */\n" );
        fprintf( file, "    HeadEntity(se);\n" );
        if( !parent ) {
            fprintf( file, "    ( void ) addAttrs; /* quell potentially unused var */\n\n" );
        }

        list = ENTITYget_supertypes( entity );
        if( ! LISTempty( list ) ) {
            LISTdo( list, e, Entity )
            /*  if there's no super class yet,
                or the super class doesn't have any attributes
                */
            fprintf( file, "        /* parent: %s */\n", ENTITYget_classname( e ) );

            super_cnt++;
            if( super_cnt == 1 ) {
                /* ignore the 1st parent */
                fprintf( file,
                         "        /* Ignore the first parent since it is part *\n%s\n",
                         "        ** of the main inheritance hierarchy        */" );
            }  else {
                fprintf( file, "    se->AppendMultInstance( new %s( se, addAttrs ) );\n",
                         ENTITYget_classname( e ) );
            }
            LISTod;

        }  else {   /*  if entity has no supertypes, it's at top of hierarchy  */
            fprintf( file, "        /*  no SuperTypes */\n" );
        }

        /* what if entity comes from other schema? */
        fprintf( file, "\n    eDesc = %s::%s%s;\n",
                 SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

        attr_list = ENTITYget_attributes( entity );

        LISTdo( attr_list, a, Variable )
        if( VARget_initializer( a ) == EXPRESSION_NULL ) {
            /*  include attribute if it is not derived  */
            generate_attribute_name( a, attrnm );
            t = VARget_type( a );

            if( ( ! VARget_inverse( a ) ) && ( ! VARis_derived( a ) ) )  {
                /*  1. create a new STEPattribute */

                /*  if type is aggregate, the variable is a pointer and needs initialized */
                if( TYPEis_aggregate( t ) ) {
                    fprintf( file, "    _%s = new %s;\n", attrnm, TYPEget_ctype( t ) );
                }
                fprintf( file, "    %sa = new STEPattribute( * %s::%s%d%s%s, %s %s_%s );\n",
                         ( first ? "STEPattribute * " : "" ), /*   first time through, declare a */
                         SCHEMAget_name( schema ),
                         ATTR_PREFIX, count,
                         ( VARis_type_shifter( a ) ? "R" : "" ),
                         attrnm,
                         ( TYPEis_entity( t ) ? "( SDAI_Application_instance_ptr * )" : "" ),
                         ( TYPEis_aggregate( t ) ? "" : "&" ),
                         attrnm );

                if( first ) {
                    first = false;
                }

                fprintf( file, "        /* initialize to NULL (even if not optional)  */\n" );
                fprintf( file, "    a -> set_null();\n" );

                fprintf( file, "        /* Put attribute on this class' attributes list so the access functions still work. */\n" );
                fprintf( file, "    attributes.push( a );\n" );

                fprintf( file, "        /* Put attribute on the attributes list for the main inheritance heirarchy.  **\n" );
                fprintf( file, "        ** The push method rejects duplicates found by comparing attrDescriptor's.   */\n" );
                fprintf( file, "    if( addAttrs ) {\n" );
                fprintf( file, "        se->attributes.push( a );\n    }\n" );

                /* if it is redefining another attribute make connection of redefined attribute to redefining attribute */
                if( VARis_type_shifter( a ) ) {
                    fprintf( file, "    MakeRedefined( a, \"%s\" );\n",
                             VARget_simple_name( a ) );
                }
            }
            count++;
        }

        LISTod;

        initializeAttrs( entity, file );

        fprintf( file, "}\n\n" );
    } /* end if(multiple_inheritance) */

}

/** return 1 if types are predefined by us */
bool TYPEis_builtin( const Type t ) {
    switch( TYPEget_body( t )->type ) { /* dunno if correct*/
        case integer_:
        case real_:
        case string_:
        case binary_:
        case boolean_:
        case number_:
        case logical_:
            return true;
        default:
            break;
    }
    return false;
}

/**************************************************************//**
 ** \fn  generate_dict_attr_name
 ** \param a, an Express attribute
 ** \param out, the C++ name
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
char * generate_dict_attr_name( Variable a, char * out ) {
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
    for( j = 0, q = out; *p != '\0' && j < BUFSIZ; p++ ) {
        /* copy p to out, 1 char at time.  Skip \n's, and convert to lc. */
        if( *p != '\n' ) {
            *q = tolower( *p );
            j++;
            q++;
        }
    }
    *q = '\0';

    sc_free( temp );
    return out;
}

/**************************************************************//**
 ** Procedure:  ENTITYincode_print
 ** Parameters:  Entity *entity --  entity being processed
 **     FILE* file  --  file being written to
 ** Returns:
 ** Description:  generates code to enter entity in STEP registry
 **      This goes to the .init.cc file
 ** Side Effects:
 ** Status:  ok 1/15/91
 ******************************************************************/
void ENTITYincode_print( Entity entity, FILE * header, FILE * impl, Schema schema ) {
#define entity_name ENTITYget_name(entity)
#define schema_name SCHEMAget_name(schema)
    char attrnm [BUFSIZ];
    char dict_attrnm [BUFSIZ];
    const char * super_schema;
    char * tmp, *tmp2;

#ifdef NEWDICT
    /* DAS New SDAI Dictionary 5/95 */
    /* insert the entity into the schema descriptor */
    fprintf( impl,
             "        ((SDAIAGGRH(Set,EntityH))%s::schema->Entities())->Add(%s::%s%s);\n",
             schema_name, schema_name, ENT_PREFIX, entity_name );
#endif

    if( ENTITYget_abstract( entity ) ) {
        if( entity->u.entity->subtype_expression ) {

            fprintf( impl, "        str.clear();\n        str.append( \"ABSTRACT SUPERTYPE OF ( \" );\n" );

            format_for_std_stringout( impl, SUBTYPEto_string( entity->u.entity->subtype_expression ) );
            fprintf( impl, "\n      str.append( \")\" );\n" );
            fprintf( impl, "        %s::%s%s->AddSupertype_Stmt( str );", schema_name, ENT_PREFIX, entity_name );
        } else {
            fprintf( impl, "        %s::%s%s->AddSupertype_Stmt( \"ABSTRACT SUPERTYPE\" );\n",
                     schema_name, ENT_PREFIX, entity_name );
        }
    } else {
        if( entity->u.entity->subtype_expression ) {
            fprintf( impl, "        str.clear();\n        str.append( \"SUPERTYPE OF ( \" );\n" );
            format_for_std_stringout( impl, SUBTYPEto_string( entity->u.entity->subtype_expression ) );
            fprintf( impl, "\n      str.append( \")\" );\n" );
            fprintf( impl, "        %s::%s%s->AddSupertype_Stmt( str );", schema_name, ENT_PREFIX, entity_name );
        }
    }
    LISTdo( ENTITYget_supertypes( entity ), sup, Entity )
    /*  set the owning schema of the supertype  */
    super_schema = SCHEMAget_name( ENTITYget_schema( sup ) );
    /* print the supertype list for this entity */
    fprintf( impl, "        %s::%s%s->AddSupertype(%s::%s%s);\n",
             schema_name, ENT_PREFIX, entity_name,
             super_schema,
             ENT_PREFIX, ENTITYget_name( sup ) );

    /* add this entity to the subtype list of it's supertype    */
    fprintf( impl, "        %s::%s%s->AddSubtype(%s::%s%s);\n",
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
            fprintf( impl, "        %s::%s%d%s%s =\n          new %s"
                     "(\"%s\",%s::%s%s,\n          %s,%s%s,\n          *%s::%s%s);\n",
                     SCHEMAget_name( schema ), ATTR_PREFIX, attr_count,
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
            fprintf( impl, "        %s::%s%d%s%s =\n          new %s"
                     "(\"%s\",%s::%s%s,\n          %s,%s%s,\n          *%s::%s%s);\n",
                     SCHEMAget_name( schema ), ATTR_PREFIX, attr_count,
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
        /*  the type wasn't named -- it must be built in or aggregate  */

        fprintf( impl, "        %s::%s%d%s%s =\n          new %s"
                 "(\"%s\",%s%s,\n          %s,%s%s,\n          *%s::%s%s);\n",
                 SCHEMAget_name( schema ), ATTR_PREFIX, attr_count,
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
        print_typechain( header, impl, v->type, typename_buf, schema, v->name->symbol.name );
        fprintf( impl, "        %s::%s%d%s%s =\n          new %s"
                 "(\"%s\",%s,%s,%s%s,\n          *%s::%s%s);\n",
                 SCHEMAget_name( schema ), ATTR_PREFIX, attr_count,
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

    fprintf( impl, "        %s::%s%s->Add%sAttr (%s::%s%d%s%s);\n",
             schema_name, ENT_PREFIX, TYPEget_name( entity ),
             ( VARget_inverse( v ) ? "Inverse" : "Explicit" ),
             SCHEMAget_name( schema ), ATTR_PREFIX, attr_count,
             ( VARis_derived( v ) ? "D" :
               ( VARis_type_shifter( v ) ? "R" :
                 ( VARget_inverse( v ) ? "I" : "" ) ) ),
             attrnm );

    if( VARis_derived( v ) && v->initializer ) {
        tmp = EXPRto_string( v->initializer );
        tmp2 = ( char * )sc_malloc( sizeof( char ) * ( strlen( tmp ) + BUFSIZ ) );
        fprintf( impl, "        %s::%s%d%s%s->initializer_(\"%s\");\n",
                 schema_name, ATTR_PREFIX, attr_count,
                 ( VARis_derived( v ) ? "D" :
                   ( VARis_type_shifter( v ) ? "R" :
                     ( VARget_inverse( v ) ? "I" : "" ) ) ),
                 attrnm, format_for_stringout( tmp, tmp2 ) );
        sc_free( tmp );
        sc_free( tmp2 );
    }
    if( VARget_inverse( v ) ) {
        fprintf( impl, "        %s::%s%d%s%s->inverted_attr_id_(\"%s\");\n",
                 schema_name, ATTR_PREFIX, attr_count,
                 ( VARis_derived( v ) ? "D" :
                   ( VARis_type_shifter( v ) ? "R" :
                     ( VARget_inverse( v ) ? "I" : "" ) ) ),
                 attrnm, v->inverse_attribute->name->symbol.name );
        if( v->type->symbol.name ) {
            fprintf( impl,
                     "        %s::%s%d%s%s->inverted_entity_id_(\"%s\");\n",
                     schema_name, ATTR_PREFIX, attr_count,
                     ( VARis_derived( v ) ? "D" :
                       ( VARis_type_shifter( v ) ? "R" :
                         ( VARget_inverse( v ) ? "I" : "" ) ) ), attrnm,
                     v->type->symbol.name );
            fprintf( impl, "// inverse entity 1 %s\n", v->type->symbol.name );
        } else {
            switch( TYPEget_body( v->type )->type ) {
                case entity_:
                    fprintf( impl,
                             "        %s%d%s%s->inverted_entity_id_(\"%s\");\n",
                             ATTR_PREFIX, attr_count,
                             ( VARis_derived( v ) ? "D" :
                               ( VARis_type_shifter( v ) ? "R" :
                                 ( VARget_inverse( v ) ? "I" : "" ) ) ), attrnm,
                             TYPEget_body( v->type )->entity->symbol.name );
                    fprintf( impl, "// inverse entity 2 %s\n", TYPEget_body( v->type )->entity->symbol.name );
                    break;
                case aggregate_:
                case array_:
                case bag_:
                case set_:
                case list_:
                    fprintf( impl,
                             "        %s::%s%d%s%s->inverted_entity_id_(\"%s\");\n",
                             schema_name, ATTR_PREFIX, attr_count,
                             ( VARis_derived( v ) ? "D" :
                               ( VARis_type_shifter( v ) ? "R" :
                                 ( VARget_inverse( v ) ? "I" : "" ) ) ), attrnm,
                             TYPEget_body( v->type )->base->symbol.name );
                    fprintf( impl, "// inverse entity 3 %s\n", TYPEget_body( v->type )->base->symbol.name );
                    break;
                default:
                    fprintf(stderr, "Error: reached default case at %s:%d", __FILE__, __LINE__ );
                    abort();
            }
        }
    }
    attr_count++;

    LISTod

    fprintf( impl, "        reg.AddEntity (*%s::%s%s);\n",
             schema_name, ENT_PREFIX, entity_name );

#undef schema_name
}

void ENTITYPrint_h( const Entity entity, FILE * header, Linked_List neededAttr, Schema schema ) {
    const char *name = ENTITYget_classname( entity );
    DEBUG( "Entering ENTITYPrint_h for %s\n", name );

    ENTITYhead_print( entity, header );
    DataMemberPrint( entity, neededAttr, header );
    MemberFunctionSign( entity, neededAttr, header );
    
    fprintf( header, "void init_%s(Registry& reg);\n\n", name );

    fprintf( header, "namespace %s {\n", SCHEMAget_name( schema ) );
    ENTITYnames_print( entity, header );
    fprintf( header, "}\n\n" );

    DEBUG( "DONE ENTITYPrint_h\n" );
}

void ENTITYPrint_cc( const Entity entity, FILE * header, FILE * impl, Linked_List neededAttr, Schema schema ) {
    const char * name = ENTITYget_classname( entity );
    
    DEBUG( "Entering ENTITYPrint_cc for %s\n", name );

    fprintf( impl, "#include \"schema.h\"\n" );
    fprintf( impl, "#include \"sc_memmgr.h\"\n" );
    fprintf( impl, "#include \"entity/%s.h\"\n\n", name );

    LIBdescribe_entity( entity, impl, schema );
    LIBstructor_print( entity, impl, schema );
    if( multiple_inheritance ) {
        LIBstructor_print_w_args( entity, impl, schema );
    }
    LIBmemberFunctionPrint( entity, neededAttr, impl );
    
    fprintf( impl, "void init_%s( Registry& reg ) {\n", name );
    fprintf( impl, "    std::string str;\n\n" );
    ENTITYincode_print( entity, header, impl, schema );
    fprintf( impl, "}\n\n" );

    DEBUG( "DONE ENTITYPrint_cc\n" );
}

/**************************************************************//**
 ** Procedure:  collectAttributes
 ** Parameters:  Linked_List curList  --  current list to store the
 **  attributes
 **   Entity curEntity -- current Entity being processed
 **   int flagParent -- flag control
 ** Returns:
 ** Description:  Retrieve the list of inherited attributes of an
 ** entity
 ******************************************************************/
enum CollectType { ALL, ALL_BUT_FIRST, FIRST_ONLY };

static void collectAttributes( Linked_List curList, const Entity curEntity, enum CollectType collect ) {
    Linked_List parent_list = ENTITYget_supertypes( curEntity );

    if( ! LISTempty( parent_list ) ) {
        if( collect != FIRST_ONLY ) {
            /*  collect attributes from parents and their supertypes */
            LISTdo( parent_list, e, Entity ) {
                if( collect == ALL_BUT_FIRST ) {
                    /*  skip first and collect from the rest */
                    collect = ALL;
                } else {
                    /*  collect attributes of this parent and its supertypes */
                    collectAttributes( curList, e, ALL );
                }
            }
            LISTod;
        } else {
            /*  collect attributes of only first parent and its supertypes */
            collectAttributes( curList, ( Entity ) LISTpeek_first( parent_list ), ALL );
        }
    }
    /*  prepend this entity's attributes to the result list */
    LISTdo( ENTITYget_attributes( curEntity ), attr, Variable ) {
        LISTadd_first( curList, ( Generic ) attr );
    }
    LISTod;
}

static bool listContainsVar( Linked_List l, Variable v ) {
    const char * vName = VARget_simple_name( v );
    LISTdo( l, curr, Variable ) {
        if( streq( vName, VARget_simple_name( curr ) ) ) {
            return true;
        }
    }
    LISTod;
    return false;
}

/**************************************************************//**
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
void ENTITYlib_print( Entity entity, Linked_List neededAttr, FILE * file, Schema schema ) {
    LIBdescribe_entity( entity, file, schema );
    LIBstructor_print( entity, file, schema );
    if( multiple_inheritance ) {
        LIBstructor_print_w_args( entity, file, schema );
    }
    LIBmemberFunctionPrint( entity, neededAttr, file );
}

/**************************************************************//**
 ** Procedure:  ENTITYPrint
 ** Parameters:  Entity *entity --  entity being processed
 **     FILE* file  --  file being written to
 ** Returns:
 ** Description:  drives the functions for printing out code in lib,
 **     include, and initialization files for a specific entity class
 ** Side Effects:  generates code in 3 files
 ** Status:  complete 1/15/91
 ******************************************************************/
void ENTITYPrint( Entity entity, FILES * files, Schema schema ) {
    FILE * hdr, * impl;
    char * n = ENTITYget_name( entity );
    Linked_List remaining = LISTcreate();
    filenames_t names = getEntityFilenames( entity );

    DEBUG( "Entering ENTITYPrint for %s\n", n );

    if( multiple_inheritance ) {
        Linked_List existing = LISTcreate();
        Linked_List required = LISTcreate();

        /*  create list of attr inherited from the parents in C++ */
        collectAttributes( existing, entity, FIRST_ONLY );

        /*  create list of attr that have to be inherited in EXPRESS */
        collectAttributes( required, entity, ALL_BUT_FIRST );

        /*  build list of unique attr that are required but havn't been */
        /*  inherited */
        LISTdo( required, attr, Variable ) {
            if( !listContainsVar( existing, attr ) &&
                    !listContainsVar( remaining, attr ) ) {
                LISTadd_first( remaining, ( Generic ) attr );
            }
        }
        LISTod;
        LIST_destroy( existing );
        LIST_destroy( required );
    }
    if( mkDirIfNone( "entity" ) == -1 ) {
        fprintf( stderr, "At %s:%d - mkdir() failed with error ", __FILE__, __LINE__);
        perror( 0 );
        abort();
    }

    hdr = FILEcreate( names.header );
    impl = FILEcreate( names.impl );
    assert( hdr && impl && "error creating files" );
    fprintf( files->unity.entity.hdr, "#include \"%s\"\n", names.header );
    fprintf( files->unity.entity.impl, "#include \"%s\"\n", names.impl );

    ENTITYPrint_h( entity, hdr, remaining, schema );
    ENTITYPrint_cc( entity, hdr, impl, remaining, schema );
    FILEclose( hdr );
    FILEclose( impl );

    /*fprintf( files->inc,   "\n/////////         ENTITY %s\n", n );
    ENTITYinc_print( entity, remaining, files -> inc );
    fprintf( files->inc,     "/////////         END_ENTITY %s\n", n );*/
    fprintf( files->inc, "#include \"entity/%s.h\"\n", ENTITYget_classname( entity ) );

    /*fprintf( files->names, "\n/////////         ENTITY %s\n", n );
    ENTITYnames_print( entity, files -> names );
    fprintf( files->names,   "/////////         END_ENTITY %s\n", n );*/

    /*fprintf( files->lib,   "\n/////////         ENTITY %s\n", n );
    ENTITYlib_print( entity, remaining, files -> lib, schema );
    fprintf( files->lib,     "/////////         END_ENTITY %s\n", n );*/

    /*fprintf( files->init,  "\n/////////         ENTITY %s\n", n );
    ENTITYincode_print( entity, files , schema );
    fprintf( files->init,    "/////////         END_ENTITY %s\n", n );*/
    fprintf( files->init, "    init_%s( reg );\n", ENTITYget_classname( entity ) );

    DEBUG( "DONE ENTITYPrint\n" );
    LIST_destroy( remaining );
}

/** print in include file: class forward prototype, class typedefs, and
 *   extern EntityDescriptor.  `externMap' = 1 if entity must be instantiated
 *   with external mapping (see Part 21, sect 11.2.5.1).
 *  Nov 2011 - MAP - print EntityDescriptor in namespace file, modify other
 *   generated code to use namespace
 */
void ENTITYprint_new( Entity entity, FILES * files, Schema schema, int externMap ) {
    const char * n;
    Linked_List wheres;
    char * whereRule, *whereRule_formatted = NULL;
    size_t whereRule_formatted_size = 0;
    char * ptr, *ptr2;
    char * uniqRule, *uniqRule_formatted;
    Linked_List uniqs;

    fprintf( files->create, "    %s::%s%s = new EntityDescriptor(\n        ",
             SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );
    fprintf( files->create, "  \"%s\", %s::schema, %s, ",
             PrettyTmpName( ENTITYget_name( entity ) ),
             SCHEMAget_name( schema ), ( ENTITYget_abstract( entity ) ? "LTrue" : "LFalse" ) );
    fprintf( files->create, "%s,\n          ", externMap ? "LTrue" :
             "LFalse" );

    fprintf( files->create, "  (Creator) create_%s );\n",
             ENTITYget_classname( entity ) );
    /* add the entity to the Schema dictionary entry */
    fprintf( files->create, "    %s::schema->AddEntity(%s::%s%s);\n", SCHEMAget_name( schema ), SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

    wheres = TYPEget_where( entity );

    if( wheres ) {
        fprintf( files->create,
                 "    %s::%s%s->_where_rules = new Where_rule__list;\n",
                 SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

        LISTdo( wheres, w, Where ) {
        whereRule = EXPRto_string( w->expr );
        ptr2 = whereRule;

        if( whereRule_formatted_size == 0 ) {
            whereRule_formatted_size = 3 * BUFSIZ;
            whereRule_formatted = ( char * )sc_malloc( sizeof( char ) * whereRule_formatted_size );
        } else if( ( strlen( whereRule ) + 300 ) > whereRule_formatted_size ) {
            sc_free( whereRule_formatted );
            whereRule_formatted_size = strlen( whereRule ) + BUFSIZ;
            whereRule_formatted = ( char * )sc_malloc( sizeof( char ) * whereRule_formatted_size );
        }
        whereRule_formatted[0] = '\0';
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
        fprintf( files->create, "        wr = new Where_rule(\"%s\");\n", whereRule_formatted );
        fprintf( files->create, "        %s::%s%s->_where_rules->Append(wr);\n",
                 SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

        sc_free( whereRule );
        ptr2 = whereRule = 0;
        } LISTod
    }

    uniqs = entity->u.entity->unique;

    if( uniqs ) {
        fprintf( files->create,
                 "        %s::%s%s->_uniqueness_rules = new Uniqueness_rule__set;\n",
                 SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );

        if( whereRule_formatted_size == 0 ) {
            uniqRule_formatted = ( char * )sc_malloc( sizeof( char ) * 2 * BUFSIZ );
            whereRule_formatted = uniqRule_formatted;
        } else {
            uniqRule_formatted = whereRule_formatted;
        }

        LISTdo( uniqs, list, Linked_List ) {
            int i = 0;
            fprintf( files->create, "        ur = new Uniqueness_rule(\"" );
            LISTdo_n( list, e, Expression, b ) {
                i++;
                if( i == 1 ) {
                    /* print label if present */
                    if( e ) {
                        fprintf( files->create, "%s : ", StrToUpper( ( ( Symbol * )e )->name ) );
                    }
                } else {
                    if( i > 2 ) {
                        fprintf( files->create, ", " );
                    }
                    uniqRule = EXPRto_string( e );
                    fprintf( files->create, "%s", uniqRule );
                    sc_free( uniqRule );
                }
            } LISTod
            fprintf( files->create, ";\\n\");\n" );
            fprintf( files->create, "        %s::%s%s->_uniqueness_rules->Append(ur);\n",
                    SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );
        } LISTod
    }

    if( whereRule_formatted_size > 0 ) {
        sc_free( whereRule_formatted );
    }

    n = ENTITYget_classname( entity );
    fprintf( files->classes, "\nclass %s;\n", n );
    fprintf( files->classes, "typedef %s *          %sH;\n", n, n );
    fprintf( files->classes, "typedef %s *          %s_ptr;\n", n, n );
    fprintf( files->classes, "typedef %s_ptr        %s_var;\n", n, n );
    fprintf( files->classes, "#define %s__set         SDAI_DAObject__set\n", n );
    fprintf( files->classes, "#define %s__set_var     SDAI_DAObject__set_var\n", n );
}
