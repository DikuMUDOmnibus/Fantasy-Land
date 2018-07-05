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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "merc.h"
#include "db.h"
#include "olc.h"
#include "recycle.h"

 #define LOAD_ASGARD_AREAS
// #define LOAD_ASHEN_AREAS
// #define LOAD_DS_AREAS

#define			MAX_STRING	6457600
#define			MAX_PERM_BLOCK	262144
#define			MAX_MEM_LIST	14

extern	OBJ_DATA	*obj_free;
extern	CHAR_DATA	*char_free;
extern  DESCRIPTOR_DATA *descriptor_free;
extern	PC_DATA		*pcdata_free;
extern  AFFECT_DATA	*affect_free;
extern  int		port;

int maxClan;
struct clan_type *clan_table;

int maxRace;
struct race_type *race_table;

int maxSocial;
struct social_type *social_table;

int maxGroup;
struct group_type *group_table;

int maxSkill;
struct skill_type *skill_table;

int maxClass;
struct class_type *class_table;

PKILL_RECORD *		recent_list;
CHAR_DATA *             player_list;
CHAR_DATA *		char_list;
HELP_DATA *		help_first;
HELP_DATA *		help_last;
SHOP_DATA *		shop_first;
SHOP_DATA *		shop_last;
NOTE_DATA *		note_list;
POLL_DATA *		first_poll;
PROG_CODE *		mprog_list;
PROG_CODE *             oprog_list;
PROG_CODE *             rprog_list;
OBJ_DATA *		object_list;
TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;
STAT_DATA		mud_stat;
AUCTION_DATA *		auction_list;
char			log_buf		[2*MAX_INPUT_LENGTH];
char *			help_greeting1;
char *			help_greeting2;
char *			help_greeting3;
char *			help_greeting4;
char *			help_greeting5;
char *                  help_greeting6;
char *                  help_greeting7;
char *                  help_greeting8;
char *                  help_greeting9;
char *                  help_greeting10;
char *                  help_greeting11;
char *                  help_greeting12;
char *			help_authors;

sh_int  gsn_boiling_blood;
sh_int	gsn_locust_swarm;
sh_int	gsn_backdraft;
sh_int	gsn_leech;
sh_int	gsn_electrify;
sh_int	gsn_glacial_aura;
sh_int	gsn_cure_blind;
sh_int	gsn_haste;
sh_int	gsn_divine_heal;
sh_int	gsn_heal;
sh_int	gsn_cure_serious;
sh_int	gsn_cure_critical;
sh_int	gsn_cure_light;
sh_int	gsn_dispel_magic;
sh_int	gsn_slow;
sh_int	gsn_weaken;
sh_int	gsn_acid_breath;
sh_int	gsn_acid_blast;
sh_int	gsn_acid_storm;
sh_int	gsn_frost_breath;
sh_int	gsn_snow_storm;
sh_int	gsn_swarm;
sh_int	gsn_magic_missile;
sh_int	gsn_fire_breath;
sh_int	gsn_fire_storm;
sh_int	gsn_fireball;
sh_int	gsn_flamestrike;
sh_int	gsn_ray_of_truth;
sh_int	gsn_angelfire;
sh_int	gsn_lightning_breath;
sh_int	gsn_electrical_storm;
sh_int	gsn_lightning_bolt;
sh_int	gsn_nightmare;
sh_int	gsn_energy_drain;
sh_int	gsn_demonfire;
sh_int	gsn_gas_breath;
sh_int	gsn_downpour;
sh_int	gsn_divinity;
sh_int	gsn_protect_good;
sh_int	gsn_protect_neutral;
sh_int	gsn_protect_evil;
sh_int	gsn_remove_curse;
sh_int	gsn_bless;
sh_int	gsn_cure_disease;
sh_int	gsn_cure_poison;
sh_int	gsn_darkvision;
sh_int	gsn_detect_invis;
sh_int	gsn_detect_hidden;
sh_int	gsn_detect_good;
sh_int	gsn_detect_evil;
sh_int	gsn_detect_magic;
sh_int	gsn_constitution;
sh_int	gsn_intellect;
sh_int	gsn_wisdom;
sh_int	gsn_armor;
sh_int	gsn_barkskin;
sh_int	gsn_shield;
sh_int	gsn_steel_skin;
sh_int	gsn_stone_skin;
sh_int	gsn_frenzy;
sh_int	gsn_farsight;
sh_int	gsn_infravision;
sh_int	gsn_giant_strength;
sh_int	gsn_pass_door;
sh_int	gsn_regeneration;
sh_int	gsn_growth;
sh_int	gsn_mana_shield;
sh_int	gsn_refresh;

sh_int			gsn_brew;
sh_int			gsn_scribe;
sh_int			gsn_ambush;
sh_int			gsn_backstab;
sh_int			gsn_camouflage;
sh_int			gsn_circle;
sh_int			gsn_dodge;
sh_int			gsn_envenom;
sh_int			gsn_feed;
sh_int			gsn_forest_meld;
sh_int			gsn_hide;
sh_int			gsn_stalk;
sh_int			gsn_hurl;
sh_int			gsn_knife_fighter;
sh_int			gsn_obfuscate;
sh_int			gsn_peek;
sh_int			gsn_pick_lock;
sh_int			gsn_rub;
sh_int			gsn_slip;
sh_int			gsn_sneak;
sh_int			gsn_steal;
sh_int			gsn_tackle;
sh_int			gsn_critical;
sh_int			gsn_critdam;
sh_int			gsn_counter;
sh_int			gsn_disarm;
sh_int			gsn_dual_wield;
sh_int			gsn_engage;
sh_int			gsn_engineer;
sh_int			gsn_enhanced_damage;
sh_int			gsn_evade;
sh_int			gsn_hone;
sh_int			gsn_kick;
sh_int			gsn_roundhouse;
sh_int			gsn_parry;
sh_int			gsn_phase;
sh_int			gsn_raze;
sh_int			gsn_repair;
sh_int			gsn_rescue;
sh_int			gsn_second_attack;
sh_int			gsn_third_attack;
sh_int			gsn_fourth_attack;
sh_int			gsn_fifth_attack;
sh_int			gsn_sixth_attack;
sh_int			gsn_seventh_attack;
sh_int			gsn_sharpen;
sh_int			gsn_blindness;
sh_int			gsn_bloodbath;
sh_int			gsn_charm_person;
sh_int			gsn_curse;
sh_int			gsn_curse_of_ages;
sh_int			gsn_divine_blessing;
sh_int			gsn_infernal_offer;
sh_int			gsn_invis;
sh_int			gsn_mana_tap;
sh_int			gsn_mass_invis;
sh_int			gsn_poison;
sh_int			gsn_plague;
sh_int			gsn_sleep;
sh_int			gsn_sanctuary;
sh_int			gsn_fly;
sh_int			gsn_acidshield;
sh_int			gsn_divine_aura;
sh_int			gsn_fireshield;
sh_int			gsn_iceshield;
sh_int			gsn_rockshield;
sh_int			gsn_shockshield;
sh_int			gsn_shrapnelshield;
sh_int			gsn_thornshield;
sh_int			gsn_vampiricshield;
sh_int			gsn_watershield;
sh_int  		gsn_axe;
sh_int  		gsn_dagger;
sh_int  		gsn_flail;
sh_int  		gsn_mace;
sh_int  		gsn_polearm;
sh_int			gsn_quarterstaff;
sh_int			gsn_shield_block;
sh_int			gsn_shield_levitate;
sh_int  		gsn_spear;
sh_int  		gsn_sword;
sh_int  		gsn_whip;
sh_int  		gsn_bash;
sh_int  		gsn_berserk;
sh_int			gsn_cyclone;
sh_int  		gsn_dirt;
sh_int  		gsn_feed;
sh_int  		gsn_hand_to_hand;
sh_int			gsn_retreat;
sh_int  		gsn_trip;
sh_int  		gsn_fast_healing;
sh_int  		gsn_haggle;
sh_int  		gsn_lore;
sh_int			gsn_master_of_magic;
sh_int  		gsn_meditation;
sh_int  		gsn_scrolls;
sh_int  		gsn_staves;
sh_int  		gsn_wands;
sh_int  		gsn_recall;
sh_int			gsn_strangle;
sh_int  		gsn_stun;
sh_int  		gsn_track;
sh_int  		gsn_gouge;
sh_int  		gsn_grip;
sh_int			gsn_troll_revival;
sh_int			gsn_savage_claws;
sh_int			gsn_axe_mastery;
sh_int			gsn_overhand;
sh_int			gsn_crush;
sh_int			gsn_onslaught;
sh_int			gsn_lunge;
sh_int			gsn_scalp;
sh_int			gsn_stake;
sh_int			gsn_ultra_damage;

/* New GSNs by Stroke */
sh_int			gsn_battlehymn;
sh_int			gsn_warcry;
sh_int			gsn_shield_smash;
sh_int			gsn_dislodge;
sh_int			gsn_awen;
sh_int                  gsn_bandage;
sh_int			gsn_gash;
sh_int                  gsn_cure_weaken;
sh_int                  gsn_deathblow;
sh_int                  gsn_devotion;
sh_int			gsn_disguise;
sh_int                  gsn_dismember;
sh_int                  gsn_charge;
sh_int                  gsn_legsweep;
sh_int                  gsn_quickdraw;
sh_int                  gsn_salve;
sh_int                  gsn_spirit;
sh_int			gsn_2nd_dual;
sh_int                  gsn_3rd_dual;
sh_int			gsn_assassinate;
sh_int			gsn_trapdisarm;
sh_int			gsn_trapset;
sh_int			gsn_storm_of_blades;
sh_int			gsn_cross_slash;
sh_int			gsn_sidestep;
sh_int			gsn_cartwheel;
sh_int			gsn_eighth_attack;
sh_int			gsn_doorbash;

int getpid();
time_t time(time_t *tloc);

/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];
char *			string_hash		[MAX_KEY_HASH];

AREA_DATA *		area_first;
AREA_DATA *		area_last;

char *			string_space;
char *			top_string;
char			str_empty	[1];

int			top_area;
int			top_ed;
int			top_exit;
int			top_help;
int			top_mob_index;
int			top_obj_index;
int			top_reset;
int			top_shop;
int			top_room;

void *			rgFreeList	[MAX_MEM_LIST];
const int		rgSizeList	[MAX_MEM_LIST]	=
{
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144
};

long			nAllocString;
long			sAllocString;
long			nAllocPerm;
long			sAllocPerm;

bool			fBootDb;
FILE *			fpArea;
char			strArea[MAX_INPUT_LENGTH];

ROOM_DAMAGE_DATA *new_room_damage	args( ( int type ) );
void	load_clans		args( ( void ) );
void	load_notes		args( ( void ) );
void	load_bans		args( ( void ) );
void	load_wizlist		args( ( void ) );
void	load_skills		args( ( void ) );
void	load_random_data	args( ( void ) );
void	reset_area		args( ( AREA_DATA *pArea ) );
void	free_help		args( ( HELP_DATA *pHelp ) );
int	flag_lookup		args( ( const char *name, const struct flag_type *flag_table ) );
void	mobile_spells		args( ( CHAR_DATA *ch, bool cast ) );

void init_mm( )
{
    srandom(time(NULL)^getpid());
}

void new_reset( ROOM_INDEX_DATA *pRoom, RESET_DATA *pReset )
{
    RESET_DATA *prev;

    if ( pRoom == NULL )
       return;

    if ( pRoom->reset_first == NULL )
    {
	pRoom->reset_first = pReset;
	return;
    }

    for ( prev = pRoom->reset_first; prev->next != NULL; prev = prev->next )
	;

    prev->next = pReset;
}

/*
bool load_ds_area;

void load_area_ds( FILE *fp )
{
    AREA_DATA *pArea;
    char      *word;
    bool      fMatch;

    load_ds_area = TRUE;

    pArea               = alloc_perm( sizeof(*pArea) );

    fread_string( fp );
    fread_string( fp );

    pArea->name		= fread_string( fp );
    pArea->credits	= fread_string( fp );
    pArea->min_vnum	= fread_number( fp );
    pArea->max_vnum	= fread_number( fp );
    pArea->security	= 8;
    pArea->age          = 15;
    pArea->nplayer      = 0;
    pArea->vnum         = top_area;
    pArea->file_name    = str_dup( strArea );
    pArea->music	= NULL;
    pArea->clan		= 0;
    pArea->run_vnum	= 0;
    pArea->area_flags	= AREA_UNLINKED;
    pArea->alignment	= '?';

    for( ; ; )
    {
	word = feof(fp) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch( UPPER(word[0]) )
	{
	    case '*':
		fMatch = TRUE;
		fread_to_eol( fp );
		break;
	    case 'B':
		if ( !str_cmp( word, "Bldrs" ) )
		{
		    fread_string( fp );
		    fMatch = TRUE;
		    break;
		}
		break;
	    case 'D':
		if ( !str_cmp( word, "Descr" ) )
		{
		    fread_string( fp );
		    fMatch = TRUE;
		    break;
		}
		break;
	    case 'E':
		if ( !str_cmp( word, "End" ) )
		{
		    fMatch = TRUE;
		    if ( area_first == NULL )
			area_first = pArea;
		    if ( area_last  != NULL )
			area_last->next = pArea;
		    area_last   = pArea;
		    pArea->next = NULL;
		    top_area++;
		    return;
		}
		break;
	    case 'F':
		if ( !str_cmp( word, "Flags" ) )
		{
		    fread_flag( fp );
		    fMatch = TRUE;
		    break;
		}
		break;
	    case 'L':
		if( !str_cmp(word,"Levels") )
		{
		    fMatch = TRUE;
		    fread_number( fp );
		    fread_number( fp );
		    break;
		}
		break;
	    case 'S':
		KEY("Sec",pArea->security,fread_number(fp));
		break;
	}
    }
}

void load_mobiles_ds( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
    char buf[20];
    int dam[3], min, max, pos, num, flag;

    for ( ; ; )
    {
        int vnum;
        char letter;
        int iHash;

        letter = fread_letter(fp);
        if( letter != '#' )
        {
		bug( "Loadmobiles: # not found.", 0 );
		exit( 1 );
        }

        vnum = fread_number(fp);
        if( vnum == 0 )
		break;

        fBootDb = FALSE;
        if((pMobIndex = get_mob_index(vnum)) != NULL )
        {
		bug( "LOADMOBILES: vnum %d duplicated.", vnum );
		exit( 1 );
        }
        fBootDb = TRUE;

	pMobIndex                       = new_mob_index( );
	pMobIndex->vnum                 = vnum;
	pMobIndex->area					= area_last;

	pMobIndex->player_name          = fread_string(fp);
	pMobIndex->short_descr          = fread_string(fp);
	pMobIndex->long_descr           = fread_string(fp);
	pMobIndex->description          = fread_string(fp);
	pMobIndex->race		 	= race_lookup(fread_string(fp));
	pMobIndex->say_descr     =   &str_empty[0];
	pMobIndex->die_descr     =   &str_empty[0];

	if ( pMobIndex->race == -1 )
	    pMobIndex->race = race_lookup( "human" );

	pMobIndex->act			= fread_flag(fp);
	pMobIndex->affected_by	= fread_flag(fp)|race_table[pMobIndex->race].aff;
	pMobIndex->shielded_by	= fread_flag(fp)|race_table[pMobIndex->race].shd;

	pMobIndex->alignment            = fread_number(fp);
	pMobIndex->group                = fread_number(fp);

	pMobIndex->level                = fread_number(fp);
	pMobIndex->hitroll              = fread_number(fp);

	dam[0] = fread_number(fp);
	fread_letter(fp);
	dam[1] = fread_number(fp);
	fread_letter(fp);
	dam[2] = fread_number( fp );
	min = 0; max = 0;
	for( pos = 0; pos < 50; pos++ )
	{
	    num = dice( dam[0], dam[1] ) + dam[2];
	    if ( num > max )
		max = num;
	    if ( num < min || min <= 0 )
		min = num;
	}
	pMobIndex->hit[0] = min;
	pMobIndex->hit[1] = max;

	dam[0] = fread_number(fp);
	fread_letter(fp);
	dam[1] = fread_number(fp);
	fread_letter(fp);
	dam[2] = fread_number( fp );
	min = 0; max = 0;
	for( pos = 0; pos < 50; pos++ )
	{
	    num = dice( dam[0], dam[1] ) + dam[2];
	    if ( num > max )
		max = num;
	    if ( num < min || min <= 0 )
		min = num;
	}
	pMobIndex->mana[0] = min;
	pMobIndex->mana[1] = max;


	pMobIndex->damage[DICE_NUMBER]	= fread_number(fp);
									  fread_letter(fp);
	pMobIndex->damage[DICE_TYPE]	= fread_number(fp);
									  fread_letter(fp);
	pMobIndex->damage[DICE_BONUS]	= fread_number(fp);
	pMobIndex->dam_type	= attack_lookup(fread_word(fp));

	pMobIndex->ac[AC_PIERCE]	= fread_number(fp) * 10;
	pMobIndex->ac[AC_BASH]		= fread_number(fp) * 10;
	pMobIndex->ac[AC_SLASH]		= fread_number(fp) * 10;
	pMobIndex->ac[AC_EXOTIC]	= fread_number(fp) * 10;

	fread_flag( fp );

	flag = fread_flag( fp );
	dam_convert( pMobIndex, flag, 0 );

	flag = fread_flag( fp );
	dam_convert( pMobIndex, flag, 50 );

	flag = fread_flag( fp );
	dam_convert( pMobIndex, flag, 150 );

	pMobIndex->start_pos		= flag_value( position_flags, fread_word( fp ) );
	pMobIndex->default_pos		= flag_value( position_flags, fread_word( fp ) );

	strcpy( buf, fread_word( fp ) );
	if ( !str_cmp( buf, "none" ) )
	    pMobIndex->sex = SEX_NEUTRAL;
	else if ( !str_cmp( buf, "male" ) )
	    pMobIndex->sex = SEX_MALE;
	else if ( !str_cmp( buf, "female" ) )
	    pMobIndex->sex = SEX_FEMALE;
	else
	    pMobIndex->sex = SEX_RANDOM;

	pMobIndex->wealth		= fread_number(fp);

	fread_flag( fp );
	fread_flag( fp );

	pMobIndex->size   = flag_value( size_flags, fread_word( fp ) );

	fread_word( fp );

	for ( ; ; )
	{
		letter = fread_letter(fp);

		if (letter == 'F')
		{
		    fread_word(fp);
		    fread_flag(fp);
		}
	    else if (letter == 'D')
	    {
		free_string(pMobIndex->die_descr);
		pMobIndex->die_descr		= fread_string(fp);
	    }
        else if (letter == 'T')
        {
		free_string(pMobIndex->say_descr);
		pMobIndex->say_descr		= fread_string(fp);
		pMobIndex->say_descr[0]		= UPPER(pMobIndex->say_descr[0]);
		}

        else if (letter == '~') // DSA format support.
        {
		break;
		}
		else
		{
		ungetc(letter,fp);
		break;
		}
	}

	if( load_ds_area )
	{
		char *word;
		bool fMatch = FALSE;

		for(;;)
		{
			word = feof(fp) ? "End" : fread_word( fp );
			fMatch = FALSE;

			switch( UPPER(word[0]))
			{
			case '*':
				fMatch = TRUE;
				fread_to_eol( fp );
				break;
			case 'A':
				if ( !str_cmp( word, "AbsorbFlags" ) )
				{ fread_flag( fp ); fMatch = TRUE; break; }
				break;
			case 'D':
				if ( !str_cmp( word, "Damroll" ) )
				{ fread_number( fp ); fMatch = TRUE; break; }
				if ( !str_cmp( word, "DamFlags" ) )
				{ fread_flag( fp ); fMatch = TRUE; break; }
				if ( !str_cmp( word, "DefenseFlags" ) )
				{ fread_flag( fp ); fMatch = TRUE; break; }
				break;
			case 'M':
				if ( !str_cmp( word, "MagicResist" ) )
				{ fread_number( fp ); fMatch = TRUE; break; }
				KEY( "MagicReflect", pMobIndex->reflection, fread_number(fp));
				KEY("MaxWorld",		pMobIndex->max_world,		fread_number(fp));
				break;
			case 'R':
				if ( !str_cmp( word, "Regain" ) )
				{ fread_number( fp ); fMatch = TRUE; break; }
				break;
			case 'S':
				KEY("Saves",		pMobIndex->saves,	fread_number(fp));
				if ( !str_cmp( word, "Speed" ) )
				{ fread_number( fp ), fMatch = TRUE; break; }
				break;
			case 'T':
				if ( !str_cmp( word, "Toughness" ) )
				{ fread_number( fp ); fMatch = TRUE; break; }
				break;
			}

			if( !str_cmp(word,"End"))
			break;

			if( !fMatch )
			{
            bug("LOADMOBILE: no match.",0);
            fread_to_eol(fp);
			}
		}
	}

        iHash                   = vnum % MAX_KEY_HASH;
        pMobIndex->next         = mob_index_hash[iHash];
        mob_index_hash[iHash]   = pMobIndex;
    }

    return;
}

void load_objects_ds( FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
    sh_int pos;

    for ( ; ; )
    {
        int vnum;
        char letter;
        int iHash;

        letter = fread_letter(fp);
        if( letter != '#' )
        {
		bug( "Load_objects: # not found.", 0 );
		exit( 1 );
        }

        vnum = fread_number(fp);
        if( vnum == 0 )
		break;

        fBootDb = FALSE;
        if( get_obj_index( vnum ) != NULL )
        {
		bug( "Load_objects: vnum %d duplicated.", vnum );
		exit( 1 );
        }
        fBootDb = TRUE;

	pObjIndex                       = new_obj_index( );
	pObjIndex->area					= area_last;
	pObjIndex->vnum                 = vnum;
	pObjIndex->size			= SIZE_NONE;
	pObjIndex->reset_num			= 0;
	pObjIndex->name                 = fread_string(fp);
	pObjIndex->short_descr          = fread_string(fp);
	pObjIndex->description          = fread_string(fp);

	fread_string(fp);

	pObjIndex->item_type            = flag_value( type_flags, fread_word( fp ) );
	pObjIndex->extra_flags          = fread_flag(fp);
	pObjIndex->wear_flags           = fread_flag(fp);

	switch(pObjIndex->item_type)
	{
	case ITEM_WEAPON:
	    pObjIndex->value[0]		= weapon_type(fread_word(fp));
	    pObjIndex->value[1]		= fread_number(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= attack_lookup(fread_word(fp));
	    pObjIndex->value[4]		= fread_flag(fp);
	    break;
	case ITEM_CONTAINER:
	case ITEM_PIT:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= fread_flag(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= fread_number(fp);
	    pObjIndex->value[4]		= fread_number(fp);
	    break;
	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
		pObjIndex->value[0]     = fread_number(fp);
		pObjIndex->value[1]     = fread_number(fp);
		pObjIndex->value[2]     = liq_lookup(fread_word(fp));
		pObjIndex->value[3]     = fread_number(fp);
		pObjIndex->value[4]     = fread_number(fp);
		break;
	case ITEM_WAND:
	case ITEM_STAFF:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= fread_number(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[4]		= fread_number(fp);
	    break;
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
 	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[2]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[3]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[4]		= skill_lookup(fread_word(fp));
	    break;
	default:
		pObjIndex->value[0]     = fread_flag(fp);
		pObjIndex->value[1]     = fread_flag(fp);
		pObjIndex->value[2]     = fread_flag(fp);
		pObjIndex->value[3]     = fread_flag(fp);
	    pObjIndex->value[4]		= fread_flag(fp);
	    break;
	}
	pObjIndex->level	= fread_number(fp);
	pObjIndex->weight	= fread_number(fp);
	pObjIndex->cost		= fread_number(fp);

	letter = fread_letter(fp);

	for ( ; ; )
	{
		letter = fread_letter(fp);

		if( letter == 'A' )
		{
			AFFECT_DATA *paf;

			paf                     = (AFFECT_DATA *)alloc_perm( sizeof(*paf) );
			paf->where				= TO_OBJECT;
			paf->type               = -1;
			paf->level              = pObjIndex->level;
			paf->duration           = -1;
			paf->dur_type		= DUR_TICKS;
			paf->location           = fread_number(fp);
			paf->modifier           = fread_number(fp);
			paf->bitvector          = 0;
			paf->next               = pObjIndex->affected;
			pObjIndex->affected     = paf;
		}
	    else if (letter == 'F')
		{
			AFFECT_DATA *paf;

			paf  = (AFFECT_DATA *)alloc_perm( sizeof(*paf) );
			letter = fread_letter(fp);
			switch (letter)
	 		{
			case 'A': paf->where = TO_AFFECTS;	break;
			case 'I': paf->where = -1;	break;
			case 'R': paf->where = -1;	break;
			case 'V': paf->where = -1;	break;
			case 'S': paf->where = TO_SHIELDS;	break;
			default:  bug( "Load_objects: Bad where on flag set.", 0 );
					  exit( 1 );
			}

			paf->type               = -1;
			paf->level              = pObjIndex->level;
			paf->duration           = -1;
			paf->dur_type		= DUR_TICKS;
			paf->location           = fread_number(fp);
			paf->modifier           = fread_number(fp);
			paf->bitvector          = fread_flag(fp);
if ( paf->where != -1 )
{
			paf->next				= pObjIndex->affected;
			pObjIndex->affected     = paf;
}
		}
		else if ( letter == 'E' )
        {
			EXTRA_DESCR_DATA *ed;
 			ed                      = (EXTRA_DESCR_DATA *)alloc_perm( sizeof(*ed) );
			ed->keyword             = fread_string(fp);
			ed->description         = fread_string(fp);
			ed->next                = pObjIndex->extra_descr;
			pObjIndex->extra_descr  = ed;
			top_ed++;
		}
	    }
		else if( letter == 'G')
		{
			fread_string(fp);
	    }
        else if (letter == '~') // DSA format support.
        {
		break;
		}
		else
        {
		ungetc( letter, fp );
		break;
		}
	}

	if( load_ds_area )
	{
		char *word;
		bool fMatch = FALSE;

		for(;;)
		{
			word = feof(fp) ? "End" : fread_word( fp );
			fMatch = FALSE;

			switch( UPPER(word[0]) )
			{
			case '*':
				fMatch = TRUE;
				fread_to_eol( fp );
				break;
			case 'C':
				if ( !str_cmp( word, "Class" ) )
				{ fread_flag( fp ); fMatch = TRUE; break; }
				KEY("ContentDescr", pObjIndex->history, fread_string(fp));
				if ( !str_cmp( word, "Clan" ) )
				{ fread_string( fp ); fMatch = TRUE; break; }
				break;
			case 'M':
				if ( !str_cmp( word, "MaxWorld" ) )
				{ fread_number( fp ); fMatch = TRUE; break; }
				break;
			case 'S':
				if ( !str_cmp( word, "SizeFlags" ) )
				{ fread_flag(fp); fMatch = TRUE; break; }
				if( !str_cmp(word,"Spec"))
				{
					fread_string( fp );
					fMatch = TRUE;
					break;
				}
				break;
			}

			if( !str_cmp(word,"End"))
			break;

			if( !fMatch )
			{
            bug("LOAD_OBJECT: no match.",0);
            fread_to_eol(fp);
			}
		}
	}

        iHash                   = vnum % MAX_KEY_HASH;
        pObjIndex->next         = obj_index_hash[iHash];
        obj_index_hash[iHash]   = pObjIndex;
    }
	return;
}

void load_rooms_ds( FILE *fp )
{
    ROOM_INDEX_DATA *pRoomIndex;

    if ( area_last == NULL )
    {
	bug( "Load_rooms_ds: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	int vnum;
	char letter;
	int iHash;

	letter = fread_letter( fp );

	if( letter != '#' )
	{
	bug( "Load_rooms: # not found.", 0 );
	exit( 1 );
	}

	vnum = fread_number( fp );
	if( vnum == 0 )
	break;

	fBootDb = FALSE;
	if( get_room_index( vnum ) != NULL )
	{
	    bug( "Load_rooms: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;

	pRoomIndex				= alloc_perm( sizeof(*pRoomIndex) );
	pRoomIndex->area		= area_last;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->name		= fread_string( fp );
	pRoomIndex->description	= fread_string( fp );
fread_number(fp);
	pRoomIndex->room_flags	= fread_flag( fp );
	pRoomIndex->sector_type	= fread_number( fp );

	pRoomIndex->heal_rate = 100;
	pRoomIndex->mana_rate = 100;

	for( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'S' )
		break;

	    if( letter == 'H')
		pRoomIndex->heal_rate = fread_number(fp);
	    else if( letter == 'M')
		pRoomIndex->mana_rate = fread_number(fp);
	    else if( letter == 'D' )
	    {
		EXIT_DATA *pexit;
			int door, locks;

			door = fread_number( fp );
			if( door < 0 || door >= MAX_DIR )
			{
			bug( "Fread_rooms: vnum %d has bad door number.", vnum );
			exit( 1 );
			}
	pexit				= alloc_perm( sizeof(*pexit) );
	pexit->description	= fread_string( fp );
	pexit->keyword		= fread_string( fp );
	locks				= fread_number( fp );
	pexit->key			= fread_number( fp );
	pexit->u1.vnum		= fread_number( fp );

	switch ( locks )
	{
	case 1: pexit->exit_info = EX_ISDOOR;						break;
	case 2: pexit->exit_info = EX_ISDOOR|EX_PICKPROOF;			break;
	case 3: pexit->exit_info = EX_ISDOOR|EX_NOPASS;				break;
	case 4: pexit->exit_info = EX_ISDOOR|EX_NOPASS|EX_PICKPROOF;break;
	}

	pexit->orig_door = door;
	pRoomIndex->exit[door]	= pexit;

	if( load_ds_area )
	{
		char *word;
		bool fMatch = FALSE;

		for(;;)
		{
			word = feof(fp) ? "End" : fread_word( fp );
			fMatch = FALSE;

			switch( UPPER(word[0]) )
			{
			case '*':
				fMatch = TRUE;
				fread_to_eol( fp );
				break;
			case 'D':
				KEY("DoorDescr", pexit->description, fread_string(fp));
			case 'F':
				KEY("Flags", pexit->exit_info, fread_flag(fp));
				break;
			case 'L':
				KEY("LockDif", pexit->key, fread_number(fp));
				break;
			}

			if( !str_cmp(word,"End"))
			break;

			if( !fMatch )
			{
            bug("LOAD_EXIT: no match.",0);
            fread_to_eol(fp);
			}
		}
	}

	    }
	    else if ( letter == 'E' )
	    {
			EXTRA_DESCR_DATA *ed;

			ed						= (EXTRA_DESCR_DATA *)alloc_perm( sizeof(*ed) );
			ed->keyword				= fread_string( fp );
			ed->description			= fread_string( fp );
			ed->next				= pRoomIndex->extra_descr;
			pRoomIndex->extra_descr	= ed;
			top_ed++;
	    }
	    else if (letter == 'O')
	    {
		fread_string( fp );
	    }
	    else
	    {
		bug( "Load_rooms: vnum %d has flag not 'DES'.", vnum );
		exit( 1 );
	    }
	}

	// Hash in the room.
	iHash					= vnum % MAX_KEY_HASH;
	pRoomIndex->next		= room_index_hash[iHash];
	room_index_hash[iHash]	= pRoomIndex;

	top_room++;
	{
		char *word;
		int last_obj_vnum = 0;
		bool fMatch = FALSE;

		for(;;)
		{
			word = feof(fp) ? "End" : fread_word( fp );
			fMatch = FALSE;

			switch( UPPER(word[0]) )
			{
			case '*':
				fMatch = TRUE;
				fread_to_eol( fp );
				break;
			case 'R':



if( !str_cmp(word,"Reset"))
{
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pMob;
    char letter;
    fMatch = TRUE;

	if((letter = fread_letter( fp ) ) == 'S')
	break;

	if ( letter == '*' )
	{
	fread_to_eol( fp );
	continue;
	}

    top_reset++;
    pReset			= alloc_perm( sizeof(*pReset) );
    pReset->command		= letter;
    pReset->arg1		= fread_number( fp );
    pReset->arg2		= 0;
    pReset->arg3		= vnum;
    pReset->arg4		= 0;

	switch(	pReset->command )
	{
	case 'M': // mobile
		pReset->arg4	= fread_number( fp );
		if ( ( pMob = get_mob_index( pReset->arg1 ) ) != NULL )
		    pReset->arg2 = pMob->max_world;
		pReset->percent	= fread_number( fp );
		break;
	case 'O': // object
		last_obj_vnum	= pReset->arg1;
		pReset->arg2	= 0;
		pReset->arg4	= 0;
		fread_number( fp );
		pReset->percent	= fread_number( fp );
		break;
	case 'P': // content
		pReset->arg3	= last_obj_vnum;
		pReset->arg2	= fread_number( fp );
		pReset->percent	= fread_number( fp );
		break;
	case 'G': // inven
		last_obj_vnum	= pReset->arg1;
		pReset->arg3	= fread_number( fp );
		pReset->arg4	= 0;
		pReset->percent	= fread_number( fp );
		break;
	case 'E': // worn
		last_obj_vnum	= pReset->arg1;
		pReset->arg3	= fread_number( fp );
		pReset->arg4	= 0;
		pReset->percent	= fread_number( fp );
		break;
 	}

    new_reset( pRoomIndex, pReset );
    pReset->next = NULL;
    fread_to_eol( fp );
}







			}

			if( !str_cmp(word,"End"))
			break;

			if( !fMatch )
			{
		            bug("LOAD_ROOM: no match.",0);
		            fread_to_eol(fp);
			}
		}
	}

    }
    return;
}

bool load_ashen_area;

void load_area_ashen( FILE *fp )
{
    AREA_DATA *pArea;
    char      *word;
    bool      fMatch;

    load_ashen_area = TRUE;

    pArea               = alloc_perm( sizeof(*pArea) );
    pArea->age          = 15;
    pArea->nplayer      = 0;
    pArea->file_name    = str_dup( strArea );
    pArea->vnum         = top_area;
    pArea->name         = str_dup( "New Area" );
    pArea->security     = 9;
    pArea->min_vnum     = 0;
    pArea->max_vnum     = 0;
    pArea->area_flags   = AREA_UNLINKED;
    pArea->alignment	= '?';

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
            case 'C':
                SKEY( "Cred", pArea->builder );
                break;

	    case 'D':
		SKEY( "Dir", pArea->directions );
		break;

            case 'E':
                if ( !str_cmp( word, "End" ) )
                {
                    fMatch = TRUE;
                    if ( area_first == NULL )
                        area_first = pArea;
                    if ( area_last  != NULL )
                        area_last->next = pArea;
                    area_last   = pArea;
                    pArea->next = NULL;
                    top_area++;
		    SET_BIT( pArea->area_flags, AREA_UNLINKED );
                    return;
                }
                break;

            case 'F':
                KEY( "Flag", pArea->area_flags, fread_flag( fp ) );
                break;

	    case 'L':
                if ( !str_cmp( word, "Level" ) )
                {
		    pArea->min_level = fread_number( fp );
		    pArea->max_level = fread_number( fp );
                    break;
                }
		break;

            case 'N':
                SKEY( "Name", pArea->name );
                break;

            case 'S':
                KEY( "Secu", pArea->security, fread_number( fp ) );
                break;

            case 'V':
                if ( !str_cmp( word, "Vnum" ) )
                {
                    pArea->min_vnum = fread_number( fp );
                    pArea->max_vnum = fread_number( fp );
                    break;
                }
                break;
        }
    }
}

void load_rooms_ashen( FILE *fp )
{
    ROOM_INDEX_DATA *pRoomIndex;

    if ( area_last == NULL )
    {
	bug( "Load_rooms_ashen: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	char letter;
	int door, iHash, vnum;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_rooms: # not found.", 0 );
	    exit( 1 );
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	fBootDb = FALSE;
	if ( get_room_index( vnum ) != NULL )
	{
	    bug( "Load_rooms: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;

	pRoomIndex			= alloc_perm( sizeof(*pRoomIndex) );
	pRoomIndex->people		= NULL;
	pRoomIndex->contents		= NULL;
	pRoomIndex->extra_descr		= NULL;
	pRoomIndex->area		= area_last;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->name		= fread_string( fp );
	pRoomIndex->description		= fread_string( fp );
	fread_number( fp );
	pRoomIndex->room_flags		= fread_flag( fp );
	pRoomIndex->sector_type		= fread_number( fp );
	pRoomIndex->light		= 0;
	for ( door = 0; door < MAX_DIR; door++ )
	    pRoomIndex->exit[door] = NULL;

	pRoomIndex->heal_rate = 100;
	pRoomIndex->mana_rate = 100;

	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'A' )
	    {
		ROOM_DAMAGE_DATA *dam	= new_room_damage( dam_type_lookup( fread_word( fp ) ) );
		dam->damage_min		= fread_number( fp );
		dam->damage_max		= fread_number( fp );
		dam->success		= fread_number( fp );
		dam->msg_victim		= fread_string( fp );
		dam->msg_room		= fread_string( fp );

		dam->next		= pRoomIndex->room_damage;
		pRoomIndex->room_damage	= dam;
	    }

	    else if ( letter == 'C')
	    {
		char *tmp = fread_string(fp);
		free_string(tmp);
	    }

	    else if ( letter == 'D' )
	    {
		EXIT_DATA *pexit;

		door = fread_number( fp );
		if ( door < 0 || door >= MAX_DIR )
		{
		    bug( "Fread_rooms: vnum %d has bad door number.", vnum );
		    exit( 1 );
		}

		pexit			= alloc_perm( sizeof(*pexit) );
		pexit->description	= fread_string( fp );
		pexit->keyword		= fread_string( fp );
	 	pexit->rs_flags         = fread_flag( fp );
		pexit->exit_info	= pexit->rs_flags;
		pexit->key		= fread_number( fp );
		pexit->u1.vnum		= fread_number( fp );
		pexit->orig_door        = door;         

		pRoomIndex->exit[door]	= pexit;
		top_exit++;
	    }

	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;

		ed			= alloc_perm( sizeof(*ed) );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= pRoomIndex->extra_descr;
		pRoomIndex->extra_descr	= ed;
		top_ed++;
	    }

	    else if ( letter == 'H' )
		pRoomIndex->heal_rate = fread_number(fp);

	    else if ( letter == 'M') 
		pRoomIndex->mana_rate = fread_number(fp);

	    else if (letter == 'O')
	    {
		char *temp = fread_string( fp );
		free_string( temp );
	    }

	    else if ( letter == 'P' )
		pRoomIndex->max_people = fread_number( fp );

	    else if ( letter == 'R' )
	    {
		PROG_LIST *pRprog;
		char *word;
		int trigger = 0;

		pRprog		= alloc_perm(sizeof(*pRprog));
		word		= fread_word( fp );

		if ( !(trigger = flag_lookup( word, rprog_flags )) )
		{
		    bug( "ROOMprogs: invalid trigger.",0);
		    exit(1);
		}

		SET_BIT( pRoomIndex->rprog_flags, trigger );
		pRprog->trig_type	= trigger;
		pRprog->vnum		= fread_number( fp );
		pRprog->trig_phrase	= fread_string( fp );
		pRprog->next		= pRoomIndex->rprogs;
		pRoomIndex->rprogs	= pRprog;
	    }

	    else if ( letter == 'S' )
		break;

	    else
	    {
		bug( "Load_rooms: vnum %d has flag not 'DES'.", vnum );
		exit( 1 );
	    }
	}

	iHash			= vnum % MAX_KEY_HASH;
	pRoomIndex->next	= room_index_hash[iHash];
	room_index_hash[iHash]	= pRoomIndex;
	top_room++;
    }

    return;
}

void load_mobiles_ashen( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char *tmp;
    int dam[3], min, max, num;

    if ( !area_last )
    {
        bug( "Load_mobiles: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        char letter;
        int iHash, pos, vnum;

        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_mobiles: # not found.", 0 );
            exit( 1 );
        }

        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;

        fBootDb = FALSE;
        if ( get_mob_index( vnum ) != NULL )
        {
            bug( "Load_mobiles: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;

        pMobIndex                       = new_mob_index( );
        pMobIndex->vnum                 = vnum;
	pMobIndex->area                 = area_last;              
        pMobIndex->player_name          = fread_string( fp );
        pMobIndex->short_descr          = fread_string( fp );
        pMobIndex->long_descr           = fread_string( fp );
        pMobIndex->description          = fread_string( fp );

	tmp = fread_string(fp);
	pMobIndex->race = race_lookup(tmp);
	free_string(tmp);
	if ( pMobIndex->race == -1 )
	    pMobIndex->race = race_lookup( "human" );

	for ( pos = 0; pos < DAM_MAX-2; pos++ )
	    pMobIndex->damage_mod[pos] = fread_number( fp );

        pMobIndex->long_descr[0]        = UPPER(pMobIndex->long_descr[0]);
        pMobIndex->description[0]       = UPPER(pMobIndex->description[0]);

        pMobIndex->act                  = fread_flag( fp );
        pMobIndex->affected_by          = fread_flag( fp )
					| race_table[pMobIndex->race].aff;
        pMobIndex->shielded_by          = fread_flag( fp )
					| race_table[pMobIndex->race].shd;
        pMobIndex->pShop                = NULL;
        pMobIndex->alignment            = fread_number( fp );
        pMobIndex->group                = fread_number( fp );

        pMobIndex->level                = fread_number( fp );
        pMobIndex->hitroll              = fread_number( fp );

	dam[0] = fread_number(fp);
	fread_letter(fp);
	dam[1] = fread_number(fp);
	fread_letter(fp);
	dam[2] = fread_number( fp );
	min = 0; max = 0;
	for( pos = 0; pos < 50; pos++ )
	{
	    num = dice( dam[0], dam[1] ) + dam[2];
	    if ( num > max )
		max = num;
	    if ( num < min || min <= 0 )
		min = num;
	}
	pMobIndex->hit[0] = UMAX( 0, min );
	pMobIndex->hit[1] = UMAX( 0, max );

	dam[0] = fread_number(fp);
	fread_letter(fp);
	dam[1] = fread_number(fp);
	fread_letter(fp);
	dam[2] = fread_number( fp );
	min = 0; max = 0;
	for( pos = 0; pos < 50; pos++ )
	{
	    num = dice( dam[0], dam[1] ) + dam[2];
	    if ( num > max )
		max = num;
	    if ( num < min || min <= 0 )
		min = num;
	}
	pMobIndex->mana[0] = UMAX( 0, min );
	pMobIndex->mana[1] = UMAX( 0, max );

	pMobIndex->damage[DICE_NUMBER]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_TYPE]	= fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_BONUS]	= fread_number( fp );
	pMobIndex->dam_type		= attack_lookup(fread_word(fp));

	pMobIndex->ac[AC_PIERCE]	= fread_number( fp ) * 10;
	pMobIndex->ac[AC_BASH]		= fread_number( fp ) * 10;
	pMobIndex->ac[AC_SLASH]		= fread_number( fp ) * 10;
	pMobIndex->ac[AC_EXOTIC]	= fread_number( fp ) * 10;

	fread_flag( fp );

	tmp = fread_word( fp );
	if ( !str_cmp( tmp, "dead" ) )
	    pMobIndex->start_pos = POS_DEAD;
	else if ( !str_cmp( tmp, "mort" ) )
	    pMobIndex->start_pos = POS_MORTAL;
	else if ( !str_cmp( tmp, "incap" ) )
	    pMobIndex->start_pos = POS_INCAP;
	else if ( !str_cmp( tmp, "stun" ) )
	    pMobIndex->start_pos = POS_STUNNED;
	else if ( !str_cmp( tmp, "sleep" ) )
	    pMobIndex->start_pos = POS_SLEEPING;
	else if ( !str_cmp( tmp, "rest" ) )
	    pMobIndex->start_pos = POS_RESTING;
	else if ( !str_cmp( tmp, "sit" ) )
	    pMobIndex->start_pos = POS_SITTING;
	else if ( !str_cmp( tmp, "fight" ) )
	    pMobIndex->start_pos = POS_FIGHTING;
	else
	    pMobIndex->start_pos = POS_STANDING;

	tmp = fread_word( fp );
	if ( !str_cmp( tmp, "dead" ) )
	    pMobIndex->default_pos = POS_DEAD;
	else if ( !str_cmp( tmp, "mort" ) )
	    pMobIndex->default_pos = POS_MORTAL;
	else if ( !str_cmp( tmp, "incap" ) )
	    pMobIndex->default_pos = POS_INCAP;
	else if ( !str_cmp( tmp, "stun" ) )
	    pMobIndex->default_pos = POS_STUNNED;
	else if ( !str_cmp( tmp, "sleep" ) )
	    pMobIndex->default_pos = POS_SLEEPING;
	else if ( !str_cmp( tmp, "rest" ) )
	    pMobIndex->default_pos = POS_RESTING;
	else if ( !str_cmp( tmp, "sit" ) )
	    pMobIndex->default_pos = POS_SITTING;
	else if ( !str_cmp( tmp, "fight" ) )
	    pMobIndex->default_pos = POS_FIGHTING;
	else
	    pMobIndex->default_pos = POS_STANDING;

	tmp = fread_word( fp );
	if ( !str_cmp( tmp, "none" ) )
	    pMobIndex->sex = SEX_NEUTRAL;
	else if ( !str_cmp( tmp, "male" ) )
	    pMobIndex->sex = SEX_MALE;
	else if ( !str_cmp( tmp, "female" ) )
	    pMobIndex->sex = SEX_FEMALE;
	else
	    pMobIndex->sex = SEX_RANDOM;

	pMobIndex->wealth		= fread_number( fp );

	fread_flag( fp );

        sprintf(buf1,"%s",fread_word(fp));
        sprintf(buf2,"%s",fread_word(fp));

        if (!str_cmp(buf1,"tiny")
        ||  !str_cmp(buf1,"small")
        ||  !str_cmp(buf1,"medium")
        ||  !str_cmp(buf1,"large")
        ||  !str_cmp(buf1,"huge")
        ||  !str_cmp(buf1,"giant"))
        {
	    pMobIndex->size   = flag_value( size_flags, buf1 );
            pMobIndex->parts  = flag_lookup( buf2, part_flags );
            pMobIndex->parts ^= race_table[pMobIndex->race].parts;
        }
        else
        {
	    pMobIndex->size   = flag_value( size_flags, buf2 );
            pMobIndex->parts  = flag_lookup( buf1, part_flags );
            pMobIndex->parts ^= race_table[pMobIndex->race].parts;

        }

	pMobIndex->die_descr		= &str_empty[0];
	pMobIndex->say_descr		= &str_empty[0];

	for ( ; ; )
        {
            letter = fread_letter( fp );

	    if ( letter == 'C' )
	    {
		tmp = fread_string( fp );

		free_string( tmp );
	    }

	    else if ( letter == 'D' )
		pMobIndex->die_descr	= fread_string( fp );

	    else if ( letter == 'F' )
            {
		char *word;
		long vector;

                word                    = fread_word(fp);
		vector			= fread_flag(fp);

		if (!str_prefix(word,"act"))
		    REMOVE_BIT(pMobIndex->act,vector);
                else if (!str_prefix(word,"aff"))
		    REMOVE_BIT(pMobIndex->affected_by,vector);
                else if (!str_prefix(word,"shd"))
		    REMOVE_BIT(pMobIndex->shielded_by,vector);
		else if (!str_prefix(word,"par"))
		    REMOVE_BIT(pMobIndex->parts,vector);
		else
		    bug("Flag remove: flag not found.",0);
	     }

             else if ( letter == 'M' )
             {
                PROG_LIST *pMprog;
                char *word;
                int trigger = 0;

                pMprog              = alloc_perm(sizeof(*pMprog));
                word                = fread_word( fp );
                if ( !(trigger = flag_lookup( word, mprog_flags )) )
                {
                    bug("MOBprogs: invalid trigger.",0);
                    exit(1);
                }
                SET_BIT( pMobIndex->mprog_flags, trigger );
                pMprog->trig_type   = trigger;
                pMprog->vnum        = fread_number( fp );
                pMprog->trig_phrase = fread_string( fp );
                pMprog->next        = pMobIndex->mprogs;
                pMobIndex->mprogs   = pMprog;
	    }

	    else if ( letter == 'R' )
		pMobIndex->reflection	= fread_number( fp );

	    else if ( letter == 'S' )
		pMobIndex->saves	= fread_number( fp );

	    else if ( letter == 'T' )
		pMobIndex->say_descr	= fread_string( fp );

	    else if ( letter == 'W' )
		pMobIndex->max_world	= fread_number( fp );

	    else
	    {
		ungetc( letter, fp );
		break;
	    }
	}

        iHash                   = vnum % MAX_KEY_HASH;
        pMobIndex->next         = mob_index_hash[iHash];
        mob_index_hash[iHash]   = pMobIndex;
    }

    return;
}

void load_objects_ashen( FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
    sh_int pos;

    if ( !area_last )
    {
        bug( "Load_objects: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        char letter, *tmp;
        int iHash, vnum;

        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_objects: # not found.", 0 );
            exit( 1 );
        }

        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;

        fBootDb = FALSE;
        if ( get_obj_index( vnum ) != NULL )
        {
            bug( "Load_objects: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;

        pObjIndex                       = new_obj_index( );
        pObjIndex->vnum                 = vnum;
	pObjIndex->area                 = area_last;   
	pObjIndex->reset_num		= 0;
	pObjIndex->size			= SIZE_NONE;
        pObjIndex->name                 = fread_string( fp );
        pObjIndex->short_descr          = fread_string( fp );
        pObjIndex->description          = fread_string( fp );

	tmp = fread_word( fp );
	if ( !str_cmp( tmp, "light" ) )
	    pObjIndex->item_type = ITEM_LIGHT;
	else if ( !str_cmp( tmp, "scroll" ) )
	    pObjIndex->item_type = ITEM_SCROLL;
	else if ( !str_cmp( tmp, "wand" ) )
	    pObjIndex->item_type = ITEM_WAND;
	else if ( !str_cmp( tmp, "staff" ) )
	    pObjIndex->item_type = ITEM_STAFF;
	else if ( !str_cmp( tmp, "weapon" ) )
	    pObjIndex->item_type = ITEM_WEAPON;
	else if ( !str_cmp( tmp, "treasure" ) )
	    pObjIndex->item_type = ITEM_TREASURE;
	else if ( !str_cmp( tmp, "armor" ) )
	    pObjIndex->item_type = ITEM_ARMOR;
	else if ( !str_cmp( tmp, "potion" ) )
	    pObjIndex->item_type = ITEM_POTION;
	else if ( !str_cmp( tmp, "clothing" ) )
	    pObjIndex->item_type = ITEM_CLOTHING;
	else if ( !str_cmp( tmp, "furniture" ) )
	    pObjIndex->item_type = ITEM_FURNITURE;
	else if ( !str_cmp( tmp, "trash" ) )
	    pObjIndex->item_type = ITEM_TRASH;
	else if ( !str_cmp( tmp, "container" ) )
	    pObjIndex->item_type = ITEM_CONTAINER;
	else if ( !str_cmp( tmp, "drink" ) )
	    pObjIndex->item_type = ITEM_DRINK_CON;
	else if ( !str_cmp( tmp, "key" ) )
	    pObjIndex->item_type = ITEM_KEY;
	else if ( !str_cmp( tmp, "food" ) )
	    pObjIndex->item_type = ITEM_FOOD;
	else if ( !str_cmp( tmp, "money" ) )
	    pObjIndex->item_type = ITEM_MONEY;
	else if ( !str_cmp( tmp, "boat" ) )
	    pObjIndex->item_type = ITEM_BOAT;
	else if ( !str_cmp( tmp, "npc_corpse" ) )
	    pObjIndex->item_type = ITEM_CORPSE_NPC;
	else if ( !str_cmp( tmp, "pc_corpse" ) )
	    pObjIndex->item_type = ITEM_CORPSE_PC;
	else if ( !str_cmp( tmp, "fountain" ) )
	    pObjIndex->item_type = ITEM_FOUNTAIN;
	else if ( !str_cmp( tmp, "pill" ) )
	    pObjIndex->item_type = ITEM_PILL;
	else if ( !str_cmp( tmp, "map" ) )
	    pObjIndex->item_type = ITEM_MAP;
	else if ( !str_cmp( tmp, "portal" ) )
	    pObjIndex->item_type = ITEM_PORTAL;
	else if ( !str_cmp( tmp, "warp_stone" ) )
	    pObjIndex->item_type = ITEM_WARP_STONE;
	else if ( !str_cmp( tmp, "gem" ) )
	    pObjIndex->item_type = ITEM_GEM;
	else if ( !str_cmp( tmp, "jewelry" ) )
	    pObjIndex->item_type = ITEM_JEWELRY;
	else if ( !str_cmp( tmp, "demonstone" ) )
	    pObjIndex->item_type = ITEM_DEMON_STONE;
	else if ( !str_cmp( tmp, "pit" ) )
	    pObjIndex->item_type = ITEM_PIT;
	else
	    pObjIndex->item_type = ITEM_TRASH;

        pObjIndex->extra_flags          = fread_flag( fp );
        pObjIndex->wear_flags           = fread_flag( fp );

	switch(pObjIndex->item_type)
	{
	case ITEM_WEAPON:
	    pObjIndex->value[0]		= weapon_type(fread_word(fp));
	    pObjIndex->value[1]		= fread_number(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= attack_lookup(fread_word(fp));
	    pObjIndex->value[4]		= fread_flag(fp);
	    break;
	case ITEM_CONTAINER:
	case ITEM_PIT:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= fread_flag(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= fread_number(fp);
	    pObjIndex->value[4]		= fread_number(fp);
	    break;
        case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
            pObjIndex->value[0]         = fread_number(fp);
            pObjIndex->value[1]         = fread_number(fp);
            pObjIndex->value[2]         = liq_lookup(fread_word(fp));
            pObjIndex->value[3]         = fread_number(fp);
            pObjIndex->value[4]         = fread_number(fp);
            break;
	case ITEM_WAND:
	case ITEM_STAFF:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= fread_number(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[4]		= fread_number(fp);
	    break;
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
 	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[2]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[3]		= skill_lookup(fread_word(fp));
	    pObjIndex->value[4]		= skill_lookup(fread_word(fp));
	    break;
	default:
            pObjIndex->value[0]             = fread_flag( fp );
            pObjIndex->value[1]             = fread_flag( fp );
            pObjIndex->value[2]             = fread_flag( fp );
            pObjIndex->value[3]             = fread_flag( fp );
	    pObjIndex->value[4]		    = fread_flag( fp );
	    break;
	}
	pObjIndex->level		= fread_number( fp );
        pObjIndex->weight               = fread_number( fp );
        pObjIndex->cost                 = fread_number( fp );

        for ( ; ; )
        {
            char letter;

            letter = fread_letter( fp );

            if ( letter == 'A' )
            {
                AFFECT_DATA *paf;

                paf                     = alloc_perm( sizeof(*paf) );
		paf->where		= TO_OBJECT;
                paf->type               = -1;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = fread_number( fp );
                paf->modifier           = fread_number( fp );
                paf->bitvector          = 0;
                paf->next               = pObjIndex->affected;
                pObjIndex->affected     = paf;
            }

//	    else if ( letter == 'B' ) 
//	    {
//		if ( !pObjIndex->special )
//		    pObjIndex->special = new_obj_specials( );
//
//		pObjIndex->special->forge_vnum	= fread_number( fp );
//		pObjIndex->special->forge_count	= fread_number( fp );
//	    }

            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA *ed;

                ed                      = alloc_perm( sizeof(*ed) );
                ed->keyword             = fread_string( fp );
                ed->description         = fread_string( fp );
                ed->next                = pObjIndex->extra_descr;
                pObjIndex->extra_descr  = ed;
                top_ed++;
            }

	    else if (letter == 'F')
            {
                AFFECT_DATA *paf;

                paf                     = alloc_perm( sizeof(*paf) );
		letter 			= fread_letter(fp);
		switch (letter)
	 	{
		case 'A':
                    paf->where          = TO_AFFECTS;
		    break;
		case 'I':
		case 'R':
		case 'V':
		    fread_number( fp );
		    fread_number( fp );
		    fread_flag( fp );
		    continue;
		case 'S':
		    paf->where		= TO_SHIELDS;
		    break;
		case 'D':
		    paf->where		= TO_DAM_MODS;
		    break;
		default:
            	    bug( "Load_objects: Bad where on flag set.", 0 );
            	   exit( 1 );
		}
                paf->type               = -1;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = fread_number(fp);
                paf->modifier           = fread_number(fp);
                paf->bitvector          = fread_flag(fp);
                paf->next		= pObjIndex->affected;
                pObjIndex->affected     = paf;
            }

	    else if ( letter == 'H' )
		pObjIndex->history	= fread_string( fp );

	    else if ( letter == 'O' )
	    {
		PROG_LIST *pOprog;
		char *word;
		int trigger = 0;

		pOprog			= alloc_perm(sizeof(*pOprog));
		word			= fread_word( fp );

		if ( !(trigger = flag_lookup( word, oprog_flags )) )
		{
		    bug( "OBJprogs: invalid trigger.",0);
		    exit(1);
		}

		SET_BIT( pObjIndex->oprog_flags, trigger );
		pOprog->trig_type	= trigger;
		pOprog->vnum	 	= fread_number( fp );
		pOprog->trig_phrase	= fread_string( fp );
		pOprog->next		= pObjIndex->oprogs;
		pObjIndex->oprogs	= pOprog;
	    }

	    else if ( letter == 'T' )
	    {
		char *word = fread_word( fp );

		if ( ( pos = class_lookup( word ) ) != -1 )
		    pObjIndex->class_can_use[pos] = FALSE;
	    }

            else
            {
                ungetc( letter, fp );
                break;
            }
        }

        iHash                   = vnum % MAX_KEY_HASH;
        pObjIndex->next         = obj_index_hash[iHash];
        obj_index_hash[iHash]   = pObjIndex;
    }

    return;
}
*/

#if defined LOAD_ASGARD_AREAS

bool load_asgard_area;

void load_area_asgard( FILE *fp )
{
    AREA_DATA *pArea;
    char      *word;
    bool      fMatch;

    load_asgard_area = TRUE;

    pArea               = alloc_perm( sizeof(*pArea) );
    pArea->age          = 15;
    pArea->nplayer      = 0;
    pArea->alignment	= '?';
    pArea->file_name    = str_dup( strArea );
    pArea->vnum         = top_area;
    pArea->name         = str_dup( "New Area" );
    pArea->security     = 9;
    pArea->min_vnum        = 0;
    pArea->max_vnum        = 0;
    pArea->area_flags   = 0;

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
           case 'N':
            SKEY( "Name", pArea->name );
            break;
           case 'S':
             KEY( "Security", pArea->security, fread_number( fp ) );
            break;
           case 'V':
            if ( !str_cmp( word, "VNUMs" ) )
            {
                pArea->min_vnum = fread_number( fp );
                pArea->max_vnum = fread_number( fp );
            }
            break;
           case 'E':
             if ( !str_cmp( word, "End" ) )
             {
                 fMatch = TRUE;
                 if ( area_first == NULL )
                    area_first = pArea;
                 if ( area_last  != NULL )
                    area_last->next = pArea;
                 area_last   = pArea;
                 pArea->next = NULL;

		SET_BIT( pArea->area_flags, AREA_UNLINKED );

                 top_area++;
                 return;
            }
            break;
           case 'B':
//            SKEY( "Builders", pArea->credits );
            break;
	   case 'C':
	    KEY( "Continent", fMatch, fread_number( fp ));
//	    SKEY( "Credits", pArea->credits );
	    break;
           case 'F':
            if ( !str_cmp( word, "Flags" ))
            {
              int aflags;
              aflags = fread_flag( fp );
              pArea->area_flags |= aflags;
            }
        }
    }
}

void load_rooms_asgard( FILE *fp )
{
    ROOM_INDEX_DATA *pRoomIndex;

    if ( area_last == NULL )
    {
	bug( "Load_rooms_asgard: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	int vnum;
	char letter;
	int door;
	int iHash;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_rooms: # not found.", 0 );
	    exit( 1 );
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	fBootDb = FALSE;
	if ( get_room_index( vnum ) != NULL )
	{
	    bug( "Load_rooms: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;

	pRoomIndex			= alloc_perm( sizeof(*pRoomIndex) );
	pRoomIndex->people		= NULL;
	pRoomIndex->contents		= NULL;
	pRoomIndex->area		= area_last;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->name		= fread_string( fp );
	pRoomIndex->description		= fread_string( fp );
	fread_number( fp );
	pRoomIndex->room_flags		= fread_flag( fp );
	pRoomIndex->sector_type		= fread_number( fp );
	pRoomIndex->light		= 0;

	for ( door = 0; door < MAX_DIR; door++ )
	    pRoomIndex->exit[door] = NULL;

	/* defaults */
	pRoomIndex->heal_rate = 100;
	pRoomIndex->mana_rate = 100;

	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'S' )
		break;

	    if ( letter == 'H')
		pRoomIndex->heal_rate = fread_number(fp);
	
	    else if ( letter == 'M')
		pRoomIndex->mana_rate = fread_number(fp);

	    else if ( letter == 'D' )
	    {
		EXIT_DATA *pexit;
		char locks[MAX_INPUT_LENGTH];

		door = fread_number( fp );
		if ( door < 0 || door >= MAX_DIR )
		{
		    bug( "Fread_rooms: vnum %d has bad door number.", vnum );
		    exit( 1 );
		}

		pexit			= alloc_perm( sizeof(*pexit) );
		pexit->description	= fread_string( fp );
		pexit->keyword		= fread_string( fp );
		pexit->exit_info	= 0;
                pexit->rs_flags         = 0;
		sprintf(locks, fread_word( fp ));
		pexit->key		= fread_number( fp );
		pexit->u1.vnum		= fread_number( fp );
		pexit->orig_door	= door;

		if (is_number(locks))
		{
			switch ( atoi(locks) )
			{
			case 1: pexit->exit_info = EX_ISDOOR;               
				pexit->rs_flags  = EX_ISDOOR;		     break;
			case 2: pexit->exit_info = EX_ISDOOR | EX_PICKPROOF;
				pexit->rs_flags  = EX_ISDOOR | EX_PICKPROOF; break;
			case 3: pexit->exit_info = EX_ISDOOR | EX_NOPASS;    
				pexit->rs_flags  = EX_ISDOOR | EX_NOPASS;    break;
			case 4: pexit->exit_info = EX_ISDOOR|EX_NOPASS|EX_PICKPROOF;
				pexit->rs_flags  = EX_ISDOOR|EX_NOPASS|EX_PICKPROOF;
				break;
			}
		}
		else
		{
			pexit->exit_info = flag_value( exit_flags, locks );
			pexit->rs_flags = flag_value( exit_flags, locks ); 
		}

		pRoomIndex->exit[door]	= pexit;
		top_exit++;
	    }
	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;

		ed			= alloc_perm( sizeof(*ed) );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= pRoomIndex->extra_descr;
		pRoomIndex->extra_descr	= ed;
		top_ed++;
	    }

	    else if (letter == 'O')
		fread_string(fp);

	    else
	    {
		bug( "Load_rooms: vnum %d has flag not 'DES'.", vnum );
		exit( 1 );
	    }
	}

	iHash			= vnum % MAX_KEY_HASH;
	pRoomIndex->next	= room_index_hash[iHash];
	room_index_hash[iHash]	= pRoomIndex;
	top_room++;
    }

    return;
}

void load_asgard_obj( FILE *fp )
{ 
    OBJ_INDEX_DATA *pObjIndex;
    char *spell;
  
    if ( !area_last )
    { 
        bug( "Load_objects: no #AREA seen yet.", 0 ); 
        exit( 1 ); 
    } 
 
    for ( ; ; ) 
    { 
        int vnum; 
        char letter; 
        int iHash; 
  
        letter                          = fread_letter( fp ); 
        if ( letter != '#' ) 
        { 
            bug( "Load_objects: # not found.", 0 ); 
            exit( 1 ); 
        } 
  
        vnum                            = fread_number( fp ); 
        if ( vnum == 0 ) 
            break; 

        fBootDb = FALSE; 
        if ( get_obj_index( vnum ) != NULL ) 
        { 
            bug( "Load_objects: vnum %d duplicated.", vnum ); 
            exit( 1 ); 
        } 
        fBootDb = TRUE; 

        pObjIndex                       = new_obj_index( );
        pObjIndex->vnum                 = vnum; 
        pObjIndex->area                 = area_last;            
        pObjIndex->reset_num            = 0; 
        pObjIndex->name                 = fread_string( fp ); 
        pObjIndex->short_descr          = fread_string( fp ); 
        pObjIndex->description          = fread_string( fp ); 

	spell = fread_string( fp );
	free_string( spell );
  

	spell = fread_word( fp );
	if ( !str_cmp( spell, "light" ) )
	    pObjIndex->item_type = ITEM_LIGHT;
	else if ( !str_cmp( spell, "scroll" ) )
	    pObjIndex->item_type = ITEM_SCROLL;
	else if ( !str_cmp( spell, "wand" ) )
	    pObjIndex->item_type = ITEM_WAND;
	else if ( !str_cmp( spell, "staff" ) )
	    pObjIndex->item_type = ITEM_STAFF;
	else if ( !str_cmp( spell, "weapon" ) )
	    pObjIndex->item_type = ITEM_WEAPON;
	else if ( !str_cmp( spell, "treasure" ) )
	    pObjIndex->item_type = ITEM_TREASURE;
	else if ( !str_cmp( spell, "armor" ) )
	    pObjIndex->item_type = ITEM_ARMOR;
	else if ( !str_cmp( spell, "potion" ) )
	    pObjIndex->item_type = ITEM_POTION;
	else if ( !str_cmp( spell, "clothing" ) )
	    pObjIndex->item_type = ITEM_CLOTHING;
	else if ( !str_cmp( spell, "furniture" ) )
	    pObjIndex->item_type = ITEM_FURNITURE;
	else if ( !str_cmp( spell, "trash" ) )
	    pObjIndex->item_type = ITEM_TRASH;
	else if ( !str_cmp( spell, "container" ) )
	    pObjIndex->item_type = ITEM_CONTAINER;
	else if ( !str_cmp( spell, "drink" ) )
	    pObjIndex->item_type = ITEM_DRINK_CON;
	else if ( !str_cmp( spell, "key" ) )
	    pObjIndex->item_type = ITEM_KEY;
	else if ( !str_cmp( spell, "food" ) )
	    pObjIndex->item_type = ITEM_FOOD;
	else if ( !str_cmp( spell, "money" ) )
	    pObjIndex->item_type = ITEM_MONEY;
	else if ( !str_cmp( spell, "boat" ) )
	    pObjIndex->item_type = ITEM_BOAT;
	else if ( !str_cmp( spell, "npc_corpse" ) )
	    pObjIndex->item_type = ITEM_CORPSE_NPC;
	else if ( !str_cmp( spell, "pc_corpse" ) )
	    pObjIndex->item_type = ITEM_CORPSE_PC;
	else if ( !str_cmp( spell, "fountain" ) )
	    pObjIndex->item_type = ITEM_FOUNTAIN;
	else if ( !str_cmp( spell, "pill" ) )
	    pObjIndex->item_type = ITEM_PILL;
	else if ( !str_cmp( spell, "map" ) )
	    pObjIndex->item_type = ITEM_MAP;
	else if ( !str_cmp( spell, "portal" ) )
	    pObjIndex->item_type = ITEM_PORTAL;
	else if ( !str_cmp( spell, "warp_stone" ) )
	    pObjIndex->item_type = ITEM_WARP_STONE;
	else if ( !str_cmp( spell, "gem" ) )
	    pObjIndex->item_type = ITEM_GEM;
	else if ( !str_cmp( spell, "jewelry" ) )
	    pObjIndex->item_type = ITEM_JEWELRY;
	else if ( !str_cmp( spell, "demonstone" ) )
	    pObjIndex->item_type = ITEM_DEMON_STONE;
	else if ( !str_cmp( spell, "pit" ) )
	    pObjIndex->item_type = ITEM_PIT;
	else
	    pObjIndex->item_type = ITEM_TRASH;

        pObjIndex->extra_flags          = fread_flag( fp ); 

	REMOVE_BIT( pObjIndex->extra_flags, ITEM_SPECIAL_SAVE );
	REMOVE_BIT( pObjIndex->extra_flags, ITEM_DISINTEGRATE );
	SET_BIT( pObjIndex->extra_flags, ITEM_NO_SAC );

        pObjIndex->wear_flags           = fread_flag( fp ); 

        switch(pObjIndex->item_type) 
        { 
        case ITEM_WEAPON: 
            pObjIndex->value[0]         = weapon_type(fread_word(fp)); 
            pObjIndex->value[1]         = fread_number(fp); 
            pObjIndex->value[2]         = fread_number(fp); 
            pObjIndex->value[3]         = attack_lookup(fread_word(fp)); 
            pObjIndex->value[4]         = fread_flag(fp); 
            break; 
        case ITEM_CONTAINER: 
        case ITEM_PIT: 
            pObjIndex->value[0]         = fread_number(fp); 
            pObjIndex->value[1]         = fread_flag(fp); 
            pObjIndex->value[2]         = fread_number(fp); 
            pObjIndex->value[3]         = fread_number(fp); 
            pObjIndex->value[4]         = fread_number(fp); 
            break; 
        case ITEM_DRINK_CON: 
        case ITEM_FOUNTAIN: 
            pObjIndex->value[0]         = fread_number(fp); 
            pObjIndex->value[1]         = fread_number(fp); 
            pObjIndex->value[2]         = liq_lookup(fread_word(fp)); 
            pObjIndex->value[3]         = fread_number(fp); 
            pObjIndex->value[4]         = fread_number(fp); 
            break; 
        case ITEM_WAND: 
        case ITEM_STAFF: 
            pObjIndex->value[0]         = fread_number(fp); 
            pObjIndex->value[1]         = fread_number(fp); 
            pObjIndex->value[2]         = fread_number(fp); 
            pObjIndex->value[3]         = skill_lookup(fread_word(fp)); 
            pObjIndex->value[4]         = fread_number(fp); 
            break; 
        case ITEM_POTION: 
        case ITEM_PILL: 
        case ITEM_SCROLL: 
            pObjIndex->value[0]         = fread_number(fp);
            pObjIndex->value[1]         = skill_lookup(fread_word(fp));
            pObjIndex->value[2]         = skill_lookup(fread_word(fp));
            pObjIndex->value[3]         = skill_lookup(fread_word(fp));
            pObjIndex->value[4]         = skill_lookup(fread_word(fp));
            break; 
        default: 
            pObjIndex->value[0]             = fread_flag( fp ); 
            pObjIndex->value[1]             = fread_flag( fp ); 
            pObjIndex->value[2]             = fread_flag( fp ); 
            pObjIndex->value[3]             = fread_flag( fp ); 
            pObjIndex->value[4]             = fread_flag( fp ); 
            break; 
        } 
        pObjIndex->level                = fread_number( fp ); 
        pObjIndex->weight               = fread_number( fp ); 
        pObjIndex->cost                 = fread_number( fp );

	fread_letter( fp );
  
        for ( ; ; ) 
        { 
            char letter; 
  
            letter = fread_letter( fp ); 
  
            if ( letter == 'A' ) 
            { 
                AFFECT_DATA *paf; 
  
                paf                     = alloc_perm( sizeof(*paf) ); 
                paf->where              = TO_OBJECT; 
                paf->type               = -1; 
                paf->level              = pObjIndex->level; 
                paf->duration           = -1; 
                paf->location           = fread_number( fp ); 
                paf->modifier           = fread_number( fp ); 
                paf->bitvector          = 0; 
                paf->next               = pObjIndex->affected; 
                pObjIndex->affected     = paf; 
            } 
 
            else if (letter == 'F') 
            { 
                AFFECT_DATA *paf; 
  
                paf                     = alloc_perm( sizeof(*paf) ); 
                letter                  = fread_letter(fp); 
                switch (letter) 
                { 
                case 'A': 
                    paf->where          = TO_AFFECTS; 
                    break; 
                case 'S': 
                    paf->where          = TO_SHIELDS; 
                    break; 
                default: 
                    bug( "Load_objects: Bad where on flag set.", 0 ); 
		    paf->where		= TO_OBJECT;
		    break;
//                   exit( 1 ); 
                } 
                paf->type               = -1; 
                paf->level              = pObjIndex->level; 
                paf->duration           = -1; 
                paf->location           = fread_number(fp); 
                paf->modifier           = fread_number(fp); 
                paf->bitvector          = fread_flag(fp); 
                paf->next               = pObjIndex->affected; 
                pObjIndex->affected     = paf; 
            } 
  
            else if ( letter == 'E' ) 
            { 
                EXTRA_DESCR_DATA *ed; 
  
                ed                      = alloc_perm( sizeof(*ed) ); 
                ed->keyword             = fread_string( fp ); 
                ed->description         = fread_string( fp ); 
                ed->next                = pObjIndex->extra_descr; 
                pObjIndex->extra_descr  = ed; 
                top_ed++; 
            } 
  
           else if ( letter == 'C') 
	   {
		spell = fread_string( fp );
		free_string( spell );
           } 
 
           else if ( letter == 'G') 
           { 
		spell = fread_string( fp );
		free_string( spell );
            } 
 
            else 
            { 
                ungetc( letter, fp ); 
                break; 
            } 
        } 
  
        iHash                   = vnum % MAX_KEY_HASH; 
        pObjIndex->next         = obj_index_hash[iHash]; 
        obj_index_hash[iHash]   = pObjIndex; 
    } 
  
    return; 
} 

void dam_convert( MOB_INDEX_DATA *pMobIndex, long flag, int type )
{
    if ( IS_SET( flag, B ) )
	pMobIndex->damage_mod[DAM_CHARM] = type;
    if ( IS_SET( flag, C ) )
	pMobIndex->damage_mod[DAM_MAGIC] = type;
    if ( IS_SET( flag, D ) )
    {
	pMobIndex->damage_mod[DAM_SLASH] = type;
	pMobIndex->damage_mod[DAM_BASH] = type;
	pMobIndex->damage_mod[DAM_PIERCE] = type;
    }
    if ( IS_SET( flag, E ) )
	pMobIndex->damage_mod[DAM_BASH] = type;
    if ( IS_SET( flag, F ) )
        pMobIndex->damage_mod[DAM_PIERCE] = type;
    if ( IS_SET( flag, G ) )
        pMobIndex->damage_mod[DAM_SLASH] = type;
    if ( IS_SET( flag, H ) )
        pMobIndex->damage_mod[DAM_FIRE] = type;
    if ( IS_SET( flag, I ) )
        pMobIndex->damage_mod[DAM_COLD] = type;
    if ( IS_SET( flag, J ) )
        pMobIndex->damage_mod[DAM_LIGHTNING] = type;
    if ( IS_SET( flag, K ) )
        pMobIndex->damage_mod[DAM_ACID] = type;
    if ( IS_SET( flag, L ) )
        pMobIndex->damage_mod[DAM_POISON] = type;
    if ( IS_SET( flag, M ) )
        pMobIndex->damage_mod[DAM_NEGATIVE] = type;
    if ( IS_SET( flag, N ) )
        pMobIndex->damage_mod[DAM_HOLY] = type;
    if ( IS_SET( flag, O ) )
        pMobIndex->damage_mod[DAM_ENERGY] = type;
    if ( IS_SET( flag, P ) )
        pMobIndex->damage_mod[DAM_MENTAL] = type;
    if ( IS_SET( flag, Q ) )
        pMobIndex->damage_mod[DAM_DISEASE] = type;
    if ( IS_SET( flag, R ) )
        pMobIndex->damage_mod[DAM_WATER] = type;
    if ( IS_SET( flag, S ) )
        pMobIndex->damage_mod[DAM_LIGHT] = type;
    if ( IS_SET( flag, T ) )
        pMobIndex->damage_mod[DAM_SOUND] = type;
    if ( IS_SET( flag, X ) )
        pMobIndex->damage_mod[DAM_WOOD] = type;
    if ( IS_SET( flag, Y ) )
        pMobIndex->damage_mod[DAM_SILVER] = type;
    if ( IS_SET( flag, Z ) )
        pMobIndex->damage_mod[DAM_IRON] = type;
}

void load_asgard_mob( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex; 
    char buf[MAX_INPUT_LENGTH];
    long flag;
    int dam[3], min, max, num, pos;
  
    if ( !area_last )
    { 
        bug( "Load_mobiles: no #AREA seen yet.", 0 ); 
        exit( 1 ); 
    } 
 
    for ( ; ; ) 
    { 
        int vnum; 
        char letter; 
        int iHash; 
  
        letter                          = fread_letter( fp ); 
        if ( letter != '#' ) 
        { 
            bug( "Load_mobiles: # not found.", 0 ); 
            exit( 1 ); 
        } 
  
        vnum                            = fread_number( fp ); 
        if ( vnum == 0 ) 
            break; 

        fBootDb = FALSE; 
        if ( get_mob_index( vnum ) != NULL ) 
        { 
            bug( "Load_mobiles: vnum %d duplicated.", vnum ); 
            exit( 1 ); 
        } 
        fBootDb = TRUE; 
  
        pMobIndex                       = new_mob_index( ); 
        pMobIndex->vnum                 = vnum; 
        pMobIndex->area                 = area_last;
        pMobIndex->player_name          = fread_string( fp ); 
        pMobIndex->short_descr          = fread_string( fp ); 
        pMobIndex->long_descr           = fread_string( fp ); 
        pMobIndex->description          = fread_string( fp ); 
        pMobIndex->race                 = race_lookup(fread_string( fp )); 

	if ( pMobIndex->race == -1 )
	    pMobIndex->race = race_lookup( "human" );
  
        pMobIndex->long_descr[0]        = UPPER(pMobIndex->long_descr[0]); 
        pMobIndex->description[0]       = UPPER(pMobIndex->description[0]); 
  
        pMobIndex->act                  = fread_flag( fp );
        pMobIndex->affected_by          = fread_flag( fp )
                                        | race_table[pMobIndex->race].aff; 
        pMobIndex->shielded_by          = fread_flag( fp ) 
                                        | race_table[pMobIndex->race].shd; 
        pMobIndex->pShop                = NULL; 
        pMobIndex->alignment            = fread_number( fp ); 
        pMobIndex->group                = fread_number( fp ); 
 
        pMobIndex->level                = fread_number( fp ); 
        pMobIndex->hitroll              = fread_number( fp );   
 
	dam[0] = fread_number(fp);
	fread_letter(fp);
	dam[1] = fread_number(fp);
	fread_letter(fp);
	dam[2] = fread_number( fp );
	min = 0; max = 0;
	for( pos = 0; pos < 50; pos++ )
	{
	    num = dice( dam[0], dam[1] ) + dam[2];
	    if ( num > max )
		max = num;
	    if ( num < min || min <= 0 )
		min = num;
	}
	pMobIndex->hit[0] = min;
	pMobIndex->hit[1] = max;

	dam[0] = fread_number(fp);
	fread_letter(fp);
	dam[1] = fread_number(fp);
	fread_letter(fp);
	dam[2] = fread_number( fp );
	min = 0; max = 0;
	for( pos = 0; pos < 50; pos++ )
	{
	    num = dice( dam[0], dam[1] ) + dam[2];
	    if ( num > max )
		max = num;
	    if ( num < min || min <= 0 )
		min = num;
	}
	pMobIndex->mana[0] = min;
	pMobIndex->mana[1] = max;

        pMobIndex->damage[DICE_NUMBER]  = fread_number( fp ); 
                                          fread_letter( fp ); 
        pMobIndex->damage[DICE_TYPE]    = fread_number( fp ); 
                                          fread_letter( fp ); 
        pMobIndex->damage[DICE_BONUS]   = fread_number( fp ); 
        pMobIndex->dam_type             = attack_lookup(fread_word(fp)); 
 
        pMobIndex->ac[AC_PIERCE]        = fread_number( fp ) * 10; 
        pMobIndex->ac[AC_BASH]          = fread_number( fp ) * 10; 
        pMobIndex->ac[AC_SLASH]         = fread_number( fp ) * 10; 
        pMobIndex->ac[AC_EXOTIC]        = fread_number( fp ) * 10; 
 
	fread_flag( fp );

	flag = fread_flag( fp );
	dam_convert( pMobIndex, flag, 0 );

	flag = fread_flag( fp );
	dam_convert( pMobIndex, flag, 50 );

	flag = fread_flag( fp );
	dam_convert( pMobIndex, flag, 150 );

	pMobIndex->start_pos		= flag_value( position_flags, fread_word( fp ) );
	pMobIndex->default_pos		= flag_value( position_flags, fread_word( fp ) );

	strcpy( buf, fread_word( fp ) );
	if ( !str_cmp( buf, "none" ) )
	    pMobIndex->sex = SEX_NEUTRAL;
	else if ( !str_cmp( buf, "male" ) )
	    pMobIndex->sex = SEX_MALE;
	else if ( !str_cmp( buf, "female" ) )
	    pMobIndex->sex = SEX_FEMALE;
	else
	    pMobIndex->sex = SEX_RANDOM;

        pMobIndex->wealth               = fread_number( fp ); 
 
	fread_flag( fp );
        pMobIndex->parts                = fread_flag( fp ) 
                                        | race_table[pMobIndex->race].parts; 
	pMobIndex->size   = flag_value( size_flags, fread_word( fp ) );

	fread_word( fp );
 
        pMobIndex->die_descr            = ""; 
        pMobIndex->say_descr            = ""; 
  
        for ( ; ; ) 
        { 
            letter = fread_letter( fp ); 
 
            if (letter == 'F') 
            { 
                char *word; 
                long vector; 
 
                word                    = fread_word(fp); 
                vector                  = fread_flag(fp); 
 
                if (!str_prefix(word,"act")) 
                    REMOVE_BIT(pMobIndex->act,vector); 
                else if (!str_prefix(word,"aff")) 
                    REMOVE_BIT(pMobIndex->affected_by,vector); 
                else if (!str_prefix(word,"shd")) 
                    REMOVE_BIT(pMobIndex->shielded_by,vector); 
                else if (!str_prefix(word,"par")) 
                    REMOVE_BIT(pMobIndex->parts,vector); 
                else 
                    bug("Flag remove: flag not found.",0); 
             } 
             else if (letter == 'D')
             { 
                pMobIndex->die_descr            = fread_string( fp ); 
             } 
             else if (letter == 'T')
             { 
                pMobIndex->say_descr            = fread_string( fp ); 
                pMobIndex->say_descr[0]         = UPPER(pMobIndex->say_descr[0]); 
             } 
             else if ( letter == 'M' )
             { 
                PROG_LIST *pMprog; 
                char *word; 
                int trigger = 0; 
 
                pMprog = alloc_perm(sizeof(*pMprog)); 
                word   = fread_word( fp ); 
                if ( !(trigger = flag_lookup( word, mprog_flags )) ) 
                { 
                    bug("MOBprogs: invalid trigger.",0); 
                    exit(1); 
                } 
                SET_BIT( pMobIndex->mprog_flags, trigger ); 
                pMprog->trig_type       = trigger; 
                pMprog->vnum            = fread_number( fp ); 
                pMprog->trig_phrase     = fread_string( fp ); 
                pMprog->next            = pMobIndex->mprogs; 
                pMobIndex->mprogs       = pMprog; 
             } 
             else 
             { 
                ungetc(letter,fp); 
                break; 
             } 
        } 
 
        iHash                   = vnum % MAX_KEY_HASH; 
        pMobIndex->next         = mob_index_hash[iHash]; 
        mob_index_hash[iHash]   = pMobIndex; 
    } 
  
    return; 
}
#endif

void load_voting_polls()
{
    FILE *list, *fp;
    POLL_DATA *poll;
    VOTE_DATA *vote;
    char name[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];

    system( "dir -1 ../data/voting_polls >../data/voting_polls/polls.txt" );

    if ( (list = fopen("../data/voting_polls/polls.txt", "a")) != NULL )
    {
	fprintf( list, "#\n\r" );
	fclose( list );
    }

    if ( (list = fopen("../data/voting_polls/polls.txt", "r")) != NULL )
    {
	for ( ; ; )
	{
	    sh_int pos = 0;

	    strcpy( name, fread_word( list ) );

	    if ( name[0] == '#' )
		break;

	    if ( !str_cmp( name, "polls.txt" ) 
	    ||   !str_cmp( name, "CVS" ) )
		continue;

	    sprintf( buf, "../data/voting_polls/%s", name );
	    if ( (fp = fopen(buf, "r")) == NULL )
	    {
		bug( "Can not access voting_poll file.", 0 );
		continue;
	    }

	    poll	= new_poll();
	    poll->name	= str_dup( name );
	    poll->next	= first_poll;
	    first_poll	= poll;

	    for ( ; ; )
	    {
		char letter = fread_letter( fp );

		if ( letter == '#' )
		    break;

		else if ( letter == 'Q' )
		{
		    poll->question = fread_string( fp );
		}

		else if ( letter == 'A' )
		{
		    poll->response[pos] = fread_string( fp );
		    pos++;
		}

		else if ( letter == 'V' )
		{
		    vote = new_vote();

		    vote->voter_name	= fread_string( fp );
		    vote->voter_ip	= fread_string( fp );
		    vote->voter_vote	= fread_number( fp );
		    vote->next_vote	= poll->vote;
		    poll->vote		= vote;
		}
	    }

	    fclose( fp );
	}
    }

    fclose( list );

    unlink( "../data/voting_polls/polls.txt" );

    return;
}

void load_area( FILE *fp )
{
    AREA_DATA *pArea;
    char      *word;
    bool      fMatch;

    #if defined LOAD_DS_AREAS
	load_ds_area = FALSE;
    #endif

    #if defined LOAD_ASHEN_AREAS
	load_ashen_area = FALSE;
    #endif

    #if defined LOAD_ASGARD_AREAS
	load_asgard_area = FALSE;
    #endif

    pArea               = alloc_perm( sizeof( *pArea ) );
    pArea->name		= fread_string( fp );
//    pArea->credits	= fread_string( fp );
    pArea->min_vnum	= fread_number( fp );
    pArea->max_vnum	= fread_number( fp );
    pArea->area_flags	= fread_flag( fp );
    pArea->security	= fread_number( fp );
    pArea->age          = 15;
    pArea->nplayer      = 0;
    pArea->vnum         = top_area;
    pArea->file_name    = str_dup( strArea );
    pArea->music	= NULL;
    pArea->clan		= 0;
    pArea->run_vnum	= 0;
    pArea->min_level	= 0;
    pArea->max_level	= 0;
    pArea->builder	= NULL;
    pArea->directions	= NULL;
    pArea->alignment	= '?';

    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER( word[0] ) )
       {
	    case 'A':
		KEY( "Alig", pArea->alignment, fread_letter( fp ) );
		break;

	    case 'B':
		SKEY( "Bldr",	pArea->builder	);
		break;

	    case 'C':
		if ( !str_cmp( word, "Clan" ) )
		{
		    char *tmp = fread_string( fp );

		    pArea->clan = clan_lookup( tmp );
		    free_string( tmp );
		    break;
		}
		break;

	    case 'E':
		if ( !str_cmp( word, "End" ) )
		{
		    fMatch = TRUE;
		    if ( area_first == NULL )
			area_first = pArea;
		    if ( area_last  != NULL )
			area_last->next = pArea;
		    area_last   = pArea;
		    pArea->next = NULL;
		    top_area++;
		    return;
		}
		break;

	    case 'L':
		if ( !str_cmp( word, "Levl" ) )
		{
		    pArea->min_level = fread_number( fp );
		    pArea->max_level = fread_number( fp );
		    break;
		}
		break;

	    case 'M':
		SKEY( "Musi", pArea->music );
		break;

	    case 'P':
		SKEY( "Path", pArea->directions );
		break;

	    case 'R':
		KEY( "Room", pArea->run_vnum,	fread_number( fp ) );
		break;
        }
    }
}

void load_help( FILE *fp )
{
    HELP_DATA *pHelp = alloc_perm( sizeof( *pHelp ) );
    top_help++;

    for( ; ; )
    {
	char *word = fread_word( fp );

	switch( UPPER( *word ) )
	{
	    case 'C':
		if ( !str_cmp( word, "Clan" ) )
		{
		    char *temp = fread_string( fp );
		    pHelp->clan = clan_lookup( temp );
		    free_string( temp );
		    break;
		}

		break;

	    case 'E':
		if ( !str_cmp( word, "End" ) )
		{
		    if ( !str_cmp( pHelp->keyword, "'GREETING1'" ) )
			help_greeting1 = pHelp->text;
		    else if ( !str_cmp( pHelp->keyword, "'GREETING2'" ) )
			help_greeting2 = pHelp->text;
		    else if ( !str_cmp( pHelp->keyword, "'GREETING3'" ) )
			help_greeting3 = pHelp->text;
		    else if ( !str_cmp( pHelp->keyword, "'GREETING4'" ) )
			help_greeting4 = pHelp->text;
		    else if ( !str_cmp( pHelp->keyword, "'GREETING5'" ) )
			help_greeting5 = pHelp->text;
		    else if ( !str_cmp( pHelp->keyword, "'GREETING6'" ) )
			help_greeting6 = pHelp->text;
		    else if ( !str_cmp( pHelp->keyword, "'GREETING7'" ) )
			help_greeting7 = pHelp->text;
		    else if ( !str_cmp( pHelp->keyword, "'GREETING8'" ) )
			help_greeting8 = pHelp->text;
		    else if ( !str_cmp( pHelp->keyword, "'GREETING9'" ) )
			help_greeting9 = pHelp->text;
		    else if ( !str_cmp( pHelp->keyword, "'GREETING10'" ) )
			help_greeting10 = pHelp->text;
		    else if ( !str_cmp( pHelp->keyword, "'GREETING11'" ) )
			help_greeting11 = pHelp->text;
		    else if ( !str_cmp( pHelp->keyword, "'GREETING12'" ) )
			help_greeting12 = pHelp->text;
		    else if ( !str_cmp( pHelp->keyword, "'AUTHORS'" ) )
			help_authors = pHelp->text;

		    if ( help_first == NULL )
			help_first = pHelp;

		    if ( help_last  != NULL )
			help_last->next = pHelp;

		    help_last	= pHelp;
		    pHelp->next	= NULL;
		    return;
		}
		break;

	    case 'K':
		LOAD( "Keyw", pHelp->keyword, fread_string( fp ) );
		break;

	    case 'N':
		LOAD( "Name", pHelp->name, fread_string( fp ) );
		break;

	    case 'L':
		LOAD( "Levl", pHelp->level, fread_number( fp ) );
		break;

	    case 'T':
		LOAD( "Text", pHelp->text, fread_string( fp ) );
		break;
	}
    }
}

void load_helps( void )
{
    FILE *fp;

    if ( ( fp = fopen( HELPS_FILE, "r" ) ) == NULL )
    {
	bug( "Load_helps: NULL file.", 0 );
	return;
    }

    for ( ; ; )
    {
	char *word = fread_word( fp );

	if ( !str_cmp( word, "#HELP" ) )
	    load_help( fp );

	else if ( !str_cmp( word, "#END" ) )
	    break;
    }

    fclose( fp );
    return;
}

void load_mobiles( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
    char *temp;
    sh_int pos;

    #if defined LOAD_DS_AREAS
	if ( load_ds_area )
	{
	    load_mobiles_ds( fp );
	    return;
	}
    #endif

    #if defined LOAD_ASHEN_AREAS
	if ( load_ashen_area )
	{
	    load_mobiles_ashen( fp );
	    return;
	}
    #endif

    #if defined LOAD_ASGARD_AREAS
	if ( load_asgard_area )
	{
	    load_asgard_mob( fp );
	    return;
	}
    #endif

    if ( !area_last )
    {
	bug( "Load_mobiles: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	char letter = fread_letter( fp );
	int iHash, vnum;

	if ( letter != '#' )
	{
	    bug( "Load_mobiles: # not found.", 0 );
	    exit( 1 );
	}

	if ( ( vnum = fread_number( fp ) ) == 0 )
	    break;

	fBootDb = FALSE;
	if ( get_mob_index( vnum ) != NULL )
	{
	    bug( "Load_mobiles: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;

	pMobIndex			= new_mob_index( );
	pMobIndex->vnum			= vnum;
	pMobIndex->area			= area_last;
	pMobIndex->pShop		= NULL;
	pMobIndex->die_descr		= &str_empty[0];
	pMobIndex->say_descr		= &str_empty[0];

	pMobIndex->player_name		= fread_string( fp );
	pMobIndex->short_descr		= fread_string( fp );
	pMobIndex->long_descr		= fread_string( fp );
	pMobIndex->description		= fread_string( fp );

	temp = fread_string( fp );
	pMobIndex->race			= race_lookup( temp );
	free_string( temp );

	if ( pMobIndex->race == -1 )
	    pMobIndex->race = race_lookup( "human" );

	for ( pos = 0; pos < DAM_MAX; pos++ )
	    pMobIndex->damage_mod[pos] = fread_number( fp );

	pMobIndex->act			= fread_flag( fp );
	pMobIndex->affected_by		= fread_flag( fp )
					| race_table[pMobIndex->race].aff;
	pMobIndex->shielded_by		= fread_flag( fp )
					| race_table[pMobIndex->race].shd;
	pMobIndex->alignment		= fread_number( fp );
	pMobIndex->group		= fread_number( fp );
	pMobIndex->level		= fread_number( fp );
	pMobIndex->hitroll		= fread_number( fp );
	pMobIndex->saves		= fread_number( fp );
	pMobIndex->hit[0]		= fread_number( fp );
	pMobIndex->hit[1]		= fread_number( fp );
	pMobIndex->mana[0]		= fread_number( fp );
	pMobIndex->mana[1]		= fread_number( fp );
	pMobIndex->damage[DICE_NUMBER]	= fread_number( fp );
	pMobIndex->damage[DICE_TYPE]	= fread_number( fp );
	pMobIndex->damage[DICE_BONUS]	= fread_number( fp );
	pMobIndex->dam_type		= attack_lookup( fread_word( fp ) );
	pMobIndex->ac[AC_PIERCE]	= fread_number( fp );
	pMobIndex->ac[AC_BASH]		= fread_number( fp );
	pMobIndex->ac[AC_SLASH]		= fread_number( fp );
	pMobIndex->ac[AC_EXOTIC]	= fread_number( fp );
	pMobIndex->start_pos		= flag_value( position_flags, fread_word( fp ) );
	pMobIndex->default_pos		= flag_value( position_flags, fread_word( fp ) );
	pMobIndex->sex			= flag_value( sex_flags, fread_word( fp ) );
	pMobIndex->wealth		= fread_number( fp );
	pMobIndex->skill_percentage	= fread_number( fp );

	for ( pos = 0; pos < maxSkill; pos++ )
	    pMobIndex->learned[pos] = pMobIndex->skill_percentage;

	pMobIndex->class	= class_lookup( fread_word( fp ) );
	pMobIndex->class	= UMAX( 0, pMobIndex->class );

	pMobIndex->size		= flag_value( size_flags, fread_word( fp ) );
	pMobIndex->parts	= flag_lookup( fread_word( fp ), part_flags )
				| race_table[pMobIndex->race].parts;

	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'A' )
		pMobIndex->absorption = fread_number( fp );

	    else if ( letter == 'B' )
		pMobIndex->bank_branch = fread_number( fp );

	    else if ( letter == 'D' )
		pMobIndex->die_descr = fread_string( fp );

	    else if ( letter == 'E' )
		pMobIndex->exp_percent = fread_number( fp );

	    else if ( letter == 'F' )
		pMobIndex->regen[0] = fread_number( fp );

	    else if ( letter == 'G' )
		pMobIndex->regen[1] = fread_number( fp );

	    else if ( letter == 'H' )
		pMobIndex->regen[2] = fread_number( fp );

	    else if ( letter == 'M' )
	    {
		PROG_LIST *pMprog;
		char *word;
		int trigger = 0;

		pMprog = alloc_perm( sizeof( *pMprog ) );
		word = fread_word( fp );

		if ( !( trigger = flag_lookup( word, mprog_flags ) ) )
		{
		    bug( "MOBprogs: invalid trigger.", 0 );
		    exit( 1 );
		}

		SET_BIT( pMobIndex->mprog_flags, trigger );
		pMprog->trig_type	= trigger;
		pMprog->vnum		= fread_number( fp );
		pMprog->trig_phrase	= fread_string( fp );
		pMprog->next		= pMobIndex->mprogs;
		pMobIndex->mprogs	= pMprog;
	    }

	    else if ( letter == 'P' )
	    {
		pMobIndex->perm_mob_pc_deaths[0] = fread_number( fp );
		pMobIndex->perm_mob_pc_deaths[1] = fread_number( fp );
	    }

	    else if ( letter == 'R' )
		pMobIndex->reflection = fread_number( fp );

	    else if ( letter == 'S' )
	    {
		temp = fread_word( fp );
		if ( ( pos = skill_lookup( temp ) ) == -1 )
		{
		    bug( "Load_mobiles: unknown skill.", 0  );
		    fread_number( fp );
		} else
		    pMobIndex->learned[pos] = fread_number( fp );
	    }

	    else if ( letter == 'T' )
		pMobIndex->say_descr = fread_string( fp );

	    else if ( letter == 'W' )
		pMobIndex->max_world = fread_number( fp );

	    else
	    {
		ungetc( letter, fp );
		break;
	    }
	}

	iHash			= vnum % MAX_KEY_HASH;
	pMobIndex->next		= mob_index_hash[iHash];
	mob_index_hash[iHash]	= pMobIndex;
    }

    return;
}

void load_objects( FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
    char *word;
    sh_int pos;

    #if defined LOAD_DS_AREAS
	if ( load_ds_area )
	{
	    load_objects_ds( fp );
	    return;
	}
    #endif

    #if defined LOAD_ASHEN_AREAS
	if ( load_ashen_area )
	{
	    load_objects_ashen( fp );
	    return;
	}
    #endif

    #if defined LOAD_ASGARD_AREAS
	if ( load_asgard_area )
	{
	    load_asgard_obj( fp );
	    return;
	}
    #endif

    if ( !area_last )
    {
	bug( "Load_objects: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	char letter;
	int iHash, vnum;

	if ( ( letter = fread_letter( fp ) ) != '#' )
	{
	    bug( "Load_objects: # not found.", 0 );
	    exit( 1 );
	}

	if ( ( vnum = fread_number( fp ) ) == 0 )
	    break;

	fBootDb = FALSE;
	if ( get_obj_index( vnum ) != NULL )
	{
	    bug( "Load_objects: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;

	pObjIndex			= new_obj_index( );
	pObjIndex->vnum			= vnum;
	pObjIndex->size			= SIZE_NONE;
	pObjIndex->area			= area_last;
	pObjIndex->reset_num		= 0;
	pObjIndex->name			= fread_string( fp );
	pObjIndex->short_descr		= fread_string( fp );
	pObjIndex->description		= fread_string( fp );
	pObjIndex->item_type		= flag_value( type_flags, fread_word( fp ) );
	pObjIndex->extra_flags		= fread_flag( fp );
	pObjIndex->wear_flags		= fread_flag( fp );

	switch( pObjIndex->item_type )
	{
	    case ITEM_WEAPON:
		pObjIndex->value[0]	= weapon_type( fread_word( fp ) );
		pObjIndex->value[1]	= fread_number( fp );
		pObjIndex->value[2]	= fread_number( fp );
		pObjIndex->value[3]	= attack_lookup( fread_word( fp ) );
		pObjIndex->value[4]	= fread_flag( fp );
		break;

	    case ITEM_CONTAINER:
	    case ITEM_PIT:
		pObjIndex->value[0]	= fread_number( fp );
		pObjIndex->value[1]	= fread_flag( fp );
		pObjIndex->value[2]	= fread_number( fp );
		pObjIndex->value[3]	= fread_number( fp );
		pObjIndex->value[4]	= fread_number( fp );
		break;

	    case ITEM_DRINK_CON:
	    case ITEM_FOUNTAIN:
		pObjIndex->value[0]	= fread_number( fp );
		pObjIndex->value[1]	= fread_number( fp );
		pObjIndex->value[2]	= liq_lookup( fread_word( fp ) );
		pObjIndex->value[3]	= fread_number( fp );
		pObjIndex->value[4]	= fread_number( fp );
		break;

	    case ITEM_WAND:
	    case ITEM_STAFF:
		pObjIndex->value[0]	= fread_number( fp );
		pObjIndex->value[1]	= fread_number( fp );
		pObjIndex->value[2]	= fread_number( fp );
		pObjIndex->value[3]	= skill_lookup( fread_word( fp ) );
		pObjIndex->value[4]	= fread_number( fp );
		break;

	    case ITEM_TRAP:
		word			= fread_word( fp );
		pObjIndex->value[0]	= URANGE( 0, flag_value( trap_type_table, word ), TRAP_MAX );
		word			= fread_word( fp );
		pObjIndex->value[1]	= URANGE( 0, dam_type_lookup( word ), DAM_MAX );
		pObjIndex->value[2]	= fread_number( fp );
		pObjIndex->value[3]	= fread_number( fp );
		pObjIndex->value[4]	= fread_number( fp );
		break;

	    case ITEM_POTION:
	    case ITEM_PILL:
	    case ITEM_SCROLL:
		pObjIndex->value[0]	= fread_number( fp );
		pObjIndex->value[1]	= skill_lookup( fread_word( fp ) );
		pObjIndex->value[2]	= skill_lookup( fread_word( fp ) );
		pObjIndex->value[3]	= skill_lookup( fread_word( fp ) );
		pObjIndex->value[4]	= skill_lookup( fread_word( fp ) );
		break;

	    default:
		pObjIndex->value[0]	= fread_flag( fp );
		pObjIndex->value[1]	= fread_flag( fp );
		pObjIndex->value[2]	= fread_flag( fp );
		pObjIndex->value[3]	= fread_flag( fp );
		pObjIndex->value[4]	= fread_flag( fp );
		break;

	}

	pObjIndex->level	= fread_number( fp );
	pObjIndex->weight	= fread_number( fp );
	pObjIndex->cost		= fread_number( fp );

	for ( ; ; )
	{
	    char letter = fread_letter( fp );

	    if ( letter == 'A' )
	    {
		AFFECT_DATA *paf	= new_affect( );
		paf->where		= TO_OBJECT;
		paf->type		= -1;
		paf->level		= pObjIndex->level;
		paf->dur_type		= DUR_TICKS;
		paf->duration		= -1;
		paf->location		= fread_number( fp );
		paf->modifier		= fread_number( fp );
		paf->bitvector		= 0;
		paf->next		= pObjIndex->affected;
		pObjIndex->affected	= paf;
	    }

	    else if ( letter == 'B' )  /* forged item */
	    {
		pObjIndex->forge_vnum	= fread_number( fp );
		pObjIndex->forge_count	= fread_number( fp );
	    }

	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed	= alloc_perm( sizeof( *ed ) );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= pObjIndex->extra_descr;
		pObjIndex->extra_descr	= ed;
		top_ed++;
	    }

	    else if ( letter == 'F' )
	    {
		AFFECT_DATA *paf = new_affect( );

		letter = fread_letter( fp );
		switch( letter )
		{
		    case 'A':	paf->where = TO_AFFECTS;	break;
		    case 'D':	paf->where = TO_DAM_MODS;	break;
		    case 'S':	paf->where = TO_SHIELDS;	break;
		    default:
			sprintf( log_buf, "Load_objects[%d]: Bad where on flag set.", vnum );
			bug( log_buf, 0 );
			exit( 1 );
		}

		paf->type		= -1;
		paf->level		= pObjIndex->level;
		paf->dur_type		= DUR_TICKS;
		paf->duration		= -1;
		paf->location		= fread_number( fp );
		paf->modifier		= fread_number( fp );
		paf->bitvector		= fread_flag( fp );
		paf->next		= pObjIndex->affected;
		pObjIndex->affected	= paf;
	    }

	    else if ( letter == 'H' )
	    {
		char *temp		= fread_string( fp );
		pObjIndex->size		= flag_value( size_flags, temp );
		free_string( temp );
            }

	    else if ( letter == 'M' )
		pObjIndex->history = fread_string( fp );

	    else if ( letter == 'O' )
	    {
		PROG_LIST *pOprog;
		char *word;
		int trigger = 0;

		pOprog = alloc_perm( sizeof( *pOprog ) );
		word = fread_word( fp );

		if ( !( trigger = flag_lookup( word, oprog_flags ) ) )
		{
		    bug( "OBJprogs: invalid trigger.", 0 );
		    exit( 1 );
		}

		SET_BIT( pObjIndex->oprog_flags, trigger );
		pOprog->trig_type	= trigger;
		pOprog->vnum		= fread_number( fp );
		pOprog->trig_phrase	= fread_string( fp );
		pOprog->next		= pObjIndex->oprogs;
		pObjIndex->oprogs	= pOprog;
	    }

	    else if ( letter == 'Q' ) /* Quest */
		pObjIndex->quest_points = fread_number( fp );

	    else if ( letter == 'R' )
		pObjIndex->targets = fread_number( fp );

	    else if ( letter == 'S' )
		pObjIndex->success = fread_number( fp );

	    else if ( letter == 'T' )
	    {
		char *word = fread_word( fp );

		if ( ( pos = class_lookup( word ) ) != -1 )
		    pObjIndex->class_can_use[pos] = FALSE;
	    }

	    else
	    {
		ungetc( letter, fp );
		break;
	    }
	}

	iHash			= vnum % MAX_KEY_HASH;
	pObjIndex->next		= obj_index_hash[iHash];
	obj_index_hash[iHash]	= pObjIndex;
    }

    return;
}

void load_rooms( FILE *fp )
{
    EXIT_DATA *pexit;
    EXTRA_DESCR_DATA *ed;
    PROG_LIST *pRprog;
    ROOM_DAMAGE_DATA *dam;
    ROOM_INDEX_DATA *pRoomIndex;
    char *word;
    int trigger = 0;

    #if defined LOAD_DS_AREAS
	if ( load_ds_area )
	{
	    load_rooms_ds( fp );
	    return;
	}
    #endif

    #if defined LOAD_ASHEN_AREAS
	if ( load_ashen_area )
	{
	    load_rooms_ashen( fp );
	    return;
	}
    #endif

    #if defined LOAD_ASGARD_AREAS
	if ( load_asgard_area )
	{
	    load_rooms_asgard( fp );
	    return;
	}
    #endif

    if ( area_last == NULL )
    {
	bug( "Load_rooms: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	char letter;
	int door, iHash, vnum;

	if ( ( letter = fread_letter( fp ) ) != '#' )
	{
	    bug( "Load_rooms: # not found.", 0 );
	    exit( 1 );
	}

	if ( ( vnum = fread_number( fp ) ) == 0 )
	    break;

	fBootDb = FALSE;
	if ( get_room_index( vnum ) != NULL )
	{
	    bug( "Load_rooms: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;

	pRoomIndex			= alloc_perm( sizeof( *pRoomIndex ) );
	pRoomIndex->people		= NULL;
	pRoomIndex->contents		= NULL;
	pRoomIndex->extra_descr		= NULL;
	pRoomIndex->music		= NULL;
	pRoomIndex->area		= area_last;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->name		= fread_string( fp );
	pRoomIndex->description		= fread_string( fp );
	pRoomIndex->room_flags		= fread_flag( fp );
	pRoomIndex->sector_type		= fread_number( fp );
	pRoomIndex->light		= 0;
	pRoomIndex->heal_rate		= 100;
	pRoomIndex->mana_rate		= 100;

	for ( door = 0; door < MAX_DIR; door++ )
	    pRoomIndex->exit[door] = NULL;

	for ( ; ; )
	{
	    if ( ( letter = fread_letter( fp ) ) == 'S' )
		break;

	    switch( letter )
	    {
		case 'A':
		    dam				= new_room_damage( dam_type_lookup( fread_word( fp ) ) );
		    dam->damage_min		= fread_number( fp );
		    dam->damage_max		= fread_number( fp );
		    dam->success		= fread_number( fp );
		    dam->msg_victim		= fread_string( fp );
		    dam->msg_room		= fread_string( fp );
		    dam->next			= pRoomIndex->room_damage;
		    pRoomIndex->room_damage	= dam;
		    break;

		case 'D':
		    door = fread_number( fp );
		    if ( door < 0 || door >= MAX_DIR )
		    {
			bug( "Fread_rooms: vnum %d has bad door number.", vnum );
			exit( 1 );
		    }

		    pexit			= alloc_perm( sizeof( *pexit ) );
		    pexit->description		= fread_string( fp );
		    pexit->keyword		= fread_string( fp );
		    pexit->exit_info		= fread_flag( fp );
		    pexit->rs_flags		= pexit->exit_info;
		    pexit->key			= fread_number( fp );
		    pexit->u1.vnum		= fread_number( fp );
		    pexit->orig_door		= door;
		    pRoomIndex->exit[door]	= pexit;
		    top_exit++;
		    break;

		case 'E':
		    ed				= alloc_perm( sizeof( *ed ) );
		    ed->keyword			= fread_string( fp );
		    ed->description		= fread_string( fp );
		    ed->next			= pRoomIndex->extra_descr;
		    pRoomIndex->extra_descr	= ed;
		    top_ed++;
		    break;

		case 'H':
		    pRoomIndex->heal_rate = fread_number( fp );
		    pRoomIndex->mana_rate = fread_number( fp );
		    break;

		case 'N':
		    pRoomIndex->music = fread_string( fp );
		    break;

		case 'P':
		    pRoomIndex->max_people = fread_number( fp );
		    break;

		case 'R':
		    pRprog		= alloc_perm( sizeof( *pRprog ) );
		    word		= fread_word( fp );

		    if ( !( trigger = flag_lookup( word, rprog_flags ) ) )
		    {
			bug( "ROOMprogs: invalid trigger.", 0 );
			exit( 1 );
		    }

		    SET_BIT( pRoomIndex->rprog_flags, trigger );
		    pRprog->trig_type	= trigger;
		    pRprog->vnum	= fread_number( fp );
		    pRprog->trig_phrase	= fread_string( fp );
		    pRprog->next	= pRoomIndex->rprogs;
		    pRoomIndex->rprogs	= pRprog;
		    break;

		default:
		    bug( "Load_rooms: vnum %d has flag not 'DEF'.", vnum );
		    exit( 1 );
		    break;
	    }
	}

	iHash			= vnum % MAX_KEY_HASH;
	pRoomIndex->next	= room_index_hash[iHash];
	room_index_hash[iHash]	= pRoomIndex;
	top_room++;
    }

    return;
}

void load_resets( FILE *fp )
{
    RESET_DATA *pReset;
    int iLastObj = 0, iLastRoom = 0;

    if ( !area_last )
    {
	bug( "Load_resets: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	EXIT_DATA *pexit;
	OBJ_INDEX_DATA *temp_index;
	ROOM_INDEX_DATA *pRoomIndex;
	char letter;

	if ( ( letter = fread_letter( fp ) ) == 'S' )
	    break;

	if ( letter == '*' )
	{
	    fread_to_eol( fp );
	    continue;
	}

	top_reset++;
	pReset		= alloc_perm( sizeof( *pReset ) );
	pReset->command	= letter;
	pReset->percent	= fread_number( fp );
	pReset->arg1	= fread_number( fp );
	pReset->arg2	= fread_number( fp );
	pReset->arg3	= ( letter == 'G' || letter == 'R' )
			    ? 0 : fread_number( fp );
	pReset->arg4	= ( letter == 'P' || letter == 'M' )
			    ? fread_number( fp ) : 0;
	fread_to_eol( fp );

	if ( pReset->percent == 0 )
	    pReset->percent = 100;

	switch ( letter )
	{
	    default:
		bug( "Load_resets: bad command '%c'.", letter );
		exit( 1 );
		break;

	    case 'M':
		get_mob_index( pReset->arg1 );
		if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) )
		{
		    new_reset( pRoomIndex, pReset );
		    iLastRoom = pReset->arg3;
		}
		break;

	    case 'O':
		temp_index = get_obj_index( pReset->arg1 );
		temp_index->reset_num++;
		if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) )
		{
		    new_reset( pRoomIndex, pReset );
		    iLastObj = pReset->arg3;
		}
		break;

	    case 'P':
		temp_index = get_obj_index( pReset->arg1 );
		temp_index->reset_num++;
		if ( ( pRoomIndex = get_room_index( iLastObj ) ) )
		    new_reset( pRoomIndex, pReset );
		break;

	    case 'G':
	    case 'E':
		temp_index = get_obj_index( pReset->arg1 );
		temp_index->reset_num++;
		if ( ( pRoomIndex = get_room_index( iLastRoom ) ) )
		{
		    new_reset( pRoomIndex, pReset );
		    iLastObj = iLastRoom;
		}
		break;

	    case 'D':
		pRoomIndex = get_room_index( pReset->arg1 );

		if ( pReset->arg2 < 0
		||   pReset->arg2 > ( MAX_DIR - 1 )
		||   !pRoomIndex
		||   !( pexit = pRoomIndex->exit[pReset->arg2] ) )
		{
		    bug( "Load_resets: 'D': exit %d not door.", pReset->arg2 );
		    exit( 1 );
		}

		if ( !IS_SET( pexit->rs_flags, EX_ISDOOR ) )
		{
		    SET_BIT( pexit->rs_flags, EX_ISDOOR );
		    SET_BIT( area_last->area_flags, AREA_CHANGED );
		    bug( "Load_resets: 'D': exit %d not flagged door.", pReset->arg2 );
		}

		switch( pReset->arg3 )
		{
		    default:
			bug( "Load_resets: 'D': bad 'locks': %d." , pReset->arg3 );
			break;

		    case 0:
			break;

		    case 1:
			SET_BIT( pexit->rs_flags, EX_CLOSED );
			SET_BIT( pexit->exit_info, EX_CLOSED );
			break;

		    case 2:
			SET_BIT( pexit->rs_flags, EX_CLOSED | EX_LOCKED );
			SET_BIT( pexit->exit_info, EX_CLOSED | EX_LOCKED );
			break;
		}
		break;

	    case 'R':
		if ( pReset->arg2 < 0 || pReset->arg2 > MAX_DIR )
		{
		    bug( "Load_resets: 'R': bad exit %d.", pReset->arg2 );
		    exit( 1 );
		}

		if ( ( pRoomIndex = get_room_index( pReset->arg1 ) ) != NULL )
		    new_reset( pRoomIndex, pReset );
		break;
	}
    }

    return;
}

void load_mobprogs( FILE *fp )
{
    PROG_CODE *pMprog;

    if ( area_last == NULL )
    {
	bug( "Load_mobprogs: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	char letter;
	int vnum;

	if ( ( letter = fread_letter( fp ) ) != '#' )
	{
	    bug( "Load_mobprogs: # not found.", 0 );
	    exit( 1 );
	}

	if ( ( vnum = fread_number( fp ) ) == 0 )
	    break;

	fBootDb = FALSE;
	if ( get_prog_index( vnum, PRG_MPROG ) != NULL )
	{
	    bug( "Load_mobprogs: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;

	pMprog		= alloc_perm( sizeof( *pMprog ) );
	pMprog->area	= area_last;
	pMprog->vnum	= vnum;

	#if defined LOAD_ASGARD_AREAS
	    if ( !load_asgard_area )
	    {
		pMprog->author	= fread_string( fp );
		pMprog->name	= fread_string( fp );
	    } else {
		pMprog->author	= str_dup( "Unknown" );
		pMprog->name	= str_dup( "Unknown" );
	    }
	#else
	    pMprog->author	= fread_string( fp );
	    pMprog->name	= fread_string( fp );
	#endif

	pMprog->code  	= fread_string( fp );
	pMprog->next	= mprog_list;
	mprog_list 	= pMprog;
    }

    return;
}

void load_shops( FILE *fp )
{
    SHOP_DATA *pShop;

    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	int iTrade;

	top_shop++;
	pShop			= alloc_perm( sizeof( *pShop ) );
	pShop->keeper		= fread_number( fp );
	if ( pShop->keeper == 0 )
	    break;
	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	    pShop->buy_type[iTrade]	= fread_number( fp );
	pShop->profit_buy	= fread_number( fp );
	pShop->profit_sell	= fread_number( fp );
	pShop->open_hour	= fread_number( fp );
	pShop->close_hour	= fread_number( fp );
				  fread_to_eol( fp );
	pMobIndex		= get_mob_index( pShop->keeper );
	pMobIndex->pShop	= pShop;

	if ( shop_first == NULL )
	    shop_first = pShop;
	if ( shop_last  != NULL )
	    shop_last->next = pShop;

	shop_last	= pShop;
	pShop->next	= NULL;
    }

    return;
}

void load_specials( FILE *fp )
{
    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	char letter;

	switch ( letter = fread_letter( fp ) )
	{
	default:
	    bug( "Load_specials: letter '%c' not *MS.", letter );
	    exit( 1 );

	case 'S':
	    return;

	case '*':
	    break;

	case 'M':
	    pMobIndex		= get_mob_index	( fread_number ( fp ) );
	    pMobIndex->spec_fun	= spec_lookup	( fread_word   ( fp ) );
	    break;
	}

	fread_to_eol( fp );
    }
}

void fix_exits( void )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit;
    int iHash;
    int door;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex  = room_index_hash[iHash];
	      pRoomIndex != NULL;
	      pRoomIndex  = pRoomIndex->next )
	{
	    for ( door = 0; door < MAX_DIR; door++ )
	    {
		if ( ( pexit = pRoomIndex->exit[door] ) != NULL )
		{
		    if ( pexit->u1.vnum <= 0
		    ||   get_room_index(pexit->u1.vnum) == NULL)
		    {
			if ( pRoomIndex->area != NULL )
			    SET_BIT(pRoomIndex->area->area_flags, AREA_CHANGED);
			pexit->u1.to_room = NULL;
			pexit->u1.vnum = 0;
		    }
		    else
		    {
			pexit->u1.to_room = get_room_index( pexit->u1.vnum );
		    }
		}
	    }
	}
    }

    return;
}

void fix_mobprogs( void )
{
    MOB_INDEX_DATA *pMobIndex;
    PROG_LIST        *list;
    PROG_CODE        *prog;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pMobIndex   = mob_index_hash[iHash];
	      pMobIndex   != NULL;
	      pMobIndex   = pMobIndex->next )
	{
	    for( list = pMobIndex->mprogs; list != NULL; list = list->next )
	    {
		if ( ( prog = get_prog_index( list->vnum, PRG_MPROG ) ) != NULL )
		{
		    free_string( list->code );
		    list->code = str_dup( prog->code );
		}
		else
		{
		    bug( "Fix_mobprogs: code vnum %d not found.", list->vnum );
		    bug( "Mobile vnum: %d.", pMobIndex->vnum );
//		    exit( 1 );
		}
	    }
	}
    }
}

void area_update( void )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *room;
    char buf[MAX_STRING_LENGTH];
    int hash;

    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {
	if ( ( IS_SET( pArea->area_flags, AREA_UNLINKED ) && pArea->nplayer == 0 )
	||   ++pArea->age < 3 )
	    continue;

	/*
	 * Check age and reset.
	 * Note: Mud School resets every 3 minutes (not 15).
	 */
	if ( pArea->nplayer == 0 || pArea->age >= 15 )
	{
	    ROOM_INDEX_DATA *pRoomIndex;

	    reset_area( pArea );
	    sprintf(buf,"%s {Vhas just been reset.",pArea->name);
	    wiznet(buf,NULL,NULL,WIZ_RESETS,0,0);

	    pArea->age = number_range( 0, 3 );
	    pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );
	    if ( pRoomIndex != NULL && pArea == pRoomIndex->area )
		pArea->age = 15 - 2;
	}
    }

    /*
     * ROOMprog Triggers!
     */
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
    {
	for ( room = room_index_hash[hash]; room; room = room->next )
	{
	    if ( room->area->nplayer <= 0 )
	    	continue;

	    if ( HAS_TRIGGER_ROOM( room, TRIG_DELAY ) && room->rprog_delay > 0 )
	    {
		if ( --room->rprog_delay <= 0 )
		    p_percent_trigger( NULL, NULL, room, NULL, NULL, NULL, TRIG_DELAY );
	    }

	    else if ( HAS_TRIGGER_ROOM( room, TRIG_RANDOM ) )
		p_percent_trigger( NULL, NULL, room, NULL, NULL, NULL, TRIG_RANDOM );
	}
    }

    return;
}

void load_social_table( void )
{
    FILE *fp;
    int i;

    if ( ( fp = fopen( "../data/info/socials.dat", "r" ) ) == NULL )
    {
	bug( "Could not open SOCIAL_FILE for reading.", 0 );
	exit( 1 );
    }

    fscanf( fp, "%d\n", &maxSocial );

    social_table = malloc( sizeof( struct social_type ) * ( maxSocial+1 ) );

    for ( i = 0; i < maxSocial; i++ )
    {
	strcpy( social_table[i].name, fread_word( fp ) );

	social_table[i].char_no_arg	= fread_string( fp );
	social_table[i].others_no_arg	= fread_string( fp );
	social_table[i].char_found	= fread_string( fp );
	social_table[i].others_found	= fread_string( fp );
	social_table[i].vict_found	= fread_string( fp );
	social_table[i].char_auto	= fread_string( fp );
	social_table[i].others_auto	= fread_string( fp );
    }

    social_table[maxSocial].name[0] = '\0';

    fclose ( fp );
}

void load_channel( FILE *fp, int chan )
{
    char *word;

    free_string( channel_table[chan].name );
    channel_table[chan].name = str_dup( fread_word( fp ) );

    for ( ; ; )
    {
	word = feof( fp ) ? "END" : fread_word( fp );

	if ( !str_cmp( word, "End" ) )
	    return;

	switch( word[0] )
	{
	    case 'A':
		LOAD( "Arena", channel_table[chan].arena, fread_number( fp ) );
		break;

	    case 'C':
		LOAD( "Censr", channel_table[chan].censor, fread_number( fp ) );
		if ( !str_cmp( word, "Color" ) )
		{
		    char temp[25];
		    int pos;

		    strcpy( temp, fread_word( fp ) );
		    pos = find_color( temp );

		    if ( pos == -1 )
			pos = 0;

		    free_string( channel_table[chan].color_default );
		    channel_table[chan].color_default = str_dup( color_table[pos].color_code );
		    break;
		}
		break;

	    case 'D':
		LOAD( "Drunk", channel_table[chan].drunk, fread_number( fp ) );
		break;

	    case 'F':
		LOAD( "Flags", channel_table[chan].bit, fread_flag( fp ) );
		break;

	    case 'L':
		LOAD( "Level", channel_table[chan].level, fread_number( fp ) );
		break;

	    case 'O':
		if ( !str_cmp( word, "Other" ) )
		{
		    free_string( channel_table[chan].other_string );
		    channel_table[chan].other_string = fread_string( fp );
		    break;
		}
		break;

	    case 'P':
		LOAD( "PTitl", channel_table[chan].pretitle, fread_number( fp ) );
		break;

	    case 'Q':
		LOAD( "Quiet", channel_table[chan].quiet, fread_number( fp ) );
		break;

	    case 'S':
		if ( !str_cmp( word, "Self" ) )
		{
		    free_string( channel_table[chan].ch_string );
		    channel_table[chan].ch_string = fread_string( fp );
		    break;
		}
		break;

	    case 'V':
		LOAD( "Vrble", channel_table[chan].color_variable, fread_letter( fp ) );
		break;
	}
    }
}

void load_variable_data( )
{
    FILE *fp;
    char *word;
    int chan = 0;

    mud_stat.good_god_string	= str_dup( "goodness" );
    mud_stat.evil_god_string	= str_dup( "pure evil" );
    mud_stat.neut_god_string	= str_dup( "neutrality" );
    mud_stat.mud_name_string	= str_dup( "The Mud" );
    mud_stat.exp_mod[0]		= 1;
    mud_stat.exp_mod[1]		= 1;
    mud_stat.multilock		= TRUE;
    mud_stat.quest_gold[0]	= 25;
    mud_stat.quest_gold[1]	= 200;
    mud_stat.quest_points[0]	= 1;
    mud_stat.quest_points[1]	= 15;
    mud_stat.quest_pracs[0]	= 1;
    mud_stat.quest_pracs[1]	= 3;
    mud_stat.quest_exp[0]	= 50;
    mud_stat.quest_exp[1]	= 500;
    mud_stat.quest_object[0]	= 1;
    mud_stat.quest_object[1]	= 3;
    mud_stat.quest_obj_vnum[0]	= 60;
    mud_stat.quest_obj_vnum[1]	= 64;
    mud_stat.timeout_mortal	= 10;
    mud_stat.timeout_immortal	= 60;
    mud_stat.timeout_ld_imm	= 10;
    mud_stat.timeout_ld_mort	= 2;

    if ( ( fp = fopen ( VARIABLE_FILE, "r" ) ) == NULL )
    {
	bug( "Cannot access variable data file.", 0 );
	return;
    }

    for ( ; ; )
    {
	word = feof( fp ) ? "END" : fread_word( fp );

	if ( !str_cmp( word, "END" ) )
	    break;

	switch( word[0] )
	{
	    case 'C':
		LOAD( "Cpslck",	mud_stat.capslock,	fread_number( fp ) );
		LOAD( "Clrlck",	mud_stat.colorlock,	fread_number( fp ) );

		if ( !str_cmp( word, "Chann" ) )
		{
		    load_channel( fp, chan );
		    chan++;
		    break;
		}
		break;

	    case 'E':
		if ( !str_cmp( word, "Evil_G" ) )
		{
		    free_string( mud_stat.evil_god_string );
		    mud_stat.evil_god_string = fread_string( fp );
		    break;
		}

		if ( !str_cmp( word, "ExpMod" ) )
		{
		    mud_stat.exp_mod[0] = fread_number( fp );
		    mud_stat.exp_mod[1] = fread_number( fp );
		    break;
		}
		break;

	    case 'G':
		if ( !str_cmp( word, "Good_G" ) )
		{
		    free_string( mud_stat.good_god_string );
		    mud_stat.good_god_string = fread_string( fp );
		    break;
		}
		break;

	    case 'I':
		LOAD( "I-Time", mud_stat.timeout_immortal, fread_number( fp ) );
		LOAD( "Imm-LD", mud_stat.timeout_ld_imm, fread_number( fp ) );
		break;

	    case 'L':
		LOAD( "LogAll",	mud_stat.fLogAll,	fread_number( fp ) );
		break;

	    case 'M':
		LOAD( "Max_On",	mud_stat.max_ever,	fread_number( fp ) );
		LOAD( "MostOn",	mud_stat.most_today,	fread_number( fp ) );
		LOAD( "MltLck",	mud_stat.multilock,	fread_number( fp ) );
		LOAD( "M-Time", mud_stat.timeout_mortal, fread_number( fp ) );
		LOAD( "Mor-LD", mud_stat.timeout_ld_mort,fread_number( fp ) );

		if ( !str_cmp( word, "MudNam" ) )
		{
		    free_string( mud_stat.mud_name_string );
		    mud_stat.mud_name_string = fread_string( fp );
		    break;
		}
		break;

	    case 'N':
		LOAD( "Newlck",	mud_stat.newlock,	fread_number( fp ) );

		if ( !str_cmp( word, "Neut_G" ) )
		{
		    free_string( mud_stat.neut_god_string );
		    mud_stat.neut_god_string = fread_string( fp );
		    break;
		}
		break;

	    case 'P':
		if ( !str_cmp( word, "PKills" ) )
		{
		    PKILL_RECORD *pk_record, *pk_list;

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

		    if ( recent_list == NULL )
			recent_list = pk_record;
		    else
		    {
			for ( pk_list = recent_list; pk_list != NULL; pk_list = pk_list->next )
			{
			    if ( pk_list->next == NULL )
			    {
				pk_list->next = pk_record;
				break;
			    }
			}
		    }
		}
		break;

	    case 'Q':
		if ( !str_cmp( word, "QstObj" ) )
		{
		    mud_stat.quest_object[0] = fread_number( fp );
		    mud_stat.quest_object[1] = fread_number( fp );
		    break;
		}

		if ( !str_cmp( word, "QsVnum" ) )
		{
		    mud_stat.quest_obj_vnum[0] = fread_number( fp );
		    mud_stat.quest_obj_vnum[1] = fread_number( fp );
		    break;
		}

		if ( !str_cmp( word, "QstExp" ) )
		{
		    mud_stat.quest_exp[0] = fread_number( fp );
		    mud_stat.quest_exp[1] = fread_number( fp );
		    break;
		}

		if ( !str_cmp( word, "QsGold" ) )
		{
		    mud_stat.quest_gold[0] = fread_number( fp );
		    mud_stat.quest_gold[1] = fread_number( fp );
		    break;
		}

		if ( !str_cmp( word, "QPoint" ) )
		{
		    mud_stat.quest_points[0] = fread_number( fp );
		    mud_stat.quest_points[1] = fread_number( fp );
		    break;
		}

		if ( !str_cmp( word, "QPracs" ) )
		{
		    mud_stat.quest_pracs[0] = fread_number( fp );
		    mud_stat.quest_pracs[1] = fread_number( fp );
		    break;
		}
		break;

	    case 'R':
		if ( !str_cmp( word, "Random" ) )
		{
		    mud_stat.random_vnum[0] = fread_number( fp );
		    mud_stat.random_vnum[1] = fread_number( fp );
		    break;
		}
		break;

	    case 'U':
		if ( !str_cmp( word, "Unique" ) )
		{
		    mud_stat.unique_vnum[0] = fread_number( fp );
		    mud_stat.unique_vnum[1] = fread_number( fp );
		    break;
		}
		break;

	    case 'W':
		LOAD( "Wizlck",	mud_stat.wizlock,	fread_number( fp ) );
		break;
	}
    }

    fclose ( fp );
}

void load_objprogs( FILE *fp )
{
    PROG_CODE *pOprog;

    if ( area_last == NULL )
    {
	bug( "Load_objprogs: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	sh_int vnum;
	char letter;

	letter		  = fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_objprogs: # not found.", 0 );
	    exit( 1 );
	}

	vnum		 = fread_number( fp );
	if ( vnum == 0 )
	    break;

	fBootDb = FALSE;
	if ( get_prog_index( vnum, PRG_OPROG ) != NULL )
	{
	    bug( "Load_objprogs: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;

	pOprog		= alloc_perm( sizeof(*pOprog) );
	pOprog->area	= area_last;
	pOprog->vnum  	= vnum;
	pOprog->author	= fread_string( fp );
	pOprog->name	= fread_string( fp );
	pOprog->code  	= fread_string( fp );
	pOprog->next	= oprog_list;
	oprog_list 	= pOprog;
    }
    return;
}

void load_roomprogs( FILE *fp )
{
    PROG_CODE *pRprog;

    if ( area_last == NULL )
    {
	bug( "Load_roomprogs: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	sh_int vnum;
	char letter;

	letter		  = fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_roomprogs: # not found.", 0 );
	    exit( 1 );
	}

	vnum		 = fread_number( fp );
	if ( vnum == 0 )
	    break;

	fBootDb = FALSE;
	if ( get_prog_index( vnum, PRG_RPROG ) != NULL )
	{
	    bug( "Load_roomprogs: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;

	pRprog		= alloc_perm( sizeof(*pRprog) );
	pRprog->area	= area_last;
	pRprog->vnum  	= vnum;
	pRprog->author	= fread_string( fp );
	pRprog->name	= fread_string( fp );
	pRprog->code  	= fread_string( fp );
	pRprog->next	= rprog_list;
	rprog_list 	= pRprog;
    }
    return;
}

void fix_objprogs( void )
{
    OBJ_INDEX_DATA *pObjIndex;
    PROG_LIST        *list;
    PROG_CODE        *prog;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pObjIndex   = obj_index_hash[iHash];
	      pObjIndex   != NULL;
	      pObjIndex   = pObjIndex->next )
	{
	    for( list = pObjIndex->oprogs; list != NULL; list = list->next )
	    {
		if ( ( prog = get_prog_index( list->vnum, PRG_OPROG ) ) != NULL )
		    list->code = prog->code;
		else
		{
		    bug( "Fix_objprogs: code vnum %d not found.", list->vnum );
		    exit( 1 );
		}
	    }
	}
    }
}

void fix_roomprogs( void )
{
    ROOM_INDEX_DATA *pRoomIndex;
    PROG_LIST        *list;
    PROG_CODE        *prog;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex   = room_index_hash[iHash];
	      pRoomIndex   != NULL;
	      pRoomIndex   = pRoomIndex->next )
	{
	    for( list = pRoomIndex->rprogs; list != NULL; list = list->next )
	    {
		if ( ( prog = get_prog_index( list->vnum, PRG_RPROG ) ) != NULL )
		    list->code = prog->code;
		else
		{
		    bug( "Fix_roomprogs: code vnum %d not found.", list->vnum );
		    exit( 1 );
		}
	    }
	}
    }
}

void load_races( void )
{
    FILE *fp;
    sh_int race = -1, lvl;

    if ( ( fp = fopen( RACES_FILE, "r" ) ) == NULL )
    {
	bug( "Load_races: No race file found.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	char *word = fread_word( fp );

	if ( !str_cmp( word,"#Race" ) )
	{
	    if ( maxRace == 0 || ++race > maxRace )
	    {
		bug( "Load_races: maxRace exceeded.", 0 );
		fclose( fp );
		exit( 1 );
	    }
	    
	    race_table[race].name	= NULL;
	    race_table[race].pc_race	= FALSE;
	    race_table[race].disabled	= FALSE;
	    race_table[race].size	= SIZE_MEDIUM;
	    race_table[race].aff	= 0;
	    race_table[race].shd	= 0;
	    race_table[race].parts	= 0;
	    race_table[race].points	= 0;
	    race_table[race].attack	= attack_lookup( "punch" );
	    race_table[race].class_mult	= new_short( maxClass, 200 );
	    race_table[race].class_can_use = new_bool( maxClass, TRUE );

	    strcpy( race_table[race].who_name, "     " );

	    for ( lvl = 0; lvl < 5; lvl++ )
		race_table[race].skills[lvl] = NULL;

	    for ( lvl = 0; lvl < DAM_MAX; lvl++ )
		race_table[race].damage_mod[lvl] = 100;

	    for ( lvl = 0; lvl < MAX_STATS; lvl++ )
	    {
		race_table[race].stats[lvl]	= 0;
		race_table[race].max_stats[lvl]	= 0;
	    }

	    for ( ; ; )
	    {
		char *match = fread_word( fp );

		if ( !str_cmp( match, "End" ) )
		    break;

		switch( UPPER( match[0] ) )
		{
		    case 'A':
			LOAD( "Affs", race_table[race].aff,	fread_flag( fp ) );
			if ( !str_cmp( match, "Attk" ) )
			{
			    char *temp = fread_string( fp );
			    race_table[race].attack = attack_lookup( temp );
			    free_string( temp );
			    break;
			}
			break;

		    case 'C':
			if ( !str_cmp( match, "Clas" ) )
			{
			    char *word = fread_word( fp );

			    if ( ( lvl = class_lookup( word ) ) != -1 )
			    {
				race_table[race].class_can_use[lvl] = fread_number( fp );
				race_table[race].class_mult[lvl] = fread_number( fp );
			    } else {
				fread_number( fp );
				fread_number( fp );
				sprintf( log_buf, "Load_races: race: %s bad class name %s.",
				    race_table[race].name, word );
				bug( log_buf, 0 );
			    }

			    break;
			}
			break;

		    case 'D':
			LOAD( "Dsbl", race_table[race].disabled,fread_number( fp ) );
			if ( !str_cmp( match, "DamM" ) )
			{
			    for ( lvl = 0; lvl < DAM_MAX; lvl++ )
				race_table[race].damage_mod[lvl] = fread_number( fp );
			    break;
			}
			break;

		    case 'M':
			if ( !str_cmp( match, "MaxS" ) )
			{
			    for ( lvl = 0; lvl < MAX_STATS; lvl++ )
				race_table[race].max_stats[lvl] = fread_number( fp );
			    break;
			}

			break;

		    case 'N':
			LOAD( "Name", race_table[race].name,	fread_string( fp ) );
			break;

		    case 'P':
			LOAD( "Part", race_table[race].parts,	fread_flag( fp ) );
			LOAD( "PcRc", race_table[race].pc_race,	fread_number( fp ) );
			LOAD( "Pnts", race_table[race].points,	fread_number( fp ) );
			break;

		    case 'S':
			LOAD( "Shds", race_table[race].shd,	fread_flag( fp ) );
			LOAD( "Size", race_table[race].size,	fread_number( fp ) );
			if ( !str_cmp( match, "Stat" ) )
			{
			    for ( lvl = 0; lvl < MAX_STATS; lvl++ )
				race_table[race].stats[lvl] = fread_number( fp );
			    break;
			}
			if ( !str_cmp( match, "Skil" ) )
			{
			    char *tmp;

			    for ( lvl = 0; lvl < 5; lvl++ )
			    {
				if ( race_table[race].skills[lvl] == NULL )
				    break;
			    }

			    if ( lvl >= 5 )
			    {
				bug( "Load_races: skill initialize exceeds 5.", 0 );
				tmp = fread_string( fp );
				free_string( tmp );
				break;
			    }

			    race_table[race].skills[lvl] = fread_string( fp );
			    break;
			}
			break;

		    case 'W':
			if ( !str_cmp( match, "WhoN" ) )
			{
			    strcpy( race_table[race].who_name, fread_string( fp ) );
			    break;
			}
			break;
		}
	    }
	}

	else if ( !str_cmp( word, "#MAX_RACE" ) )
	{
	    maxRace = fread_number( fp );
	    race_table = malloc( sizeof( struct race_type ) * ( maxRace+1 ) );
	}

	else if ( !str_cmp( word, "#End" ) )
	    break;

	else
	{
	    bug( "Load_races: bad section name.", 0 );
	    fclose( fp );
	    exit( 1 );
	}
    }

    race_table[maxRace].name = str_dup( "" );

    fclose( fp );
}

void load_classes( void )
{
    FILE *fp;
    sh_int class = -1, sn;

    if ( ( fp = fopen( CLASSES_FILE, "r" ) ) == NULL )
    {
	bug( "Load_classes: No class file found.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	char *word = fread_word( fp );

	if ( !str_cmp( word, "#Class" ) )
	{
	    if ( maxClass == 0 || ++class > maxClass )
	    {
		bug( "Load_classes: MAX_CLASS exceeded.", 0 );
		fclose( fp );
		exit( 1 );
	    }
	    
	    strcpy( class_table[class].who_name, "Who" );
	    class_table[class].name		= NULL;
	    class_table[class].attr_prime	= STAT_STR;
	    class_table[class].thac0_00		= 0;
	    class_table[class].thac0_32		= 0;
	    class_table[class].hp_min		= 0;
	    class_table[class].hp_max		= 0;
	    class_table[class].mana_percent	= 50;
	    class_table[class].disabled		= TRUE;
	    class_table[class].base_group	= NULL;
	    class_table[class].default_group	= NULL;
	    class_table[class].sub_class	= 0;
	    class_table[class].tier		= 1;

	    for ( ; ; )
	    {
		char *match = fread_word( fp );

		if ( !str_cmp( match, "End" ) )
		    break;

		switch( UPPER( match[0] ) )
		{
		    case 'A':
			LOAD( "Attr", class_table[class].attr_prime,	fread_number( fp ) );
			break;

		    case 'B':
			LOAD( "Base", class_table[class].base_group,	fread_string( fp ) );
			break;

		    case 'D':
			LOAD( "Deft", class_table[class].default_group,	fread_string( fp ) );
			LOAD( "Disa", class_table[class].disabled,	fread_number( fp ) );
			break;

		    case 'G':
			if ( !str_cmp( match, "Grup" ) )
			{
			    char *group = fread_word( fp );
			    sn = group_lookup( group );

			    if ( sn < 0 || sn > maxGroup )
			    {
				sprintf( log_buf, "Load_classes: %s: bad group %s", class_table[class].name, group );
				log_string( log_buf );
				fread_number( fp );
				break;
			    }

			    group_table[sn].rating[class] = fread_number( fp );
			    break;
			}
			break;

		    case 'H':
			if ( !str_cmp( match, "HitP" ) )
			{
			    class_table[class].hp_min	= fread_number( fp );
			    class_table[class].hp_max	= fread_number( fp );
			    break;
			}
			break;

		    case 'M':
			LOAD( "Mana", class_table[class].mana_percent,	fread_number( fp ) );
			if ( !str_cmp( match, "ManP" ) )
			{
			    fread_number( fp );
			    fread_number( fp );
			    break;
			}
			break;

		    case 'N':
			LOAD( "Name", class_table[class].name,		fread_string( fp ) );
			break;

		    case 'S':
			if ( !str_cmp( match, "SubC" ) )
			{
			    sn = class_lookup( fread_word( fp ) );
			    class_table[class].sub_class = UMAX( 0, sn );
			    break;
			}
			    
			if ( !str_cmp( match, "Skil" ) )
			{
			    char *skill = fread_word( fp );
			    sh_int sn = skill_lookup( skill );

			    if ( sn < 0 || sn > maxSkill )
			    {
				sprintf( log_buf, "Load_classes: %s: bad skill %s", class_table[class].name, skill );
				log_string( log_buf );
				fread_to_eol( fp );
				break;
			    }

			    skill_table[sn].skill_level[class] = fread_number( fp );
			    skill_table[sn].rating[class] = fread_number( fp );
			    break;
			}

			break;

		    case 'T':
			if ( !str_cmp( match, "Thac" ) )
			{
			    class_table[class].thac0_00	= fread_number( fp );
			    class_table[class].thac0_32	= fread_number( fp );
			    break;
			}
			LOAD( "Tier", class_table[class].tier,		fread_number( fp ) );
			break;

		    case 'W':
			if ( !str_cmp( match, "WhoN" ) )
			{
			    strcpy(class_table[class].who_name, fread_word( fp ) );
			    break;
			}
			break;
		}
	    }
	}

	else if ( !str_cmp( word, "#MAX_CLASS" ) )
	{
	    maxClass = fread_number( fp );
	    class_table = malloc( sizeof( struct class_type ) * ( maxClass+1 ) );

	    for ( sn = 0; sn < maxSkill; sn++ )
	    {
		skill_table[sn].skill_level = new_short( maxClass, LEVEL_IMMORTAL );
		skill_table[sn].rating = new_short( maxClass, 0 );
	    }

	    for ( sn = 0; sn < maxGroup; sn++ )
		group_table[sn].rating = new_short( maxClass, -1 );

	}

	else if ( !str_cmp( word, "#End" ) )
	    break;

	else
	{
	    bug( "Load_classes: bad section name.", 0 );
	    fclose( fp );
	    exit( 1 );
	}
    }

    fclose( fp );
    class_table[maxClass].name = str_dup( "" );
    return;
}

void boot_db( )
{
    FILE *fpList;
    int sn;
    long lhour, lday, lmonth;

    log_string( "Init data space." );

    if ( ( string_space = calloc( 1, MAX_STRING ) ) == NULL )
    {
	bug( "Boot_db: can't alloc %d string space.", MAX_STRING );
	exit( 1 );
    }
    top_string	= string_space;
    fBootDb	= TRUE;

    init_mm( );

    log_string( "Setting time and weather." );

    lhour		= (current_time - 650336715)
			/ (PULSE_TICK / PULSE_PER_SECOND);
    time_info.hour	= lhour  % 24;
    lday		= lhour  / 24;
    time_info.day	= lday   % 35;
    lmonth		= lday   / 35;
    time_info.month	= lmonth % 17;
    time_info.year	= lmonth / 17;

    if ( time_info.hour <  5 ) weather_info.sunlight = SUN_DARK;
    else if ( time_info.hour <  6 ) weather_info.sunlight = SUN_RISE;
    else if ( time_info.hour < 19 ) weather_info.sunlight = SUN_LIGHT;
    else if ( time_info.hour < 20 ) weather_info.sunlight = SUN_SET;
    else                            weather_info.sunlight = SUN_DARK;
    weather_info.change	= 0;
    weather_info.mmhg	= 960;
    if ( time_info.month >= 7 && time_info.month <=12 )
	weather_info.mmhg += number_range( 1, 50 );
    else
	weather_info.mmhg += number_range( 1, 80 );
    if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
    else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
    else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
    else                                  weather_info.sky = SKY_CLOUDLESS;

    log_string( "Loading Skills." );
    load_skills();
    for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
    {
	if ( skill_table[sn].pgsn != NULL )
	    *skill_table[sn].pgsn = sn;
    }

    log_string( "Loading Classes." );
    load_classes( );

    log_string( "Loading Races." );
    load_races( );

    log_string( "Loading Clans." );
	load_clans( );

    log_string( "Loading Notes." );
	load_notes( );
    log_string( "Loading Bans." );
	load_bans();
    log_string( "Loading Wizlist." );
	load_wizlist();
    log_string( "Loading Socials." );
	load_social_table();
    log_string( "Loading Disabled." );
	load_disabled();
    log_string( "Loading Game Variables." );
 	load_variable_data( );
	load_voting_polls( );
    log_string( "Loading Charts." );
	load_charts( FALSE );
    log_string( "Loading Helps." );
	load_helps( );

    log_string( "Reading Area List." );
    if ( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL )
    {
	perror( AREA_LIST );
	exit( 1 );
    }

    for ( ; ; )
    {
	strcpy( strArea, fread_word( fpList ) );
	if ( strArea[0] == '$' )
	    break;

	sprintf( log_buf, "reading %s", strArea );
	log_string( log_buf );

	if ( ( fpArea = fopen( strArea, "r" ) ) == NULL )
	{
	    perror( strArea );
	    continue;
	}

	for ( ; ; )
	{
	    char *word;

	    if ( fread_letter( fpArea ) != '#' )
	    {
		bug( "Boot_db: # not found.", 0 );
		exit( 1 );
	    }

	    word = fread_word( fpArea );

	    if ( word[0] == '$'               )                 break;
	    else if ( !str_cmp( word, "AOD_AREA" ) ) load_area	    (fpArea);
//	    else if ( !str_cmp( word, "AREA"	 ) ) load_area_ds   (fpArea);
//	    else if ( !str_cmp( word, "AREADATA" ) ) load_area_ashen(fpArea);

#if defined LOAD_ASGARD_AREAS
	    else if ( !str_cmp( word, "AREADATA" ) ) load_area_asgard(fpArea);
#endif

	    else if ( !str_cmp( word, "MOBILES"  ) ) load_mobiles   (fpArea);
	    else if ( !str_cmp( word, "MOBPROGS" ) ) load_mobprogs  (fpArea);
	    else if ( !str_cmp( word, "OBJPROGS" ) ) load_objprogs  (fpArea);
	    else if ( !str_cmp( word, "ROOMPROGS") ) load_roomprogs (fpArea);
	    else if ( !str_cmp( word, "OBJECTS"  ) ) load_objects   (fpArea);
	    else if ( !str_cmp( word, "RESETS"   ) ) load_resets    (fpArea);
	    else if ( !str_cmp( word, "ROOMS"    ) ) load_rooms     (fpArea);
	    else if ( !str_cmp( word, "SHOPS"    ) ) load_shops     (fpArea);
	    else if ( !str_cmp( word, "SPECIALS" ) ) load_specials  (fpArea);
	    else
	    {
		bug( "Boot_db: bad section name.", 0 );
		exit( 1 );
	    }
	}

	fclose( fpArea );
	fpArea = NULL;
    }
    fclose( fpList );

    log_string( "Fixing Exits." );
	fix_exits( );
     	fix_mobprogs( );
	fix_objprogs( );
	fix_roomprogs( );
	fBootDb	= FALSE;

    log_string( "Loading Special Items." );
	load_special_items();

    log_string( "Loading Random Item Tables." );
	load_random_data( );

    log_string( "Area Update." );
	area_update( );

    return;
}

void reset_room( ROOM_INDEX_DATA *pRoom, ARENA_DATA *match )
{
    RESET_DATA	*pReset;
    CHAR_DATA	*pMob = NULL;
    CHAR_DATA	*mob;
    CHAR_DATA	*LastMob = NULL;
    OBJ_DATA	*LastObj = NULL;
    OBJ_DATA	*pObj;
    bool	last = FALSE;
    int		iExit, level = 0;

    if ( !pRoom )
	return;

    for ( iExit = 0;  iExit < MAX_DIR;  iExit++ )
    {
	EXIT_DATA *pExit;

	if ( ( pExit = pRoom->exit[iExit] ) )
	{
	    pExit->exit_info = pExit->rs_flags;

	    if ( pExit->u1.to_room != NULL
	    &&   (pExit = pExit->u1.to_room->exit[rev_dir[iExit]]) )
	    {
		/* nail the other side */
		pExit->exit_info = pExit->rs_flags;
	    }
	}
    }

    for ( pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next )
    {
	ROOM_INDEX_DATA *pRoomIndex;
	MOB_INDEX_DATA *pMobIndex;
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_INDEX_DATA *pObjToIndex;
	char buf[MAX_STRING_LENGTH];
	int count, limit = 0;

	switch ( pReset->command )
	{
	    default:
		sprintf( buf, "Reset_room [%d]: bad command %c.",
		    pRoom->vnum, pReset->command );
		bug( buf, 0 );
		break;

	    case 'M':
		if ( (pMobIndex = get_mob_index(pReset->arg1)) == NULL )
		{
		    sprintf( buf, "Reset_room [%d]: 'M': bad vnum %d.",
			pRoom->vnum, pReset->arg1 );
		    bug( buf, 0 );
		    continue;
		}

		if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) == NULL )
		{
		    sprintf( buf, "Reset_room [%d]: 'R': bad vnum %d.",
			pRoom->vnum, pReset->arg3 );
		    bug( buf, 0 );
		    continue;
		}

		if ( match == NULL )
		{
		    if ( pMobIndex->count >= pReset->arg2
		    ||   ( pMobIndex->max_world
		    &&     pMobIndex->count >= pMobIndex->max_world ) )
		    {
			last = FALSE;
			break;
		    }

		    count = 0;
		    for ( mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room )
		    {
			if ( mob->pIndexData == pMobIndex )
			{
			    if ( ++count >= pReset->arg4 )
			    {
				last = FALSE;
				break;
			    }
			}
		    }

		    if ( count >= pReset->arg4 )
			break;
		}

		if ( number_percent( ) > pReset->percent )
		    break;

		pMob = create_mobile( pMobIndex );
		pMob->zone = pRoomIndex->area;

		if ( match != NULL )
		    pMob->arena_number = match->number;

		/* Pet shop mobiles get ACT_PET set. */
		{
		    ROOM_INDEX_DATA *pRoomIndexPrev;

		    pRoomIndexPrev = get_room_index( pRoom->vnum - 1 );

		    if ( pRoomIndexPrev
		    &&   IS_SET(pRoomIndexPrev->room_flags, ROOM_PET_SHOP) )
			SET_BIT( pMob->act, ACT_PET);
		}

		if ( pMob->level > 20
		&&   !IS_SET(pMob->act, ACT_PET)
		&&   !IS_SET(pMob->act, ACT_TRAIN)
		&&   !IS_SET(pMob->act, ACT_PRACTICE)
		&&   !IS_SET(pMob->act, ACT_NOQUEST)
		&&   !IS_SET(pMob->act, ACT_IS_PRIEST)
		&&   !IS_SET(pMob->act, ACT_IS_HEALER)
		&&   !IS_SET(pMob->act, ACT_IS_SATAN)
		&&   !IS_SET(pMob->affected_by, AFF_CHARM)
		&&   !IS_SET(pRoom->room_flags, ROOM_PET_SHOP)
		&&   !IS_SET(pRoom->room_flags, ROOM_SAFE)
		&&   !IS_SET(pRoom->area->area_flags,AREA_UNLINKED)
		&&   !IS_SET(pRoom->area->area_flags,AREA_SPECIAL)
		&&   pMob->pIndexData->pShop == NULL
		&&   number_range( 1, 125 ) == 10 )
		{
		    if ( pMob->level > 120 && number_percent( ) < 60 )
			obj_to_char(create_object(get_obj_index(FORGE_STONE_ADAMANTIUM)),pMob);

		    else if ( pMob->level > 85 && number_percent( ) < 50 )
			obj_to_char(create_object(get_obj_index(FORGE_STONE_MITHRIL)),pMob);

		    else if ( pMob->level > 70 && number_percent( ) < 50 )
			obj_to_char(create_object(get_obj_index(FORGE_STONE_TITANIUM)),pMob);

		    else if ( pMob->level > 45 && number_percent( ) < 50 )
			obj_to_char(create_object(get_obj_index(FORGE_STONE_STEEL)),pMob);

		    else if ( number_percent( ) < 60 )
			obj_to_char(create_object(get_obj_index(FORGE_STONE_BRONZE)),pMob);
		}

		char_to_room( pMob, pRoom );

		LastMob = pMob;
		level = URANGE( 0, pMob->level - 2, LEVEL_HERO - 1 );
		last = TRUE;
		break;

	    case 'O':
		if ( (pObjIndex = get_obj_index(pReset->arg1)) == NULL )
		{
		    sprintf( buf, "Reset_room [%d]: 'O' 1 : bad vnum %d",
			pRoom->vnum, pReset->arg1 );
		    bug( buf, 0 );
		    sprintf (buf,"%d %d %d %d",
			pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4);
		    bug(buf,1);
		    continue;
		}

		if ( (pRoomIndex = get_room_index(pReset->arg3)) == NULL )
		{
		    sprintf( buf, "Reset_room [%d]: 'O' 2 : bad vnum %d.",
			pRoom->vnum, pReset->arg3 );
		    bug( buf, 0 );
		    sprintf (buf,"%d %d %d %d",
			pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4);
		    bug(buf,1);
		    continue;
		}

		if ( match == NULL
		&&   count_obj_list( pObjIndex, pRoom->contents ) > 0 )
		{
		    last = FALSE;
		    break;
		}

		if ( number_percent( ) > pReset->percent )
		{
		    last = FALSE;
		    break;
		}

		pObj = create_object( pObjIndex );
		pObj->cost = 0;

		if ( match != NULL )
		    pObj->arena_number = match->number;

		obj_to_room( pObj, pRoom );
		last = TRUE;
		break;

	    case 'P':
		if ( (pObjIndex = get_obj_index(pReset->arg1)) == NULL )
		{
		    sprintf( buf, "Reset_room [%d]: 'P': bad vnum %d.",
			pRoom->vnum, pReset->arg1 );
		    bug( buf, 0 );
		    continue;
		}

		if ( (pObjToIndex = get_obj_index(pReset->arg3)) == NULL )
		{
		    sprintf( buf, "Reset_room [%d]: 'P': bad vnum %d.",
			pRoom->vnum, pReset->arg3 );
		    bug( buf, 0 );
		    continue;
		}

		if (pReset->arg2 == -1) /* no limit */
		    limit = 999;
		else
		    limit = pReset->arg2;

		if ( ( LastObj = get_obj_type( pObjToIndex ) ) == NULL
		|| ( LastObj->in_room == NULL && !last)
		|| ( number_percent( ) > pReset->percent )
		|| ( pObjIndex->count >= limit )
		|| ( count = count_obj_list( pObjIndex, LastObj->contains ) ) > pReset->arg4  )
		{
		    last = FALSE;
		    break;
		}

		while (count < pReset->arg4)
		{
		    pObj = create_object( pObjIndex );

		    if ( match != NULL )
			pObj->arena_number = match->number;

		    obj_to_obj( pObj, LastObj );
		    count++;
		    if (pObjIndex->count >= limit)
			break;
		}

		/* fix object lock state! */
		LastObj->value[1] = LastObj->pIndexData->value[1];
		last = TRUE;
		break;

	    case 'G':
	    case 'E':
		if ( (pObjIndex = get_obj_index(pReset->arg1)) == NULL )
		{
		    sprintf( buf, "Reset_room [%d]: 'E' or 'G': bad vnum %d.",
			pRoom->vnum, pReset->arg1 );
		    bug( buf, 0 );
		    continue;
		}

		if ( !last || number_percent( ) > pReset->percent )
		    break;

		if ( !LastMob )
		{
		    sprintf( buf, "Reset_room: [%d] 'E' or 'G': null mob for vnum %d.",
			pRoom->vnum, pReset->arg1 );
		    bug( buf, 0 );
		    last = FALSE;
		    break;
		}

		if ( get_eq_char( LastMob, pReset->arg3 ) != NULL
		&&   pReset->arg3 != WEAR_NONE )
		    break;

		if ( LastMob->pIndexData->pShop )
		{
		    pObj = create_object( pObjIndex );
		    SET_BIT( pObj->extra_flags, ITEM_INVENTORY );
		}

		else   /* ROM OLC else version */
		{
		    int limit;

		    if ( pReset->arg2 == -1 || pReset->arg2 == 0 )
			limit = 999;
		    else
			limit = pReset->arg2;

		    if ( pObjIndex->count < limit || number_range(0,4) == 0 )
		    {
			pObj = create_object( pObjIndex );
		    }
		    else
			break;
		}

		if ( match != NULL )
		    pObj->arena_number = match->number;

		obj_to_char( pObj, LastMob );

		if ( pReset->command == 'E' )
		{
		    equip_char( LastMob, pObj, pReset->arg3 );
		    LastMob->hit = LastMob->max_hit;
		    LastMob->mana = LastMob->max_mana;
		}

		last = TRUE;
		break;

	    case 'D':
		break;

	    case 'R':
		if ( (pRoomIndex = get_room_index(pReset->arg1)) == NULL )
		{
		    sprintf( buf, "Reset_room [%d]: 'R': bad vnum %d.",
			pRoom->vnum, pReset->arg1 );
		    bug( buf, 0 );
		    continue;
		}

		{
		    EXIT_DATA *pExit;
		    int d0, d1;

		    for ( d0 = 0; d0 < pReset->arg2 - 1; d0++ )
		    {
			d1                   = number_range( d0, pReset->arg2-1 );
			pExit                = pRoomIndex->exit[d0];
			pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
			pRoomIndex->exit[d1] = pExit;
		    }
		}
		break;
	    }
    }
}

void reset_area( AREA_DATA *pArea )
{
    ROOM_INDEX_DATA *pRoom;
    int  vnum;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
        if ( ( pRoom = get_room_index(vnum) ) )
            reset_room(pRoom,NULL);
    }
    return;
}

CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex )
{
    CHAR_DATA *mob;
    int i;

    if ( pMobIndex == NULL )
    {
	bug( "Create_mobile: NULL pMobIndex.", 0 );
	return NULL;
    }

    mob			= new_char();
    mob->pIndexData	= pMobIndex;
    mob->name		= str_dup( pMobIndex->player_name );
    mob->short_descr	= str_dup( pMobIndex->short_descr );
    mob->long_descr	= str_dup( pMobIndex->long_descr );
    mob->description	= str_dup( pMobIndex->description );
    mob->id		= get_mob_id();
    mob->spec_fun	= pMobIndex->spec_fun;
    mob->prompt		= NULL;
    mob->mprog_target	= NULL;
    mob->group		= pMobIndex->group;
    mob->act		= pMobIndex->act;
    mob->alignment	= pMobIndex->alignment;
    mob->level		= number_range(  9 * pMobIndex->level / 10,
					11 * pMobIndex->level / 10 );
    mob->magic_power	= number_range(  9 * pMobIndex->level / 10,
					11 * pMobIndex->level / 10 );
    mob->clan		= pMobIndex->area->clan;
    mob->race		= pMobIndex->race;
    mob->parts		= pMobIndex->parts;
    mob->size		= pMobIndex->size;
    mob->max_hit	= number_range( pMobIndex->hit[0], pMobIndex->hit[1] );
    mob->max_mana	= number_range( pMobIndex->mana[0], pMobIndex->mana[1] );
    mob->damage[DICE_NUMBER] = pMobIndex->damage[DICE_NUMBER];
    mob->damage[DICE_TYPE] = pMobIndex->damage[DICE_TYPE];
    mob->hitroll	= pMobIndex->hitroll;
    mob->damroll	= pMobIndex->damage[DICE_BONUS];
    mob->hit		= mob->max_hit;
    mob->mana		= mob->max_mana;
    mob->start_pos	= pMobIndex->start_pos;
    mob->default_pos	= pMobIndex->default_pos;
    mob->position	= mob->start_pos;
    mob->class		= pMobIndex->class;
    mob->saving_throw	= pMobIndex->saves;

    if ( pMobIndex->wealth == 0 )
    {
	mob->platinum = 0;
	mob->gold = 0;
	mob->silver = 0;
    } else {
	long wealth;

	wealth = number_range(pMobIndex->wealth/2, 3 * pMobIndex->wealth/2);
	mob->gold = number_range(wealth/200,wealth/100);
	mob->silver = wealth - (mob->gold * 100);
	mob->platinum = 0;

	while ( mob->silver >= 100 )
	{
	    mob->silver -= 100;
	    mob->gold++;
	}

	while ( mob->gold >= 100 )
	{
	    mob->gold -= 100;
	    mob->platinum++;
	}
    }

    for ( i = 0; i < 3; i++ )
	mob->regen[i] = pMobIndex->regen[i];

    for ( i = 0; i < 4; i++ )
	mob->armor[i] = pMobIndex->ac[i];

    if ( pMobIndex->sex == SEX_RANDOM )
	mob->sex = number_range( 1, 2 );
    else
	mob->sex = pMobIndex->sex;

    if ( pMobIndex->dam_type == 0 )
	mob->dam_type = number_range( 1, MAX_DAMAGE_MESSAGE );
    else
	mob->dam_type = pMobIndex->dam_type;

    for ( i = 0; i < DAM_MAX; i++ )
	mob->damage_mod[i] = pMobIndex->damage_mod[i];

    for ( i = 0; i < maxSkill; i++ )
	mob->learned[i] = pMobIndex->learned[i];

    for ( i = 0; i < MAX_STATS; i ++ )
	mob->perm_stat[i] = UMIN(35,11 + mob->level/4);

    mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
    mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;

    mobile_spells( mob, TRUE );

    mob->next		= char_list;
    char_list		= mob;
    pMobIndex->count++;
    return mob;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mobile(CHAR_DATA *parent, CHAR_DATA *clone)
{
    int i;
    AFFECT_DATA *paf;

    if ( parent == NULL || clone == NULL || !IS_NPC(parent))
	return;

    /* start fixing values */
    clone->name 	= str_dup(parent->name);
    clone->short_descr	= str_dup(parent->short_descr);
    clone->long_descr	= str_dup(parent->long_descr);
    clone->description	= str_dup(parent->description);
    clone->group	= parent->group;
    clone->sex		= parent->sex;
    clone->class	= parent->class;
    clone->race		= parent->race;
    clone->level	= parent->level;
    clone->timer	= parent->timer;
    clone->wait		= parent->wait;
    clone->hit		= parent->hit;
    clone->max_hit	= parent->max_hit;
    clone->mana		= parent->mana;
    clone->max_mana	= parent->max_mana;
    clone->move		= parent->move;
    clone->max_move	= parent->max_move;
    clone->gold		= parent->gold;
    clone->silver	= parent->silver;
    clone->exp		= parent->exp;
    clone->act		= parent->act;
    clone->comm		= parent->comm;
    clone->invis_level	= parent->invis_level;
    clone->affected_by	= parent->affected_by;
    clone->shielded_by	= parent->shielded_by;
    clone->position	= parent->position;
    clone->saving_throw	= parent->saving_throw;
    clone->alignment	= parent->alignment;
    clone->hitroll	= parent->hitroll;
    clone->damroll	= parent->damroll;
    clone->parts	= parent->parts;
    clone->size		= parent->size;
    clone->dam_type	= parent->dam_type;
    clone->start_pos	= parent->start_pos;
    clone->default_pos	= parent->default_pos;
    clone->spec_fun	= parent->spec_fun;

    for (i = 0; i < 4; i++)
    	clone->armor[i]	= parent->armor[i];

    for (i = 0; i < MAX_STATS; i++)
    {
	clone->perm_stat[i]	= parent->perm_stat[i];
	clone->mod_stat[i]	= parent->mod_stat[i];
    }

    for (i = 0; i < 3; i++)
	clone->damage[i]	= parent->damage[i];

    /* now add the affects */
    for (paf = parent->affected; paf != NULL; paf = paf->next)
        affect_to_char(clone,paf);
}

int ranged( int low, int high )
{
    if ( low < high )
	return number_range( low, high );

    else
	return number_range( high, low );
}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    if ( pObjIndex == NULL )
    {
	bug( "Create_object: NULL pObjIndex.", 0 );
	return NULL;
    }

    obj			= new_obj( );
    obj->pIndexData	= pObjIndex;
    obj->in_room	= NULL;
    obj->enchanted	= FALSE;
    obj->level		= pObjIndex->level;
    obj->wear_loc	= -1;
    obj->name		= str_dup( pObjIndex->name );           /* OLC */
    obj->short_descr	= str_dup( pObjIndex->short_descr );    /* OLC */
    obj->description	= str_dup( pObjIndex->description );    /* OLC */
    obj->size		= pObjIndex->size;
    obj->item_type	= pObjIndex->item_type;
    obj->extra_flags	= pObjIndex->extra_flags;
    obj->wear_flags	= pObjIndex->wear_flags;
    obj->value[0]	= pObjIndex->value[0];
    obj->value[1]	= pObjIndex->value[1];
    obj->value[2]	= pObjIndex->value[2];
    obj->value[3]	= pObjIndex->value[3];
    obj->value[4]	= pObjIndex->value[4];
    obj->weight		= pObjIndex->weight;
    obj->cost		= pObjIndex->cost;
    obj->success	= pObjIndex->success;

    switch ( obj->item_type )
    {
	default:
	    break;

	case ITEM_LIGHT:
	    if ( obj->value[2] == 999 )
		obj->value[2] = -1;
	    break;

	case ITEM_WEAPON:
	    if ( obj->value[3] == 0 )
		obj->value[3] = number_range( 1, MAX_DAMAGE_MESSAGE );
	    break;
    }

    if ( IS_SET( pObjIndex->extra_flags, ITEM_RANDOM_STATS ) )
    {
	AFFECT_DATA *paf, af_new;

	obj->enchanted = TRUE;

	obj->cost = ranged( obj->cost * 80 / 100, obj->cost * 120 / 100 );

	if ( pObjIndex->level <= LEVEL_HERO )
	{
	    obj->level = number_range( obj->level - 5, obj->level + 5 );
	    obj->level = URANGE( 1, obj->level, LEVEL_HERO );
	}

	switch( pObjIndex->item_type )
	{
	    default:
		break;

	    case ITEM_WEAPON:
		obj->value[1] += number_range( -1, 1 );
		obj->value[2] += number_range( -1, 1 );
		break;

	    case ITEM_ARMOR:
		obj->value[0] = ranged(	obj->value[0] * 95 / 100,
					obj->value[0] * 105 / 100 );
		obj->value[1] = ranged( obj->value[1] * 95 / 100,
					obj->value[1] * 105 / 100 );
		obj->value[2] = ranged(	obj->value[2] * 95 / 100,
					obj->value[2] * 105 / 100 );
		obj->value[3] = ranged(	obj->value[3] * 95 / 100,
					obj->value[3] * 105 / 100 );
		break;

	    case ITEM_SCROLL:
	    case ITEM_POTION:
	    case ITEM_PILL:
		obj->value[0] = ranged(	obj->value[0] * 95 / 100,
					obj->value[0] * 105 / 100 );
		break;

	    case ITEM_WAND:
	    case ITEM_STAFF:
		obj->value[2] = ranged(	obj->value[2] * 90 / 100,
					obj->value[2] * 110 / 100 );
		obj->value[0] = ranged(	obj->value[0] * 95 / 100,
					obj->value[0] * 105 / 100 );
		break;
	}

	for ( paf = pObjIndex->affected; paf != NULL; paf = paf->next )
	{
	    if ( paf->where == TO_OBJECT )
	    {
		switch( paf->location )
		{
		    default:
			af_new.where	= paf->where;
			af_new.type 	= UMAX( 0, paf->type );
			af_new.level	= paf->level;
			af_new.dur_type	= paf->dur_type;
			af_new.duration	= paf->duration;
			af_new.location	= paf->location;
			af_new.modifier	= paf->modifier;
			af_new.bitvector= paf->bitvector;
			affect_to_obj( obj, &af_new );
			break;

		    case APPLY_HIT:
		    case APPLY_MANA:
		    case APPLY_MOVE:
			af_new.where	= paf->where;
			af_new.type 	= UMAX( 0, paf->type );
			af_new.level	= paf->level;
			af_new.dur_type	= paf->dur_type;
			af_new.duration	= paf->duration;
			af_new.location	= paf->location;
			af_new.modifier = ranged( paf->modifier * 95 / 100,
						  paf->modifier * 105 / 100 );
			af_new.bitvector= paf->bitvector;
			affect_to_obj( obj, &af_new );
			break;

		    case APPLY_MAGIC_POWER:
		    case APPLY_SAVES:
		    case APPLY_STR:
		    case APPLY_WIS:
		    case APPLY_INT:
		    case APPLY_DEX:
		    case APPLY_CON:
			af_new.where	= paf->where;
			af_new.type 	= UMAX( 0, paf->type );
			af_new.level	= paf->level;
			af_new.dur_type	= paf->dur_type;
			af_new.duration	= paf->duration;
			af_new.location	= paf->location;
			af_new.modifier = paf->modifier + number_range( -1, 1 );
			af_new.bitvector= paf->bitvector;
			affect_to_obj( obj, &af_new );
			break;

		    case APPLY_AC:
			af_new.where	= paf->where;
			af_new.type 	= UMAX( 0, paf->type );
			af_new.level	= paf->level;
			af_new.dur_type	= paf->dur_type;
			af_new.duration	= paf->duration;
			af_new.location	= paf->location;
			af_new.modifier = ranged( paf->modifier * 90 / 100,
						  paf->modifier * 110 / 100 );
			af_new.bitvector= paf->bitvector;
			affect_to_obj( obj, &af_new );
			break;

		    case APPLY_HITROLL:
		    case APPLY_DAMROLL:
			af_new.where	= paf->where;
			af_new.type 	= UMAX( 0, paf->type );
			af_new.level	= paf->level;
			af_new.dur_type	= paf->dur_type;
			af_new.duration	= paf->duration;
			af_new.location	= paf->location;
			af_new.modifier	= ranged( paf->modifier * 90 / 100,
						  paf->modifier * 110 / 100 );
			af_new.bitvector= paf->bitvector;
			affect_to_obj( obj, &af_new );
			break;
		}
	    }

	    else if ( paf->where == TO_DAM_MODS )
	    {
		af_new.where	= paf->where;
		af_new.type 	= UMAX( 0, paf->type );
		af_new.level	= paf->level;
		af_new.dur_type	= paf->dur_type;
		af_new.duration	= paf->duration;
		af_new.location	= paf->location;
		af_new.modifier = paf->modifier + number_range( -5, 5 );
		af_new.bitvector= paf->bitvector;
		affect_to_obj( obj, &af_new );
	    }

	    else
	    {
		af_new.where	= paf->where;
		af_new.type 	= UMAX( 0, paf->type );
		af_new.level	= paf->level;
		af_new.dur_type	= paf->dur_type;
		af_new.duration	= paf->duration;
		af_new.location	= paf->location;
		af_new.modifier	= paf->modifier;
		af_new.bitvector= paf->bitvector;
		affect_to_obj( obj, &af_new );
	    }
	}
    }

    obj->next		= object_list;
    object_list		= obj;
    pObjIndex->count++;

    return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(OBJ_DATA *parent, OBJ_DATA *clone)
{
    int i;
    AFFECT_DATA *paf;
    EXTRA_DESCR_DATA *ed,*ed_new;

    if (parent == NULL || clone == NULL)
	return;

    /* start fixing the object */
    clone->name 	= str_dup(parent->name);
    clone->short_descr 	= str_dup(parent->short_descr);
    clone->description	= str_dup(parent->description);
    clone->item_type	= parent->item_type;
    clone->extra_flags	= parent->extra_flags;
    clone->wear_flags	= parent->wear_flags;
    clone->weight	= parent->weight;
    clone->cost		= parent->cost;
    clone->level	= parent->level;
    clone->timer	= parent->timer;

    for (i = 0;  i < 5; i ++)
	clone->value[i]	= parent->value[i];

    /* affects */
    clone->enchanted	= parent->enchanted;

    for (paf = parent->affected; paf != NULL; paf = paf->next)
	affect_to_obj(clone,paf);

    /* extended desc */
    for (ed = parent->extra_descr; ed != NULL; ed = ed->next)
    {
        ed_new                  = new_extra_descr();
        ed_new->keyword    	= str_dup( ed->keyword);
        ed_new->description     = str_dup( ed->description );
        ed_new->next           	= clone->extra_descr;
        clone->extra_descr  	= ed_new;
    }

}

char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed != NULL; ed = ed->next )
    {
	if ( is_name( (char *) name, ed->keyword ) )
	    return ed->description;
    }
    return NULL;
}

PROG_CODE *get_prog_index( int vnum, int type )
{
    PROG_CODE *prg;

    switch ( type )
    {
	case PRG_MPROG:
	    prg = mprog_list;
	    break;

	case PRG_OPROG:
	    prg = oprog_list;
	    break;

	case PRG_RPROG:
	    prg = rprog_list;
	    break;

	default:
	    return NULL;
    }

    for( ; prg; prg = prg->next )
    {
	if ( prg->vnum == vnum )
            return( prg );
    }

    return NULL;
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
    MOB_INDEX_DATA *pMobIndex;

    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
	  pMobIndex != NULL;
	  pMobIndex  = pMobIndex->next )
    {
	if ( pMobIndex->vnum == vnum )
	    return pMobIndex;
    }

    if ( fBootDb )
    {
	bug( "Get_mob_index: bad vnum %d.", vnum );
	exit( 1 );
    }

    return NULL;
}



/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;

    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
	  pObjIndex != NULL;
	  pObjIndex  = pObjIndex->next )
    {
	if ( pObjIndex->vnum == vnum )
	    return pObjIndex;
    }

    if ( fBootDb )
    {
	bug( "Get_obj_index: bad vnum %d.", vnum );
	exit( 1 );
    }

    return NULL;
}

ROOM_INDEX_DATA *get_room_index( int vnum )
{
    ROOM_INDEX_DATA *pRoomIndex;

    for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
	  pRoomIndex != NULL;
	  pRoomIndex  = pRoomIndex->next )
    {
	if ( pRoomIndex->vnum == vnum )
	    return pRoomIndex;
    }

    if ( fBootDb )
	bug( "Get_room_index: bad vnum %d.", vnum );

    return NULL;
}



/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;

    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    return c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit(c) )
    {
	bug( "Fread_number: bad format.", 0 );
	exit( 1 );
    }

    while ( isdigit(c) )
    {
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}

long fread_flag( FILE *fp )
{
    long number;
    char c;
    bool negative = FALSE;

    do
    {
	c = getc( fp );
    }
    while ( isspace( c ) );

    if ( c == '-' )
    {
	negative = TRUE;
	c = getc( fp );
    }

    number = 0;

    if ( !isdigit( c ) )
    {
	while ( ( 'A' <= c && c <= 'Z') || ( 'a' <= c && c <= 'z' ) )
	{
	    number += flag_convert( c );
	    c = getc( fp );
	}
    }

    while ( isdigit( c ) )
    {
	number = number * 10 + c - '0';
	c = getc( fp );
    }

    if ( c == '|' )
	number += fread_flag( fp );

    else if  ( c != ' ' )
	ungetc( c, fp );

    if ( negative )
	return -1 * number;

    return number;
}

long flag_convert( char letter )
{
    long bitsum = 0;
    char i;

    if ( 'A' <= letter && letter <= 'Z' )
    {
	bitsum = 1;
	for ( i = letter; i > 'A'; i-- )
	    bitsum *= 2;
    }

    else if ( 'a' <= letter && letter <= 'z' )
    {
	bitsum = 67108864; /* 2^26 */
	for ( i = letter; i > 'a'; i-- )
	    bitsum *= 2;
    }

    return bitsum;
}

/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 *   this function takes 40% to 50% of boot-up time.
 */
char *fread_string( FILE *fp )
{
    char *plast;
    char c;

    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING - MAX_STRING_LENGTH] )
    {
	bug( "Fread_string: MAX_STRING %d exceeded.", MAX_STRING );
	exit( 1 );
    }

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
	return &str_empty[0];

    for ( ; ; )
    {
        /*
         * Back off the char type lookup,
         *   it was too dirty for portability.
         *   -- Furey
         */

	switch ( *plast = getc(fp) )
	{
        default:
            plast++;
            break;

        case EOF:
	/* temp fix */
            bug( "Fread_string: EOF", 0 );
	    return NULL;
            /* exit( 1 ); */
            break;

        case '\n':
            plast++;
            *plast++ = '\r';
            break;

        case '\r':
            break;

        case '~':
            plast++;
	    {
		union
		{
		    char *	pc;
		    char	rgc[sizeof(char *)];
		} u1;
		int ic;
		int iHash;
		char *pHash;
		char *pHashPrev;
		char *pString;

		plast[-1] = '\0';
		iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
		for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
		{
		    for ( ic = 0; ic < sizeof(char *); ic++ )
			u1.rgc[ic] = pHash[ic];
		    pHashPrev = u1.pc;
		    pHash    += sizeof(char *);

		    if ( top_string[sizeof(char *)] == pHash[0]
		    &&   !strcmp( top_string+sizeof(char *)+1, pHash+1 ) )
			return pHash;
		}

		if ( fBootDb )
		{
		    pString		= top_string;
		    top_string		= plast;
		    u1.pc		= string_hash[iHash];
		    for ( ic = 0; ic < sizeof(char *); ic++ )
			pString[ic] = u1.rgc[ic];
		    string_hash[iHash]	= pString;

		    nAllocString += 1;
		    sAllocString += top_string - pString;
		    return pString + sizeof(char *);
		}
		else
		{
		    return str_dup( top_string + sizeof(char *) );
		}
	    }
	}
    }
}

char *fread_string_eol( FILE *fp )
{
    static bool char_special[256-EOF];
    char *plast;
    char c;

    if ( char_special[EOF-EOF] != TRUE )
    {
        char_special[EOF -  EOF] = TRUE;
        char_special['\n' - EOF] = TRUE;
        char_special['\r' - EOF] = TRUE;
    }

    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING - MAX_STRING_LENGTH] )
    {
        bug( "Fread_string: MAX_STRING %d exceeded.", MAX_STRING );
        exit( 1 );
    }

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '\n')
        return &str_empty[0];

    for ( ;; )
    {
        if ( !char_special[ ( *plast++ = getc( fp ) ) - EOF ] )
            continue;

        switch ( plast[-1] )
        {
        default:
            break;

        case EOF:
            bug( "Fread_string_eol  EOF", 0 );
            exit( 1 );
            break;

        case '\n':  case '\r':
            {
                union
                {
                    char *      pc;
                    char        rgc[sizeof(char *)];
                } u1;
                int ic;
                int iHash;
                char *pHash;
                char *pHashPrev;
                char *pString;

                plast[-1] = '\0';
                iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
                for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
                {
                    for ( ic = 0; ic < sizeof(char *); ic++ )
                        u1.rgc[ic] = pHash[ic];
                    pHashPrev = u1.pc;
                    pHash    += sizeof(char *);

                    if ( top_string[sizeof(char *)] == pHash[0]
                    &&   !strcmp( top_string+sizeof(char *)+1, pHash+1 ) )
                        return pHash;
                }

                if ( fBootDb )
                {
                    pString             = top_string;
                    top_string          = plast;
                    u1.pc               = string_hash[iHash];
                    for ( ic = 0; ic < sizeof(char *); ic++ )
                        pString[ic] = u1.rgc[ic];
                    string_hash[iHash]  = pString;

                    nAllocString += 1;
                    sAllocString += top_string - pString;
                    return pString + sizeof(char *);
                }
                else
                {
                    return str_dup( top_string + sizeof(char *) );
                }
            }
        }
    }
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;

    do
    {
	c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    do
    {
	cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }

    bug( "Fread_word: word too long.", 0 );
    exit( 1 );
    return NULL;
}

/*
 * Allocate some ordinary memory,
 *   with the expectation of freeing it someday.
 */
void *alloc_mem( int sMem )
{
    void *pMem;
    int *magic;
    int iList;

    sMem += sizeof(*magic);

    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
        if ( sMem <= rgSizeList[iList] )
            break;
    }

    if ( iList == MAX_MEM_LIST )
    {
        bug( "Alloc_mem: size %d too large.", sMem );
	return 0;
//	exit( 1 );
    }

    if ( rgFreeList[iList] == NULL )
    {
        pMem              = alloc_perm( rgSizeList[iList] );
    }
    else
    {
        pMem              = rgFreeList[iList];
        rgFreeList[iList] = * ((void **) rgFreeList[iList]);
    }

    magic = (int *) pMem;
    *magic = MAGIC_NUM;
    pMem += sizeof(*magic);

    return pMem;
}

void free_mem( void *pMem, int sMem )
{
    int iList;
    int *magic;

    pMem -= sizeof(*magic);
    magic = (int *) pMem;

    if (*magic != MAGIC_NUM)
    {
        bug("Attempt to recyle invalid memory of size %d.",sMem);
        bug((char*) pMem + sizeof(*magic),0);
        return;
    }

    *magic = 0;
    sMem += sizeof(*magic);

    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
        if ( sMem <= rgSizeList[iList] )
            break;
    }

    if ( iList == MAX_MEM_LIST )
    {
        bug( "Free_mem: size %d too large.", sMem );
        exit( 1 );
    }

    * ((void **) pMem) = rgFreeList[iList];
    rgFreeList[iList]  = pMem;

    return;
}

void *alloc_perm( int sMem )
{
    static char *pMemPerm;
    static int iMemPerm;
    void *pMem;

    while ( sMem % sizeof(long) != 0 )
	sMem++;
    if ( sMem > MAX_PERM_BLOCK )
    {
	bug( "Alloc_perm: %d too large.", sMem );
	return 0;
//	exit( 1 );
    }

    if ( pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK )
    {
	iMemPerm = 0;
	if ( ( pMemPerm = calloc( 1, MAX_PERM_BLOCK ) ) == NULL )
	{
	    perror( "Alloc_perm" );
	    exit( 1 );
	}
    }

    pMem        = pMemPerm + iMemPerm;
    iMemPerm   += sMem;
    nAllocPerm += 1;
    sAllocPerm += sMem;
    return pMem;
}

char *str_dup( const char *str )
{
    char *str_new;

    if ( str == NULL || str[0] == '\0' )
	return &str_empty[0];

    if ( str >= string_space && str < top_string )
	return (char *) str;

    str_new = alloc_mem( strlen(str) + 1 );
    strcpy( str_new, str );
    return str_new;
}

void free_string( char *pstr )
{
    if ( pstr == NULL
    ||   pstr == &str_empty[0]
    || ( pstr >= string_space && pstr < top_string ) )
	return;

    free_mem( pstr, strlen(pstr) + 1 );
    return;
}

char *str_replace_who( char *astr, char *bstr, char *cstr )
{
    char newstr[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int sstr1, sstr2;
    int ichar, jchar;
    char c1;

    c1 = bstr[0];

    if (str_infix_c(bstr, astr) )
	return astr;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    jchar = 0;

    if (sstr1 < sstr2)
	return astr;

    for ( ichar = 0; ichar <= sstr1 - sstr2; ichar++ )
    {
	if ( c1 == astr[ichar] && !str_prefix_c( bstr, astr + ichar ) )
	{
	    found = TRUE;
	    jchar = ichar;
	    ichar = sstr1;
	}
    }
    if (found)
    {
	buf[0] = '\0';
	for ( ichar = 0; ichar < jchar; ichar++ )
	{
	    sprintf(newstr, "%c", astr[ichar]);
	    strcat(buf, newstr);
	}
	strcat(buf, cstr);

	for ( ichar = jchar + sstr2; ichar < sstr1; ichar++ )
	{
	    sprintf(newstr, "%c", astr[ichar]);
	    strcat(buf, newstr);
	}

	sprintf(astr, "%s", buf );
	return astr;
    }
    return astr;
}

int sort_areas( const void *v1, const void *v2 )
{
    AREA_DATA *a1 = *(AREA_DATA **) v1;
    AREA_DATA *a2 = *(AREA_DATA **) v2;
    char name1[MAX_INPUT_LENGTH], name2[MAX_INPUT_LENGTH];
    int i;

    strcpy( name1, strip_color( a1->name ) );
    strcpy( name2, strip_color( a2->name ) );

    for ( i = 0; name1[i] != '\0'; i++ )
    {
	if ( name2[i] == '\0' )		return  1;
	if ( name1[i] == name2[i] )	continue;
	if ( name1[i] > name2[i] )	return  1;
	if ( name1[i] < name2[i] )	return -1;
    }

    return 0;
}

void do_areas( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea, *list[top_area];
    BUFFER *final = new_buf( );
    char arg[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH], levels[10];
    bool fAll = TRUE;
    bool fGood = FALSE;
    bool fEvil = FALSE;
    bool fNone = FALSE;
    bool fNeutral = FALSE;
    sh_int col = 0, lev = -1, pos = 0;

    name[0] = '\0';

    for ( ; ; )
    {
	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	    break;

	else if ( is_number( arg ) )
	    lev = atoi( arg );

	else if ( !str_prefix( arg, "good" ) )
	{
	    fAll = FALSE;
	    fGood = TRUE;
	}

	else if ( !str_prefix( arg, "evil" ) )
	{
	    fAll = FALSE;
	    fEvil = TRUE;
	}

	else if ( !str_prefix( arg, "neutral" ) )
	{
	    fAll = FALSE;
	    fNeutral = TRUE;
	}

	else if ( !str_prefix( arg, "none" ) )
	{
	    fAll = FALSE;
	    fNone = TRUE;
	}

	else
	    strcpy( name, arg );
    }

    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
	list[pos++] = pArea;

    qsort( list, top_area, sizeof( list[0] ), sort_areas );

    for ( pos = 0; pos < top_area; pos++ )
    {
	pArea = list[pos];

	if ( ( !IS_IMMORTAL( ch )
	&&     ( IS_SET( pArea->area_flags, AREA_UNLINKED )
	||       IS_SET( pArea->area_flags, AREA_SPECIAL ) ) )
	||   ( lev != -1
	&&     ( pArea->min_level > lev || pArea->max_level < lev ) )
	||   ( !fAll
	&&     ( ( pArea->alignment == 'G' && !fGood )
	||       ( pArea->alignment == 'E' && !fEvil )
	||       ( pArea->alignment == 'N' && !fNeutral )
	||       ( pArea->alignment == '?' && !fNone ) ) )
	||   ( name[0] != '\0'
	&&     !is_name( name, strip_color( pArea->name ) ) ) )
	    continue;

	if ( pArea->min_level == 1 && pArea->max_level == LEVEL_HERO )
	    sprintf( levels, " ALL " );

	else if ( pArea->min_level > LEVEL_HERO )
	    sprintf( levels, " IMM " );

	else if ( pArea->min_level == LEVEL_HERO )
	    sprintf( levels, " HRO " );

	else if ( pArea->max_level == LEVEL_HERO )
	    sprintf( levels, "%2d ++", pArea->min_level );

	else
	    sprintf( levels, "%2d %2d", pArea->min_level, pArea->max_level );

	sprintf( buf, "{s[{t%s{s] {q%-8.8s {t%c {s%s", levels, pArea->builder,
	    pArea->alignment == '?' ? ' ' : pArea->alignment,
	    end_string( pArea->name, 20 ) );
	add_buf( final, buf );

	if ( ++col % 2 == 0 )
	    add_buf( final, "\n\r" );
	else
	    add_buf( final, " " );
    }

    if ( col %2 != 0 )
	add_buf( final, "{x\n\r" );
    else
	add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );
}

void do_memory( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf;
    AREA_DATA *pArea;
    AUCTION_DATA *auc;
    BUFFER *final = new_buf();
    CHAR_DATA *wch;
    DESCRIPTOR_DATA *d;
    EXIT_DATA *exit;
    EXTRA_DESCR_DATA *ed;
    HELP_DATA *pHelp;
    MOB_INDEX_DATA *pMob;
    PROG_CODE *mProg;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    PC_DATA *pc;
    RESET_DATA *reset;
    ROOM_INDEX_DATA *pRoom;
    SHOP_DATA *shop;
    char buf[MAX_STRING_LENGTH];
    int aff_count, count, free_cnt, nMatch = 0, num_pc, vnum;

    extern EXIT_DATA *		exit_free;
    extern EXTRA_DESCR_DATA *	extra_descr_free;
    extern RESET_DATA *		reset_free;
    extern SHOP_DATA *		shop_free;
    extern MOB_INDEX_DATA *	mob_index_free;
    extern HELP_DATA *		help_free;
    extern AREA_DATA *		area_free;
    extern ROOM_INDEX_DATA *	room_index_free;
    extern OBJ_INDEX_DATA *	obj_index_free;
    extern AUCTION_DATA *	auction_free;
    extern PROG_CODE *		pcode_free;

    count = 0;
    free_cnt = 0;

    for ( auc = auction_list; auc != NULL; auc = auc->next )
	count++;

    for ( auc = auction_free; auc != NULL; auc = auc->next )
	free_cnt++;

    sprintf( buf, "{cAuctions{w:     {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	count, count * ( sizeof( *auc ) ),
	free_cnt, free_cnt * ( sizeof( *auc ) ) );
    add_buf( final, buf );

    aff_count = 0;
    num_pc = 0;

    free_cnt	= 0;
    for ( pMob = mob_index_free; pMob != NULL; pMob = pMob->next )
	free_cnt++;

    sprintf( buf, "{cMobile Vnums{w: {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	top_mob_index, top_mob_index * ( sizeof( MOB_INDEX_DATA ) ),
	free_cnt, free_cnt * ( sizeof( MOB_INDEX_DATA ) ) );
    add_buf( final, buf );

    count = 0;
    free_cnt = 0;

    for ( mProg = mprog_list; mProg != NULL; mProg = mProg->next )
	count++;
    for ( mProg = oprog_list; mProg != NULL; mProg = mProg->next )
	count++;
    for ( mProg = rprog_list; mProg != NULL; mProg = mProg->next )
	count++;

    for ( mProg = pcode_free; mProg != NULL; mProg = mProg->next )
	free_cnt++;

    sprintf( buf, "{cM/O/R Progra{w: {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	count, count * ( sizeof( *mProg ) ),
	free_cnt, free_cnt * ( sizeof( *mProg ) ) );
    add_buf( final, buf );

    count = 0;
    free_cnt = 0;

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	count++;

	if ( wch->pcdata != NULL )
	    num_pc++;

	for ( paf = wch->affected; paf != NULL; paf = paf->next )
	    aff_count++;
    }

    for ( wch = char_free; wch != NULL; wch = wch->next )
	free_cnt++;

    sprintf( buf, "{cCharacters{w:   {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	count, count * ( sizeof( *wch ) ),
	free_cnt, free_cnt * ( sizeof( *wch ) ) );
    add_buf( final, buf );

    free_cnt	= 0;
    for ( pc = pcdata_free; pc != NULL; pc = pc->next )
	free_cnt++;

    sprintf( buf, "{cPcData{w:       {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	num_pc, num_pc * ( sizeof( *pc ) ),
	free_cnt, free_cnt * ( sizeof( *pc ) ) );
    add_buf( final, buf );

    count = 0;
    free_cnt = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
	count++;

    for ( d = descriptor_free; d != NULL; d = d->next )
	free_cnt++;

    sprintf( buf, "{cDescriptors{w:  {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	count, count * ( sizeof( *d ) ),
	free_cnt, free_cnt * ( sizeof( *d ) ) );
    add_buf( final, buf );

    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    for ( paf = pObjIndex->affected; paf != NULL; paf = paf->next )
		aff_count++;
	    nMatch++;
	}
    }

    free_cnt = 0;
    for ( pObjIndex = obj_index_free; pObjIndex != NULL; pObjIndex = pObjIndex->next )
	free_cnt++;

    sprintf( buf, "{cObject Vnums{w: {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	top_obj_index, top_obj_index * ( sizeof( *pObjIndex ) ),
	free_cnt, free_cnt * ( sizeof( *pObjIndex ) ) );
    add_buf( final, buf );

    count = 0;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	count++;

	for ( paf = obj->affected; paf != NULL; paf = paf->next )
	    aff_count++;
    }

    free_cnt = 0;
    for ( obj = obj_free; obj != NULL; obj = obj->next )
	free_cnt++;

    sprintf( buf, "{cObjects{w:      {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	count, count * ( sizeof( *obj ) ),
	free_cnt, free_cnt * ( sizeof( *obj ) ) );
    add_buf( final, buf );

    free_cnt = 0;
    for ( paf = affect_free; paf != NULL; paf = paf->next )
	free_cnt++;

    sprintf( buf, "{cAffects{w:      {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	aff_count, aff_count * ( sizeof( *paf ) ),
	free_cnt, free_cnt * ( sizeof( *paf ) ) );
    add_buf( final, buf );

    free_cnt = 0;
    for ( pArea = area_free; pArea != NULL; pArea = pArea->next )
	free_cnt++;

    sprintf( buf, "{cAreas{w:        {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	top_area, top_area * ( sizeof( AREA_DATA ) ),
	free_cnt, free_cnt * ( sizeof( *pArea ) ) );
    add_buf( final, buf );

    free_cnt	= 0;
    for ( ed = extra_descr_free; ed != NULL; ed = ed->next )
	free_cnt++;

    sprintf( buf, "{cExtra Descs{w:  {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	top_ed, top_ed * ( sizeof( *ed ) ),
	free_cnt, free_cnt * ( sizeof( *ed ) ) );
    add_buf( final, buf );

    free_cnt	= 0;
    for ( exit = exit_free; exit != NULL; exit = exit->next )
	free_cnt++;

    sprintf( buf, "{cExits{w:        {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	top_exit, top_exit * ( sizeof( *exit ) ),
	free_cnt, free_cnt * ( sizeof( *exit ) ) );
    add_buf( final, buf );

    free_cnt = 0;
    for ( pHelp = help_free; pHelp != NULL; pHelp = pHelp->next )
	free_cnt++;

    sprintf( buf, "{cHelps{w:        {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	top_help, top_help * ( sizeof( HELP_DATA ) ),
	free_cnt, free_cnt * ( sizeof( *pHelp ) ) );
    add_buf( final, buf );

    free_cnt	= 0;
    for ( reset = reset_free; reset != NULL; reset = reset->next )
	free_cnt++;

    sprintf( buf, "{cResets{w:       {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	top_reset, top_reset * ( sizeof( *reset ) ),
	free_cnt, free_cnt * ( sizeof( *reset ) ) );
    add_buf( final, buf );

    free_cnt = 0;
    for ( pRoom = room_index_free; pRoom != NULL; pRoom = pRoom->next )
	free_cnt++;

    sprintf( buf, "{cRooms{w:        {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	top_room, top_room * ( sizeof( ROOM_INDEX_DATA ) ),
	free_cnt, free_cnt * ( sizeof( *pRoom ) ) );
    add_buf( final, buf );

    free_cnt	= 0;
    for ( shop = shop_free; shop != NULL; shop = shop->next )
	free_cnt++;

    sprintf( buf, "{cShops{w:        {R%5d {w({R%8d {cbytes{w), {R%4d {cfree {w({R%6d {cbytes{w).\n\r",
	top_shop, top_shop * ( sizeof( *shop ) ),
	free_cnt, free_cnt * ( sizeof( *shop ) ) );
    add_buf( final, buf );

    sprintf( buf, "{cSocials{w:      {R%5d {w({R%8d {cbytes{w).\n\r",
	maxSocial, maxSocial * ( sizeof( struct social_type ) ) );
    add_buf( final, buf );

    sprintf( buf, "{cStrings{w:      {R%5ld {w({R%8ld {cbytes{w)     ({cMAX_STRING{w: {R%d{w).\n\r",
	nAllocString, sAllocString, MAX_STRING );
    add_buf( final, buf );

    sprintf( buf, "{cPerms{w:       {R%6ld {w({R%8ld {cbytes{w).{x\n\r",
	nAllocPerm, sAllocPerm );
    add_buf( final, buf );

    page_to_char( final->string, ch );
    free_buf( final );
}

int number_range( int from, int to )
{
    int number, power;

    if ( from == 0 && to == 0 )
	return 0;

    if ( ( to = to - from + 1 ) <= 1 )
	return from;

    for ( power = 2; power < to; power <<= 1 )
	;

    while ( ( number = number_mm( ) & ( power -1 ) ) >= to )
	;

    return from + number;
}

int number_percent( void )
{
    int percent;

    while ( (percent = number_mm() & (128-1) ) > 99 )
	;

    return 1 + percent;
}

int number_door( void )
{
    int door;

    while ( ( door = number_mm() & (8-1) ) > 11)
	;

    return door;
}

int number_bits( int width )
{
    return number_mm( ) & ( ( 1 << width ) - 1 );
}

long number_mm( void )
{
    return random() >> 6;
}

int dice( int number, int size )
{
    int idice;
    int sum;

    switch ( size )
    {
    case 0: return 0;
    case 1: return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
	sum += number_range( 1, size );

    return sum;
}

void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
    {
	if ( *str == '~' )
	    *str = '-';
    }

    return;
}

void smash_dot_slash( char *str )
{
    for ( ; *str != '\0'; str++ )
    {
	if ( *str == '.' || *str == '/' )
	    *str = '-';
    }

    return;
}

/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
	bug( "Str_cmp: null astr.", 0 );
	return TRUE;
    }

    if ( bstr == NULL )
    {
	bug( "Str_cmp: null bstr.", 0 );
	return TRUE;
    }

    for ( ; *astr || *bstr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
	bug( "Strn_cmp: null astr.", 0 );
	return TRUE;
    }

    if ( bstr == NULL )
    {
	bug( "Strn_cmp: null bstr.", 0 );
	return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}

/*
 * Compare strings, case sensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix_c( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
	bug( "Strn_cmp: null astr.", 0 );
	return TRUE;
    }

    if ( bstr == NULL )
    {
	bug( "Strn_cmp: null bstr.", 0 );
	return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
	if ( *astr != *bstr )
	    return TRUE;
    }

    return FALSE;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( astr == NULL || bstr == NULL )
	return TRUE;

    if ( ( c0 = LOWER(astr[0]) ) == '\0' )
	return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
    {
	if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
	    return FALSE;
    }

    return TRUE;
}


/*
 * Compare strings, case sensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix_c( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( astr == NULL || bstr == NULL )
	return TRUE;

    if ( ( c0 = astr[0] ) == '\0' )
	return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
    {
	if ( c0 == bstr[ichar] && !str_prefix_c( astr, bstr + ichar ) )
	    return FALSE;
    }

    return TRUE;
}


/*
 * Replace a substring in a string, case insensitive...Russ Walsh
 * looks for bstr within astr and replaces it with cstr.
 */
char *str_replace( char *astr, char *bstr, char *cstr )
{
    char newstr[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int sstr1, sstr2;
    int ichar, jchar;
    char c0, c1, c2;

    if ( ( ( c0 = LOWER(astr[0]) ) == '\0' )
	|| ( ( c1 = LOWER(bstr[0]) ) == '\0' )
	|| ( ( c2 = LOWER(cstr[0]) ) == '\0' ) )
	return astr;

    if (str_infix(bstr, astr) )
	return astr;

/* make sure we don't start an infinite loop */
    if (!str_infix(bstr, cstr) )
	return astr;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    jchar = 0;

    if (sstr1 < sstr2)
	return astr;

    for ( ichar = 0; ichar <= sstr1 - sstr2; ichar++ )
    {
	if ( c1 == LOWER(astr[ichar]) && !str_prefix( bstr, astr + ichar ) )
	{
	    found = TRUE;
	    jchar = ichar;
	    ichar = sstr1;
	}
    }
    if (found)
    {
	buf[0] = '\0';
	for ( ichar = 0; ichar < jchar; ichar++ )
	{
	    sprintf(newstr, "%c", astr[ichar]);
	    strcat(buf, newstr);
	}
	strcat(buf, cstr);
	for ( ichar = jchar + sstr2; ichar < sstr1; ichar++ )
	{
	    sprintf(newstr, "%c", astr[ichar]);
	    strcat(buf, newstr);
	}
	sprintf(astr, "%s", str_replace(buf, bstr, cstr) );
	return astr;
    }
    return astr;
}

/*
 * Replace a substring in a string, case sensitive...Russ Walsh
 * looks for bstr within astr and replaces it with cstr.
 */
char *str_replace_c( char *astr, char *bstr, char *cstr )
{
    char newstr[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int sstr1, sstr2;
    int ichar, jchar;
    char c1;

/*
    char c0, c1, c2;

    if ( ( ( c0 = astr[0] ) == '\0' )
	|| ( ( c1 = bstr[0] ) == '\0' )
	|| ( ( c2 = cstr[0] ) == '\0' ) )
	return astr;
*/
    c1 = bstr[0];

    if (str_infix_c(bstr, astr) )
	return astr;

/* make sure we don't start an infinite loop */
    if (!str_infix_c(bstr, cstr) )
	return astr;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    jchar = 0;

    if (sstr1 < sstr2)
	return astr;

    for ( ichar = 0; ichar <= sstr1 - sstr2; ichar++ )
    {
	if ( c1 == astr[ichar] && !str_prefix_c( bstr, astr + ichar ) )
	{
	    found = TRUE;
	    jchar = ichar;
	    ichar = sstr1;
	}
    }
    if (found)
    {
	buf[0] = '\0';
	for ( ichar = 0; ichar < jchar; ichar++ )
	{
	    sprintf(newstr, "%c", astr[ichar]);
	    strcat(buf, newstr);
	}
	strcat(buf, cstr);

	for ( ichar = jchar + sstr2; ichar < sstr1; ichar++ )
	{
	    sprintf(newstr, "%c", astr[ichar]);
	    strcat(buf, newstr);
	}

	sprintf(astr, "%s", str_replace_c(buf, bstr, cstr) );
	return astr;
    }
    return astr;
}

/* Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
	return FALSE;
    else
	return TRUE;
}



/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
    static char strcap[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strcap[i] = LOWER(str[i]);
    strcap[i] = '\0';
    strcap[0] = UPPER(strcap[0]);
    return strcap;
}


/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, char *file, char *str )
{
    FILE *fp;
    char strtime[100];

    if ( str[0] == '\0' )
	return;

    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
	perror( file );
	send_to_char( "Could not open the file!\n\r", ch );
    }
    else
    {
	strftime( strtime, 100, "{%m/%d at %H:%M}", localtime( &current_time ));
	fprintf( fp, "%s [R:%5d] %s: %s\n",
	    strtime, ch->in_room != NULL ? ch->in_room->vnum : 0,
	    ch->name, str );
	fclose( fp );
    }

    return;
}

void bug( const char *str, int param )
{
    char buf[MAX_STRING_LENGTH];

    if ( fpArea != NULL )
    {
	int iLine;
	int iChar;

	if ( fpArea == stdin )
	{
	    iLine = 0;
	}
	else
	{
	    iChar = ftell( fpArea );
	    fseek( fpArea, 0, 0 );
	    for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
	    {
		while ( getc( fpArea ) != '\n' )
		    ;
	    }
	    fseek( fpArea, iChar, 0 );
	}

	sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
	log_string( buf );
    }

    if ( strlen(str) > MAX_STRING_LENGTH-15 )
    {
	sprintf( buf, "[*****] BUG: void bug: String too long!");
	log_string( buf );
	wiznet(buf,NULL,NULL,WIZ_OTHER,0,MAX_LEVEL);

	sprintf( buf, "[*****] COM: %s", last_command );
	log_string( buf );
	wiznet(buf,NULL,NULL,WIZ_OTHER,0,MAX_LEVEL);
	return;
    }

    strcpy( buf, "[*****] BUG: " );
    sprintf( buf + strlen(buf), str, param );
    log_string( buf );
    wiznet(buf,NULL,NULL,WIZ_OTHER,0,MAX_LEVEL);

    sprintf( buf, "[*****] COM: %s", last_command );
    log_string( buf );
    wiznet(buf,NULL,NULL,WIZ_OTHER,0,MAX_LEVEL);

    return;
}

void log_string( const char *str )
{
    FILE *fp;
    char *strtime;
    char buf[100];

    if (port == MAIN_GAME_PORT)
	strftime( buf, 100, "../log/%m-%d.log", localtime( &current_time ));
    else
	strftime( buf, 100, "../log/%m-%d.test", localtime( &current_time ));
    fp = fopen (buf, "a");
    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    fprintf( fp, "%s :: %s\n", strtime, str );

    if (port != MAIN_GAME_PORT)
	fprintf( stderr, "%s :: %s\n", strtime, str );

    fclose(fp);
    return;
}

void tail_chain( void )
{
    return;
}

void free_runbuf( DESCRIPTOR_DATA *d )
{
    if (d && d->run_buf)
    {
	free_string(d->run_buf);
	d->run_buf = NULL;
	d->run_head = NULL;
    }
    return;
}

void save_voting_polls()
{
    POLL_DATA *poll;
    VOTE_DATA *vote;
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    sh_int pos;

    for ( poll = first_poll; poll != NULL; poll = poll->next )
    {
	if ( poll->name == NULL )
	    continue;

	sprintf( buf, "../data/voting_polls/%s", poll->name );
	if ( (fp = fopen (buf, "w")) == NULL )
	{
	    bug( "Save_voting_polls: NULL file:", 0 );
	    bug( buf, 0 );
	    continue;
	}

	fprintf( fp, "Q %s~\n", poll->question );

	for ( pos = 0; pos < MAX_POLL_RESPONSES; pos++ )
	{
	    if ( poll->response[pos] == NULL )
		break;

	    fprintf( fp, "A %s~\n", poll->response[pos] );
	}

	for ( vote = poll->vote; vote != NULL; vote = vote->next_vote )
	{
	    fprintf( fp, "V %s~ %s~ %d\n",
		vote->voter_name, vote->voter_ip, vote->voter_vote );
	}

	fprintf( fp, "#\n\n" );
	fclose (fp);
    }

    return;
}

