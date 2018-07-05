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
#include "interp.h"
#include "recycle.h"
#include "simscripts.h"

void    quest_update    	args( ( void ) );
void	gquest_update		args( ( void ) );
void	auction_update		args( ( void ) );
bool	mob_cast		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int sn ) );
void	sharpen_weapon		args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void	hone_weapon		args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void	envenom_weapon		args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void	say_spell		args( ( CHAR_DATA *ch, int sn ) );
void	mobile_attack		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	auction_channel		args( ( CHAR_DATA *ch, AUCTION_DATA *auc, char *msg ) );
void	auto_two_way		args( ( sh_int clan ) );
sh_int	has_two_way		args( ( sh_int clan ) );

void advance_level( CHAR_DATA *ch, bool show )
{
    char buf[MAX_STRING_LENGTH];
    int add_hp, add_mana, add_move, add_prac;

    ch->pcdata->last_level = ( ch->pcdata->played + ( int ) ( current_time - ch->pcdata->logon ) ) / 3600;

    add_hp = con_app[get_curr_stat(ch,STAT_CON)].hitp +
	number_range(	class_table[ch->class].hp_min,
			class_table[ch->class].hp_max );

    add_mana = number_range(
	( 3 * (get_curr_stat(ch,STAT_INT) + get_curr_stat(ch,STAT_WIS))/2),
	( 4 * (get_curr_stat(ch,STAT_INT) + get_curr_stat(ch,STAT_WIS))/2) );


    add_move = number_range( get_curr_stat( ch, STAT_DEX ),
			     get_curr_stat( ch, STAT_DEX ) * 2 );

    add_prac = wis_app[get_curr_stat( ch, STAT_WIS )].practice;

    add_hp = add_hp * 9/10;
    add_hp = add_hp * 7/4;
    add_mana = add_mana * 9/10;
    add_mana = add_mana * 7/4;

    add_mana = add_mana * class_table[ch->class].mana_percent / 100;

    ch->max_hit			+= add_hp;
    ch->max_mana		+= add_mana;
    ch->max_move		+= add_move;
    ch->pcdata->perm_hit	+= add_hp;
    ch->pcdata->perm_mana	+= add_mana;
    ch->pcdata->perm_move	+= add_move;
    ch->pcdata->practice	+= add_prac;
    ch->pcdata->train		+= 2;

    if ( show )
    {
	sprintf( buf, "{yYour gain is: {g%d{y/{G%d{y hp, {g%d{y/{G%d{y m, {g%d{y/{G%d{y mv {g%d{y/{G%d{y prac.{x\n\r",
	    add_hp,	ch->max_hit,
	    add_mana,	ch->max_mana,
	    add_move,	ch->max_move,
	    add_prac,	ch->pcdata->practice );
	send_to_char( buf, ch );

	if ( number_percent( ) < 30 )
	{
	    ch->hit = ch->max_hit;
	    act( "$G {Rheals your wounds!{x",
		ch, NULL, NULL, TO_CHAR, POS_DEAD );
	}

	if ( number_percent( ) < 30 )
	{
	    ch->mana = ch->max_mana;
	    act( "$G {Menergizes you!{x",
		ch, NULL, NULL, TO_CHAR, POS_DEAD );
	}

	if ( number_percent( ) < 30 )
	{
	    ch->move = ch->max_move;
	    act( "$G {Crefreshes you!{x",
		ch, NULL, NULL, TO_CHAR, POS_DEAD );
	}

	update_pos( ch );
    }
}   

void gain_exp( CHAR_DATA *ch, int gain, bool Bonus )
{
    char buf[MAX_STRING_LENGTH];
    int count = 0;

    if ( gain == 0 ) return;

    if ( IS_NPC( ch ) || IS_SET( ch->act, PLR_NOEXP ) )
	return;

    // Group leader bonus.
    if ( Bonus && ch->leader == NULL ) {
     int grouped=0;
     CHAR_DATA *wch;
     for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player ) {
      if ( in_group(ch,wch) ) grouped++;
      if ( wch->leader || wch->master ) {
       char b[MSL];
       int groupbonus=(int) ( (float) gain * 0.1f );
       if ( groupbonus == 0 ) continue;
       sprintf(b, "{!You gain a bonus of {#%d {!experience for partying with this group.\n\r",groupbonus);
       send_to_char( b, wch );
       gain_exp(wch,gain,FALSE);
      }
     }
     if ( grouped > 0 ) {
      char b[MSL];
      int leadbonus=(int) ( (float) gain * ( (float) grouped / 7.0f ) );
      if ( leadbonus > 0 ) {
      gain+=leadbonus;
      sprintf(b, "{!You gain a bonus of {#%d {!experience for leading this group of {1%d.\n\r",leadbonus,grouped+1);
      send_to_char( b, ch );
      }
     }
    }

    if ( ch->level >= LEVEL_HERO )
    {
	ch->pcdata->devote[ch->pcdata->devote[DEVOTE_CURRENT]] += gain;
	if ( ch->pcdata->devote[ch->pcdata->devote[DEVOTE_CURRENT]] >= ch->pcdata->devote_next[ch->pcdata->devote[DEVOTE_CURRENT]] )
	{
	    switch (ch->pcdata->devote[DEVOTE_CURRENT])
	    {
	    	case DEVOTE_BODY:
	    	    send_to_char("{GYou feel more resilient, your skin seeming to harden!\n\r{x",ch);
	    	    break;
            	case DEVOTE_MIND:
            	    send_to_char("{GYou feel wisdom pour into your mind and magic seems less of a threat!\n\r{x",ch);
	    	    break;
            	case DEVOTE_SPIRIT:
            	    send_to_char("{GYou breathe in, feeling more energised and attuned to the ways of magic!\n\r{x",ch);
	    	    break;
            	case DEVOTE_GRACE:
            	    send_to_char("{GYou feel yourself becoming more adept at swiftly dodging blows and skillfully returning them!\n\r{x",ch);
	    	    break;
            	case DEVOTE_FORCE:
            	    send_to_char("{GYour muscles begin to bulge before your eyes!\n\r{x",ch);
	    	    break;
            	case DEVOTE_LIFE:
            	    send_to_char("{GYou feel ready to face another fight with renewed vigour!\n\r{x",ch);
	    	    break;
            	case DEVOTE_SKILLS:
            	    send_to_char("{GYou feel as if your knowledge of secret techniques have been furthered!\n\r{x",ch);
	    	    break;
            	case DEVOTE_SPELLS:
            	    send_to_char("{GYou feel as if your knowledge of the secrets of magic have been furthered!\n\r{x",ch);
	    	    break;
            	case DEVOTE_EQ:
            	    send_to_char("{GThe intricacies of modern equipment become easier to learn!\n\r{x",ch);
	    	    break;
            	default:
                    bug("No phrase specified for advance in devotion %d", ch->pcdata->devote[ch->pcdata->devote[DEVOTE_CURRENT]]);
                    send_to_char("{GYou feel as if your devotion to self improvement has paid off!\n\r{x",ch);
                    break;
	    }
	    do_devote_assign( ch );
	}
	return;
    }

    ch->exp = UMAX( exp_per_level( ch, ch->pcdata->points ), ch->exp + gain );
    while ( ch->level < LEVEL_HERO && ch->exp >= 
	exp_per_level( ch, ch->pcdata->points ) * ( ch->level + 1 ) )
    {
	ch->level++;
	ch->magic_power++;

	sprintf( buf, "{GYour experience in the lands of %s {Ggrants you level {y%d{G!{x\n\r",
	    mud_stat.mud_name_string, ch->level );
	send_to_char( buf, ch );

	sprintf( buf, "$N has attained level %d!", ch->level );
	wiznet( buf, ch, NULL, WIZ_LEVELS, 0, 0 );
	advance_level( ch, TRUE );
	save_char_obj( ch, 0 );

	if ( ++count >= 3 )
	{
	    sprintf( buf, "%s just gained %d levels from %d exp, player exp reset",
		ch->name, count, gain );
	    wiznet( buf, ch, NULL, 0, 0, 0 );
	    log_string( buf );

	    ch->exp = exp_per_level( ch, ch->pcdata->points ) * ch->level;
	    send_to_char( "\n\r{REXP PROBLEM NOTED, YOUR EXP HAS BEEN RESET.{x\n\r", ch );
	}
    }

    return;
}

int hit_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain =  5 + ch->level;

	switch(ch->position)
	{
	    default : 		gain /= 2;			break;
	    case POS_SLEEPING: 	gain = 3 * gain/2;		break;
	    case POS_RESTING:  					break;
	    case POS_FIGHTING:	gain /= 3;		 	break;
 	}

	
    }
    else
    {
	gain = UMAX(3,get_curr_stat(ch,STAT_CON) - 3 + ch->level/2); 
	gain += class_table[ch->class].hp_max - 10;
 	number = number_percent();

	if (number < get_skill(ch,gsn_fast_healing))
	{
	    gain += number * gain / 100;
	    if (ch->hit < ch->max_hit)
		check_improve(ch,gsn_fast_healing,TRUE,8);
	}

	switch ( ch->position )
	{
	    default:	   	gain /= 4;			break;
	    case POS_SLEEPING: 					break;
	    case POS_RESTING:  	gain /= 2;			break;
	    case POS_FIGHTING: 	gain /= 6;			break;
	}

	if ( ch->pcdata->condition[COND_HUNGER] == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

    }

    gain = gain * ch->in_room->heal_rate / 90;
    
    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[3] / 90;

    if ( IS_AFFECTED( ch, AFF_REGENERATION )
    ||   is_affected( ch, gsn_mana_tap ) )
	gain *= 2;

    if ( is_affected( ch, gsn_mana_tap ) )
	gain *= 2;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
	gain /=2 ;

    return UMIN(gain, ch->max_hit - ch->hit);
}

int mana_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain = 5 + ch->level;
 	if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 2;
	switch (ch->position)
	{
	    default:		gain /= 2;		break;
	    case POS_SLEEPING:	gain = 3 * gain/2;	break;
   	    case POS_RESTING:				break;
	    case POS_FIGHTING:	gain /= 3;		break;
    	}
    }
    else
    {
	gain = ((get_curr_stat(ch,STAT_WIS) 
	      + get_curr_stat(ch,STAT_INT) + ch->level) / 3) * 2;
 	if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 2;
	number = number_percent();
	if (number < get_skill(ch,gsn_meditation))
	{
	    gain += number * gain / 100;
	    if (ch->mana < ch->max_mana)
	        check_improve(ch,gsn_meditation,TRUE,8);
	}

	gain = gain * class_table[ch->class].mana_percent / 100;

	switch ( ch->position )
	{
	    default:		gain /= 4;			break;
	    case POS_SLEEPING: 					break;
	    case POS_RESTING:	gain /= 2;			break;
	    case POS_FIGHTING:	gain /= 6;			break;
	}

	if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

    }

    gain = gain * ch->in_room->mana_rate / 90;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[4] / 90;

    if ( IS_AFFECTED( ch, AFF_POISON ) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
        gain /=2 ;

    return UMIN(gain, ch->max_mana - ch->mana);
}



int move_gain( CHAR_DATA *ch )
{
    int gain;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain = ch->level;
 	if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 2;
    }
    else
    {
	gain = UMAX( 15, ch->level );

 	if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 2;

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_stat(ch,STAT_DEX);		break;
	case POS_RESTING:  gain += get_curr_stat(ch,STAT_DEX) / 2;	break;
	}

	if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;
    }

    gain = gain * ch->in_room->heal_rate/90;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[3] / 90;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
        gain /=2 ;

    return UMIN(gain, ch->max_move - ch->move);
}

void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;

    if ( value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
	return;

    condition				= ch->pcdata->condition[iCond];

    if (condition == -1)
        return;

    ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 48 );

    if ( ch->pcdata->condition[iCond] == 0 )
    {
	switch ( iCond )
	{
	case COND_HUNGER:
	    send_to_char( "You are hungry.\n\r",  ch );
	    break;

	case COND_THIRST:
	    send_to_char( "You are thirsty.\n\r", ch );
	    break;

	case COND_DRUNK:
	    if ( condition != 0 )
		send_to_char( "You are sober.\n\r", ch );
	    break;
	}
    }

    return;
}

bool chance( CHAR_DATA *ch, sh_int sn )
{
    if ( get_skill( ch, sn ) == 0
    ||   number_percent( ) > 50 )
	return FALSE;

    return TRUE;
}

bool mob_check_weapon( CHAR_DATA *ch, OBJ_DATA *weapon )
{
    if ( !IS_WEAPON_STAT( weapon, WEAPON_SHARP ) && chance( ch, gsn_sharpen ) )
	{ sharpen_weapon( ch, weapon ); return TRUE; }

    if ( !IS_WEAPON_STAT( weapon, WEAPON_VORPAL ) && chance( ch, gsn_hone ) )
	{ hone_weapon( ch, weapon ); return TRUE; }

    if ( !IS_WEAPON_STAT( weapon, WEAPON_FLAMING )
    &&   chance( ch, gsn_backdraft ) )
    {
	say_spell( ch, gsn_backdraft );
	(*skill_table[gsn_backdraft].spell_fun)
	    ( gsn_backdraft, ch->level, ch, (void *)weapon, TARGET_OBJ );
	return TRUE;
    }

    if ( !IS_WEAPON_STAT( weapon, WEAPON_VAMPIRIC ) && chance( ch, gsn_leech ) )
    {
	say_spell( ch, gsn_leech );
	(*skill_table[gsn_leech].spell_fun)
	    ( gsn_leech, ch->level, ch, (void *)weapon, TARGET_OBJ );
	return TRUE;
    }

    if ( !IS_WEAPON_STAT( weapon, WEAPON_SHOCKING )
    &&   chance( ch, gsn_electrify ) )
    {
	say_spell( ch, gsn_electrify );
	(*skill_table[gsn_electrify].spell_fun)
	    ( gsn_electrify, ch->level, ch, (void *)weapon, TARGET_OBJ );
	return TRUE;
    }

    if ( !IS_WEAPON_STAT( weapon, WEAPON_FROST )
    &&   chance( ch, gsn_glacial_aura ) )
    {
	say_spell( ch, gsn_glacial_aura );
	(*skill_table[gsn_glacial_aura].spell_fun)
	    ( gsn_glacial_aura, ch->level, ch, (void *)weapon, TARGET_OBJ );
	return TRUE;
    }

    if ( !IS_WEAPON_STAT( weapon, WEAPON_POISON ) )
    {
	if ( chance( ch, gsn_envenom ) )
	    { envenom_weapon( ch, weapon ); return TRUE; }

	if ( chance( ch, gsn_poison ) )
	{
	    say_spell( ch, gsn_poison );
	    (*skill_table[gsn_poison].spell_fun)
		( gsn_poison, ch->level, ch, (void *)weapon, TARGET_OBJ );
	    return TRUE;
	}
    }

    return FALSE;
}

void check_smart_mob( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    sh_int dam = 0, type = -1, pos;

    if ( ch->position == POS_FIGHTING )
    {
	if ( ( victim = ch->fighting ) == NULL
	||   victim->in_room != ch->in_room )
	    return;

	if ( IS_AFFECTED( ch, AFF_BLIND )
	&&   number_percent( ) <= 35 )
	{
	    if ( chance( ch, gsn_cure_blind )
	    &&   is_affected( ch, gsn_blindness ) )
	        { mob_cast( ch, ch, gsn_cure_blind ); return; }

	    if ( chance( ch, gsn_rub ) && is_affected( ch, gsn_dirt ) )
		{ do_rub( ch, "" ); return; }
	}

	if ( !IS_AFFECTED( ch, AFF_HASTE ) && chance( ch, gsn_haste ) )
	    { mob_cast( ch, ch, gsn_haste ); return; }

	if ( ch->hit < ch->max_hit / 5 )
	{
	    if ( victim->hit > victim->max_hit / 10 )
	    {
		if ( chance( ch, gsn_divine_heal ) )
		    { mob_cast( ch, ch, gsn_divine_heal ); return; }

		if ( chance( ch, gsn_heal ) )
		    { mob_cast( ch, ch, gsn_heal ); return; }

		if ( chance( ch, gsn_cure_serious ) )
		    { mob_cast( ch, ch, gsn_cure_serious ); return; }

		if ( chance( ch, gsn_cure_critical ) )
		    { mob_cast( ch, ch, gsn_cure_critical ); return; }

		if ( chance( ch, gsn_cure_light ) )
		    { mob_cast( ch, ch, gsn_cure_light ); return; }
	    }
	}

	if ( !IS_AFFECTED( victim, AFF_BLIND ) && number_percent( ) < 35 )
	{
	    if ( chance( ch, gsn_gouge ) )
		{ do_gouge( ch, "" ); return; }

	    if ( chance( ch, gsn_blindness ) )
		{ mob_cast( ch, victim, gsn_blindness ); return; }

	    if ( chance( ch, gsn_dirt ) )
		{ do_dirt( ch, "" ); return; }
	}

	if ( ( IS_SHIELDED( victim, SHD_SANCTUARY )
	||     IS_SHIELDED( victim, SHD_DIVINITY ) )
	&&   chance( ch, gsn_dispel_magic ) )
	    { mob_cast( ch, victim, gsn_dispel_magic ); return; }

	if ( chance( ch, gsn_engage ) )
	{
	    CHAR_DATA *rch;
	    char target[MAX_INPUT_LENGTH];
	    int hp = 0;

	    target[0] = '\0';
	    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
	    {
		if ( rch->fighting != ch || ch->fighting == rch )
		    continue;

		if ( hp == 0 || rch->hit < hp )
		{
		    strcpy ( target, rch->name );
		    hp = rch->hit;
		}
	    }

	    if ( hp != 0 )
		{ do_engage( ch, target ); return; }
	}

	if ( chance( ch, gsn_circle ) && can_see( ch, victim )
	&&   victim->hit >= victim->max_hit / 10  )
	    { do_circle( ch, "" ); return; }

	if ( chance( ch, gsn_feed )
	&&   ( victim->hit >= victim->max_hit / 10 || !can_see( victim, ch ) ) )
	    { do_feed( ch, "" ); return; }

	for ( pos = 0; pos < DAM_MAX; pos++ )
	{
	    if ( victim->damage_mod[pos] > dam )
	    {
		dam = victim->damage_mod[pos];
		type = pos;
	    }
	}

	switch( type )
	{
	    default:
		break;

	    case DAM_ACID:
		if ( chance( ch, gsn_acid_breath ) )
		    { mob_cast( ch, victim, gsn_acid_breath ); return; }

		if ( chance( ch, gsn_acid_blast ) )
		    { mob_cast( ch, victim, gsn_acid_blast ); return; }

		if ( chance( ch, gsn_acid_storm ) )
		    { mob_cast( ch, victim, gsn_acid_storm ); return; }
		break;

	    case DAM_COLD:
		if ( chance( ch, gsn_frost_breath ) )
		    { mob_cast( ch, victim, gsn_frost_breath ); return; }

		if ( chance( ch, gsn_snow_storm ) )
		    { mob_cast( ch, victim, gsn_snow_storm ); return; }
		break;

	    case DAM_DISEASE:
		if ( chance( ch, gsn_plague ) )
		    { mob_cast( ch, victim, gsn_plague ); return; }

		if ( chance( ch, gsn_locust_swarm ) )
		    { mob_cast( ch, victim, gsn_locust_swarm ); return; }

		if ( chance( ch, gsn_swarm ) )
		    { mob_cast( ch, victim, gsn_swarm ); return; }
		break;

	    case DAM_ENERGY:
		if ( chance( ch, gsn_magic_missile ) )
		    { mob_cast( ch, victim, gsn_magic_missile ); return; }
		break;

	    case DAM_FIRE:
		if ( chance( ch, gsn_fire_breath ) )
		    { mob_cast( ch, victim, gsn_fire_breath ); return; }

		if ( chance( ch, gsn_fire_storm ) )
		    { mob_cast( ch, victim, gsn_fire_storm ); return; }

		if ( chance( ch, gsn_fireball ) )
		    { mob_cast( ch, victim, gsn_fireball ); return; }

		if ( chance( ch, gsn_flamestrike ) )
		    { mob_cast( ch, victim, gsn_flamestrike ); return; }

	    case DAM_HOLY:
		if ( !IS_EVIL( ch ) && !IS_GOOD( victim )
		&&   chance( ch, gsn_ray_of_truth ) )
		    { mob_cast( ch, victim, gsn_ray_of_truth ); return; }

		if ( IS_GOOD( ch ) && chance( ch, gsn_angelfire ) )
		    { mob_cast( ch, victim, gsn_angelfire ); return; }
		break;

	    case DAM_LIGHTNING:
		if ( chance( ch, gsn_lightning_breath ) )
		    { mob_cast( ch, victim, gsn_lightning_breath ); return; }

		if ( chance( ch, gsn_electrical_storm ) )
		    { mob_cast( ch, victim, gsn_electrical_storm ); return; }

		if ( chance( ch, gsn_lightning_bolt ) )
		    { mob_cast( ch, victim, gsn_lightning_bolt ); return; }
		break;

	    case DAM_NEGATIVE:
		if ( chance( ch, gsn_nightmare ) )
		    { mob_cast( ch, victim, gsn_nightmare ); return; }

		if ( chance( ch, gsn_energy_drain ) )
		    { mob_cast( ch, victim, gsn_energy_drain ); return; }

		if ( IS_EVIL( ch ) && chance( ch, gsn_demonfire ) )
		    { mob_cast( ch, victim, gsn_demonfire ); return; }
		break;

	    case DAM_POISON:
		if ( chance( ch, gsn_gas_breath ) )
		    { mob_cast( ch, victim, gsn_gas_breath ); return; }

		if ( chance( ch, gsn_poison ) )
		    { mob_cast( ch, victim, gsn_poison ); return; }
		break;

	    case DAM_WATER:
		if ( chance( ch, gsn_downpour ) )
		    { mob_cast( ch, victim, gsn_downpour ); return; }
		break;
	}

	if ( !IS_AFFECTED( victim, AFF_CURSE ) && chance( ch, gsn_curse ) )
	    { mob_cast( ch, victim, gsn_curse ); return; }

	if ( !IS_AFFECTED( victim, AFF_SLOW ) && chance( ch, gsn_slow ) )
	    { mob_cast( ch, victim, gsn_slow ); return; }

	if ( !IS_AFFECTED( victim, AFF_WEAKEN ) && chance( ch, gsn_weaken ) )
	    { mob_cast( ch, victim, gsn_weaken ); return; }

	if ( chance( ch, gsn_berserk ) && !is_affected( ch, gsn_berserk ) )
	    { do_berserk( ch, "" ); return; }

	if ( chance( ch, gsn_bash ) && can_see( victim, ch ) )
	    { do_bash( ch, "" ); return; }

	if ( get_eq_char( victim, WEAR_WIELD ) && chance( ch, gsn_disarm ) )
	    { do_disarm( ch, "" ); return; }

	if ( IS_AFFECTED( victim, AFF_FLYING ) && chance( ch, gsn_tackle ) )
	    { do_tackle( ch, "" ); return; }

	if ( chance( ch, gsn_kick ) )
	    { do_kick( ch, "" ); return; }

	if ( !IS_AFFECTED( victim, AFF_FLYING ) && chance( ch, gsn_trip ) )
	    { do_trip( ch, "" ); return; }
    }

    else if ( ch->position == POS_STANDING )
    {
	OBJ_DATA *weapon;

	if ( !IS_SHIELDED( ch, SHD_DIVINITY )
	&&   !IS_SHIELDED( ch, SHD_SANCTUARY ) )
	{
	    if ( chance( ch, gsn_divinity ) )
		{ mob_cast( ch, ch, gsn_divinity ); return; }

	    if ( chance( ch, gsn_sanctuary ) )
		{ mob_cast( ch, ch, gsn_sanctuary ); return; }
	}

	if ( !IS_SHIELDED( ch, SHD_PROTECT_GOOD )
	&&   !IS_SHIELDED( ch, SHD_PROTECT_EVIL )
	&&   !IS_SHIELDED( ch, SHD_PROTECT_NEUTRAL ) )
	{
	    if ( chance( ch, gsn_protect_good ) )
		{ mob_cast( ch, ch, gsn_protect_good ); return; }

	    if ( chance( ch, gsn_protect_neutral ) )
		{ mob_cast( ch, ch, gsn_protect_neutral ); return; }

	    if ( chance( ch, gsn_protect_evil ) )
		{ mob_cast( ch, ch, gsn_protect_evil ); return; }
	}

	if ( ch->hit < ch->max_hit )
	{
	    if ( chance( ch, gsn_divine_heal ) )
		{ mob_cast( ch, ch, gsn_divine_heal ); return; }

	    if ( chance( ch, gsn_heal ) )
		{ mob_cast( ch, ch, gsn_heal ); return; }

	    if ( chance( ch, gsn_cure_critical ) )
		{ mob_cast( ch, ch, gsn_cure_critical ); return; }

	    if ( chance( ch, gsn_cure_serious ) )
		{ mob_cast( ch, ch, gsn_cure_serious ); return; }

	    if ( chance( ch, gsn_cure_light ) )
		{ mob_cast( ch, ch, gsn_cure_light ); return; }
	}

	if ( IS_AFFECTED( ch, AFF_BLIND ) )
	{
	    if ( chance( ch, gsn_rub ) && is_affected( ch, gsn_dirt ) )
		{ do_rub( ch, "" ); return; }

	    if ( chance( ch, gsn_cure_blind )
	    &&   is_affected( ch, gsn_blindness ) )
		{ mob_cast( ch, ch, gsn_cure_blind ); return; }
	}

	if ( !IS_AFFECTED( ch, AFF_HASTE ) && chance( ch, gsn_haste ) )
	    { mob_cast( ch, ch, gsn_haste ); return; };

	if ( IS_AFFECTED( ch, AFF_CURSE ) && chance( ch, gsn_remove_curse ) )
	    { mob_cast( ch, ch, gsn_remove_curse ); return; }

	if ( !IS_SHIELDED( ch, SHD_ACID ) && chance( ch, gsn_acidshield ) )
	    { mob_cast( ch, ch, gsn_acidshield ); return; }

	if ( !IS_SHIELDED( ch, SHD_DIVINE_AURA )
	&&   chance( ch, gsn_divine_aura ) )
	    { mob_cast( ch, ch, gsn_divine_aura ); return; }

	if ( !IS_SHIELDED( ch, SHD_FIRE ) && chance( ch, gsn_fireshield ) )
	    { mob_cast( ch, ch, gsn_fireshield ); return; }

	if ( !IS_SHIELDED( ch, SHD_ICE ) && chance( ch, gsn_iceshield ) )
	    { mob_cast( ch, ch, gsn_iceshield ); return; }

	if ( !IS_SHIELDED( ch, SHD_ROCK ) && chance( ch, gsn_rockshield ) )
	    { mob_cast( ch, ch, gsn_rockshield ); return; }

	if ( !IS_SHIELDED( ch, SHD_SHOCK ) && chance( ch, gsn_shockshield ) )
	    { mob_cast( ch, ch, gsn_shockshield ); return; }

	if ( !IS_SHIELDED( ch, SHD_SHRAPNEL )
	&&   chance( ch, gsn_shrapnelshield ) )
	    { mob_cast( ch, ch, gsn_shrapnelshield ); return; }

	if ( !IS_SHIELDED( ch, SHD_THORN ) && chance( ch, gsn_thornshield ) )
	    { mob_cast( ch, ch, gsn_thornshield ); return; }

	if ( !IS_SHIELDED( ch, SHD_VAMPIRIC )
	&&   chance( ch, gsn_vampiricshield ) )
	    { mob_cast( ch, ch, gsn_vampiricshield ); return; }

	if ( !IS_SHIELDED( ch, SHD_WATER ) && chance( ch, gsn_watershield ) )
	    { mob_cast( ch, ch, gsn_watershield ); return; }

	if ( !is_affected( ch, gsn_bless )
	&&   !is_affected( ch, gsn_divine_blessing )
	&&   !is_affected( ch, gsn_infernal_offer ) )
	{
	    if ( IS_GOOD( ch ) && chance( ch, gsn_divine_blessing ) )
		{ mob_cast( ch, ch, gsn_divine_blessing ); return; }

	    else if ( IS_EVIL( ch ) && chance( ch, gsn_infernal_offer ) )
		{ mob_cast( ch, ch, gsn_infernal_offer ); return; }

	    if ( chance( ch, gsn_bless ) )
		{ mob_cast( ch, ch, gsn_bless ); return; }
	}

	if ( IS_AFFECTED( ch, AFF_PLAGUE ) && chance( ch, gsn_cure_disease ) )
	    { mob_cast( ch, ch, gsn_cure_disease ); return; }

	if ( IS_AFFECTED( ch, AFF_POISON ) && chance( ch, gsn_cure_poison ) )
	    { mob_cast( ch, ch, gsn_cure_poison ); return; }

	if ( !IS_AFFECTED( ch, AFF_DARK_VISION )
	&&   chance( ch, gsn_darkvision ) )
	    { mob_cast( ch, ch, gsn_darkvision ); return; }

	if ( !IS_AFFECTED( ch, AFF_DETECT_INVIS )
	&&   chance( ch, gsn_detect_invis ) )
	    { mob_cast( ch, ch, gsn_detect_invis ); return; }

	if ( !IS_AFFECTED( ch, AFF_DETECT_HIDDEN )
	&&   chance( ch, gsn_detect_hidden ) )
	    { mob_cast( ch, ch, gsn_detect_hidden ); return; }

	if ( !IS_AFFECTED( ch, AFF_DETECT_GOOD )
	&&   chance( ch, gsn_detect_good ) )
	    { mob_cast( ch, ch, gsn_detect_good ); return; }

	if ( !IS_AFFECTED( ch, AFF_DETECT_EVIL )
	&&   chance( ch, gsn_detect_evil ) )
	    { mob_cast( ch, ch, gsn_detect_evil ); return; }

	if ( !IS_AFFECTED( ch, AFF_DETECT_MAGIC )
	&&   chance( ch, gsn_detect_magic ) )
	    { mob_cast( ch, ch, gsn_detect_magic ); return; }

	if ( chance( ch, gsn_constitution )
	&&   !is_affected( ch, gsn_constitution ) )
	    { mob_cast( ch, ch, gsn_constitution ); return; }

	if ( chance( ch, gsn_intellect ) && !is_affected( ch, gsn_intellect ) )
	    { mob_cast( ch, ch, gsn_intellect ); return; }

	if ( chance( ch, gsn_wisdom ) && !is_affected( ch, gsn_wisdom ) )
	    { mob_cast( ch, ch, gsn_wisdom ); return; }

	if ( chance( ch, gsn_armor ) && !is_affected( ch, gsn_armor ) )
	    { mob_cast( ch, ch, gsn_armor ); return; }

	if ( chance( ch, gsn_barkskin ) && !is_affected( ch, gsn_barkskin ) )
	    { mob_cast( ch, ch, gsn_barkskin ); return; }

	if ( chance( ch, gsn_shield ) && !is_affected( ch, gsn_shield ) )
	    { mob_cast( ch, ch, gsn_shield ); return; }

	if ( chance( ch, gsn_steel_skin )
	&&   !is_affected( ch, gsn_steel_skin ) )
	    { mob_cast( ch, ch, gsn_steel_skin ); return; }

	if ( chance( ch, gsn_stone_skin )
	&&   !is_affected( ch, gsn_stone_skin ) )
	    { mob_cast( ch, ch, gsn_stone_skin ); return; }

	if ( chance( ch, gsn_frenzy ) && !is_affected( ch, gsn_frenzy ) )
	    { mob_cast( ch, ch, gsn_frenzy ); return; }

	if ( !IS_AFFECTED( ch, AFF_FLYING ) && chance( ch, gsn_fly ) )
	    { mob_cast( ch, ch, gsn_fly ); return; }

	if ( !IS_AFFECTED( ch, AFF_FARSIGHT ) && chance( ch, gsn_farsight ) )
	    { mob_cast( ch, ch, gsn_farsight ); return; }

	if ( !IS_AFFECTED( ch, AFF_INFRARED ) && chance( ch, gsn_infravision ) )
	    { mob_cast( ch, ch, gsn_infravision ); return; }

	if ( !IS_AFFECTED( ch, AFF_GIANT_STRENGTH )
	&&   chance( ch, gsn_giant_strength ) )
	    { mob_cast( ch, ch, gsn_giant_strength ); return; }

	if ( !IS_AFFECTED( ch, AFF_PASS_DOOR ) && chance( ch, gsn_pass_door ) )
	    { mob_cast( ch, ch, gsn_pass_door ); return; }

	if ( !IS_AFFECTED( ch, AFF_REGENERATION )
	&&   chance( ch, gsn_regeneration ) )
	    { mob_cast( ch, ch, gsn_regeneration ); return; }

	if ( ch->size < SIZE_GIANT
	&&   chance( ch, gsn_growth ) && !is_affected( ch, gsn_growth ) )
	    { mob_cast( ch, ch, gsn_growth ); return; }

	if ( !IS_SHIELDED( ch, SHD_INVISIBLE ) && chance( ch, gsn_invis ) )
	    { mob_cast( ch, ch, gsn_invis ); return; }

	if ( !IS_SHIELDED( ch, SHD_MANA ) && chance( ch, gsn_mana_shield ) )
	    { mob_cast( ch, ch, gsn_mana_shield ); return; }

	if ( ch->move < ch->max_move && chance( ch, gsn_refresh ) )
	    { mob_cast( ch, ch, gsn_refresh ); return; }

	if ( ( weapon = get_eq_char( ch, WEAR_WIELD ) ) != NULL
	&&   number_percent( ) < 50 )
	{
	    if ( mob_check_weapon( ch, weapon ) )
		return;
	}

	if ( ( weapon = get_eq_char( ch, WEAR_SECONDARY ) ) != NULL
	&&   number_percent( ) < 50 )
	{
	    if ( mob_check_weapon( ch, weapon ) )
		return;
	}
    }
}

void mobile_update( void )
{
    CHAR_DATA *ch, *ch_next;
    EXIT_DATA *pexit;
    int door;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if ( !IS_NPC( ch )
	||   ch->in_room == NULL
	||   IS_AFFECTED( ch, AFF_CHARM )
	||   ( IS_SET( ch->pIndexData->area->area_flags, AREA_UNLINKED ) && ch->pIndexData->area->nplayer == 0 ) )
	    continue;

	if ( ch->wait > 0 )
	{
	    ch->wait = UMIN( 0, ch->wait - PULSE_MOBILE );
	    continue;
	}

	if ( (ch->position == POS_SLEEPING
	&&    ch->pIndexData->default_pos != POS_SLEEPING)
	||   (ch->position == POS_RESTING
	&&    ch->pIndexData->default_pos != POS_RESTING) )
	    do_stand(ch,"");

	if ( IS_SET( ch->act, ACT_SMART_MOB ) )
	    check_smart_mob( ch );

//	else
	if ( ch->spec_fun != 0 )
            ( *ch->spec_fun ) ( ch );

	if ( ch->pIndexData->pShop != NULL )
	{
	    if ((ch->platinum * 100 + ch->gold) < ch->pIndexData->wealth)
	    {
		ch->platinum += ch->pIndexData->wealth * number_range(1,20)/5000000;
		ch->gold += ch->pIndexData->wealth * number_range(1,20)/50000;
	    }
	}

	if ( ch->position == ch->pIndexData->default_pos )
	{
	    if ( HAS_TRIGGER_MOB( ch, TRIG_DELAY ) && ch->mprog_delay > 0 )
	    {
		if ( --ch->mprog_delay <= 0 )
		    p_percent_trigger( ch, NULL, NULL, NULL, NULL, NULL, TRIG_DELAY );
	    }

	    if ( HAS_TRIGGER_MOB( ch, TRIG_RANDOM) )
	    {
		if ( p_percent_trigger( ch, NULL, NULL, NULL, NULL, NULL, TRIG_RANDOM ) )
		    continue;
	    }
	}
	 
	if ( ch->position != POS_STANDING )
	    continue;

	if ( IS_SET(ch->act, ACT_SCAVENGER)
	&&   ch->in_room->contents != NULL
	&&   number_bits( 6 ) == 0 )
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int max;

	    max         = 1;
	    obj_best    = 0;
	    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	    {
		if ( CAN_WEAR(obj, ITEM_TAKE)
                && obj->item_type != ITEM_TREASURE
		&& can_loot(ch, obj)
		&& can_see_obj(ch,obj)
		&& ch->carry_number + get_obj_number(obj) <= can_carry_n(ch)
		&& get_carry_weight(ch) + get_obj_weight(obj) <= can_carry_w(ch)
		&& obj->cost > max
		&& obj->cost > 0)
		{
		    obj_best    = obj;
		    max         = obj->cost;
		}
	    }

	    if ( obj_best )
		send_to_char( get_obj(ch,obj_best,NULL,FALSE), ch );
	}

	/* Wander */
	if ( !IS_SET(ch->act, ACT_SENTINEL) 
	&& number_bits(3) == 0
	&& ( door = number_bits( 5 ) ) <= 5
	&& ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
	&& ( !IS_SET(ch->act, ACT_STAY_AREA)
	||   pexit->u1.to_room->area == ch->in_room->area ) )
	{
	    move_char( ch, door, FALSE, FALSE );
	}
    }

    return;
}

void weather_update( void )
{
    char buf[MAX_STRING_LENGTH], file[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int diff;

    // Sometimes a meteorite
    if ( number_range(0,100) <= 2  ) {
        bool struck=(number_percent( ) < 5) ? TRUE : FALSE;
	for ( d = descriptor_list; d != NULL; d = d->next )
         if ( d->character
           && d->character->in_room
           && !IS_SET(d->character->in_room->room_flags,ROOM_INDOORS) ) {
         if ( d->character ) {
          display_ascii( struck == TRUE ? "meteor_strike" : "meteorite", d->character );
          if ( struck ) send_to_char( "{!A {#meteorite{! strikes the ground nearby!{0\n\r", d->character );
          else send_to_char( "{!A {#meteorite{! burns up in the sky above you!{0\n\r", d->character );
         }
        }
        if ( struck ) {
          // Go around to 1..N random rooms and disperse the contents randomly,
          // announcing their redistribution appropriately.
          int num=number_range(0,200);
          int i=0;
          for ( ; i<num; i++ ) {
           ROOM_INDEX_DATA *pRoom=random_room();
           if ( !pRoom
            || IS_SET(pRoom->room_flags,ROOM_SAVE_CONTENTS)
            || IS_SET(pRoom->room_flags,ROOM_SAFE)
            || IS_SET(pRoom->room_flags,ROOM_NO_MOB)
            || IS_SET(pRoom->room_flags,ROOM_INDOORS)
            || IS_SET(pRoom->room_flags,ROOM_PET_SHOP)
            || IS_SET(pRoom->room_flags,ROOM_WAR)
            || IS_SET(pRoom->room_flags,ROOM_ARENA)
            || IS_SET(pRoom->room_flags,ROOM_SOLITARY)
            || IS_SET(pRoom->room_flags,ROOM_PRIVATE)
            || IS_SET(pRoom->room_flags,ROOM_WAR)
            || IS_SET(pRoom->room_flags,ROOM_ARENA)
            || IS_SET(pRoom->room_flags,ROOM_NEWBIES_ONLY)
            || IS_SET(pRoom->room_flags,ROOM_IMP_ONLY)
            || IS_SET(pRoom->room_flags,ROOM_HEROES_ONLY)
            || IS_SET(pRoom->room_flags,ROOM_GODS_ONLY)
           ) continue;
           if ( pRoom->people ) {
            CHAR_DATA *p,*pn;
            for ( p=pRoom->people; p; p=pn ) { pn=p->next_in_room;
             if ( IS_IMMORTAL(p) ) continue;
             if ( number_range(0,10) <= 3 ) continue;
             ROOM_INDEX_DATA *pTo=random_room();
             if ( pTo == pRoom ) continue;
             if ( !pTo
            || IS_SET(pTo->room_flags,ROOM_SAVE_CONTENTS)
            || IS_SET(pTo->room_flags,ROOM_SAFE)
            || IS_SET(pTo->room_flags,ROOM_NO_MOB)
            || IS_SET(pTo->room_flags,ROOM_INDOORS)
            || IS_SET(pTo->room_flags,ROOM_PET_SHOP)
            || IS_SET(pTo->room_flags,ROOM_WAR)
            || IS_SET(pTo->room_flags,ROOM_ARENA)
            || IS_SET(pTo->room_flags,ROOM_SOLITARY)
            || IS_SET(pTo->room_flags,ROOM_PRIVATE)
            || IS_SET(pTo->room_flags,ROOM_WAR)
            || IS_SET(pTo->room_flags,ROOM_ARENA)
            || IS_SET(pTo->room_flags,ROOM_NEWBIES_ONLY)
            || IS_SET(pTo->room_flags,ROOM_IMP_ONLY)
            || IS_SET(pTo->room_flags,ROOM_HEROES_ONLY)
            || IS_SET(pTo->room_flags,ROOM_GODS_ONLY)
             ) continue;
             act( "$n is cast away from the epicenter of the striking meteor!", p, NULL, NULL, TO_ROOM, POS_RESTING );
             stop_fighting(p,TRUE);
             char_from_room(p);
             char_to_room(p,pTo);
             act( "$n lands hard, cast from the epicenter of the striking meteor!", p, NULL, NULL, TO_ROOM, POS_RESTING );
             send_to_char( "You are cast aside as the meteor strikes!\n\r", p );
             do_look( p, "" );
             damage( p, p, number_range(0,p->hit/4),TYPE_UNDEFINED,DAM_AIR,FALSE,FALSE,NULL);
             // Remove and drop their weapons on the ground, damage their items
            }
           }
           // Toss objects randomly about the world
           if ( pRoom->contents ) {
            OBJ_DATA *o=pRoom->contents;
            OBJ_DATA *on;
            for ( ; o; o=on ) { on=o->next_content;
            ROOM_INDEX_DATA *pTo=random_room();
             if ( !pTo
            || IS_SET(pTo->room_flags,ROOM_SAVE_CONTENTS)
            || IS_SET(pTo->room_flags,ROOM_SAFE)
            || IS_SET(pTo->room_flags,ROOM_NO_MOB)
            || IS_SET(pTo->room_flags,ROOM_INDOORS)
            || IS_SET(pTo->room_flags,ROOM_PET_SHOP)
            || IS_SET(pTo->room_flags,ROOM_WAR)
            || IS_SET(pTo->room_flags,ROOM_ARENA)
            || IS_SET(pTo->room_flags,ROOM_SOLITARY)
            || IS_SET(pTo->room_flags,ROOM_PRIVATE)
            || IS_SET(pTo->room_flags,ROOM_WAR)
            || IS_SET(pTo->room_flags,ROOM_ARENA)
            || IS_SET(pTo->room_flags,ROOM_NEWBIES_ONLY)
            || IS_SET(pTo->room_flags,ROOM_IMP_ONLY)
            || IS_SET(pTo->room_flags,ROOM_HEROES_ONLY)
            || IS_SET(pTo->room_flags,ROOM_GODS_ONLY)
             ) continue;
             if ( number_range(0,100) > 25 ) {
              char buf[MSL];
              sprintf( buf, "%s is blasted high into the air and lands somewhere else.\n\r", capitalize(o->pIndexData->short_descr) );
              CHAR_DATA *rch;
              for ( rch=pRoom->people; rch; rch=rch->next_in_room ) send_to_char( buf, rch );
              obj_from_room(o);
              sprintf( buf, "%s falls from the sky!\n\r", capitalize(o->pIndexData->short_descr) );
              for ( rch=pTo->people; rch; rch=rch->next_in_room ) send_to_char( buf, rch );
              obj_to_room(o,pTo);
             }
            }
           }
          }
        }
    }

    buf[0] = '\0';
    file[0] = '\0';

    switch ( ++time_info.hour )
    {
    case  5:
	weather_info.sunlight = SUN_LIGHT;
	strcat( buf, "The day has begun.\n\r" );
	break;

    case  6:
	weather_info.sunlight = SUN_RISE;
	strcat( buf, "The sun rises in the east.\n\r" );
	break;

    case 19:
	weather_info.sunlight = SUN_SET;
	strcat( buf, "The sun slowly disappears in the west.\n\r" );
	break;

    case 20:
	weather_info.sunlight = SUN_DARK;
	strcat( buf, "The night has begun.\n\r" );
	break;

    case 24:
	time_info.hour = 0;
	time_info.day++;
	sprintf( file, "clock.wav" );
	break;
    }

    if ( time_info.day   >= 35 )
    {
	time_info.day = 0;
	time_info.month++;
    }

    if ( time_info.month >= 17 )
    {
	time_info.month = 0;
	time_info.year++;
    }

    /*
     * Weather change.
     */
    if ( time_info.month >= 9 && time_info.month <= 16 )
	diff = weather_info.mmhg >  985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change    = UMAX(weather_info.change, -15);
    weather_info.change    = UMIN(weather_info.change,  15);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
    weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

    switch ( weather_info.sky )
    {
    default:
	bug( "Weather_update: bad sky %d.", weather_info.sky );
	weather_info.sky = SKY_CLOUDLESS;
	break;

    case SKY_CLOUDLESS:
	if ( weather_info.mmhg <  990
	|| ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The sky is getting cloudy.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_CLOUDY:
	if ( weather_info.mmhg <  970
	|| ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
	{
            if ( time_info.month <= 2 )
            strcat( buf, "It starts to snow.\n\r" );
            else if ( time_info.month >= 15 ) strcat( buf, "It starts raining blood.\n\r" );
                 else
	         strcat( buf, "It starts to rain.\n\r" );
	    weather_info.sky = SKY_RAINING;
	}

	if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "The clouds disappear.\n\r" );
	    weather_info.sky = SKY_CLOUDLESS;
	}
	break;

    case SKY_RAINING:
	if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "Lightning flashes in the sky.\n\r" );
	    weather_info.sky = SKY_LIGHTNING;
	    switch( number_range( 1, 2 ) )
	    {
		case 1:
		    sprintf( file, "lightning1.wav" );
		    break;
		default:
		    sprintf( file, "lightning2.wav" );
		    break;
	    }
	}

	if ( weather_info.mmhg > 1030
	|| ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The rain stopped.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_LIGHTNING:
	if ( weather_info.mmhg > 1010
	|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The lightning has stopped.\n\r" );
	    weather_info.sky = SKY_RAINING;
	    break;
	}
	break;
    }

    if ( buf[0] != '\0' )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   IS_OUTSIDE(d->character)
	    &&   IS_AWAKE(d->character) )
	    {
                switch ( weather_info.sky ) {
                 default: break;
                  case SKY_CLOUDLESS: display_ascii( "cloudless", d->character ); break;
                  case SKY_CLOUDY: display_ascii( "cloudy", d->character ); break;
                  case SKY_RAINING: if ( time_info.month <= 2 ) display_ascii("snowing", d->character );
                                    else if ( time_info.month >= 15 ) display_ascii( "blood_rain", d->character );
                                         else display_ascii( "raining", d->character );
                   break;
                  case SKY_LIGHTNING: display_ascii( "lightning", d->character ); break;
                }
                switch ( time_info.hour ) {
                 default: break;
                  case 19: display_ascii( "sunset", d->character ); break;
                  case 20: display_ascii( "evening", d->character ); break;
                  case 6: display_ascii( "sunrise", d->character ); break;
                  case 12: display_ascii( "noon", d->character ); break;
                  case 0: case 24: display_ascii( "midnight", d->character ); break;
                }
		send_to_char( buf, d->character );
		if ( file[0] != '\0' )
		    send_sound_char( d->character, 100, 1, 100, "weather",
			file, SOUND_NOWEATHER );
	    }
	}
    }

    return;
}

void player_update( void )
{
    CHAR_DATA *ch, *ch_next;
    int dream;

    for ( ch = player_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->pcdata->next_player;

	if ( ch->pcdata->dtimer > 0 && --ch->pcdata->dtimer == 0 )
	    send_to_char("You are no longer considered {G[ {BDeceased {G]{x.\n\r",ch);

	if (ch->position == POS_SLEEPING)
	{
	    dream = number_range(0,550);

	    if (dream <= 150)
	    {
		send_to_char("zzzzzzzzzzzzzzzzzzzzzzzzz.\n\r",ch);
	    }
	    else if (dream <= 155)
	    {
		send_to_char("Frantically looking around, your arms hit the bars of a cold iron cage..\n\r",ch);
		send_to_char("Panicing you rattle the bars hoping to escape.. finding the door\n\r",ch);
		send_to_char("unlocked you quickly leap from the cage.. when suddenly a seemingly\n\r",ch);
		send_to_char("giant roar echoes in your ears.. Looking above you see a common yet\n\r",ch);
		send_to_char("titanic tabby cat licking its chops waiting for a delicious meal. Before\n\r",ch);
		send_to_char("you can react it pounces upon you...\n\r",ch);
	    }
	    else if (dream <= 160)
	    {
		send_to_char("Sitting before a vast table, you and a hoard of guests begin to eat..\n\r",ch);
		send_to_char("Course after fine course flow across the table... and toast after toast\n\r",ch);
		send_to_char("of fine wine flows through out the hall!  Minstrels begin to sing songs\n\r",ch);
		send_to_char("of valient bravery and lost love.. as people get up from their hosts   \n\r",ch);
		send_to_char("table to dance...\n\r",ch);
	    }
	    else if (dream <= 165)
	    {
		send_to_char("Moving your hands through the air you summon powerful magics!\n\r",ch);
		send_to_char("Creating and destoying worlds, setting ablaze all of time!\n\r",ch);
		send_to_char("Cackling with your god-like power you crush whole races between\n\r",ch);
		send_to_char("your fingertips.. and create new ones with a single thought\n\r",ch);
		send_to_char("to worship your awe inspiring power..\n\r",ch);
	    }
	    else if (dream <= 170)
	    {
		send_to_char("At the head of a vast and powerful army you lead your troops toward\n\r",ch);
		send_to_char("a seculded village.. Unspecting and innocent the towns people see your\n\r",ch);
		send_to_char("approach and begin to flee in terror! Giving the order your archers\n\r",ch);
		send_to_char("and pikemen rush toward them! Sending flaming arrow into the sky, fires\n\r",ch);
		send_to_char("begin to flame around the defenseless village.. slaughter and blood fill\n\r",ch);
		send_to_char("the scene.. as innocent lives scream for mercy..\n\r",ch);
	    }
	    else if (dream <= 175)
	    {
		send_to_char("Soaring through the skys you fly, diving and careening through the\n\r",ch);
		send_to_char("gusty wind.. Looking below you, the ground slowly dissapears, leaving\n\r",ch);
		send_to_char("you flying through empty and endless sky blue sky.. As the sun sets\n\r",ch);
		send_to_char("sending golden rays streaming acoss the air you glance at back and find\n\r",ch);
		send_to_char("you have no wings! Gasping in terror you begin to falllllll.....\n\r",ch);
	    }
	    else if (dream <= 180)
	    {
		send_to_char("Feeling the cooling surf wash over your feet and the wet sand creep\n\r",ch);
		send_to_char("its way between your toes, you smile briefly.  Looking across the\n\r",ch);
		send_to_char("horizon you cannot tell where crystal blue ocean ends and sky begins.\n\r",ch);
		send_to_char("Above you the rustling of palm trees are your only compannion on this\n\r",ch);
		send_to_char("lone beach...\n\r",ch);
	    }
	    else if (dream <= 185)
	    {
		send_to_char("A pair of gleaming red eyes, shine from the pitch black darkness,\n\r",ch);
		send_to_char("hearing the creatures breath you freeze.  Closer and closer it\n\r",ch);
		send_to_char("creeps, scraping its claws against the dungeons stone walls.\n\r",ch);
		send_to_char("Panicing you begin to run through the darkness blindly.  Out of\n\r",ch);
		send_to_char("breath you stop and listen.. all is silent in the pitch blackness...\n\r",ch);
		send_to_char("Suddenly you feel somethings icy claws bore into your flesh!\n\r",ch);
	    }
	    else if (dream <= 190)
	    {
		send_to_char("Strange colors swirl around you, blinding you.\n\r",ch); 
		act( "$n mumbles in $s sleep.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    }
	    else if ( dream <= 195)
	    {
		send_to_char("Horses thunder past you, bearing naked riders to \n\r",ch);
		send_to_char("who knows where. The sound of horns echo in the distance.\n\r",ch);
		act( "$n tosses restlessly in $s sleep.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    }
	    else if (dream <= 200)
	    {
		send_to_char("You awake in a soft, warm bed. As you bask in warm glow of firelight you\n\r",ch);
		send_to_char("wonder how long it's been since last you've had such comfort.  The \n\r",ch);
		send_to_char("firelight fades and turns from red-orange to blue to white to a \n\r",ch);
		send_to_char("spectrum. The colors surround you. You hear soft music, filling your \n\r",ch);
		send_to_char("ears and your soul. You are swept away on the bittersweet and melancholy \n\r",ch);
		send_to_char("chords, your spirit lifted and your heart twisted.. You hear faint \n\r",ch);
		send_to_char("laughter, a woman's voice, a child whimpering softly.. You begin to cry \n\r",ch); 
		send_to_char("suddenly and are not sure why.\n\r",ch);
		act( "$n sobs quietly in $s sleep for a moment then is still once again.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    }
	    else if (dream <= 205)
	    {
		send_to_char("You stand high on a mountain top, surrounded by azure skies and whitest \n\r",ch);
		send_to_char("clouds There is a blinding flash of light, and the clouds part as does \n\r",ch);
		send_to_char("the very sky as if it were a fabric that could be ripped.  From this \n\r",ch);
		send_to_char("opening fly hundreds of winged beings, majestic and terrible in \n\r",ch);
		send_to_char("their beauty. They seem to emanated light and heat.. their bodies \n\r",ch);
		send_to_char("are so bright that it hurts your eyes to look at them. You avert your \n\r",ch);
		send_to_char("eyes, and when you dare to look up the beings are gone, the sky as it \n\r",ch);
		send_to_char("had been..\n\r",ch);
		send_to_char("You notice a single golden feather at your feet and pick it up \n\r",ch);
		send_to_char("thoughtfully.\n\r",ch);
		act( "$n smiles slightly in $s sleep.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    }
	    else if (dream <= 210)
	    {
		send_to_char("You push through a crowd of people, repulsed by the smell of bodies, of \n\r",ch);
		send_to_char("heat and of greed. A thousand booths stretch out along each side of the \n\r",ch);
		send_to_char("street each selling a differnt ware or service. A particular booth \n\r",ch);
		send_to_char("catches your attention, the one in which you catch a glimpse of white \n\r",ch);
		send_to_char("and gold.. As you get closer you see a caged unicorn, pure white with a \n\r",ch);
		send_to_char("gold horn. Next to the grubby man selling tickets to touch its horn it looks all the\n\r",ch);
		send_to_char("more majestic. It turns sad, pleading doelike eyes on you, and you can\n\r",ch);
		send_to_char("hear its thoughts, its anguish, its utter lonliness and longing to be \n\r",ch); 
		send_to_char("free..\n\r",ch);
	    }   
	    else if (dream <= 215)
	    {
		send_to_char("You move but hardly notice the feeling of the floor....\n\r",ch);
		send_to_char("You grab a 20 by 30 foot canvas and vainly attempt to set it up....\n\r",ch);
		send_to_char("Three or four passers by stop to inspect your work....\n\r",ch);
		send_to_char("Everything you needed is now available but you feel as if soon it \n\r",ch);
		send_to_char("will be gone... The walls close in and a glowing presense alerts you.\n\r",ch);
		send_to_char("You dig at the walls with your fingertips, trying to get through\n\r",ch);
		send_to_char("The cinder block comes loose! You are free! You squeeze through the hole\n\r",ch);
		send_to_char("And in the next room...it is exactly the same....\n\r",ch);
	    }
	    else if (dream <= 220)
	    {
		send_to_char("You are high above the earth...plummeting towards the ground.\n\r",ch);
		send_to_char("You realize its a dream and think, OK I'll wake up in a minute.\n\r",ch);
		send_to_char("As the ground draws near you begin to scream in terror of death.\n\r",ch);
		send_to_char("You hit the ground with a sickening crunch and lay there until you \n\r",ch);
		send_to_char("bleed to death.\n\r",ch);
	    }
	    else if (dream <= 225)
	    {
		send_to_char("the shadows around you swirl and gire into the form of a dark\n\r",ch);
		send_to_char("steed...it whispers something in a strange tongue and melts\n\r",ch);
		send_to_char("into the foreground.....\n\r",ch);
	    }
	    else if (dream <= 230)
	    {
		send_to_char("Lights streak past you leaving long trails.\n\r",ch);
		send_to_char("Red, blue, yellow, white trails.\n\r",ch);
		send_to_char("A coldness unlike any you have ever felt before creeps through your \n\r",ch);
		send_to_char("bones. The only sound you hear is the slow thud of your heart.\n\r",ch);
	    }
	    else if (dream <= 235)
	    {
		send_to_char("You find yourself standing at your old bus stop *sigh*\n\r",ch);
		send_to_char("a dreary morning only 1/3 into the school year\n\r",ch);
		send_to_char("All of a sudden you are soaring high above the street!\n\r",ch);
		send_to_char("You swoop down and topple over a couple trash cans!\n\r",ch);
		send_to_char("All your fellow students turn green with envy\n\r",ch);
		send_to_char("You begin to doubt your abilities! LOOK OUT!!\n\r",ch);
		send_to_char("You are falling into a deep, black hole.....\n\r",ch);
	    }
	    else if (dream <= 240)
	    {
		send_to_char("Ahhhhh you are in perhaps THE most comfortable bed ever.\n\r",ch);
		send_to_char("The blankets arranged just so and very warm...\n\r",ch);
		send_to_char("Your dog cuddled closely to you.\n\r",ch);
		send_to_char("and your arm hangs over the edge of the bed...Ahhhhhh.\n\r",ch);
		send_to_char("then a hand comes from under the bed and GRABS your hand!\n\r",ch);
		send_to_char("You wake up in a cold sweat!\n\r",ch);
		send_to_char("Something STILL has your hand!\n\r",ch);
		send_to_char("You wake up in a cold sweat...whew your hand is ok.\n\r",ch);
	    }
	    else if (dream == 245)
	    {
		send_to_char("A warm incredible feeling of happiness washes over you.\n\r",ch);
		send_to_char("It feels like you are floating on a bed of air and a \n\r",ch);
		send_to_char("thousand butterflies are fluttering over you gently cooling\n\r",ch);
		send_to_char("you with the beating of their wings.  A far off harp\n\r",ch);
		send_to_char("peacefully fills the air with soul-stirring music.  As you\n\r",ch);
		send_to_char("open your eyes, you realize the sweet aroma filling the air comes\n\r",ch);
		send_to_char("from the hundreds of flowers on and around you with still\n\r",ch);
		send_to_char("more floating down from the sky above.  You close your eyes\n\r",ch);
		send_to_char("hoping to retain these feelings forever.\n\r",ch);
		act( "The air is filled with sweetness as hundreds of flower petals float", ch, NULL, NULL, TO_ROOM,POS_RESTING);
		act( "down from the sky to cover $n as $e lies sleeping.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    }
	    else if (dream <= 250)
	    {
		send_to_char("Right, left, onward.\n\r",ch);
		send_to_char("The hall seems to stretch endlessly.\n\r",ch);
		send_to_char("Is this the door? No.\n\r",ch);
		send_to_char("Is this the door? No.\n\r",ch);
		send_to_char("Onward...there seems to be no way out.\n\r",ch);
		send_to_char("Ahead you see a figure crumbled on the ground.  As you turn him over\n\r",ch);
		send_to_char("he crumbles into dust.  All that remains is a large black opal\n\r",ch);
		send_to_char("strung on a gold chain.\n\r",ch);
	    }
	    else if (dream <= 255)
	    {
		send_to_char("You hear a slow, pulsing beat, rushing liquid.. A bird cries out in the \n\r",ch);
		send_to_char("distance. You crane your neck to see what is around you, but \n\r",ch);
		send_to_char("everything remains hazy.\n\r",ch);
		send_to_char("You try to stand up, to walk towards the sounds, but you cannot move\n\r",ch);
		send_to_char("You legs do not respond, your arms are pinned to your chest..\n\r",ch);
		send_to_char("You struggle and try to cry out, but no sound comes..\n\r",ch);
	    }
	    else if (dream <= 260)
	    {
		send_to_char("A pair of ginger-haired twins, pale, thin and beautifully androgynous\n\r",ch);
		send_to_char("stand before you, smiling cryptically. They look at you and say, in \n\r",ch);
		send_to_char("unison,  How nice of you to visit! We've been waiting for you...\n\r",ch);
		send_to_char("The taller one looks at his brother lovingly and says Oh but I have \n\r",ch);
		send_to_char("been hungry... isn't this just lovely? \n\r",ch);
		send_to_char("He opens his mouth in a wide grin and you realize.. he has fangs.\n\r",ch);
	    }
	    else if (dream <= 265)
	    {
		send_to_char("Breathing heavily, you duck behind a stone wall.\n\r",ch);
		send_to_char("You can hear them behind you, dozens of them.\n\r",ch);
		send_to_char("It had been a long and glorius battle.  One by one\n\r",ch);
		send_to_char("you comrades had fallen.  Now there was only you left and\n\r",ch);
		send_to_char("your quiver is empty.  Breathing deeply, you calm yourself.\n\r",ch);
		send_to_char("Sword left in the chest of a battle dragon, arrows expended,\n\r",ch);
		send_to_char("all you have left is the small charm that an old woman gave you.\n\r",ch); 
	    }
	    else if (dream <= 270)
	    {
		send_to_char("Tick Tick Tick Tick ...\n\r",ch);
		send_to_char("The sound of the clock echos though the house.\n\r",ch);
		send_to_char("Somewhere between sleep and wake you drift ...\n\r",ch);
		send_to_char("Something big is going to happen, you can sense it.\n\r",ch); 
		send_to_char("TICK TICK TICK TICK ...\n\r",ch);
		send_to_char("Somehow the clock seems louder ... more ominous ...\n\r",ch);
		send_to_char("You struggle to awake, your heart quickening.\n\r",ch); 
		send_to_char("TICK TICK TICK TICK ...\n\r",ch);
	    }
	    else if (dream <= 275)
	    {
		send_to_char("A flash of spinning silver\n\r",ch);
		send_to_char("trapped in a whirling sphere of dimpled gleaming light\n\r",ch);
		send_to_char("Friction a fiction, the body slides without feeling, presses\n\r",ch);
		send_to_char("without resistance\n\r",ch);
		send_to_char("splayed like a starfish scrabbling for purchase\n\r",ch);
		send_to_char("Inevitably funneled into a chrome trumpet of doomsounding thunder\n\r",ch);
		send_to_char("spun into a slick wet thread of bare bony wire\n\r",ch);
	    }
	    else if (dream <= 280)
	    {
		send_to_char("The horse and it's rider comes trampling out of the fog called\n\r",ch);
		send_to_char("sleep. As you peer at it's indistinguishable (sic) form, it's name\n\r",ch);
		send_to_char("escapes you but for a second.  The lance is pointed straight \n\r",ch);
		send_to_char("for your forehead.  It's thunderous hooves jolt your memory. \n\r",ch);
		send_to_char("HARK!  You remember the name if the horse and  rider,it's \n\r",ch);
		send_to_char("Knight-Mare. \n\r",ch);
	    }
	    else if (dream <= 285)
	    {
		send_to_char("Someone wakes you up.\n\r",ch);
		send_to_char("Someone's backstab *** ELIMINATES *** you!\n\r",ch);
		send_to_char("You have been killed!!!\n\r",ch);
		send_to_char("\n\r",ch);
		send_to_char("\n\r",ch);
		send_to_char("\n\r",ch);
		send_to_char("Oh, You're dreaming again.  *sigh*\n\r",ch);
	    }
	    else if (dream <= 290)
	    {
		send_to_char("The abomination points at you with a crocked and gnawed finger, \n\r",ch);
		send_to_char("whispering your name over and over.  Chanting your death wish \n\r",ch);
		send_to_char("like a mantra.  It glides towards where you stand frozen. Your \n\r",ch);
		send_to_char("mind begins to scream for your legs to run... to move... to do \n\r",ch);
		send_to_char("anything but buckle and fall.  You stare in horror, as it's maw \n\r",ch);
		send_to_char("open up and... \n\r",ch);
		send_to_char("\n\r",ch);
		send_to_char("That's when you realize with disgust your bedroll has somehow been soaked. \n\r",ch);
	    }
	    else if (dream <= 295)
	    {
		send_to_char("The cat grabs the dog by the nose and bonks him on the head. \n\r",ch);
		send_to_char("The dog, growling, pokes the mouse in it's eyes.  The mouse, \n\r",ch);
		send_to_char("with one stroke, slaps both the cat and the dog.  The cat gets \n\r",ch);
		send_to_char("mad, swings his fist and kicks the mouse in the stomach.  The \n\r",ch);
		send_to_char("dog, seeing the mouse being abused, bonks the cat\n\r",ch);
		send_to_char("\n\r",ch);
		send_to_char("Oh gawd, the 3 Stoogies meet Tom and Jerry.\n\r",ch);
	    }
	    else if (dream <= 300)
	    {
		send_to_char("You feel a tickling on your arm, you try to reach over and \n\r",ch);
		send_to_char("scratch it but you find yourself frozen.  You open your eyes to \n\r",ch);
		send_to_char("see a bright light shining at you. Shadows move in the \n\r",ch);
		send_to_char("background and you hear a throbbing sound\n\r",ch);
		send_to_char("A sting from your arm feels like a needle and you realize that \n\r",ch); 
		send_to_char("your very marrow is being drawn from your bones.\n\r",ch);
		send_to_char("As you are just about to pass out you hear alien voices buzzing \n\r",ch);
		send_to_char("quietly...\n\r",ch);
	    }
	    else if (dream <= 305)
	    {
		send_to_char("You're sitting at the table, eating your steak and potatoes, \n\r",ch);
		send_to_char("when the meat you're just about to devour begins to cough and \n\r",ch);
		send_to_char("spasm.  The potatoes start to roll around on your plate, and \n\r",ch);
		send_to_char("some even jump up and down, making a huge mess of your \n\r",ch);
		send_to_char("vegetables, and a loud ruckus to boot.  Hey buddy, says the \n\r",ch);
		send_to_char("slab of flesh you called dinner.  Why don't you pick on someone \n\r",ch);
		send_to_char("your OWN size? \n\r",ch);
	    }
	    else if (dream <= 310)
	    {
		send_to_char("You walk through a valley of daisys.. \n\r",ch);
		send_to_char("over a ways you see people from different races spread out sleeping \n\r",ch); 
		send_to_char("over the field.. \n\r",ch);
		send_to_char("In the distance you hear the faint sound of music.\n\r",ch);
		send_to_char("your eyes get heavy....\n\r",ch);
		send_to_char("you collapse to the ground and sleep....\n\r",ch);
	    }
	    else if (dream <= 315)
	    {
		send_to_char("Darkness surrounds you.. cold and silent.. feeling around you, you \n\r",ch);
		send_to_char("catch ahold of a candle, and magically it lights itself.. burning \n\r",ch);
		send_to_char("slowly sending melted wax down your hand.. Looking around you find \n\r",ch);
		send_to_char("you are surrounded..by wooden walls?.. you begin to recognize \n\r",ch);
		send_to_char("the shape.. of a cofin.. panicing you begin to claw at the lid, \n\r",ch);
		send_to_char("trying in vain to push it open.. but it is impossible.. as you \n\r",ch);
		send_to_char("struggle alittle dirt tumbles in from a crack in the coffin \n\r",ch);
		send_to_char("wall.. and as the candle slowly is extinguished you realize you are \n\r",ch);
		send_to_char("doomed...\n\r",ch);
	    }
	    else if (dream <= 320)
	    {
		send_to_char("Standing in front of a large man-sized mirror.. you gaze into its \n\r",ch);
		send_to_char("depths looking into your reflection.. when suddenly you think you \n\r",ch);
		send_to_char("see the reflection wink at you.. slightly startled you realize \n\r",ch);
		send_to_char("it was just a trick of your mind.. curious you place your hand \n\r",ch);
		send_to_char("against the mirror.. and the reflection mimics your actions \n\r",ch);
		send_to_char("exactally.. drawing your hand back away from the mirror your \n\r",ch);
		send_to_char("reflection gains a demonic appearance.. reaching through the \n\r",ch); 
		send_to_char("mirror it grabs your hand and pulls you in..\n\r",ch);
	    }
	    else if (dream <= 325)
	    {
		send_to_char("Plucking a apple from a tree along the trail, you lift it to your \n\r",ch); 
		send_to_char("mouth and begin to take a bite.. a faint whisper is heard..\n\r",ch);
		send_to_char("pleeeease... pleeeease slightly puzzled you ignore the faint voice\n\r",ch);
		send_to_char("and commence to take a satisfying bite from the ripe apple, and \n\r",ch);
		send_to_char("in responce a scream of mortal pain is heard..looking down at the \n\r",ch);
		send_to_char("apple in your palm.. a twisted and painfilled face glares at you \n\r",ch); 
		send_to_char("from the plush ripe apple..\n\r",ch);
	    }
	    else if (dream <= 330)
	    {
		send_to_char("Sitting upon a plush throne of gold and frilly pillows you feel a \n\r",ch);
		send_to_char("breeze.. looking to your right and left you see a pair of beautiful \n\r",ch);
		send_to_char("servants waving peacock feather fans through the air. Stepping \n\r",ch);
		send_to_char("towards you a equally handsome servant approaches with a plate of\n\r",ch); 
		send_to_char("ripe grapes...\n\r",ch);
	    }
	    else if (dream <= 335)
	    {
		send_to_char("Being led through a crowd of hissing and booing peasants.. two guards\n\r",ch);
		send_to_char("push you along towards a strange platform in the center of the square.\n\r",ch);
		send_to_char("shackles bear their heavy burden upon your chained arms and legs..\n\r",ch);
		send_to_char("Rotten fruit and eggs sail through the air at you.. steping upon\n\r",ch);
		send_to_char("the wooden platform, you are forced to kneel and a strange wooden bar\n\r",ch); 
		send_to_char("secures your head from moving... looking upwards you see a pair of straight\n\r",ch);
		send_to_char("wooden poles and at the top a gleaming harbringer of death.. a sharp blade..\n\r",ch);
		send_to_char("hearing a voice accost you from behind the words are barely heard as the\n\r",ch);
		send_to_char("crowds yells..  you have been accused of murder.. and found guilty by\n\r",ch);
		send_to_char("the court.. executioner let the blade fall..   and the last thing you hear\n\r",ch);
		send_to_char("is the grinding sound of metal against wood.. pain.. and then oblivion\n\r",ch);
	    }
	    else if (dream <= 340)
	    {
		send_to_char("Walking through the twisting corrodors of the endless dungeon.. you trip\n\r",ch);
		send_to_char("over a rock.. trying to get up you find your arms and legs stuck.. from\n\r",ch);
		send_to_char("the last faint flickers of your torch you see a gigantic web or sticky\n\r",ch);
		send_to_char("fibers... struggling against the tight pull of the web.. you hear a\n\r",ch);
		send_to_char("strange clicking sound come from above.. and as your torch goes out\n\r",ch);
		send_to_char("a enormous black spider floats down upon you..\n\r",ch);
	    }
	    else if (dream <= 345)
	    {
		send_to_char("Soft breezes blow against your skin, the salty air bitter on your tongue.\n\r",ch); 
		send_to_char("You see them... so soft... so unreal...\n\r",ch);
		send_to_char("You walk towards them, and hold out your hand for them to take.\n\r",ch);
		send_to_char("As your hand clasps theirs, they swirl and slowly fade into nothing.\n\r",ch);
		send_to_char("You sink to the damp, white sand... they are gone. And you are alone.\n\r",ch);
	    }
	    else if (dream <= 350)
	    {
		send_to_char("You feel someone breathing down the back of your neck.. you open your eyes to\n\r",ch);
		send_to_char("see a huge cat-like creature growling behind you...it smiles, and you see a\n\r",ch);
		send_to_char("large row of sharp teeth...before a scream can pass your lips, it sinks \n\r",ch); 
		send_to_char("it's fangs into your neck. Everything goes black.\n\r",ch);
	    }
	    else if (dream <= 355)
	    {
		send_to_char("Creak.\n\r",ch);
		send_to_char("Someone ... or something is approaching.\n\r",ch);
		send_to_char("You struggle to awaken as you hear the whisper of a sword being drawn.\n\r",ch);
		send_to_char("Red eyes gleem evilly at you as you frantically roll out of bed reaching \n\r",ch); 
		send_to_char("in vain for your weapon.\n\r",ch);
		send_to_char("Swish.\n\r",ch);
		send_to_char("Darkness.\n\r",ch);
	    }
	    else if (dream <= 360)
	    {
		send_to_char("You look around at a land youve never seen before wondering why it seems so \n\r",ch); 
		send_to_char("familar.. you hear the slithy tothes giriing near by <eh?>\n\r",ch);
		send_to_char("you feel something moving around your feet and look down to see the mome raths \n\r",ch); 
		send_to_char("swarming around you....\n\r",ch);
		send_to_char("a flock of jubjub birds flap towards the sky from some near by bushes \n\r",ch); 
		send_to_char("startled by something ..\n\r",ch);
		send_to_char("You hear a sound coming nearer something about it makes you shiver...\n\r",ch);
		send_to_char("You look up into a pair of glowing eyes and realize it must be ....it cant be \n\r",ch); 
		send_to_char("..oh no it is the creature your mother told you to beware of...\n\r",ch);
		send_to_char("the Jabberwock stands there its jaws snapping\n\r",ch);
		send_to_char("you turn to run but can go no where...\n\r",ch);
		send_to_char("you scream.....it fades into silence\n\r",ch);
	    }
	    else if (dream <= 365)
	    {
		send_to_char("You stand alone on a moonlit sandy beach , a sudden bzeeze blows agaainst you \n\r",ch);
		send_to_char("face , you shiver for some unknown reason.\n\r",ch);
		send_to_char("An eerie fog swirls around you.\n\r",ch);
		send_to_char("You hear a cackle from somewhere behind you but before you can turn an arm \n\r",ch);
		send_to_char("grabs you by the neck. \n\r",ch);
		send_to_char("A dagger presses against your throat, pierceing the skin.\n\r",ch);
		send_to_char("A low voice growls in your ear 'fear the ghost pirate Acheron'\n\r",ch);
		send_to_char("The fog swirls again and is gone. You are alone and trembleing..fearing the day\n\r",ch);
		send_to_char("that he might return...\n\r",ch);
		act( "$n fidgets and mumbles strange words in $s sleep.", ch, NULL, NULL, TO_ROOM,POS_RESTING); 
	    }
	    else if (dream <= 370)
	    {
		send_to_char("You are walking down the street on a bright and sunny morning.\n\r",ch);
		send_to_char("People are laughing and smiling happily as you walk by and you smile in\n\r",ch);
		send_to_char("return.  It sure is nice to see people happy for once.\n\r",ch);
		send_to_char("You continue your walk down the street and stop to look in a store window\n\r",ch);
		send_to_char("at beautiful dishes. Looking more closely in the window you gasp in horror.\n\r",ch);
		send_to_char("YOU FORGOT TO PUT YOUR CLOTHES ON THIS MORNING!!!!\n\r",ch);
		send_to_char("You turn deep red from embrassment and hide behind a bush.\n\r",ch);
	    }
	    else if (dream <= 375)
	    {
		send_to_char("You are surrounded by golden flames.\n\r",ch);
		send_to_char("You frantically look for a way of escape, but to no avail.\n\r",ch);
		send_to_char("You smell the armoma of burning flesh.\n\r",ch);
		send_to_char("YOUR FLESH!\n\r",ch);
		send_to_char("The flames around you seem to be getting hotter.\n\r",ch);
		send_to_char("You scream but no one hears your voice or is it that no one cares.\n\r",ch);
		send_to_char("You run aimlessly but the flames seem to follow your every move.\n\r",ch);
		send_to_char("THERE IS NO ESCAPE!\n\r",ch);
		send_to_char("\n\r",ch);
		send_to_char("As the flames are about to engulf you...\n\r",ch);
		send_to_char("You realize....its only a dream....its only a dream...\n\r",ch);
		send_to_char("Or is it........\n\r",ch);
	    }
	    else if (dream <= 380)
	    {
		send_to_char("As you start to drift off to sleep you see sheep  jumping over a fence..\n\r",ch);
		send_to_char("One Sheep, Two sheep, Three Sheep, Four Sheep...\n\r",ch);
		send_to_char("Slowly as you fall into  a deeper sslleep the sheep start morphing into wolves.\n\r",ch);
		send_to_char("Even farther into your slumber the wolves again start morphing. into corpses..\n\r",ch); 
		send_to_char("Eight Corpses, Nine Corpses...\n\r",ch);
		send_to_char("You then realise that the corpses are YOUR corpses!\n\r",ch);
		send_to_char("You wake up bathed in a blood-red sweat....\n\r",ch);
	    }
	    else if (dream <= 385)
	    {
		send_to_char("'They're coming' shrieked the old woman.\n\r",ch);
		send_to_char("Sudden blurs of movement fog your already hazy eyes.\n\r",ch);
		send_to_char("Praying for strength you struggle to rise to your feet.\n\r",ch);
		send_to_char("You fall to your knees, too dizzy to stand. You realize everyone else\n\r",ch);
		send_to_char("has escaped through the passage and you are left alone...\n\r",ch);
		send_to_char("The door bursts open and you lift your head to see.\n\r",ch);
		send_to_char("Laughing in hysteria you know it's over and there is no escape.\n\r",ch);
		send_to_char("The dark figures roughly pull you to your feet \n\r",ch);
		send_to_char("sneering in disgust at your hideous appearance, the leader snaps your neck\n\r",ch);
		send_to_char("and all is silent in the  church yard as the 7 nuns dispose of your body.\n\r",ch);
	    }
	    else if (dream <= 390)
	    {
		send_to_char("You look into their eyes, unable to break the gaze because of its\n\r",ch);
		send_to_char("intensity.  She reaches for your hand and places it in hers. Holding it\n\r",ch);
		send_to_char("there for a moment of sadness, then kissing your forehead lovingly she goes.\n\r",ch);
		send_to_char("You watch her leave, and a single tear falls to the ground, you know she\n\r",ch);
		send_to_char("will never return.  Realizing she left something in your hand you open\n\r",ch);
		send_to_char("it to find a small ring with a beautiful amethyst set in it.\n\r",ch);
		send_to_char("You put the ring on your own finger and swear to never forget the most\n\r",ch);
		send_to_char("important person in your life. Once again you cry for the loss of\n\r",ch);
		send_to_char("your mother.\n\r",ch);
	    }
	    else if (dream <= 395)
	    {
		send_to_char("You hear fighting all around you, looking around you realize your in the\n\r",ch);
		send_to_char("middle of a gang fight. You notice that you are holding a 6 inch knife, and \n\r",ch); 
		send_to_char("there's a man in front of you pointing a gun at your head.\n\r",ch);
		send_to_char("All of a sudden you hear strange noises from behind you and the\n\r",ch);
		send_to_char("man with the gun whirls around to get a better look. As he does you lunge\n\r",ch);
		send_to_char("the knife into  him and knock the gun ou of his hand.\n\r",ch);
		send_to_char("reallizing what you just did you take off runnin, and keep running,\n\r",ch);
		send_to_char("leaping over a 8' fence no  problem, running, running, running until you\n\r",ch);
		send_to_char("can't run any more and you collapse under a tree. Finding yourself exhausted\n\r",ch); 
		send_to_char("you pass out without knowing it.\n\r",ch);
		send_to_char("Your vision goes blood-red and the man you stabbed is burned into\n\r",ch); 
		send_to_char("your mind until you wake up trembling  in horror.\n\r",ch);
	    }
	    else if (dream <= 400)
	    {
		send_to_char("You are in a large cell with mattresses completly covering every wall.. \n\r",ch);
		send_to_char("You try to move but find that\n\r",ch);
		send_to_char("it is nearly immpossible since you are in a straight jacket.\n\r",ch);
		send_to_char("You scream as loud as you can, hoping that someone will come, but nobody\n\r",ch);
		send_to_char("does..\n\r",ch);
		send_to_char("You're doomed to stay here for the rest of eternity, there is no escape..\n\r",ch);
	    }
	    else if (dream <= 405)
	    {
		send_to_char("Running through the forest, your heart almost pounds out of your\n\r",ch);
		send_to_char("chest. The cold sweat pouring down your cheeks blurs your vision\n\r",ch);
		send_to_char("and the burning is almost unbearable.  You can hear the rapid\n\r",ch);
		send_to_char("hoofbeats close behind you and the howling of dogs on your scent.\n\r",ch);
		send_to_char("There!! Up ahead you see a light!!! Happy tears stream down your\n\r",ch);
		send_to_char("face as you run to the flames. As you are almost upon the flames your laughter\n\r",ch); 
		send_to_char("stops and is replaced with piercing shrieks. Where are\n\r",ch);
		send_to_char("the horrible screams coming from? You turn and see 2 charred bodies\n\r",ch);
		send_to_char("running madly around, still covered with flickering fire.  You cover\n\r",ch);
		send_to_char("your mouth as the sickening feeling over comes you and you \n\r",ch);
		send_to_char("collapse into the flames and join the poor people behind you.\n\r",ch);
	    }
	    else if (dream <= 410)
	    {
		send_to_char("You think about vampires and one appears.\n\r",ch);
		send_to_char("It reaches for you and you scream in terror as\n\r",ch);
		send_to_char("it's fangs bite into your neck!\n\r",ch);
	    }
	    else if (dream <= 415)
	    {
		send_to_char("The bright beam catches you on the shoulder and you scream in pain as\n\r",ch);
		send_to_char("your flesh rips open. You fumble with your gun and try to unjam the trigger.\n\r",ch);
		send_to_char("Still unable to get the laser to work, you throw it to the ground in\n\r",ch);
		send_to_char("disgust. Holding your injured shoulder with one hand, you reach with the\n\r",ch);
		send_to_char("other into your pack, pulling out a small blinking sphere.\n\r",ch);
		send_to_char("You close your eyes and say a silent good-bye to your fleet, only\n\r",ch);
		send_to_char("praying they made it out in time.  You can hear the strange foreign\n\r",ch);
		send_to_char("shouts of the Krakians coming up quickly, and you shudder in fear.\n\r",ch);
		send_to_char("Soon they are nearing the gate you're in and you know it is time. You\n\r",ch);
		send_to_char("push a glowing button on the sphere and it begins to beep loudly.\n\r",ch);
		send_to_char("'10 seconds until utter destruction' a computerized voice intones.\n\r",ch);
		send_to_char("You clip the sphere onto the wall as the Krakians enter screaming.\n\r",ch);
		send_to_char("There is a sudden white light and then nothing....\n\r",ch);
	    }
	    else if (dream <= 420)
	    {
		send_to_char("You hear the slow flap of wings.\n\r",ch);
		send_to_char("It draws closer and closer.  Looking up\n\r",ch);
		send_to_char("you can see a large silver dragon outlined against the sun.\n\r",ch);
		send_to_char("As it draws nearer, you see a  golden lyre dangling from a strap\n\r",ch);
		send_to_char("around its neck.  Suddenly you let out a scream and run for cover.\n\r",ch);
		send_to_char("You fall.  splat.  Heartbeats later.  SPLAT!!\n\r",ch);
	    }
	    else if (dream <= 425)
	    {
		send_to_char("You sit on the green grass beneath a broad large oak tree staring out at the\n\r",ch);
		send_to_char("sunlit fields while a soft warm breeze touchs you lightly. Staring up\n\r",ch);
		send_to_char("through the leaves you can see motes of sunlight peeking through and you\n\r",ch);
		send_to_char("can only think of how wonderful it is to be alive...\n\r",ch);
		send_to_char("\n\r",ch);
		send_to_char("Suddenly, violently the weather changes, wind whipping almost through you,\n\r",ch);
		send_to_char("chilling your soul.  The plains turn a drab grey and they sun clouds over.\n\r",ch);
		send_to_char("Something caresses your shoulder and you restist temptation to look even \n\r",ch);
		send_to_char("though it frightens and freezes your blood....\n\r",ch);
		send_to_char("\n\r",ch);
		send_to_char("Like a snake striking a tendril wraps around your neck choking you.  Fighting\n\r",ch);
		send_to_char("to scream, for even a sound to escape your throat, you claw at your unkown\n\r",ch);
		send_to_char("foe.\n\r",ch);
	    }
	    else if (dream <= 430)
	    {
		send_to_char("Aaaaaahhhh, what a good nights sleep you had.  You stand up and stretch\n\r",ch);
		send_to_char("letting everybone in your body snap and pop.. Looking at the clock you\n\r",ch);
		send_to_char("realise it's only 6:30AM and that maybe it would be a good time to go\n\r",ch);
		send_to_char("check out the mud.  As you start walking to the computer room you feel a\n\r",ch);
		send_to_char("cold chill run thru  your body but just shrug it off.  As you enter the room\n\r",ch);
		send_to_char("and look over to where your computer is, you realize IT'S GONE!  Thinkinng\n\r",ch);
		send_to_char("you've been robbed you run to the phone, but find it isn't there either!\n\r",ch);
		send_to_char("Feeling scared, and invaded you look around to see what else is missing\n\r",ch);
		send_to_char("when you notice the door, and none of the windows have been broken into.\n\r",ch);
		send_to_char("Another cold chill runs thru your body and you hear a voice off in the\n\r",ch);
		send_to_char("background...\n\r",ch);
		send_to_char("'You are entering another dimension...'\n\r",ch);
	    }
	    else if (dream <= 435)
	    {
		send_to_char("Looking back you see hundreds of people running your way, all screaming your\n\r",ch);
		send_to_char("name and waving weapons in the air.  You have no idea why they're chasing you\n\r",ch);
		send_to_char("but decide it would be a good idea to get out of there!  Starting to run\n\r",ch);
		send_to_char(", you run down many streets and alleys slowly winding your way out of the\n\r",ch);
		send_to_char("town only stopping occassionaly to fight someone that jumped out in front\n\r",ch);
		send_to_char("of you. After what seems like forever you come to a small inn and decide to stay for the night.\n\r",ch); 
		send_to_char("after paying the innkeeper extra to not tell anyone your there you go up to\n\r",ch);
		send_to_char("your room and tend to your wounds. Shortly after you hear a horde of people come in and\n\r",ch);
		send_to_char("question him, then go racing off...\n\r",ch);
		send_to_char("The next morning you thank the man and go off looking for\n\r",ch);
		send_to_char("a new place to  hide, fearing they will eventually catch up to you and\n\r",ch);
		send_to_char("wonder what they will do then...\n\r",ch);
	    }
	    else if (dream <= 440)
	    {
		send_to_char("You feel a soft brush at your neck, and ignore it.\n\r",ch);
		send_to_char("Then you feel the fangs PIERCE your skin and your lifeblood being taken!\n\r",ch);
		send_to_char("\n\r",ch);
		send_to_char("You sit bolt upright, your heart pounding!\n\r",ch);
		send_to_char("You look about but see no one near you.\n\r",ch);
		send_to_char("You calm yourself and drift back to sleep, realizing it was only a dream.\n\r",ch);
		send_to_char("Or was it...........?\n\r",ch);
	    }
	    else if (dream <= 445)
	    {
		send_to_char("You hear flutes in the distance. As the fog clears you realize that you\n\r",ch);
		send_to_char("are standing in a quaint village filled with small happy fauns. A small\n\r",ch);
		send_to_char("male approaches you, smiles, puts a pipe to his lips, and begins to play\n\r",ch);
		send_to_char("a tune. He walks away - and you begin to follow. At first you are enchanted\n\r",ch);
		send_to_char("...but you slowly come to the realization you can't stop! You struggle\n\r",ch);
		send_to_char("from the sound...throwing your hands over your ears...but it solves\n\r",ch);
		send_to_char("nothing. He leads you away from the village into a dark, ominous forest.\n\r",ch);
		send_to_char("The sound begins to drill into your eardrums...then your brain...\n\r",ch);
		send_to_char("You awake with a start!\n\r",ch);
	    }
	    else if (dream <= 450)
	    {
		send_to_char("You are being led down a corridor.\n\r",ch);
		send_to_char("You are completely shackled and each arm is fiercely gripped by a decaying\n\r",ch);
		send_to_char("zombie...They force you into a large dark throne room. Before you sits\n\r",ch);
		send_to_char("Egil on a throne of chipped bone and patchworked flesh. He is in deep\n\r",ch);
		send_to_char("conversation with Withers, who stands to his left. Withers leans over and\n\r",ch);
		send_to_char("whispers something into Egil's ear, and Egil turns to glare at you. He\n\r",ch);
		send_to_char("extends his arm, makes a fist, and points his thumb down to the floor.\n\r",ch);
		send_to_char("\n\r",ch); 
		send_to_char("Excruciating pain fills your entire body, and you look down to see your\n\r",ch);
		send_to_char("skin dissolve into dust. You open your mouth, and a scream of hopelessness\n\r",ch);
		send_to_char("escapes........\n\r",ch);
		send_to_char("\n\r",ch);
		send_to_char("All goes dark.......\n\r",ch);
	    }
	    else if (dream <= 455)
	    {
		send_to_char("You're resting peacefully, dreaming about beautiful green fields, full of colorful flowers.. the \n\r",ch); 
		send_to_char("sun shines brightly overhead, when an odd sensation awakes you.\n\r",ch);
		send_to_char("You wake up to feel an odd burning sensation on your left hand, and look down to\n\r",ch);
		send_to_char("find your hand literally on fire! Bright flames and greasy orange smoke rise from the palm of your \n\r",ch); 
		send_to_char("hand, as you sit helplessly and watch the skin drip and crackle,\n\r",ch);
		send_to_char("peeling away from the bones as you begin to scream in agony..\n\r",ch);
		send_to_char("You sit bolt upright in bed, rocked to the core of your being as you realize this was only.. a \n\r",ch);
		send_to_char("dream..? \n\r",ch);
	    }
	    else if (dream <= 460)
	    {
		send_to_char("Your walking down a grey road, and the scenes all about\n\r",ch); 
		send_to_char("you are blue, thats all you see, greish blue for as far as \n\r",ch);
		send_to_char("the eye can see.\n\r",ch);
		send_to_char("You keep walking down the road, and you come upon a bench, \n\r",ch);
		send_to_char("with 2 people sitting on it reading news papers, you \n\r",ch);
		send_to_char("notice the papers are close to there faces.\n\r",ch);
		send_to_char("All of a sudden there faces morph into the news paper, and \n\r",ch);
		send_to_char("they put their hands down. They look at you then start  \n\r",ch);
		send_to_char("talking to each other.\n\r",ch);
		send_to_char("You run as fast as you can down the road, till you see a \n\r",ch);
		send_to_char("woman, made of bones. She is taking a walk, with her baby in the cart\n\r",ch); 
		send_to_char("she is pushing along, you decide to look closer, so you \n\r",ch);
		send_to_char("walk over to the cart made of bones...\n\r",ch);
		send_to_char("You look in side and the baby screams so loud it knocks \n\r",ch);
		send_to_char("you back a few feet. \n\r",ch);
		send_to_char("You run as fast as you can, you come to a cliff.\n\r",ch);
	    }
	    else if (dream <= 465)
	    {
		send_to_char("you hear them comming\n\r",ch);
		send_to_char("They're comming, there's too many of them. What will you do?\n\r",ch);
		send_to_char("Suddenly, they crest the ridge, and you are bowled over by \n\r",ch);
		send_to_char("2000 puppies who want to lick your face.\n\r",ch);
		send_to_char("They're all over. Well, at least they're housebroken.\n\r",ch);
		send_to_char("Aren't they?\n\r",ch);
	    }
	    else if (dream <= 470)
	    {
		send_to_char("You find yourself in a labyrinth of pipes and gears. All around you, you\n\r",ch);
		send_to_char("hear clanks, bangs, and the rattle of chains. You smell the oil used to\n\r",ch);
		send_to_char("lubricate the gears.  As you walk through this place, you find it\n\r",ch);
		send_to_char("looks familiar. You wonder why. Then you realize, you've\n\r",ch);
		send_to_char("seen it before! A gnome had been working on the plans a while back!\n\r",ch);
	    }
	    else if (dream <= 475)
	    {
		send_to_char("Wait, is that pipe shaking?  Yes, it is! All of the sudden\n\r",ch); 
		send_to_char("you, hear the shrill scream of alarms! You run as fast as you\n\r",ch);
		send_to_char("can, trying to get out, but it's just too big! Oh, no!\n\r",ch); 
		send_to_char("BOOOOOOOOMMMMMMMMMM!!!!!!!!!!!!!!!\n\r",ch);
		send_to_char("You wake up, with grease in your hair.\n\r",ch);
	    }
	    else if (dream <= 480)
	    {
		send_to_char("You are surrounded by complete darkness.  Sudden screams are heard all around\n\r",ch);
		send_to_char("you, and you spin around holding your hands to your ears in an attempt to shut\n\r",ch);
		send_to_char("out the screams.  You start laughing hysterically to yourself, as the screams\n\r",ch);
		send_to_char("continue their mocking tones within your head.  Red eyes are staring from all\n\r",ch);
		send_to_char("around you, and as you let out a HUGE scream of pain, it is ended quickly by\n\r",ch);
		send_to_char("your body being ripped apart by something.\n\r",ch);
	    }
	    else if (dream <= 485)
	    {
		send_to_char("As you dazely open your eyes you find yourself in a dimly lit room. You see\n\r",ch);
		send_to_char("a small light in the distance but as you reach towards it, you find out you\n\r",ch);
		send_to_char("can't move your hands. You try to pull will all your might but nothing\n\r",ch);
		send_to_char("happens. Your arms and the rest of your body stays in the same position.\n\r",ch);
		send_to_char("Suddenly the light nears and you see the faint outline of your.... MOTHER!\n\r",ch);
		send_to_char("You notice her face has a pale glow to it, and she has a large butcher knife\n\r",ch);
		send_to_char("in her hand. You try to scream but the gag prevents your scream, and your\n\r",ch);
		send_to_char("eyes widen as the knife swiftly descends!\n\r\n\r\n\r",ch);
		send_to_char("You wake up in your bed bathed in sweat and sigh in relief. As you run out\n\r",ch);
		send_to_char("to the kitchen to grab a bite, you see your mother with her back turned to\n\r",ch);
		send_to_char("you. Suddenly she turns towards you with glazing white eyes, the same pale\n\r",ch);
		send_to_char("face, and runs towards you with her butcherknife!\n\r",ch);
	    }
	    else if (dream <= 490)
	    {
		send_to_char("Ahhh.. everything is so perfect today. The green trees blow in harmony with\n\r",ch);
		send_to_char("the gentle wind, and bright blue clouds decorate the neverending sky.  \"Just\n\r",ch);
		send_to_char("a perfect day for a dip in the sea\", you say to yourself.\n\r",ch);
		send_to_char("As you wade slowly into the water, you notice how perfectly warm the\n\r",ch);
		send_to_char("temperature is, and you wiggle your toes in anticipation. As you lean you\n\r",ch);
		send_to_char("head back into the water to flow on the water, you suddenly feel something\n\r",ch);
		send_to_char("touch your leg! \"What the Hec...\" But before you get any longer, cold bony\n\r",ch);
		send_to_char("hands grasp your ankles and draw you DOWN! You frantically try to push the\n\r",ch);
		send_to_char("hands away, but their grip is just to STRONG! You try one last desperate\n\r",ch);
		send_to_char("scream of help, but only water pours down your lungs, blocking your need for\n\r",ch);
		send_to_char("air. Alas, all you can do is watch helplessly as you're dragged mercilessly\n\r",ch);
		send_to_char("to your doom. You make one last frantic claw at the darkness, but to no\n\r",ch);
		send_to_char("avail....\n\r",ch);
	    }
	    else if (dream <= 495) 
	    {
		send_to_char("You open your eyes, it is still dark.\n\r",ch);
		send_to_char("The fire has gone out.  Where are the matchs?\n\r",ch);
		send_to_char("You reach around in the dark, your hand strikes something hard.\n\r",ch);
		send_to_char("Ouch!!!!  Blood flows from your hand as you nick it on the blade of a \n\r",ch);
		send_to_char("sword. The room lights up as your blood drips onto the blade. \n\r",ch);
		send_to_char("The image of a powerful sword is burned into your mind!\n\r",ch);
		act( "$n lets out a cry and a bit of blood drips from $s hand!", ch, NULL, NULL, TO_ROOM,POS_RESTING);
		act( "Mumbling strange words, $n passes into a deeper sleep.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    }
	    else if (dream <= 500)
	    {
		send_to_char("You shudder from the cold as you see a scene of a vast frozen wasteland\n\r",ch);
		send_to_char("you begin to stumble around in the cold snow drifts and frozen ponds\n\r",ch);
		send_to_char("the cold begins to reach your fingers and toes as you begin to loose\n\r",ch);
		send_to_char("feeling in them, all of a sudden a man dressed in black appears before you\n\r",ch);
		send_to_char("in one hand he is holding a dagger and in the other...\n\r",ch);
		send_to_char("YOUR HEAD!?!! As you begin to scream it slowly fades away as you feel the frost\n\r",ch);
		send_to_char("from a short blade against your neck.\n\r",ch);
		act( "$n lets out a cry and a bit of blood drips from $s hand!", ch, NULL, NULL, TO_ROOM,POS_RESTING);
		act( "Mumbling strange words, $n passes into a deeper sleep.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    }
/*	    else if (dream <= 278)
	    {
		send_to_char("Ahhhh...  Asleep at last, it's been a long time since you've had a good\n\r",ch);
		send_to_char("nights sleep, and you figure that  tonight should be different.. Or should\n\r",ch);
		send_to_char("it!   All of a sudden you feel a burst of energy run thru your body, and\n\r",ch);
		send_to_char("you break out in a cold sweat!  YOu feel as if someone or some THING is\n\r",ch);
		send_to_char("pulling your body, down, down, down, your pulled until your at the center\n\r",ch);
		send_to_char("of the earth.  Then a deep voice speaks,   I have been waiting your arrival\n\r",ch);
		send_to_char("it booms. Looking around franticlly you see a dark figure with horns dressed\n\r",ch);
		send_to_char("in a blood-stained cloak. 'You have been very bad...' it continues 'for this\n\r",ch);
		send_to_char("I will punish you!' With that you feel a draining of strength and you fall\n\r",ch);
		send_to_char("to the ground.  Trying to get up you realise that you can't move! Then the\n\r",ch);
		send_to_char("voice booms 'And now, you DIE!'  ZZZZZZZZZZaaaaaaaaaaaaappppppp! a bolt \n\r",ch);
		send_to_char("lightning hits your body...\n\r",ch);
		send_to_char("\n\r",ch);
		send_to_char("\n\r",ch);
		send_to_char("                                  Everything falls silent.\n\r",ch);
		ch->hit = 1;
		act( "A bolt of {WLIGHTNING{0 falls from the sky and stikes $n as $e lies sleeping!", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    }
	    else if (dream <= 410)
	    {
		send_to_char("A hand grabs your shoulder and a grizzled old man\n\r",ch);
		send_to_char("peers at your wounds.\n\r",ch);
		send_to_char("'Hmmmm, let me see.'\n\r",ch);
		send_to_char("He digs in his pouch and pulls out a smelly potion.\n\r",ch);
		send_to_char("'Here, drink this.'\n\r",ch);
		send_to_char("As you raise yourself from your slumbers, he disappears.\n\r",ch);
		send_to_char("All that remains is a vial that reeks even from arms length.\n\r",ch);
	    } */
	    else
		send_to_char("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz.\n\r",ch);
	}

	if (!IS_IMMORTAL(ch))
	{
	    OBJ_DATA *obj;

	    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] > 0 )
	    {
		if ( --obj->value[2] == 0 && ch->in_room != NULL )
		{
		    --ch->in_room->light;
		    act( "$p goes out.", ch, obj, NULL, TO_ROOM,POS_RESTING);
		    act( "$p flickers and goes out.", ch, obj, NULL, TO_CHAR,POS_RESTING);
		    extract_obj( obj );
		}
	 	else if ( obj->value[2] <= 5 && ch->in_room != NULL)
		    act("$p flickers.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    }

	    gain_condition( ch, COND_DRUNK,  -1 );
	    gain_condition( ch, COND_FULL, ch->size > SIZE_MEDIUM ? -4 : -2 );
	    gain_condition( ch, COND_THIRST, -1 );
	    gain_condition( ch, COND_HUNGER, ch->size > SIZE_MEDIUM ? -2 : -1);

	    if ( ( ++ch->timer > mud_stat.timeout_mortal
	    &&     mud_stat.timeout_mortal > 0 )
	    ||   ( ch->desc == NULL && ch->timer > mud_stat.timeout_ld_mort ) )
	    {
		act( "$n disappears into the void.",
		    ch, NULL, NULL, TO_ROOM, POS_RESTING );
		send_to_char( "You disappear into the void.\n\r", ch );
		force_quit( ch, "" );
	    }
	}
	else
	{
	    if ( ( ++ch->timer > mud_stat.timeout_immortal
	    &&     mud_stat.timeout_immortal > 0
	    &&     str_cmp( ch->name, "Shryp" ) )
	    ||   ( ch->desc == NULL && ch->timer > mud_stat.timeout_ld_imm ) )
		force_quit( ch, "" );
	}
    }
}

void char_update( void )
{   
    CHAR_DATA *ch, *ch_next;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	AFFECT_DATA *paf, *paf_next;

	ch_next = ch->next;

	if ( ch->position >= POS_STUNNED )
	{
            if ( IS_NPC(ch) && ch->zone != NULL && ch->in_room != NULL
	    &&   !IS_SET( ch->act, ACT_NO_RETURN_HOME )
	    &&   ch->zone != ch->in_room->area
	    &&   ch->desc == NULL && ch->fighting == NULL 
	    &&   !IS_AFFECTED(ch,AFF_CHARM) && number_percent() < 5 )
            {
            	act("$n wanders on home.",ch,NULL,NULL,TO_ROOM,POS_RESTING);
            	extract_char(ch,TRUE);
            	continue;
            }

	    if ( ch->hit < ch->max_hit )
		ch->hit += hit_gain(ch);
	    else
		ch->hit = ch->max_hit;

	    if ( ch->mana < ch->max_mana )
		ch->mana += mana_gain(ch);
	    else
		ch->mana = ch->max_mana;

	    if ( ch->move < ch->max_move )
		ch->move += move_gain(ch);
	    else
		ch->move = ch->max_move;
	}

	if ( ch->position == POS_STUNNED )
	    update_pos( ch );

	for ( paf = ch->affected; paf != NULL; paf = paf_next )
	{
	    paf_next = paf->next;

	    if ( paf->dur_type != DUR_TICKS )
		continue;

	    else if ( paf->duration > 0 )
	    {
		paf->duration--;
		if ( number_range( 0, 4 ) == 0 && paf->level > 0 )
		  paf->level--;  /* spell strength fades with time */
            }

	    else if ( paf->duration == 0 )
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( skill_table[paf->type].msg_off )
			act( skill_table[paf->type].msg_off,
			    ch, NULL, NULL, TO_CHAR, POS_SLEEPING );
		    
		    if ( skill_table[paf->type].room_msg != NULL )
			act( skill_table[paf->type].room_msg,
			    ch, NULL, NULL, TO_ROOM, POS_RESTING );

		    if ( skill_table[paf->type].sound_off != NULL )
			send_sound_room( ch->in_room, 75, 1, 95, "skills",
			    skill_table[paf->type].sound_off, SOUND_NOSKILL );
		}

		if ( paf->type == gsn_mana_tap )
		{
		    ch->position = POS_STANDING;
		    ch->mana = ch->max_mana;
		    update_pos( ch );
		}
		affect_remove( ch, paf );
	    }
	}

        if (is_affected(ch, gsn_plague) && ch != NULL)
        {
            AFFECT_DATA *af, plague;
            CHAR_DATA *vch;
            int dam;

	    if (ch->in_room == NULL)
		continue;
            
	    act("$n writhes in agony as plague sores erupt from $s skin.",
		ch,NULL,NULL,TO_ROOM,POS_RESTING);
	    send_to_char("You writhe in agony from the plague.\n\r",ch);
            for ( af = ch->affected; af != NULL; af = af->next )
            {
            	if (af->type == gsn_plague)
                    break;
            }
        
            if (af == NULL)
            {
            	REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
		continue;
            }
        
            if (af->level == 1)
		continue;
        
	    plague.where	= TO_AFFECTS;
            plague.type 	= gsn_plague;
            plague.level 	= af->level - 1; 
	    plague.dur_type	= DUR_TICKS;
            plague.duration 	= number_range(1,2 * plague.level);
            plague.location	= APPLY_STR;
            plague.modifier 	= -5;
            plague.bitvector 	= AFF_PLAGUE;
        
            for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
            {
                if (!saves_spell(plague.level - 2,ch,vch,DAM_DISEASE) 
		&&  !IS_IMMORTAL(vch)
            	&&  !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(4) == 0)
            	{
            	    send_to_char("You feel hot and feverish.\n\r",vch);
            	    act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM,POS_RESTING);
            	    affect_join(vch,&plague);
            	}
            }
	    dam = UMIN(ch->level,af->level/5+1);
	    ch->mana -= dam;
	    ch->move -= dam;
	    damage( ch, ch, dam, gsn_plague,DAM_DISEASE,FALSE,FALSE,NULL);
        }
	else if ( IS_AFFECTED(ch, AFF_POISON) && ch != NULL
	     &&   !IS_AFFECTED(ch,AFF_SLOW))

	{
	    AFFECT_DATA *poison;

	    poison = affect_find(ch->affected,gsn_poison);

	    if (poison != NULL)
	    {
	        act( "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	        send_to_char( "You shiver and suffer.\n\r", ch );
	        damage(ch,ch,poison->level/10 + 1,gsn_poison,
		    DAM_POISON,FALSE,FALSE,NULL);
	    }
	}

	else if ( ch->position == POS_INCAP && number_range( 0, 1 ) == 0 )
	    damage( ch, ch, 20, TYPE_UNDEFINED, DAM_OTHER, FALSE, FALSE, NULL );

	else if ( ch->position == POS_MORTAL )
	    damage( ch, ch, 20, TYPE_UNDEFINED, DAM_OTHER, FALSE, FALSE, NULL );

        else if ( weather_info.sky == SKY_LIGHTNING
              && !IS_IMMORTAL(ch) && number_range(0,100) <= 1
              && ch->in_room &&  !IS_SET(ch->in_room->room_flags,ROOM_INDOORS) ) {
            damage( ch, ch, 20, TYPE_UNDEFINED, DAM_LIGHTNING, FALSE, FALSE, NULL );
            act( "{4L{$i{6g{^h{7t{&n{7i{^n{6g {$strikes {4$n{0!", ch, NULL, NULL, TO_ALL, POS_RESTING );
        }
    }
}

void obj_update( void )
{   
    AFFECT_DATA *paf, *paf_next;
    AUCTION_DATA *auc;
    CHAR_DATA *wch;
    OBJ_DATA *obj, *obj_next;
    OBJ_MULTI_DATA *mult, *mult_next;
    char buff[MAX_STRING_LENGTH], message[MAX_INPUT_LENGTH];

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	CHAR_DATA *rch;

	obj_next = obj->next;

	for ( mult = obj->multi_data; mult != NULL; mult = mult_next )
	{
	    mult_next = mult->next;

	    if ( --mult->drop_timer <= 0 )
		free_obj_multi( obj, mult );
	}

	for ( paf = obj->affected; paf != NULL; paf = paf_next )
	{
	    paf_next = paf->next;

	    if ( paf->dur_type != DUR_TICKS )
		continue;

	    if ( paf->duration > 0 )
	    {
		paf->duration--;
		if ( number_range( 0, 4 ) == 0 && paf->level > 0 )
		    paf->level--;
            }

	    else if ( paf->duration == 0 )
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_obj )
		    {
			if ( obj->carried_by != NULL )
			{
			    rch = obj->carried_by;
			    act( skill_table[paf->type].msg_obj,
				rch, obj, NULL, TO_CHAR, POS_RESTING );
			}

			if ( obj->in_room != NULL 
			&&   obj->in_room->people != NULL )
			{
			    rch = obj->in_room->people;
			    act( skill_table[paf->type].msg_obj,
				rch, obj, NULL, TO_ALL, POS_RESTING );
			}
		    }
		}

		affect_remove_obj( obj, paf );
	    }
	}

	if ( obj->in_room || ( obj->carried_by && obj->carried_by->in_room ) )
	{
	    if ( HAS_TRIGGER_OBJ( obj, TRIG_DELAY ) && obj->oprog_delay > 0 )
	    {
	        if ( --obj->oprog_delay <= 0 )
		    p_percent_trigger( NULL, obj, NULL, NULL, NULL, NULL, TRIG_DELAY );
	    }

	    else if ( ( ( obj->in_room && obj->in_room->area->nplayer > 0 )
	    	 ||   obj->carried_by ) && HAS_TRIGGER_OBJ( obj, TRIG_RANDOM ) )
		    p_percent_trigger( NULL, obj, NULL, NULL, NULL, NULL, TRIG_RANDOM );
	}

	/* Make sure the object is still there before proceeding */
	if ( !obj )
	    continue;

	if ( obj->timer <= 0 || --obj->timer > 0 )
	    continue;

	switch ( obj->item_type )
	{
	    default:
		strcpy( message, "$p crumbles into dust." );
		if ( obj->item_type != ITEM_KEY && obj->carried_by != NULL
		&&   !IS_NPC( obj->carried_by ) )
		{
		    sprintf( buff, "object %s -- %s crumbles.",
			obj->name, obj->short_descr );
		    append_file( obj->carried_by, "../log/timers.txt", buff );
		}
		break;

	    case ITEM_FOUNTAIN:
		strcpy( message, "$p dries up." );
		break;

	    case ITEM_CORPSE_NPC:
	    case ITEM_CORPSE_PC:
		strcpy(  message, "$p decays into dust." );
		break;

	    case ITEM_FOOD:
		strcpy( message, "$p decomposes." );
		break;

	    case ITEM_POTION:
		strcpy( message, "$p has evaporated from disuse." );
		break;

	    case ITEM_PORTAL:
		strcpy( message, "$p fades out of existence." );
		break;

	    case ITEM_CONTAINER: 
	    case ITEM_PIT: 
		if ( CAN_WEAR( obj, ITEM_WEAR_FLOAT ) )
		{
		    if ( obj->contains )
			strcpy( message, "$p flickers and vanishes, spilling its contents on the floor." );
		    else
			strcpy( message, "$p flickers and vanishes." );
		}

		else
		    strcpy( message, "$p crumbles into dust." );
		break;
	}

	if ( obj->pIndexData->vnum == OBJ_VNUM_FOG )
	    strcpy( message, "The wind gusts violently and $p is gone." );

	if ( obj->carried_by != NULL )
	{
	    if ( IS_NPC( obj->carried_by )
	    &&   obj->carried_by->pIndexData->pShop != NULL )
		obj->carried_by->silver += obj->cost/5;
	    else
	    {
		act( message, obj->carried_by, obj, NULL, TO_CHAR, POS_RESTING );
		if ( obj->wear_loc == WEAR_FLOAT )
		    act( message, obj->carried_by, obj, NULL, TO_ROOM, POS_RESTING );
	    }
	}

	else if ( obj->in_room != NULL
	&&        ( rch = obj->in_room->people ) != NULL )
	{
	    if ( ! ( obj->in_obj && obj->in_obj->pIndexData->item_type == ITEM_PIT
	    &&   !CAN_WEAR( obj->in_obj, ITEM_TAKE ) ) )
	    {
		act( message, rch, obj, NULL, TO_ROOM, POS_RESTING );
		act( message, rch, obj, NULL, TO_CHAR, POS_RESTING );
	    }
	}

	if ( ( ( obj->item_type == ITEM_CORPSE_PC
	||   obj->wear_loc == WEAR_FLOAT ) &&  obj->contains )
	||   obj->item_type == ITEM_CORPSE_NPC )
	{
	    OBJ_DATA *t_obj, *next_obj;

	    for ( t_obj = obj->contains; t_obj != NULL; t_obj = next_obj )
	    {
		next_obj = t_obj->next_content;

		obj_from_obj( t_obj );

		if ( obj->in_obj ) /* in another object */
		    obj_to_obj( t_obj, obj->in_obj );

		else if ( obj->carried_by )  /* carried */
		{
		    if ( obj->wear_loc == WEAR_FLOAT )
		    {
			if ( obj->carried_by->in_room == NULL )
			    extract_obj( t_obj );
			else
			{
			    set_obj_sockets( obj->carried_by, t_obj );
			    set_arena_obj( obj->carried_by, t_obj );
			    obj_to_room( t_obj, obj->carried_by->in_room );
			}
		    }

		    else
			obj_to_char( t_obj, obj->carried_by );
		}

		else if ( obj->in_room == NULL )  /* destroy it */
		    extract_obj( t_obj );

		else
		{
		    t_obj->arena_number = obj->arena_number;
		    obj_to_room( t_obj, obj->in_room );
		}
	    }
	}

	for ( auc = auction_list; auc != NULL; auc = auc->next )
	{
	    if ( auc->item != NULL && obj == auc->item )
	    {
		if ( auc->high_bidder != NULL )
		{
		    add_cost( auc->high_bidder, auc->bid_amount, auc->bid_type );
		    send_to_char( "\n\rYour bid has been returned to you.\n\r", auc->high_bidder );
		}

		sprintf( buff, "Auction stopped: {x%s", message );
		auction_channel( auc->owner, auc, buff );

		free_auction( auc );
		break;
	    }
	}

	for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
	{
	    bool found = FALSE;
	    sh_int pos;

	    if ( found )
		break;

	    for ( pos = 0; pos < MAX_BANK; pos++ )
	    {
		if ( wch->pcdata->storage_list[pos] == obj )
		{
		    wch->pcdata->storage_list[pos] = obj->next_content;
		    obj->next_content = NULL;
		    found = TRUE;
		    break;
		} else {
		    OBJ_DATA *prev;

		    for ( prev = wch->pcdata->storage_list[pos]; prev != NULL; prev = prev->next_content )
		    {
			if ( prev->next_content == obj )
			{
			    prev->next_content = obj->next_content;
			    obj->next_content = NULL;
			    found = TRUE;
			    break;
			}
		    }
		}
	    }
	}

	extract_obj( obj );
    }

    return;
}

void aggr_update( void )
{
    CHAR_DATA *wch, *wch_next;
    CHAR_DATA *ch, *ch_next;
    CHAR_DATA *vch, *vch_next;
    CHAR_DATA *victim;
    MEM_DATA *remember;
    char buf[MAX_STRING_LENGTH];

    for ( wch = player_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->pcdata->next_player;

	if ( wch->level >= LEVEL_IMMORTAL
	||   wch->in_room == NULL 
	||   IS_SET( wch->in_room->room_flags, ROOM_SAFE ) )
	    continue;

	for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
	{
	    int count;

	    ch_next = ch->next_in_room;

	    if ( !IS_NPC( ch )
	    ||   ch->pIndexData->pShop != NULL )
		continue;

	    if ( ( remember = get_mem_data( ch, wch ) ) != NULL
	    &&   !IS_AFFECTED( ch, AFF_CHARM ) )
	    {
		if ( IS_SET( remember->reaction, MEM_AFRAID )
		&&   wch->fighting == ch
		&&   ch->wait < PULSE_VIOLENCE / 2
		&&   number_bits( 2 ) == 1
		&&   IS_AWAKE( ch ) )
		{
		    do_say( ch, "No! I don't want to die!" );
		    do_flee( ch, "" );
		    break;
		}

                if ( IS_SET( remember->reaction, MEM_HOSTILE )
		&&   ch->fighting == NULL
		&&   wch->pcdata->dtimer <= 0
		&&   IS_AWAKE( ch )
		&&   !IS_AFFECTED( ch, AFF_CALM )
		&&   can_see( ch, wch ) )
		{
		    switch( number_range( 0, 4 ) )
		    {
			case 0:
			    sprintf( buf, "Get back here %s!  I am not finished with you!", wch->name );
			    do_say( ch, buf );
			    break;

			case 1:
			    sprintf( buf, "So, %s, You think your so tough?!", wch->name );
			    do_say( ch, buf );
			    do_say( ch, "Well lets see how you handle this!" );
			    break;

			case 2:
			    do_say( ch, "Hahaha, back for more so soon?" );
			    do_say( ch, "Well, you're not getting away so easy this time!" );
			    break;

			case 3:
			    act( "$n glares as you, as $e reconizes you and attacks!",
				ch, NULL, wch, TO_VICT, POS_RESTING );
			    act( "$n glares at $N, as $e recognizes $N entering the room.",
				ch, NULL, wch, TO_NOTVICT, POS_RESTING );
			    break;

			case 4:
			    do_say( ch, "WHAT!?! YOU again!?" );
			    break;
		    }

		    mobile_attack( ch, wch );
		    break;
		}
            }

	    if ( !IS_SET( ch->act, ACT_AGGRESSIVE )
	    ||   ch->fighting != NULL
	    ||   IS_AFFECTED( ch, AFF_CALM )
	    ||   IS_AFFECTED( ch, AFF_CHARM )
	    ||   !IS_AWAKE( ch )
	    ||   ( ch->clan > 0 && wch->clan == ch->clan )
	    ||   number_bits( 1 ) == 0
	    ||   !can_see( ch, wch ) )
		continue;

	    count	= 0;
	    victim	= NULL;

	    for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	    {
		vch_next = vch->next_in_room;

		if ( !IS_NPC( vch )
		&&   vch->level < LEVEL_IMMORTAL
		&&   ch->level >= vch->level - 5
		&&   can_see( ch, vch )
		&&   ( ch->clan == 0 || ch->clan != vch->clan ) )
		{
		    if ( number_range( 0, count ) == 0 )
			victim = vch;
		    count++;
		}
	    }

	    if ( victim == NULL )
		continue;

	    if ( ch->clan > 0 && ch->clan != victim->clan )
	    {
		sprintf( buf, "%s!  You don't belong here!", victim->name );
		do_say( ch, buf );

		sprintf( buf, "I shall defend the honor of %s {Swith my life!",
		    clan_table[ch->clan].color );
		do_say( ch, buf );

		sprintf( buf, "HELP!! We are being invaded by %s!", victim->name );
		do_clantalk( ch, buf );
	    }

	    mobile_attack( ch, victim );
	}
    }

    return;
}

void newbie_update( void )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

    if (++global_newbie > 0)
    {
	if (global_newbie == 1)
	    sprintf(buf,"Type '{CNEWBIE{c' to disable this announcement.");
	else if (global_newbie == 2)
	    sprintf(buf,"Type score [1-5] to see different displays.");
	else if (global_newbie == 3)
	    sprintf(buf,"The complex arena code has many features, see '{CHELP ARENA{c'.");
	else if (global_newbie == 4)
	    sprintf(buf,"Multiplaying is {CILLEGAL{c, don't even try it.");
	else if (global_newbie == 5)
	    sprintf(buf,"For a list of PK restrictions, see '{CHELP PK{c'.");
	else if (global_newbie == 6)
	    sprintf(buf,"We have an advanced PK system to prevent illegal interference.");
	else if (global_newbie == 7)
	    sprintf(buf,"A questmaster can be found south and two east from recall.");
	else if (global_newbie == 8)
	    sprintf(buf,"To turn this help off just type NEWBIE. :)");
	else if (global_newbie == 9)
	    sprintf(buf,"Please enjoy your stay, and take in the full value of the MUD.");
	else if (global_newbie == 10)
	    sprintf(buf,"Be sure to read '{CHELP ALIAS{c'.");
	else if (global_newbie == 11)
	    sprintf(buf,"Directions may be found by typing '{CHELP WORLD{c'.");
	else if (global_newbie == 12)
	    sprintf(buf,"Guildmasters may be located by typing '{CHELP GUILD{c'.");
	else if (global_newbie == 13)
	    sprintf(buf,"If you don't want to level, type '{CNOEXP{c'.");
	else if (global_newbie == 14)
	    sprintf(buf,"Seek the information channel for game announcements.");
	else if (global_newbie == 15)
	    sprintf(buf,"You must be level 2 to save!");
	else if (global_newbie == 16)
	    sprintf(buf,"The local stringmaster is located east and 2 north of MS.");
	else if (global_newbie == 17)
	    sprintf(buf,"The command, '{CCHECKSTATS{c', is useful and informative.");
	else if (global_newbie == 18)
	    sprintf(buf,"You may type 'train <#> <stat>' for multiple training.");
	else if (global_newbie == 19)
	    sprintf(buf,"Use the '{CCOMBAT{c' command to toggle unwanted spam.");
	else if (global_newbie == 20)
	    sprintf(buf,"'{CINDEX <string>{c' will display all matches of that string.");
	else if (global_newbie == 21)
	    sprintf(buf,"Type '{CHELP CUSTOMWHO{c' for info on changing your who.");
	else if (global_newbie == 22)
	    sprintf(buf,"Use the '{CBACKUP{c' command to create a backup pfile.");
	else if (global_newbie == 23)
	    sprintf(buf,"Type '{COUTFIT{c' if you are in need of equipment.");
	else if (global_newbie == 24)
	    sprintf(buf,"More equipment can be obtained from Mud School.");
	else if (global_newbie == 25)
	    sprintf(buf,"Read the help files for backstab and feed.");
	else
	{
	    sprintf(buf,"The PKill looting limit is 5 item points.");
	    global_newbie = 0;
	}

        for (d = descriptor_list; d != NULL; d = d->next)
        {
            if ( d->connected == CON_PLAYING && !IS_NPC(d->character)
            &&   !IS_SET(d->character->comm, COMM_NONEWBIE) )
            {
		sprintf( buf2, "{wThe t{Dable{ws of %s {wr{Dea{wd:\n\r{g<{G*{g> {y\"{c%s{y\" {g<{G*{g>{x\n\r",
		    mud_stat.mud_name_string, buf );
                send_to_char( buf2, d->character );
            }
        }
    }
    return;
}

void minute_update( void )
{
    DESC_CHECK_DATA *dc, *dc_next;
    DESCRIPTOR_DATA *d, *d_next;
    char buf[MAX_STRING_LENGTH];
    int pos;

    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;

	if ( d->connected != CON_PLAYING )
	{
	    if ( ++d->timer >= 30 )
	    {
		send_to_desc( "You have been disconnected due to inactivity.\n\r", d );
		close_socket( d );
	    }
	}

	else if ( d->character != NULL && d->character->pcdata != NULL )
	{
	    for ( pos = 0; pos < PENALTY_MAX; pos++ )
	    {
		if ( d->character->pcdata->penalty_time[pos] > 0 )
		{
		    if ( --d->character->pcdata->penalty_time[pos] == 0 )
		    {
			if ( pos == PENALTY_NOCHANNEL )
			    send_to_char( "Your channels have been returned to you.\n\r", d->character );

			else if ( pos == PENALTY_NOTITLE )
			    send_to_char( "Your title abilities have been return to you.\n\r", d->character );

			else if ( pos == PENALTY_CORNER )
			{
			    char_from_room( d->character );
			    char_to_room( d->character, get_room_index( ROOM_VNUM_ALTAR ) );
			    do_look( d->character, "auto" );
			    send_to_char( "Your punishment has expired.\n\r", d->character );
			}

			else if ( pos == PENALTY_FREEZE )
			    send_to_char( "You begin to thaw out.\n\r", d->character );

			else if ( pos == PENALTY_NORESTORE )
			    send_to_char( "You can now receive restores again.\n\r", d->character );

			else if ( pos == PENALTY_NOTELL )
			    send_to_char( "You can send tells again.\n\r", d->character );
		    }
		}
	    }

	    if ( d->character->pcdata->recent_pkills )
	    {
		PKILL_DATA *pkill, *pkill_next;

		for ( pkill = d->character->pcdata->recent_pkills; pkill != NULL; pkill = pkill_next )
		{
		    pkill_next = pkill->next;

		    if ( difftime( current_time, pkill->time ) > 900 )
		    {
			sprintf( buf, "{RYour protection from %s has faded!{x\n\r", pkill->player_name );
			send_to_char( buf, d->character );

			if ( pkill == d->character->pcdata->recent_pkills )
			    d->character->pcdata->recent_pkills = pkill->next;
			else
			{
			    PKILL_DATA *list;

			    for ( list = d->character->pcdata->recent_pkills; list != NULL; list = list->next )
			    {
				if ( list->next == pkill )
				{
				    list->next = pkill->next;
				    break;
				}
			    }
			}

			free_pkill( pkill );
		    }
		}
	    }
	}
    }

    for ( dc = desc_check_list; dc != NULL; dc = dc_next )
    {
	dc_next = dc->next;

	if ( --dc->wait_time <= 0 )
	{
	    if ( dc == desc_check_list )
		desc_check_list = desc_check_list->next;
	    else
	    {
		DESC_CHECK_DATA *prev;

		for ( prev = desc_check_list; prev != NULL; prev = prev->next )
		{
		    if ( prev->next == dc )
		    {
			prev->next = dc->next;
			break;
		    }
		}
	    }

	    free_desc_check( dc );
	}
    }

    for ( pos = 0; clan_table[pos].name[0] != '\0'; pos++ )
    {
	if ( clan_table[pos].edit_clan > 0 )
	{
	    clan_table[pos].edit_clan--;
	    mud_stat.clans_changed = TRUE;
	}

	if ( clan_table[pos].edit_room > 0 )
	{
	    clan_table[pos].edit_room--;
	    mud_stat.clans_changed = TRUE;
	}

	if ( clan_table[pos].edit_mob > 0 )
	{
	    clan_table[pos].edit_mob--;
	    mud_stat.clans_changed = TRUE;
	}

	if ( clan_table[pos].edit_obj > 0 )
	{
	    clan_table[pos].edit_obj--;
	    mud_stat.clans_changed = TRUE;
	}

	if ( clan_table[pos].edit_help > 0 )
	{
	    clan_table[pos].edit_help--;
	    mud_stat.clans_changed = TRUE;
	}

	if ( clan_table[pos].two_way_time > 0 )
	{
	    if ( --clan_table[pos].two_way_time <= 0 )
	    {
		if ( has_two_way( pos ) == 2 )
		    auto_two_way( pos );
	    }

	    mud_stat.clans_changed = TRUE;
	}
    }
}

void pktimer_update( void )
{
    CHAR_DATA *wch;

    for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
    {
	if ( wch->pcdata->pktimer > 0
	&&   --wch->pcdata->pktimer == 0 )
	{
	    send_to_char("{xYour heart stops pounding.{x\n\r",wch);
	    wch->pcdata->opponent = NULL;
	    wch->pcdata->attacker = FALSE;
	}
    }
    return;
}

void room_update( void )
{
    CHAR_DATA *wch;
    ROOM_DAMAGE_DATA *dam;

    for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
    {
	if ( wch->in_room == NULL
	||   wch->pcdata->dtimer > 0
	||   IS_IMMORTAL( wch ) )
	    continue;

	for ( dam = wch->in_room->room_damage; dam != NULL; dam = dam->next )
	{
	    if ( wch->damage_mod[dam->damage_type] <= 0
	    ||   number_percent( ) > dam->success )
		continue;

	    if ( dam->msg_victim[0] != '\0' )
		act( dam->msg_victim, wch, NULL, NULL, TO_CHAR, POS_DEAD );

	    if( dam->msg_room[0] != '\0' )
		act( dam->msg_room, wch, NULL, NULL, TO_ROOM, POS_RESTING );

	    damage( wch, wch, number_range( dam->damage_min, dam->damage_max ),
		 0, dam->damage_type, FALSE, FALSE, NULL );
	    update_pos( wch );
	}
    }
}

void random_restore( int type )
{
    CHAR_DATA *ch;

    for ( ch = player_list; ch != NULL; ch = ch->pcdata->next_player )
    {
	if ( ch->pcdata->penalty_time[PENALTY_NORESTORE] == 0
	&&   ch->pcdata->opponent == NULL
	&&   ch->pcdata->pktimer <= 0
	&&   ch->in_room != NULL
	&&   !IS_SET( ch->in_room->room_flags, ROOM_ARENA )
	&&   !IS_SET( ch->in_room->room_flags, ROOM_WAR ) )
	{
	    switch( type )
	    {
		default:
		    ch->hit  = ch->max_hit;
		    ch->mana = ch->max_mana;
		    ch->move = ch->max_move;
		    act( "$t has revitalized you!",
			ch, mud_stat.mud_name_string, NULL, TO_CHAR, POS_DEAD );
		    break;

		case VALUE_HIT_POINT:
		    ch->hit = ch->max_hit;
		    act( "$G {Rheals your wounds!{x",
			ch, NULL, NULL, TO_CHAR, POS_DEAD );
		    break;

		case VALUE_MANA_POINT:
		    ch->mana = ch->max_mana;
		    act( "$G {Menergizes you!{x",
			ch, NULL, NULL, TO_CHAR, POS_DEAD );
		    break;

		case VALUE_MOVE_POINT:
		    ch->move = ch->max_move;
		    act( "$G {Crefreshes you!{x",
			ch, NULL, NULL, TO_CHAR, POS_DEAD );
		    break;
	    }

	    update_pos( ch );
	}
    }
}

void update_handler( bool forced )
{
    static  int     pulse_area;
    static  int     pulse_mobile;
    static  int     pulse_auction;
    static  int     pulse_violence;
    static  int     pulse_point;
    static  int     pulse_minute;
    static  int	    pulse_quest;
    static  int	    pulse_pktimer;
    static  int     pulse_newbie;
    static  int     pulse_second;

    if ( --pulse_second <= 0 ) {
     continue_executions();  // SimpleScripts for Diku
     pulse_second=PULSE_PER_SECOND;
    }

    if ( --pulse_area <= 0 )
    {
	pulse_area = number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
	area_update( );
	save_special_items( );
        fwrite_rooms();
	do_asave( NULL, "changed" );

	if ( number_percent( ) == 1 )
	    random_restore( -1 );

	else
	{
	    if ( number_percent( ) < 2 )
		random_restore( VALUE_HIT_POINT );

	    if ( number_percent( ) < 2 )
		random_restore( VALUE_MANA_POINT );

	    if ( number_percent( ) < 2 )
		random_restore( VALUE_MOVE_POINT );
	}
    }

    if ( --pulse_quest <= 0 )
    {
	pulse_quest = PULSE_QUEST;
	quest_update( );
    }

    if ( --pulse_pktimer <= 0 )
    {
	pulse_pktimer = PULSE_PKTIMER;
	pktimer_update( );
    }

    if ( --pulse_mobile <= 0 )
    {
	CHAR_DATA *wch;

	pulse_mobile = PULSE_MOBILE;
	mobile_update( );
	room_update( );

	for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
	{
	    if ( wch->pcdata->spam_count > 0 )
		wch->pcdata->spam_count--;
	}
    }

    if ( --pulse_violence <= 0 )
    {
	pulse_violence = PULSE_VIOLENCE;
	violence_update( );
    }

    if ( --pulse_auction <= 0 )
    {
	pulse_auction = PULSE_AUCTION;
	auction_update( );
    }

    if ( --pulse_newbie  <= 0 )
    {
	pulse_newbie = PULSE_NEWBIE;
	newbie_update( );
    }

    if ( --pulse_minute <= 0 )
    {
	pulse_minute = PULSE_MINUTE;
	minute_update( );
	gquest_update( );
    }

    if ( --pulse_point <= 0 || forced )
    {
	wiznet( "TICK!", NULL, NULL, WIZ_TICKS, 0, 0 );
	pulse_point = number_range( PULSE_TICK / 2, 3 * PULSE_TICK / 2 );
	weather_update( );
	char_update( );
	obj_update( );
	player_update( );
    }

    aggr_update( );
    tail_chain( );
    return;
}

