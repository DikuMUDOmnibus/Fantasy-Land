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
 **************************************************************************/

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
#include "recycle.h"

#define A_MAX_ARGS 9

void	update_chart_ranks	args( ( CHAR_DATA *ch ) );
void	save_voting_polls	args( ( void ) );
char	*display_who_custom	args( ( CHAR_DATA *ch, CHAR_DATA *wch ) );
char	*str_replace_who	args( ( char *astr, char *bstr, char *cstr ) );

bool check_forget( CHAR_DATA *victim, CHAR_DATA *ch )
{
    sh_int pos;

    if ( victim->pcdata == NULL )
	return FALSE;

    for ( pos = 0; pos < MAX_FORGET; pos++ )
    {
	if ( victim->pcdata->forget[pos] == NULL )
	    break;

	if ( !str_cmp( ch->name, victim->pcdata->forget[pos] ) )
	    return TRUE;
    }

    return FALSE;
}

bool can_quit( CHAR_DATA *ch )
{
    AUCTION_DATA *auc;

    if ( ch->pcdata->match != NULL )
    {
	send_to_char( "Your team is still in the arena!\n\r", ch );
	return FALSE;
    }

    if ( ch->in_room && IS_SET( ch->in_room->room_flags, ROOM_WAR ) )
    {
	send_to_char( "You may not leave during a war!\n\r", ch );
	return FALSE;
    }

    if ( ch->pcdata->pktimer >= 1 || ch->pcdata->opponent != NULL )
    {
	send_to_char( "Maybe you should die first.\n\r", ch );
	return FALSE;
    }

    for ( auc = auction_list; auc != NULL; auc = auc->next )
    {
	if ( auc->high_bidder == ch || auc->owner == ch )
	{
	    send_to_char( "You still have a stake in an auction!\n\r", ch );
	    return FALSE;
	}
    }

    return TRUE;
}

void do_delet( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You must type the full command to delete yourself.\n\r", ch );
    return;
}

void do_delete( CHAR_DATA *ch, char *argument )
{
    char strsave[MAX_INPUT_LENGTH], bacsave[MAX_INPUT_LENGTH];
    extern int port;

    if ( IS_NPC( ch )
    ||   port != MAIN_GAME_PORT
    ||   !can_quit( ch ) )
	return;
  
    if ( ch->pcdata->confirm_delete )
    {
	char buf[MAX_INPUT_LENGTH];

	if ( argument[0] == '\0'
	||   strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    send_to_char( "Deletion aborted, argument does not match password.\n\r", ch );
	    send_to_char( "If you wish to delete your character, just type delete.\n\r", ch );
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}	    

	if ( is_clan( ch ) )
	{
	    update_clanlist( ch, FALSE );
	    check_roster( ch, TRUE );
	}

	unrank_charts( ch );

	sprintf( strsave, "%s%s/%s", PLAYER_DIR, initial( ch->name ),
	    capitalize( ch->name ) );
	sprintf( bacsave, "%s%s/%s", BACKUP_DIR, initial( ch->name ),
	    capitalize( ch->name ) );

	wiznet( "$N has completed deletion.", ch, NULL, 0, 0, 0 );
	stop_fighting( ch, TRUE );

	if ( ch->level > LEVEL_HERO )
	    update_wizlist( ch,  1 );

	force_quit( ch, "" );

	sprintf( buf, "%splayers/%s.dat",
	    DELETED_DIR, strsave+(strlen(PLAYER_DIR)+2) );
	rename( strsave, buf );

	sprintf( buf, "%splayers/%s.bak",
	    DELETED_DIR, bacsave+(strlen(BACKUP_DIR)+2) );
	rename( bacsave, buf );

	return;
    }

    if ( argument[0] != '\0' )
    {
	send_to_char( "Just type delete. No argument.\n\r", ch );
	return;
    }

    send_to_char( "{RWARNING: {GYou are about to permanently delete your character!\n\r", ch );
    send_to_char( "{RWARNING: {GTo confirm deletion, type: delete <password>.\n\r", ch );
    send_to_char( "{RWARNING: {GTyping delete with any other argument will undo delete status.{x\n\r", ch );
    ch->pcdata->confirm_delete = TRUE;
    wiznet( "$N is contemplating deletion.", ch, NULL, 0, 0, get_trust( ch ) );
}

void do_rerol( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You must type the full command to reroll yourself.\n\r", ch );
    return;
}

void do_reroll( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch )
    ||   !can_quit( ch ) )
	return;

    if ( ch->desc == NULL )
    {
	send_to_char( "Null descriptor, reroll aborted, contact Immortal.\n\r", ch );
	return;
    }

    if ( ch->pcdata->confirm_reroll )
    {
	CHAR_DATA *wch, *prev;
	MEM_DATA *remember, *remember_next;

	if ( argument[0] == '\0'
	||   strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    send_to_char( "Reroll aborted, argument does not match password.\n\r", ch );
	    send_to_char( "If you wish to reroll your character, just type reroll.\n\r", ch );
	    ch->pcdata->confirm_reroll = FALSE;
	    return;
	}	    

	wiznet( "$N has entered reroll status.", ch, NULL, 0, 0, 0 );
	stop_fighting( ch, TRUE );
	nuke_pets( ch );
	unrank_charts( ch );

	SET_BIT( ch->act, PLR_REROLL );

	if ( ch->level > LEVEL_HERO )
	    update_wizlist( ch, 1 );

	if ( IS_SET( ch->sound, SOUND_ON ) )
	{
	    send_to_char( "\n\r!!SOUND(Off)\n\r", ch );
	    if ( !IS_SET( ch->sound, SOUND_NOMUSIC ) )
		send_to_char( "\n\r!!MUSIC(Off)\n\r", ch );
	}

	for ( wch = char_list; wch != NULL; wch = wch->next )
	{
	    for ( remember = wch->memory; remember != NULL; remember = remember_next )
	    {
		remember_next = remember->next;

		if ( ch->id == remember->id )
		    mob_forget( wch, remember );
	    }
	}

	save_char_obj( ch, 0 );

	send_to_char( "{w< {g!{G!{g!{wHit return to continue reroll procedures{g!{G!{g! {w>{x\n\r", ch );

	char_from_room( ch );

	if ( ch == player_list )
	{
	    player_list = ch->pcdata->next_player;
	} else {
	    for ( prev = player_list; prev != NULL; prev = prev->pcdata->next_player )
	    {
		if ( prev->pcdata->next_player == ch )
		{
		    prev->pcdata->next_player = ch->pcdata->next_player;
		    break;
		}
	    }
	}

	if ( ch == char_list )
	{
	    char_list = ch->next;
	} else {
	    for ( prev = char_list; prev != NULL; prev = prev->next )
	    {
		if ( prev->next == ch )
		{
		    prev->next = ch->next;
		    break;
		}
	    }
	}

	ch->desc->connected = CON_REROLLING;
	return;
    }

    if ( argument[0] != '\0' )
    {
	send_to_char( "Just type reroll. No argument.\n\r", ch );
	return;
    }

    send_to_char( "{RWARNING: {GYou are about to start over at level 1.\n\r", ch );
    send_to_char( "{RWARNING: {GTo confirm rerolling, type: reroll <password>.\n\r", ch );
    send_to_char( "{RWARNING: {GTyping reroll with any other argument will undo reroll status.{x\n\r", ch );

    if ( !IS_HERO( ch ) )
	send_to_char( "{R{zWARNING: You are not Hero, you will stay same tier!{x\n\r", ch );

    ch->pcdata->confirm_reroll = TRUE;
    wiznet( "$N is contemplating rerolling.", ch, NULL, 0, 0, get_trust( ch ) );
}

int config_lookup( int level, char *argument )
{
    int pos;

    for ( pos = 0; config_flags[pos].name != NULL; pos++ )
    {
	if ( level >= config_flags[pos].level
	&&   !str_prefix( argument, config_flags[pos].name ) )
	    return pos;
    }

    return -1;
}

void do_configure( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int pos, trust = get_trust( ch );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Option               Status\n\r"
		      "---------------------------\n\r", ch );

	for ( pos = 0; config_flags[pos].name != NULL; pos++ )
	{
	    if ( trust >= config_flags[pos].level )
	    {
		sprintf( buf, "%-20s %s{x\n\r", config_flags[pos].name,
		    IS_SET( ch->configure, config_flags[pos].flag ) ?
		    "{GON" : "{ROFF" );
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( ( pos = config_lookup( trust, argument ) ) == -1 )
    {
	send_to_char( "Invalid configuration option.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->configure, config_flags[pos].flag ) )
    {
	REMOVE_BIT( ch->configure, config_flags[pos].flag );
	sprintf( buf, "%s: configurations removed.\n\r", config_flags[pos].name );
	send_to_char( buf, ch );
    } else {
	SET_BIT( ch->configure, config_flags[pos].flag );
	sprintf( buf, "%s: configurations set.\n\r", config_flags[pos].name );
	send_to_char( buf, ch );
    }
}

void do_channels( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int pos;

    send_to_char("{Rchannel        status\n\r",ch);
    send_to_char("{R---------------------\n\r",ch);
 
    for ( pos = 0; channel_table[pos].name != NULL; pos++ )
    {
	if ( channel_table[pos].level > get_trust( ch ) )
	    continue;

	sprintf( buf, "{w%-16s%s\n\r",
	    channel_table[pos].name,
	    IS_SET( ch->comm, channel_table[pos].bit ) ? "{yOFF" : "{gON" );
	send_to_char( buf, ch );
    }

    send_to_char("{wArena           ",ch);
    if (!IS_SET(ch->comm,COMM_NOARENA))
	send_to_char("{gON\n\r",ch);
    else
	send_to_char("{yOFF\n\r",ch);

    send_to_char("{wSocial          ",ch);
    if (!IS_SET(ch->comm,COMM_NOSOCIAL))
	send_to_char("{gON\n\r",ch);
    else
	send_to_char("{yOFF\n\r",ch);

    send_to_char("{wQuiet Mode:     ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
	send_to_char("{gON\n\r",ch);
    else
	send_to_char("{yOFF\n\r",ch);

    send_to_char("{R---------------------\n\r",ch);

    if (ch->pcdata && ch->pcdata->lines != PAGELEN)
    {
	if (ch->pcdata->lines)
	{
	    sprintf(buf,"{yYou display %d lines of scroll.{x\n\r",ch->pcdata->lines+2);
	    send_to_char(buf,ch);
 	}
	else
	    send_to_char("{yScroll buffering is off.{x\n\r",ch);
    }

    if (ch->prompt != NULL)
    {
	sprintf(buf,"{yYour current prompt is{w:{x %s{x\n\r",ch->prompt);
	send_to_char(buf,ch);
    }

    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOTELL] != 0 )
	send_to_char("{yYou cannot use tell.{x\n\r",ch);
 
    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
	send_to_char("{yYou cannot use channels.{x\n\r",ch);
}

void do_cloak( CHAR_DATA *ch, char *argument )
{
    if (IS_SET(ch->act,PLR_CLOAKED_EQ))
    {
	send_to_char("You remove the magical cloak covering your equipment.\n\r",ch);
	act( "$n removes the magical cloak covering $s equipment.",
	    ch, NULL, NULL, TO_ROOM, POS_RESTING );
	REMOVE_BIT(ch->act, PLR_CLOAKED_EQ);
    } else {
	send_to_char("You pull a magical cloak over your equipment.\n\r",ch);
	act( "$n pulls a magical cloak over $s equipment.",
	    ch, NULL, NULL, TO_ROOM, POS_RESTING );
	SET_BIT(ch->act, PLR_CLOAKED_EQ);
    }
}

void do_quiet ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_QUIET))
    {
	send_to_char("Quiet mode removed.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_QUIET);
    } else {
	send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
	SET_BIT(ch->comm,COMM_QUIET);
    }
    return;
}

void do_afk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
	return;

    if ( IS_SET( ch->comm, COMM_AFK ) )
    {
	REMOVE_BIT( ch->comm, COMM_AFK );

	if ( ch->pcdata->tells )
	{
	    sprintf( buf, "{BAFK{x mode removed.  You have {R%d{x tell%s waiting.\n\r",
		ch->pcdata->tells, ch->pcdata->tells == 1 ? "" : "s" );
	    send_to_char( buf, ch );
	    send_to_char( "Type '{Rreplay{x' to see tells.\n\r", ch );
	}
	else
	    send_to_char( "{BAFK{x mode removed.  You have no tells waiting.\n\r", ch );
    } else {
	send_to_char( "You are now in {BAFK{x mode.\n\r", ch );
	SET_BIT( ch->comm, COMM_AFK );
    }
}

void do_replay( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
	send_to_char( "You can't replay.\n\r", ch );
	return;
    }

    if ( ch->pcdata->buffer->string[0] == '\0' )
    {
	send_to_char( "You have no tells to replay.\n\r", ch );
	return;
    }

    page_to_char( ch->pcdata->buffer->string, ch );
    free_buf( ch->pcdata->buffer );
    ch->pcdata->buffer = new_buf( );
    ch->pcdata->tells = 0;
    return;
}

void do_pray( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0' )
    {
	if (IS_SET(ch->comm,COMM_NOPRAY))
	{
	    send_to_char("Pray channel is now ON.\n\r",ch);
	    REMOVE_BIT(ch->comm,COMM_NOPRAY);
	} else {
	    send_to_char("Pray channel is now OFF.\n\r",ch);
	    SET_BIT(ch->comm,COMM_NOPRAY);
	}
    } else {
	if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
	{
	    send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	    return;
	}

	REMOVE_BIT(ch->comm,COMM_NOPRAY);

	sprintf( buf, "You pray to the Immortals '{9%s{x'\n\r", argument );
	send_to_char( buf, ch );
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    CHAR_DATA *victim;

	    victim = d->original ? d->original : d->character;

	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   IS_IMMORTAL(victim)
	    &&  !IS_SET(victim->comm,COMM_NOPRAY) )
	    {
		sprintf( buf, "%s$n{x petitions to you '{9$t{x'",pretitle(ch,d->character) );
		act( buf,ch,argument,d->character,TO_VICT,POS_DEAD);
	    }
	}
    }
    return;
}

void social_channel( const char *format, CHAR_DATA *ch, const void *arg2, int type )
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    CHAR_DATA *to = char_list, *vch = ( CHAR_DATA * ) arg2;
    const char *str;
    char *i, *i2, *point;
    char fixed[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    bool fColour = FALSE;

    if ( !format || !*format || !ch || !ch->desc || !ch->in_room )
	return;

    if ( type == TO_VICT )
    {
	if ( !vch )
	{
	    bug( "Social_Channel: null vch with TO_VICT.", 0 );
	    return;
	}

	if ( !vch->in_room )
	    return;

	to = vch;
    }

    if ( type == TO_CHAR )
    {
	if ( ch == NULL )
	    return;
	to = ch;
    }

    sprintf( buf2, "{GSOCIAL:{x %s", format );

    for ( ; to ; to = to->next )
    {
	if ( to->desc == NULL
	||   to->position < POS_RESTING
	||   ( type == TO_CHAR && to != ch )
	||   ( type == TO_VICT && ( to != vch || to == ch ) )
	||   ( type == TO_ROOM && to == ch )
	||   ( type == TO_NOTVICT && ( to == ch || to == vch ) )
	||   IS_SET( to->comm, COMM_NOSOCIAL )
	||   IS_SET( to->comm, COMM_QUIET ) )
	    continue;

	if ( check_forget( to, ch ) )
	    continue;

	point = buf;
	str = buf2;

        while( *str )
        {
            if( *str != '$' && *str != '{' )
            {
                *point++ = *str++;
                continue;
            }

	    i = NULL;
	    switch( *str )
	    {
		case '$':
		    fColour = TRUE;
		    ++str;
		    i = " *** ";
		    if ( !arg2 && *str >= 'A' && *str <= 'Z' && *str != 'G' )
		    {
			bug( "Act: missing arg2 for code %d.", *str );
			i = " *** ";
		    }
		    else
		    {
			switch ( *str )
			{
			    default:  
				bug( "Act: bad code %d.", *str );
				i = " *** ";                                
				break;

			    case 'T':
				i = arg2 == NULL ? "$T" : (char *) arg2;
				break;

			    case 'n': 
				i = ch == NULL ? "$n" : PERS( ch,  to  );
				break;

			    case 'N': 
				i = vch == NULL ? "$N" : PERS( vch, to  );                         
				break;

			    case 'e': 
				i = ch == NULL ? "$e" : he_she  [URANGE(0, ch  ->sex, 2)];        
				break;

			    case 'E': 
				i = vch == NULL ? "$E" : he_she  [URANGE(0, vch ->sex, 2)];        
				break;

			    case 'm': 
				i = ch == NULL ? "$m" : him_her [URANGE(0, ch  ->sex, 2)];        
				break;

			    case 'M': 
				i = vch == NULL ? "$M" : him_her [URANGE(0, vch ->sex, 2)];        
				break;

			    case 's': 
				i = ch == NULL ? "$s" : his_her [URANGE(0, ch  ->sex, 2)];        
				break;

			    case 'S': 
				i = vch == NULL ? "$S" : his_her [URANGE(0, vch ->sex, 2)];        
				break;
 

			}
		    }
		    break;

		case '{':
		    fColour = FALSE;
		    ++str;
		    i = NULL;
		    if( IS_SET( to->act, PLR_COLOUR ) )
		    {
			i = colour( *str, to );
		    }
		    break;

		default:
		    fColour = FALSE;
		    *point++ = *str++;
		    break;
	    }

            ++str;
	    if( fColour && i )
	    {
		fixed[0] = '\0';
		i2 = fixed;

		if( IS_SET( to->act, PLR_COLOUR ) )
		{
		    for( i2 = fixed ; *i ; i++ )
	            {
			if( *i == '{' )
			{
			    i++;
			    strcat( fixed, colour( *i, to ) );
			    for( i2 = fixed ; *i2 ; i2++ )
				;
			    if (*i == '\0')
				break;
			    continue;
			}
			*i2 = *i;
			*++i2 = '\0';
		    }			
		    *i2 = '\0';
		    i = &fixed[0];
		}
	        else
		{
		    for( i2 = fixed ; *i ; i++ )
	            {
			if( *i == '{' )
			{
			    i++;
			    if( *i != '{' )
			    {
				continue;
			    }
			}
			*i2 = *i;
			*++i2 = '\0';
		    }			
		    *i2 = '\0';
		    i = &fixed[0];
		}
	    }


	    if( i )
	    {
		while( ( *point = *i ) != '\0' )
		{
		    ++point;
		    ++i;
		}
	    }
        }
 
        *point++	= '\n';
        *point++	= '\r';
        *point		= '\0';

	write_to_buffer( to->desc, buf, point - buf );
    }
}

void do_social( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int cmd;
    char command[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
   
    if ( *argument == '\0' )
    {
	if ( IS_SET( ch->comm, COMM_NOSOCIAL ) )
	{
	    REMOVE_BIT( ch->comm, COMM_NOSOCIAL );
	    send_to_char( "Social channel is now ON.\n\r", ch );
	} else {
	    SET_BIT( ch->comm, COMM_NOSOCIAL );
	    send_to_char( "Social channel is now OFF.\n\r", ch );
	}
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOSOCIAL ) )
    {
	send_to_char( "Turn on the social channel first.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
	send_to_char( "Turn off quiet first.\n\r", ch );
	return;
    }

    if ( ch->in_room->vnum == ROOM_VNUM_CORNER && !IS_IMMORTAL( ch ) )
    {
	send_to_char( "Just keep your nose in the corner like a good little player.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA )
    ||   IS_SET( ch->in_room->room_flags, ROOM_WAR ) )
    {
	send_to_char( "You do not have access to this from within a bloody cell.\n\r", ch );
	return;
    }

    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
    {
	send_to_char( "Your channels have been revoked.\n\r", ch );
	return;
    }

    argument = one_argument( argument, command );

    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( found == FALSE )
    {
	send_to_char( "You wanna social what to who?!?!\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	social_channel( social_table[cmd].others_no_arg, ch, NULL, TO_ROOM );
	social_channel( social_table[cmd].char_no_arg, ch, NULL, TO_CHAR );
	sprintf( buf, "%s, with no target.", capitalize(social_table[cmd].name) );
    } 

    else if ( ( victim = IS_IMMORTAL( ch ) ? get_char_world( ch, argument ) :
	get_pc_world( ch, argument ) ) == NULL )
	send_to_char( "They aren't here.\n\r", ch );

    else if ( victim == ch )
    {
	social_channel( social_table[cmd].others_auto, ch, victim, TO_ROOM );
	social_channel( social_table[cmd].char_auto, ch, victim, TO_CHAR );
	sprintf( buf, "%s, with self as target.", capitalize(social_table[cmd].name) );
    }

    else
    {
	social_channel( social_table[cmd].others_found, ch, victim, TO_NOTVICT );
	social_channel( social_table[cmd].char_found, ch, victim, TO_CHAR );
	social_channel( social_table[cmd].vict_found, ch, victim, TO_VICT );
	sprintf( buf, "%s, with %s{x as target.", capitalize(social_table[cmd].name), 
         IS_NPC(victim) ? victim->short_descr : victim->name );
    }
    return;
}

void do_gmote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
	send_to_char("Gmote what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
	send_to_char( "Turn on the social channel first.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
	send_to_char( "Turn off quiet first.\n\r", ch );
	return;
    }

    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
    {
	send_to_char( "The gods have revoked your channel priviliges.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA )
    ||   IS_SET( ch->in_room->room_flags, ROOM_WAR ))
    {
	send_to_char( "You do not have access to this from within a bloody cell.\n\r", ch );
        return;
    }
 
    if ( ch->in_room->vnum == ROOM_VNUM_CORNER && !IS_IMMORTAL( ch ) )
    {
	send_to_char( "Just keep your nose in the corner like a good little player.\n\r", ch );
	return;
    }

    argument = channel_parse( ch, argument, TRUE );

    social_channel( "$n $T{x", ch, argument, TO_ROOM );
    social_channel( "$n $T{x", ch, argument, TO_CHAR );
    sprintf( buf, "%s %s", ch->name, argument );
    return;
}

void do_say( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob, *mob_next;
    OBJ_DATA *obj, *obj_next;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }

    if ( is_affected( ch, skill_lookup( "silence" ) ) )
    {
	send_to_char( "Your lips fail to move.\n\r", ch );
	return;
    }

    argument = channel_parse(ch,argument,TRUE);

    sprintf( buf, "You say '{S%s{x'\n\r", argument );
    send_to_char( buf, ch );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim;

        victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING
	&&   d->character != ch 
	&&   ch->in_room == d->character->in_room
	&&   !check_forget( victim, ch ) )
        {
	    sprintf( buf, "%s$n{x says '{S$t{x'",pretitle(ch,d->character) );
	    act( buf,ch,argument,d->character,TO_VICT,POS_RESTING);
        }
    }

    for ( mob = ch->in_room->people; mob != NULL; mob = mob_next )
    {
	mob_next = mob->next_in_room;

	if ( IS_NPC(mob) && HAS_TRIGGER_MOB( mob, TRIG_SPEECH )
	&&   mob->position == mob->pIndexData->default_pos
	&&   mob != ch )
	    p_act_trigger( argument, mob, NULL, NULL, ch, NULL, NULL, TRIG_SPEECH );

	for ( obj = mob->carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( HAS_TRIGGER_OBJ( obj, TRIG_SPEECH ) )
		p_act_trigger( argument, NULL, obj, NULL, ch, NULL, NULL, TRIG_SPEECH );
	}
    }

    for ( obj = ch->in_room->contents; obj; obj = obj_next )
    {
	obj_next = obj->next_content;

	if ( HAS_TRIGGER_OBJ( obj, TRIG_SPEECH ) )
	    p_act_trigger( argument, NULL, obj, NULL, ch, NULL, NULL, TRIG_SPEECH );
    }
	
    if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_SPEECH ) )
	p_act_trigger( argument, NULL, NULL, ch->in_room, ch, NULL, NULL, TRIG_SPEECH );

    return;
}

void do_shout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0' )
    {
	send_to_char( "Shout what?\n\r", ch );
	return;
    }

    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
    {
        send_to_char( "You can't shout.\n\r", ch );
        return;
    }

    if ((ch->in_room->vnum == ROOM_VNUM_CORNER) && (!IS_IMMORTAL(ch)))
    {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    || IS_SET(ch->in_room->room_flags, ROOM_WAR))
    {
        send_to_char("You do not have access to this from within a bloody cell.\n\r",ch);
        return;
    } 

    argument = channel_parse(ch,argument,TRUE);
    
    act( "You shout '{T$T{x'", ch, NULL, argument, TO_CHAR, POS_RESTING );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   !check_forget( victim, ch ) )
	{
	    sprintf(buf, "%s$n{x shouts '{T$t{x'",pretitle(ch,d->character));
	    act(buf, ch,argument,d->character,TO_VICT,POS_RESTING);
	}
    }
    return;
}

void wiz_tells( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, "{VWIZ_TELL: %s tells %s '{U%s{V'",
	ch->name, victim->name, argument );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *wch = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   d->character != victim
	&&   IS_SET( wch->wiznet, WIZ_TELLS )
	&&   can_over_ride( wch, ch, FALSE )
	&&   can_over_ride( wch, victim, FALSE ) )
	{
	    if ( IS_SET( d->character->wiznet, WIZ_PREFIX ) )
		send_to_char( "{Y-->{x ", d->character );
	    act( buf, ch, argument, d->character, TO_VICT, POS_DEAD );
	}
    }
}

void send_tell_reply( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
	send_to_char( "What do you wish to tell?\n\r", ch );
	return;
    }

    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOTELL] != 0 )
    {
	send_to_char( "Your tells have been revoked.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->configure, CONFIG_DEAF ) )
    {
	send_to_char( "You are blocking all tells.\n\r", ch );
	return;
    }

    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
    {
	send_to_char( "Your channels are revoked!\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    if ( ch->in_room->vnum == ROOM_VNUM_CORNER && !IS_IMMORTAL( ch ) )
    {
	send_to_char( "Just keep your nose in the corner like a good little player.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA )
    ||   IS_SET( ch->in_room->room_flags, ROOM_WAR ) )
    {
	send_to_char( "You do not have access to this from within a bloody cell.\n\r", ch );
	return;
    }

    if ( ( IS_SET( victim->comm, COMM_QUIET )
    ||     IS_SET( victim->configure, CONFIG_DEAF ) )
    &&   !IS_IMMORTAL( ch ) )
    {
	act( "$E is not receiving tells.", ch, NULL, victim, TO_CHAR, POS_DEAD );
  	return;
    }

    if ( check_forget( victim, ch ) )
    {
	act( "$N doesn't seem to be listening to you.",
	    ch, NULL, victim, TO_CHAR, POS_DEAD );
	return;
    }

    argument = channel_parse( ch, argument, FALSE );
    wiz_tells( ch, victim, argument );

    if ( victim->desc == NULL && !IS_NPC( victim ) )
    {
	act( "$N seems to have misplaced $S link...adding message to $S replay buffer.",
	    ch, NULL, victim, TO_CHAR, POS_DEAD );
	sprintf( buf, "%s%s{x tells you '{U%s{x'\n\r",
	    pretitle( ch, victim ), PERS( ch, victim ), argument );
	add_buf( victim->pcdata->buffer, buf );
	victim->pcdata->tells++;
	return;
    }

    if ( IS_SET( victim->comm, COMM_AFK ) )
    {
	act( "$E is AFK, but your tell will go through when $E returns.",
	    ch, NULL, victim, TO_CHAR, POS_DEAD );

	sprintf( buf, "%s%s{x tells you '{U%s{x'\n\r",
	    pretitle( ch, victim ), PERS( ch, victim ), argument );

	if ( victim->pcdata )
	{
	    add_buf( victim->pcdata->buffer, buf );
	    victim->pcdata->tells++;
	}

	send_to_char( "(Storing Tell) ", victim );
	send_to_char( buf, victim );
	return;
    }

    if ( IS_SET( victim->configure, CONFIG_STORE )
    &&  victim->fighting != NULL && victim->pcdata != NULL )
    {
	act( "$E is fighting, but your tell will go through when $E finishes.",
	    ch, NULL, victim, TO_CHAR, POS_DEAD );

	sprintf( buf, "%s%s{x tells you '{U%s{x'\n\r",
	    pretitle( ch, victim ), PERS( ch, victim ), argument );

	add_buf( victim->pcdata->buffer, buf );
	victim->pcdata->tells++;

	send_to_char( "{g*{G*{g* {RIncoming message {g*{G*{g*{x\n\r", victim );
	return;
    }

    sprintf( buf, "You tell %s$N{x '{U$t{x'", pretitle( victim, ch ) );
    act( buf, ch, argument, victim, TO_CHAR, POS_DEAD );

    sprintf( buf, "%s$n{x tells you '{U$t{x'", pretitle( ch, victim ) );
    act( buf, ch, argument, victim, TO_VICT, POS_DEAD );

    if ( victim->pcdata )
	victim->pcdata->reply = ch;

    if ( !IS_NPC( ch ) && IS_NPC( victim )
    &&   HAS_TRIGGER_MOB( victim, TRIG_SPEECH ) )
	p_act_trigger( argument, victim, NULL, NULL, ch, NULL, NULL, TRIG_SPEECH );
}

void do_tell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    if ( ( victim = get_pc_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    send_tell_reply( ch, victim, argument );
    return;
}

void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( IS_NPC( ch ) )
	return;

    if ( ( victim = ch->pcdata->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    send_tell_reply( ch, victim, argument );
    return;
}

void do_yell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *wch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Yell what?\n\r", ch );
	return;
    }

    if ((ch->in_room->vnum == ROOM_VNUM_CORNER) && (!IS_IMMORTAL(ch)))
    {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
    }

    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
        return;

    argument = channel_parse(ch,argument,TRUE);
    
    act("You yell '{T$t{x'",ch,argument,NULL,TO_CHAR,POS_DEAD);
    for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
    {
	if ( !IS_NPC(wch)
	&&   wch != ch
	&&   wch->in_room != NULL
	&&   (wch->in_room->area == ch->in_room->area || IS_IMMORTAL(wch))
        &&   !IS_SET(wch->comm,COMM_QUIET)
	&&   !check_forget( wch, ch ) )
	{
	    if ( IS_IMMORTAL(wch) )
		sprintf(buf, "%s$n{x (%s) yells '{T$t{x'",
		    pretitle(ch,wch), ch->in_room->area->name);
	    else
		sprintf(buf, "%s$n{x yells '{T$t{x'",pretitle(ch,wch));
	    act(buf,ch,argument,wch,TO_VICT,POS_RESTING);
	}
    }
    return;
}

void do_emote( CHAR_DATA *ch, char *argument )
{
    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
	return;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }

    argument = channel_parse(ch,argument,TRUE);
 
    MOBtrigger = FALSE;
    act( "{1$n{x $T{x{0", ch, NULL, argument, TO_ROOM, POS_RESTING );
    act( "{1$n{x $T{x{0", ch, NULL, argument, TO_CHAR, POS_DEAD );
    MOBtrigger = TRUE;
    return;
}

void do_pmote( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char *letter,*name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;

    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
	return;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }

    argument = channel_parse(ch,argument,TRUE);
 
    act( "{c$n $t{x", ch, argument, NULL, TO_CHAR, POS_DEAD );

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->desc == NULL || vch == ch)
	    continue;

	if ((letter = strstr(argument,vch->name)) == NULL)
	{
          MOBtrigger = FALSE;
	    act("{c$N $t{x",vch,argument,ch,TO_CHAR,POS_RESTING);
          MOBtrigger = TRUE;
	    continue;
	}

	strcpy(temp,argument);
	temp[strlen(argument) - strlen(letter)] = '\0';
   	last[0] = '\0';
 	name = vch->name;
	
	for (; *letter != '\0'; letter++)
	{ 
	    if (*letter == '\'' && matches == strlen(vch->name))
	    {
		strcat(temp,"r");
		continue;
	    }

	    if (*letter == 's' && matches == strlen(vch->name))
	    {
		matches = 0;
		continue;
	    }
	    
 	    if (matches == strlen(vch->name))
	    {
		matches = 0;
	    }

	    if (*letter == *name)
	    {
		matches++;
		name++;
		if (matches == strlen(vch->name))
		{
		    strcat(temp,"you");
		    last[0] = '\0';
		    name = vch->name;
		    continue;
		}
		strncat(last,letter,1);
		continue;
	    }

	    matches = 0;
	    strcat(temp,last);
	    strncat(temp,letter,1);
	    last[0] = '\0';
	    name = vch->name;
	}

      MOBtrigger = FALSE;
	act("{c$N $t{x",vch,temp,ch,TO_CHAR,POS_RESTING);
      MOBtrigger = TRUE;
    }
	
    return;
}

void do_quit( CHAR_DATA *ch, char *argument )
{
    DESC_CHECK_DATA *dc, *dc_next;
    DESCRIPTOR_DATA *d, *d_next;
    int id;

    if ( IS_NPC( ch )
    ||   !can_quit( ch ) )
	return;

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "No way! You are fighting.\n\r", ch );
	return;
    }

    if ( ch->position < POS_STUNNED  )
    {
	send_to_char( "You're not DEAD yet.\n\r", ch );
	return;
    }

    if ( ch->level < 2 && ch->pcdata->tier == 1 )
    {
	if ( ch->pcdata->confirm_delete )
	{
	    if ( argument[0] != '\0' )
	    {
		send_to_char( "Delete status removed.\n\r", ch );
		ch->pcdata->confirm_delete = FALSE;
		return;
	    } else {
		if ( is_clan( ch ) )
		{
		    update_clanlist( ch, FALSE );
		    check_roster( ch, TRUE );
		}

		wiznet( "$N quits, turning $Mself into line noise.",
		    ch, NULL, 0, 0, 0 );
		stop_fighting( ch, TRUE );
		force_quit( ch, "" );
		return;
	    }
	}

	if ( argument[0] != '\0' )
	{
	    send_to_char( "Just type quit. No argument.\n\r", ch );
	    return;
	}

	send_to_char( "{RWARNING{w: {GYou must be level 2 to save.{x\n\r", ch );
	send_to_char( "Type quit again to confirm this command.\n\r", ch );
	send_to_char( "WARNING: this command is irreversible.\n\r", ch );
	send_to_char( "Typing quit with an argument will undo delete status.\n\r", ch );
	ch->pcdata->confirm_delete = TRUE;
	wiznet( "$N is contemplating quit deletion.",
	    ch, NULL, 0, 0, get_trust( ch ) );
	return;
    }

    if ( IS_SET( ch->sound, SOUND_ON ) )
    {
	send_to_char( "\n\r!!SOUND(Off)\n\r", ch );
	if ( !IS_SET( ch->sound, SOUND_NOMUSIC ) )
	    send_to_char( "\n\r!!MUSIC(Off)\n\r", ch );
    }

    if ( ch->pcdata->tells )
    {
	send_to_char( "Automatically replaying stored tells...\n\r\n\r", ch );
	do_replay( ch, "" );
	send_to_char( "\n\r", ch );
    }

    update_chart_ranks( ch );

    send_to_char( "{gJust as you begin walking away, someone asks you, \"{GYou're.. you're leaving already?\"{x\n\r",ch);
    send_to_char( "{gAs you duck through the doorway, you hear a shout of, \"{GDon't forget the door closes quite quickl-\"{x\n\r",ch);
    send_to_char( "{Y*{RK{YA{RP{YO{RW{Y*{G, the door slams right on your ass!{x\n\r\n\r",ch);
    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM, POS_RESTING );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );
    wiznet( "$N rejoins the real world.", ch, NULL, WIZ_LOGINS, 0, get_trust( ch ) );

    /*
     * After extract_char the ch is no longer valid!
     */
    save_char_obj( ch, 0 );
    id = ch->id;
    d = ch->desc;

    if ( ch != NULL && !IS_IMMORTAL( ch ) && ch->desc != NULL )
    {
	bool found = FALSE;

	for ( dc = desc_check_list; dc != NULL; dc = dc_next )
	{
	    dc_next = dc->next;

	    if ( !str_cmp( dc->name, ch->name )
	    ||   !str_cmp( dc->host, d->host ) )
	    {
		strcpy( dc->name, ch->name );
		strcpy( dc->host, d->host );
		dc->wait_time = 5;
		found = TRUE;
		break;
	    }
	}

	if ( !found )
	{
	    dc = new_desc_check( );

	    strcpy( dc->name, ch->name );
	    strcpy( dc->host, d->host );
	    dc->wait_time = 5;

	    dc->next            = desc_check_list;
	    desc_check_list     = dc;
	}
    }

    extract_char( ch, TRUE );
    if ( d != NULL )
	close_socket( d );

    /* toast evil cheating bastards */
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	CHAR_DATA *tch;

	d_next = d->next;
	tch = d->original ? d->original : d->character;
	if ( tch && tch->id == id )
	{
	    extract_char( tch, TRUE );
	    close_socket( d );
	} 
    }

    return;
}

void force_quit( CHAR_DATA *ch, char *argument )
{
    DESC_CHECK_DATA *dc, *dc_next;
    DESCRIPTOR_DATA *d, *d_next;
    int id;

    if ( IS_NPC( ch )
    ||   !can_quit( ch ) )
	return;

    if ( ch->timer != 0 )
	ch->timer = 0;

    if ( ch->pcdata->tells )
    {
	send_to_char( "Automatically replaying stored tells...\n\r\n\r", ch );
	do_replay( ch, "" );
	send_to_char( "\n\r", ch );
    }

    if ( IS_SET( ch->sound, SOUND_ON ) )
    {
	send_to_char( "\n\r!!SOUND(Off)\n\r", ch );
	if ( !IS_SET( ch->sound, SOUND_NOMUSIC ) )
	    send_to_char( "\n\r!!MUSIC(Off)\n\r", ch );
    }

    update_chart_ranks( ch );

    if ( ch->level < 2 && ch->pcdata->tier == 1 )
	send_to_char( "{RWARNING{w: {GYou must be level 2 to save.{x\n\r\n\r", ch );

    send_to_char( "{gJust as you begin walking away, someone asks you, \"{GYou're.. you're leaving already?\"{x\n\r", ch );
    send_to_char( "{gAs you duck through the doorway, you hear a shout of, \"{GDon't forget the door closes quite quickl-\"{x\n\r", ch );
    send_to_char( "{Y*{RK{YA{RP{YO{RW{Y*{G, the door slams right on your ass!{x\n\r\n\r", ch );
    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM, POS_RESTING );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );
    wiznet( "$N rejoins the real world.", ch, NULL, WIZ_LOGINS, 0, get_trust( ch ) );

    if ( argument[0] != '\0' && !str_cmp( argument, "punload" ) )
	save_char_obj( ch, 2 );
    else
	save_char_obj( ch, 0 );

    /*
     * After extract_char the ch is no longer valid!
     */
    id = ch->id;
    d = ch->desc;

    if ( ch != NULL && !IS_IMMORTAL( ch ) && ch->desc != NULL )
    {
	bool found = FALSE;

	for ( dc = desc_check_list; dc != NULL; dc = dc_next )
	{
	    dc_next = dc->next;

	    if ( !str_cmp( dc->name, ch->name )
	    ||   !str_cmp( dc->host, d->host ) )
	    {
		strcpy( dc->name, ch->name );
		strcpy( dc->host, d->host );
		dc->wait_time = 5;
		found = TRUE;
		break;
	    }
	}

	if ( !found )
	{
	    dc = new_desc_check( );

	    strcpy( dc->name, ch->name );
	    strcpy( dc->host, d->host );
	    dc->wait_time = 5;

	    dc->next            = desc_check_list;
	    desc_check_list     = dc;
	}
    }

    extract_char( ch, TRUE );
    if ( d != NULL )
	close_socket( d );

    /* toast evil cheating bastards */
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	CHAR_DATA *tch;

	d_next = d->next;
	tch = d->original ? d->original : d->character;
	if ( tch && tch->id == id )
	{
	    extract_char( tch, TRUE );
	    close_socket( d );
	} 
    }

    return;
}

void do_save( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;

    if ( IS_NPC( ch ) )
	return;

    if ( ch->level < 2 && ch->pcdata->tier == 1 )
    {
	send_to_char( "You must be level 2 to save.\n\r", ch );
	return;
    }

    for ( vch = player_list; vch != NULL; vch = vch->pcdata->next_player )
	save_char_obj( vch, 0 );

    act( "The legend of your existence has been scored on the walls of $t.",
	ch, mud_stat.mud_name_string, NULL, TO_CHAR, POS_DEAD );
}

void do_backup( CHAR_DATA *ch, char *argument )
{
    extern int port;

    if ( IS_NPC( ch ) || port != MAIN_GAME_PORT )
	return;

    if ( ch->level < 2 && ch->pcdata->tier == 1 )
    {
	send_to_char( "You must be level 2 to save a backup.\n\r", ch );
	return;
    }

    save_char_obj( ch, 3 );
    act( "Your existence has been permanently etched into the halls of $t.",
	ch, mud_stat.mud_name_string, NULL, TO_CHAR, POS_DEAD );
}

void do_follow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR, POS_DEAD );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_IMMORTAL(ch))
    {
	act("$N doesn't seem to want any followers.\n\r",
             ch,NULL,victim, TO_CHAR, POS_DEAD);
        return;
    }

    if ( !IS_NPC(ch) && !IS_NPC(victim) )
    {
	if ( ch->pcdata->opponent != NULL
	&&   !can_pk(victim,ch->pcdata->opponent) )
	{
	    printf_to_char(ch,"You may not follow %s, because they can not attack %s.\n\r",
		victim->name, ch->pcdata->opponent->name);
	    return;
	}

	if ( victim->pcdata->opponent != NULL
	&&   !can_pk(ch,victim->pcdata->opponent) )
	{
	    printf_to_char(ch,"You may not follow %s, because you can not attack %s.\n\r",
		victim->name, victim->pcdata->opponent->name);
	    return;
	}
    }

    REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    
    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}

void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT, POS_RESTING );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR, POS_DEAD );

    return;
}

void stop_follower( CHAR_DATA *ch )
{
    if ( ch->master == NULL )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	affect_strip( ch, gsn_charm_person );
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
    }

    if ( can_see( ch->master, ch ) && ch->in_room != NULL)
    {
	act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT, POS_RESTING );
    	act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR, POS_RESTING );
    }

//    if (ch->master->pet == ch)
//	ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;

    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch )
{
    CHAR_DATA *pet,*pet_next;

    for ( pet=char_list; pet != NULL; pet=pet_next ) {
     pet_next=pet->next;
     if ( pet->master == ch && IS_NPC(pet) ) {
    	stop_follower(pet);
    	if (pet->in_room != NULL)
    	    act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT,POS_RESTING);
    	extract_char(pet,TRUE);
     }
    }
    return;
}

void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL )
    {
	stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

    return;
}

#if defined(NEVER) // old order command
void do_order( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );
    one_argument(argument,arg2);

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if ( !str_cmp(arg2,"delete") || !str_cmp(arg2,"mob")
    ||   !str_cmp(arg2,"note")   || !str_cmp(arg2,"quest")
    ||   !str_cmp(arg2,"reroll") || !str_cmp(arg2,"prompt")
    ||   !str_cmp(arg2,"ques")   || !str_cmp(arg2,"que")
    ||	 !str_cmp(arg2,"junk")   || !str_cmp(arg2,"jun")
    ||   !str_cmp(arg2,"ju")     || !str_cmp(arg2,"pro")
    ||   !str_cmp(arg2,"promp")  || !str_cmp(arg2,"prom") )
    {
	send_to_char("You cannot order them to do that.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch 
	||  !can_over_ride( ch, victim, FALSE ) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;

	    if ( !IS_NPC(och)
	    &&   (!str_prefix(arg2,"report")	|| !str_prefix(arg2,"give")
	    ||    !str_prefix(arg2,"drop")	|| !str_prefix(arg2,"donate")
	    ||    !str_prefix(arg2,"cdonate")	|| !str_prefix(arg2,"nosummon")
	    ||	  !str_prefix(arg2,"nocancel")	|| !str_prefix(arg2,"slip")) )
		send_to_char("You can't order players to do that.\n\r",ch);
	    else
	    {
		sprintf( buf, "$n orders you to '%s'.", argument );
		act( buf, ch, NULL, och, TO_VICT,POS_DEAD );
		interpret( och, argument );
	    }
	}
    }

    if ( found )
    {
	WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
	send_to_char( "Ok.\n\r", ch );
    }
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}
#endif


/*
 * Locke's do_order()
 * Syntax:  order [person] [action]
 */
void do_order( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg_cmd[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );
    one_argument( argument, arg_cmd );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        to_actor( "Order whom to do what?\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
        to_actor( "You feel like taking, not giving, orders.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        fAll   = TRUE;
        victim = NULL;
    }
    else
    {
        fAll   = FALSE;
        if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
        {
            to_actor( "They aren't here.\n\r", ch );
            return;
        }

        if ( victim == ch )
        {
            to_actor( "Aye aye, right away!\n\r", ch );
            return;
        }

        if ( ( !IS_AFFECTED(victim, AFF_CHARM) && !IS_SET(victim->act,ACT_PET) )
           || victim->master != ch )
        {
            to_actor( "Do it yourself!\n\r", ch );
            return;
        }
    }

    if ( str_prefix( arg_cmd, "get"    )
      && str_prefix( arg_cmd, "drop"   )
      && str_prefix( arg_cmd, "wear"   )
     // && str_prefix( arg_cmd, "sheath" )
      && str_prefix( arg_cmd, "assist" )
      && str_prefix( arg_cmd, "swap"   )
      && str_prefix( arg_cmd, "dump"   )
      && str_prefix( arg_cmd, "eat"    )
      && str_prefix( arg_cmd, "quaff"  )
      && str_prefix( arg_cmd, "zap"    )
      && str_prefix( arg_cmd, "smote"    )
      && str_prefix( arg_cmd, "give"   )
      && str_prefix( arg_cmd, "recite" )
      && str_prefix( arg_cmd, "sit"    )
      && str_prefix( arg_cmd, "shoot"  )
      && str_prefix( arg_cmd, "tactic" )
      && str_prefix( arg_cmd, "report" )
      && str_prefix( arg_cmd, "light"  )
      && str_prefix( arg_cmd, "extinguish"  )
      && str_prefix( arg_cmd, "flee"   )
      && str_prefix( arg_cmd, "north"  )
      && str_prefix( arg_cmd, "south"  )
      && str_prefix( arg_cmd, "east"   )
      && str_prefix( arg_cmd, "west"   )
      && str_prefix( arg_cmd, "up"     )
      && str_prefix( arg_cmd, "ride"   )
      && str_prefix( arg_cmd, "mount"  )
      && str_prefix( arg_cmd, "down"   )
     // && str_prefix( arg_cmd, "northeast"  )
     // && str_prefix( arg_cmd, "southeast"  )
     // && str_prefix( arg_cmd, "northwest"  )
     // && str_prefix( arg_cmd, "southwest"  )
      && str_prefix( arg_cmd, "track"  )
      && str_prefix( arg_cmd, "pick"   )
      && str_prefix( arg_cmd, "steal"  )
      && str_prefix( arg_cmd, "sneak"  )
      && str_prefix( arg_cmd, "hide"   )
      && str_prefix( arg_cmd, "pour"   )
      && str_prefix( arg_cmd, "report" )
      && str_prefix( arg_cmd, "rest"   )
      && str_prefix( arg_cmd, "stand"  )
      && str_prefix( arg_cmd, "sleep"  )
      && str_prefix( arg_cmd, "group"  )
      && str_prefix( arg_cmd, "exit"   )
      && str_prefix( arg_cmd, "kill"   )
      && str_prefix( arg_cmd, "enter"  )
      && str_prefix( arg_cmd, "drink"  )
      && str_prefix( arg_cmd, "give"   )
      && str_prefix( arg_cmd, "put"    )
      && str_prefix( arg_cmd, "cast"   )
      && str_prefix( arg_cmd, "draw"   )
      && str_prefix( arg_cmd, "stand"  )
      && str_prefix( arg_cmd, "remove" )
      && str_prefix( arg_cmd, "wield"  )
      && str_prefix( arg_cmd, "hold"   ) )
    {
        to_actor( "It is impossible to order them to do that.\n\r", ch );
        return;
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
        och_next = och->next_in_room;

        if ( ( IS_AFFECTED(och, AFF_CHARM) || ( IS_NPC(och) && IS_SET(och->act,ACT_PET) ) )
        &&   och->master == ch
        && ( fAll || och == victim ) )
        {
            found = TRUE;
            act( "$n orders you to '$t'.", ch, argument, och, TO_VICT, POS_SLEEPING );
            interpret( och, argument );
        }
    }

    if ( found )
    to_actor( "Ok.\n\r", ch );
    else
    to_actor( "You have no followers here.\n\r", ch );
    return;
}

void do_group( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( argument[0] == '\0' )
    {
	BUFFER *final = new_buf( );
	CHAR_DATA *gch, *leader;
	char buf[MAX_STRING_LENGTH];
	char hpval[10], manaval[10], moveval[10];
	int percent;
	bool whoString;

	leader = ( ch->leader != NULL ) ? ch->leader : ch;

	add_buf( final, "{s ========================================================================\n\r|" );

	sprintf( buf, "{tLeader: {q%s%s%s",
	    pretitle( leader, ch ), PERS( leader, ch ),
	    leader->pcdata ? leader->pcdata->title : "" );
	add_buf( final, center_string( buf, 72 ) );

	add_buf( final, "{s|\n\r|========================================================================|\n\r" );

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( !is_same_group( gch, ch ) )
		continue;

	    whoString = FALSE;

	    if ( ((!IS_IMMORTAL(ch)
	    &&     gch != ch)
	    ||    !can_over_ride(ch,gch,TRUE))
	    &&   gch->pcdata && gch->pcdata->who_descr[0] != '\0' )
		whoString = TRUE;

	    if ( gch->max_hit > 0 )
	    {
		percent = 100 * gch->hit / gch->max_hit;
		sprintf( hpval, "{%c%3d",
		    percent > 66 ? 'G' : percent > 33 ? 'Y' : 'R', percent );
	    }

	    else
		sprintf( hpval, "{G100" );

	    if ( gch->max_mana > 0 )
	    {
		percent = 100 * gch->mana / gch->max_mana;
		sprintf( manaval, "{%c%3d",
		    percent > 66 ? 'G' : percent > 33 ? 'Y' : 'R', percent );
	    }

	    else
		sprintf( manaval, "{G100" );

	    if ( gch->max_move > 0 )
	    {
		percent = 100 * gch->move / gch->max_move;
		sprintf( moveval, "{%c%3d",
		    percent > 66 ? 'G' : percent > 33 ? 'Y' : 'R', percent );
	    }

	    else
		sprintf( moveval, "{G100" );

	    if ( whoString )
                sprintf( buf, "{s| {q---{s | {q--- {s| {t%s {s| {tHP: %s{q%%{s | {tMANA: %s{q%%{s |{t MOVES: %s{q%%{s |\n\r",
                    end_string( PERS( gch, ch ), 20 ),
                    hpval, manaval, moveval );
	    else	
	        sprintf( buf, "{s| {q%3d{s | %s {s| {t%s {s| {tHP: %s{q%%{s | {tMANA: %s{q%%{s |{t MOVES: %s{q%%{s |\n\r",
		    gch->level,
		    gch->pcdata || IS_SET( gch->act, ACT_SMART_MOB ) ?
		    class_table[gch->class].who_name : "   ",
		    end_string( PERS( gch, ch ), 20 ),
		    hpval, manaval, moveval );

	    add_buf( final, buf );
	}

	add_buf( final, " ========================================================================{x\n\r" );

	page_to_char( final->string, ch );
	free_buf( final );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act( "$N isn't following you.", ch, NULL, victim, TO_CHAR, POS_SLEEPING );
	return;
    }

    if ( IS_AFFECTED( victim, AFF_CHARM ) )
    {
	send_to_char( "You can't remove charmed mobs from your group.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	act( "You like your master too much to leave $m!",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	return;
    }

/*
    if ( victim->level - ch->level > 10 )
    {
	send_to_char( "They are to high of a level for your group.\n\r", ch );
	return;
    }

    if ( victim->level - ch->level < -10 )
    {
	send_to_char( "They are to low of a level for your group.\n\r", ch );
	return;
    }
 */

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act( "$n removes $N from $s group.",
	    ch, NULL, victim, TO_NOTVICT, POS_RESTING );
	act( "$n removes you from $s group.",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	act( "You remove $N from your group.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return;
    }

    victim->leader = ch;
    act( "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT, POS_RESTING );
    act( "You join $n's group.", ch, NULL, victim, TO_VICT, POS_RESTING );
    act( "$N joins your group.", ch, NULL, victim, TO_CHAR, POS_RESTING );
    return;
}

void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount_platinum = 0, amount_gold = 0, amount_silver = 0;
    int share_platinum, share_gold, share_silver;
    int extra_platinum, extra_gold, extra_silver;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
               one_argument( argument, arg3 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount_silver = atoi( arg1 );

    if (arg2[0] != '\0')
	amount_gold = atoi(arg2);

    if (arg3[0] != '\0')
	amount_platinum = atoi(arg3);

    if ( amount_platinum < 0 || amount_gold < 0 || amount_silver < 0)
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount_platinum == 0 && amount_gold == 0 && amount_silver == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( (ch->silver + (ch->gold * 100) + (ch->platinum * 10000) )
       < (amount_silver + (amount_gold * 100) + (amount_platinum * 10000) ) )
    {
	send_to_char( "You don't have that much to split.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }
	    
    share_platinum   = amount_platinum / members;
    extra_platinum   = amount_platinum % members;

    amount_gold += (extra_platinum * 100);
    share_gold   = amount_gold / members;
    extra_gold   = amount_gold % members;

    amount_silver += (extra_gold * 100);
    share_silver = amount_silver / members;
    extra_silver = amount_silver % members;

    if ( share_platinum == 0 && share_gold == 0 && share_silver == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    deduct_cost(ch,amount_platinum-extra_platinum,VALUE_PLATINUM);
    add_cost(ch,share_platinum,VALUE_PLATINUM);
    deduct_cost(ch,amount_gold-extra_gold,VALUE_GOLD);
    add_cost(ch,share_gold,VALUE_GOLD);
    deduct_cost(ch,amount_silver,VALUE_SILVER);
    add_cost(ch,share_silver+extra_silver,VALUE_SILVER);

    if (share_platinum > 0)
    {
	sprintf(buf,
	    "You split %d platinum coins. Your share is %d platinum.\n\r",
	     amount_platinum-extra_platinum,share_platinum);
	send_to_char(buf,ch);
    }
    if (share_gold > 0)
    {
	sprintf(buf,
	    "You split %d gold coins. Your share is %d gold.\n\r",
	     amount_gold-extra_gold,share_gold);
	send_to_char(buf,ch);
    }
    if (share_silver > 0)
    {
	sprintf(buf,
	    "You split %d silver coins. Your share is %d silver.\n\r",
 	    amount_silver,share_silver + extra_silver);
	send_to_char(buf,ch);
    }

    if (share_gold == 0 && share_silver == 0)
    {
	sprintf(buf,"$n splits %d platinum coins. Your share is %d platinum.",
		amount_platinum-extra_platinum,share_platinum);
    }
    else if (share_platinum == 0 && share_silver == 0)
    {
	sprintf(buf,"$n splits %d gold coins. Your share is %d gold.",
		amount_gold-extra_gold,share_gold);
    }
    else if (share_platinum == 0 && share_gold == 0)
    {
	sprintf(buf,"$n splits %d silver coins. Your share is %d silver.",
		amount_silver,share_silver);
    }
    else if (share_silver == 0)
    {
	sprintf(buf,"$n splits %d platinum and %d gold coins. giving you %d platinum and %d gold.\n\r",
	 amount_platinum-extra_platinum, amount_gold-extra_gold,
	 share_platinum, share_gold);
    }
    else if (share_gold == 0)
    {
	sprintf(buf,"$n splits %d platinum and %d silver coins. giving you %d platinum and %d silver.\n\r",
	 amount_platinum-extra_platinum, amount_silver,
	 share_platinum, share_silver);
    }
    else if (share_platinum == 0)
    {
	sprintf(buf,"$n splits %d gold and %d silver coins. giving you %d gold and %d silver.\n\r",
	 amount_gold-extra_gold, amount_silver,
	 share_gold, share_silver);
    }
    else
    {
	sprintf(buf,"$n splits %d platinum, %d gold and %d silver coins. giving you %d platinum, %d gold and %d silver.\n\r",
	 amount_platinum-extra_platinum, amount_gold-extra_gold, amount_silver,
	 share_platinum, share_gold, share_silver);
    }

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
	{
	    act( buf, ch, NULL, gch, TO_VICT, POS_RESTING );
	    add_cost(gch,share_platinum,VALUE_PLATINUM);
	    add_cost(gch,share_gold,VALUE_GOLD);
	    add_cost(gch,share_silver,VALUE_SILVER);
	}
    }

    return;
}

void do_gtell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOTELL] != 0 )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
	return;

    if (IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    || IS_SET(ch->in_room->room_flags, ROOM_WAR))
    {
        send_to_char("You do not have access to this from within a blood cell.\n\r",ch);
        return;
    }

    if ((ch->in_room->vnum == ROOM_VNUM_CORNER) && (!IS_IMMORTAL(ch)))
    {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
    }

    argument = channel_parse(ch,argument,FALSE);

    sprintf( buf, "%s tells the group '{K%s{x'\n\r", ch->name, argument );
    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )
	    send_to_char( buf, gch );
    }

    return;
}

/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach == NULL || bch == NULL)
	return FALSE;

    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}

void do_colour( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_STRING_LENGTH ];
    int  ccolor;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	if ( !IS_SET( ch->act, PLR_COLOUR ) )
	{
	    SET_BIT( ch->act, PLR_COLOUR );
	    send_to_char( "{bC{ro{yl{co{mu{gr{x is now {rON{x, Way Cool!\n\r", ch );
	} else {
	    send_to_char_bw( "Colour is now OFF, <sigh>\n\r", ch );
	    REMOVE_BIT( ch->act, PLR_COLOUR );
	}
    }

    else if ( ch->pcdata == NULL )
	send_to_char( "Not for mobs...\n\r", ch );

    else if ( !str_prefix( arg, "list" ) )
    {
	send_to_char( "\n\rColors:\n\r",ch);
	send_to_char( "     0 - Reset           9 - Bright Red\n\r",ch);
	send_to_char( "     1 - Red            10 - Bright Green\n\r",ch);
	send_to_char( "     2 - Green          11 - Yellow\n\r",ch);
	send_to_char( "     3 - Brown          12 - Bright Blue\n\r",ch);
	send_to_char( "     4 - Blue           13 - Bright Magenta\n\r",ch);
	send_to_char( "     5 - Magenta        14 - Bright Cyan\n\r",ch);
	send_to_char( "     6 - Cyan           15 - Bright White\n\r",ch);
	send_to_char( "     7 - White          16 - Black\n\r",ch);
	send_to_char( "     8 - Grey           17 - None\n\r",ch);
	send_to_char( "Channels:\n\r",ch);
	send_to_char( "     auction    cgossip    clan\n\r",ch);
	send_to_char( "     gossip     grats      gtell\n\r",ch);
	send_to_char( "     immtalk    music      ask\n\r",ch);
	send_to_char( "     quote      say        shout\n\r",ch);
	send_to_char( "     tell       wiznet     mobsay\n\r",ch);
	send_to_char( "     room       condition  fight\n\r",ch);
	send_to_char( "     opponent   witness    disarm\n\r",ch);
	send_to_char( "     qgossip    ooc        race\n\r",ch);
 	send_to_char( "     flame      hero       chat\n\r",ch);
	send_to_char( "     olc1       olc2       olc3\n\r", ch );
	send_to_char( "For a more detailed list, see HELP COLORS\n\r",ch);
	send_to_char( "For a list of current settings, see HELP SETTINGS\n\r",ch);
    }

    else if ( is_number( arg ) && ( ccolor = atoi( arg ) ) >= 0 && ccolor <= 17 )
    {
        ch->pcdata->color = ccolor;
        send_to_char( "{xOK\n\r",ch); 
    } 

    else if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax: color {{list|#|<channel> #}\n\r", ch );
    }

    else
    {
	if ( ( ccolor = atoi( argument ) ) < 0 || ccolor >= 18 )
	    send_to_char( "Color number must be 0-17\n\r", ch );

	else if (!str_prefix(arg,"auction"))
	{
	    ch->pcdata->color_auc = ccolor;
	    send_to_char( "auction channel set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"chat"))
	{
	    ch->pcdata->color_cht = ccolor;
	    send_to_char( "Chat channel set.\n\r",ch);
	}
	else if (!str_prefix(arg,"cgossip"))
	{
	    ch->pcdata->color_cgo = ccolor;
	    send_to_char( "Clan gossip channel set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"cult") || !str_prefix(arg,"clan"))
	{
	    ch->pcdata->color_cla = ccolor;
	    send_to_char( "Clan chant channel set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"gossip"))
	{
	    ch->pcdata->color_gos = ccolor;
	    send_to_char( "gossip channel set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"grats"))
	{
	    ch->pcdata->color_gra = ccolor;
	    send_to_char( "grats channel set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"olc1"))
	{
	    ch->pcdata->color_olc1 = ccolor;
	    send_to_char( "OLC color set.\n\r", ch );
	}
	else if (!str_prefix(arg,"olc2"))
	{
	    ch->pcdata->color_olc2 = ccolor;
	    send_to_char( "OLC color set.\n\r", ch );
	}
	else if (!str_prefix(arg,"olc3"))
	{
	    ch->pcdata->color_olc3 = ccolor;
	    send_to_char( "OLC color set.\n\r", ch );
	}
	else if (!str_prefix(arg,"gtell"))
	{
	    ch->pcdata->color_gte = ccolor;
	    send_to_char( "group tell channel set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"immtalk"))
	{
	    ch->pcdata->color_imm = ccolor;
	    send_to_char( "immortal talk channel set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"ask"))
	{
	    ch->pcdata->color_que = ccolor;
	    send_to_char( "question/answer channel set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"quote"))
	{
	    ch->pcdata->color_quo = ccolor;
	    send_to_char( "quote channel set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"say"))
	{
	    ch->pcdata->color_say = ccolor;
	    send_to_char( "say channel set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"shout"))
	{
	    ch->pcdata->color_sho = ccolor;
	    send_to_char( "shout/yell channel set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"tell"))
	{
	    ch->pcdata->color_tel = ccolor;
	    send_to_char( "tell/reply channel set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"wiznet"))
	{
	    ch->pcdata->color_wiz = ccolor;
	    send_to_char( "wiznet channel set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"mobsay"))
	{
	    ch->pcdata->color_mob = ccolor;
	    send_to_char( "mobile talk channel set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"room"))
	{
	    ch->pcdata->color_roo = ccolor;
	    send_to_char( "room name display set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"condition"))
	{
	    ch->pcdata->color_con = ccolor;
	    send_to_char( "character condition display set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"fight"))
	{
	    ch->pcdata->color_fig = ccolor;
	    send_to_char( "your fight actions set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"opponent"))
	{
	    ch->pcdata->color_opp = ccolor;
	    send_to_char( "opponents fight actions set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"disarm"))
	{
	    ch->pcdata->color_dis = ccolor;
	    send_to_char( "disarm display set.\n\r",ch); 
	}
	else if (!str_prefix(arg,"witness"))
	{
	    ch->pcdata->color_wit = ccolor;
	    send_to_char( "witness fight actions set.\n\r",ch); 
	}
        else if (!str_prefix(arg,"qgossip"))
        {
            ch->pcdata->color_qgo = ccolor;
            send_to_char( "quest gossip channel set.\n\r",ch);
        }
        else if (!str_prefix(arg,"ooc"))
        {
            ch->pcdata->color_ooc = ccolor;
            send_to_char( "ooc channel set.\n\r",ch);
        }
        else if (!str_prefix(arg,"racetalk"))
	{
	    ch->pcdata->color_rac = ccolor;
	    send_to_char( "racetalk channel set.\n\r",ch);
	}
	else if (!str_prefix(arg,"flame"))
	{
	    ch->pcdata->color_fla = ccolor;
	    send_to_char( "flame channel set.\n\r",ch);
	}
	else if (!str_prefix(arg,"herotalk"))
	{
	    ch->pcdata->color_her = ccolor;
	    send_to_char( "Hero channel set.\n\r",ch);
	}
	else if (!str_prefix(arg,"ic"))
	{
	    ch->pcdata->color_ic = ccolor;
	    send_to_char( "IC channel set.\n\r",ch);
	}
	else if (!str_prefix(arg,"pray"))
	{
	    ch->pcdata->color_pra = ccolor;
	    send_to_char( "Pray channel set.\n\r",ch);
	}
	else
	{
	    send_to_char( "Syntax: color {{list|#|<channel> #}\n\r", ch );
	}
    }
}

void do_customwho( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch))
    {
	send_to_char("Mobiles can not use custom who.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	if (IS_SET(ch->act,PLR_CUSTOM_WHO))
	{
	    send_to_char("You will now see default who.\n\r",ch);
	    REMOVE_BIT(ch->act, PLR_CUSTOM_WHO);
	} else {
	    send_to_char("You will now see custom who (be sure to set a variable with \"customwho <string>\".\n\r",ch);
	    SET_BIT(ch->act, PLR_CUSTOM_WHO);
	}
	return;
    }

    if (!str_cmp(argument,"show"))
    {
	if (ch->pcdata->who_output[0] == '\0')
	{
	    send_to_char("You have no customwho settings.\n\r",ch);
	} else {
	    send_to_char("Your customwho setting is:\n\r\n\r",ch);
	    send_to_char_bw(ch->pcdata->who_output,ch);
	    send_to_char("\n\r\n\r",ch);
	    send_to_char(ch->pcdata->who_output,ch);
	    send_to_char("\n\r\n\r",ch);
	    send_to_char( display_who_custom( ch, ch ), ch );
	    send_to_char("\n\r",ch);
	}
	return;
    }
	
    smash_tilde(argument);
    free_string(ch->pcdata->who_output);
    ch->pcdata->who_output = str_dup(argument);
    send_to_char("Your custom who string is:",ch);
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);

    return;
}

void do_noexp( CHAR_DATA *ch, char *argument )
{
    if (IS_SET(ch->act,PLR_NOEXP))
    {
	send_to_char("You will now recieve experience bonuses.\n\r",ch);
	REMOVE_BIT(ch->act, PLR_NOEXP);
    } else {
	send_to_char("You will now recieve no experience for your deeds.\n\r",ch);
	SET_BIT(ch->act, PLR_NOEXP);
    }
}

void do_newbie( CHAR_DATA *ch, char *argument )
{
    if (!IS_SET(ch->comm, COMM_NONEWBIE))
    {
	send_to_char("Newbie info channel is now off.\n\r",ch);
	SET_BIT(ch->comm, COMM_NONEWBIE);
    } else {
	send_to_char("Newbie info channel is now on.\n\r",ch);
	REMOVE_BIT(ch->comm, COMM_NONEWBIE);
    }
    return;
}

char * channel_parse( CHAR_DATA *ch, char *string, bool censor )
{
    CENSOR_DATA *cen;
    char buf[2*MAX_INPUT_LENGTH];
    char temp;
    int pos = 0;
    int randomnum;
    extern CENSOR_DATA *censor_list;

    if ( !IS_IMMORTAL(ch) )
    {
	if ( mud_stat.colorlock || IS_SET(ch->act, PLR_CENSORED) )
	    string = strip_color( string );

	if ( mud_stat.capslock || IS_SET(ch->act, PLR_CENSORED) )
	    string = strip_caps( string );

	if ( censor )
	{
	    for ( cen = censor_list; cen != NULL; cen = cen->next )
		str_replace(string,cen->word,cen->replace);
	}
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 0 )
    {
	struct struckdrunk drunk[] =
	{
	    {3, 10,
		{"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"}},
	    {8, 5,
		{"b", "b", "b", "B", "B", "vb"}},
	    {3, 5,
		{"c", "c", "C", "cj", "sj", "zj"}},
	    {5, 2,
		{"d", "d", "D"}},
	    {3, 3,
		{"e", "e", "eh", "E"}},
	    {4, 5,
		{"f", "f", "ff", "fff", "fFf", "F"}},
	    {8, 2,
		{"g", "g", "G"}},
	    {9, 6,
		{"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}},
	    {7, 6,
		{"i", "i", "Iii", "ii", "iI", "Ii", "I"}},
	    {9, 5,
		{"j", "j", "jj", "Jj", "jJ", "J"}},
	    {7, 2,
		{"k", "k", "K"}},
	    {3, 2,
		{"l", "l", "L"}},
	    {5, 8,
		{"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"}},
	    {6, 6,
		{"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
	    {3, 6,
		{"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"}},
	    {3, 2,
		{"p", "p", "P"}},
	    {5, 5,
		{"q", "q", "Q", "ku", "ququ", "kukeleku"}},
	    {4, 2,
		{"r", "r", "R"}},
	    {2, 5,
		{"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"}},
	    {5, 2,
		{"t", "t", "T"}},
	    {3, 6,
		{"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"}},
	    {4, 2,
		{"v", "v", "V"}},
	    {4, 2,
		{"w", "w", "W"}},
	    {5, 6,
		{"x", "x", "X", "ks", "iks", "kz", "xz"}},
	    {3, 2,
		{"y", "y", "Y"}},
	    {2, 9,
		{"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}}
	};

	if ( ch->pcdata->condition[COND_DRUNK] > 0 )
	{
	    do
	    {
		temp = toupper (*string);
		if ((temp >= 'A') && (temp <= 'Z'))
		{
		    if (ch->pcdata->condition[COND_DRUNK] > drunk[temp - 'A'].min_drunk_level)
		    {
			randomnum = number_range (0, drunk[temp - 'A'].number_of_rep);
			strcpy (&buf[pos], drunk[temp - 'A'].replacement[randomnum]);
			pos += strlen (drunk[temp - 'A'].replacement[randomnum]);
		    }
		    else
			buf[pos++] = *string;
		} else {
		    if ((temp >= '0') && (temp <= '9'))
		    {
			temp = '0' + number_range (0, 9);
			buf[pos++] = temp;
		    }
		    else
			buf[pos++] = *string;
		}
	    }
	    while (*string++);
	    buf[pos] = '\0';
	    strcpy(string, buf);
	}
    }
    return (string);
}

void do_info( CHAR_DATA *ch, char *argument )
{
   int flag;
   int col = 0;
   char buf[MAX_STRING_LENGTH];

   if ( argument[0] == '\0' )
   {
	send_to_char("Option         Status\n\r",ch);
	send_to_char("---------------------\n\r",ch);
	buf[0] = '\0';

	for (flag = 0; info_table[flag].name != NULL; flag++)
	{
	    sprintf( buf, "{w%-14s %s\t", info_table[flag].name,
	    !IS_SET(ch->info,info_table[flag].flag) ? "{RON" : "{GOFF" );
	    send_to_char(buf, ch);
	    col++;

	    if (col == 3)
	    {
		send_to_char("\n\r",ch);
		col = 0;
	    }
	}
	send_to_char("{x\n\r",ch);
	return;
    }

    if (!str_prefix(argument,"on"))
    {
	send_to_char("Information channel activated!\n\r",ch);
	REMOVE_BIT(ch->info,INFO_ON);
	return;
    }

    if (!str_prefix(argument,"off"))
    {
	send_to_char("Information channel de-activated.\n\r",ch);
	SET_BIT(ch->info,INFO_ON);
	return;
    }

    flag = info_lookup(argument);

    if (flag == -1)
    {
	send_to_char("{RNo such option.{x\n\r",ch);
	return;
    }

    if (!IS_SET(ch->info,info_table[flag].flag))
    {
	sprintf(buf,"{wYou will no longer see {R%s {won information.{x\n\r",
	    info_table[flag].name);
	send_to_char(buf,ch);
	SET_BIT(ch->info,info_table[flag].flag);
	return;
    } else {
	sprintf(buf,"{wYou will now see {R%s {won information.{x\n\r",
	    info_table[flag].name);
	send_to_char(buf,ch);
	REMOVE_BIT(ch->info,info_table[flag].flag);
	return;
    }
    return;
}

void info( char *string, CHAR_DATA *ch, CHAR_DATA *victim, long flag )
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   (!flag || (!IS_SET(d->character->info,flag)
		    &&  !IS_SET(d->character->info,INFO_ON)))
	&&   d->character != ch
	&&   d->character != victim )
	{
	    act( string, d->character, NULL, ch, TO_CHAR, POS_DEAD );
	}
    }
    return;
}

void do_combat( CHAR_DATA *ch, char *argument )
{
   int flag;
   int col = 0;
   char buf[MAX_STRING_LENGTH];

   if ( argument[0] == '\0' )
   {
	send_to_char("Option         Status\n\r",ch);
	send_to_char("---------------------\n\r",ch);
	buf[0] = '\0';

	for (flag = 0; combat_table[flag].name != NULL; flag++)
	{
	    sprintf( buf, "{w%-14s %s\t", combat_table[flag].name,
	    !IS_SET(ch->combat,combat_table[flag].flag) ? "{RON" : "{GOFF" );
	    send_to_char(buf, ch);
	    col++;

	    if (col == 3)
	    {
		send_to_char("\n\r",ch);
		col = 0;
	    }
	}
	send_to_char("{x\n\r",ch);
	return;
    }

    if (!str_prefix(argument,"on"))
    {
	send_to_char("Combat information activated!\n\r",ch);
	REMOVE_BIT(ch->combat,COMBAT_ON);
	return;
    }

    if (!str_prefix(argument,"off"))
    {
	send_to_char("All non-essential combat messages disabled.\n\r",ch);
	SET_BIT(ch->combat,COMBAT_ON);
	return;
    }

    flag = combat_lookup(argument);

    if (flag == -1)
    {
	send_to_char("{RNo such option.{x\n\r",ch);
	return;
    }

    if (!IS_SET(ch->combat,combat_table[flag].flag))
    {
	sprintf(buf,"{wYou will no longer see {R%s {wduring combat.{x\n\r",
	    combat_table[flag].name);
	send_to_char(buf,ch);
	SET_BIT(ch->combat,combat_table[flag].flag);
    } else {
	sprintf(buf,"{wYou will now see {R%s {wduring combat.{x\n\r",
	    combat_table[flag].name);
	send_to_char(buf,ch);
	REMOVE_BIT(ch->combat,combat_table[flag].flag);
    }
    return;
}

void do_chat( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	send_to_char("This channel is reserved for players.\n\r",ch);
	return;
    }

    if ( argument[0] == '\0' )
    {
	sprintf(buf, "Your current chat channel is %d.\n\r"
		     "To disable this channel set your chat to 0.\n\r",
	    ch->pcdata->chat_chan);
	send_to_char(buf,ch);
	return;
    }

    if ( is_number(argument) )
    {
	int value = atoi(argument);

	if ( value < 0 || value > 50000 )
	{
	    send_to_char("Valid channels are 0 through 50,000.\n\r",ch);
	    return;
	}

	sprintf(buf, "Your chat channel has been changed from %d to %d.\n\r",
	    ch->pcdata->chat_chan, value);
	send_to_char(buf,ch);
	ch->pcdata->chat_chan = value;
	return;
    }

    if ( !str_cmp(argument,"who") )
    {
	int match = 0;

	sprintf(buf,"{dCurrent players using chat channel {x[{d%5d{x]:{d\n\r",
	    ch->pcdata->chat_chan);
	send_to_char(buf,ch);

	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != NULL
	    &&   !IS_NPC(d->character)
	    &&   can_see(ch,d->character)
	    &&   d->character->pcdata->chat_chan == ch->pcdata->chat_chan )
	    {
		match++;
		sprintf(buf, "%-15s", d->character->name);
		send_to_char(buf,ch);
	    }

	    if ( match == 3 )
	    {
		send_to_char("\n\r",ch);
		match = 0;
	    }
	}
	send_to_char("{x\n\r",ch);
	return;
    }

    if ( IS_IMMORTAL(ch) && !str_cmp(argument,"all") )
    {
	int match = 0;

	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != NULL
	    &&   !IS_NPC(d->character)
	    &&   can_see(ch,d->character) )
	    {
		match++;
		sprintf(buf, "[{c%5d{x]{C%-15s{x",
		    d->character->pcdata->chat_chan, d->character->name);
		send_to_char(buf,ch);
	    }

	    if ( match == 3 )
	    {
		send_to_char("\n\r",ch);
		match = 0;
	    }
	}
	send_to_char("\n\r",ch);
	return;
    }

    if ( ch->pcdata->chat_chan == 0 )
    {
	send_to_char("You must first pick a chat channel!\n\r",ch);
	return;
    }

    if ( IS_SET(ch->comm,COMM_QUIET) )
    {
	send_to_char("You must turn off quiet mode first.\n\r",ch);
	return;
    }

    if ( (IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    ||   IS_SET(ch->in_room->room_flags, ROOM_WAR))
    &&   !IS_IMMORTAL(ch) )
    {
	send_to_char("You do not have access to this from within a bloody cell.\n\r",ch);
	return;
    }

    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
    {
	send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	return;
    }

    if ( ch->in_room->vnum == ROOM_VNUM_CORNER && !IS_IMMORTAL(ch) )
    {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
    }

    argument = channel_parse(ch,argument,FALSE);
    
    sprintf(buf, "You chat <%d> '{d%s{x'\n\r", ch->pcdata->chat_chan, argument);
    send_to_char(buf,ch);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   !IS_SET(victim->comm,COMM_QUIET)
	&&   (victim->pcdata->chat_chan == ch->pcdata->chat_chan
	||   IS_SET(victim->wiznet,WIZ_CHATS))
	&&   !check_forget( victim, ch ) )
	{
	    sprintf( buf, "%s$n{x chats <%d> '{d$t{x'", pretitle(ch,d->character),
		ch->pcdata->chat_chan);
	    act( buf,ch,argument,d->character,TO_VICT,POS_DEAD);
	}
    }

    return;
}

signed char lookup_alias( PC_DATA *pc, char *name )
{
    unsigned char i, match = 0;

    for (i = 0; i < MAX_ALIAS; i++)
    {
	if (!pc->alias[i])
	    continue;
	if (!str_prefix(pc->alias[i], name))
	{
	    if (!str_cmp(pc->alias[i], name))
	    {
		match = 1;
		break;
	    }
	}
    }

    return match ? i : -1;
}

char *mytok( char *first, const char *sep, char **rest )
{
    short int i, len;
    char *str, *str_o, *out;
    char *r;

    i = len = 0;

    if (first)
    {
	*rest = NULL;
	len = strlen(first);
	str = malloc(len + 1);
	strcpy(str, first);
    }
    else if (!*rest)
	return NULL;
    else
    {
	len = strlen(*rest);
	str = malloc(len + 1);
	strcpy(str, *rest);
	free(*rest);
	*rest = NULL;
    }
    out = malloc(len + 1);
    *out = 0;
    str_o = str;

    if (!*sep)
    {
	strcpy(out, str);
	free(str_o);
	return out;
    }

    for (; strchr(sep, *str) && *str; str++);
    
    if (!*str)
    {
	free(str_o);
	free(out);
	return NULL;
    }

    for (i = 0; str[i] && !strchr(sep, str[i]); i++);
    strncpy(out, str, i);
    *(out + i) = 0;

    str = str + i;
    for (; strchr(sep, *str) && *str; str++);

    if (*str)
    {
	r = malloc(strlen(str) + 1);
	strcpy(r, str);
	*rest = r;
    }

    free(str_o);
    return out;
}

void extract_alias( char *out, char *body, char *args )
{
    int i = 0, num;
    char *list[A_MAX_ARGS];
    char tmp[4*MAX_INPUT_LENGTH];
    const char *sep = " \n";
    char *rest;

    *out = 0;
    for (list[i] = mytok(args, sep, &rest);
       i < A_MAX_ARGS;
       list[i++] = mytok(args, sep, &rest));

    for (i = 0; i < strlen(body); i++)
    {
	if (body[i] == '$')
	{
	    i++;
	    if (!isdigit(body[i]))
	    {
		if (body[i] == '*')
		{
		    strcat(out, args);
		    continue;
		}
		else if (body[i] == '$')
		{
		    strcat(out, "$");
		    continue;
		}
		else
		    continue;
	    }
	    tmp[0] = body[i];
	    tmp[1] = 0;
	    num = atoi(tmp);
	    if (num > A_MAX_ARGS || num < 1)
		continue;
	    num--;
	    if (!list[num])
		strcpy(tmp, "");
	    else
		strcpy(tmp, list[num]);
	    strcat(out, tmp);
	} else {
	    strncat(out, &body[i], 1); 
	}
    }
    free(rest);
    for (i = 0; i < A_MAX_ARGS; i++)
	free(list[i]);
}

void do_alia( CHAR_DATA *ch, char *argument )
{
    send_to_char( "I'm sorry, alias must be entered in full.\n\r", ch );
    return;
}

void do_alias( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    sh_int pos;

    if ( ch->desc == NULL )
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;

    if ( IS_NPC( rch ) )
	return;

    smash_tilde( argument );

    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	BUFFER *final;

	if ( rch->pcdata->alias[0] == NULL )
	{
	    send_to_char( "You have no aliases defined.\n\r", ch );
	    return;
	}

	final = new_buf( );

	add_buf( final, "{s ============================================================================= \n\r"
			"{s| {tNum {s|      {qName {s| {tCommand                                                   {s|\n\r"
			"{s|-----------------------------------------------------------------------------|\n\r" );

	for ( pos = 0; pos < MAX_ALIAS; pos++ )
	{
	    if ( rch->pcdata->alias[pos] == NULL
	    ||	 rch->pcdata->alias_sub[pos] == NULL )
		break;

	    sprintf( buf, "{s| {t%3d {s|{q%s {s| {t%s{s|\n\r",
		pos+1, begin_string( rch->pcdata->alias[pos], 10 ),
		end_string( rch->pcdata->alias_sub[pos], 58 ) );
	    add_buf( final, buf );
	}

	add_buf( final, "{s ============================================================================={x \n\r" );
	page_to_char( final->string, ch );
	free_buf( final );

	return;
    }

    if ( !str_prefix( "una", arg ) || !str_cmp( "alias", arg ) )
    {
	send_to_char( "Sorry, that word is reserved.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	for ( pos = 0; pos < MAX_ALIAS; pos++ )
	{
	    if ( rch->pcdata->alias[pos] == NULL
	    ||	 rch->pcdata->alias_sub[pos] == NULL )
		break;

	    if ( !str_cmp( arg, rch->pcdata->alias[pos] ) )
	    {
		sprintf( buf, "%s aliases to '%s'.\n\r",
		    rch->pcdata->alias[pos], rch->pcdata->alias_sub[pos] );
		send_to_char( buf, ch );
		return;
	    }
	}

	send_to_char( "That alias is not defined.\n\r", ch );
	return;
    }

    if ( !str_prefix( argument, "delete") || !str_prefix( argument, "prefix" ) )
    {
	send_to_char( "That shall not be done!\n\r", ch );
	return;
    }

    for ( pos = 0; pos < MAX_ALIAS; pos++ )
    {
	if ( rch->pcdata->alias[pos] == NULL )
	    break;

	if ( !str_cmp( arg, rch->pcdata->alias[pos] ) )
	{
	    free_string( rch->pcdata->alias_sub[pos] );
	    rch->pcdata->alias_sub[pos] = str_dup( argument );
	    sprintf( buf, "%s is now realiased to '%s'.\n\r", arg, argument );
	    send_to_char( buf, ch );
	    return;
	}
    }

     if ( pos >= MAX_ALIAS )
     {
	send_to_char( "Sorry, you have reached the alias limit.\n\r", ch );
	return;
     }
  
     rch->pcdata->alias[pos]		= str_dup( arg );
     rch->pcdata->alias_sub[pos]	= str_dup( argument );
     sprintf( buf, "%s is now aliased to '%s'.\n\r", arg, argument );
     send_to_char( buf, ch );
}

void do_unalias( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    bool found = FALSE;
    sh_int pos;
 
    if ( ch->desc == NULL )
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;
 
    if ( IS_NPC( rch ) )
	return;
 
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unalias what?\n\r", ch );
	return;
    }

    for ( pos = 0; pos < MAX_ALIAS; pos++ )
    {
	if ( rch->pcdata->alias[pos] == NULL )
	    break;

	if ( found )
	{
	    rch->pcdata->alias[pos-1]		= rch->pcdata->alias[pos];
	    rch->pcdata->alias_sub[pos-1]	= rch->pcdata->alias_sub[pos];
	    rch->pcdata->alias[pos]		= NULL;
	    rch->pcdata->alias_sub[pos]		= NULL;
	    continue;
	}

	if ( !strcmp( arg, rch->pcdata->alias[pos] ) )
	{
	    send_to_char( "Alias removed.\n\r", ch );
	    free_string( rch->pcdata->alias[pos] );
	    free_string( rch->pcdata->alias_sub[pos] );
	    rch->pcdata->alias[pos] = NULL;
	    rch->pcdata->alias_sub[pos] = NULL;
	    found = TRUE;
	}
    }

    if ( !found )
	send_to_char( "No alias of that name to remove.\n\r", ch );
}

void smash_extra_seperator( CHAR_DATA *ch, char *str )
{
    sh_int count = 0;

    for ( ; *str != '\0'; str++ )
    {
	if ( *str == ';' )
	{
	    if ( ++count > 5 )
	    {
		send_to_char("{RMax commands: 5, Passing extra seperators.{x\n\r\n\r",ch);
		*str = '\0';
	    }
	}
    }

    return;
}

void substitute_alias( DESCRIPTOR_DATA *d, char *argument )
{
    CHAR_DATA *ch;
    char outcmd[4*MAX_INPUT_LENGTH];
    char a_name[4*MAX_INPUT_LENGTH];
    char a_body[4*MAX_INPUT_LENGTH];
    char incomm[4*MAX_INPUT_LENGTH];
    char *a_part;
    static char *a;
    int pos;
    const char *sep = ";";

    ch = d->original ? d->original : d->character;

    if ( ch->pcdata && ch->pcdata->prefix[0] != '\0'
    &&   str_prefix( "prefix", argument ) )
    {
	if ( strlen( ch->pcdata->prefix ) + strlen( argument ) > 3*MAX_INPUT_LENGTH )
	    send_to_char( "Line to long, prefix not processed.\r\n", ch );
	else
	{
	    sprintf( incomm, "%s %s",ch->pcdata->prefix, argument );
	    argument = incomm;
	}
    }

    if (IS_NPC(ch) || ch->pcdata->alias[0] == NULL
    ||	!str_prefix("alias",argument) || !str_prefix("una",argument) 
    ||  !str_prefix("prefix",argument) || d->connected != CON_PLAYING) 
    {
	interpret(d->character,argument);
	return;
    }

    strcpy(incomm, argument);
    strcpy(outcmd, argument);
    a = one_argument(incomm, a_name);
    strcpy(incomm, a);

    if (d->a_cur[0] == '\0')
    {
	if ((pos = lookup_alias(ch->pcdata, a_name)) >= 0)
	{
	    strcpy(a_body, ch->pcdata->alias_sub[pos]);
	    smash_extra_seperator( ch, a_body );
	    a_part = mytok(a_body, sep, &a);
	    if (!a)
	  	strcpy(d->a_cur, "");
	    else
		strcpy(d->a_cur, a);
	} else {
	    interpret( d->character, outcmd );
	    return;
	}
    } else {
	strcpy(a_body, d->a_cur);
	a_part = mytok(a_body, sep, &a);
	if (!a)
	    strcpy(d->a_cur, "");
	else
	    strcpy(d->a_cur, a);
    }

    if (strlen(outcmd) > MAX_INPUT_LENGTH - 2)
    {
	send_to_desc("Alias extraction too long.\n\r",d);
	close_socket(d);
	return;
    }

    /* a_part = command_name; incomm = command_parameters; */
    extract_alias(outcmd, a_part, incomm);

    interpret( d->character, outcmd );
    free(a_part);
    free(a);
    return;
}

void do_forget(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int pos;
    bool found = FALSE;

    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC(rch))
	return;

    smash_tilde( argument );

    argument = one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
	if (rch->pcdata->forget[0] == NULL)
	{
	    send_to_char("You are not forgetting anyone.\n\r",ch);
	    return;
	}
	send_to_char("You are currently forgetting:\n\r",ch);

	for (pos = 0; pos < MAX_FORGET; pos++)
	{
	    if (rch->pcdata->forget[pos] == NULL)
		break;

	    sprintf(buf,"    %s\n\r",rch->pcdata->forget[pos]);
	    send_to_char(buf,ch);
	}
	return;
    }

    for (pos = 0; pos < MAX_FORGET; pos++)
    {
	if (rch->pcdata->forget[pos] == NULL)
	    break;

	if (!str_cmp(arg,rch->pcdata->forget[pos]))
	{
	    send_to_char("You have already forgotten that person.\n\r",ch);
	    return;
	}
    }

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;

 	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;
	
	wch = ( d->original != NULL ) ? d->original : d->character;

 	if (!can_see(ch,wch))
	    continue;

	if (!str_cmp(arg,wch->name))
	{
	    found = TRUE;
	    if (wch == ch)
	    {
		send_to_char("You forget yourself for a moment, but it passes.\n\r",ch);
		return;
	    }
	    if (wch->level >= LEVEL_IMMORTAL)
	    {
		send_to_char("That person is very hard to forget.\n\r",ch);
		return;
	    }
	}
    }

    if (!found)
    {
	send_to_char("No one by that name is playing.\n\r",ch);
	return;
    }

    for (pos = 0; pos < MAX_FORGET; pos++)
    {
	if (rch->pcdata->forget[pos] == NULL)
	    break;
     }

     if (pos >= MAX_FORGET)
     {
	send_to_char("Sorry, you have reached the forget limit.\n\r",ch);
	return;
     }
  
     /* make a new forget */
     rch->pcdata->forget[pos]		= str_dup(arg);
     sprintf(buf,"You are now deaf to %s.\n\r",capitalize(arg));
     send_to_char(buf,ch);
}

void do_remembe(CHAR_DATA *ch, char *argument)
{
    send_to_char("I'm sorry, remember must be entered in full.\n\r",ch);
    return;
}

void do_remember(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    int pos;
    bool found = FALSE;
 
    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;
 
    if (IS_NPC(rch))
	return;
 
    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	if (rch->pcdata->forget[0] == NULL)
	{
	    send_to_char("You are not forgetting anyone.\n\r",ch);
	    return;
	}
	send_to_char("You are currently forgetting:\n\r",ch);

	for (pos = 0; pos < MAX_FORGET; pos++)
	{
	    if (rch->pcdata->forget[pos] == NULL)
		break;

	    sprintf(buf,"    %s\n\r",rch->pcdata->forget[pos]);
	    send_to_char(buf,ch);
	}
	return;
    }

    for (pos = 0; pos < MAX_FORGET; pos++)
    {
	if (rch->pcdata->forget[pos] == NULL)
	    break;

	if (found)
	{
	    rch->pcdata->forget[pos-1]		= rch->pcdata->forget[pos];
	    rch->pcdata->forget[pos]		= NULL;
	    continue;
	}

	if(!strcmp(arg,rch->pcdata->forget[pos]))
	{
	    send_to_char("Forget removed.\n\r",ch);
	    free_string(rch->pcdata->forget[pos]);
	    rch->pcdata->forget[pos] = NULL;
	    found = TRUE;
	}
    }

    if (!found)
	send_to_char("No one by that name is forgotten.\n\r",ch);
}

void do_racial( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r  racial refresh\n\r  racial discharge\n\r", ch );
	return;
    }

    if ( race_table[ch->race].aff == 0 && race_table[ch->race].shd == 0 )
	send_to_char( "Your race has no spells.\n\r", ch );

    else if ( !str_prefix( argument, "discharge" ) )
    {
	send_to_char( "You discharge your racial abilities.\n\r", ch );
	racial_spells( ch, FALSE );
    }

    else if ( !str_prefix( argument, "refresh" ) )
    {
	send_to_char( "You refresh your racial abilities.\n\r", ch );
	racial_spells( ch, TRUE );
    }

    else
	do_racial( ch, "" );
}

void do_sound( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] != '\0' )
    {
	if ( !str_cmp( argument, "on" ) )
	{
	    SET_BIT( ch->sound, SOUND_ON );
	    send_to_char( "Sounds are now turned on.\n\r", ch );
	    send_to_char( "For your client to play sound it must be configured to run MSP.\n\r", ch );
	}

	else if ( !str_cmp( argument, "off" ) )
	{
	    REMOVE_BIT( ch->sound, SOUND_ON );
	    send_to_char( "\n\r!!SOUND(Off)\n\r!!MUSIC(Off)\n\r", ch );
	    send_to_char( "{BAll MUD sounds have been turned off.{x\n\r", ch );
	}

	else if ( !str_prefix( argument, "music" )
	     ||   !str_prefix( argument, "background" ) )
	{
	    if ( IS_SET( ch->sound, SOUND_NOMUSIC ) )
	    {
		REMOVE_BIT( ch->sound, SOUND_NOMUSIC );
		send_to_char( "Background music enabled.\n\r", ch );
	    } else {
		SET_BIT( ch->sound, SOUND_NOMUSIC );
		send_to_char( "\n\r!!MUSIC(Off)\n\rBackground music disabled.", ch );
	    }
	}

	else if ( !str_prefix( argument, "combat" ) )
	{
	    if ( IS_SET( ch->sound, SOUND_NOCOMBAT ) )
	    {
		REMOVE_BIT( ch->sound, SOUND_NOCOMBAT );
		send_to_char( "Combat sounds enabled.\n\r", ch );
	    } else {
		SET_BIT( ch->sound, SOUND_NOCOMBAT );
		send_to_char( "Combat sounds disabled.\n\r", ch );
	    }
	}

	else if ( !str_prefix( argument, "weather" ) )
	{
	    if ( IS_SET( ch->sound, SOUND_NOWEATHER ) )
	    {
		REMOVE_BIT( ch->sound, SOUND_NOWEATHER );
		send_to_char( "Weather sounds disabled.\n\r", ch );
	    } else {
		SET_BIT( ch->sound, SOUND_NOWEATHER );
		send_to_char( "Weather sounds enabled.\n\r", ch );
	    }
	}

	else if ( !str_prefix( argument, "skills" ) )
	{
	    if ( IS_SET( ch->sound, SOUND_NOSKILL ) )
	    {
		REMOVE_BIT( ch->sound, SOUND_NOSKILL );
		send_to_char( "Skill sounds enabled.\n\r", ch );
	    } else {
		SET_BIT( ch->sound, SOUND_NOSKILL );
		send_to_char( "Skill sounds disabled.\n\r", ch );
	    }
	}

	else if ( !str_prefix( argument, "clans" ) )
	{
	    if ( IS_SET( ch->sound, SOUND_NOCLAN ) )
	    {
		REMOVE_BIT( ch->sound, SOUND_NOCLAN );
		send_to_char( "Clan sounds enabled.\n\r", ch );
	    } else {
		SET_BIT( ch->sound, SOUND_NOCLAN );
		send_to_char( "Clan sounds disabled.\n\r", ch );
	    }
	}

	else if ( !str_prefix( argument, "areas" ) )
	{
	    if ( IS_SET( ch->sound, SOUND_NOZONE ) )
	    {
		REMOVE_BIT( ch->sound, SOUND_NOZONE );
		send_to_char( "Area sounds enabled.\n\r", ch );
	    } else {
		SET_BIT( ch->sound, SOUND_NOZONE );
		send_to_char( "Area sounds disabled.\n\r", ch );
	    }
	}

	else if ( !str_prefix( argument, "miscellaneous" ) )
	{
	    if ( IS_SET( ch->sound, SOUND_NOMISC ) )
	    {
		REMOVE_BIT( ch->sound, SOUND_NOMISC );
		send_to_char( "Miscellaneous sounds enabled.\n\r", ch );
	    } else {
		SET_BIT( ch->sound, SOUND_NOMISC );
		send_to_char( "Miscellaneous sounds disabled.\n\r", ch );
	    }
	}

	return;
    }

    send_to_char( "{BSounds          Status{x\n\r", ch );
    send_to_char( "{b=--------------------={x\n\r", ch );

    send_to_char( "Sound             ", ch );
    if ( IS_SET( ch->sound, SOUND_ON ) )
	send_to_char( "{RON{x\n\r", ch );
    else
	send_to_char( "{gOFF{x\n\r", ch );

    send_to_char( "Background Music  ", ch );
    if ( !IS_SET( ch->sound, SOUND_NOMUSIC ) )
	send_to_char( "{RON{x\n\r", ch );
    else
	send_to_char( "{gOFF{x\n\r", ch );

    send_to_char( "Combat            ", ch );
    if ( !IS_SET( ch->sound, SOUND_NOCOMBAT ) )
	send_to_char( "{RON{x\n\r", ch );
    else
	send_to_char( "{gOFF{x\n\r", ch );

    send_to_char( "Weather           ", ch );
    if ( !IS_SET( ch->sound, SOUND_NOWEATHER ) )
	send_to_char( "{RON{x\n\r", ch );
    else
	send_to_char( "{gOFF{x\n\r", ch );

    send_to_char( "Skills            ", ch );
    if ( !IS_SET( ch->sound, SOUND_NOSKILL ) )
	send_to_char( "{RON{x\n\r", ch );
    else
	send_to_char( "{gOFF{x\n\r", ch );

    send_to_char( "Clans             ", ch );
    if ( !IS_SET( ch->sound, SOUND_NOCLAN ) )
	send_to_char( "{RON{x\n\r", ch );
    else
	send_to_char( "{gOFF{x\n\r", ch );

    send_to_char( "Zones             ", ch );
    if ( !IS_SET( ch->sound, SOUND_NOZONE ) )
	send_to_char( "{RON{x\n\r", ch );
    else
	send_to_char( "{gOFF{x\n\r", ch );

    send_to_char( "Miscellaneous     ", ch );
    if ( !IS_SET( ch->sound, SOUND_NOMISC ) )
	send_to_char( "{RON{x\n\r", ch );
    else
	send_to_char( "{gOFF{x\n\r", ch );
}

POLL_DATA *poll_lookup( char *argument )
{
    POLL_DATA *poll;

    for ( poll = first_poll; poll != NULL; poll = poll->next )
    {
	if ( !str_prefix( argument, poll->name ) )
	    return poll;
    }

    return NULL;
}

sh_int get_poll_response( POLL_DATA *poll, char *argument )
{
    sh_int pos;

    for ( pos = 0; pos < MAX_POLL_RESPONSES; pos++ )
    {
	if ( poll->response[pos] == NULL )
	    return -1;

	if ( !str_prefix( argument, poll->response[pos] ) )
	    return pos;
    }

    return -1;
}

void do_vote( CHAR_DATA *ch, char *argument )
{
    POLL_DATA *poll;
    VOTE_DATA *vote;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    sh_int pos;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Mobiles don't deserve opinions!\n\r", ch );
	return;
    }

    if ( arg1[0] == '\0' )
    {
	send_to_char(	"Syntax: vote list                         (list all polls)\n"
			"        vote results                      (show tallies)\n"
			"        vote <poll> info                  (get information)\n"
			"        vote <poll> <your response>       (vote)\n\r", ch);
	if ( IS_IMMORTAL(ch) )
	{
	   send_to_char("\n        vote create <poll>                (create a new poll)\n"
			"        vote <poll> <A0-9> <string>       (assign an answer)\n"
			"        vote <poll> question <string>     (assign the question)\n"
			"        vote <poll> results               (detailed results)\n"
			"        vote <poll> delete                (delete a poll)\n", ch);

	}
	return;
    }

    if ( !str_prefix(arg1, "list") )
    {
	if ( first_poll == NULL )
	{
	    send_to_char("No current polls exist.\n\r",ch);
	    return;
	}

	send_to_char( "Current Polls:\n-------------------------------------------------\n\r",ch);
	for ( poll = first_poll; poll != NULL; poll = poll->next )
	{
	    sprintf( buf, " %10s | %s\n",
		capitalize(poll->name), poll->question );
	    send_to_char( buf, ch );
	}

	return;
    }

    if ( !str_prefix(arg1, "results") )
    {
	sh_int votes[MAX_POLL_RESPONSES];

	if ( first_poll == NULL )
	{
	    send_to_char("No current polls exist.\n\r",ch);
	    return;
	}

	send_to_char( "Current Poll Tallies:\n-------------------------------------------------\n\r",ch);

	for ( poll = first_poll; poll != NULL; poll = poll->next )
	{
	    for ( pos = 0; pos < MAX_POLL_RESPONSES; pos++ )
		votes[pos] = 0;

	    for ( vote = poll->vote; vote != NULL; vote = vote->next_vote )
		votes[vote->voter_vote]++;

	    sprintf( buf, "\n%s\n----------------------------------------------\n",
		poll->question );
	    send_to_char( buf, ch );

	    for ( pos = 0; pos < MAX_POLL_RESPONSES; pos++ )
	    {
		if ( poll->response[pos] == NULL )
		    break;

		sprintf( buf, " [%3d Votes] %2d) %s\n",
		    votes[pos], pos, poll->response[pos] );
		send_to_char( buf, ch );
	    }
	}

	return;
    }

    if ( !str_prefix(arg1, "create") && IS_IMMORTAL(ch) )
    {
	if ( arg2[0] == '\0' )
	{
	    send_to_char("Vote create <name>.\n\r",ch);
	    return;
	}

	poll		= new_poll();
	poll->name	= str_dup( arg2 );
	poll->next	= first_poll;
	first_poll	= poll;

	save_voting_polls();

	send_to_char("New poll created.\n\r",ch);
	return;
    }

    if ( (poll = poll_lookup(arg1)) == NULL )
    {
	send_to_char("That poll doesn't appear to be available.\n\r",ch);
	return;
    }

    if ( arg2[0] == '\0' || !str_prefix(arg2, "info") )
    {
	sprintf( buf, "Poll:     %s\n", capitalize(poll->name) );
	send_to_char(buf,ch);

	sprintf( buf, "Question: %s\n", poll->question );
	send_to_char(buf,ch);

	for ( pos = 0; pos < MAX_POLL_RESPONSES; pos++ )
	{
	    if ( poll->response[pos] == NULL )
		break;

	    sprintf( buf, " %2d) %s\n", pos, poll->response[pos] );
	    send_to_char( buf, ch );
	}

	return;
    }

    if ( IS_IMMORTAL(ch) )
    {
	if ( !str_cmp(arg2, "delete") )
	{
	    if ( poll == first_poll )
		first_poll = poll->next;
	    else
	    {
		POLL_DATA *prev;

		for ( prev = first_poll; prev != NULL; prev = prev->next )
		{
		    if ( prev->next == poll )
		    {
			prev->next = poll->next;
			break;
		    }
		}
	    }

	    sprintf( buf, "../data/voting_polls/%s", poll->name );
	    unlink ( buf );

	    send_to_char("Poll deleted.\n\r",ch);
	    free_poll( poll );
	    save_voting_polls();
	    return;
	}

	if ( !str_prefix( arg2, "question" ) )
	{
	    if ( argument[0] == '\0' )
	    {
		send_to_char("vote <poll> question <new string>.\n\r",ch);
		return;
	    }

	    send_to_char("Question modified.\n\r",ch);
	    free_string( poll->question );
	    poll->question = str_dup( argument );
	    save_voting_polls();
	    return;
	}

	if ( !str_cmp(arg2, "A0") )
	    pos = 0;
	else if ( !str_cmp(arg2, "A1") )
	    pos = 1;
	else if ( !str_cmp(arg2, "A2") )
	    pos = 2;
	else if ( !str_cmp(arg2, "A3") )
	    pos = 3;
	else if ( !str_cmp(arg2, "A4") )
	    pos = 4;
	else if ( !str_cmp(arg2, "A5") )
	    pos = 5;
	else if ( !str_cmp(arg2, "A6") )
	    pos = 6;
	else if ( !str_cmp(arg2, "A7") )
	    pos = 7;
	else if ( !str_cmp(arg2, "A8") )
	    pos = 8;
	else if ( !str_cmp(arg2, "A9") )
	    pos = 9;
	else
	    pos = -1;

	if ( argument[0] != '\0' && pos != -1 )
	{
	    free_string( poll->response[pos] );
	    poll->response[pos] = str_dup( argument );
	    save_voting_polls();
	    send_to_char("Voting answer changed.\n\r",ch);
	    return;
	}

	if ( !str_prefix( arg2, "results" ) )
	{
	    sprintf( buf, "Results for %s poll:\n", poll->name );
	    send_to_char( buf, ch );

	    for ( vote = poll->vote; vote != NULL; vote = vote->next_vote )
	    {
		sprintf( buf, "%2d %-10s %s\n",
		    vote->voter_vote, vote->voter_name,
		    IS_TRUSTED( ch, MAX_LEVEL-1 ) ? vote->voter_ip : "" );
		send_to_char( buf, ch );
	    }

	    return;
	}
    }

    if ( ch->pcdata->tier <= 0 )
    {
	send_to_char("You must be at least second tier to vote.\n\r",ch);
	return;
    }

    if ( !is_number(arg2) )
	pos = get_poll_response(poll,arg2);
    else
	pos = atoi(arg2);

    if ( pos < 0 || pos >= MAX_POLL_RESPONSES || poll->response[pos] == NULL )
    {
	send_to_char("That is not a valid voting response.\n\r",ch);
	return;
    }

    for ( vote = poll->vote; vote != NULL; vote = vote->next_vote )
    {
	if ( !str_cmp( vote->voter_name, ch->name )
	||   !str_cmp( vote->voter_ip, ch->pcdata->socket ) )
	{
	    if ( vote->voter_vote == pos )
	    {
		sprintf(buf, "Changing your '%s' vote from '%s' to '%s' makes a whole lot of sense...\n\r",
		    poll->name, poll->response[pos], poll->response[pos]);
		send_to_char(buf,ch);
		return;
	    }

	    sprintf(buf, "Changing your '%s' vote from '%s' to '%s'...\n\r",
		    poll->name, poll->response[vote->voter_vote],
		    poll->response[pos]);
	    send_to_char(buf,ch);
	    vote->voter_vote = pos;
	    save_voting_polls();
	    return;
	}
    }    

    vote		= new_vote();
    vote->voter_name	= str_dup( ch->name );
    vote->voter_ip	= str_dup( ch->pcdata->socket );
    vote->voter_vote	= pos;

    vote->next_vote	= poll->vote;
    poll->vote		= vote;

    save_voting_polls();

    sprintf( buf, "Thank you for your vote of '%s' concerning '%s'.\n\r",
	poll->response[pos], poll->name );
    send_to_char( buf, ch );
}

char * parse_channel( CHAR_DATA *ch, char *string, int chan )
{
    extern CENSOR_DATA *censor_list;
    CENSOR_DATA *cen;
    char buf[2*MAX_INPUT_LENGTH];
    char temp;
    int pos = 0;
    int randomnum;

    if ( !IS_IMMORTAL( ch ) )
    {
	if ( mud_stat.colorlock || IS_SET( ch->act, PLR_CENSORED ) )
	    string = strip_color( string );

	if ( mud_stat.capslock || IS_SET( ch->act, PLR_CENSORED ) )
	    string = strip_caps( string );

	if ( channel_table[chan].censor )
	{
	    for ( cen = censor_list; cen != NULL; cen = cen->next )
		str_replace( string, cen->word, cen->replace );
	}
    }

    if ( channel_table[chan].drunk
    &&   !IS_NPC( ch )
    &&   ch->pcdata->condition[COND_DRUNK] > 0 )
    {
	struct struckdrunk drunk[] =
	{
	    {3, 10,
		{"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"}},
	    {8, 5,
		{"b", "b", "b", "B", "B", "vb"}},
	    {3, 5,
		{"c", "c", "C", "cj", "sj", "zj"}},
	    {5, 2,
		{"d", "d", "D"}},
	    {3, 3,
		{"e", "e", "eh", "E"}},
	    {4, 5,
		{"f", "f", "ff", "fff", "fFf", "F"}},
	    {8, 2,
		{"g", "g", "G"}},
	    {9, 6,
		{"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}},
	    {7, 6,
		{"i", "i", "Iii", "ii", "iI", "Ii", "I"}},
	    {9, 5,
		{"j", "j", "jj", "Jj", "jJ", "J"}},
	    {7, 2,
		{"k", "k", "K"}},
	    {3, 2,
		{"l", "l", "L"}},
	    {5, 8,
		{"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"}},
	    {6, 6,
		{"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
	    {3, 6,
		{"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"}},
	    {3, 2,
		{"p", "p", "P"}},
	    {5, 5,
		{"q", "q", "Q", "ku", "ququ", "kukeleku"}},
	    {4, 2,
		{"r", "r", "R"}},
	    {2, 5,
		{"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"}},
	    {5, 2,
		{"t", "t", "T"}},
	    {3, 6,
		{"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"}},
	    {4, 2,
		{"v", "v", "V"}},
	    {4, 2,
		{"w", "w", "W"}},
	    {5, 6,
		{"x", "x", "X", "ks", "iks", "kz", "xz"}},
	    {3, 2,
		{"y", "y", "Y"}},
	    {2, 9,
		{"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}}
	};

	if ( ch->pcdata->condition[COND_DRUNK] > 0 )
	{
	    do
	    {
		temp = toupper ( *string );
		if ( ( temp >= 'A' ) && ( temp <= 'Z' ) )
		{
		    if ( ch->pcdata->condition[COND_DRUNK] > drunk[temp - 'A'].min_drunk_level )
		    {
			randomnum = number_range (0, drunk[temp - 'A'].number_of_rep);
			strcpy (&buf[pos], drunk[temp - 'A'].replacement[randomnum]);
			pos += strlen (drunk[temp - 'A'].replacement[randomnum]);
		    }
		    else
			buf[pos++] = *string;
		} else {
		    if ( ( temp >= '0') && (temp <= '9' ) )
		    {
			temp = '0' + number_range ( 0, 9 );
			buf[pos++] = temp;
		    }
		    else
			buf[pos++] = *string;
		}
	    }

	    while ( *string++ );
	    buf[pos] = '\0';
	    strcpy( string, buf );
	}
    }
    return ( string );
}

int channel_lookup( char *argument )
{
    int pos;

    for ( pos = 0; channel_table[pos].name != NULL; pos++ )
    {
	if ( !str_prefix( argument, channel_table[pos].name ) )
	    return pos;
    }

    return -1;
}

void process_channel( CHAR_DATA *ch, char *argument, int chan )
{
    char buf[MAX_STRING_LENGTH], buf2[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    bool clangossip = FALSE, questgossip = FALSE, racetalk = FALSE;

    if ( chan == -1 )
    {
	send_to_char( "You found a bug in one of the channels!\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	TOGGLE_BIT( ch->comm, channel_table[chan].bit );
	sprintf( buf, "%s channel is now %s.\n\r",
	    channel_table[chan].name,
	    IS_SET( ch->comm, channel_table[chan].bit ) ? "OFF" : "ON" );
	send_to_char( buf, ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_QUIET ) && !channel_table[chan].quiet )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch );
	return;
    }

    if ( !channel_table[chan].arena
    &&   ( IS_SET( ch->in_room->room_flags, ROOM_ARENA )
    ||     IS_SET( ch->in_room->room_flags, ROOM_WAR ) )
    &&     get_trust(ch) <= LEVEL_HERO )
    {
	send_to_char( "You do not have access to this from within a bloody cell.\n\r", ch );
	return;
    }

    if ( ch->pcdata && ch->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
    {
	send_to_char( "The gods have revoked your channel priviliges.\n\r", ch );
	return;
    }

    if ( ch->in_room->vnum == ROOM_VNUM_CORNER && !IS_IMMORTAL( ch ) )
    {
	send_to_char( "Just keep your nose in the corner like a good little player.\n\r", ch );
	return;
    }

    if ( chan == channel_lookup( "questgossip" ) )
	questgossip = TRUE;
    else if ( chan == channel_lookup( "clangossip" ) )
	clangossip = TRUE;
    else if ( chan == channel_lookup( "racetalk" ) )
	racetalk = TRUE;

    REMOVE_BIT( ch->comm, channel_table[chan].bit );

    argument = parse_channel( ch, argument, chan );

    sprintf( buf, "%s", channel_table[chan].ch_string );
    if ( channel_table[chan].pretitle )
    {
	sprintf( buf2, "%s$n{x", pretitle( ch, NULL ) );
	str_replace_who( buf, "$n", buf2 );
    }
    act( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected != CON_PLAYING
	||   d->character == ch
	||   IS_SET( victim->comm, channel_table[chan].bit )
	||   ( channel_table[chan].quiet && IS_SET( victim->comm, COMM_QUIET ) )
	||   victim->level < channel_table[chan].level
	||   ( clangossip && !IS_IMMORTAL( victim ) && !is_clan( victim ) )
	||   ( questgossip && !IS_IMMORTAL( victim ) && !victim->pcdata->on_quest )
	||   ( racetalk && victim->race != ch->race )
	||   check_forget( victim, ch ) )
	    continue;

	sprintf( buf, "%s", channel_table[chan].other_string );

	if ( channel_table[chan].pretitle )
	{
	    sprintf( buf2, "%s$n{x", pretitle( ch, d->character ) );
	    str_replace_who( buf, "$n", buf2 );
	}

	act( buf, ch, argument, d->character, TO_VICT, POS_DEAD );
    }
}

void do_answer( CHAR_DATA *ch, char *argument )
{
    process_channel( ch, argument, channel_lookup( "answer" ) );
    return;
}

void do_ask( CHAR_DATA *ch, char *argument )
{
    process_channel( ch, argument, channel_lookup( "ask" ) );
    return;
}

void do_barter( CHAR_DATA *ch, char *argument )
{
    process_channel( ch, argument, channel_lookup( "barter" ) );
    return;
}

void do_cgossip( CHAR_DATA *ch, char *argument )
{
    if ( !is_clan( ch ) && !IS_IMMORTAL( ch ) )
    {
	send_to_char( "You are not in a clan!\n\r", ch );
	return;
    }

    process_channel( ch, argument, channel_lookup( "clangossip" ) );
    return;
}

void do_flame( CHAR_DATA *ch, char *argument )
{
    process_channel( ch, argument, channel_lookup( "flame" ) );
    return;
}

void do_gossip( CHAR_DATA *ch, char *argument )
{
    process_channel( ch, argument, channel_lookup( "gossip" ) );
    return;
}

void do_grats( CHAR_DATA *ch, char *argument )
{
    process_channel( ch, argument, channel_lookup( "grats" ) );
    return;
}

void do_herotalk( CHAR_DATA *ch, char *argument )
{
    process_channel( ch, argument, channel_lookup( "herotalk" ) );
    return;
}

void do_ic( CHAR_DATA *ch, char *argument )
{
    process_channel( ch, argument, channel_lookup( "ic" ) );
    return;
}

void do_immtalk( CHAR_DATA *ch, char *argument )
{
    process_channel( ch, argument, channel_lookup( "immtalk" ) );
    return;
}

void do_ooc( CHAR_DATA *ch, char *argument )
{
    process_channel( ch, argument, channel_lookup( "ooc" ) );
    return;
}

void do_qgossip( CHAR_DATA *ch, char *argument )
{
    if ( ch->pcdata && !ch->pcdata->on_quest && !IS_IMMORTAL( ch ) )
    {   
	send_to_char( "You are not on a quest!\n\r", ch );
	return;
    }   

    process_channel( ch, argument, channel_lookup( "questgossip" ) );
    return;
}

void do_quote( CHAR_DATA *ch, char *argument )
{
    process_channel( ch, argument, channel_lookup( "quote" ) );
    return;
}

void do_racetalk( CHAR_DATA *ch, char *argument )
{
    process_channel( ch, argument, channel_lookup( "racetalk" ) );
    return;
}
