#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"

#define MOB_VNUM_ARENA_HEALER	55200
#define ROOM_VNUM_AWINNER	55242
#define ROOM_VNUM_ALOSER	55243
#define ROOM_VNUM_ARENA_MIN	55200
#define ROOM_VNUM_ARENA_MAX	55240
#define ROOM_VNUM_ARENA_PREP	55241

DECLARE_DO_FUN( do_look		);

ARENA_DATA *arena_matches, *arena_free;

ARENA_DATA *new_arena( void )
{
    ARENA_DATA *match;
    sh_int pos;

    if ( arena_free == NULL )
	match		= alloc_perm( sizeof( *match ) );
    else
    {
	match		= arena_free;
	arena_free	= arena_free->next;
    }

    for ( pos = 0; pos < MAX_ARENA_TEAMS; pos++ )
    {
	match->team_name[pos][0] = '\0';
	match->team[pos]	 = NULL;
    }

    match->next		= NULL;
    match->specials	= 0;

    match->disabled_skills = new_bool( maxSkill, FALSE );

    if ( arena_matches == NULL )
	arena_matches = match;
    else
    {
	ARENA_DATA *arena;

	for ( arena = arena_matches; arena != NULL; arena = arena->next )
	{
	    if ( arena->next == NULL )
	    {
		arena->next = match;
		break;
	    }
	}
    }

    return match;
}

void free_arena( ARENA_DATA *match )
{
    if ( match == arena_matches )
	arena_matches = arena_matches->next;
    else
    {
	ARENA_DATA *arena;

	for ( arena = arena_matches; arena != NULL; arena = arena->next )
	{
	    if ( arena->next == match )
	    {
		arena->next = match->next;
		break;
	    }
	}
    }

    free_bool( match->disabled_skills );

    match->next	= arena_free;
    arena_free	= match;

    if ( arena_matches == NULL )
	arena_match_count = 0;
}

struct	info_type	arena_specials	[]	=
{
    { "-potions",	ARENA_NO_POTION		},
    { "-scrolls",	ARENA_NO_SCROLL		},
    { "-pills",		ARENA_NO_PILL		},
    { "-flee",		ARENA_NO_FLEE		},
    { "+healer",	ARENA_PLUS_HEALER	},
    { NULL,		0			}
};

char * specials_bit_name( ARENA_DATA *match )
{
    static char buf[MAX_STRING_LENGTH];
    sh_int pos;

    buf[0] = '\0';

    if ( match->specials & ARENA_NO_POTION	) strcat( buf, ", no_potions"  );
    if ( match->specials & ARENA_NO_SCROLL	) strcat( buf, ", no_scrolls"  );
    if ( match->specials & ARENA_NO_PILL	) strcat( buf, ", no_pills"    );
    if ( match->specials & ARENA_NO_FLEE	) strcat( buf, ", no_flee"     );
    if ( match->specials & ARENA_PLUS_HEALER	) strcat( buf, ", with_healer" );

    for ( pos = 0; skill_table[pos].name[0] != '\0'; pos++ )
    {
	if ( match->disabled_skills[pos] == TRUE )
	{
	    strcat( buf, ", no_" );
	    strcat( buf, skill_table[pos].name );
	}
    }

    return ( buf[0] != '\0' ) ? buf+1 : " none";
}

ARENA_DATA *arena_lookup( sh_int number )
{
    ARENA_DATA *match;

    for ( match = arena_matches; match != NULL; match = match->next )
    {
	if ( match->number == number )
	    return match;
    }

    return NULL;
}

void clear_challenge( CHAR_DATA *wch[MAX_ARENA_TEAMS] )
{
    CHAR_DATA *vch, *vch_next;
    sh_int count;

    for ( count = 0; count < MAX_ARENA_TEAMS; count++ )
    {
	for ( vch = wch[count]; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->pcdata->next_arena;

	    vch->pcdata->next_arena = NULL;
	}
    }
}

void do_challenge( CHAR_DATA *ch, char *argument )
{
    ARENA_DATA *match;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *wch, *teams[MAX_ARENA_TEAMS];
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    sh_int count, people = 1;
    long specials = 0;
    bool team = FALSE, show_spec = FALSE;
    bool skills[maxSkill];

    if ( IS_NPC( ch ) )
	return;

    if ( IS_SET( ch->comm,COMM_NOARENA ) )
    {
	send_to_char( "You can't hear the arena so therefore you can't use it!\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "You must provide at least one opponent.\n\r", ch );
	return;
    }

    if ( ch->in_room->vnum == ROOM_VNUM_CORNER && !IS_IMMORTAL( ch ) )
    {
	send_to_char( "Just keep your nose in the corner like a good little player.\n\r", ch );
	return;
    }

    if ( ch->pcdata->pktimer > 0 || ch->pcdata->opponent != NULL )
    {
	send_to_char( "You can't arena while being hunted!\n\r", ch );
	return;
    }

    if ( ch->pcdata->match != NULL )
    {
	send_to_char( "You are already involved in a match.\n\r", ch );
	return;
    }

    if ( !check_builder( ch, get_room_index( ROOM_VNUM_ARENA_PREP )->area, TRUE ) )
	return;

    for ( count = 0; count < MAX_ARENA_TEAMS; count++ )
	teams[count] = NULL;

    for ( count = 0; count < maxSkill; count++ )
	skills[count] = FALSE;

    teams[0] = ch;

    for ( ; ; )
    {
	CHAR_DATA *vch;
	char arg[MAX_INPUT_LENGTH];
	bool found = FALSE;

	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	    break;

	for ( count = 0; arena_specials[count].name != NULL; count++ )
	{
	    if ( !str_prefix( arg, arena_specials[count].name ) )
	    {
		SET_BIT( specials, arena_specials[count].flag );
		show_spec = TRUE;
		found = TRUE;
		break;
	    }
	}

	if ( found )
	    continue;

	if ( arg[0] == '-' )
	{
	    if ( ( count = skill_lookup( arg+1 ) ) == -1 )
	    {
		printf_to_char( ch, "Invalid skill or special flag: %s.\n\r", arg );
		clear_challenge( teams );
		return;
	    }

	    skills[count] = TRUE;
	    show_spec = TRUE;
	    continue;
	}

	if ( !str_cmp( arg, "&" ) )
	{
	    if ( teams[people] == NULL )
	    {
		sprintf( buf, "You must select at least 1 opponent for team %d before moving to team %d.\n\r",
		    people+1, people+2 );
		send_to_char( buf, ch );
		clear_challenge( teams );
		return;
	    }

	    if ( ++people >= MAX_ARENA_TEAMS )
	    {
		send_to_char( "That exceedes the max arena teams.\n\r", ch );
		clear_challenge( teams );
		return;
	    }

	    team = FALSE;
	    continue;
	}

	if ( !str_cmp( arg, "with" ) )
	{
	    team = TRUE;
	    continue;
	}

	if ( ( wch = get_pc_world( ch, arg ) ) == NULL )
	{
	    act( "Competitor $t not found.", ch, arg, NULL, TO_CHAR, POS_RESTING );
	    clear_challenge( teams );
	    return;
	}

	if ( wch == ch )
	{
	    send_to_char( "You can't challenge yourself!\n\r", ch );
	    clear_challenge( teams );
	    return;
	}

	if ( wch->desc == NULL )
	{
	    act( "$N appears to be linkdead right now.",
		ch, NULL, wch, TO_CHAR, POS_RESTING );
	    clear_challenge( teams );
	    return;
	}

	if ( IS_SET( wch->comm, COMM_NOARENA ) )
	{
	    act( "$N appears to be ignoring all arena challenges.",
		ch, NULL, wch, TO_CHAR, POS_RESTING );
	    clear_challenge( teams );
	    return;
	}

	if ( IS_SET( wch->comm, COMM_AFK ) )
	{
	    act( "$N is currently AFK.", ch, NULL, wch, TO_CHAR, POS_RESTING );
	    clear_challenge( teams );
	    return;
	}

	if ( wch->in_room->vnum == ROOM_VNUM_CORNER && !IS_IMMORTAL( wch ) )
	{
	    act( "$N is in the corner!", ch, NULL, wch, TO_CHAR, POS_RESTING );
	    clear_challenge( teams );
	    return;
	}

	if ( !check_builder( wch, get_room_index( ROOM_VNUM_ARENA_PREP )->area, FALSE ) )
	{
	    act( "$N looks a little busy...", ch, NULL, wch, TO_CHAR, POS_RESTING );
	    clear_challenge( teams );
	    return;
	}

	if ( wch->pcdata->pktimer > 0 || wch->pcdata->opponent != NULL )
	{
	    act( "$N can't arena while being hunted!",
		ch, NULL, wch, TO_CHAR, POS_RESTING );
	    clear_challenge( teams );
	    return;
	}

	if ( wch->pcdata->match != NULL )
	{
	    act( "$N is already involved in an arena match.",
		ch, NULL, wch, TO_CHAR, POS_RESTING );
	    clear_challenge( teams );
	    return;
	}

	for ( count = 0; count < MAX_ARENA_TEAMS; count++ )
	{
	    for ( vch = teams[count]; vch != NULL; vch = vch->pcdata->next_arena )
	    {
		if ( vch == wch )
		{
		    send_to_char( "Each person can be named only once.\n\r", ch );
		    clear_challenge( teams );
		    return;
		}
	    }
	}

	if ( team )
	{
	    wch->pcdata->next_arena	= teams[0];
	    teams[0]			= wch;
	} else {
	    wch->pcdata->next_arena	= teams[people];
	    teams[people]		= wch;
	}
    }

    if ( teams[1] == NULL )
    {
	send_to_char( "No opponent was challenged.\n\r", ch );
	clear_challenge( teams );
	return;
    }

    match		= new_arena( );
    match->number	= ++arena_match_count;
    match->specials	= specials;

    for ( count = 0; count < maxSkill; count++ )
	match->disabled_skills[count] = skills[count];

    for ( count = 0; count < MAX_ARENA_TEAMS; count++ )
    {
	if ( teams[count] == NULL )
	    break;

	match->team[count]		= teams[count];
	match->team_name[count][0]	= '\0';

	for ( wch = match->team[count]; wch != NULL; wch = wch->pcdata->next_arena )
	{
	    sprintf( buf, " %s", wch->name );
	    strcat( match->team_name[count], buf );

	    wch->pcdata->match		= match;
	    wch->pcdata->team		= count;
	    wch->arena_number		= match->number;
	}
    }

    sprintf( buf, "{W<{GARENA{W> [{G%d{W] Issued{w: {BTeam1 {R({W%s{R)",
	match->number, match->team_name[0]+1 );

    for ( count = 1; count < MAX_ARENA_TEAMS; count++ )
    {
	if ( match->team[count] == NULL )
	    break;

	sprintf( buf2, "{w, {BTeam%d {R({W%s{R)",
	    count+1, match->team_name[count]+1 );
	strcat( buf, buf2 );
    }

    strcat( buf, "{w!{x\n\r" );

    if ( show_spec )
    {
	sprintf( buf2, "{W<{GARENA{W> [{G%d{W] {BSpecial conditions{w:{W%s{B.{x\n\r",
		match->number, specials_bit_name( match ) );
	strcat( buf, buf2 );
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   !IS_SET( d->character->comm, COMM_NOARENA ) )
	{
            send_to_char( buf, d->character );

	    if ( IS_NPC( d->character ) || d->character == ch )
		continue;

	    if ( d->character->pcdata->match == match )
	    {
		sprintf( buf2, "{W<{GARENA{W> {xYou have been selected to compete on team number %d.\n\r",
		    d->character->pcdata->team+1 );
		send_to_char( buf2, d->character );
		send_to_char( "Type {GACCEPT{x to join.\n\r", d->character );
		send_to_char( "Type {GDECLINE{x to decline.\n\r", d->character );
	    }
	}
    }

    if ( ch->in_room != NULL )
	ch->pcdata->was_in_room = ch->in_room;

    char_from_room( ch );
    char_to_room( ch, get_room_index( ROOM_VNUM_ARENA_PREP ) );

    if ( ch->pet != NULL )
    {
	char_from_room( ch->pet );
	char_to_room( ch->pet, ch->in_room );
    }

    do_look( ch, "auto" );
    return;
}

void arena_start( ARENA_DATA *match )
{
    CHAR_DATA *wch, *wch_next;
    OBJ_DATA *obj, *obj_next;
    DESCRIPTOR_DATA *d;
    ROOM_INDEX_DATA *pRoom;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int count, highest_level = 0;

    for ( count = 0; count < MAX_ARENA_TEAMS; count++ )
    {
	if ( match->team[count] == NULL )
	    break;

	for ( wch = match->team[count]; wch != NULL; wch = wch->pcdata->next_arena )
	{
	    if ( wch->in_room->vnum != ROOM_VNUM_ARENA_PREP )
		return;
	}
    }

    sprintf( buf, "{W<{GARENA{W> [{G%d{W] Accepted{w: {BTeam1 {R({W%s{R)",
	match->number, match->team_name[0]+1 );

    for ( count = 1; count < MAX_ARENA_TEAMS; count++ )
    {
	if ( match->team[count] == NULL )
	    break;

	sprintf( buf2, "{w, {BTeam%d {R({W%s{R)",
	    count+1, match->team_name[count]+1 );
	strcat( buf, buf2 );
    }

    strcat( buf, "{w!{x\n\r" );

    for ( count = 0; count < maxSkill; count++ )
    {
	if ( match->disabled_skills[count] == TRUE )
	{
	    SET_BIT( match->specials, ARENA_PROGRESSING );
	    break;
	}
    }

    if ( match->specials )
    {
	sprintf( buf2, "{W<{GARENA{W> [{G%d{W] {BSpecial conditions{w:{W%s{B.{x\n\r",
	    match->number, specials_bit_name( match ) );
	strcat( buf, buf2 );
    }

    SET_BIT( match->specials, ARENA_PROGRESSING );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   !IS_SET( d->character->comm,COMM_NOARENA ) )
	    send_to_char( buf, d->character );
    }

    for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
    {
	if ( wch->pcdata->match != match )
	    continue;

	save_char_obj( wch, 1 );

	highest_level		= UMAX( highest_level, wch->level );
	wch->pcdata->dtimer	= 0;
	wch->hit		= wch->max_hit;
	wch->mana		= wch->max_mana;
	wch->move		= wch->max_move;
	update_pos( wch );

	char_from_room( wch );
	char_to_room( wch, get_room_index(
	    number_range( ROOM_VNUM_ARENA_MIN, ROOM_VNUM_ARENA_MAX ) ) );

	if ( wch->pet != NULL )
	{
	    char_from_room( wch->pet );
	    char_to_room( wch->pet, wch->in_room );
	    wch->pet->arena_number = match->number;
	}

	act( "$n arrives to test $s skills!",
	    wch, NULL, NULL, TO_ROOM, POS_RESTING );
	do_look( wch, "auto" );
    }

    for ( count = ROOM_VNUM_ARENA_MIN-3; count < ROOM_VNUM_ARENA_MAX; count++ )
    {
	if ( ( pRoom = get_room_index( count ) ) != NULL )
	{
	    for ( wch = pRoom->people; wch != NULL; wch = wch_next )
	    {
		wch_next = wch->next_in_room;

		if ( wch->pcdata == NULL && wch->arena_number == 0 )
		    extract_char( wch, TRUE );
	    }

	    for ( obj = pRoom->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;

		if ( obj->arena_number == 0 )
		{
		    obj_from_room( obj );
		    extract_obj( obj );
		}
	    }
	}
    }

    if ( IS_SET( match->specials, ARENA_PLUS_HEALER ) )
    {
	if ( ( wch = create_mobile( get_mob_index( MOB_VNUM_ARENA_HEALER ) ) ) == NULL )
	{
	    bug( "Arena Healer: NULL MOB!", 0 );
	    return;
	}

	wch->level = highest_level;
	wch->arena_number = match->number;
	char_to_room( wch, get_room_index(
	    number_range( ROOM_VNUM_ARENA_MIN, ROOM_VNUM_ARENA_MAX ) ) );
    }
}

void do_accept( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
        return;

    if ( ch->pcdata->match == NULL )
    {
	send_to_char( "You aren't involved in a current arena match.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->pcdata->match->specials, ARENA_PROGRESSING ) )
    {
	send_to_char( "The match has already begun!\n\r", ch );
	return;
    }

    if ( ch->in_room != NULL && ch->in_room->vnum == ROOM_VNUM_ARENA_PREP )
    {
	send_to_char( "You are already in this challenge!\n\r", ch );
	return;
    }

    if ( ch->in_room->vnum == ROOM_VNUM_CORNER && !IS_IMMORTAL( ch ) )
    {
	send_to_char( "Just keep your nose in the corner like a good little player.\n\r", ch );
	return;
    }

    sprintf( buf, "{W<{GARENA{W> [{G%d{W]{x %s accepts the current challenge on behalf of team %d!\n\r",
	ch->pcdata->match->number, ch->name, ch->pcdata->team+1 );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   !IS_SET( d->character->comm,COMM_NOARENA ) )
	    send_to_char( buf, d->character );
    }

    if ( ch->in_room != NULL )
	ch->pcdata->was_in_room = ch->in_room;

    char_from_room( ch );
    char_to_room( ch, get_room_index( ROOM_VNUM_ARENA_PREP ) );

    if ( ch->pet != NULL )
    {
	char_from_room( ch->pet );
	char_to_room( ch->pet, ch->in_room );
    }

    do_look( ch, "auto" );
    arena_start( ch->pcdata->match );
}

void do_decline( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( IS_NPC( ch ) )
        return;

    if ( ch->pcdata->match == NULL )
    {
	send_to_char( "You are not involved in an arena challenge.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->pcdata->match->specials, ARENA_PROGRESSING ) )
    {
	send_to_char( "The match has already begun!\n\r", ch );
	return;
    }

    sprintf( buf, "{W<{GARENA{W> [{G%d{W]{x Current challenge aborted, declined by %s of the team %d!\n\r",
	ch->pcdata->match->number, ch->name, ch->pcdata->team+1 );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   !IS_SET( d->character->comm, COMM_NOARENA ) )
	    send_to_char( buf, d->character );
    }

    arena_clear( ch->pcdata->match );
}

void arena_clear( ARENA_DATA *match )
{
    ARENA_DATA *arena;
    CHAR_DATA *wch, *wch_next;
    OBJ_DATA *obj, *obj_next;
    ROOM_INDEX_DATA *pRoom;
    int count;

    for ( arena = arena_matches; arena != NULL; arena = arena->next )
    {
	if ( match == NULL || arena == match )
	{
	    for ( count = 0; count < MAX_ARENA_TEAMS; count++ )
	    {
		for ( wch = arena->team[count]; wch != NULL; wch = wch_next )
		{
		    wch_next = wch->pcdata->next_arena;

		    wch->arena_number		= 0;
		    wch->pcdata->next_arena	= NULL;
		    wch->pcdata->match		= NULL;
		    wch->pcdata->team		= 0;

		    if ( wch->in_room != NULL
		    &&   ( wch->in_room->vnum == ROOM_VNUM_ARENA_PREP
		    ||     IS_SET( wch->in_room->room_flags,ROOM_ARENA ) ) )
		    {
			if ( wch->fighting != NULL )
			    stop_fighting( wch, TRUE );

			char_from_room( wch );
			if ( wch->pcdata->was_in_room != NULL )
			    char_to_room( wch, wch->pcdata->was_in_room );
			else
			    char_to_room( wch, get_room_index( ROOM_VNUM_ALTAR ) );

			if ( wch->pet != NULL )
			{
			    char_from_room( wch->pet );
			    char_to_room( wch->pet, wch->in_room );
			}

			do_look( wch, "auto" );
		    }
		}
	    }

	    free_arena( arena );
	}
    }

    for ( count = ROOM_VNUM_ARENA_MIN-3; count < ROOM_VNUM_ARENA_MAX; count++ )
    {
	if ( ( pRoom = get_room_index( count ) ) != NULL )
	{
	    for ( wch = pRoom->people; wch != NULL; wch = wch_next )
	    {
		wch_next = wch->next_in_room;

		if ( wch->pcdata == NULL
		&&   ( match == NULL || wch->arena_number == match->number ) )
		    extract_char( wch, TRUE );
	    }

	    for ( obj = pRoom->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;

		if ( match == NULL || obj->arena_number == match->number )
		{
		    obj_from_room( obj );
		    extract_obj( obj );
		}
	    }
	}
    }
}

void do_arenastatus( CHAR_DATA *ch, char *argument )
{
    ARENA_DATA *match;
    CHAR_DATA *wch;
    char buf[MAX_STRING_LENGTH];
    sh_int count;

    send_to_char( "{GArena Status{w:\n\r{W---------------------------------------------{x\n\r", ch );

    if ( argument[0] == '\0' )
    {
	if ( arena_matches == NULL )
	{
	    send_to_char( "{GNo current challenges are pending.{x\n\r", ch );
	    return;
	}

	for ( match = arena_matches; match != NULL; match = match->next )
	{
	    sprintf( buf, "\n\r {GChallenge {W[{G%d{W]{g:\n\r", match->number );
	    send_to_char( buf, ch );

	    for ( count = 0; count < MAX_ARENA_TEAMS; count++ )
	    {
		if ( match->team[count] == NULL )
		    break;

		sprintf( buf,"    Team %d: %s\n\r", count+1,
		    match->team_name[count] );
		send_to_char( buf, ch );
	    }
	}

	return;
    }

    if ( !is_number( argument ) )
    {
	send_to_char( "\n\rAstat or astat number.\n\r", ch );
	return;
    }

    if ( ( match = arena_lookup( atoi( argument ) ) ) == NULL )
    {
	sprintf( buf, "Match number %d is not in use.\n\r", atoi( argument ) );
	send_to_char( buf, ch );
	return;
    }

    sprintf( buf, "{GSpecial Conditions{w:{W%s{G.\n\r\n\r",
	specials_bit_name( match ) );
    send_to_char( buf, ch );

    for ( count = 0; count < MAX_ARENA_TEAMS; count++ )
    {
	if ( match->team[count] == NULL )
	    break;

	for ( wch = match->team[count]; wch != NULL; wch = wch->pcdata->next_arena )
	{
	    sprintf( buf, "{W[{G%15s{W] [{GTeam %d{W] ", wch->name, count+1 );
	    send_to_char( buf, ch );

	    if ( IS_SET( wch->in_room->room_flags, ROOM_ARENA ) )
		send_to_char( "{BALIVE{x\n\r", ch );

	    else if ( wch->in_room->vnum == ROOM_VNUM_ARENA_PREP )
		send_to_char( "{BACCEPTED{x\n\r", ch );

	    else if ( wch->arena_number == 0 )
		send_to_char( "{YDEAD{x\n\r", ch );

	    else
		send_to_char( "{YWAITING{x\n\r", ch );
	}
    }
}

void do_arena( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {   
	if ( ch->pcdata->match != NULL )
	    send_to_char( "Your committed now, might as well fight it out.\n\r", ch );

	else if ( IS_SET( ch->comm, COMM_NOARENA ) )
	{
	    send_to_char( "You choose to be involved with arena activities.\n\r", ch );
	    REMOVE_BIT( ch->comm, COMM_NOARENA );
	}

	else
	{
	    send_to_char( "You choose not to be involved with arena activities.\n\r", ch );
	    SET_BIT( ch->comm, COMM_NOARENA );
	}

	return;
    }
       
    if ( ch->pcdata->match == NULL )
    {
	send_to_char( "You aren't in the arena!\n\r", ch );
	return;
    }
       
    sprintf( buf, "You [{GArena{x] '{R%s{x'\n\r", argument );
    send_to_char( buf, ch );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim;

        victim = d->original ? d->original : d->character;

        if ( d->connected == CON_PLAYING
        &&   d->character != ch
	&&   !IS_NPC( d->character )
	&&   d->character->pcdata->team == ch->pcdata->team
	&&   d->character->pcdata->match == ch->pcdata->match )
        {
	    sprintf( buf, "%s$n{x [{GArena{x] '{R$t{x'", pretitle( ch, d->character ) );
	    act( buf, ch, argument, d->character, TO_VICT, POS_DEAD );
	}
    }
}

void arena_recover( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *location;

    if ( is_clan( ch ) )
	location = get_room_index( clan_table[ch->clan].hall );

    else if ( ch->alignment < 0 )
	location = get_room_index( ROOM_VNUM_ALTARB );

    else
	location = get_room_index( ROOM_VNUM_ALTAR );

    if ( location == NULL )
    {
	send_to_char( "You found a bug! NULL room!\n\r", ch );
	return;
    }

    if ( ch->in_room != NULL && ch->in_room == location )
	return;

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    if ( ch->in_room != NULL )
    {
	act( "$n exits the arena!", ch, NULL, NULL, TO_ROOM, POS_RESTING );
	char_from_room( ch );
    }

    if ( ch->pcdata->was_in_room != NULL )
	char_to_room( ch, ch->pcdata->was_in_room );
    else
	char_to_room( ch, location );

    if ( ch->pet != NULL )
    {
	char_from_room( ch->pet );
	char_to_room( ch->pet, ch->in_room );
    }

    act( "$n arrives from the arena!", ch, NULL, NULL, TO_ROOM, POS_RESTING );
    do_look( ch, "auto" );
}

void check_arena( CHAR_DATA *ch, CHAR_DATA *victim )
{
    FILE *fp;
    ARENA_DATA *match;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *wch, *next_arena;
    char *tell_string;
    char buf[MAX_STRING_LENGTH];
    char name[MAX_STRING_LENGTH];
    sh_int team, kills, pos, tell_count, count[MAX_ARENA_TEAMS];
    bool found = FALSE;

    if ( IS_NPC( victim ) )
    {
	stop_fighting( victim, TRUE );
	extract_char( victim, TRUE );
	return;
    }

    if ( IS_NPC( ch ) )
    {
	if ( ch->master != NULL && !IS_NPC( ch->master ) )
	    ch = ch->master;
    }

    if ( ( match = arena_lookup( ch->arena_number ) ) == NULL )
	return;

    sprintf( buf, "{W<{GARENA{W> [{G%d{W] {B%s{x, of team {B%d{x, has defeated {B%s{x, of team {B%d{x!\n\r",
	match->number, ch->pcdata ? ch->name : ch->short_descr,
	ch->pcdata ? ch->pcdata->team+1 : 0, victim->name,
	victim->pcdata->team+1 );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   !IS_SET( d->character->comm, COMM_NOARENA ) )
	    send_to_char( buf, d->character );
    }

    stop_fighting( ch, TRUE );
    stop_fighting( victim, TRUE );

    if ( ch->pcdata )
	ch->pcdata->arenakills++;

    sprintf( name, "%sarena/%s", PLAYER_DIR, capitalize( victim->name ) );

    if ( ( fp = fopen( name, "r" ) ) != NULL )
    {
	fclose( fp );
	kills		= victim->pcdata->arenakills;
	next_arena	= victim->pcdata->next_arena;
	team		= victim->pcdata->team;
	d		= victim->desc;
	tell_count	= victim->pcdata->tells;
	tell_string	= victim->pcdata->buffer->string;
	extract_char( victim, TRUE );

	if ( d != NULL )
	{
	    d->character = NULL;
	    load_char_obj( d, victim->name, TRUE, FALSE );
	    victim = d->character;
	    free_string( victim->pcdata->socket );
	    victim->pcdata->socket = str_dup( d->host );
	    racial_spells( victim, TRUE );
	    do_devote_assign( victim );
	} else {
	    d = new_descriptor( );
	    load_char_obj( d, victim->name, TRUE, FALSE );
	    victim = d->character;
	    free_descriptor( d );
	    victim->desc = NULL;
	}

	victim->next		   = char_list;
	char_list		   = victim;
	victim->pcdata->next_player= player_list;
	player_list		   = victim;
	victim->pcdata->arenakills = kills;
	victim->pcdata->next_arena = next_arena;
	victim->pcdata->team	   = team;
	victim->pcdata->match	   = match;

	if ( tell_count )
	{
	    victim->pcdata->tells  = tell_count;
	    add_buf( victim->pcdata->buffer, tell_string );
	}

	if ( victim->pcdata->was_in_room != NULL )
	    char_to_room( victim, victim->pcdata->was_in_room );
	else
	    char_to_room( victim, get_room_index( ROOM_VNUM_ALOSER ) );
    } else {
	victim->hit  = victim->max_hit;
	victim->mana = victim->max_mana;
	victim->move = victim->max_move;
	update_pos( victim );
	char_from_room( victim );
	char_from_room( victim->pet );
	char_to_room( victim, get_room_index( ROOM_VNUM_ALOSER ) );
    }

    if ( victim->pet != NULL )
    {
	char_to_room( victim->pet, victim->in_room );
	victim->pet->arena_number = 0;
    }

    victim->pcdata->arenadeath++;
    victim->arena_number = 0;

    do_look( victim, "auto" );

    if ( victim->pcdata->tells )
    {
	sprintf( buf, "{GYou have {R%d{G tells waiting.\n\r", victim->pcdata->tells );
	send_to_char( buf, victim );
	send_to_char( "Type 'replay' to see tells.{x\n\r", victim );
    }

    for ( pos = 0; pos < MAX_ARENA_TEAMS; pos++ )
	count[pos] = 0;

    for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
    {
	if ( IS_SET( wch->in_room->room_flags, ROOM_ARENA ) )
	{
	    if ( wch->pcdata->match == match )
		count[wch->pcdata->team]++;
	}
    }

    for ( kills = 0; kills < MAX_ARENA_TEAMS; kills++ )
    {
	if ( count[kills] != 0 )
	{
	    if ( found )
		return;

	    found = TRUE;
	    pos = kills;
	}
    }

    if ( found )
    {
	sprintf( buf, "{W<{GARENA{W> [{G%d{W] {BTeam %d {R({W%s{R) {xhas emerged as the victor!\n\r",
	    match->number, pos+1, match->team_name[pos]+1 );

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   !IS_SET( d->character->comm, COMM_NOARENA ) )
		send_to_char( buf, d->character );
	}

	for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
	{
	    if ( IS_SET( wch->in_room->room_flags, ROOM_ARENA ) )
	    {
		if ( wch->pcdata->match == match )
		{
		    stop_fighting( wch, TRUE );
		    sprintf( name, "%sarena/%s",
			PLAYER_DIR, capitalize( wch->name ) );

		    if ( ( fp = fopen( name, "r" ) ) != NULL )
		    {
			fclose( fp );
			kills		= wch->pcdata->arenakills;
			next_arena	= wch->pcdata->next_arena;
			d		= wch->desc;
			extract_char( wch, TRUE );

			if ( d != NULL )
			{
			    d->character = NULL;
			    load_char_obj( d, wch->name, TRUE, FALSE );
			    wch = d->character;
			    free_string( wch->pcdata->socket );
			    wch->pcdata->socket = str_dup( d->host );
			    racial_spells( wch, TRUE );
			    do_devote_assign( wch );
			} else {
			    d = new_descriptor( );
			    load_char_obj( d, wch->name, TRUE, FALSE );
			    wch = d->character;
			    free_descriptor( d );
			    wch->desc = NULL;
			}

			wch->next = char_list;
			char_list = wch;
			wch->pcdata->next_player = player_list;
			player_list = wch;
			wch->pcdata->arenakills = kills;
			wch->pcdata->next_arena = next_arena;

			if ( wch->pcdata->was_in_room != NULL )
			    char_to_room( wch, wch->pcdata->was_in_room );
			else
			    char_to_room( wch, get_room_index( ROOM_VNUM_AWINNER ) );
		    } else {
			wch->hit  = wch->max_hit;
			wch->mana = wch->max_mana;
			wch->move = wch->max_move;
			update_pos( wch );
			char_from_room( wch );
			char_to_room( wch, get_room_index( ROOM_VNUM_AWINNER ) );
		    }

		    if ( wch->pet != NULL )
			char_to_room( wch->pet, wch->in_room );

		    do_look( wch, "auto" );

		    if ( wch->pcdata->tells )
		    {
			sprintf( buf, "{GYou have {R%d{G tells waiting.\n\r", wch->pcdata->tells );
			send_to_char( buf, wch );
			send_to_char( "Type 'replay' to see tells.{x\n\r", wch );
		    }
		}
	    }
	}

	for ( kills = 0; kills < MAX_ARENA_TEAMS; kills++ )
	{
	    for ( wch = match->team[kills]; wch != NULL; wch = wch->pcdata->next_arena )
	    {
		if ( kills == pos )
		    wch->pcdata->arenawins++;
		else
		    wch->pcdata->arenaloss++;
	    }
	}

	arena_clear( match );
    }
}

void do_aclear( CHAR_DATA *ch, char *argument )
{
    ARENA_DATA *match;

    if ( argument[0] == '\0' )
    {
	send_to_char(	"Syntax: aclear [match #]\n\r"
			"        aclear all\n\r", ch );
	return;
    }

    else if ( !str_cmp( argument, "all" ) )
	match = NULL;

    else if ( !is_number( argument )
    ||        ( match = arena_lookup( atoi( argument ) ) ) == NULL )
    {
	do_aclear( ch, "" );
	return;
    }

    arena_clear( match );
    send_to_char( "Arena cleared.\n\r", ch );
}

void set_arena_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( ch->arena_number > 0 )
	obj->arena_number = ch->arena_number;
}

bool arena_flag( CHAR_DATA *ch, int flag )
{
    if ( !IS_NPC( ch )
    &&   ch->pcdata->match != NULL
    &&   IS_SET( ch->in_room->room_flags, ROOM_ARENA )
    &&   IS_SET( ch->pcdata->match->specials, flag ) )
    {
	send_to_char( "{RYou agreed not to do that during this match.{x\n\r", ch );
	return TRUE;
    }

    return FALSE;
}
