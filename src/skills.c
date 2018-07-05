/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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
#include "merc.h"
#include "magic.h"
#include "recycle.h"

/* command procedures needed */
DECLARE_DO_FUN(do_groups	);
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_say		);

int spell_num;

void	do_class	args( ( CHAR_DATA *ch, char *argument ) );

char *color_percent( sh_int percent )
{
    static char buf[10];

    if ( percent >= 100 )
	sprintf( buf, "{w%d", percent );
    else if ( percent >= 90 )
	sprintf( buf, "{g%d", percent );
    else if ( percent >= 80 )
	sprintf( buf, "{Y%d", percent );
    else if ( percent >= 70 )
	sprintf( buf, "{y%d", percent );
    else if ( percent >= 50 )
	sprintf( buf, "{R%d", percent );
    else
	sprintf( buf, "{r%d", percent );

    return buf;
}

sh_int get_skill_level( CHAR_DATA *ch, sh_int sn )
{
    sh_int num;

    for ( num = 0; num < 5; num++ )
    {
	if ( race_table[ch->race].skills[num] == NULL )
	    break;

	if ( !str_cmp(skill_table[sn].name,race_table[ch->race].skills[num]) )
	    return 1;
    }

    return skill_table[sn].skill_level[ch->class];
}

sh_int get_skill_rating( CHAR_DATA *ch, sh_int sn )
{
    sh_int num;

    for ( num = 0; num < 5; num++ )
    {
	if ( race_table[ch->race].skills[num] == NULL )
	    break;

	if ( !str_cmp(skill_table[sn].name,race_table[ch->race].skills[num]) )
	    return 3;
    }

    return skill_table[sn].rating[ch->class];
}

sh_int get_group_rating( CHAR_DATA *ch, sh_int sn )
{
    return group_table[sn].rating[ch->class];
}

void do_gain( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *trainer;
    int gn = 0, sn = 0;

    if (IS_NPC(ch))
	return;

    /* find a trainer */
    for ( trainer = ch->in_room->people;
	  trainer != NULL;
	  trainer = trainer->next_in_room)
	if (IS_NPC(trainer) && IS_SET(trainer->act,ACT_GAIN))
	    break;

    if (trainer == NULL)
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	do_say(trainer,"{aPardon me?{x");
	return;
    }

    if (!str_prefix(arg,"list"))
    {
	BUFFER *final = new_buf();
	int col;

	col = 0;

	add_buf(final,"{y----------------------------------{WGROUPS{y----------------------------------\n\r"
		      "{cgroup             {mcost    {cgroup             {mcost    {cgroup             {mcost\n\r" );

	for ( gn = 0; group_table[gn].name[0] != '\0'; gn++ )
	{
	    if ( !ch->pcdata->group_known[gn]
	    &&   get_group_rating(ch,gn) > 0 )
	    {
		sprintf(buf,"{c%-19s{m%3d    ",
		    group_table[gn].name,get_group_rating(ch,gn));
		add_buf(final,buf);
		if (++col % 3 == 0)
		    add_buf(final,"\n\r");
	    }
	}
	if (col % 3 != 0)
	    add_buf(final,"\n\r");

	add_buf(final,"\n\r");

	col = 0;

	add_buf(final,"{y----------------------------------{WSKILLS{y----------------------------------\n\r");
	add_buf(final,"{wlvl {cskill         {mcost    {wlvl {cskill         {mcost    {wlvl {cskill         {mcost\n\r");
        
        for (sn = 0; skill_table[sn].name[0] != '\0'; sn++)
        {
            if ( !ch->learned[sn]
            &&   get_skill_rating(ch,sn) > 0
	    &&   skill_table[sn].spell_fun == spell_null
	    &&   get_skill_level(ch,sn) < LEVEL_IMMORTAL )
            {
                sprintf(buf,"{w%3d {c%-15s {m%2d    ",
		    get_skill_level(ch,sn), skill_table[sn].name,
		    get_skill_rating(ch,sn));
		add_buf(final,buf);

                if (++col % 3 == 0)
		    add_buf(final,"\n\r");
            }
        }
        if (col % 3 != 0)
	    add_buf(final,"\n\r");

	add_buf(final,"{y--------------------------------------------------------------------------{x\n\r\n\r");
	page_to_char(final->string,ch);
	free_buf(final);
        return;
    }

    if (!str_prefix(arg,"convert"))
    {
	char arg1[MAX_INPUT_LENGTH];
	int num = 1;

	argument = one_argument( argument, arg1 );

	if (argument[0] != '\0' && is_number(argument) )
	    num = atoi(argument);

	if (num < 1)
	{
	    act("$N tells you '{UNumber must be greater than zero.{x'",
		ch,NULL,trainer,TO_CHAR,POS_RESTING);
	    return;
	}

	if (ch->pcdata->practice < 6 * num)
	{
	    act("$N tells you '{UYou do not have enough practices.{x'",
		ch,NULL,trainer,TO_CHAR,POS_RESTING);
	    return;
	}

	act("$N helps you apply your practice to training",
		ch,NULL,trainer,TO_CHAR,POS_RESTING);
	ch->pcdata->practice -= 6*num;
	ch->pcdata->train +=num ;
	return;
    }

    if (!str_prefix(arg,"study"))
    {
	char arg1[MAX_INPUT_LENGTH];
	int num = 1;

	argument = one_argument( argument, arg1 );

	if (argument[0] != '\0' && is_number(argument) )
	    num = atoi(argument);

	if (num < 1)
	{
	    act("$N tells you '{UNumber must be greater than zero.{x'",
		ch,NULL,trainer,TO_CHAR,POS_RESTING);
	    return;
	}

	if (ch->pcdata->train < num)
	{
            act("$N tells you '{UYou do not have enough trains.{x'",
                ch,NULL,trainer,TO_CHAR,POS_RESTING);
            return;
        }

        act("$N helps you apply your training to practice",
                ch,NULL,trainer,TO_CHAR,POS_RESTING);
        ch->pcdata->train -= num;
	ch->pcdata->practice += 6*num;
	return;
    }

    if (!str_prefix(arg,"points"))
    {
	long exp_dif, exp_tnl;

	if (ch->pcdata->train < 2)
	{
	    act("$N tells you '{aYou are not yet ready.{x'",
		ch,NULL,trainer,TO_CHAR,POS_RESTING);
	    return;
	}

	if (ch->pcdata->points <= 40)
	{
	    act("$N tells you '{aThere would be no point in that.{x'",
		ch,NULL,trainer,TO_CHAR,POS_RESTING);
	    return;
	}

	act("$N trains you, and you feel more at ease with your skills.",
	    ch,NULL,trainer,TO_CHAR,POS_RESTING);

	exp_tnl = ( ( ch->level + 1 ) * exp_per_level( ch, ch->pcdata->points ) - ch->exp );

	ch->pcdata->train -= 2;
	ch->pcdata->points -= 1;
	ch->exp = exp_per_level( ch, ch->pcdata->points ) * ch->level;

	exp_dif = ( ( ch->level + 1 ) * exp_per_level( ch, ch->pcdata->points ) - ch->exp ) - exp_tnl;

	if ( exp_dif > 0 )
	    ch->exp += exp_dif;

	return;
    }

    /* else add a group/skill */

    gn = group_lookup(argument);
    if (gn > 0)
    {
	if (ch->pcdata->group_known[gn])
	{
	    act("$N tells you '{aYou already know that group!{x'",
		ch,NULL,trainer,TO_CHAR,POS_RESTING);
	    return;
	}

	if ( get_group_rating(ch,gn) <= 0 )
	{
	    act("$N tells you '{aThat group is beyond your powers.{x'",
		ch,NULL,trainer,TO_CHAR,POS_RESTING);
	    return;
	}

	if ( ch->pcdata->train < get_group_rating(ch,gn) )
	{
	    act("$N tells you '{aYou are not yet ready for that group.{x'",
		ch,NULL,trainer,TO_CHAR,POS_RESTING);
	    return;
	}

	/* add the group */
	gn_add(ch,gn);
	act("$N trains you in the art of $t",
	    ch,group_table[gn].name,trainer,TO_CHAR,POS_RESTING);
	ch->pcdata->train -= get_group_rating(ch,gn);
	return;
    }

    sn = skill_lookup(argument);
    if (sn > -1)
    {
	if (skill_table[sn].spell_fun != spell_null)
	{
	    act("$N tells you '{aYou must learn the full group.{x'",
		ch,NULL,trainer,TO_CHAR,POS_RESTING);
	    return;
	}


        if (ch->learned[sn])
        {
            act("$N tells you '{aYou already know that skill!{x'",
                ch,NULL,trainer,TO_CHAR,POS_RESTING);
            return;
        }

	if ( get_skill_rating(ch,sn) <= 0 )
        {
            act("$N tells you '{aThat skill is beyond your powers.{x'",
                ch,NULL,trainer,TO_CHAR,POS_RESTING);
            return;
        }

	if ( ch->pcdata->train < get_skill_rating(ch,sn) )
        {
            act("$N tells you '{aYou are not yet ready for that skill.{x'",
                ch,NULL,trainer,TO_CHAR,POS_RESTING);
            return;
        }

        /* add the skill */
	ch->learned[sn] = 1;
        act("$N trains you in the art of $t",
            ch,skill_table[sn].name,trainer,TO_CHAR,POS_RESTING);
        ch->pcdata->train -= get_skill_rating(ch,sn);
        return;
    }

    act("$N tells you '{aI do not understand...{x'",ch,NULL,trainer,TO_CHAR,POS_RESTING);
}

void do_spells( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    char spell_list[LEVEL_IMMORTAL][1024];
    char buf[MAX_STRING_LENGTH], spell_columns[LEVEL_IMMORTAL];
    bool found = FALSE;
    sh_int sn, lev;

    for ( lev = 0; lev < LEVEL_IMMORTAL; lev++ )
    {
	spell_columns[lev] = 0;
	spell_list[lev][0] = '\0';
    }

    for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
    {
	if ( ( lev = get_skill_level( ch, sn ) ) < LEVEL_IMMORTAL
	&&     skill_table[sn].spell_fun != spell_null
	&&     ch->learned[sn] > 0 )
	{
	    found = TRUE;

	    if ( ch->level < lev )
		sprintf( buf, "{W%-18.18s       {wn/a     ",
		    skill_table[sn].name );
	    else
	    {
		sprintf( buf, "{W%-18.18s {C%3d {cmana %5s{w%% ",
		    skill_table[sn].name, skill_table[sn].cost_mana,
		    color_percent( ch->learned[sn] ) );
	    }

	    if ( spell_list[lev][0] == '\0' )
		sprintf( spell_list[lev], "\n\r{DLevel {m%3d: %s{x", lev, buf );
	    else
	    {
		if ( ++spell_columns[lev] % 2 == 0 )
		    strcat( spell_list[lev], "\n\r           " );
		strcat( spell_list[lev], buf );
	    }
	}
    }

    if ( !found )
    {
	send_to_char( "You know no spells.\n\r", ch );
	return;
    }

    final = new_buf( );

    for ( lev = 0; lev < LEVEL_IMMORTAL; lev++ )
    {
	if ( spell_list[lev][0] != '\0' )
	    add_buf( final, spell_list[lev] );
    }

    add_buf( final, "{x\n\r" );
    page_to_char( final->string, ch );
    free_buf( final );
}

void do_skills( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    char skill_list[LEVEL_IMMORTAL][1024];
    char buf[MAX_STRING_LENGTH], skill_columns[LEVEL_IMMORTAL];
    bool found = FALSE;
    sh_int lev, sn;

    for ( lev = 0; lev < LEVEL_IMMORTAL; lev++ )
    {
	skill_columns[lev] = 0;
	skill_list[lev][0] = '\0';
    }

    for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
    {
	if ( ( lev = get_skill_level( ch, sn ) ) < LEVEL_IMMORTAL
	&&     skill_table[sn].spell_fun == spell_null
	&&     ch->learned[sn] > 0 )
	{
	    found = TRUE;

	    if ( ch->level < lev )
		sprintf( buf, "{W%-18.18s  {wn/a      ", skill_table[sn].name );
	    else
		sprintf( buf, "{W%-18.18s %5s{w%%      ",
		    skill_table[sn].name, color_percent( ch->learned[sn] ) );

	    if ( skill_list[lev][0] == '\0' )
		sprintf( skill_list[lev], "\n\r{DLevel {m%3d: %s{x", lev, buf );
	    else
	    {
		if ( ++skill_columns[lev] % 2 == 0 )
		    strcat( skill_list[lev], "\n\r           " );
		strcat( skill_list[lev], buf );
	    }
	}
    }

    if ( !found )
    {
	send_to_char( "You know no skills.\n\r", ch );
	return;
    }

    final = new_buf( );

    for ( lev = 0; lev < LEVEL_IMMORTAL; lev++ )
    {
	if ( skill_list[lev][0] != '\0' )
	    add_buf( final, skill_list[lev] );
    }

    add_buf( final, "{x\n\r" );
    page_to_char( final->string, ch );
    free_buf( final );
}

void list_group_costs(CHAR_DATA *ch)
{
    char buf[100];
    int gn,sn,col;

    if (IS_NPC(ch))
	return;

    col = 0;

    send_to_char("\n\r{y----------------------------------{WGROUPS{y----------------------------------\n\r"
		 "{cgroup             {mcost    {cgroup             {mcost    {cgroup             {mcost\n\r", ch );

    for ( gn = 0; group_table[gn].name[0] != '\0'; gn++ )
    {
        if ( !ch->pcdata->gen_data->group_chosen[gn]
	&&   !ch->pcdata->group_known[gn]
	&&   get_group_rating(ch,gn) > 0 )
	{
	    sprintf(buf,"{c%-19s{m%3d    ",group_table[gn].name,
		get_group_rating(ch,gn));
	    send_to_char(buf,ch);

	    if (++col % 3 == 0)
		send_to_char("\n\r",ch);
	}
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);

    col = 0;

    send_to_char("{y----------------------------------{WSKILLS{y----------------------------------\n\r",ch);
    send_to_char("{wlvl {cskill         {mcost    {wlvl {cskill         {mcost    {wlvl {mskill         {mcost\n\r",ch);

    for (sn = 0; skill_table[sn].name[0] != '\0'; sn++)
    {
	if (!ch->pcdata->gen_data->skill_chosen[sn]
	&&  ch->learned[sn] == 0
	&&  skill_table[sn].spell_fun == spell_null
	&&  get_skill_rating(ch,sn) > 0
	&&  get_skill_level(ch,sn) < LEVEL_IMMORTAL )
        {
            sprintf(buf,"{w%3d {c%-15s {m%2d    ",
                get_skill_level(ch,sn), skill_table[sn].name,
		get_skill_rating(ch,sn));

            send_to_char(buf,ch);
	    if (++col % 3 == 0)
	        send_to_char( "\n\r", ch );
        }
    }
    if ( col % 3 != 0 )
        send_to_char("\n\r",ch);
    send_to_char("{y--------------------------------------------------------------------------{x\n\r\n\r",ch);

    sprintf(buf,"{CCreation points{w: %d\n\r",ch->pcdata->points);
    send_to_char(buf,ch);
    sprintf(buf,"{CExperience per level{w: {w%ld{x\n\r",
	    exp_per_level(ch,ch->pcdata->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}

void list_group_chosen(CHAR_DATA *ch)
{
    char buf[100];
    int gn,sn,col;

    if (IS_NPC(ch))
        return;

    col = 0;

    send_to_char("\n\r{y----------------------------------{WGROUPS{y----------------------------------\n\r"
		 "{cgroup             {mcost    {cgroup             {mcost    {cgroup             {mcost\n\r", ch );

    for ( gn = 0; group_table[gn].name[0] != '\0'; gn++ )
    {
        if ( ch->pcdata->gen_data->group_chosen[gn]
	&&   get_group_rating(ch,gn) > 0 )
        {
            sprintf(buf,"{c%-19s{m%3d    ",group_table[gn].name,
		get_group_rating(ch,gn));
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);

    col = 0;

    send_to_char("{y----------------------------------{WSKILLS{y----------------------------------\n\r",ch);
    send_to_char("{wlvl {cskill         {mcost    {wlvl {cskill         {mcost    {wlvl {cskill         {mcost\n\r",ch);

    for (sn = 0; skill_table[sn].name[0] != '\0'; sn++)
    {
        if (ch->pcdata->gen_data->skill_chosen[sn]
	&&  get_skill_rating(ch,sn) > 0)
        {
            sprintf(buf,"{w%3d {c%-15s {m%2d    ",
		get_skill_level(ch,sn), skill_table[sn].name,
		get_skill_rating(ch,sn));
            send_to_char(buf,ch);

            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
	send_to_char("\n\r",ch);
    send_to_char("{y--------------------------------------------------------------------------{x\n\r\n\r",ch);

    sprintf(buf,"{CCreation points{w: %d\n\r{x",ch->pcdata->gen_data->points_chosen);
    send_to_char(buf,ch);
    sprintf(buf,"{CExperience per level{w: %ld{x\n\r",
	    exp_per_level(ch,ch->pcdata->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}

long exp_per_level( CHAR_DATA *ch, int points )
{
    long expl = 1000, inc = 500;

    if ( IS_NPC(ch) )
	return 1000;

    if ( points < 40 )
	return 1000 * (race_table[ch->race].class_mult[ch->class]) / 100;

    /* processing */
    points -= 40;

    while ( points > 9 )
    {
	expl += inc;
        points -= 10;

        if ( points > 9 )
	{
	    expl += inc;
	    inc = 3 * inc / 2;
	    points -= 10;
	}
    }

    expl += points * inc / 10;

    return expl * (race_table[ch->race].class_mult[ch->class])/100;
}

bool parse_gen_groups( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int gn, sn, i;

    if ( argument[0] == '\0' )
	return FALSE;

    argument = one_argument(argument,arg);

    if ( !str_prefix( arg, "help" ) )
    {
	send_to_char( CLEAR_SCREEN, ch );
	if ( argument[0] == '\0' )
	{
	    do_help( ch, "group help" );
	    return TRUE;
	}

        do_help( ch, argument );
	return TRUE;
    }

    else if ( !str_prefix( arg, "add" ) )
    {
	if ( argument[0] == '\0' )
	{
	    send_to_char( "You must provide a skill name.\n\r", ch );
	    return TRUE;
	}

	if ( ( gn = group_lookup( argument ) ) != -1 )
	{
	    if ( ch->pcdata->gen_data->group_chosen[gn]
	    ||   ch->pcdata->group_known[gn] )
	    {
		send_to_char( "You already know that group!\n\r", ch );
		return TRUE;
	    }

	    if ( get_group_rating(ch,gn) < 1 )
	    {
	  	send_to_char("That group is not available.\n\r",ch);
	 	return TRUE;
	    }

	    sprintf( buf, "{c%s{w group added{x.\n\r", group_table[gn].name );
	    buf[2] = UPPER ( buf[2] );
	    send_to_char( buf, ch );
	    ch->pcdata->gen_data->group_chosen[gn] = TRUE;
	    ch->pcdata->gen_data->points_chosen += get_group_rating( ch, gn );
	    gn_add( ch, gn );
	    ch->pcdata->points += get_group_rating( ch, gn );
	    return TRUE;
	}

	else if ( (sn = skill_lookup( argument ) ) != -1 )
	{
	    if ( ch->pcdata->gen_data->skill_chosen[sn]
	    ||   ch->learned[sn] > 0 )
	    {
		send_to_char( "You already know that skill!\n\r", ch );
		return TRUE;
	    }

	    if ( get_skill_rating( ch, sn ) < 1
	    ||   get_skill_level( ch, sn ) > LEVEL_HERO
	    ||   skill_table[sn].spell_fun != spell_null )
	    {
		send_to_char( "That skill is not available.\n\r", ch );
		return TRUE;
	    }

	    sprintf( buf, "{c%s{w skill added.{x\n\r", skill_table[sn].name );
	    buf[2] = UPPER ( buf[2] );
	    send_to_char( buf, ch );
	    ch->pcdata->gen_data->skill_chosen[sn] = TRUE;
	    ch->pcdata->gen_data->points_chosen += get_skill_rating( ch, sn );
	    ch->learned[sn] = 1;
	    ch->pcdata->points += get_skill_rating( ch, sn );
	    return TRUE;
	}

	send_to_char( "No skills or groups by that name...\n\r", ch );
	return TRUE;
    }

    else if ( !str_cmp( arg, "drop" ) )
    {
	if ( argument[0] == '\0' )
  	{
	    send_to_char( "You must provide a skill to drop.\n\r", ch );
	    return TRUE;
	}

	else if ( ( gn = group_lookup( argument ) ) != -1
	     &&   ch->pcdata->gen_data->group_chosen[gn] )
	{
	    send_to_char( "{yGroup dropped.{x\n\r", ch );
	    ch->pcdata->gen_data->group_chosen[gn] = FALSE;
	    ch->pcdata->gen_data->points_chosen -= get_group_rating( ch, gn );
	    gn_remove( ch, gn );
	    for ( i = 0; group_table[i].name[0] != '\0'; i++ )
	    {
		if ( ch->pcdata->gen_data->group_chosen[gn] )
		    gn_add( ch, gn );
	    }
	    ch->pcdata->points -= get_group_rating( ch, gn );
	    return TRUE;
	}

	else if ( ( sn = skill_lookup( argument ) ) != -1
	     &&   ch->pcdata->gen_data->skill_chosen[sn] )
	{
	    send_to_char( "{ySkill dropped.{x\n\r", ch );
	    ch->pcdata->gen_data->skill_chosen[sn] = FALSE;
	    ch->pcdata->gen_data->points_chosen -= get_skill_rating( ch, sn );
	    ch->learned[sn] = 0;
	    ch->pcdata->points -= get_skill_rating( ch, sn );
	    return TRUE;
	}

	else
	{
	    send_to_char( "You haven't bought any such skill or group.\n\r", ch );
	    return TRUE;
	}
    }

    else if ( !str_prefix( arg, "premise" ) )
    {
	send_to_char( CLEAR_SCREEN, ch );
	do_help( ch, "premise" );
	return TRUE;
    }

    else if ( !str_prefix( arg, "list" ) )
    {
	send_to_char( CLEAR_SCREEN, ch );
	list_group_costs( ch );
	return TRUE;
    }

    else if ( !str_prefix( arg, "learned" ) )
    {
	send_to_char( CLEAR_SCREEN, ch );
	list_group_chosen( ch );
	return TRUE;
    }

    else if ( !str_prefix( arg, "info" ) )
    {
	send_to_char( CLEAR_SCREEN, ch );
	do_groups( ch, argument );
	return TRUE;
    }

    else if ( !str_prefix( arg, "class" ) )
    {
	send_to_char( CLEAR_SCREEN, ch );
	do_class( ch, argument );
	return TRUE;
    }

    else
	return FALSE;
}

void do_groups(CHAR_DATA *ch, char *argument)
{
    char buf[100];
    int gn,sn,col;

    if (IS_NPC(ch))
	return;

    col = 0;

    send_to_char("{WUse the class command (help class) for a listing\n\r",ch);
    send_to_char("more in tune with your current character.{x\n\r\n\r",ch);

    if (argument[0] == '\0')
    {   /* show all groups */

	for (gn = 0; group_table[gn].name[0] != '\0'; gn++)
        {
	    if (ch->pcdata->group_known[gn])
	    {
		sprintf(buf,"{c%-20s{x ",group_table[gn].name);
		send_to_char(buf,ch);
		if (++col % 3 == 0)
		    send_to_char("\n\r",ch);
	    }
        }
        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );
        sprintf(buf,"{CCreation points{w: %d{x\n\r",ch->pcdata->points);
	send_to_char(buf,ch);
	return;
     }

     if (!str_cmp(argument,"all"))    /* show all groups */
     {
        for (gn = 0; group_table[gn].name[0] != '\0'; gn++)
        {
	    sprintf(buf,"{c%-20s {x",group_table[gn].name);
            send_to_char(buf,ch);
	    if (++col % 3 == 0)
            	send_to_char("\n\r",ch);
        }
        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );
	return;
     }


     /* show the sub-members of a group */
     gn = group_lookup(argument);
     if (gn == -1)
     {
	send_to_char("No group of that name exist.\n\r",ch);
	send_to_char(
	    "Type 'groups all' or 'info all' for a full listing.\n\r",ch);
	return;
     }

     for (sn = 0; sn < MAX_IN_GROUP; sn++)
     {
	if (group_table[gn].spells[sn] == NULL)
	    break;
	sprintf(buf,"{c%-20s{x ",group_table[gn].spells[sn]);
	send_to_char(buf,ch);
	if (++col % 3 == 0)
	    send_to_char("\n\r",ch);
     }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
}

void do_class(CHAR_DATA *ch, char *argument)
{
    char buf[100];
    int gn,sn,tn,col;

    if (IS_NPC(ch))
	return;

    col = 0;

    if (argument[0] == '\0')
    {   /* show all groups */

	send_to_char( "{GGroups you currently have:\n\r", ch );
	send_to_char( "{y--------------------------\n\r", ch );
	for (gn = 0; group_table[gn].name[0] != '\0'; gn++)
        {
	    if (ch->pcdata->group_known[gn])
	    {
		sprintf(buf,"{c%-20s ",group_table[gn].name);
		send_to_char(buf,ch);
		if (++col % 3 == 0)
		    send_to_char("{x\n\r",ch);
	    }
        }
        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );
        sprintf(buf,"{CCreation points{w: %d{x\n\r",ch->pcdata->points);
	send_to_char(buf,ch);
	return;
     }

     if (!str_cmp(argument,"all"))    /* show all groups */
     {
	send_to_char( "{gGroups available to your class:\n\r", ch );
	send_to_char( "{y-------------------------------\n\r", ch );
        for (gn = 0; group_table[gn].name[0] != '\0'; gn++)
        {
	    if (get_group_rating(ch,gn) > 0)
	    {
		sprintf(buf,"{c%-20s ",group_table[gn].name);
		send_to_char(buf,ch);
		if (++col % 3 == 0)
		    send_to_char("{x\n\r",ch);
	    }
        }
        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );
	return;
     }


     /* show the sub-members of a group */
     gn = group_lookup(argument);
     if (gn == -1)
     {
	send_to_char("{GNo group of that name exist.\n\r",ch);
	send_to_char(
	    "Type {R'{yclass all{R' {Gfor a full listing.{x\n\r",ch);
	return;
     }

    send_to_char( "{GSpells available in this group:\n\r", ch );
    send_to_char( "{y-------------------------------\n\r", ch );
    send_to_char( "{wLevel{y-{GSpell{y--------------- {wLevel{y-{GSpell{y---------------\n\r", ch );
    for (sn = 0; sn < MAX_IN_GROUP; sn++)
    {
	if (group_table[gn].spells[sn] == NULL)
	    break;
	if ( ( tn = spell_avail( ch, group_table[gn].spells[sn] ) ) >= 0)
	{
	    sprintf(buf,"{w%-5d {m%-20s ",
		get_skill_level(ch,tn), group_table[gn].spells[sn]);
	    send_to_char(buf,ch);
	    if (++col % 2 == 0)
		send_to_char("{x\n\r",ch);
	}
    }
    if ( col % 2 != 0 )
	send_to_char( "{x\n\r", ch );
}

int spell_avail( CHAR_DATA *ch, const char *name )
{
    /* checks to see if a spell is available to a class */
    int sn, found = -1;

    if (IS_NPC(ch))
	return skill_lookup(name);

    for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
    {
	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&  !str_prefix(name,skill_table[sn].name))
	{
	    if (get_skill_level(ch,sn) <= LEVEL_HERO)
		return sn;
	}
    }
    return found;
}

void check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier )
{
    int chance, race;
    char buf[100];

    if ( IS_SET( skill_table[sn].flags, SKILL_DISABLED )
    ||   ch->learned[sn] == 0
    ||   ch->learned[sn] == 100
    ||   ch->level < get_skill_level( ch, sn )
    ||   get_skill_rating( ch, sn ) == 0
    ||   ch->in_room == NULL
    ||   IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(ch->in_room->room_flags, ROOM_WAR) )
	return;

    if ( number_range(0,100) < 3 ) {
    int bonus=number_range(0,100);
    char b[MSL];
    sprintf( b, "{^Super! {6Skill bonus: {#%d xp.{0\n\r", bonus );
    send_to_char( b, ch );
    gain_exp( ch, bonus, FALSE );
    }

    /* check to see if the character has a chance to learn */
    race = get_skill_rating( ch, sn );
    if ( race <= 0 )
	race = 1;
    chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
    chance /= (	multiplier * race * 4 );
    chance += ch->level;

    if ( number_range( 1, 1300 ) > chance )
	return;

    if (success)
    {
	chance = URANGE(5,100 - ch->learned[sn], 95);
	if (number_percent() < chance)
	{
	    ch->learned[sn]++;
	    sprintf(buf,"{CYou have become better at {W%s{C! (%s{W%%{C){x\n\r",
		skill_table[sn].name,
		color_percent( ch->learned[sn] ) );
	    send_to_char(buf,ch);
	    gain_exp(ch,2 * race, FALSE);
	}
    }

    else
    {
	chance = URANGE(5,ch->learned[sn]/2,30);
	if (number_percent() < chance)
	{
	    ch->learned[sn]++;
	    sprintf(buf,"{CYou learn from your mistakes, and your {W%s {Cskill improves. (%s{W%%{C){x\n\r",
		skill_table[sn].name,
		color_percent( ch->learned[sn] ) );
	    send_to_char(buf,ch);
	    gain_exp(ch,2 * race, FALSE);
	}
    }
}

int group_lookup( const char *name )
{
    int gn;

    for ( gn = 0; group_table[gn].name[0] != '\0'; gn++ )
    {
        if ( LOWER(name[0]) == LOWER(group_table[gn].name[0])
        &&   !str_prefix( name, group_table[gn].name ) )
            return gn;
    }

    return -1;
}

void gn_add( CHAR_DATA *ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = TRUE;
    for ( i = 0; i < MAX_IN_GROUP; i++)
    {
        if (group_table[gn].spells[i] == NULL)
            break;
        group_add(ch,group_table[gn].spells[i],FALSE);
    }
}

void gn_remove( CHAR_DATA *ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = FALSE;

    for ( i = 0; i < MAX_IN_GROUP; i ++)
    {
	if (group_table[gn].spells[i] == NULL)
	    break;
	group_remove(ch,group_table[gn].spells[i]);
    }
}

void group_add( CHAR_DATA *ch, const char *name, bool deduct)
{
    int sn,gn;

    if (IS_NPC(ch)) /* NPCs do not have skills */
	return;

    sn = skill_lookup(name);

    if (sn != -1)
    {
	if (ch->learned[sn] == 0) /* i.e. not known */
	{
	    ch->learned[sn] = 1;
	    if (deduct)
		ch->pcdata->points += get_skill_rating(ch,sn);
	}
	return;
    }

    gn = group_lookup(name);

    if (gn != -1)
    {
	if (ch->pcdata->group_known[gn] == FALSE)
	{
	    ch->pcdata->group_known[gn] = TRUE;
	    if (deduct)
		ch->pcdata->points += get_group_rating(ch,gn);
	}
	gn_add(ch,gn); /* make sure all skills in the group are known */
    }
}

void group_remove(CHAR_DATA *ch, const char *name)
{
    int sn, gn;

     sn = skill_lookup(name);

    if (sn != -1)
    {
	ch->learned[sn] = 0;
	return;
    }

    /* now check groups */

    gn = group_lookup(name);

    if (gn != -1 && ch->pcdata->group_known[gn] == TRUE)
    {
	ch->pcdata->group_known[gn] = FALSE;
	gn_remove(ch,gn);  /* be sure to call gn_add on all remaining groups */
    }
}

sh_int fun_lookup( char *spell )
{
    sh_int i;

    for ( i = 0; function_table[i].string != NULL; i++ )
    {
	if ( !str_cmp( function_table[i].string, spell ) )
	    return i;
    }

    sprintf( log_buf, "Fun_lookup: skill %s not located on function_table.", spell );
    bug( log_buf, 0 );
    return -1;
}

const struct function_type function_table[] =
{
    { "reserved",		spell_null,		NULL		     },
    { "absorb magic",		spell_absorb_magic,	NULL		     },
    { "acid blast",		spell_acid_blast,	&gsn_acid_blast	     },
    { "acid breath",		spell_acid_breath,	&gsn_acid_breath     },
    { "acid storm",		spell_acid_storm,	&gsn_acid_storm	     },
    { "acidshield",		spell_acidshield,	&gsn_acidshield	     },
    { "angelfire",		spell_angelfire,	&gsn_angelfire	     },
    { "animate",		spell_animate,		NULL		     },
    { "armor",			spell_armor,		&gsn_armor	     },
    { "awen",			spell_awen, 		NULL		     },
    { "backdraft",		spell_backdraft,	&gsn_backdraft	     },
    { "barkskin",		spell_barkskin,		&gsn_barkskin	     },
    { "bless",			spell_bless,		&gsn_bless	     },
    { "blindness",		spell_blindness,	&gsn_blindness	     },
    { "blink",			spell_blink,		NULL		     },
    { "blizzard",		spell_blizzard,		NULL		     },
    { "bloodbath",		spell_bloodbath,	&gsn_bloodbath	     },
    { "burning hands",		spell_burning_hands,	NULL		     },
    { "call lightning",		spell_call_lightning,	NULL		     },
    { "calm",			spell_calm,		NULL		     },
    { "cancellation",		spell_cancellation,	NULL		     },
    { "cause critical",		spell_cause_critical,	NULL		     },
    { "cause light",		spell_cause_light,	NULL		     },
    { "cause serious",		spell_cause_serious,	NULL		     },
    { "chain lightning",	spell_chain_lightning,	NULL		     },
    { "change sex",		spell_change_sex,	NULL		     },
    { "channel",		spell_channel,		NULL		     },
    { "charm person",		spell_charm_person,	&gsn_charm_person    },
    { "chill touch",		spell_chill_touch,	NULL		     },
    { "colour spray",		spell_colour_spray,	NULL		     },
    { "conjure",		spell_conjure,		NULL		     },
    { "constitution",		spell_constitution,	&gsn_constitution    },
    { "continual light",	spell_continual_light,	NULL		     },
    { "control weather",	spell_control_weather,	NULL		     },
    { "corrupt potion",		spell_corrupt_potion,   NULL		     },
    { "create food",		spell_create_food,	NULL		     },
    { "create raft",		spell_create_raft,	NULL		     },
    { "create rose",		spell_create_rose,	NULL		     },
    { "create spring",		spell_create_spring,	NULL		     },
    { "create water",		spell_create_water,	NULL		     },
    { "cure blindness",		spell_cure_blindness,	&gsn_cure_blind	     },
    { "cure critical",		spell_cure_critical,	&gsn_cure_critical   },
    { "cure disease",		spell_cure_disease,	&gsn_cure_disease    },
    { "cure light",		spell_cure_light,	&gsn_cure_light	     },
    { "cure poison",		spell_cure_poison,	&gsn_cure_poison     },
    { "cure serious",		spell_cure_serious,	&gsn_cure_serious    },
    { "cure weaken",		spell_cure_weaken,	NULL		     },
    { "curse",			spell_curse,		&gsn_curse	     },
    { "darkvision",		spell_darkvision,	&gsn_darkvision	     },
    { "decrement",		spell_decrement,	NULL		     },
    { "demonfire",		spell_demonfire,	&gsn_demonfire	     },
    { "detect evil",		spell_detect_evil,	&gsn_detect_evil     },
    { "detect good",		spell_detect_good,	&gsn_detect_good     },
    { "detect hidden",		spell_detect_hidden,	&gsn_detect_hidden   },
    { "detect invis",		spell_detect_invis,	&gsn_detect_invis    },
    { "detect magic",		spell_detect_magic,	&gsn_detect_magic    },
    { "detect neutral",		spell_detect_neutral,	NULL		     },
    { "detect poison",		spell_detect_poison,	NULL		     },
    { "detect terrain",		spell_detect_terrain,	NULL		     },
    { "deviant point",		spell_deviant_point,	NULL		     },
    { "devotion",		spell_devotion,		NULL		     },
    { "diminish",		spell_diminish,		NULL		     },
    { "dispel evil",		spell_dispel_evil,	NULL		     },
    { "dispel good",		spell_dispel_good,	NULL		     },
    { "dispel magic",		spell_dispel_magic,	&gsn_dispel_magic    },
    { "dispel neutral",		spell_dispel_neutral,	NULL		     },
    { "divine aura",		spell_divine_aura,	&gsn_divine_aura     },
    { "divine blessing",	spell_divine_blessing,	&gsn_divine_blessing },
    { "divine heal",		spell_divine_heal,	&gsn_divine_heal     },
    { "divine truth",		spell_divine_truth,	NULL		     },
    { "divinity",		spell_divinity,		&gsn_divinity	     },
    { "downpour",		spell_downpour,		&gsn_downpour	     },
    { "earthquake",		spell_earthquake,	NULL		     },
    { "electrical storm",	spell_electrical_storm,	&gsn_electrical_storm},
    { "electrify",		spell_electrify,	&gsn_electrify	     },
    { "embalm",			spell_embalm,		NULL		     },
    { "empower potion",		spell_empower_potion,	NULL		     },
    { "empower scroll",		spell_empower_scroll,	NULL		     },
    { "enchant armor",		spell_enchant_armor,	NULL		     },
    { "enchant weapon",		spell_enchant_weapon,	NULL		     },
    { "energy curse",		spell_energy_curse,	NULL		     },
    { "energy drain",		spell_energy_drain,	&gsn_energy_drain    },
    { "faerie fire",		spell_faerie_fire,	NULL		     },
    { "faerie fog",		spell_faerie_fog,	NULL		     },
    { "faith",			spell_faith,	        NULL     	     },
    { "farsight",		spell_farsight,		&gsn_farsight	     },
    { "feeble mind",		spell_feeble_mind,	NULL		     },
    { "fire breath",		spell_fire_breath,	&gsn_fire_breath     },
    { "fireball",		spell_fireball,		&gsn_fireball	     },
    { "fireproof",		spell_fireproof,	NULL		     },
    { "fireshield",		spell_fireshield,	&gsn_fireshield	     },
    { "fire storm",		spell_fire_storm,	&gsn_fire_storm	     },
    { "flamestrike",		spell_flamestrike,	&gsn_flamestrike     },
    { "flash",			spell_flash,		NULL		     },
    { "floating disc",		spell_floating_disc,	NULL		     },
    { "fly",			spell_fly,		NULL		     },
    { "frenzy",			spell_frenzy,		&gsn_frenzy	     },
    { "frost breath",		spell_frost_breath,	&gsn_frost_breath    },
    { "gas breath",		spell_gas_breath,	&gsn_gas_breath	     },
    { "gate",			spell_gate,		NULL		     },
    { "general purpose",	spell_general_purpose,	NULL		     },
    { "giant strength",		spell_giant_strength,	&gsn_giant_strength  },
    { "glacial aura",		spell_glacial_aura,	&gsn_glacial_aura    },
    { "glacier",		spell_glacier,		NULL		     },
    { "golden armor",		spell_golden_armor,	NULL		     },
    { "groupheal",		spell_groupheal,	NULL		     },
    { "growth",			spell_growth,		&gsn_growth	     },
    { "hail storm",		spell_hail_storm,	NULL		     },
    { "harm",			spell_harm,		NULL		     },
    { "haste",			spell_haste,		&gsn_haste	     },
    { "heal",			spell_heal,		&gsn_heal	     },
    { "heat metal",		spell_heat_metal,	NULL		     },
    { "heresy",			spell_heresy,		NULL		     },
    { "high explosive",		spell_high_explosive,	NULL		     },
    { "holy word",		spell_holy_word,	NULL		     },
    { "hurricane",		spell_hurricane,	NULL		     },
    { "iceshield",		spell_iceshield,	&gsn_iceshield	     },
    { "icicle",			spell_icicle,		NULL		     },
    { "identify",		spell_identify,		NULL		     },
    { "incinerate",		spell_incinerate,	NULL		     },
    { "infernal offering",	spell_infernal_offering,&gsn_infernal_offer  },
    { "infravision",		spell_infravision,	&gsn_infravision     },
    { "intellect",		spell_intellect,	&gsn_intellect	     },
    { "invisibility",		spell_invis,		&gsn_invis	     },
    { "iron skin",		spell_iron_skin,	NULL		     },
    { "kailfli",		spell_kailfli,		NULL		     },
    { "know alignment",		spell_know_alignment,	NULL		     },
    { "leech",			spell_leech,		&gsn_leech	     },
    { "life curse",		spell_life_curse,	NULL		     },
    { "lightning bolt",		spell_lightning_bolt,	&gsn_lightning_bolt  },
    { "lightning breath",	spell_lightning_breath,	&gsn_lightning_breath},
    { "locate object",		spell_locate_object,	NULL		     },
    { "locust swarm",		spell_locust_swarm,	&gsn_locust_swarm    },
    { "luminrati",		spell_luminrati,	NULL		     },
    { "magic missile",		spell_magic_missile,	&gsn_magic_missile   },
    { "mana shield",		spell_mana_shield,	&gsn_mana_shield     },
    { "mana tap",		spell_mana_tap,		&gsn_mana_tap	     },
    { "mass healing",		spell_mass_healing,	NULL		     },
    { "mass invis",		spell_mass_invis,	&gsn_mass_invis	     },
    { "memory lapse",		spell_memory_lapse,	NULL		     },
    { "meteor swarm",		spell_meteor_swarm,	NULL		     },
    { "mindblast",		spell_mindblast,	NULL		     },
    { "negate alignment",	spell_negate_alignment,	NULL		     },
    { "nexus",			spell_nexus,		NULL		     },
    { "nightmare",		spell_nightmare,	&gsn_nightmare	     },
    { "pass door",		spell_pass_door,	&gsn_pass_door	     },
    { "plague",			spell_plague,		&gsn_plague	     },
    { "poison",			spell_poison,		&gsn_poison	     },
    { "portal",			spell_portal,		NULL		     },
    { "power of gods",		spell_power_of_gods,	NULL		     },
    { "preserve",		spell_preserve,		NULL		     },
    { "project force",		spell_project_force,	NULL		     },
    { "protection evil",	spell_protection_evil,	&gsn_protect_evil    },
    { "protection good",	spell_protection_good,	&gsn_protect_good    },
    { "protection neutral",	spell_protection_neutral,&gsn_protect_neutral},
    { "protection voodoo",	spell_protection_voodoo,NULL		     },
    { "psi barrier",		spell_psi_barrier,	NULL		     },
    { "psi twister",		spell_psi_twister,	NULL		     },
    { "ray of deceit",		spell_ray_of_deceit,	NULL		     },
    { "ray of truth",		spell_ray_of_truth,	&gsn_ray_of_truth    },
    { "recharge",		spell_recharge,		NULL		     },
    { "refresh",		spell_refresh,		&gsn_refresh	     },
    { "regeneration",		spell_regeneration,	&gsn_regeneration    },
    { "remove curse",		spell_remove_curse,	&gsn_remove_curse    },
    { "restore health",		spell_restore_health,	NULL		     },
    { "restore magic",		spell_restore_magic,	NULL		     },
    { "restore mana",		spell_restore_mana,	NULL		     },
    { "restore movement",	spell_restore_movement,	NULL		     },
    { "resurrect",		spell_resurrect,	NULL		     },
    { "rockshield",		spell_rockshield,	&gsn_rockshield	     },
    { "sanctuary",		spell_sanctuary,	&gsn_sanctuary	     },
    { "shield",			spell_shield,		&gsn_shield	     },
    { "shocking grasp",		spell_shocking_grasp,	NULL		     },
    { "shockshield",		spell_shockshield,	&gsn_shockshield     },
    { "shrapnelshield",		spell_shrapnelshield,	&gsn_shrapnelshield  },
    { "shrink",			spell_shrink,		NULL		     },
    { "silence",		spell_silence,		NULL		     },
    { "sleep",			spell_sleep,		&gsn_sleep	     },
    { "slow",			spell_slow,		&gsn_slow	     },
    { "smite health",		spell_smite_health,	NULL		     },
    { "smite magic",		spell_smite_magic,	NULL		     },
    { "smite movement",		spell_smite_movement,	NULL		     },
    { "snow storm",		spell_snow_storm,	&gsn_snow_storm	     },
    { "soul blade",		spell_soul_blade,	NULL		     },
    { "stamina curse",		spell_stamina_curse,	NULL		     },
    { "steel skin",		spell_steel_skin,	&gsn_steel_skin	     },
    { "stone skin",		spell_stone_skin,	&gsn_stone_skin	     },
    { "summon",			spell_summon,		NULL		     },
    { "swarm",			spell_swarm,		&gsn_swarm	     },
    { "teleport",		spell_teleport,		NULL		     },
    { "thornshield",		spell_thornshield,	&gsn_thornshield     },
    { "thunderbolt",		spell_thunderbolt,	NULL		     },
    { "transfix",		spell_transfix,		NULL		     },
    { "transport",		spell_transport,	NULL		     },
    { "vampiricshield",		spell_vampiricshield,	&gsn_vampiricshield  },
    { "ventriloquate",		spell_ventriloquate,	NULL		     },
    { "vigorize",		spell_vigorize,		NULL		     },
    { "voodoo",			spell_voodoo,		NULL		     },
    { "watershield",		spell_watershield,	&gsn_watershield     },
    { "weaken",			spell_weaken,		&gsn_weaken	     },
    { "wisdom",			spell_wisdom,		&gsn_wisdom	     },
    { "wizard eye",		spell_wizard_eye,	NULL		     },
    { "word of recall",		spell_word_of_recall,	NULL		     },
    { "2nd dual",		spell_null,		&gsn_2nd_dual	     },
    { "3rd dual",		spell_null,		&gsn_3rd_dual	     },
    { "assassinate",		spell_null,		&gsn_assassinate     },
    { "ambush",			spell_null,		&gsn_ambush	     },
    { "axe",			spell_null,		&gsn_axe	     },
    { "axe mastery",		spell_null,		&gsn_axe_mastery     },
    { "backstab",		spell_null,		&gsn_backstab	     },
    { "bandage",		spell_null,		&gsn_bandage	     },
    { "bash",			spell_null,		&gsn_bash	     },
    { "battlehymn",		spell_null,		&gsn_battlehymn	     },
    { "berserk",		spell_null,		&gsn_berserk	     },
    { "brew",			spell_null,		&gsn_brew	     },
    { "camouflage",		spell_null,		&gsn_camouflage	     },
    { "cartwheel",		spell_null,		&gsn_cartwheel	     },
    { "charge",			spell_null,		&gsn_charge	     },
    { "circle",			spell_null,		&gsn_circle	     },
    { "counter",		spell_null,		&gsn_counter	     },
    { "critical strike",	spell_null,		&gsn_critical	     },
    { "critical damage",	spell_null,		&gsn_critdam	     },
    { "cross slash",		spell_null,		&gsn_cross_slash     },
    { "crush",			spell_null,		&gsn_crush	     },
    { "curse of ages",		spell_null,		&gsn_curse_of_ages   },
    { "cyclone",		spell_null,		&gsn_cyclone	     },
    { "dagger",			spell_null,		&gsn_dagger	     },
    { "deathblow",		spell_null,		&gsn_deathblow	     },
    { "devotion",		spell_null,		&gsn_devotion	     },
    { "dirt kicking",		spell_null,		&gsn_dirt	     },
    { "disarm",			spell_null,		&gsn_disarm	     },
    { "disguise",		spell_null,		&gsn_disguise	     },
    { "dismember",		spell_null,		&gsn_dismember	     },
    { "dislodge",		spell_null,		&gsn_dislodge	     },
    { "dodge",			spell_null,		&gsn_dodge	     },
    { "doorbash",		spell_null,		&gsn_doorbash	     },
    { "dual wield",		spell_null,		&gsn_dual_wield	     },
    { "eighth attack",		spell_null,		&gsn_eighth_attack   },
    { "engage",			spell_null,		&gsn_engage	     },
    { "engineer",		spell_null,		&gsn_engineer	     },
    { "enhanced damage",	spell_null,		&gsn_enhanced_damage },
    { "envenom",		spell_null,		&gsn_envenom	     },
    { "evade",			spell_null,		&gsn_evade	     },
    { "fast healing",		spell_null,		&gsn_fast_healing    },
    { "feed",			spell_null,		&gsn_feed	     },
    { "fifth attack",		spell_null,		&gsn_fifth_attack    },
    { "flail",			spell_null,		&gsn_flail	     },
    { "forest meld",		spell_null,		&gsn_forest_meld     },
    { "fourth attack",		spell_null,		&gsn_fourth_attack   },
    { "gash",			spell_null,		&gsn_gash	     },
    { "gouge",			spell_null,		&gsn_gouge	     },
    { "grip",			spell_null,		&gsn_grip	     },
    { "haggle",			spell_null,		&gsn_haggle	     },
    { "hand to hand",		spell_null,		&gsn_hand_to_hand    },
    { "heel",			spell_null,		NULL		     },
    { "hide",			spell_null,		&gsn_hide	     },
    { "hone",			spell_null,		&gsn_hone	     },
    { "hurl",			spell_null,	 	&gsn_hurl	     },
    { "kick",			spell_null,		&gsn_kick	     },
    { "knife fighter",		spell_null,		&gsn_knife_fighter   },
    { "legsweep",		spell_null,		&gsn_legsweep	     },
    { "lore",			spell_null,		&gsn_lore	     },
    { "lunge",			spell_null,		&gsn_lunge	     },
    { "mace",			spell_null,		&gsn_mace	     },
    { "master of magic",	spell_null,		&gsn_master_of_magic },
    { "meditation",		spell_null,		&gsn_meditation	     },
    { "obfuscate",		spell_null,		&gsn_obfuscate	     },
    { "onslaught",		spell_null,		&gsn_onslaught	     },
    { "overhand",		spell_null,		&gsn_overhand	     },
    { "parry",			spell_null,		&gsn_parry	     },
    { "peek",			spell_null,		&gsn_peek	     },
    { "phase",			spell_null,		&gsn_phase	     },
    { "pick lock",		spell_null,		&gsn_pick_lock	     },
    { "polearm",		spell_null,		&gsn_polearm	     },
    { "quarterstaff",		spell_null,		&gsn_quarterstaff    },
    { "quickdraw",		spell_null,		&gsn_quickdraw	     },
    { "raze",			spell_null,		&gsn_raze	     },
    { "recall",			spell_null,		&gsn_recall	     },
    { "repair",			spell_null,		&gsn_repair	     },
    { "rescue",			spell_null,		&gsn_rescue	     },
    { "retreat",		spell_null,		&gsn_retreat	     },
	{ "roundhouse",			spell_null,		&gsn_roundhouse	     },
    { "rub",			spell_null,		&gsn_rub	     },
    { "salve",			spell_null,		&gsn_salve	     },
    { "savage claws",		spell_null,		&gsn_savage_claws    },
    { "scalp",			spell_null,		&gsn_scalp	     },
    { "scribe",			spell_null,		&gsn_scribe	     },
    { "scrolls",		spell_null,		&gsn_scrolls	     },
    { "second attack",		spell_null,		&gsn_second_attack   },
    { "seventh attack",		spell_null,		&gsn_seventh_attack  },
    { "sharpen",		spell_null,		&gsn_sharpen	     },
    { "shield block",		spell_null,		&gsn_shield_block    },
    { "shield levitate",	spell_null,		&gsn_shield_levitate },
    { "shield smash",		spell_null,		&gsn_shield_smash    },
    { "sidestep",		spell_null,		&gsn_sidestep	     },
    { "sixth attack",		spell_null,		&gsn_sixth_attack    },
    { "slip",			spell_null,		&gsn_slip	     },
    { "sneak",			spell_null,		&gsn_sneak	     },
    { "spear",			spell_null,		&gsn_spear	     },
    { "spirit",			spell_null,		&gsn_spirit	     },
    { "stake",			spell_null,		&gsn_stake	     },
    { "stalk",			spell_null,		&gsn_stalk	     },
    { "staves",			spell_null,		&gsn_staves	     },
    { "strangle",		spell_null,		&gsn_strangle	     },
    { "steal",			spell_null,		&gsn_steal	     },
    { "storm of blades",	spell_null,		&gsn_storm_of_blades },
    { "stun",			spell_null,		&gsn_stun	     },
    { "sword",			spell_null,		&gsn_sword	     },
    { "tackle",			spell_null,		&gsn_tackle	     },
    { "third attack",		spell_null,		&gsn_third_attack    },
    { "track",			spell_null,		&gsn_track	     },
    { "trap disarm",		spell_null,		&gsn_trapdisarm	     },
    { "trap set",		spell_null,		&gsn_trapset	     },
    { "trip",			spell_null,		&gsn_trip	     },
    { "troll revival",		spell_null,		&gsn_troll_revival   },
    { "ultra damage",		spell_null,		&gsn_ultra_damage    },
    { "wands",			spell_null,		&gsn_wands	     },
    { "warcry",			spell_null,		&gsn_warcry	     },
    { "whip",			spell_null,		&gsn_whip	     },
    { NULL,			spell_null,		NULL		     }
};

char * pos_save( sh_int pos )
{
    if ( pos >= POS_DEAD && pos <= POS_STANDING )
	return position_flags[pos].name;

    return position_flags[POS_STANDING].name;
}

sh_int pos_load( char *pos )
{
    sh_int new_pos;

    for ( new_pos = POS_DEAD; new_pos <= POS_STANDING; new_pos++ )
    {
	if ( !str_prefix( pos, position_flags[new_pos].name ) )
	    return new_pos;
    }
    return -1;
}

const struct target_type target_table[] =
{
    { "IGNORE",		TAR_IGNORE 		},
    { "CHAR_OFFENSIVE",	TAR_CHAR_OFFENSIVE	},
    { "CHAR_DEFENSIVE",	TAR_CHAR_DEFENSIVE	},
    { "CHAR_SELF",	TAR_CHAR_SELF		},
    { "OBJ_INV",	TAR_OBJ_INV		},
    { "OBJ_CHAR_DEF",	TAR_OBJ_CHAR_DEF	},
    { "OBJ_CHAR_OFF",	TAR_OBJ_CHAR_OFF	},
    { "OBJ_TRAN",	TAR_OBJ_TRAN		}
};

char * target_save( sh_int pos )
{
    if ( pos >= TAR_IGNORE && pos <= TAR_OBJ_TRAN )
	return target_table[pos].name;

    return target_table[TAR_IGNORE].name;
}

sh_int target_load( char *pos )
{
    sh_int new_pos;

    for ( new_pos = TAR_IGNORE; new_pos <= TAR_OBJ_TRAN; new_pos++ )
    {
	if ( !str_prefix( pos, target_table[new_pos].name ) )
	    return new_pos;
    }

    return -1;
}

int sort_skill_table( const void *v1, const void *v2 )
{
    int idx1 = *(int *) v1;
    int idx2 = *(int *) v2;
    int i = 0;

    if ( !str_cmp( skill_table[idx1].name, "reserved" ) )
	return -1;

    if ( !str_cmp( skill_table[idx2].name, "reserved" ) )
	return 1;

    if ( skill_table[idx1].spell_fun == spell_null
    &&   skill_table[idx2].spell_fun != spell_null )
	return 1;

    if ( skill_table[idx1].spell_fun != spell_null
    &&   skill_table[idx2].spell_fun == spell_null )
	return -1;

    for ( i = 0; skill_table[idx1].name[i] != '\0'; i++ )
    {
	if ( skill_table[idx1].name[i] == skill_table[idx2].name[i] )
	    continue;

	if ( skill_table[idx1].name[i] > skill_table[idx2].name[i] )
	    return 1;

	if ( skill_table[idx1].name[i] < skill_table[idx2].name[i] )
	    return -1;
    }

    return 0;
}

int sort_group_table( const void *v1, const void *v2 )
{
    int idx1 = *(int *) v1;
    int idx2 = *(int *) v2;
    int i = 0;

    for ( i = 0; group_table[idx1].name[i] != '\0'; i++ )
    {
	if ( group_table[idx1].name[i] == group_table[idx2].name[i] )
	    continue;

	if ( group_table[idx1].name[i] > group_table[idx2].name[i] )
	    return 1;

	if ( group_table[idx1].name[i] < group_table[idx2].name[i] )
	    return -1;
    }

    return 0;
}

int sort_group_spells( const void *v1, const void *v2 )
{
    int idx1 = *(int *) v1;
    int idx2 = *(int *) v2;
    int i = 0;

    for ( i = 0; group_table[spell_num].spells[idx1][i] != '\0'; i++ )
    {
	if ( group_table[spell_num].spells[idx2] == NULL )
	    return 1;

	if ( group_table[spell_num].spells[idx1][i]
	==   group_table[spell_num].spells[idx2][i] )
	    continue;

	if ( group_table[spell_num].spells[idx1][i] >
	     group_table[spell_num].spells[idx2][i] )
	    return 1;

	if ( group_table[spell_num].spells[idx1][i] <
	     group_table[spell_num].spells[idx2][i] )
	    return -1;
    }

    return 0;
}

void save_skills( )
{
    FILE *fp;
    sh_int count, i, pos, sn;
    int skill_index[maxSkill];
    int group_index[maxGroup];

    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Save_skills: can not open file for writing.", 0 );
	return;
    }

    fprintf( fp, "#MAX_SKILL %d\n",	maxSkill			);
    fprintf( fp, "#MAX_GROUP %d\n\n",	maxGroup			);

    for ( pos = 0; skill_table[pos].name[0] != '\0'; pos++ )
	skill_index[pos] = pos;

    qsort( skill_index, pos, sizeof( int ), sort_skill_table );

    for ( count = 0; count < pos; count++ )
    {
	sn = skill_index[count];

	fprintf( fp, "#SKILL\n"						);
	fprintf( fp, "Name %s~\n",	skill_table[sn].name		);

	if ( skill_table[sn].target != TAR_IGNORE )
	    fprintf( fp, "Targ %s\n",	target_save( skill_table[sn].target ) );

	if ( skill_table[sn].minimum_position != POS_STANDING )
	    fprintf( fp, "Posi %s\n",	pos_save( skill_table[sn].minimum_position ) );

	if ( skill_table[sn].flags )
	    fprintf( fp, "Flag %s\n",	print_flags( skill_table[sn].flags ) );

	if ( skill_table[sn].cost_hp != 0 )
	    fprintf( fp, "Hitp %d\n",	skill_table[sn].cost_hp		);

	if ( skill_table[sn].cost_mana != 0 )
	    fprintf( fp, "Mana %d\n",	skill_table[sn].cost_mana	);

	if ( skill_table[sn].cost_move != 0 )
	    fprintf( fp, "Move %d\n",	skill_table[sn].cost_move	);

	if ( skill_table[sn].beats != 24 )
	    fprintf( fp, "Beat %d\n",	skill_table[sn].beats		);

	if ( skill_table[sn].room_msg )
	    fprintf( fp, "Room %s~\n",	skill_table[sn].room_msg	);

	if ( skill_table[sn].sound_cast )
	    fprintf( fp, "SndC %s~\n",	skill_table[sn].sound_cast	);

	if ( skill_table[sn].sound_off )
	    fprintf( fp, "SndO %s~\n",	skill_table[sn].sound_off	);

	if ( skill_table[sn].cost_potion )
	    fprintf( fp, "CPot %d %d %d %d %d\n",
		skill_table[sn].cost_potion->cubic,
		skill_table[sn].cost_potion->aquest,
		skill_table[sn].cost_potion->iquest,
		skill_table[sn].cost_potion->level,
		skill_table[sn].cost_potion->max );

	if ( skill_table[sn].cost_scroll )
	    fprintf( fp, "CScr %d %d %d %d %d\n",
		skill_table[sn].cost_scroll->cubic,
		skill_table[sn].cost_scroll->aquest,
		skill_table[sn].cost_scroll->iquest,
		skill_table[sn].cost_scroll->level,
		skill_table[sn].cost_scroll->max );

	if ( skill_table[sn].cost_pill )
	    fprintf( fp, "CPil %d %d %d %d %d\n",
		skill_table[sn].cost_pill->cubic,
		skill_table[sn].cost_pill->aquest,
		skill_table[sn].cost_pill->iquest,
		skill_table[sn].cost_pill->level,
		skill_table[sn].cost_pill->max );

	if ( skill_table[sn].cost_wand )
	    fprintf( fp, "CWnd %d %d %d %d %d\n",
		skill_table[sn].cost_wand->cubic,
		skill_table[sn].cost_wand->aquest,
		skill_table[sn].cost_wand->iquest,
		skill_table[sn].cost_wand->level,
		skill_table[sn].cost_wand->max );

	if ( skill_table[sn].cost_staff )
	    fprintf( fp, "CStf %d %d %d %d %d\n",
		skill_table[sn].cost_staff->cubic,
		skill_table[sn].cost_staff->aquest,
		skill_table[sn].cost_staff->iquest,
		skill_table[sn].cost_staff->level,
		skill_table[sn].cost_staff->max );

	fprintf( fp, "Dama %s~\n",	skill_table[sn].noun_damage	);
	fprintf( fp, "Msge %s~\n",	skill_table[sn].msg_off		);
	fprintf( fp, "ObjM %s~\n",	skill_table[sn].msg_obj		);
	fprintf( fp, "End\n\n"						);
    }

    for ( pos = 0; group_table[pos].name[0] != '\0'; pos++ )
	group_index[pos] = pos;

    qsort( group_index, pos, sizeof( int ), sort_group_table );

    for ( count = 0; count < pos; count++ )
    {
	int cnt, k, idx[MAX_IN_GROUP];

	sn = group_index[count];

	fprintf( fp, "#GROUP\n"						);
	fprintf( fp, "Name %s~\n",	group_table[sn].name		);

	spell_num = sn;

	for ( k = 0; k <= MAX_IN_GROUP && group_table[sn].spells[k]; k++ )
	    idx[k] = k;

	qsort( idx, k, sizeof( int ), sort_group_spells );

	for ( cnt = 0; cnt < k; cnt++ )
	{
	    i = idx[cnt];

	    if ( group_table[sn].spells[i] == NULL )
		break;

	    fprintf( fp, "Skil %s~\n",	group_table[sn].spells[i]	);
	}

	fprintf( fp, "End\n\n"						);
    }

    fprintf( fp, "#END\n"						);
    fclose( fp );
    rename( TEMP_FILE, SKILLS_FILE );
    mud_stat.skills_changed = FALSE;
    return;
}

void load_skills( )
{
    FILE *fp;
    sh_int gn = -1, sn = -1, lvl;

    if ( ( fp = fopen( SKILLS_FILE, "r" ) ) == NULL )
    {
	bug( "Load_skills: can not access file to read.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	char *word = fread_word( fp );

	if ( !str_cmp( word, "#SKILL" ) )
	{
	    if ( maxSkill == 0 || ++sn > maxSkill )
	    {
		bug( "Load_skills: MAX_SKILL exceeded.", 0 );
		fclose( fp );
		exit( 1 );
	    }

	    skill_table[sn].name		= NULL;
	    skill_table[sn].pgsn		= NULL;
	    skill_table[sn].room_msg		= NULL;
	    skill_table[sn].sound_cast		= NULL;
	    skill_table[sn].sound_off		= NULL;
	    skill_table[sn].skill_level		= NULL;
	    skill_table[sn].rating		= NULL;
	    skill_table[sn].cost_scroll		= NULL;
	    skill_table[sn].cost_pill		= NULL;
	    skill_table[sn].cost_potion		= NULL;
	    skill_table[sn].cost_wand		= NULL;
	    skill_table[sn].cost_staff		= NULL;
	    skill_table[sn].target		= TAR_IGNORE;
	    skill_table[sn].minimum_position	= POS_STANDING;
	    skill_table[sn].spell_fun		= spell_null;
	    skill_table[sn].cost_hp		= 0;
	    skill_table[sn].cost_mana		= 0;
	    skill_table[sn].cost_move		= 0;
	    skill_table[sn].beats		= 24;
	    skill_table[sn].flags		= 0;
	    skill_table[sn].msg_off		= "";
	    skill_table[sn].msg_obj		= "";
	    skill_table[sn].noun_damage		= "";

	    for ( ; ; )
	    {
		char *match = fread_word( fp );

		if ( !str_cmp( match, "End" ) )
		{
		    if ( ( lvl = fun_lookup( skill_table[sn].name ) ) != -1 )
		    {
			skill_table[sn].spell_fun = function_table[lvl].spell;
			skill_table[sn].pgsn	  = function_table[lvl].gsn;
		    }

		    if ( skill_table[sn].target == -1 )
			skill_table[sn].target = TAR_IGNORE;

		    if ( skill_table[sn].minimum_position == -1 )
			skill_table[sn].minimum_position = POS_STANDING;

		    break;
		}

		switch( UPPER( match[0] ) )
		{
		    case 'B':
			LOAD( "Beat", skill_table[sn].beats, fread_number( fp ) );
			break;

		    case 'C':
			if ( !str_cmp( match, "CPot" ) )
			{
			    skill_table[sn].cost_potion		= alloc_mem( sizeof( COST_DATA ) );
			    skill_table[sn].cost_potion->cubic	= fread_number( fp );
			    skill_table[sn].cost_potion->aquest	= fread_number( fp );
			    skill_table[sn].cost_potion->iquest	= fread_number( fp );
			    skill_table[sn].cost_potion->level	= fread_number( fp );
			    skill_table[sn].cost_potion->max	= fread_number( fp );
			    break;
			}

			if ( !str_cmp( match, "CScr" ) )
			{
			    skill_table[sn].cost_scroll		= alloc_mem( sizeof( COST_DATA ) );
			    skill_table[sn].cost_scroll->cubic	= fread_number( fp );
			    skill_table[sn].cost_scroll->aquest	= fread_number( fp );
			    skill_table[sn].cost_scroll->iquest	= fread_number( fp );
			    skill_table[sn].cost_scroll->level	= fread_number( fp );
			    skill_table[sn].cost_scroll->max	= fread_number( fp );
			    break;
			}

			if ( !str_cmp( match, "CPil" ) )
			{
			    skill_table[sn].cost_pill		= alloc_mem( sizeof( COST_DATA ) );
			    skill_table[sn].cost_pill->cubic	= fread_number( fp );
			    skill_table[sn].cost_pill->aquest	= fread_number( fp );
			    skill_table[sn].cost_pill->iquest	= fread_number( fp );
			    skill_table[sn].cost_pill->level	= fread_number( fp );
			    skill_table[sn].cost_pill->max	= fread_number( fp );
			    break;
			}

			if ( !str_cmp( match, "CWnd" ) )
			{
			    skill_table[sn].cost_wand		= alloc_mem( sizeof( COST_DATA ) );
			    skill_table[sn].cost_wand->cubic	= fread_number( fp );
			    skill_table[sn].cost_wand->aquest	= fread_number( fp );
			    skill_table[sn].cost_wand->iquest	= fread_number( fp );
			    skill_table[sn].cost_wand->level	= fread_number( fp );
			    skill_table[sn].cost_wand->max	= fread_number( fp );
			    break;
			}

			if ( !str_cmp( match, "CStf" ) )
			{
			    skill_table[sn].cost_staff		= alloc_mem( sizeof( COST_DATA ) );
			    skill_table[sn].cost_staff->cubic	= fread_number( fp );
			    skill_table[sn].cost_staff->aquest	= fread_number( fp );
			    skill_table[sn].cost_staff->iquest	= fread_number( fp );
			    skill_table[sn].cost_staff->level	= fread_number( fp );
			    skill_table[sn].cost_staff->max	= fread_number( fp );
			    break;
			}

			break;

		    case 'D':
			LOAD( "Dama", skill_table[sn].noun_damage, fread_string( fp ) );
			break;

		    case 'F':
			LOAD( "Flag", skill_table[sn].flags, fread_flag( fp ) );
			break;

		    case 'H':
			LOAD( "Hitp", skill_table[sn].cost_hp, fread_number( fp ) );
			break;

		    case 'M':
			LOAD( "Mana", skill_table[sn].cost_mana, fread_number( fp ) );
			LOAD( "Move", skill_table[sn].cost_move, fread_number( fp ) );
			LOAD( "Msge", skill_table[sn].msg_off, fread_string( fp ) );
			break;

		    case 'N':
			LOAD( "Name", skill_table[sn].name, fread_string( fp ) );
			break;

		    case 'O':
			LOAD( "ObjM", skill_table[sn].msg_obj, fread_string( fp ) );
			break;

		    case 'P':
			LOAD( "Posi", skill_table[sn].minimum_position, pos_load( fread_word( fp ) ) );
			break;

		    case 'R':
			LOAD( "Room", skill_table[sn].room_msg,	fread_string( fp ) );
			break;

		    case 'S':
			LOAD( "SndC", skill_table[sn].sound_cast, fread_string( fp ) );
			LOAD( "SndO", skill_table[sn].sound_off, fread_string( fp ) );
			break;

		    case 'T':
			LOAD( "Targ", skill_table[sn].target, target_load( fread_word( fp ) ) );
			break;

		}
	    }
	}

	else if ( !str_cmp( word, "#GROUP" ) )
	{
	    sh_int sp = -1;

	    if ( maxGroup == 0 || ++gn > maxGroup )
	    {
		bug( "Load_skills: MAX_GROUP exceeded.", 0 );
		fclose( fp );
		exit( 1 );
	    }

	    group_table[gn].name = NULL;

	    for ( lvl = 0; lvl < MAX_IN_GROUP; lvl++ )
		group_table[gn].spells[lvl] = NULL;

	    for ( ; ; )
	    {
		char *match = fread_word( fp );

		if ( !str_cmp( match, "End" ) )
		    break;

		switch( UPPER( match[0] ) )
		{
		    case 'N':
			LOAD( "Name", group_table[gn].name, fread_string( fp ) );
			break;

		    case 'S':
			if ( !str_cmp( match, "Skil" ) )
			{
			    char *tmp;

			    if ( ++sp > MAX_IN_GROUP )
			    {
				bug( "Load_skills: group initialize exceeds MAX_IN_GROUP.", 0 );
				tmp = fread_string( fp );
				free_string( tmp );
				break;
			    }

			    group_table[gn].spells[sp] = fread_string( fp );
			    break;
			}
			break;
		}
	    }
	}

	else if ( !str_cmp( word, "#MAX_SKILL" ) )
	{
	    maxSkill = fread_number( fp );
	    skill_table = malloc( sizeof( struct skill_type ) * ( maxSkill + 1 ) );
	}

	else if ( !str_cmp( word, "#MAX_GROUP" ) )
	{
	    maxGroup = fread_number( fp );
	    group_table = malloc( sizeof( struct group_type ) * ( maxGroup + 1 ) );
	}

	else if ( !str_cmp( word, "#END" ) )
	    break;

	else
	{
	    bug( "Load_skills: bad section name.", 0 );
	    fclose( fp );
	    exit( 1 );
	}
    }

    group_table[maxGroup].name = str_dup( "" );
    skill_table[maxSkill].name = str_dup( "" );

    fclose( fp );

    return;
}

void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    if ( argument[0] == '\0' )
    {
	BUFFER *final = new_buf();
	int col = 0;

	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( ch->level < get_skill_level( ch, sn )
	    ||   ch->learned[sn] < 1 )
		continue;

	    sprintf( buf, "{c%-18s %5s{w%%  ",
		skill_table[sn].name,
		color_percent( ch->learned[sn] ) );
	    add_buf( final, buf );

	    if ( ++col % 3 == 0 )
		add_buf( final, "\n\r" );
	}

	if ( col % 3 != 0 )
	    add_buf( final, "\n\r" );

	sprintf( buf, "{WYou have {c%d {Wpractice sessions left.{x\n\r",
	    ch->pcdata ? ch->pcdata->practice : 0 );
	add_buf( final, buf );

	page_to_char( final->string, ch );
	free_buf( final );
	return;
    } else {
	CHAR_DATA *mob;
	int adept = 75;

	if ( IS_NPC( ch ) )
	    return;

	if ( !IS_AWAKE(ch) )
	{
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    return;
	}

	for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		break;
	}

	if ( mob == NULL )
	{
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	if ( ch->pcdata->practice <= 0 )
	{
	    send_to_char( "{CYou have no practice sessions left.{x\n\r", ch );
	    return;
	}

	sn = find_spell( ch,argument );

	if ( sn < 0
	||   (!IS_NPC(ch)
	&&    (ch->level < get_skill_level(ch,sn)
	||     ch->learned[sn] < 1
	||     get_skill_rating(ch,sn) == 0)
             )
         )
	{
	    send_to_char( "{CYou can't practice that.{x\n\r", ch );
	    return;
	}

	if ( ch->learned[sn] >= adept )
	{
	    sprintf( buf, "{CYou are already a master of {W%s{C.{x\n\r",
		skill_table[sn].name );
	    send_to_char( buf, ch );
	} else {
	    ch->pcdata->practice--;
	    ch->learned[sn] +=
		int_app[get_curr_stat(ch,STAT_INT)].learn / get_skill_rating(ch,sn);

	    if ( ch->learned[sn] < adept )
	    {
		sprintf(buf,"{CYou practice {W%s{C < %s {C>.{x\n\r",
		    skill_table[sn].name,
		    color_percent( ch->learned[sn] ) );
		send_to_char(buf,ch);

		act( "{C$n practices {W$T{C.{x",
		    ch, NULL, skill_table[sn].name, TO_ROOM,POS_RESTING);
	    } else {
		ch->learned[sn] = adept;
		sprintf(buf,"{CYou have now mastered {W%s {C< %s {C>.{x\n\r",
		    skill_table[sn].name,
		    color_percent( ch->learned[sn] ) );
		send_to_char(buf,ch);
		act( "{C$n has mastered {W$T{C.{x",
		    ch, NULL, skill_table[sn].name, TO_ROOM,POS_RESTING);
	    }
	}
    }
    return;
}

bool cost_of_skill( CHAR_DATA *ch, int sn )
{
    int cost_hp = skill_table[sn].cost_hp;
    int cost_mana = skill_table[sn].cost_mana;
    int cost_move = skill_table[sn].cost_move;
    int chance;

    if ( IS_NPC( ch ) )
    {
	if ( cost_hp > 0 )
	    cost_hp /= 2;

	if ( cost_mana > 0 )
	    cost_mana /= 2;

	if ( cost_move > 0 )
	    cost_move /= 2;
    }

    if ( ch->hit < cost_hp )
    {
	printf_to_char( ch, "You do not feel rejuvenated enough for %s.\n\r", skill_table[sn].name );
	return FALSE;
    }

    if ( ch->move < cost_move )
    {
	printf_to_char( ch, "You do not feel rested enough for %s.\n\r", skill_table[sn].name );
	return FALSE;
    }

    if ( cost_mana > 0
    &&   ( chance = get_skill( ch, gsn_master_of_magic ) ) != 0 )
    {
	if ( number_percent( ) < 9 * chance / 10 )
	{
	    check_improve( ch, gsn_master_of_magic, TRUE, 5 );
	    cost_mana /= 2;
	}
	else
	    check_improve( ch, gsn_master_of_magic, FALSE, 5 );
    }

    if ( ch->mana < cost_mana )
    {
	printf_to_char( ch, "You do not feel energized enough for %s.\n\r", skill_table[sn].name );
	return FALSE;
    }

    ch->hit -= cost_hp;
    ch->mana -= cost_mana;
    ch->move -= cost_move;

    WAIT_STATE( ch, skill_table[sn].beats );

    return TRUE;
}



/*
 * Check for a skill.
 * Send player skill list entry as pSkill; or use the Teacher's or
 * call with find_skill()
 */
#if defined(NEVER)
bool skill_check( CHAR_DATA *ch, SKILL *pSkill, int modifier )
{
    int level,l,roll=number_percent( );
    float ratio=0;
    bool success=FALSE;

    if ( !pSkill ) return TRUE;

    /* Spawn last used time */
    if ( !NPC(ch) )
        pSkill->last_used = 0;

    if ( NPC(ch) && learned(ch,pSkill->dbkey) == 0 ) return number_percent() > 40;

    ratio = 1.0 + ( (float) modifier ) / 100.0;

    level = (l=learned(ch,pSkill->dbkey)) * ratio;

    roll = roll + (roll/2);

    success = URANGE(0,level,130) > roll;

    { char buf[MAX_STRING_LENGTH];
    sprintf( buf, "Skill> %s using %s %d+%d%% yielded %d%% r:%1.3f against %d %s",
              NAME(ch), pSkill->name, l, modifier, level, ratio, roll, success ? "success" : "fail" );
    NOTIFY( buf, LEVEL_IMMORTAL, WIZ_NOTIFY_DAMAGE );
    }

    if( success )
    {
        /* Improvement by use chance 0% ... 4% */
    if ( ((4 - int_app[URANGE(0,get_curr_int(ch),25)].learn)/4)+1 >=number_range(1,100))
        advance_skill( ch, pSkill, number_range(1,pSkill->max_prac),0);
        return TRUE;
    }
    else
    {
        /* Learn from mistakes 0% ... 3% */
    if ( ((2 - int_app[URANGE(0,get_curr_int(ch),25)].learn)/4)+1 >= number_range(1,100) )
        advance_skill( ch, pSkill, number_range(1,pSkill->max_prac),0);
        return FALSE;
    }
}
#endif


