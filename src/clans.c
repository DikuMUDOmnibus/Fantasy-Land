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

#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "clans.h"
#include "recycle.h"

AREA_DATA *get_vnum_area	args( ( int vnum ) );
bool	clan_edit_create	args( ( CHAR_DATA *ch, char *argument ) );
void	save_area		args( ( AREA_DATA *pArea, bool deleted ) );
void	save_area_list		args( ( void ) );
void	fread_char		args( ( CHAR_DATA *ch, FILE *fp, sh_int type ) );
long	flag_lookup2		args( ( const char *name, const struct flag_type *flag_table ) );
void	show_flag_cmds		args( ( CHAR_DATA *ch, const struct flag_type *flag_table ) );
void	reset_char		args( ( CHAR_DATA *ch ) );

void clan_log( CHAR_DATA *ch, sh_int clan, char *argument, sh_int cubic, sh_int aquest, sh_int iquest )
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    char *strtime;

    strtime = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';

    sprintf( buf, "../log/clans/%s.log", clan_table[clan].name );
    if ( ( fp = fopen( buf, "a" ) ) != NULL )
    {
	fprintf( fp, "%-12s %-12s | %-20s for %5d cubic, %5d aquest, %5d iquest.\n",
	    strtime, ch ? ch->name : "", argument, cubic, aquest, iquest );
	fclose( fp );
    }
}

sh_int count_cubics( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    sh_int count = 0;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == OBJ_VNUM_CUBIC )
	    count++;
    }

    return count;
}

void extract_cubics( CHAR_DATA *ch, sh_int count )
{
    OBJ_DATA *obj, *obj_next;
    sh_int pos = 0;

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;

	if ( obj->pIndexData->vnum == OBJ_VNUM_CUBIC )
	{
	    extract_obj( obj );
	    if ( ++pos == count )
		return;
	}
    }

    sprintf( log_buf, "Extract_cubics: %s found %d of %d.",
	ch->name, pos, count );
    bug( log_buf, 0 );
}

bool can_use_clan_mob( CHAR_DATA *ch, CHAR_DATA *mobile )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_IMMORTAL(ch)
    &&   mobile->pIndexData->area->clan != 0
    &&   mobile->pIndexData->area->clan != ch->clan )
    {
	sprintf( buf, "I'm sorry %s, but I only do business with %s {Smembers.",
	    PERS( ch, mobile ), clan_table[mobile->clan].color );
	do_say( mobile, buf );
	return FALSE;
    }

    return TRUE;
}

void send_to_clan( char *message, CHAR_DATA *ch, sh_int to_clan )
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->clan == to_clan )
	    act( message, d->character, NULL, ch, TO_CHAR, POS_DEAD );
    }
}

void do_clead( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: clead <char>\n\r", ch );
	return;
    }

    if ( ( victim = get_pc_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't playing.\n\r", ch );
	return;
    }

    if ( !is_clan( victim ) )
    {
	send_to_char( "This person is not in a clan.\n\r", ch );
	return;
    }

    if ( clan_table[victim->clan].independent )
    {
	sprintf( buf, "This person is a %s.\n\r",
	    clan_table[victim->clan].color );
	send_to_char( buf, ch );
	return;
    }

    if ( victim->pcdata->clan_rank == MAX_CRNK-1 )
    {
	sprintf( buf, "They are no longer leader of clan {D<{x%s{D>{x.\n\r",
	    clan_table[victim->clan].color );
	send_to_char( buf, ch );

	sprintf( buf, "You are no longer leader of clan {D<{x%s{D>{x.\n\r",
	    clan_table[victim->clan].color );
	send_to_char( buf, victim );

	victim->pcdata->clan_rank--;
	check_roster( victim, FALSE );
    } else {
	sprintf( buf, "They are now leader of clan {D<{x%s{D>{x.\n\r",
	    clan_table[victim->clan].color );
	send_to_char( buf, ch );

	sprintf( buf, "You are now leader of clan {D<{x%s{D>{x.\n\r",
	    clan_table[victim->clan].color );
	send_to_char( buf, victim );

	victim->pcdata->clan_rank = MAX_CRNK-1;
	check_roster( victim, FALSE );
    }
}

void do_guild( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    sh_int clan;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: guild <char> <clan name>\n\r", ch );
	return;
    }

    if ( ( victim = get_pc_world( ch, arg ) ) == NULL
    ||   !can_over_ride( ch, victim, TRUE ) )
    {
	send_to_char( "They aren't playing.\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "none" ) )
    {
	send_to_char( "They are now clanless.\n\r", ch );
	send_to_char( "You are now a member of no clan!\n\r", victim );

	if ( is_clan( victim ) )
	    update_clanlist( victim, FALSE );

	victim->clan = 0;
	victim->pcdata->clan_rank = 0;
	check_roster( victim, TRUE );
	return;
    }

    if ( ( clan = clan_lookup( argument ) ) == 0 )
    {
	send_to_char( "No such clan exists.\n\r", ch );
	return;
    }

    sprintf( buf, "They are now a member of clan [%s].\n\r",
	clan_table[clan].color );
    send_to_char( buf, ch );

    sprintf( buf, "You are now a member of clan [%s].\n\r",
	clan_table[clan].color );
    send_to_char( buf, victim );

    if ( clan_table[clan].independent )
	check_roster( victim, FALSE );

    victim->pcdata->clan_rank = 0;

    if ( is_clan( victim ) )
	update_clanlist( victim, FALSE );

    victim->clan = clan;
    update_clanlist( victim, TRUE );
    check_roster( victim, FALSE );
    return;
}

void do_member( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    bool absent = FALSE, done = FALSE;

    if ( IS_NPC( ch ) )
	return;

    if ( ch->in_room != NULL
    &&   ( IS_SET( ch->in_room->room_flags, ROOM_ARENA )
    ||     IS_SET( ch->in_room->room_flags, ROOM_WAR ) ) )
    {
	send_to_char( "You can't use that in the arena.\n\r", ch );
	return;
    }

    if ( ch->pcdata->clan_rank != MAX_CRNK-1 )
    {
	if ( is_clan( ch ) && !clan_table[ch->clan].independent )
	{
	    send_to_char( "You are not a clan leader.\n\r", ch );
	    return;
	}

	if ( !ch->pcdata->invited )
	{
	    send_to_char( "You have not been invited to join a clan.\n\r", ch );
	    return;
	}

	if ( !str_cmp( argument, "accept" ) )
	{
	    if ( clan_table[ch->pcdata->invited].members+1 >
		 clan_table[ch->pcdata->invited].max_mem )
	    {
		send_to_char( "I'm sorry, but by joining you exceed their clan member limits.\n\r", ch );
		return;
	    }

	    sprintf( buf, "{RYou are now a member of clan {x< %s >.\n\r",
		clan_table[ch->pcdata->invited].color );
	    send_to_char( buf, ch );

	    if ( is_clan( ch ) )
		update_clanlist( ch, FALSE );

	    sprintf( buf, "{G$N has joined the ranks of %s{G!{x",
		clan_table[ch->pcdata->invited].color );
	    send_to_clan( buf, ch, ch->pcdata->invited );

	    ch->clan = ch->pcdata->invited;
	    update_clanlist( ch, TRUE );
	    ch->pcdata->invited = 0;
	    ch->pcdata->clan_rank = 0;
	    check_roster( ch, FALSE );
	    return;
	}

	if ( !str_cmp( argument, "deny" ) )
	{
	    send_to_char( "You turn down the invitation.\n\r", ch );

	    sprintf( buf, "{G$N has refused to join the ranks of %s{G!{x",
		clan_table[ch->pcdata->invited].color );
	    send_to_clan( buf, ch, ch->pcdata->invited );

	    ch->pcdata->invited = 0;
	    return;
	}

	send_to_char( "Syntax: member <accept|deny>\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: member <char>\n\r", ch );
	return;
    }

    if ( ( victim = get_pc_world( ch, argument ) ) == NULL )
    {
	FILE *fp;
	char buf[MAX_STRING_LENGTH];

	sprintf( buf, "../player/%s/%s",
	    initial( argument ), capitalize( argument ) );

	if ( ( fp = fopen( buf, "r" ) ) == NULL )
	{
	    send_to_char( "That player does not exist.\n\r", ch );
	    return;
	}

	fclose( fp );

	d = new_descriptor( );
	if ( !load_char_obj( d, argument, FALSE, FALSE ) )
	{
	    send_to_char( "They are not logged in, or have a pfile.\n\r", ch );
	    free_descriptor( d );
	    free_char( d->character );
	    return;
	}

	victim = d->character;
	free_descriptor( d );
	victim->desc = NULL;
	absent = TRUE;
    }

    if ( victim == ch )
    {
	send_to_char( "To declan yourself, you must appoint another leader and have them do it, or disband your clan.\n\r", ch );
	done = TRUE;
    }

    if ( is_clan( victim )
    &&   !is_same_clan( ch, victim )
    &&   !clan_table[victim->clan].independent )
    {
	send_to_char( "They are in another clan already.\n\r", ch );
	done = TRUE;
    }

    if ( victim->pcdata->match != NULL
    ||   ( victim->in_room != NULL
    &&     ( IS_SET( victim->in_room->room_flags, ROOM_ARENA )
    ||       IS_SET( victim->in_room->room_flags, ROOM_WAR ) ) ) )
    {
	send_to_char( "Not while they are in the arena.\n\r", ch );
	done = TRUE;
    }

    if ( !done && is_clan( victim ) && !clan_table[victim->clan].independent )
    {
	send_to_char( "They are now clanless.\n\r", ch );
	send_to_char( "Your clan leader has kicked you out!\n\r", victim );

	update_clanlist( victim, FALSE );

	victim->pcdata->clan_rank = 0;

	if ( clan_table[victim->clan].pkill )
	{
	    victim->clan = clan_lookup( "condemned" );
	    check_roster( victim, FALSE );
	} else {
	    victim->clan = 0;
	    check_roster( victim, TRUE );
	}

	update_clanlist( victim, TRUE );
	save_char_obj( victim, 2 );
	done = TRUE;
    }

    if ( absent )
    {
	OBJ_DATA *obj, *obj_next;

	for ( obj = victim->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( obj->item_type == ITEM_WEAPON )
		extract_obj( obj );
	}

	for ( obj = victim->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    extract_obj( obj );
	}

	if ( victim->desc != NULL )
	    victim->desc->character = NULL;

	nuke_pets( victim );
	free_char( victim );

	if ( !done )
	    send_to_char( "They are not here.\n\r", ch );

	return;
    }

    if ( done )
	return;

    if ( victim->pcdata->invited )
    {
	if ( victim->pcdata->invited == ch->clan )
	{
	    sprintf( buf, "%s {Rhas withdrawn its invitation.{x\n\r",
		clan_table[victim->pcdata->invited].color );
	    send_to_char( buf, victim );

	    sprintf( buf, "%s {Rhas withdrawn its invitation to $N.",
		clan_table[victim->pcdata->invited].color );
	    send_to_clan( buf, victim, victim->pcdata->invited );

	    victim->pcdata->invited = 0;
	    return;
	}

	send_to_char( "They have already been invited to join a clan.\n\r", ch );
	return;
    }

    if ( clan_table[ch->clan].members+1 > clan_table[ch->clan].max_mem )
    {
	send_to_char( "I'm sorry, that will exceed you clan member limits.\n\r", ch );
	return;
    }

    send_to_clan( "{R$N has been invited to join your clan.{x", victim, ch->clan );

    sprintf( buf, "{RYou have been invited to join clan {x[%s].\n\r",
	clan_table[ch->clan].color );
    send_to_char( buf, victim );

    send_to_char( "{YUse {Gmember accept{Y to join this clan,{x\n\r", victim );
    send_to_char( "{Yor {Gmember deny{Y to turn down the invitation.{x\n\r", victim );
    victim->pcdata->invited = ch->clan;
}

void do_clanlist( CHAR_DATA *ch, char *argument )
{
    BUFFER *final = new_buf( );
    ROSTER_DATA *out_list;
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    char *motto;
    int clan;

    add_buf( final, " {s===========================================================================\n\r" );

    for ( clan = 1; clan_table[clan].name[0] != '\0'; clan++ )
    {
	sprintf( buf, "| %s {s- %s{s| {qMembers: {t%4d  {s|   {gKills: {t%4d  {s|   {rDefeats: {t%4d {s|\n\r",
	    clan_table[clan].pkill ? "{R>{r>{RPK{r<{R<" : "{yNON-PK",
	    end_string( clan_table[clan].color, 13 ),
	    clan_table[clan].members,
	    clan_table[clan].kills,
	    clan_table[clan].deaths );
	add_buf( final, buf );

	add_buf( final, "|===========================================================================|\n\r" );

	sprintf( buf, "|                 {qName: {s| {t%-50s{s|\n\r",
	    clan_table[clan].name );
	add_buf( final, buf );

	if ( !clan_table[clan].independent )
	{
	    buf2[0] = '\0';
	    for ( out_list = clan_table[clan].roster; out_list; out_list = out_list->next )
	    {
		if ( out_list->rank == MAX_CRNK-1 )
		{
		    sprintf( buf, "{q, {t%s", out_list->name );
		    strcat( buf2, buf );
		}
	    }

	    sprintf( buf, "|              {qLeaders: {s| %s{s|\n\r",
		end_string( buf2[0] == '\0' ? "{tNone" : buf2+4, 50 ) );
	    add_buf( final, buf );
	}

	if ( IS_IMMORTAL( ch ) )
	{
	    sprintf( buf2, "{q%d {tcubics, {q%d {taquest, {q%d {tiquest",
		clan_table[clan].cubics,
		clan_table[clan].aquest, clan_table[clan].iquest );
	    sprintf( buf, "|                {qWorth: {s| %s{s|\n\r",
		end_string( buf2, 50 ) );
	    add_buf( final, buf );

	    sprintf( buf, "|               {qRecall: {s| [{t%5d{s] {t%s{s|\n\r",
		clan_table[clan].hall,
		end_string( get_room_index( clan_table[clan].hall ) == NULL ?
		"{RNone{x" : get_room_index( clan_table[clan].hall )->name, 42 ) );
	    add_buf( final, buf );
	}

	motto = clan_table[clan].exname;
	motto = length_argument( motto, buf2, 50 );

	sprintf( buf, "|                {qMotto: {s| {q%s{s|\n\r",
	    end_string( buf2, 50 ) );
	add_buf( final, buf );

	while ( *motto != '\0' )
	{
	    motto = length_argument( motto, buf2, 50 );
	    sprintf( buf, "|                       {s| {q%s{s|\n\r",
		end_string( buf2, 50 ) );
	    add_buf( final, buf );
	}

	add_buf( final, " ===========================================================================\n\r" );
    }

    add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );
}

void do_clantalk( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] != '\0' && !str_cmp( argument, "list" ) )
    {
	do_clanlist( ch, "" );
	return;
    }

    if ( !is_clan( ch ) || clan_table[ch->clan].independent )
    {
	send_to_char( "You aren't in a clan.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_WAR )
    ||   IS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
    {
	send_to_char( "Your voice cannot carry past the sounds of death.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( IS_SET( ch->comm, COMM_NOCLAN ) )
	{
	    send_to_char( "Clan channel is now ON\n\r", ch );
	    REMOVE_BIT( ch->comm, COMM_NOCLAN );
	} else {
	    send_to_char( "Clan channel is now OFF\n\r", ch );
	    SET_BIT( ch->comm, COMM_NOCLAN );
	}

	return;
    }

    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
    {
	send_to_char( "The gods have revoked your channel priviliges.\n\r", ch );
	return;
    }

    REMOVE_BIT( ch->comm, COMM_NOCLAN );

    argument = channel_parse( ch, argument, FALSE );

    sprintf( buf, "You clan '{F%s{x'\n\r", argument );
    send_to_char( buf, ch );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   ( is_same_clan( ch, d->character )
	||     IS_SET( d->character->wiznet, WIZ_CLANS ) )
	&&   !IS_SET( d->character->comm, COMM_NOCLAN )
	&&   !IS_SET( d->character->comm, COMM_QUIET )
	&&   !check_forget( d->character, ch ) )
	{
	    if ( IS_SET( d->character->wiznet, WIZ_CLANS ) )
		sprintf( buf, "%s$n{x <%s> clans '{F$t{x'",
		    pretitle( ch, d->character ), clan_table[ch->clan].color );
	    else
		sprintf( buf, "%s$n{x clans '{F$t{x'", pretitle( ch, d->character ) );

	    act( buf, ch, argument, d->character, TO_VICT, POS_DEAD );
	}
    }

    return;
}

ROSTER_DATA *new_roster( void )
{
    ROSTER_DATA *roster;

    if ( roster_free == NULL )
	roster = alloc_perm( sizeof( *roster ) );
    else
    {
	roster = roster_free;
	roster_free = roster_free->next;
    }

    return roster;
}

void free_roster( ROSTER_DATA *roster )
{
    free_string( roster->name );
    roster->name = NULL;

    roster->next = roster_free;
    roster_free = roster;
}

void update_clanlist( CHAR_DATA *ch, bool add )
{
    if ( IS_NPC( ch ) )
	return;

    if ( add )
	clan_table[ch->clan].members++;
    else
	clan_table[ch->clan].members--;

    if ( clan_table[ch->clan].members < 0 )
	clan_table[ch->clan].members = 0;

    mud_stat.clans_changed = TRUE;
    return;
}

void check_clandeath( CHAR_DATA *killer, CHAR_DATA *victim )
{
    if ( is_clan( killer ) && is_clan( victim ) )
    {
	clan_table[killer->clan].kills++;
	clan_table[victim->clan].deaths++;
	mud_stat.clans_changed = TRUE;
    }

    return;
}

void check_roster( CHAR_DATA *ch, bool remove )
{
    ROSTER_DATA *list;
    int i;

    for ( i = 0; clan_table[i].name[0] != '\0'; i++ )
    {
	bool found = FALSE;

	if ( clan_table[i].roster == NULL )
	{
	    if ( !remove && ch->clan == i && ch->clan != 0 )
	    {
		list		= new_roster( );
		list->name	= str_dup( ch->name );
		list->rank	= ch->pcdata->clan_rank;
		clan_table[i].roster = list;
		mud_stat.clans_changed = TRUE;
	    }

	    continue;
	}

	for ( list = clan_table[i].roster; list != NULL; list = list->next )
	{
	    if ( !str_cmp( ch->name, list->name ) )
	    {
		if ( remove || i != ch->clan )
		{
		    if ( clan_table[i].roster == list )
			clan_table[i].roster = list->next;
		    else
		    {
			ROSTER_DATA *pos;

			for ( pos = clan_table[i].roster; pos; pos = pos->next )
			{
			    if ( pos->next == list )
			    {
				pos->next = list->next;
				break;
			    }
			}
		    }

		    free_roster( list );
		    mud_stat.clans_changed = TRUE;
		    break;
		}

		else
		{
		    if ( list->rank != ch->pcdata->clan_rank )
		    {
			list->rank = ch->pcdata->clan_rank;
			mud_stat.clans_changed = TRUE;
		    }

		    found = TRUE;
		    break;
		}
	    }
	}

	if ( !found && i == ch->clan && !remove && ch->clan != 0 )
	{
	    list		= new_roster( );
	    list->name		= str_dup( ch->name );
	    list->rank		= ch->pcdata->clan_rank;
	    list->next		= clan_table[i].roster;
	    clan_table[i].roster= list;
	    mud_stat.clans_changed = TRUE;
	}
    }
}

int count_roster( int clan, int rank )
{
    ROSTER_DATA *rost;
    sh_int count = 0;

    for ( rost = clan_table[clan].roster; rost != NULL; rost = rost->next )
    {
	if ( rost->rank == rank )
	    count++;
    }

    return count;
}

void members_of_roster( CHAR_DATA *ch, BUFFER *final, int clan, int rank )
{
    CHAR_DATA *wch;
    FILE *fp;
    ROSTER_DATA *rost;
    char buf[MAX_STRING_LENGTH], class[20], file[100], race[20];

    for ( rost = clan_table[clan].roster; rost != NULL; rost = rost->next )
    {
	if ( rost->rank != rank )
	    continue;

	sprintf( file, "%s%s/%s",
	    PLAYER_DIR, initial( rost->name ), capitalize( rost->name ) );

	if ( ( fp = fopen( file, "r" ) ) != NULL )
	{
	    char *word = fread_word( fp );

	    wch = new_char( );
	    wch->pcdata = new_pcdata( );

	    if ( !str_cmp( word, "#PLAYER" ) )
		fread_char( wch, fp, 2 );
	    fclose( fp );

	    if ( wch->pcdata
	    &&   wch->pcdata->who_descr[0] != '\0'
	    &&   wch != ch
	    &&   ( !IS_IMMORTAL( ch ) || !can_over_ride( ch, wch, TRUE ) ) )
		sprintf( buf, "| {t%-15s       {s| {t--- {s|{t   ---   {s|{t     ---    {s|{t%3d {gkills {s|{t%3d {rdeaths {s|\n\r",
		    wch->name, wch->pcdata->pkills, wch->pcdata->pdeath );
	    else
	    {
		sprintf( class, "{%c%c{%c%s",
		    class_table[wch->class].who_name[1],
		    class_table[wch->class].who_name[2],
		    class_table[wch->class].who_name[4],
		    class_table[wch->class].name+1 );

		sprintf( race, "%s",
		    center_string( capitalize( race_table[wch->race].name ), 9 ) );

		sprintf( buf, "| {t%-15s       {s| {t%3d {s|{t%s{s|%s{s|{t%3d {gkills {s|{t%3d {rdeaths {s|\n\r",
		    wch->name, wch->level, race,
		    center_string( class, 12 ),
		    wch->pcdata->pkills, wch->pcdata->pdeath );
	    }

	    add_buf( final, buf );

	    free_char( wch );
	}
    }
}

void do_roster( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    char *motto;
    sh_int clan, rank;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  roster <clan>\n\r", ch );
	return;
    }

    if ( ( clan = clan_lookup( argument ) ) == 0 )
    {
	send_to_char( "That's not a clan!\n\r", ch );
	return;
    }

    final = new_buf( );

    add_buf( final, " {s===========================================================================\n\r" );

    sprintf( buf, "| %s {s- %s{s| {qMembers: {t%4d  {s|   {gKills: {t%4d  {s|   {rDefeats: {t%4d {s|\n\r",
	clan_table[clan].pkill ? "{R>{r>{RPK{r<{R<" : "{yNON-PK",
	end_string( clan_table[clan].color, 13 ),
	clan_table[clan].members,
	clan_table[clan].kills,
	clan_table[clan].deaths );
    add_buf( final, buf );

    add_buf( final, "|===========================================================================|\n\r" );

    motto = clan_table[clan].exname;
    motto = length_argument( motto, buf2, 50 );

    sprintf( buf, "| {qMotto:                {s| {q%s{s|\n\r",
	end_string( buf2, 50 ) );
    add_buf( final, buf );

    while ( *motto != '\0' )
    {
	motto = length_argument( motto, buf2, 50 );
	sprintf( buf, "|                       {s| {q%s{s|\n\r",
	    end_string( buf2, 50 ) );
	add_buf( final, buf );
    }

    if ( IS_IMMORTAL( ch ) )
    {
	sprintf( buf2, "{q%d {tcubics, {q%d {taquest, {q%d {tiquest",
	    clan_table[clan].cubics, clan_table[clan].aquest, clan_table[clan].iquest );
	sprintf( buf, "| {qWorth:                {s| %s{s|\n\r",
	    end_string( buf2, 50 ) );
	add_buf( final, buf );
    }

    add_buf( final, "|===========================================================================|\n\r" );

    if ( !clan_table[clan].independent )
    {
	for ( rank = MAX_CRNK - 1; rank > 0; rank-- )
	{
	    sprintf( buf2, "%s:", clan_table[clan].crnk[rank] );
	    sprintf( buf, "| {q%s{s|****************** {q%2d {tMember(s) {s*******************|\n\r",
		end_string( buf2, 22 ), count_roster( clan, rank ) );
	    add_buf( final, buf );

	    members_of_roster( ch, final, clan, rank );

	    add_buf( final, "|---------------------------------------------------------------------------|\n\r" );
	}
    }

    sprintf( buf2, "%s:", clan_table[clan].crnk[0] );
    sprintf( buf, "| {q%s {s|****************** {q%2d {tMember(s) {s*******************|\n\r",
	end_string( buf2, 21 ), count_roster( clan, 0 ) );
    add_buf( final, buf );

    members_of_roster( ch, final, clan, 0 );

    add_buf( final, " ==========================================================================={x\n\r" );

    page_to_char( final->string, ch );
    free_buf( final );
}

void do_promote( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
    {
	send_to_char( "NPC's can not promote someone.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: promote <char>\n\r", ch );
	return;
    }

    if ( ( victim = get_pc_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    if ( ch->pcdata->clan_rank != MAX_CRNK-1 )
    {
	send_to_char( "It would be a wise idea to leave promotions up to your clan leader...\n\r", ch );
	return;
    }

    if ( victim->clan != ch->clan )
    {
	send_to_char( "Perhaps thier clan leader does not wish them promoted...\n\r", ch );
	return;
    }

    if ( victim->pcdata->clan_rank >= MAX_CRNK-1 )
    {
	send_to_char( "Thats a little higher then you are...\n\r", ch );
	victim->pcdata->clan_rank = MAX_CRNK-1;
	return;
    }

    victim->pcdata->clan_rank++;

    sprintf( buf, "{G$N has been promoted up to [%s{G].{x",
	clan_table[victim->clan].crnk[victim->pcdata->clan_rank] );
    send_to_clan( buf, victim, victim->clan );

    check_roster( victim, FALSE );
}

void do_demote( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
    {
	send_to_char( "NPC's can not demote someone.\n\r",ch);
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: demote <char>\n\r",ch);
	return;
    }

    if ( ( victim = get_pc_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    if ( ch->pcdata->clan_rank != MAX_CRNK-1 )
    {
	send_to_char( "Perhaps you should leave that decision up to the clan leader...\n\r", ch );
	return;
    }

    if ( victim->clan != ch->clan )
    {
	send_to_char( "Perhaps their clan leader does not wish them to be demoted...\n\r", ch );
	return;
    }

    if ( victim->pcdata->clan_rank <= 0 )
    {
	send_to_char( "That person is already good for shit...\n\r", ch );
	victim->pcdata->clan_rank = 0;
	return;
    }

    victim->pcdata->clan_rank--;

    sprintf( buf, "{G$N has been demoted down to [%s{G].{x",
	clan_table[victim->clan].crnk[victim->pcdata->clan_rank] );
    send_to_clan( buf, victim, victim->clan );

    check_roster( victim, FALSE );
}

void do_account( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *wch = NULL;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( IS_NPC( ch ) )
	return;

    if ( !IS_TRUSTED( ch, MAX_LEVEL-1 )
    &&   ( !is_clan( ch ) || clan_table[ch->clan].independent ) )
    {
	send_to_char( "You don't need a clan bank account.\n\r", ch );
	return;
    }

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: account deposit <number> <type>\n\r"
//		      "        account withdraw <number> <type>\n\r"
		      "        account balance\n\r"
		      "\n\rAcceptable types: cubics, aquest, iquest\n\r", ch );
	return;
    }
/*
    for ( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room )
    {
	if ( IS_NPC( wch ) && IS_SET( wch->act, ACT_IS_CHANGER ) )
	    break;
    }

    if ( wch == NULL && !IS_TRUSTED( ch, MAX_LEVEL - 1 ) )
    {
	send_to_char( "You can't seem to find the banker.\n\r", ch );
	return;
    }

    if ( wch != NULL && !IS_TRUSTED( ch, MAX_LEVEL-1 )
    &&   ( wch->clan == 0 || wch->clan != ch->clan ) )
    {
	sprintf( buf, "$N says '{SI am sorry $n, but I don't handle banking affairs for %s{S.{x'",
	    clan_table[ch->clan].color );
	act( buf, ch, NULL, wch, TO_CHAR, POS_RESTING );
	return;
    }
*/
    if ( wch == NULL )
	wch = ch;

    if ( !str_prefix( arg1, "balance" ) )
    {
	sprintf( buf, "%s currently has %d cubics, %d aquest and %d iquest in its account.\n\r",
	    clan_table[wch->clan].color,
	    clan_table[wch->clan].cubics,
	    clan_table[wch->clan].aquest,
	    clan_table[wch->clan].iquest );
	send_to_char( buf, ch );
	return;
    }

    if ( arg2[0] == '\0' || argument[0] == '\0' )
    {
	do_account(ch,"");
	return;
    }

    if ( !is_number( arg2 ) )
    {
	send_to_char( "Value must be an intiger!\n\r\n\r", ch );
	do_account( ch, "" );
	return;
    }

    value = atoi( arg2 );

    if ( value < 0 )
    {
	send_to_char( "Value must be a positive intiger!\n\r\n\r", ch );
	do_account( ch, "" );
	return;
    }

    if ( !str_prefix( arg1, "deposit" ) )
    {
	if ( !str_prefix( argument, "cubics" ) )
	{
	    if ( value > count_cubics( ch ) )
	    {
		send_to_char( "You do not have that many cubics.\n\r", ch );
		return;
	    }

	    extract_cubics( ch, value );

	    sprintf( buf, "%s clan balance raised from %d cubics to %d cubics.\n\r",
		clan_table[wch->clan].color,
		clan_table[wch->clan].cubics,
		clan_table[wch->clan].cubics + value );
	    send_to_char( buf, ch );
	    clan_table[wch->clan].cubics += value;
	    clan_log( ch, ch->clan, "deposit_cubic", value, 0, 0 );
	    mud_stat.clans_changed = TRUE;
	    return;
	}

	if ( !str_prefix( argument, "aquest" ) )
	{
	    if ( value > ch->pcdata->questpoints )
	    {
		sprintf( buf, "You may not deposit %d aquest because you only have %d aquest.\n\r",
		    value, ch->pcdata->questpoints );
		send_to_char( buf, ch );
		return;
	    }

	    sprintf( buf, "%s clan balance raised from %d aquest to %d aquest.\n\r",
		clan_table[wch->clan].color, clan_table[wch->clan].aquest, clan_table[wch->clan].aquest + value );
	    send_to_char( buf, ch );
	    ch->pcdata->questpoints -= value;
	    clan_table[wch->clan].aquest += value;
	    mud_stat.clans_changed = TRUE;
	    clan_log( ch, ch->clan, "deposit_aquest", 0, value, 0 );
	    return;
	}

	if ( !str_prefix( argument, "iquest" ) )
	{
	    if ( value > ch->pcdata->deviant_points[0] )
	    {
		sprintf( buf, "You may not deposit %d iquest because you only have %d iquest.\n\r",
		    value, ch->pcdata->deviant_points[0] );
		send_to_char( buf, ch );
		return;
	    }

	    sprintf( buf, "%s clan balance raised from %d iquest to %d iquest.\n\r",
		clan_table[wch->clan].color, clan_table[wch->clan].iquest, clan_table[wch->clan].iquest + value );
	    send_to_char( buf, ch );
	    ch->pcdata->deviant_points[0] -= value;
	    clan_table[wch->clan].iquest += value;
	    mud_stat.clans_changed = TRUE;
	    clan_log( ch, ch->clan, "deposit_iquest", 0, 0, value );
	    return;
	}

	send_to_char( "Unknown type of deposit.\n\r\n\r", ch );
	do_account( ch, "" );
	return;
    }
/*
    if ( !str_prefix( arg1, "withdraw" ) )
    {
	if ( ch->pcdata->clan_rank != MAX_CRNK-1 )
	{
	    send_to_char( "Only leaders may withdraw from clan accounts.\n\r", ch );
	    return;
	}

	if ( !str_prefix( argument, "cubics" ) )
	{
	    OBJ_DATA *cubic;
	    int total;

	    if ( value > clan_table[wch->clan].cubics )
	    {
		sprintf( buf, "You may not withdraw %d cubics because your clan only has %d cubics.\n\r",
		    value, clan_table[wch->clan].cubics );
		send_to_char( buf, ch );
		return;
	    }

	    for ( total = 0; total < value; total++ )
	    {
		if ( ( cubic = create_object( get_obj_index( OBJ_VNUM_CUBIC ) ) ) == NULL )
		    return;

		obj_to_char( cubic, ch );
	    }

	    sprintf( buf, "%s clan balance lowered from %d cubics to %d cubics.\n\r",
		clan_table[wch->clan].color,
		clan_table[wch->clan].cubics,
		clan_table[wch->clan].cubics - value );
	    send_to_char( buf, ch );
	    clan_table[wch->clan].cubics -= value;

	    mud_stat.clans_changed = TRUE;

	    return;
	}

	send_to_char( "Unknown type of withdrawl.\n\r\n\r", ch );
	do_account( ch, "" );
	return;
    }
*/
    do_account( ch, "" );
    return;
}

void save_clans( )
{
    FILE *fp;
    PKILL_RECORD *pk_record;
    ROSTER_DATA *out_list;
    int i, rank;
    extern int port;

    if ( port != MAIN_GAME_PORT )
	return;

    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Save_clans: error opening clan file for saving.", 0 );
	return;
    }

    fprintf( fp, "#MAX_CLAN %d\n\n",	maxClan				);

    for ( i = 0; clan_table[i].name[0] != '\0'; i++ )
    {
	fprintf( fp, "#Clan\n"						);
	fprintf( fp, "CName %s~\n",	clan_table[i].name		);
	fprintf( fp, "Who_N %s~\n",	clan_table[i].who_name		);
	fprintf( fp, "Color %s~\n",	clan_table[i].color		);
	fprintf( fp, "Descr %s~\n",	clan_table[i].exname		);

	if ( clan_table[i].hall != ROOM_VNUM_ALTAR )
	    fprintf( fp, "Recal %d\n",	clan_table[i].hall		);

	if ( clan_table[i].pit != OBJ_VNUM_PIT )
	    fprintf( fp, "Donat %d\n",	clan_table[i].pit		);

	if ( clan_table[i].portal_room != 0 )
	    fprintf( fp, "Porta %d\n",	clan_table[i].portal_room	);

	if ( clan_table[i].two_way_link != 0 )
	    fprintf( fp, "TwoWy %d\n",	clan_table[i].two_way_link	);

	if ( clan_table[i].two_way_time != 0 )
	    fprintf( fp, "TwoTm %d\n",	clan_table[i].two_way_time	);

	if ( clan_table[i].independent )
	    fprintf( fp, "Indep %d\n",	clan_table[i].independent	);

	if ( !clan_table[i].pkill )
	    fprintf( fp, "Pkill %d\n",	clan_table[i].pkill		);

	if ( clan_table[i].max_mem != 5 )
	    fprintf( fp, "MaxMe %d\n",	clan_table[i].max_mem		);

	if ( clan_table[i].edit_clan != 0 )
	    fprintf( fp, "EditC %d\n",	clan_table[i].edit_clan		);

	if ( clan_table[i].edit_mob != 0 )
	    fprintf( fp, "EditM %d\n", clan_table[i].edit_mob		);

	if ( clan_table[i].edit_obj != 0 )
	    fprintf( fp, "EditO %d\n", clan_table[i].edit_obj		);

	if ( clan_table[i].edit_room != 0 )
	    fprintf( fp, "EditR %d\n", clan_table[i].edit_room		);

	if ( clan_table[i].edit_help != 0 )
	    fprintf( fp, "EditH %d\n", clan_table[i].edit_help		);

	if ( clan_table[i].members != 0 )
	    fprintf( fp, "Membe %d\n",	clan_table[i].members		);

	if ( clan_table[i].kills != 0 )
	    fprintf( fp, "Kills %d\n",	clan_table[i].kills		);

	if ( clan_table[i].deaths != 0 )
	    fprintf( fp, "Death %d\n",	clan_table[i].deaths		);

	if ( clan_table[i].cubics != 0 )
	    fprintf( fp, "Cubic %d\n",	clan_table[i].cubics		);

	if ( clan_table[i].aquest != 0 )
	    fprintf( fp, "Aqust %d\n",	clan_table[i].aquest		);

	if ( clan_table[i].iquest != 0 )
	    fprintf( fp, "Iqust %d\n",	clan_table[i].iquest		);

	if ( !clan_table[i].independent )
	{
	    for ( rank = 1; rank < MAX_CRNK; rank++ )
		fprintf( fp, "Rank%d %s~\n", rank-1, clan_table[i].crnk[rank] );
	}

	for ( pk_record = clan_table[i].kills_list; pk_record != NULL; pk_record = pk_record->next )
	    fprintf( fp, "KRCRD %s~ %s~ %s~ %s~ %d %d %ld %d %d %s~\n",
		pk_record->killer_name,	pk_record->victim_name,
		pk_record->killer_clan,	pk_record->victim_clan,
		pk_record->level[0],	pk_record->level[1],
		pk_record->pkill_time,	pk_record->pkill_points,
		pk_record->bounty,	pk_record->assist_string );

	for ( pk_record = clan_table[i].death_list; pk_record != NULL; pk_record = pk_record->next )
	    fprintf( fp, "DRCRD %s~ %s~ %s~ %s~ %d %d %ld %d %d %s~\n",
		pk_record->killer_name,	pk_record->victim_name,
		pk_record->killer_clan,	pk_record->victim_clan,
		pk_record->level[0],	pk_record->level[1],
		pk_record->pkill_time,	pk_record->pkill_points,
		pk_record->bounty,	pk_record->assist_string );

	fprintf( fp, "Roster\n" );
	for ( out_list = clan_table[i].roster; out_list != NULL; out_list = out_list->next )
	    fprintf( fp, "%d %s~\n", out_list->rank, out_list->name );
	fprintf( fp, "-1\n\n" );
    }

    fprintf( fp, "#END\n" );
    fclose( fp );
    rename( TEMP_FILE, "../data/info/clans.dat" );
    mud_stat.clans_changed = FALSE;
    return;
}

void load_clans( )
{
    FILE *fp;
    PKILL_RECORD *pk_record, *pk_list;
    ROSTER_DATA *in_list;
    int c = -1, temprank;

    if ( ( fp = fopen( "../data/info/clans.dat", "r" ) ) == NULL )
    {
	bug( "Load_clans: can not open file to read.", 0 );
	return;
    }

    for ( ; ; )
    {
	char *word;
	int rank;

	word = fread_word( fp );

	if ( !str_cmp( word, "#Clan" ) )
	{
	    c++;

	    if ( c > maxClan )
	    {
		bug( "Load_clans: maxClan exceeded.", 0 );
		fclose( fp );
		clan_table[maxClan].name = str_dup( "" );
		return;
	    }

	    clan_table[c].name		= NULL;
	    clan_table[c].who_name	= NULL;
	    clan_table[c].color		= NULL;
	    clan_table[c].hall		= ROOM_VNUM_ALTAR;
	    clan_table[c].pit		= OBJ_VNUM_PIT;
	    clan_table[c].independent	= FALSE;
	    clan_table[c].pkill		= TRUE;
	    clan_table[c].exname	= NULL;
	    clan_table[c].max_mem	= 5;
	    clan_table[c].roster	= NULL;
	    clan_table[c].kills_list	= NULL;
	    clan_table[c].death_list	= NULL;

	    for ( rank = 1; rank < MAX_CRNK; rank++ )
		clan_table[c].crnk[rank] = NULL;

	    clan_table[c].crnk[0] = str_dup( "Unranked" );

	    for ( ; ; )
	    {
		char *match = fread_word( fp );

		if ( !str_cmp( match, "Roster" ) )
		    break;

		switch( UPPER( match[0] ) )
		{
		    case 'A':
			LOAD( "Aqust", clan_table[c].aquest,	fread_number( fp ) );
			break;

		    case 'C':
			LOAD( "CName", clan_table[c].name,	fread_string( fp ) );
			LOAD( "Color", clan_table[c].color,	fread_string( fp ) );
			LOAD( "Cubic", clan_table[c].cubics,	fread_number( fp ) );
			break;

		    case 'D':
			LOAD( "Descr", clan_table[c].exname,	fread_string( fp ) );
			LOAD( "Donat", clan_table[c].pit,	fread_number( fp ) );
			LOAD( "Death", clan_table[c].deaths,	fread_number( fp ) );

			if ( !str_cmp( match, "DRCRD" ) )
			{
			    pk_record			= new_pk_record( );
			    pk_record->killer_name	= fread_string( fp );
			    pk_record->victim_name	= fread_string( fp );
			    pk_record->killer_clan	= fread_string( fp );
			    pk_record->victim_clan	= fread_string( fp );
			    pk_record->level[0]		= fread_number( fp );
			    pk_record->level[1]		= fread_number( fp );
			    pk_record->pkill_time	= fread_number( fp );
			    pk_record->pkill_points	= fread_number( fp );
			    pk_record->bounty		= fread_number( fp );
			    pk_record->assist_string	= fread_string( fp );

			    if ( clan_table[c].death_list == NULL )
				clan_table[c].death_list = pk_record;
			    else
			    {
				for ( pk_list = clan_table[c].death_list; pk_list != NULL; pk_list = pk_list->next )
				{
				    if ( pk_list->next == NULL )
				    {
					pk_list->next	= pk_record;
					break;
				    }
				}
			    }
			    break;
			}
			break;

		    case 'E':
			LOAD( "EditC", clan_table[c].edit_clan, fread_number( fp ) );
			LOAD( "EditH", clan_table[c].edit_help, fread_number( fp ) );
			LOAD( "EditM", clan_table[c].edit_mob, fread_number( fp ) );
			LOAD( "EditO", clan_table[c].edit_obj, fread_number( fp ) );
			LOAD( "EditR", clan_table[c].edit_room, fread_number( fp ) );
			break;

		    case 'I':
			LOAD( "Indep", clan_table[c].independent, fread_number( fp ) );
			LOAD( "Iqust", clan_table[c].iquest,	fread_number( fp ) );
			break;

		    case 'K':
			LOAD( "Kills", clan_table[c].kills,	fread_number( fp ) );

			if ( !str_cmp( match, "KRCRD" ) )
			{
			    pk_record			= new_pk_record( );
			    pk_record->killer_name	= fread_string( fp );
			    pk_record->victim_name	= fread_string( fp );
			    pk_record->killer_clan	= fread_string( fp );
			    pk_record->victim_clan	= fread_string( fp );
			    pk_record->level[0]		= fread_number( fp );
			    pk_record->level[1]		= fread_number( fp );
			    pk_record->pkill_time	= fread_number( fp );
			    pk_record->pkill_points	= fread_number( fp );
			    pk_record->bounty		= fread_number( fp );
			    pk_record->assist_string	= fread_string( fp );

			    if ( clan_table[c].kills_list == NULL )
				clan_table[c].kills_list = pk_record;
			    else
			    {
				for ( pk_list = clan_table[c].kills_list; pk_list != NULL; pk_list = pk_list->next )
				{
				    if ( pk_list->next == NULL )
				    {
					pk_list->next	= pk_record;
					break;
				    }
				}
			    }
			    break;
			}
			break;

		    case 'M':
			LOAD( "MaxMe", clan_table[c].max_mem,	fread_number( fp ) );
			LOAD( "Membe", clan_table[c].members,	fread_number( fp ) );
			break;

		    case 'P':
			LOAD( "Pkill", clan_table[c].pkill,	fread_number( fp ) );
			LOAD( "Porta", clan_table[c].portal_room, fread_number( fp ) );
			break;

		    case 'R':
			LOAD( "Recal", clan_table[c].hall,	fread_number( fp ) );
			LOAD( "Rank0", clan_table[c].crnk[1],	fread_string( fp ) );
			LOAD( "Rank1", clan_table[c].crnk[2],	fread_string( fp ) );
			LOAD( "Rank2", clan_table[c].crnk[3],	fread_string( fp ) );
			LOAD( "Rank3", clan_table[c].crnk[4],	fread_string( fp ) );
			LOAD( "Rank4", clan_table[c].crnk[5],	fread_string( fp ) );
			break;

		    case 'T':
			LOAD( "TwoWy", clan_table[c].two_way_link, fread_number( fp ) );
			LOAD( "TwoTm", clan_table[c].two_way_time, fread_number( fp ) );
			break;

		    case 'W':
			LOAD( "Who_N", clan_table[c].who_name,	fread_string( fp ) );
			break;
		}
	    }

	    for ( ; ; )
	    {
		if ( feof( fp ) )
		    break;

		if ( ( temprank = fread_number( fp ) ) == -1 )
		    break;

		in_list			= new_roster( );
		in_list->rank		= temprank;
		in_list->name		= fread_string( fp );
		in_list->next		= clan_table[c].roster;
		clan_table[c].roster	= in_list;
	    }
	}

	else if ( !str_cmp( word, "#MAX_CLAN" ) )
	{
	    maxClan	= fread_number( fp );
	    clan_table	= malloc( sizeof( struct clan_type ) * ( maxClan+1 ) );
	}

        else if ( !str_cmp( word, "#END" ) )
	    break;

        else
        {
	    bug( "Load_clans: bad section name.", 0 );
	    break;
        }
    }

    clan_table[maxClan].name = str_dup( "" );
    fclose( fp );
}

void do_condemn( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Only playing characters may proclaim their souls condemned!\n\r", ch );
	return;
    }

    if ( ch->clan != 0 )
    {
	send_to_char( "You are already in a clan.\n\r", ch );
	return;
    }

    if ( ch->pcdata->confirm_condemn )
    {
	if ( argument[0] == '\0' || str_cmp( argument, "yes" ) )
	{
	    send_to_char( "Condemned status removed.\n\r", ch );
	    ch->pcdata->confirm_condemn = FALSE;
	    return;
	}

	send_to_char( "You have proclaimed your soul condemned!\n\r", ch );
	ch->clan = clan_lookup( "condemned" );
	ch->pcdata->clan_rank = 0;
	ch->pcdata->confirm_condemn = FALSE;
	update_clanlist( ch, TRUE );
	check_roster( ch, FALSE );
	sprintf( buf, "{r[{RINFO{r]{w: {G%s {whas proclaimed thier soul to be %s{w!{x",
	    ch->name, clan_table[ch->clan].color );
	info( buf, ch, NULL, INFO_OTHER );
	return;
    }

    if ( argument[0] != '\0' )
    {
	send_to_char( "No argument is used with this command.\n\r", ch );
	return;
    }

    send_to_char( "{RWARNING: {GBy condemning your soul, you open yourself up to PKILL.\n\r"
		  "{RWARNING: {GThis command is permanent!\n\r"
		  "{RWARNING: {GYou are advised to read 'HELP PK' before confirming.\n\r"
		  "{RWARNING: {GTo confirm your condemnation, type 'condem yes'.\n\r"
		  "{RWARNING: {GTo remove condem consider status type 'condemn no'{x\n\r", ch );

    ch->pcdata->confirm_condemn = TRUE;

    sprintf( buf, "{r[{RINFO{r]{w: {G%s {wis considering tainting their soul %s{w!{x",
	ch->name, clan_table[clan_lookup("condemned")].color );
    info( buf, ch, NULL, INFO_OTHER );

    return;
}

sh_int get_reset_number( CHAR_DATA *ch, int type )
{
    MOB_INDEX_DATA *pMob;
    RESET_DATA *pReset;
    sh_int pos = 1;

    switch( type )
    {
	default:
	    break;

	case ITEM_PORTAL:
	case ITEM_PIT:
	case ITEM_FURNITURE:
	case ITEM_FOUNTAIN:
	    return 0;
    }

    for ( pReset = ch->in_room->reset_first; pReset != NULL; pReset = pReset->next )
    {
	pos++;

	if ( pReset->command == 'M' )
	{
	    if ( ( pMob = get_mob_index( pReset->arg1 ) ) != NULL )
	    {
		if ( pMob->pShop != NULL && type != ITEM_KEY )
		{
		    if ( !IS_IMMORTAL( ch ) && pReset->next && pReset->next->command == 'G' )
		    {
			send_to_char( "Each shopkeeper may only have 1 item.\n\r", ch );
			return -1;
		    }

		    return pos;
		}

		if ( pMob->pShop == NULL && type == ITEM_KEY && IS_SET( pMob->act, ACT_SENTINEL ) )
		    return pos;
	    }
	}
    }

    send_to_char( "You must have a shopkeeper in this room to add most items or a guard with sentinel flag for a key.\n\r", ch );
    return -1;
}

AREA_DATA *get_clan_area( sh_int clan )
{
    AREA_DATA *pArea;

    if ( clan == 0 )
	return NULL;

    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {
	if ( pArea->clan == clan )
	    return pArea;
    }

    return NULL;
}

HELP_DATA *get_clan_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char arg[MAX_INPUT_LENGTH];
    sh_int count, number;

    if ( argument == NULL || argument[0] == '\0' )
    {
	send_to_char( "Which help file are you looking for?\n\r", ch );
	return NULL;
    }

    number = number_argument( argument, arg );
    count = 0;

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	if ( pHelp->clan != 0
	&&   pHelp->clan == ch->clan
	&&   is_name( arg, pHelp->keyword )
	&&   ++count == number )
	    return pHelp;
    }

    send_to_char( "Your clan owns no such help file.\n\r", ch );
    return NULL;
}

MOB_INDEX_DATA *get_clan_mob( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    sh_int count, number;
    int vnum;

    if ( argument == NULL || argument[0] == '\0' )
    {
	send_to_char( "Which mobile are you looking for?\n\r", ch );
	return NULL;
    }

    if ( ( pArea = get_clan_area( ch->clan ) ) == NULL )
    {
	send_to_char( "Error looking up clan area.\n\r", ch );
	return NULL;
    }

    number = number_argument( argument, arg );
    count = 0;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pMob = get_mob_index( vnum ) ) != NULL
	&&   is_name( arg, pMob->player_name )
	&&   ++count == number )
	    return pMob;
    }

    send_to_char( "Your clan owns no such mobile.\n\r", ch );
    return NULL;
}

MOB_INDEX_DATA *new_clan_mobile( CHAR_DATA *ch )
{
    MOB_INDEX_DATA *pMob;
    RESET_DATA *pReset;
    int iHash, vnum;

    for ( vnum = ch->in_room->area->min_vnum; ; vnum++ )
    {
	if ( vnum > ch->in_room->area->max_vnum )
	{
	    send_to_char( "Your clan has ran out of free vnums for new mobiles.\n\r", ch );
	    return NULL;
	}

	if ( get_mob_index( vnum ) == NULL )
	    break;
    }

    pMob		= new_mob_index( );
    pMob->vnum		= vnum;
    pMob->area		= ch->in_room->area;
    pMob->description	= &str_empty[0];

    iHash		= vnum % MAX_KEY_HASH;
    pMob->next		= mob_index_hash[iHash];
    mob_index_hash[iHash] = pMob;

    pReset		= new_reset_data( );
    pReset->command	= 'M';
    pReset->arg1	= vnum;
    pReset->arg2	= 1;
    pReset->arg3	= ch->in_room->vnum;
    pReset->arg4	= 1;
    pReset->percent	= 100;

    add_reset( ch->in_room, pReset, 0 );

    clan_table[ch->clan].edit_mob += 60;

    return pMob;
}

OBJ_INDEX_DATA *get_clan_obj( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_INPUT_LENGTH];
    sh_int count, number;
    int vnum;

    if ( argument == NULL || argument[0] == '\0' )
    {
	send_to_char( "Which object are you looking for?\n\r", ch );
	return NULL;
    }

    if ( ( pArea = get_clan_area( ch->clan ) ) == NULL )
    {
	send_to_char( "Error looking up clan area.\n\r", ch );
	return NULL;
    }

    number = number_argument( argument, arg );
    count = 0;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pObj = get_obj_index( vnum ) ) != NULL
	&&   is_name( arg, pObj->name )
	&&   ++count == number )
	    return pObj;
    }

    send_to_char( "Your clan owns no such object.\n\r", ch );
    return NULL;
}

void reset_clan_objects( OBJ_INDEX_DATA *pObj )
{
    OBJ_DATA *obj;
    sh_int pos;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObj )
	{
	    obj->level		= pObj->level;
	    obj->cost		= pObj->cost;
	    obj->wear_flags	= pObj->wear_flags;
	    obj->extra_flags	= pObj->extra_flags;

	    for ( pos = 0; pos < 5; pos++ )
		obj->value[pos] = pObj->value[pos];

	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( pObj->short_descr );

	    free_string( obj->description );
	    obj->description = str_dup( pObj->description );

	    free_string( obj->name );
	    obj->name = str_dup( pObj->name );

	    if ( obj->carried_by != NULL
	    &&   obj->wear_loc != WEAR_NONE )
		reset_char( obj->carried_by );
	}
    }
}

void set_obj_stats( OBJ_INDEX_DATA *pObj )
{
    sh_int pos;

    pObj->cost = pObj->level * 100;

    switch( pObj->item_type )
    {
	default:
	    break;

	case ITEM_ARMOR:
	    for ( pos = 0; pos < 4; pos++ )
		pObj->value[pos] = pObj->level / 3;
	    break;

	case ITEM_WEAPON:
	    pObj->value[1] = pObj->level / 5;
	    pObj->value[2] = pObj->level / 10;
	    break;

	case ITEM_POTION:
	case ITEM_SCROLL:
	case ITEM_PILL:
	case ITEM_WAND:
	case ITEM_STAFF:
	    pObj->value[0] = pObj->level * 11 / 10;
	    break;

	case ITEM_LIGHT:
	    pObj->value[2] = pObj->level * 40;
	    break;

	case ITEM_PIT:
	    pObj->value[0] = pObj->level * 20;
	    pObj->value[3] = pObj->level * 2;
	    break;

	case ITEM_FURNITURE:
	    pObj->value[3] = pObj->level * 10;
	    pObj->value[4] = pObj->level * 10;
	    break;

	case ITEM_CONTAINER:
	    pObj->value[0] = pObj->level * 2;
	    pObj->value[3] = pObj->level * 2;
	    break;

	case ITEM_DRINK_CON:
	    pObj->value[0] = pObj->level * 3;
	    pObj->value[1] = pObj->level * 3;
	    break;

	case ITEM_FOOD:
	    pObj->value[0] = pObj->level / 2;
	    pObj->value[1] = pObj->level / 2;
	    break;
    }
}

void reset_clan_mobs( MOB_INDEX_DATA *pMob )
{
    CHAR_DATA *wch, *wch_next;
    ROOM_INDEX_DATA *pRoom;
    int vnum;

    for ( vnum = pMob->area->min_vnum; vnum <= pMob->area->max_vnum; vnum++ )
    {
	if ( ( pRoom = get_room_index( vnum ) ) != NULL )
	{
	    for ( wch = pRoom->people; wch != NULL; wch = wch_next )
	    {
		wch_next = wch->next_in_room;

		if ( IS_NPC( wch ) && wch->pIndexData == pMob )
		    extract_char( wch, TRUE );
	    }
	}
    }

    reset_area( pMob->area );
}

sh_int check_value( CHAR_DATA *ch, COST_DATA *price, OBJ_INDEX_DATA *pObj, sh_int pos )
{
    sh_int found = 0, value;

    for ( value = 1; value <= 5; value++ )
    {
	if ( value == 5 )
	{
	    send_to_char( "That object already has the maximum number of spells attached.\n\r", ch );
	    return -1;
	}

	if ( pObj->value[value] == pos
	&&   ++found >= price->max )
	{
	    send_to_char( "That object already has the maximum number of that spell on it.\n\r", ch );
	    return -1;
	}

	if ( pObj->value[value] == 0 )
	    return value;
    }

    return -1;
}

sh_int clan_flag_lookup( char *argument, const struct clan_flag_type *table )
{
    sh_int pos;

    for ( pos = 0; table[pos].name != NULL; pos++ )
    {
	if ( !str_prefix( argument, table[pos].name ) )
	    return pos;
    }

    return -1;
}

sh_int apply_flag_lookup( char *argument )
{
    sh_int pos;

    for ( pos = 0; obj_apply_table[pos].name != NULL; pos++ )
    {
	if ( !str_prefix( argument, obj_apply_table[pos].name ) )
	    return pos;
    }

    return -1;
}

sh_int price_lookup( CHAR_DATA *ch, char *argument )
{
    sh_int pos;

    for ( pos = 0; price_table[pos].name != NULL; pos++ )
    {
	if ( !str_prefix( argument, price_table[pos].name ) )
	    return pos;
    }

    send_to_char( "Error looking up price, contact an Immortal.\n\r", ch );
    return -1;
}

bool can_afford( CHAR_DATA *ch, sh_int pos, int mult )
{
    if ( clan_table[ch->clan].cubics < price_table[pos].cost_cubic * mult )
    {
	send_to_char( "Your clan account lacks the cubics for that upgrade.\n\r", ch );
	return FALSE;
    }

    if ( clan_table[ch->clan].aquest < price_table[pos].cost_aquest * mult )
    {
	send_to_char( "Your clan account lacks the aquest points for that upgrade.\n\r", ch );
	return FALSE;
    }

    if ( clan_table[ch->clan].iquest < price_table[pos].cost_iquest * mult )
    {
	send_to_char( "Your clan account lacks the iquest points for that upgrade.\n\r", ch );
	return FALSE;
    }

    return TRUE;
}

void charge_cost( CHAR_DATA *ch, sh_int pos, int mult )
{
    char buf[MAX_STRING_LENGTH];

    clan_table[ch->clan].cubics -= price_table[pos].cost_cubic * mult;
    clan_table[ch->clan].aquest -= price_table[pos].cost_aquest * mult;
    clan_table[ch->clan].iquest -= price_table[pos].cost_iquest * mult;

    sprintf( buf, "\n\rYou clan was charged %d cubics, %d aquest and %d iquest points for your upgrades.\n\r",
	price_table[pos].cost_cubic * mult,
	price_table[pos].cost_aquest * mult,
	price_table[pos].cost_iquest * mult );
    send_to_char( buf, ch );

    clan_log( ch, ch->clan, price_table[pos].name, price_table[pos].cost_cubic * mult,
	price_table[pos].cost_aquest * mult, price_table[pos].cost_iquest * mult );

    mud_stat.clans_changed = TRUE;
}

bool charge_cost_flag( CHAR_DATA *ch, sh_int pos, const struct clan_flag_type *table )
{
    char buf[MAX_STRING_LENGTH];

    if ( clan_table[ch->clan].cubics < table[pos].cost_cubic )
    {
	send_to_char( "Your clan account lacks the cubics for that upgrade.\n\r", ch );
	return FALSE;
    }

    if ( clan_table[ch->clan].aquest < table[pos].cost_aquest )
    {
	send_to_char( "Your clan account lacks the aquest points for that upgrade.\n\r", ch );
	return FALSE;
    }

    if ( clan_table[ch->clan].iquest < table[pos].cost_iquest )
    {
	send_to_char( "Your clan account lacks the iquest points for that upgrade.\n\r", ch );
	return FALSE;
    }

    clan_table[ch->clan].cubics -= table[pos].cost_cubic;
    clan_table[ch->clan].aquest -= table[pos].cost_aquest;
    clan_table[ch->clan].iquest -= table[pos].cost_iquest;

    sprintf( buf, "You clan was charged %d cubics, %d aquest and %d iquest points for your upgrades.\n\r",
	table[pos].cost_cubic,
	table[pos].cost_aquest,
	table[pos].cost_iquest );
    send_to_char( buf, ch );

    clan_log( ch, ch->clan, table[pos].name, table[pos].cost_cubic,
	table[pos].cost_aquest, table[pos].cost_iquest );

    mud_stat.clans_changed = TRUE;
    return TRUE;
}

void make_clan_area( CHAR_DATA *ch )
{
    AREA_DATA *pArea;
    FILE *fp;
    ROOM_INDEX_DATA *pRoom;
    char buf[MAX_STRING_LENGTH];
    int vnum = 60000;

    while ( get_vnum_area( vnum ) != NULL )
	vnum += 50;

    pArea		= new_area( );
    area_last->next	= pArea;
    area_last		= pArea;

    sprintf( buf, "%s Clan Hall", clan_table[ch->clan].name );
    buf[0] = UPPER( buf[0] );
    free_string( pArea->name );
    pArea->name = str_dup( buf );

    sprintf( buf, "%s.cln", clan_table[ch->clan].name );
    if ( ( fp = fopen( buf, "r" ) ) != NULL )
    {
	sh_int count;
	fclose( fp );
	for ( count = 0; ; count++ )
	{
	    sprintf( buf, "%s%d.cln", clan_table[ch->clan].name, count );
	    if ( ( fp = fopen( buf, "r" ) ) == NULL )
		break;
	    fclose( fp );
	}
    }
    free_string( pArea->file_name );
    pArea->file_name = str_dup( buf );

    free_string( pArea->builder );
    pArea->builder = str_dup( ch->name );

    pArea->security	= 9;
    pArea->min_vnum	= vnum;
    pArea->max_vnum	= vnum + 49;
    pArea->alignment	= '?';
    pArea->min_level	= 1;
    pArea->max_level	= LEVEL_HERO;
    pArea->clan		= ch->clan;

    SET_BIT( pArea->area_flags, AREA_SPECIAL );

    pRoom	= new_room_index( );
    pRoom->vnum	= vnum;
    pRoom->area	= pArea;
    pRoom->next	= room_index_hash[vnum % MAX_KEY_HASH];
    room_index_hash[vnum % MAX_KEY_HASH]= pRoom;

    sprintf( buf, "%s Clan Recall", clan_table[ch->clan].name );
    buf[0] = UPPER( buf[0] );
    free_string( pRoom->name );
    pRoom->name = str_dup( buf );

    pRoom->exit[DIR_DOWN]		= new_exit( );
    pRoom->exit[DIR_DOWN]->u1.to_room	= get_room_index( 3014 );
    pRoom->exit[DIR_DOWN]->orig_door	= DIR_DOWN;

    clan_table[ch->clan].hall		= vnum;
    clan_table[ch->clan].edit_clan	= 720;
    clan_table[ch->clan].edit_room	= 720;
    clan_table[ch->clan].two_way_time	= 720;

    save_area( pArea, FALSE );
    save_area_list( );
}

sh_int count_max( CHAR_DATA *ch, int type )
{
    return clan_table[ch->clan].max_mem;
}

sh_int count_none( CHAR_DATA *ch, int type )
{
    return 1;
}

sh_int count_helps( CHAR_DATA *ch, int type )
{
    HELP_DATA *pHelp;

    if ( ch->clan == 0 )
	return 1;

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	if ( pHelp->clan == ch->clan )
	    type++;
    }

    return type;
}

sh_int count_rooms( CHAR_DATA *ch, int type )
{
    AREA_DATA *pArea;
    int vnum;

    if ( ( pArea = get_clan_area( ch->clan ) ) == NULL )
	return 1;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( get_room_index( vnum ) != NULL )
	   type++;
    }

    return type;
}

sh_int count_mobiles( CHAR_DATA *ch, int type )
{
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMob;
    int count = 0, vnum;

    if ( ( pArea = get_clan_area( ch->clan ) ) == NULL )
	return 1;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pMob = get_mob_index( vnum ) ) != NULL
	&&   ( type == COUNT_ALL
	||     ( type == COUNT_SHOP && pMob->pShop != NULL )
	||     ( type == COUNT_GUARD && pMob->pShop == NULL ) ) )
	    count++;
    }

    if ( type != COUNT_ALL )
	count++;

    return UMAX( count, 1 );
}

sh_int count_object( CHAR_DATA *ch, int type )
{
    AREA_DATA *pArea;
    OBJ_INDEX_DATA *pObj;
    int count = 0, vnum;

    if ( ( pArea = get_clan_area( ch->clan ) ) == NULL )
	return 1;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pObj = get_obj_index( vnum ) ) != NULL )
	{
	    if ( type == COUNT_ALL || pObj->item_type == type )
		count++;
	}
    }

    if ( type != COUNT_ALL )
	count++;

    return UMAX( count, 1 );
}

CLAN_COMMAND( add_spell )
{
    COST_DATA *price;
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_INPUT_LENGTH];
    sh_int pos, value;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: add-spell <object> <spell>.\n\r", ch );
	return FALSE;
    }

    if ( ( pObj = get_clan_obj( ch, arg ) ) == NULL )
	return FALSE;

    if ( ( pos = skill_lookup( argument ) ) == -1 )
    {
	send_to_char( "Invalid spell.\n\r", ch );
	return FALSE;
    }

    switch( pObj->item_type )
    {
	default:
	    send_to_char( "You can not add spells to that object type!\n\r", ch );
	    return FALSE;

	case ITEM_WAND:
	    if ( pObj->value[3] != 0 )
	    {
		send_to_char( "That object already has a spell attached to it.\n\r", ch );
		return FALSE;
	    }
	    value = 3;
	    price = skill_table[pos].cost_wand;
	    break;

	case ITEM_STAFF:
	    if ( pObj->value[3] != 0 )
	    {
		send_to_char( "That object already has a spell attached to it.\n\r", ch );
		return FALSE;
	    }
	    value = 3;
	    price = skill_table[pos].cost_staff;
	    break;

	case ITEM_POTION:
	    price = skill_table[pos].cost_potion;
	    value = check_value( ch, price, pObj, pos );
	    break;

	case ITEM_SCROLL:
	    price = skill_table[pos].cost_scroll;
	    value = check_value( ch, price, pObj, pos );
	    break;

	case ITEM_PILL:
	    price = skill_table[pos].cost_pill;
	    value = check_value( ch, price, pObj, pos );
	    break;
    }

    if ( value == -1 )
	return FALSE;

    if ( price == NULL )
    {
	send_to_char( "That spell can not be added to that item type.\n\r", ch );
	return FALSE;
    }

    if ( pObj->level < price->level )
    {
	send_to_char( "Object level is less than minimum level required to hold that spell.\n\r", ch );
	return FALSE;
    }

    if ( clan_table[ch->clan].cubics < price->cubic )
    {
	send_to_char( "Your clan does not have enough cubics for that upgrade.\n\r", ch );
	return FALSE;
    }

    if ( clan_table[ch->clan].aquest < price->aquest )
    {
	send_to_char( "Your clan does not have enough aquest points for that upgrade.\n\r", ch );
	return FALSE;
    }

    if ( clan_table[ch->clan].iquest < price->iquest )
    {
	send_to_char( "Your clan does not have enough iquest points for that upgrade.\n\r", ch );
	return FALSE;
    }

    pObj->value[value] = pos;

    clan_table[ch->clan].cubics -= price->cubic;
    clan_table[ch->clan].aquest -= price->aquest;
    clan_table[ch->clan].iquest -= price->iquest;

    sprintf( arg, "You clan was charged %d cubics, %d aquest and %d iquest points for your upgrade.\n\r",
	price->cubic, price->aquest, price->iquest );
    send_to_char( arg, ch );

    clan_log( ch, ch->clan, skill_table[pos].name, price->cubic, price->aquest, price->iquest );

    mud_stat.clans_changed = TRUE;
    reset_clan_objects( pObj );

    return TRUE;
}

CLAN_COMMAND( apply_object )
{
    AFFECT_DATA *paf, *new_af = NULL;
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_INPUT_LENGTH];
    sh_int pos, count = 0;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "eq-apply <object> <type>.\n\r\n\r", ch );
	return FALSE;
    }

    if ( ( pObj = get_clan_obj( ch, arg ) ) == NULL )
	return FALSE;

    switch( pObj->item_type )
    {
	default:
	    send_to_char( "You can't add affect modifiers to that object type.\n\r", ch );
	    return FALSE;

	case ITEM_ARMOR:
	case ITEM_WEAPON:
	case ITEM_LIGHT:
	case ITEM_JEWELRY:
	case ITEM_CONTAINER:
	    break;
    }

    if ( ( pos = apply_flag_lookup( argument ) ) == -1 )
    {
	send_to_char( "That apply upgrade is not available.\n\r", ch );
	return FALSE;
    }

    if ( clan_table[ch->clan].cubics < obj_apply_table[pos].cost_cubic )
    {
	send_to_char( "Your clan does not have enough cubics for that upgrade.\n\r", ch );
	return FALSE;
    }

    if ( clan_table[ch->clan].aquest < obj_apply_table[pos].cost_aquest )
    {
	send_to_char( "Your clan does not have enough aquest points for that upgrade.\n\r", ch );
	return FALSE;
    }

    if ( clan_table[ch->clan].iquest < obj_apply_table[pos].cost_iquest )
    {
	send_to_char( "Your clan does not have enough iquest points for that upgrade.\n\r", ch );
	return FALSE;
    }

    for ( paf = pObj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->where == TO_OBJECT )
	{
	    count++;
	    if ( paf->location == obj_apply_table[pos].where )
	    {
		new_af = paf;
		if ( abs( paf->modifier ) >= abs( obj_apply_table[pos].max ) )
		{
		    send_to_char( "That affect is already maxed out on that object.\n\r", ch );
		    return FALSE;
		}
	    }
	}
    }

    if ( new_af != NULL )
	new_af->modifier += obj_apply_table[pos].mod;
    else
    {
	if ( count >= pObj->level / 15 )
	{
	    send_to_char( "Max number of affects already added to that object.\n\r", ch );
	    return FALSE;
	}

	new_af			= new_affect( );
	new_af->where		= TO_OBJECT;
	new_af->location	= obj_apply_table[pos].where;
	new_af->modifier	= obj_apply_table[pos].mod;
	new_af->dur_type	= DUR_TICKS;
	new_af->bitvector	= 0;
	new_af->duration	= -1;
	new_af->type		= -1;
	new_af->level		= pObj->level;
	new_af->next		= pObj->affected;
	pObj->affected		= new_af;
    }

    clan_table[ch->clan].cubics -= obj_apply_table[pos].cost_cubic;
    clan_table[ch->clan].aquest -= obj_apply_table[pos].cost_aquest;
    clan_table[ch->clan].iquest -= obj_apply_table[pos].cost_iquest;

    sprintf( arg, "You clan was charged %d cubics, %d aquest and %d iquest points for your upgrades.\n\r",
	obj_apply_table[pos].cost_cubic,
	obj_apply_table[pos].cost_aquest,
	obj_apply_table[pos].cost_iquest );
    send_to_char( arg, ch );

    clan_log( ch, ch->clan, obj_apply_table[pos].name, obj_apply_table[pos].cost_cubic,
	obj_apply_table[pos].cost_aquest, obj_apply_table[pos].cost_iquest );

    mud_stat.clans_changed = TRUE;
    reset_clan_objects( pObj );

    return TRUE;
}

CLAN_COMMAND( disband_clan )
{
    ch->desc->editor = ED_CLAN;
    ch->desc->pEdit = (void *)(int)ch->clan;

    send_to_char( "{RWARNING: {GYou are about to {RPERMANENTLY DELETE{G your clan!\n"
		  "{RWARNING: {GThis will delete your {RENTIRE{G clan, including {RCLAN HALL{G and {RBANK ACCOUNTS{G!\n"
		  "{RWARNING: {GTo confirm this command type '{RDELETE <clan name>{G' now!\n"
		  "{RWARNING: {GTo abort this process type '{RDONE{G'!{x\n\r", ch );

    return TRUE;
}

CLAN_COMMAND( edit_clan )
{
    if ( clan_table[ch->clan].edit_clan <= 0 )
    {
	send_to_char( "Your clan edit time has expired.\n\r", ch );
	return FALSE;
    }

    ch->desc->editor = ED_CLAN;
    ch->desc->pEdit = (void *)(int)ch->clan;
    clan_edit_show( ch, "" );
    return FALSE;
}

CLAN_COMMAND( edit_help )
{
    HELP_DATA *pHelp;

    if ( ( pHelp = get_clan_help( ch, argument ) ) == NULL )
	return FALSE;

    if ( clan_table[ch->clan].edit_help <= 0 )
    {
	send_to_char( "Your help file edit time has expired.\n\r", ch );
	return FALSE;
    }

    ch->desc->pEdit = (void *)pHelp;
    ch->desc->editor = ED_HELP;
    hedit_show( ch, "" );
    return FALSE;
}

CLAN_COMMAND( edit_mobile )
{
    MOB_INDEX_DATA *pMob;

    if ( ( pMob = get_clan_mob( ch, argument ) ) == NULL )
	return FALSE;

    if ( clan_table[ch->clan].edit_mob <= 0 )
    {
	send_to_char( "Your mobile edit time has expired.\n\r", ch );
	return FALSE;
    }

    ch->desc->pEdit = (void *)pMob;
    ch->desc->editor = ED_MOBILE;
    medit_show( ch, "" );
    return FALSE;
}

CLAN_COMMAND( edit_object )
{
    OBJ_INDEX_DATA *pObj;

    if ( ( pObj = get_clan_obj( ch, argument ) ) == NULL )
	return FALSE;

    if ( clan_table[ch->clan].edit_obj <= 0 )
    {
	send_to_char( "Your object edit time has expired.\n\r", ch );
	return FALSE;
    }

    ch->desc->pEdit = (void *)pObj;
    ch->desc->editor = ED_OBJECT;
    oedit_show( ch, "" );
    return FALSE;
}

CLAN_COMMAND( edit_room )
{
    if ( clan_table[ch->clan].edit_room <= 0 )
    {
	send_to_char( "Your room edit time has expired.\n\r", ch );
	return FALSE;
    }

    if ( ch->in_room->area->clan != ch->clan )
    {
	send_to_char( "This room does not appear to belong to your clan.\n\r", ch );
	return FALSE;
    }

    ch->desc->editor = ED_ROOM;
    ch->desc->pEdit = (void *)ch->in_room;
    redit_show( ch, "" );
    return FALSE;
}

CLAN_COMMAND( equip_guard )
{
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMob;
    OBJ_INDEX_DATA *pObj;
    RESET_DATA *pReset;
    ROOM_INDEX_DATA *pRoom;
    char arg[MAX_INPUT_LENGTH];
    int wear_loc, vnum;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: equip-guard <guard> <object>.\n\r", ch );
	return FALSE;
    }

    if ( ( pArea = get_clan_area( ch->clan ) ) == NULL )
    {
	send_to_char( "Error looking up clan area.\n\r", ch );
	return FALSE;
    }

    if ( ( pMob = get_clan_mob( ch, arg ) ) == NULL )
	return FALSE;

    if ( ( pObj = get_clan_obj( ch, argument ) ) == NULL )
	return FALSE;

    if ( pMob->pShop != NULL )
    {
	send_to_char( "You can't equip shopkeepers.\n\r", ch );
	return FALSE;
    }

    if ( pObj->item_type == ITEM_LIGHT )
	wear_loc = WEAR_LIGHT;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_FINGER ) )
	wear_loc = WEAR_FINGER_L;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_NECK ) )
	wear_loc = WEAR_NECK_1;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_BODY ) )
	wear_loc = WEAR_BODY;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_HEAD ) )
	wear_loc = WEAR_HEAD;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_LEGS ) )
	wear_loc = WEAR_LEGS;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_FEET ) )
	wear_loc = WEAR_FEET;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_HANDS ) )
	wear_loc = WEAR_HANDS;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_ARMS ) )
	wear_loc = WEAR_ARMS;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_SHIELD ) )
	wear_loc = WEAR_SHIELD;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_ABOUT ) )
	wear_loc = WEAR_ABOUT;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_WAIST ) )
	wear_loc = WEAR_WAIST;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_WRIST ) )
	wear_loc = WEAR_WRIST_L;

    else if ( IS_SET( pObj->wear_flags, ITEM_WIELD ) )
	wear_loc = WEAR_WIELD;

    else if ( IS_SET( pObj->wear_flags, ITEM_HOLD ) )
	wear_loc = WEAR_HOLD;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_ANKLE ) )
	wear_loc = WEAR_ANKLE_L;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_EAR ) )
	wear_loc = WEAR_EAR_L;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_CHEST ) )
	wear_loc = WEAR_CHEST;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_FLOAT ) )
	wear_loc = WEAR_FLOAT;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_EYES ) )
	wear_loc = WEAR_EYES;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_FACE ) )
	wear_loc = WEAR_FACE;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_CLAN ) )
	wear_loc = WEAR_CLAN;

    else if ( IS_SET( pObj->wear_flags, ITEM_WEAR_BACK ) )
	wear_loc = WEAR_BACK;

    else
    {
	send_to_char( "That object is not wearable.\n\r", ch );
	return FALSE;
    }

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pRoom = get_room_index( vnum ) ) != NULL )
	{
	    sh_int count = 1;

	    for ( pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next )
	    {
		count++;

		if ( pReset->command == 'M'
		&&   pReset->arg1 == pMob->vnum )
		{
		    for ( pReset = pReset->next; pReset != NULL && pReset->command == 'E'; pReset = pReset->next )
		    {
			if ( pReset->arg2 == wear_loc
			||   pReset->arg3 == wear_loc )
			{
			    send_to_char( "That guard appears to already have an object equipped in the wear location.\n\r", ch );
			    return FALSE;
			}
		    }

		    pReset		= new_reset_data( );
		    pReset->command	= 'E';
		    pReset->percent	= 100;
		    pReset->arg1	= pObj->vnum;
		    pReset->arg2	= wear_loc;
		    pReset->arg3	= wear_loc;
		    pReset->arg4	= 0;

		    add_reset( pRoom, pReset, count );

		    reset_clan_mobs( pMob );
		    return TRUE;
		}		
	    }
	}
    }

    return FALSE;
}

CLAN_COMMAND( flag_exit )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int door, pos;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "flag-exit <direction> <flag>.\n\r", ch );
	return FALSE;
    }

    else if ( !str_prefix( arg, "north" ) )	door = DIR_NORTH;
    else if ( !str_prefix( arg, "south" ) )	door = DIR_SOUTH;
    else if ( !str_prefix( arg, "west" ) )	door = DIR_WEST;
    else if ( !str_prefix( arg, "east" ) )	door = DIR_EAST;
    else if ( !str_prefix( arg, "down" ) )	door = DIR_DOWN;
    else if ( !str_prefix( arg, "up" ) )	door = DIR_UP;
    else
    {
	flag_exit( ch, "", type );
	return FALSE;
    }

    if ( ( pos = clan_flag_lookup( argument, exit_flag_table ) ) == -1 )
    {
	send_to_char( "That flag upgrade is not available.\n\r", ch );
	return FALSE;
    }

    if ( ch->in_room->exit[door] == NULL )
    {
	send_to_char( "There is no exit in that direction.\n\r", ch );
	return FALSE;
    }

    if ( IS_SET( ch->in_room->exit[door]->rs_flags, exit_flag_table[pos].bit ) )
    {
	send_to_char( "That exit already has that flag.\n\r", ch );
	return FALSE;
    }

    if ( exit_flag_table[pos].restrict
    &&   !IS_SET( ch->in_room->exit[door]->rs_flags, exit_flag_table[pos].restrict ) )
    {
	send_to_char( "That exit is missing the requirement flag.\n\r", ch );
	return FALSE;
    }

    if ( !charge_cost_flag( ch, pos, exit_flag_table ) )
	return FALSE;

    SET_BIT( ch->in_room->exit[door]->rs_flags, exit_flag_table[pos].bit );
    ch->in_room->exit[door]->exit_info = ch->in_room->exit[door]->rs_flags;

    if ( ch->in_room->exit[door]->u1.to_room != NULL
    &&   ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]] != NULL
    &&   ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]]->u1.to_room == ch->in_room )
    {
	SET_BIT( ch->in_room->exit[door]->u1.to_room->area->area_flags, AREA_CHANGED );
	SET_BIT( ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]]->rs_flags, exit_flag_table[pos].bit );
	ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]]->exit_info =
	ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]]->rs_flags;
    }

    return TRUE;
}

CLAN_COMMAND( flag_guard )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    sh_int pos;
    long bits;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "flag-guard <mobile> <flag>.\n\r", ch );
	return FALSE;
    }

    if ( ( pMob = get_clan_mob( ch, arg ) ) == NULL )
	return FALSE;

    if ( ( pos = clan_flag_lookup( argument, mob_flag_table ) ) == -1 )
    {
	send_to_char( "That flag upgrade is not available.\n\r", ch );
	return FALSE;
    }

    if ( mob_flag_table[pos].where == TO_AFFECTS )
	bits = pMob->affected_by;
    else if ( mob_flag_table[pos].where == TO_SHIELDS )
	bits = pMob->shielded_by;
    else if ( mob_flag_table[pos].where == TO_ACT )
	bits = pMob->act;
    else
    {
	send_to_char( "Error: invalid bit setting.\n\r", ch );
	return FALSE;
    }

    if ( IS_SET( bits, mob_flag_table[pos].bit ) )
    {
	send_to_char( "The mobile already has that flag.\n\r", ch );
	return FALSE;
    }

    if ( IS_SET( bits, mob_flag_table[pos].restrict ) )
    {
	send_to_char( "Mobile has restricted flag on, can not have both flags.\n\r", ch );
	return FALSE;
    }

    if ( !charge_cost_flag( ch, pos, mob_flag_table ) )
	return FALSE;

    if ( mob_flag_table[pos].where == TO_AFFECTS )
	SET_BIT( pMob->affected_by, mob_flag_table[pos].bit );
    else if ( mob_flag_table[pos].where == TO_SHIELDS )
	SET_BIT( pMob->shielded_by, mob_flag_table[pos].bit );
    else if ( mob_flag_table[pos].where == TO_ACT )
	SET_BIT( pMob->act, mob_flag_table[pos].bit );

    reset_clan_mobs( pMob );

    return TRUE;
}

CLAN_COMMAND( flag_object )
{
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_INPUT_LENGTH];
    sh_int pos;
    long bits;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "flag-object <object> <flag>.\n\r", ch );
	return FALSE;
    }

    if ( ( pObj = get_clan_obj( ch, arg ) ) == NULL )
	return FALSE;

    if ( ( pos = clan_flag_lookup( argument, obj_flag_table ) ) == -1 )
    {
	send_to_char( "That flag upgrade is not available.\n\r", ch );
	return FALSE;
    }

    if ( obj_flag_table[pos].where == TO_OBJECT )
	bits = pObj->extra_flags;
    else if ( obj_flag_table[pos].where == TO_WEAPON )
    {
	if ( pObj->item_type != ITEM_WEAPON )
	{
	    send_to_char( "That flag is reserved for weapons.\n\r", ch );
	    return FALSE;
	}
	bits = pObj->value[4];
    }
    else
    {
	send_to_char( "Error: invalid bit setting.\n\r", ch );
	return FALSE;
    }

    if ( IS_SET( bits, obj_flag_table[pos].bit ) )
    {
	send_to_char( "The object already has that flag.\n\r", ch );
	return FALSE;
    }

    if ( IS_SET( bits, obj_flag_table[pos].restrict ) )
    {
	send_to_char( "Object has restricted flag on, can not have both flags.\n\r", ch );
	return FALSE;
    }

    if ( !charge_cost_flag( ch, pos, obj_flag_table ) )
	return FALSE;

    if ( obj_flag_table[pos].where == TO_OBJECT )
	SET_BIT( pObj->extra_flags, obj_flag_table[pos].bit );
    else if ( obj_flag_table[pos].where == TO_WEAPON )
	SET_BIT( pObj->value[4], obj_flag_table[pos].bit );

    reset_clan_objects( pObj );

    return TRUE;
}

CLAN_COMMAND( flag_room )
{
    sh_int pos;

    if ( argument[0] == '\0' )
    {
	send_to_char( "flag-room <flag>.\n\r", ch );
	return FALSE;
    }

    if ( ( pos = clan_flag_lookup( argument, room_flag_table ) ) == -1 )
    {
	send_to_char( "That flag upgrade is not available.\n\r", ch );
	return FALSE;
    }

    if ( IS_SET( ch->in_room->room_flags, room_flag_table[pos].bit ) )
    {
	send_to_char( "This room already has that flag.\n\r", ch );
	return FALSE;
    }

    if ( IS_SET( ch->in_room->room_flags, room_flag_table[pos].restrict ) )
    {
	send_to_char( "Room has restricted flag on, can not have both flags.\n\r", ch );
	return FALSE;
    }

    if ( !charge_cost_flag( ch, pos, room_flag_table ) )
	return FALSE;

    SET_BIT( ch->in_room->room_flags, room_flag_table[pos].bit );

    return TRUE;
}

CLAN_COMMAND( level_guard )
{
    MOB_INDEX_DATA *pMob;
    char buf[MAX_STRING_LENGTH];
    sh_int cost, pos;

    if ( ( pos = price_lookup( ch, "level-guard" ) ) == -1 )
	return FALSE;

    if ( argument[0] == '\0' )
    {
	BUFFER *final = new_buf( );
	bool found = FALSE;
	int vnum;

	add_buf( final, "{qMobile (Cost = level / 20 * Base) {s[{tLvl{s/{qMax{s]  {tx {s[{q Cubics{s] [{q  Aquest{s] [{q Iquest{s]\n\r" );
	add_buf( final, "-----------------------------------------------------------------------------\n\r" );

	for ( vnum = ch->in_room->area->min_vnum; vnum <= ch->in_room->area->max_vnum; vnum++ )
	{
	    if ( ( pMob = get_mob_index( vnum ) ) != NULL
	    &&   pMob->pShop == NULL )
	    {
		cost = pMob->level / 20;

		sprintf( buf, "{q%s {s[{t%3d{s/{q140{s] {t%2d {s[{t%3d{s/{q%3d{s] [{t%3d{s/{q%4d{s] [{t%3d{s/{q%3d{s]\n\r",
		    end_string( pMob->short_descr, 33 ),
		    pMob->level,
		    cost,
		    price_table[pos].cost_cubic,
		    cost * price_table[pos].cost_cubic,
		    price_table[pos].cost_aquest,
		    cost * price_table[pos].cost_aquest,
		    price_table[pos].cost_iquest,
		    cost * price_table[pos].cost_iquest );
		add_buf( final, buf );
		found = TRUE;
	    }
	}

	if ( !found )
	{
	    sprintf( buf, "{qBase Cost                         {s[{t 20{s/{q140{s] {t 1 {s[{t---{s/{q%3d{s] [{t---{s/{q%4d{s] [{t---{s/{q%3d{s]\n\r",
		price_table[pos].cost_cubic,
		price_table[pos].cost_aquest,
		price_table[pos].cost_iquest );
	    add_buf( final, buf );
	}
	else
	    add_buf( final, "{x" );

	page_to_char( final->string, ch );
	free_buf( final );

	return FALSE;
    }

    if ( ( pMob = get_clan_mob( ch, argument ) ) == NULL )
	return FALSE;

    if ( pMob->level >= 140 )
    {
	send_to_char( "That guard has reached the maximum amount of level upgrades.\n\r", ch );
	return FALSE;
    }

    if ( pMob->pShop != NULL )
    {
	send_to_char( "You can't upgrade shopkeepers.\n\r", ch );
	return FALSE;
    }

    cost = pMob->level / 20;

    if ( !can_afford( ch, pos, cost ) )
	return FALSE;

    charge_cost( ch, pos, cost );

    pMob->level		+= 20;
    pMob->hit[0]	= pMob->level * 150;
    pMob->hit[1]	= pMob->level * 150;
    pMob->mana[0]	= pMob->level * 150;
    pMob->mana[1]	= pMob->level * 150;
    pMob->damage[0]	= pMob->level / 5;
    pMob->damage[1]	= pMob->level / 10;
    pMob->damage[2]	= pMob->level * 5 / 2;
    pMob->hitroll	= pMob->level * 2;
    pMob->saves		= -pMob->level / 4;

    for ( pos = 0; pos < 4; pos++ )
	pMob->ac[pos] = -pMob->level * 20;

    reset_clan_mobs( pMob );

    sprintf( buf, "Upgraded %s to level %d.\n\r",
	pMob->short_descr, pMob->level );
    send_to_char( buf, ch );

    return TRUE;
}

CLAN_COMMAND( level_object )
{
    OBJ_INDEX_DATA *pObj;
    char buf[MAX_STRING_LENGTH];
    sh_int cost, pos;

    if ( ( pos = price_lookup( ch, "level-object" ) ) == -1 )
	return FALSE;

    if ( argument[0] == '\0' )
    {
	BUFFER *final = new_buf( );
	bool found = FALSE;
	int vnum;

	add_buf( final, "{qObject (Cost = level / 20 * Base) {s[{tLvl{s/{qMax{s]  {tx {s[{q Cubics{s] [{q  Aquest{s] [{q Iquest{s]\n\r" );
	add_buf( final, "-----------------------------------------------------------------------------\n\r" );

	for ( vnum = ch->in_room->area->min_vnum; vnum <= ch->in_room->area->max_vnum; vnum++ )
	{
	    if ( ( pObj = get_obj_index( vnum ) ) != NULL )
	    {
		cost = pObj->level / 20;

		sprintf( buf, "{q%s {s[{t%3d{s/{q101{s] {t%2d {s[{t%3d{s/{q%3d{s] [{t%3d{s/{q%4d{s] [{t%3d{s/{q%3d{s]\n\r",
		    end_string( pObj->short_descr, 33 ),
		    pObj->level,
		    cost,
		    price_table[pos].cost_cubic,
		    cost * price_table[pos].cost_cubic,
		    price_table[pos].cost_aquest,
		    cost * price_table[pos].cost_aquest,
		    price_table[pos].cost_iquest,
		    cost * price_table[pos].cost_iquest );
		add_buf( final, buf );
		found = TRUE;
	    }
	}

	if ( !found )
	{
	    sprintf( buf, "{qBase Cost                         {s[{t 1{s/{q101{s] {t 1 {s[{t---{s/{q%3d{s] [{t---{s/{q%4d{s] [{t---{s/{q%3d{s]\n\r",
		price_table[pos].cost_cubic,
		price_table[pos].cost_aquest,
		price_table[pos].cost_iquest );
	    add_buf( final, buf );
	}
	else
	    add_buf( final, "{x" );

	page_to_char( final->string, ch );
	free_buf( final );

	return FALSE;
    }

    if ( ( pObj = get_clan_obj( ch, argument ) ) == NULL )
	return FALSE;

    if ( pObj->level >= 101 )
    {
	send_to_char( "That object has reached the maximum amount of level upgrades.\n\r", ch );
	return FALSE;
    }

    cost = pObj->level / 20;

    if ( !can_afford( ch, pos, cost ) )
	return FALSE;

    charge_cost( ch, pos, cost );

    pObj->level	+= 20;

    set_obj_stats( pObj );
    reset_clan_objects( pObj );

    sprintf( buf, "Upgraded %s to level %d.\n\r",
	pObj->short_descr, pObj->level );
    send_to_char( buf, ch );

    return TRUE;
}

CLAN_COMMAND( list_exit_flag )
{
    BUFFER *final = new_buf( );
    char buf[MAX_STRING_LENGTH];
    sh_int pos;

    add_buf( final, "{tUpgrade              {qCubics AQuest IQuest {s[{tRequires                           {s]\n\r" );
    add_buf( final, "{s-------------------------------------------------------------------------------\n\r" );

    for ( pos = 0; exit_flag_table[pos].name != NULL; pos++ )
    {
	sprintf( buf, "{t%-20.20s    {q%3d   %4d     %2d {s[{t%-35s{s]\n\r",
	    exit_flag_table[pos].name,
	    exit_flag_table[pos].cost_cubic,
	    exit_flag_table[pos].cost_aquest,
	    exit_flag_table[pos].cost_iquest,
	    flag_string( exit_flags, exit_flag_table[pos].restrict ) );
	add_buf( final, buf );
    }

    add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );
    return FALSE;
}

CLAN_COMMAND( list_helps )
{
    BUFFER *final = new_buf( );
    HELP_DATA *pHelp;
    char buf[MAX_STRING_LENGTH];
    sh_int i = 0;

    if ( ch->clan != 0 )
    {
	add_buf( final, "{s[{tNum{s] {qNAME                           {s[{tKEYWORDS                                {s]\n\r"
			"{s-------------------------------------------------------------------------------\n\r" );

	for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
	{
	    if ( pHelp->clan == ch->clan )
	    {
		i++;

		sprintf( buf, "{s[{t%3d{s] {q%s {s[{t%-40.40s{s]\n\r",
		    i, end_string( pHelp->name, 30 ), pHelp->keyword );
		add_buf( final, buf );
	    }
	}

	add_buf( final, "{x" );
    }

    if ( i == 0 )
	send_to_char( "No matches found.\n\r", ch );
    else
	page_to_char( final->string, ch );

    free_buf( final );

    return FALSE;
}

CLAN_COMMAND( list_mflag )
{
    BUFFER *final = new_buf( );
    char buf[MAX_STRING_LENGTH];
    sh_int pos;

    add_buf( final, "{tUpgrade              {qCubics AQuest IQuest {s[{tRestrictions                       {s]\n\r" );
    add_buf( final, "{s-------------------------------------------------------------------------------\n\r" );

    for ( pos = 0; mob_flag_table[pos].name != NULL; pos++ )
    {
	sprintf( buf, "{t%-20.20s    {q%3d   %4d     %2d {s[{t%-35s{s]\n\r",
	    mob_flag_table[pos].name,
	    mob_flag_table[pos].cost_cubic,
	    mob_flag_table[pos].cost_aquest,
	    mob_flag_table[pos].cost_iquest,
	    flag_string( mob_flag_table[pos].where == TO_AFFECTS ? affect_flags :
		mob_flag_table[pos].where == TO_SHIELDS ? shield_flags : act_flags, mob_flag_table[pos].restrict ) );
	add_buf( final, buf );
    }

    add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );
    return FALSE;
}

CLAN_COMMAND( list_mobiles )
{
    AREA_DATA *pArea;
    BUFFER *final;
    MOB_INDEX_DATA *pMob;
    char buf[MAX_STRING_LENGTH];
    sh_int count = 0, mobs;
    int vnum;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: list-mobiles [all/guards/shops]\n\r", ch );
	return FALSE;
    }

    else if ( !str_prefix( argument, "all" ) )
	mobs = COUNT_ALL;

    else if ( !str_prefix( argument, "guards" ) )
	mobs = COUNT_GUARD;

    else if ( !str_prefix( argument, "shops" ) || !str_prefix( argument, "shopkeepers" ) )
	mobs = COUNT_SHOP;

    else
    {
	list_mobiles( ch, "", type );
	return FALSE;
    }

    if ( ( pArea = get_clan_area( ch->clan ) ) == NULL )
    {
	send_to_char( "Error looking up clan area.\n\r", ch );
	return FALSE;
    }

    final = new_buf( );

    add_buf( final, " {t#{s) {qName                 {tType       {s[{qLvl{s] {tShort Description\n\r"
		    "{s------------------------------------------------------------------------------\n\r" );

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pMob = get_mob_index( vnum ) ) != NULL )
	{
	    if ( mobs == COUNT_ALL
	    ||   ( mobs == COUNT_SHOP && pMob->pShop != NULL )
	    ||   ( mobs == COUNT_GUARD && pMob->pShop == NULL ) )
	    {
		count++;

		sprintf( buf, "{t%2d{s) {q%-20.20s {t%-10s {s[{q%3d{s] {t%s\n\r",
		    count, pMob->player_name,
		    pMob->pShop != NULL ? "SHOPKEEPER" : "GUARD",
		    pMob->level, pMob->short_descr );
		add_buf( final, buf );
	    }
	}
    }

    add_buf( final, "{x" );
    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

CLAN_COMMAND( list_oapply )
{
    BUFFER *final = new_buf( );
    char buf[150];
    sh_int pos;

    add_buf( final, "{tAffect               {s[{qEach{s]  [{q Max{s]  [{qCubic{s] [{qAQuest{s] [{qIquest{s]\n\r"
		    "--------------------------------------------------------------\n\r" );

    for ( pos = 0; obj_apply_table[pos].name != NULL; pos++ )
    {
	sprintf( buf, "{t%-20s {s[{q%4d{s]  [{q%4d{s]  [{q%5d{s] [{q %5d{s] [{q %5d{s]\n\r",
	    obj_apply_table[pos].name, obj_apply_table[pos].mod,
	    obj_apply_table[pos].max, obj_apply_table[pos].cost_cubic,
	    obj_apply_table[pos].cost_aquest, obj_apply_table[pos].cost_iquest );
	add_buf( final, buf );
    }

    add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

CLAN_COMMAND( list_objects )
{
    AREA_DATA *pArea;
    BUFFER *final;
    OBJ_INDEX_DATA *pObj;
    char buf[MAX_STRING_LENGTH];
    sh_int count = 0, objects;
    int vnum;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: list-objects [all/<item type>]\n\r", ch );
	return FALSE;
    }

    else if ( !str_cmp( argument, "all" ) )
	objects = COUNT_ALL;

    else if ( ( objects = flag_value( type_flags, argument ) ) == NO_FLAG )
    {
	list_objects( ch, "", type );
	return FALSE;
    }

    if ( ( pArea = get_clan_area( ch->clan ) ) == NULL )
    {
	send_to_char( "Error looking up clan area.\n\r", ch );
	return FALSE;
    }

    final = new_buf( );

    add_buf( final, " {t#{s) {qName                 {tType       {s[{qLvl{s] {tShort Description\n\r"
		    "{s------------------------------------------------------------------------------\n\r" );

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pObj = get_obj_index( vnum ) ) != NULL )
	{
	    if ( objects == COUNT_ALL || pObj->item_type == objects )
	    {
		count++;

		sprintf( buf, "{t%2d{s) {q%-20.20s {t%-10.10s {s[{q%3d{s] {t%s\n\r",
		    count, pObj->name,
		    flag_string( type_flags, pObj->item_type ),
		    pObj->level, pObj->short_descr );
		add_buf( final, buf );
	    }
	}
    }

    add_buf( final, "{x" );
    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

CLAN_COMMAND( list_oflag )
{
    BUFFER *final = new_buf( );
    char buf[MAX_STRING_LENGTH];
    sh_int pos;

    add_buf( final, "{tUpgrade              {qCubics AQuest IQuest {s[{tRestrictions                       {s]\n\r" );
    add_buf( final, "{s-------------------------------------------------------------------------------\n\r" );

    for ( pos = 0; obj_flag_table[pos].name != NULL; pos++ )
    {
	sprintf( buf, "{t%-20.20s    {q%3d   %4d     %2d {s[{t%-35s{s]\n\r",
	    obj_flag_table[pos].name,
	    obj_flag_table[pos].cost_cubic,
	    obj_flag_table[pos].cost_aquest,
	    obj_flag_table[pos].cost_iquest,
	    flag_string( obj_flag_table[pos].where == TO_OBJECT ? extra_flags : weapon_type2, obj_flag_table[pos].restrict ) );
	add_buf( final, buf );
    }

    add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );
    return FALSE;
}

CLAN_COMMAND( list_rflag )
{
    BUFFER *final = new_buf( );
    char buf[MAX_STRING_LENGTH];
    sh_int pos;

    add_buf( final, "{tUpgrade              {qCubics AQuest IQuest {s[{tRestrictions                       {s]\n\r" );
    add_buf( final, "{s-------------------------------------------------------------------------------\n\r" );

    for ( pos = 0; room_flag_table[pos].name != NULL; pos++ )
    {
	sprintf( buf, "{t%-20.20s   {q%4d   %4d     %2d {s[{t%-35s{s]\n\r",
	    room_flag_table[pos].name,
	    room_flag_table[pos].cost_cubic,
	    room_flag_table[pos].cost_aquest,
	    room_flag_table[pos].cost_iquest,
	    flag_string( room_flags, room_flag_table[pos].restrict ) );
	add_buf( final, buf );
    }

    add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );
    return FALSE;
}

CLAN_COMMAND( list_rooms )
{
    AREA_DATA *pArea;
    BUFFER *final;
    ROOM_INDEX_DATA *pRoom;
    char buf[MAX_STRING_LENGTH], flags[20];
    sh_int count = 0;
    int vnum;

    if ( ( pArea = get_clan_area( ch->clan ) ) == NULL )
    {
	send_to_char( "Error looking up clan area.\n\r", ch );
	return FALSE;
    }

    final = new_buf( );

    add_buf( final, "{q     A------------- {tActive Portal Room\n\r"
		    "{q      C------------ {tCursed No Recall Room\n\r"
		    "{q       D----------- {tDark\n\r"
		    "{q        H---------- {tHealing Room\n\r"
		    "{q         I--------- {tIcy Room\n\r"
		    "{q          M-------- {tNo Mobs\n\r"
		    "{q           N------- {tIndoors\n\r"
		    "{q            O------ {tPortal Room\n\r"
		    "{q             P----- {tPrivate Room\n\r"
		    "{q              R---- {tSet Recall Room\n\r"
		    "{q               S--- {tSafe Room\n\r"
		    "{q                T-- {tTwo Way Link Room\n\r"
		    "{q                 W- {tNo Where Room\n\r\n\r" 
    		    " {t#{s) [{qACDHIMNOPRSTW{s] {tRoom Name\n\r"
		    "{s-----------------------------------------------------------\n\r" );

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pRoom = get_room_index( vnum ) ) != NULL )
	{
	    count++;

	    sprintf( flags, "[{qACDHIMNOPRSTW{s]" );

	    if ( clan_table[ch->clan].portal_room != vnum )
		flags[3] = '.';
	    if ( !IS_SET( pRoom->room_flags, ROOM_NO_RECALL ) )
		flags[4] = '.';
	    if ( !IS_SET( pRoom->room_flags, ROOM_DARK ) )
		flags[5] = '.';
	    if ( pRoom->heal_rate == 100 )
		flags[6] = '.';
	    if ( !IS_SET( pRoom->room_flags, ROOM_ICY ) )
		flags[7] = '.';
	    if ( !IS_SET( pRoom->room_flags, ROOM_NO_MOB ) )
		flags[8] = '.';
	    if ( !IS_SET( pRoom->room_flags, ROOM_INDOORS ) )
		flags[9] = '.';
	    if ( !IS_SET( pRoom->room_flags, ROOM_CLAN_PORTAL ) )
		flags[10] = '.';
	    if ( !IS_SET( pRoom->room_flags, ROOM_PRIVATE ) )
		flags[11] = '.';
	    if ( clan_table[ch->clan].hall != vnum )
		flags[12] = '.';
	    if ( !IS_SET( pRoom->room_flags, ROOM_SAFE ) )
		flags[13] = '.';
	    if ( clan_table[ch->clan].two_way_link != vnum )
		flags[14] = '.';
	    if ( !IS_SET( pRoom->room_flags, ROOM_NOWHERE ) )
		flags[15] = '.';	

	    sprintf( buf, "{t%2d{s) {s%s {t%s\n\r", count, flags, pRoom->name );
	    add_buf( final, buf );
	}
    }

    add_buf( final, "{x" );
    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

CLAN_COMMAND( list_spells )
{
    BUFFER *final = new_buf( );
    COST_DATA *price;
    char buf[MAX_STRING_LENGTH];
    bool fPotion = FALSE, fScroll = FALSE, fPill = FALSE, fWand = FALSE, fStaff = FALSE;
    sh_int pos;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: list-spells [type].\n\r\n\rValid types are: potions, scrolls, pills, wands, staves.\n\r", ch );
	return FALSE;
    }

    else if ( !str_prefix( argument, "potions" ) )
	fPotion = TRUE;

    else if ( !str_prefix( argument, "scrolls" ) )
	fScroll = TRUE;

    else if ( !str_prefix( argument, "pills" ) )
	fPill = TRUE;

    else if ( !str_prefix( argument, "wands" ) )
	fWand = TRUE;

    else if ( !str_prefix( argument, "staves" ) || !str_prefix( argument, "staffs" ) )
	fStaff = TRUE;

    else
    {
	list_spells( ch, "", type );
	return FALSE;
    }

    add_buf( final, "{qName                      {s[{qCubics{s] [{qAquest{s] [{qIquest{s] [{qLevel{s] [{qMax{s]\n\r"
		    "{s------------------------------------------------------------------\n\r" );

    for ( pos = 0; skill_table[pos].name[0] != '\0'; pos++ )
    {
	if ( fPotion )		price = skill_table[pos].cost_potion;
	else if ( fScroll )	price = skill_table[pos].cost_scroll;
	else if ( fPill )	price = skill_table[pos].cost_pill;
	else if ( fWand )	price = skill_table[pos].cost_wand;
	else if ( fStaff )	price = skill_table[pos].cost_staff;
	else			return FALSE;

	if ( price == NULL )
	    continue;

	sprintf( buf, "{q%-25s {s[{q%6d{s] [{q%6d{s] [{q%6d{s] [{q %3d {s] [{q%3d{s]\n\r",
	    skill_table[pos].name, price->cubic, price->aquest,
	    price->iquest, price->level, price->max );
	add_buf( final, buf );
    }

    add_buf( final, "{x" );
    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

CLAN_COMMAND( make_clan )
{
    char arg[MAX_INPUT_LENGTH];
    bool fPKClan;
    sh_int pos;

    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "PK" ) )
	fPKClan = TRUE;

    else if ( !str_cmp( arg, "NON-PK" ) )
	fPKClan = FALSE;

    else
    {
	send_to_char( "Syntax: new-clan PK <name>, or new-clan NON-PK <name>.\n\r", ch );
	return FALSE;
    }

    if ( !IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	send_to_char( "You must be in a safe room to create a clan.\n\r", ch );
	return FALSE;
    }

    if ( is_clan( ch ) && !clan_table[ch->clan].independent )
    {
	send_to_char( "You are already in another clan.\n\r", ch );
	return FALSE;
    }

    if ( ( pos = price_lookup( ch, "new-clan" ) ) == -1 )
	return FALSE;

    if ( ch->pcdata->questpoints < price_table[pos].cost_aquest )
    {
	send_to_char( "You do not have enough quest points to make a new clan.\n\r", ch );
	return FALSE;
    }

    if ( ch->pcdata->deviant_points[0] < price_table[pos].cost_iquest )
    {
	send_to_char( "You do not have enough iquest points to make a new clan.\n\r", ch );
	return FALSE;
    }

    if ( count_cubics( ch ) < price_table[pos].cost_cubic )
    {
	send_to_char( "You do not have enough cubics to make a new clan.\n\r", ch );
	return FALSE;
    }

    if ( !clan_edit_create( ch, argument ) )
	return FALSE;

    extract_cubics( ch, price_table[pos].cost_cubic );
    ch->pcdata->questpoints -= price_table[pos].cost_aquest;
    ch->pcdata->deviant_points[0] -= price_table[pos].cost_iquest;

    send_to_char( CLEAR_SCREEN, ch );
    send_to_char( "{RCongratulations, you are now creating a new clan!\n\r"
		  "Type 'done' when you are finished.\n\r"
		  "Type 'commands' for a list of commands.\n\r"
		  "Type 'show' to see the current values.\n\r\n\r"
		  "You have 12 hours to edit your clan data and first room.\n\r"
		  "You have 12 hours to link your two way exit.\n\r"
		  "After this time, you must purchase changes.{x\n\r", ch );

    if ( is_clan( ch ) )
	update_clanlist( ch, FALSE );

    ch->clan = maxClan-1;
    ch->pcdata->clan_rank = MAX_CRNK-1;

    clan_table[ch->clan].pkill = fPKClan;

    update_clanlist( ch, TRUE );
    check_roster( ch, FALSE );
    make_clan_area( ch );

    clan_log( ch, ch->clan, price_table[pos].name, price_table[pos].cost_cubic,
	price_table[pos].cost_aquest, price_table[pos].cost_iquest );
    return TRUE;
}

CLAN_COMMAND( make_help )
{
    HELP_DATA *pHelp;
    extern HELP_DATA *help_first, *help_last;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: new-help [keyword(s)]\n\r", ch );
	return FALSE;
    }

    pHelp = new_help( );

    free_string( pHelp->keyword );
    pHelp->keyword	= str_dup( argument );
    pHelp->clan		= ch->clan;

    if ( help_first == NULL )
	help_first = pHelp;

    if ( help_last != NULL )
	help_last->next = pHelp;

    help_last		= pHelp;
    pHelp->next		= NULL;
    ch->desc->pEdit	= (void *)pHelp;
    ch->desc->editor	= ED_HELP;

    clan_table[ch->clan].edit_help += 60;

    send_to_char( "\n\rCreated new help file and added one hour to helpfile edit time.\n\r", ch );

    return TRUE;
}

CLAN_COMMAND( make_guard )
{
    CHAR_DATA *wch;
    MOB_INDEX_DATA *pMob;
    RESET_DATA *pReset;
    char buf[100];
    sh_int pos = 0;

    for ( pReset = ch->in_room->reset_first; pReset != NULL; pReset = pReset->next )
    {
	if ( pReset->command == 'M'
	&&   ( pMob = get_mob_index( pReset->arg1 ) ) != NULL
	&&   pMob->pShop == NULL
	&&   ++pos >= 2 )
	{
	    send_to_char( "Max guard resets per room is 2.\n\r", ch );
	    return FALSE;
	}
    }

    if ( ( pMob = new_clan_mobile( ch ) ) == NULL )
	return FALSE;

    pMob->level		= 20;
    pMob->act		= ACT_STAY_AREA|ACT_AGGRESSIVE;
    pMob->hit[0]	= 2000;
    pMob->hit[1]	= 2000;
    pMob->mana[0]	= 2000;
    pMob->mana[1]	= 2000;
    pMob->hitroll	= 25;
    pMob->damage[0]	= 5;
    pMob->damage[1]	= 5;
    pMob->damage[2]	= 10;

    for ( pos = 0; pos < 4; pos++ )
	pMob->ac[pos] = -400;

    sprintf( buf, "guard %s", clan_table[ch->clan].name );
    pMob->player_name	= str_dup( buf );

    sprintf( buf, "the guardian of %s", clan_table[ch->clan].color );
    pMob->short_descr	= str_dup( buf );

    sprintf( buf, "Here stands the guardian of %s.\n\r",
	clan_table[ch->clan].color );
    pMob->long_descr	= str_dup( buf );

    ch->desc->pEdit	= (void *)pMob;
    ch->desc->editor	= ED_MOBILE;
    medit_show( ch, "" );

    if ( ( wch = create_mobile( pMob ) ) != NULL )
	char_to_room( wch, ch->in_room );

    send_to_char( "\n\rCreated new guard and added one hour to edit mobile time.\n\r", ch );

    return TRUE;
}

CLAN_COMMAND( make_object )
{
    CHAR_DATA *wch;
    MOB_INDEX_DATA *pMob = NULL;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObj;
    RESET_DATA *pReset;
    ROOM_INDEX_DATA *pRoom = ch->in_room;
    char buf[MAX_STRING_LENGTH], temp[MAX_INPUT_LENGTH];
    bool found = FALSE;
    int val0 = 0, val1 = 0, val2 = 0, val3 = 0, val4 = 0;
    int door = 0, iHash, reset = 0, wear = 0, vnum, key2_reset = 1;

    switch( type )
    {
	default:
	    send_to_char( "BUG, make_object called with invalid type.\n\r", ch );
	    return FALSE;

	case ITEM_KEY:
	    switch( UPPER( *argument ) )
	    {
		default:
		    send_to_char( "Syntax: new-key <door direction>.\n\r", ch );
		    return FALSE;

		case 'N':	door = DIR_NORTH;	break;
		case 'S':	door = DIR_SOUTH;	break;
		case 'W':	door = DIR_WEST;	break;
		case 'E':	door = DIR_EAST;	break;
		case 'U':	door = DIR_UP;		break;
		case 'D':	door = DIR_DOWN;	break;
	    }

	    if ( ch->in_room->exit[door] == NULL )
	    {
		send_to_char( "There is no exit in that direction.\n\r", ch );
		return FALSE;
	    }

	    if ( !IS_SET( ch->in_room->exit[door]->rs_flags, EX_ISDOOR )
	    ||   !IS_SET( ch->in_room->exit[door]->rs_flags, EX_CLOSED ) )
	    {
		send_to_char( "To lock a door, it must have door and closed flags.\n\r", ch );
		return FALSE;
	    }

	    if ( IS_SET( ch->in_room->exit[door]->rs_flags, EX_LOCKED ) )
	    {
		send_to_char( "That exit is already locked.\n\r", ch );
		return FALSE;
	    }

	    if ( ch->in_room->exit[door]->u1.to_room == NULL
	    ||   ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]] == NULL
	    ||   ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]]->u1.to_room != ch->in_room )
	    {
		send_to_char( "Error: Can not find room on other side of door.\n\r", ch );
		return FALSE;
	    }

	    for ( pReset = ch->in_room->exit[door]->u1.to_room->reset_first; pReset; pReset = pReset->next )
	    {
		key2_reset++;

		if ( pReset->command == 'M' )
		{
		    if ( ( pMob = get_mob_index( pReset->arg1 ) ) != NULL )
		    {
			if ( pMob->pShop == NULL && IS_SET( pMob->act, ACT_SENTINEL ) )
			{
			    found = TRUE;
			    break;
			}
		    }
		}
	    }

	    if ( !found )
	    {
		send_to_char( "Can not find guard in next room with sentinel flag to give key to.\n\r", ch );
		return FALSE;
	    }

	    wear = ITEM_TAKE|ITEM_HOLD;
	    break;

	case ITEM_ARMOR:
	case ITEM_CONTAINER:
	case ITEM_JEWELRY:
	    if ( argument[0] == '\0'
	    ||   ( wear = flag_lookup2( argument, wear_flags ) ) == NO_FLAG )
	    {
		send_to_char( "Syntax: buy <wear location>.\n\r\n\r", ch );
		show_flag_cmds( ch, wear_flags );
		return FALSE;
	    }

	    if ( wear == ITEM_TAKE )
	    {
		send_to_char( "Take flag is auto set, pick the wear slot.\n\r", ch );
		return FALSE;
	    }

	    if ( wear == ITEM_WIELD )
	    {
		send_to_char( "Wield is for weapons.\n\r", ch );
		return FALSE;
	    }

	    SET_BIT( wear, ITEM_TAKE );
	    break;

	case ITEM_PORTAL:
	    if ( ( pRoom = get_room_index( clan_table[ch->clan].portal_room ) ) == NULL )
	    {
		send_to_char( "Your clan has no portal room set up.\n\r", ch );
		return FALSE;
	    }

	    if ( ch->in_room->area->clan != 0 )
	    {
		send_to_char( "You can not make portals to clan areas.\n\r", ch );
		return FALSE;
	    }

	    for ( vnum = pRoom->area->min_vnum; vnum <= pRoom->area->max_vnum; vnum++ )
	    {
		if ( ( pObj = get_obj_index( vnum ) ) != NULL
		&&   pObj->value[3] == ch->in_room->vnum )
		{
		    send_to_char( "Your clan already has a portal to this room.\n\r", ch );
		    return FALSE;
		}
	    }

	    for ( obj = pRoom->contents; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->pIndexData->area->clan == ch->clan
		&&   obj->item_type == ITEM_PORTAL
		&&   ++reset >= 3 )
		{
		    send_to_char( "Your portal room has reached the max number of portals.\n\rTo buy more portals you must designate a different room as a portal room.\n\r", ch );
		    return FALSE;
		}
	    }
	    break;

	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
	case ITEM_WAND:
	case ITEM_STAFF:
	    wear = ITEM_TAKE|ITEM_HOLD;
	    break;

	case ITEM_LIGHT:
	    wear = ITEM_TAKE;
	    break;

	case ITEM_WEAPON:
	    argument = one_argument( argument, buf );
	    if ( buf[0] == '\0' || argument[0] == '\0' )
	    {
		send_to_char( "Syntax: new-weapon <type> <dam-noun>\n\r", ch );
		return FALSE;
	    }

	    if ( ( val0 = flag_value( weapon_class, buf ) ) == NO_FLAG )
	    {
		send_to_char( "Invalid weapon type.\n\r\n\r", ch );
		show_help( ch, "wclass" );
		return FALSE;
	    }

	    if ( ( val3 = attack_lookup( argument ) ) == 0 )
	    {
		send_to_char( "Invalid damage noun.\n\r\n\r", ch );
		show_help( ch, "weapon" );
		return FALSE;
	    }
	    wear = ITEM_TAKE|ITEM_WIELD;
	    break;

	case ITEM_PIT:
	    if ( clan_table[ch->clan].pit != OBJ_VNUM_PIT )
	    {
		send_to_char( "Your clan already has a donation pit.\n\r", ch );
		return FALSE;
	    }
	    break;

	case ITEM_FURNITURE:
	    break;

	case ITEM_BOAT:
	case ITEM_FOOD:
	    wear = ITEM_TAKE;
	    break;

	case ITEM_FOUNTAIN:
	    if ( argument[0] == '\0'
	    ||   ( val2 = liq_lookup( argument ) ) == -1 )
	    {
		send_to_char( "Syntax: new-fountain <liquid>.\n\r\n\r", ch );
		show_help( ch, "liquid" );
		return FALSE;
	    }
	    break;

	case ITEM_DRINK_CON:
	    if ( argument[0] == '\0'
	    ||   ( val2 = liq_lookup( argument ) ) == -1 )
	    {
		send_to_char( "Syntax: new-drink-container <liquid>.\n\r\n\r", ch );
		show_help( ch, "liquid" );
		return FALSE;
	    }
	    wear = ITEM_TAKE;
	    break;
    }

    if ( ( reset = get_reset_number( ch, type ) ) == -1 )
	return FALSE;

    for ( vnum = pRoom->area->min_vnum; ; vnum++ )
    {
	if ( vnum > pRoom->area->max_vnum )
	{
	    send_to_char( "Your clan has ran out of free vnums for new objects.\n\r", ch );
	    return FALSE;
	}

	if ( get_obj_index( vnum ) == NULL )
	    break;
    }

    pObj		= new_obj_index( );
    pObj->vnum		= vnum;
    pObj->level		= 21;
    pObj->area		= pRoom->area;
    pObj->extra_flags	= ITEM_SELL_EXTRACT;
    pObj->item_type	= type;
    pObj->wear_flags	= wear;
    pObj->value[0]	= val0;
    pObj->value[1]	= val1;
    pObj->value[2]	= val2;
    pObj->value[3]	= val3;
    pObj->value[4]	= val4;

    sprintf( temp, "%s", flag_string( type_flags, type ) );

    sprintf( buf, "%s new %s", temp, clan_table[ch->clan].name );
    pObj->name		= str_dup( buf );

    sprintf( buf, "a new %s for %s", temp, clan_table[ch->clan].color );
    pObj->short_descr	= str_dup( buf );

    sprintf( buf, "A new %s for %s is here.", temp, clan_table[ch->clan].color );
    pObj->description	= str_dup( buf );

    iHash		= vnum % MAX_KEY_HASH;
    pObj->next		= obj_index_hash[iHash];
    obj_index_hash[iHash] = pObj;

    set_obj_stats( pObj );

    switch( type )
    {
	default:
	    break;

	case ITEM_PIT:
	    clan_table[ch->clan].pit = vnum;
	    break;

	case ITEM_FURNITURE:
	    pObj->value[0] = 2;
	    pObj->value[1] = 10000;
	    pObj->value[2] = STAND_AT|STAND_ON|STAND_IN|SIT_AT|SIT_ON|SIT_IN|REST_AT|REST_ON|REST_IN|SLEEP_AT|SLEEP_ON|SLEEP_IN;
	    break;

	case ITEM_CONTAINER:
	    pObj->value[4] = 30;
	    break;

	case ITEM_FOUNTAIN:
	    pObj->value[0] = -1;
	    pObj->value[1] = -1;
	    break;

	case ITEM_KEY:
	    SET_BIT( ch->in_room->exit[door]->rs_flags, EX_LOCKED );
	    ch->in_room->exit[door]->exit_info = ch->in_room->exit[door]->rs_flags;
	    ch->in_room->exit[door]->key = vnum;
	    SET_BIT( ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]]->rs_flags, EX_LOCKED );
	    ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]]->exit_info =
	    ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]]->rs_flags;
	    ch->in_room->exit[door]->u1.to_room->exit[rev_dir[door]]->key = vnum;

	    pReset		= new_reset_data( );
	    pReset->command	= 'G';
	    pReset->percent	= 100;
	    pReset->arg1	= vnum;
	    pReset->arg2	= WEAR_NONE;
	    pReset->arg3	= 0;
	    pReset->arg4	= 0;

	    add_reset( ch->in_room->exit[door]->u1.to_room, pReset, key2_reset );

	    for ( wch = ch->in_room->exit[door]->u1.to_room->people; wch != NULL; wch = wch->next_in_room )
	    {
		if ( wch->pIndexData && wch->pIndexData == pMob )
		{
		    if ( ( obj = create_object( pObj ) ) != NULL )
		    {
			obj_to_char( obj, wch );
			break;
		    }
		}
	    }

	    break;
    }

    pReset		= new_reset_data( );
    pReset->percent	= 100;
    pReset->arg1	= vnum;
    pReset->arg4	= 0;

    if ( IS_SET( pObj->wear_flags, ITEM_TAKE ) )
    {
	pReset->command	= 'G';
	pReset->arg2	= WEAR_NONE;
	pReset->arg3	= 0;

	for ( wch = pRoom->people; wch != NULL; wch = wch->next_in_room )
	{
	    if ( ( type != ITEM_KEY && wch->pIndexData && wch->pIndexData->pShop )
	    ||   ( type == ITEM_KEY && wch->pIndexData && !wch->pIndexData->pShop ) )
	    {
		if ( ( obj = create_object( pObj ) ) != NULL )
		{
		    if ( type != ITEM_KEY )
			SET_BIT( obj->extra_flags, ITEM_INVENTORY );
		    obj_to_char( obj, wch );
		    break;
		}
	    }
	}
    } else {
	pReset->command	= 'O';
	pReset->arg2	= 0;
	pReset->arg3	= pRoom->vnum;

	if ( ( obj = create_object( pObj ) ) != NULL )
	    obj_to_room( obj, pRoom );
    }

    add_reset( pRoom, pReset, reset );

    clan_table[ch->clan].edit_obj += 60;

    ch->desc->pEdit	= (void *)pObj;
    ch->desc->editor	= ED_OBJECT;
    oedit_show( ch, "" );

    sprintf( buf, "\n\rCreated new %s and added one hour to object edit time.\n\r", temp );
    send_to_char( buf, ch );

    return TRUE;
}

CLAN_COMMAND( make_room )
{
    ROOM_INDEX_DATA *pRoom;
    char buf[100];
    sh_int door;
    int iHash, vnum;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Which direction should your new room be?\n\r", ch );
	return FALSE;
    }

    else if ( !str_prefix( argument, "north" ) )	door = DIR_NORTH;
    else if ( !str_prefix( argument, "south" ) )	door = DIR_SOUTH;
    else if ( !str_prefix( argument, "west" ) )		door = DIR_WEST;
    else if ( !str_prefix( argument, "east" ) )		door = DIR_EAST;
    else if ( !str_prefix( argument, "down" ) )		door = DIR_DOWN;
    else if ( !str_prefix( argument, "up" ) )		door = DIR_UP;
    else
    {
	send_to_char( "That is not a valid direction.\n\r", ch );
	return FALSE;
    }

    if ( ch->in_room->exit[door] != NULL )
    {
	send_to_char( "There is already another room in that direction.\n\r", ch );
	return FALSE;
    }

    for ( vnum = ch->in_room->area->min_vnum; ; vnum++ )
    {
	if ( vnum > ch->in_room->area->max_vnum )
	{
	    send_to_char( "Your clan has ran out of free vnums for new rooms.\n\r", ch );
	    return FALSE;
	}

	if ( get_room_index( vnum ) == NULL )
	    break;
    }

    sprintf( buf, "%s New Clan Room", clan_table[ch->clan].color );

    pRoom		= new_room_index( );
    pRoom->vnum		= vnum;
    pRoom->area		= ch->in_room->area;
    pRoom->name		= str_dup( buf );
    iHash		= vnum % MAX_KEY_HASH;
    pRoom->next		= room_index_hash[iHash];
    room_index_hash[iHash] = pRoom;

    ch->in_room->exit[door]		= new_exit( );
    ch->in_room->exit[door]->u1.to_room	= pRoom;
    ch->in_room->exit[door]->orig_door	= door;

    pRoom->exit[rev_dir[door]]			= new_exit( );
    pRoom->exit[rev_dir[door]]->u1.to_room	= ch->in_room;
    pRoom->exit[rev_dir[door]]->orig_door	= rev_dir[door];

    char_from_room( ch );
    char_to_room( ch, pRoom );

    ch->desc->editor = ED_ROOM;
    ch->desc->pEdit = (void *)ch->in_room;

    redit_show( ch, "" );

    clan_table[ch->clan].edit_room += 60;

    send_to_char( "\n\rCreated new room and added one hour to edit room time.\n\r", ch );

    return TRUE;
}

CLAN_COMMAND( make_shop )
{
    CHAR_DATA *wch;
    MOB_INDEX_DATA *pMob;
    char buf[100];
    sh_int pos;

    for ( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room )
    {
	if ( wch->pIndexData && wch->pIndexData->pShop )
	{
	    send_to_char( "This room already has a shopkeeper.\n\r", ch );
	    return FALSE;
	}
    }

    if ( ( pMob = new_clan_mobile( ch ) ) == NULL )
	return FALSE;

    pMob->level		= LEVEL_HERO;
    pMob->pShop		= new_shop( );
    pMob->pShop->keeper	= pMob->vnum;
    pMob->act		= ACT_SENTINEL;
    pMob->hit[0]	= 5000;
    pMob->hit[1]	= 5000;
    pMob->mana[0]	= 5000;
    pMob->mana[1]	= 5000;

    for ( pos = 0; pos < DAM_MAX; pos++ )
	pMob->damage_mod[pos] = 0;

    if ( !shop_first )
	shop_first = pMob->pShop;

    if ( shop_last )
	shop_last->next = pMob->pShop;

    shop_last = pMob->pShop;

    sprintf( buf, "shopkeeper %s", clan_table[ch->clan].name );
    pMob->player_name	= str_dup( buf );

    sprintf( buf, "the shopkeeper of %s", clan_table[ch->clan].color );
    pMob->short_descr	= str_dup( buf );

    sprintf( buf, "Here stands the shopkeeper for %s.\n\r",
	clan_table[ch->clan].color );
    pMob->long_descr	= str_dup( buf );

    ch->desc->pEdit	= (void *)pMob;
    ch->desc->editor	= ED_MOBILE;
    medit_show( ch, "" );

    if ( ( wch = create_mobile( pMob ) ) != NULL )
	char_to_room( wch, ch->in_room );

    send_to_char( "\n\rCreated new shopkeeper and added one hour to edit mobile time.\n\r", ch );

    return TRUE;
}

CLAN_COMMAND( make_two_way )
{
    ROOM_INDEX_DATA *pRoom, *temp;
    sh_int door, pos;
    int vnum;

    if ( ( pRoom = get_room_index( clan_table[ch->clan].two_way_link ) ) == NULL )
    {
	send_to_char( "Can not find room to link two, use set-two-way in clan hall.\n\r", ch );
	return FALSE;
    }

    if ( pRoom->area->clan != ch->clan )
    {
	send_to_char( "Something broke.\n\r", ch );
	return FALSE;
    }

    switch ( UPPER( *argument ) )
    {
	default:
	    send_to_char( "Syntax: make-two-way <direction>.\n\r", ch );
	    return FALSE;

	case 'N':	door = DIR_NORTH;	break;
	case 'S':	door = DIR_SOUTH;	break;
	case 'W':	door = DIR_WEST;	break;
	case 'E':	door = DIR_EAST;	break;
	case 'U':	door = DIR_UP;		break;
	case 'D':	door = DIR_DOWN;	break;
    }

    if ( ch->in_room->area->clan == ch->clan )
    {
	send_to_char( "Two way exits are for outside your clan hall.\n\r", ch );
	return FALSE;
    }

    if ( ch->in_room->exit[door] != NULL )
    {
	send_to_char( "This room already has a door in that direction.\n\r", ch );
	return FALSE;
    }

    if ( pRoom->exit[rev_dir[door]] != NULL )
    {
	send_to_char( "Your currently designated clan room already has an exit facing this way.\n\r", ch );
	return FALSE;
    }

    for ( vnum = pRoom->area->min_vnum; vnum <= pRoom->area->max_vnum; vnum++ )
    {
	if ( ( temp = get_room_index( vnum ) ) != NULL )
	{
	    for ( pos = 0; pos < MAX_DIR; pos++ )
	    {
		if ( temp->exit[pos]
		&&   temp->exit[pos]->u1.to_room
		&&   temp->exit[pos]->u1.to_room->area != pRoom->area
		&&   temp->exit[pos]->u1.to_room->exit[rev_dir[pos]]
		&&   temp->exit[pos]->u1.to_room->exit[rev_dir[pos]]->u1.to_room
		&&   temp->exit[pos]->u1.to_room->exit[rev_dir[pos]]->u1.to_room == temp )
		{
		    ROOM_INDEX_DATA *to_room = temp->exit[pos]->u1.to_room;

		    free_exit( temp->exit[pos] );
		    temp->exit[pos] = NULL;

		    free_exit( to_room->exit[rev_dir[pos]] );
		    to_room->exit[rev_dir[pos]] = NULL;

		    SET_BIT( to_room->area->area_flags, AREA_CHANGED );
		}
	    }
	}
    }

    ch->in_room->exit[door]		= new_exit( );
    ch->in_room->exit[door]->u1.to_room	= pRoom;
    ch->in_room->exit[door]->orig_door	= door;

    pRoom->exit[rev_dir[door]]			= new_exit( );
    pRoom->exit[rev_dir[door]]->u1.to_room	= ch->in_room;
    pRoom->exit[rev_dir[door]]->orig_door	= rev_dir[door];

    SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
    SET_BIT( pRoom->area->area_flags, AREA_CHANGED );

    send_to_char( "Two way exit moved.\n\r", ch );

    return clan_table[ch->clan].two_way_time <= 0;
}

CLAN_COMMAND( resist_guard )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    sh_int pos, total = 0;

    argument = one_argument( argument, arg );

    if ( ( pMob = get_clan_mob( ch, arg ) ) == NULL )
	return FALSE;

    if ( pMob->pShop != NULL )
    {
	send_to_char( "You can't upgrade shopkeepers.\n\r", ch );
	return FALSE;
    }

    for ( pos = 0; pos < DAM_MAX; pos++ )
	total += pMob->damage_mod[pos];

    if ( total <= 1500 ) // 26 * 100 = 2600 total
    {
	send_to_char( "You already have the lowest possible damage percentages on this mobile.\n\r", ch );
	return FALSE;
    }

    if ( argument[0] == '\0' || ( pos = dam_type_lookup( argument ) ) == -1 )
    {
	send_to_char( "That damage type does not exist.\n\r", ch );
	return FALSE;
    }

    if ( pMob->damage_mod[pos] <= 0 )
    {
	send_to_char( "Damage mod already at or below 0.\n\r", ch );
	return FALSE;
    }

    pMob->damage_mod[pos] -= 10;

    reset_clan_mobs( pMob );

    return TRUE;
}

CLAN_COMMAND( resist_object )
{
    AFFECT_DATA *paf;
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_INPUT_LENGTH];
    sh_int pos;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "resist-object <object> <type>.\n\r", ch );
	return FALSE;
    }

    if ( ( pObj = get_clan_obj( ch, arg ) ) == NULL )
	return FALSE;

    switch( pObj->item_type )
    {
	default:
	    send_to_char( "You can't add resistance modifiers to that object type.\n\r", ch );
	    return FALSE;

	case ITEM_ARMOR:
	case ITEM_WEAPON:
	case ITEM_LIGHT:
	case ITEM_JEWELRY:
	case ITEM_CONTAINER:
	    break;
    }

    if ( ( pos = dam_type_lookup( argument ) ) == -1 )
    {
	send_to_char( "That damage type does not exist.\n\r", ch );
	return FALSE;
    }

    for ( paf = pObj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->where == TO_DAM_MODS )
	{
	    send_to_char( "Only one resistance per object.\n\r", ch );
	    return FALSE;
	}
    }

    paf			= new_affect( );
    paf->location	= pos;
    paf->modifier	= -5;
    paf->where		= TO_DAM_MODS;
    paf->type		= -1;
    paf->dur_type	= DUR_TICKS;
    paf->duration	= -1;
    paf->bitvector	= 0;
    paf->level		= pObj->level;
    paf->next		= pObj->affected;
    pObj->affected	= paf;

    reset_clan_objects( pObj );

    send_to_char( "Resistance applied.\n\r", ch );

    return TRUE;
}

CLAN_COMMAND( set_portal )
{
    if ( !IS_SET( ch->in_room->room_flags, ROOM_CLAN_PORTAL ) )
    {
	send_to_char( "You must flag this room as a portal room to designate portals here.\n\r", ch );
	return FALSE;
    }

    clan_table[ch->clan].portal_room = ch->in_room->vnum;
    send_to_char( "New portal assigned to this room.\n\r", ch );
    return TRUE;
}

CLAN_COMMAND( set_recall )
{
    if ( ch->in_room->vnum == clan_table[ch->clan].hall )
    {
	send_to_char( "This room is already set as your recall room.\n\r", ch );
	return FALSE;
    }

    clan_table[ch->clan].hall = ch->in_room->vnum;
    send_to_char( "Recall room set.\n\r", ch );
    return TRUE;
}

CLAN_COMMAND( set_two_way )
{
    clan_table[ch->clan].two_way_link = ch->in_room->vnum;
    send_to_char( "Two way link room set, now to to another room to link back here.\n\r", ch );
    return TRUE;
}

CLAN_COMMAND( show_prices )
{
    BUFFER *final = new_buf( );
    char buf[MAX_STRING_LENGTH];
    sh_int count, pos;

    add_buf( final, "{qOption        Mult {s[ {qCubics{s] [{q   Aquest{s] [ {qIquest{s] {tNotes\n\r" );
    add_buf( final, "{s------------------------------------------------------------------------------------\n\r" );

    for ( pos = 0; price_table[pos].name != NULL; pos++ )
    {
	if ( price_table[pos].type == DO_NOT_CHARGE
	&&   price_table[pos].cmd != make_clan )
	{
	    sprintf( buf, "{q%-16.16s  %s{s[{t---{s/{q---{s] [{t----{s/{q----{s] [{t---{s/{q---{s]%s{t%-32s\n\r",
		price_table[pos].name,
		IS_SET( price_table[pos].flags, FLAG_LEADER ) ? "{R*" : " ",
		IS_SET( price_table[pos].flags, FLAG_HALL ) ? "{R*" : " ",
		price_table[pos].notes );
	}
	else
	{
	    count = price_table[pos].mult( ch, price_table[pos].type );

	    sprintf( buf, "{q%-16.16s{t%2d%s{s[{t%3d{s/{q%3d{s] [{t%4d{s/{q%4d{s] [{t%3d{s/{q%3d{s]%s{t%-32s\n\r",
		price_table[pos].name,
		count,
		IS_SET( price_table[pos].flags, FLAG_LEADER ) ? "{R*" : " ",
		price_table[pos].cost_cubic,
		count * price_table[pos].cost_cubic,
		price_table[pos].cost_aquest,
		count * price_table[pos].cost_aquest,
		price_table[pos].cost_iquest,
		count * price_table[pos].cost_iquest,
		IS_SET( price_table[pos].flags, FLAG_HALL ) ? "{R*" : " ",
		price_table[pos].notes );
	}

	add_buf( final, buf );
    }

    add_buf( final, "{s------------------------------------------------------------------------------------\n\r" );
    add_buf( final, "     {tLeader Only->{R*{tBase{s/{qTotl  {tBase{s/{qTotal {tBase{s/{qTotl{R*{t<-In Hall Only{x\n\r" );

    page_to_char( final->string, ch );
    free_buf( final );
    return FALSE;
}

CLAN_COMMAND( stat_clan )
{
    void *old = ch->desc->pEdit;
    int edit = ch->desc->editor;

    ch->desc->pEdit	= (void *)(int)ch->clan;
    ch->desc->editor	= ED_CLAN;

    clan_edit_show( ch, "" );

    ch->desc->pEdit	= old;
    ch->desc->editor	= edit;

    return FALSE;
}

CLAN_COMMAND( stat_help )
{
    HELP_DATA *pHelp;
    void *old = ch->desc->pEdit;
    int edit = ch->desc->editor;

    if ( ( pHelp = get_clan_help( ch, argument ) ) == NULL )
	return FALSE;

    ch->desc->pEdit	= (void *)pHelp;
    ch->desc->editor	= ED_HELP;

    hedit_show( ch, "" );

    ch->desc->pEdit	= old;
    ch->desc->editor	= edit;

    return FALSE;
}

CLAN_COMMAND( stat_mob )
{
    MOB_INDEX_DATA *pMob;
    void *old = ch->desc->pEdit;
    int edit = ch->desc->editor;

    if ( ( pMob = get_clan_mob( ch, argument ) ) == NULL )
	return FALSE;

    ch->desc->pEdit	= (void *)pMob;
    ch->desc->editor	= ED_MOBILE;

    medit_show( ch, "" );

    ch->desc->pEdit	= old;
    ch->desc->editor	= edit;

    return FALSE;
}

CLAN_COMMAND( stat_object )
{
    OBJ_INDEX_DATA *pObj;
    void *old = ch->desc->pEdit;
    int edit = ch->desc->editor;

    if ( ( pObj = get_clan_obj( ch, argument ) ) == NULL )
	return FALSE;

    ch->desc->pEdit	= (void *)pObj;
    ch->desc->editor	= ED_OBJECT;

    oedit_show( ch, "" );

    ch->desc->pEdit	= old;
    ch->desc->editor	= edit;

    return FALSE;
}

CLAN_COMMAND( stat_room )
{
    void *old = ch->desc->pEdit;
    int edit = ch->desc->editor;

    if ( ch->in_room->area->clan != ch->clan )
    {
	send_to_char( "You may only stat rooms from your own clan.\n\r", ch );
	return FALSE;
    }

    ch->desc->pEdit	= (void *)ch->in_room;
    ch->desc->editor	= ED_ROOM;

    redit_show( ch, "" );

    ch->desc->pEdit	= old;
    ch->desc->editor	= edit;
    return FALSE;
}

CLAN_COMMAND( time_clan )
{
    if ( clan_table[ch->clan].edit_clan > 0 )
    {
	send_to_char( "You still have time left to edit your clan!\n\r", ch );
	return FALSE;
    }

    clan_table[ch->clan].edit_clan = 120;
    send_to_char( "Clan data time extended.\n\r", ch );
    return TRUE;
}

CLAN_COMMAND( time_helps )
{
    if ( clan_table[ch->clan].edit_help > 0 )
    {
	send_to_char( "You still have time left to edit your help files!\n\r", ch );
	return FALSE;
    }

    clan_table[ch->clan].edit_help = 120;
    send_to_char( "Clan help file time extended.\n\r", ch );
    return TRUE;
}

CLAN_COMMAND( time_mobiles )
{
    if ( clan_table[ch->clan].edit_mob > 0 )
    {
	send_to_char( "You still have time left to edit your mobiles!\n\r", ch );
	return FALSE;
    }

    clan_table[ch->clan].edit_mob = 120;
    send_to_char( "Clan mobile time extended.\n\r", ch );
    return TRUE;
}

CLAN_COMMAND( time_objects )
{
    if ( clan_table[ch->clan].edit_obj > 0 )
    {
	send_to_char( "You still have time left to edit your objects!\n\r", ch );
	return FALSE;
    }

    clan_table[ch->clan].edit_obj = 120;
    send_to_char( "Clan object time extended.\n\r", ch );
    return TRUE;
}

CLAN_COMMAND( time_rooms )
{
    if ( clan_table[ch->clan].edit_room > 0 )
    {
	send_to_char( "You still have time left to edit your rooms!\n\r", ch );
	return FALSE;
    }

    clan_table[ch->clan].edit_room = 120;
    send_to_char( "Clan room time extended.\n\r", ch );
    return TRUE;
}

CLAN_COMMAND( up_max_member )
{
    clan_table[ch->clan].max_mem++;
    send_to_char( "Max members increased.\n\r", ch );
    return TRUE;
}

CLAN_COMMAND( upgrade_room )
{
    if ( ch->in_room->heal_rate >= 1000
    ||   ch->in_room->mana_rate >= 1000 )
    {
	send_to_char( "Healing rates are already at maximum for this room.\n\r", ch );
	return FALSE;
    }

    ch->in_room->heal_rate += 50;
    ch->in_room->mana_rate += 50;

    send_to_char( "Room healing upgraded.\n\r", ch );

    return TRUE;
}

sh_int has_two_way( sh_int clan )
{
    AREA_DATA *pArea = get_clan_area( clan );
    ROOM_INDEX_DATA *pRoom;
    int door, vnum;

    if ( pArea == NULL )
	return 0;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pRoom = get_room_index( vnum ) ) != NULL )
	{
	    for ( door = 0; door < MAX_DIR; door++ )
	    {
		if ( pRoom->exit[door]
		&&   pRoom->exit[door]->u1.to_room
		&&   pRoom->exit[door]->u1.to_room->area != pArea
		&&   pRoom->exit[door]->u1.to_room->exit[rev_dir[door]]
		&&   pRoom->exit[door]->u1.to_room->exit[rev_dir[door]]->u1.to_room
		&&   pRoom->exit[door]->u1.to_room->exit[rev_dir[door]]->u1.to_room == pRoom )
		    return 1;
	    }
	}
    }

    return 2;
}

void auto_two_way( sh_int clan )
{
    AREA_DATA *pArea = get_clan_area( clan );
    ROOM_INDEX_DATA *pRoom, *link;
    int door, rand, loop, vnum;

    if ( pArea == NULL )
	return;

    for ( loop = 0; loop < 50; loop++ )
    {
	rand = number_range( 3395, 3399 );

	if ( number_percent( ) < 30
	&&   ( link = get_room_index( rand ) ) != NULL )
	{
	    for ( door = 0; door < MAX_DIR; door++ )
	    {
		if ( link->exit[door] == NULL
		&&   number_percent( ) < 30 )
		{
		    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
		    {
			if ( number_percent( ) < 30
			&&   ( pRoom = get_room_index( vnum ) ) != NULL )
			{
			    if ( pRoom->exit[rev_dir[door]] == NULL )
			    {
				link->exit[door]		= new_exit( );
				link->exit[door]->u1.to_room	= pRoom;
				link->exit[door]->orig_door	= door;

				pRoom->exit[rev_dir[door]]		= new_exit( );
				pRoom->exit[rev_dir[door]]->u1.to_room	= link;
				pRoom->exit[rev_dir[door]]->orig_door	= rev_dir[door];
				SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
				SET_BIT( link->area->area_flags, AREA_CHANGED );
				clan_log( NULL, clan, "AUTO-TWO-WAY SET", 0, 0, 0 );
				return;
			    }
			}
		    }
		}
	    }
	}
    }
}

void do_clan_manage( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int cost = 0, pos;

    if ( ch->pcdata == NULL
    ||   ch->in_room == NULL
    ||   ch->desc == NULL )
    {
	send_to_char( "NPC or NULL room/desc detected.\n\r", ch );
	return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	if ( IS_IMMORTAL( ch ) )
	{
	    send_to_char( "Immortal Options:\n\r"
			  "  clan_manage list          (show prices)\n\r"
			  "  clan_manage check_two_way (check all clans for existence of two way exits)\n\r"
			  "  clan_manage auto_two_way  (automatically set missing two way exits for all clans)\n\r", ch );
	    return;
	}

	show_prices( ch, "", 0 );
	return;
    }

    if ( IS_IMMORTAL( ch ) )
    {
	if ( !str_prefix( arg, "check_two_way" ) )
	{
	    for ( pos = 0; clan_table[pos].name[0] != '\0'; pos++ )
	    {
		sh_int new = has_two_way( pos );

		sprintf( arg, "Clan %-20s: Two way exit %s{x\n\r",
		    clan_table[pos].name,
		    new == 0 ? "{YHALL MISSING" : new == 1 ? "{GFOUND" : "{RMISSING" );
		send_to_char( arg, ch );
	    }

	    return;
	}

	else if ( !str_cmp( arg, "auto_two_way" ) )
	{
	    for ( pos = 0; clan_table[pos].name[0] != '\0'; pos++ )
	    {
		if ( has_two_way( pos ) == 2 )
		    auto_two_way( pos );
	    }

	    send_to_char( "Automatically assigning all missing clan hall exits.\n\r", ch );
	    return;
	}
    }

    for ( pos = 0; price_table[pos].name != NULL; pos++ )
    {
	if ( price_table[pos].cmd != make_clan
	&&   !str_prefix( arg, price_table[pos].name ) )
	{
	    if ( IS_SET( price_table[pos].flags, FLAG_LEADER )
	    &&   ch->pcdata->clan_rank != MAX_CRNK-1 )
	    {
		send_to_char( "That command is reserved for leaders.\n\r", ch );
		return;
	    }

	    if ( IS_SET( price_table[pos].flags, FLAG_HALL )
	    &&   ch->in_room->area->clan != ch->clan )
	    {
		send_to_char( "That upgrade command will only work in your own clan hall.\n\r", ch );
		return;
	    }

	    if ( price_table[pos].type != DO_NOT_CHARGE )
	    {
		cost = price_table[pos].mult( ch, price_table[pos].type );
		if ( !can_afford( ch, pos, cost ) )
		    return;
	    }

	    if ( ( *price_table[pos].cmd ) ( ch, argument, price_table[pos].type ) )
	    {
		AREA_DATA *pArea = get_clan_area( ch->clan );

		if ( price_table[pos].type != DO_NOT_CHARGE )
		    charge_cost( ch, pos, cost );

		if ( pArea != NULL )
		    SET_BIT( pArea->area_flags, AREA_CHANGED );
	    }

	    return;
	}
    }

    do_clan_manage( ch, "" );
}

const struct clan_flag_type mob_flag_table[] =
{
//    "NAME",			CUBIC,	AQ,	IQ,	WHERE,		BIT			RESTRICT
    { "darkvision",		10,	5,	0,	TO_AFFECTS,	AFF_DARK_VISION,	0					},
    { "detect-evil",		5,	5,	0,	TO_AFFECTS,	AFF_DETECT_EVIL,	0					},
    { "detect-good",		5,	5,	0,	TO_AFFECTS,	AFF_DETECT_GOOD,	0					},
    { "detect-hidden",		20,	100,	5,	TO_AFFECTS,	AFF_DETECT_HIDDEN,	0					},
    { "detect-invis",		20,	100,	5,	TO_AFFECTS,	AFF_DETECT_INVIS,	0					},
    { "detect-magic",		5,	5,	0,	TO_AFFECTS,	AFF_DETECT_MAGIC,	0					},
    { "detect-neutral",		5,	5,	0,	TO_AFFECTS,	AFF_DETECT_NEUTRAL,	0					},
    { "fly",			15,	100,	5,	TO_AFFECTS,	AFF_FLYING,		0					},
    { "giant-strength",		20,	50,	2,	TO_AFFECTS,	AFF_GIANT_STRENGTH,	0					},
    { "haste",			50,	400,	15,	TO_AFFECTS,	AFF_HASTE,		0					},
    { "hide",			25,	250,	10,	TO_AFFECTS,	AFF_HIDE,		0					},
    { "infravision",		15,	75,	2,	TO_AFFECTS,	AFF_INFRARED,		0					},
    { "regeneration",		10,	5,	0,	TO_AFFECTS,	AFF_REGENERATION,	0					},
    { "sneak",			20,	10,	2,	TO_AFFECTS,	AFF_SNEAK,		0					},

    { "absorb-magic",		50,	450,	25,	TO_SHIELDS,	SHD_ABSORB,		0					},
    { "acidshield",		25,	100,	5,	TO_SHIELDS,	SHD_ACID,		0					},
    { "divine-aura",		25,	100,	5,	TO_SHIELDS,	SHD_DIVINE_AURA,	0					},
    { "divinity",		300,	1500,	30,	TO_SHIELDS,	SHD_DIVINITY,		SHD_SANCTUARY				},
    { "fireshield",		25,	100,	5,	TO_SHIELDS,	SHD_FIRE,		0					},
    { "iceshield",		25,	100,	5,	TO_SHIELDS,	SHD_ICE,		0					},
    { "invisibility",		25,	75,	5,	TO_SHIELDS,	SHD_INVISIBLE,		0					},
    { "mana-shield",		75,	400,	25,	TO_SHIELDS,	SHD_MANA,		0					},
    { "protection-evil",	75,	300,	20,	TO_SHIELDS,	SHD_PROTECT_EVIL,	SHD_PROTECT_GOOD|SHD_PROTECT_NEUTRAL	},
    { "protection-good",	75,	300,	20,	TO_SHIELDS,	SHD_PROTECT_GOOD,	SHD_PROTECT_EVIL|SHD_PROTECT_NEUTRAL	},
    { "protection-neutral",	75,	300,	20,	TO_SHIELDS,	SHD_PROTECT_NEUTRAL,	SHD_PROTECT_EVIL|SHD_PROTECT_GOOD	},
    { "rockshield",		25,	100,	5,	TO_SHIELDS,	SHD_ROCK,		0					},
    { "sanctuary",		200,	1000,	20,	TO_SHIELDS,	SHD_SANCTUARY,		SHD_DIVINITY				},
    { "shockshield",		25,	100,	5,	TO_SHIELDS,	SHD_SHOCK,		0					},
    { "shrapnelshield",		25,	100,	5,	TO_SHIELDS,	SHD_SHRAPNEL,		0					},
    { "thornshield",		25,	100,	5,	TO_SHIELDS,	SHD_THORN,		0					},
    { "vampiricshield",		25,	100,	5,	TO_SHIELDS,	SHD_VAMPIRIC,		0					},
    { "watershield",		25,	100,	5,	TO_SHIELDS,	SHD_WATER,		0					},

    { "area-attack",		200,	800,	20,	TO_ACT,		ACT_AREA_ATTACK,	0					},
    { "gain",			100,	500,	10,	TO_ACT,		ACT_GAIN,		0					},
    { "healer",			300,	1500,	25,	TO_ACT,		ACT_IS_HEALER,		ACT_IS_PRIEST|ACT_IS_SATAN		},
    { "is-priest",		250,	1500,	25,	TO_ACT,		ACT_IS_PRIEST,		ACT_IS_HEALER|ACT_IS_SATAN		},
    { "is-satan",		250,	1500,	25,	TO_ACT,		ACT_IS_SATAN,		ACT_IS_HEALER|ACT_IS_PRIEST		},
    { "practice",		200,	1000,	10,	TO_ACT,		ACT_PRACTICE,		0					},
    { "scavenger",		10,	15,	0,	TO_ACT,		ACT_SCAVENGER,		0					},
    { "sentinel",		15,	30,	0,	TO_ACT,		ACT_SENTINEL,		0					},
    { "train",			200,	1000,	10,	TO_ACT,		ACT_TRAIN,		0					},
    { "wimpy",			15,	25,	0,	TO_ACT,		ACT_WIMPY,		0					},

    { NULL,			0,	0,	0,	0,		0,			0					}
};

const struct clan_flag_type room_flag_table[] =
{
//    "NAME",		CUBIC,	AQ,	IQ,	WHERE,	BIT			RESTRICT
    { "dark",		10,	50,	0,	0,	ROOM_DARK,		0	},
    { "icy",		10,	50,	2,	0,	ROOM_ICY,		0	},
    { "indoors",	25,	100,	5,	0,	ROOM_INDOORS,		0	},
    { "no-mob",		10,	20,	2,	0,	ROOM_NO_MOB,		0	},
    { "no-recall",	50,	200,	10,	0,	ROOM_NO_RECALL,		0	},
    { "no-where",	25,	75,	5,	0,	ROOM_NOWHERE,		0	},
    { "portal-room",	50,	100,	10,	0,	ROOM_CLAN_PORTAL,	0	},
    { "private",	700,	2500,	50,	0,	ROOM_PRIVATE,		0	},
    { "safe",		1000,	4000,	50,	0,	ROOM_SAFE,		0	},
    { NULL,		0,	0,	0,	0,	0,			0	}
};

const struct clan_flag_type obj_flag_table[] =
{
//    "NAME",		CUBIC,	AQ,	IQ,	WHERE,		BIT			RESTRICT
    { "glow",		10,	10,	1,	TO_OBJECT,	ITEM_GLOW,		0		},
    { "hum",		10,	10,	1,	TO_OBJECT,	ITEM_HUM,		0		},
    { "dark",		10,	10,	1,	TO_OBJECT,	ITEM_DARK,		0		},
    { "evil",		10,	10,	1,	TO_OBJECT,	ITEM_EVIL,		0		},
    { "invis",		10,	10,	1,	TO_OBJECT,	ITEM_INVIS,		0		},
    { "magic",		10,	10,	1,	TO_OBJECT,	ITEM_MAGIC,		0		},
    { "nodrop",		75,	300,	10,	TO_OBJECT,	ITEM_NODROP,		0		},
    { "bless",		10,	10,	1,	TO_OBJECT,	ITEM_BLESS,		0		},
    { "antigood",	10,	10,	1,	TO_OBJECT,	ITEM_ANTI_GOOD,		0		},
    { "antievil",	10,	10,	1,	TO_OBJECT,	ITEM_ANTI_EVIL,		0		},
    { "antineutral",	10,	10,	1,	TO_OBJECT,	ITEM_ANTI_NEUTRAL,	0		},
    { "noremove",	75,	300,	10,	TO_OBJECT,	ITEM_NOREMOVE,		0		},
    { "nopurge",	10,	10,	1,	TO_OBJECT,	ITEM_NOPURGE,		0		},
    { "rotdeath",	10,	10,	1,	TO_OBJECT,	ITEM_ROT_DEATH,		0		},
    { "visdeath",	10,	10,	1,	TO_OBJECT,	ITEM_VIS_DEATH,		0		},
    { "nonmetal",	100,	500,	1,	TO_OBJECT,	ITEM_NONMETAL,		0		},
    { "meltdrop",	10,	10,	1,	TO_OBJECT,	ITEM_MELT_DROP,		0		},
    { "burnproof",	150,	750,	25,	TO_OBJECT,	ITEM_BURN_PROOF,	0		},
    { "nouncurse",	50,	250,	10,	TO_OBJECT,	ITEM_NOUNCURSE,		0		},
    { "nolocate",	25,	100,	5,	TO_OBJECT,	ITEM_NOLOCATE,		0		},
    { "special_save",	200,	700,	35,	TO_OBJECT,	ITEM_SPECIAL_SAVE,	0		},
    { "nosac",		25,	50,	5,	TO_OBJECT,	ITEM_NO_SAC,		0		},

    { "weapon-flaming",	75,	400,	25,	TO_WEAPON,	WEAPON_FLAMING,		WEAPON_FROST	},
    { "weapon-frost",	75,	350,	25,	TO_WEAPON,	WEAPON_FROST,		WEAPON_FLAMING	},
    { "weapon-vampiric",50,	300,	25,	TO_WEAPON,	WEAPON_VAMPIRIC,	0		},
    { "weapon-sharp",	100,	500,	35,	TO_WEAPON,	WEAPON_SHARP,		WEAPON_VORPAL	},
    { "weapon-vorpal",	100,	500,	35,	TO_WEAPON,	WEAPON_VORPAL,		WEAPON_SHARP	},
    { "weapon-twohands",50,	300,	20,	TO_WEAPON,	WEAPON_TWO_HANDS,	0		},
    { "weapon-shocking",65,	360,	25,	TO_WEAPON,	WEAPON_SHOCKING,	0		},
    { "weapon-poison",	50,	300,	25,	TO_WEAPON,	WEAPON_POISON,		0		},
    { NULL,		0,	0,	0,	0,		0,			0		}
};

const struct clan_flag_type exit_flag_table[] =
{
//    "NAME",		CUBIC,	AQ,	IQ,	WHERE,		BIT			REQUIRES
    { "closed",		10,	50,	5,	0,		EX_CLOSED,		EX_ISDOOR	},
    { "door",		20,	100,	5,	0,		EX_ISDOOR,		0		},
//  { "locked",		20,	100,	10,	0,		EX_LOCKED,		EX_CLOSED	},
    { "hidden",		20,	100,	10,	0,		EX_HIDDEN,		EX_ISDOOR	},
    { "noblink",	20,	100,	10,	0,		EX_NOBLINK,		EX_ISDOOR	},
    { "pickproof",	20,	100,	10,	0,		EX_PICKPROOF,		EX_LOCKED	},
    { "bashproof",	20,	100,	10,	0,		EX_BASHPROOF,		EX_LOCKED	},
    { "nopass",		20,	100,	10,	0,		EX_NOPASS,		EX_ISDOOR	},
    { "noscan",		20,	100,	10,	0,		EX_NO_SCAN,		EX_ISDOOR	},
    { NULL,		0,	0,	0,	0,		0,			0		}
};

const struct obj_apply_type obj_apply_table[] =
{
//    NAME,		EACH,	MAX,	WHERE,			CUBICS,	AQUEST,	IQUEST
    { "strength",	1,	4,	APPLY_STR,		25,	300,	10	},
    { "dexterity",	1,	4,	APPLY_DEX,		25,	300,	10	},
    { "intelligence",	1,	4,	APPLY_INT,		25,	300,	10	},
    { "wisdom",		1,	4,	APPLY_WIS,		25,	300,	10	},
    { "constitution",	1,	4,	APPLY_CON,		25,	300,	10	},
    { "mana",		25,	300,	APPLY_MANA,		35,	400,	15	},
    { "hp",		25,	300,	APPLY_HIT,		35,	400,	15	},
    { "move",		25,	300,	APPLY_MOVE,		35,	400,	15	},
    { "ac",		-10,	-50,	APPLY_AC,		20,	200,	5	},
    { "hitroll",	5,	35,	APPLY_HITROLL,		30,	250,	5	},
    { "damroll",	5,	35,	APPLY_DAMROLL,		25,	250,	5	},
    { "saves",		-1,	-5,	APPLY_SAVES,		30,	300,	5	},
    { "magic-power",	1,	5,	APPLY_MAGIC_POWER,	50,	450,	10	},
    { "hp-regen",	10,	100,	APPLY_REGEN_HP,		25,	350,	5	},
    { "mana-regen",	10,	100,	APPLY_REGEN_MANA,	25,	350,	5	},
    { "move-regen",	10,	100,	APPLY_REGEN_MOVE,	25,	350,	5	},
    { NULL,		0,	0,	0,			0,	0,	0	}
};

const struct price_type price_table[] =
{
//    "NAME",			COMMAND,	FLAGS,			CUBIC,	AQ,	IQ,	TYPE, 		counter,	"NOTES"
    { "list-prices",		show_prices,	0,			0,	0,	0,	DO_NOT_CHARGE, 	count_none,	"Show Current Price List"		},
    { "list-mobiles",		list_mobiles,	0,			0,	0,	0,	DO_NOT_CHARGE,	count_none,	"List Your Clan Mobiles"		},
    { "list-objects",		list_objects,	0,			0,	0,	0,	DO_NOT_CHARGE,	count_none,	"List Your Clan Objects"		},
    { "list-rooms",		list_rooms,	0,			0,	0,	0,	DO_NOT_CHARGE,	count_none,	"List Your Clan Rooms"			},
    { "list-exit-flag",		list_exit_flag,	0,			0,	0,	0,	DO_NOT_CHARGE,	count_none,	"List Current Exit Flag Prices"		},
    { "list-mob-flag",		list_mflag,	0,			0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Show Current Mob Flag Prices"		},
    { "list-room-flag",		list_rflag,	0,			0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Show Current Room Flag Prices"		},
    { "list-obj-flag",		list_oflag,	0,			0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Show Current Object Flag Prices"	},
    { "list-obj-affect",	list_oapply,	0,			0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Show Object Affect Prices"		},
    { "list-spells",		list_spells,	0,			0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Show Spells Price List"		},
    { "list-helps",		list_helps,	0,			0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Show Your Help Files"			},
    { "disband",		disband_clan,	FLAG_LEADER,		0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Permanently Delete Entire Clan"	},
    { "new-clan",		make_clan,	0,			75,	750,	10,	DO_NOT_CHARGE,	count_none,	"Purchase a Brand New Clan"		},
    { "new-help",		make_help,	FLAG_LEADER,		20,	20,	1,	1,		count_helps,	"Price * (Number of Helps + 1)"		},
    { "new-room",		make_room,	FLAG_HALL|FLAG_LEADER,	5,	50,	1,	1, 		count_rooms,	"Price * (Number of Rooms + 1)"		},
    { "new-shop",		make_shop,	FLAG_HALL|FLAG_LEADER,	30,	150,	5,	COUNT_SHOP, 	count_mobiles,	"Price * (Number of Shops + 1)"		},
    { "new-guard",		make_guard,	FLAG_HALL|FLAG_LEADER,	40,	400,	5,	COUNT_GUARD, 	count_mobiles,	"Price * (Number of Guards + 1)"	},
    { "new-armor",		make_object,	FLAG_HALL|FLAG_LEADER,	50,	500,	15,	ITEM_ARMOR,	count_object,	"Price * (Number of Armor + 1)"		},
    { "new-potion",		make_object,	FLAG_HALL|FLAG_LEADER,	50,	500,	15,	ITEM_POTION,	count_object,	"Price * (Number of Potions + 1)"	},
    { "new-pill",		make_object,	FLAG_HALL|FLAG_LEADER,	50,	500,	15,	ITEM_PILL,	count_object,	"Price * (Number of Pills + 1)"		},
    { "new-scroll",		make_object,	FLAG_HALL|FLAG_LEADER,	50,	500,	15,	ITEM_SCROLL,	count_object,	"Price * (Number of Scrolls + 1)"	},
    { "new-wand",		make_object,	FLAG_HALL|FLAG_LEADER,	50,	500,	15,	ITEM_WAND,	count_object,	"Price * (Number of Wands + 1)"		},
    { "new-staff",		make_object,	FLAG_HALL|FLAG_LEADER,	50,	500,	15,	ITEM_STAFF,	count_object,	"Price * (Number of Staves + 1)"	},
    { "new-light",		make_object,	FLAG_HALL|FLAG_LEADER,	50,	500,	15,	ITEM_LIGHT,	count_object,	"Price * (Number of Lights + 1)"	},
    { "new-pit",		make_object,	FLAG_HALL|FLAG_LEADER,	100,	1000,	35,	ITEM_PIT,	count_none,	"Add a Donation Pit"			},
    { "new-weapon",		make_object,	FLAG_HALL|FLAG_LEADER,	75,	750,	20,	ITEM_WEAPON,	count_object,	"Price * (Number of Weapons + 1)"	},
    { "new-furniture",		make_object,	FLAG_HALL|FLAG_LEADER,	100,	1000,	35,	ITEM_FURNITURE,	count_object,	"Price * (Number of Furniture + 1)"	},
    { "new-container",		make_object,	FLAG_HALL|FLAG_LEADER,	75,	700,	20,	ITEM_CONTAINER,	count_object,	"Price * (Number of Containers + 1)"	},
    { "new-drink-container",	make_object,	FLAG_HALL|FLAG_LEADER,	50,	500,	15,	ITEM_DRINK_CON,	count_object,	"Price * (Number of Drinks + 1)"	},
    { "new-food",		make_object,	FLAG_HALL|FLAG_LEADER,	25,	300,	15,	ITEM_FOOD,	count_object,	"Price * (Number of Food + 1)"		},
    { "new-boat",		make_object,	FLAG_HALL|FLAG_LEADER,	25,	300,	15,	ITEM_BOAT,	count_object,	"Price * (Number of Boats + 1)"		},
    { "new-fountain",		make_object,	FLAG_HALL|FLAG_LEADER,  75,	700,	20,	ITEM_FOUNTAIN,	count_object,	"Price * (Number of Fountains + 1)"	},
    { "new-jewelry",		make_object,	FLAG_HALL|FLAG_LEADER,	40,	400,	10,	ITEM_JEWELRY,	count_object,	"Price * (Number of Jewelry + 1)"	},
    { "new-key",		make_object,	FLAG_HALL|FLAG_LEADER,	50,	500,	20,	ITEM_KEY,	count_object,	"Make a Key For Guard, Lock Door"	},
    { "new-portal",		make_object,	FLAG_LEADER,		10,	50,	5,	ITEM_PORTAL,	count_object,	"Buy a New Portal"			},
    { "time-clan",		time_clan,	FLAG_LEADER,		35,	250,	5,	1, 		count_none,	"+2 Hours to Edit Clan Data"		},
    { "time-rooms",		time_rooms,	FLAG_LEADER,		5,	0,	0,	COUNT_ALL,	count_rooms,	"+2 Hours Edit Rooms (* #Rooms)"	},
    { "time-mobiles",		time_mobiles,	FLAG_LEADER,		5,	0,	0,	COUNT_ALL,	count_mobiles,	"+2 Hours Edit Mobs (* #Mobiles)"	},
    { "time-objects",		time_objects,	FLAG_LEADER,		5,	0,	0,	COUNT_ALL,	count_object,	"+2 Hours Edit Objs (* #Objects)"	},
    { "time-helps",		time_helps,	FLAG_LEADER,		5,	0,	0,	COUNT_ALL,	count_helps,	"+2 Hours Edit Helps (* #Helps)"	},
    { "level-guard",		level_guard,	FLAG_LEADER,		10,	50,	1,	DO_NOT_CHARGE,	count_none,	"Use No Arg For Price List"		},
    { "level-object",		level_object,	FLAG_LEADER,		10,	50,	1,	DO_NOT_CHARGE,	count_none,	"Use No Arg For Price List"		},
    { "resist-guard",		resist_guard,	FLAG_LEADER,		20,	100,	5,	1, 		count_none,	"-10% Resistance, 1500% Total Min"	},
    { "resist-object",		resist_object,	FLAG_LEADER,		75,	500,	30,	1,		count_none,	"-5% Resistance, 1 Per Object"		},
    { "flag-guard",		flag_guard,	FLAG_LEADER,		0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Flag Guard, Price Variable"		},
    { "flag-object",		flag_object,	FLAG_LEADER,		0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Flag An Object, Price Variable"	},
    { "flag-room",		flag_room,	FLAG_HALL|FLAG_LEADER,	0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Flag Current Room, Price Variable"	},
    { "flag-exit",		flag_exit,	FLAG_HALL|FLAG_LEADER,	0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Flag An Exit In Current Room"		},
    { "equip-guard",		equip_guard,	FLAG_LEADER,		20,	250,	5,	1,		count_none,	"Equip A Guard With An Item"		},
    { "eq-affect",		apply_object,	FLAG_LEADER,		0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Add Hit/Dam/Hp/etc to Items"		},
    { "add-spell",		add_spell,	FLAG_LEADER,		0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Add Spells to Magical Items"		},
    { "set-portal",		set_portal,	FLAG_HALL|FLAG_LEADER,	0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Set Current Room as Portal Room"	},
    { "set-recall",		set_recall,	FLAG_HALL|FLAG_LEADER,	25,	100,	5,	1, 		count_none,	"Set Current Room as Clan Recall"	},
    { "set-two-way",		set_two_way,	FLAG_HALL|FLAG_LEADER,	0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Set Current Room as Two Way"		},
    { "make-two-way",		make_two_way,	FLAG_LEADER,		10,	100,	5,	1,		count_none,	"Setup Two Way Exit"			},
    { "room-upgrade",		upgrade_room,	FLAG_HALL|FLAG_LEADER,	25,	100,	5,	1,		count_none,	"+50 Healing Bonus, Max 1000"		},
    { "max-members",		up_max_member,	FLAG_LEADER,		100,	500,	10,	1,		count_max,	"Clan Max Member + 1"			},
    { "edit-clan",		edit_clan,	FLAG_LEADER,		0,	0,	0,	DO_NOT_CHARGE, 	count_none,	"Edit Your General Clan Data"		},
    { "edit-room",		edit_room,	FLAG_HALL|FLAG_LEADER,	0,	0,	0,	DO_NOT_CHARGE, 	count_none,	"Edit the Room You Are In"		},
    { "edit-mobile",		edit_mobile,	FLAG_LEADER,		0,	0,	0,	DO_NOT_CHARGE, 	count_none,	"Edit One of Your Mobiles"		},
    { "edit-object",		edit_object,	FLAG_LEADER,		0,	0,	0,	DO_NOT_CHARGE, 	count_none,	"Edit One of Your Objects"		},
    { "edit-help",		edit_help,	FLAG_LEADER,		0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Edit One of Your Help Files"		},
    { "stat-clan",		stat_clan,	0,			0,	0,	0,	DO_NOT_CHARGE, 	count_none,	"Show General Clan Info"		},
    { "stat-mobile",		stat_mob,	0,			0,	0,	0,	DO_NOT_CHARGE, 	count_none,	"Show Info of a Mobile"			},
    { "stat-room",		stat_room,	FLAG_HALL,		0,	0,	0,	DO_NOT_CHARGE, 	count_none,	"Show Info of Room You Are In"		},
    { "stat-object",		stat_object,	0,			0,	0,	0,	DO_NOT_CHARGE, 	count_none,	"Show Info of an Object"		},
    { "stat-help",		stat_help,	0,			0,	0,	0,	DO_NOT_CHARGE,	count_none,	"Show Info of a Help File"		},
    { NULL,			NULL,		0,			0,	0,	0,	DO_NOT_CHARGE, 	NULL,		NULL					}
//    "NAME",			COMMAND,	FLAGS,			CUBIC,	AQ,	IQ,	TYPE, 		counter,	"NOTES"
};

