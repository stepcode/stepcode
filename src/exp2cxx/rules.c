#include "rules.h"
#include <express/type.h>
#include "classes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* print Where_rule's. for types, schema should be null - tename will include schema name */
void WHEREprint( const char * tename, Linked_List wheres, FILE * impl, Schema schema, bool needWR ) {
    if( exp2cxx_api_version == 2 ) {
        int count = 0;
        if( !wheres ) {
            return;
        }
        fprintf( impl, "    {\n    const char * whereRules[] = {\n" );
        LISTdo( wheres, compactWhere, Where ) {
            char * expression = EXPRto_string( compactWhere->expr );
            char * escaped = ( char * )malloc( strlen( expression ) * 2 + BUFSIZ );
            fprintf( impl, "        \"%s%s(%s);\\n\",\n",
                     compactWhere->label ? compactWhere->label->name : "",
                     compactWhere->label ? ": " : "",
                     format_for_stringout( expression, escaped ) );
            free( expression );
            free( escaped );
            ++count;
        } LISTod
        fprintf( impl, "    };\n    InitializeWhereRules( *%s%s%s, "
                 "whereRules, %d );\n    }\n",
                 schema ? SCHEMAget_name( schema ) : "",
                 schema ? "::" ENT_PREFIX : "", tename, count );
        return;
    }

    if( wheres ) {
        fprintf( impl, "    %s%s%s->_where_rules = new Where_rule__list;\n", ( schema ? SCHEMAget_name( schema ) : "" ), ( schema ? "::" ENT_PREFIX : "" ), tename );
        if( needWR ) {
            fprintf( impl, "        Where_rule * wr;\n" );
        }

        LISTdo( wheres, w, Where ) {
            fprintf( impl, "        str.clear();\n");
            if( w->label ) {
                fprintf( impl, "        str.append( \"%s: (\" );\n", w->label->name );
            } else {
                /* no label */
                fprintf( impl, "        str.append( \"(\" );\n");
            }
            format_for_std_stringout( impl, EXPRto_string( w->expr ) );

            fprintf( impl, "        str.append( \");\\n\" );\n");

            fprintf( impl, "        wr = new Where_rule( str.c_str() );\n" );
            fprintf( impl, "        %s%s%s->_where_rules->Append( wr );\n", ( schema ? SCHEMAget_name( schema ) : "" ), ( schema ? "::" ENT_PREFIX : "" ), tename );

        } LISTod
    }
}

/* print Uniqueness_rule's */
void UNIQUEprint( Entity entity, FILE * impl, Schema schema ) {
    Linked_List uniqs = entity->u.entity->unique;
    if( exp2cxx_api_version == 2 ) {
        int count = 0;
        if( !uniqs ) {
            return;
        }
        fprintf( impl,
                 "    {\n    const char * uniquenessRules[] = {\n" );
        LISTdo( uniqs, compactList, Linked_List ) {
            int item = 0;
            fprintf( impl, "        \"" );
            LISTdo_n( compactList, compactExpression, Expression, b ) {
                ++item;
                if( item == 1 ) {
                    if( compactExpression ) {
                        fprintf( impl, "%s : ",
                                 StrToUpper( ( ( Symbol * )compactExpression )->name ) );
                    }
                } else {
                    char * expression = EXPRto_string( compactExpression );
                    char * escaped = ( char * )malloc(
                        strlen( expression ) * 2 + BUFSIZ );
                    if( item > 2 ) {
                        fprintf( impl, ", " );
                    }
                    fprintf( impl, "%s",
                             format_for_stringout( expression, escaped ) );
                    free( expression );
                    free( escaped );
                }
            } LISTod
            fprintf( impl, "\",\n" );
            ++count;
        } LISTod
        fprintf( impl, "    };\n    InitializeUniquenessRules( *%s::%s%s, "
                 "uniquenessRules, %d );\n    }\n",
                 SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ),
                 count );
        return;
    }

    if( uniqs ) {
        fprintf( impl, "        %s::%s%s->_uniqueness_rules = new Uniqueness_rule__set;\n", SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );
        fprintf( impl, "        Uniqueness_rule * ur;\n" );
        LISTdo( uniqs, list, Linked_List ) {
            int i = 0;
            fprintf( impl, "        str.clear();\n");
            LISTdo_n( list, e, Expression, b ) {
                i++;
                if( i == 1 ) {
                    /* print label if present */
                    if( e ) {
                        fprintf( impl, "    str.append( \"%s : \" );\n", StrToUpper( ( ( Symbol * )e )->name ) );
                    }
                } else {
                    if( i > 2 ) {
                        fprintf( impl, "    str.append( \", \" );\n");
                    }
                    format_for_std_stringout( impl, EXPRto_string( e ) );
                }
            } LISTod
            fprintf( impl, "    ur = new Uniqueness_rule( str.c_str() );\n" );
            fprintf( impl, "    %s::%s%s->_uniqueness_rules->Append(ur);\n", SCHEMAget_name( schema ), ENT_PREFIX, ENTITYget_name( entity ) );
        } LISTod
    }
}
