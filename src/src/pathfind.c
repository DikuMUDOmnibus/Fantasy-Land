/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/*************************************************************************** 
*       ROT 1.4 is copyright 1996-1997 by Russ Walsh                       * 
*       By using this code, you have agreed to follow the terms of the     * 
*       ROT license, in the file doc/rot.license                           * 
***************************************************************************/

/*
 * Pathfinding code by Zarko/Nico
 */
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "merc.h"

// Structure definitions
typedef struct room_node
{
    struct room_node *next;
    struct room_node *previous;
    ROOM_INDEX_DATA *room;
} ROOM_NODE;

typedef struct room_list
{
    ROOM_NODE *begin;
} ROOM_LIST;

typedef bool	PATHFIND_CONDITION	args( ( ROOM_INDEX_DATA *room ) );
#define PF_CONDITION( fun )		bool fun ( ROOM_INDEX_DATA *room )

// Global variables to pathfind.c
AREA_DATA *pathfind_area;
PATHFIND_CONDITION *pathfind_func;
ROOM_INDEX_DATA *pathfind_end;
ROOM_LIST *pathfind_toProcess;
ROOM_LIST *pathfind_processed;
ROOM_NODE *room_node_free;
char pathfind_path[MAX_STRING_LENGTH];

// Memory stuff
ROOM_NODE * new_room_node( ROOM_INDEX_DATA *room )
{
    ROOM_NODE *node;
    
    if ( room_node_free == NULL )
	node		= malloc( sizeof( ROOM_NODE ) );
    else
    {
	node		= room_node_free;
	room_node_free	= node->next;
    }

    // No need to initalise the previous/next values
    node->room	    = room;
    
    return node;
}

void free_room_node( ROOM_NODE *node )
{
    node->next	    = room_node_free;
    room_node_free  = node;
}

// List stuff
bool rl_empty( ROOM_LIST *list )
{
    return list->begin == NULL;
}

ROOM_INDEX_DATA * rl_pop_back( ROOM_LIST *list )
{
    ROOM_NODE *node;
    ROOM_INDEX_DATA *room;
    
    if ( rl_empty( list ) )
	return NULL;

    if ( ( node = list->begin->previous ) == list->begin )
	list->begin = NULL;
    else
    {
	list->begin->previous	    = node->previous;
	list->begin->previous->next = list->begin;
    }
    
    room    = node->room;
    free_room_node( node );
    return room;
}

ROOM_INDEX_DATA * rl_peek_back( ROOM_LIST *list )
{
    if ( list->begin == NULL )
	return NULL;

    return list->begin->previous->room;
}

void rl_push_back( ROOM_LIST *list, ROOM_INDEX_DATA *room )
{
    ROOM_NODE *node = new_room_node( room );
    
    if ( list->begin == NULL )
    {
	list->begin	= node;
	node->next	= node;
	node->previous	= node;
	return;
    }
    
    node->next			= list->begin;
    node->previous		= list->begin->previous;
    list->begin->previous->next	= node;
    list->begin->previous	= node;
}

ROOM_INDEX_DATA * rl_pop_front( ROOM_LIST *list )
{
    ROOM_NODE *node;
    ROOM_INDEX_DATA *room;
    
    if ( rl_empty( list ) )
	return NULL;

    if ( ( node = list->begin ) == list->begin->next )
	list->begin = NULL;
    else
    {
	node->next->previous	= node->previous;
	node->previous->next	= node->next;
	list->begin		= node->next;
    }
    
    room    = node->room;
    free_room_node( node );
    return room;
}

ROOM_INDEX_DATA * rl_peek_front( ROOM_LIST *list )
{
    if ( list->begin == NULL )
	return NULL;

    return list->begin->room;
}

void rl_push_front( ROOM_LIST *list, ROOM_INDEX_DATA *room )
{
    rl_push_back( list, room );
    list->begin	= list->begin->previous;
}


// The main beef
void clean_room_pathfind( ROOM_LIST *list )
{
    ROOM_INDEX_DATA *room;
    
    if ( list == NULL )
	return;
	
    while ( ( room = rl_pop_front( list ) ) != NULL )
	room->visited_new	= NULL;
}

void cleanup_pathfind( )
{
    clean_room_pathfind( pathfind_toProcess );
    clean_room_pathfind( pathfind_processed );
    pathfind_end    = NULL;
    pathfind_func   = NULL;
    pathfind_area   = NULL;
}

void init_pathfind( ROOM_INDEX_DATA *end, PATHFIND_CONDITION *func )
{
    if ( pathfind_toProcess == NULL )
	pathfind_toProcess	= malloc( sizeof( ROOM_LIST ) );
    pathfind_toProcess->begin	= NULL;
    
    if ( pathfind_processed == NULL )
	pathfind_processed	= malloc( sizeof( ROOM_LIST ) );
    pathfind_processed->begin	= NULL;
    
    pathfind_end		= end;
    pathfind_func		= func;
    pathfind_path[0]		= '\0';
}

void add_to_process( ROOM_INDEX_DATA *parent, ROOM_INDEX_DATA *room )
{
    if ( room->visited_new
    ||   IS_SET( room->area->area_flags, AREA_UNLINKED ) )
	return;
	
    room->visited_new   = parent;
    rl_push_back( pathfind_toProcess, room );
}

char get_direction_to( ROOM_INDEX_DATA *source, ROOM_INDEX_DATA *destination )
{
    char directions[MAX_DIR+1] = { 'n', 'e', 's', 'w', 'u', 'd', '\0' };
    int i;
    
    for ( i = 0; i < MAX_DIR; i++ )
    {
	if ( source->exit[i] && source->exit[i]->u1.to_room == destination )
	    return directions[i];
    }
	    
    return ' ';
}

char * combine_directions( char *directions )
{
    static char combined[MAX_STRING_LENGTH];
    char buf[5];
    int i, count = 0;
    char c = directions[0];
    
    combined[0]	= '\0';
    
    for ( i = 0; directions[i] != '\0'; count = 0 )
    {
	while ( directions[i] == c )
	{
	    count++;
	    i++;
	}

	if ( count > 1 )
	    sprintf( buf, "%d%c", count, c );
	else
	    sprintf( buf, "%c", c );

	strcat( combined, buf );
	c   = directions[i];
    }
    
    return combined;
}

void flip_reverse_it( char *str )
{
    char buf[MAX_STRING_LENGTH];
    int i, length = strlen( str );
    
    strcpy( buf, str );
    
    for ( i = 0; i < length; i++ )
	str[i]	= buf[length-i-1];
    
    str[length]	= '\0';
}

void build_path( ROOM_INDEX_DATA *room )
{
    int i = 0;
    while ( room->visited_new != room )
    {
	pathfind_path[i++]  = get_direction_to( room->visited_new, room );
	room		    = room->visited_new;
    }
    pathfind_path[i]	    = '\0';
    flip_reverse_it( pathfind_path );
}

void recursive_room_bfs( )
{
    ROOM_INDEX_DATA *room;
    int i;
    
    if ( ( room = rl_pop_front( pathfind_toProcess ) ) == NULL )
	return;
    
    if ( room == pathfind_end )
    {
	rl_push_front( pathfind_processed, room );
	build_path( room );
	return;
    }
    
    for ( i = 0; i < MAX_DIR; i++ )
    {
	if ( room->exit[i]
	&&   room->exit[i]->u1.to_room )
	    add_to_process( room, room->exit[i]->u1.to_room );
    }

    rl_push_front( pathfind_processed, room );
    recursive_room_bfs();
}

void recursive_func_bfs( )
{
    ROOM_INDEX_DATA *room;
    int i;

    if ( (room = rl_pop_front( pathfind_toProcess )) == NULL )
	return;

    if ( ( *pathfind_func )( room ) )
    {
	rl_push_front( pathfind_processed, room );
	build_path( room );
	return;
    }

    for ( i = 0; i < MAX_DIR; i++ )
    {
	if ( room->exit[i]
	&&   room->exit[i]->u1.to_room )
	    add_to_process( room, room->exit[i]->u1.to_room );
    }

    rl_push_front( pathfind_processed, room );
    recursive_func_bfs( );
}

void bfs_pathfind( ROOM_INDEX_DATA *start, ROOM_INDEX_DATA *end, PATHFIND_CONDITION *func )
{
    init_pathfind( end, func );
    
    if ( ( start == NULL || end == NULL ) && func == NULL )
	return;

    start->visited_new = start;
    rl_push_front( pathfind_toProcess, start );

    if ( end == NULL )
	recursive_func_bfs( );
    else
	recursive_room_bfs( );

    cleanup_pathfind( );
}

PF_CONDITION( check_area )
{
    return room->area != NULL && room->area == pathfind_area;
}

char *pathfind( ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to, AREA_DATA *pArea, bool combine )
{
    PATHFIND_CONDITION *func = NULL;

    if ( pArea != NULL )
    {
	pathfind_area = pArea;
	func = &check_area;
    }
    
    bfs_pathfind( from, to, func );
    
    if ( pathfind_path[0] == '\0' )
	return NULL;
    else if ( combine )
	return combine_directions( pathfind_path );
    else
	return pathfind_path;
}

void do_lookup( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea = NULL;
    ROOM_INDEX_DATA *target = NULL;
    char buf[MAX_STRING_LENGTH];
    char *path;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: lookup <area>\n\r", ch );

	if ( IS_IMMORTAL( ch ) )
	    send_to_char( "        lookup <vnum>\n\r", ch );
	return;
    }
    
    if ( ch->in_room == NULL )
	return;

    if ( IS_IMMORTAL( ch ) && is_number( argument ) )
    {
	int vnum = atoi( argument );
	
	if ( ( target = get_room_index( vnum ) ) == NULL )
	{
	    send_to_char( "The room vnum does not exist.\n\r", ch );
	    return;
	}
	
	if ( target == ch->in_room )
	{
	    send_to_char( "You're there already!\n\r", ch );
	    return;
	}
	
	sprintf( buf, "Directions to %d: ", vnum );
	send_to_char( buf, ch );
    } else {
        if ( ( pArea = area_lookup( ch, argument ) ) == NULL )
	{
	    send_to_char( "Area not found.\n\r", ch );
	    return;
	}

	if ( pArea == ch->in_room->area )
	{
	    send_to_char( "You're there already!\n\r", ch );
	    return;
	}

	sprintf( buf, "Directions to %s{x: ", pArea->name );
	send_to_char( buf, ch );
    }

    if ( ( path = pathfind( ch->in_room, target, pArea, TRUE ) ) == NULL )
	send_to_char( "Cannot find path.\n\r", ch );
    else
    {
	send_to_char( path, ch );
	send_to_char( "\n\r", ch );
    }
}

void do_jog( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char buf[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];
    char *p;
    bool dFound = FALSE;

    if ( !ch->desc || *argument == '\0' )
    {
	send_to_char( "You run in place!\n\r", ch );
	return;
    }

    if ( !IS_NPC( ch ) && ch->pcdata->pktimer > 0 )
    {
	send_to_char( "No running with a pktimer.\n\r", ch );
	return;
    }

    if ( ( pArea = area_lookup( ch, argument ) ) != NULL )
    {
	if ( !IS_IMMORTAL( ch ) && ch->in_room && ch->in_room->vnum != 3014 )
	{
	    send_to_char( "Running to areas can only be done from Market Square.\n\r", ch );
	    return;
	} else {
	    if ( ch->in_room != NULL
	    &&   ch->in_room->area == pArea )
	    {
		send_to_char( "You are already there!\n\r", ch );
		return;
	    }

	    if ( IS_SET( pArea->area_flags, AREA_NO_RUN ) )
	    {
		sprintf( buf, "%s can not be run to.\n\r", pArea->name );
		send_to_char( buf, ch );
		return;
	    }

	    if ( pArea->directions != NULL )
		argument = pArea->directions;

	    else if ( ( argument = pathfind( ch->in_room, NULL, pArea, TRUE ) ) == NULL )
	    {
		sprintf( buf, "%s is not reachable from here.\n\r", pArea->name );
		send_to_char( buf, ch );
		return;
	    }
	}
    }

    buf[0] = '\0';

    while ( *argument != '\0' )
    {
	argument = one_argument( argument, arg );
	strcat( buf, arg );
    }

    for( p = buf + strlen( buf ) - 1; p >= buf; p-- )
    {
	if ( !isdigit( *p ) )
	{
	    switch( *p )
	    {
		case 'n':
		case 's':
		case 'e':
		case 'w':
		case 'u':
		case 'd': dFound = TRUE;
                break;

		case 'o': break;

		default: send_to_char( "Invalid direction!\n\r", ch );
		return;
	    }
	}

	else if ( !dFound )
	    *p = '\0';
    }

    if ( !dFound )
    {
	send_to_char( "No directions specified!\n\r", ch );
	return;
    }

    ch->desc->run_buf = str_dup( buf );
    ch->desc->run_head = ch->desc->run_buf;

    if ( pArea != NULL )
    {
	sprintf( buf, "You start running to %s.\n\r", pArea->name );
	send_to_char( buf, ch );
    }
    else
	send_to_char( "You start running...\n\r", ch );

    return;
}

void do_stalk( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    char *path;
    sh_int direction;

    if ( IS_NPC(ch)
    ||   get_skill( ch, gsn_stalk )  <= 0 )
    {
	send_to_char("Huh?\n\r",ch);
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Whom are you trying to hunt?\n\r", ch );
	return;
    }
  
    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	send_to_char("No-one around by that name.\n\r", ch );
	return;
    }

    if ( ch->in_room == victim->in_room )
    {
	act( "$N is here!", ch, NULL, victim, TO_CHAR, POS_RESTING );
	return;
    }

    if ( !cost_of_skill( ch, gsn_stalk ) )
	return;

    act( "$n carefully sniffs the air.", ch, NULL, NULL, TO_ROOM, POS_RESTING );

    if ( ( path = pathfind( ch->in_room, victim->in_room, NULL, FALSE ) ) == NULL )
    {
	act( "You couldn't find a path to $N from here.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return;
    }

    if ( get_skill( ch, gsn_stalk ) < number_percent( ) )
	direction = number_range( 1, 6 );
    else
    {
	switch ( *path )
	{
	    default:
		send_to_char( "Error!\n\r", ch );
		return;

	    case 'n':	direction = 0;	break;
	    case 'e':	direction = 1;	break;
	    case 's':	direction = 2;	break;
	    case 'w':	direction = 3;	break;
	    case 'u':	direction = 4;	break;
	    case 'd':	direction = 5;	break;
	}
    }

    sprintf( buf, "$N is %s from here.", dir_name[direction] );
    act( buf, ch, NULL, victim, TO_CHAR, POS_RESTING );

    check_improve( ch, gsn_stalk, TRUE, 1 );
    return;
}

