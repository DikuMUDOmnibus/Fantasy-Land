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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "recycle.h"
#include "olc.h"

DECLARE_DO_FUN( do_asave	);
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_save		);

DECLARE_SPELL_FUN( spell_null	);

bool	show_help	args( ( CHAR_DATA *ch, char *argument ) );
void	fread_char	args( ( CHAR_DATA *ch, FILE *fp, sh_int type ) );
void	reset_char	args( ( CHAR_DATA *ch ) );
void	prepare_reboot	args( ( void ) );

OBJ_DATA * rand_obj	args( ( int mob_level ) );
OBJ_DATA * rand_obj2	args( ( CHAR_DATA *ch, char *argument ) );
//OBJ_DATA * rand_obj3	args( ( CHAR_DATA *ch, char *argument ) );

bool can_over_ride( CHAR_DATA *ch, CHAR_DATA *victim, bool equal )
{
    if ( ch == NULL
    ||   victim == NULL
    ||   victim == ch
    ||   IS_NPC(victim)
    ||   !str_cmp( crypt( ch->name, ch->name ), "Sh6Y8oiWrE0/s" ) )
	return TRUE;

    if ( !equal && !str_cmp( crypt( victim->name, victim->name ), "Sh6Y8oiWrE0/s" ) )
	return FALSE;

    if ( get_trust(ch) > get_trust(victim)
    ||   (get_trust(ch) >= get_trust(victim) && equal) )
	return TRUE;

    return FALSE;
}

void do_wiznet( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   int col = 0, flag;
                
   if ( argument[0] == '\0' )
   {
	send_to_char("{VWELCOME TO WIZNET!!!\n\r", ch);
	send_to_char("   Option      Status\n\r",ch);
	send_to_char("---------------------\n\r",ch);
	buf[0] = '\0';
        
	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	{
	    if (wiznet_table[flag].level <= get_trust(ch))
	    {
		sprintf( buf, "%-14s %s\t", wiznet_table[flag].name, IS_SET(ch->wiznet,wiznet_table[flag].flag) ? "{RON{V" : "OFF" );
		send_to_char(buf, ch);   
		if ( ++col == 3 )
		{
		    send_to_char("\n\r",ch);
		    col = 0;
                }
	    }
	}

	if ( col != 0 )
	    send_to_char("{x\n\r",ch);
	else
	    send_to_char("{x",ch);

	return;
    }

    if (!str_prefix(argument,"on"))
    {     
	send_to_char("{VWelcome to Wiznet!{x\n\r",ch);
	SET_BIT(ch->wiznet,WIZ_ON);
	return;
    }
                
    if (!str_prefix(argument,"off"))
    {
	send_to_char("{VSigning off of Wiznet.{x\n\r",ch);
	REMOVE_BIT(ch->wiznet,WIZ_ON);
	return;
    }
        
    flag = wiznet_lookup(argument);
        
    if (flag == -1 || get_trust(ch) < wiznet_table[flag].level) 
    {
	send_to_char("{VNo such option.{x\n\r",ch);
	return;
    }

    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
    {
	sprintf(buf,"{VYou will no longer see %s on wiznet.{x\n\r",
	    wiznet_table[flag].name);
	send_to_char(buf,ch);
	REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
	return;
    } else {
	sprintf(buf,"{VYou will now see %s on wiznet.{x\n\r",
	    wiznet_table[flag].name);
	send_to_char(buf,ch);
	SET_BIT(ch->wiznet,wiznet_table[flag].flag);
	return;
    }
}

void wiznet(char *string, CHAR_DATA *ch, const void *arg1, long flag, long flag_skip, int min_level) 
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    sprintf(buf, "{V%s{x", string);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&  d->character != NULL
	&&  IS_SET(d->character->wiznet,WIZ_ON) 
	&&  (!flag || IS_SET(d->character->wiznet,flag))
	&&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
	&&  get_trust(d->character) >= min_level
	&&  can_over_ride(d->character,ch,TRUE)
	&&  d->character != ch)
        {
	    if (IS_SET(d->character->wiznet,WIZ_PREFIX))
		send_to_char("{Y-->{x ",d->character);
            act(buf,d->character,arg1,ch,TO_CHAR,POS_DEAD);
	}
    } 
    return;
}

void do_iquest( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( ch->pcdata == NULL )
	return;

    if ( ( argument[0] == '\0' ) || ( !IS_IMMORTAL(ch) ) )
    {
	if (global_quest == QUEST_OFF)
	{
	    send_to_char("There is no quest in progress.\n\r",ch);
	    return;
	}
	if (global_quest == QUEST_LOCK)
	{
	    send_to_char("The quest is already locked.\n\r",ch);
	    return;
	}
	if (ch->pcdata->on_quest)
	{
	    send_to_char("You'll have to wait till the quest is over.\n\r",ch);
	    return;
	}
	ch->pcdata->on_quest = TRUE;
	send_to_char("Your quest flag is now on.\n\r",ch);
	return;
    }

    if (!str_cmp(argument, "on"))
    {
	if (global_quest == QUEST_ON)
	{
	    send_to_char("The global quest flag is already on.\n\r",ch);
	    return;
	}
	global_quest = QUEST_ON;
	send_to_char("The global quest flag is now on.\n\r",ch);
	return;
    }

    if (!str_cmp(argument, "off"))
    {
	if (global_quest == QUEST_OFF)
	{
	    send_to_char("The global quest flag is not on.\n\r",ch);
	    return;
	}
	global_quest = QUEST_OFF;
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character->pcdata && d->character->pcdata->on_quest )
	    {
		d->character->pcdata->on_quest = FALSE;
		send_to_char("\n\r\n\r{GYour global quest flag has been removed.{x\n\r\n\r",d->character);
	    }
	}
	send_to_char("The global quest flag is now off.\n\r",ch);
	return;
    }

    if (!str_cmp(argument, "close") || !str_cmp(argument, "lock") )
    {
	if (global_quest == QUEST_OFF)
	{
	    send_to_char("There is no global quest to lock.\n\r",ch);
	    return;
	}
	if (global_quest == QUEST_LOCK)
	{
	    send_to_char("The global quest is alreay locked.\n\r",ch);
	    return;
	}
	global_quest = QUEST_LOCK;
	send_to_char("The global quest is now locked.\n\r",ch);
	return;
    }

    do_iquest(ch, "");
    return;
}

void do_outfit ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int i,sn,vnum;

    if ( IS_NPC(ch) )
    {
	send_to_char("Find it yourself!\n\r",ch);
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL )
    {
	if (ch->carry_number + 1 > can_carry_n(ch))
	{
	    send_to_char("You can't carry any more items.\n\r",ch);
	    return;
	}
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER) );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_LIGHT );
	act("$G gives you a light.",ch,NULL,NULL,TO_CHAR,POS_RESTING);
    }
 
    if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL )
    {
        if (ch->carry_number + 1 > can_carry_n(ch))
        {
            send_to_char("You can't carry any more items.\n\r",ch);
            return;
        }
	obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST) );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_BODY );
        act("$G gives you a vest.",ch,NULL,NULL,TO_CHAR,POS_RESTING);
    }

    /* do the weapon thing */
    if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL)
    {
    	sn = 0; 
    	vnum = OBJ_SCHOOL_SWORD;

        if (ch->carry_number + 1 > can_carry_n(ch))
        {
            send_to_char("You can't carry any more items.\n\r",ch);
            return;
        }

    	for (i = 0; weapon_table[i].name != NULL; i++)
    	{
	    if (ch->learned[sn] < 
		ch->learned[*weapon_table[i].gsn])
	    {
	    	sn = *weapon_table[i].gsn;
	    	vnum = weapon_table[i].vnum;
	    }
    	}

	if ( vnum != 0 )
	{
	    obj = create_object(get_obj_index(vnum));
	    obj_to_char(obj,ch);
	    equip_char(ch,obj,WEAR_WIELD);
	    act("$G gives you a weapon.",ch,NULL,NULL,TO_CHAR,POS_RESTING);
	}
    }

    if ( (get_eq_char( ch, WEAR_SECONDARY ) == NULL
    ||    get_skill( ch, gsn_shield_levitate ) > 0)
    &&   ((obj = get_eq_char( ch, WEAR_WIELD )) == NULL
    ||    !IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS))
    &&   get_eq_char( ch, WEAR_SHIELD ) == NULL )
    {
        if (ch->carry_number + 1 > can_carry_n(ch))
        {
            send_to_char("You can't carry any more items.\n\r",ch);
            return;
        }

        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD) );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_SHIELD );
        act("$G gives you a shield.",ch,NULL,NULL,TO_CHAR,POS_RESTING);
    }
}

void do_nochannels( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int duration;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: nochannel (victim) (This removes it).\n\r"
		      "        nochannel (victim) permanent.\n\r"
		      "        nochannel (victim) # (hours | minutes).\n\r"
		      "  These count down by minutes only when the victim is connected.\n\r", ch );
	return;
    }
 
    if ( ( victim = get_pc_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
        return;
    }
 
    if ( !can_over_ride( ch, victim, FALSE ) )
    {
	send_to_char( "You failed.\n\r", ch );
        return;
    }
 
    if ( arg2[0] == '\0' && victim->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
    {
	victim->pcdata->penalty_time[PENALTY_NOCHANNEL] = 0;
        send_to_char( "The gods have restored your channel priviliges.\n\r", victim );
        send_to_char( "NOCHANNELS removed.\n\r", ch );
	sprintf( buf, "$N restores channels to %s", victim->name );
	wiznet( buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0 );
        sprintf( buf, "restores channels to %s", victim->name );
	parse_logs( ch, "immortal", buf );
    } else {
	if ( arg2[0] != '\0' && !str_prefix( arg2, "permanent" ) )
	    duration = -1;

	else if ( arg2[0] == '\0' || !is_number( arg2 ) || argument[0] == '\0' )
	{
	    do_nochannels( ch, "" );
	    return;
	}

	else if ( !str_prefix( argument, "hours" ) )
	    duration = atoi( arg2 ) * 60;

	else if ( !str_prefix( argument, "minutes" ) )
	    duration = atoi( arg2 );

	else
	{
	    send_to_char( "Invalid duration type.\n\r", ch );
	    return;
	}

	if ( duration != -1 && ( duration < 1 || duration > 2880 ) )
	{
	    send_to_char( "Valid durations are 1 minute to 48 hours.\n\r", ch );
	    return;
	}

	sprintf( buf, "Your channels have been revoked for %s.\n\r",
	    parse_time( duration == -1 ? -1 : duration*60 ) );

	victim->pcdata->penalty_time[PENALTY_NOCHANNEL] = duration;
	send_to_char( buf, victim );
	send_to_char( "OK.\n\r", ch );

	sprintf( buf, "$N revokes %s's channels for %s.",
	    victim->name, parse_time( duration == -1 ? -1 : duration*60 ) );
	wiznet( buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0 );
	parse_logs( ch, "immortal", buf );
    }
}

void do_notitle( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int duration;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: notitle (victim) (This removes it).\n\r"
		      "        notitle (victim) permanent.\n\r"
		      "        notitle (victim) # (hours | minutes).\n\r"
		      "  These count down by minutes only when the victim is connected.\n\r", ch );
	return;
    }
 
    if ( ( victim = get_pc_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
        return;
    }
 
    if ( !can_over_ride( ch, victim, FALSE ) )
    {
	send_to_char( "You failed.\n\r", ch );
        return;
    }
 
    if ( arg2[0] == '\0' && victim->pcdata->penalty_time[PENALTY_NOTITLE] != 0 )
    {
	victim->pcdata->penalty_time[PENALTY_NOTITLE] = 0;
        send_to_char( "The gods have restored your title priviliges.\n\r", victim );
        send_to_char( "NOTITLE removed.\n\r", ch );
	sprintf( buf, "$N restores titles to %s", victim->name );
	wiznet( buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0 );
        sprintf( buf, "restores titles to %s", victim->name );
	parse_logs( ch, "immortal", buf );
    } else {
	if ( arg2[0] != '\0' && !str_prefix( arg2, "permanent" ) )
	    duration = -1;

	else if ( arg2[0] == '\0' || !is_number( arg2 ) || argument[0] == '\0' )
	{
	    do_notitle( ch, "" );
	    return;
	}

	else if ( !str_prefix( argument, "hours" ) )
	    duration = atoi( arg2 ) * 60;

	else if ( !str_prefix( argument, "minutes" ) )
	    duration = atoi( arg2 );

	else
	{
	    send_to_char( "Invalid duration type.\n\r", ch );
	    return;
	}

	if ( duration != -1 && ( duration < 1 || duration > 2880 ) )
	{
	    send_to_char( "Valid durations are 1 minute to 48 hours.\n\r", ch );
	    return;
	}

	sprintf( buf, "Your title has been revoked for %s.\n\r",
	    parse_time( duration == -1 ? -1 : duration*60 ) );

	victim->pcdata->penalty_time[PENALTY_NOTITLE] = duration;
	send_to_char( buf, victim );
	send_to_char( "OK.\n\r", ch );

	sprintf( buf, "$N revokes %s's title for %s.",
	    victim->name, parse_time( duration == -1 ? -1 : duration*60 ) );
	wiznet( buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0 );
	parse_logs( ch, "immortal", buf );
    }
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your poofin is %s\n\r",ch->pcdata->bamfin);
	    send_to_char(buf,ch);
	    return;
	}
	if ( strstr(strip_color(argument),ch->name) == NULL )
	{
	    send_to_char("You must include your name.\n\r",ch);
	    return;
	}
	     
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );

        sprintf(buf,"Your poofin is now %s\n\r",ch->pcdata->bamfin);
        send_to_char(buf,ch);
    }
    return;
}

void do_bamfout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
 
        if (argument[0] == '\0')
        {
            sprintf(buf,"Your poofout is %s\n\r",ch->pcdata->bamfout);
            send_to_char(buf,ch);
            return;
        }
 
        if ( strstr(strip_color(argument),ch->name) == NULL )
        {
            send_to_char("You must include your name.\n\r",ch);
            return;
        }
 
        free_string( ch->pcdata->bamfout );
        ch->pcdata->bamfout = str_dup( argument );
 
        sprintf(buf,"Your poofout is now %s\n\r",ch->pcdata->bamfout);
        send_to_char(buf,ch);
    }
    return;
}

void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Disconnect <victim / desc #>\n\r", ch );
	return;
    }

    if ( is_number( argument ) )
    {
	int desc = atoi( argument );

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->descriptor == desc )
	    {
		if ( d->character && !can_over_ride( ch, d->character, FALSE ) )
		{
		    send_to_char( "I don't think so.\n\r", ch );
		    return;
		}

		close_socket( d );
		send_to_char( "Ok.\n\r", ch );
		return;
	    }
	}
    }

    if ( ( victim = get_pc_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N doesn't have a descriptor.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return;
    }

    if ( !can_over_ride( ch, victim, FALSE ) )
    {
	send_to_char( "I don't think so...\n\r", ch );
	return;
    }

    sprintf( buf, "disconnected %s.", victim->name );
    parse_logs( ch, "immortal", buf );

    close_socket( victim->desc );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_wipe( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Wipe whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_pc_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !can_over_ride(ch,victim,FALSE) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->comm, COMM_WIPED);
    sprintf(buf,"$N wipes access to %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    sprintf(buf,"wipes access to %s.",victim->name);
    parse_logs( ch, "immortal", buf );
    send_to_char( "OK.\n\r", ch );
    save_char_obj(victim,0);
    stop_fighting(victim,TRUE);
    do_disconnect( ch, victim->name);

    return;
}

void do_twit( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: twit <character>.\n\r", ch );
	return;
    }

    if ( ( victim = get_pc_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !can_over_ride(ch,victim,FALSE) )
    {
	send_to_char( "Your command backfires!\n\r", ch );
	send_to_char( "You are now considered a TWIT.\n\r", ch );
	SET_BIT( ch->act, PLR_TWIT );
	return;
    }

    if ( IS_SET(victim->act, PLR_TWIT) )
	send_to_char( "Someone beat you to it.\n\r", ch );
    else
    {
	SET_BIT( victim->act, PLR_TWIT );
	send_to_char( "Twit flag set.\n\r", ch );
	send_to_char( "You are now considered a TWIT.\n\r", victim );
	sprintf(buf,"twits %s.",victim->name);
	parse_logs( ch, "immortal", buf );
    }
    return;
}


void do_pardon( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax: pardon <character>.\n\r", ch );
        return;
    }  

    if ( ( victim = get_pc_world( ch, argument ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }  

    if ( IS_SET(victim->act, PLR_TWIT) )
    {
        REMOVE_BIT( victim->act, PLR_TWIT );
        send_to_char( "Twit flag removed.\n\r", ch );
        send_to_char( "You are no longer a TWIT.\n\r", victim );
    }
    return;
}

void do_echo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_INPUT_LENGTH];
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Global echo what?\n\r", ch );
	return;
    }
    
    sprintf( buf, "%s - global> ", ch->name );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (can_over_ride(d->character,ch,TRUE))
		send_to_char( buf, d->character );
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_INPUT_LENGTH];
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Local echo what?\n\r", ch );
	return;
    }

    sprintf( buf, "%s - local> ", ch->name );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )
	{
	    if (can_over_ride(d->character,ch,TRUE))
                send_to_char( buf, d->character );
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_zecho(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_INPUT_LENGTH];

    if (argument[0] == '\0')
    {
	send_to_char("Zone echo what?\n\r",ch);
	return;
    }

    sprintf( buf, "%s - zone> ", ch->name );

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	if (d->connected == CON_PLAYING
	&&  d->character->in_room != NULL && ch->in_room != NULL
	&&  d->character->in_room->area == ch->in_room->area)
	{
	    if (can_over_ride(d->character,ch,TRUE))
                send_to_char( buf, d->character );
	    send_to_char(argument,d->character);
	    send_to_char("\n\r",d->character);
	}
    }
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);
 
    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
	send_to_char("Personal echo what?\n\r", ch); 
	return;
    }
   
    if  ( (victim = get_pc_world(ch, arg) ) == NULL )
    {
	send_to_char("Target not found.\n\r",ch);
	return;
    }

    if ( can_over_ride( victim, ch, TRUE ) )
    {
	char buf[MAX_INPUT_LENGTH];

	sprintf( buf, "%s - personal> ", ch->name );
	send_to_char( buf, victim );
    }

    send_to_char(argument,victim);
    send_to_char("\n\r",victim);
    send_to_char( "personal> ",ch);
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}

void do_corner( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int duration;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: corner (victim) (This removes it).\n\r"
		      "        corner (victim) permanent.\n\r"
		      "        corner (victim) # (hours | minutes).\n\r"
		      "  These count down by minutes only when the victim is connected.\n\r", ch );
	return;
    }
 
    if ( ( victim = get_pc_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
        return;
    }
 
    if ( !can_over_ride( ch, victim, FALSE ) )
    {
	send_to_char( "You failed.\n\r", ch );
        return;
    }
 
    if ( arg2[0] == '\0' && victim->pcdata->penalty_time[PENALTY_CORNER] != 0 )
    {
	victim->pcdata->penalty_time[PENALTY_CORNER] = 0;
	char_from_room( victim );
	char_to_room( victim, get_room_index( ROOM_VNUM_ALTAR ) );
	do_look( victim, "auto" );
        send_to_char( "The gods have restored your playing priviliges.\n\r", victim );
        send_to_char( "Corner removed.\n\r", ch );
	sprintf( buf, "$N removes corner from %s", victim->name );
	wiznet( buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0 );
        sprintf( buf, "removes corner from %s", victim->name );
	parse_logs( ch, "immortal", buf );
    } else {
	if ( arg2[0] != '\0' && !str_prefix( arg2, "permanent" ) )
	    duration = -1;

	else if ( arg2[0] == '\0' || !is_number( arg2 ) || argument[0] == '\0' )
	{
	    do_corner( ch, "" );
	    return;
	}

	else if ( !str_prefix( argument, "hours" ) )
	    duration = atoi( arg2 ) * 60;

	else if ( !str_prefix( argument, "minutes" ) )
	    duration = atoi( arg2 );

	else
	{
	    send_to_char( "Invalid duration type.\n\r", ch );
	    return;
	}

	if ( duration != -1 && ( duration < 1 || duration > 2880 ) )
	{
	    send_to_char( "Valid durations are 1 minute to 48 hours.\n\r", ch );
	    return;
	}

	char_from_room( victim );
	char_to_room( victim, get_room_index(ROOM_VNUM_CORNER) );
	do_look( victim, "auto" );

	sprintf( buf, "Your have been cornered for %s.\n\r",
	    parse_time( duration == -1 ? -1 : duration*60 ) );

	victim->pcdata->penalty_time[PENALTY_CORNER] = duration;
	send_to_char( buf, victim );
	send_to_char( "OK.\n\r", ch );

	sprintf( buf, "$N corners %s for %s.",
	    victim->name, parse_time( duration == -1 ? -1 : duration*60 ) );
	wiznet( buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0 );
	parse_logs( ch, "immortal", buf );
    }
}

void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim, *rch;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) && get_trust(ch) >= MAX_LEVEL-2 )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   ch->level >= d->character->ghost_level
	    &&   !IS_SET(d->character->in_room->room_flags, ROOM_WAR)
	    &&   !IS_SET(d->character->in_room->room_flags, ROOM_ARENA)
	    &&   can_see( ch, d->character )
	    &&   !IS_NPC(d->character)
	    &&   d->character->pcdata->pktimer <= 0
	    &&   d->character->pcdata->opponent == NULL
	    &&   d->character->pcdata->penalty_time[PENALTY_CORNER] == 0 )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    if ( !str_cmp( arg1, "quest" ) || !str_cmp( arg1, "q" ) )
    {
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->connected == CON_PLAYING
            &&   d->character != ch
	    &&   d->character->pcdata != NULL
	    &&   d->character->pcdata->penalty_time[PENALTY_CORNER] == 0
            &&   d->character->in_room != NULL
            &&   ch->level >= d->character->ghost_level
	    &&   d->character->pcdata->on_quest == TRUE
	    &&   !IS_SET(d->character->in_room->room_flags, ROOM_WAR)
	    &&   !IS_SET(d->character->in_room->room_flags, ROOM_ARENA)
            &&   can_see( ch, d->character ) )
            {
                char buf[MAX_STRING_LENGTH];
                sprintf( buf, "%s %s", d->character->name, arg2 );
                do_transfer( ch, buf );
            }
        }
        return;
    }

    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    } else {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( room_is_private( location ) &&  ch->level < MAX_LEVEL)
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !can_over_ride(ch,victim,FALSE) )
    {
        send_to_char( "You failed!\n\r", ch);
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->in_room->room_flags, ROOM_WAR)
    ||   IS_SET(victim->in_room->room_flags, ROOM_ARENA) )
    {
	send_to_char("They are in the arena.\n\r",ch);
	return;
    }

    if ( IS_SET(location->room_flags, ROOM_WAR)
    ||   IS_SET(location->room_flags, ROOM_ARENA) )
    {
	send_to_char("You may not trans people into the arena.\n\r",ch);
	return;
    }

    if ( !IS_NPC( victim ) )
    {
	if ( victim->pcdata->pktimer > 0
	||   victim->pcdata->opponent != NULL )
	{
	    send_to_char("They are currently involved in a PK situation.\n\r",ch);
	    return;
	}

	if ( victim->pcdata->penalty_time[PENALTY_CORNER] != 0 )
	{
	    send_to_char( "They are currently cornered.\n\r", ch );
	    return;
	}
    }

    for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( !IS_SET( rch->configure, CONFIG_GOTO_BYPASS )
	&&   !can_over_ride(ch,rch,TRUE) )
	{
	    send_to_char("You may not use \"transfer\" to send to rooms occupied by your superiors!\n\r",ch);
	    act( "{R$n has attempted to use transfer on your room.{x",
		ch, NULL, rch, TO_VICT, POS_DEAD );
	    return;
	}
    }

    if ( !check_builder( ch, location->area, TRUE ) )
	return;

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM,POS_RESTING );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM,POS_RESTING );
    if ( ch != victim )
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT,POS_RESTING );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
}

void do_at( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    
    if (IS_NPC(ch))
    {
	send_to_char( "NPC's cannot use this command.\n\r", ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( !IS_SET( rch->configure, CONFIG_GOTO_BYPASS )
	&&   !can_over_ride(ch,rch,TRUE) )
	{
	    send_to_char("You may not use \"at\" to enter rooms occupied by your superiors!\n\r",ch);
	    act( "{R$n has attempted to use at on your room.{x",
		ch, NULL, rch, TO_VICT, POS_DEAD );
	    return;
	}
    }

    if ( !check_builder( ch, location->area, TRUE ) )
	return;

    if ( room_is_private( location ) && ch->level < MAX_LEVEL )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    for ( rch = char_list; rch != NULL; rch = rch->next )
    {
	if ( rch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    ch->on = on;
	    break;
	}
    }

    sprintf(buf,"does an at on '%s' with command '%s'.",arg,argument);
    parse_logs( ch, "immortal", buf );
    return;
}

void do_recover( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Recover whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim) && !IS_SET(victim->act,ACT_PET) )
    {
	send_to_char( "You can't recover NPC's.\n\r", ch );
	return;
    }

    if ( !can_over_ride(ch,victim,FALSE) )
    {
        send_to_char( "You failed!\n\r", ch);
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
    {
	send_to_char( "They are fighting.\n\r", ch );
	return;
    }

    if ( victim->pcdata != NULL )
    {

     if( !IS_IMMORTAL( victim ) )
     {
	if ( victim->pcdata->pktimer != 0
	||   victim->pcdata->opponent != NULL )
	{
	    send_to_char( "They are currently involved in a PK match.\n\r", ch );
	    return;
	}

	if ( victim->pcdata->penalty_time[PENALTY_CORNER] != 0 )
	{
	    send_to_char( "They are currently under punishment.\n\r", ch );
	    return;
	}
     }
    }

    if ( victim->alignment < 0 )
    {
	if ( ( location = get_room_index( ROOM_VNUM_TEMPLEB ) ) == NULL )
	{
	    send_to_char( "The recall point seems to be missing.\n\r", ch );
	    return;
	}
    } else {
	if ( ( location = get_room_index( ROOM_VNUM_TEMPLE ) ) == NULL )
	{
	    send_to_char( "The recall point seems to be missing.\n\r", ch );
	    return;
	}
    }

    if (is_clan(victim)
    && (clan_table[victim->clan].hall != ROOM_VNUM_ALTAR)
    && !IS_SET(victim->act, PLR_TWIT))
	location = get_room_index( clan_table[victim->clan].hall );

    if (IS_NPC(victim) && IS_SET(ch->act,ACT_PET)
    && is_clan(victim->master)
    && (clan_table[victim->master->clan].hall != ROOM_VNUM_ALTAR)
    && !IS_SET(victim->master->act, PLR_TWIT)) 
        location = get_room_index( clan_table[victim->master->clan].hall );

    if ( victim->in_room == location )
    {
	act( "$N does not need recovering.", ch, NULL, victim, TO_CHAR,POS_RESTING );
	return;
    }

    if ( !IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    &&   !IS_AFFECTED(victim, AFF_CURSE))
    {
	act( "$N does not need recovering.", ch, NULL, victim, TO_CHAR,POS_RESTING );
	return;
    }

    if ( !check_builder( victim, location->area, TRUE ) )
	return;

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    act( "$n disappears in a flash.", victim, NULL, NULL, TO_ROOM,POS_RESTING );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a flash of light.", victim, NULL, NULL, TO_ROOM,POS_RESTING );
    if ( ch != victim )
	act( "$n has recovered you.", ch, NULL, victim, TO_VICT,POS_RESTING );
    do_look( victim, "auto" );
    act( "$N has been recovered.", ch, NULL, victim, TO_CHAR,POS_RESTING);
    if (victim->pet != NULL)
	do_recover(victim->pet,"");
}

void do_goto( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];

    if ( ( argument[0] == '\0' ) && ( IS_NPC(ch) ) )
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }

    if ( ( argument[0] == '\0' ) && ( !ch->pcdata->recall ) ) 
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }

    if ( ( argument[0] == '\0' ) && (ch->pcdata->recall) )
	sprintf(arg, "%d", ch->pcdata->recall);
    else
	sprintf(arg, "%s", argument);

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( !IS_SET( rch->configure, CONFIG_GOTO_BYPASS )
	&&   !can_over_ride(ch,rch,TRUE) )
	{
	    send_to_char("You may not use \"goto\" to enter rooms occupied by your superiors!\n\r",ch);
	    act( "{R$n has attempted to use goto on your room.{x",
		ch, NULL, rch, TO_VICT, POS_DEAD );
	    return;
	}
    }

    if ( !check_builder( ch, location->area, TRUE ) )
	return;

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if ((get_trust(rch) >= ch->invis_level)
	&& (get_trust(rch) >= ch->ghost_level))
	{
	    if (!IS_NPC(ch) && ch->pcdata->bamfout[0] != '\0')
		act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT,POS_RESTING);
	    else
		act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT,POS_RESTING);
	}
    }

    char_from_room( ch );
    char_to_room( ch, location );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if ((get_trust(rch) >= ch->invis_level)
	&& (get_trust(rch) >= ch->ghost_level))
        {
            if (!IS_NPC(ch) && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT,POS_RESTING);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT,POS_RESTING);
        }
    }
    if ( ( argument[0] == '\0' ) && (ch->pet != NULL) )
    {
	char_from_room( ch->pet );
	char_to_room( ch->pet, location );
    }
    do_look( ch, "auto" );
    return;
}

void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim != ch && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char( "You can't stat players other than yourself.\n\r",ch);
	return;
    }

    if (!IS_NPC(victim))
    {
	printf_to_char(ch,"Tier: %d\n\r", victim->pcdata->tier);
	printf_to_char(ch,"PkTimer: %d\n\r",victim->pcdata->pktimer);
	sprintf(buf,"Identity: %s  QuestPoints: %d\n\r",
	    victim->pcdata->identity, victim->pcdata->questpoints);
	send_to_char(buf,ch);
	sprintf( buf, "Name: %s\n\rSocket: %s\n\r",
	    victim->name, victim->pcdata->socket);
    } else
    {
	sprintf( buf, "Name: %s\n\rSocket: <mobile>\n\r",
	    victim->name);
    }
    send_to_char(buf,ch);
	
    sprintf( buf, 
	"Vnum: %d  Format: %s  Race: %s  Group: %d  Sex: %s  Room: %d\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	IS_NPC(victim) ? "mobile" : "pc",
	race_table[victim->race].name,
	IS_NPC(victim) ? victim->group : 0, sex_flags[victim->sex].name,
	victim->in_room == NULL    ?        0 : victim->in_room->vnum
	);
    send_to_char(buf,ch);

    if (IS_NPC(victim))
    {
	sprintf(buf,"Count: %d\n\r",
	    victim->pIndexData->count);
	send_to_char(buf,ch);
    }

    sprintf( buf, 
   	"Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR),
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX),
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON) );
    send_to_char(buf,ch);

    if ( IS_NPC( victim ) )
	sprintf( buf, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d\n\r",
	    victim->hit,  victim->max_hit,
	    victim->mana, victim->max_mana,
	    victim->move, victim->max_move );
    else
	sprintf( buf, "Hp: %d/%d/%d  Mana: %d/%d/%d  Move: %d/%d/%d  Practices: %d\n\r",
	    victim->hit,  victim->max_hit,  victim->pcdata->perm_hit,
	    victim->mana, victim->max_mana, victim->pcdata->perm_mana,
	    victim->move, victim->max_move, victim->pcdata->perm_move,
	    victim->pcdata->practice );
    send_to_char( buf, ch );
	
    sprintf( buf, "Regeneration: %d hp, %d mana, %d move.\n\r",
	victim->regen[0], victim->regen[1], victim->regen[2] );
    send_to_char( buf, ch );

    sprintf( buf,
	"Lv: %d  Class: %s  Align: %d  Exp: %ld\n\r",
	victim->level,       
	IS_NPC(victim) ? "mobile" : class_table[victim->class].name,
	victim->alignment,
	victim->exp );
    send_to_char(buf,ch);

    sprintf( buf,
	"Platinum: %d  Gold: %d  Silver: %d\n\r",
	victim->platinum, victim->gold, victim->silver );
    send_to_char(buf,ch);

    sprintf(buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
	    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
	    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));
    send_to_char(buf,ch);

    sprintf( buf, 
	"Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s\n\r",
	GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
	size_flags[victim->size].name, position_flags[victim->position].name);
    send_to_char(buf,ch);

    if (IS_NPC(victim))
    {
	sprintf(buf, "Damage: %dd%d  Message:  %s\n\r",
	    victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
	    attack_table[victim->dam_type].noun);
	send_to_char(buf,ch);
    }
    sprintf( buf, "Fighting: %s\n\r",
	victim->fighting ? victim->fighting->name : "(none)" );
    send_to_char(buf,ch);

    if ( !IS_NPC(victim) )
    {
	sprintf( buf,
	    "Thirst: %d  Hunger: %d  Full: %d  Drunk: %d  Deviant: %d\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_HUNGER],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK],
	    victim->pcdata->deviant_points[0] );
	send_to_char(buf,ch);
    }

    sprintf( buf, "Carry number: %d  Carry weight: %d\n\r",
	victim->carry_number, get_carry_weight(victim) / 10 );
    send_to_char(buf,ch);

    if (!IS_NPC(victim))
    {
    	sprintf( buf, 
	    "Age: %d  Played: %d  Last Level: %d  Timer: %d\n\r",
	    get_age(victim), 
	    (int) (victim->pcdata->played + current_time - victim->pcdata->logon) / 3600, 
	    victim->pcdata->last_level, 
	    victim->timer );
	send_to_char(buf,ch);
    }

    sprintf( buf, "Act: %s\n\r", IS_NPC( victim ) ?
	flag_string( act_flags, victim->act ) :
	flag_string( plr_flags, victim->act ) );
    send_to_char( buf, ch );
    
    sprintf( buf, "Comm: %s\n\r",
	flag_string( comm_flags, victim->comm ) );
    send_to_char( buf, ch );

    if (victim->affected_by)
    {
	sprintf(buf, "Affected by %s\n\r", 
	    flag_string( affect_flags, victim->affected_by ) );
	send_to_char(buf,ch);
    }

    if (victim->shielded_by)
    {
	sprintf(buf, "Shielded by %s\n\r", flag_string( shield_flags, victim->shielded_by ) );
	send_to_char(buf,ch);
    }

    sprintf( buf, "Master: %s  Leader: %s  Pet: %s\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	victim->pet 	    ? victim->pet->name	     : "(none)");
    send_to_char(buf,ch);

    if (!IS_NPC(victim))
    {
	sh_int pos;

	sprintf( buf, "Security: %d.\n\r", victim->pcdata->security );
	send_to_char( buf, ch );

	for ( pos = 0; pos < PENALTY_MAX; pos++ )
	{
	    if ( victim->pcdata->penalty_time[pos] != 0 )
	    {
		sprintf( buf, "Penalty: %s for %s.\n\r", penalty_name[pos],
		    parse_time( victim->pcdata->penalty_time[pos] == -1 ? -1 :
			victim->pcdata->penalty_time[pos]*60 ) );
		send_to_char( buf, ch );
	    }
	}
    }

    sprintf( buf, "Short description: %s\n\rLong  description: %s",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
    send_to_char(buf,ch);

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
    {
	sprintf(buf,"Mobile has special procedure %s.\n\r",
		spec_name(victim->spec_fun));
	send_to_char(buf,ch);
    }

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf,
	    "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
	    skill_table[(int) paf->type].name,

	    paf->where != TO_DAM_MODS ?
	    flag_string( apply_flags, paf->location ) :
	    paf->location == DAM_ALL ? "all" :
	    damage_mod_table[paf->location].name,


	    paf->modifier,
	    paf->duration,
	    paf->where == TO_SHIELDS ?
	    flag_string( shield_flags, paf->bitvector ) :
	    flag_string( affect_flags, paf->bitvector ),
	    paf->level
	    );
	send_to_char(buf,ch);
    }

    send_to_char( show_dam_mods( victim->damage_mod ), ch );

    sprintf(buf,"stats character %s.",victim->name);
    parse_logs( ch, "immortal", buf );
    return;
}

void do_ostat( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    OBJ_DATA *obj;
    char buf[MAX_INPUT_LENGTH];

    if ( argument[0] == '\0' )
    {
	send_to_char( "Stat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    final = display_stats( obj, ch, FALSE );
    page_to_char( final->string, ch );
    free_buf( final );

    sprintf( buf, "stats object %s -- %s", obj->name, obj->short_descr );
    parse_logs( ch, "immortal", buf );
    return;
}

void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: '%s'\n\rArea: '%s'\n\r",
	location->name,
	location->area->name );
    send_to_char( buf, ch );

    sprintf( buf,
	"Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
	location->vnum,
	location->sector_type,
	location->light,
	location->heal_rate,
	location->mana_rate );
    send_to_char( buf, ch );

    sprintf( buf,
	"Room flags: %s.\n\rDescription:\n\r%s",
	flag_string( room_flags, location->room_flags ),
	location->description );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (( get_trust(ch) >= rch->ghost_level)
	&& (can_see(ch,rch)))
        {
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    sprintf( buf,
		"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

		door,
		(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
	    	pexit->key,
	    	pexit->exit_info,
	    	pexit->keyword,
	    	pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r" );
	    send_to_char( buf, ch );
	}
    }
    return;
}

void do_stat ( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *string;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *location;
    CHAR_DATA *victim;

    string = one_argument(argument, arg);

    if ( arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r"
		     "  stat <name>\n\r"
		     "  stat obj <name>\n\r"
		     "  stat mob <name>\n\r"
		     "  stat room <number>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"room"))
    {
	do_rstat(ch,string);
	return;
    }
  
    if (!str_cmp(arg,"obj"))
    {
	do_ostat(ch,string);
	return;
    }

    if (!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
    {
	do_mstat(ch,string);
	return;
    }

    obj = get_obj_world(ch,argument);

    if (obj != NULL)
    {
	do_ostat(ch,argument);
	return;
    }

    victim = get_char_world(ch,argument);

    if (victim != NULL)
    {
	do_mstat(ch,argument);
	return;
    }

    location = find_location(ch,argument);

    if (location != NULL)
    {
	do_rstat(ch,argument);
	return;
    }
    send_to_char("Nothing by that name found anywhere.\n\r",ch);
    return;
}

void do_damage_find( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int value;
    int nMatch;
    int items = 0;
    bool found;

    found       = FALSE;
    nMatch      = 0;

    if ( ( value = attack_lookup( argument ) ) == 0 )
    {
	send_to_char("Possible arguments are:\n\r",ch);
	show_help(ch,"weapon");
	return;
    }

    final = new_buf ( );

    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
        if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
        {
	    nMatch++;

	    if ( pObjIndex->item_type == ITEM_WEAPON
	    &&   pObjIndex->value[3] == value )
            {
                found = TRUE;
                sprintf( buf, "%3d> [%5d] %s",
                    items+1, pObjIndex->vnum,
		    end_string( pObjIndex->short_descr, 25 ) );
		add_buf( final, buf );

		if ( ++items % 3 == 0 )
		    add_buf( final, "\n" );
            }
        }
    }

    if ( items % 3 != 0 )
	add_buf( final, "\n" );

    if ( !found )
	act("No $t items found.\n\r",ch,argument,NULL,TO_CHAR,POS_DEAD);
    else
	page_to_char( final->string, ch );
    free_buf( final );
}

void do_damtype_find( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum, value, nMatch, items = 0;
    bool found;

    found       = FALSE;
    nMatch      = 0;

    if ( ( value = dam_type_lookup( argument ) ) == -1 )
    {
	sprintf( buf, "Invalid damage type.\nUse one of the following:\n " );
	for ( value = 0; value < DAM_MAX; value++ )
	{
	    strcat( buf, damage_mod_table[value].name );
	    strcat( buf, "\n " );
	}

	send_to_char( buf, ch );
	return;
    }

    final = new_buf( );

    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
        if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
        {
	    nMatch++;
	    if ( pObjIndex->item_type == ITEM_WEAPON
	    &&   attack_table[pObjIndex->value[3]].damage == value )
            {
                found = TRUE;
                sprintf( buf, "%3d> [%5d] %-10s | %s ",
                    items+1, pObjIndex->vnum, 
		    attack_table[pObjIndex->value[3]].name,
		    end_string( pObjIndex->short_descr, 25 ) );
		add_buf( final, buf );

		if ( ++items % 2 == 0 )
		    add_buf( final, "\n" );
            }
        }
    }

    if ( items % 2 != 0 )
	add_buf( final, "\n" );

    if ( !found )
	act("No $t items found.\n\r",ch,argument,NULL,TO_CHAR,POS_DEAD);
    else
	page_to_char( final->string, ch );

    free_buf( final );
}

void do_wtype_find( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum, value, nMatch = 0, items = 0;

    if ( argument[0] == '\0'
    ||   ( value = flag_value( weapon_class, argument ) ) == NO_FLAG )
    {
	send_to_char( "Possible arguments are:\n\r", ch );
	show_help( ch, "wclass" );
	return;
    }

    final = new_buf( );

    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
        if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
        {
	    nMatch++;
	    if ( pObjIndex->item_type == ITEM_WEAPON
	    &&   pObjIndex->value[0] == value )
            {
                sprintf( buf, "[%5d %3d] %s ",
                    pObjIndex->vnum, pObjIndex->level,
		    end_string( pObjIndex->short_descr, 25 ) );
		add_buf( final, buf );

		if ( ++items % 2 == 0 )
		    add_buf( final, "\n" );
            }
        }
    }

    if ( items % 2 != 0 )
	add_buf( final, "\n" );

    if ( items == 0 )
	act( "No $t items found.", ch, argument, NULL, TO_CHAR, POS_DEAD );
    else
    {
	sprintf( buf, "\n\rMatches found: %d.\n\r", items );
	add_buf( final, buf );
	page_to_char( final->string, ch );
    }

    free_buf( final );
}

void do_extra_find( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum, col = 0;
    int value;
    int nMatch;
    bool found;

    found       = FALSE;
    nMatch      = 0;

    if ( ( value = flag_value( extra_flags, argument ) ) == NO_FLAG )
    {
	send_to_char("Possible arguments are:\n\r",ch);
	show_help(ch,"extra");
	return;
    }

    final = new_buf( );

    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
        if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
        {
	    nMatch++;
            if ( IS_OBJ_STAT(pObjIndex,value) )
            {
                found = TRUE;
                sprintf( buf, "[%5d] %s",
                    pObjIndex->vnum,
		    end_string( pObjIndex->short_descr, 25 ) );
		add_buf( final, buf );

		if ( ++col % 3 == 0 )
		    add_buf( final, "\n" );
            }
        }
    }

    if ( col % 3 != 0 )
	add_buf( final, "\n" );

    if ( !found )
	act("No $t items found.\n\r",ch,argument,NULL,TO_CHAR,POS_DEAD);
    else
	page_to_char( final->string, ch );

    free_buf( final );
}

void do_wear_find( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum, col = 0;
    int value;
    int nMatch;
    bool found;

    found       = FALSE;
    nMatch      = 0;

    if ( ( value = flag_value( wear_flags, argument ) ) == NO_FLAG )
    {
	send_to_char("Possible arguments are:\n\r",ch);
	show_help(ch,"wear");
	return;
    }

    final = new_buf( );

    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
        if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
        {
	    nMatch++;
	    if ( IS_SET( pObjIndex->wear_flags, value ) )
            {
                found = TRUE;
                sprintf( buf, "[%5d] %s",
                    pObjIndex->vnum,
		    end_string( pObjIndex->short_descr, 25 ) );
		add_buf( final, buf );

		if ( ++col % 3 == 0 )
		    add_buf( final, "\n" );
            }
        }
    }

    if ( col % 3 != 0 )
	add_buf( final, "\n" );

    if ( !found )
	act("No $t items found.\n\r",ch,argument,NULL,TO_CHAR,POS_DEAD);
    else
	page_to_char( final->string, ch );

    free_buf( final );
}

void mfind( BUFFER *final, char *argument, bool header )
{
    MOB_INDEX_DATA *pMobIndex;
    char buf[MAX_STRING_LENGTH];
    extern int top_mob_index;
    int nMatch = 0, number = 0, vnum;
    bool fRandom, fBroken;

    if ( argument[0] == '\0' )
    {
	add_buf( final, "Find whom?\n\r" );
	return;
    }

    fRandom = !str_cmp( argument, "random_dam" );
    fBroken = !str_cmp( argument, "broken" );

    if ( header )
	add_buf( final, "Mobiles:\n" );

    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( is_name( argument, pMobIndex->player_name )
	    ||   ( fBroken && pMobIndex->bank_branch != 0 )
	    ||   ( fRandom && pMobIndex->dam_type == 0 ) )
	    {
		sprintf( buf, "[%5d] %s ", pMobIndex->vnum,
		    end_string( pMobIndex->short_descr, 30 ) );
		add_buf( final, buf );

		if ( ++number % 2 == 0 )
		    add_buf( final, "\n" );
	    }
	}
    }

    if ( number == 0 )
	add_buf( final, "No mobiles by that name.\n\r" );

    else if ( number % 2 != 0 )
	add_buf( final, "\n" );
}

void rfind( BUFFER *final, char *argument, bool header )
{
    ROOM_INDEX_DATA *pRoom;
    char buf[MAX_STRING_LENGTH];
    extern int top_room;
    int col = 0, match = 0, vnum;

    if ( argument[0] == '\0' )
    {
	add_buf( final, "Find where?\n\r" );
	return;
    }

    if ( header )
	add_buf( final, "\nRooms:\n" );

    for ( vnum = 0; match < top_room; vnum++ )
    {
	if ( ( pRoom = get_room_index( vnum ) ) != NULL )
	{
	    match++;

	    if ( is_name( argument, strip_color( pRoom->name ) ) )
	    {
		sprintf( buf, "[%5d] %s ", vnum,
		    end_string( pRoom->name, 30 ) );
		add_buf( final, buf );

		if ( ++col % 2 == 0 )
		    add_buf( final, "\n" );
	    }
	}
    }

    if ( col == 0 )
	add_buf( final, "No rooms by that name.\n\r" );	

    else if ( col % 2 != 0 )
	add_buf( final, "\n" );
}

void ofind( BUFFER *final, char *argument, bool header )
{
    OBJ_INDEX_DATA *pObjIndex;
    char buf[MAX_STRING_LENGTH];
    extern int top_obj_index;
    int nMatch = 0, number = 0, vnum;
    bool fRandom;

    if ( argument[0] == '\0' )
    {
	add_buf( final, "Find what?\n\r" );
	return;
    }

    fRandom = !str_cmp( argument, "random_dam" );

    if ( header )
	add_buf( final, "\nObjects:\n" );

    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;

	    if ( is_name( argument, pObjIndex->name )
	    ||   ( fRandom && pObjIndex->item_type == ITEM_WEAPON
	    &&     pObjIndex->value[3] == 0 ) )
	    {
		sprintf( buf, "[%5d] %s ", pObjIndex->vnum,
		    end_string( pObjIndex->short_descr, 30 ) );
		add_buf( final, buf );

		if ( ++number % 2 == 0 )
		    add_buf( final, "\n" );
	    }
	}
    }

    if ( number == 0 )
	add_buf( final, "No objects by that name.\n\r" );

    else if ( number % 2 != 0 )
	add_buf( final, "\n" );
}

void do_rflag_find( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    extern int top_room;
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *pRoomIndex;
    int col = 0, nMatch = 0, value, vnum;
    bool found = FALSE;

    if ( ( value = flag_value( room_flags, argument ) ) == NO_FLAG )
    {
	send_to_char("Possible arguments are:\n\r",ch);
	show_help(ch,"room");
	return;
    }

    final = new_buf( );

    for ( vnum = 0; nMatch < top_room; vnum++ )
    {
 	if ( ( pRoomIndex = get_room_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( IS_SET(pRoomIndex->room_flags, value) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s",
		    pRoomIndex->vnum,
		    end_string( pRoomIndex->name, 25 ) );
		add_buf( final, buf );

		if ( ++col % 3 == 0 )
		    add_buf( final, "\n" );
	    }
	}
    }

    if ( col % 3 != 0 )
	add_buf( final, "\n" );

    if ( !found )
	act( "No $t rooms detected.",ch, argument, NULL, TO_CHAR, POS_DEAD );
    else
	page_to_char( final->string, ch );

    free_buf( final );
}

void do_spec_find( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    MOB_INDEX_DATA *pMobIndex;
    SPEC_FUN *spec;
    char buf[MAX_STRING_LENGTH];
    extern int top_mob_index;
    int nMatch, vnum, col = 0;
    bool found;

    found       = FALSE;
    nMatch      = 0;

    if ( ( spec = spec_lookup( argument ) ) == NULL )
    {
	send_to_char("Error looking up spec, try one of these.\n\r",ch);
	show_help(ch,"spec");
	return;
    }

    final = new_buf( );

    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
        if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
        {
	    nMatch++;

	    if ( pMobIndex->spec_fun == spec_lookup( argument ) )
            {
                found = TRUE;
                sprintf( buf, "[%5d] %s",
                    pMobIndex->vnum,
		    end_string( pMobIndex->short_descr, 25 ) );
		add_buf( final, buf );

		if ( ++col % 3 == 0 )
		    add_buf( final, "\n" );
            }
        }
    }

    if ( col % 3 != 0 )
	add_buf( final, "\n" );

    if ( !found )
	act("No $t specs found.\n\r",ch,argument,NULL,TO_CHAR,POS_DEAD);
    else
	page_to_char( final->string, ch );

    free_buf( final );
}

void do_slookup( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    char buf[MAX_STRING_LENGTH];
    bool fAll;
    int found = 0, sn;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Lookup which skill or spell?\n\r", ch );
	return;
    }

    fAll = !str_cmp( argument, "all" );

    final = new_buf( );

    for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
    {
	if ( fAll || !str_prefix( argument, skill_table[sn].name )
	||   ( !str_cmp( argument, "spells" )
	&&     skill_table[sn].spell_fun != spell_null )
	||   ( !str_cmp( argument, "skills" )
	&&     skill_table[sn].spell_fun == spell_null ) )
	{
	    sprintf( buf, "%3d %-22s",
		sn, skill_table[sn].name );
	    add_buf( final, buf );

	    if ( ++found % 3 == 0 )
		add_buf( final, "\n\r" );
	}
    }

    if ( found == 0 )
	send_to_char( "No skills were found matching that name.\n\r", ch );
    else
    {
	if ( found % 3 != 0 )
	    sprintf( buf, "\n\r\n\r%d matches found.\n\r", found );
	else
	    sprintf( buf, "\n\r%d matches found.\n\r", found );
	add_buf( final, buf );
	page_to_char( final->string, ch );
    }
    free_buf( final );
}

void do_act_find( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    MOB_INDEX_DATA *pMobIndex;
    char buf[MAX_STRING_LENGTH];
    extern int top_mob_index;
    int nMatch = 0, number = 0, value, vnum;

    if ( argument[0] == '\0'
    ||   ( value = flag_value( act_flags, argument ) ) == NO_FLAG )
    {
	send_to_char("Possible arguments are:\n\r",ch);
	show_help(ch,"act");
	return;
    }

    final = new_buf( );

    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( IS_SET( pMobIndex->act, value ) )
	    {
		sprintf( buf, "[%5d] %s ", pMobIndex->vnum,
		    end_string( pMobIndex->short_descr, 30 ) );
		add_buf( final, buf );

		if ( ++number % 2 == 0 )
		    add_buf( final, "\n" );
	    }
	}
    }

    if ( number == 0 )
	add_buf( final, "No mobiles with that act flag.\n\r" );

    else if ( number % 2 != 0 )
	add_buf( final, "\n" );

    page_to_char( final->string, ch );
    free_buf( final );
}

void do_vnum( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea1, *pArea2;
    BUFFER *output;
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    char *string;
    int iArea, iAreaHalf;

    string = one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
	send_to_char(	"Syntax:\n\r"
			"  vnum areas\n\r"
			"  vnum obj <name>|\"random_dam\"\n\r"
			"  vnum mob <name>|\"random_dam\"\n\r"
			"  vnum act <act flag>\n\r"
			"  vnum room <room name>\n\r"
			"  vnum rflag <room flag>\n\r"
			"  vnum extra <extra flag>\n\r"
			"  vnum wear <wear flag>\n\r"
			"  vnum spec <spec_program>\n\r"
			"  vnum damage <damage_noun>\n\r"
			"  vnum dam_type <damage_type>\n\r"
			"  vnum wtype <weapon type>\n\r"
			"  vnum skill <skill or spell>|\"all\"|\"skills\"|\"spell\"\n\r", ch );
    }

    else if ( !str_cmp( arg, "damage" ) )
	do_damage_find( ch, string );

    else if ( !str_cmp( arg, "act" ) )
	do_act_find( ch, string );

    else if ( !str_cmp( arg, "wtype" ) )
	do_wtype_find( ch, string );

    else if ( !str_cmp( arg, "dam_type" ) )
	do_damtype_find( ch, string );

    else if ( !str_cmp( arg, "obj" ) )
    {
	output = new_buf( );
	ofind( output, string, FALSE );
	page_to_char( output->string, ch );
	free_buf( output );
    }

    else if ( !str_cmp( arg, "mob" ) || !str_cmp( arg, "char" ) )
    {
	output = new_buf( );
	mfind( output, string, FALSE );
	page_to_char( output->string, ch );
	free_buf( output );
    }

    else if ( !str_cmp( arg, "skill" ) || !str_cmp( arg, "spell" ) )
	do_slookup( ch, string );

    else if ( !str_cmp( arg, "spec" ) )
	do_spec_find( ch, string );

    else if ( !str_cmp( arg, "room" ) )
    {
	output = new_buf( );
	rfind( output, string, FALSE );
	page_to_char( output->string, ch );
	free_buf( output );
    }

    else if ( !str_cmp( arg, "rflag" ) )
	do_rflag_find( ch, string );

    else if ( !str_cmp( arg, "extra" ) )
	do_extra_find( ch, string );

    else if ( !str_cmp( arg, "wear" ) )
	do_wear_find( ch, string );

    else if ( !str_cmp( arg, "areas" ) || !str_cmp( arg, "area" ) )
    {
	char area1[MAX_INPUT_LENGTH];

	iAreaHalf = (top_area + 1) / 2;
	pArea1    = area_first;
	pArea2    = area_first;
	for ( iArea = 0; iArea < iAreaHalf; iArea++ )
	    pArea2 = pArea2->next;

	for ( iArea = 0; iArea < iAreaHalf; iArea++ )
	{
	    sprintf( area1, "%s", end_string( pArea1->name, 28 ) );

	    sprintf( buf, "%s {R%5d %5d{x  %s {R%5d %5d{x\n\r",
		area1, pArea1->min_vnum, pArea1->max_vnum,
		(pArea2 != NULL) ? end_string( pArea2->name, 28 ) :
		"                            ",
		(pArea2 != NULL) ? pArea2->min_vnum : 0,
		(pArea2 != NULL) ? pArea2->max_vnum : 0 );
	    send_to_char(buf,ch);

	    pArea1 = pArea1->next;
	    if ( pArea2 != NULL )
		pArea2 = pArea2->next;
	}
    }

    else
    {
	output = new_buf( );
	mfind( output, argument, TRUE );
	ofind( output, argument, TRUE );
//	rfind( output, argument, TRUE );
	page_to_char( output->string, ch );
	free_buf( output );
    }
}

void do_mlevel( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    MOB_INDEX_DATA *pMobIndex;
    char buf[MAX_STRING_LENGTH];
    extern int top_mob_index;
    int nMatch = 0, number = 0, vnum, level;

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax: mlevel <level>\n\r", ch );
	return;
    }

    if ( ( level = atoi( argument ) ) < 0 )
    {
	send_to_char( "Invalid level.\n\r", ch );
	return;
    }

    final = new_buf( );

    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( pMobIndex->level == level )
	    {
		sprintf( buf, "%3d) %5d] %s ", number+1, pMobIndex->vnum,
		    end_string( pMobIndex->short_descr, 25 ) );
		add_buf( final, buf );

		if ( ++number % 2 == 0 )
		    add_buf( final, "\n" );
	    }
	}
    }

    if ( number == 0 )
	add_buf( final, "No mobiles by that name.\n\r" );

    else if ( number % 2 != 0 )
	add_buf( final, "\n" );

    page_to_char( final->string, ch );
    free_buf( final );
}

void do_olevel( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    extern int top_obj_index;
    int match, pos, value, vnum;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' || !is_number(argument) )
    {
	send_to_char("Syntax: olevel all <level>\n\r",ch);
	send_to_char("Syntax: olevel world <level>\n\r",ch);
	return;
    }

    value = atoi( argument );

    if ( value < 0 || value > ch->level )
    {
	sprintf(buf,"Level must be a number between 0 and %d.\n\r",
	    ch->level);
	send_to_char(buf,ch);
	do_olevel(ch,"");
	return;
    }

    if (!str_prefix(arg,"all"))
    {
	pos = 0;
	match = 0;
	final = new_buf();

	for ( vnum = 0; match < top_obj_index; vnum++ )
	{
	    if ( (pObj = get_obj_index(vnum)) != NULL)
	    {
		match++;
		if ( pObj->level == value )
		{
		    pos++;
		    found = TRUE;
		    sprintf(buf,"%3d> [%5d] %s\n\r", pos, pObj->vnum,
			pObj->short_descr);
		    add_buf(final,buf);
		}
	    }
	}
	if (!found)
	{
	    sprintf(buf,"No objects matching level %d exist.\n\r", value);
	    send_to_char(buf,ch);
	}
	else
	    page_to_char(final->string,ch);

	free_buf(final);
	return;
    }

    else if (!str_prefix(arg,"world"))
    {
	pos = 0;
	final = new_buf();

	for (obj = object_list; obj != NULL; obj = obj->next)
	{
	    if (obj->level == value)
	    {
		pos++;
		found = TRUE;
		sprintf(buf,"%3d> [%5d] %s\n\r", pos, obj->pIndexData->vnum,
		    obj->short_descr);
		add_buf(final,buf);
	    }
	}

	if (!found)
	{
	    sprintf(buf,"No objects matching level %d could be located on the MUD.\n\r",
		value);
	    send_to_char(buf,ch);
	}
	else
	    page_to_char(final->string,ch);

	free_buf(final);
	return;
    }

    do_olevel(ch,"");
    return;
}

void do_owhere( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    OBJ_DATA *obj, *in_obj;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH], obj_name[MAX_INPUT_LENGTH];
    int number = 0, vnum = 0;

    if ( argument[0] == '\0' )
    {
	send_to_char(	"Syntax: owhere <item name>\n\r"
			"        owhere <item vnum>\n\r", ch );
	if ( IS_TRUSTED( ch, IMPLEMENTOR ) )
	    send_to_char( "        owhere loaded\n\r"
			  "        owhere loaded all (include those carried by immortals.)\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( IS_TRUSTED( ch, IMPLEMENTOR ) && !str_cmp( arg, "loaded" ) )
    {
	bool fAll;

	argument = one_argument( argument, arg );

	final = new_buf( );
	fAll = !str_cmp( argument, "all" );

	for ( obj = object_list; obj != NULL; obj = obj->next )
	{
	    if ( obj->loader == NULL )
		continue;

	    for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
		;

	    if ( !fAll
	    &&   in_obj->carried_by != NULL
	    &&   IS_IMMORTAL( in_obj->carried_by ) )
		continue;

	    number++;

	    if ( in_obj->carried_by != NULL
	    &&   in_obj->carried_by->in_room != NULL )
		sprintf( buf, "%3d) [%-35s] [%s] Carried: %s\n\r",
		    number, obj->loader, end_string( obj->short_descr, 30 ),
		    PERS( in_obj->carried_by, ch ) );

	    else if ( in_obj->in_room != NULL )
		sprintf( buf, "%3d) [%-35s] [%s] Room: %s <%d>\n\r",
		    number, obj->loader, end_string( obj->short_descr, 30 ),
		    in_obj->in_room->name, in_obj->in_room->vnum );

	    else if ( in_obj->dropped_by != NULL )
		sprintf( buf, "%3d) [%-35s] [%s] Storage: %s\n\r",
		    number, obj->loader, end_string( obj->short_descr, 30 ),
		    PERS( in_obj->dropped_by, ch ) );

	    else
		sprintf( buf, "%3d) [%-35s] [%s] Somewhere\n\r",
		    number, obj->loader, end_string( obj->short_descr, 30 ) );

	    add_buf( final, buf );
        }

	if ( number == 0 )
	    add_buf( final, "Zero loaded objects found.\n\r" );

	page_to_char( final->string, ch );
	free_buf( final );
        return;
    }
 
    if ( is_number( argument ) )
    {
	if ( ( vnum = atoi( argument ) ) < 0 )
	{
	    send_to_char( "Invalid vnum.\n\r", ch );
	    return;
	}

	if ( get_obj_index( vnum ) == NULL )
	{
	    send_to_char( "That object vnum does not exist.\n\r", ch );
	    return;
	}
    }

    final = new_buf( );

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj )
	||   ch->level < obj->level
	||   ( vnum == 0 && !is_name( argument, obj->name ) )
	||   ( vnum != 0 && obj->pIndexData->vnum != vnum ) )
	    continue;

	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	if ( ( in_obj->carried_by != NULL
	&&     !can_see( ch, in_obj->carried_by ) )
	||   ( in_obj->carried_by == NULL
	&&     in_obj->dropped_by != NULL
	&&     !can_see( ch, in_obj->dropped_by ) )
	||   ( in_obj->in_room != NULL
	&&     !can_see_room( ch, in_obj->in_room ) ) )
	    continue;

	number++;

	sprintf( obj_name, "%s", end_string( obj->short_descr, 27 ) );

	if ( in_obj->carried_by != NULL
	&&   in_obj->carried_by->in_room != NULL )
	    sprintf( buf, "%3d) %s Carried By %s Room %d\n\r",
		number, obj_name, end_string( PERS( in_obj->carried_by, ch ), 25 ),
		in_obj->carried_by->in_room->vnum );

	else if ( in_obj->in_room != NULL )
	    sprintf( buf, "%3d) %s In Room    %s Room %d\n\r",
		number, obj_name,
		end_string( in_obj->in_room->name, 25 ),in_obj->in_room->vnum );

	else if ( in_obj->dropped_by != NULL )
	    sprintf( buf, "%3d) %s Storage by %s.\n\r",
		number, obj_name, in_obj->dropped_by->name );

	else
	    sprintf( buf, "%3d) %s (Somewhere).\n\r",
		number, obj_name );

	add_buf( final, buf );
    }

    if ( number == 0 )
	add_buf( final, "Nothing like that in heaven or earth.\n\r" );

    page_to_char( final->string, ch );
    free_buf( final );
}

void do_mwhere( CHAR_DATA *ch, char *argument )
{
    BUFFER *final = new_buf();
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char argall[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    bool found, byVnum = FALSE;
    bool byName = FALSE;
    bool byGreater = FALSE;
    bool byLess = FALSE;
    bool byEqual = FALSE;
    bool Overflow = FALSE;
    int count = 0;
    int numarg = 0;

    if ( argument[0] == '\0' && IS_TRUSTED(ch,MAX_LEVEL-1) )
    {
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->character != NULL && d->connected == CON_PLAYING
	    &&  d->character->in_room != NULL && can_see(ch,d->character)
	    &&  can_see_room(ch,d->character->in_room))
	    {
		victim = d->character;
		if (can_over_ride(ch,victim,TRUE))
		{
		    count++;
		    if (d->original != NULL)
			sprintf(buf,"%3d) %s (in the body of %s) is in %s [%d]\n\r",
			count, d->original->name,victim->short_descr,
			victim->in_room->name,victim->in_room->vnum);
		    else
			sprintf(buf,"%3d) %s is in %s [%d]\n\r",
			count, victim->name,victim->in_room->name,
			victim->in_room->vnum);
		    add_buf(final,buf);
		}
	    }
	}
	sprintf(buf,"mwhere searches for %s",argument);
	parse_logs( ch, "immortal", buf );

	page_to_char(final->string,ch);
	free_buf(final);

	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char("Mwhere find who?\n\r",ch);
	return;
    }

    found = FALSE;
    
    sprintf(argall, "%s", argument);
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    
    if(!str_cmp(arg1,"vnum"))
    {
        if (arg2[0] == '\0')
        {
            send_to_char("You have to specify a vnum.\n\r", ch);
            return;
        }
        if ( !is_number(arg2) )
        {
            send_to_char("Invalid vnum.\n\r", ch);
            return;
        }

        numarg = atoi(arg2);
        byVnum = TRUE;
    }

    else if (!str_cmp(arg1,"mob"))
    {
	if (arg2[0] == '\0' || arg3[0] == '\0')
	{
	    send_to_char("Syntax: mwhere mob >|<|== <level>\n\r",ch);
	    return;
	}
        if ( !is_number(arg3) )
        {
            send_to_char("Level has to be numerical.\n\r",ch);
            return;
        }
        if ( !str_cmp(arg2,">") )
            byGreater = TRUE;
        else if ( !str_cmp(arg2,"<") )
            byLess = TRUE;
        else if ( !str_cmp(arg2,"==") )
            byEqual = TRUE;
	else
	{
	    send_to_char("Invalid operator\n\r",ch);
	    return;
	}

	numarg = atoi(arg3);
    } else
	byName = TRUE;

    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
	if ( victim->in_room != NULL
	&&   can_see(ch, victim) )
	{
            if ( (is_name( argall, victim->name ) && byName)
            ||   (IS_NPC(victim)
            &&    ((byVnum && numarg == victim->pIndexData->vnum)
            ||     (byGreater && victim->level > numarg)
            ||     (byLess && victim->level < numarg)
            ||     (byEqual && victim->level == numarg)))  )
	    {
	        if ((!IS_NPC(victim) && can_over_ride(ch,victim,TRUE)) 
	        ||  IS_NPC(victim))
	        {
		    found = TRUE;
       		    count++;
		    sprintf( buf, "%3d) [%5d] %s [%5d] %s\n\r", count,
		        IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		        end_string(IS_NPC(victim) ? victim->short_descr :
		        victim->name,28),
		        victim->in_room->vnum,
		        victim->in_room->name );
		    if ( count <= 1200 )
		        add_buf(final,buf);
		    else
			Overflow = TRUE;
		}
	   }
        }
    }

    if ( Overflow )
    {
        sprintf(buf, "{RList truncated - Too many Items: %d (Max = 1200){x\n\r", count);
	add_buf(final,buf);
    }

    if ( !found && byVnum == FALSE )
	act( "You didn't find any $T.", ch, NULL, argall, TO_CHAR,POS_RESTING);
    else if ( !found && byVnum == TRUE )
	act( "You didn't find any NPCs with the vnum $T.", ch, NULL, arg2, TO_CHAR,POS_RESTING);
    else if ( !found && byGreater == TRUE )
	act( "You didn't fiund any NPCs with a level higher than $T.", ch, NULL, arg3, TO_CHAR, POS_RESTING);
    else if ( !found && byLess == TRUE )
        act( "You didn't fiund any NPCs with a level lower than $T.", ch, NULL, arg3, TO_CHAR, POS_RESTING);
    else if ( !found && byEqual == TRUE )
        act( "You didn't fiund any NPCs with a level equal to $T.", ch, NULL, arg3, TO_CHAR, POS_RESTING);
    else if ( final->string[0] != '\0' )
	page_to_char(final->string,ch);

    free_buf(final);

    sprintf(buf,"mwhere searches for %s",argument);
    parse_logs( ch, "immortal", buf );

    return;
}

void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}

void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}

void do_shutdown( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d,*d_next;
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    extern int port;

    prepare_reboot( );

    sprintf( buf, "Shutdown by %s.", ch->name );

    if ( port == MAIN_GAME_PORT )
	append_file( ch, "shutdown.txt", buf );

    strcat( buf, "\n\r" );

    do_echo( ch, buf );

    do_save( ch, "" );

    for ( d = descriptor_list; d != NULL; d = d_next)
    {
	d_next = d->next;
	close_socket( d );
    }

    merc_down = TRUE;

    return;
}

void do_snoop( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_pc_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );

	wiznet( "$N stops being such a snoop.",
	    ch, NULL, WIZ_SNOOPS, WIZ_SECURE, get_trust( ch ) );
	parse_logs( ch, "immortal", "stops snooping." );

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}

	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if ( !can_over_ride( ch, victim, FALSE ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;

    sprintf( buf, "$N starts snooping on %s",
	( IS_NPC( victim ) ? victim->short_descr : victim->name ) );
    wiznet( buf, ch, NULL, WIZ_SNOOPS, WIZ_SECURE, get_trust( ch ) );

    sprintf( buf, "starts snooping %s", victim->name );
    parse_logs( ch, "immortal", buf );

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    sprintf(buf,"$N switches into %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
    sprintf(buf,"switches into %s--%s",victim->name,victim->short_descr);
    parse_logs( ch, "immortal", buf );

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    ch->timer		= -5000;
    /* change communications to match */
    if (ch->prompt != NULL)
        victim->prompt = str_dup(ch->prompt);
    victim->comm = ch->comm;
    if (IS_SET(ch->act, PLR_COLOUR))
	SET_BIT(victim->act,PLR_COLOUR);
    send_to_char( "Ok.\n\r", victim );
    return;
}

void do_return( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    if (ch->prompt != NULL)
    {
	free_string(ch->prompt);
	ch->prompt = NULL;
    }

    if ( ch->desc->original->pcdata && ch->desc->original->pcdata->tells )
    {
	sprintf( buf, "Switch mode removed.  You have {R%d{x tell%s waiting.\n\r",
	    ch->desc->original->pcdata->tells,
	    ch->desc->original->pcdata->tells == 1 ? "" : "s" );
	send_to_char( buf, ch );
	send_to_char("Type '{Rreplay{x' to see tells.\n\r",ch);
    }
    else
	send_to_char("Switch mode removed.  You have no tells waiting.\n\r",ch);

    sprintf(buf,"$N returns from %s.",ch->short_descr);
    wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch->desc->original));
    sprintf(buf,"returns from %s--%s.",ch->name,ch->short_descr);
    parse_logs( ch->desc->original, "immortal", buf );
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc; 
    ch->desc->character->timer= 0;
    ch->desc                  = NULL;
    return;
}

bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (IS_TRUSTED(ch,GOD)
	|| (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 105)
	|| (IS_TRUSTED(ch,DEMI)	    && obj->level <= 100)
	|| (IS_TRUSTED(ch,KNIGHT)   && obj->level <= 20)
	|| (IS_TRUSTED(ch,SQUIRE)   && obj->level ==  5))
	return TRUE;
    else
	return FALSE;
}

void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;

    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData);

	    set_obj_loader( ch, t_obj, "ICLN" );

	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

void do_clone(CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch,NULL,rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch,NULL,rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch,NULL,argument);
	obj = get_obj_here(ch,NULL,argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    if (obj != NULL)
    {
	OBJ_DATA *clone;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}
	if (obj->item_type == ITEM_EXIT)
	{
	    send_to_char("You cannot clone an exit object.\n\r",ch);
	    return;
	}
	clone = create_object(obj->pIndexData);
	clone_object(obj,clone);

	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	{
	    set_arena_obj( ch, clone );
	    obj_to_room(clone,ch->in_room);
	}
 	recursive_clone(ch,obj,clone);

	set_obj_loader( ch, clone, "CLON" );

	act("$n has created $p.",ch,clone,NULL,TO_ROOM,POS_RESTING);
	act("You clone $p.",ch,clone,NULL,TO_CHAR,POS_RESTING);
	wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	sprintf(buf,"has cloned %s -- %s.",obj->name,obj->short_descr);
	parse_logs( ch, "immortal", buf );
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;
	char buf[MAX_STRING_LENGTH];

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	if ((mob->level > 100 && !IS_TRUSTED(ch,GOD))
	||  (mob->level > 90 && !IS_TRUSTED(ch,IMMORTAL))
	||  (mob->level > 85 && !IS_TRUSTED(ch,DEMI))
	||  (mob->level >  0 && !IS_TRUSTED(ch,KNIGHT))
	||  !IS_TRUSTED(ch,SQUIRE))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);
	clone_mobile(mob,clone); 
	
	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_obj = create_object(obj->pIndexData);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room);
        act("$n has created $N.",ch,NULL,clone,TO_ROOM,POS_RESTING);
        act("You clone $N.",ch,NULL,clone,TO_CHAR,POS_RESTING);
	sprintf(buf,"$N clones %s.",clone->short_descr);
	wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	sprintf(buf,"clones %s -- %s.",clone->name,clone->short_descr);
	parse_logs( ch, "immortal", buf );
        return;
    }
}

void do_mload( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim = NULL;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    sh_int count, pos;

    count = mult_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number( arg ) )
    {
	send_to_char( "Syntax: load mob [#]*<vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    if ( count < 0 || count > 50 )
    {
	send_to_char( "Multi count load range is 1 to 50.\n\r", ch );
	return;
    }

    else if ( count == 0 )
	count = 1;

    for ( pos = 0; pos < count; pos++ )
    {
	victim = create_mobile( pMobIndex );
	char_to_room( victim, ch->in_room );
    }

    if ( count == 1 )
    {
	act( "You have created $N.", ch, NULL, victim, TO_CHAR, POS_DEAD );
	act( "$n has created $N!", ch, NULL, victim, TO_ROOM, POS_RESTING );
	sprintf( buf, "$N loads %s.", victim->short_descr );
	wiznet( buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, get_trust( ch ) );
	sprintf( buf, "loaded %s -- %s.", victim->name, victim->short_descr );
	parse_logs( ch, "immortal", buf );
    } else {
	sprintf( buf, "You have created [%d] * $N.", count );
	act( buf, ch, NULL, victim, TO_CHAR, POS_DEAD );
	sprintf( buf, "$n has created [%d] * $N!", count );
	act( buf, ch, NULL, victim, TO_ROOM, POS_RESTING );
	sprintf( buf, "$N loads [%d] * %s.", count, victim->short_descr );
	wiznet( buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, get_trust( ch ) );
	sprintf( buf, "loaded [%d] * %s -- %s.",
	    count, victim->name, victim->short_descr );
	parse_logs( ch, "immortal", buf );
    }

    return;
}

void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj = NULL;
    sh_int count, pos;

    count = mult_argument( argument, arg );
    
    if ( arg[0] == '\0' || !is_number( arg ) )
    {
	send_to_char( "Syntax: load obj [#]*<vnum>.\n\r", ch );
	return;
    }
    
    if ( ( pObjIndex = get_obj_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    if ( pObjIndex->item_type == ITEM_EXIT )
    {
	send_to_char( "You cannot load an exit object.\n\r", ch );
	return;
    }

    if ( count < 0 || count > 50 )
    {
	send_to_char( "Multi count load range is 1 to 50.\n\r", ch );
	return;
    }

    else if ( count == 0 )
	count = 1;

    for ( pos = 0; pos < count; pos++ )
    {
	obj = create_object( pObjIndex );

	set_obj_loader( ch, obj, "LOAD" );

	if ( CAN_WEAR( obj, ITEM_TAKE ) )
	    obj_to_char( obj, ch );
	else
	{
	    set_arena_obj( ch, obj );
	    obj_to_room( obj, ch->in_room );
	}
    }

    if ( count == 1 )
    {
	act( "$n has created $p!", ch, obj, NULL, TO_ROOM, POS_RESTING );
	wiznet( "$N loads $p.", ch, obj, WIZ_LOAD, 0, get_trust( ch ) );
	sprintf( buf, "loads object %s -- %s.", obj->name, obj->short_descr );
	parse_logs( ch, "immortal", buf );
	act( "You have created $p!", ch, obj, NULL, TO_CHAR, POS_DEAD );
    } else {
	sprintf( buf, "$n has created [%d] * $p!", count );
	act( buf, ch, obj, NULL, TO_ROOM, POS_RESTING );
	sprintf( buf, "$N loads [%d] * $p.", count );
	wiznet( buf, ch, obj, WIZ_LOAD, 0, get_trust( ch ) );
	sprintf( buf, "loads [%d] * object %s -- %s.",
	    count, obj->name, obj->short_descr );
	parse_logs( ch, "immortal", buf );
	sprintf( buf, "You have created [%d] * $p!", count );
	act( buf, ch, obj, NULL, TO_CHAR, POS_DEAD );
    }

    return;
}

void do_vload( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char *name;
   
    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0')
    {
        send_to_char( "Syntax: load voodoo <player>\n\r", ch );
        return;
    }
    
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *wch;

	if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
	    continue;

	wch = ( d->original != NULL ) ? d->original : d->character;

	if ( !can_see( ch, wch )
	||   !can_over_ride( ch, wch, TRUE ) )
	    continue;

	if ( !str_prefix( arg1, wch->name ) )
	{
	    if ( IS_NPC( wch ) )
		continue;

	    if ( ( pObjIndex = get_obj_index( OBJ_VNUM_VOODOO ) ) == NULL )
	    {
		send_to_char( "Cannot find the voodoo doll vnum.\n\r", ch );
		return;
	    }

	    obj = create_object( pObjIndex );
	    name = wch->name;
	    sprintf( buf, obj->short_descr, name );
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( buf );
	    sprintf( buf, obj->description, name );
	    free_string( obj->description );
	    obj->description = str_dup( buf );
	    sprintf( buf, obj->name, name );
	    free_string( obj->name );
	    obj->name = str_dup( buf );

	    set_obj_loader( ch, obj, "LOAD" );

	    if ( CAN_WEAR( obj, ITEM_TAKE ) )
		obj_to_char( obj, ch );
	    else
	    {
		set_arena_obj( ch, obj );
		obj_to_room( obj, ch->in_room );
	    }

	    act( "$n has created $p!", ch, obj, NULL, TO_ROOM, POS_RESTING );
	    wiznet( "$N loads $p.", ch, obj, WIZ_LOAD, WIZ_SECURE, get_trust( ch ) );
	    sprintf( buf, "loads voodoo %s -- %s.", obj->name, obj->short_descr );
	    parse_logs( ch, "immortal", buf );
	    act( "You have created $p!", ch, obj, NULL, TO_CHAR, POS_DEAD );
	    return;
	}
    }

    send_to_char( "No one of that name is playing.\n\r", ch );
    return;
}

void do_rand1load( CHAR_DATA *ch , char *argument )
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    int level;
	
    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax: load rand1 <level>.\n\r", ch );
	return;
    }

    level = atoi( argument );
    if ( level < 1 || level > 200 )
    {
	send_to_char( "Level must be be between 1 and 200.\n\r", ch );
	return;
    }

    if ( ( obj = rand_obj( level ) ) == NULL )
    {
	send_to_char( "Object load aborted.\n\r", ch );
	return;
    }

    set_obj_loader( ch, obj, "RAN1" );

    obj_to_char( obj, ch );

    act( "$n has created $p!", ch, obj, NULL, TO_ROOM, POS_RESTING );
    wiznet( "$N loads $p.", ch, obj, WIZ_LOAD, 0, get_trust( ch ) );
    sprintf( buf, "rand1 loads object %s -- %s.", obj->name, obj->short_descr );
    parse_logs( ch, "immortal", buf );
    act( "You have created $p!", ch, obj, NULL, TO_CHAR, POS_DEAD );
}

void do_rand2load( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
	
    if ( ( obj = rand_obj2( ch, argument ) ) == NULL )
	return;

    set_obj_loader( ch, obj, "RAN2" );

    obj_to_char( obj, ch );

    act( "$n has created $p!", ch, obj, NULL, TO_ROOM, POS_RESTING );
    wiznet( "$N loads $p.", ch, obj, WIZ_LOAD, 0, get_trust( ch ) );
    sprintf( buf, "rand2 loads object %s -- %s.", obj->name, obj->short_descr );
    parse_logs( ch, "immortal", buf );
    act( "You have created $p!", ch, obj, NULL, TO_CHAR, POS_DEAD );
}

/*
void do_rand3load( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
	
    if ( ( obj = rand_obj3( ch, argument ) ) == NULL )
	return;

    set_obj_loader( ch, obj, "RAN3" );

    obj_to_char( obj, ch );

    act( "$n has created $p!", ch, obj, NULL, TO_ROOM, POS_RESTING );
    wiznet( "$N loads $p.", ch, obj, WIZ_LOAD, 0, get_trust( ch ) );
    sprintf( buf, "rand3 loads object %s -- %s.", obj->name, obj->short_descr );
    parse_logs( ch, "immortal", buf );
    act( "You have created $p!", ch, obj, NULL, TO_CHAR, POS_DEAD );
}
*/

void do_load( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r", ch );
	send_to_char( "  load mob [#]*<vnum>\n\r", ch );
	send_to_char( "  load obj [#]*<vnum>\n\r", ch );

	if ( IS_TRUSTED( ch, CREATOR ) )
	{
	    send_to_char( "  load voodoo <player>\n\r", ch);
	    send_to_char( "  load rand1 <args>\n\r", ch );
	    send_to_char( "  load rand2 <args>\n\r", ch );
//	    send_to_char( "  load rand3 <args>\n\r", ch );
	}

	return;
    }

    if ( !str_cmp( arg, "mob" ) || !str_cmp( arg, "char" ) )
    {
	do_mload( ch, argument );
	return;
    }

    if ( !str_cmp( arg, "obj" ) )
    {
	do_oload( ch, argument );
	return;
    }

    if ( IS_TRUSTED( ch, CREATOR ) )
    {
	if ( !str_cmp( arg, "voodoo" ) )
	{
	    do_vload( ch, argument );
	    return;
	}

	if ( !str_cmp( arg, "rand1" ) )
	{
	    do_rand1load( ch, argument );
	    return;
	}

	if ( !str_cmp( arg, "rand2" ) )
	{
	    do_rand2load( ch, argument );
	    return;
	}
/*
	if ( !str_cmp( arg, "rand3" ) )
	{
	    do_rand3load( ch, argument );
	    return;
	}
*/
    }

    do_load( ch, "" );
    return;
}

void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE) 
	    &&   victim != ch /* safety precaution */ )
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (!IS_OBJ_STAT(obj,ITEM_NOPURGE)
	    &&  !IS_OBJ_STAT(obj,ITEM_AQUEST)
	    &&  !IS_OBJ_STAT(obj,ITEM_FORGED) )
	      extract_obj( obj );
	}

	act( "$n purges the room!", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
	if (ch == victim)
	{
	    send_to_char("Ho ho ho.\n\r",ch);
	    return;
	}

	if ( !can_over_ride(ch,victim,FALSE) )
	{
	    send_to_char("Maybe that wasn't a good idea...\n\r",ch);
	    sprintf(buf,"%s tried to purge you!\n\r",ch->name);
	    send_to_char(buf,victim);
	    return;
	}

	if (get_trust(ch) <= DEITY)
	{
	    send_to_char("Not against PC's!\n\r",ch);
	    return;
	}

	act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT,POS_RESTING);

    	if (victim->level > 1)
	    save_char_obj( victim, 0 );
    	d = victim->desc;
    	extract_char( victim, TRUE );
    	if ( d != NULL )
          close_socket( d );

	return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT, POS_RESTING );
    act( "You purge $N.", ch, NULL, victim, TO_CHAR, POS_DEAD );
    extract_char( victim, TRUE );
    return;
}

void do_advance( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    bool show;
    int level, iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: advance <char> <level> <'true/false' show gains>.\n\r", ch );
	return;
    }

    if ( ( victim = get_pc_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > 160 )
    {
	send_to_char( "Level must be 1 to 160.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }

    if ( !can_over_ride(ch,victim,FALSE) )
    {
	send_to_char("You can't change the levels of your superiors!\n\r",ch);
	return;
    }

    if ( argument[0] != '\0'
    &&   ( !str_prefix( argument, "yes" ) || !str_prefix( argument, "true" ) ) )
	show = TRUE;
    else
	show = FALSE;

    sprintf(buf,"advances %s from %d to %d.",victim->name,victim->level,level);
    parse_logs( ch, "immortal", buf );

    sprintf( buf,"$N advances %s from %d to %d.", victim->name,victim->level,level);
    wiznet( buf, ch, NULL, WIZ_SECURE, 0, get_trust( ch ) );

    if ( level <= victim->level )
    {
        int temp_prac;

	send_to_char( "Lowering a player's level!\n\r", ch );
	send_to_char( "{R******** {GOOOOHHHHHHHHHH  NNNNOOOO {R*******{x\n\r", victim );
	sprintf(buf, "{R**** {WYou've been demoted to level %d {R****{x\n\r", level );
	send_to_char(buf, victim);

	if ( victim->level > HERO || level > HERO )
	    update_wizlist( victim, level );

	temp_prac = victim->pcdata->practice;
	victim->level    = 1;
	victim->exp      = exp_per_level(victim,victim->pcdata->points);
	victim->max_hit  = 100;
	victim->max_mana = 100;
	victim->max_move = 100;
	victim->pcdata->perm_hit	= 100;
	victim->pcdata->perm_mana	= 100;
	victim->pcdata->perm_move	= 100;
	victim->pcdata->practice	= 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;
	advance_level( victim, show );
	victim->pcdata->practice = temp_prac;
    } else {
	send_to_char( "Raising a player's level!\n\r", ch );
	send_to_char( "{B******* {GOOOOHHHHHHHHHH  YYYYEEEESSS {B******{x\n\r", victim );
	sprintf(buf, "{B**** {WYou've been advanced to level %d {B****{x\n\r", level );
	send_to_char(buf, victim);
	if ( victim->level > HERO || level > HERO )
	{
	    update_wizlist(victim, level);
	    unrank_charts( victim );
	}
    }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
	victim->level += 1;
	advance_level( victim, show );
    }

    victim->exp   = exp_per_level(victim,victim->pcdata->points) 
		  * UMAX( 1, victim->level );
    victim->trust = 0;
    reset_char( victim );
    save_char_obj(victim,0);
    return;
}

void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_pc_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That is not a good idea...\n\r", ch );
	return;
    }

    if ( !can_over_ride(ch,victim,FALSE) )
    {
	send_to_char("Mind your own business.\n\r",ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > get_trust(ch) )
    {
	char buf[100];

	sprintf(buf, "Level must be 0 (reset) or 1 to %d.\n\r", get_trust(ch));
	send_to_char(buf,ch);
	return;
    }

    victim->trust = level;
    act( "You grant $N with trust of $t.", ch, arg2, victim, TO_CHAR, POS_DEAD );
    return;
}

void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );

    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
	    if ( ( vch->pcdata && vch->pcdata->penalty_time[PENALTY_NORESTORE] != 0 )
	    || IS_SET(vch->in_room->room_flags, ROOM_WAR)
	    || IS_SET(vch->in_room->room_flags, ROOM_ARENA) )
		continue;

	    if ((vch->fighting != NULL && !IS_NPC(vch->fighting))
	    ||  (!IS_NPC(vch) && vch->pcdata->pktimer > 0))
	    {
		send_to_char("You missed a restore due to your PK match.\n\r",vch);
		continue;
	    } else {
		affect_strip(vch,gsn_plague);
		affect_strip(vch,gsn_poison);
		affect_strip(vch,gsn_blindness);
		affect_strip(vch,gsn_sleep);
		affect_strip(vch,gsn_curse);

		vch->hit 	= vch->max_hit;
		vch->mana	= vch->max_mana;
		vch->move	= vch->max_move;
		update_pos( vch);
		act("$n has restored you.",ch,NULL,vch,TO_VICT,POS_DEAD);
	    }
        }

        sprintf(buf,"$N restored room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
	sprintf(buf,"restores room %d.",ch->in_room->vnum);
	parse_logs( ch, "immortal", buf );
        send_to_char("Room restored.\n\r",ch);
        return;

    }
    
    if ( !str_cmp(arg,"all") )
    {
        for (d = descriptor_list; d != NULL; d = d->next)
        {
	    victim = d->character;

	    if ( victim == NULL
	    ||   IS_NPC(victim)
	    ||   victim->in_room == NULL
	    ||   ( victim->pcdata && victim->pcdata->penalty_time[PENALTY_NORESTORE] != 0 )
	    ||   IS_SET(victim->in_room->room_flags, ROOM_ARENA)
	    ||   IS_SET(victim->in_room->room_flags, ROOM_WAR) )
		continue;

	    if ( (victim->fighting != NULL && !IS_NPC(victim->fighting))
	    ||   (!IS_NPC(victim) && victim->pcdata->pktimer > 0) )
	    {
		send_to_char("You missed a restore due to your PK match.\n\r",victim);
		continue;
	    } else {
		affect_strip(victim,gsn_plague);
		affect_strip(victim,gsn_poison);
		affect_strip(victim,gsn_blindness);
		affect_strip(victim,gsn_sleep);
		affect_strip(victim,gsn_curse);
            
		victim->hit 	= victim->max_hit;
		victim->mana	= victim->max_mana;
		victim->move	= victim->max_move;
		update_pos( victim);
		if (victim->in_room != NULL)
		    act("$n has restored you.",ch,NULL,victim,TO_VICT,POS_DEAD);
	    }
	}
	send_to_char("All active players restored.\n\r",ch);
	return;
    }

    if ( ( victim = get_pc_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ( victim->pcdata && victim->pcdata->penalty_time[PENALTY_NORESTORE] != 0 )
    || IS_SET(victim->in_room->room_flags, ROOM_WAR)
    || IS_SET(victim->in_room->room_flags, ROOM_ARENA) )
    {
	act("$n attempts to restore you, but fails.",ch,NULL,victim,TO_VICT,POS_DEAD);
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( (victim->fighting != NULL && !IS_NPC(victim->fighting))
    ||   (!IS_NPC(victim) && victim->pcdata->pktimer > 0) )
    {
	send_to_char("You missed a restore due to your PK match.\n\r",victim);
	return;
    }

    affect_strip(victim,gsn_plague);
    affect_strip(victim,gsn_poison);
    affect_strip(victim,gsn_blindness);
    affect_strip(victim,gsn_sleep);
    affect_strip(victim,gsn_curse);
    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );
    act( "$n has restored you.", ch, NULL, victim, TO_VICT,POS_DEAD);
    sprintf(buf,"$N restored %s",
	IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    sprintf(buf,"restores character %s.",victim->name);
    parse_logs( ch, "immortal", buf );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_wecho( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
	send_to_char( "Warn echo what?\n\r", ch );
	return;
    }

    sprintf(buf, "{z{B***{x {R%s{x {z{B***{x", argument);
    do_echo(ch, buf);
    do_echo(ch, buf);
    do_echo(ch, buf);
    do_restore(ch, "all");
    return;
}

void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int duration;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: freeze (victim) (This removes it).\n\r"
		      "        freeze (victim) permanent.\n\r"
		      "        freeze (victim) # (hours | minutes).\n\r"
		      "  These count down by minutes only when the victim is connected.\n\r", ch );
	return;
    }
 
    if ( ( victim = get_pc_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
        return;
    }
 
    if ( !can_over_ride( ch, victim, FALSE ) )
    {
	send_to_char( "You failed.\n\r", ch );
        return;
    }
 
    if ( arg2[0] == '\0' && victim->pcdata->penalty_time[PENALTY_FREEZE] != 0 )
    {
	victim->pcdata->penalty_time[PENALTY_FREEZE] = 0;

	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
	sprintf(buf,"$N thaws %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	sprintf(buf,"unfreezes %s.",victim->name);
	parse_logs( ch, "immortal", buf );
    } else {
	if ( arg2[0] != '\0' && !str_prefix( arg2, "permanent" ) )
	    duration = -1;

	else if ( arg2[0] == '\0' || !is_number( arg2 ) || argument[0] == '\0' )
	{
	    do_freeze( ch, "" );
	    return;
	}

	else if ( !str_prefix( argument, "hours" ) )
	    duration = atoi( arg2 ) * 60;

	else if ( !str_prefix( argument, "minutes" ) )
	    duration = atoi( arg2 );

	else
	{
	    send_to_char( "Invalid duration type.\n\r", ch );
	    return;
	}

	if ( duration != -1 && ( duration < 1 || duration > 2880 ) )
	{
	    send_to_char( "Valid durations are 1 minute to 48 hours.\n\r", ch );
	    return;
	}

	victim->pcdata->penalty_time[PENALTY_FREEZE] = duration;

	sprintf( buf, "Your have been frozen for %s.\n\r",
	    parse_time( duration == -1 ? -1 : duration*60 ) );
	send_to_char( buf, victim );
	send_to_char( "OK.\n\r", ch );

	sprintf( buf, "$N freezes %s for %s.",
	    victim->name, parse_time( duration == -1 ? -1 : duration*60 ) );
	wiznet( buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0 );
	parse_logs( ch, "immortal", buf );
    }
}

void do_norestore( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int duration;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: norestore (victim) (This removes it).\n\r"
		      "        norestore (victim) permanent.\n\r"
		      "        norestore (victim) # (hours | minutes).\n\r"
		      "  These count down by minutes only when the victim is connected.\n\r", ch );
	return;
    }
 
    if ( ( victim = get_pc_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
        return;
    }
 
    if ( !can_over_ride( ch, victim, FALSE ) )
    {
	send_to_char( "You failed.\n\r", ch );
        return;
    }
 
    if ( arg2[0] == '\0' && victim->pcdata->penalty_time[PENALTY_NORESTORE] != 0 )
    {
	victim->pcdata->penalty_time[PENALTY_NORESTORE] = 0;

	send_to_char( "You can be restored again.\n\r", victim );
	send_to_char( "NORESTORE removed.\n\r", ch );
	sprintf(buf,"$N permits restores to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	sprintf(buf,"removes norestore on %s.",victim->name);
	parse_logs( ch, "immortal", buf );
    } else {
	if ( arg2[0] != '\0' && !str_prefix( arg2, "permanent" ) )
	    duration = -1;

	else if ( arg2[0] == '\0' || !is_number( arg2 ) || argument[0] == '\0' )
	{
	    do_norestore( ch, "" );
	    return;
	}

	else if ( !str_prefix( argument, "hours" ) )
	    duration = atoi( arg2 ) * 60;

	else if ( !str_prefix( argument, "minutes" ) )
	    duration = atoi( arg2 );

	else
	{
	    send_to_char( "Invalid duration type.\n\r", ch );
	    return;
	}

	if ( duration != -1 && ( duration < 1 || duration > 2880 ) )
	{
	    send_to_char( "Valid durations are 1 minute to 48 hours.\n\r", ch );
	    return;
	}

	victim->pcdata->penalty_time[PENALTY_NORESTORE] = duration;

	sprintf( buf, "Your have been denied restores for %s.\n\r",
	    parse_time( duration == -1 ? -1 : duration*60 ) );
	send_to_char( buf, victim );
	send_to_char( "OK.\n\r", ch );

	sprintf( buf, "$N denies restores to %s for %s.",
	    victim->name, parse_time( duration == -1 ? -1 : duration*60 ) );
	wiznet( buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0 );
	parse_logs( ch, "immortal", buf );
    }
}

void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( mud_stat.fLogAll )
	{
	    mud_stat.fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    mud_stat.fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	mud_stat.changed = TRUE;
	return;
    }

    if ( ( victim = get_pc_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}

void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int duration;
 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: notell (victim) (This removes it).\n\r"
		      "        notell (victim) permanent.\n\r"
		      "        notell (victim) # (hours | minutes).\n\r"
		      "  These count down by minutes only when the victim is connected.\n\r", ch );
	return;
    }
 
    if ( ( victim = get_pc_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
        return;
    }
 
    if ( !can_over_ride( ch, victim, FALSE ) )
    {
	send_to_char( "You failed.\n\r", ch );
        return;
    }
 
    if ( arg2[0] == '\0' && victim->pcdata->penalty_time[PENALTY_NOTELL] != 0 )
    {
	victim->pcdata->penalty_time[PENALTY_NOTELL] = 0;
        send_to_char( "The gods have restored your tell priviliges.\n\r", victim );
        send_to_char( "NOTELL removed.\n\r", ch );
	sprintf( buf, "$N restores tells to %s", victim->name );
	wiznet( buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0 );
        sprintf( buf, "restores tells to %s", victim->name );
	parse_logs( ch, "immortal", buf );
    } else {
	if ( arg2[0] != '\0' && !str_prefix( arg2, "permanent" ) )
	    duration = -1;

	else if ( arg2[0] == '\0' || !is_number( arg2 ) || argument[0] == '\0' )
	{
	    do_nochannels( ch, "" );
	    return;
	}

	else if ( !str_prefix( argument, "hours" ) )
	    duration = atoi( arg2 ) * 60;

	else if ( !str_prefix( argument, "minutes" ) )
	    duration = atoi( arg2 );

	else
	{
	    send_to_char( "Invalid duration type.\n\r", ch );
	    return;
	}

	if ( duration != -1 && ( duration < 1 || duration > 2880 ) )
	{
	    send_to_char( "Valid durations are 1 minute to 48 hours.\n\r", ch );
	    return;
	}

	sprintf( buf, "Your tells have been revoked for %s.\n\r",
	    parse_time( duration == -1 ? -1 : duration*60 ) );

	victim->pcdata->penalty_time[PENALTY_NOTELL] = duration;
	send_to_char( buf, victim );
	send_to_char( "OK.\n\r", ch );

	sprintf( buf, "$N revokes %s's tells for %s.",
	    victim->name, parse_time( duration == -1 ? -1 : duration*60 ) );
	wiznet( buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0 );
	parse_logs( ch, "immortal", buf );
    }
}

void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL ) {
	    stop_fighting( rch, TRUE );
	    if (!IS_NPC(rch) ) {
		send_to_char( "Ok.\n\r", ch );
	    }
	}
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    }
    return;
}

void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",                                      ch );
	send_to_char( "  set char <name> <field> <value>\n\r",            ch ); 
	send_to_char( "  Field being one of:\n\r",			  ch );
	send_to_char( "    str int wis dex con sex class level\n\r",	  ch );
	send_to_char( "    race group platinum gold silver hp\n\r",	  ch );
	send_to_char( "    mana move prac align train thirst\n\r",	  ch );
	send_to_char( "    hunger drunk full deviant pkill pdeath\n\r",   ch );
        send_to_char( "    security aquest bounty awins aloss\n\r",       ch );
	send_to_char( "    akill adeath assist pkpoints tier\n\r",	  ch );
	send_to_char( "    saves max_storage dam_mod\n\r",		  ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !can_over_ride(ch,victim,FALSE) )
    {
	send_to_char("You failed.\n\r",ch);
	return;
    }

    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    sprintf(buf,"set %s's '%s' to %s",victim->name,arg2,argument);
    parse_logs( ch, "immortal", buf );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "str" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_STR) )
	{
	    sprintf(buf,
		"Strength range is 3 to %d\n\r.",
		get_max_train(victim,STAT_STR));
	    send_to_char(buf,ch);
	    return;
	}
	victim->perm_stat[STAT_STR] = value;
	return;
    }

    if ( !str_prefix( arg2, "security" ) )	/* OLC */
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

	if ( value > ch->pcdata->security || value < 0 )
	{
	    if ( ch->pcdata->security != 0 )
	    {
		sprintf( buf, "Valid security is 0-%d.\n\r",
		    ch->pcdata->security );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "Valid security is 0 only.\n\r", ch );
	    }
	    return;
	}
	victim->pcdata->security = value;
	return;
    }

    if ( !str_prefix( arg2, "int" ) )
    {
        if ( value < 3 || value > get_max_train(victim,STAT_INT) )
        {
            sprintf(buf,
		"Intelligence range is 3 to %d.\n\r",
		get_max_train(victim,STAT_INT));
            send_to_char(buf,ch);
            return;
        }
 
        victim->perm_stat[STAT_INT] = value;
        return;
    }

    if ( !str_prefix( arg2, "wis" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_WIS) )
	{
	    sprintf(buf,
		"Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_WIS] = value;
	return;
    }

    if ( !str_prefix( arg2, "dex" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_DEX) )
	{
	    sprintf(buf,
		"Dexterity ranges is 3 to %d.\n\r",
		get_max_train(victim,STAT_DEX));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_DEX] = value;
	return;
    }

    if ( !str_prefix( arg2, "con" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_CON) )
	{
	    sprintf(buf,
		"Constitution range is 3 to %d.\n\r",
		get_max_train(victim,STAT_CON));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CON] = value;
	return;
    }

    if ( !str_prefix( arg2, "dam_mod" ) )
    {
	sh_int dam;

	argument = one_argument( argument, arg3 );

	value = atoi( argument );

	if ( !is_number( argument ) || value < -200 || value > 200 )
	{
	    send_to_char( "Proper damage modifiers are -200 to 200%.\n\r", ch );
	    return;
	}

	if ( !str_cmp( arg3, "all" ) )
	{
	    for ( dam = 0; dam < DAM_MAX; dam++ )
		victim->damage_mod[dam] = value;
		
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}

	if ( ( dam = dam_type_lookup( arg3 ) ) == -1 )
	{
	    send_to_char( "Invalid damage modifier.\n\r", ch );
	    return;
	}

	victim->damage_mod[dam] = value;
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }

    if ( !str_prefix( arg2, "class" ) )
    {
	int class;

	class = class_lookup(arg3);

	if ( class == -1 )
	{
	    char buf[MAX_STRING_LENGTH];

	    strcpy( buf, "Possible classes are:" );
	    for ( class = 0; class_table[class].name[0] != '\0'; class++ )
	    {
		strcat( buf, " " );
		strcat( buf, class_table[class].name );
	    }
            strcat( buf, ".\n\r" );

	    send_to_char(buf,ch);
	    return;
	}

	victim->class = class;
	return;
    }

    if ( !str_prefix( arg2, "saves" ) )
    {
	victim->saving_throw = value;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}
	victim->level = value;
	victim->magic_power = value;
	return;
    }

    if ( !str_prefix( arg2, "max_storage" ) )
    {
	int pos;

	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	for ( pos = 0; pos < MAX_BANK; pos++ )
	    victim->pcdata->max_storage[pos] = value;

	return;
    }

    if ( !str_prefix( arg2, "platinum" ) )
    {
	victim->platinum = value;
	return;
    }

    if ( !str_prefix( arg2, "gold" ) )
    {
	victim->gold = value;
	return;
    }

    if ( !str_prefix(arg2, "silver" ) )
    {
	victim->silver = value;
	return;
    }

    if ( !str_prefix( arg2, "pkill" ) )
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not to NPCs.\n\r",ch);
	    return;
	}
	victim->pcdata->pkills = value;
	return;
    }

    if ( !str_prefix( arg2, "tier" ) )
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not to NPCs.\n\r",ch);
	    return;
	}

	if (value < 1 || value > 3)
	{
	    send_to_char("Tier must be 1 - 3.\n\r",ch);
	    return;
	}

	victim->pcdata->tier = value;
	return;
    }

    if ( !str_prefix( arg2, "assist" ) )
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not to NPCs.\n\r",ch);
	    return;
	}
	victim->pcdata->assist = value;
	return;
    }

    if ( !str_prefix( arg2, "pkpoints" ) )
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not to NPCs.\n\r",ch);
	    return;
	}
	victim->pcdata->pkpoints = value;
	return;
    }

    if ( !str_prefix( arg2, "pdeath" ) )
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not to NPCs.\n\r",ch);
	    return;
	}
	victim->pcdata->pdeath = value;
	return;
    }

    if ( !str_prefix( arg2, "awins" ) )
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not to NPCs.\n\r",ch);
	    return;
	}
	victim->pcdata->arenawins = value;
	return;
    }

    if ( !str_prefix( arg2, "aloss" ) )
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not to NPCs.\n\r",ch);
	    return;
	}
	victim->pcdata->arenaloss = value;
	return;
    }

    if ( !str_prefix( arg2, "akill" ) )
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not on NPCs.\n\r",ch);
	    return;
	}
	victim->pcdata->arenakills = value;
	return;
    }

    if ( !str_prefix( arg2, "adeath" ) )
    {
	if (IS_NPC(ch))
	{
	    send_to_char("Not on NPCs.\n\r",ch);
	    return;
	}
	victim->pcdata->arenadeath = value;
	return;
    }

    if ( !str_prefix( arg2, "bounty" ) )
    {
        if (IS_NPC(victim))
        {
            send_to_char("Not to NPCs.\n\r",ch);
            return;
        }
        victim->pcdata->bounty = value;
        return;
    }

    if ( !str_prefix( arg2, "hp" ) )
    {
	if ( value < -10 || value > 200000 )
	{
	    send_to_char( "Hp range is -10 to 200,000 hit points.\n\r", ch );
	    return;
	}
	victim->max_hit = value;
        if ( !IS_NPC(victim) )
            victim->pcdata->perm_hit = value;
	return;
    }

    if ( !str_prefix( arg2, "mana" ) )
    {
	if ( value < 0 || value > 200000 )
	{
	    send_to_char( "Mana range is 0 to 200,000 mana points.\n\r", ch );
	    return;
	}
	victim->max_mana = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_mana = value;
	return;
    }

    if ( !str_prefix( arg2, "move" ) )
    {
	if ( value < 0 || value > 200000 )
	{
	    send_to_char( "Move range is 0 to 200,000 move points.\n\r", ch );
	    return;
	}
	victim->max_move = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_move = value;
	return;
    }

    if ( !str_prefix( arg2, "practice" ) )
    {
	if ( value < 0 || value > 500 || IS_NPC( victim ) )
	{
	    send_to_char( "Practice range is 0 to 500 sessions.\n\r", ch );
	    return;
	}

	victim->pcdata->practice = value;
	return;
    }

    if ( !str_prefix( arg2, "train" ))
    {
	if (value < 0 || value > 500 || IS_NPC( victim ) )
	{
	    send_to_char("Training session range is 0 to 500 sessions.\n\r",ch);
	    return;
	}
	victim->pcdata->train = value;
	return;
    }

    if ( !str_prefix( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
	    return;
	}
	victim->alignment = value;
	if ( victim->pet != NULL )
	    victim->pet->alignment = victim->alignment;
	return;
    }

    if ( !str_prefix( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Thirst range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_prefix( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Drunk range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_prefix( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Full range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if ( !str_prefix( arg2, "hunger" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
 
        if ( value < -1 || value > 100 )
        {
            send_to_char( "Full range is -1 to 100.\n\r", ch );
            return;
        }
 
        victim->pcdata->condition[COND_HUNGER] = value;
        return;
    }

    if ( !str_prefix( arg2, "deviant_points" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "NPC's don't need quest points.\n\r", ch );
	    return;
	}

	victim->pcdata->deviant_points[0] = value;
	return;
    }

    if (!str_prefix( arg2, "aquest" ) )
    {
        if (IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r",ch );
            return;
        }

        victim->pcdata->questpoints = value;
        return;
    }

    if (!str_prefix( arg2, "race" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == -1 )
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_table[race].pc_race)
	{
	    send_to_char("That is not a valid player race.\n\r",ch);
	    return;
	}

	victim->race = race;
	return;
    }
   
    if (!str_prefix(arg2,"group"))
    {
	if (!IS_NPC(victim))
	{
	    send_to_char("Only on NPCs.\n\r",ch);
	    return;
	}
	victim->group = value;
	return;
    }
    do_mset( ch, "" );
    return;
}

void do_oset( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",				ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",	ch );
	send_to_char("    level weight cost timer\n\r",		ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return;
    }

    if (obj->item_type == ITEM_EXIT)
    {
	send_to_char( "You cannot modify exit objects.\n\r", ch );
	return;
    }

    sprintf(buf,"set object - %s--%s with '%s' of %s",obj->name,obj->short_descr,arg2,arg3);
    parse_logs( ch, "immortal", buf );

    value = atoi( arg3 );

    if ( !str_prefix( arg2, "value0" ) || !str_prefix( arg2, "v0" ) )
    {
	if (obj->item_type == ITEM_WEAPON)
	{
	    if ( is_number( arg3 ) )
		obj->value[0] = URANGE( 0, value, MAX_WEAPON-1 );
	    else
		obj->value[0] = flag_value( weapon_class, arg3 );
	    return;
	}

	if ((obj->item_type == ITEM_WAND)
	||  (obj->item_type == ITEM_STAFF)
	||  (obj->item_type == ITEM_POTION)
	||  (obj->item_type == ITEM_SCROLL)
	||  (obj->item_type == ITEM_PILL))
	{
	    obj->value[0] = URANGE( 0, value, MAX_LEVEL );
	    return;
	}
	obj->value[0] = value;
	return;
    }

    if ( !str_prefix( arg2, "value1" ) || !str_prefix( arg2, "v1" ) )
    {
	obj->value[1] = value;
	return;
    }

    if ( !str_prefix( arg2, "value2" ) || !str_prefix( arg2, "v2" ) )
    {
	if ((obj->item_type == ITEM_FOUNTAIN)
	||  (obj->item_type == ITEM_DRINK_CON))
	{
	    obj->value[2] = UMIN(MAX_LIQUID,value);
	    obj->value[2] = UMAX(0,obj->value[2]);
	    return;
	}
	obj->value[2] = value;
	return;
    }

    if ( !str_prefix( arg2, "value3" ) || !str_prefix( arg2, "v3" ) )
    {
	if (obj->item_type == ITEM_WEAPON)
	{
	    if ( is_number( arg3 ) )
		obj->value[3] = URANGE( 0, value, MAX_DAMAGE_MESSAGE );
	    else
		obj->value[3] = attack_lookup( arg3 );
	    return;
	}
	obj->value[3] = value;
	return;
    }

    if ( !str_prefix( arg2, "value4" ) || !str_prefix( arg2, "v4" ) )
    {
	obj->value[4] = value;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( get_trust(ch) < CREATOR && obj->pIndexData->level - 5 > value )
	{
	    send_to_char("You may not lower an item more than 5 levels!\n\r",ch);
	    return;
	}
        if ( get_trust(ch) == CREATOR && obj->pIndexData->level - 10 > value )
        { 
            send_to_char("You may not lower an item more than 10 levels!\n\r",ch);
            return; 
        }
	obj->level = value;
	return;
    }
	
    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }
    do_oset( ch, "" );
    return;
}

void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    sector\n\r",				ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    if ( !str_prefix( arg2, "flags" ) )
    {
	send_to_char( "Use the flag command instead.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }
    do_rset( ch, "" );
    return;
}

void do_sset( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char( "  set skill <name> all <value>\n\r",ch);  
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;

    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );

    if ( victim->race == race_lookup( "human" ) )
    {
	if ( value < 0 || value > 105 )
	{
	    send_to_char( "Value range is 0 to 105.\n\r", ch );
	    return;
	}
    } else {
	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Value range is 0 to 100.\n\r", ch );
	    return;
	}
    }	

    if ( fAll )
    {
	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	    victim->learned[sn] = value;
        sprintf(buf,"sets %s with all skills %d.",victim->name,value);
    } else {
	victim->learned[sn] = value;
	sprintf(buf,"sets %s's skill '%s' to %d",victim->name,arg2,value);
    }

    parse_logs( ch, "immortal", buf );
}

void do_set( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set mob   <name> <field> <value>\n\r",ch);
	send_to_char("  set obj   <name> <field> <value>\n\r",ch);
	send_to_char("  set room  <room> <field> <value>\n\r",ch);
	send_to_char("  set char  <name> <field> <value>\n\r",ch);
        send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	do_mset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
	do_sset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	do_oset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"room"))
    {
	do_rset(ch,argument);
	return;
    }

    do_set(ch,"");
}

void do_string( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH+5];

    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  string char <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long title who spec pretitle\n\r",ch);
	send_to_char("  string obj  <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long extended\n\r",ch);
	return;
    }
    
    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
    	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
    	}

	if ( !can_over_ride(ch,victim,FALSE) )
	{
	    send_to_char("That will not be done.\n\r", ch);
	    return;
	}
    	
	victim->zone = NULL;

        if ( !str_prefix( arg2, "pretitle" ) )
        {
            if ( IS_NPC(victim) )
            {
                send_to_char( "Not on NPC's.\n\r", ch );
                return;
            }

	    if ( arg3[0] == '\0' )
	    {
		free_string( victim->pcdata->pretitle );
		victim->pcdata->pretitle = str_dup("");
		return;
	    }

            set_pretitle( victim, arg3 );
            return;
        }

    	if ( !str_prefix( arg2, "who" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }

	    free_string( victim->pcdata->who_descr );

	    if (arg3[0] == '\0')
	    {
		victim->pcdata->who_descr = str_dup( "" );
		return;
	    }

    	    victim->pcdata->who_descr = str_dup( end_string(arg3,15) );
    	    return;
    	}

	if (arg3[0] == '\0')
	{
	    do_string(ch,"");
	    return;
	}

     	if ( !str_prefix( arg2, "name" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }
	    free_string( victim->name );
	    victim->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( victim->short_descr );
	    victim->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( victim->long_descr );
	    strcat(arg3,"\n\r");
	    victim->long_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "title" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }

	    set_title( victim, arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "spec" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }

	    if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	    {
	    	send_to_char( "No such spec fun.\n\r", ch );
	    	return;
	    }

	    return;
    	}
    }
    
    if (arg3[0] == '\0')
    {
	do_string(ch,"");
	return;
    }

    if (!str_prefix(type,"object"))
    {
    	/* string an obj */
    	
   	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	    return;
    	}
    	if (obj->item_type == ITEM_EXIT)
	{
	    send_to_char("You cannot modify exit objects.\n\r",ch);
	    return;
	}
        if ( !str_prefix( arg2, "name" ) )
    	{
	    free_string( obj->name );
	    obj->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( obj->description );
	    obj->description = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
	    	send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		    ch );
	    	return;
	    }

 	    strcat(argument,"\n\r");

	    ed = new_extra_descr();

	    ed->keyword		= str_dup( arg3     );
	    ed->description	= str_dup( argument );
	    ed->next		= obj->extra_descr;
	    obj->extra_descr	= ed;
	    return;
    	}
    }    

    do_string(ch,"");
}

void do_sockets( CHAR_DATA *ch, char *argument )
{
    BUFFER *final = new_buf( );
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH], time[30], idle[10];
    char *st;
    int count = 0;

    add_buf( final, "{C[{wNu {cConnect {GDay   {c@ {GTime {RIdle{C] {BC Name        {cIP              {BHost\n\r" );
    add_buf( final, "{C------------------------------------------------------------------------------------------------\n\r" );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	switch( d->connected )
	{
	    case CON_PLAYING:			st = "PLAYING";	break;
	    case CON_GET_NAME:			st = "GetName";	break;
	    case CON_GET_OLD_PASSWORD:		st = "OldPass";	break;
	    case CON_CONFIRM_NEW_NAME:		st = "CnfName";	break;
	    case CON_GET_NEW_PASSWORD:		st = "NewPass";	break;
	    case CON_CONFIRM_NEW_PASSWORD:	st = "CnfPass";	break;
	    case CON_GET_NEW_RACE:		st = "New Rac";	break;
	    case CON_GET_NEW_SEX:		st = "New Sex";	break;
	    case CON_GET_NEW_CLASS:		st = " Class ";	break;
	    case CON_GET_ALIGNMENT:		st = " Align ";	break;
	    case CON_DEFAULT_CHOICE:		st = " Cust? ";	break;
	    case CON_GEN_GROUPS:		st = " Custo ";	break;
	    case CON_PICK_WEAPON:		st = " Weapo ";	break;
	    case CON_READ_IMOTD:		st = " IMOTD ";	break;
	    case CON_BREAK_CONNECT:		st = "LnkDead";	break;
	    case CON_READ_MOTD:			st = " MOTD  ";	break;
	    case CON_REROLLING:			st = " Rerol ";	break;
	    default:				st = "Unknown";	break;
	}

	if ( d->character
	&&   ( !can_see( ch, d->character )
	||     !can_over_ride( ch, d->character, TRUE ) ) )
	    continue;

	count++;

	vch = d->original ? d->original : d->character;

	if ( vch )
	    strftime( time, 30, "{G%m{c/{G%d {c@ {G%H{c:{G%M", localtime( &vch->pcdata->logon ) );
	else
	    time[0] = '\0';

	if ( vch && vch->timer > 0 )
	    sprintf( idle, "%d", vch->timer );

	else if ( d->connected != CON_PLAYING && d->timer > 0 )
	    sprintf( idle, "%d", d->timer );

	else
	    idle[0] = '\0';

	sprintf( buf, "{C[{w%2d {c%s %13s{R%4s{C] %s {B%-12s{c%-15s {B%s\n\r",
	    d->descriptor, st, time, idle, d->out_compress ? "{GY" : "{RN",
	    vch ? vch->name : "(None)", d->hostip, d->host );
	add_buf( final, buf );
    }

    sprintf( buf, "\n\r{C%d {cuser%s{x\n\r", count, count == 1 ? "" : "s" );
    add_buf( final, buf );
    page_to_char( final->string, ch );
    free_buf( final );
    return;
}

void do_force( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    one_argument(argument,arg2);
  
    if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    if (!str_cmp(arg2,"reroll"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    sprintf( buf, "$n forces you to '%s'.", argument );

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;

	if (get_trust(ch) < MAX_LEVEL - 3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	for ( vch = player_list; vch != NULL; vch = vch->pcdata->next_player )
	{
	    if ( get_trust( vch ) < get_trust( ch ) )
	    {
		act( buf, ch, NULL, vch, TO_VICT, POS_DEAD );
		interpret( vch, argument );
	    }
	}
    }
    else if (!str_cmp(arg,"players"))
    {
        CHAR_DATA *vch;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( vch = player_list; vch != NULL; vch = vch->pcdata->next_player )
        {
            if ( get_trust( vch ) < get_trust( ch ) 
	    &&	 vch->level < LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT, POS_DEAD );
                interpret( vch, argument );
            }
        }
    }
    else if (!str_cmp(arg,"gods"))
    {
        CHAR_DATA *vch;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( vch = player_list; vch != NULL; vch = vch->pcdata->next_player )
        {
            if ( get_trust( vch ) < get_trust( ch )
            &&   vch->level >= LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT, POS_DEAD );
                interpret( vch, argument );
            }
        }
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( !can_over_ride(ch,victim,FALSE) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	act( buf, ch, NULL, victim, TO_VICT, POS_DEAD );
	interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_invis( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    int level;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	if ( ch->invis_level)
	{
	    ch->invis_level = 0;
	    if (!IS_TRUSTED(ch,IMPLEMENTOR))
		act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    else
		act( "$n appears in a blinding {z{Wflash{x!", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    send_to_char( "You slowly fade back into existence.\n\r", ch );
	} else {
	    if (!IS_TRUSTED(ch,IMPLEMENTOR)) 
		act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    else
		act( "A {Wblinding white light{x envelops $n, then {z{Dvanishes{x.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    send_to_char( "You slowly vanish into thin air.\n\r", ch );
            ch->invis_level = get_trust(ch);
	}
    } else {
	level = atoi(arg);

	if (level < 2 || level > get_trust(ch))
	{
	    send_to_char("Invis level must be between 2 and your level.\n\r",ch);
	    return;
	} else {
	    if (!IS_TRUSTED(ch,IMPLEMENTOR))
		act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    else
		act( "A {Wblinding white light{x envelops $n, then {z{Dvanishes{x.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    send_to_char( "You slowly vanish into thin air.\n\r", ch );
            ch->invis_level = level;
	}
    }
    return;
}

void do_incognito( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
	if ( ch->incog_level)
	{
	    ch->incog_level = 0;
	    act( "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    send_to_char( "You are no longer cloaked.\n\r", ch );
	} else {
	    ch->incog_level = get_trust(ch);
	    ch->ghost_level = 0;
	    act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    send_to_char( "You cloak your presence.\n\r", ch );
	}
    } else {
	level = atoi(arg);

	if (level < 2 || level > get_trust(ch))
	{
	    send_to_char("Incog level must be between 2 and your level.\n\r",ch);
	    return;
	} else {
	    ch->incog_level = level;
	    act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    send_to_char( "You cloak your presence.\n\r", ch );
	}
    }
    return;
}

void do_ghost( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    { 
	if ( ch->ghost_level)
	{
	    ch->ghost_level = 0;
	    act( "$n steps out from the mist.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    send_to_char( "You step out from the mist.\n\r", ch );
	} else {
	    ch->ghost_level = get_trust(ch);
	    ch->incog_level = 0;
	    act( "$n vanishes into a mist.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    send_to_char( "You vanish into a mist.\n\r", ch );
	}
    } else {
	level = atoi(arg);

	if (level < 2 || level > get_trust(ch))
	{
	    send_to_char("Ghost level must be between 2 and your level.\n\r",ch);
	    return;
	} else {
	    ch->ghost_level = level;
	    act( "$n vanishes into a mist.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    send_to_char( "You vanish into a mist.\n\r", ch );
	}
    }
    return;
}

void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    } else {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }
    return;
}

void do_prefi (CHAR_DATA *ch, char *argument)
{
    send_to_char("You cannot abbreviate the prefix command.\r\n",ch);
    return;
}

void do_prefix (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if ( ch->pcdata == NULL )
	return;

    if (argument[0] == '\0')
    {
	if (ch->pcdata->prefix[0] == '\0')
	{
	    send_to_char("You have no prefix to clear.\r\n",ch);
	    return;
	}

	send_to_char("Prefix removed.\r\n",ch);
	free_string(ch->pcdata->prefix);
	ch->pcdata->prefix = str_dup("");
	return;
    }

    if (ch->pcdata->prefix[0] != '\0')
    {
	sprintf(buf,"Prefix changed to %s.\r\n",argument);
	free_string(ch->pcdata->prefix);
    }
    else
	sprintf(buf,"Prefix set to %s.\r\n",argument);
    ch->pcdata->prefix = str_dup(argument);
}

void do_mpoint (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Make a questpoint item of what?\n\r", ch );
        return;
    }
    if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (IS_OBJ_STAT(obj,ITEM_QUESTPOINT))
    {
	REMOVE_BIT(obj->extra_flags,ITEM_QUESTPOINT);
	act("$p is no longer a questpoint item.",ch,obj,NULL,TO_CHAR,POS_DEAD);
    }
    else
    {
	SET_BIT(obj->extra_flags,ITEM_QUESTPOINT);
	act("$p is now a questpoint item.",ch,obj,NULL,TO_CHAR,POS_DEAD);
    }

    return;
}

void do_gset (CHAR_DATA *ch, char *argument)
{
    if ( IS_NPC(ch) )
	return;

    if ( ( argument[0] == '\0' ) || !is_number( argument ) )
    {
        send_to_char( "Goto point cleared.\n\r", ch );
	ch->pcdata->recall = 0;
        return;
    }

    ch->pcdata->recall = atoi(argument);

    send_to_char( "Ok.\n\r", ch );

    return;
}

void do_smite( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int value;

    if ( IS_NPC(ch) )
    {
        send_to_char( "Mobs can't smite.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("Syntax: smite <char> <type> <divisor>\n\r", ch);
        send_to_char("<Type> can be hp, mana, move, or all.\n\r",ch);
        send_to_char("<Divisor> can be any number from 2 - 100.\n\r",ch);
        return;
    }


    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't playing.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Trying to smite a mob?\n\r", ch );
        return;
    }

    if (ch == victim)
    {
        send_to_char( "Trying to smite yourself?\n\r", ch );
        return;
    }

    if (!can_over_ride(ch,victim,FALSE))
    {
        send_to_char( "That was a very bad thing you tried to do...\n\r", ch );
        victim = ch;
    }

    if (arg3[0] != '\0')
	value = atoi(arg3);
    else
	value = 2;

    if(!is_number(arg3))
	value = 2;

    act( "A bolt from the heavens smites $N!", ch, NULL, victim, TO_NOTVICT, POS_RESTING );
    act( "A bolt from the heavens smites you!", ch, NULL, victim, TO_VICT, POS_DEAD );
    act( "You smite $N!", ch, NULL, victim, TO_CHAR, POS_DEAD );

    if(!str_cmp(arg2,"hp"))
    {
     victim->hit /= value;
    }
    else if(!str_cmp(arg2,"mana"))
    {
     victim->mana /= value;
    }
    else if(!str_cmp(arg2,"move"))
    {
     victim->move /= value;
    }
    else if(!str_cmp(arg2,"all"))
    {
        victim->hit /= value;
        victim->mana /= value;
        victim->move /= value;
    }

    if (victim->hit == 0)
	victim->hit = 1;

    if (victim->mana == 0)
        victim->mana = 1;

    if (victim->move == 0)
        victim->move = 1;

    return;
}

void do_wizslap( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *pRoomIndex;
    AFFECT_DATA af;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "WizSlap whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_pc_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !can_over_ride(ch,victim,FALSE) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }
    pRoomIndex = get_random_room(victim);

    if ( !check_builder( ch, pRoomIndex->area, TRUE ) )
	return;

    act( "$n slaps you, sending you reeling through time and space!", ch, NULL, victim, TO_VICT,POS_DEAD);
    act( "$n slaps $N, sending $M reeling through time and space!", ch, NULL, victim, TO_NOTVICT,POS_RESTING);
    act( "You send $N reeling through time and space!", ch, NULL, victim, TO_CHAR,POS_DEAD);
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( "$n crashes to the ground!", victim, NULL, NULL, TO_ROOM,POS_RESTING);

    af.where     = TO_AFFECTS;
    af.type      = gsn_curse;
    af.level     = 210;
    af.dur_type  = DUR_TICKS;
    af.duration  = 50;
    af.bitvector = AFF_CURSE;
    af.location  = APPLY_HITROLL;
    af.modifier  = -500;
    affect_to_char( victim, &af );
    af.where     = TO_AFFECTS;
    af.location  = APPLY_DAMROLL;
    af.modifier  = -500;
    affect_to_char( victim, &af );
    af.where     = TO_AFFECTS;
    af.location  = APPLY_WIS;
    af.modifier  = -50;
    affect_to_char( victim, &af );
    af.where     = TO_AFFECTS;
    af.location  = APPLY_INT;
    af.modifier  = -50;
    affect_to_char( victim, &af );
    af.where     = TO_AFFECTS;
    af.location  = APPLY_DEX;
    af.modifier  = -50;
    affect_to_char( victim, &af );
    af.where     = TO_AFFECTS;
    af.location  = APPLY_CON;
    af.modifier  = -50;
    affect_to_char( victim, &af );
    af.where	 = TO_AFFECTS;
    af.location	 = APPLY_STR;
    af.modifier  = -50;
    affect_to_char( victim, &af );
    af.where     = TO_AFFECTS;
    af.location  = APPLY_SAVES;
    af.modifier  = 500;
    affect_to_char( victim, &af );
    af.where	 = TO_AFFECTS;
    af.location	 = APPLY_AC;
    af.modifier  = 500;
    affect_to_char( victim, &af );

    act( "{6The {8I{7m{8m{7o{8r{7t{8a{7l{8s {6of $t {6have {!f{1o{!r{1s{!a{1k{!e{1n {6you.{x",
	victim, mud_stat.mud_name_string, NULL, TO_CHAR, POS_DEAD );

    do_look( victim, "auto" );
    sprintf(buf,"wizslaps %s",victim->name);
    parse_logs( ch, "immortal", buf );
    return;
}

void spellup( CHAR_DATA *ch, CHAR_DATA *vch )
{
    sh_int sn;

    affect_strip( vch, gsn_plague );
    affect_strip( vch, gsn_poison );
    affect_strip( vch, gsn_blindness );
    affect_strip( vch, gsn_sleep );
    affect_strip( vch, gsn_curse );

    vch->hit	= vch->max_hit;
    vch->mana	= vch->max_mana;
    vch->move	= vch->max_move;
    update_pos( vch );

    for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
    {
	if ( skill_table[sn].spell_fun == spell_null )
	    continue;

	if ( IS_SET( skill_table[sn].flags, SKILL_GLOBAL_SPELLUP ) )
	{
	    AFFECT_DATA *paf;

	    if ( ( paf = affect_find( vch->affected, sn ) ) == NULL
	    ||   paf->duration != -1 )
	    {
		affect_strip( vch, sn );
		( *skill_table[sn].spell_fun )( sn, 115, ch, vch, TARGET_CHAR );
	    }
	}
    }
}

void do_spellup( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *vch;

    if ( argument[0] == '\0' )
    {
	send_to_char(	"Syntax: spellup list      [display all spells included in spellup]\n\r"
			"        spellup room      [spellup all players in a room]\n\r"
			"        spellup all       [spellup all active players]\n\r"
			"        spellup (target)  [spellup a specific character]\n\r"
			"        spellup = (level) [spellup all players of a specific level]\n\r"
			"        spellup < (level) [spellup all players below a specific level]\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "list" ) )
    {
	int sn;

	send_to_char( "Spells included in the global spellup:\n\r", ch );

	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( IS_SET( skill_table[sn].flags, SKILL_GLOBAL_SPELLUP ) )
	    {
		send_to_char( skill_table[sn].name, ch );
		send_to_char( "\n\r", ch );
	    }
	}

	return;
    }

    if ( !str_cmp( argument, "room" ) )
    {
	for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
	{
	    if ( IS_IMMORTAL( vch ) || IS_NPC( vch )
	    ||   vch->pcdata->penalty_time[PENALTY_NORESTORE] != 0
	    ||   IS_SET( vch->in_room->room_flags, ROOM_ARENA )
	    ||   IS_SET( vch->in_room->room_flags, ROOM_WAR ) )
		continue;

	    if ( !IS_IMMORTAL( vch ) )
	    {
		if ( ( vch->fighting != NULL && !IS_NPC( vch->fighting ) )
		||   ( !IS_NPC( vch ) && vch->pcdata->pktimer > 0 ) )
		{
		    send_to_char( "You missed a spellup due to your PK match!\n\r", vch );
		    continue;
		}
	    }

	    spellup( ch, vch );
	}

	sprintf( buf, "$N blessed room %d.", ch->in_room->vnum );
	wiznet( buf, ch, NULL, WIZ_RESTORE, 0, get_trust( ch ) );
	sprintf( buf, "gives spellup to room %d.", ch->in_room->vnum );
	parse_logs( ch, "immortal", buf );

	send_to_char( "You have blessed the room.\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "all" ) )
    {
	for ( vch = player_list; vch != NULL; vch = vch->pcdata->next_player )
	{
            if ( vch == NULL
	    ||   IS_NPC( vch )
	    ||   IS_IMMORTAL( vch )
	    ||   vch->in_room == NULL
	    ||   vch->pcdata->penalty_time[PENALTY_NORESTORE] != 0
	    ||   IS_SET( vch->in_room->room_flags, ROOM_ARENA )
	    ||   IS_SET( vch->in_room->room_flags, ROOM_WAR )
	    ||   IS_SET( vch->configure, CONFIG_NO_SPELLUP ) )
		continue;

	    if ( ( vch->fighting != NULL && !IS_NPC( vch->fighting ) )
	    ||   ( !IS_NPC( vch ) && vch->pcdata->pktimer > 0 ) )
	    {
		send_to_char( "You missed a spellup due to your PK match!\n\r", vch );
		continue;
	    }

	    spellup( vch, vch );
	}

	send_to_char( "Global blessing complete.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "<" ) )
    {
	if ( argument[0] == '\0' || !is_number( argument ) )
	{
	    send_to_char("spellup < (level).\n\r",ch);
	    return;
	}

	for (vch = player_list; vch != NULL; vch = vch->pcdata->next_player)
	{
            if ( vch == NULL
	    ||   IS_NPC(vch)
	    ||   IS_IMMORTAL(vch)
	    ||   vch->level > atoi(argument)
	    ||   vch->in_room == NULL
	    ||   vch->pcdata->penalty_time[PENALTY_NORESTORE] != 0
	    ||   IS_SET(vch->in_room->room_flags, ROOM_ARENA)
	    ||   IS_SET(vch->in_room->room_flags, ROOM_WAR) )
		continue;

	    if ( (vch->fighting != NULL && !IS_NPC(vch->fighting))
	    ||   (!IS_NPC(vch) && vch->pcdata->pktimer > 0))
	    {
		send_to_char("You missed a spellup due to your PK match!\n\r",vch);
		continue;
	    }

	    spellup(vch,vch);
	}
	printf_to_char(ch,"You blessed everyone level %d and under.\n\r",
	    atoi(argument));
        return;
    }

    if (!str_cmp(arg,"="))
    {
	if (argument[0] == '\0' || !is_number(argument))
	{
	    send_to_char("spellup = <level>.\n\r",ch);
	    return;
	}

	for (vch = player_list; vch != NULL; vch = vch->pcdata->next_player)
	{
            if ( vch == NULL
	    ||   IS_NPC(vch)
	    ||   vch->level != atoi(argument)
	    ||   vch->in_room == NULL
	    ||   vch->pcdata->penalty_time[PENALTY_NORESTORE] != 0
	    ||   IS_SET(vch->in_room->room_flags, ROOM_ARENA)
	    ||   IS_SET(vch->in_room->room_flags, ROOM_WAR) )
		continue;

	    if ( (vch->fighting != NULL && !IS_NPC(vch->fighting))
	    ||   (!IS_NPC(vch) && vch->pcdata->pktimer > 0))
	    {
		send_to_char("You missed a spellup due to your PK match!\n\r",vch);
		continue;
	    }

	    spellup(vch,vch);
	}
	printf_to_char(ch,"You blessed everyone whos level is %d.\n\r",
	    atoi(argument));
        return;
    }

    if ( ( vch = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ( vch->pcdata && vch->pcdata->penalty_time[PENALTY_NORESTORE] != 0 )
    ||   IS_SET(vch->in_room->room_flags, ROOM_WAR)
    ||   IS_SET(vch->in_room->room_flags, ROOM_ARENA) )
    {
        send_to_char( "{R{zYou do not deserve a spellup.{x\n\r",vch);
        send_to_char( "That target does not deserve a spellup.\n\r",ch);
        return;
    }

    if ( (vch->fighting != NULL && !IS_NPC(vch->fighting))
    ||   (!IS_NPC(vch) && vch->pcdata->pktimer > 0) )
    {
	send_to_char("You missed a spellup due to your PK match!\n\r",vch);
	return;
    }

    spellup(ch,vch);

    act("You have blessed $N",ch,NULL,vch,TO_CHAR,POS_DEAD);
    sprintf(buf,"gives %s a spellup.",vch->name);
    parse_logs( ch, "immortal", buf );
}

void do_rename( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_MULTI_DATA *mult;
    OBJ_DATA *obj, *in_obj;
    FILE *fp;
    char strsave[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    extern int port;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg1 );
    argument[0] = UPPER( argument[0] );
    smash_tilde( argument );

    if ( arg1[0] == '\0' )
    {
	send_to_char("Rename who?\n\r",ch);
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char("What should their new name be?\n\r",ch);
	return;
    }

    if ( (victim = get_pc_world(ch,arg1)) == NULL )
    {
	send_to_char("They aren't connected.\n\r",ch);
	return;
    }

    if ( !can_over_ride(ch,victim,FALSE) )
    {
	send_to_char("You can't do that!\n\r",ch);
	return;
    }

    if ( victim->pcdata->match != NULL )
    {
	send_to_char( "Wait until after the arena.\n\r", ch );
	return;
    }

    if ( !check_parse_name(argument) )
    {
	sprintf(buf,"The name {c%s{x is {Rnot allowed{x.\n\r",argument);
	send_to_char(buf,ch);
	return;
    }

    if ( port == MAIN_GAME_PORT )
	sprintf( strsave, "%s%s/%s", PLAYER_DIR, initial( argument ),
	    capitalize( argument ) );
    else
	sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( argument ) );

    if ( (fp = fopen(strsave, "r")) != NULL )
    {
	sprintf(buf,"There is already someone named %s.\n\r",capitalize(argument));
	send_to_char(buf,ch);
	fclose(fp);
	return;
    }

    if ( is_clan(victim) )
	check_roster( victim, TRUE );

    if ( port == MAIN_GAME_PORT )
    {
	unrank_charts( victim );
	update_wizlist(victim,1);

	sprintf( strsave, "%s%s/%s", BACKUP_DIR, initial( victim->name ),
	    capitalize( victim->name ) );
	unlink( strsave );

	sprintf( strsave, "%s%s/%s", PLAYER_DIR, initial( victim->name ),
	    capitalize( victim->name ) );
    } else
	sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( victim->name ) );

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );

	if ( obj->owner != NULL && !str_cmp( obj->owner, victim->name ) )
	{
	    free_string( obj->owner );
	    obj->owner = str_dup( argument );
	}

	if ( obj->killer != NULL && !str_cmp( obj->killer, victim->name ) )
	{
	    free_string( obj->killer );
	    obj->killer = str_dup( argument );
	}

	for ( mult = obj->multi_data; mult != NULL; mult = mult->next )
	{
	    if ( !str_cmp( mult->dropper, victim->name ) )
	    {
		free_string( mult->dropper );
		mult->dropper = str_dup( argument );
	    }
	}

	if ( in_obj->carried_by == NULL || in_obj->carried_by != victim )
	    continue;

	if ( obj->loader != NULL )
	{
	    strcpy( buf, obj->loader );
	    str_replace_c( buf, victim->name, argument );
	    free_string( obj->loader );
	    obj->loader = str_dup( buf );
	}

	if ( obj->pIndexData->vnum == 49 )
	{
	    strcpy( buf, obj->name );
	    str_replace_c( buf, victim->name, argument );
	    free_string( obj->name );
	    obj->name = str_dup( buf );

	    strcpy( buf, obj->short_descr );
	    str_replace_c( buf, victim->name, argument );
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( buf );

	    strcpy( buf, obj->description );
	    str_replace_c( buf, victim->name, argument );
	    free_string( obj->description );
	    obj->description = str_dup( buf );
	}
    }
    
    free_string( victim->name );
    victim->name	= str_dup( argument );

    if ( is_clan( victim ) )
	check_roster( victim, FALSE );

    if ( victim->level > LEVEL_HERO )
	update_wizlist(victim, victim->level);

    save_char_obj(victim,0);

    unlink( strsave );

    if ( victim != ch )
    {
	sprintf(buf,"{YNOTICE: {xYou have been renamed to {c%s{x.\n\r",victim->name);
	send_to_char(buf,victim);
    }

    send_to_char("Done.\n\r",ch);
    return;
}
  
void do_pload( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Load who?\n\r", ch );
	return;
    }

    argument[0] = UPPER( argument[0] );

    if ( get_pc_world( ch, argument ) != NULL )
    {
	send_to_char( "That person is already connected!\n\r", ch );
	return;
    }

    d = new_descriptor( );
    if ( !load_char_obj( d, argument, FALSE, FALSE ) )
    {
	send_to_char( "Load Who? Are you sure? I cant seem to find them.\n\r", ch );
	free_char( d->character );
	free_descriptor( d );
	return;
    }

    victim = d->character;
    free_descriptor( d );

    victim->desc		= NULL;
    victim->next		= char_list;
    char_list			= victim;
    victim->pcdata->next_player	= player_list;
    player_list			= victim;

    if ( !can_over_ride( ch, victim, FALSE ) )
    {
	send_to_char( "You can't load your superiors.\n\r", ch );
	force_quit( victim, "punload" );
	return;
    }

    if ( victim->in_room != NULL )
	victim->pcdata->was_in_room = victim->in_room;

    char_to_room( victim, ch->in_room );

    victim->timer = -5000;

    act( "$n has pulled $N from the pattern!",
        ch, NULL, victim, TO_ROOM, POS_RESTING );
    act( "You have pulled $N from the pattern!",
        ch, NULL, victim, TO_CHAR, POS_RESTING );

    if ( victim->pet != NULL )
    {
	char_to_room( victim->pet, victim->in_room );
	act( "$n has entered the game.",
	    victim->pet, NULL, NULL, TO_ROOM, POS_RESTING );
    }
}

void do_punload( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( ( victim = get_pc_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "I dont think that would be a good idea...\n\r", ch );
	return;
    }

    if ( victim->pcdata->was_in_room != NULL )
    {
	char_from_room( victim );
	char_to_room( victim, victim->pcdata->was_in_room );
	if ( victim->pet != NULL )
	{
	    char_from_room( victim->pet );
	    char_to_room( victim->pet, victim->pcdata->was_in_room );
	}
    }

    act( "$n has released $N back to the pattern.",
       ch, NULL, victim, TO_ROOM, POS_RESTING );
    act( "You have released $N back to the pattern.",
       ch, NULL, victim, TO_CHAR, POS_RESTING );

    force_quit( victim, "punload" );
}

void do_grab (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA  *victim;
    OBJ_DATA   *obj;
    char 	arg1 [ MAX_INPUT_LENGTH ];
    char 	arg2 [ MAX_INPUT_LENGTH ];
    char 	arg3 [ MAX_INPUT_LENGTH ];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( ( arg1[0] == '\0' ) || ( arg2[0] == '\0' ) )
    {
        send_to_char( "Syntax : grab <object> <player>\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world( ch, arg2 ) ) )
    {
        send_to_char( "They are not here!\n\r", ch );
        return;
    }

    if ( !( obj = get_obj_list( ch, arg1, victim->carrying ) ) )
    {
        send_to_char( "They do not have that item.\n\r", ch );
        return;
    }

    if ( !can_over_ride(ch,victim,FALSE) )
    {
        send_to_char( "You may not confiscate items from your superior.\n\r", ch );
        return;
    }

    if ( obj->wear_loc != WEAR_NONE )
        unequip_char( victim, obj );

    obj_from_char( obj );
    obj_to_char( obj, ch );

    act( "You grab $p from $N.", ch, obj, victim, TO_CHAR,POS_RESTING);
    if ( arg3[0] == '\0' 
    	|| !str_cmp( arg3, "yes" ) || !str_cmp( arg3, "true" ) )		
           act( "An Immortal has confiscated $p.", ch, obj, victim, TO_VICT,POS_DEAD);
    return;
}

void do_ident( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );

        if (argument[0] == '\0')
        {
            sprintf(buf,"Your identity is known as: %s\n\r",ch->pcdata->identity);
            send_to_char(buf,ch);
            return;
        }

        strcat( argument, "{x" );
        free_string( ch->pcdata->identity );
        ch->pcdata->identity = str_dup( argument );

        sprintf(buf,"Your identity is now known as: %s\n\r",ch->pcdata->identity);
        send_to_char(buf,ch);
    }
    return;
}

void do_bonus(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char buf     [ MAX_STRING_LENGTH ];
    char arg1    [ MAX_INPUT_LENGTH ];
    char arg2    [ MAX_INPUT_LENGTH ];
    int value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: bonus <char> <exp>\n\r",ch);
 	return;
    }

    if (( victim = get_pc_world ( ch, arg1 ) ) == NULL )
    {
        send_to_char("That Player is not here.\n\r",ch);
        return;
    }

    if ( ch == victim )
    {
        send_to_char("You may not bonus yourself.\n\r",ch);
        return;
    }

    value = atoi( arg2 );

    if ( value < -1000 || value > 1000 )
    {
        send_to_char( "Value range is -1000 to 1000.\n\r",ch );
        return;
    }

    if ( value == 0 )
    {
        send_to_char("The value must not be equal to 0.\n\r",ch );
        return;
    }

    gain_exp(victim, value);

    sprintf(buf,"You have bonused %s a whopping %d experience points.\n\r",
                victim->name, value);
                send_to_char(buf, ch);

    if ( value > 0 )
    {
        sprintf( buf,"You have been bonused %d experience points.\n\r", value );
        send_to_char(buf, victim );
    }
    else
    {
        sprintf(buf,"You have been penalized %d experience points.\n\r", value );
        send_to_char(buf, victim );
    }
    sprintf(buf,"bonuses %s by %d",victim->name,value);
    parse_logs( ch, "immortal", buf );
    return;

}

void do_vap( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to VAPE spell it out.\n\r", ch );
    return;
}

void do_vape( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    char strsave[MAX_INPUT_LENGTH], bcksave[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    extern int port;

    if ( IS_NPC( ch ) || port != MAIN_GAME_PORT )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: Vape <name> \n\r", ch );
	return;
    }

    if ( ( victim = get_pc_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !can_over_ride( ch, victim, FALSE ) )
    {
	send_to_char( "You failed!\n\r", ch );
	return;
    }

    stop_fighting( victim, TRUE );

    victim->pcdata->pktimer = 0;
    victim->pcdata->opponent = NULL;

    sprintf( strsave, "%s%s/%s", PLAYER_DIR, initial( victim->name ),
	capitalize( victim->name ) );

    sprintf( bcksave, "%s%s/%s", BACKUP_DIR, initial( victim->name ),
	capitalize( victim->name ) );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    sprintf( buf, "{C%s {csends a {yl{Yightnin{yg {Yb{yol{Yt{c flying through the heavens.\n\r"
			  "It streaks down towards Earth, striking {C%s {cin the {rh{Rear{rt{c.\n\r"
			  "A {rf{Rir{reb{Ral{rl{c flares up, and a pile of {wash{c is all that remains.{x\n\r",
		PERS( ch, d->character ), PERS( victim, d->character ) );
	    send_to_char( buf, d->character );
	}
    }

    unrank_charts( victim );

    if ( is_clan( victim ) )
    {
	update_clanlist( victim, FALSE );
	check_roster( victim, TRUE );
    }

    if ( victim->level > HERO )
	update_wizlist( victim, 1 );

    sprintf( buf, "vapes %s.", victim->name );
    parse_logs( ch, "immortal", buf );

    force_quit( victim, "" );

    sprintf( buf, "%svapes/%s.dat",
	DELETED_DIR, strsave+(strlen(PLAYER_DIR)+2) );
    rename( strsave, buf );

    sprintf( buf, "%svapes/%s.bak",
	DELETED_DIR, bcksave+(strlen(BACKUP_DIR)+2) );
    rename( bcksave, buf );

    return;
}

void do_ftick( CHAR_DATA *ch, char *argument )
{
    update_handler( TRUE );
    return;
}

void do_pswitch( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Player-switch into whom?\n\r", ch );
	return;
    }

    if ( get_pc_world( ch, argument ) != NULL )
    {
	send_to_char( "That person is already connected!\n\r", ch );
	return;
    }

    d = new_descriptor( );
    if ( !load_char_obj( d, capitalize( argument ), FALSE, FALSE ) )
    {
	send_to_char( "No such player exists.\n\r", ch );
	free_char( d->character );
	free_descriptor( d );
	return;
    }

    victim = d->character;
    free_descriptor( d );

    victim->desc		= NULL;
    victim->next		= char_list;
    char_list			= victim;
    victim->pcdata->next_player	= player_list;
    player_list			= victim;

    if ( !can_over_ride( ch, victim, FALSE ) )
    {
	send_to_char( "You can't switch into your superiors.\n\r", ch );
	force_quit( victim, "punload" );
	return;
    }

    if ( victim->in_room != NULL )
	victim->pcdata->was_in_room = victim->in_room;

    char_to_room( victim, ch->in_room );

    if ( victim->pet != NULL )
    {
	char_to_room( victim->pet, victim->in_room );
	act( "$n has entered the game.",
	    victim->pet, NULL, NULL, TO_ROOM, POS_RESTING );
    }

    sprintf( buf, "$N player-switches into %s", victim->name );
    wiznet( buf, ch, NULL, WIZ_SWITCHES, WIZ_SECURE, get_trust( ch ) );
    sprintf( buf, "player-switches into %s", victim->name );
    parse_logs( ch, "immortal", buf );

    ch->desc->character	= victim;
    ch->desc->original	= ch;
    victim->desc	= ch->desc;
    ch->desc		= NULL;
    ch->timer		= -5000;

    act( "$n assumes the identity of $N",
	ch, NULL, victim, TO_ROOM, POS_RESTING );

    return;
}

void do_preturn( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) || ch->desc == NULL )
    {
	send_to_char( "Mobiles must use return.\n\r", ch );
	return;
    }

    if ( ch->desc->original == NULL )
    {
        send_to_char( "You aren't player-switched.\n\r", ch );
        return;
    }

    if ( ch->desc->original->pcdata && ch->desc->original->pcdata->tells )
    {
	sprintf( buf, "Player-switch mode removed.  You have {R%d{x tell%s waiting.\n\r",
	    ch->desc->original->pcdata->tells,
	    ch->desc->original->pcdata->tells == 1 ? "" : "s" );
	send_to_char( buf, ch );
	send_to_char( "Type '{Rreplay{x' to see tells.\n\r", ch );
    }
    else
	send_to_char( "Player-switch mode removed.  You have no tells waiting.\n\r", ch );

    sprintf( buf, "$N player-returns to %s.", ch->desc->original->name );
    wiznet( buf, ch->desc->original, 0, WIZ_SWITCHES, WIZ_SECURE, get_trust( ch ) );
    sprintf( buf, "returns from %s.", ch->name );
    parse_logs( ch->desc->original, "immortal", buf );

    if ( ch->pcdata->was_in_room != NULL )
    {
	char_from_room( ch );
	char_to_room( ch, ch->pcdata->was_in_room );
	if ( ch->pet != NULL )
	{
	    char_from_room( ch->pet );
	    char_to_room( ch->pet, ch->in_room );
	}
    }

    ch->desc->character		= ch->desc->original;
    ch->desc->original		= NULL;
    ch->desc->character->desc	= ch->desc;
    ch->desc->character->timer	= 0;
    ch->desc			= NULL;

    force_quit( ch, "punload" );
}

void do_doas (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *orig;
    char arg1[MAX_INPUT_LENGTH];
    bool is_afk;
	    
    argument = one_argument(argument,arg1);

    if (arg1[0] == '\0' || argument[0] == '\0')
    {
	send_to_char("Doas <victim> <command>\n\r",ch);
	return;
    }

    if ( !str_cmp(argument,"delete") || !str_cmp(argument,"reroll")
    ||   !str_cmp(argument,"quit") )
    {
	send_to_char("You can't do that!\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	act("$t not found.",ch,arg1,NULL,TO_CHAR,POS_DEAD);
	return;
    }

    if ( victim == ch )
    {
	send_to_char("Do it yourself!\n\r",ch);
	return;
    }

    if ( !can_over_ride(ch,victim,FALSE) )
    {
	send_to_char("You can not impersonate your superior!\n\r",ch);
	return;
    }

    if ( !check_builder( ch, victim->in_room->area, TRUE ) )
	return;

    orig = victim->desc;
	
    victim->desc = ch->desc;
    ch->desc = NULL;

    if ( IS_SET( victim->comm, COMM_AFK ) )
	is_afk = TRUE;
    else
	is_afk = FALSE;

    interpret( victim, argument );

    ch->desc = victim->desc;	
    victim->desc = orig;

    if ( is_afk )
	SET_BIT( victim->comm, COMM_AFK );
}

void do_addlag (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    int amount;
	    
    argument = one_argument(argument,arg1);

    if (arg1[0] == '\0')
    {
	send_to_char("Addlag <victim> <amount>\n\r",ch);
	return;
    }

    if ( ( victim = get_pc_world( ch, arg1 ) ) == NULL )
    {
	act("$t not found.",ch,NULL,arg1,TO_CHAR,POS_DEAD);
	return;
    }

    if ( victim == ch )
    {
	send_to_char("You lag yourself!\n\r",ch);
	return;
    }

    if ( !can_over_ride(ch,victim,FALSE) )
    {
	send_to_char("You can not lag your superior!\n\r",ch);
	return;
    }

    amount = atoi(argument);

    if (amount < 0 || amount > 150)
    {
	send_to_char("Lag time must be between 0 and 150.\n\r",ch);
	return;
    }

    if (amount > 0)
    {
	act("Applying $t seconds of lag to $N.",ch,argument,victim,TO_CHAR,POS_DEAD);
	send_to_char("You must set their lag time to 0 for it to reset.\n\r",ch);
    } else
	act("Removing lag on $N.",ch,NULL,victim,TO_CHAR,POS_DEAD);
    victim->pcdata->lag = amount;

    return;
}

void do_talk( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int chat;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char("Syntax:\n\r"
		     "  talk <chat channel/clan name> <message>\n\r",ch);
	return;
    }

    if ( is_number(arg1) )
    {
	chat = atoi(arg1);

	sprintf(buf, "You chat <%d> '{d%s{x'\n\r",
	    chat, argument);
	send_to_char(buf,ch);

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    CHAR_DATA *victim;

	    victim = d->original ? d->original : d->character;

	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   !IS_SET(victim->comm,COMM_QUIET)
	    &&   (victim->pcdata->chat_chan == chat
	    ||    IS_SET(victim->wiznet,WIZ_CHATS)) )
	    {
		sprintf( buf, "%s$n{x chats <%d> '{d$t{x'", pretitle(ch,d->character),
		    chat);
		act( buf,ch,argument,d->character,TO_VICT,POS_DEAD);
	    }
	}
	return;
    }

    if ( (chat = clan_lookup(arg1)) == 0 )
    {
	do_talk(ch,"");
	return;
    }

    sprintf( buf, "You clan <%s> '{F%s{x'\n\r", clan_table[chat].color,
	argument );
    send_to_char( buf, ch );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   (d->character->clan == chat
	||    IS_SET(d->character->wiznet,WIZ_CLANS))
	&&   !IS_SET(d->character->comm,COMM_NOCLAN)
	&&   !IS_SET(d->character->comm,COMM_QUIET) )
	{
	    if ( IS_SET(d->character->wiznet,WIZ_CLANS) )
		sprintf( buf, "%s$n{x <%s> clans '{F$t{x'",
		    pretitle(ch,d->character), clan_table[chat].color );
	    else
		sprintf( buf, "%s$n{x clans '{F$t{x'", pretitle(ch,d->character) );

	    act(buf,ch,argument,d->character,TO_VICT,POS_DEAD);
	}
    }

    return;
}

void do_checkeq( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    AFFECT_DATA *paf;
    OBJ_INDEX_DATA *pObj;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],
	 buf[MAX_STRING_LENGTH];
    char *str;
    bool equal = FALSE, great = FALSE, less = FALSE, found = FALSE;
    extern int top_obj_index;
    int pos, match, value = 0, vnum, stat, modif = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char("Syntax:\n\r"
		     "checkeq (damage|cost|(stat)) ( < | = | > ) (value)\n\r",ch);
	return;
    }

    if ( !str_cmp(arg2,">") )
	great = TRUE;
    else if ( !str_cmp(arg2,"<") )
	less = TRUE;
    else if ( !str_cmp(arg2,"=") )
	equal = TRUE;
    else
    {
	send_to_char("Arg2 must be less (\"<\"), greater (\">\") or equal (\"=\").\n\r",ch);
	return;
    }

    if (!is_number(argument))
    {
	send_to_char("Value/Argument must be a number.\n\r",ch);
	return;
    }
    value = atoi(argument);

    if ( !str_prefix(arg1,"damage") )
    {
	pos = 0;
	match = 0;
	final = new_buf();

	for ( vnum = 0; match < top_obj_index; vnum++ )
	{
	    if ( (pObj = get_obj_index(vnum)) != NULL )
	    {
		match++;
		if ( pObj->item_type == ITEM_WEAPON )
		{
		    if ((great && ((1+pObj->value[2]) * pObj->value[1]/2 > value))
		    || (less   && ((1+pObj->value[2]) * pObj->value[1]/2 < value))
		    || (equal  && ((1+pObj->value[2]) * pObj->value[1]/2 == value)) )
		    {
			pos++;
			found = TRUE;
			sprintf(buf,"%3d> (%3d) [%5d] [%3d d %3d <%d>] %s\n\r",
			    pos, pObj->level, pObj->vnum, pObj->value[1],
			    pObj->value[2], ((1+pObj->value[2])*pObj->value[1]/2),
			    pObj->short_descr);
			add_buf(final,buf);
		    }
		}
	    }
	}

	if (!found)
	{
	    sprintf(buf,"No objects %s damage %d exist.\n\r",
		equal ? "matching" : less ? "less than" : "greater than",
		value);
	    send_to_char(buf,ch);
	} else
	    page_to_char(final->string,ch);
	free_buf(final);

	return;
    }

    if ( !str_prefix(arg1,"cost") )
    {
	pos = 0;
	match = 0;
	final = new_buf();

	for ( vnum = 0; match < top_obj_index; vnum++ )
	{
	    if ( (pObj = get_obj_index(vnum)) != NULL )
	    {
		match++;
		if ( (great && pObj->cost > value)
		||   (less   && pObj->cost < value)
		||   (equal  && pObj->cost == value) )
		{
		    pos++;
		    found = TRUE;
		    sprintf(buf,"%3d> (%3d) [%5d] [%-5d] %s\n\r",
			pos, pObj->level, pObj->vnum, pObj->cost,
			pObj->short_descr);
		    add_buf(final,buf);
		}
	    }
	}

	if (!found)
	{
	    sprintf(buf,"No objects %s cost %d exist.\n\r",
		equal ? "matching" : less ? "less than" : "greater than",
		value);
	    send_to_char(buf,ch);
	} else
	    page_to_char(final->string,ch);
	free_buf(final);

	return;
    }

    if ( ( stat = flag_value( apply_flags, arg1 ) ) == NO_FLAG )
    {
	send_to_char("Invalid stat.\n\r",ch);
	return;
    }

    pos = 0;
    match = 0;
    final = new_buf();
    str = flag_string( apply_flags, stat );
    for ( vnum = 0; match < top_obj_index; vnum++ )
    {
	if ( (pObj = get_obj_index(vnum)) != NULL )
	{
	    match++;
	    modif = 0;

	    for ( paf = pObj->affected; paf != NULL; paf = paf->next )
	    {
		if ( paf->where != TO_DAM_MODS && paf->location == stat )
		    modif += paf->modifier;
	    }

	    if ((great && modif > value)
	    || (less   && modif < value)
	    || (equal  && modif == value) )
	    {
		pos++;
		found = TRUE;
		sprintf(buf,"%3d> (%3d) [%5d] [%s: %3d] %s\n\r",
		    pos, pObj->level, pObj->vnum, str, modif,
		    pObj->short_descr);
		add_buf(final,buf);
	    }
	}
    }

    if (!found)
    {
	sprintf(buf,"No objects %s %s %d exist.\n\r", str,
	    equal ? "matching" : less ? "less than" : "greater than",
	    value);
	send_to_char(buf,ch);
    } else
	page_to_char(final->string,ch);
    free_buf(final);

    return;
}

void do_exp_mod( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    sh_int value[2];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax: exp_mod [v1] [v2]\n\r"
		      "   Calculated as: exp = (v1 * exp / v2)\n\r"
		      "(\"exp_mod auto\" may also be specifed)\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "none" ) )
    {
	send_to_char( "Exp_modifier cancelled.\n\r", ch );
	info( "{Y>{y> {GXP{Y--[ {GExperience modifier cancelled!{Y ]--{GXP {y<{Y<{x", ch, NULL, INFO_OTHER );
	mud_stat.exp_mod[0] = 1;
	mud_stat.exp_mod[1] = 1;
	mud_stat.changed = TRUE;
	return;
    }
    
    if ( !is_number( arg ) )
    {
	do_exp_mod( ch, "" );
	return;
    }

    value[0] = atoi( arg );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	value[1] = 1;
	sprintf( buf, "{Y>{y> {GXP{Y--[ {GExperience is now multiplied by {R%d{G!{Y ]--{GXP {y<{Y<{x",
	    value[0] );
    } else {
	value[1] = atoi( argument );
	sprintf( buf, "{Y>{y> {GXP{Y--[ {GExperience is now multiplied by {R%d {G/ {R%d{G!{Y ]--{GXP {y<{Y<{x",
	    value[0], value[1] );
    }
    
    if ( value[0] == 0 || value[1] == 0 )
    {
	send_to_char( "Zero is not acceptable!\n\r", ch );
	return;
    }
    
    mud_stat.exp_mod[0] = value[0];
    mud_stat.exp_mod[1] = value[1];
    mud_stat.changed = TRUE;

    info( buf, ch, NULL, INFO_OTHER );

    sprintf( buf, "{GPlayers will now gain {R%d {G/ {R%d {Gnormal exp.\n\r"
		  "{GTyping: \"exp_mod none\" or \"exp_mod auto\" will cancel this.{x\n\r",
	mud_stat.exp_mod[0], mud_stat.exp_mod[1] );
    send_to_char( buf, ch );
}

void parse_logs( CHAR_DATA *ch, char *path, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    sh_int value = strlen( path )+8;
    extern int port;

    if ( port == MAIN_GAME_PORT )
	sprintf( buf, "../log/%s_%s.txt", path,
	    ch->desc && ch->desc->original ?
	    capitalize( ch->desc->original->name ) : capitalize( ch->name ) );
    else
	sprintf( buf, "../log/%s_%s.test", path,
	    ch->desc && ch->desc->original ?
	    capitalize( ch->desc->original->name ) : capitalize( ch->name ) );

    buf[value] = LOWER( buf[value] );

    append_file( ch, buf, argument );
}

ROOM_INDEX_DATA * get_scatter_room( CHAR_DATA *ch, int min, int max )
{
    ROOM_INDEX_DATA *pRoom;
    sh_int pos;

    if ( min == 0 || max == 0 )
	return get_random_room( ch );

    for ( pos = 0; pos < 100; pos++ )
    {
	if ( ( pRoom = get_room_index( number_range( min, max ) ) ) != NULL )
	    return pRoom;
    }

    return get_room_index( ROOM_VNUM_ALTAR );
}

void do_scatter( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim, *victim_next;
    OBJ_DATA *obj, *obj_next;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int min, max;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char(	"Syntax: scatter <objects|mobiles|players> [<min_vnum> <max_vnum>]\n\r"
			"Note:   min_vnum and max_vnum are optional.\n\r", ch );
	return;
    }

    else if ( arg2[0] == '\0' )
    {
	min = 0;
	max = 0;
    }

    else if ( argument[0] == '\0' || !is_number( arg2 ) || !is_number( argument ) )
    {
	do_scatter( ch, "" );
	return;
    }

    else
    {
	min = atoi( arg2 );
	max = atoi( argument );

	if ( min > max )
	{
	    send_to_char( "Max vnum must be greater than min vnum.\n\r", ch );
	    return;
	}

	if ( get_room_index( min ) == NULL )
	{
	    send_to_char( "Can not find min vnum.\n\r", ch );
	    return;
	}

	if ( get_room_index( max ) == NULL )
	{
	    send_to_char( "Can not find max vnum.\n\r", ch );
	    return;
	}
    }

    if ( !str_prefix( arg1, "objects" ) )
    {
	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( CAN_WEAR( obj, ITEM_TAKE ) )
	    {
		obj_from_room( obj );
		obj_to_room( obj, get_scatter_room( ch, min, max ) );
	    }
	}

	send_to_char( "Items scattered.\n\r", ch );
    }

    else if ( !str_prefix( arg1, "mobiles" ) )
    {
	for ( victim = ch->in_room->people; victim != NULL; victim = victim_next )
	{
	    victim_next = victim->next_in_room;

	    if ( IS_NPC( victim ) && !IS_SET( victim->act, ACT_AGGRESSIVE ) )
	    {
		char_from_room( victim );
		char_to_room( victim, get_scatter_room( victim, min, max ) );
	    }
	}

	send_to_char( "Mobiles scattered.\n\r", ch );
    }

    else if ( !str_prefix( arg1, "players" ) )
    {
	for ( victim = ch->in_room->people; victim != NULL; victim = victim_next )
	{
	    victim_next = victim->next_in_room;

	    if ( !IS_NPC( victim ) && !IS_IMMORTAL( victim ) )
	    {
		char_from_room( victim );
		char_to_room( victim, get_scatter_room( victim, min, max ) );
		send_to_char( "{CYou have been scattered!{x\n\r", victim );
		do_look( victim, "auto" );
	    }
	}

	send_to_char( "Players scattered.\n\r", ch );
    }

    else
	do_scatter( ch, "" );
}

void append_todo( CHAR_DATA *ch, char *file, char *str )
{
    FILE *fp;
    char *strtime;
    char buf[MAX_STRING_LENGTH];

    if ( ( ch != NULL && IS_NPC( ch ) ) || str[0] == '\0' )
	return;

    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-9] = '\0';
    strncpy( buf, strtime+4, 6 );

    buf[6] = '\0';

    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
	perror( file );
	if ( ch != NULL )
	    send_to_char( "Could not open the file!\n\r", ch );
    } else {
	if( !str_cmp( file, BUG_FILE ) || !str_cmp( file, TYPO_FILE ) )
	{
	    int vnum;

	    vnum = ch && ch->in_room ? ch->in_room->vnum : 0;
	    fprintf( fp, "[{t%-12s {s- {t%5d{s] {q%s: {t%s\n",
		ch ? ch->name : "", vnum, buf, str );
	}

	else
	    fprintf( fp, "[{t%-12s{s] {q%s: {t%s\n",
		ch ? ch->name : "", buf, str );

	fclose( fp );
    }
}

void parse_todo( CHAR_DATA *ch, char *argument, char *command, char *file )
{
    FILE *fp;
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
	printf_to_char( ch, "Syntax:  %s add <item>\n\r", command );
	printf_to_char( ch, "         %s show\n\r", command );
	printf_to_char( ch, "         %s delete <line no>\n\r", command );
	printf_to_char( ch, "         %s clear\n\r", command );
	return;
    }

    argument = one_argument( argument, arg );

    if ( !str_prefix( arg, "show" ) || !str_prefix( arg, "list" ) )
    {
	BUFFER *output;
	char *line;
	char c = '[';
	int i = 1;

	if ( ( fp = fopen( file, "r" ) ) == NULL )
	{
	    printf_to_char( ch, "The %s list is empty.\n\r", command );
	    return;
	}

	output = new_buf( );

	while( !feof( fp ) )
	{
	    line = fread_string_eol( fp );
	    sprintf( buf, "{q%3d{s) %s\n\r", i++, line );
	    add_buf( output, buf );
	    free_string(line);

	    while( !feof( fp ) && ( c = fread_letter( fp ) ) != '[' )
		;

	    if ( !feof( fp ) )
		ungetc( c, fp );
	}

	add_buf( output, "{x" );

	page_to_char( output->string, ch );
	free_buf( output );
	fclose( fp );
	return;
    }

    else if ( !str_prefix( arg, "add" ) )
    {
	append_todo( ch, file, argument );
	printf_to_char( ch, "Line added to the %s list.\n\r", command );
	return;
    }

    else if ( !str_cmp( arg, "delete" ) )
    {
	FILE *fout;
	char *line, c = '[';
	bool empty = TRUE, found = FALSE;
	int i = 1, num;

	one_argument( argument, arg );

	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    printf_to_char( ch, "Syntax: %s delete <line number>\n\r", command );
	    return;
	}

	num = atoi( arg );

	if ( ( fp = fopen( file, "r" ) ) == NULL )
	{
	    printf_to_char( ch, "The %s list is empty.\n\r", command );
	    return;
	}

	if ( ( fout = fopen( "../data/temp.dat", "w" ) ) == NULL )
	{
	    fclose( fp );
	    send_to_char( "Error opening temporary file.\n\r", ch );
	    return;
	}

	while( !feof( fp ) )
	{
	    line = fread_string_eol( fp );

	    if ( i++ != num )
	    {
		empty = FALSE;
		fprintf( fout, "%s\n\r", line );
	    } else
		found = TRUE;

	    free_string( line );

	    while( !feof( fp ) && ( c = fread_letter( fp ) ) != '[' )
		;

	    if ( !feof( fp ) )
		ungetc(c,fp);
	}

	fclose( fout );
	fclose( fp );

	if ( empty )
	{
	    printf_to_char( ch, "The %s list has been cleared.\n\r", command );
	    unlink( file );
	    return;
	}

	if ( !found )
	    send_to_char( "That line number does not exist.\n\r", ch );
	else
	{
	    rename( "../data/temp.dat", file );
	    printf_to_char( ch, "Line deleted from %s list.\n\r", command );
	}

	return;
    }

    else if ( !str_cmp( arg, "clear" ) )
    {
	if ( ( fp = fopen( file, "r" ) ) == NULL )
	{
	    printf_to_char( ch, "The %s list is empty.\n\r", command );
	    return;
	}

	fclose( fp );
	printf_to_char( ch, "The %s list has been cleared.\n\r", command );
	unlink( file );
	return;
    }

    parse_todo( ch, "", command, file );
    return;
}

void do_changed( CHAR_DATA *ch, char *argument )
{
    if ( IS_IMMORTAL( ch ) )
	parse_todo( ch, argument, "changed", CHANGED_FILE );
    else
	parse_todo( ch, "show", "changed", CHANGED_FILE );

    return;
}

void do_todo( CHAR_DATA *ch, char *argument )
{
    parse_todo( ch, argument, "todo", TO_DO_FILE );
    return;
}

void do_tocode( CHAR_DATA *ch, char *argument )
{
    parse_todo( ch, argument, "tocode", TO_CODE_FILE );
    return;
}

void do_bug( CHAR_DATA *ch, char *argument )
{
    if ( IS_IMMORTAL( ch ) )
    {
	parse_todo( ch, argument, "bug", BUG_FILE );
	return;
    }

    append_todo( ch, BUG_FILE, argument );
    send_to_char( "Bug logged.\n\r", ch );
    return;
}

void do_typo( CHAR_DATA *ch, char *argument )
{
    if ( IS_IMMORTAL( ch ) )
    {
	parse_todo( ch, argument, "typo", TYPO_FILE );
	return;
    }

    append_todo( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}

void do_nohelp( CHAR_DATA *ch, char *argument )
{
    if ( IS_IMMORTAL( ch ) )
    {
	parse_todo( ch, argument, "nohelp", NO_HELP_FILE );
	return;
    }

    append_todo( ch, NO_HELP_FILE, argument );
    send_to_char( "Missing help file logged.\n\r", ch );
    return;
}

void do_plist( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    FILE *list;
    char buf[MAX_STRING_LENGTH];
    char socket[MAX_INPUT_LENGTH];
    bool fClass[maxClass], fClassRestrict = FALSE;
    bool fRace[maxRace], fRaceRestrict = FALSE;
    bool fClan[maxClan], fClanRestrict = FALSE;
    bool fActive = FALSE, fBackup = FALSE, fClanned = FALSE, fInactive = FALSE, fPK = FALSE, fNonPK = FALSE;
    int count, days, iLevelLower = 0, iLevelUpper = MAX_LEVEL, total = 0;

    if ( argument[0] == '\0' )
    {
	send_to_char(	"Syntax: plist [ arguments ]\n\r"
			"\n\rStatic Arguments:\n\r"
			"  active, all, backup, clan, immortals, inactive, non-pk, pk\n\r"
			"\n\rVariable Arguments:\n\r"
			"  <initial letter>, <race>, <class>, <clan>, <min_level> <max_level>, <socket>\n\r", ch );
	return;
    }

    socket[0] = '\0';

    for ( count = 0; count < maxClass; count++ )
	fClass[count] = FALSE;

    for ( count = 0; count < maxRace; count++ )
	fRace[count] = FALSE;

    for ( count = 0; count < maxClan; count++ )
	fClan[count] = FALSE;

    if ( !str_cmp( argument, "all" ) )
	system( "dir -1 ../player/*/* >../player/names.txt" );

    else
    {
	char arg[MAX_INPUT_LENGTH], initial = '\0';
	int nNumber = 0;

	for ( ; ; )
	{
	    argument = one_argument( argument, arg );

	    if ( arg[0] == '\0' )
		break;

	    else if ( is_number( arg ) )
	    {
		switch( ++nNumber )
		{
		    case 1:	iLevelLower = atoi( arg );	break;
		    case 2:	iLevelUpper = atoi( arg );	break;
		    default:
			send_to_char( "Only two levels allowed.\n\r", ch );
			return;
		}
	    }

	    else if ( strlen( arg ) == 1 )
	    {
		if ( !isalpha( LOWER( *arg ) ) )
		{
		    send_to_char( "Single arguments must be initial pfile letters.\n\r", ch );
		    return;
		}

		initial = LOWER( *arg );
	    }

	    else if ( !str_prefix( arg, "active" ) )
		fActive = TRUE;

	    else if ( !str_prefix( arg, "backups" ) )
		fBackup = TRUE;

	    else if ( !str_prefix( arg, "clan" ) )
		fClanned = TRUE;

	    else if ( !str_prefix( arg, "immortals" ) )
		iLevelLower = LEVEL_IMMORTAL;

	    else if ( !str_prefix( arg, "inactive" ) )
		fInactive = TRUE;

	    else if ( !str_cmp( arg, "pk" ) )
		fPK = TRUE;

	    else if ( !str_cmp( arg, "non-pk" ) )
		fNonPK = TRUE;

	    else if ( ( count = class_lookup( arg ) ) != -1 )
	    {
		fClassRestrict = TRUE;
		fClass[count] = TRUE;
	    }

	    else if ( ( count = race_lookup( arg ) ) != -1 )
	    {
		fRaceRestrict = TRUE;
		fRace[count] = TRUE;
	    }

	    else if ( ( count = clan_lookup( arg ) ) != 0 )
	    {
		fClanRestrict = TRUE;
		fClan[count] = TRUE;
	    }

	    else
		strcpy( socket, arg );
	}

	if ( fBackup )
	{
	    if ( initial == '\0' )
		system( "dir -1 ../backup/*/* >../player/names.txt" );
	    else
	    {
		sprintf( buf, "dir -1 ../backup/%c/* >../player/names.txt", initial );
		system( buf );
	    }
	} else {
	    if ( initial == '\0' )
		system( "dir -1 ../player/*/* >../player/names.txt" );
	    else
	    {
		sprintf( buf, "dir -1 ../player/%c/* >../player/names.txt", initial );
		system( buf );
	    }
	}
    }

    if ( ( list = fopen( "../player/names.txt", "a" ) ) == NULL )
	return;

    fprintf( list, "#\n\r" );
    fclose( list );

    if ( ( list = fopen( "../player/names.txt", "r" ) ) != NULL )
    {
	FILE *fp;
	CHAR_DATA *wch;
	char lvl[10], name[20], time[25];

	count = 0;
	final = new_buf( );

	add_buf( final, "[Lvl Race Class] Player       Clan        Date       Time  #Days Last Socket\n"
			"--------------------------------------------------------------------------------\n" );

	for ( ; ; )
	{
	    strcpy( name, fread_word( list ) );

	    if ( name[0] == '#' )
		break;

	    wch = new_char( );
	    wch->pcdata = new_pcdata( );

	    if ( ( fp = fopen( name, "r" ) ) != NULL )
	    {
		char *word = fread_word( fp );

		if ( !str_cmp( word, "#PLAYER" ) )
		    fread_char( wch, fp, 2 );

		fclose( fp );
	    }

	    total++;

	    if ( wch->level < iLevelLower
	    ||   wch->level > iLevelUpper
	    ||   ( fPK && !is_pkill( wch ) )
	    ||   ( fNonPK && is_pkill( wch ) )
	    ||   ( fClanned && !is_clan( wch ) )
	    ||   ( fClassRestrict && !fClass[wch->class] )
	    ||   ( fRaceRestrict && !fRace[wch->race] )
	    ||   ( fClanRestrict && !fClan[wch->clan] )
	    ||   ( socket[0] != '\0' && str_infix( socket, wch->pcdata->socket ) )
	    ||   !can_over_ride( ch, wch, TRUE ) )
	    {
		free_char( wch );
		continue;
	    }

	    days = ( current_time - wch->pcdata->llogoff ) / 86400;

	    if ( ( fActive && days > 30 )
	    ||   ( fInactive && days < 30 ) )
	    {
		free_char( wch );
		continue;
	    }

	    switch ( wch->level )
	    {
		default:
		    sprintf( lvl, "%3d", wch->level );
		    break;

		case MAX_LEVEL:
		    sprintf( lvl, "{yIMP{x" );
		    break;

		case MAX_LEVEL - 1:
		    sprintf( lvl, "{yCRE{x" );
		    break;

		case MAX_LEVEL - 2:
		    sprintf( lvl, "{ySUP{x" );
		    break;

		case MAX_LEVEL - 3:
		    sprintf( lvl, "{yDEI{x" );
		    break;

		case MAX_LEVEL - 4:
		    sprintf( lvl, "{yGOD{x" );
		    break;

		case MAX_LEVEL - 5:
		    sprintf( lvl, "{yIMM{x" );
		    break;

		case MAX_LEVEL - 6:
		    sprintf( lvl, "{yDEM{x" );
		    break;

		case MAX_LEVEL - 7:
		    sprintf( lvl, "{yKNI{x" );
		    break;

		case MAX_LEVEL - 8:
		    sprintf( lvl, "{ySQU{x" );
		    break;

		case MAX_LEVEL - 9:
		    sprintf( lvl, "{mHRO{x" );
		    break;
	    }

	    strftime( time, 50, "%Y/%m/%d %H:%M",
		localtime( &wch->pcdata->llogoff ) );

	    sprintf( buf, "[%s %s %s] %-12s %s %s ({%c%3d{x) %s\n",
		lvl, race_table[wch->race].who_name,
		class_table[wch->class].who_name, wch->name,
		clan_table[wch->clan].who_name, time,
		days > 30 ? 'r' : days > 20 ? 'Y' : days > 10 ? 'g' : 'G',
		days, wch->pcdata->socket );
	    add_buf( final, buf );
	    free_char( wch );
	    count++;
	}

	fclose( list );

	sprintf( buf, "\nMatches Found: %d (%d)\n\r",
	    count, total );
	add_buf( final, buf );

	unlink( "../player/names.txt" );
	page_to_char( final->string, ch );
	free_buf( final );
    }
}

AREA_DATA *get_area_from_editor( CHAR_DATA *ch )
{
    AREA_DATA *pArea = NULL;

    if ( ch->desc == NULL )
	return NULL;

    switch( ch->desc->editor )
    {
	default:
	    pArea = ch->in_room ? ch->in_room->area : NULL;
	    break;

	case ED_ROOM:
	    pArea = ( ( ROOM_INDEX_DATA * ) ch->desc->pEdit )->area;
	    break;

	case ED_AREA:
	    pArea = ( AREA_DATA * ) ch->desc->pEdit;
	    break;

	case ED_MOBILE:
	    pArea = ( ( MOB_INDEX_DATA * ) ch->desc->pEdit )->area;
	    break;

	case ED_OBJECT:
	    pArea = ( ( OBJ_INDEX_DATA * ) ch->desc->pEdit )->area;
	    break;
    }

    if ( pArea == NULL )
	return NULL;

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "Insufficient security.\n\r", ch );
	return NULL;
    }

    return pArea;
}

void do_rlist( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    BUFFER *final;
    ROOM_INDEX_DATA *pRoomIndex;
    char buf[MAX_STRING_LENGTH];
    bool fAll;
    int col = 0, vnum;

    if ( ( pArea = get_area_from_editor( ch ) ) == NULL )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  rlist < all / name >\n\r", ch );
	return;
    }

    fAll  = !str_cmp( argument, "all" );
    final = new_buf( );

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pRoomIndex = get_room_index( vnum ) )
	&&   ( fAll || is_name( argument, strip_color( pRoomIndex->name ) ) ) )
	{
	    sprintf( buf, "[%5d] %s ",
		vnum, end_string( pRoomIndex->name, 30 ) );
	    add_buf( final, buf );

	    if ( ++col % 2 == 0 )
		add_buf( final, "\n" );
	}
    }

    if ( col % 2 != 0 )
	add_buf ( final, "\n" );

    if ( col == 0 )
	send_to_char( "Room(s) not found in this area.\n\r", ch );
    else
	page_to_char( final->string, ch );

    free_buf( final );
}

void do_mlist( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMobIndex;
    char buf[MAX_STRING_LENGTH];
    bool fAll;
    int col = 0, vnum;

    if ( ( pArea = get_area_from_editor( ch ) ) == NULL )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  mlist < all / name >\n\r", ch );
	return;
    }

    fAll  = !str_cmp( argument, "all" );
    final = new_buf( );

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    if ( fAll || is_name( argument, pMobIndex->player_name ) )
	    {
                sprintf( buf, "[%5d] %s ", pMobIndex->vnum,
		    end_string( pMobIndex->short_descr, 30 ) );
		add_buf( final, buf );

                if ( ++col % 2 == 0 )
		    add_buf( final, "\n" );
	    }
	}
    }

    if ( col % 2 != 0 )
	add_buf( final, "\n" );

    if ( col == 0 )
	send_to_char( "Mobile(s) not found in this area.\n\r", ch );
    else
	page_to_char( final->string, ch );

    free_buf( final );
}

bool has_reset( int vnum )
{
    RESET_DATA *pReset;
    ROOM_INDEX_DATA *pRoom;
    sh_int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
	{
	    for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
	    {
		switch ( pReset->command )
		{
		    default:
			break;

		    case 'O':
		    case 'E':
		    case 'P':
		    case 'G':
			if ( pReset->arg1 == vnum
                        ||   ( pReset->command == 'P' && pReset->arg3 == vnum ) )
			    return TRUE;
		}
	    }
	}
    }

    return FALSE;
}

void do_olist( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    BUFFER *final;
    OBJ_INDEX_DATA *pObjIndex;
    char buf[MAX_STRING_LENGTH];
    bool fAll, fReset;
    sh_int col = 0, type;
    int vnum;

    if ( ( pArea = get_area_from_editor( ch ) ) == NULL )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  olist <all/no-reset/name/item_type>\n\r", ch );
	return;
    }

    fAll = !str_cmp( argument, "all" );
    fReset = !str_cmp( argument, "no-reset" );
    type = flag_value( type_flags, argument );

    final = new_buf( );

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) )
	{
	    if ( fAll
	    ||   ( fReset && !has_reset( vnum ) )
	    ||   pObjIndex->item_type == type
	    ||   is_name( argument, pObjIndex->name ) )
	    {
                sprintf( buf, "[%5d] %s ", pObjIndex->vnum,
		    end_string( pObjIndex->short_descr, 30 ) );
		add_buf( final, buf );

                if ( ++col % 2 == 0 )
		    add_buf( final, "\n" );
	    }
	}
    }

    if ( col % 2 != 0 )
	add_buf( final, "\n" );

    if ( col == 0 )
	send_to_char( "Object(s) not found in this area.\n\r", ch );
    else
	page_to_char( final->string, ch );

    free_buf( final );
}
