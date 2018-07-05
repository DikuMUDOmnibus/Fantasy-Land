/***************************************************************************
 *  File: string.c                                                         *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

char *string_insertline( CHAR_DATA *ch, char *orig, int line, char *addstring )
{
    char *rdesc;
    char xbuf[4*MAX_STRING_LENGTH];
    int i = 0, current_line = 1;

    strcpy( xbuf, orig );

    for ( rdesc = orig; *rdesc; rdesc++ )
    {
	if ( current_line == line )
	    break;

	i++;

	if ( *rdesc == '\r' )
	    current_line++;
    }

    if ( !*rdesc )
    {
	send_to_char( "That line does not exist.\n\r", ch );
	free_string( orig );
	return str_dup( xbuf );
    }

    xbuf[i] = '\0';

    if ( *addstring )
	strcat( xbuf, addstring );
    strcat( xbuf, "\n\r" );
    strcat( xbuf, &orig[i] );

    send_to_char( "Line inserted.\n\r", ch );

    free_string( orig );
    return str_dup( xbuf );
}

char *string_deleteline( char *orig, int line )
{
    char *rdesc;
    char xbuf[4*MAX_STRING_LENGTH];
    int i = 0, current_line = 1;

    xbuf[0] = '\0';

    for ( rdesc = orig; *rdesc; rdesc++ )
    {
	if ( current_line != line )
	{
	    xbuf[i] = *rdesc;
	    i++;
	}

	if ( *rdesc == '\r' )
	    current_line++;
    }

    xbuf[i] = 0;

    free_string( orig );
    return str_dup( xbuf );
}

char *string_replace_line( CHAR_DATA *ch, char *orig, int line, char *new )
{
    char *rdesc;
    char xbuf[4*MAX_STRING_LENGTH];
    bool fReplaced = FALSE;
    int current_line = 1, i = 0;

    strcpy( xbuf, orig );

    for ( rdesc = orig; *rdesc; rdesc++ )
    {
	if ( current_line == line && !fReplaced )
	{
	    xbuf[i] = '\0';

	    if ( *new )
		strcat( xbuf, new );
	    strcat( xbuf, "\n\r" );
	    fReplaced = TRUE;
	}

	if ( current_line == line + 1 )
	{
	    strcat( xbuf, &orig[i] );

	    send_to_char( "Line replaced.\n\r", ch );

	    free_string( orig );
	    return str_dup( xbuf );
	}

	i++;

	if ( *rdesc == '\r' )
	    current_line++;
    }

    if ( current_line - 1 != line )
    {
	send_to_char( "That line does not exist.\n\r", ch );
	free_string( orig );
	return str_dup( xbuf );
    }

    send_to_char( "Line replaced.\n\r", ch );
    free_string( orig );
    return str_dup( xbuf );
}

void string_append( CHAR_DATA *ch, char **pString )
{
    send_to_char( "-=========- Entering APPEND Mode -==========-\n\r"
		  "       Type .h on a new line for help        \n\r"
		  "     Terminate with a @ on a blank line.     \n\r"
		  "-===========================================-\n\r", ch );

    if ( *pString == NULL )
	*pString = str_dup( "" );

    ch->desc->pString = pString;

    string_add( ch, ".s" );
}

char *string_replace( char *orig, char *old, char *new )
{
    char xbuf[4*MAX_STRING_LENGTH];
    int i;

    strcpy( xbuf, orig );

    if ( strstr( orig, old ) != NULL )
    {
	i = strlen( orig ) - strlen( strstr( orig, old ) );
	xbuf[i] = '\0';
	strcat( xbuf, new );
	strcat( xbuf, &orig[i+strlen( old )] );
    }

    free_string( orig );
    return str_dup( xbuf );
}

char *first_arg( char *argument, char *arg_first )
{
    char cEnd;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"'
    ||   *argument == '%'  || *argument == '(' )
    {
	if ( *argument == '(' )
	{
	    cEnd = ')';
	    argument++;
	}
	else
	    cEnd = *argument++;
    }

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}

	*arg_first = *argument;
	arg_first++;
	argument++;
    }

    *arg_first = '\0';

    return argument;
}

void string_add( CHAR_DATA *ch, char *argument )
{
    char buf[4*MAX_STRING_LENGTH], temp[MAX_INPUT_LENGTH];

    smash_tilde( argument );

    if ( *argument == '.' )
    {
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg1 );

	if ( !str_cmp( arg1, "./" ) )
	{
	    interpret( ch, argument );
	    send_to_char( "Command performed.\n\r", ch );
	    return;
        }

	if ( !str_cmp( arg1, ".c" ) )
	{
	    send_to_char( "String cleared.\n\r", ch );
	    free_string( *ch->desc->pString );
	    *ch->desc->pString = str_dup( "" );
	    return;
        }

	if ( !str_cmp( arg1, ".s" ) )
	{
	    char *rdesc;
	    int i = 1;

	    sprintf( buf, "%2d} ", i );
	    send_to_char( buf, ch );

	    for ( rdesc = *ch->desc->pString; *rdesc; rdesc++ )
	    {
		if ( *rdesc != '{' )
		{
		    sprintf( buf, "%c", rdesc[0] );
		    send_to_char( buf, ch );
		} else {
		    if ( rdesc[1] == 'Z' )
			send_to_char( "<Z>", ch );
		    else
		    {
			sprintf( buf, "%c%c", rdesc[0], rdesc[1] );
			send_to_char( buf, ch );
		    }
		    rdesc++;
		}

		if ( *rdesc == '\r' && *( rdesc + 1 ) )
		{
		    i++;
		    sprintf( buf, "%2d} ", i );
		    send_to_char( buf, ch );
		}
	    }

	    return;
	}

	argument = first_arg( argument, arg2 );

	if ( !str_cmp( arg1, ".r" ) )
	{
	    if ( arg2[0] == '\0' )
	    {
		send_to_char( "usage:  .r \"old string\" new string\n\r", ch );
		return;
	    }

	    *ch->desc->pString = string_replace( *ch->desc->pString, arg2, argument );
	    sprintf( buf, "'%s' replaced with '%s'.\n\r\n\r", arg2, argument );
	    send_to_char( buf, ch );
	    string_add( ch, ".s" );
	    return;
	}

	if ( !str_cmp( arg1, ".rl" ) || !str_cmp( arg1, ".lr" ) )
	{
	    if ( arg2[0] == '\0' || !is_number( arg2 ) )
	    {
		send_to_char( "usage:  .rl <line> <text>\n\r", ch );
		return;
	    }

	    *ch->desc->pString = string_replace_line( ch, *ch->desc->pString, atoi( arg2 ), argument );
	    string_add( ch, ".s" );
	    return;
	}

	if ( !str_cmp( arg1, ".d" ) || !str_cmp( arg1, ".ld" ) )
	{
	    if ( arg2[0] == '\0' || !is_number( arg2 ) )
	    {
		send_to_char( "usage:  .d <line>\n\r", ch );
		return;
	    }

	    *ch->desc->pString = string_deleteline( *ch->desc->pString, atoi( arg2 ) );
	    sprintf( buf, "Line %d deleted.\n\r\n\r", atoi ( arg2 ) );
	    send_to_char( buf, ch );
	    string_add( ch, ".s" );
	    return;
	}

	if ( !str_cmp( arg1, ".f" ) )
	{
	    *ch->desc->pString = format_string( *ch->desc->pString, atoi( arg2 ) );
	    string_add( ch, ".s" );
	    return;
	}

	if ( !str_cmp( arg1, ".i" ) )
	{
	    if ( arg2[0] == '\0' || !is_number( arg2 ) )
	    {
		send_to_char( "usage:  .i <line> <text>\n\r", ch );
		return;
	    }

	    if ( strlen( *ch->desc->pString ) + strlen( argument ) >= 4*MAX_STRING_LENGTH-2 )
	    {
		send_to_char( "String too long, last line skipped.\n\r", ch );
		return;
	    }

	    *ch->desc->pString = string_insertline( ch, *ch->desc->pString, atoi( arg2 ), argument );
	    string_add( ch, ".s" );
	    return;
	}

	if ( !str_cmp( arg1, ".h" ) )
	{
	    send_to_char( "Sedit help (commands on blank line):    \n\r"
			  ".r 'old' new      - replace a substring \n\r"
			  "                    (requires '', \"\") \n\r"
			  ".rl <#> <text>    - replaces a line     \n\r"
			  ".h                - get help (this info)\n\r"
			  ".s                - show string so far  \n\r"
			  ".f (width)        - (word wrap) string  \n\r"
			  ".c                - clear string so far \n\r"
			  ".d <line#>        - delete a line       \n\r"
			  ".i <line#> <text> - insert a line       \n\r"
			  "./ <command>      - do a regular command\n\r"
			  "@                 - end string          \n\r", ch );
	    return;
	}

	send_to_char( "SEdit:  Invalid dot command.\n\r", ch );
	return;
    }

    if ( *argument == '@' || *argument == '~' || !str_cmp( argument, "END" ) )
    {
	switch( ch->desc->editor )
	{
	    default:
		break;

	    case ED_AREA:
		SET_BIT( ((AREA_DATA *)ch->desc->pEdit)->area_flags, AREA_CHANGED );
		break;

	    case ED_ROOM:
		SET_BIT( ((ROOM_INDEX_DATA *)ch->desc->pEdit)->area->area_flags, AREA_CHANGED );
		break;

	    case ED_OBJECT:
		SET_BIT( ((OBJ_INDEX_DATA *)ch->desc->pEdit)->area->area_flags, AREA_CHANGED );
		break;

	    case ED_MOBILE:
		SET_BIT( ((MOB_INDEX_DATA *)ch->desc->pEdit)->area->area_flags, AREA_CHANGED );
		break;

	    case ED_MPCODE:
	    case ED_OPCODE:
	    case ED_RPCODE:
		SET_BIT( ((PROG_CODE *)ch->desc->pEdit)->area->area_flags, AREA_CHANGED );
		break;

	    case ED_HELP:
		mud_stat.helps_changed = TRUE;
		break;

	    case ED_SHOP:
		SET_BIT( get_mob_index( ((SHOP_DATA *)ch->desc->pEdit)->keeper )->area->area_flags, AREA_CHANGED );
		break;

	    case ED_SKILL:
	    case ED_GROUP:
		mud_stat.skills_changed = TRUE;
		break;

	    case ED_CLASS:
		mud_stat.classes_changed = TRUE;
		break;

	    case ED_CLAN:
		mud_stat.clans_changed = TRUE;
		break;

	    case ED_RACE:
		mud_stat.races_changed = TRUE;
		break;

	    case ED_SOCIAL:
		mud_stat.socials_changed = TRUE;
		break;

	    case ED_ROOM_DAM:
		SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
		break;

	    case ED_CHANNEL:
		mud_stat.changed = TRUE;
		break;

	    case ED_PREFIX:
	    case ED_SUFFIX:
		mud_stat.randoms_changed = TRUE;
		break;
	}

	ch->desc->pString = NULL;
	return;
    }

    strcpy( buf, *ch->desc->pString );

    // Parsed lines: max 75 chars:
    while ( *argument != '\0' )
    {
	argument = length_argument( argument, temp, 75 );

	if ( strlen( buf ) + strlen( temp ) >= ( 4*MAX_STRING_LENGTH ) - 2 )
	{
	    send_to_char( "String too long input terminated.\n\r", ch );
	    free_string( *ch->desc->pString );
	    *ch->desc->pString = str_dup( buf );
	    return;
	}

	strcat( buf, temp );
	strcat( buf, "\n\r" );
    }

    free_string( *ch->desc->pString );
    *ch->desc->pString = str_dup( buf );
    return;
}

char *format_string( char *oldstring, int width )
{
    char xbuf[4*MAX_STRING_LENGTH];
    char xbuf2[4*MAX_STRING_LENGTH];
    char *rdesc;
    int j = 0, i = 0, count = 0;

    xbuf[0] = '\0';

    for ( rdesc = oldstring; *rdesc; rdesc++ )
    {
	if ( *rdesc == '\n' )
	{
	    if ( xbuf[i-1] != ' ' )
		xbuf[i++] = ' ';
	}

	else if ( *rdesc == '\r' )
	    ;

	else if ( *rdesc == ' ' )
	{
	    if ( xbuf[i-1] != ' ' )
		xbuf[i++] = ' ' ;
	}

	else if ( *rdesc == ')' )
	{
	    if ( xbuf[i-1] == ' ' && xbuf[i-2] == ' '
	    && ( xbuf[i-3] == '.' || xbuf[i-3] == '?' || xbuf[i-3] == '!' ) )
	    {
		xbuf[i-2] = *rdesc;
		xbuf[i-1] = ' ';
		xbuf[i++] = ' ';
	    }
	    else
		xbuf[i++] = *rdesc;
	}

	else if ( *rdesc == '.' || *rdesc == '?' || *rdesc == '!' )
	{
	    if ( xbuf[i-1] == ' ' && xbuf[i-2] == ' '
	    && ( xbuf[i-3] == '.' || xbuf[i-3] == '?' || xbuf[i-3] == '!' ) )
	    {
		xbuf[i-2] =* rdesc;
		if ( *(rdesc+1) != '\"' )
		{
		    xbuf[i-1] = ' ';
		    xbuf[i++] = ' ';
		} else {
		    xbuf[i-1] = '\"';
		    xbuf[i++] = ' ';
		    xbuf[i++] = ' ';
		    rdesc++;
		}
	    } else {
		xbuf[i] = *rdesc;
		if ( *(rdesc+1) != '\"' )
		{
		    xbuf[i+1] = ' ';
		    xbuf[i+2] = ' ';
		    i += 3;
		} else {
		    xbuf[i+1] = '\"';
		    xbuf[i+2] = ' ';
		    xbuf[i+3] = ' ';
		    i += 4;
		    rdesc++;
		}
	    }
	} else {
	    xbuf[i] = *rdesc;
	    i++;
	}
    }

    xbuf[i] = '\0';

    if ( width == 0 || width > 200 )
	width = 62;

    else if ( width < 20 )
	width = 20;

    xbuf2[0] = 0;
    i = 0;
    while ( xbuf[i] != '\0' )
    {
	if ( count >= width && xbuf[i] == ' ' )
	{
	    i++;
	    count = 0;

	    while ( xbuf[i] == ' ' )
		i++;

	    if ( xbuf[i] == '{'   && xbuf[i+1] == 'x'
	    &&   xbuf[i+2] == ' ' && xbuf[i+3] == '\0' )
	    {
		xbuf2[j] = '{';
		xbuf2[j+1] = 'x';
		j+=2;
		break;
	    }

	    xbuf2[j] = '\n';
	    xbuf2[j+1] = '\r';
	    j+=2;
	}

	if ( xbuf[i] == '{' && xbuf[i+1] != '{' )
	    count--;
	else
	    count++;

	xbuf2[j] = xbuf[i];
	i++;
	j++;
    }

    xbuf2[j] = '\0';
    strcat( xbuf2, "\n\r" );

    free_string( oldstring );
    return str_dup( xbuf2 );
}

int strlen_color( char *argument )
{
    char *str;
    int length;

    if ( argument == NULL || argument[0] == '\0' )
        return 0;

    length = 0;
    str = argument;

    while ( *str != '\0' )
    {
	if ( *str == '{' )
	{
	    str++;

	    if ( *str == '{' || *str == '-' )
		length++;

	    str++;
	    continue;
	}

	str++;
	length++;
    }

    return length;
}

int actlen_color( const char *argument )
{
    const char *str;
    int length = 0;

    if ( argument == NULL || argument[0] == '\0' )
        return 0;

    str = argument;

    while ( *str != '\0' )
    {
	if ( *str != '{' )
	{
	    str++;
	    length++;
	    continue;
	}

	if ( *(++str) == '{' )
	    length++;

	length += strlen( colour2( *str ) );
	str++;
    }

    return length;
}

char *center_string( const char *txt, int length )
{
    static char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    bool place = FALSE;

    if ( txt == NULL )
	strcpy( buf, "(Null)" );
    else
	strcpy( buf, txt );

    if ( length <= 0 )
    {
	bug( "Center_string: invalid length!", 0 );
	return buf;
    }

    if ( strlen_color( buf ) > length )
    {
	int place = strlen( buf );

	while ( strlen_color( buf ) > length )
	    buf[--place] = '\0';

	if ( buf[place-1] == '{' && buf[place-2] != '{' )
	    buf[place-1] = '\0';

	strcat( buf, "{x" );
	return buf;
    }

    while( strlen_color( buf ) < length )
    {
	if ( !place )
	    strcat( buf, " " );
	else
	{
	    sprintf( buf2, " %s", buf );
	    strcpy( buf, buf2 );
	}
	place = !place;
    }

    return buf;
}

char *end_string( const char *txt, int length )
{
    static char buf[MAX_STRING_LENGTH];

    if ( txt == NULL )
	strcpy( buf, "(Null)" );
    else
	strcpy( buf, txt );

    if ( length <= 0 )
    {
	bug( "End_string: invalid length!", 0 );
	return buf;
    }

    if ( strlen_color( buf ) < length )
    {
	while ( strlen_color( buf ) < length )
	    strcat( buf, " " );
    } else {
	int place = strlen( buf );

	while ( strlen_color( buf ) > length )
	    buf[--place] = '\0';

	if ( buf[place-1] == '{' && buf[place-2] != '{' )
	    buf[place-1] = '\0';

	strcat( buf, "{x" );
    }

    return buf;
}

char *begin_string( const char *txt, int length )
{
    static char buf[MAX_STRING_LENGTH];
    char str[MAX_STRING_LENGTH];

    if ( txt == NULL )
	strcpy( buf, "(Null)" );
    else
	strcpy( buf, txt );

    if ( length <= 0 )
    {
	bug( "Begin_string: invalid length!", 0 );
	return buf;
    }

    if ( strlen_color( buf ) < length )
    {
	while ( strlen_color( buf ) < length )
	{
	    sprintf( str, " %s", buf );
	    strcpy( buf, str );
	}
    } else {
	int place = strlen( buf );

	while ( strlen_color( buf ) > length )
	    buf[--place] = '\0';

	if ( buf[place-1] == '{' && buf[place-2] != '{' )
	    buf[place-1] = '\0';

	strcat( buf, "{x" );
    }

    return buf;
}

char *strip_color( const char *argument )
{
    static char buf[MAX_INPUT_LENGTH];
    char *str;

    buf[0] = '\0';
    str = buf;

    for ( ; *argument != '\0'; argument++ )
    {
	if ( *argument == '{' )
	{
	    argument++;
	    continue;
	}

	*str++ = *argument;
    }

    *str++ = '\0';

    return buf;
}

char *strip_spaces( const char *argument )
{
    static char buf[MAX_INPUT_LENGTH];
    char *str;

    buf[0] = '\0';
    str = buf;

    for ( ; *argument != '\0'; argument++ )
    {
	if ( *argument != ' ' )
	    *str++ = *argument;
    }

    *str++ = '\0';

    return buf;
}

char *strip_caps( const char *argument )
{
    static char buf[MAX_INPUT_LENGTH];
    char *str;

    buf[0] = '\0';
    str = buf;

    for ( ; *argument != '\0'; argument++ )
    {
	if ( isalpha( *argument ) )
	    *str++ = LOWER( *argument );
	else
	    *str++ = *argument;
    }

    *str++ = '\0';

    return buf;
}

char *next_arg( char *argument, char *arg_first )
{
    while ( *argument != '\0' )
    {
	*arg_first++ = *argument++;

	if ( *argument == ' ' )
	    break;
    }

    *arg_first = '\0';

    return argument;
}

char *length_argument( char *argument, char *arg_first, int length )
{
    char next[MAX_INPUT_LENGTH];

    *arg_first = '\0';

    while( *argument )
    {
	next_arg( argument, next );

	if ( strlen_color( next ) > length )
	{
	    strcpy( arg_first, "{RHey dumbass, how about some SPACES between your words!{x" );
	    return "";
	}

	if ( strlen_color( arg_first ) + strlen_color( next ) > length )
	    break;

	strcat( arg_first, next );

	argument = next_arg( argument, next );
    }

    while ( isspace( *argument ) )
	argument++;

    return argument;
}
