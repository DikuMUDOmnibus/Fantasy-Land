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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"

DECLARE_DO_FUN( do_clantalk	);
DECLARE_DO_FUN( do_draw		);
DECLARE_DO_FUN( do_emote	);
DECLARE_DO_FUN( do_get		);
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_match	);
DECLARE_DO_FUN( do_sacrifice	);
DECLARE_DO_FUN( do_say		);
DECLARE_DO_FUN( do_yell		);

DECLARE_SPELL_FUN( spell_null	);

void	check_gquest	args( ( CHAR_DATA *killer, CHAR_DATA *victim ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    check_war       args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_arena	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary ) );
bool	check_dispel	args( ( int dis_level, CHAR_DATA *victim, int sn ) );
OBJ_DATA * rand_obj	args( ( int level ) );

bool check_stun_remember( CHAR_DATA *ch )
{
    if ( ch->stunned )
    {
	send_to_char( "You are too stunned to do that right now.\n\r", ch );
	return FALSE;
    }

    if ( ch->remember )
    {
	send_to_char( "You can not remember how to perform that right now.\n\r", ch );
	return FALSE;
    }

    return TRUE;
}

void add_recent( PKILL_RECORD *pk_record )
{
    PKILL_RECORD *pk, *pk_next;
    sh_int max_recent = 20;

    pk_record->next	= recent_list;
    recent_list		= pk_record;

    for ( pk = recent_list; pk != NULL; pk = pk_next )
    {
	pk_next	= pk->next;

	max_recent--;

	if ( max_recent <= 0 )
	{
	    if ( pk_next != NULL )
		pk_next = pk->next->next;

	    free_pk_record( pk );
	}
    }

    mud_stat.changed = TRUE;
}

bool is_voodood( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *object;
    bool found;

    if ( ch->level > HERO )
	return FALSE;

    found = FALSE;
    for ( object = victim->carrying; object != NULL; object = object->next_content )
    {
	if (object->pIndexData->vnum == OBJ_VNUM_VOODOO)
	{
	    char arg[MAX_INPUT_LENGTH];

	    one_argument(object->name, arg);
	    if (!str_cmp(arg, ch->name))
		return TRUE;
	}
    }

    return FALSE;
}

bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( victim->in_room == NULL || ch->in_room == NULL )
	return TRUE;

    if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA )
    &&   IS_SET( victim->in_room->room_flags, ROOM_ARENA ) )
    {
	if ( IS_NPC( ch ) )
	{
	    if ( ch->master != NULL && !IS_NPC( ch->master ) )
		ch = ch->master;
	    else
		return FALSE;
	}

	if ( IS_NPC( victim ) )
	{
	    if ( victim->master != NULL && !IS_NPC( victim->master ) )
		victim = victim->master;
	    else
		return FALSE;
	}

	if ( ch->pcdata->match != victim->pcdata->match )
	    return TRUE;

	if ( ch->pcdata->team == victim->pcdata->team )
	{
	    act( "You and $N are both on the SAME team!",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	    return TRUE;
	}

        return FALSE;
    }

    if ( victim->fighting == ch
    ||   victim == ch
    ||   IS_IMMORTAL( ch ) )
	return FALSE;

    if ( IS_SET( victim->in_room->room_flags, ROOM_SAFE )
    &&   ( !victim->pcdata || victim->pcdata->pktimer <= 0 ) )
    {
	send_to_char( "Not in this room.\n\r", ch );
	return TRUE;
    }

    if ( IS_NPC( victim ) )
    {
	if ( victim->pIndexData->pShop != NULL )
	{
	    send_to_char( "The shopkeeper wouldn't like that.\n\r", ch );
	    return TRUE;
	}

	if ( IS_SET( ch->in_room->room_flags, ROOM_PET_SHOP ) )
	{
	    send_to_char( "Noise in this room might upset the animals.\n\r", ch );
	    return TRUE;
 	}

	if ( victim->clan == 0
	&&   ( IS_SET( victim->act, ACT_TRAIN )
	||     IS_SET( victim->act, ACT_PRACTICE )
	||     IS_SET( victim->act, ACT_IS_HEALER )
	||     IS_SET( victim->act, ACT_IS_SATAN )
	||     IS_SET( victim->act, ACT_IS_PRIEST ) ) )
	{
	    act( "I don't think $G would approve.",
		ch, NULL, NULL, TO_CHAR, POS_RESTING );
	    return TRUE;
	}

	if ( !IS_NPC( ch ) )
	{
	    if ( IS_SET( victim->act, ACT_PET ) )
	    {
		act( "But $N looks so cute and cuddly...",
		    ch, NULL, victim, TO_CHAR, POS_RESTING );
		return TRUE;
	    }

	    if ( IS_AFFECTED( victim, AFF_CHARM ) && ch != victim->master )
	    {
		send_to_char( "You don't own that monster.\n\r", ch );
		return TRUE;
	    }
	}
    } else {
	if ( IS_NPC( ch ) )
	{
	    if ( victim->pcdata->dtimer > 0 )
		return TRUE;

	    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master != NULL
	    &&   ch->master->fighting != victim )
	    {
		send_to_char( "Players are your friends!\n\r", ch );
		return TRUE;
	    }
	} else {
	    PKILL_DATA *pkill;

	    if ( victim->in_room->area->clan != 0
	    &&   victim->in_room->area->clan != victim->clan
	    &&   ch->in_room->area->clan == ch->clan )
		return FALSE;

	    if ( IS_SET( victim->act, PLR_TWIT )
	    ||   is_voodood( ch, victim ) )
		return FALSE;

	    for ( pkill = ch->pcdata->recent_pkills; pkill != NULL; pkill = pkill->next )
	    {
		if ( !str_cmp( victim->name, pkill->player_name ) )
		{
		    if ( pkill->killer )
			send_to_char("You have already killed that person once within the last hour!\n\r",ch);
		    else
			send_to_char("That person has already killed you once within the past hour!\n\r",ch);
		    return TRUE;
		}
	    }

	    if ( ch->level < 20 || victim->level < 20 )
	    {
		send_to_char( "Wait until both parties are level 20.\n\r", ch );
		return TRUE;
	    }

	    if ( ch->pcdata->on_quest || victim->pcdata->on_quest )
	    {
		send_to_char( "Quest flag prevents player killing.\n\r", ch );
		return TRUE;
	    }

	    if ( !is_clan( ch ) )
	    {
		send_to_char( "Join a clan if you want to fight players.\n\r", ch );
		return TRUE;
	    }

	    if ( !is_pkill( ch ) )
	    {
		send_to_char( "Your clan does not allow player fighting.\n\r", ch );
		return TRUE;
	    }

	    if ( !is_clan( victim ) )
	    {
		send_to_char( "They aren't in a clan, leave them alone.\n\r", ch );
		return TRUE;
	    }

	    if ( !is_pkill( victim ) )
	    {
		send_to_char( "They are in a no pkill clan, leave them alone.\n\r", ch );
		return TRUE;
	    }

	    if ( ch->pcdata->tier == victim->pcdata->tier )
	    {
		if ( ch->level > victim->level + 15
		||   ch->level < victim->level - 15 )
		{
		    send_to_char( "Pick on someone your own size.\n\r", ch );
		    return TRUE;
		}
	    }

	    if ( ch->pcdata->tier - victim->pcdata->tier == 1
	    ||   ch->pcdata->tier - victim->pcdata->tier == -1 )
	    {
		if ( ch->level > victim->level + 10
		||   ch->level < victim->level - 10 )
		{
		    send_to_char( "Pick on someone your own size.\n\r", ch );
		    return TRUE;
		}
	    }

	    if ( ch->pcdata->tier - victim->pcdata->tier == 2
	    ||   ch->pcdata->tier - victim->pcdata->tier == -2 )
	    {
		if ( ch->level > victim->level + 5
		||   ch->level < victim->level - 5 )
		{
		    send_to_char( "Pick on someone your own size.\n\r", ch );
		    return TRUE;
		}
	    }

	    if ( victim->pcdata->dtimer > 0 )
	    {
		send_to_char( "But they have just been killed!\n\r", ch );
		return TRUE;
	    }
	}
    }

    if ( ch->pcdata && ch->pcdata->dtimer > 0 )
    {
	send_to_char( "You just lost your {RKilled{x flag.\n\r", ch );
	ch->pcdata->dtimer = 0;
    }

    return FALSE;
}

void check_assist( CHAR_DATA *ch, CHAR_DATA *victim )
{
    CHAR_DATA *rch, *rch_next;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch_next )
    {
	rch_next = rch->next_in_room;

	if ( IS_AWAKE( rch ) && rch->fighting == NULL )
	{
	    if ( !IS_NPC( ch ) && IS_NPC( rch )
	    &&   IS_SET( rch->act, ACT_ASSIST_PLAYERS )
	    &&   rch->level + 6 > victim->level )
	    {
		do_emote( rch, "{Rscreams and attacks!{x" );
		mobile_attack( rch, victim );
		continue;
	    }

	    /* PC Cases */
	    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) )
	    {
		if ( !IS_NPC( victim ) )
		    break;

		if ( ( ( !IS_NPC( rch )
		&&     IS_SET( rch->act, PLR_AUTOASSIST ) )
		||     IS_AFFECTED( rch, AFF_CHARM ) )
		&&     is_same_group( ch, rch )
		&&     !IS_AFFECTED( rch, AFF_BLIND )
		&&     !is_safe( rch, victim ) )
		    multi_hit( rch, victim, TYPE_UNDEFINED, TRUE );

		continue;
	    }

	    /* now check the NPC cases */
 	    if ( IS_NPC( ch ) && !IS_AFFECTED( ch, AFF_CHARM ) && IS_NPC( rch ) )
	    {
		if ( IS_SET( rch->act, ACT_ASSIST_ALL )
		||   ( rch->group && rch->group == ch->group )
		||   ( rch->race == ch->race
		&&     IS_SET( rch->act, ACT_ASSIST_RACE ) )
		||   ( IS_SET( rch->act, ACT_ASSIST_ALIGN )
		&&     ( ( IS_GOOD( rch )    && IS_GOOD( ch ) )
		||       ( IS_EVIL( rch )    && IS_EVIL( ch ) )
		||       ( IS_NEUTRAL( rch ) && IS_NEUTRAL( ch ) ) ) )
		||   ( rch->pIndexData == ch->pIndexData
		&&     IS_SET( rch->act, ACT_ASSIST_VNUM ) ) )
	   	{
		    CHAR_DATA *vch, *target = NULL;
		    int number = 0;

		    if ( number_bits( 1 ) == 0 )
			continue;

		    for ( vch = ch->in_room->people; vch; vch = vch->next )
		    {
			if ( can_see( rch, vch )
			&&   is_same_group( vch, victim )
			&&   number_range( 0, number ) == 0 )
			{
			    target = vch;
			    number++;
			}
		    }

		    if ( target != NULL )
		    {
			do_emote( rch, "{Rscreams and attacks!{x" );
			mobile_attack( rch, target );
		    }
		}
	    }
	}
    }
}

void check_troll_revival( CHAR_DATA *ch )
{
    sh_int chance;

    if ( ( chance = get_skill( ch, gsn_troll_revival ) ) == 0 )
	return;

    if ( ( 4 * chance / 5 ) > number_percent( ) )
    {
	ch->hit += ( number_range( 3 * ch->level / 4, 3 * ch->level / 2 ) );
	update_pos( ch );
	check_improve( ch, gsn_troll_revival, TRUE, 20 );
    }
    else
	check_improve( ch, gsn_troll_revival, FALSE, 20 );
}

void violence_update( void )
{
    CHAR_DATA *ch, *ch_next, *victim;
    OBJ_DATA *obj, *obj_next;
    bool room_trig = FALSE;
    int sn;

    sn = -1;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next	= ch->next;

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

	if ( IS_AWAKE( ch ) && ch->in_room == victim->in_room )
	    multi_hit( ch, victim, TYPE_UNDEFINED, TRUE );
	else
	    stop_fighting( ch, FALSE );

	if ( ( victim = ch->fighting ) == NULL )
	    continue;

	check_assist( ch, victim );
	check_troll_revival( ch );

	if ( IS_NPC( ch ) )
	{
	    if ( HAS_TRIGGER_MOB( ch, TRIG_FIGHT ) )
		p_percent_trigger( ch, NULL, NULL, victim, NULL, NULL, TRIG_FIGHT );

	    if ( HAS_TRIGGER_MOB( ch, TRIG_HPCNT ) )
		p_hprct_trigger( ch, victim );
	}

	for ( obj = ch->carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( obj->wear_loc != WEAR_NONE && HAS_TRIGGER_OBJ( obj, TRIG_FIGHT ) )
		p_percent_trigger( NULL, obj, NULL, victim, NULL, NULL, TRIG_FIGHT );
	}

	if ( ch && ch->in_room
	&&   HAS_TRIGGER_ROOM( ch->in_room, TRIG_FIGHT )
	&&   room_trig == FALSE )
	{
	    room_trig = TRUE;
	    p_percent_trigger( NULL, NULL, ch->in_room, victim, NULL, NULL, TRIG_FIGHT );
	}
    }

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	AFFECT_DATA *paf, *paf_next;
	char buf[MAX_STRING_LENGTH];

	ch_next = ch->next;

	if ( ch->stunned && --ch->stunned <= 0 )
	{
	    send_to_char( "You regain your equilibrium.\n\r", ch );
	    act( "$n regains $s equilibrium.",
		ch, NULL, NULL, TO_ROOM, POS_RESTING );
	}

	if ( ch->remember && --ch->remember <= 0 )
	{
	    send_to_char( "{BYou get a hold of the grip on reality, and begin to remember lost techniques.{x\n\r", ch );
	    act( "{B$n regains $s grip on reality.{x", ch, NULL, NULL, TO_ROOM, POS_RESTING );
	}

	if ( ( victim = ch->fighting ) != NULL )
	{
	    if ( ch->desc && IS_SET( ch->combat, COMBAT_SHD_COMBINE ) )
	    {
		if ( ch->desc->shd_damage[0] )
		{
		    sprintf( buf, "{hYour shields strike your foes! {D({WDam:%d{D){x\n\r",
			ch->desc->shd_damage[0] );
		    send_to_char( buf, ch );
		}

		if ( ch->desc->shd_damage[1] )
		{
		    sprintf( buf, "{iYou are struck by shields of your foes! {D({WDam:%d{D){x\n\r",
			ch->desc->shd_damage[1] );
		    send_to_char( buf, ch );
		}

		ch->desc->shd_damage[0] = 0;
		ch->desc->shd_damage[1] = 0;
	    }

	    if ( IS_NPC( victim ) && victim->clan > 0
	    &&   number_percent( ) <= 10 )
	    {
		sprintf( buf, "HELP!! We are being invaded by %s!", ch->name );
		do_clantalk( victim, buf );
	    }
	}

	if ( ( ch->regen[0] > 0 && ch->hit < ch->max_hit ) || ch->regen[0] < 0 )
	{
	    ch->hit += ch->regen[0];
	    update_pos( ch );
	}

	if ( ( ch->regen[1] > 0 && ch->mana < ch->max_mana ) || ch->regen[1] < 0 )
	    ch->mana += ch->regen[1];

	if ( ( ch->regen[2] > 0 && ch->move < ch->max_move ) || ch->regen[2] < 0 )
	    ch->move += ch->regen[2];

	for ( paf = ch->affected; paf != NULL; paf = paf_next )
	{
	    paf_next = paf->next;

	    if ( paf->dur_type != DUR_ROUNDS )
		continue;

	    if ( paf->duration > 0 )
	    {
		paf->duration--;
		if ( number_range( 0, 4 ) == 0 && paf->level > 0 )
		    paf->level--;  /* spell strength fades with time */
            }

	    else if ( paf->duration < 0 )
		;

	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( skill_table[paf->type].msg_off )
		    {
			send_to_char( skill_table[paf->type].msg_off, ch );
			send_to_char( "\n\r", ch );
		    }

		    if ( skill_table[paf->type].room_msg != NULL )
			act( skill_table[paf->type].room_msg,
			    ch, NULL, NULL, TO_ROOM, POS_RESTING );

		    if ( skill_table[paf->type].sound_off != NULL )
		    {
			send_sound_room( ch->in_room, 75, 1, 95, "skills",
			    skill_table[paf->type].sound_off, SOUND_NOSKILL );
		    }
		}

		affect_remove( ch, paf );
	    }
	}
    }
}

bool check_evade( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE( victim )
    ||   ( chance = get_skill( victim, gsn_evade ) ) == 0
    ||   !can_see( victim, ch ) )
	return FALSE;

    if ( number_percent( ) >= chance / 10 )
    {
	check_improve( victim, gsn_evade, FALSE, 20 );
        return FALSE;
    }

    combat( "{iYou evade $n{i's attack.{x",
	ch, NULL, victim, TO_VICT, COMBAT_EVASION );

    combat( "{h$N {hevades your attack.{x",
	ch, NULL, victim, TO_CHAR, COMBAT_EVASION );

    check_improve( victim, gsn_evade, TRUE, 20 );
    return TRUE;
}

bool check_counter( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    OBJ_DATA *wield;
    int chance;

    if ( ( wield = get_eq_char( victim, WEAR_WIELD ) ) == NULL
    ||   ( chance = get_skill( victim, gsn_counter ) ) <= 0
    ||   !IS_AWAKE( victim )
    ||   !can_see( victim, ch )
    ||   dt == gsn_ambush
    ||   dt == gsn_backstab
    ||   dt == gsn_charge
    ||   dt == gsn_assassinate
    ||   dt == gsn_gash
    ||   dt == gsn_circle )
	return FALSE;

    chance /= 10;
    chance += ( get_curr_stat(victim,STAT_DEX) - get_curr_stat(ch,STAT_DEX) );
    chance += get_weapon_skill(victim,get_weapon_sn(victim,FALSE)) - get_weapon_skill(ch,get_weapon_sn(ch,FALSE));
    chance += ( get_curr_stat(victim,STAT_STR) - get_curr_stat(ch,STAT_STR) );

    if ( number_percent( ) >= chance )
        return FALSE;

    combat( "{iYou reverse $n{i's attack and counter with your own!",
	ch, NULL, victim, TO_VICT, COMBAT_COUNTER );
    combat( "{h$N{h reverses your attack!",
	ch, NULL, victim, TO_CHAR, COMBAT_COUNTER );

    one_hit( victim, ch, gsn_counter, FALSE );

    check_improve( victim, gsn_counter, TRUE, 6 );

    return TRUE;
}

bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_dodge) / 2;

    if (!can_see(victim,ch))
	chance /= 3;

    chance += (victim->level - ch->level) / 3;

    if ( number_percent( ) >= chance )
        return FALSE;

    combat("{iYou dodge $n{i's attack.{x",
	ch,NULL,victim,TO_VICT,COMBAT_EVASION);

    combat("{h$N {hdodges your attack.{x",
	ch,NULL,victim,TO_CHAR,COMBAT_EVASION);

    check_improve(victim,gsn_dodge,TRUE,6);
    return TRUE;
}

bool check_sidestep( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_sidestep) / 2;

    if (!can_see(victim,ch))
	chance /= 3;

    chance += (victim->level - ch->level) / 3;

    if ( number_percent( ) >= chance )
        return FALSE;

    combat("{iYour sidestep $n{i's attack.{x",
	ch,NULL,victim,TO_VICT,COMBAT_EVASION);

    combat("{h$N {hsidesteps your attack.{x",
	ch,NULL,victim,TO_CHAR,COMBAT_EVASION);

    check_improve(victim,gsn_sidestep,TRUE,6);
    return TRUE;
}

bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;
    OBJ_DATA *wield;

    wield = get_eq_char( ch, WEAR_WIELD );

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_parry) / 2;

    if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
    {
	if (IS_NPC(victim))
	    chance /= 3;
	else
	    return FALSE;
    }

    if (!can_see(ch,victim))
	chance /= 3;

    chance += (victim->level - ch->level) / 3;

    if ( number_percent( ) >= chance  )
	return FALSE;

    combat("{iYou parry $n{i's attack.{x",
	ch,NULL,victim,TO_VICT,COMBAT_EVASION);
    combat("{h$N {hparries your attack.{x",
	ch,NULL,victim,TO_CHAR,COMBAT_EVASION);
    check_improve(victim,gsn_parry,TRUE,6);
    return TRUE;
}

bool check_phase( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim)
    ||   (chance = get_skill(victim,gsn_phase) / 2) == 0 )
	return FALSE;

    if (!can_see(victim,ch))
	chance /= 3;

    chance += (victim->level - ch->level) / 3;

    if ( number_percent( ) >= chance )
        return FALSE;

    combat("{iYour body phases $n{i's attack.{x",
	ch,NULL,victim,TO_VICT,COMBAT_EVASION);
    combat("{h$N{h's body phases your attack.{x",
	ch,NULL,victim,TO_CHAR,COMBAT_EVASION);
    check_improve(victim,gsn_phase,TRUE,6);
    return TRUE;
}

bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *shield;
    sh_int chance, levitate = 0;

    if ( !IS_AWAKE(victim)
    ||   ( shield = get_eq_char( victim, WEAR_SHIELD ) ) == NULL )
        return FALSE;

    chance = get_skill(victim,gsn_shield_block) / 5 + 3;

    chance += (victim->level - ch->level) / 3;

    if ( number_percent( ) >= chance )
        return FALSE;

    if ( get_eq_char( victim, WEAR_SECONDARY ) != NULL )
    {
	levitate = get_skill(victim,gsn_shield_levitate);

	if ( number_percent() > levitate
	&&   number_percent() > 50 )
	{
	    check_improve(victim,gsn_shield_levitate,FALSE,1);

	    act("{jYour concentration breaks, and $n{j's attack sends $p {jflying!{x",
		ch,shield,victim,TO_VICT,POS_DEAD);
	    act("{jYour attack sends $N{j's shield flying!{x",
		ch,NULL,victim,TO_CHAR,POS_DEAD);
	    combat("{k$n{k's attack sends $N{k's shield airborn!{x",
		ch,NULL,victim,TO_NOTVICT,COMBAT_OTHER);

	    unequip_char( victim, shield );
	    obj_from_char( shield );

	    if ( IS_OBJ_STAT(shield,ITEM_NODROP)
	    ||   IS_OBJ_STAT(shield,ITEM_INVENTORY)
	    ||   IS_OBJ_STAT(shield,ITEM_AQUEST)
	    ||   IS_OBJ_STAT(shield,ITEM_FORGED)
	    ||   IS_IMMORTAL(victim) )
	    {
		act( "{cA magical aura draws $p {cto you.{x",
		    victim, shield, NULL, TO_CHAR, POS_DEAD );
		act( "A magical aura draws $p to $n.",
		    victim, shield, NULL, TO_ROOM, POS_RESTING );
		obj_to_char( shield, victim );
	    } else {
		shield->disarmed_from = victim;
		set_obj_sockets(victim,shield);
		set_arena_obj( victim, shield );
		obj_to_room( shield, victim->in_room );

		if ( IS_NPC(victim) && victim->wait == 0
		&&   can_see_obj(victim,shield) )
		    send_to_char( get_obj(victim,shield,NULL,FALSE), victim );
	    }

	    return FALSE;
	}
    }

    combat("{iYou block $n{i's attack with your shield.{x",
	ch,NULL,victim,TO_VICT,COMBAT_EVASION);
    combat("{h$N {hblocks your attack with a shield.{x",
	ch,NULL,victim,TO_CHAR,COMBAT_EVASION);

    if ( levitate )
	check_improve(victim,gsn_shield_levitate,TRUE,1);

    check_improve(victim,gsn_shield_block,TRUE,3);

    return TRUE;
}

bool check_critical(CHAR_DATA *ch, CHAR_DATA *victim, bool second)
{
    OBJ_DATA *obj;

    if (!second)
	obj = get_eq_char(ch,WEAR_WIELD);
    else
	obj = get_eq_char(ch,WEAR_SECONDARY);

    if ( obj == NULL
    ||   get_skill(ch,gsn_critical) <  1
    ||   number_range(0,125) > get_skill(ch,gsn_critical)
    ||   number_range(0,100) > 25 )
	return FALSE;

    combat( "$p {!CRITICALLY STRIKES{x $N!",
	ch, obj, victim, TO_CHAR, COMBAT_CRITICAL );
    combat( "{!CRITICAL STRIKE!{x",
	ch, obj, victim, TO_VICT, COMBAT_CRITICAL );

    check_improve(ch,gsn_critical,TRUE,6);

    return TRUE;
}

bool check_critdam(CHAR_DATA *ch, CHAR_DATA *victim, bool second)
{
    OBJ_DATA *obj;

    if (!second)
	obj = get_eq_char(ch,WEAR_WIELD);
    else
	obj = get_eq_char(ch,WEAR_SECONDARY);

    if ( obj == NULL
    ||   get_skill(ch,gsn_critdam) <  1
    ||   number_range(0,125) > get_skill(ch,gsn_critdam)
    ||   number_range(0,100) > 25 )
	return FALSE;

    combat( "You maneuver $p to cause {#CRITICAL DAMAGE{x to $N!",
	ch, obj, victim, TO_CHAR, COMBAT_CRITICAL );

    combat( "$N maneuvers $p to cause {#CRITICAL DAMAGE{x to you!",
	victim, obj, ch, TO_CHAR, COMBAT_CRITICAL );

    check_improve(ch,gsn_critdam,TRUE,6);

    return TRUE;
}

int check_damage_bonus( CHAR_DATA *ch, int sn, int dam )
{
    int chance;

    if ( sn == gsn_hand_to_hand )
    {
	if ( ( chance = get_skill( ch, gsn_savage_claws ) ) <= 0 )
	    return dam;

	if ( number_percent( ) < chance )
	{
	    check_improve( ch, gsn_savage_claws, TRUE, 15 );
	    return 115 * dam / 100;
	} else {
	    check_improve( ch, gsn_savage_claws, FALSE, 15 );
	    return dam;
	}
    }

    else if ( sn == gsn_axe )
    {
	if ( ( chance = get_skill( ch, gsn_axe_mastery ) ) <= 0 )
	    return dam;

	if ( number_percent( ) < chance )
	{
	    check_improve( ch, gsn_axe_mastery, TRUE, 15 );
	    return 11 * dam / 10;
	} else {
	    check_improve( ch, gsn_axe_mastery, FALSE, 15 );
	    return dam;
	}
    }

    else if ( sn == gsn_dagger )
    {
	if ( ( chance = get_skill( ch, gsn_knife_fighter ) ) <= 0 )
	    return dam;

	if ( number_percent( ) < chance )
	{
	    check_improve( ch, gsn_knife_fighter, TRUE, 15 );
	    return 11 * dam / 10;
	} else {
	    check_improve( ch, gsn_knife_fighter, FALSE, 15 );
	    return dam;
	}
    }

    return dam;
}

void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn,skill;
    int dam_type;
    bool result, checkabsorb = FALSE;

    sn = -1;

    if ( victim == ch || ch == NULL || victim == NULL
    ||   victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    if ( !secondary )
        wield = get_eq_char( ch, WEAR_WIELD );
    else
        wield = get_eq_char( ch, WEAR_SECONDARY );

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else
	    dt += ch->dam_type;
	     
        if ( IS_NPC(victim) )
        {
            if ( victim->pIndexData->absorption
            &&   number_percent() <= victim->pIndexData->absorption )
                checkabsorb = TRUE;
        }
    }

    if (dt < TYPE_HIT)
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    sn = get_weapon_sn(ch,secondary);

    skill = 20 + get_weapon_skill( ch, sn );

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    thac0_00 = class_table[ch->class].thac0_00;
    thac0_32 = class_table[ch->class].thac0_32;

    thac0 = (thac0_00 + ch->level * (thac0_32 - thac0_00) / 32);

    if (thac0 < 0)
        thac0 = thac0/2;

    if (thac0 < -5)
        thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL(ch) * skill/100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_ambush)
	thac0 -= 8 * (100 - get_skill(ch,gsn_ambush));

    else if (dt == gsn_backstab)
	thac0 -= 8 * (100 - get_skill(ch,gsn_backstab));

    else if (dt == gsn_circle)
	thac0 -= 8 * (100 - get_skill(ch,gsn_circle));

    else if (dt == gsn_assassinate)
	thac0 -= 8 * (100 - get_skill(ch,gsn_assassinate));

    switch(dam_type)
    {
	case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;	break;
	case(DAM_BASH):	 victim_ac = GET_AC(victim,AC_BASH)/10;		break;
	case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
	default:	 victim_ac = GET_AC(victim,AC_EXOTIC)/10;	break;
    };

    if (victim_ac < -15)
	victim_ac = (victim_ac + 15) / 5 - 15;

    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    if ( victim->position < POS_FIGHTING)
	victim_ac += 4;

    if (victim->position < POS_RESTING)
	victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac )
    ||   checkabsorb )
    {
	/* Miss. */
	damage( ch, victim, 0, dt, dam_type, TRUE, secondary, NULL );
	tail_chain( );
	return;
    }

    if ( dt >= TYPE_HIT && !victim->stunned )
    {
	if ( check_parry( ch, victim )
	||   check_dodge( ch, victim )
	||   check_shield_block( ch, victim )
	||   check_phase( ch, victim )
	||   check_sidestep( ch, victim ) )
	{
	    if ( victim->fighting == NULL )
		set_fighting( victim, ch );

	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );

	    return;
	}
    }

    if ( IS_NPC(ch) && wield == NULL )
	dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);

    else
    {
	if (sn != -1)
	    check_improve(ch,sn,TRUE,5);

	if ( wield != NULL )
	{
	    dam = dice(wield->value[1],wield->value[2]) * skill/100;

	    if ( get_eq_char( ch, WEAR_SHIELD ) != NULL )
		dam = dam * 9/10;
	}

	else
	    dam = number_range( 1 + 4 * skill/100, 2 * ch->level * skill/100 );
    }

    if ( IS_NPC( ch ) && ch->level <= 20 )
	dam = dam * 10 / 15;

    if ( get_skill(ch,gsn_enhanced_damage) > 0 )
    {
        if ( number_percent( ) <= get_skill( ch, gsn_enhanced_damage ) / 2 )
        {
            check_improve(ch,gsn_enhanced_damage,TRUE,6);
            dam += dam/13;
        }
    }

    if ( get_skill( ch, gsn_ultra_damage ) > 0 )
    {
	if ( number_percent( ) <= get_skill( ch, gsn_ultra_damage ) / 2 )
	{
	    check_improve( ch, gsn_ultra_damage, TRUE, 4 );
	    dam += dam/10;
	}
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;
    else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

    if ( wield != NULL )
    {
	if ( dt == gsn_ambush )
	{
	    if ( wield->value[0] != WEAPON_POLEARM )
		dam *= (ch->level / 10);
	    else
		dam *= (ch->level / 5);
	}

	else if ( dt == gsn_backstab )
	{
	    if ( wield->value[0] != WEAPON_DAGGER )
		dam *= ( 3 * (ch->level / 12) / 2 );
	    else
		dam *= ( 3 * (ch->level / 6) / 2 );
	}

	else if ( dt == gsn_assassinate )
	{
	    if ( wield->value[0] != WEAPON_DAGGER )
		dam *= ( 3 * (ch->level / 11) / 2 );
	    else
		dam *= ( 3 * (ch->level / 6) / 2 );
	}

	else if ( dt == gsn_circle )
	{
	    if ( wield->value[0] != WEAPON_DAGGER )
		dam *= ( 3 * (ch->level / 14) / 2 );
	    else
		dam *= ( 3 * (ch->level / 8) / 2 );
	}

	else if ( dt == gsn_charge )
	{
	    if ( wield->value[0] != WEAPON_POLEARM )
		dam *= ( 3 * (ch->level / 18) / 2 );
	    else
		dam *= ( 3 * (ch->level / 12) / 2 );
	}

	else if ( dt == gsn_gash )
	{
	    if ( wield->value[0] != WEAPON_SWORD )
		dam *= 4;
	    else
		dam *= 6;
	}

	else if ( dt == gsn_deathblow  )
	{
	    if ( number_percent( ) < 40 )
	    {
		act( "{i$n{i strikes a deadly blow against you with $p{h!{x",
		    ch, wield, victim, TO_VICT, POS_RESTING );
		act( "{hYou strike a deadly blow against $N{h with $p{h!{x",
		    ch, wield, victim, TO_CHAR, POS_RESTING );
		combat( "{k$n{k strikes a deadly blow against $N {kwith $p{k!{x",
		    ch, wield, victim, TO_NOTVICT, COMBAT_OTHER );
		dam *= ( 3 * ( ch->level / 4 ) );
	    }

	    else
		dam *= ( ch->level / 10 );
	}

	else if ( dt == gsn_storm_of_blades )
	    dam = dam * 3 / 2;

	else if ( dt == gsn_cross_slash )
	    dam *= 5;

	else if ( dt == gsn_dismember )
	    dam *= 5;
    }

    dam += ( GET_DAMROLL( ch ) ) * skill / 100;

    if ( dam <= 0 )
	dam = 1;

    if ( IS_NPC( ch ) )
    {
	if ( dam > 35 && ch->level < 20 )
	    dam = ( dam / 2 ) + number_range( 1, 10 );

	if ( dam > 50 && ch->level < 30 )
	    dam = ( dam / 2 ) + number_range( 1, 20 );
    }

    dam = check_damage_bonus( ch, sn, dam );

    if ( check_counter( ch, victim, dam, dt ) )
	return;

    if ( ch != victim )
    {
	if ( check_critdam( ch, victim, secondary ) )
	    dam = dam * number_range( 115, 120 ) / 100;

	if ( check_critical( ch, victim, secondary ) )
	    dam = dam * number_range( 105, 110 ) / 100;
    }

    if ( wield != NULL )
    {
	if ( IS_WEAPON_STAT( wield, WEAPON_SHARP )
	&&   number_percent( ) <= skill / 8 )
	{
	    act( "{iThe sharpened edge of $p {idraws blood!{x",
		ch, wield, victim, TO_VICT, POS_RESTING );
	    act( "{hThe sharpened edge of $p {hdraws blood!{x",
		ch, wield, victim, TO_CHAR, POS_RESTING );
	    dam = dam * 3 / 2;
	}

	if ( IS_WEAPON_STAT( wield, WEAPON_VORPAL )
	&&   number_percent( ) <= skill / 10 )
	{
	    act( "{iThe vorpal edge of $p {idigs deep into your wounds!{x",
		ch, wield, victim, TO_VICT, POS_RESTING );
	    act( "{hThe vorpal edge of $p {hdigs deep into $N{h's wounds!{x",
		ch, wield, victim, TO_CHAR, POS_RESTING );
	    dam *= 2;
	}
    }

    result = damage( ch, victim, dam, dt, dam_type, TRUE, secondary, NULL );

    if ( result && ch->fighting == victim && wield != NULL )
    {
	int dam;

	if ( IS_WEAPON_STAT( wield, WEAPON_POISON ) && number_percent( ) > 65 )
	{
	    int level;
	    AFFECT_DATA *poison, af;

	    if ( ( poison = affect_find( wield->affected, gsn_poison ) ) == NULL )
		level = wield->level;
	    else
		level = poison->level;

	    if ( !saves_spell( level / 2, ch, victim, DAM_POISON ) )
	    {
		combat( "$n is {ypoisoned{x by the venom on $p.",
		    victim, wield, NULL, TO_ROOM, COMBAT_FLAGS );

		af.where	= TO_AFFECTS;
		af.type		= gsn_poison;
		af.level	= level * 3/4;
		af.dur_type	= DUR_TICKS;
		af.duration	= level / 2;
		af.location	= APPLY_STR;
		af.modifier	= -1;
		af.bitvector	= AFF_POISON;
		affect_join( victim, &af );
	    }

	    if ( poison != NULL )
	    {
		poison->level	= UMAX( 0, poison->level - 2 );
		poison->duration= UMAX( 0, poison->duration - 1 );

		if ( poison->level == 0 || poison->duration == 0 )
		    act( "The {ypoison{x on $p has worn off.",
			ch, wield, NULL, TO_CHAR, POS_RESTING );
	    }
	}

	if ( IS_WEAPON_STAT( wield, WEAPON_VAMPIRIC ) && number_percent( ) > 65 )
	{
	    dam = number_range( 1, wield->level + 1 );
	    combat( "{k$p draws life from $n.{x",
		victim, wield, NULL, TO_ROOM, COMBAT_FLAGS );
	    combat( "{iYou feel $p drawing your life away.{x",
		victim, wield, NULL, TO_CHAR, COMBAT_FLAGS );
	    damage( ch, victim, dam, 0, DAM_NEGATIVE, FALSE, secondary, NULL );
	    ch->alignment = URANGE( -1000, ch->alignment - 1, 1000 );
	    if ( ch->pet != NULL )
		ch->pet->alignment = ch->alignment;
	    ch->hit += dam/2;
	}

	if ( IS_WEAPON_STAT( wield, WEAPON_FLAMING ) && number_percent( ) > 65 )
	{
	    dam = number_range( 1, wield->level / 2 + 1 );
	    combat( "{k$n is {rburned{k by $p.{x",
		victim, wield, NULL, TO_ROOM, COMBAT_FLAGS );
	    combat( "{i$p {rsears{i your flesh.{x",
		victim, wield, NULL, TO_CHAR, COMBAT_FLAGS );
	    fire_effect( ch, (void *) victim, wield->level/2, dam/5, TARGET_CHAR );
	    damage( ch, victim, dam, 0, DAM_FIRE, FALSE, secondary, NULL );
	}

	if ( IS_WEAPON_STAT( wield, WEAPON_FROST ) && number_percent( ) > 65 )
	{
	    dam = number_range( 1, wield->level / 6 + 2 );
	    combat( "{k$p {cfreezes{k $n.{x",
		victim, wield, NULL, TO_ROOM, COMBAT_FLAGS );
	    combat( "{iThe {Ccold{i touch of $p surrounds you with {Cice.{x",
		victim, wield, NULL, TO_CHAR, COMBAT_FLAGS );
	    cold_effect( ch, victim, wield->level/2, dam, TARGET_CHAR );
	    damage( ch, victim, dam, 0, DAM_COLD, FALSE, secondary, NULL );
	}

	if ( IS_WEAPON_STAT( wield, WEAPON_SHOCKING ) && number_percent( ) > 65 )
	{
	    dam = number_range( 1, wield->level/5 + 2 );
	    combat( "{k$n is struck by {Ylightning{k from $p.{x",
		victim, wield, NULL, TO_ROOM, COMBAT_FLAGS );
	    combat( "{iYou are {Yshocked{i by $p.{x",
		victim, wield, NULL, TO_CHAR, COMBAT_FLAGS );
	    shock_effect( ch, victim, wield->level/2, dam, TARGET_CHAR );
	    damage( ch, victim, dam, 0, DAM_LIGHTNING, FALSE, secondary, NULL );
	}
    }

    if ( ch->fighting == victim && result )
    {
	if ( IS_SHIELDED( victim, SHD_ACID ) )
	{
	    dam = number_range( 20, 30 );
	    damage( victim, ch, dam, gsn_acidshield, DAM_ACID, TRUE, FALSE, NULL );
	}

	if ( IS_SHIELDED( victim, SHD_DIVINE_AURA ) )
	{
	    dam = number_range( 25, 45 );
	    damage( victim, ch, dam, gsn_divine_aura, DAM_HOLY, TRUE, FALSE, NULL );
	}

	if ( IS_SHIELDED( victim, SHD_ICE ) )
	{
	    dam = number_range( 5, 15 );
	    damage( victim, ch, dam, gsn_iceshield, DAM_COLD, TRUE, FALSE, NULL );
	}

	if ( IS_SHIELDED( victim, SHD_FIRE ) )
	{
	    dam = number_range( 10, 20 );
	    damage( victim, ch, dam, gsn_fireshield, DAM_FIRE, TRUE, FALSE, NULL );
	}

	if ( IS_SHIELDED( victim, SHD_ROCK ) )
	{
	    dam = number_range( 10, 25 );
	    damage( victim, ch, dam, gsn_rockshield, DAM_BASH, TRUE, FALSE, NULL );
	}

	if ( IS_SHIELDED( victim, SHD_SHOCK ) )
	{
	    dam = number_range( 15, 25 );
	    damage( victim, ch, dam, gsn_shockshield, DAM_LIGHTNING, TRUE, FALSE, NULL );
	}

	if ( IS_SHIELDED( victim, SHD_SHRAPNEL ) )
	{
	    dam = number_range( 10, 25 );
	    damage( victim, ch, dam, gsn_shrapnelshield, DAM_IRON, TRUE, FALSE, NULL );
	}

	if ( IS_SHIELDED( victim, SHD_THORN ) )
	{
	    dam = number_range( 10, 25 );
	    damage( victim, ch, dam, gsn_thornshield, DAM_PIERCE, TRUE, FALSE, NULL );
	}

	if ( IS_SHIELDED( victim, SHD_VAMPIRIC ) )
	{
	    dam = number_range( 10, 20 );
	    damage( victim, ch, dam, gsn_vampiricshield, DAM_NEGATIVE, TRUE, FALSE, NULL );
	}

	if ( IS_SHIELDED( victim, SHD_WATER ) )
	{
	    dam = number_range( 10, 20 );
	    damage( victim, ch, dam, gsn_watershield, DAM_WATER, TRUE, FALSE, NULL );
	}
    }

    tail_chain( );
    return;
}

void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool check_area_attack )
{
    int chance;

    if ( ch->desc == NULL )
    {
	ch->wait = UMAX( 0, ch->wait - PULSE_VIOLENCE );

	for ( chance = 0; chance < MAX_DAZE; chance++ )
	    ch->daze[chance] = UMAX( 0, ch->daze[chance] - PULSE_VIOLENCE );
    }

    if ( victim->pcdata && victim->pcdata->dtimer )
	return;

    if ( ch->position < POS_RESTING || ch->stunned )
        return;

    one_hit( ch, victim, dt, FALSE );

    if ( victim->fighting == NULL
    ||   ( victim->pcdata && victim->pcdata->dtimer > 0 )
    ||   ( ch->pcdata && ch->pcdata->dtimer > 0 ) )
	return;

    if ( check_area_attack && ch && ch->pIndexData && ch->in_room
    &&   IS_SET( ch->act, ACT_AREA_ATTACK ) )
    {
	CHAR_DATA *vch, *vch_next;

	for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next_in_room;

	    if ( vch != victim && vch->fighting == ch )
		multi_hit( ch, vch, dt, FALSE );
	}
    }

    if ( get_eq_char( ch, WEAR_SECONDARY ) )
    {
	chance = ( get_skill( ch, gsn_dual_wield ) * 2 / 3 ) + 33;

	if ( !can_see( ch, victim ) )
	    chance = 2 * chance / 3;

	if ( number_percent( ) < chance )
	{
	    one_hit( ch, victim, dt, TRUE );
	    check_improve( ch, gsn_dual_wield, TRUE, 3 );

	    if ( victim->fighting == NULL
	    ||   ( victim->pcdata && victim->pcdata->dtimer > 0 )
	    ||   ( ch->pcdata && ch->pcdata->dtimer > 0 ) )
		return;
	}

	chance = get_skill( ch, gsn_2nd_dual ) * 3 / 4;

	if ( !can_see( ch, victim ) )
	    chance = 2 * chance / 3;

	if ( number_percent( ) < chance )
	{
	    one_hit( ch, victim, dt, TRUE );
	    check_improve( ch, gsn_2nd_dual, TRUE, 3 );

	    if ( victim->fighting == NULL
	    ||   ( victim->pcdata && victim->pcdata->dtimer > 0 )
	    ||   ( ch->pcdata && ch->pcdata->dtimer > 0 ) )
		return;
	}

	chance = get_skill( ch, gsn_3rd_dual ) * 3 / 4;

	if ( !can_see( ch, victim ) )
	    chance = 2 * chance / 3;

	if ( number_percent( ) < chance )
	{
	    one_hit( ch, victim, dt, TRUE );
	    check_improve( ch, gsn_3rd_dual, TRUE, 3 );

	    if ( victim->fighting == NULL
	    ||   ( victim->pcdata && victim->pcdata->dtimer > 0 )
	    ||   ( ch->pcdata && ch->pcdata->dtimer > 0 ) )
		return;
	}
    }

    if ( IS_AFFECTED( ch, AFF_HASTE ) )
	one_hit( ch, victim, dt, FALSE );

    if ( victim->fighting == NULL
    ||   dt == gsn_backstab
    ||   dt == gsn_circle
    ||   dt == gsn_charge
    ||   dt == gsn_assassinate
    ||   dt == gsn_cyclone
    ||   dt == gsn_storm_of_blades
    ||   ( victim->pcdata && victim->pcdata->dtimer > 0 )
    ||   ( ch->pcdata && ch->pcdata->dtimer > 0 ) )
	return;

    chance = get_skill( ch, gsn_second_attack ) / 2;

    if ( IS_AFFECTED( ch, AFF_SLOW ) )
	chance /= 2;

    if ( !can_see( ch, victim ) )
	chance = 2 * chance / 3;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve( ch, gsn_second_attack, TRUE, 5 );

	if ( victim->fighting == NULL
	||   ( victim->pcdata && victim->pcdata->dtimer > 0 )
	||   ( ch->pcdata && ch->pcdata->dtimer > 0 ) )
	    return;
    }

    chance = get_skill( ch, gsn_third_attack ) / 2;

    if ( IS_AFFECTED( ch, AFF_SLOW ) )
	chance /= 2;

    if ( !can_see( ch, victim ) )
	chance = 2 * chance / 3;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve( ch, gsn_third_attack, TRUE, 6 );

	if ( victim->fighting == NULL
	||   ( victim->pcdata && victim->pcdata->dtimer > 0 )
	||   ( ch->pcdata && ch->pcdata->dtimer > 0 ) )
	    return;
    }

    chance = get_skill( ch, gsn_fourth_attack ) / 2;

    if ( IS_AFFECTED( ch, AFF_SLOW ) )
	chance /= 3;

    if ( !can_see( ch, victim ) )
	chance = 2 * chance / 3;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve( ch, gsn_fourth_attack, TRUE, 6 );

	if ( victim->fighting == NULL
	||   ( victim->pcdata && victim->pcdata->dtimer > 0 )
	||   ( ch->pcdata && ch->pcdata->dtimer > 0 ) )
	    return;
    }

    if ( IS_AFFECTED( ch, AFF_SLOW ) )
	return;

    chance = get_skill( ch, gsn_fifth_attack ) / 2;

    if ( !can_see( ch, victim ) )
	chance = 2 * chance / 3;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve( ch, gsn_fifth_attack, TRUE, 6 );
	if ( victim->fighting == NULL
	||   ( victim->pcdata && victim->pcdata->dtimer > 0 )
	||   ( ch->pcdata && ch->pcdata->dtimer > 0 ) )
	    return;
    }

    chance = get_skill( ch, gsn_sixth_attack ) / 2;

    if ( !can_see( ch, victim ) )
	chance = 2 * chance / 3;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve( ch, gsn_sixth_attack, TRUE, 6 );
	if ( victim->fighting == NULL
	||   ( victim->pcdata && victim->pcdata->dtimer > 0 )
	||   ( ch->pcdata && ch->pcdata->dtimer > 0 ) )
	    return;
    }

    chance = get_skill( ch, gsn_seventh_attack) / 2;

    if ( !can_see( ch, victim ) )
	chance = 2 * chance / 3;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve( ch, gsn_seventh_attack, TRUE, 6 );
	if ( victim->fighting == NULL
	||   ( victim->pcdata && victim->pcdata->dtimer > 0 )
	||   ( ch->pcdata && ch->pcdata->dtimer > 0 ) )
	    return;
    }

    chance = get_skill( ch, gsn_eighth_attack) / 2;

    if ( !can_see( ch, victim ) )
        chance = 2 * chance / 3;

    if ( number_percent( ) < chance )
    {
        one_hit( ch, victim, dt, FALSE );
        check_improve( ch, gsn_eighth_attack, TRUE, 6 );
    }
}

int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels )
{
    int align, change, level_range, base_exp, xp;
        
    if ( IS_SET( gch->in_room->room_flags, ROOM_ARENA )
    ||   IS_SET( gch->in_room->room_flags, ROOM_WAR ) )
	return 0;

    level_range = victim->level - gch->level;

    switch ( level_range )
    {
 	default : 	base_exp =   0;		break;
	case -9 :	base_exp =   1;		break;
	case -8 :	base_exp =   2;		break;
	case -7 :	base_exp =   5;		break;
	case -6 : 	base_exp =   9;		break;
	case -5 :	base_exp =  11;		break;
	case -4 :	base_exp =  22;		break;
	case -3 :	base_exp =  33;		break;
	case -2 :	base_exp =  50;		break;
	case -1 :	base_exp =  66;		break;
	case  0 :	base_exp =  83;		break;
	case  1 :	base_exp =  99;		break;
	case  2 :	base_exp = 121;		break;
	case  3 :	base_exp = 143;		break;
	case  4 :	base_exp = 165;		break;
    }

    if ( !IS_NPC( gch ) && !IS_NPC( victim ) )
	base_exp += 500;

    align = victim->alignment - gch->alignment;

    if ( align > 500 ) /* monster is more good than slayer */
    {
	change = ( align - 500 ) * base_exp / 500 * gch->level/total_levels;
	change = UMAX( 1, change );
        gch->alignment = URANGE( -1000, gch->alignment - change, 1000 );
	if ( gch->pet != NULL )
	    gch->pet->alignment = gch->alignment;
    }

    else if ( align < -500 ) /* monster is more evil than slayer */
    {
	change = ( -1 * align - 500 ) * base_exp/500 * gch->level/total_levels;
	change = UMAX( 1, change );
	gch->alignment = URANGE( -1000, gch->alignment + change, 1000 );
        if ( gch->pet != NULL )
            gch->pet->alignment = gch->alignment;
    }

    else /* improve this someday */
    {
	change =  gch->alignment * base_exp/500 * gch->level/total_levels;
	gch->alignment = URANGE( -1000, gch->alignment - change, 1000 );
        if ( gch->pet != NULL )
            gch->pet->alignment = gch->alignment;
    }

    if ( IS_SET( gch->act, PLR_NOEXP ) )
	return 0;

    if ( level_range > 4 )
	base_exp = 160 + 20 * ( level_range - 4 );

    if ( IS_SET( victim->act, ACT_SMART_MOB ) )
	base_exp += base_exp / 2;

    else if ( victim->spec_fun == spec_lookup( "spec_cast_cleric" )
    ||   victim->spec_fun == spec_lookup( "spec_cast_mage" )
    ||	 victim->spec_fun == spec_lookup( "spec_cast_undead" ) )
	base_exp += base_exp / 3;

    /* calculate exp multiplier */
    if (gch->alignment > 500)  /* for goodie two shoes */
    {
	if (victim->alignment < -750)
	    xp = (base_exp *4)/3;

 	else if (victim->alignment < -500)
	    xp = (base_exp * 5)/4;

        else if (victim->alignment < -250)
	    xp = (base_exp * 3)/4;

        else if (victim->alignment > 750)
	    xp = base_exp / 4;

   	else if (victim->alignment > 500)
	    xp = base_exp / 2;

	else
	    xp = base_exp;
    }

    else if (gch->alignment < -500) /* for baddies */
    {
	if (victim->alignment > 750)
	    xp = (base_exp * 5)/4;

  	else if (victim->alignment > 500)
	    xp = (base_exp * 11)/10;

   	else if (victim->alignment < -750)
	    xp = base_exp/2;

	else if (victim->alignment < -500)
	    xp = (base_exp * 3)/4;

	else if (victim->alignment < -250)
	    xp = (base_exp * 9)/10;

	else
	    xp = base_exp;
    }

    else if (gch->alignment > 200)  /* a little good */
    {

	if (victim->alignment < -500)
	    xp = (base_exp * 6)/5;

 	else if (victim->alignment > 750)
	    xp = base_exp/2;

	else if (victim->alignment > 0)
	    xp = (base_exp * 3)/4;

	else
	    xp = base_exp;
    }

    else if (gch->alignment < -200) /* a little bad */
    {
	if (victim->alignment > 500)
	    xp = (base_exp * 6)/5;

	else if (victim->alignment < -750)
	    xp = base_exp/2;

	else if (victim->alignment < 0)
	    xp = (base_exp * 3)/4;

	else
	    xp = base_exp;
    }

    else /* neutral */
    {

	if (victim->alignment > 500 || victim->alignment < -500)
	    xp = (base_exp * 4)/3;

	else if (victim->alignment < 200 && victim->alignment > -200)
	    xp = base_exp/2;

 	else
	    xp = base_exp;
    }

    xp /= 3;

    /* randomize the rewards */
    xp = number_range ( xp * 10/4, xp * 20/4 );

    if ( victim->pIndexData != NULL && victim->pIndexData->exp_percent != 100 )
	xp = xp * victim->pIndexData->exp_percent / 100;

    if ( gch->level >= LEVEL_HERO )
	xp /= 3;

    else if ( gch->level > 15 )
	xp /= 2;

    else
	xp = xp * 3 / 4;

    // adjust for grouping
    xp = xp * gch->level * 3 / ( UMAX( 1, total_levels -1 ) );

    /* adjust for global exp_mod */
    xp = mud_stat.exp_mod[0] * xp / mud_stat.exp_mod[1];

    /*if ( gch->level < 15 )
	return URANGE( 0, xp, 2000  );

    else if ( gch->level < LEVEL_HERO )
	return URANGE( 0, xp, 1400 );

    else
	return URANGE( 250, xp, 750 );*/
    return xp;
}

void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    CHAR_DATA *gch, *lch;
    char buf[MAX_STRING_LENGTH];
    int members, group_levels, xp;

    if ( victim == ch )
	return;

    members = 0;
    group_levels = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
        {
	    members++;
	    group_levels += IS_NPC(gch) ? gch->level / 2 : gch->level;

	    if ( !IS_NPC( gch ) && IS_NPC( victim )
	    &&   IS_SET( gch->act, PLR_QUESTOR ) )
	    {
		if ( gch->pcdata->questobj == 0
		&&   gch->pcdata->questmob == victim->pIndexData->vnum )
		{
		    send_to_char( "{RYou have almost completed your {GQUEST{R!\n\r", gch );
		    send_to_char( "Return to the questmaster before your time runs out!{x\n\r", gch );
		    gch->pcdata->questmob = -1;
		}
	    }
	}
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
	group_levels = ch->level;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_NPC(gch) )
	    continue;

	if ( gch->level - lch->level >= 11 )
	{
	    send_to_char( "You are too high for this group.\n\r", gch );
	    continue;
	}

	if ( gch->level - lch->level <= -11 )
	{
	    send_to_char( "You are too low for this group.\n\r", gch );
	    continue;
	}

        xp = xp_compute( gch, victim, group_levels );
	sprintf( buf, "{BYou receive {W%d{B experience points.{x\n\r", xp );
	send_to_char( buf, gch );
	gain_exp( gch, xp );

	if ( is_affected(gch,gsn_infernal_offer) && !IS_EVIL(gch) )
	{
	    affect_strip(gch,gsn_infernal_offer);
	    sprintf(buf,"%s has revoked your blessing.\n\r",
		mud_stat.evil_god_string);
	    send_to_char(buf,gch);
	}

	else if ( is_affected(gch,gsn_divine_blessing) && !IS_GOOD(gch) )
	{
	    affect_strip(gch,gsn_divine_blessing);
	    sprintf(buf,"%s has revoked your blessing.\n\r",
		mud_stat.good_god_string);
	    send_to_char(buf,gch);
	}

	if ( gch->level < LEVEL_IMMORTAL
	&&   gch->in_room != NULL
	&&   !IS_SET(gch->in_room->room_flags, ROOM_ARENA)
	&&   !IS_SET(gch->in_room->room_flags, ROOM_WAR) )
	{
	    OBJ_DATA *obj, *obj_next;

	    for ( obj = gch->carrying; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;

		if ( obj->wear_loc == WEAR_NONE )
		    continue;

		if ( (IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(gch)   )
		||   (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(gch)   )
		||   (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(gch)) )
		{
		    act( "{cYou are {Wzapped{c by $p.{x",
			gch, obj, NULL, TO_CHAR, POS_RESTING );
		    act( "$n is {Wzapped{x by $p.",
			gch, obj, NULL, TO_ROOM, POS_RESTING );
		    obj_from_char( obj );

		    if ( IS_OBJ_STAT(obj, ITEM_NODROP)
		    ||   IS_OBJ_STAT(obj, ITEM_INVENTORY)
		    ||   IS_OBJ_STAT(obj, ITEM_AQUEST)
		    ||   IS_OBJ_STAT(obj, ITEM_FORGED) )
		    {
			act( "{cA magical aura draws $p {cto you.{x",
			    gch, obj, NULL, TO_CHAR, POS_DEAD );
			act( "A magical aura draws $p to $n.",
			    gch, obj, NULL, TO_ROOM, POS_RESTING );
			obj_to_char( obj, gch );
		    } else {
			obj->disarmed_from = gch;
			set_obj_sockets( gch, obj );
			set_arena_obj( gch, obj );
			obj_to_room( obj, gch->in_room );
		    }
		}
	    }
	}
    }

    return;
}

void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;

    if ( ( victim = ch->fighting ) == NULL )
    {        
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( !check_stun_remember( ch ) )
	return;

    if ( arena_flag( ch, ARENA_NO_FLEE ) )
	return;

    was_in = ch->in_room;

    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_range(0,5);
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	|| ( IS_SET(pexit->exit_info, EX_CLOSED)
	&&   (IS_SET(pexit->exit_info,EX_NOPASS)
	||    !IS_AFFECTED(ch, AFF_PASS_DOOR)) )
	||   number_range(0,ch->daze[DAZE_FLEE]) != 0
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	move_char( ch, door, FALSE, FALSE );

	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n has {Yfled{x!", ch, NULL, NULL, TO_ROOM,POS_RESTING);

	if ( ch->fighting != NULL && !IS_NPC(ch->fighting) )
	    check_pktimer( ch, ch->fighting, FALSE );

        if ( !IS_NPC(ch) )
        {
            send_to_char( "{BYou {Yflee{B from combat!{x\n\r", ch );

	    if ( class_table[ch->class].sub_class == class_lookup( "thief" )
            &&   (number_percent() < 3 * (ch->level/2)) )
            {
                if ( IS_NPC(victim) || !ch->pcdata || !ch->pcdata->attacker )
                    send_to_char( "You {Ysnuck away{x safely.\n\r", ch);
            } else {
                send_to_char( "You lost 10 exp.\n\r", ch);
                gain_exp( ch, -10 );
            }
        }

	if ( IS_NPC(ch) )
	    mob_remember(ch,victim,MEM_AFRAID);

	for (victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
	{
	    if (victim->fighting != NULL && victim->fighting == ch)
		victim->wait = 0;
	}

	ch->in_room = now_in;
	stop_fighting( ch, TRUE );
	return;
    }

    send_to_char( "{z{CPANIC!{x{B You couldn't escape!{x\n\r", ch );
}

void death_cry( CHAR_DATA *ch, CHAR_DATA *killer )
{
    ROOM_INDEX_DATA *was_in_room;
    char buf[MAX_STRING_LENGTH];
    char *msg;
    int door;
    int vnum;

    if ( ch->in_room == NULL )
	return;

    vnum = 0;
    msg = "You hear $n's death cry.";

    if (IS_NPC(ch))
    {
	if ( ch->clan != 0 )
	{
	    sprintf(buf,"I have been slain by %s!", PERS(killer,ch) );
	    do_clantalk(ch,buf);
	}

	if (!IS_SET(ch->act, ACT_NO_BODY) )
	{
	    switch ( number_range(0,15))
	    {
	    case  0:
		msg  = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case  1:
		msg  = "$n splatters blood on your armor.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case  2:
		if (IS_SET(ch->parts,PART_GUTS))
		{
		    msg = "$n spills $s guts all over the floor.";
		    vnum = OBJ_VNUM_GUTS;
		}
		break;
	    case  3:
		if (IS_SET(ch->parts,PART_HEAD))
		{
		    msg  = "$n's severed head plops on the ground.";
		    vnum = OBJ_VNUM_SEVERED_HEAD;
		}
		break;
	    case  4:
		if (IS_SET(ch->parts,PART_HEART))
		{
		    msg  = "$n's heart is torn from $s chest.";
		    vnum = OBJ_VNUM_TORN_HEART;
		}
		break;
	    case  5:
		if (IS_SET(ch->parts,PART_ARMS))
		{
		    msg  = "$n's arm is sliced from $s dead body.";
		    vnum = OBJ_VNUM_SLICED_ARM;
		}
		break;
	    case  6:
		if (IS_SET(ch->parts,PART_LEGS))
		{
		    msg  = "$n's leg is sliced from $s dead body.";
		    vnum = OBJ_VNUM_SLICED_LEG;
		}
		break;
	    case 7:
		if (IS_SET(ch->parts,PART_BRAINS))
		{
		    msg = "$n's head is shattered, and $s brains splash all over you.";
		    vnum = OBJ_VNUM_BRAINS;
		}
		break;
	    case 8:
		if (IS_SET(ch->parts,PART_EYE))
		{
		    msg = "$n's eyes fly from $s head.";
		    vnum = OBJ_VNUM_EYE;
		}
		break;
	    case 9:
		if (IS_SET(ch->parts,PART_HANDS))
		{
		    msg = "$n's hand is torn from $s arm.";
		    vnum = OBJ_VNUM_HAND;
		}
		break;
	    case 10:
		if (IS_SET(ch->parts,PART_EAR))
		{
		    msg = "$n's ear is ripped off.";
		    vnum = OBJ_VNUM_EAR;
		}
		break;
	    case 11:
		if (IS_SET(ch->parts,PART_FINGERS))
		{
		    msg = "$n's fingers fly from $s hand.";
		    vnum = OBJ_VNUM_FINGER;
		}
		break;
	    case 12:
		if (IS_SET(ch->parts,PART_FEET))
		{
		    msg = "$n's feet are severed at the ankles.";
		    vnum = OBJ_VNUM_FOOT;
		}
		break;
	    case 13:
		if (IS_SET(ch->parts,PART_WINGS))
		{
		    msg = "$n's wings fly from $s back.";
		    vnum = OBJ_VNUM_WING;
		}
		break;
	    case 14:
	    case 15:
		msg  = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    }
	}
    } else if (ch->level > 19)
    {
	switch ( number_range(0,20) )
	{
	    case  0:
		msg  = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case  1:
		msg  = "$n splatters blood on your armor.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case  2:
		if (IS_SET(ch->parts,PART_GUTS))
		{
		    msg = "$n spills $s guts all over the floor.";
		    vnum = OBJ_VNUM_GUTS;
		}
		break;
	    case  3:
		if (IS_SET(ch->parts,PART_HEAD))
		{
		    msg  = "$n's severed head plops on the ground.";
		    vnum = OBJ_VNUM_SEVERED_HEAD;
		}
		break;
	    case  4:
		if (IS_SET(ch->parts,PART_HEART))
		{
		    msg  = "$n's heart is torn from $s chest.";
		    vnum = OBJ_VNUM_TORN_HEART;
		}
		break;
	    case  5:
		if (IS_SET(ch->parts,PART_ARMS))
		{
		    msg  = "$n's arm is sliced from $s dead body.";
		    vnum = OBJ_VNUM_SLICED_ARM;
		}
		break;
	    case  6:
		if (IS_SET(ch->parts,PART_LEGS))
		{
		    msg  = "$n's leg is sliced from $s dead body.";
		    vnum = OBJ_VNUM_SLICED_LEG;
		}
		break;
	    case 7:
		if (IS_SET(ch->parts,PART_BRAINS))
		{
		    msg = "$n's head is shattered, and $s brains splash all over you.";
		    vnum = OBJ_VNUM_BRAINS;
		}
		break;
	    case  8:
		msg  = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case  9:
		msg  = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case  10:
		if (IS_SET(ch->parts,PART_HEAD))
		{
		    msg  = "$n's severed head plops on the ground.";
		    vnum = OBJ_VNUM_SEVERED_HEAD;
		}
		break;
	    case  11:
		if (IS_SET(ch->parts,PART_HEART))
		{
		    msg  = "$n's heart is torn from $s chest.";
		    vnum = OBJ_VNUM_TORN_HEART;
		}
		break;
	    case  12:
		if (IS_SET(ch->parts,PART_ARMS))
		{
		    msg  = "$n's arm is sliced from $s dead body.";
		    vnum = OBJ_VNUM_SLICED_ARM;
		}
		break;
	    case  13:
		if (IS_SET(ch->parts,PART_LEGS))
		{
		    msg  = "$n's leg is sliced from $s dead body.";
		    vnum = OBJ_VNUM_SLICED_LEG;
		}
		break;
	    case 14:
		if (IS_SET(ch->parts,PART_BRAINS))
		{
		    msg = "$n's head is shattered, and $s brains splash all over you.";
		    vnum = OBJ_VNUM_BRAINS;
		}
		break;
	    case 15:
		if (IS_SET(ch->parts,PART_EYE))
		{
		    msg = "$n's eyes fly from $s head.";
		    vnum = OBJ_VNUM_EYE;
		}
		break;
	    case 16:
		if (IS_SET(ch->parts,PART_HANDS))
		{
		    msg = "$n's hand is torn from $s arm.";
		    vnum = OBJ_VNUM_HAND;
		}
		break;
	    case 17:
		if (IS_SET(ch->parts,PART_EAR))
		{
		    msg = "$n's ear is ripped off.";
		    vnum = OBJ_VNUM_EAR;
		}
		break;
	    case 18:
		if (IS_SET(ch->parts,PART_FINGERS))
		{
		    msg = "$n's fingers fly from $s hand.";
		    vnum = OBJ_VNUM_FINGER;
		}
		break;
	    case 19:
		if (IS_SET(ch->parts,PART_FEET))
		{
		    msg = "$n's feet are severed at the ankles.";
		    vnum = OBJ_VNUM_FOOT;
		}
		break;
	    case 20:
		if (IS_SET(ch->parts,PART_WINGS))
		{
		    msg = "$n's wings fly from $s back.";
		    vnum = OBJ_VNUM_WING;
		}
		break;
	}
    }

    act( msg, ch, NULL, NULL, TO_ROOM,POS_RESTING);

    if (( vnum == 0 ) && !IS_SET(ch->act, ACT_NO_BODY) )
    {
	switch ( number_bits(4))
	{
	case  0:
		vnum = 0;
		break;
	case  1:
		vnum = OBJ_VNUM_BLOOD;
		break;
	case  2:
		vnum = 0;
		break;
	case  3:
		vnum = OBJ_VNUM_BLOOD;
		break;
	case  4:
		vnum = 0;
		break;
	case  5:
		vnum = OBJ_VNUM_BLOOD;
		break;
	case  6:
		vnum = 0;
		break;
	case 7:
		vnum = OBJ_VNUM_BLOOD;
	}
    }

    if ( vnum != 0 )
    {
	char buf[MAX_STRING_LENGTH];
	char name[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	if ( IS_NPC(killer) )
	    vnum = OBJ_VNUM_BLOOD;

	strcpy(name,IS_NPC(ch) ? ch->short_descr : ch->name);
	obj		= create_object( get_obj_index( vnum ) );

	set_obj_sockets(killer,obj);

	if (vnum == OBJ_VNUM_BLOOD)
	    obj->timer = number_range( 1, 4 );
	else if (!IS_NPC(ch))
	    obj->timer = number_range( 20, 60 );
	else
	    obj->timer = number_range( 4, 7 );

	if ( vnum != OBJ_VNUM_BLOOD )
	{
	    sprintf( buf, obj->short_descr, name );
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( buf );

	    sprintf( buf, obj->description, name );
	    free_string( obj->description );
	    obj->description = str_dup( buf );

            sprintf( buf, obj->name, ch->name );
            free_string( obj->name );
            obj->name = str_dup( buf );
	}

	if (IS_NPC(ch))
	    obj->value[4] = 0;
	else
	    obj->value[4] = 1;

	set_arena_obj( ch, obj );
	obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM,POS_RESTING);
	}
    }
    ch->in_room = was_in_room;

    return;
}

void make_corpse( CHAR_DATA *ch, CHAR_DATA *killer )
{
    char buf[MAX_STRING_LENGTH];
    char name[MAX_INPUT_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool immkill = FALSE;

    if ( ch->in_room == NULL )
	return;

    if ( IS_IMMORTAL(killer) && IS_NPC(ch) )
	immkill = TRUE;

    if ( IS_NPC(ch) )
    {
	if (IS_SET(ch->act, ACT_NO_BODY) )
	{
	    if (IS_SET(ch->act, ACT_NB_DROP) )
	    {
		for ( obj = ch->carrying; obj != NULL; obj = obj_next )
		{
		    obj_next = obj->next_content;
		    obj_from_char( obj );

		    if ( obj->pIndexData->vnum >= FORGE_STONE_ADAMANTIUM
		    &&   obj->pIndexData->vnum <= FORGE_STONE_BRONZE
		    &&   killer->level - ch->level > 30 )
		    {
			extract_obj( obj );
			continue;
		    }

		    set_obj_sockets(killer,obj);

		    if ( immkill && obj->loader == NULL )
			set_obj_loader( killer, obj, "KILL" );
		    if (obj->item_type == ITEM_POTION)
			obj->timer = number_range(500,1000);
		    if (obj->item_type == ITEM_SCROLL)
			obj->timer = number_range(1000,2500);
		    if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))
		    {
			obj->timer = number_range(5,10);
			REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);
		    }
		    REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);

		    act("$p falls to the floor.",ch,obj,NULL,TO_ROOM,POS_RESTING);
		    set_arena_obj( ch, obj );
		    obj_to_room(obj,ch->in_room);
		}
	    }
	    return;
	}
	strcpy(name,ch->short_descr);
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC));
	corpse->timer	= number_range( 3, 6 );

	if ( ch->gold > 0 || ch->platinum > 0 )
	{
	    obj_to_obj( create_money( ch->platinum, ch->gold, ch->silver ), corpse );
	    ch->platinum = 0;
	    ch->gold = 0;
	    ch->silver = 0;
	}
	corpse->cost = 0;
    }
    else
    {
	strcpy(name,ch->name);
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC));

	corpse->owner = str_dup(ch->name);
	corpse->killer = str_dup(killer->name);
	corpse->timer = 2000;

	if (ch->platinum > 1 || ch->gold > 1 || ch->silver > 1)
	{
	    obj_to_obj(create_money(ch->platinum/2, ch->gold / 2, ch->silver/2), corpse);
	    ch->platinum -= ch->platinum/2;
	    ch->gold -= ch->gold/2;
	    ch->silver -= ch->silver/2;
	}
    }

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    sprintf( buf, corpse->name, name );
    free_string( corpse->name );
    corpse->name = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;

	obj_from_char( obj );

	if ( obj->pIndexData->vnum >= FORGE_STONE_ADAMANTIUM
	&&   obj->pIndexData->vnum <= FORGE_STONE_BRONZE
	&&   killer->level - ch->level > 30
	&&   IS_NPC( ch ) )
	{
	    extract_obj( obj );
	    continue;
	}

	if ( immkill && obj->loader == NULL )
	    set_obj_loader( killer, obj, "KILL" );

	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(500,1000);

	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(1000,2500);

	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))
	{
	    obj->timer = number_range(5,10);
	    REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);
	}
	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);

	obj_to_obj( obj, corpse );
    }

    set_obj_sockets( killer, corpse );
    set_arena_obj( killer, corpse );
    obj_to_room( corpse, ch->in_room );

    if ( IS_NPC( ch ) && !IS_NPC( killer )
    &&   ch->level > 0 && number_percent( ) <= 5 )
    {
	if ( ( obj = rand_obj( ch->level ) ) != NULL )
	{
	    if ( immkill )
		set_obj_loader( killer, obj, "IPOP" );
	    else
		set_obj_loader( killer, obj, "MPOP" );

	    obj_to_obj( obj, corpse );
	}
    }

    return;
}

void raw_kill( CHAR_DATA *victim, CHAR_DATA *killer )
{
    CHAR_DATA *wch;
    char buf[MAX_STRING_LENGTH];
    char pkbuf[MAX_STRING_LENGTH];
    int i;

    if ( killer == NULL )
    {
        bug( "Raw_kill: NULL killer", 0 );
	return;
    }

    if ( IS_NPC( killer ) && killer->master && !IS_NPC( killer->master ) )
	killer = killer->master;

    killer->wait = 0;

    if ( victim == NULL )
    {
	bug( "Raw_kill: NULL victim", 0 );
	return;
    }

    if ( killer->in_room != NULL && victim->in_room != NULL )
    {
	if ( IS_SET( killer->in_room->room_flags, ROOM_ARENA )
	&&   IS_SET( victim->in_room->room_flags, ROOM_ARENA ) )
	    return;
    }

    death_cry( victim, killer );
    stop_fighting( victim, TRUE );
    make_corpse( victim, killer );

    if ( IS_NPC( victim ) )
    {
	if ( !IS_NPC( killer ) )
	{
	    check_gquest( killer, victim );

	    if ( ++killer->pcdata->mobkills % 50 == 0 )
		rank_chart( killer, "mobkills", killer->pcdata->mobkills );

	    if ( !IS_IMMORTAL( killer ) )
	    {
		victim->pIndexData->mob_pc_deaths[0]++;
		victim->pIndexData->perm_mob_pc_deaths[0]++;

		if ( victim->pIndexData->area != NULL )
		    SET_BIT( victim->pIndexData->area->area_flags, AREA_CHANGED );
	    }
	}

	extract_char( victim, TRUE );
	return;
    }

    if ( IS_NPC( killer ) && !IS_NPC( victim ) )
    {
	MEM_DATA *remember, *remember_next;

	killer->pIndexData->mob_pc_deaths[1]++;
	killer->pIndexData->perm_mob_pc_deaths[1]++;

	if ( killer->pIndexData->area != NULL )
	    SET_BIT( killer->pIndexData->area->area_flags, AREA_CHANGED );

	victim->pcdata->mobdeath++;
	rank_chart( victim, "mobdeaths", victim->pcdata->mobdeath );
	     
	for ( remember = killer->memory; remember != NULL; remember = remember_next )
	{
	    remember_next = remember->next;

	    if ( victim->id == remember->id )
		mob_forget( killer, remember );
	}
    }

    if ( !IS_NPC( killer ) && ( victim != killer ) )
    {
	PKILL_DATA *pkill_data;
	OBJ_DATA *soul;

	if ( !IS_SET( victim->act, PLR_TWIT )
	&&   !IS_SET( killer->act, PLR_TWIT )
	&&   !IS_IMMORTAL( killer ) )
	{
	    PKILL_RECORD *pk_record;
	    char assists[MAX_STRING_LENGTH];
	    sh_int value;

	    assists[0] = '\0';

	    for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
	    {
		if ( wch->pcdata->opponent != NULL
		&&   wch->pcdata->opponent == victim )
		{
		    if ( wch != killer )
		    {
			wch->pcdata->assist++;
			rank_chart( wch, "assists", wch->pcdata->assist );

			sprintf( buf, " %s", wch->name );
			strcat( assists, buf );
		    }

		    wch->pcdata->opponent = NULL;
		    wch->pcdata->pktimer = 0;
		}
	    }

	    pkill_data				= new_pkill( );
	    pkill_data->player_name		= str_dup( victim->name );
	    pkill_data->time			= current_time;
	    pkill_data->killer			= TRUE;
	    pkill_data->next			= killer->pcdata->recent_pkills;
	    killer->pcdata->recent_pkills	= pkill_data;

	    pkill_data				= new_pkill( );
	    pkill_data->player_name		= str_dup( killer->name );
	    pkill_data->time			= current_time;
	    pkill_data->killer			= FALSE;
	    pkill_data->next			= victim->pcdata->recent_pkills;
	    victim->pcdata->recent_pkills	= pkill_data;

	    killer->pcdata->pkills++;
	    victim->pcdata->pdeath++;

	    if ( ( value = victim->pcdata->pkpoints / 4 ) != 0 )
	    {
		victim->pcdata->pkpoints -= value;
		killer->pcdata->pkpoints += value;

		if ( victim->pcdata->pkpoints < 3 )
		    victim->pcdata->pkpoints = 0;

		sprintf( pkbuf, "{RYou gain {G%d {RPK points, giving you a total of {G%d{R!{x\n\r",
		    value, killer->pcdata->pkpoints );
		send_to_char( pkbuf, killer );

		sprintf( pkbuf, "{RYou lose {G%d {RPK points, leaving you with {G%d{R.{x\n\r",
		    value, victim->pcdata->pkpoints );
		send_to_char( pkbuf, victim );

		rank_chart( killer, "pkpoints", killer->pcdata->pkpoints );
		rank_chart( victim, "pkpoints", victim->pcdata->pkpoints );
	    }

	    if ( assists[0] == '\0' )
		strcpy( assists, " none" );

	    pk_record			= new_pk_record( );
	    pk_record->killer_name	= str_dup( killer->name );
	    pk_record->victim_name	= str_dup( victim->name );
	    pk_record->killer_clan	= str_dup( clan_table[killer->clan].color );
	    pk_record->victim_clan	= str_dup( clan_table[victim->clan].color );
	    pk_record->assist_string	= str_dup( assists+1 );
	    pk_record->level[0] 	= killer->level;
	    pk_record->level[1]		= victim->level;
	    pk_record->pkill_time	= current_time;
	    pk_record->pkill_points	= value;
	    pk_record->bounty		= victim->pcdata->bounty;
	    pk_record->next		= victim->pcdata->death_list;
	    victim->pcdata->death_list	= pk_record;

	    pk_record			= new_pk_record( );
	    pk_record->killer_name	= str_dup( killer->name );
	    pk_record->victim_name	= str_dup( victim->name );
	    pk_record->killer_clan	= str_dup( clan_table[killer->clan].color );
	    pk_record->victim_clan	= str_dup( clan_table[victim->clan].color );
	    pk_record->assist_string	= str_dup( assists+1 );
	    pk_record->level[0] 	= killer->level;
	    pk_record->level[1]		= victim->level;
	    pk_record->pkill_time	= current_time;
	    pk_record->pkill_points	= value;
	    pk_record->bounty		= victim->pcdata->bounty;
	    pk_record->next		= killer->pcdata->kills_list;
	    killer->pcdata->kills_list	= pk_record;

	    sprintf( pkbuf, "pkills %s for %d pkpoints.", victim->name, value );
	    append_file( killer, PKLOG, pkbuf );

	    pk_record			= new_pk_record( );
	    pk_record->killer_name	= str_dup( killer->name );
	    pk_record->victim_name	= str_dup( victim->name );
	    pk_record->killer_clan	= str_dup( clan_table[killer->clan].color );
	    pk_record->victim_clan	= str_dup( clan_table[victim->clan].color );
	    pk_record->assist_string	= str_dup( assists+1 );
	    pk_record->level[0] 	= killer->level;
	    pk_record->level[1]		= victim->level;
	    pk_record->pkill_time	= current_time;
	    pk_record->pkill_points	= value;
	    pk_record->bounty		= victim->pcdata->bounty;
	    add_recent( pk_record );

	    rank_chart( killer, "pkills",	killer->pcdata->pkills	 );
	    rank_chart( victim,	"pdeaths",	victim->pcdata->pdeath	 );

	    if ( is_clan( killer ) && is_clan( victim ) )
	    {
		pk_record		= new_pk_record( );
		pk_record->killer_name	= str_dup( killer->name );
		pk_record->victim_name	= str_dup( victim->name );
		pk_record->killer_clan	= str_dup( clan_table[killer->clan].color );
		pk_record->victim_clan	= str_dup( clan_table[victim->clan].color );
		pk_record->assist_string= str_dup( assists+1 );
		pk_record->level[0] 	= killer->level;
		pk_record->level[1]	= victim->level;
		pk_record->pkill_time	= current_time;
		pk_record->pkill_points	= value;
		pk_record->bounty	= victim->pcdata->bounty;
		pk_record->next		= clan_table[victim->clan].death_list;
		clan_table[victim->clan].death_list = pk_record;

		pk_record		= new_pk_record();
		pk_record->killer_name	= str_dup( killer->name );
		pk_record->victim_name	= str_dup( victim->name );
		pk_record->killer_clan	= str_dup( clan_table[killer->clan].color );
		pk_record->victim_clan	= str_dup( clan_table[victim->clan].color );
		pk_record->assist_string= str_dup( assists+1 );
		pk_record->level[0] 	= killer->level;
		pk_record->level[1]	= victim->level;
		pk_record->pkill_time	= current_time;
		pk_record->pkill_points	= value;
		pk_record->bounty	= victim->pcdata->bounty;
		pk_record->next		= clan_table[killer->clan].kills_list;
		clan_table[killer->clan].kills_list = pk_record;

		check_clandeath( killer, victim );
	    }
	} else {
	    sprintf( pkbuf, "pkills %s.", victim->name );
	    append_file( killer, PKLOG, pkbuf );
	}

	soul = create_object( get_obj_index( 43 ) );

	sprintf( buf, soul->short_descr, victim->name, killer->name );
    	free_string( soul->short_descr );
    	soul->short_descr = str_dup( buf );

	sprintf( buf, soul->description, victim->name );
    	free_string( soul->description );
    	soul->description = str_dup( buf );

	sprintf( buf, soul->name, victim->name, killer->name );
    	free_string( soul->name );
    	soul->name = str_dup( buf );

	obj_to_char( soul, killer );

	sprintf(buf, "{RYou have captured the soul of {G%s{R!{x\n\r",
	    victim->name );
	send_to_char( buf, killer );

	sprintf(buf, "{RYou surrender your soul to {G%s{R.{x\n\r",
	    killer->name );
	send_to_char( buf, victim );

	if ( victim->pcdata->bounty > 0 )
	{
	    sprintf( buf, "$N recieves a {#%d{x {8pl{7a{&ti{7n{8um{x bounty that was placed against %s!",
		victim->pcdata->bounty, victim->name );
	    info( buf, killer, victim, INFO_BOUNTY );

	    sprintf( buf, "You recive a {#%d{x {8pl{7a{&ti{7n{8um{x bounty, for killing %s.\n\r",
		victim->pcdata->bounty, victim->name );
	    send_to_char( buf, killer );

	    killer->platinum += victim->pcdata->bounty;
	    victim->pcdata->bounty = 0;
	    rank_chart( victim, "bounty", 0 );
	}

	if ( killer->pcdata->opponent != NULL
	&&   killer->pcdata->opponent == victim )
	{
	    killer->pcdata->opponent = NULL;
	    killer->pcdata->pktimer = 0;
	}
	victim->pcdata->opponent = NULL;
	victim->pcdata->pktimer = 0;

	WAIT_STATE( victim, 6 * PULSE_VIOLENCE );
    }

    victim->pcdata->dtimer = 10;
    sprintf( buf, "{RASSASSINATION{7: {x%s%s {7has just been assassinated by {x%s%s{7!{x",
	victim->pcdata->pretitle, victim->name, ( IS_NPC( killer ) ? "" :
	killer->pcdata->pretitle ), ( IS_NPC( killer ) ?
	killer->short_descr : killer->name ) );
    info( buf, killer, victim, INFO_DEATHS );

    extract_char( victim, FALSE );

    while ( victim->affected )
	affect_remove( victim, victim->affected );

    racial_spells( victim, TRUE );
    do_devote_assign( victim );

    for ( i = 0; i < 4; i++ )
    	victim->armor[i]= 100;

    victim->position	= POS_RESTING;
    victim->hit		= UMAX( 1, victim->hit  );
    victim->mana	= UMAX( 1, victim->mana );
    victim->move	= UMAX( 1, victim->move );

    return;
}

void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt,
	bool immune, char *special_string )
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs, *vp, *attack;
    char punct;

    if ( ch == NULL || victim == NULL )
	return;

    if ( ch->pcdata && dam > ch->pcdata->max_damage )
    {
	ch->pcdata->max_damage = dam;
	rank_chart( ch, "damage", dam );
    }

    if ( dam < 0 )
    {
	vs = "{YREVIVE";
	vp = "{YREVIVES";
    }

    else if ( dam == 0 )
    {
	vs = "{7miss";
	vp = "{7misses";
    }

    else if ( dam <= 5 )
    {
	vs = "{1scratch";
	vp = "{1scratches";
    }

    else if ( dam <= 10 )
    {
	vs = "{@graze";
	vp = "{@grazes";
    }

    else if ( dam <= 15 )
    {
	vs = "{6hit";
	vp = "{6hits";
    }

    else if ( dam <= 20 )
    {
	vs = "{1i{!n{1ju{!r{1e";
	vp = "{1i{!n{1jur{!e{1s";
    }

    else if ( dam <= 30 )
    {
	vs = "{8w{7oun{8d";
	vp = "{8w{7ound{8s";
    }

    else if ( dam <= 40 )
    {
	vs = "{5maul";
	vp = "{5mauls";
    }

    else if ( dam <= 50 )
    {
	vs = "{6d{^e{6cima{^t{6e";
	vp = "{6d{^e{6cimat{^e{6s";
    }

    else if ( dam <= 60 )
    {
	vs = "{4d{$e{4vasta{$t{4e";
	vp = "{4d{$e{4vastat{$e{4s";
    }

    else if ( dam <= 70 )
    {
	vs = "{%m{5ai{%m";
	vp = "{%m{5aim{%s";
    }

    else if ( dam <= 80 )
    {
	vs = "{8M{7U{8T{7I{8L{7A{8T{7E";
	vp = "{8M{7U{8T{7I{8L{7A{8T{7E{8S";
    }

    else if ( dam <= 90 )
    {
	vs = "{5D{%I{5SE{%MB{5OW{%E{5L";
	vp = "{5D{%I{5SEM{%B{5OWE{%L{5S";
    }

    else if ( dam <= 100 )
    {
	vs = "{!D{1I{7S{8MEM{7B{1E{!R";
	vp = "{!D{1I{7S{8MEMB{7E{1R{!S";
    }

    else if ( dam <= 110 )
    {
	vs = "{7M{&A{7SSAC{&R{7E";
	vp = "{7M{&A{7SSACR{&E{7S";
    }

    else if ( dam <= 120 )
    {
	vs = "{1<-{$M{4A{$NG{4L{$E{1->";
	vp = "{1<-{$M{4A{$NGL{4E{$S{1->";
    }

    else if ( dam <= 130 )
    {
	vs = "{%*** {1D{!E{1M{!O{1L{!I{1S{!H {%***";
	vp = "{%*** {1D{!E{1M{!O{1L{!I{1S{!H{1E{!S {%***";
    }

    else if ( dam <= 140 )
    {
	vs = "{6*** {#D{3E{#VASTA{3T{#E {6***";
	vp = "{6*** {#D{3E{#VASTAT{3E{#S {6***";
    }

    else if ( dam <= 150 )
    {
	vs = "{7={&={7= {@O{2B{@L{2I{@T{2E{@R{2A{@T{2E {7={&={7=";
	vp = "{7={&={7= {@O{2B{@L{2I{@T{2E{@R{2A{@T{2E{@S {7={&={7=";
    }

    else if ( dam <= 160 )
    {
	vs = "{$>{4>{$> {!A{1N{!N{1I{!H{1I{!L{1A{!T{1E {$<{4<{$<";
	vp = "{$>{4>{$> {!A{1N{!N{1I{!H{1I{!L{1A{!T{1E{!S {$<{4<{$<";
    }

    else if ( dam <= 170 )
    {
	vs = "{^<{6<{^< {$E{4R{$A{4D{$I{4C{$A{4T{$E {^>{6>{^>";
	vp = "{^<{6<{^< {$E{4R{$A{4D{$I{4C{$A{4T{$E{4S {^>{6>{^>";
    }

    else if ( dam <= 180 )
    {
	vs = "{1-{!={7*{8> {&G{7R{&A{7T{&E {8<{7*{!={1-";
	vp = "{1-{!={7*{8> {&G{7R{&AT{7E{&S {8<{7*{!={1-";
    }

    else if ( dam <= 190 )
    {
	vs = "{y->{G*{y<- {rP{mE{DR{wF{WO{wR{DA{mT{rE {y->{G*{y<-";
	vp = "{y->{G*{y<- {rP{mE{DR{wF{WOR{wA{DT{mE{rS {y->{G*{y<-";
    }

    else if ( dam <= 220 )
    {
	vs = "{r({Y#{r<{G+{r>{Y#{r) {BEXTERMINATE {r({Y#{r<{G+{r>{Y#{r)";
	vp = "{r({Y#{r<{G+{r>{Y#{r) {BEXTERMINATES {r({Y#{r<{G+{r>{Y#{r)";
    }

    else if ( dam <= 240 )
    {
	vs = "{R[{r-{R[{r={yE{rX{yTIRPA{rT{yE{r={R]{r-{R]";
	vp = "{R[{r-{R[{r={yE{rX{yTIRPAT{rE{yS{r={R]{r-{R]";
    }

    else if ( dam <= 265 )
    {
	vs = "{w--{D=< {cA{CT{cOMI{CZ{cE {D>={w--";
	vp = "{w--{D=< {cA{CT{cOMIZ{CE{cS {D>={w--";
    }

    else if ( dam <= 285 )
    {
	vs = "{r-={R<{g@{R>{r=- {GE{gF{GFA{gC{GE {r-={R<{g@{R>{r=-";
	vp = "{r-={R<{g@{R>{r=- {GE{gF{GFAC{gE{GS {r-={R<{g@{R>{r=-";
    }

    else if ( dam <= 320 )
    {
	vs = "{y/{Y\\{y/ {rE{RV{rI{RS{rCE{RR{rA{RT{rE {y\\{Y/{y\\";
	vp = "{y/{Y\\{y/ {rE{RV{rI{RS{rCER{RA{rT{RE{rS {y\\{Y/{y\\";
    }

    else if ( dam <= 350 )
    {
	vs = "{R>{w={W-{w={W-{w={R> {mR{DA{mVA{DG{mE {R<{w={W-{w={W-{w={R<";
	vp = "{R>{w={W-{w={W-{w={R> {mR{DA{mVAG{DE{mS {R<{w={W-{w={W-{w={R<";
    }

    else if ( dam <= 400 )
    {
	vs = "{1<{7-{&*{7-{1> {$I{4M{$P{4A{$L{4E {1<{7-{&*{7-{1>";
	vp = "{1<{7-{&*{7-{1> {$I{4M{$P{4A{$L{4E{$S {1<{7-{&*{7-{1>";
    }

    else if ( dam <= 450 )
    {
	vs = "{R-{r+{R- {BSMITE {R-{r+{R-";
	vp = "{R-{r+{R- {BSMITES {R-{r+{R-";
    }

    else if ( dam <= 500 )
    {
	vs = "{^-={W*{^=- {6LIQUIDATE {^-={W*{^=-";
	vp = "{^-={W*{^=- {6LIQUIDATES {^-={W*{^=-";
    }

    else if ( dam <= 580 )
    {
	vs = "{6-{^={cB{CL{cE{CM{cI{CS{cH{^={6-";
	vp = "{6-{^={6B{^L{6E{^M{6I{^S{6H{^E{6S{^={6-";
    }

    else if ( dam <= 665 )
    {
	vs = "{!-{1={!+{1*{!/{$D{4I{$S{4F{$I{4G{$U{4R{$E{!\{1*{!+{1={!-";
	vp = "{!-{1={!+{1*{!/{$D{4I{$S{4F{$I{4G{$U{4R{$E{4S{!\{1*{!+{1={!-";
    }

    else if ( dam == 666 )
    {
	vs = "{#C{3A{#L{3L {#F{3O{#R{3T{#H {!D{1E{!M{1O{!N{1S {#O{3F {!H{1E{!L{1L {#U{3P{#O{3N";
	vp = "{#C{3A{#L{3L{#S {3F{#O{3R{#T{3H {!D{1E{!M{1O{!N{1S {#O{3F {!H{1E{!L{1L {#U{3P{#O{3N";
    }

    else if ( dam <= 735 )
    {
	vs = "{b*{B#{b* {rW{RR{rE{RA{rK H{RA{rV{RO{rC {RON {b*{B#{b*";
	vp = "{b*{B#{b* {rW{RR{rEA{RK{rS H{RA{rV{RO{rC {RON {b*{B#{b*";
    }

    else if ( dam <= 776 )
    {
	vs = "{8BEAT THE {^LIVING {#DAYLIGHTS {@outta";
	vp = "{8BEATS THE {^LIVING {#DAYLIGHTS {@outta";
    }

    else if ( dam == 777 )
    {
	vs = "{3D{#R{3O{#P {#{zBARS OF GOLD{x {#O{3N {#T{3H{#E {3H{#E{3A{#D {3O{#F";
	vp = "{3D{#R{3O{#P{3S {#{zBARS OF GOLD{x {#O{3N {#T{3H{#E {3H{#E{3A{#D {3O{#F";
    }

    else if ( dam <= 850 )
    {
	vs = "{8<{7-{8={7*{8+{7^ {%R{5AZ{%E {8^{7+{8*{7={8-{7>";
	vp = "{8<{7-{8={7*{8+{7^ {%R{5A{%Z{5E{%S {8^{7+{8*{7={8-{7>";
    }

    else if ( dam <= 975 )
    {
	vs = "{W<->{!*{W<=> {!CREMATE {W<=>{!*{W<->";
	vp = "{W<->{!*{W<=> {!CREMATES {W<=>{!*{W<->";
    }

    else if ( dam <= 1100 )
    {
	vs = "{W<{8=-{W|{8-={W> {!SUNDER {W<{8=-{W|{8-={W>";
	vp = "{W<{8=-{W|{8-={W> {!SUNDERS {W<{8=-{W|{8-={W>";
    }

    else if ( dam <= 1250 )
    {
	vs = "{M<{8-{M><{8-=-{M><{8-{M> {8IMP{7L{8ODE {M<{8-{M><{8-=-{M><{8-{M>";
	vp = "{M<{8-{M><{8-=-{M><{8-{M> {8IMP{7LO{8DES {M<{8-{M><{8-=-{M><{8-{M>";
    }

    else if ( dam <= 1400 )
    {
	vs = "{$<{4*{$> {@P{2I{@L{2L{@A{2G{@E {$<{4*{$>";
	vp = "{$<{4*{$> {@P{2I{@L{2L{@A{2G{@E{2S {$<{4*{$>";
    }

    else if ( dam <= 1650 )
    {
	vs = "{B[{!@@{W*{!@@{B]{W-=> {8RUP{7T{8URE {W<=-{B[{!@@{W*{!@@{B]";
	vp = "{B[{!@@{W*{!@@{B]{W-=> {8RUP{7TU{8RES {W<=-{B[{!@@{W*{!@@{B]";
    }

    else if ( dam <= 3500 )
    {
	vs = "{6do {$U{4N{8S{7PE{&A{7KA{8B{4L{$E {6things to";
	vp = "{6does {$U{4N{8S{7PE{&A{7KA{8B{4L{$E {6things to";
    }

    else
    {
	vs = "{8B{7R{8I{7N{8G F{7O{8R{7T{8H T{7H{8E {^D{6I{^V{6I{^N{6E {!F{1U{!R{1Y {8O{7F $G {8A{7G{8A{7I{8N{7S{8T";
	vp = "{8B{7R{8I{7N{8G{7S {8F{7O{8R{7T{8H T{7H{8E {^D{6I{^V{6I{^N{6E {!F{1U{!R{1Y {8O{7F $G {8A{7G{8A{7I{8N{7S{8T";
    }

    punct = ( dam <= 80 ) ? '.' : '!';

    if ( dt == TYPE_HIT )
    {
	if ( ch == victim )
	{
	    sprintf(buf1, "$n %s $melf%c {D({WDam{D:{W%d{D){x",vp,punct,dam);
	    sprintf(buf2, "{BYou %s {Byourself%c {D({WDam{D:{W%d{D){x",vs,punct,dam);
	} else {
	    sprintf(buf1, "{k$n %s $N%c {D({WDam{D:{W%d{D){x",  vp, punct, dam);
	    sprintf(buf2, "{BYou %s {B$N{B%c {D({WDam{D:{W%d{D){x", vs, punct, dam);
	    sprintf(buf3, "{c$n %s {cyou{c%c {D({WDam{D:{W%d{D){x", vp, punct, dam);
	}
    } else {
	if ( special_string != NULL )
	    attack = special_string;

	else if ( dt >= 0 && dt < maxSkill )
	    attack = skill_table[dt].noun_damage;

	else if ( dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE )
	    attack = attack_table[dt - TYPE_HIT].noun;

	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt = TYPE_HIT;
	    attack = attack_table[0].name;
	}

	if ( immune )
	{
	    if ( ch == victim )
	    {
		sprintf( buf1, "{k$n is unaffected by $s own %s.{x", attack );
		sprintf( buf2, "{BLuckily, you are immune to that.{x" );
	    } else {
 	    	sprintf( buf1, "{k$N is unaffected by $n's %s!{x",attack );
	    	sprintf( buf2, "{B$N {Bis unaffected by your %s!{x",attack );
	    	sprintf( buf3, "{c$n{c's %s is powerless against you.{x",attack );
	    }
	} else {
	    if ( ch == victim )
	    {
		sprintf( buf1, "{k$n{k's %s %s {k$m{k%c {D({WDam:%d{D){x",
		    attack, vp, punct, dam );
		sprintf( buf2, "{hYour %s %s {hyou%c {D({WDam:%d{D){x",
		    attack, vp, punct, dam );
	    } else {
	    	sprintf( buf1, "{k$n{k's %s %s {k$N{k%c {D({WDam:%d{D){x",
		    attack, vp, punct, dam );
	    	sprintf( buf2, "{hYour %s %s {h$N{h%c {D({WDam:%d{D){x",
		    attack, vp, punct, dam );
	    	sprintf( buf3, "{i$n{i's %s %s {iyou%c {D({WDam:%d{D){x",
		    attack, vp, punct, dam );
	    }
	}
    }

    if ( ch == victim )
    {
	combat( buf1, ch, NULL, NULL, TO_ROOM, COMBAT_OTHER );
	act( buf2, ch, NULL, NULL, TO_CHAR, POS_RESTING );
    }

    else
    if ( dt == gsn_acidshield	  || dt == gsn_divine_aura
    ||   dt == gsn_fireshield	  || dt == gsn_iceshield
    ||   dt == gsn_rockshield	  || dt == gsn_shockshield
    ||   dt == gsn_shrapnelshield || dt == gsn_thornshield
    ||   dt == gsn_vampiricshield || dt == gsn_watershield )
    {
	if ( ch->desc )
	    ch->desc->shd_damage[0] += dam;

	if ( victim->desc )
	    victim->desc->shd_damage[1] += dam;

	combat( buf1, ch, NULL, victim, TO_NOTVICT, COMBAT_SHIELDS );
	combat( buf2, ch, NULL, victim, TO_CHAR, COMBAT_SHIELDS );
	combat( buf3, ch, NULL, victim, TO_VICT, COMBAT_SHIELDS );
    }

    else if ( dam == 0 )
    {
    	combat( buf1, ch, NULL, victim, TO_NOTVICT, COMBAT_MISSES );
    	combat( buf2, ch, NULL, victim, TO_CHAR, COMBAT_MISSES );
    	combat( buf3, ch, NULL, victim, TO_VICT, COMBAT_MISSES );
    }

    else
    {
    	combat( buf1, ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
    	act( buf2, ch, NULL, victim, TO_CHAR, POS_RESTING );
    	act( buf3, ch, NULL, victim, TO_VICT, POS_RESTING );
    }

    return;
}

bool damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt,
	int dam_type, bool show, bool secondary, char *special_string )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    bool immune;
    int chance;

    if ( victim == NULL
    ||   ch == NULL
    ||   victim->in_room == NULL
    ||   victim->position == POS_DEAD )
	return FALSE;

    if ( !IS_NPC(victim)
    &&   !IS_IMMORTAL(ch)
    &&   victim->pcdata->dtimer > 0
    &&   !IS_SET(victim->in_room->room_flags, ROOM_ARENA)
    &&   !IS_SET(victim->in_room->room_flags, ROOM_WAR) )
	return FALSE;

    if ( dt != gsn_hurl && dt != gsn_trapset && ch->in_room != NULL
    &&   ch->in_room != victim->in_room )
	return FALSE;

    if ( ch != victim && IS_AFFECTED( victim, AFF_SLEEP ) )
    {
	affect_strip( victim, gsn_sleep );
	affect_strip( victim, gsn_strangle );
    }

    if ( dam > 8000 && dt >= TYPE_HIT && !IS_IMMORTAL(ch) && !IS_NPC(ch) )
    {
	OBJ_DATA *obj;

	sprintf(buf, "%s: Damage: %d: (victim: %s) (weapon %s)!",
	ch->name,dam,victim->name,(get_eq_char(ch,WEAR_WIELD) != NULL) ?
        (get_eq_char(ch,WEAR_WIELD)->name) : "NONE" );
	bug( buf, 0 );
	dam = 8000;
	obj = get_eq_char( ch, WEAR_WIELD );
	send_to_char("{cYou {z{Breally{x{c shouldn't cheat.{x\n\r",ch);
	if (obj != NULL)
	    extract_obj(obj);
    }

    if ( victim != ch )
    {
	if ( is_safe( ch, victim ) )
	    return FALSE;

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
            {
		if ( dt != gsn_trapset )
		{
		    set_fighting( victim, ch );

		    if ( ch->fighting == NULL )
			set_fighting( ch, victim );

		    if ( IS_NPC( victim ) && HAS_TRIGGER_MOB( victim, TRIG_KILL ) )
			p_percent_trigger( victim, NULL, NULL, ch, NULL, NULL, TRIG_KILL );
		}
	    }

	    if ( victim->timer <= 4 && dt != gsn_trapset )
	    	victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED && dt != gsn_trapset )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );
	}

	if ( victim->master == ch )
	    stop_follower( victim );
    }

    if ( IS_SHIELDED( ch, SHD_INVISIBLE ) )
    {
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
	REMOVE_BIT( ch->shielded_by, SHD_INVISIBLE );
	act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM, POS_RESTING );
    }

    affect_strip( ch, gsn_obfuscate );
    affect_strip( ch, gsn_camouflage );
    affect_strip( ch, gsn_forest_meld );

    check_pktimer( ch, victim, TRUE );

    if ( !IS_NPC( ch ) && IS_NPC( victim ) )
	mob_remember( victim, ch, MEM_HOSTILE );
    
    if ( dt < TYPE_HIT
    &&   skill_table[dt].spell_fun != spell_null
    &&   ( chance = get_skill( ch, gsn_master_of_magic ) ) > 0 )
    {
        if ( number_percent( ) < chance )
	{
	    check_improve( ch, gsn_master_of_magic, TRUE, 15 );
	    dam = 11 * dam / 10;
	} else
	    check_improve( ch, gsn_master_of_magic, FALSE, 15 );
    }
    
    if ( dam > 35 )
	dam = ( dam - 35 ) / 2 + 35;
    if ( dam > 80 )
	dam = ( dam - 80 ) / 2 + 80;

    if ( IS_NPC( ch ) )
	dam = 7 * dam / 10;

    if ( dam > 1 )
    {
	if ( IS_SHIELDED( victim, SHD_DIVINITY ) )
	    dam /= 2;

	else if ( IS_SHIELDED( victim, SHD_SANCTUARY ) )
	    dam -= dam / 3;

	if ( (IS_SHIELDED(victim, SHD_PROTECT_EVIL) && IS_EVIL(ch))
	||   (IS_SHIELDED(victim, SHD_PROTECT_GOOD) && IS_GOOD(ch))
	||   (IS_SHIELDED(victim, SHD_PROTECT_NEUTRAL) && IS_NEUTRAL(ch)) )
	    dam -= dam / 4;

	if ( ch->pcdata != NULL )
	{
	    if ( ch->pcdata->condition[COND_DRUNK] > 10 )
		dam = 9 * dam / 10;

	    if ( ch->pcdata->condition[COND_HUNGER] == 0 )
		dam = 9 * dam / 10;

	    if ( ch->pcdata->condition[COND_THIRST] == 0 )
		dam = 9 * dam / 10;
	}
    }

    if ( victim->damage_mod[dam_type] == 0
    ||   (victim->damage_mod[dam_type] < 0 && ch == victim ) )
    {
	immune = TRUE;
	dam = 0;
    } else {
	immune = FALSE;
	dam = dam * victim->damage_mod[dam_type] / 100;
    }
    
    if ( IS_SHIELDED(victim,SHD_MANA) && show && dt >= TYPE_HIT && dam > 0 )
    {
	int chance = 45;

	if ( number_percent() < chance )
	{
	    char buf[MAX_STRING_LENGTH];
	    int absorb = ( dam / 10 ) * number_range(1,4);

	    if ( absorb > 0 && victim->mana >= absorb )
	    {
		sprintf(buf,"Your mana shield absorbs some of the damage! {D({WAbsorb{D:{W%d{D){x\n\r", absorb);
		send_to_char(buf,victim);
		sprintf(buf,"$n's mana shield absorbs some of the damage! {D({WAbsorb{D:{W%d{D){x", absorb);
		act(buf,victim,NULL,NULL,TO_ROOM,POS_RESTING);
		dam -= absorb;
		victim->mana -= absorb;
	    }
	}
    }

    if ( show )
    	dam_message( ch, victim, dam, dt, immune, special_string );

    if ( dam == 0 )
	return FALSE;

    if ( dt == gsn_feed )
    {
	ch->hit = UMIN(ch->hit+((dam/3)*2), ch->max_hit);
	update_pos( ch );
    }

    victim->hit -= dam;

    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1
    &&   victim->in_room != NULL
    &&   !IS_SET(victim->in_room->room_flags, ROOM_ARENA)
    &&   !IS_SET(victim->in_room->room_flags, ROOM_WAR) )
	victim->hit = 1;

    update_pos( victim );

    switch( victim->position )
    {
	case POS_MORTAL:
	    act( "{c$n is mortally wounded, and will die soon, if not aided.{x", victim, NULL, NULL, TO_ROOM,POS_RESTING);
	    send_to_char("{cYou are mortally wounded, and will die soon, if not aided.{x\n\r", victim);
	    break;

	case POS_INCAP:
	    act( "{c$n is incapacitated and will slowly die, if not aided.{x",victim, NULL, NULL, TO_ROOM,POS_RESTING);
	    send_to_char("{cYou are incapacitated and will slowly {z{Rdie{x{c, if not aided.{x\n\r", victim);
	    break;

	case POS_STUNNED:
	    act( "{c$n is stunned, but will probably recover.{x", victim, NULL, NULL, TO_ROOM,POS_RESTING);
	    send_to_char("{cYou are stunned, but will probably recover.{x\n\r", victim);
	    break;

	case POS_DEAD:
	    if ((IS_NPC(victim)) && ( victim->pIndexData->die_descr[0] != '\0'))
		act( "{c$n $T{x", victim, 0, victim->pIndexData->die_descr, TO_ROOM,POS_RESTING);
	    else
		act( "{c$n is {CDEAD!!{x", victim, 0, 0, TO_ROOM,POS_RESTING);
	    send_to_char( "{cYou have been {RKILLED!!{x\n\r\n\r", victim );
	    break;

	default:
	    if ( dam > victim->max_hit / 4 )
	    {
		combat("{cThat really did {RHURT!{x",
		    victim,NULL,NULL,TO_CHAR,COMBAT_BLEEDING);
	    }
	    if ( victim->hit < victim->max_hit / 4 )
	    {
		combat("{cYou sure are {z{RBLEEDING!{x",
		    victim,NULL,NULL,TO_CHAR,COMBAT_BLEEDING);
	    }
	    break;
    }

    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    if ( victim->position == POS_DEAD )
    {
	group_gain( ch, victim );

	if ( ch->in_room != NULL && victim->in_room != NULL
	&&   IS_SET(ch->in_room->room_flags,ROOM_ARENA)
	&&   IS_SET(victim->in_room->room_flags,ROOM_ARENA) )
	{
	    check_arena( ch, victim );
	    return TRUE;
	}

	if ( !IS_SET(victim->in_room->room_flags, ROOM_WAR) )
	{
            sprintf( log_buf, "%s{V got toasted by %s{V at %s{V [room %d]",
                (IS_NPC(victim) ? victim->short_descr : victim->name),
                (IS_NPC(ch) ? ch->short_descr : ch->name),
                ch->in_room->name, ch->in_room->vnum);

            if (IS_NPC(victim))
                wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
            else
	    {
                wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);
		sprintf( log_buf, "%s killed by %s at %d", victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name), ch->in_room->vnum );
		log_string( log_buf );
	    }
	}

	if ( IS_NPC( victim ) && HAS_TRIGGER_MOB( victim, TRIG_DEATH) )
	{
	    victim->position = POS_STANDING;
	    p_percent_trigger( victim, NULL, NULL, ch, NULL, NULL, TRIG_DEATH );
	}

	raw_kill( victim, ch );

	if ( ch != victim && !IS_NPC( ch ) && IS_SET( victim->act, PLR_TWIT ) )
	    REMOVE_BIT( victim->act, PLR_TWIT );

	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    OBJ_DATA *coins;

	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents );
	    
	    if ( IS_SET(ch->act, PLR_AUTOLOOT) && corpse && corpse->contains)
		do_get( ch, "all corpse" );

 	    if ( IS_SET(ch->act,PLR_AUTOGOLD)
	    &&   corpse && corpse->contains
	    &&   !IS_SET(ch->act,PLR_AUTOLOOT) )
		if ((coins = get_obj_list(ch,"coins",corpse->contains)) != NULL)
	      	    do_get(ch, "coins corpse");

	    if ( IS_SET(ch->act, PLR_AUTOSAC) )
	    {
		if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
		    return TRUE;
		else
		    do_sacrifice( ch, "corpse" );
	    }
	}
	return TRUE;
    }

    if ( victim == ch )
	return TRUE;

    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5)
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	    do_flee( victim, "" );
    }

    tail_chain( );
    return TRUE;
}

bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    if ( victim->in_room == NULL
    ||   ch->in_room == NULL
    ||   ( victim == ch && area )
    ||   ( area && is_same_group( ch, victim ) ) )
	return TRUE;

    if ( victim->fighting == ch
    ||   victim == ch
    ||   ( !IS_NPC( ch ) && IS_IMMORTAL( ch ) ) )
	return FALSE;

    if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA )
    &&   IS_SET( victim->in_room->room_flags, ROOM_ARENA ) )
    {
	if ( IS_NPC( ch ) )
	{
	    if ( ch->master != NULL && !IS_NPC( ch->master ) )
		ch = ch->master;
	    else
		return FALSE;
	}

	if ( IS_NPC( victim ) )
	{
	    if ( victim->master != NULL && !IS_NPC( victim->master ) )
		victim = victim->master;
	    else
		return FALSE;
	}

	if ( ch->pcdata->match != victim->pcdata->match )
	    return TRUE;

	if ( ch->pcdata->team == victim->pcdata->team )
	{
	    act( "You and $N are both on the SAME team!",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	    return TRUE;
	}

        return FALSE;
    }

    if ( IS_NPC( victim ) )
    {
	if ( IS_SET( victim->in_room->room_flags, ROOM_SAFE )
	||   victim->pIndexData->pShop != NULL )
	    return TRUE;

	if ( victim->clan == 0
	&&   ( IS_SET( victim->act, ACT_TRAIN )
	||     IS_SET( victim->act, ACT_PRACTICE )
	||     IS_SET( victim->act, ACT_IS_HEALER )
	||     IS_SET( victim->act, ACT_IS_SATAN )
	||     IS_SET( victim->act, ACT_IS_PRIEST ) ) )
	    return TRUE;

	if ( !IS_NPC( ch ) )
	{
	    if ( IS_SET( victim->act, ACT_PET )
	    ||   ( IS_AFFECTED( victim, AFF_CHARM ) && ( area || ch != victim->master ) )
	    ||   ( victim->fighting != NULL && !is_same_group( ch, victim->fighting ) ) )
		return TRUE;
	}
	else
	{
	    if ( area && !is_same_group( victim, ch->fighting ) )
		return TRUE;
	}
    }
    else
    {
	if ( area && victim->level >= LEVEL_IMMORTAL )
	    return TRUE;

	if ( IS_NPC( ch ) )
	{
	    if ( IS_AFFECTED( ch, AFF_CHARM )
	    &&   ch->master != NULL
	    &&   ch->master->fighting != victim )
		return TRUE;

	    if ( IS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
		return TRUE;

	    if ( ch->fighting != NULL && !is_same_group( ch->fighting, victim ) )
		return TRUE;
	}
	else
	{
	    PKILL_DATA *pkill;

	    if ( IS_SET( victim->act,PLR_TWIT )
	    ||   is_voodood( ch, victim ) )
		return FALSE;

	    for ( pkill = ch->pcdata->recent_pkills; pkill != NULL; pkill = pkill->next )
	    {
		if ( !str_cmp( victim->name, pkill->player_name ) )
		{
		    if ( pkill->killer )
			send_to_char( "You have already killed that person once within the last hour!\n\r", ch );
		    else
			send_to_char( "That person has already killed you once within the past hour!\n\r", ch );
		    return TRUE;
		}
	    }

	    if ( !is_clan( ch )
	    ||   !is_pkill( ch )
            ||   IS_SET( victim->in_room->room_flags, ROOM_SAFE )
            ||   ch->pcdata->on_quest
            ||   victim->pcdata->on_quest
	    ||   !is_clan( victim )
	    ||   !is_pkill( victim ) )
		return TRUE;

	    if ( ch->pcdata->tier == victim->pcdata->tier )
	    {
		if ( ch->level > victim->level + 15
		||   ch->level < victim->level - 15 )
		{
		    send_to_char( "Pick on someone your own size.\n\r", ch );
		    return TRUE;
		}
	    }

	    if ( ch->pcdata->tier - victim->pcdata->tier == 1
	    ||   ch->pcdata->tier - victim->pcdata->tier == -1 )
	    {
		if ( ch->level > victim->level + 10
		||   ch->level < victim->level - 10 )
		{
		    send_to_char( "Pick on someone your own size.\n\r", ch );
		    return TRUE;
		}
	    }

	    if ( ch->pcdata->tier - victim->pcdata->tier == 2
	    ||   ch->pcdata->tier - victim->pcdata->tier == -2 )
	    {
		if ( ch->level > victim->level + 5
		||   ch->level < victim->level - 5 )
		{
		    send_to_char( "Pick on someone your own size.\n\r", ch );
		    return TRUE;
		}
	    }
	}
    }

    return FALSE;
}

void update_pos( CHAR_DATA *victim )
{
    if ( IS_AFFECTED(victim, AFF_SLEEP) && victim->hit > 0 )
    {
	victim->position = POS_SLEEPING;
	return;
    }

    if ( victim->hit > 0 )
    {
    	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_STANDING;
	return;
    }

    if ( IS_NPC(victim) && victim->hit < 1 )
    {
	victim->position = POS_DEAD;
	return;
    }

    if ( victim->hit <= -11 )		victim->position = POS_DEAD;
    else if ( victim->hit <= -6 )	victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 )	victim->position = POS_INCAP;
    else				victim->position = POS_STUNNED;
}

void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fighting != NULL )
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
    {
	affect_strip( ch, gsn_sleep );
	affect_strip( ch, gsn_strangle );
    }

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    return;
}

void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch, *tch;
    char buf[MAX_STRING_LENGTH];
    bool fight;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch == ch || ( fBoth && fch->fighting == ch ) )
	{
	    fight = FALSE;
	    for (tch = fch->in_room->people; tch != NULL; tch = tch->next_in_room)
	    {
		if (fch != ch && tch->fighting && tch->fighting == fch)
		{
		    fight = TRUE;
		    break;
		}
	    }

	    if (!fight)
	    {
		fch->fighting	= NULL;
		fch->position	= IS_NPC(fch) ? fch->default_pos : POS_STANDING;
	    } else {
		fch->fighting	= tch;
		fch->position	= POS_FIGHTING;
	    }
	    update_pos( fch );

	    if ( fch->position != POS_FIGHTING && fch->pcdata && fch->pcdata->tells
	    &&   IS_SET(fch->configure,CONFIG_STORE) )
	    {
		sprintf( buf, "You have {R%d{x tells waiting.\n\r", fch->pcdata->tells );
		send_to_char( buf, fch );
		send_to_char("Type 'replay' to see tells.\n\r",fch);
	    }
	}
    }

    return;
}

void disarm( CHAR_DATA *ch, CHAR_DATA *victim, const int bitname )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, bitname ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
	act("{j$S{j weapon won't budge!{x",ch,NULL,victim,TO_CHAR,POS_RESTING);
	act("{j$n{j tries to disarm you, but your weapon won't budge!{x",
	    ch,NULL,victim,TO_VICT,POS_RESTING);
	combat("{k$n{k tries to disarm $N{k, but fails.{x",
	    ch,NULL,victim,TO_NOTVICT,COMBAT_OTHER);
	return;
    }

    act( "{j$n{j DISARMS you and sends your weapon flying!{x",
	 ch, NULL, victim, TO_VICT,POS_RESTING);
    act( "{jYou disarm $N{j!{x",  ch, NULL, victim, TO_CHAR,POS_RESTING);
    combat("{k$n{k disarms $N!{x",
	ch,NULL,victim,TO_NOTVICT,COMBAT_OTHER);

    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY)
    ||   IS_OBJ_STAT(obj,ITEM_AQUEST) || IS_OBJ_STAT(obj,ITEM_FORGED)
    ||   IS_IMMORTAL(victim) )
    {
	act( "{cA magical aura draws $p {cto you.{x",
	    victim, obj, NULL, TO_CHAR, POS_DEAD );
	act( "{cA magical aura draws $p {cto $n{c.{x",
	    victim, obj, NULL, TO_ROOM, POS_RESTING );
	obj_to_char( obj, victim );
    } else {
	obj->disarmed_from = victim;
	set_obj_sockets( victim, obj );
	set_arena_obj( victim, obj );
	obj_to_room( obj, victim->in_room );
	if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
	    send_to_char( get_obj(victim,obj,NULL,FALSE), victim );
    }

    if ( get_skill( victim, gsn_quickdraw ) > 0 )
    {
	if ( bitname == WEAR_WIELD
	&&   get_eq_char( victim, WEAR_SHEATH_1 ) != NULL )
	    do_draw( victim, "primary" );
	else if ( bitname == WEAR_SECONDARY
	     &&   get_eq_char( victim, WEAR_SHEATH_2 ) != NULL )
	    do_draw( victim, "secondary" );
    }

    return;
}

void do_berserk( CHAR_DATA *ch, char *argument )
{
    int chance, hp_percent;

    if ( ( chance = get_skill( ch, gsn_berserk ) ) == 0 )
    {
	send_to_char( "{hYou turn {rred{h in the face, but nothing happens.{x\n\r", ch );
	return;
    }

    if ( is_affected( ch, gsn_berserk ) )
    {
	send_to_char( "{hYou get a little madder.{x\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CALM ) )
    {
	send_to_char( "{hYou're feeling to mellow to berserk.{x\n\r", ch );
	return;
    }

    if ( !cost_of_skill( ch, gsn_berserk ) )
	return;

    /* modifiers */

    /* fighting */
    if ( ch->position == POS_FIGHTING )
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if ( number_percent( ) < chance )
    {
	AFFECT_DATA af;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN( ch->hit, ch->max_hit );

	send_to_char( "{hYour pulse races as you are consumed by {rrage!{x\n\r", ch );
	act( "{k$n gets a {cw{gi{rl{yd{k look in $s eyes.{x",
	    ch, NULL, NULL, TO_ROOM, POS_RESTING );
	check_improve( ch, gsn_berserk, TRUE, 2 );

	af.where	= TO_AFFECTS;
	af.type		= gsn_berserk;
	af.level	= ch->level;
	af.dur_type	= DUR_TICKS;
	af.duration	= ch->level / 8;
	af.modifier	= ch->level;
	af.bitvector 	= 0;
	af.location	= APPLY_HITROLL;
	affect_to_char( ch, &af );

	af.location	= APPLY_DAMROLL;
	affect_to_char( ch, &af );

	af.modifier	= UMAX( 10, 10 * ( ch->level / 5 ) );
	af.location	= APPLY_AC;
	affect_to_char( ch, &af );
    } else {
	send_to_char( "{hYour pulse speeds up, but nothing happens.{x\n\r", ch );
	check_improve( ch, gsn_berserk, FALSE, 2 );
    }
}

void do_vdpi( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    AFFECT_DATA af;
    bool found = FALSE;

    argument = one_argument( argument, arg1 );

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;

	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;

	wch = ( d->original != NULL ) ? d->original : d->character;

	if (!can_see(ch,wch))
	    continue;

	if (!str_cmp(arg1,wch->name) && !found)
	{
	    if (IS_NPC(wch))
		continue;

	    if (IS_IMMORTAL(wch) && (wch->level > ch->level))
	    {
		send_to_char( "That's not a good idea.\n\r", ch);
		return;
	    }

	    if (!can_pk(ch,wch) && !IS_IMMORTAL(ch))
	    {
		send_to_char("Voodoo dolls can not be used out of pk range.\n\r",ch);
		return;
	    }

	    if (IS_SHIELDED(wch,SHD_PROTECT_VOODOO))
	    {
		send_to_char( "They are still realing from a previous voodoo.\n\r",ch);
		return;
	    }

	    found = TRUE;

	    send_to_char( "You stick a pin into your voodoo doll.\n\r",ch);
	    act( "$n sticks a pin into a voodoo doll.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    send_to_char( "{RYou double over with a sudden pain in your gut!{x\n\r",wch);
	    act( "$n suddenly doubles over with a look of extreme pain!",wch, NULL,NULL,TO_ROOM,POS_RESTING);

	    if ( IS_IMMORTAL(ch) )
		return;

	    af.where      = TO_SHIELDS;
	    af.type       = skill_lookup("protection voodoo");
	    af.level      = wch->level;
	    af.duration   = 1;
	    af.dur_type   = DUR_TICKS;
	    af.location   = APPLY_NONE;
	    af.modifier   = 0;
	    af.bitvector  = SHD_PROTECT_VOODOO;
	    affect_to_char(wch,&af);
	    wch->hit  = wch->hit * 9/10;
	    wch->mana = wch->mana * 9/10;
	    wch->move = wch->move * 9/10;
	    return;
	}
    }
    send_to_char("Your victim doesn't seem to be in the realm.\n\r",ch);
    return;
}

void do_vdtr( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    AFFECT_DATA af;
    bool found = FALSE;

    argument = one_argument( argument, arg1 );

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;

	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;

	wch = ( d->original != NULL ) ? d->original : d->character;

	if (!can_see(ch,wch))
	    continue;

	if (!str_cmp(arg1,wch->name) && !found)
	{
	    if (IS_NPC(wch))
		continue;

	    if (!can_pk(ch,wch) && !IS_IMMORTAL(ch))
	    {
		send_to_char("Voodoo dolls can not be used out of pk range.\n\r",ch);
		return;
	    }

	    if (IS_IMMORTAL(wch) && (wch->level > ch->level))
	    {
		send_to_char( "That's not a good idea.\n\r", ch);
		return;
	    }

	    if (IS_SHIELDED(wch,SHD_PROTECT_VOODOO))
	    {
                send_to_char( "They are still realing from a previous voodoo.\n\r",ch);
		return;
	    }

	    found = TRUE;

	    send_to_char( "You slam your voodoo doll against the ground.\n\r",ch);
	    act( "$n slams a voodoo doll against the ground.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    send_to_char( "{RYour feet slide out from under you!{x\n\r",wch);
	    send_to_char( "{RYou hit the ground face first!{x\n\r",wch);
	    act( "$n trips over $s own feet, and does a nose dive into the ground!",wch, NULL,NULL,TO_ROOM,POS_RESTING);

	    if ( IS_IMMORTAL(ch) )
		return;

            af.where      = TO_SHIELDS;
            af.type       = skill_lookup("protection voodoo");
            af.level      = wch->level;
	    af.dur_type   = DUR_TICKS;
            af.duration   = 1;
            af.location   = APPLY_NONE;
            af.modifier   = 0;
            af.bitvector  = SHD_PROTECT_VOODOO;
            affect_to_char(wch,&af);
	    DAZE_STATE(wch, 6 * PULSE_VIOLENCE, DAZE_SKILL);
	    DAZE_STATE(wch, 6 * PULSE_VIOLENCE, DAZE_SPELL);
	    DAZE_STATE(wch, 6 * PULSE_VIOLENCE, DAZE_FLEE);
	    return;
	}
    }
    send_to_char("Your victim doesn't seem to be in the realm.\n\r",ch);
    return;
}

void do_vdth( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    AFFECT_DATA af;
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    bool found = FALSE;
    int attempt;

    argument = one_argument( argument, arg1 );

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;

	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;

	wch = ( d->original != NULL ) ? d->original : d->character;

	if (!can_see(ch,wch))
	    continue;

	if (!str_cmp(arg1,wch->name) && !found)
	{
	    if (IS_NPC(wch))
		continue;

	    if (IS_IMMORTAL(wch) && (wch->level > ch->level))
	    {
		send_to_char( "That's not a good idea.\n\r", ch);
		return;
	    }

	    if (!can_pk(ch,wch))
	    {
		send_to_char("Voodoo dolls can not be used out of pk range.\n\r",ch);
		return;
	    }

	    if (IS_SHIELDED(wch,SHD_PROTECT_VOODOO))
	    {
                send_to_char( "They are still realing from a previous voodoo.\n\r",ch);
		return;
	    }

	    found = TRUE;

	    send_to_char( "You toss your voodoo doll into the air.\n\r",ch);
	    act( "$n tosses a voodoo doll into the air.", ch, NULL, NULL, TO_ROOM,POS_RESTING);

	    if ( !IS_IMMORTAL(ch) )
	    {
		af.where      = TO_SHIELDS;
		af.type       = skill_lookup("protection voodoo");
		af.level      = wch->level;
	 	af.dur_type   = DUR_TICKS;
		af.duration   = 1;
		af.location   = APPLY_NONE;
		af.modifier   = 0;
		af.bitvector  = SHD_PROTECT_VOODOO;
		affect_to_char(wch,&af);
	    }

	    if ((wch->fighting != NULL) || (number_percent() < 25))
	    {
		send_to_char( "{RA sudden gust of wind throws you through the air!{x\n\r",wch);
		send_to_char( "{RYou slam face first into the nearest wall!{x\n\r",wch);
		act( "A sudden gust of wind picks up $n and throws $m into a wall!",wch, NULL,NULL,TO_ROOM,POS_RESTING);
		return;
	    }
	    wch->position = POS_STANDING;
	    was_in = wch->in_room;
	    for ( attempt = 0; attempt < 6; attempt++ )
	    {
		EXIT_DATA *pexit;
		int door;

		door = number_door( );
		if ( ( pexit = was_in->exit[door] ) == 0
		||   pexit->u1.to_room == NULL
		||   IS_SET(pexit->exit_info, EX_CLOSED)
		|| ( IS_NPC(wch)
		&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
		    continue;

		move_char( wch, door, FALSE, TRUE );
		if ( ( now_in = wch->in_room ) == was_in )
		    continue;

		wch->in_room = was_in;
		sprintf(buf, "A sudden gust of wind picks up $n and throws $m to the %s.", dir_name[door]);
		act( buf, wch, NULL, NULL, TO_ROOM,POS_RESTING);
		send_to_char( "{RA sudden gust of wind throws you through the air!{x\n\r",wch);
		wch->in_room = now_in;
		act( "$n sails into the room and slams face first into a wall!",wch,NULL,NULL,TO_ROOM,POS_RESTING);
		do_look( wch, "auto" );
		send_to_char( "{RYou slam face first into the nearest wall!{x\n\r",wch);
		return;
	    }
	    send_to_char( "{RA sudden gust of wind throws you through the air!{x\n\r",wch);
            send_to_char( "{RYou slam face first into the nearest wall!{x\n\r",wch);
            act( "A sudden gust of wind picks up $n and throws $m into a wall!",wch, NULL,NULL,TO_ROOM,POS_RESTING);
            return;
	}
    }
    send_to_char("Your victim doesn't seem to be in the realm.\n\r",ch);
    return;
}

void do_voodoo( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *doll;

    if (IS_NPC(ch))
        return;

    doll = get_eq_char(ch,WEAR_HOLD);
    if  (doll == NULL || (doll->pIndexData->vnum != OBJ_VNUM_VOODOO))
    {
        send_to_char("You are not holding a voodoo doll.\n\r",ch);
        return;
    }

    if ( get_skill( ch, skill_lookup( "voodoo" ) ) == 0 )
    {
	send_to_char( "You start playing with a doll, how childish.\n\r", ch );
	return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax: voodoo <action>\n\r",ch);
	send_to_char("Actions: pin trip throw\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"pin"))
    {
	do_vdpi(ch,doll->name);
	return;
    }

    if (!str_cmp(arg,"trip"))
    {
	do_vdtr(ch,doll->name);
	return;
    }

    if (!str_cmp(arg,"throw"))
    {
	do_vdth(ch,doll->name);
	return;
    }

    do_voodoo(ch,"");
}

void do_bash( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    if ( ( chance = get_skill( ch, gsn_bash ) ) == 0 )
    {
	send_to_char("Bashing? What's that?\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("Bash whom?\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,NULL,argument)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	spam_check(ch);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You try to bash your brains out, but fail.\n\r",ch);
	return;
    }

    if ( !check_stun_remember( ch ) )
	return;

    if ( !can_see( ch, victim ) )
    {
	send_to_char("You get a running start, and slam right into a wall.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   check_evade( ch, victim ) )
	return;

    WAIT_STATE(ch,skill_table[gsn_bash].beats);

    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    chance += (ch->size - victim->size) * 3;

    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;

    /* speed */
    if ( IS_AFFECTED(ch,AFF_HASTE) )
        chance += 10;
    if ( IS_AFFECTED(victim,AFF_HASTE) )
        chance -= 20;

    /* level */
    chance += (ch->level - victim->level) / 2;

    if (number_percent() < chance )
    {
	if (victim->position < POS_FIGHTING)
	{
	    act("{i$n{i lands on top of you, squashing you like a bug!{x",ch,NULL,victim,TO_VICT,POS_RESTING);
	    act("{hYou fall on $N{h, squashing $M like a bug!{x",ch,NULL,victim,TO_CHAR,POS_RESTING);
	    combat("{k$n{k falls on top of $N{k, squashing $M like a bug.{x",
		ch,NULL,victim,TO_NOTVICT,COMBAT_OTHER);
	} else {
	    act("{i$n{i sends you sprawling with a powerful bash!{x",ch,NULL,victim,TO_VICT,POS_RESTING);
	    act("{hYou slam into $N{h, and send $M flying!{x",ch,NULL,victim,TO_CHAR,POS_RESTING);
	    combat("{k$n{k sends $N {ksprawling with a powerful bash.{x",
		ch,NULL,victim,TO_NOTVICT,COMBAT_OTHER);
	}
	check_improve(ch,gsn_bash,TRUE,1);

	DAZE_STATE(victim, PULSE_VIOLENCE, DAZE_SKILL);
	DAZE_STATE(victim, PULSE_VIOLENCE, DAZE_SPELL);
	DAZE_STATE(victim, PULSE_VIOLENCE, DAZE_FLEE);

	damage(ch,victim,number_range( ch->level, 2*ch->level ),gsn_bash, DAM_BASH,TRUE,FALSE,NULL);
	victim->position = POS_RESTING;

	chance = ( 2 * get_skill(ch,gsn_stun) / 5);

	if (number_percent() < chance )
	{
	    chance = (get_skill(ch,gsn_stun)/4);
	    if (number_percent() < chance )
	    {
		victim->stunned = 2;
	    } else {
		victim->stunned = 1;
	    }
	    act("{iYou are stunned, and have trouble getting back up!{x",
		ch,NULL,victim,TO_VICT,POS_RESTING);
	    act("{h$N{h is stunned by your bash!{x",ch,NULL,victim,TO_CHAR,POS_RESTING);
	    combat("{k$N{k is having trouble getting back up.{x",
		ch,NULL,victim,TO_NOTVICT,COMBAT_OTHER);
	    check_improve(ch,gsn_stun,TRUE,1);
	}
    }
    else
    {
	damage(ch,victim,0,gsn_bash,DAM_BASH,FALSE,FALSE,NULL);
	act("{hYou fall flat on your face!{x",
	    ch,NULL,victim,TO_CHAR,POS_RESTING);
	combat("{k$n{k falls flat on $s face.{x",
	    ch,NULL,victim,TO_NOTVICT,COMBAT_OTHER);
	act("{iYou evade $n{i's bash, causing $m to fall flat on $s face.{x",
	    ch,NULL,victim,TO_VICT,POS_RESTING);
	check_improve(ch,gsn_bash,FALSE,1);
	ch->position = POS_RESTING;
    }
}

void do_dirt( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    if ( ( chance = get_skill( ch, gsn_dirt ) ) == 0 )
    {
	send_to_char( "{hYou get your feet dirty.{x\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "Whose eyes do you wish to kick dirt into?\n\r", ch );
	    return;
	}
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	spam_check( ch );
	return;
    }

    if ( IS_AFFECTED( victim, AFF_BLIND ) )
    {
	act( "{h$E's already been blinded.{x",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Very funny.\n\r", ch );
	return;
    }

    if ( !check_stun_remember( ch )
    ||   is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   check_evade( ch, victim ) )
	return;

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat( ch, STAT_DEX ) / 2;
    chance -= get_curr_stat( victim, STAT_DEX );

    /* speed  */
    if ( IS_AFFECTED( ch, AFF_HASTE ) )
	chance += 15;
    if ( IS_AFFECTED( victim, AFF_HASTE ) )
	chance -= 15;

    /* level */
    chance += URANGE( -15, ( ch->level - victim->level ) / 2, 15 );

    /* sloppy hack to prevent false zeroes */
    if ( chance % 5 == 0 )
	chance += 1;

    /* terrain */
    switch( ch->in_room->sector_type )
    {
	case SECT_INSIDE:	chance -= 20;	break;
	case SECT_CITY:		chance -= 10;	break;
	case SECT_FIELD:	chance +=  5;	break;
	case SECT_FOREST:			break;
	case SECT_HILLS:			break;
	case SECT_MOUNTAIN:	chance -= 10;	break;
	case SECT_WATER_SWIM:	chance  =  0;	break;
	case SECT_WATER_NOSWIM:	chance  =  0;	break;
	case SECT_AIR:		chance  =  0;  	break;
	case SECT_DESERT:	chance += 10;   break;
    }

    if ( chance == 0 )
    {
	send_to_char( "{hThere isn't any dirt to kick.{x\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_dirt].beats );

    /* now the attack */
    if ( number_percent( ) < chance )
    {
	AFFECT_DATA af;

	combat( "{k$N is blinded by the dirt in $S eyes!{x",
	    ch, NULL, victim, TO_ROOM, COMBAT_OTHER );
	act( "{k$N is blinded by dirt in $s eyes!{x",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	act( "{i$n kicks dirt in your eyes!{x",
	    ch, NULL, victim, TO_VICT, POS_RESTING );

	damage( ch, victim, number_range( 2, 5 ), gsn_dirt, DAM_OTHER, FALSE, FALSE, NULL );

	send_to_char( "{DYou can't see a thing!{x\n\r", victim );
	check_improve( ch, gsn_dirt, TRUE, 2 );

	af.where	= TO_AFFECTS;
	af.type 	= gsn_dirt;
	af.level 	= ch->level;
	af.dur_type	= DUR_TICKS;
	af.duration	= 0;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;

	affect_to_char( victim, &af );
    } else {
	damage( ch, victim, 0, gsn_dirt, DAM_OTHER, TRUE, FALSE, NULL );
	check_improve( ch, gsn_dirt, FALSE, 2 );
    }
}

void do_gouge( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    if ( ( chance = get_skill( ch, gsn_gouge ) ) == 0 )
    {
	send_to_char( "Gouge?  What's that?{x\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "Whose eyes do you wish to poke?\n\r", ch );
	    return;
	}
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	spam_check( ch );
	return;
    }

    if ( IS_AFFECTED( victim, AFF_BLIND ) )
    {
	act( "{h$E's already been blinded.{x",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Very funny.\n\r", ch );
	return;
    }

    if ( !check_stun_remember( ch )
    ||   is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   check_evade( ch, victim )
    ||   !cost_of_skill( ch, gsn_gouge ) )
	return;

    chance += get_curr_stat( ch, STAT_DEX );
    chance -= get_curr_stat( victim, STAT_DEX );

    if ( IS_AFFECTED( ch, AFF_HASTE ) )
	chance += 10;

    if ( IS_AFFECTED( victim, AFF_HASTE ) )
	chance -= 25;

    chance += ( ch->level - victim->level ) / 2;

    /* sloppy hack to prevent false zeroes */
    if ( chance % 5 == 0 )
	chance += 1;

    if ( number_percent( ) < chance )
    {
	AFFECT_DATA af;

	combat( "{k$N is blinded by a poke in the eyes!{x",
	    ch, NULL, victim, TO_ROOM, COMBAT_OTHER );
	act( "{k$N is blinded by a poke in the eyes!{x",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	act( "{i$n gouges at your eyes!{x",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	send_to_char( "{DYou see nothing but stars!{x\n\r", victim );

        damage( ch, victim, number_range( 2, 5 ), gsn_gouge, DAM_OTHER, FALSE, FALSE, NULL );

	check_improve( ch, gsn_gouge, TRUE, 2 );

	af.where	= TO_AFFECTS;
	af.type		= gsn_gouge;
	af.level	= ch->level;
	af.dur_type	= DUR_TICKS;
	af.duration	= 0;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;

	affect_to_char( victim, &af );
    } else {
	damage( ch, victim, 0, gsn_gouge, DAM_OTHER, TRUE, FALSE, NULL );
	check_improve( ch, gsn_gouge, FALSE, 2 );
    }
}

void do_tackle( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    if ( (chance = get_skill(ch,gsn_tackle)) == 0 )
    {
	send_to_char("You attempt to make a tackle and fall flat on your face!\n\r",ch);
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( (victim = ch->fighting) == NULL )
	{
	    send_to_char("Whom do you wish to tackle?\n\r",ch);
	    return;
 	}
    }

    else if ( (victim = get_char_room(ch,NULL,argument)) == NULL )
    {
	send_to_char("They aren't here.\n\r",ch);
	spam_check(ch);
	return;
    }

    if ( !check_stun_remember( ch ) )
	return;

    if ( !can_see(ch, victim) )
    {
	send_to_char("Your charge misses by a mile!\n\r",ch);
	return;
    }

    if ( victim == ch )
    {
	send_to_char("That would be a bit difficult.\n\r",ch);
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   check_evade( ch, victim ) )
	return;

    WAIT_STATE( ch, skill_table[gsn_tackle].beats );

    if ( !IS_AFFECTED(victim, AFF_FLYING)
    &&   !is_affected(victim, gsn_fly) )
    {
	act("$N's feet are already anchored securly to the ground!",
	    ch, NULL, victim, TO_CHAR, POS_DEAD );
	return;
    }

    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX);
    chance += get_curr_stat(ch,STAT_STR);
    chance -= get_curr_stat(victim,STAT_STR);

    if ( saves_spell( ch->level, ch, victim, DAM_BASH ) )
	chance /= 4;
    else
	chance /= 2;

    if ( number_percent() < chance )
    {
	act("{i$n{i takes you to the ground with a powerful tackle!{x",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	act("{hYou tackle $N{h and wrestle $M to the ground!{x",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	combat("{k$n tackles $N{k, wrestling $M to the ground.{x",
	    ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
	affect_strip( victim, gsn_fly );
	REMOVE_BIT( victim->affected_by, AFF_FLYING );
	damage( ch, victim, number_range( ch->level/2, ch->level ), gsn_tackle,
	    DAM_BASH, TRUE, FALSE, NULL );
	check_improve( ch, gsn_tackle, TRUE, 3 );
    } else {
	act("{i$n{i tries to tackle you, but fails.{x",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	act("{hYou fail to tackle $N.{x",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	combat("{k$n attempts to tackle $N{k, but fails.{x",
	    ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
	damage( ch, victim, number_range( ch->level/5, ch->level/2 ), gsn_tackle,
	    DAM_BASH, TRUE, FALSE, NULL );
	check_improve( ch, gsn_tackle, FALSE, 3 );
    }
}

void do_trip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( ( chance = get_skill(ch,gsn_trip) ) == 0 )
    {
	send_to_char("Tripping?  What's that?\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("Whom do you wish to trip?\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,NULL,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	spam_check(ch);
	return;
    }

    if ( !check_stun_remember( ch )
    ||   is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   check_evade( ch, victim ) )
	return;

    if (IS_AFFECTED(victim,AFF_FLYING))
    {
	act("{h$S feet aren't on the ground.{x",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("{h$N is already down.{x",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if (victim == ch)
    {
	send_to_char("{hYou fall flat on your face!{x\n\r",ch);
	WAIT_STATE(ch,2 * skill_table[gsn_trip].beats);
	combat("{k$n trips over $s own feet!{x",
	    ch,NULL,NULL,TO_ROOM,COMBAT_OTHER);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return;
    }

    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX);

    /* speed */
    if ( IS_AFFECTED(ch,AFF_HASTE) )
	chance += 10;
    if ( IS_AFFECTED(victim,AFF_HASTE) )
	chance -= 15;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* now the attack */
    if (number_percent() < chance)
    {
	act("{i$n{i trips you and you go down!{x",ch,NULL,victim,TO_VICT,POS_RESTING);
	act("{hYou trip $N{h and $E goes down!{x",ch,NULL,victim,TO_CHAR,POS_RESTING);
	combat("{k$n trips $N{k, sending $M to the ground.{x",
	    ch,NULL,victim,TO_NOTVICT,COMBAT_OTHER);
	check_improve(ch,gsn_trip,TRUE,1);

	DAZE_STATE(victim,2 * PULSE_VIOLENCE, DAZE_SKILL);
	DAZE_STATE(victim,2 * PULSE_VIOLENCE, DAZE_SPELL);
	DAZE_STATE(victim,2 * PULSE_VIOLENCE, DAZE_FLEE);
        WAIT_STATE(ch,skill_table[gsn_trip].beats);
	damage(ch,victim,number_range(50,200) * (ch->level / 45),gsn_trip,
	    DAM_BASH,TRUE,FALSE,NULL);
	victim->position = POS_RESTING;
    }
    else
    {
	damage(ch,victim,0,gsn_trip,DAM_BASH,TRUE,FALSE,NULL);
	WAIT_STATE(ch,skill_table[gsn_trip].beats*2/3);
	check_improve(ch,gsn_trip,FALSE,1);
    }
}

void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	spam_check( ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "{hYou hit yourself.  {z{COuch!{x\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED, TRUE );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR,POS_RESTING);
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE ) )
	return;

    if (!IS_NPC(victim))
    {
	char buf[100];

	sprintf(buf, "Help! I am being attacked by %s!",PERS(ch,victim));
	do_yell( victim, buf );
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    multi_hit( ch, victim, TYPE_UNDEFINED, TRUE );
    return;
}

void do_murder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Murder whom?\n\r", ch );
	return;
    }

    if (IS_NPC(ch))
	return;

    if (IS_AFFECTED(ch,AFF_CHARM))
	return;

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	spam_check( ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR,POS_RESTING);
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE ) )
	return;

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );

    if (!IS_NPC(victim))
    {
	sprintf(buf, "Help! I am being attacked by %s!",PERS(ch,victim));
	do_yell( victim, buf );
    }

    multi_hit( ch, victim, TYPE_UNDEFINED, TRUE );
    return;
}

bool run_ambush( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;
    int chance;

    if ( ( chance = get_skill( ch, gsn_ambush ) ) <= 0 )
    {
	send_to_char( "You fall flat on your face as you attempt to sneak up on them.\n\r", ch );
	return FALSE;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
    {
	send_to_char( "{hYou need to wield a primary weapon to ambush.{x\n\r", ch );
	return FALSE;
    }

    if ( IS_AWAKE( victim ) && can_see( victim, ch ) )
    {
	act( "$N saw your ambush from a mile a way!",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return FALSE;
    }

    WAIT_STATE( ch, skill_table[gsn_ambush].beats );

    if ( check_evade( ch, victim ) )
    {
	multi_hit( victim, ch, TYPE_UNDEFINED, TRUE );
	return TRUE;
    }

    if ( number_percent( ) < chance
    ||   ( chance >= 2 && !IS_AWAKE( victim ) ) )
    {
	check_improve( ch, gsn_ambush, TRUE, 2 );
	multi_hit( ch, victim, gsn_ambush, TRUE );
    } else {
	check_improve( ch, gsn_ambush, FALSE, 2 );
	damage( ch, victim, 0, gsn_ambush, DAM_OTHER, TRUE, FALSE, NULL );
    }

    return TRUE;
}

void do_ambush( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Ambush whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	spam_check( ch );

	if ( is_affected( ch, gsn_camouflage )
	&&   number_percent( ) > ( get_skill( ch, gsn_camouflage ) - 75 ) )
	{
	    send_to_char( "Your wild swing has exposed you!\n\r", ch );
	    affect_strip( ch, gsn_camouflage );
	}

        return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE ) )
	return;

    run_ambush( ch, victim );
    return;
}

bool run_backstab( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( ( chance = get_skill( ch, gsn_backstab ) ) <= 0 )
    {
	send_to_char( "You fall flat on your face as you attempt to sneak up on them.\n\r", ch );
	return FALSE;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL )
    {
        send_to_char( "{hYou need to wield a primary weapon to backstab.{x\n\r", ch );
        return FALSE;
    }

    if ( check_evade( ch, victim ) )
    {
	multi_hit( victim, ch, TYPE_UNDEFINED, TRUE );
	return TRUE;
    }

    if ( victim->position > POS_SLEEPING
    &&   victim->hit < victim->max_hit / 3
    &&   can_see( victim, ch ) )
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );

	if ( victim->fighting == NULL || victim->fighting != ch )
	    multi_hit( victim, ch, TYPE_UNDEFINED, TRUE );
	return TRUE;
    }

    if ( ch->pcdata == NULL )
	do_say( ch, "Ahh another victim..." );

    WAIT_STATE( ch, skill_table[gsn_backstab].beats );

    if ( number_percent( ) < chance
    ||   ( chance >= 2 && !IS_AWAKE( victim ) ) )
    {
	check_improve( ch, gsn_backstab, TRUE, 2 );
	multi_hit( ch, victim, gsn_backstab, TRUE );
    } else {
	check_improve( ch, gsn_backstab, FALSE, 2 );
	damage( ch, victim, 0, gsn_backstab, DAM_OTHER, TRUE, FALSE, NULL );
    }

    return TRUE;
}

void do_backstab( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Backstab whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	spam_check( ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE ) )
	return;

    run_backstab( ch, victim );
    return;
}

void do_charge( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    if ( ( chance = get_skill( ch, gsn_charge ) ) <= 0 )
    {
	send_to_char( "You fall flat on your face as you attempt to sneak up on them.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Charge whom?\n\r", ch );
	return;
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	spam_check( ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL )
    {
        send_to_char( "{hYou need to wield a primary weapon to charge.{x\n\r", ch );
        return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE ) )
	return;

    if ( check_evade( ch, victim ) )
    {
	multi_hit( victim, ch, TYPE_UNDEFINED, TRUE );
	return;
    }

    if ( victim->position > POS_SLEEPING
    &&   victim->hit < victim->max_hit / 3
    &&   can_see( victim, ch ) )
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	if ( victim->fighting == NULL || victim->fighting != ch )
	    multi_hit( victim, ch, TYPE_UNDEFINED, TRUE );
	return;
    }

    if ( ch->pcdata == NULL )
	do_say( ch, "Ahh another victim..." );

    WAIT_STATE( ch, skill_table[gsn_charge].beats );

    if ( number_percent( ) < chance
    ||   ( chance >= 2 && !IS_AWAKE( victim ) ) )
    {
	check_improve( ch, gsn_charge, TRUE, 2 );
	multi_hit( ch, victim, gsn_charge, TRUE );
    } else {
	check_improve( ch, gsn_charge, FALSE, 2 );
	damage( ch, victim, 0, gsn_charge, DAM_OTHER, TRUE, FALSE, NULL );
    }
}

void do_circle( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    if ( ( chance = get_skill( ch, gsn_circle ) ) <= 0 )
    {
	send_to_char( "Circle? What's that?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "You aren't fighting anyone.\n\r", ch );
	    return;
	}
    } else {
	if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
	{
	    send_to_char( "You can't circle someone who isn't here!\n\r", ch );
	    return;
	}

	if ( victim->fighting != ch )
	{
	    act( "$N isn't fighting you!",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	    return;
	}
    }

    if ( victim == ch )
    {
	send_to_char( "Your arms can't reach around that far!\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL && !IS_NPC( ch ) )
    {
	send_to_char( "You need to wield a primary weapon to circle.\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit / 10 )
    {
	act( "$N is hurt and suspicious ... you can't sneak around.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return;
    }

    if ( !can_see( ch, victim ) )
    {
	send_to_char("You stumble blindly into a wall.\n\r",ch);
	return;
    }

    if ( !check_stun_remember( ch )
    ||   !cost_of_skill( ch, gsn_circle )
    ||   check_evade( ch, victim ) )
	return;

    if ( number_percent( ) < chance )
    {
	check_improve( ch, gsn_circle, TRUE, 2 );
	act( "{i$n circles around behind you.{x",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	act( "{hYou circle around $N.{x",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	combat( "{k$n circles around behind $N.{x",
	    ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
	multi_hit( ch, victim, gsn_circle, TRUE );
    } else {
	check_improve( ch, gsn_circle, FALSE, 2 );
	act( "{i$n tries to circle around you.{x",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	act( "{h$N circles with you.{x",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	combat( "{k$n tries to circle around $N.{x",
	    ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
        damage( ch, victim, 0, gsn_circle, DAM_OTHER, TRUE, FALSE, NULL );
    }
}

void do_gash( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    if ( ( chance = get_skill( ch, gsn_gash ) ) <= 0 )
    {
	send_to_char( "Gash? What's that?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "You aren't fighting anyone.\n\r", ch );
	    return;
	}
    } else {
	if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
	{
	    send_to_char( "You can't gash someone who isn't here!\n\r", ch );
	    return;
	}

	if ( victim->fighting != ch )
	{
	    act( "$N isn't fighting you!",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	    return;
	}
    }

    if ( victim == ch )
    {
	send_to_char( "Your arms can't reach around that far!\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL && !IS_NPC( ch ) )
    {
	send_to_char( "You need to wield a primary weapon to gash.\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit / 10
    &&   can_see( victim, ch ) )
    {
	act( "$N is hurt and suspicious ... you can't sneak around.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	return;
    }

    if ( !check_stun_remember( ch )
    ||   check_evade( ch, victim )
    ||   !cost_of_skill( ch, gsn_gash ) )
	return;

    if ( number_percent( ) < chance )
    {
	check_improve( ch, gsn_gash, TRUE, 2 );
	act( "{i$n {itears deep into your back.{x",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	act( "{hYou tear deep gashes in $N{h's back.{x",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	combat( "{k$n {k tears deep gashes in $N{k's back.{x",
	    ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
	multi_hit( ch, victim, gsn_gash, TRUE );
    } else {
	check_improve( ch, gsn_gash, FALSE, 2 );
	act( "{i$n{i tries to gash your back.{x",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	act( "{h$N {h narrowly avoids your gash.{x",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	combat( "{k$N{k narrowly avoids $n{k's gashing.{x",
	    ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
        damage( ch, victim, 0, gsn_gash, DAM_OTHER, TRUE, FALSE, NULL );
    }
}

void do_cyclone( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch, *vch_next;
    OBJ_DATA *obj;
    bool found = FALSE;
    int chance, move;

    if ( ( chance = get_skill( ch, gsn_cyclone ) ) <= 0 )
    {
	send_to_char( "You feel dizzy, and a bit confused.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
    {
	send_to_char( "You need to wield a primary weapon first.\n\r", ch );
	return;
    }

    if ( !check_stun_remember( ch ) )
	return;

    move = number_range( 1, 10 );

    if ( ch->move < move )
    {
	send_to_char( "Your a little too tired right now.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
	chance = chance * 2 / 3;

    act( "$n holds $p firmly, and starts {@s{2p{@i{2n{@n{2i{@n{2g{x around...",
	ch, obj, NULL, TO_ROOM, POS_RESTING );

    act( "You hold $p firmly, and start {@s{2p{@i{2n{@n{2i{@n{2g{x around...",
	ch, obj, NULL, TO_CHAR, POS_RESTING );

    WAIT_STATE( ch, skill_table[gsn_cyclone].beats );

    if ( number_percent( ) >= chance )
    {
	send_to_char( "You get a little dizzy and stumble about.\n\r", ch );
	check_improve( ch, gsn_cyclone, FALSE, 6 );
	return;
    }

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	if ( is_safe( ch, vch )
	||   IS_IMMORTAL( vch )
	||   is_same_group( ch, vch )
	||   check_evade( ch, vch ) )
	    continue;

	if ( IS_NPC( vch )
	&&   vch->fighting != NULL
	&&   !IS_NPC(vch->fighting)
	&&   vch->fighting != ch
	&&   !is_same_group( ch, vch->fighting )
	&&   !can_pk( ch, vch->fighting ) )
	    continue;

	found = TRUE;
	act( "$n turns towards YOU!", ch, NULL, vch, TO_VICT, POS_RESTING );
	multi_hit( ch, vch, gsn_cyclone, TRUE );
    }

    ch->move -= move;

    if ( !found )
    {
	act( "$n looks dizzy, and a tiny bit embarassed.",
	    ch, NULL, NULL, TO_ROOM, POS_RESTING );
	act( "You feel dizzy, and a tiny bit embarassed.",
	    ch, NULL, NULL, TO_CHAR, POS_RESTING );
	DAZE_STATE( ch, 4 * PULSE_VIOLENCE, DAZE_SKILL );
	DAZE_STATE( ch, 4 * PULSE_VIOLENCE, DAZE_SPELL );
	DAZE_STATE( ch, 4 * PULSE_VIOLENCE, DAZE_FLEE );
    }

    else
	check_improve( ch, gsn_cyclone, TRUE, 5 );
}

void do_feed( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    sh_int chance;

    if ( ( chance = get_skill( ch, gsn_feed ) ) == 0 )
    {
	send_to_char( "Feed? What's that?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "Whose neck do you wish to bite?\n\r", ch );
	    return;
	}
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	spam_check( ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You can't do that to yourself!\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   check_evade( ch, victim ) )
	return;

    if ( !check_stun_remember( ch ) )
	return;

    if ( victim->hit < victim->max_hit / 10 && can_see( victim, ch ) )
    {
	act( "$N is hurt and suspicious ... you can't get close enough.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	if ( victim->fighting == NULL || victim->fighting != ch )
	    multi_hit( victim, ch, TYPE_UNDEFINED, TRUE );
	return;
    }

    if ( !can_see( ch, victim ) )
    {
	send_to_char( "You can't feed on someone you can't see.\n\r", ch );
	return;
    }

    if ( !check_kill_steal( ch, victim, TRUE ) )
	return;

    WAIT_STATE( ch, skill_table[gsn_feed].beats );

    if ( number_percent( ) < chance
    || ( get_skill(ch,gsn_feed) >= 2 && !IS_AWAKE(victim) ) )
    {
	int dam;

        check_improve(ch,gsn_feed,TRUE,1);
	act( "{i$n bites you.{x", ch, NULL, victim, TO_VICT,POS_RESTING);
	act( "{hYou bite $N.{x", ch, NULL, victim, TO_CHAR,POS_RESTING);
	combat("{k$n bites $N.{x",
	    ch,NULL,victim,TO_NOTVICT,COMBAT_OTHER);

	dam = dice( ch->level, ch->level / 2 );

	if ( is_affected( victim, gsn_bloodbath ) )
	    dam = dam * 3 / 2;

        damage( ch, victim, dam, gsn_feed, DAM_NEGATIVE, TRUE, FALSE, NULL );

    } else {
        check_improve(ch,gsn_feed,FALSE,1);
	act( "{i$n tries to bite you, but hits only air.{x", ch, NULL, victim, TO_VICT,POS_RESTING);
	act( "{hYou chomp a mouthfull of air.{x", ch, NULL, victim, TO_CHAR,POS_RESTING);
	combat("{k$n tries to bite $N.{x",
	    ch,NULL,victim,TO_NOTVICT,COMBAT_OTHER);
        damage( ch, victim, 0, gsn_feed,DAM_NEGATIVE,TRUE,FALSE,NULL);
    }

    return;
}

void do_retreat( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    int door;

    if ( ( victim = ch->fighting ) == NULL )
    {
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( !can_see( ch, victim ) )
    {
        send_to_char( "You stumble around while attempting to retreat blind.\n\r", ch );
        return;
    }

    if ( !check_stun_remember( ch ) )
	return;

    if ( arena_flag( ch, ARENA_NO_FLEE ) )
	return;

	 if ( !str_prefix( argument, "north" ) ) door = 0;
    else if ( !str_prefix( argument, "east"  ) ) door = 1;
    else if ( !str_prefix( argument, "south" ) ) door = 2;
    else if ( !str_prefix( argument, "west"  ) ) door = 3;
    else if ( !str_prefix( argument, "up"    ) ) door = 4;
    else if ( !str_prefix( argument, "down"  ) ) door = 5;
    else					 door = number_range( 0, 5 );

    WAIT_STATE( ch, skill_table[gsn_retreat].beats );

    if ( number_percent() > (4*get_skill(ch,gsn_retreat)/5)
    ||   (pexit = ch->in_room->exit[door]) == 0
    ||   pexit->u1.to_room == NULL
    ||   (IS_SET(pexit->exit_info, EX_CLOSED)
    &&    (IS_SET(pexit->exit_info,EX_NOPASS)
    ||     !IS_AFFECTED(ch, AFF_PASS_DOOR)) )
    ||   number_range(0,ch->daze[DAZE_FLEE]) != 0
    ||   (IS_NPC(ch)
    &&    IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)) )
    {
	send_to_char("Your retreat failed!\n\r",ch);
	check_improve(ch,gsn_retreat,FALSE,4);
	return;
    }

    if ( ch->fighting != NULL && !IS_NPC(ch->fighting) )
	check_pktimer( ch, ch->fighting, FALSE );

    if ( !IS_NPC(ch) )
    {
	send_to_char( "{BYou {Yretreat{B from combat!{x\n\r", ch );

	if ( class_table[ch->class].sub_class == class_lookup( "thief" )
        &&   (number_percent() < 3 * (ch->level/2)) )
        {
	    if ( IS_NPC(victim) || !ch->pcdata || !ch->pcdata->attacker )
		send_to_char( "You {Ysnuck away{x safely.\n\r", ch);
	} else {
	    send_to_char( "You lost 10 exp.\n\r", ch);
	    gain_exp( ch, -10 );
        }
    }

    if ( IS_NPC(ch) )
	mob_remember(ch,victim,MEM_AFRAID);

    for (victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
    {
	if (victim->fighting != NULL && victim->fighting == ch)
	    victim->wait = 0;
    }

    stop_fighting( ch, TRUE );
    act( "$n has {Yretreated{x!", ch, NULL, NULL, TO_ROOM, POS_RESTING);
    check_improve(ch,gsn_retreat,TRUE,4);

    move_char( ch, door, FALSE, FALSE );

    return;
}

void do_raze( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    sh_int chance, dam, dam_type;

    argument = one_argument( argument, arg );

    if ( ( chance = get_skill( ch, gsn_raze ) ) <= 0 )
    {
	send_to_char( "You can't seem to control the elements.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "Whom do you wish to raze elements upon?\n\r", ch );
	    return;
	}
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	spam_check( ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   check_evade( ch, victim ) )
	return;

    if ( arg[0] == '\0'
    ||   ( dam_type = dam_type_lookup( arg ) ) == -1 )
    {
	act( "What kind of damage do you wish to raze upon $N?",
	    ch, NULL, victim, TO_CHAR, POS_DEAD );
	return;
    }

    if ( !cost_of_skill( ch, gsn_raze ) )
	return;

    if ( number_percent( ) > chance )
    {
	send_to_char( "Your connection with the elements fizzles for a moment.\n\r", ch );
	check_improve( ch, gsn_raze, FALSE, 5 );
	return;
    }

    check_improve( ch, gsn_raze, TRUE, 5 );

    dam = dice( ch->level, ch->level / 2 );

    if ( saves_spell( ch->level, ch, victim, dam_type ) )
	dam = 2 * dam / 3;

    sprintf( buf, "%s raze", damage_mod_table[dam_type].name );

    damage( ch, victim, dam, gsn_raze, dam_type, TRUE, FALSE, buf );
}

void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about {Yfleeing{x instead?\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }

    if (is_safe(ch,fch))
    {
	send_to_char("Not on that target!\n\r",ch);
	return;
    }

    if ( !IS_NPC(victim)
    &&   !is_same_group(ch,victim)
    &&   !can_pk(ch,victim) )
    {
	if ( ch->clan != victim->clan )
	{
	    send_to_char("Kill stealing outside of pk range is not permitted.\n\r",ch);
	    return;
	}
    }

    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( number_percent( ) > get_skill(ch,gsn_rescue))
    {
	send_to_char( "You fail the rescue.\n\r", ch );
	check_improve(ch,gsn_rescue,FALSE,1);
	return;
    }

    act( "{yYou rescue $N!{x",  ch, NULL, victim, TO_CHAR,POS_RESTING);
    act( "{y$n rescues you!{x", ch, NULL, victim, TO_VICT,POS_RESTING);
    combat("{y$n rescues $N!{x",ch,NULL,victim,TO_NOTVICT,COMBAT_OTHER);
    check_improve(ch,gsn_rescue,TRUE,1);

    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );

    if (ch->fighting == NULL )
	set_fighting( ch, fch );

    set_fighting( fch, ch );

    return;
}

void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int dam;

    if ( get_skill(ch,gsn_kick) <= 0 )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if (argument[0] == '\0')
    {
	if ( (victim = ch->fighting) == NULL )
	{
	    send_to_char("Kick who?\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,NULL,argument)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	spam_check( ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char("You can't do that to yourself!\n\r",ch);
	return;
    }

    if ( is_safe( ch, victim )
    ||   check_evade( ch, victim ) )
	return;

    if ( !check_stun_remember( ch ) )
	return;

    if ( !check_kill_steal( ch, victim, TRUE ) )
	return;

    dam = number_range( 1, 3*ch->level/4 );

    WAIT_STATE( ch, skill_table[gsn_kick].beats );
    if ( get_skill(ch,gsn_kick) > number_percent())
    {
	damage(ch,victim,number_range( dam, (ch->level*1.5) ), gsn_kick,DAM_BASH,TRUE,FALSE,NULL);
 	if (IS_AFFECTED(ch,AFF_HASTE))
	    damage(ch,victim,number_range(dam,ch->level),gsn_kick,DAM_BASH,TRUE,FALSE,NULL);
        if ( get_skill(ch,gsn_second_attack) > number_percent( ) )
            damage(ch,victim,number_range( dam * 4, (ch->level * 4 ) ), gsn_kick,DAM_BASH,TRUE,FALSE,NULL);
        if ( get_skill(ch,gsn_third_attack) > number_range(60, 150 ) )
            damage(ch,victim,number_range( dam * 3, (ch->level * 3 ) ), gsn_kick,DAM_BASH,TRUE,FALSE,NULL);
        if ( get_skill(ch,gsn_fourth_attack) > number_range(50, 140 ) )
            damage(ch,victim,number_range( dam * 2, (ch->level * 2 ) ), gsn_kick,DAM_BASH,TRUE,FALSE,NULL);
        if ( get_skill(ch,gsn_fifth_attack) > number_range( 45, 140 ) )
            damage(ch,victim,number_range( dam * 1.5, (ch->level * 1.5) ), gsn_kick,DAM_BASH,TRUE,FALSE,NULL);
	if ( get_skill(ch,gsn_sixth_attack) > number_range( 50, 160 ) )
            damage(ch,victim,number_range( dam * 1.5, (ch->level * 1.5) ), gsn_kick,DAM_BASH,TRUE,FALSE,NULL);
	check_improve(ch,gsn_kick,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_kick,DAM_BASH,TRUE,FALSE,NULL);
	check_improve(ch,gsn_kick,FALSE,1);
    }
    return;
}

void do_roundhouse( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int dam;
	int skill;

    if ( (skill=get_skill(ch,gsn_roundhouse)) <= 0 )
    {
		send_to_char("You better leave the martial arts to fighters.\n\r", ch );
		return;
    }

    if (argument[0] == '\0')
    {
		if ( (victim = ch->fighting) == NULL )
		{
			send_to_char("Roundhouse who?\n\r",ch);
			return;
		}
    }
    else if ((victim = get_char_room(ch,NULL,argument)) == NULL)
    {
		send_to_char("They aren't here.\n\r",ch);
		spam_check( ch );
		return;
    }

    if ( victim == ch )
    {
		send_to_char("You can't do that to yourself!\n\r",ch);
		return;
    }

    if ( is_safe( ch, victim )
    ||   check_evade( ch, victim ) )
	return;

    if ( !check_stun_remember( ch ) )
	return;

    if ( !check_kill_steal( ch, victim, TRUE ) )
	return;

    dam = number_range( 1, 3*ch->level/4 );

    WAIT_STATE( ch, skill_table[gsn_roundhouse].beats );
    if ( skill > number_percent())
    {
	damage(ch,victim,number_range( dam, (ch->level*1.5) ), gsn_roundhouse,DAM_BASH,TRUE,FALSE,NULL);
 	if (IS_AFFECTED(ch,AFF_HASTE))
	    damage(ch,victim,number_range(dam,ch->level),gsn_roundhouse,DAM_BASH,TRUE,FALSE,NULL);
        if ( get_skill(ch,gsn_second_attack) > number_percent( ) )
            damage(ch,victim,number_range( dam * 4, (ch->level * 4 ) ), gsn_roundhouse,DAM_BASH,TRUE,FALSE,NULL);
        if ( get_skill(ch,gsn_third_attack) > number_range(60, 150 ) )
            damage(ch,victim,number_range( dam * 3, (ch->level * 3 ) ), gsn_roundhouse,DAM_BASH,TRUE,FALSE,NULL);
        if ( get_skill(ch,gsn_fourth_attack) > number_range(50, 140 ) )
            damage(ch,victim,number_range( dam * 2, (ch->level * 2 ) ), gsn_roundhouse,DAM_BASH,TRUE,FALSE,NULL);
        if ( get_skill(ch,gsn_fifth_attack) > number_range( 45, 140 ) )
            damage(ch,victim,number_range( dam * 1.5, (ch->level * 1.5) ), gsn_roundhouse,DAM_BASH,TRUE,FALSE,NULL);
	if ( get_skill(ch,gsn_sixth_attack) > number_range( 50, 160 ) )
            damage(ch,victim,number_range( dam * 1.5, (ch->level * 1.5) ), gsn_roundhouse,DAM_BASH,TRUE,FALSE,NULL);
	check_improve(ch,gsn_roundhouse,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_roundhouse,DAM_BASH,TRUE,FALSE,NULL);
	check_improve(ch,gsn_roundhouse,FALSE,1);
    }
    return;
}

void do_rub( CHAR_DATA *ch, char *argument )
{
    sh_int chance;

    if ( ( chance = get_skill( ch, gsn_rub ) ) == 0 )
    {
	send_to_char( "You rub your eyes, but no luck.\n\r", ch );
	return;
    }

    if ( !IS_AFFECTED( ch, AFF_BLIND ) )
    {
	send_to_char( "You are not blind!\n\r", ch );
	return;
    }

    if ( !cost_of_skill( ch, gsn_rub ) )
	return;

    if ( is_affected( ch, gsn_dirt ) )
    {
        if ( ( 3 * chance / 4 ) > number_percent( ) )
        {
	    send_to_char( "{RYou rub the dirt out of your eyes.{x\n\r", ch );
	    act( "{R$n rubs the dirt from $s eyes.{x", ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    affect_strip( ch, gsn_dirt );
	    check_improve( ch, gsn_rub, TRUE, 5 );
        } else {
            send_to_char( "{RYou rub but no luck.{x\n\r", ch );
	    check_improve( ch, gsn_rub, FALSE, 5 );
	}
    }

    else if ( is_affected( ch, gsn_fire_breath ) )
    {
        if ( ( 3 * chance / 5 ) > number_percent( ) )
        {
	    send_to_char( "{RYou rub the smoke from your eyes.{x\n\r", ch );
	    act( "{R$n rubs the smoke from $s eyes.{x", ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    affect_strip( ch, gsn_fire_breath );
	    check_improve( ch, gsn_rub, TRUE, 5 );
        } else {
            send_to_char( "{RYou rub but no luck.{x\n\r", ch );
	    check_improve( ch, gsn_rub, FALSE, 5 );
	}
    }

    else
        send_to_char( "That won't help in this situation.\n\r", ch );
}

void do_strangle( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_strangle)) == 0)
    {
	send_to_char("You attempt a strangle but miss the throat.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	send_to_char("Strangle whom?\n\r",ch);
	return;
    }

    if ((victim = get_char_room(ch,NULL,arg)) == NULL)
    {
	send_to_char("You attempt the strangle, but hit thin air.\n\r",ch);
	spam_check(ch);
	return;
    }

    if (!IS_AWAKE(victim))
    {
	act("$N is already sleeping.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You turn blue and lose your grip.\n\r",ch);
	return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->dtimer > 0 )
    {
	send_to_char("After the shock of death, you can not muster the strength for it.\n\r",ch);
	return;
    }

    if ( is_safe( ch, victim )
    ||   check_evade( ch, victim ) )
	return;

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR,POS_RESTING);
	return;
    }

    /* level */
/*    chance += (ch->level - victim->level) * 2; */

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
        chance += 1;

    affect_strip(ch,gsn_obfuscate);
    affect_strip(ch,gsn_camouflage);
    affect_strip(ch,gsn_forest_meld);

    if ( number_range(1,125) > chance )
    {
	act("$N chokes and gags.",ch,NULL,victim,TO_CHAR,POS_RESTING);
	send_to_char("You choke and gag.\n\r",victim);
	damage(ch,victim,(number_range(1,200)*ch->level/100),gsn_strangle,DAM_OTHER,TRUE,FALSE,NULL);
	check_improve(ch,gsn_strangle,FALSE,2);
	WAIT_STATE(ch,skill_table[gsn_strangle].beats);
    } else {
	AFFECT_DATA af;
	act("$n turns blue and passes out!",victim,NULL,NULL,TO_ROOM,POS_RESTING);
	act("$n grabs your throat and you lose all awareness!",ch,NULL,victim,TO_VICT,POS_RESTING);
	send_to_char("You slip into unconsciousness.\n\r",victim);
	act("$n slips into unconsciousness.",victim,NULL,NULL,TO_ROOM,POS_RESTING);
	check_improve(ch,gsn_strangle,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_strangle].beats);

	af.where	= TO_AFFECTS;
	af.type 	= gsn_strangle;
	af.level 	= ch->level;
	af.dur_type	= DUR_TICKS;
	af.duration	= number_range(0,2);
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector 	= AFF_SLEEP;

	affect_join(victim,&af);
        victim->position = POS_SLEEPING;

	if (!IS_NPC(victim)
	&&  !IS_SET(ch->in_room->room_flags,ROOM_ARENA)
	&&  !IS_SET(ch->in_room->room_flags,ROOM_WAR) )
	{
	    sprintf(buf,"{w({RPK{w) {V$N strangles %s.", victim->name);
	    wiznet(buf,ch,NULL,WIZ_PKILLS,0,0);
	    check_pktimer( ch, victim, TRUE );
	}
    }
}

void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *wield;
    OBJ_DATA *obj;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;
    int bitname = 0;

    hth = 0;

    wield = get_eq_char( ch, WEAR_WIELD );

    if ((chance = get_skill(ch,gsn_disarm)) == 0)
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL
    &&   (hth = get_skill(ch,gsn_hand_to_hand)) == 0 )
    {
	send_to_char( "You must wield a weapon to disarm.\n\r", ch );
	return;
    }

    if (argument[0] == '\0')
    {
	if ( (victim = ch->fighting) == NULL )
	{
	    send_to_char("Disarm whom?\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,NULL,argument)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	spam_check(ch);
	return;
    }

    if ( victim == ch )
    {
	send_to_char("You can't do that to yourself!\n\r",ch);
	return;
    }

    if ( is_safe( ch, victim )
    ||   check_evade( ch, victim ) )
	return;

    if (!str_prefix( argument, "primary"))
	bitname = 1;
    else if (!str_prefix( argument, "secondary"))
	bitname = 2;
    else
	bitname = number_range(1,2);

    if (bitname == 1 || (get_eq_char(victim,WEAR_SECONDARY) == NULL) )
	bitname = WEAR_WIELD;
    else
	bitname = WEAR_SECONDARY;

    if ( !check_stun_remember( ch ) )
	return;

    if ( ( obj = get_eq_char( victim, bitname ) ) == NULL )
    {
	send_to_char( "{hYour opponent is not wielding a weapon.{x\n\r", ch );
	return;
    }

    if ( !check_kill_steal( ch, victim, TRUE ) )
	return;

    if (ch->fighting == NULL)
	multi_hit( ch, victim, TYPE_UNDEFINED, TRUE );

    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch,FALSE));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim,FALSE));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim,FALSE));

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
	chance = chance * hth/150;
    else
	chance = chance * ch_weapon/100;

    chance += (ch_vict_weapon/2 - vict_weapon) / 2;
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 3 * get_curr_stat(victim,STAT_STR) / 2;
    chance += (ch->level - victim->level) * 2;
    chance /= 2;

    if (number_percent() < chance)
    {
	if (((chance = get_skill(victim,gsn_grip)) == 0) )
	{
	    WAIT_STATE( ch, skill_table[gsn_disarm].beats );
	    disarm( ch, victim, bitname );
	    check_improve(ch,gsn_disarm,TRUE,1);
	    return;
	}
	if (number_percent() > (chance/2))
	{
	    WAIT_STATE( ch, skill_table[gsn_disarm].beats );
	    disarm( ch, victim, bitname );
	    check_improve(ch,gsn_disarm,TRUE,1);
	    check_improve(victim,gsn_grip,FALSE,1);
	    return;
	}
	check_improve(victim,gsn_grip,TRUE,1);
    }
    WAIT_STATE(ch,skill_table[gsn_disarm].beats);
    act("{hYou fail to disarm $N{h.{x",ch,NULL,victim,TO_CHAR,POS_RESTING);
    act("{i$n{i tries to disarm you, but fails.{x",ch,NULL,victim,TO_VICT,POS_RESTING);
    combat("{k$n {ktries to disarm $N{k, but fails.{x",
	ch,NULL,victim,TO_NOTVICT,COMBAT_OTHER);
    check_improve(ch,gsn_disarm,FALSE,1);

    return;
}

void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to {RSLAY{x, spell it out.\n\r", ch );
    return;
}

void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) || ( get_trust(ch) >= CREATOR
			&&   can_over_ride(ch,victim,FALSE) ) )
    {
	act( "{hYou slay $M in cold blood!{x",  ch, NULL, victim, TO_CHAR,POS_RESTING);
	act( "{i$n slays you in cold blood!{x", ch, NULL, victim, TO_VICT,POS_RESTING);
	act( "{k$n slays $N in cold blood!{x",  ch, NULL, victim, TO_NOTVICT,POS_RESTING);
	raw_kill( victim, ch );
    }
    else
    {
	act( "{i$N wields a sword called '{z{RGodSlayer{i'!{x",  ch, NULL, victim, TO_CHAR,POS_RESTING);
	act( "{hYou wield a sword called '{z{RGodSlayer{h'!{x", ch, NULL, victim, TO_VICT,POS_RESTING);
	act( "{k$N wields a sword called '{z{RGodSlayer{k'!{x",  ch, NULL, victim, TO_NOTVICT,POS_RESTING);
        act("{i$N's slice takes off your left arm!{x",ch,NULL,victim,TO_CHAR,POS_RESTING);
        act("{hYour slice takes off $n's left arm!{x",ch,NULL,victim,TO_VICT,POS_RESTING);
        act("{k$N's slice takes off $n's left arm!{x",ch,NULL,victim,TO_NOTVICT,POS_RESTING);
        act("{i$N's slice takes off your right arm!{x",ch,NULL,victim,TO_CHAR,POS_RESTING);
        act("{hYour slice takes off $n's right arm!{x",ch,NULL,victim,TO_VICT,POS_RESTING);
        act("{k$N's slice takes off $n's right arm!{x",ch,NULL,victim,TO_NOTVICT,POS_RESTING);
        act("{i$N's slice cuts off both of your legs!{x",ch,NULL,victim,TO_CHAR,POS_RESTING);
        act("{hYour slice cuts off both of $n's legs!{x",ch,NULL,victim,TO_VICT,POS_RESTING);
        act("{k$N's slice cuts off both of $n's legs!{x",ch,NULL,victim,TO_NOTVICT,POS_RESTING);
        act("{i$N's slice beheads you!{x",ch,NULL,victim,TO_CHAR,POS_RESTING);
        act("{hYour slice beheads $n!{x",ch,NULL,victim,TO_VICT,POS_RESTING);
        act("{k$N's slice beheads $n!{x",ch,NULL,victim,TO_NOTVICT,POS_RESTING);
        act("{iYou are DEAD!!!{x",ch,NULL,victim,TO_CHAR,POS_RESTING);
	act("{h$n is DEAD!!!{x",ch,NULL,victim,TO_VICT,POS_RESTING);
	act("{k$n is DEAD!!!{x", ch,NULL,victim,TO_NOTVICT,POS_RESTING);
	act("A sword called '{z{RGodSlayer{x' vanishes.",ch,NULL,victim,TO_VICT,POS_RESTING);
	act("A sword called '{z{RGodSlayer{x' vanishes.",ch,NULL,victim,TO_NOTVICT,POS_RESTING);
	raw_kill( ch, victim );
    }
    return;
}

bool can_pk(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (ch == NULL || victim == NULL || IS_NPC(ch) || IS_NPC(victim))
	return TRUE;

    if (ch->pcdata->tier == victim->pcdata->tier)
    {
        if ( ch->level > victim->level + 15
        ||   ch->level < victim->level - 15 )
            return FALSE;
    }
    if ( ch->pcdata->tier - victim->pcdata->tier == 1
    ||   ch->pcdata->tier - victim->pcdata->tier == -1 )
    {
        if ( ch->level > victim->level + 10
        ||   ch->level < victim->level - 10 )
            return FALSE;
    }
    if ( ch->pcdata->tier - victim->pcdata->tier == 2
    ||   ch->pcdata->tier - victim->pcdata->tier == -2 )
    {
        if ( ch->level > victim->level + 5
        ||   ch->level < victim->level - 5 )
            return FALSE;
    }

    if ( ch == victim
    ||   !is_clan(victim)
    ||   !is_clan(ch)
    ||   !is_pkill(victim)
    ||   !is_pkill(ch)
    ||   ch->level < 20
    ||   victim->level < 20 )
        return FALSE;
    else
        return TRUE;
}

void check_pktimer( CHAR_DATA *ch, CHAR_DATA *victim, bool wizshow )
{
    CHAR_DATA *fch;

    if ( IS_NPC( ch )
    ||   IS_NPC( victim )
    ||   ch == victim
    ||   IS_SET( ch->in_room->room_flags, ROOM_ARENA )
    ||   IS_SET( ch->in_room->room_flags, ROOM_WAR ) )
	return;

    if ( wizshow && ch->pcdata->pktimer <= 0 )
    {
	char buf[MAX_STRING_LENGTH];

	if ( ch->in_room == NULL )
	    sprintf( buf, "{w({RPK{w) {V$N attacks %s.", victim->name );
	else
	    sprintf( buf, "{w({RPK{w) {V$N attacks %s at [%d] %s{V.",
		victim->name, ch->in_room->vnum, ch->in_room->name );
	wiznet( buf, ch, NULL, WIZ_PKILLS, 0, 0 );
    }

    if ( ch->master != NULL && !can_pk( victim, ch->master ) )
    {
	act( "$n stops following you.",
	    ch, NULL, ch->master, TO_VICT, POS_RESTING );
	act( "You stop following $N.",
	    ch, NULL, ch->master, TO_CHAR, POS_RESTING );
	ch->leader = NULL;
	ch->master = NULL;
    }

    if ( victim->master != NULL && !can_pk( ch, victim->master ) )
    {
	act( "$n stops following you.",
	    victim, NULL, victim->master, TO_VICT, POS_RESTING );
	act( "You stop following $N.",
	    victim, NULL, victim->master, TO_CHAR, POS_RESTING );
	victim->leader = NULL;
	victim->master = NULL;
    }

    for ( fch = player_list; fch != NULL; fch = fch->pcdata->next_player )
    {
	if ( fch->master == NULL )
	    continue;

	if ( fch->master == ch && !can_pk( fch, victim ) )
	{
	    act( "$n stops following you.",
		fch, NULL, fch->master, TO_VICT, POS_RESTING );
	    act( "You stop following $N.",
		fch, NULL, fch->master, TO_CHAR, POS_RESTING );
	    fch->leader = NULL;
	    fch->master = NULL;
	}

	if ( fch->master == victim && !can_pk( fch, ch ) )
	{
	    act( "$n stops following you.",
		fch, NULL, fch->master, TO_VICT, POS_RESTING );
	    act( "You stop following $N.",
		fch, NULL, fch->master, TO_CHAR, POS_RESTING );
	    fch->leader = NULL;
	    fch->master = NULL;
	}
    }

    if ( ch != victim
    &&   ( IS_AFFECTED( victim, AFF_CHARM )
    ||     is_affected( victim, gsn_charm_person ) ) )
    {
	affect_strip( victim, gsn_charm_person );
	REMOVE_BIT( victim->affected_by, AFF_CHARM );
    }

    if ( ch->pcdata->pktimer <= 0 )
	ch->pcdata->attacker = TRUE;

    if ( victim->pcdata->pktimer <= 0 )
	victim->pcdata->attacker = FALSE;

    ch->pcdata->pktimer      = 30;
    victim->pcdata->pktimer  = 30;
    ch->pcdata->opponent     = victim;
    victim->pcdata->opponent = ch;

    return;
}

bool check_pktest( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ((IS_NPC(ch) && ch->master == NULL)
    ||   IS_NPC(victim)
    ||   victim == ch
    ||   victim->pcdata->pktimer <= 0
    ||   victim->pcdata->opponent == NULL
    ||   victim->pcdata->opponent == ch )
	return TRUE;

    if (IS_NPC(ch) && ch->master != NULL)
    {
	if (!can_pk(ch->master,victim->pcdata->opponent))
	{
	    printf_to_char(ch->master,"%s is being hunted by %s, whom you may not attack.\n\r", victim->name, victim->pcdata->opponent->name);
	    printf_to_char(ch->master,"It is therefore deemed illegal to assist %s.\n\r", victim->name);
	    return FALSE;
	}
    }

    if (!can_pk(ch,victim->pcdata->opponent))
    {
	printf_to_char(ch,"%s is being hunted by %s, whom you may not attack.\n\r", victim->name, victim->pcdata->opponent->name);
	printf_to_char(ch,"It is therefore deemed illegal to assist %s.\n\r", victim->name);
	return FALSE;
    }
    return TRUE;
}

void do_engage( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim, *rch;
    char buf[MAX_STRING_LENGTH];
    int chance;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Whom do you wish to engage?\n\r", ch );
	return;
    }

    if ( ( chance = get_skill( ch, gsn_engage ) ) <= 0 )
    {
	send_to_char( "You know not how to engage opponents.\n\r", ch );
	return;
    }

    if ( ch->fighting == NULL )
    {
	send_to_char( "You must be fighting someone in order to engage another.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    if ( victim->fighting != ch )
    {
	send_to_char( "They are not fighting you.\n\r", ch );
	return;
    }

    if ( victim == ch->fighting )
    {
	act( "You are already fighting $m!",
	    ch, NULL, victim, TO_CHAR, POS_DEAD );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_HASTE ) )
	chance += 10;
    if ( IS_AFFECTED( ch->fighting, AFF_HASTE ) )
	chance -= 10;
    if ( IS_AFFECTED( ch, AFF_SLOW ) )
	chance -= 20;
    if ( IS_AFFECTED( ch->fighting, AFF_SLOW ) )
	chance += 20;
    chance += ( get_curr_stat( ch, STAT_DEX )
	    -   get_curr_stat( ch->fighting, STAT_DEX ) );

    WAIT_STATE( ch, skill_table[gsn_engage].beats );

    if ( number_percent( ) < URANGE( 5, chance, 95 ) )
    {
	sprintf( buf, "{hYou stop fighting %s {hand engage in combat against %s{h!{x\n\r",
	    PERS( ch->fighting, ch ), PERS( victim, ch ) );
	send_to_char( buf, ch );

	sprintf( buf, "{i%s {istops fighting %s {iand engages in combat against you!{x\n\r",
	    PERS( ch, victim ), PERS( ch->fighting, victim ) );
	send_to_char( buf, victim );

	sprintf( buf, "{i%s {istops fighting with you and engages in combat against %s{i!{x\n\r",
	    PERS( ch, ch->fighting ), PERS( victim, ch->fighting ) );
	send_to_char( buf, ch->fighting );

	for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
	{
	    if ( rch != ch && rch != victim && rch != ch->fighting )
	    {
		sprintf( buf, "{k%s {kstops fighting %s {kand engages in combat against %s{k.{x\n\r",
		    PERS( ch, rch ), PERS( ch->fighting, rch ), PERS ( victim, rch ) );
		send_to_char( buf, rch );
	    }
	}

	stop_fighting( ch, FALSE );
	set_fighting( ch, victim );
	check_improve( ch, gsn_engage, TRUE, 2 );
    } else {
	sprintf( buf, "{h%s {hsteps in front of you, preventing your engage against %s{h!{x\n\r",
	    PERS( ch->fighting, ch ), PERS( victim, ch ) );
	send_to_char( buf, ch );

	sprintf( buf, "{i%s {isteps in front of %s {istopping an engage against you!{x\n\r",
	    PERS( ch->fighting, victim ), PERS( ch, victim ) );
	send_to_char( buf, victim );

	sprintf( buf, "{iYou step in front of %s{i, stopping an engage against %s{i!{x\n\r",
	    PERS( ch, ch->fighting ), PERS( victim, ch->fighting ) );
	send_to_char( buf, ch->fighting );

	for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
	{
	    if ( rch != ch && rch != victim && rch != ch->fighting )
	    {
		sprintf( buf, "{k%s {ksteps in front of %s{k, stopping an engage against %s{k.{x\n\r",
		    PERS( ch->fighting, rch ), PERS( ch, rch ), PERS ( victim, rch ) );
		send_to_char( buf, rch );
	    }
	}

	check_improve( ch, gsn_engage, FALSE, 2 );
    }
}

void do_deathblow( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance;

    if ( ( chance = get_skill( ch, gsn_deathblow ) ) == 0 )
    {
	send_to_char( "You swing wildly, and look a little foolish.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "Deathblow whom?\n\r", ch );
	    return;
	}
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	spam_check( ch );
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
    ||   obj->value[0] != WEAPON_AXE )
    {
	send_to_char( "You must wield an axe to use deathblow.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You deathblow yourself.\n\r", ch );
	return;
    }

    if ( !check_stun_remember( ch )
    ||   is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   !cost_of_skill( ch, gsn_deathblow )
    ||   check_evade( ch, victim ) )
	return;

    if ( number_percent( ) > 9 * chance / 10 )
    {
	damage( ch, victim, 0, gsn_deathblow, obj->value[3], TRUE, FALSE, NULL );
	check_improve( ch, gsn_deathblow, FALSE, 3 );
    } else {
	one_hit( ch, victim, gsn_deathblow, FALSE );
	check_improve( ch, gsn_deathblow, TRUE, 3 );
    }

}

void do_dismember( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance;

    if ( ( chance = get_skill( ch, gsn_dismember ) ) == 0 )
    {
	send_to_char( "You swing wildly, and look a little foolish.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "Dismember whom?\n\r", ch );
	    return;
	}
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	spam_check( ch );
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
    ||   obj->value[0] != WEAPON_AXE )
    {
	send_to_char( "You must wield an axe to use dismember.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You dismember yourself.\n\r", ch );
	return;
    }

    if ( is_affected( victim, gsn_dismember ) )
    {
	send_to_char( "They have already been dismembered.\n\r", ch );
	return;
    }

    if ( !check_stun_remember( ch )
    ||   is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   !cost_of_skill( ch, gsn_dismember )
    ||   check_evade( ch, victim ) )
	return;

    if ( number_percent( ) > 9 * chance / 10 )
    {
	damage( ch, victim, 0, gsn_dismember, obj->value[3], TRUE, FALSE, NULL );
	check_improve( ch, gsn_dismember, FALSE, 3 );
    } else {
	AFFECT_DATA af;

	af.where	= TO_AFFECTS;
	af.type		= gsn_dismember;
	af.level	= ch->level;
	af.dur_type	= DUR_ROUNDS;
	af.duration	= 10;
	af.location	= APPLY_STR;
	af.modifier	= -3;
	af.bitvector	= 0;
	affect_to_char( victim, &af );

	af.location	= APPLY_DEX;
	affect_to_char( victim, &af );

	af.location	= APPLY_CON;
	affect_to_char( victim, &af );

	combat( "{h$n {hslices $N {hup, causing pieces to fly everywhere!{x",
	    ch ,obj, victim, TO_ROOM, COMBAT_OTHER );
	act( "{kYou slice $N {kup, spreading pieces of him around the room!{x",
	    ch, obj, victim, TO_CHAR, POS_RESTING );
	act( "{i$n {icuts you to pieces!{x",
	    ch, obj, victim, TO_VICT, POS_RESTING );

	one_hit( ch, victim, gsn_dismember, FALSE );
	check_improve( ch, gsn_dismember, TRUE, 3 );

	DAZE_STATE( victim, 2 * PULSE_VIOLENCE, DAZE_SPELL );
	DAZE_STATE( victim, 2 * PULSE_VIOLENCE, DAZE_SKILL );
	DAZE_STATE( victim, 2 * PULSE_VIOLENCE, DAZE_FLEE );
    }

}

void do_battlehymn( CHAR_DATA *ch, char *argument )
{
    int chance, hp_percent;

    if ( ( chance = get_skill( ch, gsn_battlehymn ) ) <= 0 )
    {
	send_to_char( "{hYou wonder who is making a strange squealing sound, until you realise that its you.{x\n\r", ch );
	return;
    }

    if ( is_affected( ch, gsn_battlehymn ) )
    {
	send_to_char( "{hYour spirits are inspired as much as they will go.{x\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CALM ) )
    {
	send_to_char( "You are not really in a mood to be singing.\n\r", ch );
	return;
    }

    if ( !cost_of_skill( ch, gsn_battlehymn ) )
	return;

    if ( ch->position == POS_FIGHTING )
        chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit / ch->max_hit;

    chance += 25 - hp_percent / 2;

    if ( number_percent( ) < chance )
    {
	AFFECT_DATA af;

	ch->hit += ch->level * 2;
	ch->hit = UMIN( ch->hit, ch->max_hit );

	send_to_char( "Your songs of glory infuse you with the power of {rWar{x!\n\r", ch );

	act( "{k$n has the song of war shimmering in $s eyes.{x",
	    ch, NULL, NULL, TO_ROOM, POS_RESTING );

	af.where	= TO_AFFECTS;
	af.type		= gsn_battlehymn;
	af.level	= ch->level;
	af.dur_type	= DUR_TICKS;
	af.duration	= 10;
	af.modifier	= -ch->level / 8;
	af.location	= APPLY_SAVES;
	af.bitvector	= 0;

	affect_to_char( ch, &af );

	check_improve( ch, gsn_battlehymn, TRUE, 2 );
    } else {
        send_to_char( "You sing a bit too offkey for this to work.\n\r", ch );
        check_improve( ch, gsn_battlehymn, FALSE, 2 );
    }
}

void do_warcry( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
    int chance;

    if ( ( chance = get_skill( ch, gsn_warcry ) ) <= 0 )
    {
	send_to_char( "You can cry in war...that's about it.\n\r", ch );
	return;
    }

    if ( is_affected( ch, gsn_warcry ) )
    {
	send_to_char( "You are already in a war frenzy.\n\r", ch );
	return;
    }

    if ( !cost_of_skill( ch, gsn_warcry ) )
	return;

    if ( number_percent( ) > chance )
    {
	send_to_char( "You grunt softly.\n\r", ch );
	act( "{k$n makes some soft gruting noises.{x",
	    ch, NULL, NULL, TO_ROOM, POS_RESTING );
	check_improve( ch, gsn_warcry, FALSE, 1 );
    } else {
	af.where	= TO_AFFECTS;
	af.type		= gsn_warcry;
	af.level	= ch->level;
	af.dur_type	= DUR_TICKS;
	af.location	= APPLY_HITROLL;
	af.duration	= ch->level / 15;
	af.modifier	= ch->level / 5;
	af.bitvector	= 0;
	affect_to_char( ch, &af );

	af.location	= APPLY_DAMROLL;
	affect_to_char( ch, &af );

	send_to_char( "You feel righteous as you yell out your warcry.\n\r", ch );
	act( "{k$n lets out a warcry.{x",
	    ch, NULL, NULL, TO_ROOM, POS_RESTING );

	check_improve( ch, gsn_warcry, TRUE, 1 );
    }
}

void do_shield_smash( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *shield;
    int chance, dam;

    if ( ( chance = get_skill( ch, gsn_shield_smash ) ) < 1 )
    {
	send_to_char( "You have a hard enough time defending yourself with a shield.\n\r", ch );
	return;
    }

    if ( ( shield = get_eq_char( ch, WEAR_SHIELD ) ) == NULL )
    {
	send_to_char( "You'll need a shield to smash with.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0')
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "{kWho do you want to smash?{x\n\r", ch );
	    return;
	}
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	spam_check( ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You try to smash your brains out, but fail.\n\r", ch );
	return;
    }

    if ( !check_stun_remember( ch ) )
	return;

    if( !can_see( ch, victim ) )
    {
	send_to_char( "Are they to your left or to your right? bleh - better rub.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   check_evade( ch, victim ) )
	return;

    WAIT_STATE( ch, skill_table[gsn_shield_smash].beats );

    if ( ch->size < victim->size )
	chance += ( ch->size - victim->size ) * 15;
    else
	chance += ( ch->size - victim->size ) * 10;

    /* stats */
    chance += get_curr_stat( ch, STAT_STR ) * 2;
    chance -= get_curr_stat( victim, STAT_CON );
    chance -= get_curr_stat( victim, STAT_DEX ) * 4 / 3;
    chance += GET_AC( victim, AC_BASH ) / 80;

    /* speed */
    if ( IS_AFFECTED( ch, AFF_HASTE ) )
	chance += 20;
    if ( IS_AFFECTED( victim, AFF_HASTE ) )
	chance -= 15;

    if ( number_percent( ) > chance )
    {
	damage( ch, victim, 0, gsn_shield_smash, DAM_BASH, TRUE, FALSE, NULL );
	check_improve( ch, gsn_shield_smash, FALSE, 3 );
    } else {
	DAZE_STATE( victim, 2 * PULSE_VIOLENCE, DAZE_FLEE );
	victim->position = POS_RESTING;
	dam = number_range( ch->level, ch->level * 2 ) + ( shield->level * 4 );
	damage( ch, victim, dam, gsn_shield_smash, DAM_BASH, TRUE, FALSE, NULL );
	check_improve( ch, gsn_shield_smash, TRUE, 3 );

	chance = get_skill( ch, gsn_stun ) / 5;

	if ( number_percent( ) < chance )
	{
	    chance = get_skill( ch, gsn_stun ) / 5;

	    if ( number_percent( ) < chance )
		victim->stunned = 2;
	    else
		victim->stunned = 1;

	    send_to_char( "{iYou are stunned, and have trouble getting back up!{x", ch );
	    act( "{h$N {his stunned by your smash!{x",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	    act( "{k$N {kis having trouble getting back up.{x",
		ch, NULL, victim, TO_NOTVICT, POS_RESTING );

	    check_improve( ch, gsn_stun, TRUE, 4 );
	}

	else
	    check_improve( ch, gsn_stun, FALSE, 4 );
    }
}

void dislodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, WEAR_SHIELD ) ) == NULL )
	return;

    if ( IS_OBJ_STAT( obj, ITEM_NOREMOVE ) )
    {
	act( "{jYou try to dislodge $p{j, but it won't budge!{x",
	    ch, obj, victim, TO_CHAR, POS_RESTING );
	act( "{j$n tries to dislodge your shield, but $p{j won't budge!{x",
	    ch, obj, victim, TO_VICT, POS_RESTING );
	combat( "{k$n tries to dislodge $N{k's shield, but fails.{x",
	    ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
	return;
    }

    act( "{j$n dislodges your shield and sends $p{j flying!{x",
	ch,  obj, victim, TO_VICT, POS_RESTING );
    act( "{jYou dislodge $N{j's shield and send $p{j flying!{x",
	ch, obj, victim, TO_CHAR, POS_RESTING );
    combat( "{k$n dislodges $N{k's shield and sends $p{k flying!{x",
	ch, obj, victim, TO_NOTVICT, COMBAT_OTHER );

    obj_from_char( obj );

    if ( IS_OBJ_STAT (obj, ITEM_NODROP ) || IS_OBJ_STAT( obj, ITEM_INVENTORY )
    ||   IS_OBJ_STAT (obj, ITEM_AQUEST ) || IS_OBJ_STAT( obj, ITEM_FORGED )
    ||   IS_IMMORTAL(victim) )
    {
	act( "{cA magical aura draws $p {cto you.{x",
	    victim, obj, NULL, TO_CHAR, POS_DEAD );
	act( "{cA magical aura draws $p {cto $n{c.{x",
	    victim, obj, NULL, TO_ROOM, POS_RESTING );
	obj_to_char( obj, victim );
    } else {
	obj->disarmed_from = victim;
	set_obj_sockets( victim, obj );
	set_arena_obj( victim, obj );
	obj_to_room( obj, victim->in_room );

	if ( IS_NPC( victim )
	&&   victim->wait == 0 && can_see_obj( victim, obj ) )
	    send_to_char( get_obj( victim, obj, NULL, FALSE ), victim );
    }
}

void do_dislodge( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance, hth = 0, ch_weapon, vict_weapon, ch_vict_weapon;

    if ( ( chance = get_skill( ch, gsn_dislodge ) ) <= 0 )
    {
	send_to_char( "You do not know how to dislodge opponents.\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL
    &&   ( ( hth = get_skill( ch, gsn_hand_to_hand ) ) == 0 ) )
    {
	send_to_char( "You must wield a weapon to dislodge.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "Dislodge whom?\n\r", ch );
	    return;
	}
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	spam_check( ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You can't do that to yourself!\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   check_evade( ch, victim ) )
	return;

    if ( !check_stun_remember( ch ) )
	return;

    if ( ( obj = get_eq_char( victim, WEAR_SHIELD ) ) == NULL )
    {
	send_to_char( "{hYour opponent is not wearing a shield.{x\n\r", ch );
	return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill( ch, get_weapon_sn( ch, TRUE ) );
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim, TRUE));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim, TRUE));

    /* skill */
    if ( get_eq_char( ch, WEAR_WIELD ) == NULL )
	chance = chance * hth / 150;
    else
	chance = chance * ch_weapon / 100;

    chance += ( ch_vict_weapon / 2 - vict_weapon ) / 2;

    /* dex vs. strength */
    chance += get_curr_stat( ch, STAT_DEX );
    chance -= 2 * get_curr_stat( victim, STAT_STR );

    /* level */
    chance += ( ch->level - victim->level ) * 2;
    chance /= 2;
	chance = UMIN( chance, 50 );

    /* and now the attack */
    if ( number_percent( ) < chance )
    {
	if ( ( chance = get_skill( victim, gsn_grip ) ) < 1 )
	{
	    WAIT_STATE( ch, skill_table[gsn_dislodge].beats );
	    dislodge( ch, victim );
	    check_improve( ch, gsn_dislodge, TRUE, 2 );
	    return;
	}

	if ( number_percent( ) > chance / 5 * 4 )
	{
	    WAIT_STATE( ch, skill_table[gsn_dislodge].beats );
	    dislodge( ch, victim );
	    check_improve( ch, gsn_dislodge, TRUE, 2 );
	    check_improve( victim, gsn_grip, FALSE, 2 );
	    return;
	}

	check_improve( victim, gsn_grip, TRUE, 2 );
    }

    WAIT_STATE( ch, skill_table[gsn_dislodge].beats );

    act( "{hYou fail to dislodge $N{h's shield.{x",
	ch, NULL, victim, TO_CHAR, POS_RESTING );
    act( "{i$n tries to dislodge your shield, but fails.{x",
	ch, NULL, victim, TO_VICT, POS_RESTING );
    combat( "{k$n {ktries to dislodge $N{k's shield, but fails.{x",
	ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );

    check_improve( ch, gsn_dislodge, FALSE, 3 );
}

void do_legsweep( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch, *vch_next;
    bool found = FALSE;
    int chance;

    if ( ( chance = get_skill( ch, gsn_legsweep ) ) <= 0 )
    {
	send_to_char( "Use your legs for something else buddy.\n\r", ch );
	return;
    }

    if ( !check_stun_remember( ch )
    ||   !cost_of_skill( ch, gsn_legsweep ) )
	return;

    if ( number_percent( ) >= chance )
    {
	send_to_char( "You get a little dizzy and stumble about.\n\r", ch );
	check_improve( ch, gsn_legsweep, FALSE, 6 );
	return;
    }

    for ( vch = ch->in_room->people; vch; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	if ( vch == ch
	||   is_safe( ch, vch )
	||   IS_IMMORTAL( vch )
	||   !check_kill_steal( ch, vch, FALSE )
	||   is_same_group( ch, vch )
	||   check_evade( ch, vch ) )
	    continue;

	found = TRUE;

	if ( vch->position < POS_FIGHTING )
	{
	    act( "$N is already on the ground.",
		ch, NULL, vch, TO_CHAR, POS_RESTING );
	    continue;
	}

	if ( IS_AFFECTED( vch, AFF_FLYING ) )
	{
	    act( "{G$N is flying and unaffected by your legsweep!{x",
		ch, NULL, vch, TO_CHAR, POS_RESTING );
	    act( "$n tries to legsweep you, but you fly right over $s leg!",
		ch, NULL, vch, TO_VICT, POS_RESTING );

	    if ( vch->fighting == NULL )
		multi_hit( vch, ch, TYPE_UNDEFINED, TRUE );
	    continue;
        }

	if ( number_percent( ) < chance + ch->level - vch->level )
	{
	    act( "You sweep $N's legs out from under $M.",
		ch, NULL, vch, TO_CHAR, POS_RESTING );
	    act( "$n sweeps your legs out from under you.",
		ch, NULL, vch, TO_VICT, POS_RESTING );
	    combat( "$n sweeps $N's legs out from under $M.",
		ch, NULL, vch, TO_NOTVICT, COMBAT_OTHER );

	    damage( ch, vch, number_range( ch->level, ch->level * 2 ),
		gsn_legsweep, DAM_BASH, TRUE, FALSE, NULL );

	    DAZE_STATE( vch, 2 * PULSE_VIOLENCE, DAZE_SPELL );
	    DAZE_STATE( vch, 2 * PULSE_VIOLENCE, DAZE_SKILL );
	    DAZE_STATE( vch, 2 * PULSE_VIOLENCE, DAZE_FLEE );

	    vch->position = POS_RESTING;
	} else {
	    act( "Your legsweep misses $N.",
		ch, NULL, vch, TO_CHAR, POS_RESTING );
	    act( "$n's legsweep misses you.",
		ch, NULL, vch, TO_VICT, POS_RESTING );
	    combat( "$n's legsweep misses $N.",
		ch, NULL, vch, TO_NOTVICT, COMBAT_OTHER );
	    damage( ch, vch, 0, gsn_legsweep, DAM_BASH, TRUE, FALSE, NULL );
	}
    }

    if ( found )
	check_improve( ch, gsn_legsweep, TRUE, 3 );
    else
    {
	act( "$n looks dizzy, and a tiny bit embarassed.",
	    ch, NULL, NULL, TO_ROOM, POS_RESTING );
	act( "You feel dizzy, and a tiny bit embarassed.",
	    ch, NULL, NULL, TO_CHAR, POS_RESTING );

	DAZE_STATE( ch, 2 * PULSE_VIOLENCE, DAZE_SPELL );
	DAZE_STATE( ch, 2 * PULSE_VIOLENCE, DAZE_SKILL );
	DAZE_STATE( ch, 2 * PULSE_VIOLENCE, DAZE_FLEE );
    }
}

void do_bandage( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    if ( ( chance = get_skill( ch, gsn_bandage ) ) < 1 )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
	victim = ch;

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->hit == victim->max_hit )
    {
	if ( ch == victim )
	    send_to_char( "You aren't even wounded.\n\r", ch );
	else
	    send_to_char( "They aren't even wounded.\n\r", ch );
	return;
    }

    if ( !cost_of_skill( ch, gsn_bandage ) )
	return;

    if ( number_percent( ) > UMIN( chance, 90 ) )
    {
	send_to_char( "You failed.\n\r", ch );
	check_improve( ch, gsn_bandage, FALSE, 2 );
 	return;
    }

    victim->hit = UMIN( victim->hit+100, victim->max_hit );
    check_improve( ch, gsn_bandage, TRUE, 2 );

    if ( ch == victim )
    {
	act( "$n applies bandages to $s wounds.",
	    ch, NULL, victim, TO_ROOM, POS_RESTING );
	act( "You apply bandages to your wounds.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
    } else {
	act( "$n applies bandages to $N's wounds.",
	    ch, NULL, victim, TO_NOTVICT, POS_RESTING );
	act( "$n applies bandages to your wounds.",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	act( "You apply bandages to $N's wounds.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	send_to_char( show_condition( ch, victim, VALUE_HIT_POINT ), ch );
    }
}

void do_salve( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af, *paf;
    CHAR_DATA *victim;
    bool found = FALSE;
    int chance;

    if ( ( chance = get_skill( ch, gsn_salve ) ) < 1 )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
	victim = ch;

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->hit == victim->max_hit )
    {
	if ( ch == victim )
	    send_to_char( "You aren't even wounded.\n\r", ch );
	else
	    send_to_char( "They aren't even wounded.\n\r", ch );
	return;
    }

    if ( !cost_of_skill( ch, gsn_salve ) )
	return;

    if ( number_percent( ) > UMIN( chance, 90 ) )
    {
	send_to_char( "You failed.\n\r", ch );
	check_improve( ch, gsn_salve, FALSE, 2 );
 	return;
    }

    victim->hit = UMIN( victim->hit+100, victim->max_hit );
    check_improve( ch, gsn_salve, TRUE, 2 );

    if ( ch == victim )
    {
	act( "$n applies salve to $s wounds.",
	    ch, NULL, victim, TO_ROOM, POS_RESTING );
	act( "You apply salve to your wounds.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
    } else {
	act( "$n applies salve to $N's wounds.",
	    ch, NULL, victim, TO_NOTVICT, POS_RESTING );
	act( "$n applies salve to your wounds.",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	act( "You apply salve to $N's wounds.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	send_to_char( show_condition( ch, victim, VALUE_HIT_POINT ), ch );
    }

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->where == TO_AFFECTS && paf->type == gsn_salve )
	{
	    found		= TRUE;
	    paf->duration	= ch->level / 20;
	    paf->modifier	= UMIN( paf->modifier + 5, 100 );
	}
    }

    if ( !found )
    {
	af.where	= TO_AFFECTS;
	af.type		= gsn_salve;
	af.dur_type	= DUR_TICKS;
	af.duration	= ch->level / 20;
	af.location	= APPLY_HITROLL;
	af.modifier	= 5;
	af.bitvector	= 0;
	affect_to_char( victim, &af );

	af.location	= APPLY_DAMROLL;
	affect_to_char( victim, &af );
    }
}

void do_assassinate( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    if ( ( chance = get_skill( ch, gsn_assassinate ) ) <= 0 )
    {
	send_to_char( "Your methods of assassination prove useless.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Assassinate whom?\n\r", ch );
	return;
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
	spam_check( ch );
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "Assassinate yourself?  Have you gone mad?\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL )
    {
	send_to_char( "{hYou need to wield a weapon to assassinate.{x\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE ) )
	return;

    if ( check_evade( ch, victim ) )
    {
	multi_hit( victim, ch, TYPE_UNDEFINED, TRUE );
	return;
    }

    if ( victim->position > POS_SLEEPING
    &&   victim->hit < victim->max_hit / 3
    &&   can_see( victim, ch ) )
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	if ( victim->fighting == NULL || victim->fighting != ch )
	    multi_hit( victim, ch, TYPE_UNDEFINED, TRUE );
	return;
    }

    if ( ch->pcdata == NULL )
	do_say( ch, "Ahh yet another victim..." );

    WAIT_STATE( ch, skill_table[gsn_assassinate].beats );

    if ( number_percent( ) < chance
    ||   ( chance >= 2 && !IS_AWAKE( victim ) ) )
    {
	act( "{G$N's weakness has been exposed.{x",
	    ch, NULL, victim, TO_CHAR, POS_RESTING );
	act( "{GYour weakness has been exposed.{x",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	combat( "{G$N's weakness has been exposed.{x",
	    ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
	check_improve( ch, gsn_assassinate, TRUE, 3 );
	multi_hit( ch, victim, gsn_assassinate, TRUE );
    } else {
	check_improve( ch, gsn_assassinate, FALSE, 3 );
	damage( ch, victim, 0, gsn_assassinate, DAM_OTHER, TRUE, FALSE, NULL );
    }
}

void do_storm_blades( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch, *vch_next;
    OBJ_DATA *obj;
    int chance;

    if ( ( chance = get_skill( ch, gsn_storm_of_blades ) ) <= 0 )
    {
	send_to_char( "You franticly swing your blades around.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
    {
	send_to_char( "You need to wield a primary weapon first.\n\r", ch );
	return;
    }

    if ( !check_stun_remember( ch )
    ||   !cost_of_skill( ch, gsn_storm_of_blades ) )
	return;

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
	chance = chance * 2 / 3;

    act( "$n holds $p firmly, and starts a storm of blades!",
	ch, obj, NULL, TO_ROOM, POS_RESTING );

    act( "You hold $p firmly, and start a storm of blades!",
	ch, obj, NULL, TO_CHAR, POS_RESTING );

    if ( number_percent( ) >= chance )
    {
	send_to_char( "You swing your blades wildly and miss everything.\n\r", ch );
	check_improve( ch, gsn_storm_of_blades, FALSE, 4 );
	return;
    }

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	if ( is_safe( ch, vch )
	||   is_same_group( ch, vch )
	||   IS_IMMORTAL( vch )
	||   check_evade( ch, vch ) )
	    continue;

	if ( IS_NPC( vch )
	&&   vch->fighting != NULL
	&&   !IS_NPC(vch->fighting)
	&&   vch->fighting != ch
	&&   !is_same_group( ch, vch->fighting )
	&&   !can_pk( ch, vch->fighting ) )
	    continue;

	act( "$n unleashes $s blades on you!",
	    ch, NULL, vch, TO_VICT, POS_RESTING );
	multi_hit( ch, vch, gsn_storm_of_blades, TRUE );
    }

    check_improve( ch, gsn_storm_of_blades, TRUE, 4 );
}

void do_cross_slash( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    if ( ( chance = get_skill( ch, gsn_cross_slash ) ) <= 0 )
    {
	send_to_char( "Cross Slash?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "You aren't fighting anyone.\n\r", ch );
	    return;
	}
    } else {
	if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
	{
	    send_to_char( "You can't cross slash someone who isn't here!\n\r", ch );
	    return;
	}
    }

    if ( victim == ch )
    {
	send_to_char( "Your arms can't reach that far!\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL
    ||   get_eq_char( ch, WEAR_SECONDARY ) == NULL )
    {
        send_to_char( "{hYou need to wield two weapons to cross slash.{x\n\r", ch );
        return;
    }

    if ( !check_stun_remember( ch ) )
	return;

    if ( !can_see( ch, victim ) )
    {
	send_to_char( "You swing blindly at the wall.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   !cost_of_skill( ch, gsn_cross_slash )
    ||   check_evade( ch, victim ) )
	return;

    if (  number_percent( ) < chance )
    {
	one_hit( ch, victim, gsn_cross_slash, FALSE );
	one_hit( ch, victim, gsn_cross_slash, TRUE );
	check_improve( ch, gsn_cross_slash, TRUE, 2 );
    } else {
        damage( ch, victim, 0, gsn_cross_slash, DAM_OTHER, TRUE, FALSE, NULL );
        damage( ch, victim, 0, gsn_cross_slash, DAM_OTHER, TRUE, FALSE, NULL );
	check_improve( ch, gsn_cross_slash, FALSE, 2 );
    }
}

void do_overhand( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance, dam;

    if ( ( chance = get_skill( ch, gsn_overhand ) ) <= 0 )
    {
	send_to_char( "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "Whom do you wish to overhand?\n\r", ch );
	    return;
	}
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You try an overhand to yourself, but fail.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
    ||   ( obj->value[0] != WEAPON_AXE && obj->value[0] != WEAPON_MACE ) )
    {
	send_to_char( "{hYou need to wield an axe or mace to overhand.{x\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   !check_stun_remember( ch )
    ||   !cost_of_skill( ch, gsn_overhand ) )
	return;

    dam = number_range( 10, 20 );

    if ( chance > number_percent( ) )
    {
	if ( damage( ch, victim, dam, gsn_overhand, DAM_BASH, TRUE, FALSE, NULL )
	&&   number_percent( ) < 25 )
	{
	    act( "{hYour overhand {YBASHES{h $N on the head!{x",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	    act( "{h$n overhand {YBASHES{h you in the skull!{x",
		ch, NULL, victim, TO_VICT, POS_RESTING );
	    combat( "{k$n's overhand {YBASHES{h $N skull in!{x",
		ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
	    victim->stunned = 1;
	}
	check_improve( ch, gsn_overhand, TRUE, 2 );
    } else {
	damage( ch, victim, 0, gsn_overhand, DAM_BASH, TRUE, FALSE, NULL );
	check_improve( ch, gsn_overhand, FALSE, 2 );
    }

    return;
}

void do_lunge( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int dam, skill;

    if ( ( skill = get_skill( ch, gsn_lunge ) ) <= 0 )
    {
	send_to_char( "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
    ||   obj->value[0] != WEAPON_SWORD )
    {
	send_to_char( "{hYou need to wield a sword to lunge.{x\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "Whom do you wish to lunge at?\n\r", ch );
	    return;
	}
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You try to lunge at yourself, but fail.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   !check_stun_remember( ch )
    ||   !cost_of_skill( ch, gsn_lunge ) )
	return;

    dam = dice( obj->value[1], obj->value[2] ) * skill / 100;
    dam += ( dam * 2 * get_skill( ch, gsn_enhanced_damage ) / 100 );
    dam += ( dam * 2 * get_skill( ch, gsn_critical ) / 105 );
    dam += ( dam * 2 * get_skill( ch, gsn_ultra_damage ) / 105 );

    if ( !can_see( ch, victim ) )
	dam *= 0.65;

    if ( skill > number_percent( ) )
    {
	damage( ch, victim, dam, gsn_lunge, attack_table[obj->value[3]].damage, TRUE, FALSE, NULL );
	check_improve( ch, gsn_lunge, TRUE, 3 );
    } else {
	damage( ch, victim, 0, gsn_lunge, attack_table[obj->value[3]].damage, TRUE, FALSE, NULL );
	check_improve( ch, gsn_lunge, FALSE, 3 );
    }

    return;
}

void do_crush( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance, dam;

    if ( ( chance = get_skill( ch, gsn_crush ) ) <= 0 )
    {
	send_to_char( "But, You might break a nail!\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( !check_stun_remember( ch ) )
	return;

    /* check to see if they are unarmed */
    if ( ( get_eq_char( ch, WEAR_WIELD ) ) != NULL )
    {
	send_to_char( "You may not use this while wielding a weapon.\n\r", ch );
	return;
    }

    dam = number_range( 1, 100 );
    WAIT_STATE( ch, skill_table[gsn_crush].beats );

    if ( number_percent( ) > chance )
    {
	send_to_char( "You failed.\n\r", ch );
	check_improve( ch, gsn_crush, FALSE, 2 );
	return;
    }

    if ( damage( ch, victim, 2*chance + number_range( dam, (ch->level*2) ),
	gsn_crush, DAM_BASH, TRUE, FALSE, NULL ) )
    {
	check_improve( ch, gsn_crush, TRUE, 2 );
	if ( dam > 90 )
	{
	    /* Head */
	    AFFECT_DATA af;
	    act( "{hYou grab $N {hby the neck and {YSQUEEZE{h!{x", ch, NULL, victim, TO_CHAR, POS_RESTING );
	    act( "{i$n {igrabs you by the neck and things go {8black{i.{x", ch, NULL, victim, TO_VICT, POS_RESTING );
	    combat( "{k$n {kgrabs $N {kby the neck and {YSQUEEZES{h!{x", ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );

	    stop_fighting( victim, TRUE );
	    victim->position = POS_SLEEPING;

	    af.where	= TO_AFFECTS;
	    af.type 	= gsn_strangle;
	    af.level 	= ch->level;
	    af.dur_type	= DUR_TICKS;
	    af.duration	= 0;
	    af.location	= APPLY_AC;
	    af.modifier	= 350;
	    af.bitvector = AFF_SLEEP;
	    affect_join( victim, &af );
	    return;
	}

	else if ( dam > 60 )
	{
	    /* Torso */
	    act( "{hYou grab $N{h's chest and {YSQUEEZE{h!{x", ch, NULL, victim, TO_CHAR, POS_RESTING );
	    act( "{i$n {igrabs your chest and {YSQUEEZES{i!{x", ch, NULL, victim, TO_VICT, POS_RESTING );
	    combat( "{k$n {kgrabs $N{k's chest and {YSQUEEZES{k!{x", ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
	    victim->move *= .50;
	    return;
	}

	else if ( dam > 30 )
	{
	    /* Arm */
	    act( "{hYou grab $N {hby the arm and {YSQUEEZE{h!{x", ch, NULL, victim, TO_CHAR, POS_RESTING );
	    act( "{i$n {igrabs you by the arm and {YSQUEEZES{i!{x", ch, NULL, victim, TO_VICT, POS_RESTING );
	    combat( "{k$n {kgrabs $N {kby the arm and {YSQUEEZES{k!{x", ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
	    disarm( ch, victim, WEAR_WIELD );
	    return;
	}
    }
}

void do_onslaught( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int skill;

    if ( ( skill = get_skill( ch, gsn_onslaught ) ) <= 0 )
    {
	send_to_char( "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "You work yourself up and snort.\n\r", ch );
	    return;
        }
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You try to work up an onslaught against yourself, but fail.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   !check_stun_remember( ch )
    ||   !cost_of_skill( ch, gsn_onslaught ) )
	return;

    if ( skill > number_percent( ) )
    {
	multi_hit( ch, victim, gsn_onslaught, TRUE );
	check_improve( ch, gsn_onslaught, TRUE, 3 );
    } else {
	damage( ch, victim, 0, gsn_onslaught, DAM_BASH, TRUE, FALSE, NULL );
	check_improve( ch, gsn_onslaught, FALSE, 3 );
    }
    return;
}

void do_scalp( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance, dam;

    if ( ( chance = get_skill( ch, gsn_scalp ) ) <= 0 )
    {
	send_to_char( "You might hurt yourself!\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "Scalp whom?\n\r", ch );
	    return;
	}
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Your going to hurt your self silly.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
    ||   obj->value[0] != WEAPON_AXE )
    {
	send_to_char( "{hYou need to wield an axe to scalp.{x\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   !check_stun_remember( ch )
    ||   !cost_of_skill( ch, gsn_scalp ) )
	return;

    if ( chance > number_percent( ) )
    {
	dam = dice( obj->value[1], obj->value[2] ) * chance / 100;
	dam += ( dam * get_skill( ch, gsn_enhanced_damage ) / 100 );
	dam += ( dam * get_skill( ch, gsn_critical ) / 100 );
	dam += ( dam * get_skill( ch, gsn_ultra_damage ) / 100 );

	if ( !can_see( ch, victim ) )
	    dam *= 0.65;

	damage( ch, victim, dam, gsn_scalp, attack_table[obj->value[3]].damage, TRUE, FALSE, NULL );

	check_improve( ch, gsn_scalp, FALSE, 3 );
    } else {
	damage( ch, victim, 0, gsn_scalp, DAM_OTHER, TRUE, FALSE, NULL );
	check_improve( ch, gsn_scalp, FALSE, 3 );
    }

    return;
}

void do_stake( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    if ( ( chance = get_skill( ch, gsn_stake ) ) <= 0 )
    {
	send_to_char( "Stake? What's that?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( ( victim = ch->fighting ) == NULL )
	{
	    send_to_char( "Stake what undead??\n\r", ch );
	    return;
	}
    }

    else if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Suicide is a sin my friend.\n\r", ch );
	return;
    }

    if ( !IS_NPC( victim ) && victim->race != race_lookup( "vampire" ) )
    {
	send_to_char( "You cannot stake a non-vampire player.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim )
    &&   !is_name( "vampire", victim->name )
    &&   !is_name( "undead", victim->name )
    &&   !is_name( "zombie", victim->name )
    &&   !is_name( "corpse", victim->name ) )
    {
	send_to_char( "You cannot stake this mob.\n\r", ch );
	return;
    }

    if ( !can_see( ch, victim ) )
    {
	send_to_char( "You stake blindly into the air.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim )
    ||   !check_kill_steal( ch, victim, TRUE )
    ||   !check_stun_remember( ch )
    ||   !cost_of_skill( ch, gsn_stake ) )
	return;

    if ( ch->size < victim->size )
	chance += ( ch->size - victim->size ) * 2;
    else
	chance += ( ch->size - victim->size ) * 2;

    chance -= GET_AC( victim, AC_PIERCE ) / 30;

    if ( IS_AFFECTED( ch, AFF_HASTE ) )
	chance += 10;

    if ( IS_AFFECTED( victim, AFF_HASTE ) )
	chance -= 10;

    chance += ( ch->level - ( victim->level ) ) / 3;

    if ( number_percent( ) < chance )
    {
	int dam = dice( ch->level / 5, ch->level / 3 );

	act( "$n has struck a stake in your heart!",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	act( "You slam a stake into $N!", ch, NULL, victim, TO_CHAR, POS_RESTING );
	combat( "$n slams a stake into $N .",
	    ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
	check_improve( ch, gsn_stake, TRUE, 2 );

	damage( ch, victim, dam, gsn_stake, DAM_WOOD, TRUE, FALSE, NULL );
    } else {
        act( "Your stake misses $N.", ch, NULL, victim, TO_CHAR, POS_RESTING );
	combat( "$n falls flat on $s face.",
	    ch, NULL, victim, TO_NOTVICT, COMBAT_OTHER );
	act( "You evade $n's stake,causing $m to fall flat on $s face.",
	    ch, NULL, victim, TO_VICT, POS_RESTING );
	damage( ch, victim, 0, gsn_stake, DAM_WOOD, TRUE, FALSE, NULL );
	check_improve( ch, gsn_stake, FALSE, 2 );
    }
}

void mobile_attack( CHAR_DATA *ch, CHAR_DATA *victim )
{
    free_runbuf( victim->desc );

    if ( !run_ambush( ch, victim )
    &&   !run_backstab( ch, victim ) )
	multi_hit( ch, victim, TYPE_UNDEFINED, TRUE );
}

