#include <string.h>

#include "class_strings.h"
#include "express/type.h"

const char * ClassName( const char * oldname ) {
    int i = 0, j = 0;
    static char newname [BUFSIZ];
    if( !oldname ) {
        return ( "" );
    }
    strcpy( newname, ENTITYCLASS_PREFIX )    ;
    j = strlen( ENTITYCLASS_PREFIX )    ;
    newname [j] = ToUpper( oldname [i] );
    ++i;
    ++j;
    while( oldname [i] != '\0' ) {
        newname [j] = ToLower( oldname [i] );
        /*  if (oldname [i] == '_')  */
        /*  character is '_'    */
        /*      newname [++j] = ToUpper (oldname [++i]);*/
        ++i;
        ++j;
    }
    newname [j] = '\0';
    return ( newname );
}

const char * ENTITYget_classname( Entity ent ) {
    const char * oldname = ENTITYget_name( ent );
    return ( ClassName( oldname ) );
}

const char * TYPEget_ctype( const Type t ) {
    Class_Of_Type ctype;
    Type bt;
    static char retval [BUFSIZ];

    if( TYPEinherits_from( t, aggregate_ ) ) {
        bt = TYPEget_body( t )->base;
        if( TYPEinherits_from( bt, aggregate_ ) ) {
            return( "GenericAggregate" );
        }

        ctype = TYPEget_type( bt );
        if( ctype == integer_ ) {
            return ( "IntAggregate" );
        }
        if( ( ctype == number_ ) || ( ctype == real_ ) ) {
            return ( "RealAggregate" );
        }
        if( ctype == entity_ ) {
            return( "EntityAggregate" );
        }
        if( ( ctype == enumeration_ ) || ( ctype == select_ ) )  {
            strcpy( retval, TYPEget_ctype( bt ) );
            strcat( retval, "_agg" );
            return ( retval );
        }
        if( ctype == logical_ ) {
            return ( "LOGICALS" );
        }
        if( ctype == boolean_ ) {
            return ( "BOOLEANS" );
        }
        if( ctype == string_ ) {
            return( "StringAggregate" );
        }
        if( ctype == binary_ ) {
            return( "BinaryAggregate" );
        }
    }

    /*  the rest is for things that are not aggregates  */
    ctype = TYPEget_type( t );

    /*    case TYPE_LOGICAL:    */
    if( ctype == logical_ ) {
        return ( "SDAI_LOGICAL" );
    }

    /*    case TYPE_BOOLEAN:    */
    if( ctype == boolean_ ) {
        return ( "SDAI_BOOLEAN" );
    }

    /*      case TYPE_INTEGER:  */
    if( ctype == integer_ ) {
        return ( "SDAI_Integer" );
    }

    /*      case TYPE_REAL:
     *        case TYPE_NUMBER:   */
    if( ( ctype == number_ ) || ( ctype == real_ ) ) {
        return ( "SDAI_Real" );
    }

    /*      case TYPE_STRING:   */
    if( ctype == string_ ) {
        return ( "SDAI_String" );
    }

    /*      case TYPE_BINARY:   */
    if( ctype == binary_ ) {
        return ( "SDAI_Binary" );
    }

    /*      case TYPE_ENTITY:   */
    if( ctype == entity_ ) {
        strncpy( retval, TypeName( t ), BUFSIZ - 2 );
        strcat( retval, "_ptr" );
        return retval;
        /*  return ("STEPentityH");    */
    }
    /*    case TYPE_ENUM:   */
    /*    case TYPE_SELECT: */
    if( ctype == enumeration_ ) {
        strncpy( retval, TypeName( t ), BUFSIZ - 2 );
        strcat( retval, "_var" );
        return retval;
    }
    if( ctype == select_ )  {
        return ( TypeName( t ) );
    }

    /*  default returns undefined   */
    return ( "SCLundefined" );
}

const char * TypeName( Type t ) {
    static char name [BUFSIZ];
    strcpy( name, TYPE_PREFIX );
    if( TYPEget_name( t ) ) {
        strncat( name, FirstToUpper( TYPEget_name( t ) ), BUFSIZ - strlen( TYPE_PREFIX ) - 1 );
    } else {
        return TYPEget_ctype( t );
    }
    return name;
}

char ToLower( char c ) {
    if( isupper( c ) ) {
        return ( tolower( c ) );
    } else {
        return ( c );
    }

}

char ToUpper( char c ) {
    if( islower( c ) ) {
        return ( toupper( c ) );
    } else {
        return ( c );
    }
}

const char * StrToLower( const char * word ) {
    static char newword [MAX_LEN];
    int i = 0;
    if( !word ) {
        return 0;
    }
    while( word [i] != '\0' ) {
        newword [i] = ToLower( word [i] );
        ++i;
    }
    newword [i] = '\0';
    return ( newword )    ;

}

const char * StrToUpper( const char * word ) {
    static char newword [MAX_LEN];
    int i = 0;

    while( word [i] != '\0' ) {
        newword [i] = ToUpper( word [i] );
        ++i;
    }
    newword [i] = '\0';
    return ( newword );
}

const char * StrToConstant( const char * word ) {
    static char newword[MAX_LEN];
    int i = 0;

    while( word [i] != '\0' ) {
        if( word [i] == '/' || word [i] == '.' ) {
            newword [i] = '_';
        } else {
            newword [i] = ToUpper( word [i] );
        }
        ++i;

    }
    newword [i] = '\0';
    return ( newword );
}

const char * FirstToUpper( const char * word ) {
    static char newword [MAX_LEN];

    strncpy( newword, word, MAX_LEN );
    newword[0] = ToUpper( newword[0] );
    return ( newword );
}
