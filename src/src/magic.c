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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"

DECLARE_DO_FUN( do_flee		);
DECLARE_DO_FUN( do_look		);

bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, bool show ) );
BUFFER *show_char_list	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
BUFFER *show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
					bool fShort, bool fShowNothing ) );
sh_int	get_skill_level	args( ( CHAR_DATA *ch, sh_int sn ) );

char *global_spell_arg;

sh_int skill_lookup( const char *name )
{
    sh_int sn;

    for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
    {
	if ( name[0] == skill_table[sn].name[0]
	&&   !str_prefix( name, skill_table[sn].name ) )
	    return sn;
    }

    return -1;
}

int check_curse_of_ages( CHAR_DATA *ch, int level )
{
    int chance;

    if ( ( chance = get_skill( ch, gsn_curse_of_ages ) ) == 0 )
	return level;

    if ( number_percent( ) > 3 * chance / 4 )
    {
	check_improve( ch, gsn_curse_of_ages, FALSE, 4 );
	return level;
    }

    check_improve( ch, gsn_curse_of_ages, TRUE, 3 );
    return 11 * level / 10;
}

sh_int find_spell( CHAR_DATA *ch, const char *name )
{
    int sn, found = -1;

    if (IS_NPC(ch))
	return skill_lookup(name);

    for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
    {
	if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !str_prefix(name,skill_table[sn].name))
	{
	    if ( found == -1)
		found = sn;
	
	    if ( ch->level >= get_skill_level( ch, sn )
	    &&   ch->learned[sn] > 0 )
		return sn;
	}
    }
    return found;
}

void say_spell( CHAR_DATA *ch, int sn )
{
    char buf  [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *	old;
	char *	new;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "		},
	{ "ar",		"abra"		},
	{ "au",		"kada"		},
	{ "bless",	"fido"		},
	{ "blind",	"nose"		},
	{ "bur",	"mosa"		},
	{ "cu",		"judi"		},
	{ "de",		"oculo"		},
	{ "en",		"unso"		},
	{ "light",	"dies"		},
	{ "lo",		"hi"		},
	{ "mor",	"zak"		},
	{ "move",	"sido"		},
	{ "ness",	"lacri"		},
	{ "ning",	"illa"		},
	{ "per",	"duda"		},
	{ "ra",		"gru"		},
	{ "fresh",	"ima"		},
	{ "re",		"candus"	},
	{ "son",	"sabru"		},
	{ "tect",	"infra"		},
	{ "tri",	"cula"		},
	{ "ven",	"nofo"		},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]	= '\0';
    for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
    {
	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
	{
	    if ( !str_prefix( syl_table[iSyl].old, pName ) )
	    {
		strcat( buf, syl_table[iSyl].new );
		break;
	    }
	}

	if ( length == 0 )
	    length = 1;
    }

    sprintf( buf2, "$n utters the words, '{c%s{x'.", buf );
    sprintf( buf,  "$n utters the words, '{c%s{x'.", skill_table[sn].name );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( rch != ch )
	    act( ch->class == rch->class ? buf : buf2, ch, NULL, rch, TO_VICT,POS_RESTING);
    }

    return;
}

bool saves_spell( int level, CHAR_DATA *ch, CHAR_DATA *victim, int dam_type )
{
    int save;

    if ( victim->damage_mod[dam_type] <= 0 )
	return TRUE;

//    save = ( victim->level - level ) * 2 - ( 3 * victim->saving_throw / 2 );
    save = ( victim->magic_power - level ) * 2 - ( 3 * victim->saving_throw / 2 );

    if ( victim->damage_mod[dam_type] < 20 )
	save += 15;
    else if ( victim->damage_mod[dam_type] < 50 )
	save += 10;
    else if ( victim->damage_mod[dam_type] < 75 )
	save += 5;
    else if ( victim->damage_mod[dam_type] > 180 )
	save -= 15;
    else if ( victim->damage_mod[dam_type] > 150 )
	save -= 10;
    else if ( victim->damage_mod[dam_type] > 125 )
	save -= 5;

    if ( dam_type != DAM_MAGIC )
    {
	if ( victim->damage_mod[DAM_MAGIC] < 20 )
	    save += 15;
	else if ( victim->damage_mod[DAM_MAGIC] < 50 )
	    save += 10;
	else if ( victim->damage_mod[DAM_MAGIC] < 75 )
	    save += 5;
	else if ( victim->damage_mod[DAM_MAGIC] > 180 )
	    save -= 15;
	else if ( victim->damage_mod[DAM_MAGIC] > 150 )
	    save -= 10;
	else if ( victim->damage_mod[DAM_MAGIC] > 125 )
	    save -= 5;
    }

    save = URANGE( 5, save, 95 );

    return number_percent( ) < save;
}

bool saves_dispel( int dis_level, int spell_level, int duration )
{
    int save;

    if (duration == -1)
      spell_level += 5;
      /* very hard to dispel permanent effects */

    save = 50 + (spell_level - dis_level) * 5;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

bool check_dispel( int dis_level, CHAR_DATA *victim, int sn )
{
    AFFECT_DATA *af;

    if (is_affected(victim, sn))
    {
        for ( af = victim->affected; af != NULL; af = af->next )
        {
            if ( af->type == sn )
            {
                if (!saves_dispel(dis_level,af->level,af->duration))
                {
                    affect_strip(victim,sn);
        	    if ( skill_table[sn].msg_off )
        	    {
            		send_to_char( skill_table[sn].msg_off, victim );
            		send_to_char( "\n\r", victim );
        	    }
		    if ( skill_table[sn].room_msg )
		    {
			act(skill_table[sn].room_msg,
			    victim,NULL,NULL,TO_ROOM,POS_RESTING);
		    }
		    if ( skill_table[sn].sound_off != NULL )
		    {
			send_sound_room( victim->in_room, 75, 1, 95, "skills",
			    skill_table[sn].sound_off, SOUND_NOSKILL );
		    }
		    return TRUE;
		}
		else
		    af->level--;
            }
        }
    }
    return FALSE;
}

int mana_cost (CHAR_DATA *ch, int min_mana, int level)
{
    if (ch->level + 2 == level)
	return 1000;
    return UMAX(min_mana,(100/(2 + ch->level - level)));
}

void brew_scribe( CHAR_DATA *ch, char *argument, int type )
{
    OBJ_DATA *obj, *comp[3];
    char arg[3][MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char nam[MAX_STRING_LENGTH];
    char *number[3];
    int skill, mana, sn[3], i;

    if ( IS_NPC(ch) && ch->desc == NULL )
	return;

    number[0] = "first";
    number[1] = "second";
    number[2] = "third";

    for ( i = 0; i < 3; i++ )
    {
	sn[i]	 = -1;
	comp[i]	 = NULL;
	argument = one_argument( argument, arg[i] );
    }

    if ( type == 2 )
    {
	if ( (skill = get_skill(ch,gsn_scribe)) <= 0 )
	{
	    send_to_char("You know absolutely nothing about scribing.\n\r",ch);
	    return;
	}
    }

    else if ( type ==  1 )
    {
	if ( (skill = get_skill(ch,gsn_brew)) <= 0 )
	{
	    send_to_char("You know absolutely nothing about brewing.\n\r",ch);
	    return;
	}

    }

    else
    {
	send_to_char("Void scribe_brew called with invalid type.\n\r", ch);
	return;
    }

    if ( arg[0][0] == '\0' )
    {
	send_to_char( "You must specify at least one spell to bind.\n\r", ch );
	return;
    }

    for ( i = 0; i < 3; i++ )
    {
	if ( arg[i][0] == '\0' )
	    break;

	if ( (sn[i] = find_spell(ch,arg[i])) < 0
	||   (skill_table[sn[i]].spell_fun == spell_null) )
	{
	    sprintf(buf,"Which spell do you wish to bind as your %s spell?\n\r",number[i]);
	    send_to_char(buf,ch);
	    return;
	}

	if ( ( type == 1 && IS_SET( skill_table[sn[i]].flags, SKILL_NO_BREW ) )
	||   ( type == 2 && IS_SET( skill_table[sn[i]].flags, SKILL_NO_SCRIBE ) ) )
	{
	    sprintf(buf,"%s may not be bound.\n\r",
		capitalize( skill_table[sn[i]].name ) );
	    send_to_char(buf,ch);
	    return;
	}
    }

    for ( i = 0; i < 3; i++ )
    {
	if ( sn[i] == -1 )
	    break;

	for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	{
	    if ( obj->wear_loc == WEAR_NONE
	    &&   can_see_obj(ch,obj)
	    &&   obj->value[0] > i
	    &&   ( i == 0
	    ||    (i == 1 && obj != comp[0])
	    ||    (i == 2 && obj != comp[0] && obj != comp[1])) )
	    {
		if ((type == 1 && obj->item_type == ITEM_COMPONENT_BREW)
		||  (type == 2 && obj->item_type == ITEM_COMPONENT_SCRIBE))
		{
		    comp[i] = obj;
		    break;
		}
	    }
	}

	if ( comp[i] == NULL )
	{
	    sprintf(buf,"You don't have the necessary component for binding the %s spell.\n\r",
		number[i]);
	    send_to_char(buf,ch);
	    return;
	}
    }

    mana = (skill_table[sn[0]].cost_mana * 3)
	 + (sn[1] == -1 ? 0 : skill_table[sn[1]].cost_mana * 5)
	 + (sn[2] == -1 ? 0 : skill_table[sn[2]].cost_mana * 7);

    if ( ch->mana < mana )
    {
	send_to_char("You do not have enough mana.\n\r",ch);
	return;
    }

    ch->mana -= mana;

    for ( i = 0; i < 3; i++ )
    {
	if ( comp[i] == NULL )
	    break;

	sprintf(buf,"You draw upon the power of %s!\n\r",comp[i]->short_descr);
	send_to_char(buf,ch);
	obj_from_char( comp[i] );
	extract_obj( comp[i] );
    }

    if ( type == 2 )
    {
	if ( number_percent() < ( 9 * skill / 10 ) )
	{
	    obj = create_object(get_obj_index(OBJ_VNUM_SCROLL));
	    act("{G$n{G scribbles some lines on an ancient parchment.{x",
		ch,NULL,NULL,TO_ROOM,POS_RESTING);
	    send_to_char("{GYou scribble some lines on an ancient parchment.{x\n\r",ch);
	    check_improve(ch,gsn_scribe,TRUE,3);
	} else {
	    act("{G$n{G scribbles some lines on an ancient parchment, only to notice a mistake.{x",
		ch,NULL,NULL,TO_ROOM,POS_RESTING);
	    send_to_char("{GYou scribble some lines on an ancient parchment, only to notice a mistake.{x\n\r",ch);
	    check_improve(ch,gsn_scribe,FALSE,3);
	    return;
	}
    }

    else
    {
	if ( number_percent() < ( 9 * skill / 10 ) )
	{
	    obj = create_object(get_obj_index(OBJ_VNUM_POTION));

	    act("{GWith considerable effort, $n{G brews a strange potion.{x",
		ch,NULL,NULL,TO_ROOM,POS_RESTING);
	    send_to_char("{GWith considerable effort, you brew a strange potion.{x\n\r",ch);
	    check_improve(ch,gsn_brew,TRUE,3);
	} else {
	    act("{G$n{G tries to brew a potion and fails miserably.{x",
		ch,NULL,NULL,TO_ROOM,POS_RESTING);
	    send_to_char("{GYou try to brew a potion and fail miserably.{x\n\r",ch);
	    check_improve(ch,gsn_brew,FALSE,3);
	    return;
	}
    }

    if ( obj == NULL )
	return;

    obj->value[0]	= ch->level - 5;
    obj->value[1]	= sn[0];
    obj->value[2]	= sn[1];
    obj->value[3]	= sn[2];
    obj->level		= ch->level;

    sprintf(nam,"%s%s%s%s%s", skill_table[sn[0]].name, sn[1] == -1 ? "" : " ",
	sn[1] == -1 ? "" : skill_table[sn[1]].name, sn[2] == -1 ? "" : " ",
	sn[2] == -1 ? "" : skill_table[sn[2]].name );

    sprintf(buf, obj->name, nam);
    free_string( obj->name );
    obj->name = str_dup( buf );

    sprintf(buf, obj->short_descr, nam);
    free_string( obj->short_descr );
    obj->short_descr = str_dup( buf );

    sprintf(buf, obj->description, nam);
    free_string( obj->description );
    obj->description = str_dup( buf );

    obj_to_char( obj, ch );

    return;
}

void do_brew( CHAR_DATA *ch, char *argument )
{
    brew_scribe( ch, argument, 1 );
    WAIT_STATE( ch, skill_table[gsn_brew].beats );
    return;
}

void do_scribe( CHAR_DATA *ch, char *argument )
{
    brew_scribe( ch, argument, 2 );
    WAIT_STATE( ch, skill_table[gsn_scribe].beats );
    return;
}

void do_cast( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char argall[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim = NULL;
    OBJ_DATA *obj = NULL;
    void *vo = NULL;
    int sn, target = TARGET_NONE;;

    argument = one_argument( argument, arg1 );
    sprintf(argall,"%s",argument);
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Cast which what where?\n\r", ch );
	return;
    }

    if ( ch->stunned )
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    if ( (sn = find_spell(ch,arg1)) < 0
    ||    get_skill(ch,sn) <= 0 )
    {
	send_to_char( "You don't know any spells of that name.\n\r", ch );
	return;
    }

    if ( skill_table[sn].spell_fun == spell_null )
    {
	sprintf(buf,"%s is a skill not a spell dumbass!\n\r",
	    capitalize(skill_table[sn].name));
	send_to_char(buf,ch);
	return;
    }

    if ( ch->position < skill_table[sn].minimum_position && !IS_IMMORTAL(ch) )
    {
	sprintf(buf,"{R%s {crequires much more concentration than you can currently focus.{x\n\r",
	    capitalize(skill_table[sn].name));
	send_to_char(buf,ch);
	return;
    }

    if ( is_affected( ch, skill_lookup( "silence" ) ) )
    {
	send_to_char( "Your lips fail to move.\n\r", ch );
	return;
    }

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Do_cast: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	global_spell_arg = argall;
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char( "Cast the spell on whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		spam_check( ch );
		return;
	    }
	}

	if ( !IS_NPC(ch) )
	{
            if (is_safe(ch,victim) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return;
	    }

	    if ( IS_NPC(victim)
	    &&   victim->fighting != NULL
	    &&   !IS_NPC(victim->fighting)
	    &&   victim->fighting != ch
	    &&   !is_same_group(ch,victim->fighting)
	    &&   !can_pk(ch,victim->fighting) )
	    {
		send_to_char("Kill stealing outside of pk range is not permitted.\n\r",ch);
		return;
	    }
	}

	check_pktimer(ch,victim,TRUE);

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    victim = ch;
	}
	else
	{
	    if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}

	if ( victim != ch
	&&   victim->fighting != NULL
	&&   !IS_NPC(victim->fighting)
	&&   !is_same_group(ch,victim->fighting)
	&&   victim->fighting != ch
	&&   ch->in_room != NULL
	&&   !IS_SET(ch->in_room->room_flags, ROOM_ARENA)
	&&   !IS_SET(ch->in_room->room_flags, ROOM_WAR)
	&&   !can_pk(ch,victim->fighting) )
	{
            send_to_char("Spelling up outside of pk range is not permitted.\n\r",ch);
            return;
        }

	if (!check_pktest(ch,victim))
	    return;

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SELF:
	if ( arg2[0] != '\0' && !is_name( arg2, ch->name ) )
	{
	    send_to_char( "You cannot cast this spell on another.\n\r", ch );
	    return;
	}

	vo = (void *) ch;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What should the spell be cast upon?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You are not carrying that.\n\r", ch );
	    return;
	}

	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
	if (arg2[0] == '\0')
	{
	    if ((victim = ch->fighting) == NULL)
	    {
		send_to_char("Cast the spell on whom or what?\n\r",ch);
		return;
	    }

	    target = TARGET_CHAR;
	}

	else if ((victim = get_char_room(ch,NULL,arg2)) != NULL)
	{
	    target = TARGET_CHAR;
	}

	if (target == TARGET_CHAR) /* check the sanity of the attack */
	{
 	    if (is_safe(ch,victim) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return;
	    }

            if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
            {
                send_to_char( "You can't do that on your own follower.\n\r",
                    ch );
                return;
            }

	    if ( IS_NPC(victim)
	    &&   victim->fighting != NULL
	    &&   !IS_NPC(victim->fighting)
	    &&   victim->fighting != ch
	    &&   !is_same_group(ch,victim->fighting)
	    &&   !can_pk(ch,victim->fighting) )
	    {
		send_to_char("Kill stealing outside of pk range is not permitted.\n\r",ch);
		return;
	    }

	    check_pktimer(ch,victim,TRUE);

	    vo = (void *) victim;
 	}

	else if ((obj = get_obj_here(ch,NULL,arg2)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}

	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    spam_check( ch );
	    return;
	}
	break;

    case TAR_OBJ_CHAR_DEF:
        if (arg2[0] == '\0')
        {
            vo = (void *) ch;
            target = TARGET_CHAR;
        }
        else if ((victim = get_char_room(ch,NULL,arg2)) != NULL)
        {

	    if ( victim != ch
	    &&   victim->fighting != NULL
	    &&   !IS_NPC(victim->fighting)
	    &&   victim->fighting != ch
	    &&   !is_same_group(ch,victim->fighting)
	    &&   ch->in_room != NULL
	    &&   !IS_SET(ch->in_room->room_flags, ROOM_ARENA)
	    &&   !IS_SET(ch->in_room->room_flags, ROOM_WAR)
	    &&   !can_pk(ch,victim->fighting) )
	    {
        	send_to_char("Spelling up outside of pk range is not permitted.\n\r",ch);
        	return;
	    }

	    if (!check_pktest(ch,victim))
		return;

            vo = (void *) victim;
            target = TARGET_CHAR;
	}
	else if ((obj = get_obj_carry(ch,arg2)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break;

    case TAR_OBJ_TRAN:
	if (arg2[0] == '\0')
	{
	    send_to_char("Transport what to whom?\n\r",ch);
	    return;
	}

	if (argument[0] == '\0')
	{
	    send_to_char("Transport it to whom?\n\r",ch);
	    return;
	}

        if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
        {
            send_to_char( "You are not carrying that.\n\r", ch );
            return;
        }

	if ( ( victim = get_char_world( ch, argument ) ) == NULL
	|| IS_NPC(victim) )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( obj->wear_loc != WEAR_NONE )
	{
	    send_to_char( "You must remove it first.\n\r", ch );
	    return;
	}

	if (IS_SET(victim->act,PLR_NOTRAN)
	&& ch->level < SQUIRE )
	{
	    send_to_char( "They do not accept transports.\n\r", ch);
	    return;
	}

	if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
	{
	    act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR,POS_RESTING);
	    return;
	}

	if (get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w( victim ) )
	{
	    act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR,POS_RESTING);
	    return;
	}

	if ( !can_see_obj( victim, obj ) )
	{
	    act( "$N can't see it.", ch, NULL, victim, TO_CHAR,POS_RESTING);
	    return;
	}

	if ( IS_SET(ch->in_room->room_flags, ROOM_WAR)
	||   IS_SET(ch->in_room->room_flags, ROOM_ARENA) )
	{
	    send_to_char("You may not transport stuff from the arena.\n\r",ch);
	    return;
	}

	if ( IS_SET(victim->in_room->room_flags, ROOM_WAR)
	||   IS_SET(victim->in_room->room_flags, ROOM_ARENA) )
	{
	    send_to_char("You may not transport stuff into the arena.\n\r",ch);
	    return;
	}

	if ( IS_SET(ch->affected_by, AFF_CHARM) )
	{
	    send_to_char("You can't use transport while charmed.\n\r",ch);
	    return;
	}

	if ( !cost_of_skill( ch, sn ) )
	    return;

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "It seems happy where it is.\n\r", ch);
	    check_improve(ch,sn,FALSE,1);
	    return;
	}

	if ((obj->pIndexData->vnum == OBJ_VNUM_VOODOO)
	&& (ch->level <= HERO))
	{
	    send_to_char( "You can't transport voodoo dolls.\n\r",ch);
	    check_improve(ch,sn,FALSE,1);
	    return;
	}

	if (!check_pktest(ch,victim))
	    return;

	if (!obj_droptest(victim,obj))
	{
	    act("You drop $p as it {Rs{rc{Ra{rl{Rd{rs{x you upon touching it!",
		victim,obj,NULL,TO_CHAR,POS_RESTING);
	    act("$n is {Rs{rc{Ra{rl{Rd{re{Rd{x by $p as $e grabs it!",
		victim,obj,NULL,TO_ROOM,POS_RESTING);
	    act("$p {Dv{wa{Dn{wi{Ds{wh{De{ws in a {Dm{wi{Ds{wt{x.",
		victim,obj,NULL,TO_CHAR,POS_RESTING);
	    act("$p {Dv{wa{Dn{wi{Ds{wh{De{ws in a {Dm{wi{Ds{wt{x.",
		victim,obj,NULL,TO_ROOM,POS_RESTING);
	    extract_obj(obj);
	    return;
	}

	if ( number_percent( ) > get_skill(ch,sn) )
	{
	    sprintf(buf,"{cYou lost your concentration while attempting to cast {R%s{c.{x\n\r",
		skill_table[sn].name);
	    send_to_char(buf,ch);
	    check_improve(ch,sn,FALSE,1);
	}
	else
	{
	    obj_from_char( obj );
	    obj_to_char( obj, victim );
	    act( "$p glows {Ggreen{x, then disappears.", ch, obj, victim, TO_CHAR,POS_RESTING);
	    act( "$p suddenly appears in your inventory.", ch, obj, victim, TO_VICT,POS_RESTING);
	    act( "$p glows {Ggreen{x, then disappears from $n's inventory.", ch, obj, victim, TO_NOTVICT,POS_RESTING);
	    check_improve(ch,sn,TRUE,1);
	}
	return;
	break;
    }

    if ( !cost_of_skill( ch, sn ) )
	return;

    if ( str_cmp( skill_table[sn].name, "ventriloquate" ) )
	say_spell( ch, sn );

    if ( number_percent( ) > get_skill(ch,sn) )
    {
	sprintf(buf,"{cYou lost your concentration while attempting to cast {R%s{c.{x\n\r",
	    skill_table[sn].name);
	send_to_char(buf,ch);
	check_improve(ch,sn,FALSE,1);
    } else {
	bool run_spell = TRUE;

	if ( ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
	||   ( skill_table[sn].target == TAR_OBJ_CHAR_OFF
	&&     target == TARGET_CHAR ) ) )
	{
	    int chance;

	    if ( IS_NPC( victim )
	    &&   victim->pIndexData->reflection
	    &&   victim->pIndexData->reflection > number_percent( ) )
	    {
		act( "$N reflects your spell back at you!", ch, NULL, vo, TO_CHAR, POS_RESTING );
		act( "$N reflects $n's spell back at $m!", ch, NULL, vo, TO_ROOM, POS_RESTING );
		act( "You reflect $n's spell back at $m!", ch, NULL, vo, TO_VICT, POS_RESTING );
		vo = ch;
		ch = victim;
	    }

	    if ( ( chance = get_skill( victim, gsn_cartwheel ) ) != 0 )
	    {
		if ( number_percent( ) < chance / 4 )
		{
		    act( "You cartwheel out of the way from $N's spell!",
			victim, NULL, ch, TO_CHAR, POS_RESTING );
		    act( "$n cartwheels to avoid $N's spell!",
			victim, NULL, ch, TO_NOTVICT, POS_RESTING );
		    act( "$n cartwheels out of the way of your spell!",
			victim, NULL, ch, TO_VICT, POS_RESTING );
		    check_improve( victim, gsn_cartwheel, TRUE, 3 );
		    run_spell = FALSE;
		}

		else
		    check_improve( victim, gsn_cartwheel, FALSE, 5 );
	    }

	    if ( run_spell
	    &&   IS_SHIELDED( victim, SHD_ABSORB )
	    &&   number_percent( ) <= 25 )
	    {
		act( "You absorb the magic of $n's offensive spell!",
		    ch, NULL, victim, TO_VICT, POS_RESTING );
		act( "$N absorbs your magic spell!",
		    ch, NULL, victim, TO_CHAR, POS_RESTING );
		run_spell = FALSE;
	    }
	}

	if ( run_spell )
	{
	    if ( ( *skill_table[sn].spell_fun )
		( sn, ch->magic_power, ch, vo, target ) )
	    {
		if ( skill_table[sn].sound_cast != NULL )
		    send_sound_room( ch->in_room, 75, 1, 95, "skills",
			skill_table[sn].sound_cast, SOUND_NOSKILL );
		check_improve( ch, sn, TRUE, 1 );
	    }
	}
    }

    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch && victim->master != ch)
    {
	CHAR_DATA *vch;

	for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
	{
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED, TRUE );
		break;
	    }
	}
    }
}

void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, char *argument )
{
    void *vo;
    int target = TARGET_NONE;
    bool run_spell = TRUE;

    if ( sn <= 0 )
	return;

    if ( sn >= maxSkill || skill_table[sn].spell_fun == 0 )
    {
	bug( "Obj_cast_spell: bad sn %d.", sn );
	return;
    }

    if ( ch->fighting != NULL )
	WAIT_STATE( ch, skill_table[sn].beats );

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( victim == NULL )
	    victim = ch->fighting;
	if ( victim == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	if (is_safe(ch,victim) && ch != victim)
	{
	    send_to_char("Something isn't right...\n\r",ch);
	    return;
	}
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_SELF:
	if ( victim == NULL )
	    victim = ch;
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( obj == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
        if ( victim == NULL && obj == NULL)
	{
	    if (ch->fighting != NULL)
		victim = ch->fighting;
	    else
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }
	}

	if (victim != NULL)
	{
	    if (is_safe_spell(ch,victim,FALSE) && ch != victim)
	    {
		send_to_char("Somehting isn't right...\n\r",ch);
		return;
	    }

	    vo = (void *) victim;
	    target = TARGET_CHAR;
	} else {
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
        break;

    case TAR_OBJ_CHAR_DEF:
	if (victim == NULL && obj == NULL)
	{
	    vo = (void *) ch;
	    target = TARGET_CHAR;
	}
	else if (victim != NULL)
	{
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	}
	else
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}

	break;
    }

    if ( ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   ( skill_table[sn].target == TAR_OBJ_CHAR_OFF
    &&     target == TARGET_CHAR ) ) )
    {
	int chance;

	if ( IS_NPC( victim )
	&&   victim->pIndexData->reflection
	&&   victim->pIndexData->reflection > number_percent( ) )
	{
	    act( "$N reflects your spell back at you!", ch, NULL, vo, TO_CHAR, POS_RESTING );
	    act( "$N reflects $n's spell back at $m!", ch, NULL, vo, TO_ROOM, POS_RESTING );
	    act( "You reflect $n's spell back at $m!", ch, NULL, vo, TO_VICT, POS_RESTING );
	    vo = ch;
	    ch = victim;
	}

	if ( ( chance = get_skill( victim, gsn_cartwheel ) ) != 0 )
	{
	    if ( number_percent( ) < chance / 4 )
	    {
		act( "You cartwheel out of the way from $N's spell!",
		    victim, NULL, ch, TO_CHAR, POS_RESTING );
		act( "$n cartwheels to avoid $N's spell!",
		    victim, NULL, ch, TO_NOTVICT, POS_RESTING );
		act( "$n cartwheels out of the way of your spell!",
		    victim, NULL, ch, TO_VICT, POS_RESTING );
		check_improve( victim, gsn_cartwheel, TRUE, 3 );
		run_spell = FALSE;
	    }

	    else
		check_improve( victim, gsn_cartwheel, FALSE, 5 );
	}

	if ( run_spell
	&&   IS_SHIELDED( victim, SHD_ABSORB )
	&&   number_percent( ) <= 25 )
	{
	    act( "You absorb the magic of $n's offensive spell!",
		ch, NULL, victim, TO_VICT, POS_RESTING );
	    act( "$N absorbs your magic spell!",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	    run_spell = FALSE;
	}
    }

    if ( run_spell )
    {
	if ( ( *skill_table[sn].spell_fun ) ( sn, level, ch, vo, target ) )
	    send_sound_room( ch->in_room, 75, 1, 95, "skills",
		skill_table[sn].sound_cast, SOUND_NOSKILL );
    }

    if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED, TRUE );
		break;
	    }
	}
    }

    return;
}

bool mob_cast( CHAR_DATA *ch, CHAR_DATA *victim, int sn )
{
    if ( sn < 0 || ch->stunned )
	return FALSE;

    if ( is_affected( ch, skill_lookup( "silence" ) ) )
    {
	send_to_char( "Your lips fail to move.\n\r", ch );
	return FALSE;
    }

    if ( !cost_of_skill( ch, sn ) )
	return FALSE;

    say_spell(ch,sn);

    if ( number_percent() > ch->learned[sn] )
    {
	send_to_char( "You lost your concentration.\n\r", ch );
	return TRUE;
    }

    if ( skill_table[sn].target == TAR_CHAR_DEFENSIVE
    ||   skill_table[sn].target == TAR_OBJ_CHAR_DEF )
	(*skill_table[sn].spell_fun) (sn,4*ch->magic_power/5,ch,ch,TARGET_CHAR);
    else
	(*skill_table[sn].spell_fun) (sn,4*ch->magic_power/5,ch,victim,TARGET_CHAR);

    if ( skill_table[sn].sound_cast != NULL )
	send_sound_room( ch->in_room, 75, 1, 95, "skills",
	    skill_table[sn].sound_cast, SOUND_NOSKILL );

    return TRUE;
}

bool spell_cancel( int level, CHAR_DATA *victim, bool dispel )
{
    AFFECT_DATA *paf, *paf_next;
    bool found = FALSE;
    sh_int count, sn;

    count = URANGE( 1, level / 20, 6 );

    for ( paf = victim->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;

	if ( paf->type > 0 && paf->type < maxSkill )
	    sn = paf->type;
	else
	    sn = 0;

	if ( ( ( dispel && IS_SET( skill_table[sn].flags, SKILL_CAN_DISPEL ) )
	||     ( !dispel && IS_SET( skill_table[sn].flags, SKILL_CAN_CANCEL ) ) )
	&&   check_dispel( level, victim, sn ) )
	{
	    found = TRUE;
	    if ( --count <= 0 )
		break;
	}
    }

    if ( IS_SHIELDED( victim, SHD_SANCTUARY )
    &&   !is_affected( victim, gsn_sanctuary )
    &&   !saves_dispel( level, victim->level, -1 ) )
    {
	REMOVE_BIT( victim->shielded_by, SHD_SANCTUARY );
	act( "The white aura around $n's body vanishes.",
	    victim, NULL, NULL, TO_ROOM, POS_RESTING );
	found = TRUE;
    }

    if ( IS_SHIELDED( victim, SHD_DIVINITY )
    &&   !is_affected( victim, gsn_divinity )
    &&   !saves_dispel( level, victim->level, -1 ) )
    {
	REMOVE_BIT( victim->shielded_by, SHD_DIVINITY );
	act( "The white aura around $n's body vanishes.",
	    victim, NULL, NULL, TO_ROOM, POS_RESTING );
	found = TRUE;
    }

    return found;
}

bool spell_acidshield( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == NULL )
	return FALSE;

    if ( IS_SHIELDED( victim, SHD_ACID ) )
    {
	if( victim == ch )
	    send_to_char( "You are already surrounded by an {Ga{gc{Gi{gd{Gi{gc{x shield.\n\r", ch );
	else
	    act( "$N is already surrounded by an {Ga{gc{Gi{gd{Gi{gc{x shield.",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	= TO_SHIELDS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= perm_affect ? -1 : 5 + level / 5;
    af.location	= APPLY_NONE;
    af.modifier	= 0;
    af.bitvector= SHD_ACID;

    affect_to_char( victim, &af );
    send_to_char( "You are surrounded by an {G{ga{Gc{gi{Gd{gi{Gc{x shield.\n\r", victim );
    act( "$n is surrounded by an {Ga{gc{Gi{gd{Gi{gc{x shield.",
	victim, NULL, NULL, TO_ROOM, POS_RESTING );
    return TRUE;
}

bool spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( victim == NULL )
	return FALSE;

    dam = dice( level * 3, 15 );

    if ( saves_spell( level, ch, victim, DAM_ACID ) )
	 dam = dice( level * 3, 11 );

    damage( ch, victim, dam, sn, DAM_ACID, TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, hpch, hp_dam, dice_dam;

    if ( victim == NULL )
	return FALSE;

    act( "$n spits acid at $N.", ch, NULL, victim, TO_NOTVICT, POS_RESTING );
    act( "$n spits a stream of corrosive acid at you.",
	ch, NULL, victim, TO_VICT, POS_RESTING );
    act( "You spit acid at $N.", ch, NULL, victim, TO_CHAR, POS_RESTING );

    hpch	= UMAX( 10, ch->max_mana );
    hp_dam	= number_range( hpch / 9 + 1, hpch / 5 );
    dice_dam	= dice( 2 * level, 45 );
    dam		= UMAX( hp_dam + dice_dam / 10, dice_dam + hp_dam / 10 );

    if ( saves_spell( level, ch, victim, DAM_ACID ) )
    {
	damage( ch, victim, 3 * dam / 4, sn, DAM_ACID, TRUE, FALSE, NULL );
	acid_effect( ch, victim, level / 2, dam / 4, TARGET_CHAR );
    } else {
	damage( ch, victim, dam, sn, DAM_ACID, TRUE, FALSE, NULL );
	acid_effect( ch, victim, level, dam, TARGET_CHAR );
    }

    return TRUE;
}

bool spell_acid_storm( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch;
    int dam;

    if ( victim == NULL )
	return FALSE;

    send_to_char( "You call forth {Ga{gc{Gid{gi{Gc{x showers from the heavens!\n\r", ch );
    act( "$n calls forth {Ga{gc{Gid{gi{Gc{x showers from the heavens!",
	ch, NULL, NULL, TO_ROOM, POS_RESTING );

    act( "Your {Ga{gc{Gid{gi{Gc{x showers rain down on $N!",
	ch, NULL, victim, TO_CHAR, POS_RESTING );
    send_to_char( "Your skin burns from {Ga{gci{Gd{x!\n\r", victim );

    if ( !saves_spell( level, ch, victim, DAM_ACID ) )
	dam = dice( level * 3, 7 );
    else
	dam = dice( level * 3, 5 );

    damage( ch, victim, dam, sn, DAM_ACID, TRUE, FALSE, NULL );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if ( vch != victim && !is_safe_spell( ch, vch, TRUE )
	&&   vch->damage_mod[DAM_ACID] > 0 )
	{
	    act( "Your {Ga{gc{Gid{gi{Gc{x showers rain down on $N!",
		ch, NULL, vch, TO_CHAR, POS_RESTING );
	    send_to_char("Your skin burns from {Ga{gci{Gd{x!\n\r",vch);

	    if ( !saves_spell( level, ch, vch, DAM_ACID ) )
		dam = dice( level * 3, 5 );
	    else
		dam = dice( level * 3, 3 );

	    damage( ch, vch, dam, sn, DAM_ACID, TRUE, FALSE, NULL );
	}
    }

    return TRUE;
}

bool spell_angelfire( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( victim == NULL )
	return FALSE;

    if ( !IS_NPC( ch ) && !IS_GOOD( ch ) )
    {
	victim = ch;
	send_to_char( "The angels turn upon you!\n\r", ch );
    }

    ch->alignment = URANGE( -1000, ch->alignment + 50, 1000 );

    if ( ch->pet != NULL )
	ch->pet->alignment = ch->alignment;

    if ( victim != ch )
    {
	act( "$n calls forth the angels of Heaven upon $N!",
	    ch, NULL, victim, TO_NOTVICT, POS_RESTING );
	act( "$n has assailed you with the angels of Heaven!",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	send_to_char( "You conjure forth the angels of Heaven!\n\r", ch );
    }

    dam = dice( level * 3, 11 );

    if ( saves_spell( level, ch, victim, DAM_HOLY ) )
	dam = dice( level * 3, 10 );

    damage( ch, victim, dam, sn, DAM_HOLY, TRUE, FALSE, NULL );
    spell_curse( gsn_curse, level / 2, ch, victim, TARGET_CHAR );

    return TRUE;
}

void initial_cap( char *argument )
{
    int pos;

    for ( pos = 0; argument[pos] != '\0'; pos += 2 )
    {
	if ( argument[pos] != '{' )
	{
	    argument[pos] = UPPER( argument[pos] );
	    return;
	}
    }
}

bool spell_animate( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *pet;
    OBJ_DATA *bpart = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];

    if ( bpart == NULL
    ||   bpart->pIndexData->vnum < 11
    ||   bpart->pIndexData->vnum > 22 )
    {
	send_to_char( "That's not a body part!\n\r", ch );
	return FALSE;
    }

    if ( ( pet = create_mobile( get_mob_index( MOB_VNUM_ANIMATE ) ) ) == NULL )
    {
	send_to_char( "Can not create mobile, contact IMMORTAL.\n\r", ch );
	return FALSE;
    }

    SET_BIT( pet->affected_by, AFF_CHARM );

    sprintf( buf, "%s\n{GIt's branded with the mark of %s.{x\n\r",
	bpart->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );

    free_string( pet->short_descr );
    pet->short_descr = str_dup( bpart->short_descr );

    free_string( pet->name );
    pet->name = str_dup( bpart->name );

    sprintf( buf, "%s is floating here.\n\r", bpart->short_descr );
    initial_cap( buf );
    free_string( pet->long_descr );
    pet->long_descr = str_dup( buf );

    char_to_room( pet, ch->in_room );
    add_follower( pet, ch );
    pet->leader = ch;
    obj_from_char( bpart );
    extract_obj( bpart );

    act( "$N floats up and starts following you.",
	ch, NULL, pet, TO_CHAR, POS_RESTING );
    act( "$N floats up and starts following $n.",
	ch, NULL, pet, TO_ROOM, POS_RESTING );
    return TRUE;
}

bool spell_preserve( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *part = (OBJ_DATA *) vo;

    if ( part == NULL
    ||   part->pIndexData->vnum < 11
    ||   part->pIndexData->vnum > 22 )
    {
	send_to_char( "You may only preserve severed body parts.\n\r", ch );
	return FALSE;
    }

    if ( part->enchanted == TRUE )
    {
	send_to_char( "That item has already been preserved.\n\r", ch );
	return FALSE;
    }

    part->timer += level / 3;
    part->enchanted = TRUE;

    act( "With great skill, you preserve $p.",
	ch, part, NULL, TO_CHAR, POS_RESTING );
    act( "$n stares at $p at starts chanting.",
	ch, part, NULL, TO_ROOM, POS_RESTING );

    return TRUE;
}

bool spell_embalm( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *corpse = (OBJ_DATA *) vo;

    if ( corpse == NULL
    ||   ( corpse->item_type != ITEM_CORPSE_PC
    &&     corpse->item_type != ITEM_CORPSE_NPC ) )
    {
	send_to_char( "You may only embalm corpses.\n\r", ch );
	return FALSE;
    }

    if ( corpse->enchanted == TRUE )
    {
	send_to_char( "That item has already been embalmed.\n\r", ch );
	return FALSE;
    }

    corpse->enchanted = TRUE;
    corpse->timer += level / 3;

    act( "With great skill, you embalm $p.",
	ch, corpse, NULL, TO_CHAR, POS_RESTING );
    act( "$n stares at $p at starts chanting.",
	ch, corpse, NULL, TO_ROOM, POS_RESTING );

    return TRUE;
}

bool spell_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == NULL )
	return FALSE;

    if ( is_affected( victim, sn ) )
    {
	if ( victim == ch )
	    send_to_char( "You are already armored.\n\r", ch );
	else
	    act( "$N is already armored.",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= level / 5;
    af.modifier	= -level;
    af.location	= APPLY_AC;
    af.bitvector= 0;
    affect_to_char( victim, &af );

    send_to_char( "You feel someone protecting you.\n\r", victim );

    if ( ch != victim )
	act( "$N is protected by your magic.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );

    return TRUE;
}

bool spell_awen( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == NULL )
	return FALSE;

    if ( is_affected( victim, sn ) )
    {
	if ( victim == ch )
	    send_to_char( "You can't be any more ready for battle!\n\r", ch );
	else
	    act( "$N can't be any more ready for battle!",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= level / 5;
    af.location	= APPLY_HITROLL;
    af.modifier	= level / 4;
    af.bitvector= 0;
    affect_to_char( victim, &af );

    af.location	= APPLY_DAMROLL;
    affect_to_char( victim, &af );

    send_to_char( "You are surrounded by a battle awen.\n\r", victim );

    if ( ch != victim )
	act( "$N is consumed by a battle awen.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );

    return TRUE;
}

bool spell_backdraft( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if ( obj == NULL || obj->item_type != ITEM_WEAPON )
    {
	send_to_char( "Backdraft may only be used on weapons.\n\r", ch );
	return FALSE;
    }

    if ( IS_WEAPON_STAT( obj, WEAPON_FLAMING ) )
    {
	act( "$p is already embedded with fire.",
	    ch, obj, NULL, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	= TO_WEAPON;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= level / 4;
    af.location	= 0;
    af.modifier	= 0;
    af.bitvector= WEAPON_FLAMING;
    affect_to_obj( obj, &af );

    act( "$p is embedded with {rf{Rir{re{x.",
	ch, obj, NULL, TO_ALL, POS_RESTING );
    return TRUE;
}

bool spell_barkskin( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == NULL )
	return FALSE;

    if ( is_affected( victim, sn ) )
    {
	if ( victim == ch )
	    send_to_char( "Your skin is already as tough {Gt{gre{Ge {ybark{x.\n\r", ch );
	else
	    act( "$N's skin is already as tough as {Gt{gre{Ge {ybark{x.",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= level / 5;
    af.modifier	= -level;
    af.location	= APPLY_AC;
    af.bitvector= 0;
    affect_to_char( victim, &af );

    send_to_char( "Your skin turns to {Gt{gre{Ge {ybark{x.\n\r", victim );

    if ( ch != victim )
	act("$N's skin turns to {Gt{gre{Ge {ybark{x.",
	    ch,NULL,victim,TO_CHAR,POS_RESTING);

    return TRUE;
}

bool spell_bless( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;

    if ( target == TARGET_OBJ )
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	if ( obj == NULL )
	    return FALSE;

	if ( IS_OBJ_STAT( obj, ITEM_BLESS ) )
	{
	    act( "$p is already blessed.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	    return FALSE;
	}

	if ( IS_OBJ_STAT( obj, ITEM_EVIL ) )
	{
	    AFFECT_DATA *paf = affect_find( obj->affected, gsn_curse );

	    if ( !saves_dispel( level, paf != NULL ? paf->level : obj->level, 0 ) )
	    {
		if ( paf != NULL )
			affect_remove_obj( obj, paf );
		act( "$p glows a pale blue.",
		    ch, obj, NULL, TO_ALL, POS_RESTING );
		REMOVE_BIT( obj->extra_flags, ITEM_EVIL );
		return TRUE;
	    } else {
		act( "The evil of $p is too powerful for you to overcome.",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
		return TRUE;
	    }
	}

	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.dur_type	= DUR_TICKS;
	af.duration	= -1;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= ITEM_BLESS;
	affect_to_obj( obj, &af );

	act( "$p glows with a holy aura.",
	    ch, obj, NULL, TO_ALL, POS_RESTING );
	return TRUE;
    }

    else
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if ( victim == NULL )
	    return FALSE;

	if ( is_affected( victim, sn )
	||   is_affected( victim, gsn_infernal_offer )
	||   is_affected( victim, gsn_divine_blessing ) )
	{
	    if ( victim == ch )
		send_to_char( "You are already blessed.\n\r", ch );
	    else
		act( "$N already has divine favor.",
		    ch, NULL, victim, TO_CHAR, POS_RESTING );
	    return FALSE;
	}

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.dur_type	= DUR_TICKS;
	af.duration	= 6 + level / 3;
	af.location	= APPLY_HITROLL;
	af.modifier	= level / 8;
	af.bitvector	= 0;
	affect_to_char( victim, &af );

	af.location	= APPLY_SAVES;
	af.modifier	= 0 - level / 8;
	affect_to_char( victim, &af );
	send_to_char( "You feel righteous.\n\r", victim );

	if ( ch != victim )
	    act( "You grant $N the favor of your god.",
		ch, NULL, victim, TO_CHAR, POS_RESTING );

	return TRUE;
    }

    return FALSE;
}

bool spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == NULL )
	return FALSE;

    if ( IS_AFFECTED( victim, AFF_BLIND ) )
    {
	act( "$N is already blind.", ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    level = check_curse_of_ages( ch, level );

    if ( saves_spell( level, ch, victim, DAM_OTHER ) )
    {
	act( "You failed to blind $N.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return TRUE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.location	= APPLY_HITROLL;
    af.modifier	= -level;
    af.dur_type	= DUR_TICKS;
    af.duration	= perm_affect ? -1 : level / 20;
    af.bitvector= AFF_BLIND;
    affect_to_char( victim, &af );
    send_to_char( "You are blinded!\n\r", victim );
    act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_bloodbath( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected(victim,gsn_bloodbath) )
    {
	act("$N's wounds are already bleeding uncontrollably.",
	    ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    if ( saves_spell( 3 * level / 2, ch, victim, DAM_OTHER ) )
    {
	send_to_char("Spell failed.\n\r",ch);
	return TRUE;
    }

    if ( victim != ch )
    {
	combat("{x$N screams in terrible agony as {Rb{rl{Ro{ro{Rd{x gushes forth from $s many wounds!",
	    ch,NULL,victim,TO_NOTVICT,COMBAT_OTHER);
	send_to_char("{xYou scream in terrible agony as {Rb{rl{Ro{ro{Rd{x gushes forth from your many wounds!\n\r",victim);
	act("{x$N screams in terrible agony as {Rb{rl{Ro{ro{Rd{x gushes forth from $s many wounds!",
	    ch,NULL,victim,TO_CHAR,POS_RESTING);
    }

    af.where	 = TO_DAM_MODS;
    af.type	 = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration	 = 2;
    af.location	 = DAM_NEGATIVE;
    af.modifier	 = 10;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    return TRUE;
}
bool spell_boiling_blood(int sn,int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo; 
    AFFECT_DATA af; 
    if (is_affected(victim,sn))
		{
		send_to_char( "Your {1blood{x is already {!boiling{x!\n\r", victim ); 
		return FALSE;
		}
	else
		{
		af.where     = TO_AFFECTS; 
		af.type      = sn; 
		af.level     = level; 
		af.dur_type  = DUR_TICKS;
		af.duration  = level/2; 
		af.location  = APPLY_STR; 
		af.modifier  = 4 + (level >= 18) + (level >= 25) + (level >= 32); 
		af.bitvector = AFF_HASTE; 
		affect_to_char( victim, &af ); 
		af.location  = APPLY_DEX; 
		af.modifier  = 4 + (level >= 18) + (level >= 25) + (level >= 32); 
		affect_to_char( victim, &af ); 
		send_to_char( "Your {1blood{x begins to {!boil{x!\n\r", victim ); 
		act("$n's {1blood{x begins to {!boil{x!\n\r",victim,NULL,NULL,TO_ROOM,POS_RESTING);
		}
	return TRUE;
}
bool spell_burning_hands(int sn,int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level,4);

    if ( saves_spell( level, ch, victim,DAM_FIRE) )
	dam = dice(level, 3);

    damage( ch, victim, dam, sn, DAM_FIRE,TRUE,FALSE,NULL);
    return TRUE;
}

bool spell_call_lightning( int sn, int level,CHAR_DATA *ch,void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You must be out of doors.\n\r", ch );
	return FALSE;
    }

    if ( weather_info.sky < SKY_RAINING )
    {
	send_to_char( "You need bad weather.\n\r", ch );
	return FALSE;
    }

    dam = dice( level * 6, 10);

    act( "$G's {Ylightning{x strikes your foes!", ch, NULL, NULL, TO_CHAR,POS_RESTING);
    act( "$n calls $G's {Ylightning{x to strike $s foes!",
	ch, NULL, NULL, TO_ROOM,POS_RESTING);

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
	    {
		damage( ch, vch, saves_spell( level,ch, vch,DAM_LIGHTNING)
		? dam / 2 : dam, sn,DAM_LIGHTNING,TRUE,FALSE,NULL);
	    }
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area
	&&   IS_OUTSIDE(vch)
	&&   IS_AWAKE(vch) )
	    send_to_char( "{z{YLightning{x flashes in the sky.\n\r", vch );
    }

    return TRUE;
}

bool spell_calm( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;
    int chance;
    AFFECT_DATA af;

    if ( ch->in_room == NULL || IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	send_to_char( "Not in a safe room!\n\r", ch );
	return FALSE;
    }

    /* get sum of all mobile levels in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->position == POS_FIGHTING)
	{
	    count++;
	    if (IS_NPC(vch))
	      mlevel += vch->level;
	    else
	      mlevel += vch->level/2;
	    high_level = UMAX(high_level,vch->level);
	}
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (IS_IMMORTAL(ch)) /* always works */
      mlevel = 0;

    if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
   	{
	    if ( IS_AFFECTED( vch, AFF_CALM ) )
		continue;

	    send_to_char("A wave of calm passes over you.\n\r",vch);

	    if (vch->fighting || vch->position == POS_FIGHTING)
	      stop_fighting(vch,TRUE);


	    af.where = TO_AFFECTS;
	    af.type = sn;
  	    af.level = level;
	    af.dur_type  = DUR_TICKS;
	    af.duration = perm_affect ? -1 : 1;
	    af.location = APPLY_HITROLL;
	    af.modifier = -level;
	    af.bitvector = AFF_CALM;
	    affect_to_char(vch,&af);

	    af.location = APPLY_DAMROLL;
	    affect_to_char(vch,&af);
	}
    }
    return TRUE;
}

bool spell_cancellation( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    level += 2;

    if ( (!IS_NPC(ch) && IS_NPC(victim))
    ||    (IS_NPC(ch) && !IS_NPC(victim)) )
    {
	send_to_char("You failed, try dispel magic.\n\r",ch);
	return FALSE;
    }

    if ( !IS_NPC(victim) && IS_SET(victim->act,PLR_NOCANCEL) && victim != ch
    &&   !IS_IMMORTAL( ch ) )
    {
	send_to_char("You must use dispel magic on that target.\n\r",ch);
	return FALSE;
    }

    if (spell_cancel(level,victim, FALSE))
    {
	if (ch == victim)
	    send_to_char("Ok.\n\r",ch);
	else
	    act("You magic weakens $N.",ch,NULL,victim,TO_CHAR,POS_RESTING);
    } else {
	if (ch == victim)
	    send_to_char("You failed.\n\r",ch);
	else
	    act("Your magic fails to weaken $N.",ch,NULL,victim,TO_CHAR,POS_RESTING);
    }
    return TRUE;
}

bool spell_cause_light( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    damage( ch, victim, dice(level / 2, level / 4) + level, sn,DAM_MAGIC,TRUE,FALSE,NULL);

    return TRUE;
}

bool spell_cause_critical(int sn,int level,CHAR_DATA *ch,void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    damage( ch, victim, dice(level / 2, level / 3) + level, sn,DAM_MAGIC,TRUE,FALSE,NULL);
    return TRUE;
}

bool spell_cause_serious(int sn,int level,CHAR_DATA *ch,void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    damage( ch, victim, dice(level / 2, level / 2) + level, sn,DAM_MAGIC,TRUE,FALSE,NULL);
    return TRUE;
}

bool spell_chain_lightning(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *tmp_vict,*last_vict,*next_vict;
    bool found;
    int dam;

    combat("A lightning bolt leaps from $n's hand and arcs to $N.",
        ch,NULL,victim,TO_ROOM,COMBAT_CHAIN_SPAM);
    combat("A lightning bolt leaps from your hand and arcs to $N.",
	ch,NULL,victim,TO_CHAR,COMBAT_CHAIN_SPAM);
    combat("A lightning bolt leaps from $n's hand and hits you!",
	ch,NULL,victim,TO_VICT,COMBAT_CHAIN_SPAM);

    dam = dice(level * 3, 7);

    if (saves_spell(level,ch,victim,DAM_LIGHTNING))
 	dam = dice(level * 3,6);

    damage(ch,victim,dam,sn,DAM_LIGHTNING,TRUE,FALSE,NULL);
    last_vict = victim;
    level -= 4;   /* decrement damage */

    /* new targets */
    while ( level > 0 && ch && ch->in_room )
    {
	found = FALSE;
	for (tmp_vict = ch->in_room->people;
	     tmp_vict != NULL;
	     tmp_vict = next_vict)
	{
	  next_vict = tmp_vict->next_in_room;
	  if (!is_safe_spell(ch,tmp_vict,TRUE) && tmp_vict != last_vict)
	  {
	    found = TRUE;
	    last_vict = tmp_vict;
	    combat("The bolt arcs to $n!",tmp_vict,NULL,NULL,TO_ROOM,COMBAT_CHAIN_SPAM);
	    combat("The bolt hits you!",tmp_vict,NULL,NULL,TO_CHAR,COMBAT_CHAIN_SPAM);
	    dam = dice(level * 3,7);
	    if (saves_spell(level,ch,tmp_vict,DAM_LIGHTNING))
		dam = dice(level * 3,6);
	    damage(ch,tmp_vict,dam,sn,DAM_LIGHTNING,TRUE,FALSE,NULL);
	    level -= 4;  /* decrement damage */
	  }
	}   /* end target searching loop */

	if (!found) /* no target found, hit the caster */
	{
	  if (ch == NULL)
     	    return TRUE;

	  if (last_vict == ch) /* no double hits */
	  {
	    combat("The bolt seems to have fizzled out.",ch,NULL,NULL,TO_ROOM,COMBAT_CHAIN_SPAM);
	    combat("The bolt grounds out through your body.",
		ch,NULL,NULL,TO_CHAR,COMBAT_CHAIN_SPAM);
	    return TRUE;
	  }

	  last_vict = ch;
	  combat("The bolt arcs to $n...whoops!",ch,NULL,NULL,TO_ROOM,COMBAT_CHAIN_SPAM);
	  combat("You are struck by your own lightning!",ch,NULL,NULL,TO_CHAR,COMBAT_CHAIN_SPAM);
	  dam = dice(level,7);
	  if (saves_spell(level,victim,ch,DAM_LIGHTNING))
	   dam = dice(level,6);
	  damage(ch,ch,dam,sn,DAM_LIGHTNING,TRUE,FALSE,NULL);
	  level -= 4;  /* decrement damage */
	  if (ch == NULL)
	    return TRUE;
	}
    }
    return TRUE;
}

bool spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ))
    {
	if (victim == ch)
	  send_to_char("You've already been changed.\n\r",ch);
	else
	  act("$N has already had $s(?) sex changed.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }
    level = check_curse_of_ages( ch, level );

    if (saves_spell(level ,ch,  victim,DAM_OTHER))
	return TRUE;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = level / 25;
    af.location  = APPLY_SEX;
    do
    {
	af.modifier  = number_range( 0, 2 ) - victim->sex;
    }
    while ( af.modifier == 0 );
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel different.\n\r", victim );
    act("$n doesn't look like $mself anymore...",victim,NULL,NULL,TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_channel( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( victim == ch )
    {
	send_to_char( "That doesn't make much sense.\n\r", ch );
	return FALSE;
    }

    if ( ch->mana < 2000 )
    {
	send_to_char( "You can not muster the energy.\n\r", ch );
	return FALSE;
    }

    victim->mana = UMIN( victim->mana + 1000, victim->max_mana );
    ch->mana -= 2000;
    update_pos( victim );

    send_to_char( "An energetic feeling fills your body.\n\r", victim );

    send_to_char( "Ok.\n\r", ch );
    show_condition( ch, victim, VALUE_MANA_POINT );

    return TRUE;
}

bool spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *wch;
    AFFECT_DATA af;
    sh_int count = 0;

    if ( victim == ch )
    {
	send_to_char( "You like yourself even better!\n\r", ch );
	return FALSE;
    }

    if ( victim->position == POS_SLEEPING )
    {
	send_to_char( "You can not get your victim's attention.\n\r", ch );
	send_to_char( "Your slumbers are briefly troubled.\n\r", victim );
	return FALSE;
    }

    if ( is_safe(ch,victim)
    ||   IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level < victim->level
    ||   saves_spell( level, ch, victim,DAM_CHARM) )
	return FALSE;

    if ( IS_SET(victim->in_room->room_flags,ROOM_LAW) )
    {
	send_to_char("The mayor does not allow charming in the city limits.\n\r",ch);
	return FALSE;
    }

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( IS_AFFECTED(wch,AFF_CHARM)
	&&   wch->master == ch )
	    count++;
    }

    if ( count > (level / 20) + 2 )
    {
	send_to_char("Your willpower is not strong enough to control that many followers.\n\r",ch);
	return FALSE;
    }

    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = level / 10;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );

    act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT,POS_RESTING);

    if ( ch != victim )
	act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR,POS_RESTING);
    return TRUE;
}

bool spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam;

    dam = dice(level,4);

    if ( !saves_spell( level/3, ch, victim, DAM_COLD ) )
    {
	act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
	af.where     = TO_AFFECTS;
	af.type      = sn;
        af.level     = level;
	af.dur_type  = DUR_TICKS;
	af.duration  = level / 20;
	af.location  = APPLY_CON;
	af.modifier  = - level / 25;
	af.bitvector = 0;
	affect_join( victim, &af );
    }
    else
    {
	dam = dice(level,3);
    }

    damage( ch, victim, dam, sn, DAM_COLD,TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    int loop;

    struct flag_type damages[] =
    {
	{ "{rn{Re{rg{Ra{rt{Ri{rv{Re",	DAM_NEGATIVE	}, // Red
	{ "{Rf{yi{Rr{ye",		DAM_FIRE	}, // Orange
	{ "{Ylight",			DAM_LIGHT	}, // Yellow
	{ "{Ge{ga{Gr{gt{Gh",		DAM_EARTH	}, // Green
	{ "{Bc{Co{Bl{Cd",		DAM_COLD	}, // Blue
	{ "{blightning",		DAM_LIGHTNING	}, // Indigo
	{ "{Mmental",			DAM_MENTAL	}, // Violet
	{ NULL,				DAM_OTHER	}
    };

    for ( loop = 0; damages[loop].name != NULL; loop++ )
    {
	if ( victim == NULL || !victim->valid
	||   ( loop > 0 && ch->fighting == NULL ) )
	    return TRUE;

	sprintf( buf, "%s {cspray{x", damages[loop].name );
	damage( ch, victim, dice( level / 2, 4 ), sn, damages[loop].bit, TRUE, FALSE, buf );
    }

    return TRUE;
}

bool spell_conjure( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    char buf[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    OBJ_DATA *stone;
    CHAR_DATA *pet;

    if (IS_NPC(ch))
	return FALSE;

    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch)
    &&  (stone == NULL || stone->item_type != ITEM_DEMON_STONE))
    {
        send_to_char("You lack the proper component for this spell.\n\r",ch);
        return FALSE;
    }

    if ( ch->pet != NULL )
    {
	send_to_char("You failed.\n\r",ch);
	return FALSE;
    }

    if (stone != NULL && stone->item_type == ITEM_DEMON_STONE)
    {
	if (stone->value[0] < 1)
	{
	    act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR,POS_RESTING);
	    act("$n draws upon the power of $p.",ch,stone,NULL,TO_ROOM,POS_RESTING);
	    act("It flares brightly and explodes into dust.",ch,stone,NULL,TO_CHAR,POS_RESTING);
	    act("It flares brightly and explodes into dust.",ch,stone,NULL,TO_ROOM,POS_RESTING);
	    extract_obj( stone );
	    return TRUE;
	}
    }

    pMobIndex = get_mob_index( MOB_VNUM_DEMON );
    pet = create_mobile( pMobIndex );
    if ( pet == NULL )
	return FALSE;
    if (!IS_SET(pet->act, ACT_PET) )
        SET_BIT(pet->act, ACT_PET);
    if (!IS_SET(pet->affected_by, AFF_CHARM) )
        SET_BIT(pet->affected_by, AFF_CHARM);
    sprintf( buf, "%s{GThe mark of %s is on its forehead.{x\n\r",
        pet->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );
    char_to_room( pet, ch->in_room );
    if (stone != NULL && stone->item_type == ITEM_DEMON_STONE)
    {
        stone->value[0] = UMAX(0, stone->value[0]-1);
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR,POS_RESTING);
        act("$n draws upon the power of $p.",ch,stone,NULL,TO_ROOM,POS_RESTING);
        act("It flares brightly and $N appears.",ch,stone,pet,TO_CHAR,POS_RESTING);
        act("It flares brightly and $N appears.",ch,stone,pet,TO_ROOM,POS_RESTING);
    } else {
        act("$N suddenly appears in the room.",ch,NULL,pet,TO_CHAR,POS_RESTING);
        act("$N suddenly appears in the room.",ch,NULL,pet,TO_ROOM,POS_RESTING);
    }
    add_follower( pet, ch );
    pet->leader = ch;
    ch->pet = pet;
    pet->alignment = ch->alignment;
    pet->level = level * 3 / 2;
    pet->max_hit = number_range(ch->mana * 1 / 3, ch->mana );
    pet->hit = pet->max_hit;
    pet->max_mana = number_range(ch->mana * 1 / 3, ch->mana );
    pet->mana = pet->max_mana;
    pet->damage[DICE_NUMBER] = level / 5;
    pet->damage[DICE_TYPE] = level / 5;
    pet->damroll = number_range( level * 7, level * 12 );
    pet->hitroll = number_range( level * 7, level * 12 );
    pet->armor[0] = -level;
    pet->armor[1] = -level;
    pet->armor[2] = -level;
    pet->armor[3] = -level;

    return TRUE;
}

bool spell_constitution( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your constitution can't get any better.\n\r",ch);
 	else
	  act("$N's constitution can't get any better.",
	      ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = level/10;
    af.location  = APPLY_CON;
    af.modifier  = UMAX(1,level / 30);
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char( "You feel your constitution increase.\n\r", victim );

    act("$n's constitution increases.",victim,NULL,NULL,TO_ROOM,POS_RESTING);

    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_continual_light(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    AFFECT_DATA af;
    OBJ_DATA *light;
    char buf[MAX_STRING_LENGTH];

    sprintf(buf,"%s", global_spell_arg);

    if (buf[0] != '\0')
    {
	light = get_obj_carry(ch,buf);

	if (light == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return FALSE;
	}

	if (IS_OBJ_STAT(light,ITEM_GLOW))
	{
	    act("$p is already glowing.",ch,light,NULL,TO_CHAR,POS_RESTING);
	    return FALSE;
	}

	SET_BIT(light->extra_flags,ITEM_GLOW);
	act("$p glows with a white light.",ch,light,NULL,TO_ALL,POS_RESTING);
	return TRUE;
    }

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ) );

    if ( light == NULL )
	return FALSE;

    light->level    = level - 5;
    light->value[2] = level;
    light->timer    = level * 2;

    af.where        = TO_OBJECT;
    af.type         = 0;
    af.level        = level;
    af.duration     = -1;
    af.location     = APPLY_HITROLL;
    af.modifier     = level/6;
    af.bitvector    = 0;
    affect_to_obj(light,&af);

    af.location     = APPLY_DAMROLL;
    affect_to_obj(light,&af);

    af.location     = APPLY_HIT;
    af.modifier     = level * 2;
    affect_to_obj(light,&af);

    af.location     = APPLY_MANA;
    affect_to_obj(light,&af);

    set_arena_obj( ch, light );
    obj_to_room( light, ch->in_room );
    act( "$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM,POS_RESTING);
    act( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR,POS_RESTING);
    return TRUE;
}

bool spell_control_weather(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    if ( !str_cmp( global_spell_arg, "better" ) )
	weather_info.change += 20;
    else if ( !str_cmp( global_spell_arg, "worse" ) )
	weather_info.change -= 20;
    else
	send_to_char ("Do you want it to get better or worse?\n\r", ch );

    send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *mushroom;

    mushroom = create_object( get_obj_index( number_range(26,31) ) );

    if ( mushroom == NULL )
	return FALSE;

    mushroom->value[0] = level / 2;
    mushroom->value[1] = level;
    set_arena_obj( ch, mushroom );
    obj_to_room( mushroom, ch->in_room );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM,POS_RESTING);
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR,POS_RESTING);
    return TRUE;
}

bool spell_create_raft( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *raft;

    raft = create_object( get_obj_index( 45 ) );

    if ( raft == NULL )
	return FALSE;

    raft->level = level;
    raft->timer = level / 3;

    if (ch->carry_number + get_obj_number(raft) > can_carry_n(ch))
    {
	send_to_char("You can't carry that many items.\n\r",ch);
	extract_obj(raft);
	return FALSE;
    }

    if (get_carry_weight(ch) + get_obj_weight(raft) > can_carry_w(ch))
    {
	send_to_char("You can't carry that much weight.\n\r",ch);
	extract_obj(raft);
	return FALSE;
    }

    act("$n quickly cunjures up $p.",ch,raft,NULL,TO_ROOM,POS_RESTING);
    act("You quickly weave up $p.",ch,raft,NULL,TO_CHAR,POS_RESTING);

    obj_to_char(raft,ch);
    return TRUE;
}

bool spell_create_rose( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *rose;

    rose = create_object(get_obj_index(number_range(32,40) ));

    if ( rose == NULL )
	return FALSE;

    act( "$n has pulled $p from up $s sleeve!", ch, rose, NULL, TO_ROOM,POS_RESTING);
    act( "You pull $p out from up your sleeve!", ch, rose, NULL, TO_CHAR,POS_RESTING);

    if (ch->carry_number + get_obj_number(rose) > can_carry_n(ch))
    {
	send_to_char("You can't carry that many items.\n\r",ch);
	extract_obj(rose);
	return FALSE;
    }

    if (get_carry_weight(ch) + get_obj_weight(rose) > can_carry_w(ch))
    {
	send_to_char("You can't carry that much weight.\n\r",ch);
	extract_obj(rose);
	return FALSE;
    }

    obj_to_char(rose,ch);
    return FALSE;
}

bool spell_create_spring(int sn,int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *spring;

    spring = create_object( get_obj_index( number_range( 49, 54 ) ) );

    if ( spring == NULL )
	return FALSE;

    spring->timer = level / 10;
    set_arena_obj( ch, spring );
    obj_to_room( spring, ch->in_room );
    act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM,POS_RESTING);
    act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR,POS_RESTING);
    return TRUE;
}

bool spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if ( obj == NULL )
	return FALSE;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "It is unable to hold water.\n\r", ch );
	return FALSE;
    }

    if ( obj->value[2] != 0 && obj->value[1] != 0 )
    {
	send_to_char( "It contains some other liquid.\n\r", ch );
	return FALSE;
    }

    water = UMIN(
		level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		obj->value[0] - obj->value[1]
		);

    if ( water > 0 )
    {
	obj->value[2] = 0;
	obj->value[1] += water;
	if ( !is_name( "water", obj->name ) )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "%s water", obj->name );
	    free_string( obj->name );
	    obj->name = str_dup( buf );
	}
	act( "$p is filled.", ch, obj, NULL, TO_CHAR,POS_RESTING);
    }

    return TRUE;
}

bool spell_cure_blindness(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_blindness ) )
    {
        if (victim == ch)
          send_to_char("You aren't blind.\n\r",ch);
        else
          act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    if (!check_dispel(3*level/2,victim,gsn_blindness))
        send_to_char("Spell failed.\n\r",ch);

    return TRUE;
}

bool spell_cure_critical( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    victim->hit = UMIN( victim->hit + 150, victim->max_hit );

    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );

    if ( ch != victim )
    {
	send_to_char( "OK.\n\r", ch );
	show_condition( ch, victim, VALUE_HIT_POINT );
    }

    return TRUE;
}

bool spell_cure_disease( int sn, int level, CHAR_DATA *ch,void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_plague ) )
    {
        if (victim == ch)
          send_to_char("You aren't ill.\n\r",ch);
        else
          act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    if (check_dispel(3*level/2,victim,gsn_plague))
    {
	act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
    }
    else
	send_to_char("Spell failed.\n\r",ch);

    return TRUE;
}

bool spell_cure_light( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    victim->hit = UMIN( victim->hit + 50, victim->max_hit );

    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
    {
	send_to_char( "Ok.\n\r", ch );
	show_condition( ch, victim, VALUE_HIT_POINT );
    }

    return TRUE;
}

bool spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_poison ) )
    {
        if (victim == ch)
          send_to_char("You aren't poisoned.\n\r",ch);
        else
          act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    if (check_dispel(3*level/2,victim,gsn_poison))
    {
        act("$n looks much better.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
    }
    else
        send_to_char("Spell failed.\n\r",ch);

    return TRUE;
}

bool spell_cure_serious( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    victim->hit = UMIN( victim->hit + 100, victim->max_hit );

    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
    {
	send_to_char( "Ok.\n\r", ch );
	show_condition( ch, victim, VALUE_HIT_POINT );
    }

    return TRUE;
}

bool spell_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    if (target == TARGET_OBJ)
    {
        obj = (OBJ_DATA *) vo;
        if (IS_OBJ_STAT(obj,ITEM_EVIL))
        {
            act("$p is already filled with evil.",ch,obj,NULL,TO_CHAR,POS_RESTING);
            return FALSE;
        }

        if (IS_OBJ_STAT(obj,ITEM_BLESS))
        {
            AFFECT_DATA *paf;

            paf = affect_find(obj->affected,gsn_bless);
            if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
            {
                if (paf != NULL)
                    affect_remove_obj(obj,paf);
                act("$p glows with a red aura.",ch,obj,NULL,TO_ALL,POS_RESTING);
                REMOVE_BIT(obj->extra_flags,ITEM_BLESS);
                return TRUE;
            }
            else
            {
                act("The holy aura of $p is too powerful for you to overcome.",
                    ch,obj,NULL,TO_CHAR,POS_RESTING);
                return TRUE;
            }
        }

        af.where        = TO_OBJECT;
        af.type         = sn;
        af.level        = level;
	af.dur_type	= DUR_TICKS;
        af.duration     = 2 * level;
        af.location     = APPLY_SAVES;
        af.modifier     = +1;
        af.bitvector    = ITEM_EVIL;
        affect_to_obj(obj,&af);

        act("$p glows with a malevolent aura.",ch,obj,NULL,TO_ALL,POS_RESTING);
        return TRUE;
    }

    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED(victim,AFF_CURSE))
    {
        act( "$N is already cursed.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    level = check_curse_of_ages( ch, level );

    if (saves_spell(level,ch,victim,DAM_NEGATIVE))
    {
        act( "You failed to curse $N.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return TRUE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 12;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * (level / 15);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVES;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

    send_to_char( "You feel unclean.\n\r", victim );
    if ( ch != victim )
	act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR,POS_RESTING);
    return TRUE;
}

bool spell_darkvision( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DARK_VISION) )
    {
	if (victim == ch)
	  send_to_char("You can already sense hidden things.\n\r",ch);
	else
	  act("$N can already sense hidden things.",
	    ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 4;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DARK_VISION;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_demonfire( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( !IS_NPC(ch) && !IS_EVIL(ch) )
    {
        victim = ch;
	send_to_char("The demons turn upon you!\n\r",ch);
    }

    ch->alignment = URANGE(-1000, ch->alignment - 50,1000);
    if ( ch->pet != NULL )
	ch->pet->alignment = ch->alignment;

    if (victim != ch)
    {
	act("$n calls forth the demons of Hell upon $N!",
	    ch,NULL,victim,TO_NOTVICT,POS_RESTING);
        act("$n has assailed you with the demons of Hell!",
	    ch,NULL,victim,TO_VICT,POS_RESTING);
	send_to_char("You conjure forth the demons of hell!\n\r",ch);
    }

   dam = dice(level * 3,5);

    if ( saves_spell( level,ch, victim,DAM_NEGATIVE) )
	dam = dice(level * 3,4);

    damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, FALSE, NULL);
    spell_curse( gsn_curse, level / 2, ch, victim, TARGET_CHAR );
    return TRUE;
}

bool spell_detect_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) )
    {
	if (victim == ch)
	  send_to_char("You can already sense evil.\n\r",ch);
	else
	  act("$N can already detect evil.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 2;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_detect_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_GOOD) )
    {
        if (victim == ch)
          send_to_char("You can already sense good.\n\r",ch);
        else
          act("$N can already detect good.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 2;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_GOOD;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_detect_hidden(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_HIDDEN) )
    {
        if (victim == ch)
          send_to_char("You are already as alert as you can be. \n\r",ch);
        else
          act("$N can already sense hidden lifeforms.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( victim, &af );
    send_to_char( "Your awareness improves.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
    {
        if (victim == ch)
          send_to_char("You can already see invisible.\n\r",ch);
        else
          act("$N can already see invisible things.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 2;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_detect_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) )
    {
        if (victim == ch)
          send_to_char("You can already sense magical auras.\n\r",ch);
        else
          act("$N can already detect magic.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 2;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_detect_neutral( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_NEUTRAL) )
    {
        if (victim == ch)
          send_to_char("You can already sense neutral.\n\r",ch);
        else
          act("$N can already detect neutral.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 2;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_NEUTRAL;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
	if ( obj->value[3] != 0 )
	    send_to_char( "You smell poisonous fumes.\n\r", ch );
	else
	    send_to_char( "It looks delicious.\n\r", ch );
	return TRUE;
    } else {
	send_to_char( "It doesn't look poisoned.\n\r", ch );
	return FALSE;
    }
}

bool spell_detect_terrain( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( !victim )
	return FALSE;

    if ( IS_AFFECTED( victim, AFF_DETECT_TERRAIN ) )
    {
	if ( victim == ch )
	    send_to_char( "You can already sense the terrain around you.\n\r", ch );
	else
	    act( "$N can already detect the terrain around $M.",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= perm_affect ? -1 : level / 2;
    af.modifier	= 0;
    af.location	= APPLY_NONE;
    af.bitvector= AFF_DETECT_TERRAIN;
    affect_to_char( victim, &af );

    send_to_char( "Your begin to examine the ground around you.\n\r", victim );

    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );

    return TRUE;
}

bool spell_deviant_point( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (IS_NPC(victim))
	return FALSE;

    victim->pcdata->deviant_points[0]++;
    victim->pcdata->deviant_points[1]++;
    rank_chart( victim, "deviant", victim->pcdata->deviant_points[1] );
    send_to_char( "{YYou've gained a {wD{Devian{wt {rP{Roin{rt{Y!{x\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_devotion( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if ( ch == victim )
	    send_to_char( "You are already fully devoted.\n\r", ch );
	else
	    act( "$N is already full of devotion.",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= level / 10;
    af.location	= APPLY_SAVES;
    af.modifier	= level / -5.1;
    af.bitvector= 0;
    affect_to_char( victim, &af );

    send_to_char( "Your eyes shine with devotion.\n\r", victim );
    act( "$N's eyes shine with devotion.",
	ch, NULL, victim, TO_ROOM, POS_RESTING );

    return TRUE;
}

bool spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( !IS_NPC(ch) && IS_EVIL(ch) )
	victim = ch;

    if ( IS_GOOD(victim) )
    {
	act( "$G protects $n.", victim, NULL, NULL, TO_ROOM, POS_RESTING);
	return TRUE;
    }

    if ( IS_NEUTRAL(victim) )
    {
	act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR,POS_RESTING);
	return TRUE;
    }

   dam = dice(level,4);

    if ( saves_spell( level, ch,victim,DAM_HOLY) )
	dam = dice(level,3);

    damage( ch, victim, dam, sn, DAM_HOLY ,TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_dispel_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( !IS_NPC(ch) && IS_GOOD(ch) )
        victim = ch;

    if ( IS_EVIL(victim) )
    {
        act( "$G protects $n.", victim, NULL, NULL, TO_ROOM, POS_RESTING);
        return TRUE;
    }

    if ( IS_NEUTRAL(victim) )
    {
        act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR,POS_RESTING);
        return TRUE;
    }

    dam = dice(level,4);

    if ( saves_spell( level, ch,victim,DAM_NEGATIVE) )
	dam = dice(level,3);

    damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_dispel_neutral( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( !IS_NPC( ch ) && IS_NEUTRAL( ch ) )
        victim = ch;

    else if ( IS_NEUTRAL( victim ) )
    {
        act( "$G protects $n.", victim, NULL, NULL, TO_ROOM, POS_RESTING );
        return TRUE;
    }

    dam = dice(level,4);

    if ( saves_spell( level, ch,victim,DAM_MAGIC ) )
	dam = dice(level,3);

    damage( ch, victim, dam, sn, DAM_MAGIC ,TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (saves_spell(level,ch,victim,DAM_OTHER))
    {
	send_to_char( "You feel a brief tingling sensation.\n\r",victim);
	send_to_char( "You failed.\n\r", ch);
	return TRUE;
    }

    if (spell_cancel(level,victim,TRUE))
    {
	if (ch == victim)
	    send_to_char("Ok.\n\r",ch);
	else
	    act("You magic weakens $N.",ch,NULL,victim,TO_CHAR,POS_RESTING);
    } else {
	if (ch == victim)
	    send_to_char("You failed.\n\r",ch);
	else
	    act("Your magic fails to weaken $N.",ch,NULL,victim,TO_CHAR,POS_RESTING);
    }
    return TRUE;
}

bool spell_divine_aura( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_DIVINE_AURA))
    {
        if(victim == ch)
            send_to_char("You are already surrounded by a {Yd{yi{Yv{yi{Yn{ye{x aura.\n\r", ch);
        else
            act("$N is already surrounded by a {Yd{yi{Yv{yi{Yn{ye{x aura.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : 5 + level / 2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_DIVINE_AURA;

   affect_to_char(victim, &af);
   send_to_char("You are surrounded by a {Yd{yi{Yv{yi{Yn{ye{x aura.\n\r", victim);
   act("$n is surrounded by a {Yd{yi{Yv{yi{Yn{ye{x aura.",victim, NULL,NULL,TO_ROOM,POS_RESTING);
   return TRUE;
}

bool spell_divine_blessing( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA*)vo;

    if ( is_affected(victim, sn)
    ||   is_affected(victim, gsn_bless)
    ||   is_affected(victim, gsn_infernal_offer) )
    {
	if ( ch == victim )
	    send_to_char("You are already divinely blessed.\n\r", victim);
	else
	    act("$N is already divinely blessed.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    if ( !IS_GOOD(victim) )
    {
	if ( ch == victim )
	    send_to_char("Your alignment is not good enough to ask such a favour.\n\r",ch);
	else
	    act("You pray for $N but the gods ignore your wish.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration	 = level / 4;
    af.location	 = APPLY_HITROLL;
    af.modifier	 = level / 4;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.location	 = APPLY_DAMROLL;
    affect_to_char(victim, &af);

    af.location	 = APPLY_SAVES;
    af.modifier	 = -level / 5;
    affect_to_char(victim, &af);

    send_to_char("{xYou are surrounded by a {Ys{yh{Wi{xm{Ym{ye{Wr{xi{Yn{yg{x aura.\n\r",victim);
    act("{x$n{x is surrounded by a {Ys{yh{Wi{xm{Ym{ye{Wr{xi{Yn{yg{x aura.",
	victim,NULL,NULL,TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_divine_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    victim->hit = UMIN( victim->hit + 600, victim->max_hit );

    update_pos( victim );

    send_to_char( "Your body tingles from divine intervention.\n\r", victim );

    if ( ch != victim )
    {
        send_to_char( "You call forth your god to bless them.\n\r", ch );
	show_condition( ch, victim, VALUE_HIT_POINT );
    }

    return TRUE;
}

bool spell_divinity( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_SHIELDED(victim, SHD_DIVINITY) )
    {
	if (victim == ch)
	  send_to_char("You are already in sanctuary.\n\r",ch);
	else
	  act("$N is already in sanctuary.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    if ( IS_SHIELDED( victim, SHD_SANCTUARY ) )
    {
	affect_strip( victim, gsn_sanctuary );
	REMOVE_BIT( victim->shielded_by, SHD_SANCTUARY );
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_DIVINITY;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a powerful white aura.", victim, NULL, NULL, TO_ROOM,POS_RESTING);
    send_to_char( "You are surrounded by a powerful white aura.\n\r", victim );
    return TRUE;
}

bool spell_downpour( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch;
    int dam;

    send_to_char("You call forth {Bt{bo{Br{be{Bn{bt{Bi{ba{Bl{x rains from the heavens!\n\r",ch);
    act("$n calls forth {Bt{bo{Br{be{Bn{bt{Bi{ba{Bl{x rains from the heavens!",
	ch,NULL,NULL,TO_ROOM,POS_RESTING);

    act("Your {Bt{bo{Br{be{Bn{bt{Bi{ba{Bl{x rains shower down on $N!",
	ch,NULL,victim,TO_CHAR,POS_RESTING);
    send_to_char("Your body is soaked from {Bt{bo{Br{be{Bn{bt{Bi{ba{Bl{x rains!\n\r",victim);

    dam = dice(level * 3,7);

    if ( saves_spell( level, ch, victim, DAM_WATER ) )
	dam = dice(level * 3,6);

    damage( ch, victim, dam, sn, DAM_WATER, TRUE, FALSE, NULL );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if ( vch != victim && !is_safe_spell(ch,vch,TRUE)
	&&   vch->damage_mod[DAM_WATER] > 0 )
	{
	    act("Your {Bt{bo{Br{be{Bn{bt{Bi{ba{Bl{x rains shower down on $N!",
		ch,NULL,vch,TO_CHAR,POS_RESTING);
	    send_to_char("Your body is soaked from {Bt{bo{Br{be{Bn{bt{Bi{ba{Bl{x rains!\n\r",vch);

	   dam = dice(level * 3,7);

	    if ( saves_spell( level, ch, vch, DAM_WATER ) )
		dam = dice(level * 3,6);

	    damage( ch, vch, dam, sn, DAM_WATER, TRUE, FALSE, NULL );
	}
    }
    return TRUE;
}

bool spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    send_to_char( "The earth trembles beneath your feet!\n\r", ch );
    act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM,POS_RESTING);

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
	    {
		if (IS_AFFECTED(vch,AFF_FLYING))
		    damage(ch,vch,0,sn,DAM_BASH,TRUE,FALSE,NULL);
		else
		    damage( ch,vch,(level * 6) + dice(level, 7), sn, DAM_BASH,TRUE,FALSE,NULL);
	    }
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char( "The earth trembles and shivers.\n\r", vch );
    }

    return TRUE;
}

bool spell_electrical_storm( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch;
    int dam;

    send_to_char("You call forth {Yf{yl{Ya{ys{Yh{ye{Ys{x of {Yl{yi{Yg{yh{Yt{x from the heavens!\n\r",ch);
    act("$n calls forth {Yf{yl{Ya{ys{Yh{ye{Ys{x of {Yl{yi{Yg{yh{Yt{x from the heavens!",
	ch,NULL,NULL,TO_ROOM,POS_RESTING);

    act("Your {Yf{yl{Ya{ys{Yh{ye{Ys{x of {Yl{yi{Yg{yh{Yt{x shower down at $N!",
	ch,NULL,victim,TO_CHAR,POS_RESTING);
    send_to_char("You are temporarily blinded by {Yf{yl{Ya{ys{Yh{ye{Ys{x of {Yl{yi{Yg{yh{Yt{x!\n\r",victim);

    dam = dice(level * 3,7);

    if ( saves_spell( level, ch, victim, DAM_LIGHT ) )
	dam = dice(level * 3,6);

    damage( ch, victim, dam, sn, DAM_LIGHT, TRUE, FALSE, NULL );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if ( vch != victim && !is_safe_spell(ch,vch,TRUE)
	&&   vch->damage_mod[DAM_LIGHT] > 0 )
	{
	    act("Your {Yf{yl{Ya{ys{Yh{ye{Ys{x of {Yl{yi{Yg{yh{Yt{x shower down at $N!",
		ch,NULL,vch,TO_CHAR,POS_RESTING);
	    send_to_char("You are temporarily blinded by {Yf{yl{Ya{ys{Yh{ye{Ys{x of {Yl{yi{Yg{yh{Yt{x!\n\r",vch);

	    dam = dice( level * 3,7 );

	    if ( saves_spell( level, ch, vch, DAM_LIGHT ) )
		dam = dice(level * 3,6);

	    damage( ch, vch, dam, sn, DAM_LIGHT, TRUE, FALSE, NULL );
	}
    }

    return TRUE;
}

bool spell_electrify( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if (obj->item_type != ITEM_WEAPON)
    {
	send_to_char("Electrify may only be used on weapons.\n\r",ch);
	return FALSE;
    }

    if (IS_WEAPON_STAT(obj,WEAPON_SHOCKING))
    {
	act("$p is already overcome with electricity.",
	    ch,obj,NULL,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    af.where	 = TO_WEAPON;
    af.type	 = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration	 = level / 4;
    af.location	 = 0;
    af.modifier	 = 0;
    af.bitvector = WEAPON_SHOCKING;
    affect_to_obj(obj,&af);

    act("$p is overcome with electricity.",ch,obj,NULL,TO_ALL,POS_RESTING);
    return TRUE;
}

bool spell_empower_potion( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *object;
    char buf[MAX_STRING_LENGTH];
    int new_sn;

    if ( ( new_sn = find_spell( ch, global_spell_arg ) ) < 0 )
    {
	send_to_char( "What spell do you wish to bind?\n\r", ch );
	return FALSE;
    }

    if ( skill_table[new_sn].spell_fun == spell_null )
    {
	send_to_char( "That is not a valid spell.\n\r", ch );
	return FALSE;
    }

    if ( IS_SET( skill_table[new_sn].flags, SKILL_NO_EMPOWER_POTION ) )
    {
	sprintf( buf, "The powers of {C%s {xare too complicated to bind!\n\r",
	    skill_table[new_sn].name );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( ch->mana < skill_table[new_sn].cost_mana )
    {
        send_to_char( "You do not have enough mana.\n\r", ch );
        return FALSE;
    }
    ch->mana -= skill_table[new_sn].cost_mana;

    if ( ( object = create_object( get_obj_index( OBJ_VNUM_POTION ) ) ) == NULL )
    {
	send_to_char( "Error: Missing blank potion vnum!\n\r", ch );
	return FALSE;
    }

    if ( ch->carry_number + get_obj_number( object ) > can_carry_n( ch ) )
    {
	send_to_char( "You can't carry that many items.\n\r", ch );
	extract_obj( object );
	return FALSE;
    }

    if ( get_carry_weight( ch ) + get_obj_weight( object ) > can_carry_w( ch ) )
    {
	send_to_char( "You can't carry that much weight.\n\r", ch );
	extract_obj( object );
	return FALSE;
    }

    object->value[0]	= level;
    object->value[1]	= new_sn;
    object->level	= ch->level-5;
    object->timer	= level;
    object->success	= get_skill( ch, new_sn );

    sprintf( buf, object->short_descr, skill_table[new_sn].name );
    free_string( object->short_descr );
    object->short_descr = str_dup( buf );

    sprintf( buf, object->name, skill_table[new_sn].name );
    free_string( object->name );
    object->name = str_dup( buf );

    sprintf( buf, object->description, skill_table[new_sn].name );
    free_string( object->description );
    object->description = str_dup( buf );

    act( "$n has conjured up $p!", ch, object, NULL, TO_ROOM, POS_RESTING );
    act( "You have conjured up $p!", ch, object, NULL, TO_CHAR, POS_RESTING );

    obj_to_char( object, ch );
    return TRUE;
}

bool spell_empower_scroll( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *object;
    char buf[MAX_STRING_LENGTH];
    int new_sn;

    if ( ( new_sn = find_spell( ch, global_spell_arg ) ) < 0 )
    {
	send_to_char( "What spell do you wish to bind?\n\r", ch );
	return FALSE;
    }

    if ( skill_table[new_sn].spell_fun == spell_null )
    {
	send_to_char( "That is not a valid spell.\n\r", ch );
	return FALSE;
    }

    if ( IS_SET( skill_table[new_sn].flags, SKILL_NO_EMPOWER_SCROLL ) )
    {
	sprintf( buf, "The powers of {C%s {xare too complicated to bind!\n\r",
	    skill_table[new_sn].name );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( ch->mana < skill_table[new_sn].cost_mana )
    {
        send_to_char( "You do not have enough mana.\n\r", ch );
        return FALSE;
    }

    ch->mana -= skill_table[new_sn].cost_mana;

    if ( ( object = create_object( get_obj_index( OBJ_VNUM_SCROLL ) ) ) == NULL )
    {
	send_to_char( "Error: Missing blank scroll vnum!\n\r", ch );
	return FALSE;
    }

    if ( ch->carry_number + get_obj_number( object ) > can_carry_n( ch ) )
    {
	send_to_char( "You can't carry that many items.\n\r", ch );
	extract_obj( object );
	return FALSE;
    }

    if ( get_carry_weight( ch ) + get_obj_weight( object ) > can_carry_w( ch ) )
    {
	send_to_char( "You can't carry that much weight.\n\r", ch );
	extract_obj( object );
	return FALSE;
    }

    object->value[0]	= level;
    object->value[1]	= new_sn;
    object->level	= ch->level-5;
    object->timer	= level;
    object->success	= get_skill( ch, new_sn );

    sprintf( buf, object->short_descr, skill_table[new_sn].name );
    free_string( object->short_descr );
    object->short_descr = str_dup( buf );

    sprintf( buf, object->name, skill_table[new_sn].name );
    free_string( object->name );
    object->name = str_dup( buf );

    sprintf( buf, object->description, skill_table[new_sn].name );
    free_string( object->description );
    object->description = str_dup( buf );

    act( "$n has conjured up $p!", ch, object, NULL, TO_ROOM, POS_RESTING );
    act( "You have conjured up $p!", ch, object, NULL, TO_CHAR, POS_RESTING );

    obj_to_char( object, ch );
    return TRUE;
}

bool spell_enchant_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
    bool ac_found = FALSE;
    int result, fail;
    int ac_bonus, added;

    if ( obj == NULL )
	return FALSE;

    if (obj->item_type != ITEM_ARMOR)
    {
	send_to_char("That isn't an armor.\n\r",ch);
	return FALSE;
    }

    if (obj->wear_loc != -1)
    {
	send_to_char("The item must be carried to be enchanted.\n\r",ch);
	return FALSE;
    }

    if ( IS_OBJ_STAT(obj, ITEM_AQUEST) || IS_OBJ_STAT(obj, ITEM_FORGED) )
    {
	send_to_char("That item may not be enchanted.\n\r",ch);
	return FALSE;
    }

    /* this means they have no bonus */
    ac_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    {
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_AC )
            {
	    	ac_bonus = paf->modifier;
		ac_found = TRUE;
	    	fail += (ac_bonus * ac_bonus);
 	    }

	    else  /* things get a little harder */
	    	fail += 20;
    	}
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_AC )
  	{
	    ac_bonus = paf->modifier;
	    ac_found = TRUE;
	    fail += (ac_bonus * ac_bonus);
	}

	else /* things get a little harder */
	    fail += 20;
    }

    /* apply other modifiers */
    fail -= level;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,85);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_CHAR,POS_RESTING);
	act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_ROOM,POS_RESTING);
	extract_obj(obj);
	return TRUE;
    }

    if (result < (fail / 3)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next;
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return TRUE;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_char("Nothing seemed to happen.\n\r",ch);
	return TRUE;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
	{
	    af_new = new_affect();

	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->where	= paf->where;
	    af_new->type 	= UMAX(0,paf->type);
	    af_new->level	= paf->level;
	    af_new->dur_type	= paf->dur_type;
	    af_new->duration	= paf->duration;
	    af_new->location	= paf->location;
	    af_new->modifier	= paf->modifier;
	    af_new->bitvector	= paf->bitvector;
	}
    }

    if (result <= (90 - level/5))  /* success! */
    {
	act("$p shimmers with a gold aura.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	act("$p shimmers with a gold aura.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = -5;
    }

    else  /* exceptional enchant */
    {
	act("$p glows a brillant gold!",ch,obj,NULL,TO_CHAR,POS_RESTING);
	act("$p glows a brillant gold!",ch,obj,NULL,TO_ROOM,POS_RESTING);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = -10;
    }

    /* now add the enchantments */
    if (ac_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_AC)
	    {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
	    }
	}
    }
    else /* add a new affect */
    {
 	paf = new_affect();

	paf->where	= TO_OBJECT;
	paf->type	= sn;
	paf->level	= level;
	paf->dur_type	= DUR_TICKS;
	paf->duration	= -1;
	paf->location	= APPLY_AC;
	paf->modifier	=  added;
	paf->bitvector  = 0;
    	paf->next	= obj->affected;
    	obj->affected	= paf;
    }
    return TRUE;
}

bool spell_enchant_weapon( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
    int result, fail;
    int hit_bonus, dam_bonus, added;
    bool hit_found = FALSE, dam_found = FALSE;

    if ( obj == NULL )
	return FALSE;

    if (obj->item_type != ITEM_WEAPON)
    {
	send_to_char("That isn't a weapon.\n\r",ch);
	return FALSE;
    }

    if (obj->wear_loc != -1)
    {
	send_to_char("The item must be carried to be enchanted.\n\r",ch);
	return FALSE;
    }

    if ( IS_OBJ_STAT(obj, ITEM_AQUEST) || IS_OBJ_STAT(obj, ITEM_FORGED) )
    {
	send_to_char("That item may not be enchanted.\n\r",ch);
	return FALSE;
    }

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_HITROLL )
            {
	    	hit_bonus = paf->modifier;
		hit_found = TRUE;
	    	fail += 2 * (hit_bonus * hit_bonus);
 	    }

	    else if (paf->location == APPLY_DAMROLL )
	    {
	    	dam_bonus = paf->modifier;
		dam_found = TRUE;
	    	fail += (dam_bonus * dam_bonus);
	    }

	    else  /* things get a little harder */
	    	fail += 25;
    	}

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_HITROLL )
  	{
	    hit_bonus = paf->modifier;
	    hit_found = TRUE;
	    fail += 2 * (hit_bonus * hit_bonus);
	}

	else if (paf->location == APPLY_DAMROLL )
  	{
	    dam_bonus = paf->modifier;
	    dam_found = TRUE;
	    fail += (dam_bonus * dam_bonus);
	}

	else /* things get a little harder */
	    fail += 25;
    }

    /* apply other modifiers */
    fail -= 3 * level/2;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,95);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$p shivers violently and explodes!",ch,obj,NULL,TO_CHAR,POS_RESTING);
	act("$p shivers violently and explodeds!",ch,obj,NULL,TO_ROOM,POS_RESTING);
	extract_obj(obj);
	return TRUE;
    }

    if (result < (fail / 2)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next;
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return TRUE;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_char("Nothing seemed to happen.\n\r",ch);
	return TRUE;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
	{
	    af_new = new_affect();

	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->where	= paf->where;
	    af_new->type 	= UMAX(0,paf->type);
	    af_new->level	= paf->level;
	    af_new->dur_type	= paf->dur_type;
	    af_new->duration	= paf->duration;
	    af_new->location	= paf->location;
	    af_new->modifier	= paf->modifier;
	    af_new->bitvector	= paf->bitvector;
	}
    }

    if (result <= (100 - level/5))  /* success! */
    {
	act("$p glows blue.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	act("$p glows blue.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = 2;
    }

    else  /* exceptional enchant */
    {
	act("$p glows a brillant blue!",ch,obj,NULL,TO_CHAR,POS_RESTING);
	act("$p glows a brillant blue!",ch,obj,NULL,TO_ROOM,POS_RESTING);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = 4;
    }

    /* now add the enchantments */
    if (dam_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_DAMROLL)
	    {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
		if (paf->modifier > 4)
		    SET_BIT(obj->extra_flags,ITEM_HUM);
	    }
	}
    }
    else /* add a new affect */
    {
	paf = new_affect();

	paf->where	= TO_OBJECT;
	paf->type	= sn;
	paf->level	= level;
	paf->dur_type	= DUR_TICKS;
	paf->duration	= -1;
	paf->location	= APPLY_DAMROLL;
	paf->modifier	= added;
	paf->bitvector  = 0;
    	paf->next	= obj->affected;
    	obj->affected	= paf;
    }

    if (hit_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
            if ( paf->location == APPLY_HITROLL)
            {
		paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX(paf->level,level);
                if (paf->modifier > 4)
                    SET_BIT(obj->extra_flags,ITEM_HUM);
            }
	}
    }
    else /* add a new affect */
    {
        paf = new_affect();

        paf->type       = sn;
        paf->level      = level;
	paf->dur_type	= DUR_TICKS;
        paf->duration   = -1;
        paf->location   = APPLY_HITROLL;
        paf->modifier   = added;
        paf->bitvector  = 0;
        paf->next       = obj->affected;
        obj->affected   = paf;
    }
    return TRUE;
}

bool spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( victim != ch )
    {
        ch->alignment = URANGE(-1000, ch->alignment - 50,1000);
	if ( ch->pet != NULL )
	    ch->pet->alignment = ch->alignment;
    }

    level = check_curse_of_ages( ch, level );

    if ( saves_spell( level-5, ch, victim, DAM_NEGATIVE ) )
    {
	send_to_char("You feel a momentary chill.\n\r",victim);
	send_to_char("You failed to drain any life.\n\r",ch);
	return TRUE;
    }

    if ( victim->level <= 2 )
    {
	dam		 = ch->hit + 1;
    } else {
	victim->mana	*= .5;
	victim->move	*= .5;
	dam = dice(level,4);
	if ( ch != victim )
	    ch->hit	+= dam;
    }

    send_to_char("You feel your life slipping away!\n\r",victim);
    send_to_char("Wow....what a rush!\n\r",ch);
    damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, FALSE, NULL );

    if ( ch->in_room != NULL
    &&   !IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    &&   !IS_SET(ch->in_room->room_flags, ROOM_WAR) )
    {
	OBJ_DATA *obj, *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	    {
		act( "{cYou are {Wzapped{c by $p.{x",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
		act( "$n is {Wzapped{x by $p.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );
		obj_from_char( obj );

		if ( IS_OBJ_STAT(obj, ITEM_NODROP)
		||   IS_OBJ_STAT(obj, ITEM_INVENTORY)
		||   IS_OBJ_STAT(obj, ITEM_AQUEST)
		||   IS_OBJ_STAT(obj, ITEM_FORGED) )
		{
		    act( "{cA magical aura draws $p {cto you.{x",
			ch, obj, NULL, TO_CHAR, POS_DEAD );
		    act( "A magical aura draws $p to $n.",
			ch, obj, NULL, TO_ROOM, POS_RESTING );
		    obj_to_char( obj, ch );
		} else {
		    set_obj_sockets( ch, obj );
		    set_arena_obj( ch, obj );
		    obj_to_room( obj, ch->in_room );
		}
	    }
	}
    }
    return TRUE;
}

bool spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( !victim )
	return FALSE;

    if ( IS_AFFECTED( victim, AFF_FAERIE_FIRE )
    ||   is_affected( victim, sn ) )
    {
	act( "$N is already glowing.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where     = TO_DAM_MODS;
    af.type      = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 25;
    af.location  = DAM_ALL;
    af.modifier  = 10 + (level / 10);
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char( victim, &af );

    send_to_char( "You are surrounded by a pink outline.\n\r", victim );
    act( "$n is surrounded by a pink outline.",
	victim, NULL, NULL, TO_ROOM, POS_RESTING );
    return TRUE;
}

bool spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *ich;
    OBJ_DATA *fog;

    for ( fog = ch->in_room->contents; fog != NULL; fog = fog->next_content )
    {
        if ( fog->pIndexData->vnum == OBJ_VNUM_FOG )
	{
	    send_to_char( "This room is already affected by faerie fog!\n\r", ch );
	    return FALSE;
	}
    }

    act( "$n conjures a cloud of purple smoke.",
	ch, NULL, NULL, TO_ROOM, POS_RESTING );
    send_to_char( "You conjure a cloud of purple smoke.\n\r", ch );

    if ( ch->in_room != NULL )
    {
	if ( ( fog = create_object( get_obj_index( OBJ_VNUM_FOG ) ) ) == NULL )
	{
	    send_to_char( "Uhh, NULL fog vnum!\n\r", ch );
	    return FALSE;
	}

	fog->timer = level / 20;

	set_arena_obj( ch, fog );
	obj_to_room( fog, ch->in_room );
    }

    for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
	if ( ich->invis_level > 0
	||   ich == ch || saves_spell( 2 * level / 3, ch, ich, DAM_OTHER ) )
	    continue;

	affect_strip ( ich, gsn_invis			);
	affect_strip ( ich, gsn_mass_invis		);
	affect_strip ( ich, gsn_sneak			);
	affect_strip ( ich, gsn_obfuscate		);
	affect_strip ( ich, gsn_camouflage		);
	affect_strip ( ich, gsn_forest_meld		);
	REMOVE_BIT   ( ich->affected_by, AFF_HIDE	);
	REMOVE_BIT   ( ich->shielded_by, SHD_INVISIBLE	);
	REMOVE_BIT   ( ich->affected_by, AFF_SNEAK	);
	act( "$n is revealed!", ich, NULL, NULL, TO_ROOM,POS_RESTING);
	send_to_char( "You are revealed!\n\r", ich );
    }

    return TRUE;
}

bool spell_faith( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if ( ch == victim )
	    send_to_char( "Your faith is already strong.\n\r", ch );
	else
	    send_to_char( "Their faith is already strong.\n\r", ch );
	return FALSE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type = DUR_TICKS;
    af.duration	= level/5;
    af.location	= APPLY_DAMROLL;
    af.modifier	= level/2;
    af.bitvector= 0;
    affect_to_char( victim, &af );

    af.location	= APPLY_HITROLL;
    af.modifier	= level/2;
    affect_to_char( victim, &af );

    af.where    = TO_DAM_MODS;
    af.location = DAM_NEGATIVE;
    af.modifier = -level/5;
    affect_to_char( victim, &af );

    if ( ch != victim )
	send_to_char( "OK.\n\r", ch );
    send_to_char( "Your faith grows strong.\n\r", victim );

    return TRUE;
}

bool spell_farsight(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FARSIGHT) )
    {
        if (victim == ch)
          send_to_char("Your eyes are already as good as they get.\n\r",ch);
        else
          act("$N can see just fine.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FARSIGHT;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes jump into focus.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level * 3,4);

    if ( saves_spell( level,ch, victim, DAM_FIRE) )
	dam = dice(level * 3,3);

    damage( ch, victim, dam, sn, DAM_FIRE ,TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_fireproof(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
    {
        act("$p is already protected from burning.",ch,obj,NULL,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_OBJECT;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = number_range(level/2,level/3);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_BURN_PROOF;

    affect_to_obj(obj,&af);

    act("You protect $p from fire.",ch,obj,NULL,TO_CHAR,POS_RESTING);
    act("$p is surrounded by a protective aura.",ch,obj,NULL,TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_fireshield(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_FIRE))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded by a {Rfirey{x shield.\r\n", ch);
	else
	    act("$N is already surrounded by a {Rfiery{x shield.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : 5 + level / 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_FIRE;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by a {Rfiery{x shield.\n\r", victim);
    act("$n is surrounded by a {Rfiery{x shield.",victim, NULL,NULL,TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch, *vch_next;
    int dam, hpch, hp_dam, dice_dam;

    act( "$n breathes forth a cone of fire.",
	ch, NULL, NULL, TO_ROOM, POS_RESTING );
    act( "You breath forth a cone of fire.",
	ch, NULL, NULL, TO_CHAR, POS_RESTING );

    hpch	= UMAX( 10, ch->max_mana );
    hp_dam	= number_range( hpch / 9 + 1, hpch / 5 );
    dice_dam	= dice( 2 * level, 45 );
    dam		= UMAX( hp_dam + dice_dam / 10, dice_dam + hp_dam / 10 );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	if ( is_safe_spell( ch, vch, TRUE ) )
	    continue;

	if ( saves_spell( level, ch, vch, DAM_FIRE ) )
	{
	    damage( ch, vch, 3 * dam / 4, sn, DAM_FIRE, TRUE, FALSE, NULL );
	    fire_effect( ch, vch, level / 2, dam / 4, TARGET_CHAR );
	} else {
	    damage( ch, vch, dam, sn, DAM_FIRE, TRUE, FALSE, NULL );
	    fire_effect( ch, vch, level, dam, TARGET_CHAR );
	}
    }

    fire_effect( ch, ch->in_room, level, dam / 2, TARGET_ROOM );

    return TRUE;
}

bool spell_fire_storm( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch;
    int dam;

    send_to_char("You call forth {Rf{rl{Rami{rn{Rg{x showers from the heavens!\n\r",ch);
    act("$n calls forth {Rf{rl{Rami{rn{Rg{x showers from the heavens!",
	ch,NULL,NULL,TO_ROOM,POS_RESTING);

    act("Your {Rf{rl{Rami{rn{Rg{x showers rain down on $N!",
	ch,NULL,victim,TO_CHAR,POS_RESTING);
    send_to_char("Your skin burns from {Rf{rir{Re{x!\n\r",victim);

    dam = dice(level * 3,7);

    if ( saves_spell( level, ch, victim, DAM_FIRE ) )
	dam = dice(level * 3,6);

    damage( ch, victim, dam, sn, DAM_FIRE, TRUE, FALSE, NULL );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if ( vch != victim && !is_safe_spell(ch,vch,TRUE)
	&&   vch->damage_mod[DAM_FIRE] > 0 )
	{
	    act("Your {Rf{rl{Rami{rn{Rg{x showers rain down on $N!",
		ch,NULL,vch,TO_CHAR,POS_RESTING);
	    send_to_char("Your skin burns from {Rf{rir{Re{x!\n\r",vch);

	    dam = dice(level * 3,7);

	    if ( saves_spell( level, ch, vch, DAM_FIRE ) )
		dam = dice(level * 3,6);

	    damage( ch, vch, dam, sn, DAM_FIRE, TRUE, FALSE, NULL );
	}
    }
    return TRUE;
}

bool spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level * 3,4);

    if ( saves_spell( level, ch,victim,DAM_FIRE) )
	dam = dice(level * 3,3);

    damage( ch, victim, dam, sn, DAM_FIRE ,TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_floating_disc( int sn, int level,CHAR_DATA *ch,void *vo,int target )
{
    AFFECT_DATA af;
    OBJ_DATA *disc, *floating;

    floating = get_eq_char(ch,WEAR_FLOAT);
    if (floating != NULL && IS_OBJ_STAT(floating,ITEM_NOREMOVE))
    {
	act("You can't remove $p.",ch,floating,NULL,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    disc = create_object(get_obj_index(OBJ_VNUM_DISC));

    if ( disc == NULL )
	return FALSE;

    disc->value[0]	= level * 10; /* 10 pounds per level capacity */
    disc->value[3]	= level * 5; /* 5 pounds per level max per item */
    disc->level		= level;
    disc->timer		= level / 4;
    af.where		= TO_OBJECT;
    af.type		= 0;
    af.level		= level;
    af.dur_type		= DUR_TICKS;
    af.duration		= -1;
    af.location		= APPLY_HITROLL;
    af.modifier		= level/6;
    af.bitvector	= 0;
    affect_to_obj(disc,&af);

    af.location		= APPLY_DAMROLL;
    affect_to_obj(disc,&af);

    af.location		= APPLY_HIT;
    af.modifier		= level;
    affect_to_obj(disc,&af);

    af.location		= APPLY_MANA;
    affect_to_obj(disc,&af);

    af.where		= TO_OBJECT;
    af.location		= APPLY_SAVES;
    af.modifier		= ch->level / -20;
    affect_to_obj(disc,&af);

    act("$n has created $p.",ch,disc,NULL,TO_ROOM,POS_RESTING);
    act("You create $p.\n\r",ch,disc,NULL,TO_CHAR,POS_RESTING);
    obj_to_char(disc,ch);
    wear_obj(ch,disc,TRUE,TRUE);
    return TRUE;
}

bool spell_fly( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FLYING) )
    {
	if (victim == ch)
	  send_to_char("You are already airborne.\n\r",ch);
	else
	  act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char( "Your feet rise off the ground.\n\r", victim );
    act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_frenzy( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim,sn))
    {
	if (victim == ch)
	  send_to_char("You are already in a frenzy.\n\r",ch);
	else
	  act("$N is already in a frenzy.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    if (is_affected(victim,skill_lookup("calm")))
    {
	if (victim == ch)
	  send_to_char("Why don't you just relax for a while?\n\r",ch);
	else
	  act("$N doesn't look like $e wants to fight anymore.",
	      ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    if ((IS_GOOD(ch) && !IS_GOOD(victim)) ||
	(IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) ||
	(IS_EVIL(ch) && !IS_EVIL(victim))
       )
    {
	act("Your god doesn't seem to like $N",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration	 = level / 3;
    af.modifier  = level / 3;
    af.bitvector = 0;

    af.location  = APPLY_HITROLL;
    affect_to_char(victim,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_char(victim,&af);

    af.modifier  = 10 * (level / 12);
    af.location  = APPLY_AC;
    affect_to_char(victim,&af);

    send_to_char("You are filled with holy wrath!\n\r",victim);
    act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch, *vch_next;
    int dam, hpch, hp_dam, dice_dam;

    act( "$n breathes out a freezing cone of frost!",
	ch, NULL, NULL, TO_ROOM, POS_RESTING );
    act( "You breath out a cone of frost.",
	ch, NULL, NULL, TO_CHAR, POS_RESTING );

    hpch	= UMAX( 10, ch->max_mana );
    hp_dam	= number_range( hpch / 9 + 1, hpch / 5 );
    dice_dam	= dice( 2 * level, 45 );
    dam		= UMAX( hp_dam + dice_dam / 10, dice_dam + hp_dam / 10 );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	if ( is_safe_spell( ch, vch, TRUE ) )
	    continue;

	if ( saves_spell( level, ch, vch, DAM_COLD ) )
	{
	    damage( ch, vch, 3 * dam / 4, sn, DAM_COLD, TRUE, FALSE, NULL );
	    cold_effect( ch, vch, level / 2, dam / 4, TARGET_CHAR );
	} else {
	    damage( ch, vch, dam, sn, DAM_COLD, TRUE, FALSE, NULL );
	    cold_effect( ch, vch, level, dam, TARGET_CHAR );
	}
    }

    cold_effect( ch, ch->in_room, level, dam / 2, TARGET_ROOM );

    return TRUE;
}

bool spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch, *vch_next;
    int dam, hpch, hp_dam, dice_dam;

    act( "$n breathes out a cloud of poisonous gas!",
	ch, NULL, NULL, TO_ROOM, POS_RESTING );
    act( "You breath out a cloud of poisonous gas.",
	ch, NULL, NULL, TO_CHAR, POS_RESTING );

    hpch	= UMAX( 10, ch->max_mana );
    hp_dam	= number_range( hpch / 9 + 1, hpch / 5 );
    dice_dam	= dice( 2 * level, 45 );
    dam		= UMAX( hp_dam + dice_dam / 10, dice_dam + hp_dam / 10 );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	if ( is_safe_spell( ch, vch, TRUE ) )
	    continue;

	if ( saves_spell( level, ch, vch, DAM_POISON ) )
	{
	    damage( ch, vch, 3 * dam / 4, sn, DAM_POISON, TRUE, FALSE, NULL );
	    poison_effect( ch, vch, level / 2, dam / 4, TARGET_CHAR );
	} else {
	    damage( ch, vch, dam, sn, DAM_POISON, TRUE, FALSE, NULL );
	    poison_effect( ch, vch, level, dam, TARGET_CHAR );
	}
    }

    poison_effect( ch, ch->in_room, level, dam, TARGET_ROOM );

    return TRUE;
}

bool spell_gate( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    bool gate_pet;

    if ( ( victim = get_char_world( ch, global_spell_arg ) ) == NULL
    ||   (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON))
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||	 IS_SET(victim->in_room->room_flags, ROOM_ARENA)
    ||   IS_SET(victim->in_room->room_flags, ROOM_WAR)
    ||   IS_SET(ch->in_room->room_flags, ROOM_WAR)
    ||   IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level > LEVEL_HERO)
    ||   (IS_NPC(victim) && victim->in_room->area->clan != 0)
    ||   (IS_NPC(victim) && saves_spell( level, ch,victim,DAM_OTHER) ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return FALSE;
    }

    if ( IS_SET(victim->in_room->room_flags, ROOM_NO_GATE) )
    {
	send_to_char("A magical force blocks your teleportation to that room.\n\r",ch);
	return FALSE;
    }

    if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
	gate_pet = TRUE;
    else
	gate_pet = FALSE;

    act("$n steps through a gate and vanishes.",ch,NULL,NULL,TO_ROOM,POS_RESTING);
    send_to_char("You step through a gate and vanish.\n\r",ch);
    char_from_room(ch);
    char_to_room(ch,victim->in_room);

    act("$n has arrived through a gate.",ch,NULL,NULL,TO_ROOM,POS_RESTING);
    do_look(ch,"auto");

    if (gate_pet)
    {
	act("$n steps through a gate and vanishes.",ch->pet,NULL,NULL,TO_ROOM,POS_RESTING);
	send_to_char("You step through a gate and vanish.\n\r",ch->pet);
	char_from_room(ch->pet);
	char_to_room(ch->pet,victim->in_room);
	act("$n has arrived through a gate.",ch->pet,NULL,NULL,TO_ROOM,POS_RESTING);
	do_look(ch->pet,"auto");
    }
    return TRUE;
}

bool spell_general_purpose(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 25, level * 3 );

    if ( saves_spell( level, ch,victim, DAM_PIERCE) )
        dam /= 2;

    damage( ch, victim, dam, sn, DAM_PIERCE ,TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_giant_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already as strong as you can get!\n\r",ch);
	else
	  act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    if (IS_AFFECTED(victim,AFF_WEAKEN))
    {
	if (!check_dispel(level,victim,gsn_weaken))
	{
	    if (victim != ch)
	        send_to_char("Spell failed.\n\r",ch);
	    send_to_char("You are still weak and frail.\n\r",victim);
	    return TRUE;
	}
        return TRUE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = level / 2;
    af.location  = APPLY_STR;
    af.modifier  = UMAX(1,level / 30);
    af.bitvector = AFF_GIANT_STRENGTH;
    affect_to_char( victim, &af );
    send_to_char( "Your muscles surge with heightened power!\n\r", victim );
    act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_glacial_aura( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if (obj->item_type != ITEM_WEAPON)
    {
	send_to_char("Glacial aura may only be used on weapons.\n\r",ch);
	return FALSE;
    }

    if (IS_WEAPON_STAT(obj,WEAPON_FROST))
    {
	act("$p is already embedded with ice.",
	    ch,obj,NULL,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    af.where	 = TO_WEAPON;
    af.type	 = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration	 = level / 4;
    af.location	 = 0;
    af.modifier	 = 0;
    af.bitvector = WEAPON_FROST;
    affect_to_obj(obj,&af);

    act("$p is embedded with {Ci{cc{Ce{x.",ch,obj,NULL,TO_ALL,POS_RESTING);
    return TRUE;
}

bool spell_growth( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *obj, *obj_next;

    if ( victim->size == SIZE_GIANT )
    {
	if ( victim == ch )
	    send_to_char("You can't get any larger.\n\r",ch);
	else
	    act("$N can't get any larger.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    if ( is_affected( victim, sn ) )
    {
	if ( victim == ch )
	    send_to_char("Your size has already been increased.\n\r",ch);
	else
	    act("$N's size has already been increased.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    if ( is_affected(victim, skill_lookup("shrink")) )
    {
	if ( !check_dispel(level,victim,skill_lookup("shrink")) )
	{
	    if ( victim != ch )
		send_to_char("Spell failed.\n\r",ch);
	    send_to_char("Your feel larger for an instant.\n\r",victim);
	    return TRUE;
	}

	act("$n looks more like $s normal self.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
	return TRUE;
    }

    af.where	 = TO_AFFECTS;
    af.level	 = level;
    af.type	 = sn;
    af.dur_type  = DUR_TICKS;
    af.duration	 = level / 20;
    af.location	 = APPLY_SIZE;
    af.modifier	 = 1;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    send_to_char("Your head jerks in pain as your body grows.\n\r",victim);
    act("$n's head jerks in pain as $s body grows.",
	victim,NULL,NULL,TO_ROOM,POS_RESTING);

    for ( obj = victim->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;

	if ( obj->wear_loc != WEAR_NONE )
	{
	    if ( obj->size != SIZE_NONE && obj->size < victim->size-1 )
	    {
		act("$p appears to shrink, and squeezes off!",
		    victim,obj,NULL,TO_CHAR,POS_DEAD);
		obj_from_char(obj);

		if ( IS_OBJ_STAT(obj, ITEM_NODROP)
		||   IS_OBJ_STAT(obj, ITEM_INVENTORY)
		||   IS_OBJ_STAT(obj, ITEM_AQUEST)
		||   IS_OBJ_STAT(obj, ITEM_FORGED) )
		    obj_to_char(obj,victim);
		else
		{
		    set_obj_sockets( victim, obj );
		    set_arena_obj( victim, obj );
		    obj_to_room(obj,victim->in_room);
		}
	    }
	}
    }

    return TRUE;
}

bool spell_harm( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level,11);

    if ( saves_spell( level,ch, victim,DAM_MAGIC) )
	dam = dice(level,10);

    damage( ch, victim, dam, sn, DAM_MAGIC ,TRUE, FALSE, NULL);
    return TRUE;
}

bool spell_haste( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE) )
    {
	if (victim == ch)
	  send_to_char("You can't move any faster!\n\r",ch);
 	else
	  act("$N is already moving as fast as $E can.",
	      ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    if (is_affected(victim,gsn_slow))
    {
	if (!check_dispel(level,victim,gsn_slow))
	{
	    if (victim != ch)
	        send_to_char("Spell failed.\n\r",ch);
	    send_to_char("You feel momentarily faster.\n\r",victim);
	    return TRUE;
	}
        return TRUE;
    }

    else if (IS_AFFECTED(victim,AFF_SLOW))
    {
	if (!saves_dispel(level, victim->level,-1))
	{
	    REMOVE_BIT(victim->affected_by,AFF_SLOW);
	    act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
	    return TRUE;

	}
        act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
        return TRUE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : ch == victim ? level / 2 : level / 4;
    af.location  = APPLY_DEX;
    af.modifier  = UMAX(1,level / 30);
    af.bitvector = AFF_HASTE;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself moving more quickly.\n\r", victim );
    act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_heal( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->hit = UMIN( victim->hit + 250, victim->max_hit );
    update_pos( victim );
    send_to_char( "A warm feeling fills your body.\n\r", victim );

    if ( ch != victim )
    {
	send_to_char( "Ok.\n\r", ch );
	show_condition( ch, victim, VALUE_HIT_POINT );
    }

    return TRUE;
}

bool spell_heat_metal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose, *obj_next;
    int dam = 0;
    bool fail = TRUE;

   if ( !saves_spell(level,ch,victim,DAM_FIRE)
   &&   !IS_IMMORTAL(victim) )
   {
        for ( obj_lose = victim->carrying;
	      obj_lose != NULL;
	      obj_lose = obj_next)
        {
	    obj_next = obj_lose->next_content;
            if ( number_range(1,2 * level) > obj_lose->level
	    &&   !saves_spell(level,ch,victim,DAM_FIRE)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_NONMETAL)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_NODROP)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_AQUEST)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_FORGED)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_BURN_PROOF))
            {
		fire_effect(ch,obj_lose,level,dam,TARGET_OBJ);

                switch ( obj_lose->item_type )
                {
               	case ITEM_ARMOR:
		if (obj_lose->wear_loc != -1) /* remove the item */
		{
		    if (can_drop_obj(victim,obj_lose)
		    &&  (obj_lose->weight / 10) <
			number_range(1,2 * get_curr_stat(victim,STAT_DEX))
		    &&  remove_obj( victim, obj_lose->wear_loc, TRUE ))
		    {
                        if ( is_pkill(victim) )
                        {
                            act("$n yelps and throws $p to the ground!",
			        victim,obj_lose,NULL,TO_ROOM,POS_RESTING);
		            act("You remove and drop $p before it burns you.",
			        victim,obj_lose,NULL,TO_CHAR,POS_RESTING);
			    dam += (number_range(1,obj_lose->level) / 3);
                            obj_from_char(obj_lose);
			    set_obj_sockets( victim, obj_lose );
			    set_arena_obj( victim, obj_lose );
                            obj_to_room(obj_lose, victim->in_room);
			} else {
			    act("$n yelps and removes $p!",
			        victim,obj_lose,NULL,TO_ROOM,POS_RESTING);
		            act("You remove $p before it burns you.",
			        victim,obj_lose,NULL,TO_CHAR,POS_RESTING);
			    dam += (number_range(1,obj_lose->level));
			}
                        fail = FALSE;
                    }
		    else /* stuck on the body! ouch! */
		    {
			act("Your skin is seared by $p!",
			    victim,obj_lose,NULL,TO_CHAR,POS_RESTING);
			dam += (number_range(1,obj_lose->level));
			fail = FALSE;
		    }

		}
		else /* drop it if we can */
		{
		    if (can_drop_obj(victim,obj_lose) && is_pkill(victim))
		    {
                        act("$n yelps and throws $p to the ground!",
                            victim,obj_lose,NULL,TO_ROOM,POS_RESTING);
                        act("You drop $p before it burns you.",
                            victim,obj_lose,NULL,TO_CHAR,POS_RESTING);
                        dam += (number_range(1,obj_lose->level) / 6);
                        obj_from_char(obj_lose);
                        set_obj_sockets( victim, obj_lose );
                        set_arena_obj( victim, obj_lose );
                        obj_to_room(obj_lose, victim->in_room);
			fail = FALSE;
                    }
		    else /* cannot drop */
		    {
                        act("Your skin is seared by $p!",
                            victim,obj_lose,NULL,TO_CHAR,POS_RESTING);
                        dam += (number_range(1,obj_lose->level) / 2);
			fail = FALSE;
                    }
		}
                break;
                case ITEM_WEAPON:
		if (obj_lose->wear_loc != -1) /* try to drop it */
		{
		    if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
			continue;

		    if (can_drop_obj(victim,obj_lose)
		    &&  remove_obj(victim,obj_lose->wear_loc,TRUE))
		    {
                        if ( is_pkill(victim) )
                        {
                            act("$n is burned by $p, and throws it to the ground.",
			        victim,obj_lose,NULL,TO_ROOM,POS_RESTING);
			    send_to_char(
			        "You throw your red-hot weapon to the ground!\n\r",
			        victim);
			    dam += 1;
                            obj_from_char(obj_lose);
                            set_obj_sockets( victim, obj_lose );
                            set_arena_obj( victim, obj_lose );
                            obj_to_room(obj_lose, victim->in_room);
                        } else {
                            act("$n is burned by $p, and removes it.",
			        victim,obj_lose,NULL,TO_ROOM,POS_RESTING);
		            act("You remove your red-hot weapon.",
			        victim,obj_lose,NULL,TO_CHAR,POS_RESTING);
			    dam += 2;
                        }
			fail = FALSE;
		    }
		    else /* YOWCH! */
		    {
			send_to_char("Your weapon sears your flesh!\n\r",
			    victim);
			dam += number_range(1,obj_lose->level);
			fail = FALSE;
		    }
		}
                else /* drop it if we can */
                {
                    if (can_drop_obj(victim,obj_lose)
                    &&  is_pkill(victim) )
                    {
                        act("$n throws a burning hot $p to the ground!",
                            victim,obj_lose,NULL,TO_ROOM,POS_RESTING);
                        act("You and drop $p before it burns you.",
                            victim,obj_lose,NULL,TO_CHAR,POS_RESTING);
                        dam += (number_range(1,obj_lose->level) / 6);
                        obj_from_char(obj_lose);
                        set_obj_sockets( victim, obj_lose );
                        set_arena_obj( victim, obj_lose );
                        obj_to_room(obj_lose, victim->in_room);
                        fail = FALSE;
                    }
                    else /* cannot drop */
                    {
                        act("Your skin is seared by $p!",
                            victim,obj_lose,NULL,TO_CHAR,POS_RESTING);
                        dam += (number_range(1,obj_lose->level) / 2);
                        fail = FALSE;
                    }
                }
                break;
		}
	    }
	}
    }
    if (fail)
    {
        send_to_char("Your spell had no effect.\n\r", ch);
	send_to_char("You feel momentarily warmer.\n\r",victim);
    }
    else /* damage! */
    {
	if (saves_spell(level,ch,victim,DAM_FIRE))
	    dam = 2 * dam / 3;
	damage(ch,victim,dam,sn,DAM_FIRE,TRUE,FALSE,NULL);
    }

    return TRUE;
}

bool spell_heresy( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( victim == ch )
    {
	send_to_char( "You can not mark yourself for heresy.\n\r", ch );
	return FALSE;
    }

    if ( is_affected( victim, sn ) )
    {
	act( "$N is already marked for heresy.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= 3;
    af.location	= APPLY_HITROLL;
    af.modifier	= -20;
    af.bitvector= 0;
    affect_to_char( victim, &af );

    af.location = APPLY_DAMROLL;
    affect_to_char( victim, &af );

    af.location = APPLY_SAVES;
    af.modifier = 12;
    affect_to_char( victim, &af );

    send_to_char( "You've been marked for heresy.\n\r", victim );
    act( "$n has been marked for heresy.",
	victim, NULL, NULL, TO_ROOM, POS_RESTING );
    return TRUE;
}

bool spell_high_explosive(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level,7);
    if ( saves_spell( level, ch,victim, DAM_PIERCE) )
        dam = dice(level,6);

    damage( ch, victim, dam, sn, DAM_PIERCE ,TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_holy_word(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    act("$n utters a word of divine power!",ch,NULL,NULL,TO_ROOM,POS_RESTING);
    send_to_char("You utter a word of divine power.\n\r",ch);

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;

	if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
	    (IS_EVIL(ch) && IS_EVIL(vch)) ||
	    (IS_NEUTRAL(ch) && IS_NEUTRAL(vch)) )
	{
 	  send_to_char("You feel more powerful.\n\r",vch);
	  spell_frenzy(gsn_frenzy,level,ch,(void *) vch,TARGET_CHAR);
	  spell_bless(gsn_bless,level,ch,(void *) vch,TARGET_CHAR);
	}

	else if ((IS_GOOD(ch) && IS_EVIL(vch)) ||
		 (IS_EVIL(ch) && IS_GOOD(vch)) )
	{
	  if (!is_safe_spell(ch,vch,TRUE))
	  {
            spell_curse(gsn_curse,level,ch,(void *) vch,TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);
	    dam = dice(level,6);
	    damage(ch,vch,dam,sn,DAM_ENERGY,TRUE,FALSE,NULL);
	  }
	}

        else if (IS_NEUTRAL(ch))
	{
	  if (!is_safe_spell(ch,vch,TRUE))
	  {
            spell_curse(gsn_curse,level/2,ch,(void *) vch,TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);
	    dam = dice(level,5);
	    damage(ch,vch,dam,sn,DAM_ENERGY,TRUE,FALSE,NULL);
   	  }
	}
    }

    send_to_char("You feel drained.\n\r",ch);
    ch->move = 0;
    ch->hit /= 2;
    return TRUE;
}

bool spell_iceshield( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_ICE))
    {
	if(victim == ch)
	    send_to_char("You are already surrounded by an {Cicy{x shield.\n\r", ch);
	else
	    act("$N is already surrounded by an {Cicy{x shield.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : 5 + level / 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_ICE;

   affect_to_char(victim, &af);
   send_to_char("You are surrounded by an {Cicy{x shield.\n\r", victim);
   act("$n is surrounded by an {Cicy{x shield.",victim, NULL,NULL,TO_ROOM,POS_RESTING);
   return TRUE;
}

bool spell_identify( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    BUFFER *final;
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    final = display_stats(obj,ch,FALSE);
    page_to_char(final->string,ch);
    free_buf(final);
    return TRUE;
}

bool spell_infernal_offering( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA*)vo;

    if ( is_affected(victim, sn)
    ||   is_affected(victim, gsn_bless)
    ||   is_affected(victim, gsn_divine_blessing) )
    {
	if ( victim == ch )
	    send_to_char("You have already received a reward for your offering.\n\r",victim);
	else
	    act("$N has already received a reward for $S offering.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    if ( !IS_EVIL(victim) )
    {
	if ( victim == ch )
	    send_to_char("Your offering is rejected.\n\r",ch);
	else
	    act("You offer $N but $E is rejected.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    af.where	 = TO_AFFECTS;
    af.type	 = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration	 = level / 4;
    af.location	 = APPLY_HITROLL;
    af.modifier	 = level / 4;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.location	 = APPLY_DAMROLL;
    affect_to_char(victim, &af);

    af.location	 = APPLY_SAVES;
    af.modifier	 = -level / 5;
    affect_to_char(victim, &af);

    send_to_char("{xYou are surrounded by a {Wg{xhostl{Wy{x light.\n\r",victim);
    act("{x$n{x is surrounded by a {Wg{xhostl{Wy{x light.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_infravision( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_INFRARED) )
    {
	if (victim == ch)
	  send_to_char("You can already see in the dark.\n\r",ch);
	else
	  act("$N already has infravision.\n\r",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }
    act( "$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM,POS_RESTING);

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes glow red.\n\r", victim );
    return TRUE;
}

bool spell_intellect( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your intellect can't get any better.\n\r",ch);
 	else
	  act("$N's intellect can't get any better.",
	      ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = level/10;
    af.location  = APPLY_INT;
    af.modifier  = UMAX(1,level / 30);
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char( "You feel your intellect increase.\n\r", victim );

    act("$n's intellect increases.",victim,NULL,NULL,TO_ROOM,POS_RESTING);

    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* object invisibility */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (IS_OBJ_STAT(obj,ITEM_INVIS))
	{
	    act("$p is already invisible.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    return FALSE;
	}

	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.dur_type	= DUR_TICKS;
	af.duration	= level / 2;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= ITEM_INVIS;
	affect_to_obj(obj,&af);

	act("$p fades out of sight.",ch,obj,NULL,TO_ALL,POS_RESTING);
	return TRUE;
    }

    /* character invisibility */
    victim = (CHAR_DATA *) vo;

    if ( IS_SHIELDED(victim, SHD_INVISIBLE) )
	return FALSE;

    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM,POS_RESTING);

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_INVISIBLE;
    affect_to_char( victim, &af );
    send_to_char( "You fade out of existence.\n\r", victim );
    return TRUE;
}

bool spell_know_alignment(int sn,int level,CHAR_DATA *ch,void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *msg;
    int ap;

    ap = victim->alignment;

         if ( ap >  700 ) msg = "$N has a pure and good aura.";
    else if ( ap >  350 ) msg = "$N is of excellent moral character.";
    else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "$N is a black-hearted murderer.";
    else msg = "$N is the embodiment of pure evil!.";

    act( msg, ch, NULL, victim, TO_CHAR,POS_RESTING);
    return TRUE;
}

bool spell_leech( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if (obj->item_type != ITEM_WEAPON)
    {
	send_to_char("Leech may only be used on weapons.\n\r",ch);
	return FALSE;
    }

    if (IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC))
    {
	act("$p is already overcome with leeches.",
	    ch,obj,NULL,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    af.where	 = TO_WEAPON;
    af.type	 = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration	 = level / 4;
    af.location	 = 0;
    af.modifier	 = 0;
    af.bitvector = WEAPON_VAMPIRIC;
    affect_to_obj(obj,&af);

    act("$p is embedded with leeches{x.",ch,obj,NULL,TO_ALL,POS_RESTING);
    return TRUE;
}

bool spell_lightning_bolt(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level * 3,4);

    if ( saves_spell( level,ch, victim,DAM_LIGHTNING) )
	dam = dice(level * 3,3);

    damage( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_lightning_breath( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, hpch, hp_dam, dice_dam;

    act( "$n breathes a bolt of lightning at $N.",
	ch, NULL, victim, TO_NOTVICT, POS_RESTING );
    act( "$n breathes a bolt of lightning at you!",
	ch, NULL, victim, TO_VICT, POS_RESTING );
    act( "You breathe a bolt of lightning at $N.",
	ch, NULL, victim, TO_CHAR, POS_RESTING );

    hpch	= UMAX( 10, ch->max_mana );
    hp_dam	= number_range( hpch / 9 + 1, hpch / 5 );
    dice_dam	= dice( 2 * level, 45 );
    dam		= UMAX( hp_dam + dice_dam / 10, dice_dam + hp_dam / 10 );

    if ( saves_spell( level, ch, victim, DAM_LIGHTNING ) )
    {
	damage( ch, victim, 3 * dam / 4, sn, DAM_LIGHTNING, TRUE, FALSE, NULL );
	shock_effect( ch, victim, level / 4, dam / 4, TARGET_CHAR );
    } else {
	damage( ch, victim, dam, sn, DAM_LIGHTNING, TRUE, FALSE, NULL );
	shock_effect( ch, victim, level / 2, dam, TARGET_CHAR );
    }

    return TRUE;
}

bool spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    char buf[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found = FALSE;
    int number = 0;
    int max_found;

    max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;
    buf[0] = '\0';

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj )
	||   !is_name( global_spell_arg, obj->name )
    	||   IS_OBJ_STAT(obj,ITEM_NOLOCATE)
	||   number_percent() > 2 * level
	||   ch->level < obj->level
	||   IS_OBJ_STAT(obj, ITEM_AQUEST)
	||   IS_OBJ_STAT(obj, ITEM_FORGED)
	||   IS_SET(obj->pIndexData->area->area_flags,AREA_UNLINKED) )
	    continue;

	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );

	if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by))
	{
	    sprintf( buf, "%s is carried by %s\n\r",
                obj->short_descr,
		PERS(in_obj->carried_by, ch) );
	}
	else
	{
	    if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
		sprintf( buf, "%s is in %s [Room %d]\n\r",
                    obj->short_descr,
		    in_obj->in_room->name, in_obj->in_room->vnum);
	    else
	    	sprintf( buf, "%s is in %s\n\r",
		    obj->short_descr, in_obj->in_room == NULL
		    	? "somewhere" : in_obj->in_room->name );
	}

	send_to_char(buf,ch);
	found = TRUE;
        number++;

    	if (number >= max_found)
	    break;
    }

    if ( !found )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return FALSE;
    }

    return TRUE;
}

bool spell_locust_swarm( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch;
    int dam;

    send_to_char("You call forth swarms of lucusts from the heavens!\n\r",ch);
    act("$n calls forth swarms of locusts from the heavens!",
	ch,NULL,NULL,TO_ROOM,POS_RESTING);

    act("Your swarm of locusts attacks $N!",
	ch,NULL,victim,TO_CHAR,POS_RESTING);
    send_to_char("You are attacked by a awarm of locusts!\n\r",victim);

    level = check_curse_of_ages( ch, level );

    dam = dice(level * 3,7);

    if ( saves_spell( level, ch, victim, DAM_DISEASE ) )
	dam = dice(level * 3,6);

    damage( ch, victim, dam, sn, DAM_DISEASE, TRUE, FALSE, NULL );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if ( vch != victim && !is_safe_spell(ch,vch,TRUE)
	&&   vch->damage_mod[DAM_DISEASE] > 0 )
	{
	    act("Your swarm of locusts attacks $N!",
		ch,NULL,vch,TO_CHAR,POS_RESTING);
	    send_to_char("You are attacked by a swarm of locusts!\n\r",vch);

	    dam = dice(level * 3,7);

	    if ( saves_spell( level, ch, vch, DAM_DISEASE ) )
		dam = dice(level * 3,6);

	    damage( ch, vch, dam, sn, DAM_DISEASE, TRUE, FALSE, NULL );
	}
    }
    return TRUE;
}

bool spell_magic_missile( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, pos = level-20;

    dam = dice(level * 2,4);
    if ( saves_spell( level, ch, victim, DAM_ENERGY ) )
	dam = dice(level * 2,3);
    damage( ch, victim, dam, sn, DAM_ENERGY ,TRUE, FALSE, NULL );

    while( pos >= 0 && ch->fighting != NULL )
    {
	dam = dice(level * 2,4);
	if ( saves_spell( level, ch, victim, DAM_ENERGY ) )
	   dam = dice(level * 2,3);
	damage( ch, victim, dam, sn, DAM_ENERGY ,TRUE, FALSE, NULL );
	pos -= 20;
    }

    return TRUE;
}

bool spell_mana_shield( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( IS_SHIELDED(victim, SHD_MANA) )
    {
	if ( victim == ch )
	    send_to_char("You are already surrounded by a mana shield.\n\r", ch );
	else
	    act("$N is already surrounded by a mana shield.",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	 = TO_SHIELDS;
    af.type	 = sn;
    af.level	 = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 5;
    af.location	 = APPLY_NONE;
    af.modifier	 = 0;
    af.bitvector = SHD_MANA;
    affect_to_char(victim, &af);

    send_to_char("You are surrounded by a mana shield.\n\r",victim);
    act("$n is surrounded by a mana shield.",victim, NULL,NULL, TO_ROOM, POS_RESTING);

    return TRUE;
}

bool spell_mass_healing(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ((IS_NPC(ch) && IS_NPC(gch)) ||
	    (!IS_NPC(ch) && !IS_NPC(gch)))
	{
	    spell_heal(gsn_heal,level,ch,(void *) gch,TARGET_CHAR);
	    spell_refresh(gsn_refresh,level,ch,(void *) gch,TARGET_CHAR);
	}
    }
    return TRUE;
}

bool spell_mass_invis( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_SHIELDED(gch, SHD_INVISIBLE) )
	    continue;
	act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM,POS_RESTING);
	send_to_char( "You slowly fade out of existence.\n\r", gch );

	af.where     = TO_SHIELDS;
	af.type      = sn;
    	af.level     = level/2;
	af.dur_type  = DUR_TICKS;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = SHD_INVISIBLE;
	affect_to_char( gch, &af );
    }
    send_to_char( "Ok.\n\r", ch );

    return TRUE;
}

bool spell_nexus( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;
    ROOM_INDEX_DATA *to_room, *from_room;

    from_room = ch->in_room;

    if ( ( victim = get_char_world( ch, global_spell_arg ) ) == NULL
    ||   victim == ch
    ||   (to_room = victim->in_room) == NULL
    ||   !can_see_room(ch,to_room) || !can_see_room(ch,from_room)
    ||   IS_SET(to_room->room_flags, ROOM_SAFE)
    ||	 IS_SET(from_room->room_flags,ROOM_SAFE)
    ||   IS_SET(to_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(to_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(to_room->room_flags, ROOM_ARENA)
    ||   IS_SET(to_room->room_flags, ROOM_WAR)
    ||   IS_SET(from_room->room_flags, ROOM_ARENA)
    ||   IS_SET(from_room->room_flags, ROOM_WAR)
    ||   IS_SET(to_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(from_room->room_flags,ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
    ||   (IS_NPC(victim) && victim->in_room->area->clan != 0)
    ||   (IS_NPC(victim) && saves_spell( level, ch,victim,DAM_OTHER) )
    ||	 (is_clan(victim) && (!is_same_clan(ch,victim)
    ||   clan_table[victim->clan].independent)))
    {
        send_to_char( "You failed.\n\r", ch );
        return FALSE;
    }

    if ( victim->in_room->area->clan != 0 )
    {
	send_to_char("Not to a clan room!\n\r",ch);
	return FALSE;
    }

    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch)
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
        send_to_char("You lack the proper component for this spell.\n\r",ch);
        return FALSE;
    }

    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR,POS_RESTING);
        act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR,POS_RESTING);
        extract_obj(stone);
    }

    /* portal one */
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL));

    if ( portal == NULL )
	return FALSE;

    portal->timer = 1 + level / 10;
    portal->value[3] = to_room->vnum;

    set_arena_obj( ch, portal );
    obj_to_room(portal,from_room);

    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM,POS_RESTING);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR,POS_RESTING);

    /* no second portal if rooms are the same */
    if (to_room == from_room)
	return TRUE;

    /* portal two */
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL));

    if ( portal == NULL )
	return FALSE;

    portal->timer = 1 + level/10;
    portal->value[3] = from_room->vnum;

    set_arena_obj( ch, portal );
    obj_to_room(portal,to_room);

    if (to_room->people != NULL)
    {
	act("$p rises up from the ground.",to_room->people,portal,NULL,TO_ROOM,POS_RESTING);
	act("$p rises up from the ground.",to_room->people,portal,NULL,TO_CHAR,POS_RESTING);
    }
    return TRUE;
}

bool spell_nightmare( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    level = check_curse_of_ages( ch, level );

    if ( number_percent( ) > 60
    ||   saves_spell( level, ch, victim, DAM_MENTAL ) )
    {
	send_to_char( "You failed.\n\r", ch );
	send_to_char( "Horrific visions quickly flash through your mind.\n\r", victim );
	return TRUE;
    }

    act( "You have conjured up $N's worst nightmare!",
	ch, NULL, victim, TO_CHAR, POS_DEAD );
    act( "$n has conjured up your WORST nightmare!",
	ch, NULL, victim, TO_VICT, POS_DEAD );
    act( "$n has conjured up $N's WORST nightmare!",
	ch, NULL, victim, TO_NOTVICT, POS_RESTING );

    do_flee( victim, "" );

    return TRUE;
}

bool spell_null( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    send_to_char( "That's not a spell!\n\r", ch );
    return FALSE;
}

bool spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
	if (victim == ch)
	  send_to_char("You are already out of phase.\n\r",ch);
	else
	  act("$N is already shifted out of phase.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 4;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );
    act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM,POS_RESTING);
    send_to_char( "You turn translucent.\n\r", victim );
    return TRUE;
}

bool spell_plague( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    level = check_curse_of_ages( ch, level );

    if (saves_spell(level,ch,victim,DAM_DISEASE) )
    {
	if (ch == victim)
	  send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
	else
	  act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return TRUE;
    }

    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level	 = level * 3/4;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level;
    af.location  = APPLY_STR;
    af.modifier  = level / -20;
    af.bitvector = AFF_PLAGUE;
    affect_join(victim,&af);

    send_to_char("You scream in agony as plague sores erupt from your skin.\n\r",victim);
    act("$n screams in agony as plague sores erupt from $s skin.",
	victim,NULL,NULL,TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
	{
	    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	    {
		act("Your spell fails to corrupt $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
		return TRUE;
	    }
	    obj->value[3] = 1;
	    act("$p is infused with poisonous vapors.",ch,obj,NULL,TO_ALL,POS_RESTING);
	    return TRUE;
	}

	if (obj->item_type == ITEM_WEAPON)
	{
	    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	    {
		act("$p is already envenomed.",ch,obj,NULL,TO_CHAR,POS_RESTING);
		return FALSE;
	    }

	    af.where	 = TO_WEAPON;
	    af.type	 = sn;
	    af.level	 = level / 2;
	    af.dur_type  = DUR_TICKS;
	    af.duration	 = level/8;
 	    af.location	 = 0;
	    af.modifier	 = 0;
	    af.bitvector = WEAPON_POISON;
	    affect_to_obj(obj,&af);

	    act("$p is coated with deadly venom.",ch,obj,NULL,TO_ALL,POS_RESTING);
	    return TRUE;
	}

	act("You can't poison $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    victim = (CHAR_DATA *) vo;

    level = check_curse_of_ages( ch, level );

    if ( saves_spell( level, ch, victim,DAM_POISON) )
    {
	act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
	send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
	return TRUE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 25;
    af.location  = APPLY_DEX;
    af.modifier  = level / -20;
    af.bitvector = AFF_POISON;
    affect_join( victim, &af );
    send_to_char( "You feel very sick.\n\r", victim );
    act("$n looks very ill.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_portal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;

        if ( ( victim = get_char_world( ch, global_spell_arg ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(victim->in_room->room_flags, ROOM_ARENA)
    ||   IS_SET(victim->in_room->room_flags, ROOM_WAR)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    ||   IS_SET(ch->in_room->room_flags, ROOM_WAR)
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
    ||   (IS_NPC(victim) && victim->in_room->area->clan != 0)
    ||   (IS_NPC(victim) && saves_spell( level, ch,victim,DAM_OTHER) )
    ||	(is_clan(victim) && (!is_same_clan(ch,victim)
    ||  clan_table[victim->clan].independent)))
    {
        send_to_char( "You failed.\n\r", ch );
        return FALSE;
    }

    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch)
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
	send_to_char("You lack the proper component for this spell.\n\r",ch);
	return FALSE;
    }

    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
     	act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR,POS_RESTING);
     	act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR,POS_RESTING);
     	extract_obj(stone);
    }

    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL));

    if ( portal == NULL )
	return FALSE;

    portal->timer = 2 + level / 25;
    portal->value[3] = victim->in_room->vnum;

    set_arena_obj( ch, portal );
    obj_to_room(portal,ch->in_room);

    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM,POS_RESTING);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR,POS_RESTING);
    return TRUE;
}

bool spell_protection_evil(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_SHIELDED(victim, SHD_PROTECT_EVIL)
    ||   IS_SHIELDED(victim, SHD_PROTECT_NEUTRAL)
    ||   IS_SHIELDED(victim, SHD_PROTECT_GOOD))
    {
        if (victim == ch)
          send_to_char("You are already protected.\n\r",ch);
        else
          act("$N is already protected.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 6;
    af.location  = APPLY_SAVES;
    af.modifier  = level / -20;
    af.bitvector = SHD_PROTECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "You feel holy and pure.\n\r", victim );
    if ( ch != victim )
        act("$N is protected from evil.",ch,NULL,victim,TO_CHAR,POS_RESTING);
    return TRUE;
}

bool spell_protection_good(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_SHIELDED(victim, SHD_PROTECT_GOOD)
    ||   IS_SHIELDED(victim, SHD_PROTECT_NEUTRAL)
    ||   IS_SHIELDED(victim, SHD_PROTECT_EVIL))
    {
        if (victim == ch)
          send_to_char("You are already protected.\n\r",ch);
        else
          act("$N is already protected.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 6;
    af.location  = APPLY_SAVES;
    af.modifier  = level / -20;
    af.bitvector = SHD_PROTECT_GOOD;
    affect_to_char( victim, &af );
    send_to_char( "You feel aligned with darkness.\n\r", victim );
    if ( ch != victim )
        act("$N is protected from good.",ch,NULL,victim,TO_CHAR,POS_RESTING);
    return TRUE;
}

bool spell_protection_neutral(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_SHIELDED(victim, SHD_PROTECT_GOOD)
    ||   IS_SHIELDED(victim, SHD_PROTECT_NEUTRAL)
    ||   IS_SHIELDED(victim, SHD_PROTECT_EVIL))
    {
        if (victim == ch)
          send_to_char("You are already protected.\n\r",ch);
        else
          act("$N is already protected.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 6;
    af.location  = APPLY_SAVES;
    af.modifier  = level / -20;
    af.bitvector = SHD_PROTECT_NEUTRAL;
    affect_to_char( victim, &af );
    send_to_char( "You feel protected from neutral.\n\r", victim );
    if ( ch != victim )
        act("$N is protected from neutral.",ch,NULL,victim,TO_CHAR,POS_RESTING);
    return TRUE;
}

bool spell_protection_voodoo(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, SHD_PROTECT_VOODOO) )
    {
	return FALSE;
    }

    af.where	= TO_SHIELDS;
    af.type	= sn;
    af.level	= level;
    af.dur_type = DUR_TICKS;
    af.duration = perm_affect ? -1 : level;
    af.location	= APPLY_NONE;
    af.modifier	= 0;
    af.bitvector = SHD_PROTECT_VOODOO;
    affect_to_char( victim, &af );
    return TRUE;
}

bool spell_ray_of_deceit(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( IS_GOOD( ch ) )
    {
        victim = ch;
        send_to_char( "The energy explodes inside you!\n\r", ch );
    }

    if ( victim != ch )
    {
	act( "$n raises $s hand, and a blinding ray of horror shoots forth!",
	    ch, NULL, NULL, TO_ROOM, POS_RESTING );
	send_to_char( "You raise your hand and a blinding ray of horror shoots forth!\n\r", ch );
    }

    dam = dice(level * 3,7);

    if ( IS_EVIL( victim ) )
	dam = dice(level * 3,6);

    if ( saves_spell( level, ch, victim, DAM_NEGATIVE ) )
	dam = dice(level * 2,5);

    if ( ch->alignment > -900 && ch->alignment < -500 )
	dam = dice(level * 3 / 2,5);

    else if ( ch->alignment > -500 && ch->alignment < -300 )
	dam = dice(level * 3,4);

    damage( ch, victim, dam, sn, DAM_NEGATIVE, TRUE, FALSE, NULL );
    spell_blindness( gsn_blindness, level / 3, ch, (void *) victim, TARGET_CHAR );
    return TRUE;
}

bool spell_ray_of_truth (int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (IS_EVIL(ch) )
    {
        victim = ch;
        send_to_char("The energy explodes inside you!\n\r",ch);
    }

    if (victim != ch)
    {
        act("$n raises $s hand, and a blinding ray of light shoots forth!",
            ch,NULL,NULL,TO_ROOM,POS_RESTING);
        send_to_char(
	   "You raise your hand and a blinding ray of light shoots forth!\n\r",
	   ch);
    }

    dam = dice(level * 3,7);

    if (IS_GOOD(victim))
	dam = dice(level * 3,6);

    if ( saves_spell( level,ch, victim,DAM_LIGHT) )
	dam = dice(level * 2,5);

    if (ch->alignment < 900 && ch->alignment > 500)
	dam = dice(level * 3 / 2,5);

    else if (ch->alignment < 500 && ch->alignment > 300)
	dam = dice(level,4);

    damage( ch, victim, dam, sn, DAM_LIGHT, TRUE, FALSE, NULL );
    spell_blindness(gsn_blindness,
	level / 3, ch, (void *) victim,TARGET_CHAR);
    return TRUE;
}

bool spell_recharge( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int chance, percent;

    if ((obj->item_type != ITEM_WAND) & (obj->item_type != ITEM_STAFF))
    {
	send_to_char("That item does not carry charges.\n\r",ch);
	return FALSE;
    }

    if (obj->value[3] >= 3 * level / 2)
    {
	send_to_char("Your skills are not great enough for that.\n\r",ch);
	return FALSE;
    }

    if (obj->value[1] == 0)
    {
	send_to_char("That item has already been recharged once.\n\r",ch);
	return FALSE;
    }

    chance = 40 + 2 * level;

    chance -= obj->value[3]; /* harder to do high-level spells */
    chance -= (obj->value[1] - obj->value[2]) *
	      (obj->value[1] - obj->value[2]);

    chance = UMAX(level/2,chance);

    percent = number_percent();

    if (percent < chance / 2)
    {
	act("$p glows softly.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	act("$p glows softly.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	obj->value[2] = UMAX(obj->value[1],obj->value[2]);
	obj->value[1] = 0;
	return TRUE;
    }

    else if (percent <= chance)
    {
	int chargeback,chargemax;

	act("$p glows softly.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	act("$p glows softly.",ch,obj,NULL,TO_CHAR,POS_RESTING);

	chargemax = obj->value[1] - obj->value[2];

	if (chargemax > 0)
	    chargeback = UMAX(1,chargemax * percent / 100);
	else
	    chargeback = 0;

	obj->value[2] += chargeback;
	obj->value[1] = 0;
	return TRUE;
    }

    else if (percent <= UMIN(95, 3 * chance / 2))
    {
	send_to_char("Nothing seems to happen.\n\r",ch);
	if (obj->value[1] > 1)
	    obj->value[1]--;
	return TRUE;
    }

    else /* whoops! */
    {
	act("$p glows brightly and explodes!",ch,obj,NULL,TO_CHAR,POS_RESTING);
	act("$p glows brightly and explodes!",ch,obj,NULL,TO_ROOM,POS_RESTING);
	extract_obj(obj);
    }
    return TRUE;
}

bool spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->move = UMIN( victim->move + 2*level, victim->max_move );
    if (victim->max_move == victim->move)
        send_to_char("You feel fully refreshed!\n\r",victim);
    else
        send_to_char( "You feel less tired.\n\r", victim );
    if ( ch != victim )
    {
        send_to_char( "Ok.\n\r", ch );
	show_condition( ch, victim, VALUE_MOVE_POINT );
    }
    return TRUE;
}

bool spell_regeneration( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_REGENERATION) )
    {
	if (victim == ch)
	  send_to_char("The gods already aid your wounds.\n\r",ch);
 	else
	  act("The gods already aid $N's wounds.",
	      ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 4;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_REGENERATION;
    affect_to_char( victim, &af );

    send_to_char( "You feel your wounds closing more quickly.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool found = FALSE;

    /* do object cases first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	{
	    if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE)
	    &&  !saves_dispel(level + 2,obj->level,0))
	    {
		REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
		REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
		act("$p glows blue.",ch,obj,NULL,TO_ALL,POS_RESTING);
		return TRUE;
	    }

	    act("The curse on $p is beyond your power.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    return TRUE;
	}
	else
	{
	    act("There is no curse on $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    return FALSE;
	}
    }

    /* characters */
    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED(victim,AFF_CURSE))
    {
	if (check_dispel(level,victim,gsn_curse))
	{
	    send_to_char("You feel better.\n\r",victim);
	    act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
	} else {
	    send_to_char("You feel more righteous for a moment.\n\r",victim);
	    if (ch != victim)
		act("$n's curse dulls slighlty.",victim,NULL,NULL,TO_CHAR,POS_RESTING);
	}
    }

   for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
   {
        if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	&&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
        {   /* attempt to remove curse */
            if (!saves_dispel(level,obj->level,0))
            {
                found = TRUE;
                REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
                REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
                act("Your $p glows blue.",victim,obj,NULL,TO_CHAR,POS_RESTING);
                act("$n's $p glows blue.",victim,obj,NULL,TO_ROOM,POS_RESTING);
            }
         }
    }
    return TRUE;
}

bool spell_restore_health( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( victim == NULL )
	return FALSE;

    victim->hit = victim->max_hit;
    update_pos( victim );

    send_to_char( "You feel your life pulsating through your veins.\n\r", victim );

    return TRUE;
}

bool spell_restore_magic( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( victim == NULL )
	return FALSE;

    victim->mana = victim->max_mana;
    update_pos( victim );

    send_to_char( "You feel magic pulsating through your veins.\n\r", victim );

    return TRUE;
}

bool spell_restore_mana( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->mana = UMIN( victim->mana + 51, victim->max_mana );
    if (victim->max_mana == victim->mana)
        send_to_char("You feel fully focused!\n\r",victim);
    else
        send_to_char( "You feel more focused.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_restore_movement( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( victim == NULL )
	return FALSE;

    victim->move = victim->max_move;
    update_pos( victim );

    send_to_char( "You feel youth pulsating through your veins.\n\r", victim );

    return TRUE;
}

bool spell_resurrect( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    OBJ_DATA *obj, *cobj, *obj_next;
    CHAR_DATA *pet;
    int	length;

    if ( ( obj = get_obj_here( ch, NULL, "corpse" ) ) == NULL )
    {
	send_to_char( "There's no corpse here.\n\r", ch );
	return FALSE;
    }

    if ( ch->pet != NULL )
    {
	send_to_char("You failed.\n\r",ch);
	return FALSE;
    }

    if (!can_loot(ch,obj))
    {
	send_to_char("You may only resurrect your victims.\n\r",ch);
	return FALSE;
    }

    pMobIndex = get_mob_index( MOB_VNUM_CORPSE );
    pet = create_mobile( pMobIndex );

    if ( pet == NULL )
	return FALSE;

    if (!IS_SET(pet->act, ACT_PET) )
        SET_BIT(pet->act, ACT_PET);
    if (!IS_SET(pet->affected_by, AFF_CHARM) )
        SET_BIT(pet->affected_by, AFF_CHARM);

    sprintf( buf, "%s{GThe mark of %s is on it's forehead.{x.\n\r",
	pet->description, ch->name );

    free_string( pet->description );
    pet->description = str_dup( buf );

    free_string( pet->short_descr );
    pet->short_descr = str_dup( str_replace(obj->short_descr, "corpse", "zombie") );

    sprintf( buf, "%s", str_replace(obj->description, "corpse", "zombie") );

    length = strlen(buf)-12;
    strncpy( arg, buf, length);
    arg[length] = '\0';
    sprintf( buf, "%s standing here.\n\r", arg);
    free_string( pet->long_descr );
    pet->long_descr = str_dup( buf );
    char_to_room( pet, ch->in_room );
    add_follower( pet, ch );
    pet->leader = ch;
    ch->pet = pet;
    pet->alignment = ch->alignment;
    pet->level = UMAX(1, UMIN(109, ((ch->level/2)+(obj->level/2))));
    pet->max_hit = pet->level * 15;
    pet->hit = pet->max_hit;
    pet->max_mana = number_range(ch->mana * 1 / 3, ch->mana );
    pet->mana = pet->max_mana;
    pet->damage[DICE_NUMBER] = pet->level / 5;
    pet->damage[DICE_TYPE] = pet->level / 5;
    pet->damroll = number_range( pet->level * 7, pet->level * 12 );
    pet->hitroll = number_range( pet->level * 7, pet->level * 12 );
    pet->armor[0] = -pet->level;
    pet->armor[1] = -pet->level;
    pet->armor[2] = -pet->level;
    pet->armor[3] = -pet->level;

    if ( obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC )
    {
	sprintf( buf, "{rs{yki{rn {ybag {xof %s{x", obj->owner );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, "skin bag of %s", obj->owner );
	free_string( obj->name );
	obj->name = str_dup( buf );

	sprintf( buf, "You see, matted together, some sort of {ybag{x made out of a carcass with the likeness of %s.", obj->owner );
	free_string( obj->description );
	obj->description = str_dup( buf );
    } else {
	for ( cobj = obj->contains; cobj != NULL; cobj = obj_next )
	{
	    obj_next = cobj->next_content;
	    obj_from_obj( cobj );
	    set_arena_obj( ch, cobj );
	    obj_to_room( cobj, ch->in_room );
	}
	extract_obj( obj );
    }

    sprintf( buf, "%s stands up and starts following you.\n\r", pet->short_descr);
    send_to_char( buf, ch);
    sprintf( buf, "%s stands up and starts following $n.", pet->short_descr);
    act( buf, ch, NULL, NULL, TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_rockshield( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_ROCK))
    {
        if(victim == ch)
            send_to_char("You are already surrounded by a {yrocky{x shield.\n\r", ch);
        else
            act("$N is already surrounded by a {yrocky{x shield.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : 5 + level / 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_ROCK;

   affect_to_char(victim, &af);
   send_to_char("You are surrounded by a {yrocky{x shield.\n\r", victim);
   act("$n is surrounded by a {yrocky{x shield.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
   return TRUE;
}

bool spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_SHIELDED(victim, SHD_SANCTUARY)
    ||   IS_SHIELDED(victim, SHD_DIVINITY) )
    {
	if (victim == ch)
	  send_to_char("You are already in sanctuary.\n\r",ch);
	else
	  act("$N is already in sanctuary.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_SANCTUARY;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM,POS_RESTING);
    send_to_char( "You are surrounded by a white aura.\n\r", victim );
    return TRUE;
}

bool spell_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already shielded from harm.\n\r",ch);
	else
	  act("$N is already protected by a shield.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = level / 4;
    af.location  = APPLY_AC;
    af.modifier  = level / -3;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM,POS_RESTING);
    send_to_char( "You are surrounded by a force shield.\n\r", victim );
    return TRUE;
}

bool spell_shocking_grasp(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

   dam = dice(level,4);

    if ( saves_spell( level,ch, victim,DAM_LIGHTNING) )
	dam = dice(level,3);

    damage( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_shockshield(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_SHOCK))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded in a {Bcrackling{x shield.\n\r", ch);
	else
	    act("$N is already surrounded by a {Bcrackling{x shield.",ch, NULL, victim, TO_CHAR,POS_RESTING);
	return FALSE;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : 5 + level / 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_SHOCK;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by a {Bcrackling{x field.\n\r",victim);
    act("$n is surrounded by a {Bcrackling{x shield.",victim, NULL,NULL, TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_shrapnelshield( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_SHRAPNEL))
    {
        if(victim == ch)
            send_to_char("You are already surrounded by a {8s{7h{8r{7a{8p{7n{8e{7l{x shield.\n\r", ch);
        else
            act("$N is already surrounded by a {8s{7h{8r{7a{8p{7n{8e{7l{x shield.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : 5 + level / 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_SHRAPNEL;

   affect_to_char(victim, &af);
   send_to_char("You are surrounded by a {8s{7h{8r{7a{8p{7n{8e{7l{x shield.\n\r", victim);
   act("$n is surrounded by a {8s{7h{8r{7a{8p{7n{8e{7l{x shield.",victim, NULL,NULL,TO_ROOM,POS_RESTING);
   return TRUE;
}

bool spell_shrink( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    OBJ_DATA *obj, *obj_next;

    if ( victim->size == SIZE_TINY )
    {
	if ( victim == ch )
	    send_to_char("You can't get any smaller.\n\r",ch);
	else
	    act("$N can't get any smaller.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    if ( is_affected( victim, sn ) )
    {
	if ( victim == ch )
	    send_to_char("Your size has already been lowered.\n\r",ch);
	else
	    act("$N's size has already been lowered.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    if ( saves_spell(level,ch,victim,DAM_OTHER) )
    {
	if ( victim != ch )
	    send_to_char("Your spell failed.\n\r",ch);
	else
	    send_to_char("You feel a sharp pain but it subsides quickly.\n\r",victim);
	return TRUE;
    }

    if ( is_affected(victim,gsn_growth) )
    {
	if ( !check_dispel(level,victim,gsn_growth) )
	{
	    if ( victim != ch )
		send_to_char("Spell failed.\n\r",ch);
	    send_to_char("Your feel smaller for an instant.\n\r",victim);
	    return TRUE;
	}

	act("$n looks more like $s normal self.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
	return TRUE;
    }

    af.where	 = TO_AFFECTS;
    af.level	 = level;
    af.type	 = sn;
    af.dur_type  = DUR_TICKS;
    af.duration	 = level / 20;
    af.location	 = APPLY_SIZE;
    af.modifier	 = -1;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    send_to_char("Your head jerks in pain as your body shrinks.\n\r",victim);
    act("$n's head jerks in pain as $s body shrinks.",
	victim,NULL,NULL,TO_ROOM,POS_RESTING);

    for ( obj = victim->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;

	if ( obj->wear_loc != WEAR_NONE )
	{
	    if ( obj->size != SIZE_NONE && obj->size > victim->size+1 )
	    {
		act("$p appears to grow, and slips off!",
		    victim,obj,NULL,TO_CHAR,POS_DEAD);
		obj_from_char(obj);

		if ( IS_OBJ_STAT(obj, ITEM_NODROP)
		||   IS_OBJ_STAT(obj, ITEM_INVENTORY)
		||   IS_OBJ_STAT(obj, ITEM_AQUEST)
		||   IS_OBJ_STAT(obj, ITEM_FORGED) )
		    obj_to_char(obj,victim);
		else
		{
		    set_obj_sockets( victim, obj );
		    set_arena_obj( victim, obj );
		    obj_to_room(obj,victim->in_room);
		}
	    }
	}
    }

    return TRUE;
}

bool spell_silence( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( !victim )
	return FALSE;

    if ( is_affected( victim, sn ) )
    {
	act( "$N has already been silenced.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    if ( ( level + 15 ) < victim->level
    ||   saves_spell( level, ch, victim, DAM_MENTAL ) )
    {
	send_to_char( "Spell failed.\n\r", ch );
	return TRUE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = number_range( level / 25, level / 20 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_join( victim, &af );

    send_to_char( "Your lips quit moving.\n\r", victim );
    act( "$n's lips quit moving.", victim, NULL, NULL, TO_ROOM, POS_RESTING );

    return TRUE;
}

bool spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    char buf[MAX_STRING_LENGTH];

    if ( !IS_AWAKE(victim) )
    {
	send_to_char("They are already sleeping.\n\r",ch);
	return FALSE;
    }

    if ( (level + 15) < victim->level
    ||   saves_spell( level-4, ch, victim, DAM_MENTAL ) )
    {
	send_to_char("You failed.\n\r",ch);
	return TRUE;
    }

    if ( !IS_NPC(ch) && ch->pcdata->dtimer > 0 )
    {
        send_to_char("After the shock of death, you can not muster the concentration for it.\n\r",ch);
        return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = number_range(level / 25, level / 20);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
    act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM,POS_RESTING);
    victim->position = POS_SLEEPING;

    if (!IS_NPC(victim)
    &&  !IS_SET(ch->in_room->room_flags,ROOM_ARENA)
    &&  !IS_SET(ch->in_room->room_flags,ROOM_WAR) )
    {
	sprintf(buf,"{w({RPK{w) {V$N sleeps %s.", victim->name);
	wiznet(buf,ch,NULL,WIZ_PKILLS,0,0);
    }

    return TRUE;
}

bool spell_mana_tap( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = IS_SET( victim->in_room->room_flags, ROOM_SAFE ) ? 2 : 0;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    send_to_char( "You slowly begin to meditate.\n\r", victim );
    act( "$n slowly begins to meditate.", victim, NULL, NULL, TO_ROOM,POS_RESTING );

    victim->position = POS_SLEEPING;

    return TRUE;
}

bool spell_slow( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( (is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
    &&    !IS_AFFECTED( victim, AFF_HASTE) )
    {
        if (victim == ch)
          send_to_char("You can't move any slower!\n\r",ch);
        else
          act("$N can't get any slower than that.",
              ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    level = check_curse_of_ages( ch, level );

    if (saves_spell(level,ch,victim,DAM_OTHER) )
    {
	if (victim != ch)
            send_to_char("Nothing seemed to happen.\n\r",ch);
        send_to_char("You feel momentarily lethargic.\n\r",victim);
        return TRUE;
    }

    if (is_affected(victim,gsn_haste))
    {
        if (!check_dispel(level,victim,gsn_haste))
        {
	    if (victim != ch)
            	send_to_char("Spell failed.\n\r",ch);
            send_to_char("You feel momentarily slower.\n\r",victim);
            return TRUE;
        }

	send_to_char("Ok.\n\r",ch);
        return TRUE;
    }

    else if (IS_AFFECTED(victim,AFF_HASTE))
    {
	if ( !saves_dispel(level, victim->level,-1) )
	{
	    REMOVE_BIT(victim->affected_by,AFF_HASTE);
	    send_to_char("Ok.\n\r",ch);
	    act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
	    return TRUE;
	}
	if (victim != ch)
	    send_to_char("Spell failed.\n\r",ch);
        send_to_char("You feel momentarily slower.\n\r",victim);
	return TRUE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 20;
    af.location  = APPLY_DEX;
    af.modifier	 = -level / 20;
    af.bitvector = AFF_SLOW;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
    act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_steel_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your skin is already as strong as {Ds{wtee{Dl{x.\n\r",ch);
	else
	  act("$N is already as strong as {Ds{wtee{Dl{x.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = level / 5;
    af.location  = APPLY_AC;
    af.modifier  = -3 * level / 2;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's skin turns {Dm{wetali{Dc{x.", victim, NULL, NULL, TO_ROOM,POS_RESTING);
    send_to_char( "Your skin turns {Dm{wetali{Dc{x.\n\r", victim );
    return TRUE;
}

bool spell_snow_storm( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch;
    int dam;

    send_to_char("You call forth {cf{Cr{ceezi{Cn{cg{x fluries from the heavens!\n\r",ch);
    act("$n calls forth {cf{Cr{ceezi{Cn{cg{x fluries from the heavens!",
	ch,NULL,NULL,TO_ROOM,POS_RESTING);

    act("Your {cf{Cr{ceezi{Cn{cg{x fluries snow down on $N!",
	ch,NULL,victim,TO_CHAR,POS_RESTING);
    send_to_char("Your skin {cf{Cr{ceez{Ce{cs{x from the blizzard!\n\r",victim);

    dam = dice(level * 3,7);

    if ( saves_spell( level, ch, victim, DAM_COLD ) )
	dam = dice(level * 3,6);

    damage( ch, victim, dam, sn, DAM_COLD, TRUE, FALSE, NULL );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if ( vch != victim && !is_safe_spell(ch,vch,TRUE)
	&&   vch->damage_mod[DAM_COLD] > 0 )
	{
	    act("Your {cf{Cr{ceezi{Cn{cg{x fluries snow down on $N!",
		ch,NULL,vch,TO_CHAR,POS_RESTING);
	    send_to_char("Your skin {cf{Cr{ceez{Ce{cs{x from the blizzard!\n\r",vch);

	    dam = dice(level * 3,7);

	    if ( saves_spell( level, ch, vch, DAM_COLD ) )
		dam = dice(level * 3,6);

	    damage( ch, vch, dam, sn, DAM_COLD, TRUE, FALSE, NULL );
	}
    }
    return TRUE;
}

bool spell_stone_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your skin is already as hard as a rock.\n\r",ch);
	else
	  act("$N is already as hard as can be.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = level / 2;
    af.location  = APPLY_AC;
    af.modifier  = - level;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM,POS_RESTING);
    send_to_char( "Your skin turns to stone.\n\r", victim );
    return TRUE;
}

bool spell_summon( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;

    if ( ( victim = get_char_world( ch, global_spell_arg ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_ARENA)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(victim->in_room->room_flags, ROOM_WAR)
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(ch->in_room->room_flags, ROOM_WAR)
    ||   IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)
    ||   victim->fighting != NULL
    ||   (IS_NPC(victim) && victim->clan != 0)
    ||	 (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    ||   (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON))
    ||   (IS_NPC(victim) && saves_spell( level,ch, victim,DAM_OTHER)) )
    {
	send_to_char( "You failed.\n\r", ch );
	return FALSE;
    }

    if ( ch->in_room->area->clan != 0 && ch->in_room->area->clan != victim->clan )
    {
	send_to_char("No more jackass!\n\r",ch);
	return FALSE;
    }

    if (!check_pktest(ch,victim))
	return FALSE;

    act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM,POS_RESTING);
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM,POS_RESTING);
    act( "$n has summoned you!", ch, NULL, victim,   TO_VICT,POS_RESTING);
    do_look( victim, "auto" );
    return TRUE;
}

bool spell_swarm( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( victim != ch )
    {
	combat("$n calls forth a swarm of diseases upon $N!",
	    ch,NULL,victim,TO_NOTVICT,COMBAT_OTHER);
	act("$n has assailed you with a swarm of diseases!",
	    ch,NULL,victim,TO_VICT,POS_RESTING);
	send_to_char("You conjure forth a swarm of diseases!\n\r",ch);
    }

    dam = dice(level * 3,7);

    if ( saves_spell(level, ch, victim, DAM_DISEASE) )
	dam = dice(level * 3,6);

    damage( ch, victim, dam, sn, DAM_DISEASE, TRUE, FALSE, NULL );
    spell_blindness(gsn_blindness, level/2, ch, (void *) victim, TARGET_CHAR);
    spell_slow(gsn_slow, level/2, ch, (void *) victim, TARGET_CHAR);

    return TRUE;
}

bool spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;

    if ( victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(victim->in_room->room_flags, ROOM_ARENA)
    ||   IS_SET(victim->in_room->room_flags, ROOM_WAR)
    ||   IS_SET(ch->in_room->room_flags, ROOM_WAR)
    ||   IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    ||   IS_AFFECTED(victim,AFF_CURSE)
    || ( !IS_NPC(ch) && victim->fighting != NULL )
    || ( victim != ch
    && ( saves_spell( level - 5,ch, victim,DAM_OTHER))))
    {
	send_to_char( "You failed.\n\r", ch );
	return FALSE;
    }

    if ( !IS_NPC(victim) && victim->pcdata->pktimer > 0 )
    {
	send_to_char("Teleport may not be used to escape PK.\n\r",ch);
	return FALSE;
    }

    if (is_safe(ch,victim) && victim != ch)
    {
	send_to_char("You may not teleport that victim.\n\r",ch);
	return FALSE;
    }

    pRoomIndex = get_random_room(victim);

    if (victim != ch)
	send_to_char("You have been teleported!\n\r",victim);

    act( "$n vanishes!", victim, NULL, NULL, TO_ROOM,POS_RESTING);

    if ( ch->fighting != NULL && ch->fighting == victim )
	stop_fighting( ch, TRUE );
    stop_fighting( victim, TRUE );

    char_from_room( victim );
    char_to_room( victim, pRoomIndex );

    act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM,POS_RESTING);
    do_look( victim, "auto" );
    return TRUE;
}

#define MAX_BLINK	20

bool spell_blink( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *in_room, *to_room;
    char arg[MAX_INPUT_LENGTH];
    int count, door, move[MAX_BLINK], pos;

    if ( !IS_NPC( ch ) && ch->pcdata->pktimer > 0 )
    {
	send_to_char( "Blink may not be used to escape PK.\n\r", ch );
	return FALSE;
    }

    for ( count = 0; count < MAX_BLINK; )
    {
	if ( global_spell_arg == NULL || *global_spell_arg == '\0' )
	{
	    move[count] = -1;
	    break;
	}

	arg[0] = '\0';
	while( isdigit( *global_spell_arg ) )
	    strncat( arg, global_spell_arg++, 1 );

	pos = arg[0] == '\0' ? 1 : atoi( arg );

	if ( pos <= 0 )
	{
	    send_to_char( "Invalid directions.\n\r", ch );
	    return FALSE;
	}
	arg[0] = '\0';

	switch ( LOWER( *global_spell_arg ) )
	{
	    case 'n': door = DIR_NORTH;	break;
	    case 's': door = DIR_SOUTH;	break;
	    case 'w': door = DIR_WEST;	break;
	    case 'e': door = DIR_EAST;	break;
	    case 'u': door = DIR_UP;	break;
	    case 'd': door = DIR_DOWN;	break;
	    default:
		send_to_char( "Invalid directions.\n\r", ch );
		return FALSE;
	}

	for ( ; pos > 0 && count < MAX_BLINK; pos-- )
	{
	    move[count] = door;
	    count++;
	}

	(void) *global_spell_arg++;
    }

    in_room = ch->in_room;

    act( "$n blinks away!", ch, NULL, NULL, TO_ROOM, POS_RESTING );

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );
    char_from_room( ch );

    for ( count = 0; count < MAX_BLINK && move[count] != -1; count++ )
    {
	if ( ( pexit = in_room->exit[move[count]] ) == NULL
	||   ( to_room = pexit->u1.to_room ) == NULL
	||   !can_see_room( ch, pexit->u1.to_room ) )
	{
	    send_to_char( "{RYou blink yourself right into a wall.{x\n\r\n\r", ch );
	    break;
	}

	if ( IS_SET( pexit->exit_info, EX_NOBLINK ) )
	{
	    send_to_char( "{RYou blink yourself right into a magical barrier.{x\n\r\n\r", ch );
	    break;
	}

	if ( IS_SET( to_room->area->area_flags, AREA_UNLINKED )
	&&   ch->pcdata && !IS_IMMORTAL( ch ) )
	{
	    send_to_char( "I'm sorry, that exit leads to an unlinked area and you may not enter.\n\r", ch );
	    break;
	}

	in_room = to_room;
    }

    char_to_room( ch, in_room );

    act( "$n suddenly pops into existence.", ch, NULL, NULL, TO_ROOM, POS_RESTING );
    do_look( ch, "auto" );
    return TRUE;
}

#undef MAX_BLINK

bool spell_thornshield( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_THORN))
    {
        if(victim == ch)
            send_to_char("You are already surrounded by a {gt{yh{go{yr{gn{yy{x shield.\n\r", ch);
        else
            act("$N is already surrounded by a {gt{yh{go{yr{gn{yy{x shield.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : 5 + level / 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_THORN;

   affect_to_char(victim, &af);
   send_to_char("You are surrounded by a {gt{yh{go{yr{gn{yy{x shield.\n\r", victim);
   act("$n is surrounded by a {gt{yh{go{yr{gn{yy{x shield.",victim, NULL,NULL,TO_ROOM,POS_RESTING);
   return TRUE;
}

bool spell_transport( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    return TRUE;
}

bool spell_vampiricshield( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_VAMPIRIC))
    {
        if(victim == ch)
            send_to_char("You are already surrounded by a {Wv{wa{Wm{wp{Wi{wr{Wi{wc{x shield.\n\r", ch);
        else
            act("$N is already surrounded by a {Wv{wa{Wm{wp{Wi{wr{Wi{wc{x shield.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : 5 + level / 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_VAMPIRIC;

   affect_to_char(victim, &af);
   send_to_char("You are surrounded by a {Wv{wa{Wm{wp{Wi{wr{Wi{wc{x shield.\n\r", victim);
   act("$n is surrounded by a {Wv{wa{Wm{wp{Wi{wr{Wi{wc{x shield.",victim, NULL,NULL,TO_ROOM,POS_RESTING);
   return TRUE;
}

bool spell_ventriloquate( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;

    global_spell_arg = one_argument( global_spell_arg, speaker );

    sprintf( buf1, "%s says '{S%s{x'\n\r",              speaker, global_spell_arg );
    sprintf( buf2, "Someone makes %s say '{S%s{x'\n\r", speaker, global_spell_arg );
    buf1[0] = UPPER(buf1[0]);

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if ( !is_name( speaker, vch->name ) )
	    send_to_char( saves_spell(level,ch,vch,DAM_OTHER) ? buf2 : buf1, vch );
    }

    return TRUE;
}

bool spell_voodoo( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    char name[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *bpart;
    OBJ_DATA *doll;

    bpart = get_eq_char(ch,WEAR_HOLD);

    if ( !clan_table[ch->clan].pkill )
    {
	send_to_char("The voodoo spell is restricted to pkill characters only.\n\r",ch);
	return FALSE;
    }

    if ( bpart == NULL
    ||   bpart->pIndexData->vnum < 11
    ||   bpart->pIndexData->vnum > 22 )
    {
	send_to_char("You are not holding a body part.\n\r",ch);
	return FALSE;
    }

    if (bpart->value[4] == 0)
    {
	send_to_char("This body part is from a mobile.\n\r",ch);
	return FALSE;
    }

    one_argument(bpart->name, name);
    doll = create_object(get_obj_index(OBJ_VNUM_VOODOO));

    if ( doll == NULL )
	return FALSE;

    sprintf( buf, doll->short_descr, name );
    free_string( doll->short_descr );
    doll->short_descr = str_dup( buf );
    sprintf( buf, doll->description, name );
    free_string( doll->description );
    doll->description = str_dup( buf );
    sprintf( buf, doll->name, name );
    free_string( doll->name );
    doll->name = str_dup( buf );
    act( "$p morphs into a voodoo doll",ch,bpart,NULL,TO_CHAR,POS_RESTING);
    obj_from_char( bpart );
    obj_to_char(doll,ch);
    equip_char(ch,doll,WEAR_HOLD);
    act( "$n has created $p!", ch, doll, NULL, TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_watershield( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_WATER))
    {
        if(victim == ch)
            send_to_char("You are already surrounded by a {Bwatery{x shield.\n\r", ch);
        else
            act("$N is already surrounded by a {Bwatery{x shield.",ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : 5 + level / 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_WATER;

   affect_to_char(victim, &af);
   send_to_char("You are surrounded by a {Bwatery{x shield.\n\r", victim);
   act("$n is surrounded by a {Bwatery{x shield.",victim, NULL,NULL,TO_ROOM,POS_RESTING);
   return TRUE;
}

bool spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    level = check_curse_of_ages( ch, level );

    if (IS_AFFECTED(victim,AFF_GIANT_STRENGTH))
    {
	if (!check_dispel(level,victim,gsn_giant_strength))
	{
	    if (victim != ch)
	        send_to_char("Spell failed.\n\r",ch);
	    send_to_char("You feel momentarily weaker.\n\r",victim);
	    return TRUE;
	}
        return TRUE;
    }

    if ( is_affected( victim, sn ) )
    {
	act("$N is already weakened.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return FALSE;
    }

    if ( saves_spell( level,ch, victim,DAM_OTHER) )
    {
	act("You failed to weaken $N.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	send_to_char("Your muscles quiver then feel normal again.\n\r",victim);
	return TRUE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = perm_affect ? -1 : level / 10;
    af.location  = APPLY_STR;
    af.modifier  = -1 * (level / 5);
    af.bitvector = AFF_WEAKEN;
    affect_to_char( victim, &af );
    send_to_char( "You feel your strength slip away.\n\r", victim );
    act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
    return TRUE;
}

bool spell_wizard_eye( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    BUFFER *final, *chars;
    char buf[MAX_STRING_LENGTH];

    if ( (victim = get_char_world(ch,global_spell_arg)) == NULL
    ||    victim->in_room == NULL
    ||   (IS_IMMORTAL(victim) && !can_over_ride(ch,victim,TRUE)) )
    {
	send_to_char("Spell failed.\n\r",ch);
	return FALSE;
    }

    if (IS_NPC( victim )
    &&  victim->pIndexData->vnum >= 5
    &&  victim->pIndexData->vnum <= 20 )
    {
	send_to_char("Off for a bit of soul hunting eh? Do it the hard way.\n\r",ch);
	return FALSE;
    }

    if ( saves_spell(level,ch,victim,DAM_OTHER) )
    {
	send_to_char("You sense a tingle in your eyes.\n\r",victim);
	send_to_char("Spell failed.\n\r",ch);
	return TRUE;
    }

    sprintf(buf,"{e%s{x", victim->in_room->name);
    send_to_char(buf,ch);

    if ( IS_IMMORTAL(ch) && (IS_NPC(ch) || IS_SET(ch->act,PLR_HOLYLIGHT)) )
    {
	sprintf(buf," [Room %d]",victim->in_room->vnum);
	send_to_char(buf,ch);
    }

    send_to_char("\n\r",ch);

    if ( !IS_NPC(ch) && !IS_SET(ch->configure, CONFIG_BRIEF) )
    {
	send_to_char("  ",ch);
	send_to_char(victim->in_room->description,ch);
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
    {
	EXIT_DATA *pexit;
	extern char * const dir_name[];
	sh_int door, outlet;
	bool found, round;

	found = FALSE;

	sprintf(buf,"\n\r[Exits:");
	for ( door = 0; door < 6; door++ )
	{
	    round = FALSE;
	    outlet = door;

	    if ( ( ch->alignment < 0 )
	    &&   ( pexit = ch->in_room->exit[door+6] ) != NULL )
		outlet += 6;

	    if ( ( pexit = ch->in_room->exit[outlet] ) != NULL
	    &&   pexit->u1.to_room != NULL
	    &&   can_see_room(ch,pexit->u1.to_room)
	    &&   !IS_SET(pexit->exit_info, EX_CLOSED) )
	    {
		found = TRUE;
		round = TRUE;

		strcat( buf, " " );
		strcat( buf, dir_name[outlet] );
	    }

	    if ( !round )
	    {
		OBJ_DATA *portal;

		portal = get_obj_exit( dir_name[door], victim->in_room->contents );

		if ( portal != NULL )
		{
		    found = TRUE;
		    round = TRUE;

		    strcat( buf, " " );
		    strcat( buf, dir_name[door] );
		}
	    }
	}
	if ( !found )
	    strcat( buf, " none" );
	strcat( buf, "]\n\r" );
	send_to_char(buf,ch);
    }

    final = show_list_to_char( victim->in_room->contents, ch, FALSE, FALSE );
    chars = show_char_list( victim->in_room->people, ch );
    add_buf( final, chars->string );
    free_buf( chars );

    page_to_char( final->string, ch );
    free_buf( final );

    return TRUE;
}

bool spell_wisdom( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your wisdom can't get any better.\n\r",ch);
 	else
	  act("$N's wisdom can't get any better.",
	      ch,NULL,victim,TO_CHAR,POS_RESTING);
        return FALSE;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.dur_type  = DUR_TICKS;
    af.duration  = level/10;
    af.location  = APPLY_WIS;
    af.modifier  = UMAX(1,level / 30);
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char( "You feel your wisdom increase.\n\r", victim );

    act("$n's wisdom increases.",victim,NULL,NULL,TO_ROOM,POS_RESTING);

    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

bool spell_word_of_recall( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *location;

    if (IS_NPC(victim))
      return FALSE;

    if (IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL)
    ||  IS_SET(victim->in_room->room_flags,ROOM_ARENA)
    ||  IS_SET(victim->in_room->room_flags,ROOM_WAR)
    ||  IS_SET(ch->in_room->room_flags,ROOM_WAR)
    ||  IS_SET(ch->in_room->room_flags,ROOM_ARENA)
    ||	IS_AFFECTED(victim,AFF_CURSE))
    {
	send_to_char("Spell failed.\n\r",victim);
	return FALSE;
    }

    if (is_safe(ch,victim) && victim != ch)
    {
	send_to_char("You may not cast word of recall on that target.\n\r",ch);
	return FALSE;
    }

    if ( ch->alignment < 0 )
    {
	if ((location = get_room_index( ROOM_VNUM_TEMPLEB)) == NULL)
	{
	    send_to_char("You are completely lost.\n\r",victim);
	    return FALSE;
	}
    }
    else
    {
	if ((location = get_room_index( ROOM_VNUM_TEMPLE)) == NULL)
	{
	    send_to_char("You are completely lost.\n\r",victim);
	    return FALSE;
	}
    }

    if ( !check_builder( victim, location->area, TRUE ) )
	return FALSE;

    if (victim->fighting != NULL)
	stop_fighting(victim,TRUE);

    ch->move /= 2;
    act("$n disappears.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
    char_from_room(victim);
    char_to_room(victim,location);
    act("$n appears in the room.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
    do_look(victim,"auto");
    return TRUE;
}

bool spell_incinerate( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam = 0;

    if ( !victim )
	return FALSE;

    if ( saves_spell( level, ch, victim, DAM_FIRE ) )
    {
	fire_effect( ch, victim, level / 6, dam, TARGET_CHAR );
	dam = dice( level * 3, 5 );
    } else {
	fire_effect( ch, victim, level / 3, dam, TARGET_CHAR );
	dam = dice( level * 3, 11 );
    }

    damage( ch, victim, dam, sn, DAM_FIRE, TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_thunderbolt( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam = 0;

    if ( !victim )
	return FALSE;

    if ( saves_spell( level, ch, victim, DAM_LIGHTNING ) )
    {
	shock_effect( ch, victim, level / 6, dam, TARGET_CHAR );
	dam = dice( level * 3, 5 );
    } else {
	shock_effect( ch, victim, level / 3, dam, TARGET_CHAR );
	dam = dice( level * 3, 11 );
    }

    damage( ch, victim, dam, sn, DAM_LIGHTNING, TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_glacier( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam = 0;

    if ( !victim )
	return FALSE;

    if ( saves_spell( level, ch, victim, DAM_COLD ) )
    {
	cold_effect( ch, victim, level / 6, dam, TARGET_CHAR );
	dam = dice( level * 3, 5 );
    } else {
	cold_effect( ch, victim, level / 3, dam, TARGET_CHAR );
	dam = dice( level * 3, 11 );
    }

    damage( ch, victim, dam, sn, DAM_COLD, TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_blizzard( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch;
    int dam;

    send_to_char("You call forth {cf{Cr{ceezi{Cn{cg{x fluries from the heavens!\n\r",ch);
    act("$n calls forth {cf{Cr{ceezi{Cn{cg{x fluries from the heavens!",
	ch,NULL,NULL,TO_ROOM,POS_RESTING);

    act("Your {cf{Cr{ceezi{Cn{cg{x fluries snow down on $N!",
	ch,NULL,victim,TO_CHAR,POS_RESTING);
    send_to_char("Your skin {cf{Cr{ceez{Ce{cs{x from the blizzard!\n\r",victim);

    dam = dice(level * 3,7);

    if ( saves_spell( level, ch, victim, DAM_COLD ) )
	dam = dice(level * 3,6);

    damage( ch, victim, dam, sn, DAM_COLD, TRUE, FALSE, NULL );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if ( vch != victim && !is_safe_spell(ch,vch,TRUE)
	&&   vch->damage_mod[DAM_COLD] > 0 )
	{
	    act("Your {cf{Cr{ceezi{Cn{cg{x fluries snow down on $N!",
		ch,NULL,vch,TO_CHAR,POS_RESTING);
	    send_to_char("Your skin {cf{Cr{ceez{Ce{cs{x from the blizzard!\n\r",vch);

	    dam = dice(level * 3,7);

	    if ( saves_spell( level, ch, vch, DAM_COLD ) )
		dam = dice(level * 3,6);

	    damage( ch, vch, dam, sn, DAM_COLD, TRUE, FALSE, NULL );
	}
    }
    return TRUE;
}

bool spell_icicle( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level * 3,4);

    if ( saves_spell( level, ch,victim,DAM_COLD) )
	dam = dice(level * 3,3);

    damage( ch, victim, dam, sn, DAM_COLD ,TRUE, FALSE, NULL );
    return TRUE;
}

bool spell_corrupt_potion( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->pIndexData->item_type != ITEM_POTION )
    {
	act( "$p is not a potion.", ch, obj, NULL, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    if ( affect_find( obj->affected, sn ) != NULL )
    {
	act( "$p has already been corrupted.",
	    ch, obj, NULL, TO_CHAR, POS_STANDING );
	return FALSE;
    }

    af.where	= TO_OBJECT;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= -1;
    af.location	= APPLY_NONE;
    af.modifier	= 0;
    af.bitvector= 0;

    affect_to_obj( obj, &af );

    obj->value[0] = level;
    obj->value[1] = gsn_weaken;
    obj->value[2] = gsn_poison;
    obj->value[3] = gsn_curse;
    obj->value[4] = gsn_plague;

    act( "You corrupt $p.", ch, obj, NULL, TO_CHAR, POS_STANDING );
    act( "$n corrupts $p.", ch, obj, NULL, TO_ROOM, POS_STANDING );

    return TRUE;
}

bool spell_power_of_gods( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if ( victim == ch )
	    send_to_char( "You can't get much more powerful than you are.\n\r", ch );
	else
	    act( "$N is about as powerful as $E is going to get.",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= level / 10;
    af.bitvector= 0;
    af.modifier	= level * 5 / 4;
    af.location	= APPLY_HITROLL;
    affect_to_char( victim, &af );

    af.location	= APPLY_DAMROLL;
    affect_to_char( victim, &af );

    af.modifier	= level * 5;
    af.location	= APPLY_HIT;
    affect_to_char( victim, &af );

    send_to_char( "Your muscles surge with godlike power.\n\r", victim );
    act( "$n's muscles surge with godlike power.",
	victim, NULL, NULL, TO_ROOM, POS_RESTING );
    return TRUE;
}

bool spell_cure_weaken( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_weaken ) )
    {
	if ( victim == ch )
	    send_to_char( "You aren't feeling very weak right now.\n\r", ch );
	else
	    act( "$N doesn't look too weak right now.",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    if ( check_dispel( level, victim, gsn_weaken ) )
        act( "$n looks a little stronger.",
	    victim, NULL, NULL, TO_ROOM, POS_RESTING );
    else
	send_to_char( "Spell failed.\n\r", ch );

    return TRUE;
}

bool spell_golden_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if ( victim == ch )
	    send_to_char( "Your armor already glows with a golden aura.\n\r", ch );
	else
	    act( "$N's armor already glows with a golden aura.",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	= TO_DAM_MODS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= level / 10;
    af.bitvector= 0;
    af.modifier	= -level / 7;
    af.location	= DAM_SLASH;
    affect_to_char( victim, &af );

    af.location	= DAM_PIERCE;
    affect_to_char( victim, &af );

    af.location	= DAM_BASH;
    affect_to_char( victim, &af );

    send_to_char( "Your armor glows with a golden aura.\n\r", victim );
    act( "$n's armor glows with a golden aura.",
	victim, NULL, NULL, TO_ROOM, POS_RESTING );
    return TRUE;
}

bool spell_absorb_magic( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( IS_SHIELDED( victim, SHD_ABSORB ) )
    {
	if ( ch == victim )
	    send_to_char( "You are already able to absorb magic.\n\r", ch );
	else
	    send_to_char( "They are already able to absorb magic.\n\r", ch );
	return FALSE;
    }

    af.where	= TO_SHIELDS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= perm_affect ? -1 : level/20;
    af.location	= APPLY_NONE;
    af.modifier	= 0;
    af.bitvector= SHD_ABSORB;
    affect_to_char( victim, &af );

    act( "$N is surrounded by a strange {^mystical aura{x.", ch, NULL, victim, TO_NOTVICT, POS_RESTING );
    send_to_char( "You are surrounded by a {^mystical absorbing aura{x.\n\r", victim  );
    return TRUE;
}

bool spell_kailfli( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if ( is_affected( victim, sn ) )
    {
	if ( victim == ch )
	    send_to_char( "You are already filled with chi.\n\r", ch );
	else
	    act( "$N already has the aura of chi.", ch, NULL, victim, TO_CHAR, POS_RESTING );
        return FALSE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= perm_affect ? -1 : 3 + ( level / 20 );
    af.location	= APPLY_HITROLL;
    af.modifier	= 7 + level / 20;
    af.bitvector= 0;
    affect_to_char( victim, &af );

    af.location	= APPLY_DAMROLL;
    affect_to_char( victim, &af );

    send_to_char( "{WChi {^surronds {xyour soul.\n\r", victim );

    if ( ch != victim )
	act( "You grant $N your {^blessing{x of {Wluck{x.", ch, NULL, victim, TO_CHAR, POS_RESTING );

    return TRUE;
}

bool spell_flash( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch, *vch_next;
    AFFECT_DATA af;

    send_to_char( "You dispel a {&white {xbright {#flash{x from your hands into the room.\n\r", ch );
    act( "A {&white {xbright {#flash{x dispels from $n's hands, flooding the room.", ch, NULL, NULL, TO_ROOM, POS_RESTING );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	if ( IS_AFFECTED( vch, AFF_BLIND )
	||   is_safe_spell( ch, vch, TRUE )
	||   !check_kill_steal( ch, vch, FALSE ) )
	    continue;

	if ( saves_spell( level * 7 / 6, ch, vch, DAM_LIGHT ) )
	{
	    act( "$N resists the light.", ch, NULL, vch, TO_CHAR, POS_RESTING );
	    continue;
	}

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.location	= APPLY_HITROLL;
	af.modifier	= -level;
	af.dur_type	= DUR_ROUNDS;
	af.duration	= number_range( 2, 4 );
	af.bitvector	= AFF_BLIND;
        affect_to_char( vch, &af );

        send_to_char( "You are blinded by the White light!\n\r", vch );
        act("$n appears to be blinded by the flash.",vch,NULL,NULL,TO_ROOM,POS_RESTING );

	if ( vch->fighting == NULL )
	    multi_hit( vch, ch, TYPE_UNDEFINED, TRUE );
    }

    fire_effect( ch, ch->in_room, level, 0, TARGET_ROOM );

    return TRUE;
}

bool spell_diminish( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == NULL || victim == ch )
    {
	send_to_char( "Do you really want to do that?\n\r", ch );
	return FALSE;
    }

    if ( is_affected( victim, sn ) )
    {
	act( "$N is already weakened.", ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    if ( saves_spell( level * 5 / 4, ch, victim, DAM_NEGATIVE ) )
    {
	send_to_char( "Spell failed.\n\r", ch );
	return TRUE;
    }

    af.where	= TO_DAM_MODS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_ROUNDS;
    af.duration	= 20;
    af.location	= DAM_ALL;
    af.modifier	= 10;
    af.bitvector= 0;
    affect_to_char( victim, &af );

    af.location = DAM_LIGHTNING;
    affect_to_char( victim, &af );

    send_to_char( "{8You no longer feel safe from a dull butter knife.{x\n\r", victim );
    act( "$N is weakened by your magic.", ch, NULL, victim, TO_CHAR, POS_RESTING );
    return TRUE;
}

bool spell_decrement( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
	send_to_char( "You really don't want to do that.\n\r", ch );
	return FALSE;
    }

    if ( is_affected( victim, sn ) )
    {
	act( "$N is already weakened by decrement.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
        return FALSE;
    }

    if ( saves_spell( level * 5 / 4, ch, victim, DAM_NEGATIVE ) )
    {
	send_to_char( "Spell failed.\n\r", ch );
	return TRUE;
    }

    af.where	= TO_DAM_MODS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_ROUNDS;
    af.duration	= number_range( 5, 10 );
    af.location	= DAM_ALL;
    af.modifier	= number_range( 20, 30 );
    af.bitvector= 0;
    affect_to_char( victim, &af );

    act( "$n is surrounded by a {8black {^aura{x.",
	victim, NULL, NULL, TO_ROOM, POS_RESTING );

    send_to_char( "You are surrounded by a {8black {^aura{x.\n\r", victim );
    return TRUE;
}

bool spell_groupheal( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( ch, gch ) )
	    spell_heal( gsn_heal, level, ch, (void *) gch, TARGET_CHAR );
    }

    return TRUE;
}

bool spell_project_force( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, pos;

    for ( pos = 0; pos < 2; pos++ )
    {
	if ( saves_spell( level, ch, victim, DAM_MENTAL ) )
	    dam = dice( 7, 9 ) + level;
	else
	    dam = ( dice( 7, 9 ) + level ) / 2;

	damage( ch, victim, dam, sn, DAM_MENTAL, TRUE, FALSE, NULL );
    }

    return TRUE;
}

bool spell_psi_twister( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch, *vch_next;
    int dam, pos;

    send_to_char( "{cYou release your mental powers to do its will in the room!{x\n\r", ch );
    act( "$n levitates amid a swirl of {8psychic {cenergy{x.", ch, NULL, NULL, TO_ROOM, POS_RESTING );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	if ( !is_safe_spell( ch, vch, TRUE )
	&&   check_kill_steal( ch, vch, FALSE ) )
	{
	    for ( pos = 0; pos < 2; pos++ )
	    {
		dam = dice( level, 8 ) + ( level * .5 );
		if ( saves_spell( level * 3/2, ch, vch, DAM_MENTAL ) )
		    dam *= 0.5;

		damage( ch, vch, dam, sn, DAM_MENTAL, TRUE, FALSE, NULL );
	    }
        }
    }

    return TRUE;
}

bool spell_mindblast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, pos;

    for ( pos = 0; pos < 2; pos++ )
    {
	dam = UMAX( 50, victim->hit - dice( 14, 20 ) );
	if ( saves_spell( level, ch, victim, DAM_MENTAL ) )
	    dam = UMIN( 60, dam / 2 );
	dam = UMIN( 200, dam );

	damage( ch, victim, dam, sn, DAM_MENTAL, TRUE, FALSE, NULL );
    }

    return TRUE;
}

bool spell_psi_barrier( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if ( victim == ch )
	    send_to_char( "You are already surrounded by a barrier.\n\r", ch );
	else
	    act( "$N is already surrounded by a barrier.",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	= TO_DAM_MODS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= perm_affect ? -1 : level / 20;
    af.location	= DAM_MENTAL;
    af.modifier	= -25;
    af.bitvector= 0;
    affect_to_char( victim, &af );

    act( "$n is surrouned by a {8psychic {xbarrier.",
	victim, NULL, NULL, TO_ROOM, POS_RESTING );
    send_to_char( "{cYou are surrouned by a psychic barrier{x.\n\r", victim );
    return TRUE;
}

bool spell_vigorize( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if ( victim == ch )
	    send_to_char( "You don't feel any more vigorized.\n\r", ch );
	else
	    act( "$N doesn't look any more vigorized.",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= level / 25;
    af.location	= APPLY_HIT;
    af.bitvector= 0;

    if ( victim->level < 15 )
	af.modifier = victim->max_hit/5 + 150;
    else
	af.modifier = UMIN( victim->max_hit/5, 32767 );

    affect_to_char( victim, &af );

    victim->hit += af.modifier;

    if ( victim->hit > victim->max_hit )
	victim->hit = victim->max_hit;

    act( "$n has been vigorized.", victim, NULL, NULL, TO_ROOM, POS_RESTING );
    send_to_char( "{cYou feel much more vigorized{x.\n\r", victim );
    return TRUE;
}

bool spell_life_curse( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == NULL )
	return FALSE;

    if ( is_affected( victim, sn ) )
    {
	act( "$N's life has already been cursed.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    if ( saves_spell( level, ch, victim, DAM_NEGATIVE ) )
    {
	send_to_char( "Spell failed.\n\r", ch );
	return TRUE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= level / 25;
    af.location	= APPLY_HIT;
    af.modifier = UMAX( -victim->max_hit/5, -32767 );
    af.bitvector= 0;

    affect_to_char( victim, &af );

    victim->hit -= af.modifier;

    if ( victim->hit > victim->max_hit )
	victim->hit = victim->max_hit;

    act( "$n has had $s lifeforce cursed.", victim, NULL, NULL, TO_ROOM, POS_RESTING );
    send_to_char( "{cYou feel that your life has been cursed{x.\n\r", victim );
    return TRUE;
}

bool spell_energy_curse( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == NULL )
	return FALSE;

    if ( is_affected( victim, sn ) )
    {
	act( "$N's energy has already been cursed.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    if ( saves_spell( level, ch, victim, DAM_NEGATIVE ) )
    {
	send_to_char( "Spell failed.\n\r", ch );
	return TRUE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= level / 25;
    af.location	= APPLY_MANA;
    af.modifier = UMAX( -victim->max_mana/5, -32767 );
    af.bitvector= 0;

    affect_to_char( victim, &af );

    victim->mana -= af.modifier;

    if ( victim->mana > victim->max_mana )
	victim->mana = victim->max_mana;

    act( "$n has had $s energy cursed.", victim, NULL, NULL, TO_ROOM, POS_RESTING );
    send_to_char( "{cYou feel that your energy has been cursed{x.\n\r", victim );
    return TRUE;
}

bool spell_stamina_curse( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == NULL )
	return FALSE;

    if ( is_affected( victim, sn ) )
    {
	act( "$N's stamina has already been cursed.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    if ( saves_spell( level, ch, victim, DAM_NEGATIVE ) )
    {
	send_to_char( "Spell failed.\n\r", ch );
	return TRUE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= level / 25;
    af.location	= APPLY_MOVE;
    af.modifier = UMAX( -victim->max_move/5, -32767 );
    af.bitvector= 0;

    affect_to_char( victim, &af );

    victim->move -= af.modifier;

    if ( victim->move > victim->max_move )
	victim->move = victim->max_move;

    act( "$n has had $s stamina cursed.", victim, NULL, NULL, TO_ROOM, POS_RESTING );
    send_to_char( "{cYou feel that your stamina has been cursed{x.\n\r", victim );
    return TRUE;
}

bool spell_meteor_swarm( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch, *vch_next;
    int dam, pos;

    send_to_char( "Meteors crash into the Earth!\n\r", ch );
    act( "$n raises his hands, provoking meteors towards the Earth.",
	ch, NULL, NULL, TO_ROOM, POS_RESTING );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	send_to_char( "The earth shakes as deadly meteors crash into the ground.\n\r", vch );

	if ( !is_safe_spell( ch, vch, TRUE )
	&&   check_kill_steal( ch, vch, FALSE ) )
	{
	    for ( pos = 0; pos < 6; pos++ )
	    {
		if ( IS_AFFECTED( vch, AFF_FLYING ) )
		    dam = dice( 18, 20 ) + level;
		else
		    dam = dice( 15, 18 ) + level;

		if ( saves_spell( level, ch, vch, DAM_BASH ) )
		    dam *= 0.5;

		damage( ch, vch, dam, sn, DAM_BASH, TRUE, FALSE, NULL );
	    }
	}
    }

    return TRUE;
}

bool spell_hurricane( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch, *vch_next;
    int dam, pos;

    if ( !IS_OUTSIDE( ch ) )
    {
	send_to_char( "You must be out of doors.\n\r", ch );
	return FALSE;
    }

    if ( weather_info.sky < SKY_RAINING )
    {
	send_to_char( "But, there are no storm clouds in the sky.\n\r", ch );
	return FALSE;
    }

    send_to_char( "You conjure a deadly hurricane!\n\r", ch );
    act( "$n begins chanting, a DEADLY Gust of wind from a Hurricane tears the area in pieces.",
	ch, NULL, NULL, TO_ROOM, POS_RESTING );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next;

	if ( vch->in_room == ch->in_room )
	{
	    if ( !is_safe_spell( ch, vch, TRUE ) )
	    {
		for ( pos = 0; pos < 5; pos++ )
		{
		    if ( IS_AFFECTED( vch, AFF_FLYING ) )
			dam = dice( 12, 14 ) + level;
		    else
			dam = dice( 14, 14 ) + level;

		    if ( saves_spell( level, ch, vch, DAM_WATER ) )
			dam *= 0.5;

		    damage( ch, vch, dam, sn, DAM_WATER, TRUE, FALSE, NULL );
		}
	    }
	}

	else if ( vch->in_room->area == ch->in_room->area )
	    send_to_char( "{8A deadly hurricane seems to be approaching.{x\n\r", vch );
    }

    return TRUE;
}

bool spell_feeble_mind( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
	send_to_char( "Why would you want to do that?\n\r", ch );
	return FALSE;
    }

    if ( is_affected( victim, sn ) )
    {
	act( "$N is already affected.", ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    if ( saves_spell( level, ch, victim, DAM_MENTAL ) )
    {
	send_to_char( "Spell failed.\n\r", ch );
	return TRUE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= level/10;
    af.modifier	= (level/30 + 4) * -1;
    af.location	= APPLY_INT;
    af.bitvector= 0;
    affect_to_char( victim, &af );

    af.location	= APPLY_WIS;
    affect_to_char( victim, &af );

    send_to_char( "You all of sudden feel really dumb.\n\r", victim );
    act( "$N is looking a bit dumb today.", ch, NULL, victim, TO_CHAR, POS_RESTING );
    return TRUE;
}

bool spell_hail_storm( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch, *vch_next;
    int dam, pos;

    act( "$n conjures a {8deadly {Whail storm{x!", ch, NULL, NULL, TO_ROOM, POS_RESTING );
    send_to_char( "You conjure a {8deadly {Whail storm{x!\n\r", ch );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	if ( !is_safe_spell( ch, vch, TRUE ) )
	{
	    for ( pos = 0; pos < 4; pos++ )
	    {
		dam = dice( 8, 8 ) + level;

		if ( saves_spell( level, ch, vch, DAM_COLD ) )
		    dam *= 0.5;

		damage( ch, vch, dam, sn, DAM_COLD, TRUE, FALSE, NULL );
	    }
	}
    }

    return TRUE;
}

bool spell_divine_truth( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch, *vch_next;
    AFFECT_DATA af;

    send_to_char( "You dispel a {Wstun flash{x from your hands into the room.\n\r", ch );
    act( "A {Wstun flash{x dispels from $n's hands, stunning the room.",
	ch, NULL, NULL, TO_ROOM, POS_RESTING );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	if ( is_affected( vch, sn )
	||   is_safe_spell( ch, vch, TRUE )
	||   !check_kill_steal( ch, vch, FALSE ) )
	    continue;

	if ( saves_spell( level * 7 / 6, ch, vch, DAM_LIGHT ) )
	{
	    act( "$N resists the light.", ch, NULL, vch, TO_CHAR, POS_RESTING );
	    continue;
	}


	if ( vch->fighting == NULL )
	    multi_hit( vch, ch, TYPE_UNDEFINED, TRUE );

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.dur_type	= DUR_TICKS;
	af.duration	= number_range( 3, 6 );
	af.bitvector	= 0;
        affect_to_char( vch, &af );
	vch->stunned = 3;

        send_to_char( "You have been stunned.\n\r", vch );
        act( "$n appears to be stunned.", vch, NULL, NULL, TO_ROOM, POS_RESTING );
    }

    return TRUE;
}

bool spell_iron_skin( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if ( victim == ch )
	    send_to_char( "Your skin is already as hard as iron.\n\r", ch );
	else
	    act( "$N's skin is already as hard as iron.",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= level/10;
    af.location	= APPLY_AC;
    af.modifier	= -60;
    af.bitvector= 0;
    affect_to_char( victim, &af );

    act( "$n's skin turns to {8iron{x.", victim, NULL, NULL, TO_ROOM, POS_RESTING );
    send_to_char( "Your skin turns to {8iron{x.\n\r", victim );
    return TRUE;
}

bool spell_luminrati( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, hp_dam, dice_dam, hpch;

    act( "{@$n {!has summoned the {#I{3N{#C{3R{#E{3D{#I{3B{#L{3E {!powers of {$L{4u{$m{4i{$n{4r{$a{4t{$i {!upon {@$N{!.{x",
	ch, NULL, victim, TO_NOTVICT, POS_RESTING );
    act( "{@$n {!has summoned the {$L{4u{$m{4i{$n{4r{$a{4t{$i {!against you!{x",
	ch, NULL, victim, TO_VICT, POS_RESTING );
    act( "{!You have summoned the {$L{4u{$m{4i{$n{4r{$a{4t{$i {!against {@$N{!.{x",
	ch, NULL, victim, TO_CHAR, POS_RESTING );

    hpch = UMAX( 10, ch->max_mana );
    hp_dam = number_range( hpch / 9 + 1, hpch / 4 );
    dice_dam = dice( level, 18 );
    dam = UMAX( hp_dam + dice_dam, dice_dam + hp_dam );

    if ( saves_spell( level, ch, victim, DAM_LIGHTNING ) )
	dam /= 2;

    damage( ch, victim, dam, sn, DAM_LIGHTNING, TRUE, FALSE, NULL );

    hp_dam = number_range( hpch / 9 + 1, hpch / 4 );
    dice_dam = dice( level, 18 );
    dam = UMAX( hp_dam + dice_dam, dice_dam + hp_dam );

    if ( saves_spell( level, ch, victim, DAM_LIGHTNING ) )
	dam /= 2;

    damage( ch, victim, dam, sn, DAM_LIGHTNING, TRUE, FALSE, NULL );

    shock_effect( ch, victim, level / 2, dam / 3, TARGET_CHAR );
    return TRUE;
}

bool spell_memory_lapse( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
	send_to_char( "That wouldn't be a smart thing to do.\n\r", ch );
	return FALSE;
    }

    if ( is_affected( victim, sn ) )
    {
	act( "$N can't get any worse.", ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.location	= APPLY_NONE;
    af.modifier	= 0;
    af.dur_type	= DUR_TICKS;
    af.duration	= 1;
    af.bitvector= 0;
    affect_to_char( victim, &af );

    victim->remember = number_range( 5, 10 );
    victim->stunned = 3;

    act( "$n screams in terror, grabs his head in pain and falls to the ground confused.",
	victim, NULL, NULL, TO_ROOM, POS_RESTING );

    send_to_char( "{2Your head {Y{zBURSTS {2into chaos, causing you to lose track of the world around you.{x\n\r", victim );

    damage( ch, victim, dice( 5, 10 ), sn, DAM_MENTAL, TRUE, FALSE, NULL );

    return TRUE;
}

bool spell_negate_alignment( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *)vo;
    int flags_to_rm = 0;
    int risk = 10;

    if ( IS_GOOD( ch ) )
    {
	if ( IS_SET( obj->extra_flags, ITEM_EVIL ) )
	    flags_to_rm += ITEM_EVIL;

	if ( IS_SET( obj->extra_flags, ITEM_ANTI_GOOD ) )
	    flags_to_rm += ITEM_ANTI_GOOD;
    }

    else if ( IS_EVIL( ch ) )
    {
	if ( IS_SET( obj->extra_flags, ITEM_ANTI_EVIL ) )
	    flags_to_rm += ITEM_ANTI_EVIL;
    }

    else
    {
	if ( IS_SET( obj->extra_flags, ITEM_ANTI_NEUTRAL ) )
	    flags_to_rm += ITEM_ANTI_NEUTRAL;
    }

    if ( !flags_to_rm )
    {
	send_to_char( "Your gods find nothing offensive about this item.\n\r", ch );
	return FALSE;
    }

    if ( obj->level > ( ch->level + 10 ) )
	risk += 5 * ( obj->level - ( ch->level + 10 ) );

    if ( number_percent( ) <= risk )
    {
	send_to_char( "{rYou have offended your gods! {mKaboom!  {rThe item explodes!  *sigh*{x\n\r", ch );
	act( "{r$p shivers violently and explodes!{x", ch, obj, NULL, TO_ROOM, POS_RESTING );
	extract_obj( obj );
    }

    else if ( number_percent( ) < ( ch->level * 2 / 3 + ( get_curr_stat( ch, STAT_WIS ) - 20 ) ) )
    {
	send_to_char( "{cYour gods have favored you...they negate the alignment of the item. {x\n\r", ch );
	act( "{c$p glows with the color of neutrality{x", ch, obj, NULL, TO_ROOM, POS_RESTING );
	obj->extra_flags -= flags_to_rm;
	obj->timer = level * 3;
    }

    else
	send_to_char( "The item resists your efforts at negation.\n\r", ch );

    return TRUE;
}

bool spell_soul_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *)vo, *sblade;

    if ( obj == NULL )
	return FALSE;

    if ( obj->item_type != ITEM_CORPSE_NPC )
    {
	if ( obj->item_type == ITEM_CORPSE_PC )
	    send_to_char( "Player's souls are protected by the gods.\n\r", ch );
	else
	    send_to_char( "It would serve no purpose...\n\r", ch );
        return FALSE;
    }

    if ( obj->level > ( level + 10 ) )
    {
	send_to_char( "You cannot forge such a powerful soul into a blade.\n\r", ch );
	return FALSE;
    }

    if ( ( sblade = create_object( get_obj_index( OBJ_VNUM_SOULBLADE ) ) ) == NULL )
    {
	send_to_char( "Error: obj vnum missing.\n\r", ch );
	return FALSE;
    }

    sblade->level	= level;
    sblade->value[1]	= ( obj->level / 20 ) + ( level / 20 ) + 5;
    sblade->value[2]	= ( obj->level / 20 ) + ( level / 20 ) + 4;
    sblade->timer	= level / 2;

    obj_to_char( sblade, ch );

    act( "$n steals the {8soul {xfrom $P, and forges $p.",
	ch, sblade, obj, TO_ROOM, POS_RESTING );
    act( "You steal the {8soul {xfrom $P, and forge $p.",
	ch, sblade, obj, TO_CHAR, POS_RESTING );

    extract_obj( obj );

    return TRUE;
}

bool spell_transfix( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA *)vo;

    if ( !victim )
	return FALSE;

    if ( is_affected( victim, sn ) )
    {
	act( "{h$N {his already distracted by your display of lights.{x",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );	
	return FALSE;
    }

    if ( ch->pcdata && ch->pcdata->pktimer > 0 )
    {
	send_to_char( "Your heart's pounding to fast!\n\r", ch );
	return FALSE;
    }

    if ( victim == ch )
    {
	send_to_char( "You try to transfix yourself, you now move very slowly.\n\r", ch );
	return FALSE;
    }

    if ( saves_spell( level, ch, victim, DAM_MENTAL ) )
    {
	act( "{h$N {hisn't distracted by your display of lights!{x",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	act( "{h$n {htries to transfix you, but you ignore his attempt!{x",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	combat( "{k$n tries to transfix $N{k, but fails.{x",
	    ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
	return TRUE;
    }

    act( "{i$n {itransfixes you with a dazzling display of pretty lights.  Ahhh...{x",
	ch, NULL, victim, TO_VICT, POS_DEAD );
    act( "{hYou transfix $N {hwith a brilliant display of lights!{x",
	ch, NULL, victim, TO_CHAR, POS_RESTING );
    combat( "{k$n {ktransfixes $N {kwith a brilliant display of lights.{x",
	ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );

    damage( ch, victim, 10, sn, DAM_MENTAL, TRUE, FALSE, NULL );

    af.where	= TO_AFFECTS;
    af.type	= sn;
    af.level	= level;
    af.dur_type	= DUR_TICKS;
    af.duration	= number_range( 0, 1 );
    af.location	= 0;
    af.modifier	= 0;
    af.bitvector= 0;
    affect_to_char( victim, &af );

    WAIT_STATE( victim, PULSE_VIOLENCE * 3 );
    return TRUE;
}

bool spell_smite_health( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( victim == NULL )
	return FALSE;

    victim->hit = 1;
    update_pos( victim );

    send_to_char( "{RYou have been smited nearly to death!{x\n\r", victim );

    return TRUE;
}

bool spell_smite_magic( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( victim == NULL )
	return FALSE;

    victim->mana = 1;
    update_pos( victim );

    send_to_char( "{RYou no longer feel like casting any spells!{x\n\r", victim );

    return TRUE;
}

bool spell_smite_movement( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( victim == NULL )
	return FALSE;

    victim->move = 1;
    update_pos( victim );

    send_to_char( "{RYou no longer feel like moving around!{x\n\r", victim );

    return TRUE;
}

