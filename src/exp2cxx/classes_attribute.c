/** *****************************************************************
** \file classes_attribute.c
** FedEx parser output module for generating C++  class definitions
**
** Development of FedEx was funded by the United States Government,
** and is not subject to copyright.

*******************************************************************
The conventions used in this binding follow the proposed specification
for the STEP Standard Data Access Interface as defined in document
N350 ( August 31, 1993 ) of ISO 10303 TC184/SC4/WG7.
*******************************************************************/
#include <sc_memmgr.h>
#include <stdlib.h>
#include <assert.h>
#include <sc_mkdir.h>
#include "classes.h"
#include <ordered_attrs.h>

#include <sc_trace_fprintf.h>

extern int old_accessors;
extern int print_logging;

/**************************************************************//**
 ** Procedure:  generate_attribute_name
 ** Parameters:  Variable a, an Express attribute; char *out, the C++ name
 ** Description:  converts an Express name into the corresponding C++ name
 **       see relation to generate_dict_attr_name() DAS
 ** Side Effects:
 ** Status:  complete 8/5/93
 ******************************************************************/
char * generate_attribute_name( Variable a, char * out ) {
    char * temp, *q;
    const char * p;
    int i;

    temp = EXPRto_string( VARget_name( a ) );
    p = StrToLower( temp );
    if( ! strncmp( p, "self\\", 5 ) ) {
        p += 5;
    }
    /*  copy p to out  */
    /* DAR - fixed so that '\n's removed */
    for( i = 0, q = out; *p != '\0' && i < BUFSIZ; p++ ) {
        /* copy p to out, 1 char at time.  Skip \n's and spaces, convert
         *  '.' to '_', and convert to lowercase. */
        if( ( *p != '\n' ) && ( *p != ' ' ) ) {
            if( *p == '.' ) {
                *q = '_';
            } else {
                *q = *p;
            }
            i++;
            q++;
        }
    }
    *q = '\0';
    sc_free( temp );
    return out;
}

char * generate_attribute_func_name( Variable a, char * out ) {
    generate_attribute_name( a, out );
    strncpy( out, StrToLower( out ), BUFSIZ );
    if( old_accessors ) {
        out[0] = toupper( out[0] );
    } else {
        out[strlen( out )] = '_';
    }
    return out;
}

/**************************************************************//**
 ** Procedure:  ATTRsign_access_method
 ** Parameters:  const Variable a --  attribute to print
                                      access method signature for
 ** FILE* file  --  file being written to
 ** Returns:  nothing
 ** Description:  prints the signature for an access method
 **               based on the attribute type
 **       DAS i.e. prints the header for the attr. access functions
 **       (get and put attr value) in the entity class def in .h file
 ** Side Effects:
 ** Status:  complete 17-Feb-1992
 ******************************************************************/
void ATTRsign_access_methods( Variable a, const char * objtype, FILE * file ) {

    Type t = VARget_type( a );
    char ctype [BUFSIZ];
    char attrnm [BUFSIZ];

    generate_attribute_func_name( a, attrnm );

    strncpy( ctype, AccessType( t ), BUFSIZ );
    ctype[BUFSIZ-1] = '\0';
    fprintf( file, "        %s %s() const;\n", ctype, attrnm );
    fprintf( file, "        %s %s();\n", ctype, attrnm );
    fprintf( file, "        void %s (const %s x);\n", attrnm, ctype );
    if( VARget_inverse( a ) ) {
        fprintf( file, "        //static setter/getter pair, necessary for late binding\n" );
        fprintf( file, "        static %s get_%s( const SDAI_Application_instance * obj ) {\n", ctype, attrnm );
        fprintf( file, "            return ( ( %s * ) obj )->%s();\n        }\n", objtype, attrnm );
        fprintf( file, "        static void set_%s( SDAI_Application_instance * obj, const %s x) {\n", attrnm, ctype );
        fprintf( file, "            ( ( %s * ) obj )->%s( x );\n        }\n", objtype, attrnm );
    }
    fprintf( file, "\n" );
    return;
}

/**************************************************************//**
 ** Procedure:  ATTRprint_access_methods_get_head
 ** Parameters:  const Variable a --  attribute to find the type for
 ** FILE* file  --  file being written
 ** Type t - type of the attribute
 ** Class_Of_Type class -- type name of the class
 ** const char *attrnm -- name of the attribute
 ** char *ctype -- (possibly returned) name of the attribute c++ type
 ** Returns:  name to be used for the type of the c++ access functions
 ** Description:  prints the access method get head based on the attribute type
 **     DAS which being translated is it prints the function header
 **     for the get attr value access function defined for an
 **     entity class. This is the .cc file version.
 ** Side Effects:
 ** Status:  complete 7/15/93       by DDH
 ******************************************************************/
void ATTRprint_access_methods_get_head( const char * classnm, Variable a,
                                        FILE * file ) {
    Type t = VARget_type( a );
    char ctype [BUFSIZ];   /*  return type of the get function  */
    char funcnm [BUFSIZ];  /*  name of member function  */

    generate_attribute_func_name( a, funcnm );

    /* ///////////////////////////////////////////////// */

    strncpy( ctype, AccessType( t ), BUFSIZ );
    ctype[BUFSIZ-1] = '\0';
    fprintf( file, "\n%s %s::%s() ", ctype, classnm, funcnm );
    return;
}

/**************************************************************//**
 ** Procedure:  ATTRprint_access_methods_put_head
 ** Parameters:  const Variable a --  attribute to find the type for
 ** FILE* file  --  file being written to
 ** Type t - type of the attribute
 ** Class_Of_Type class -- type name of the class
 ** const char *attrnm -- name of the attribute
 ** char *ctype -- name of the attribute c++ type
 ** Returns:  name to be used for the type of the c++ access functions
 ** Description:  prints the access method put head based on the attribute type
 **     DAS which being translated is it prints the function header
 **     for the put attr value access function defined for an
 **     entity class. This is the .cc file version.
 ** Side Effects:
 ** Status:  complete 7/15/93       by DDH
 ******************************************************************/
void ATTRprint_access_methods_put_head( CONST char * entnm, Variable a, FILE * file ) {

    Type t = VARget_type( a );
    char ctype [BUFSIZ];
    char funcnm [BUFSIZ];

    generate_attribute_func_name( a, funcnm );

    strncpy( ctype, AccessType( t ), BUFSIZ );
    ctype[BUFSIZ-1] = '\0';
    fprintf( file, "\nvoid %s::%s( const %s x ) ", entnm, funcnm, ctype );

    return;
}

void AGGRprint_access_methods( CONST char * entnm, Variable a, FILE * file,
                               char * ctype, char * attrnm ) {
    ATTRprint_access_methods_get_head( entnm, a, file );
    fprintf( file, "{\n    if( !_%s ) {\n        _%s = new %s;\n    }\n", attrnm, attrnm, TypeName( a->type ) );
    fprintf( file, "    return ( %s ) %s_%s;\n}\n", ctype, ( ( a->type->u.type->body->base ) ? "" : "& " ), attrnm );
    ATTRprint_access_methods_get_head( entnm, a, file );
    fprintf( file, "const {\n" );
    fprintf( file, "    return ( %s ) %s_%s;\n}\n", ctype, ( ( a->type->u.type->body->base ) ? "" : "& " ), attrnm );
    ATTRprint_access_methods_put_head( entnm, a, file );
    fprintf( file, "{\n    if( !_%s ) {\n        _%s = new %s;\n    }\n", attrnm, attrnm, TypeName( a->type ) );
    fprintf( file, "    _%s%sShallowCopy( * x );\n}\n", attrnm, ( ( a->type->u.type->body->base ) ? "->" : "." ) );
    return;
}

/** print logging code for access methods for attrs that are entities
 * \p var is the variable name, minus preceding underscore, or null if 'x' is to be used
 * \p dir is either "returned" or "assigned"
 */
void ATTRprint_access_methods_entity_logging( const char * entnm, const char * funcnm, const char * nm,
                                              const char * var, const char * dir, FILE * file ) {
    if( print_logging ) {
        fprintf( file, "#ifdef SC_LOGGING\n" );
        fprintf( file, "    if( *logStream ) {\n" );
        fprintf( file, "        logStream -> open( SCLLOGFILE, ios::app );\n" );
        fprintf( file, "        if( !( %s%s == S_ENTITY_NULL ) )\n        {\n", ( var ? "_" : "" ), ( var ? var : "x" ) );
        fprintf( file, "            *logStream << time( NULL ) << \" SDAI %s::%s() %s: \";\n", entnm, funcnm, dir );
        fprintf( file, "            *logStream << \"reference to Sdai%s entity #\"", nm );
        fprintf( file,                        " << %s%s->STEPfile_id << std::endl;\n", ( var ? "_" : "" ), ( var ? var : "x" ) );
        fprintf( file, "        } else {\n" );
        fprintf( file, "            *logStream << time( NULL ) << \" SDAI %s::%s() %s: \";\n", entnm, funcnm, dir );
        fprintf( file, "            *logStream << \"null entity\" << std::endl;\n        }\n" );
        fprintf( file, "        logStream->close();\n" );
        fprintf( file, "    }\n" );
        fprintf( file, "#endif\n" );
    }
}

/** print access methods for attrs that are entities
 * prints const and non-const getters and a setter
 */
void ATTRprint_access_methods_entity( const char * entnm, const char * attrnm, const char * funcnm, const char * nm,
                                      const char * ctype, Variable a, FILE * file ) {
    fprintf( file, "const {\n" );
    ATTRprint_access_methods_entity_logging( entnm, funcnm, nm, attrnm, "returned", file);
    fprintf( file, "    return (%s) _%s;\n}\n", ctype, attrnm );

    ATTRprint_access_methods_get_head( entnm, a, file );
    fprintf( file, "{\n" );
    ATTRprint_access_methods_entity_logging( entnm, funcnm, nm, attrnm, "returned", file);
    fprintf( file, "    if( !_%s ) {\n        _%s = new %s;\n    }\n", attrnm, attrnm, TypeName( a->type ) );
    fprintf( file, "    return (%s) _%s;\n}\n", ctype, attrnm );

    ATTRprint_access_methods_put_head( entnm, a, file );
    fprintf( file, "{\n" );
    ATTRprint_access_methods_entity_logging( entnm, funcnm, nm, 0, "assigned", file);
    fprintf( file, "    _%s = x;\n}\n", attrnm );
    return;
}

/**************************************************************//**
 ** Procedure:  ATTRprint_access_methods
 ** Parameters:  const Variable a --  attribute to find the type for
 ** FILE* file  --  file being written to
 ** Description:  prints the access method based on the attribute type
 **       i.e. get and put value access functions defined in a class
 **       generated for an entity.
 ** Side Effects:
 ** Status:  complete 1/15/91
 **     updated 17-Feb-1992 to print to library file instead of header
 ** updated 15-July-1993 to call the get/put head functions by DDH
 ******************************************************************/
void ATTRprint_access_methods( const char * entnm, Variable a, FILE * file ) {
    Type t = VARget_type( a );
    Class_Of_Type class;
    char ctype [BUFSIZ];  /*  type of data member  */
    char attrnm [BUFSIZ];
    char membernm[BUFSIZ];
    char funcnm [BUFSIZ];  /*  name of member function  */

    char nm [BUFSIZ];
    /* I believe nm has the name of the underlying type without Sdai in front of it */
    if( TYPEget_name( t ) ) {
        strncpy( nm, FirstToUpper( TYPEget_name( t ) ), BUFSIZ - 1 );
    }

    generate_attribute_func_name( a, funcnm );
    generate_attribute_name( a, attrnm );
    strcpy( membernm, attrnm );
    membernm[0] = toupper( membernm[0] );
    class = TYPEget_type( t );
    strncpy( ctype, AccessType( t ), BUFSIZ );

    if( isAggregate( a ) ) {
        AGGRprint_access_methods( entnm, a, file, ctype, attrnm );
        return;
    }
    ATTRprint_access_methods_get_head( entnm, a, file );

    /*      case TYPE_ENTITY:   */
    if( class == entity_ )  {
        ATTRprint_access_methods_entity( entnm, attrnm, funcnm, nm, ctype, a, file );
        return;
    }
    /*    case TYPE_LOGICAL:    */
    if( ( class == boolean_ ) || ( class == logical_ ) )  {

        fprintf( file, "const {\n" );
        if( print_logging ) {
            fprintf( file, "#ifdef SC_LOGGING\n" );
            fprintf( file, "    if(*logStream)\n    {\n" );
            fprintf( file, "        logStream->open(SCLLOGFILE,ios::app);\n" );
            fprintf( file, "        if(!_%s.is_null())\n        {\n", attrnm );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n",
                     entnm, funcnm );
            fprintf( file,
                     "            *logStream << _%s.element_at(_%s.asInt()) << std::endl;\n",
                     attrnm, attrnm );
            fprintf( file, "        }\n        else\n        {\n" );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n",
                     entnm, funcnm );
            fprintf( file,
                     "            *logStream << \"unset\" << std::endl;\n        }\n" );
            fprintf( file, "            logStream->close();\n" );
            fprintf( file, "    }\n" );
            fprintf( file, "#endif\n" );
        }
        fprintf( file, "    return (%s) _%s;\n}\n", ctype, attrnm );

        ATTRprint_access_methods_put_head( entnm, a, file );
        fprintf( file, "{\n" );
        if( print_logging ) {
            fprintf( file, "#ifdef SC_LOGGING\n" );
            fprintf( file, "    if(*logStream)\n    {\n" );
            fprintf( file, "        *logStream << time(NULL) << \" SDAI %s::%s() assigned: \";\n",
                     entnm, funcnm );
            fprintf( file,
                     "        *logStream << _%s.element_at(x) << std::endl;\n", attrnm );
            fprintf( file, "    }\n" );
            fprintf( file, "#endif\n" );
        }
        fprintf( file, "    _%s.put (x);\n}\n", attrnm );
        return;
    }
    /*    case TYPE_ENUM:   */
    if( class == enumeration_ )  {
        fprintf( file, "const {\n" );
        if( print_logging ) {
            fprintf( file, "#ifdef SC_LOGGING\n" );
            fprintf( file, "    if(*logStream)\n    {\n" );
            fprintf( file, "        if(!_%s.is_null())\n        {\n", attrnm );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n",
                     entnm, funcnm );
            fprintf( file,
                     "            *logStream << _%s.element_at(_%s.asInt()) << std::endl;\n",
                     attrnm, attrnm );
            fprintf( file, "        }\n        else\n        {\n" );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n",
                     entnm, funcnm );
            fprintf( file,
                     "            *logStream << \"unset\" << std::endl;\n        }\n    }\n" );
            fprintf( file, "#endif\n" );
        }
        fprintf( file, "    return (%s) _%s;\n}\n",
                 EnumName( TYPEget_name( t ) ), attrnm );

        ATTRprint_access_methods_put_head( entnm, a, file );
        fprintf( file, "{\n" );
        if( print_logging ) {
            fprintf( file, "#ifdef SC_LOGGING\n" );
            fprintf( file, "    if(*logStream)\n    {\n" );
            fprintf( file, "        *logStream << time(NULL) << \" SDAI %s::%s() assigned: \";\n",
                     entnm, funcnm );
            fprintf( file,
                     "        *logStream << _%s.element_at(x) << std::endl;\n", attrnm );
            fprintf( file, "    }\n" );
            fprintf( file, "#endif\n" );
        }
        fprintf( file, "    _%s.put (x);\n}\n", attrnm );
        return;
    }
    /*    case TYPE_SELECT: */
    if( class == select_ )  {
        fprintf( file, "const {\n    return (const %s) &_%s;\n    }\n",  ctype, attrnm );
        ATTRprint_access_methods_put_head( entnm, a, file );
        fprintf( file, " {\n    _%s = x;\n    }\n", attrnm );
        return;
    }
    /*    case TYPE_AGGRETATES: */
    /* handled in AGGRprint_access_methods(entnm, a, file, t, ctype, attrnm) */


    /*  case STRING:*/
    /*      case TYPE_BINARY:   */
    if( ( class == string_ ) || ( class == binary_ ) )  {
        fprintf( file, "const {\n" );
        if( print_logging ) {
            fprintf( file, "#ifdef SC_LOGGING\n" );
            fprintf( file, "    if(*logStream)\n    {\n" );
            fprintf( file, "        if(!_%s.is_null())\n        {\n", attrnm );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n", entnm, funcnm );
            fprintf( file, "            *logStream << _%s << std::endl;\n", attrnm );
            fprintf( file, "        }\n        else\n        {\n" );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n", entnm, funcnm );
            fprintf( file, "            *logStream << \"unset\" << std::endl;\n        }\n    }\n" );
            fprintf( file, "#endif\n" );

        }
        fprintf( file, "    return (const %s) _%s;\n}\n", ctype, attrnm );
        ATTRprint_access_methods_put_head( entnm, a, file );
        fprintf( file, "{\n" );
        if( print_logging ) {
            fprintf( file, "#ifdef SC_LOGGING\n" );
            fprintf( file, "    if(*logStream)\n    {\n" );
            fprintf( file, "        if(!x)\n        {\n" );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n", entnm, funcnm );
            fprintf( file, "            *logStream << x << std::endl;\n" );
            fprintf( file, "        }\n        else\n        {\n" );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n", entnm, funcnm );
            fprintf( file, "            *logStream << \"unset\" << std::endl;\n        }\n    }\n" );
            fprintf( file, "#endif\n" );
        }
        fprintf( file, "    _%s = x;\n}\n", attrnm );
        return;
    }
    /*      case TYPE_INTEGER:  */
    if( class == integer_ ) {
        fprintf( file, "const {\n" );
        if( print_logging ) {
            fprintf( file, "#ifdef SC_LOGGING\n" );
            fprintf( file, "    if(*logStream)\n    {\n" );
            fprintf( file, "        if(!(_%s == S_INT_NULL) )\n        {\n", attrnm );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n",
                     entnm, funcnm );
            fprintf( file,
                     "            *logStream << _%s << std::endl;\n", attrnm );
            fprintf( file, "        }\n        else\n        {\n" );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n",
                     entnm, funcnm );
            fprintf( file,
                     "            *logStream << \"unset\" << std::endl;\n        }\n    }\n" );
            fprintf( file, "#endif\n" );
        }
        /*  default:  INTEGER   */
        /*  is the same type as the data member  */
        fprintf( file, "    return (const %s) _%s;\n}\n", ctype, attrnm );
        ATTRprint_access_methods_put_head( entnm, a, file );
        fprintf( file, "{\n" );
        if( print_logging ) {
            fprintf( file, "#ifdef SC_LOGGING\n" );
            fprintf( file, "    if(*logStream)\n    {\n" );
            fprintf( file, "        if(!(x == S_INT_NULL) )\n        {\n" );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n", entnm, funcnm );
            fprintf( file, "            *logStream << x << std::endl;\n" );
            fprintf( file, "        }\n        else\n        {\n" );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n", entnm, funcnm );
            fprintf( file, "            *logStream << \"unset\" << std::endl;\n        }\n    }\n" );
            fprintf( file, "#endif\n" );
            /*  default:  INTEGER   */
            /*  is the same type as the data member  */
        }
        fprintf( file, "    _%s = x;\n}\n", attrnm );
    }

    /*      case TYPE_REAL:
        case TYPE_NUMBER:   */
    if( ( class == number_ ) || ( class == real_ ) ) {
        fprintf( file, "const {\n" );
        if( print_logging ) {
            fprintf( file, "#ifdef SC_LOGGING\n" );
            fprintf( file, "    if(*logStream)\n    {\n" );
            fprintf( file, "        if(!(_%s == S_REAL_NULL) )\n        {\n", attrnm );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n", entnm, funcnm );
            fprintf( file, "            *logStream << _%s << std::endl;\n", attrnm );
            fprintf( file, "        }\n        else\n        {\n" );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n", entnm, funcnm );
            fprintf( file, "            *logStream << \"unset\" << std::endl;\n        }\n    }\n" );
            fprintf( file, "#endif\n" );
        }
        fprintf( file, "    return (const %s) _%s;\n}\n", ctype, attrnm );
        ATTRprint_access_methods_put_head( entnm, a, file );
        fprintf( file, "{\n" );
        if( print_logging ) {
            fprintf( file, "#ifdef SC_LOGGING\n" );
            fprintf( file, "    if(*logStream)\n    {\n" );
            fprintf( file, "        if(!(_%s == S_REAL_NULL) )\n        {\n", attrnm );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n", entnm, funcnm );
            fprintf( file, "            *logStream << _%s << std::endl;\n", attrnm );
            fprintf( file, "        }\n        else\n        {\n" );
            fprintf( file, "            *logStream << time(NULL) << \" SDAI %s::%s() returned: \";\n", entnm, funcnm );
            fprintf( file, "            *logStream << \"unset\" << std::endl;\n        }\n    }\n" );
            fprintf( file, "#endif\n" );
        }
        fprintf( file, "    _%s = x;\n}\n", attrnm );
    }
}
