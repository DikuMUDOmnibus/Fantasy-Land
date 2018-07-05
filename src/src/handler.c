/**************************************************************************r
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
#include "recycle.h"

DECLARE_DO_FUN( do_return	);

DECLARE_SPELL_FUN( spell_null	);

sh_int	get_skill_level	args( ( CHAR_DATA *ch, sh_int sn ) );

sh_int dam_type_lookup( char *argument )
{
    sh_int dam;

    if ( argument[0] == '\0' )
	return -1;

    for ( dam = 0; dam < DAM_MAX; dam++ )
    {
	if ( !str_prefix( argument, damage_mod_table[dam].name ) )
	    return dam;
    }

    return -1;
}

sh_int count_users( OBJ_DATA *obj )
{
    CHAR_DATA *fch;
    sh_int count = 0;

    if ( obj->in_room == NULL )
	return 0;

    for ( fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room )
    {
	if ( fch->on == obj )
	    count++;
    }

    return count;
}

sh_int race_lookup( const char *name )
{
    sh_int race;

    for ( race = 0; race < maxRace; race++ )
    {
	if ( LOWER( name[0] ) == LOWER( race_table[race].name[0] )
	&&   !str_prefix( name, race_table[race].name ) )
	    return race;
    }

    return -1;
}

sh_int liq_lookup( const char *name )
{
    sh_int liq;

    for ( liq = 0; liq_table[liq].liq_name != NULL; liq++ )
    {
	if ( LOWER( name[0] ) == LOWER( liq_table[liq].liq_name[0] )
	&&   !str_prefix( name, liq_table[liq].liq_name ) )
	    return liq;
    }

    return -1;
}

sh_int weapon_type( const char *name )
{
    sh_int type;

    for ( type = 0; weapon_table[type].name != NULL; type++ )
    {
	if ( LOWER( name[0] ) == LOWER( weapon_table[type].name[0] )
	&&   !str_prefix( name, weapon_table[type].name ) )
	    return weapon_table[type].type;
    }

    return WEAPON_EXOTIC;
}

sh_int attack_lookup( const char *name )
{
    sh_int att;

    for ( att = 0; attack_table[att].name != NULL; att++ )
    {
	if ( LOWER( name[0] ) == LOWER( attack_table[att].name[0] )
	&&   !str_prefix( name, attack_table[att].name ) )
	    return att;
    }

    return 0;
}

sh_int wiznet_lookup( const char *name )
{
    sh_int flag;

    for ( flag = 0; wiznet_table[flag].name != NULL; flag++ )
    {
	if ( LOWER( name[0] ) == LOWER( wiznet_table[flag].name[0] )
	&&   !str_prefix( name, wiznet_table[flag].name ) )
	    return flag;
    }

    return -1;
}

sh_int info_lookup( const char *name )
{
    sh_int flag;

    for ( flag = 0; info_table[flag].name != NULL; flag++ )
    {
	if ( LOWER( name[0] ) == LOWER( info_table[flag].name[0] )
	&&   !str_prefix( name, info_table[flag].name ) )
	    return flag;
    }

    return -1;
}

sh_int combat_lookup( const char *name )
{
    sh_int flag;

    for ( flag = 0; combat_table[flag].name != NULL; flag++ )
    {
	if ( LOWER( name[0] ) == LOWER( combat_table[flag].name[0] )
	&&   !str_prefix( name, combat_table[flag].name ) )
	    return flag;
    }

    return -1;
}

sh_int class_lookup( const char *name )
{
    sh_int class;

    for ( class = 0; class_table[class].name[0] != '\0'; class++ )
    {
	if ( LOWER( name[0] ) == LOWER( class_table[class].name[0] )
	&&   !str_prefix( name, class_table[class].name ) )
	    return class;
    }

    return -1;
}

bool is_clan( CHAR_DATA *ch )
{
    return ch->clan;
}

bool is_pkill( CHAR_DATA *ch )
{
    if ( is_clan( ch ) )
	return clan_table[ch->clan].pkill;
    else
	return FALSE;
}

bool is_same_clan( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( clan_table[ch->clan].independent )
	return FALSE;
    else
	return ( ch->clan == victim->clan );
}

sh_int get_skill( CHAR_DATA *ch, sh_int sn )
{
    sh_int skill;

    if ( sn < 0 || sn > maxSkill )
    {
	sprintf( log_buf, "Bad sn by %s of %d in get_skill.", ch->name, sn );
	bug( log_buf, 0 );
	skill = 0;
    }

    else if ( IS_SET( skill_table[sn].flags, SKILL_DISABLED ) )
    {
	char buf[100];

	sprintf( buf, "{RSkill '%s' disabled.{x\n\r", skill_table[sn].name );
	send_to_char( buf, ch );
	skill = 0;
    }

    else if ( !IS_NPC( ch )
    &&   ch->pcdata->match != NULL
    &&   IS_SET( ch->in_room->room_flags, ROOM_ARENA )
    &&   ch->pcdata->match->disabled_skills[sn] == TRUE )
    {
	char buf[100];

	sprintf( buf, "{RSkill '%s' disabled for this match.{x\n\r", skill_table[sn].name );
	send_to_char( buf, ch );
	skill = 0;
    }

    else
    {
	if ( ( ch->pIndexData != NULL
	&&     get_skill_level( ch, sn ) > LEVEL_HERO
	&&     ch->learned[sn] == ch->pIndexData->skill_percentage )
	||   ch->level < get_skill_level( ch, sn ) )
	    skill = 0;

	else
	    skill = ch->learned[sn];
    }

    if ( skill_table[sn].spell_fun != spell_null )
    {
	if ( ch->daze[DAZE_SPELL] > 0 )
	    skill = 2 * skill / 3;
    }

    else if ( ch->daze[DAZE_SKILL] > 0 )
	skill = 2 * skill / 3;

    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	skill = 9 * skill / 10;

    return URANGE( 0, skill, 100 );
}

sh_int get_weapon_sn( CHAR_DATA *ch, bool secondary )
{
    OBJ_DATA *wield;
    sh_int sn;

    if ( !secondary )
	wield = get_eq_char( ch, WEAR_WIELD );
    else
	wield = get_eq_char( ch, WEAR_SECONDARY );

    if ( wield == NULL || wield->item_type != ITEM_WEAPON )
	sn = gsn_hand_to_hand;

    else switch ( wield->value[0] )
    {
	default:			sn = -1;		break;
	case WEAPON_SWORD:		sn = gsn_sword;		break;
	case WEAPON_DAGGER:		sn = gsn_dagger;	break;
	case WEAPON_SPEAR:		sn = gsn_spear;		break;
	case WEAPON_MACE:		sn = gsn_mace;		break;
	case WEAPON_AXE:		sn = gsn_axe;		break;
	case WEAPON_FLAIL:		sn = gsn_flail;		break;
	case WEAPON_WHIP:		sn = gsn_whip;		break;
	case WEAPON_POLEARM:		sn = gsn_polearm;	break;
	case WEAPON_QUARTERSTAFF:	sn = gsn_quarterstaff;	break;
   }

   return sn;
}

sh_int get_weapon_skill( CHAR_DATA *ch, sh_int sn )
{
    sh_int skill;

    /* -1 is exotic */
    if ( IS_NPC( ch ) )
    {
	if ( sn == -1 )
	    skill = 3 * ch->level;
	else if ( sn == gsn_hand_to_hand )
	    skill = 40 + 2 * ch->level;
	else
	    skill = 40 + 5 * ch->level / 2;
    }

    else
    {
	if ( sn == -1 )
	    skill = 3 * ch->level;
	else
	    skill = ch->learned[sn];
    }

    return URANGE( 0, skill, 100 );
}

void affect_mods_reset( CHAR_DATA *ch, AFFECT_DATA *af )
{
    sh_int i, mod;

    for ( ; af != NULL; af = af->next )
    {
	mod = af->modifier;

	if ( af->where == TO_DAM_MODS )
	{
	    if ( af->location == DAM_ALL )
	    {
		for ( i = 0; i < DAM_MAX; i++ )
		    ch->damage_mod[i] += mod;
	    } else
		ch->damage_mod[af->location] += mod;
	} else {
	    switch( af->location )
	    {
		case APPLY_STR:		ch->mod_stat[STAT_STR]	+= mod;	break;
		case APPLY_DEX:		ch->mod_stat[STAT_DEX]	+= mod; break;
		case APPLY_INT:		ch->mod_stat[STAT_INT]	+= mod; break;
		case APPLY_WIS:		ch->mod_stat[STAT_WIS]	+= mod; break;
		case APPLY_CON:		ch->mod_stat[STAT_CON]	+= mod; break;
		case APPLY_SEX:		ch->sex			+= mod; break;
		case APPLY_MANA:	ch->max_mana		+= mod; break;
		case APPLY_HIT:		ch->max_hit		+= mod; break;
		case APPLY_MOVE:	ch->max_move		+= mod; break;
		case APPLY_HITROLL:	ch->hitroll		+= mod; break;
		case APPLY_DAMROLL:	ch->damroll		+= mod; break;
		case APPLY_SAVES:	ch->saving_throw 	+= mod; break;
		case APPLY_SIZE:	ch->size	 	+= mod; break;
		case APPLY_MAGIC_POWER:	ch->magic_power		+= mod;	break;
		case APPLY_REGEN_HP:	ch->regen[0]		+= mod;	break;
		case APPLY_REGEN_MANA:	ch->regen[1]		+= mod;	break;
		case APPLY_REGEN_MOVE:	ch->regen[2]		+= mod;	break;
		case APPLY_AC:
		    for ( i = 0; i < 4; i ++ )
			ch->armor[i] += mod;
		    break;

		case APPLY_DAMAGE:
		    for ( i = 0; i < 4; i ++ )
			ch->armor[i] += mod;
		    ch->hitroll -= mod;
		    ch->damroll -= mod;
		    break;
	    }
	}
    }
}

void reset_char( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    int i, loc;

    if ( IS_NPC( ch ) )
	return;

    for ( i = 0; i < MAX_STATS; i++ )
	ch->mod_stat[i] = 0;

    for ( i = 0; i < DAM_MAX; i++ )
	ch->damage_mod[i] = race_table[ch->race].damage_mod[i];

    if ( ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2 )
	ch->pcdata->true_sex = 0;

    for ( i = 0; i < 4; i++ )
    	ch->armor[i]	= 100;

    ch->sex		= ch->pcdata->true_sex;
    ch->max_hit 	= ch->pcdata->perm_hit;
    ch->max_mana	= ch->pcdata->perm_mana;
    ch->max_move	= ch->pcdata->perm_move;
    ch->hitroll		= 0;
    ch->damroll		= 0;
    ch->saving_throw	= 0;
    ch->magic_power	= ch->level;

    for ( loc = 0; loc < MAX_WEAR; loc++ )
    {
	if ( ( obj = get_eq_char( ch, loc ) ) == NULL )
	    continue;

	for ( i = 0; i < 4; i++ )
	    ch->armor[i] -= apply_ac( obj, loc, i );

	if ( !obj->enchanted )
	    affect_mods_reset( ch, obj->pIndexData->affected );

	affect_mods_reset( ch, obj->affected );
    }

    affect_mods_reset( ch, ch->affected );

    if ( ch->sex < 0 || ch->sex > 2 )
	ch->sex = ch->pcdata->true_sex;
}

sh_int get_trust( CHAR_DATA *ch )
{
    if ( ch == NULL )
	return 0;

    if ( ch->desc != NULL && ch->desc->original != NULL )
    {
	log_string( "Using desc original.\n\r" );
	ch = ch->desc->original;
    }

    if ( ch == NULL )
	log_string( "WTF!\n\r" );

    if ( ch->trust != 0 )
	return ch->trust;

    if ( IS_NPC( ch ) && ch->level >= LEVEL_HERO )
	return LEVEL_HERO;

    return ch->level;
}

sh_int get_age( CHAR_DATA *ch )
{
    if ( ch->pcdata == NULL )
	return 0;

    return 17 + ( ch->pcdata->played + ( int ) ( current_time - ch->pcdata->logon ) ) / 72000;
}

sh_int get_curr_stat( CHAR_DATA *ch, sh_int stat )
{
    sh_int max;

    if ( IS_NPC( ch ) || ch->level > LEVEL_IMMORTAL )
	max = 35;

    else
    {
	max = race_table[ch->race].max_stats[stat] + 8;

	if ( class_table[ch->class].attr_prime == stat )
	    max += 2;

 	max = UMIN( max, 35 );
    }

    return URANGE( 0, ch->perm_stat[stat] + ch->mod_stat[stat], max );
}

sh_int get_max_train( CHAR_DATA *ch, sh_int stat )
{
    sh_int max;

    if ( IS_NPC( ch ) || ch->level > LEVEL_IMMORTAL )
	return 35;

    max = race_table[ch->race].max_stats[stat];
    if ( class_table[ch->class].attr_prime == stat )
	max += 2;

    return UMIN( max, 35 );
}

sh_int can_carry_n( CHAR_DATA *ch )
{
    if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
	return 1000;

    if ( IS_NPC( ch ) && IS_SET( ch->act, ACT_PET ) )
	return 0;

    return MAX_WEAR + 2 * get_curr_stat( ch, STAT_DEX ) + ( 2 * ch->level / 3 );
}

int can_carry_w( CHAR_DATA *ch )
{
    if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
	return 10000000;

    if ( IS_NPC( ch ) && IS_SET( ch->act, ACT_PET ) )
	return 0;

    return str_app[get_curr_stat( ch, STAT_STR )].carry * 10 + ch->level * 20;
}

bool is_name( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;

    string = str;

    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )
    {
	str = one_argument( str, part );

	if ( part[0] == '\0' )
	    return TRUE;

	/* check to see if this is part of namelist */
	list = namelist;
	for ( ; ; )
	{
	    list = one_argument( list, name );
	    if ( name[0] == '\0' )  /* this name was not found */
		return FALSE;

	    if ( !str_prefix( string, name ) )
		return TRUE; /* full pattern match */

	    if ( !str_prefix( part, name ) )
		break;
	}
    }
}

bool is_exact_name( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;

    string = str;

    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
	str = one_argument( str, part );

	if ( part[0] == '\0' )
	    return TRUE;

	/* check to see if this is part of namelist */
	list = namelist;
	for ( ; ; )  /* start parsing namelist */
	{
	    list = one_argument( list, name );
	    if ( name[0] == '\0' )  /* this name was not found */
		return FALSE;

	    if ( !str_cmp( string, name ) )
		return TRUE; /* full pattern match */

	    if ( !str_cmp( part, name ) )
		break;
	}
    }
}

void affect_enchant( OBJ_DATA *obj )
{
    if ( !obj->enchanted )
    {
	AFFECT_DATA *paf, *af_new;

	obj->enchanted = TRUE;

	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	{
	    af_new		= new_affect( );
	    af_new->where	= paf->where;
	    af_new->type	= UMAX( 0, paf->type );
	    af_new->level	= paf->level;
	    af_new->dur_type	= paf->dur_type;
	    af_new->duration	= paf->duration;
	    af_new->location	= paf->location;
	    af_new->modifier	= paf->modifier;
	    af_new->bitvector	= paf->bitvector;
	    af_new->next	= obj->affected;
	    obj->affected	= af_new;
	}
    }
}

void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    sh_int mod, i;

    mod = paf->modifier;

    if ( fAdd )
    {
	switch( paf->where )
	{
	    default:
		break;

	    case TO_AFFECTS:
		SET_BIT( ch->affected_by, paf->bitvector );
		break;

	    case TO_SHIELDS:
		SET_BIT( ch->shielded_by, paf->bitvector );
		break;

	    case TO_DAM_MODS:
		if ( paf->location == DAM_ALL )
		{
		    for ( i = 0; i < DAM_MAX; i++ )
			ch->damage_mod[i] += paf->modifier;
		} else
		    ch->damage_mod[paf->location] += paf->modifier;
		return;
	}
    } else {
	switch( paf->where )
	{
	    default:
		break;

	    case TO_AFFECTS:
		REMOVE_BIT( ch->affected_by, paf->bitvector );
		break;

	    case TO_SHIELDS:
		REMOVE_BIT( ch->shielded_by, paf->bitvector );
		break;

	    case TO_DAM_MODS:
		if ( paf->location == DAM_ALL )
		{
		    for ( i = 0; i < DAM_MAX; i++ )
			ch->damage_mod[i] -= paf->modifier;
		} else
		    ch->damage_mod[paf->location] -= paf->modifier;
		return;
	}
	mod = 0 - mod;
    }

    switch( paf->location )
    {
	default:
	    sprintf( log_buf, "Affect_modify: %s: unknown location %d.",
		ch->name, paf->location );
	    bug( log_buf, 0 );
	    return;

	case APPLY_NONE:					break;
	case APPLY_STR:		ch->mod_stat[STAT_STR]	+= mod;	break;
	case APPLY_DEX:		ch->mod_stat[STAT_DEX]	+= mod;	break;
	case APPLY_INT:		ch->mod_stat[STAT_INT]	+= mod;	break;
	case APPLY_WIS:		ch->mod_stat[STAT_WIS]	+= mod;	break;
	case APPLY_CON:		ch->mod_stat[STAT_CON]	+= mod;	break;
	case APPLY_SEX:		ch->sex			+= mod;	break;
	case APPLY_MANA:	ch->max_mana		+= mod;	break;
	case APPLY_HIT:		ch->max_hit		+= mod;	break;
	case APPLY_MOVE:	ch->max_move		+= mod;	break;
	case APPLY_HITROLL:	ch->hitroll		+= mod;	break;
	case APPLY_DAMROLL:	ch->damroll		+= mod;	break;
	case APPLY_SAVES:	ch->saving_throw	+= mod;	break;
	case APPLY_SIZE:	ch->size		+= mod; break;
	case APPLY_MAGIC_POWER:	ch->magic_power		+= mod;	break;
	case APPLY_REGEN_HP:	ch->regen[0]		+= mod;	break;
	case APPLY_REGEN_MANA:	ch->regen[1]		+= mod;	break;
	case APPLY_REGEN_MOVE:	ch->regen[2]		+= mod;	break;
	case APPLY_DAMAGE:
	    for ( i = 0; i < 4; i ++ )
		ch->armor[i]	+= mod;
	    ch->hitroll		-= mod;
	    ch->damroll		-= mod;
	    break;
	case APPLY_AC:
	    for ( i = 0; i < 4; i++ )
		ch->armor[i] += mod;
	    break;
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( !IS_NPC( ch ) && ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
    &&   get_obj_weight( wield ) > ( str_app[get_curr_stat(ch,STAT_STR)].wield * 10 ) )
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act( "You drop $p.", ch, wield, NULL, TO_CHAR, POS_RESTING );
	    act( "$n drops $p.", ch, wield, NULL, TO_ROOM, POS_RESTING );

	    obj_from_char( wield );

	    if ( IS_OBJ_STAT( wield, ITEM_NODROP )
	    ||   IS_OBJ_STAT( wield, ITEM_INVENTORY )
	    ||   IS_OBJ_STAT( wield, ITEM_AQUEST )
	    ||   IS_OBJ_STAT( wield, ITEM_FORGED ) )
		obj_to_char( wield, ch );
	    else
	    {
		set_obj_sockets( ch, wield );
		wield->disarmed_from = ch;
		set_arena_obj( ch, wield );
		obj_to_room( wield, ch->in_room );
	    }
	    depth--;
	}
    }

    if ( !IS_NPC( ch ) && ( wield = get_eq_char( ch, WEAR_SECONDARY ) ) != NULL
    &&   get_obj_weight( wield ) > ( str_app[get_curr_stat(ch,STAT_STR)].wield * 10 ) )
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act( "You drop $p.", ch, wield, NULL, TO_CHAR, POS_RESTING );
	    act( "$n drops $p.", ch, wield, NULL, TO_ROOM, POS_RESTING );

	    obj_from_char( wield );

	    if ( IS_OBJ_STAT( wield, ITEM_NODROP )
	    ||   IS_OBJ_STAT( wield, ITEM_INVENTORY )
	    ||   IS_OBJ_STAT( wield, ITEM_AQUEST )
	    ||   IS_OBJ_STAT( wield, ITEM_FORGED ) )
		obj_to_char( wield, ch );
	    else
	    {
		set_obj_sockets( ch, wield );
		wield->disarmed_from = ch;
		set_arena_obj( ch, wield );
		obj_to_room( wield, ch->in_room );
	    }
	    depth--;
	}
    }

    return;
}

AFFECT_DATA *affect_find( AFFECT_DATA *paf, sh_int sn )
{
    AFFECT_DATA *paf_find;

    for ( paf_find = paf; paf_find != NULL; paf_find = paf_find->next )
    {
	if ( paf_find->type == sn )
	    return paf_find;
    }

    return NULL;
}

void affect_check( CHAR_DATA *ch, sh_int where, int vector )
{
    AFFECT_DATA *paf;

    if ( where == TO_OBJECT || where == TO_WEAPON || vector == 0 )
	return;

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->where == where && paf->bitvector & vector )
	{
	    switch( where )
	    {
		default:
		    break;

		case TO_AFFECTS:
		    SET_BIT( ch->affected_by, paf->bitvector );
		    break;

		case TO_SHIELDS:
		    SET_BIT( ch->shielded_by, paf->bitvector );
		    break;
	    }

	    return;
	}
    }
}

void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new		= new_affect( );
    *paf_new		= *paf;
    paf_new->next	= ch->affected;
    ch->affected	= paf_new;

    affect_modify( ch, paf_new, TRUE );
    return;
}

void affect_to_obj( OBJ_DATA *obj, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new		= new_affect( );
    *paf_new		= *paf;
    paf_new->next	= obj->affected;
    obj->affected	= paf_new;

    if ( paf->bitvector )
    {
	switch( paf->where )
	{
	    default:
		return;

	    case TO_OBJECT:
		SET_BIT( obj->extra_flags, paf->bitvector );
		break;

	    case TO_WEAPON:
		if ( obj->item_type == ITEM_WEAPON )
		    SET_BIT( obj->value[4], paf->bitvector );
		break;
	}
    }

    return;
}

void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    sh_int where;
    int vector;

    if ( ch->affected == NULL )
    {
	bug( "Affect_remove: no affect.", 0 );
	return;
    }

    affect_modify( ch, paf, FALSE );

    where	= paf->where;
    vector	= paf->bitvector;

    if ( paf == ch->affected )
	ch->affected = paf->next;
    else
    {
	AFFECT_DATA *prev;

	for ( prev = ch->affected; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == paf )
	    {
		prev->next = paf->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Affect_remove: cannot find paf.", 0 );
	    return;
	}
    }

    free_affect( paf );
    affect_check( ch, where, vector );
    return;
}

void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf )
{
    sh_int where;
    int vector;

    if ( obj->affected == NULL )
    {
	bug( "Affect_remove_object: no affect.", 0 );
	return;
    }

    if ( obj->carried_by != NULL && obj->wear_loc != -1 )
	affect_modify( obj->carried_by, paf, FALSE );

    where	= paf->where;
    vector	= paf->bitvector;

    if ( paf->bitvector )
    {
	switch( paf->where )
	{
	    default:
		break;

	    case TO_OBJECT:
		REMOVE_BIT( obj->extra_flags, paf->bitvector );
		break;

	    case TO_WEAPON:
		if ( obj->item_type == ITEM_WEAPON )
		    REMOVE_BIT( obj->value[4], paf->bitvector );
		break;
	}
    }

    if ( paf == obj->affected )
        obj->affected = paf->next;
    else
    {
	AFFECT_DATA *prev;

	for ( prev = obj->affected; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == paf )
	    {
		prev->next = paf->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Affect_remove_object: cannot find paf.", 0 );
	    return;
	}
    }

    free_affect( paf );

    if ( obj->carried_by != NULL && obj->wear_loc != -1 )
	affect_check( obj->carried_by, where, vector );
    return;
}

void affect_strip( CHAR_DATA *ch, sh_int sn )
{
    AFFECT_DATA *paf, *paf_next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;

	if ( paf->type == sn )
	    affect_remove( ch, paf );
    }

    return;
}

void perm_affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf, *paf_next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;

	if ( paf->type == sn && paf->duration == -1 )
	{
	    int where	= paf->where;
	    int vector	= paf->bitvector;

	    affect_remove( ch, paf );
	    affect_check( ch, where, vector );

	    if ( skill_table[sn].msg_off && skill_table[sn].msg_off[0] )
	    {
		send_to_char( skill_table[sn].msg_off, ch );
		send_to_char( "\n\r", ch );
	    }

	    if ( skill_table[sn].room_msg && skill_table[sn].room_msg[0] )
		act( skill_table[sn].room_msg,
		    ch, NULL, NULL, TO_ROOM, POS_RESTING );
	}
    }

    return;
}

bool is_affected( CHAR_DATA *ch, sh_int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}

void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;

    for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    {
	if ( paf_old->type == paf->type )
	{
	    paf->level = (paf->level += paf_old->level) / 2;
	    paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    affect_to_char( ch, paf );
    return;
}

void char_from_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ch->in_room == NULL )
    {
	bug( "Char_from_room: NULL.", 0 );
	return;
    }

    if ( !IS_NPC( ch ) )
	--ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    if ( ch == ch->in_room->people )
	ch->in_room->people = ch->next_in_room;
    else
    {
	CHAR_DATA *prev;

	for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
	{
	    if ( prev->next_in_room == ch )
	    {
		prev->next_in_room = ch->next_in_room;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Char_from_room: ch not found.", 0 );
    }

    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    ch->on 	     = NULL;
    return;
}

void check_trap( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( ch == NULL )
    {
	bug( "check_trap: NULL ch", 0 );
	return;
    }

    if ( obj == NULL )
    {
	bug( "check_trap: NULL obj", 0 );
	return;
    }

    if ( obj->dropped_by == NULL )
    {
	bug( "check_trap: obj->dropped_by == NULL", obj->pIndexData->vnum );
	return;
    }

    if ( obj->dropped_by != ch && !can_pk( ch, obj->dropped_by ) )
	return;
    
    if ( can_see_obj( ch, obj )
    &&   number_percent( ) > 60
    &&   obj->dropped_by != ch )
        return;
    
    act( "$p springs up and hits you!",
	ch, obj, NULL, TO_CHAR, POS_RESTING );

    act( "$p springs up and hits $n!",
	ch, obj, NULL, TO_ROOM, POS_RESTING );

    if ( obj->value[0] == TRAP_DAMAGE )
    {
	damage( obj->dropped_by, ch, number_range( obj->value[2], obj->value[3] ),
	    gsn_trapset, obj->value[1], TRUE, FALSE, NULL );
    }

    else if ( obj->value[0] == TRAP_MANA )
    {
	ch->mana -= number_range( obj->value[2], obj->value[3] );
	if ( ch->mana < 0 )
	    ch->mana = 0;
    }

    else if ( obj->value[0] == TRAP_TELEPORT )
    {
	act( "{R$p {Rsends you flying through space!{x\n\r",
	    ch, obj, NULL, TO_CHAR, POS_RESTING );

	char_from_room( ch );
	char_to_room( ch, get_random_room( ch ) );
    }

    else if ( obj->value[0] == TRAP_LATCH )
    {
	AFFECT_DATA af;

	af.where	= TO_AFFECTS;
	af.type 	= gsn_trapset;
	af.level 	= obj->level;
	af.dur_type	= DUR_ROUNDS;
	af.duration	= 6;
	af.location	= APPLY_DEX;
	af.modifier	= -10;
	af.bitvector 	= 0;
	af.next		= NULL;

	affect_to_char( ch, &af );

	ch->move = ch->move * 3 / 4;
	act( "$p laches onto you!",
	    ch, obj, NULL, TO_CHAR, POS_RESTING );
    }

    if ( ch != NULL )
	WAIT_STATE( ch, PULSE_VIOLENCE );

    obj_from_room( obj );
    extract_obj( obj );
}

void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;

    if ( pRoomIndex == NULL )
    {
	ROOM_INDEX_DATA *room;

	bug( "Char_to_room: NULL.", 0 );

	if ( ch->alignment < 0 )
	{
	    if ((room = get_room_index(ROOM_VNUM_TEMPLEB)) != NULL)
		char_to_room(ch,room);
	}
	else
	{
	    if ((room = get_room_index(ROOM_VNUM_TEMPLE)) != NULL)
		char_to_room(ch,room);
	}

	return;
    }

    if ( IS_AFFECTED(ch, AFF_HIDE) )
	REMOVE_BIT(ch->affected_by, AFF_HIDE);

    ch->in_room		= pRoomIndex;
    ch->next_in_room	= pRoomIndex->people;
    pRoomIndex->people	= ch;

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == OBJ_VNUM_FOG )
	{
	    bool found = FALSE;

	    if ( is_affected(ch,gsn_obfuscate) && number_percent() < 90 )
	    {
		affect_strip(ch,gsn_obfuscate);
		found = TRUE;
	    }

	    if ( is_affected(ch,gsn_invis) && number_percent() < 90 )
	    {
		affect_strip(ch,gsn_invis);
		found = TRUE;
	    }

	    if ( is_affected(ch,gsn_forest_meld) && number_percent() < 90 )
	    {
		affect_strip(ch,gsn_forest_meld);
		found = TRUE;
	    }

	    if ( found && number_percent() < 85 )
		act("You are exposed by $p!",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    break;
	}

	if ( obj->item_type == ITEM_TRAP
	&&   obj->dropped_by != ch
	&&   !is_same_group(ch, obj->dropped_by)
	&&   !IS_SET( obj->wear_flags, ITEM_TAKE ) )
	{
	    check_trap( ch, obj );
	    
	    if ( ch->in_room == NULL )
	        return;
	}
    }

    if ( !IS_NPC(ch) )
	++ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0 )
	++ch->in_room->light;

    if ( ch->in_room->music != NULL && str_cmp( ch->in_room->music, "" ) )
	music( ch, 55, -1, "music", ch->in_room->music, SOUND_NOMUSIC );

    else if ( ch->in_room->area != NULL
	 &&   ch->in_room->area->music != NULL
	 &&   str_cmp( ch->in_room->area->music, "" ) )
	music( ch, 55, -1, "music", ch->in_room->area->music, SOUND_NOMUSIC );

    else if ( IS_SET( ch->sound, SOUND_ON )
	 &&   !IS_SET( ch->sound, SOUND_NOMUSIC ) )
	send_to_char( "!!MUSIC(Off)\n\r", ch );

    if (IS_AFFECTED(ch,AFF_PLAGUE))
    {
        AFFECT_DATA *af, plague;
        CHAR_DATA *vch;

        for ( af = ch->affected; af != NULL; af = af->next )
        {
            if (af->type == gsn_plague)
                break;
        }

        if (af == NULL)
        {
            REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
            return;
        }

        if (af->level == 1)
            return;

	plague.where		= TO_AFFECTS;
        plague.type 		= gsn_plague;
        plague.level 		= af->level - 1;
	plague.dur_type		= DUR_TICKS;
        plague.duration 	= number_range(1,2 * plague.level);
        plague.location		= APPLY_STR;
        plague.modifier 	= -5;
        plague.bitvector 	= AFF_PLAGUE;
	plague.next		= NULL;

        for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            if (!saves_spell(plague.level - 2,ch,vch,DAM_DISEASE)
	    &&  !IS_IMMORTAL(vch) &&
            	!IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(6) == 0)
            {
            	send_to_char("You feel hot and feverish.\n\r",vch);
            	act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM,POS_RESTING);
            	affect_join(vch,&plague);
            }
        }
    }

    return;
}

/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    if ( obj == NULL || ch == NULL )
    {
	bug( "OBJ_TO_CHAR: NULL ch/obj.", 0 );
	return;
    }

    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    ch->carry_number	+= get_obj_number( obj );
    ch->carry_weight	+= get_obj_weight( obj );
}



/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    CHAR_DATA *ch;

    if ( ( ch = obj->carried_by ) == NULL )
    {
	bug( "Obj_from_char: null ch.", 0 );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
	unequip_char( ch, obj );

    if ( ch->carrying == obj )
    {
	ch->carrying = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = ch->carrying; prev != NULL; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Obj_from_char: obj not in list.", 0 );
    }

    obj->carried_by	 = NULL;
    obj->next_content	 = NULL;
    ch->carry_number	-= get_obj_number( obj );
    ch->carry_weight	-= get_obj_weight( obj );
    return;
}



/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear, int type )
{
    if ( obj->item_type != ITEM_ARMOR )
	return 0;

    switch ( iWear )
    {
    case WEAR_BODY:	return 3 * obj->value[type];
    case WEAR_HEAD:	return 2 * obj->value[type];
    case WEAR_LEGS:	return     obj->value[type];
    case WEAR_FEET:	return     obj->value[type];
    case WEAR_HANDS:	return     obj->value[type];
    case WEAR_ARMS:	return     obj->value[type];
    case WEAR_SHIELD:	return     obj->value[type];
    case WEAR_FINGER_L:	return     obj->value[type];
    case WEAR_FINGER_R: return     obj->value[type];
    case WEAR_NECK_1:	return     obj->value[type];
    case WEAR_NECK_2:	return     obj->value[type];
    case WEAR_ABOUT:	return 2 * obj->value[type];
    case WEAR_WAIST:	return     obj->value[type];
    case WEAR_WRIST_L:	return     obj->value[type];
    case WEAR_WRIST_R:	return     obj->value[type];
    case WEAR_HOLD:	return     obj->value[type];
    case WEAR_FACE:	return 2 * obj->value[type];
    case WEAR_EAR_L:	return	   obj->value[type];
    case WEAR_EAR_R:	return     obj->value[type];
    case WEAR_BACK:	return	   obj->value[type];
    case WEAR_ANKLE_L:	return	   obj->value[type];
    case WEAR_ANKLE_R:	return	   obj->value[type];
    case WEAR_CHEST:	return	   obj->value[type];
    case WEAR_EYES:	return	   obj->value[type];
    case WEAR_CLAN:	return	   obj->value[type];
    }

    return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == iWear )
	    return obj;
    }

    return NULL;
}

void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    AFFECT_DATA *paf;
    char buf[MAX_STRING_LENGTH];
    sh_int flag, sn;
    int i;

    if ( get_eq_char( ch, iWear ) != NULL )
    {
	sprintf(buf,"Character: %s (vnum: %d).", ch->name,
	    IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
	bug( "Equip_char: already equipped (%d).", iWear );
	bug( buf, 0 );
	bug( "Object Vnum: (%d).", obj->pIndexData->vnum );
	return;
    }

    if (ch->level < LEVEL_IMMORTAL)
    {
	if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	{
	    act( "{RYou are {Yzapped {Rby $p and drop it.{x", ch, obj, NULL, TO_CHAR,POS_RESTING);
	    act( "{R$n is {Yzapped {Rby $p and drops it.{x",  ch, obj, NULL, TO_ROOM,POS_RESTING);
	    obj_from_char( obj );
	    set_obj_sockets( ch, obj );
	    set_arena_obj( ch, obj );
	    obj_to_room( obj, ch->in_room );
	    return;
	}
    }

    if (IS_OBJ_STAT(obj,ITEM_QUESTPOINT) && !IS_NPC(ch))
    {
	REMOVE_BIT(obj->extra_flags,ITEM_QUESTPOINT);
	ch->pcdata->deviant_points[0]++;
	ch->pcdata->deviant_points[1]++;
	rank_chart( ch, "deviant", ch->pcdata->deviant_points[1] );
	act("{YYou gained a {wD{Devian{wt {rP{Roin{rt {Yfrom {x$p{Y!{x",ch,obj,NULL,TO_CHAR,POS_DEAD);
	act("{Y$n gained a {wD{Devian{wt {rP{Roin{rt {Yfrom {x$p{Y!{x",ch,obj,NULL,TO_ROOM,POS_RESTING);
    }

    for (i = 0; i < 4; i++)
    	ch->armor[i]	-= apply_ac( obj, iWear, i );
    obj->wear_loc	= iWear;

    if (!obj->enchanted)
    {
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	{
		for ( flag = 0; object_affects[flag].spell != NULL; flag++ )
		{
		    if ( paf->where == object_affects[flag].location
		    &&   object_affects[flag].bit & paf->bitvector )
		    {
			if ( !IS_NPC(ch)
			&&   (sn = skill_lookup(object_affects[flag].spell)) != -1
			&&   skill_table[sn].spell_fun != spell_null )
			{
			    perm_affect = TRUE;
			    (*skill_table[sn].spell_fun)
				(sn,paf->level,ch,ch,TARGET_CHAR);
			    perm_affect = FALSE;
			}
		    }
		}

	        affect_modify( ch, paf, TRUE );
	}
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	    for ( flag = 0; object_affects[flag].spell != NULL; flag++ )
	    {
		if ( paf->where == object_affects[flag].location
		&&   object_affects[flag].bit & paf->bitvector )
		{
		    if ( !IS_NPC(ch)
		    &&   (sn = skill_lookup(object_affects[flag].spell)) != -1
		    &&   skill_table[sn].spell_fun != spell_null )
		    {
			perm_affect = TRUE;
			(*skill_table[sn].spell_fun)
			    (sn,paf->level,ch,ch,TARGET_CHAR);
			perm_affect = FALSE;
		    }
		}
	    }

	    affect_modify( ch, paf, TRUE );
    }

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL )
	++ch->in_room->light;

    return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf = NULL;
    sh_int flag, sn;
    int i;

    if ( obj->wear_loc == WEAR_NONE )
    {
	bug( "Unequip_char: already unequipped.", 0 );
	return;
    }

    for (i = 0; i < 4; i++)
    	ch->armor[i]	+= apply_ac( obj, obj->wear_loc,i );
    obj->wear_loc	 = -1;

    if (!obj->enchanted)
    {
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	{
		for ( flag = 0; object_affects[flag].spell != NULL; flag++ )
		{
		    if ( paf->where == object_affects[flag].location
		    &&   object_affects[flag].bit & paf->bitvector )
		    {
			if ( (sn = skill_lookup(object_affects[flag].spell)) != -1 )
			{
			    perm_affect_strip(ch,sn);
			} else {
			    if ( object_affects[flag].location == TO_AFFECTS )
				REMOVE_BIT(ch->affected_by,object_affects[flag].bit);
			    else
				REMOVE_BIT(ch->shielded_by,object_affects[flag].bit);
			}
		    }
		}

	        affect_modify( ch, paf, FALSE );
//		affect_check(ch,paf->where,paf->bitvector);
	}
    }
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	    for ( flag = 0; object_affects[flag].spell != NULL; flag++ )
	    {
		if ( paf->where == object_affects[flag].location
		&&   object_affects[flag].bit & paf->bitvector )
		{
		    if ( (sn = skill_lookup(object_affects[flag].spell)) != -1 )
		    {
			perm_affect_strip(ch,sn);
		    } else {
			if ( object_affects[flag].location == TO_AFFECTS )
			    REMOVE_BIT(ch->affected_by,affect_flags[flag].bit);
			else
			    REMOVE_BIT(ch->shielded_by,affect_flags[flag].bit);
		    }
		}
	    }

	    affect_modify( ch, paf, FALSE );
//	    affect_check(ch,paf->where,paf->bitvector);
    }

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
	affect_check(ch,paf->where,paf->bitvector);

    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;
    }

    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
    ROOM_INDEX_DATA *in_room;
    CHAR_DATA *ch;

    if ( ( in_room = obj->in_room ) == NULL )
    {
	bug( "obj_from_room: NULL.", 0 );
	return;
    }

    for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
	if (ch->on == obj)
	    ch->on = NULL;

    if ( obj == in_room->contents )
    {
	in_room->contents = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = in_room->contents; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_room: obj not found.", 0 );
	    return;
	}
    }

    obj->in_room      = NULL;
    obj->next_content = NULL;
    return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{
    obj->next_content		= pRoomIndex->contents;
    pRoomIndex->contents	= obj;
    obj->in_room		= pRoomIndex;
    obj->carried_by		= NULL;
    obj->in_obj			= NULL;
    return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    obj->next_content		= obj_to->contains;
    obj_to->contains		= obj;
    obj->in_obj			= obj_to;
    obj->in_room		= NULL;
    obj->carried_by		= NULL;
    if (obj_to->pIndexData->item_type == ITEM_PIT)
        obj->cost = 0;

    for ( ; obj_to != NULL; obj_to = obj_to->in_obj )
    {
	if ( obj_to->carried_by != NULL )
	{
	    obj_to->carried_by->carry_number += get_obj_number( obj );
	    obj_to->carried_by->carry_weight += get_obj_weight( obj )
		* WEIGHT_MULT(obj_to) / 100;
	}
    }

    return;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
	bug( "Obj_from_obj: null obj_from.", 0 );
	return;
    }

    if ( obj == obj_from->contains )
    {
	obj_from->contains = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = obj_from->contains; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_obj: obj not found.", 0 );
	    return;
	}
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

    for ( ; obj_from != NULL; obj_from = obj_from->in_obj )
    {
	if ( obj_from->carried_by != NULL )
	{
	    obj_from->carried_by->carry_number -= get_obj_number( obj );
	    obj_from->carried_by->carry_weight -= get_obj_weight( obj )
		* WEIGHT_MULT(obj_from) / 100;
	}
    }

    return;
}



/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;

    if ( obj->in_room != NULL )
	obj_from_room( obj );
    else if ( obj->carried_by != NULL )
	obj_from_char( obj );
    else if ( obj->in_obj != NULL )
	obj_from_obj( obj );

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
	obj_next = obj_content->next_content;
	extract_obj( obj_content );
    }

    if ( object_list == obj )
    {
	object_list = obj->next;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = object_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == obj )
	    {
		prev->next = obj->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
	    return;
	}
    }

    --obj->pIndexData->count;
    free_obj(obj);
    return;
}

void extract_char( CHAR_DATA *ch, bool fPull )
{
    CHAR_DATA *wch;
    OBJ_DATA *obj, *obj_next;

    if ( ch == NULL )
    {
        bug("Extract_char: NULL ch",0);
        return;
    }

    if ( ch->in_room == NULL )
    {
	sprintf( log_buf, "Extract_char: (%s) NULL Room. Attempting fix", ch->name );
	bug( log_buf, 0 );
	char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
    }

    nuke_pets( ch );

    if ( fPull )
	die_follower( ch );

    stop_fighting( ch, TRUE );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;

	if ( obj->item_type == ITEM_WEAPON )
	    extract_obj( obj );
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	extract_obj( obj );
    }

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next;

	if ( obj->disarmed_from != NULL && obj->disarmed_from == ch )
	    obj->disarmed_from = NULL;

	if ( ( obj->item_type == ITEM_TRAP
	||     obj->item_type == ITEM_MINE )
	&&   obj->dropped_by == ch
	&&   obj->carried_by == NULL )
	{
	    obj_from_room( obj );
	    extract_obj( obj );
	}
    }

    char_from_room( ch );

    if ( !fPull )
    {
	if ( clan_table[ch->clan].hall == ROOM_VNUM_ALTAR
	&&   ch->alignment < 0 )
	    char_to_room( ch, get_room_index( ROOM_VNUM_ALTARB ) );
	else
	    char_to_room( ch, get_room_index( clan_table[ch->clan].hall ) );
	return;
    }

    if ( IS_NPC( ch ) )
	--ch->pIndexData->count;

    if ( ch->desc != NULL && ch->desc->original != NULL )
    {
	do_return( ch, "" );
	ch->desc = NULL;
    }

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
     	if ( ch->mprog_target == wch )
	    wch->mprog_target = NULL;

	if ( wch->pcdata && wch->pcdata->reply == ch )
	    wch->pcdata->reply = NULL;

	if ( !IS_NPC( wch ) && !IS_NPC( ch ) )
	{
	    if ( wch->pcdata->opponent == ch )
	    {
		wch->pcdata->opponent = NULL;
		wch->pcdata->pktimer = 0;
	    }
	}
    }

    if ( !IS_NPC( ch ) )
    {
	if ( ch == player_list )
	    player_list = ch->pcdata->next_player;
	else
	{
	    CHAR_DATA *prev;

	    for ( prev = player_list; prev != NULL; prev = prev->pcdata->next_player )
	    {
		if ( prev->pcdata->next_player == ch )
		{
		    prev->pcdata->next_player = ch->pcdata->next_player;
		    break;
		}
	    }

	    if ( prev == NULL )
	    {
		sprintf( log_buf, "Extract_char: player (%s) not found.", ch->name );
		bug( log_buf, 0 );
	    }
	}
    }

    if ( ch == char_list )
	char_list = ch->next;
    else
    {
	CHAR_DATA *prev;

	for ( prev = char_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == ch )
	    {
		prev->next = ch->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    sprintf( log_buf, "Extract_char: char (%s) not found.", ch->name );
	    bug( log_buf, 0 );
	    return;
	}
    }

    if ( ch->desc != NULL )
	ch->desc->character = NULL;
    free_char( ch );
    return;
}

CHAR_DATA *get_char_room( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int count, number;

    if ( argument == NULL || argument[0] == '\0' )
	return NULL;

    number = number_argument( argument, arg );
    count  = 0;

    if ( ch && room )
    {
	bug( "get_char_room received multiple types (ch/room)", 0 );
	return NULL;
    }

    if ( ch )
	rch = ch->in_room->people;
    else
	rch = room->people;

    for ( ; rch != NULL; rch = rch->next_in_room )
    {
	if ( ( ch && get_trust( ch ) < rch->ghost_level )
	||   ( ch && !can_see( ch, rch ) )
	||   !is_name( arg, rch->name ) )
	    continue;

	if ( ++count == number )
	    return rch;
    }

    if ( !str_cmp( arg, "self" ) )
	return ch;

    return NULL;
}

CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int count, number;

    if ( argument == NULL || argument[0] == '\0' )
	return NULL;

    if ( ch && ( wch = get_char_room( ch, NULL, argument ) ) != NULL )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;

    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
	if ( wch->in_room == NULL
	||   !is_name( arg, wch->name ) )
	    continue;

	if ( ch )
	{
	    if ( !can_see( ch, wch )
	    ||   ( IS_SET( wch->in_room->area->area_flags, AREA_UNLINKED )
	    &&     !IS_IMMORTAL( ch ) )
	    ||   !check_builder( ch, wch->in_room->area, FALSE ) )
		continue;
	}

	if ( ++count == number )
	    return wch;
    }

    return NULL;
}

CHAR_DATA *get_pc_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;

    one_argument( argument, arg );

    if ( !str_cmp( arg, "self" ) && !IS_NPC( ch ) )
	return ch;

    for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
    {
        if ( !str_prefix(arg, wch->name)
	&&   can_see(ch,wch) )
            return wch;
    }

    return NULL;
}

AREA_DATA *area_lookup( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {
	if ( !IS_IMMORTAL( ch )
	&&   ( IS_SET( pArea->area_flags, AREA_UNLINKED )
	||      pArea->clan != 0 ) )
	    continue;

	if ( is_name( argument, strip_color( pArea->name ) ) )
	    return pArea;
    }

    return NULL;
}

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( (obj->item_type == ITEM_EXIT) && is_name(arg, obj->name) )
	{
	    if ( ++count == number )
		return obj;
	}

	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

/*
 * Find an exit obj in a list. (no ch)
 */
OBJ_DATA *get_obj_exit( char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( (obj->item_type == ITEM_EXIT) && is_name(arg, obj->name) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

/*
 * Find an obj in a list. (no ch)
 */
OBJ_DATA *get_obj_item( char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int count, number;

    if ( ch == NULL )
	return NULL;

    number = number_argument( argument, arg );
    count  = 0;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	} else if ( obj->wear_loc == WEAR_NONE
	&&   IS_NPC(ch)
	&&   ch->pIndexData->vnum >= 5
	&&   ch->pIndexData->vnum <= 20
        &&   is_name( arg, obj->name ) )
	{
            if ( ++count == number )
                return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument, bool character )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int count, number;

    number = number_argument( argument, arg );
    count  = 0;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc != WEAR_NONE
	&&  ( character ? can_see_obj( ch, obj ) : TRUE)
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument )
{
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];
    int number, count;

    if ( ch && room )
    {
	bug( "get_obj_here received a ch and a room",0);
	return NULL;
    }

    number = number_argument( argument, arg );
    count = 0;

    if ( ch )
    {
	obj = get_obj_list( ch, argument, ch->in_room->contents );
	if ( obj != NULL )
	    return obj;

	if ( ( obj = get_obj_carry( ch, argument ) ) != NULL )
	    return obj;

	if ( ( obj = get_obj_wear( ch, argument, TRUE ) ) != NULL )
	    return obj;
    } else {
	for ( obj = room->contents; obj; obj = obj->next_content )
	{
	    if ( !is_name( arg, obj->name ) )
		continue;
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int count, number;

    if ( ch && ( obj = get_obj_here( ch, NULL, argument ) ) != NULL )
	return obj;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( ( obj->carried_by != NULL
	&&     !can_see( ch, obj->carried_by ) )
	||   ( ch && ( !can_see_obj( ch, obj )
		  ||   !check_builder( ch, obj->pIndexData->area, FALSE ) ) )
	||   !is_name( arg, obj->name ) )
	    continue;

	if ( ++count == number )
	    return obj;
    }

    return NULL;
}

void deduct_cost( CHAR_DATA *ch, int cost, int value )
{
    while ( ch->silver >= 100 )
    {
	ch->gold++;
	ch->silver -= 100;
    }

    while ( ch->gold >= 100 )
    {
	ch->platinum++;
	ch->gold -= 100;
    }

    switch( value )
    {
	case VALUE_PLATINUM:
	    ch->platinum -= cost;
	    if ( ch->platinum < 0 )
	    {
		bug( "deduct costs: platinum %d < 0", ch->platinum );
		ch->platinum = 0;
	    }
	    break;

	case VALUE_GOLD:
	    while ( ch->gold < cost )
	    {
		ch->platinum--;
		ch->gold += 100;
	    }

	    ch->gold -= cost;
	    if ( ch->platinum < 0 )
	    {
		bug( "deduct costs: platinum %d < 0", ch->platinum );
		ch->platinum = 0;
	    }

	    if ( ch->gold < 0 )
	    {
		bug( "deduct costs: gold %d < 0", ch->gold );
		ch->gold = 0;
	    }
	    break;

	case VALUE_SILVER:
	    while ( ch->silver < cost )
	    {
		ch->gold--;
		ch->silver += 100;

		if ( ch->gold < 0 )
		{
		    ch->platinum--;
		    ch->gold += 100;
		}
	    }

	    ch->silver -= cost;
	    if ( ch->platinum < 0 )
	    {
		bug( "deduct costs: platinum %d < 0", ch->platinum );
		ch->platinum = 0;
	    }

	    if ( ch->gold < 0 )
	    {
		bug( "deduct costs: gold %d < 0", ch->gold );
		ch->gold = 0;
	    }

	    if ( ch->silver < 0 )
	    {
		bug( "deduct costs: silver %d < 0", ch->silver );
		ch->silver = 0;
	    }
	    break;

	case VALUE_QUEST_POINT:
	    if ( ch->pcdata )
		ch->pcdata->questpoints -= cost;
	    break;

	case VALUE_IQUEST_POINT:
	    if ( ch->pcdata )
		ch->pcdata->deviant_points[0] -= cost;
	    break;

	default:
	    bug( "deduct costs: invalid value.", 0 );
	    break;
    }
}

void add_cost( CHAR_DATA *ch, int cost, int value )
{
    while ( ch->silver >= 100 )
    {
	ch->gold++;
	ch->silver -= 100;
    }

    while ( ch->gold >= 100 )
    {
	ch->platinum++;
	ch->gold -= 100;
    }

    switch ( value )
    {
	case VALUE_PLATINUM:
	    ch->platinum += cost;
	    if ( ch->platinum > 50000 )
		ch->platinum = 50000;
	    break;

	case VALUE_GOLD:
	    ch->gold += cost;
	    while ( ch->gold >= 100 )
	    {
		ch->platinum++;
		ch->gold -= 100;
	    }

	    if ( ch->platinum > 50000 )
		ch->platinum = 50000;
	    break;

	case VALUE_SILVER:
	    ch->silver += cost;
	    while ( ch->silver >= 100 )
	    {
		ch->gold++;
		ch->silver -= 100;
	    }

	    while ( ch->gold >= 100 )
	    {
		ch->platinum++;
		ch->gold -= 100;
	    }

	    if ( ch->platinum > 50000 )
		ch->platinum = 50000;
	    break;

	case VALUE_QUEST_POINT:
	    if ( ch->pcdata )
		ch->pcdata->questpoints += cost;
	    break;

	case VALUE_IQUEST_POINT:
	    if ( ch->pcdata )
		ch->pcdata->deviant_points[0] += cost;
	    break;

	default:
	    bug( "deduct costs: invalid value.", 0 );
	    break;
    }
}

/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int platinum, int gold, int silver )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if ( platinum < 0 || gold < 0 || silver < 0
    || (platinum == 0 && gold == 0 && silver == 0) )
    {
	bug( "Create_money: zero or negative money.",
	    UMIN(UMIN(platinum,gold),silver));
	platinum = UMAX(1,platinum);
	gold = UMAX(1,gold);
	silver = UMAX(1,silver);
    }

    if (platinum == 0 && gold == 0 && silver == 1)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_SILVER_ONE ) );
    }
    else if (platinum == 0 && gold == 1 && silver == 0)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_GOLD_ONE) );
    }
    else if (platinum == 1 && gold == 0 && silver == 0)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_PLATINUM_ONE) );
    }
    else if (gold == 0 && silver == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_PLATINUM_SOME ) );
        sprintf( buf, obj->short_descr, platinum );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[2]           = platinum;
        obj->cost               = platinum;
	obj->weight		= platinum/20;
    }
    else if (platinum == 0 && silver == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_GOLD_SOME ) );
        sprintf( buf, obj->short_descr, gold );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[1]           = gold;
        obj->cost               = gold;
	obj->weight		= gold/20;
    }
    else if (platinum == 0 && gold == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SILVER_SOME ) );
        sprintf( buf, obj->short_descr, silver );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[0]           = silver;
        obj->cost               = silver;
	obj->weight		= silver/20;
    }

    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_COINS ) );
	obj->value[0]		= silver;
	obj->value[1]		= gold;
	obj->value[2]		= platinum;
	obj->cost		= (10000 * platinum) + (100 * gold) + silver;
	obj->weight		= (platinum + gold + silver)/20;
    }

    return obj;
}



/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
    int number;

    if (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY
    ||  obj->item_type == ITEM_GEM	 || obj->item_type == ITEM_TREASURE
    ||  obj->item_type == ITEM_PIT )
        number = 0;
    else
        number = 1;

    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        number += get_obj_number( obj );

    return number;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;
    OBJ_DATA *tobj;

    weight = obj->weight;
    for ( tobj = obj->contains; tobj != NULL; tobj = tobj->next_content )
	weight += get_obj_weight( tobj ) * WEIGHT_MULT(obj) / 100;

    return weight;
}

int get_true_weight(OBJ_DATA *obj)
{
    int weight;

    weight = obj->weight;
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        weight += get_obj_weight( obj );

    return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
    if ( pRoomIndex == NULL )
	return TRUE;

    if ( pRoomIndex->light > 0 )
	return FALSE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
    ||   pRoomIndex->sector_type == SECT_CITY )
	return FALSE;

    if ( weather_info.sunlight == SUN_SET
    ||   weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}

bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int count = 0;;

    for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room )
	if ( !IS_NPC( rch ) )
	    count++;

    if ( ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
    ||   ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
    ||   ( pRoomIndex->max_people > 0 && count >= pRoomIndex->max_people )
    ||   ( IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) ) )
	return TRUE;

    return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    if ( IS_SET( pRoomIndex->room_flags, ROOM_IMP_ONLY )
    &&   get_trust( ch ) < MAX_LEVEL )
	return FALSE;

    if ( IS_SET( pRoomIndex->room_flags, ROOM_HEROES_ONLY )
    &&   !IS_HERO( ch ) )
	return FALSE;

    if ( !IS_IMMORTAL( ch ) )
    {
	if ( IS_SET( pRoomIndex->area->area_flags, AREA_UNLINKED )
	||   IS_SET( pRoomIndex->room_flags, ROOM_GODS_ONLY )
	||   ( IS_SET( pRoomIndex->room_flags, ROOM_NEWBIES_ONLY )
	&&     ch->level > 9 ) )
	    return FALSE;
    }

    return TRUE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( ch == NULL || victim == NULL )
	return FALSE;

    if ( ch == victim )
	return TRUE;

    if ( ch->in_room != NULL && victim->in_room != NULL
    &&   IS_SET( ch->in_room->room_flags, ROOM_ARENA )
    &&   IS_SET( victim->in_room->room_flags, ROOM_ARENA )
    &&   ch->arena_number != victim->arena_number
    &&   !IS_IMMORTAL( ch ) )
	return FALSE;

    chance = get_trust( ch );
    if ( chance < victim->invis_level
    ||   ( chance < victim->incog_level
    &&     ch->in_room != victim->in_room ) )
	return FALSE;

    if ( ( ch->pIndexData != NULL && IS_SET( ch->act, ACT_SEE_ALL ) )
    ||   ( !IS_NPC( victim ) && !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) )
    ||   ( IS_NPC( victim ) && IS_IMMORTAL( ch ) ) )
	return TRUE;

    if ( IS_AFFECTED( ch, AFF_BLIND )
    ||   ( room_is_dark( ch->in_room ) && !IS_AFFECTED( ch, AFF_INFRARED ) )
    ||   ( IS_SHIELDED( victim, SHD_INVISIBLE )
    &&     !IS_AFFECTED( ch, AFF_DETECT_INVIS ) ) )
	return FALSE;

    if ( IS_AFFECTED( victim, AFF_SNEAK )
    &&   !IS_AFFECTED( ch, AFF_DETECT_HIDDEN )
    &&   victim->fighting == NULL )
    {
	chance = get_skill( victim, gsn_sneak );

	chance += get_curr_stat( victim, STAT_DEX ) * 3 / 2;
 	chance -= get_curr_stat( ch, STAT_INT ) * 2;
	chance -= ch->level - victim->level * 3 / 2;

	if ( number_percent( ) < chance )
	    return FALSE;
    }

    if ( ( IS_AFFECTED( victim, AFF_HIDE )
    &&     !IS_AFFECTED( ch, AFF_DETECT_HIDDEN )
    &&     victim->fighting == NULL )
    ||   is_affected( victim, gsn_camouflage )
    ||   is_affected( victim, gsn_forest_meld )
    ||   (is_affected( victim, gsn_obfuscate )
    &&    !IS_AFFECTED( ch, AFF_DARK_VISION )) )
	return FALSE;

    return TRUE;
}

bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( ch == NULL || obj == NULL
    ||   obj->item_type == ITEM_EXIT
    ||   ( obj->arena_number > 0 && obj->arena_number != ch->arena_number ) )
	return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;

    if ( IS_SET( obj->extra_flags, ITEM_VIS_DEATH )
    ||   ( IS_AFFECTED( ch, AFF_BLIND )
    &&     ( obj->item_type != ITEM_POTION
    ||       !IS_SET(obj->extra_flags,ITEM_HUM) ) ) )
	return FALSE;

    if ( obj->item_type == ITEM_TRAP
    &&   IS_SET( obj->extra_flags, ITEM_DARK )
    &&   obj->dropped_by != ch )
        return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	return TRUE;

    if ( IS_SET(obj->extra_flags, ITEM_INVIS)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
        return FALSE;

    if ( IS_OBJ_STAT(obj,ITEM_GLOW))
	return TRUE;

    if ( IS_SET(obj->extra_flags, ITEM_DARK)
    &&   !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
    &&   obj->item_type != ITEM_TRAP )
	return FALSE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    return TRUE;
}

bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_SET(obj->extra_flags, ITEM_NODROP)
    ||   ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL ) )
	return TRUE;

    return FALSE;
}

void reset_affects( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    int loc;

    for (loc = 0; loc < MAX_WEAR; loc++)
    {
        obj = get_eq_char(ch, loc);

        if (obj == NULL)
            continue;

        if ( obj->wear_loc != WEAR_NONE )
            unequip_char( ch, obj );
        else
            continue;

        equip_char( ch, obj, loc );
    }
}

int flag_lookup (const char *name, const struct flag_type *flag_table)
{
    int flag;

    for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
	if (LOWER(name[0]) == LOWER(flag_table[flag].name[0])
	&&  !str_prefix(name,flag_table[flag].name))
	    return flag_table[flag].bit;
    }

    return 0;
}

int clan_lookup(const char *name)
{
    int clan;

    for (clan = 0; clan_table[clan].name[0] != '\0'; clan++)
    {
	if (LOWER(name[0]) == LOWER(clan_table[clan].name[0])
	&&  !str_prefix(name,clan_table[clan].name))
	    return clan;
    }

    return 0;
}

char * pretitle( CHAR_DATA *ch, CHAR_DATA *victim )
{

    if ( IS_NPC( ch )
    ||   ch->pcdata->pretitle[0] == '\0'
    ||   ( victim != NULL && !can_see( victim, ch ) ) )
	return "";

    return ch->pcdata->pretitle;
}

void spam_check( CHAR_DATA *ch )
{
    if ( ch->pcdata == NULL
    ||   IS_IMMORTAL( ch )
    ||   ch->fighting != NULL )
	return;

    if ( ++ch->pcdata->spam_count >= 3 )
    {
	ch->pcdata->spam_count = 0;
	WAIT_STATE(ch,PULSE_VIOLENCE);
    }
}

void racial_spells( CHAR_DATA *ch, bool cast )
{
    sh_int flag, sn;

    if ( cast )
    {
	for ( flag = 0; object_affects[flag].spell != NULL; flag++ )
	{
	    if ( ( object_affects[flag].location == TO_AFFECTS
	    &&     race_table[ch->race].aff & object_affects[flag].bit )
	    ||   ( object_affects[flag].location == TO_SHIELDS
	    &&     race_table[ch->race].shd & object_affects[flag].bit ) )
	    {
		if ( ( sn = skill_lookup( object_affects[flag].spell ) ) != -1
		&&   skill_table[sn].spell_fun != spell_null )
		{
		    if ( !is_affected( ch, sn ) )
		    {
			perm_affect = TRUE;
			( *skill_table[sn].spell_fun )
			    ( sn, ch->level, ch, ch, TARGET_CHAR );
			perm_affect = FALSE;
		    }
		} else {
		    if ( object_affects[flag].location == TO_AFFECTS )
			SET_BIT( ch->affected_by, object_affects[flag].bit );
		    else
			SET_BIT( ch->shielded_by, object_affects[flag].bit );
		}
	    }
	}
    }

    else
    {
	for ( flag = 0; object_affects[flag].spell != NULL; flag++ )
	{
	    if ( ( object_affects[flag].location == TO_AFFECTS
	    &&     race_table[ch->race].aff & object_affects[flag].bit )
	    ||   ( object_affects[flag].location == TO_SHIELDS
	    &&     race_table[ch->race].shd & object_affects[flag].bit ) )
	    {
		if ( ( sn = skill_lookup( object_affects[flag].spell ) ) != -1 )
		{
		    perm_affect_strip( ch, sn );
		} else {
		    if ( object_affects[flag].location == TO_AFFECTS )
			REMOVE_BIT( ch->affected_by, object_affects[flag].bit );
		    else
			REMOVE_BIT( ch->shielded_by, object_affects[flag].bit );
		}
	    }
	}
    }
}

void mobile_spells( CHAR_DATA *ch, bool cast )
{
    sh_int flag, sn;

    if ( ch->pIndexData == NULL )
    {
	bug( "Mobile_spells called on PC character!", 0 );
	return;
    }

    if ( cast )
    {
	for ( flag = 0; object_affects[flag].spell != NULL; flag++ )
	{
	    if ( ( object_affects[flag].location == TO_AFFECTS
	    &&     ch->pIndexData->affected_by & object_affects[flag].bit )
	    ||   ( object_affects[flag].location == TO_SHIELDS
	    &&     ch->pIndexData->shielded_by & object_affects[flag].bit ) )
	    {
		if ( ( sn = skill_lookup( object_affects[flag].spell ) ) != -1
		&&   skill_table[sn].spell_fun != spell_null )
		{
		    if ( !is_affected( ch, sn ) )
		    {
			perm_affect = TRUE;
			( *skill_table[sn].spell_fun )
			    ( sn, ch->level, ch, ch, TARGET_CHAR );
			perm_affect = FALSE;
		    }
		} else {
		    if ( object_affects[flag].location == TO_AFFECTS )
			SET_BIT( ch->affected_by, object_affects[flag].bit );
		    else
			SET_BIT( ch->shielded_by, object_affects[flag].bit );
		}
	    }
	}
    }

    else
    {
	for ( flag = 0; object_affects[flag].spell != NULL; flag++ )
	{
	    if ( ( object_affects[flag].location == TO_AFFECTS
	    &&     ch->pIndexData->affected_by & object_affects[flag].bit )
	    ||   ( object_affects[flag].location == TO_SHIELDS
	    &&     ch->pIndexData->shielded_by & object_affects[flag].bit ) )
	    {
		if ( ( sn = skill_lookup( object_affects[flag].spell ) ) != -1 )
		{
		    perm_affect_strip( ch, sn );
		} else {
		    if ( object_affects[flag].location == TO_AFFECTS )
			REMOVE_BIT( ch->affected_by, object_affects[flag].bit );
		    else
			REMOVE_BIT( ch->shielded_by, object_affects[flag].bit );
		}
	    }
	}
    }
}

void send_sound_char( CHAR_DATA *ch, int volu, int limit, int pri, char *dir, char *file, long flag )
{
    char buf[MAX_STRING_LENGTH];

    if ( volu < 1 || volu > 100 )
	volu = 75;

    if ( pri < 1 || pri > 100 )
	pri = 75;

    if ( !IS_SET( ch->sound, SOUND_ON ) || IS_SET( ch->sound, flag ) )
	return;

    sprintf( buf, "!!SOUND(sounds/%s/%s V=%d L=%d P=%d T=%s U=http://dsmud.genesismuds.com/sounds)\n\r",
	dir, file, volu, limit, pri, dir );
    send_to_char( buf, ch );
}

void send_sound_room( ROOM_INDEX_DATA *room, int volu, int limit, int pri, char *dir, char *file, long flag )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if ( !room )
	return;

    if ( volu < 1 || volu > 100 )
	volu = 75;

    if ( pri < 1 || pri > 100 )
	pri = 75;

    for ( victim = room->people; victim != NULL; victim = victim->next_in_room )
    {
	if ( IS_NPC( victim )
	||   !IS_SET( victim->sound, SOUND_ON )
	||   IS_SET( victim->sound, flag ) )
	    continue;

    	sprintf( buf, "!!SOUND(sounds/%s/%s V=%d L=%d P=%d T=%s U=http://dsmud.genesismuds.com/sounds)\n\r",
	    dir, file, volu, limit, pri, dir );
    	send_to_char( buf, victim );
    }
}

void send_sound_area( AREA_DATA *area, int volu, int limit, int pri, char *dir, char *file, long flag )
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    if ( volu < 1 || volu > 100 )
	volu = 75;

    if ( pri < 1 || pri > 100 )
	pri = 75;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING
	&&   victim->in_room != NULL
	&&   victim->in_room->area == area )
        {
	    if ( !IS_SET( victim->sound, SOUND_ON )
	    ||   IS_SET( victim->sound, flag ) )
		continue;

	    sprintf( buf, "!!SOUND(sounds/%s/%s V=%d L=%d P=%d T=%s U=http://dsmud.genesismuds.com/sounds)\n\r",
		dir, file, volu, limit, pri, dir );
	    send_to_char( buf, d->character );
	}
    }
}

void music( CHAR_DATA *ch, int volu, int limit, char *dir, char *file, long flag )
{
    char buf[MAX_STRING_LENGTH];

    if ( volu < 1 || volu > 100 )
	volu = 75;

    if ( !IS_SET( ch->sound, SOUND_ON )
    ||   IS_SET( ch->sound, flag ) )
	return;

    sprintf( buf, "!!MUSIC(sounds/%s/%s V=%d L=%d C=1 T=%s U=http://dsmud.genesismuds.com/sounds)\n\r",
	dir, file, volu, limit, dir );
    send_to_char( buf, ch );
}

bool check_builder( CHAR_DATA *ch, AREA_DATA *pArea, bool show )
{
    if ( ch->pcdata == NULL
    ||   pArea == NULL
    ||   ch->level < LEVEL_IMMORTAL
    ||   get_trust( ch ) >= 104
    ||   ch->pcdata->security >= pArea->security )
	return TRUE;

    if ( show )
	send_to_char( "{RDon't worry about that and get back to work!{x\n\r", ch );

    return FALSE;
}

int sort_damages( const void *v1, const void *v2 )
{
    int idx1 = *(int *) v1;
    int idx2 = *(int *) v2;
    int i = 0;

    for ( i = 0; damage_mod_table[idx1].name[i] != '\0'; i++ )
    {
	if ( damage_mod_table[idx2].name == NULL )
	    return 1;

	if ( damage_mod_table[idx1].name[i]
	==   damage_mod_table[idx2].name[i] )
	    continue;

	if ( damage_mod_table[idx1].name[i] >
	     damage_mod_table[idx2].name[i] )
	    return 1;

	if ( damage_mod_table[idx1].name[i] <
	     damage_mod_table[idx2].name[i] )
	    return -1;
    }

    return 0;
}

char *show_dam_mods( sh_int damage_mod[DAM_MAX] )
{
    static char final[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int index[DAM_MAX], lvl, pos;

    for ( pos = 0; pos < DAM_MAX; pos++ )
	index[pos] = pos;

    qsort( index, pos, sizeof(int), sort_damages );

    sprintf( final, "{qDamage Modifiers:" );

    for ( pos = 0; pos < DAM_MAX; pos++ )
    {
	lvl = index[pos];

	if ( pos % 4 == 0 )
	    strcat( final, "\n\r" );

	sprintf( buf, "  {s[{t%-9s{s:{%c%4d{t%%{s]",
	    damage_mod_table[lvl].name,
	    damage_mod[lvl] < 50 ? 'G' :
	    damage_mod[lvl] < 90 ? 'g' :
	    damage_mod[lvl] < 111 ? 'y' :
	    damage_mod[lvl] < 151 ? 'r' : 'R',
	    damage_mod[lvl] );
	strcat( final, buf );
    }

    strcat( final, "{x\n\r" );

    return final;
}

int social_lookup( const char *name )
{
    int i;

    for ( i = 0; i < maxSocial ; i++ )
    {
	if ( !str_prefix( name, social_table[i].name ) )
	    return i;
    }

    return -1;
}

char * parse_time( int time )
{
    static char dur[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int pos = time, hour = 0, minute = 0;

    if ( time == -1 )
	return "a permanent duration";

    else if ( time == 0 )
	return "NONE";

    while( pos >= 3600 )
    {
	hour++;
	pos -= 3600;
    }

    while( pos >= 60 )
    {
	minute++;
	pos -= 60;
    }

    dur[0] = '\0';
    if ( hour > 0 )
    {
	sprintf( buf, ", %d hour%s", hour, hour == 1 ? "" : "s" );
	strcat( dur, buf );
    }

    if ( minute > 0 )
    {
	sprintf( buf, ", %d minute%s", minute, minute == 1 ? "" : "s" );
	strcat( dur, buf );
    }

    if ( pos > 0 )
    {
	sprintf( buf, ", %d second%s", pos, pos == 1 ? "" : "s" );
	strcat( dur, buf );
    }

    return dur+2;
}

bool check_kill_steal( CHAR_DATA *ch, CHAR_DATA *victim, bool show )
{
    if ( IS_NPC( victim )
    &&   victim->fighting != NULL
    &&   !IS_NPC( victim->fighting )
    &&   victim->fighting != ch
    &&   !is_same_group( ch, victim->fighting )
    &&   !can_pk( ch, victim->fighting ) )
    {
	if ( show )
	    send_to_char( "{RKill stealing outside of pk range is not permitted.{x\n\r", ch );

	return FALSE;
    }

    return TRUE;
}

char * show_condition( CHAR_DATA *ch, CHAR_DATA *victim, sh_int type )
{
    static char meter[100];
    int percent;

    sprintf( meter, "{R%s", PERS( victim, ch ) );

    switch( type )
    {
	case VALUE_HIT_POINT:
	    if ( victim->max_hit > 0 )
		percent = victim->hit * 100 / victim->max_hit;
	    else
		percent = -1;
	    break;

	case VALUE_MANA_POINT:
	    if ( victim->max_mana > 0 )
		percent = victim->mana * 100 / victim->max_mana;
	    else
		percent = -1;
	    break;

	case VALUE_MOVE_POINT:
	    if ( victim->max_move > 0 )
		percent = victim->move * 100 / victim->max_move;
	    else
		percent = -1;
	    break;

	default:
	    bug( "Show_condition: invalid type.", 0 );
	    return "Invalid condition type called.\n\r";
    }

    if ( percent >= 100 )
    {
	if ( !IS_SET( ch->combat, COMBAT_METER ) )
	    strcat( meter, ": {D[{B***{G****{Y***{D]{x\n\r" );
	else
	    strcat( meter, "{R is laughing at your feeble attempts.{x\n\r" );
    }

    else if ( percent >= 90 )
    {
	if ( !IS_SET( ch->combat, COMBAT_METER ) )
	    strcat( meter,": {D[{B***{G****{Y** {D]{x\n\r" );
	else
	    strcat( meter, "{R has FINALLY started to notice you!{x\n\r" );
    }

    else if ( percent >= 80 )
    {
	if ( !IS_SET( ch->combat, COMBAT_METER ) )
	    strcat( meter, ": {D[{B***{G****{Y*  {D]{x\n\r" );
	else
	    strcat( meter, "{R is laughing at your feeble attempts.{x\n\r" );
    }

    else if ( percent >= 70 )
    {
	if ( !IS_SET( ch->combat, COMBAT_METER ) )
	    strcat( meter, ": {D[{B***{G****   {D]{x\n\r" );
	else
	    strcat( meter, "{R has FINALLY started to notice you!{x\n\r" );
    }

    else if ( percent >= 60 )
    {
	if ( !IS_SET( ch->combat, COMBAT_METER ) )
	    strcat( meter, ": {D[{B***{G***    {D]{x\n\r" );
	else
	    strcat( meter, "{R is spewing blood and guts.{x\n\r" );
    }

    else if ( percent >= 50 )
    {
	if ( !IS_SET( ch->combat, COMBAT_METER ) )
	    strcat( meter, ": {D[{B***{G**     {D]{x\n\r" );
	else
	    strcat( meter, "{R is spewing blood and guts all over.{x\n\r" );
    }

    else if ( percent >= 40 )
    {
	if ( !IS_SET( ch->combat, COMBAT_METER ) )
	    strcat( meter,": {D[{B***{G*      {D]{x\n\r" );
	else
	    strcat( meter, "{R is begging you for mercy.{x\n\r" );
    }

    else if ( percent >= 30 )
    {
	if ( !IS_SET( ch->combat, COMBAT_METER ) )
	    strcat( meter, ": {D[{B***       {D]{x\n\r" );
	else
	    strcat( meter, "{R is begging you for mercy.{x\n\r" );
    }

    else if ( percent >= 20 )
    {
	if ( !IS_SET( ch->combat, COMBAT_METER ) )
	    strcat( meter, ": {D[{B**        {D]{x\n\r" );
	else
	{
	    strcat( meter, "{R is praying to " );

	    if ( IS_GOOD( victim ) )
		strcat( meter, mud_stat.good_god_string );
	    else if ( IS_EVIL( victim ) )
		strcat( meter, mud_stat.evil_god_string );
	    else
		strcat( meter, mud_stat.neut_god_string );

	    strcat( meter, " {Rfor mercy.{x\n\r" );
	}
    }

    else if ( percent >= 10 )
    {
	if ( !IS_SET( ch->combat, COMBAT_METER ) )
	    strcat( meter, ": {D[{B*         {D]{x\n\r" );
	else
	    strcat( meter, "{R is embracing death.{x\n\r" );
    }

    else
    {
	if ( !IS_SET( ch->combat, COMBAT_METER ) )
	    strcat( meter, ": {D[          {D]{x\n\r" );
	else
	    strcat( meter, "{R is embracing death.{x\n\r" );
    }

    return meter;
}

int find_color( char *argument )
{
    int pos;

    for ( pos = 0; color_table[pos].name != NULL; pos++ )
    {
	if ( !str_prefix( argument, color_table[pos].name ) )
	    return pos;
    }

    return -1;
}

char * color_string( char *code )
{
    int pos;

    for ( pos = 0; color_table[pos].name != NULL; pos++ )
    {
	if ( !str_cmp( color_table[pos].color_code, code ) )
	    return color_table[pos].name;
    }

    return "UNKNOWN";
}

char * return_classes( bool class[maxClass] )
{
    static char buf[MAX_INPUT_LENGTH];
    bool found = FALSE;
    int cnt;

    buf[0] = '\0';

    for ( cnt = 0; cnt < maxClass; cnt++ )
    {
	if ( !class[cnt] )
	{
	    strcat( buf, ", " );
	    strcat( buf, class_table[cnt].name );
	    found = TRUE;
	}
    }

    if ( !found )
	sprintf( buf, ", none" );

    return buf+2;
}
