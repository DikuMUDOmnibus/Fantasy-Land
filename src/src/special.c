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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"

DECLARE_DO_FUN( do_backstab	);
DECLARE_DO_FUN( do_close	);
DECLARE_DO_FUN( do_flee		);
DECLARE_DO_FUN( do_get		);
DECLARE_DO_FUN( do_murder	);
DECLARE_DO_FUN( do_open		);
DECLARE_DO_FUN( do_say		);
DECLARE_DO_FUN( do_wear		);
DECLARE_DO_FUN( do_yell		);

void	say_spell	args( ( CHAR_DATA *ch, int sn ) );
bool	mob_cast	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int sn ) );

DECLARE_SPEC_FUN( spec_stringer		);
DECLARE_SPEC_FUN( spec_mortician	);
DECLARE_SPEC_FUN( spec_questmaster	);
DECLARE_SPEC_FUN( spec_breath_any	);
DECLARE_SPEC_FUN( spec_breath_acid	);
DECLARE_SPEC_FUN( spec_breath_fire	);
DECLARE_SPEC_FUN( spec_breath_frost	);
DECLARE_SPEC_FUN( spec_breath_gas	);
DECLARE_SPEC_FUN( spec_breath_lightning	);
DECLARE_SPEC_FUN( spec_cast_adept	);
DECLARE_SPEC_FUN( spec_cast_cleric	);
DECLARE_SPEC_FUN( spec_cast_judge	);
DECLARE_SPEC_FUN( spec_cast_mage	);
DECLARE_SPEC_FUN( spec_cast_undead	);
DECLARE_SPEC_FUN( spec_executioner	);
DECLARE_SPEC_FUN( spec_fido		);
DECLARE_SPEC_FUN( spec_forger		);
DECLARE_SPEC_FUN( spec_guard		);
DECLARE_SPEC_FUN( spec_janitor		);
DECLARE_SPEC_FUN( spec_mayor		);
DECLARE_SPEC_FUN( spec_poison		);
DECLARE_SPEC_FUN( spec_thief		);
DECLARE_SPEC_FUN( spec_nasty		);
DECLARE_SPEC_FUN( spec_dog_pee		);
DECLARE_SPEC_FUN( spec_cast_clan_adept	);

const   struct  spec_type    spec_table[] =
{
    { "spec_questmaster",	spec_questmaster	},
    { "spec_stringer",		spec_stringer		},
    { "spec_mortician",		spec_mortician		},
    { "spec_breath_any",	spec_breath_any		},
    { "spec_breath_acid",	spec_breath_acid	},
    { "spec_breath_fire",	spec_breath_fire	},
    { "spec_breath_frost",	spec_breath_frost	},
    { "spec_breath_gas",	spec_breath_gas		},
    { "spec_breath_lightning",	spec_breath_lightning	},	
    { "spec_cast_adept",	spec_cast_adept		},
    { "spec_cast_cleric",	spec_cast_cleric	},
    { "spec_cast_judge",	spec_cast_judge		},
    { "spec_cast_mage",		spec_cast_mage		},
    { "spec_cast_undead",	spec_cast_undead	},
    { "spec_executioner",	spec_executioner	},
    { "spec_forger",		spec_forger		},
    { "spec_fido",		spec_fido		},
    { "spec_guard",		spec_guard		},
    { "spec_janitor",		spec_janitor		},
    { "spec_mayor",		spec_mayor		},
    { "spec_poison",		spec_poison		},
    { "spec_thief",		spec_thief		},
    { "spec_nasty",		spec_nasty		},
    { "spec_dog_pee",		spec_dog_pee		},
    { "spec_cast_clan_adept",	spec_cast_clan_adept	},
    { NULL,			NULL			}
};

SPEC_FUN *spec_lookup( const char *name )
{
    int i;
 
    for ( i = 0; spec_table[i].name != NULL; i++ )
    {
	if ( LOWER( name[0] ) == LOWER( spec_table[i].name[0] )
	&&   !str_prefix( name, spec_table[i].name ) )
	    return spec_table[i].function;
    }
 
    return 0;
}

char *spec_name( SPEC_FUN *function )
{
    int i;

    for ( i = 0; spec_table[i].function != NULL; i++ )
    {
	if ( function == spec_table[i].function )
	    return spec_table[i].name;
    }

    return NULL;
}

bool spec_questmaster( CHAR_DATA *ch )
{
    if ( ch->fighting != NULL )
	return spec_cast_mage( ch );

    return FALSE;
}

bool spec_forger( CHAR_DATA *ch )
{
    if ( ch->fighting != NULL )
	return spec_cast_mage( ch );

    return FALSE;
}

bool spec_stringer( CHAR_DATA *ch )
{
    if ( ch->fighting != NULL )
	return spec_cast_mage( ch );

    return FALSE;
}

bool spec_mortician( CHAR_DATA *ch )
{
    if ( ch->fighting != NULL )
	return spec_cast_mage( ch );

    return FALSE;
}

bool spec_dog_pee( CHAR_DATA *ch )
{
    CHAR_DATA *victim, *v_next;

    if ( !IS_AWAKE( ch ) )
	return FALSE;
 
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( victim != ch && can_see( ch, victim )
	&&   number_bits( 1 ) == 0 && !IS_NPC( victim ) )
	    break;
    }
 
    if ( victim == NULL )
        return FALSE;

    switch ( number_bits( 3 ) )
    {
	case 0:
	    return FALSE;

	case 1:
	    return FALSE;

	case 2:
	    act( "$n lifts $s hind leg, and pees on $N's feet.",
		ch, NULL, victim, TO_NOTVICT, POS_RESTING );
	    act( "$n lifts $s hind leg, and pees on your feet.",
		ch, NULL, victim, TO_VICT, POS_RESTING );
	    return TRUE;
    }
 
    return FALSE;
}

bool spec_nasty( CHAR_DATA *ch )
{
    CHAR_DATA *victim, *v_next;
    long gold;

    if ( !IS_AWAKE( ch ) )
	return FALSE;
 
    if ( ch->position != POS_FIGHTING )
    {
	for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
	{
	    v_next = victim->next_in_room;

	    if ( !IS_NPC( victim )
	    &&   victim->level > ch->level
	    &&   victim->level < ch->level + 10 )
	    {
		do_backstab( ch, victim->name );

		if ( ch->position != POS_FIGHTING )
		    do_murder( ch, victim->name );
		/* should steal some coins right away? :) */
		return TRUE;
	    }
	}
	return FALSE;    /*  No one to attack */
    }
 
    /* okay, we must be fighting.... steal some coins and flee */
    if ( ( victim = ch->fighting ) == NULL )
	return FALSE;   /* let's be paranoid.... */
 
    switch ( number_bits( 2 ) )
    {
	case 0:
	    act( "$n rips apart your coin purse, spilling your gold!",
		ch, NULL, victim, TO_VICT, POS_RESTING );
	    act( "You slash apart $N's coin purse and gather his gold.",
		ch, NULL, victim, TO_CHAR, POS_RESTING );
	    act( "$N's coin purse is ripped apart!",
		ch, NULL, victim, TO_NOTVICT, POS_RESTING );
	    gold = victim->gold / 10;  /* steal 10% of his gold */
	    victim->gold -= gold;
	    ch->gold     += gold;
	    return TRUE;

        case 1:
	    do_flee( ch, "" );
	    return TRUE;

	default:
	    return FALSE;
    }
}

bool dragon( CHAR_DATA *ch, int sn )
{
    CHAR_DATA *vch, *victim = NULL;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if ( vch->fighting == ch )
	{
	    if ( victim == NULL || vch->hit < victim->hit )
		victim = vch;
	}
    }

    if ( victim == NULL )
	return FALSE;

    return mob_cast( ch, victim, sn );
}

bool spec_breath_any( CHAR_DATA *ch )
{
    if ( ch->position != POS_FIGHTING )
	return FALSE;

    switch ( number_bits( 3 ) )
    {
	case 0: return spec_breath_fire( ch );

	case 1:
	case 2: return spec_breath_lightning( ch );

	case 3: return spec_breath_gas( ch );

	case 4: return spec_breath_acid( ch );

	case 5:
	case 6:
	case 7: return spec_breath_frost( ch );
    }

    return FALSE;
}

bool spec_breath_acid( CHAR_DATA *ch )
{
    return dragon( ch, gsn_acid_breath );
}

bool spec_breath_fire( CHAR_DATA *ch )
{
    return dragon( ch, gsn_fire_breath );
}

bool spec_breath_frost( CHAR_DATA *ch )
{
    return dragon( ch, gsn_frost_breath );
}

bool spec_breath_gas( CHAR_DATA *ch )
{
    if ( ch->position != POS_FIGHTING )
	return FALSE;

    return mob_cast( ch, ch, gsn_gas_breath );
}

bool spec_breath_lightning( CHAR_DATA *ch )
{
    return dragon( ch, gsn_lightning_breath );
}

bool spec_cast_adept( CHAR_DATA *ch )
{
    CHAR_DATA *victim, *v_next;
    int sn = -1;

    if ( !IS_AWAKE( ch ) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( victim != ch
	&&   can_see( ch, victim )
	&&   number_bits( 1 ) == 0 
	&&   !IS_NPC( victim )
	&&   ( victim->pcdata->tier == 1 || victim->level < 31 ) )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    switch ( number_range( 0,16 ) )
    {
	case 0:		sn = gsn_armor;			break;
	case 1:		sn = gsn_bless;			break;
	case 2:		sn = gsn_cure_blind;		break;
	case 3:		sn = gsn_cure_light;		break;
	case 4:		sn = gsn_cure_poison;		break;
	case 5:		sn = gsn_refresh;		break;
	case 6:		sn = gsn_cure_disease;		break;
	case 7:		sn = gsn_sanctuary;		break;
	case 8:		sn = gsn_shield;		break;
	case 9:		sn = gsn_heal;			break;
	case 10:	sn = gsn_haste;			break;
	case 11:	sn = gsn_iceshield;		break;
	case 12:	sn = gsn_fireshield;		break;
	case 13:	sn = gsn_shockshield;		break;
	case 14:	sn = gsn_wisdom;		break;
	case 15:	sn = gsn_intellect;		break;
	case 16:	sn = gsn_constitution;		break;
    }

    if ( sn == -1 )
	return FALSE;

    say_spell( ch, sn );
    ( *skill_table[sn].spell_fun ) ( sn, ch->level, ch, victim, TARGET_CHAR );
    return TRUE;
}

bool spec_cast_clan_adept( CHAR_DATA *ch )
{
    CHAR_DATA *victim, *v_next;
    int sn = -1;

    if ( !IS_AWAKE( ch ) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( victim != ch
	&&   can_see( ch, victim )
	&&   number_bits( 1 ) == 0 
	&&   !IS_NPC( victim )
	&&   victim->level < 51
	&&   victim->clan == ch->clan )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    switch ( number_range( 0,16 ) )
    {
	case 0:		sn = gsn_armor;			break;
	case 1:		sn = gsn_bless;			break;
	case 2:		sn = gsn_cure_blind;		break;
	case 3:		sn = gsn_cure_light;		break;
	case 4:		sn = gsn_cure_poison;		break;
	case 5:		sn = gsn_refresh;		break;
	case 6:		sn = gsn_cure_disease;		break;
	case 7:		sn = gsn_sanctuary;		break;
	case 8:		sn = gsn_shield;		break;
	case 9:		sn = gsn_heal;			break;
	case 10:	sn = gsn_haste;			break;
	case 11:	sn = gsn_iceshield;		break;
	case 12:	sn = gsn_fireshield;		break;
	case 13:	sn = gsn_shockshield;		break;
	case 14:	sn = gsn_wisdom;		break;
	case 15:	sn = gsn_intellect;		break;
	case 16:	sn = gsn_constitution;		break;
    }

    if ( sn == -1 )
	return FALSE;

    say_spell( ch, sn );
    ( *skill_table[sn].spell_fun ) ( sn, ch->level, ch, victim, TARGET_CHAR );
    return TRUE;
}

bool spec_cast_cleric( CHAR_DATA *ch )
{
    CHAR_DATA *victim, *v_next;
    sh_int min_level = 0, sn = -1;

    if ( ch->position != POS_FIGHTING )
    {
	if ( ch->position != POS_STANDING
	||   number_range( 0, 9 ) < 3 )
	    return FALSE;

	switch( number_range( 0, 15 ) )
	{
	    case  0:
		sn = gsn_armor;
		if ( !is_affected( ch, sn ) )
		    min_level = 1;
		break;

	    case  1:
		sn = gsn_shield;
		if ( !is_affected( ch, sn ) )
		    min_level = 60;
		break;

	    case  2:
		sn = gsn_heal;
		if ( ch->hit < ch->max_hit )
		    min_level = 25;
		break;

	    case  3:
		if ( !IS_SET( ch->shielded_by, SHD_SANCTUARY ) )
		{
		    min_level = 50;
		    sn = gsn_sanctuary;
		}
		break;

	    case  4:
		if ( !IS_SET( ch->shielded_by, SHD_ICE ) )
		{
		    min_level = 40;
		    sn = gsn_iceshield;
		}
		break;

	    case  5:
		if ( !IS_SET( ch->shielded_by, SHD_FIRE ) )
		{
		    min_level = 60;
		    sn = gsn_fireshield;
		}
		break;

	    case  6:
		if ( !IS_SET( ch->shielded_by, SHD_SHOCK ) )
		{
		    min_level = 75;
		    sn = gsn_shockshield;
		}
		break;

	    case  7:
	    case  8:
		if ( ch->hit < ch->max_hit )
		{
		    min_level = 80;
		    sn = gsn_divine_heal;
		}
		break;

	    case  9:
		if ( !IS_SET( ch->shielded_by, SHD_DIVINE_AURA ) )
		{
		    min_level = 85;
		    sn = gsn_divine_aura;
		}
		break;

	    case 10:
		break;

	    case 11:
		sn = gsn_stone_skin;
		if ( !is_affected( ch, sn ) )
		    min_level = 70;
		break;

	    case 12:
	    case 13:
		if ( !IS_SET( ch->affected_by, AFF_FLYING ) )
		{
		    min_level = 35;
		    sn = gsn_fly;
		}
		break;

	    case 14:
		if ( !IS_SET( ch->affected_by, AFF_GIANT_STRENGTH ) )
		{
		    min_level = 40;
		    sn = gsn_giant_strength;
		}
		break;

	    case 15:
		if ( is_affected( ch, gsn_blindness ) )
		{
		    min_level = 10;
		    sn = gsn_cure_blind;
		}
		break;

	    default:
		break;
	}

	if ( min_level <= 0 || ch->level < min_level || sn < 0
	||   !mob_cast( ch, ch, sn ) )
	    return FALSE;

	return TRUE;
    }

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	victim = ch->fighting;

    if ( ch->level < 10
    ||   ch->stunned
    ||   victim == NULL )
	return FALSE;

    for ( ; ; )
    {
	switch ( number_range( 0, 18 ) )
	{
	    case  0: min_level = 10; sn = gsn_blindness;		  break;
	    case  1: min_level = 13; sn = skill_lookup("cause serious");  break;
	    case  2: min_level = 17; sn = skill_lookup("earthquake");	  break;
	    case  3: min_level = 19; sn = skill_lookup("cause critical"); break;
	    case  4: min_level = 20; sn = skill_lookup("dispel evil");	  break;
	    case  5: min_level = 22; sn = gsn_curse;			  break;
	    case  6: min_level = 12; sn = skill_lookup("change sex");	  break;
	    case  7: min_level = 23; sn = gsn_flamestrike;		  break;
	    case  8: min_level = 85; sn = gsn_demonfire;		  break;
	    case  9: min_level = 85; sn = gsn_angelfire;		  break;
	    case 10: min_level = 25; sn = skill_lookup("harm");		  break;
	    case 11: min_level = 25; sn = gsn_plague;			  break;
	    case 12: min_level = 45; sn = gsn_cure_blind;		  break;
	    case 13:	  
	    case 14: min_level = 26; sn = gsn_dispel_magic;		  break;
	    case 15:
	    case 16: min_level = 35; sn = gsn_divine_heal;		  break;
	    default: min_level = 55; sn = gsn_heal;			  break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( sn < 0 || !mob_cast( ch, victim, sn ) )
	return FALSE;

    return TRUE;
}

bool spec_cast_judge( CHAR_DATA *ch )
{
    CHAR_DATA *victim, *v_next;
    int sn;
 
    if ( ch->position != POS_FIGHTING )
        return FALSE;
 
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
        v_next = victim->next_in_room;

        if ( victim->fighting == ch && number_bits( 2 ) == 0 )
            break;
    }
 
    if ( victim == NULL
    ||   ch->stunned
    ||   ( sn = skill_lookup( "high explosive" ) ) < 0 )
	return FALSE;
 
    say_spell( ch, sn );
    ( *skill_table[sn].spell_fun ) ( sn, ch->level, ch, victim, TARGET_CHAR );
    return TRUE;
}

bool spec_cast_mage( CHAR_DATA *ch )
{
    CHAR_DATA *victim, *v_next;
    sh_int min_level = 0, sn = -1;

    if ( ch->position != POS_FIGHTING )
    {
	if ( ch->position != POS_STANDING
	||   number_range( 0, 9 ) < 3 )
	    return FALSE;

	switch( number_range( 0, 25 ) )
	{
	    case  0:
		sn = gsn_armor;
		if ( !is_affected( ch, sn ) )
		    min_level = 3;
		break;

	    case  1:
		sn = gsn_shield;
		if ( !is_affected( ch, sn ) )
		    min_level = 40;
		break;

	    case  2:
		if ( !IS_SET( ch->shielded_by, SHD_ACID ) )
		{
		    min_level = 85;
		    sn = gsn_acidshield;
		}
		break;

	    case  3:
		break;

	    case  4:
		if ( !IS_SET( ch->shielded_by, SHD_ICE ) )
		{
		    min_level = 35;
		    sn = gsn_iceshield;
		}
		break;

	    case  5:
		if ( !IS_SET( ch->shielded_by, SHD_FIRE ) )
		{
		    min_level = 50;
		    sn = gsn_fireshield;
		}
		break;

	    case  6:
		if ( !IS_SET( ch->shielded_by, SHD_SHOCK ) )
		{
		    min_level = 75;
		    sn = gsn_shockshield;
		}
		break;

	    case  7:
		if ( !IS_SET( ch->affected_by, AFF_HASTE ) )
		{
		    min_level = 55;
		    sn = gsn_haste;
		}
		break;

	    case  8:
		sn = gsn_frenzy;
		if ( !is_affected( ch, sn ) )
		    min_level = 70;
		break;

	    case  9:
		if ( !IS_SET( ch->shielded_by, SHD_SHRAPNEL ) )
		{
		    min_level = 90;
		    sn = gsn_shrapnelshield;
		}
		break;

	    case 10:
		break;

	    case 11:
		sn = gsn_stone_skin;
		if ( !is_affected( ch, sn ) )
		    min_level = 60;
		break;

	    case 12:
	    case 13:
		if ( !IS_SET( ch->affected_by, AFF_FLYING ) )
		{
		    min_level = 25;
		    sn = gsn_fly;
		}
		break;

	    case 14:
		if ( !IS_SET( ch->affected_by, AFF_GIANT_STRENGTH ) )
		{
		    min_level	= 45;
		    sn = gsn_giant_strength;
		}
		break;

	    default:
		break;
	}

	if ( min_level <= 0 || ch->level < min_level || sn < 0
	||   !mob_cast( ch, ch, sn ) )
	    return FALSE;

	return TRUE;
    }

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	victim = ch->fighting;

    if ( ch->stunned || victim == NULL )
	return FALSE;

    for ( ; ; )
    {
	switch ( number_range(0,12) )
	{
	    case  0: min_level =  0; sn = gsn_blindness;		break;
	    case  1: min_level =  3; sn = skill_lookup("chill touch");	break;
	    case  2: min_level =  7; sn = gsn_weaken;			break;
	    case  3: min_level =  8; sn = skill_lookup("teleport");	break;
	    case  4: min_level = 11; sn = skill_lookup("colour spray");	break;
	    case  5: min_level = 12; sn = skill_lookup("change sex");	break;
	    case  6: min_level = 13; sn = gsn_energy_drain;		break;
	    case  7:
	    case  8:
	    case  9: min_level = 15; sn = gsn_fireball;			break;
	    case 10: min_level = 20; sn = gsn_plague;			break;
	    default: min_level = 20; sn = gsn_acid_blast;		break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( sn < 0 || !mob_cast( ch, victim, sn ) )
	return FALSE;

    return TRUE;
}

bool spec_cast_undead( CHAR_DATA *ch )
{
    CHAR_DATA *victim, *v_next;
    sh_int min_level = 0, sn = -1;

    if ( ch->position != POS_FIGHTING )
    {
	if ( ch->position != POS_STANDING )
	    return FALSE;

	switch( number_range( 0, 5 ) )
	{
	    case 0:
		if ( !IS_SET( ch->affected_by, AFF_FLYING ) )
		{
		    min_level = 45;
		    sn = gsn_fly;
		}
		break;

	    case 1:
		if ( !IS_SET( ch->affected_by, AFF_GIANT_STRENGTH ) )
		{
		    min_level = 35;
		    sn = gsn_giant_strength;
		}
		break;

	    case 2:
		if ( !IS_SET( ch->shielded_by, SHD_VAMPIRIC ) )
		{
		    min_level = 65;
		    sn = gsn_vampiricshield;
		}
		break;

	    case 3:
		if ( !IS_SET( ch->affected_by, AFF_HASTE ) )
		{
		    min_level = 60;
		    sn = gsn_haste;
		}
		break;

	    case 4:
		sn = gsn_frenzy;
		if ( !is_affected( ch, sn ) )
		    min_level = 50;
		break;

	    case 5:
		if ( !IS_SET( ch->shielded_by, SHD_SHRAPNEL ) )
		{
		    min_level = 85;
		    sn = gsn_shrapnelshield;
		}
		break;

	    default:
		break;
	}

	if ( min_level <= 0 || ch->level < min_level || sn < 0
	||   !mob_cast( ch, ch, sn ) )
	    return FALSE;

	return TRUE;
    }

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	victim = ch->fighting;

    if ( ch->stunned || victim == NULL )
	return FALSE;

    for ( ; ; )
    {
	switch ( number_range( 0, 12 ) )
	{
	    case  0: min_level =  0; sn = gsn_curse;			break;
	    case  1: min_level =  3; sn = gsn_weaken;			break;
	    case  2: min_level =  6; sn = skill_lookup("chill touch");	break;
	    case  3: min_level =  9; sn = gsn_blindness;		break;
	    case  4: min_level = 12; sn = gsn_poison;			break;
	    case  5: min_level = 15; sn = gsn_energy_drain;		break;
	    case  6: min_level = 18; sn = skill_lookup("harm");		break;
	    case  7: min_level = 21; sn = skill_lookup("teleport");	break;
	    case  8: min_level = 20; sn = gsn_plague;			break;
	    case  9: min_level = 18; sn = skill_lookup("harm");		break;
	    default: min_level = 90; sn = gsn_swarm;			break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( sn < 0 || !mob_cast( ch, victim, sn ) )
	return FALSE;

    return TRUE;
}

bool spec_executioner( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim, *v_next;

    if ( !IS_AWAKE( ch ) || ch->fighting != NULL )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_TWIT )
	&&   can_see( ch, victim ) )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    sprintf( buf, "{a%s is a TWIT!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!{x",
	victim->name );
    do_yell( ch, buf );
    multi_hit( ch, victim, TYPE_UNDEFINED, TRUE );
    return TRUE;
}

bool spec_fido( CHAR_DATA *ch )
{
    OBJ_DATA *corpse, *c_next, *obj, *obj_next;

    if ( !IS_AWAKE( ch ) )
	return FALSE;

    for ( corpse = ch->in_room->contents; corpse != NULL; corpse = c_next )
    {
	c_next = corpse->next_content;

	if ( corpse->item_type != ITEM_CORPSE_NPC )
	    continue;

	act( "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM, POS_RESTING );

	for ( obj = corpse->contains; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    obj_from_obj( obj );
	    set_arena_obj( ch, obj );
	    obj_to_room( obj, ch->in_room );
	}
	extract_obj( corpse );
	return TRUE;
    }

    return FALSE;
}

bool spec_guard( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim, *v_next, *ech = NULL;
    int max_evil = 300;

    if ( !IS_AWAKE( ch ) || ch->fighting != NULL )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_TWIT )
	&&   can_see( ch, victim ) )
	    break;

	if ( victim->fighting != NULL
	&&   victim->fighting != ch
	&&   victim->fighting->in_room == victim->in_room
	&&   victim->alignment < max_evil )
	{
	    max_evil = victim->alignment;
	    ech      = victim;
	}
    }

    if ( victim != NULL )
    {
	sprintf( buf, "{a%s is a TWIT!  PROTECT THE INNOCENT!!  BANZAI!!{x",
	    victim->name );
	do_yell( ch, buf );
	multi_hit( ch, victim, TYPE_UNDEFINED, TRUE );
	return TRUE;
    }

    if ( ech != NULL )
    {
	act( "$n screams '{aPROTECT THE INNOCENT!!  BANZAI!!{x",
	    ch, NULL, NULL, TO_ROOM,POS_RESTING);
	multi_hit( ch, ech, TYPE_UNDEFINED, TRUE );
	return TRUE;
    }

    return FALSE;
}

bool spec_janitor( CHAR_DATA *ch )
{
    OBJ_DATA *trash, *trash_next;

    if ( !IS_AWAKE( ch ) )
	return FALSE;

    for ( trash = ch->in_room->contents; trash != NULL; trash = trash_next )
    {
	trash_next = trash->next_content;

	if ( !IS_SET( trash->wear_flags, ITEM_TAKE )
	||   !can_loot( ch, trash )
	||   ch->carry_number + get_obj_number( trash ) > can_carry_n( ch )
	||   get_carry_weight( ch ) + get_obj_weight( trash ) > can_carry_w( ch ) )
	    continue;

	if ( trash->item_type == ITEM_DRINK_CON
	||   trash->item_type == ITEM_TRASH
	||   trash->cost < 10 )
	{
	    act( "$n picks up some trash.", ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    obj_from_room( trash );
	    obj_to_char( trash, ch );
	    return TRUE;
	}
    }

    return FALSE;
}

bool spec_mayor( CHAR_DATA *ch )
{
    static const char open_path[] =
	"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";
    static const char close_path[] =
	"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";
    static const char *path;
    static int pos;
    static bool move;

    if ( !move )
    {
	if ( time_info.hour == 6 )
	{
	    path = open_path;
	    move = TRUE;
	    pos  = 0;
	}

	if ( time_info.hour == 20 )
	{
	    path = close_path;
	    move = TRUE;
	    pos  = 0;
	}
    }

    if ( ch->fighting != NULL )
	return spec_cast_mage( ch );

    if ( !move || ch->position < POS_SLEEPING )
	return FALSE;

    switch ( path[pos] )
    {
	case '0':
	case '1':
	case '2':
	case '3':
	    move_char( ch, path[pos] - '0', FALSE, FALSE );
	    break;

	case 'W':
	    ch->position = POS_STANDING;
	    act( "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    break;

	case 'S':
	    ch->position = POS_SLEEPING;
	    act( "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    break;

	case 'a':
	    act( "{x$n says '{aHello Honey!{x'", ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    break;

	case 'b':
	    act( "{x$n says '{aWhat a view!  I must do something about that dump!{x'",
		ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    break;

	case 'c':
	    act( "{x$n says '{aVandals!  Youngsters have no respect for anything!{x'",
		ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    break;

	case 'd':
	    act( "{x$n says '{aGood day, citizens!{x'", ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    break;

	case 'e':
	    act( "{x$n says '{aI hereby declare the city of {rD{Ra{rm{Rn{ra{Rt{ri{Ro{rn {aopen!{x'",
		ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    break;

	case 'E':
	    act( "{x$n says '{aI hereby declare the city of {rD{Ra{rm{Rn{ra{Rt{ri{Ro{rn {aclosed!{x'",
		ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    break;

	case 'O':
//	    do_unlock( ch, "gate" );
	    do_open( ch, "gate" );
	    break;

	case 'C':
	    do_close( ch, "gate" );
//	    do_lock( ch, "gate" );
	    break;

	case '.' :
	    move = FALSE;
	    break;
    }

    pos++;
    return FALSE;
}

bool spec_poison( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ch->position != POS_FIGHTING
    ||   ( victim = ch->fighting ) == NULL
    ||   number_percent( ) > 2 * ch->level
    ||   ch->stunned )
	return FALSE;

    act( "You bite $N!", ch, NULL, victim, TO_CHAR, POS_RESTING );
    act( "$n bites $N!", ch, NULL, victim, TO_NOTVICT, POS_RESTING );
    act( "$n bites you!", ch, NULL, victim, TO_VICT, POS_RESTING );
    spell_poison( gsn_poison, ch->level, ch, victim, TARGET_CHAR );
    return TRUE;
}

bool spec_thief( CHAR_DATA *ch )
{
    CHAR_DATA *victim, *v_next;
    long gold, silver;

    if ( ch->position != POS_STANDING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( IS_NPC( victim )
	||   victim->level >= LEVEL_IMMORTAL
	||   number_bits( 5 ) != 0 
	||   !can_see( ch, victim ) )
	    continue;

	if ( IS_AWAKE( victim ) && number_range( 0, ch->level ) == 0 )
	{
	    act( "You discover $n's hands in your {zwallet!{x",
		ch, NULL, victim, TO_VICT, POS_RESTING );
	    act( "$N discovers $n's hands in $S {zwallet!{x",
		ch, NULL, victim, TO_NOTVICT, POS_RESTING );
	    return TRUE;
	} else {
	    gold = victim->gold * UMIN( number_range( 1, 20 ), ch->level / 2 ) / 100;
	    gold = UMIN( gold, ch->level * ch->level * 10 );
	    ch->gold += gold;
	    victim->gold -= gold;
	    silver = victim->silver * UMIN( number_range( 1, 20 ), ch->level / 2 ) / 100;
	    silver = UMIN (silver, ch->level * ch->level * 25 );
	    ch->silver += silver;
	    victim->silver -= silver;
	    return TRUE;
	}
    }

    return FALSE;
}

void do_heal( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    SPELL_FUN *spell;
    char *words;
    int cost, sn;

    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( IS_NPC( mob ) && IS_SET( mob->act, ACT_IS_HEALER ) )
	    break;
    }

    if ( mob == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if ( !can_use_clan_mob( ch, mob ) )
	return;

    if ( argument[0] == '\0' )
    {
	act( "$N says '{aI offer the following spells:{x'",
	     ch, NULL, mob, TO_CHAR, POS_DEAD );
	send_to_char(	"  {Glight{y:   {gcure light wounds    {c10 {Yg{yol{Yd\n\r"
			"  {Gserious{y: {gcure serious wounds  {c15 {Yg{yol{Yd\n\r"
			"  {Gcritic{y:  {gcure critical wounds {c25 {Yg{yol{Yd\n\r"
			"  {Gheal{y:    {ghealing spell        {c50 {Yg{yol{Yd\n\r"
			"  {Gblind{y:   {gcure blindness       {c20 {Yg{yol{Yd\n\r"
			"  {Gdisease{y: {gcure disease         {c15 {Yg{yol{Yd\n\r"
			"  {Gpoison{y:  {gcure poison          {c25 {Yg{yol{Yd\n\r"
			"  {Guncurse{y: {gremove curse         {c50 {Yg{yol{Yd\n\r"
			"  {Grefresh{y: {grestore movement     {c 5 {Yg{yol{Yd\n\r"
			"  {Gmana{y:    {grestore mana         {c10 {Yg{yol{Yd\n\r"
			" {xType heal <type> to be healed.\n\r", ch );
	return;
    }

    if ( !str_prefix( argument, "light" ) )
    {
	spell = spell_cure_light;
	sn    = skill_lookup( "cure light" );
	words = "judicandus dies";
	cost  = 1000;
    }

    else if ( !str_prefix( argument, "serious" ) )
    {
	spell = spell_cure_serious;
	sn    = skill_lookup( "cure serious" );
	words = "judicandus gzfuajg";
	cost  = 1500;
    }

    else if ( !str_prefix( argument, "critical" ) )
    {
	spell = spell_cure_critical;
	sn    = skill_lookup( "cure critical" );
	words = "judicandus qfuhuqar";
	cost  = 2500;
    }

    else if ( !str_prefix( argument, "heal" ) )
    {
	spell = spell_heal;
	sn    = gsn_heal;
	words = "pzar";
	cost  = 5000;
    }

    else if ( !str_prefix( argument, "blindness" ) )
    {
	spell = spell_cure_blindness;
	sn    = gsn_cure_blind;
	words = "judicandus noselacri";
	cost  = 2000;
    }

    else if ( !str_prefix( argument, "disease" ) )
    {
	spell = spell_cure_disease;
	sn    = gsn_cure_disease;
	words = "judicandus eugzagz";
	cost = 1500;
    }

    else if ( !str_prefix( argument, "poison" ) )
    {
	spell = spell_cure_poison;
	sn    = gsn_cure_poison;
	words = "judicandus sausabru";
	cost  = 2500;
    }

    else if ( !str_prefix( argument, "uncurse" )
	 ||   !str_prefix( argument, "curse" ) )
    {
	spell = spell_remove_curse;
	sn    = gsn_remove_curse;
	words = "candussido judifgz";
	cost  = 5000;
    }

    else if ( !str_prefix( argument, "mana" )
	 ||   !str_prefix( argument, "energize" ) )
    {
	spell = NULL;
	sn    = -1;
	words = "energizer";
	cost  = 1000;
    }

    else if ( !str_prefix( argument, "refresh" )
	 ||   !str_prefix( argument, "moves" ) )
    {
	spell = spell_refresh;
	sn    = gsn_refresh;
	words = "candusima";
	cost  = 500;
    }

    else
    {
	act( "$N says '{aType 'heal' for a list of spells.{x'",
	    ch, NULL, mob, TO_CHAR, POS_RESTING );
	return;
    }

    if ( cost > ( ( ch->platinum * 10000 ) + ( ch->gold * 100 ) + ch->silver ) )
    {
	act( "$N says '{aYou do not have enough gold for my services.{x'",
	    ch, NULL, mob, TO_CHAR, POS_RESTING );
	return;
    }

    WAIT_STATE( ch, PULSE_VIOLENCE );

    deduct_cost( ch, cost, VALUE_SILVER );

    act( "$n utters the words '{a$T{x'.", mob, NULL, words, TO_ROOM, POS_RESTING );

    if ( spell == NULL )
    {
	ch->mana += dice( 2, 8 ) + mob->level / 3;
	ch->mana = UMIN( ch->mana, ch->max_mana );
	send_to_char( "A warm {Yglow{x passes through you.\n\r", ch );
	return;
    }

    if ( sn == -1 )
	return;

    spell( sn, mob->level, mob, ch, TARGET_CHAR );
    return;
}

bool remove_voodoo( CHAR_DATA *ch )
{
    DESCRIPTOR_DATA *d;
    OBJ_DATA *object;
    bool found = FALSE;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	if ( d->connected != CON_PLAYING )
	    continue;

        victim = ( d->original != NULL ) ? d->original : d->character;

	if ( IS_IMMORTAL( victim )
	&&   ( get_trust( victim ) > get_trust( ch ) ) )
	    continue;

	for ( object = victim->carrying; object != NULL; object = object->next_content )
	{
	    if ( object->pIndexData->vnum == OBJ_VNUM_VOODOO )
	    {
		char arg[MAX_INPUT_LENGTH];

		one_argument( object->name, arg );

		if ( !str_cmp( arg, ch->name ) )
		{
		    found = TRUE;
		    object->timer = 1;
		}
	    }
	}
    }

    if ( !found )
	return FALSE;

    return TRUE;
}

void do_repent( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    SPELL_FUN *spell;
    char buf[MAX_STRING_LENGTH];
    int cost, sn;
    char *words;

    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( IS_NPC( mob ) && IS_SET( mob->act, ACT_IS_PRIEST ) )
	    break;
    }

    if ( mob == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if ( !can_use_clan_mob( ch, mob ) )
	return;

    if ( argument[0] == '\0' )
    {
	act( "$N says '{aI offer the following services:{x'",
	    ch, NULL, mob, TO_CHAR, POS_DEAD );
	sprintf( buf,	"  {Galignment{y: %s{g's forgiveness   {c  50 {Yg{yol{Yd\n\r"
			"  {Gbless{y:     %s{g's blessing      {c 100 {Yg{yol{Yd\n\r"
			"  {Gsanctuary{y: %s{g's protection    {c 100 {Yg{yol{Yd\n\r"
			"  {Gvoodoo{y:    {gRemove voodoo curses  {c5000 {Yg{yol{Yd\n\r"
			" {xType repent <type> to be forgiven.\n\r",
	    mud_stat.good_god_string, mud_stat.good_god_string,
	    mud_stat.good_god_string );
	send_to_char( buf, ch );
	return;
    }

    if ( !str_prefix( argument, "bless" ) )
    {
	spell = spell_bless;
	sn    = gsn_bless;
	words = "amin";
	cost  = 10000;
    }

    else if ( !str_prefix( argument, "sanctuary" ) )
    {
	spell = spell_sanctuary;
	sn    = gsn_sanctuary;
	words = "unam";
	cost  = 10000;
    }

    else if ( !str_prefix( argument, "alignment" ) )
    {
	spell = NULL;
	sn    = -1;
	words = mud_stat.good_god_string;
	cost  = 5000;
    }
                                 
    else if ( !str_prefix( argument, "voodoo" ) )
    {
	spell = NULL;
	sn    = -1;
	words = "mojo";
	cost  = 500000;
    }

    else
    {
	act( "$N says '{aType 'repent' for a list of spells.{x'",
	    ch, NULL, mob, TO_CHAR, POS_RESTING );
	return;
    }

    if ( cost > ( ( ch->platinum * 10000 ) + ( ch->gold * 100 ) + ch->silver ) )
    {
	act( "$N says '{aYou do not have enough gold for my services.{x'",
	    ch, NULL, mob, TO_CHAR, POS_RESTING );
	return;
    }

    if ( spell != NULL && ch->alignment < 0 )
    {
	act(" $n says '$G {adoes not protect the evil at heart!{x'",
	    mob, NULL, ch, TO_VICT, POS_RESTING );
	return;
    }

    WAIT_STATE( ch, PULSE_VIOLENCE );

    deduct_cost( ch, cost, VALUE_SILVER );

    act( "$n utters the words '{ajudicandus $T{x'.",
	mob, NULL, words, TO_ROOM, POS_RESTING );

    if ( !str_prefix( argument, "voodoo" ) )
    {
	if ( remove_voodoo( ch ) )
	    act( "$N tells you '{aThe voodoo curses on you will soon be destroyed.{x'",
		ch, NULL, mob, TO_CHAR, POS_RESTING );
	else
	    act( "$N tells you '{aI couldn't find any voodoo dolls with your name.{x'",
		ch, NULL, mob, TO_CHAR, POS_RESTING );
	return;
    }

    if ( spell == NULL )  /* Increase alignment toward good */
    {
	OBJ_DATA *obj, *obj_next;

	ch->alignment += 200;
	ch->alignment = UMIN( ch->alignment, 1000 );
	if ( ch->pet != NULL )
	    ch->pet->alignment = ch->alignment;

	sprintf( buf, "{cYou feel %s{c's anger toward your actions!{x\n\r",
	    mud_stat.evil_god_string );
	send_to_char( buf, ch );

	if ( is_affected( ch, gsn_infernal_offer ) && !IS_EVIL( ch ) )
	{
	    affect_strip( ch, gsn_infernal_offer );
	    sprintf( buf, "%s has revoked your blessing.\n\r", mud_stat.evil_god_string );
	    send_to_char( buf, ch );
	}

	else if ( is_affected( ch, gsn_divine_blessing ) && !IS_GOOD( ch ) )
	{
	    affect_strip( ch, gsn_divine_blessing );
	    sprintf( buf, "%s has revoked your blessing.\n\r", mud_stat.good_god_string );
	    send_to_char( buf, ch );
	}

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL )    && IS_EVIL( ch )    )
	    ||   ( IS_OBJ_STAT( obj, ITEM_ANTI_GOOD )    && IS_GOOD( ch )    )
	    ||   ( IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL ) && IS_NEUTRAL( ch ) ) )
	    {
 		act( "{cYou are {Wzapped{c by $p.{x", ch, obj, NULL, TO_CHAR, POS_RESTING );
		act( "$n is {Wzapped{x by $p.", ch, obj, NULL, TO_ROOM, POS_RESTING );
		obj_from_char( obj );

		if ( IS_OBJ_STAT( obj, ITEM_NODROP )
		||   IS_OBJ_STAT( obj, ITEM_INVENTORY )
		||   IS_OBJ_STAT( obj, ITEM_AQUEST )
		||   IS_OBJ_STAT( obj, ITEM_FORGED ) )
		{
		    act( "{cA magical aura draws $p {cto you.{x",
			ch, obj, NULL, TO_CHAR, POS_DEAD );
		    act( "A magical aura draws $p to $n.",
			ch, obj, NULL, TO_ROOM, POS_RESTING );
		    obj_to_char( obj, ch );
		} else {
		    obj->disarmed_from = ch;
		    set_obj_sockets( ch, obj );
		    set_arena_obj( ch, obj );
		    obj_to_room( obj, ch->in_room );
		}
	    }
	}

	return;
    }

    if ( sn == -1 )
	return;

    spell( sn, mob->level, mob, ch, TARGET_CHAR );
    return;
}

void do_curse( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    SPELL_FUN *spell;
    char *words;
    char buf[MAX_STRING_LENGTH];
    int cost, sn;

    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( IS_NPC( mob ) && IS_SET( mob->act, ACT_IS_SATAN ) )
	    break;
    }

    if ( mob == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if ( !can_use_clan_mob( ch, mob ) )
	return;

    if (argument[0] == '\0' )
    {
	act( "$N says '{aI offer the following services:{x'",
	    ch, NULL, mob, TO_CHAR, POS_DEAD );
	sprintf( buf,	"  {Galignment{y: %s{g's favor          {c  50 {Yg{yol{Yd\n\r"
			"  {Gbless{y:     %s{g's blessing       {c 100 {Yg{yol{Yd\n\r"
			"  {Gsanctuary{y: %s{g's protection     {c 100 {Yg{yol{Yd\n\r"
			"  {Gvoodoo{y:    {gRemove voodoo curses   {c5000 {Yg{yol{Yd\n\r"
			" {xType curse <type> to be cursed.\n\r",
	    mud_stat.evil_god_string, mud_stat.evil_god_string,
	    mud_stat.evil_god_string );
	send_to_char( buf, ch );
	return;
    }

    if ( !str_prefix( argument, "bless" ) )
    {
	spell = spell_bless;
	sn    = gsn_bless;
	words = "fido";
	cost  = 10000;
    }

    else if ( !str_prefix( argument, "sanctuary" ) )
    {
	spell = spell_sanctuary;
	sn    = gsn_sanctuary;
	words = "unam";
	cost  = 10000;
    }

    else if ( !str_prefix( argument, "alignment" ) )
    {
	spell = NULL;
	sn    = -1;
	words = mud_stat.evil_god_string;
	cost  = 5000;
    }

    else if ( !str_prefix( argument, "voodoo" ) )
    {
	spell = NULL;
	sn    = -1;
	words = "mojo";
	cost  = 500000;
    }

    else
    {
	act( "$N says '{aType 'curse' for a list of spells.{x'",
	    ch, NULL, mob, TO_CHAR, POS_RESTING );
	return;
    }

    if ( cost > ( ( ch->platinum * 10000 ) + ( ch->gold * 100 ) + ch->silver ) )
    {
	act( "$N says '{aYou do not have enough gold for my services.{x'",
	    ch, NULL, mob, TO_CHAR, POS_RESTING );
	return;
    }

    if ( spell != NULL && ch->alignment >= 0 )
    {
	act( "$n says '$G {Sdoes not protect the pure of heart!{x'",
	    mob, NULL, ch, TO_VICT, POS_RESTING );
	return;
    }

    WAIT_STATE( ch, PULSE_VIOLENCE );

    deduct_cost( ch, cost, VALUE_SILVER );
    act("$n utters the words '{ajudicandus $T{x'.",
	mob, NULL, words, TO_ROOM, POS_RESTING );

    if ( !str_prefix( argument, "voodoo" ) )
    {
	if ( remove_voodoo( ch ) )
	    act( "$N tells you '{UThe voodoo curses on you will soon be destroyed{x'",
		ch, NULL, mob, TO_CHAR, POS_RESTING );
	else
	    act( "$n tells you '{SI couldn't find any voodoo dolls with your name.{x'",
		ch, NULL, mob, TO_CHAR, POS_RESTING );
	return;
    }

    if ( spell == NULL )  /* Decrease alignment toward evil */
    {
	OBJ_DATA *obj, *obj_next;

	ch->alignment -= 200;
	ch->alignment = UMAX( ch->alignment, -1000 );
	if ( ch->pet != NULL )
	    ch->pet->alignment = ch->alignment;

	sprintf( buf,"{cYou feel %s{c's anger toward your actions!\n\r",
	    mud_stat.good_god_string );
	send_to_char( buf, ch );

	if ( is_affected( ch, gsn_infernal_offer ) && !IS_EVIL( ch ) )
	{
	    affect_strip( ch, gsn_infernal_offer );
	    sprintf( buf, "%s has revoked your blessing.\n\r", mud_stat.evil_god_string );
	    send_to_char( buf, ch );
	}

	else if ( is_affected( ch, gsn_divine_blessing ) && !IS_GOOD( ch ) )
	{
	    affect_strip( ch, gsn_divine_blessing );
	    sprintf( buf, "%s has revoked your blessing.\n\r", mud_stat.good_god_string );
	    send_to_char( buf, ch );
	}

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL )    && IS_EVIL( ch )    )
	    ||   ( IS_OBJ_STAT( obj, ITEM_ANTI_GOOD )    && IS_GOOD( ch )    )
	    ||   ( IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL ) && IS_NEUTRAL( ch ) ) )
	    {
		act( "{cYou are {Wzapped{c by $p.{x", ch, obj, NULL, TO_CHAR, POS_DEAD );
		act( "$n is {Wzapped{x by $p.",   ch, obj, NULL, TO_ROOM, POS_RESTING );
		obj_from_char( obj );

		if ( IS_OBJ_STAT( obj, ITEM_NODROP )
		||   IS_OBJ_STAT( obj, ITEM_INVENTORY )
		||   IS_OBJ_STAT( obj, ITEM_AQUEST )
		||   IS_OBJ_STAT( obj, ITEM_FORGED ) )
		{
		    act( "{cA magical aura draws $p {cto you.{x",
			ch, obj, NULL, TO_CHAR, POS_DEAD );
		    act( "A magical aura draws $p to $n.",
			ch, obj, NULL, TO_ROOM, POS_RESTING );
		    obj_to_char( obj, ch );
		} else {
		    obj->disarmed_from = ch;
		    set_obj_sockets( ch, obj );
		    set_arena_obj( ch, obj );
		    obj_to_room( obj, ch->in_room );
		}
	    }
	}

	return;
    }

    if ( sn == -1 )
	return;

    spell( sn, mob->level, mob, ch, TARGET_CHAR );
    return;
}
