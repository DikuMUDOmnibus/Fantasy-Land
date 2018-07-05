/***************************************************************************
*  Random Object Code - Written Exclusively for Asgardian Nightmare Mud    *
*  by Chris Langlois(tas@intrepid.inetsolve.com) and Gabe Volker           *
***************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "olc.h"
#include "rand_obj.h"
#include "recycle.h"

#define MAX_EXCEP_APPLY         12
#define RANDOMS_FILE		"../data/info/randoms.dat"

void	show_flag_cmds	args( ( CHAR_DATA *ch, const struct flag_type *flag_table ) );

char * get_random_editing( DESCRIPTOR_DATA *d )
{
    if ( d->editor == ED_PREFIX )
	return prefix_table[((int)d->pEdit)].name;
    else
	return suffix_table[((int)d->pEdit)].name;
}

sh_int random_lookup( RANDOM_DATA *table, char *argument )
{
    sh_int number;

    for ( number = 0; table[number].name[0] != '\0'; number++ )
    {  
	if ( !str_prefix( argument, table[number].name ) )
	    return number;
    }

    return -1;
}

void show_random_table( CHAR_DATA *ch, RANDOM_DATA *table )
{
    BUFFER *final = new_buf( );
    char buf[20];
    sh_int pos, col = 0;
   
    for ( pos = 0; table[pos].name[0] != '\0'; pos++ )
    {
	sprintf( buf, "{q%3d{s) {t%-15.15s", pos, table[pos].name );
	add_buf( final, buf );

	if ( ++col % 4 == 0 )
	    add_buf( final, "\n\r" );
    }

    if ( col % 4 != 0 )
	add_buf( final, "{x\n\r" );
    else
	add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );
}

MOD_DATA *new_random_mod( )
{
    MOD_DATA *mod;

    if ( random_mod_free == NULL )
	mod = alloc_perm( sizeof( *mod ) );
    else
    {
	mod = random_mod_free;
	random_mod_free = random_mod_free->next;
    }

    return mod;
}

void free_random_mod( MOD_DATA *mod )
{
    mod->next		= random_mod_free;
    random_mod_free	= mod;
}

const struct excep_apply_data eapply_table[MAX_EXCEP_APPLY] =
{
//  { Apply_Type,	Min,	Max	},
    { APPLY_STR,	  1,	 2	},
    { APPLY_INT,	  1,	 2	},
    { APPLY_WIS,	  1,	 2	},
    { APPLY_DEX,	  1,	 2	},
    { APPLY_CON,	  1,	 2	},
    { APPLY_MANA,	 10,	50	},
    { APPLY_MOVE,	 10,	50	},
    { APPLY_HIT,	 10,	50	},
    { APPLY_HITROLL,	  5,	15	},
    { APPLY_DAMROLL,	  5,	15	},
    { APPLY_SAVES,	 -3,	-1	},
    { APPLY_AC,		-20,	-5	}
};

void make_exceptional( OBJ_DATA *obj, bool name )
{
    AFFECT_DATA af;
    long flag = 0;
    sh_int pos;

    if ( obj->item_type == ITEM_WEAPON )
    {
//	obj->value[1] += number_range( 0, 2 );
//	obj->value[2] += number_range( 0, 2 );

	switch( dice( 1, 12 ) )
	{
	    default:						break;
	    case 1: flag = WEAPON_TWO_HANDS;			break;
	    case 2: flag = WEAPON_SHOCKING;			break;
	    case 3: flag = WEAPON_VORPAL;			break;
	    case 4: flag = WEAPON_FROST;			break;
	    case 5: flag = WEAPON_POISON;			break;
	    case 6: flag = WEAPON_VAMPIRIC;			break;
	    case 7: flag = WEAPON_FLAMING;			break;
	    case 8: obj->value[1] += number_range( 0, 1 );	break;
	    case 9: obj->value[2] += number_range( 0, 1 );	break;
	}

	// Add the random flag
	if ( flag != 0 )
	    SET_BIT( obj->value[4], flag );
    }

    else if ( obj->item_type == ITEM_ARMOR )
    {
	obj->value[1] += number_range( 0, 10 );
	obj->value[2] += number_range( 0, 10 );
	obj->value[3] += number_range( 0, 10 );
	obj->value[4] += number_range( 0, 10 );

	switch( dice( 1, 8 ) )
	{
	    default:				break;
	    case 1: flag = ITEM_MAGIC;		break;
	    case 2: flag = ITEM_HUM;		break;
	    case 3: flag = ITEM_BLESS;		break;
	    case 4: flag = ITEM_GLOW;		break;
	    case 5: flag = ITEM_EVIL;		break;
	}

	if ( flag != 0 )
	    SET_BIT( obj->extra_flags, flag );
    }

    // Now for some random Applies....
    for ( pos = 0; pos < 2; pos++ )
    {
	if ( number_percent( ) < 35 )
	{
	    flag	= number_range( 0, MAX_EXCEP_APPLY-1 );
	    af.location	= eapply_table[flag].apply_type;
	    af.where	= TO_OBJECT;
	    af.type	= 0;
	    af.duration	= -1;
	    af.dur_type	= DUR_TICKS;
	    af.bitvector= 0;
	    af.level	= obj->level;
	    af.modifier	= number_range( eapply_table[flag].min,
					eapply_table[flag].max );
	    affect_to_obj( obj, &af );
	}
    }

    if ( name )
    {
	char buf[MAX_STRING_LENGTH];

	sprintf( buf, "Exceptional %s", obj->short_descr );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, "%s exceptional", obj->name );
	free_string( obj->name );
	obj->name = str_dup( buf );
    }
}

void make_cracked( OBJ_DATA *obj, bool name )
{
    if ( obj->item_type == ITEM_WEAPON )
    {
	obj->value[1] -= number_range( 0, 1 );
	obj->value[2] -= number_range( 0, 1 );
	obj->value[1] = UMAX( 0, obj->value[1] );
	obj->value[2] = UMAX( 0, obj->value[2] );
    }

    else if ( obj->item_type == ITEM_ARMOR )
    {
	obj->value[1] -= number_range( 0, 8 );
	obj->value[2] -= number_range( 0, 8 );
	obj->value[3] -= number_range( 0, 8 );
	obj->value[4] -= number_range( 0, 8 );
    }

    if ( name )
    {
	char buf[MAX_STRING_LENGTH];

	sprintf( buf, "Cracked %s", obj->short_descr );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, "%s cracked", obj->name );
	free_string( obj->name );
	obj->name = str_dup( buf );
    }
}

void process_mods( OBJ_DATA *obj, RANDOM_DATA *table, int pos )
{
    AFFECT_DATA af;
    MOD_DATA *pMod;

    if ( table[pos].affect != 0 )
    {
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.where	= TO_AFFECTS;
	af.type		= 0;
	af.duration	= -1;
	af.dur_type	= DUR_TICKS;
	af.bitvector	= table[pos].affect;
	af.level	= obj->level;
	affect_to_obj( obj, &af );
    }

    if ( table[pos].shield != 0 )
    {
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.where	= TO_SHIELDS;
	af.type		= 0;
	af.duration	= -1;
	af.dur_type	= DUR_TICKS;
	af.bitvector	= table[pos].shield;
	af.level	= obj->level;
	affect_to_obj( obj, &af );
    }

    for ( pMod = table[pos].mods; pMod != NULL; pMod = pMod->next )
    {
	af.where	= pMod->where;
	af.type		= 0;
	af.duration	= -1;
	af.dur_type	= DUR_TICKS;
	af.bitvector	= 0;
	af.level	= obj->level;
	af.location	= pMod->location;
	af.modifier	= number_range( pMod->min, pMod->max );
	affect_to_obj( obj, &af );
    }
}

bool combine_affects( OBJ_DATA *obj, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf2;
    bool found = FALSE;

    for ( paf2 = obj->affected; paf2 != NULL; paf2 = paf2->next )
    {
	if ( paf2 != paf && paf2->location == paf->location
	&&   paf2->where == paf->where )
	{
	    paf->modifier += paf2->modifier;
	    found = TRUE;
	    break;
	}
    }

    if ( found )
    {
	AFFECT_DATA *paf3;

	if ( obj->affected == paf2 )
	    obj->affected = paf2->next;
	else
	{
	    for ( paf3 = obj->affected; paf3 != NULL; paf3 = paf3->next )
	    {
		if ( paf3->next == paf2 )
		{
		    paf3->next = paf2->next;
		    break;
		}
	    }
	}		

	free_affect( paf2 );
    }

    return found;
}

OBJ_DATA *rand_obj( int mob_level )
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    char buf_name[100], buf_short[100];
    bool add_prefix = FALSE, add_suffix = FALSE;
    sh_int prefix_number = 0, suffix_number = 0;
    sh_int align = 0, pslevel = 0, loop, rand_number;

    // 1-100 random
    rand_number = number_percent( );

    // Unique Objects - 2%
    if ( rand_number <= 2 && mob_level + 1 >= ( number_percent( ) / 2 ) + 91 )
    {
	// Pick only objects at or below the level of the killed mobile
	for ( loop = 0; loop < 200; loop++ )
	{
	    rand_number = number_range( mud_stat.unique_vnum[0], mud_stat.unique_vnum[1] );
	    if ( ( obj = create_object( get_obj_index( rand_number ) ) ) == NULL )
		continue;

	    if ( obj->level <= UMAX( 1, mob_level - 20 ) )
		break;

	    extract_obj( obj );
	}

	if ( obj == NULL )
	    return NULL;

	return obj;
    }

    buf_name[0] = '\0';
    buf_short[0] = '\0';

    // Magical Objects - 22%
    if ( rand_number <= 24 )
    {
	// Pick only objects at or below the level of the killed mobile
	for ( loop = 0; loop < 200; loop++ )
	{
	    rand_number = number_range( mud_stat.random_vnum[0], mud_stat.random_vnum[1] );
	    if ( ( obj = create_object( get_obj_index( rand_number ) ) ) == NULL )
		continue;

	    if ( obj->level <= UMAX( 1, mob_level - 20 ) )
		break;

	    extract_obj( obj );
	}

	if ( obj == NULL )
	    return NULL;

	// Check and see if Quality Changes
	// Exceptional
	if( ( rand_number = number_percent( ) ) < 15 )
	{
	    obj->cost = ( obj->level * 100 );
	    mob_level -= 7;
	    make_exceptional( obj, FALSE );
	    strcat( buf_name, "exceptional " );
	    strcat( buf_short, "Exceptional " );
	}

	else if ( rand_number > 85 )
	{
	    obj->cost = ( obj->level * 50 );
	    mob_level += 2;
	    make_cracked( obj, FALSE );
	    strcat( buf_name, "cracked " );
	    strcat( buf_short, "Cracked " );
	}

	else
	    obj->cost = ( obj->level * 75 );

	// Prefix, Suffix or Both?
	rand_number = dice ( 1, 3 );
	if ( rand_number == 1 )
	    add_prefix = TRUE;

	else if ( rand_number == 2 )
	    add_suffix = TRUE;

	else
	{
	    add_prefix = TRUE;
	    add_suffix = TRUE;
	}

	mob_level = UMAX( 1, mob_level );
	
	// Pick out Prefix/Suffix and be sure that the levels of the two
	// combined is less than or equal to the mob's level
	for ( loop = 0; loop < 200; loop++ )
	{
	    pslevel = 0;

	    if ( add_prefix )
	    {
		// Pick Prefix
		prefix_number = number_range ( 0, maxPrefix-1 );
		pslevel = prefix_table[prefix_number].level;
	    }

	    if ( add_suffix )
	    {
		// Pick Suffix
		suffix_number = number_range ( 0, maxSuffix-1 );
		pslevel += suffix_table[suffix_number].level;
	    }

	    if ( pslevel < mob_level )
		break;
	}

	if( add_prefix )
	{
	    // Add the Prefix
	    strcat( buf_short, prefix_table[prefix_number].name );
	    strcat( buf_short, " " );

	    strcat( buf_name, prefix_table[prefix_number].name );
	    strcat( buf_name, " " );

	    // Add the affects of the prefix
	    process_mods( obj, prefix_table, prefix_number );

	    // Compute align of object with prefix modifier
	    align += prefix_table[prefix_number].align;
	}

	// Add obj->short_descr to the total buf_short string
	strcat( buf_short, obj->short_descr );
	strcat( buf_name, obj->name );

	if ( add_suffix )
	{
	    // Add the Suffix
	    strcat( buf_short, " of " );
	    strcat( buf_short, suffix_table[suffix_number].name );

	    strcat( buf_name, " " );
	    strcat( buf_name, suffix_table[suffix_number].name );

	    // Add the affects of the suffix
	    process_mods( obj, suffix_table, suffix_number );

	    // Compute align of object with suffix modifier
	    align += suffix_table[suffix_number].align;
	}

	// Add alignment restrictions based on object alignment
	if ( align >= 600 )
	{
	    SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	    SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
	}

	else if ( align >= 300 && align < 600 )
	    SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );

	else if ( align > -600 && align <= -300 )
	    SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );

	else if ( align <= -600 )
	{
	    SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	    SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
	}

	// If suffix or prefix only, increase suff or pref level to make pslevel
	if ( ( add_prefix && !add_suffix )
	||   ( add_suffix && !add_prefix ) )
	{
	    pslevel *= 5;
	    pslevel /= 3;
	}

	// Set object level to combined pre/suffix level or 101.. whichever is lower
        obj->level = UMAX ( pslevel, obj->level );
        obj->level = UMIN ( obj->level, 101 );

	// Assign buf_short as the item's short description
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf_short );

	// Add the word 'special' to the item name for easy location
	strcat( buf_name, " special" );
    }

    // Regular Objects - 75%
    else
    {
	//  Pick only objects at or below the level of the killed mobile
	for ( loop = 0; loop < 200; loop++ )
	{
	    rand_number = number_range( mud_stat.random_vnum[0], mud_stat.random_vnum[1] );
	    if ( ( obj = create_object( get_obj_index( rand_number ) ) ) == NULL )
		continue;

	    if ( obj->level <= UMAX( 1, mob_level - 20 ) )
		break;

	    extract_obj( obj );
	}

	if ( obj == NULL )
	    return NULL;

	// Exceptional
	if( ( rand_number = number_percent( ) ) < 15 )
	{
	    obj->cost = ( obj->level * 1500 );
	    make_exceptional( obj, FALSE );
	    strcat( buf_short, "exceptional " );
	    strcat( buf_short, obj->short_descr );
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( buf_short );

	    // Add the word 'exceptional' to the item name for easy location
	    sprintf( buf_name, "%s exceptional", obj->name );
	}

	else if ( rand_number > 85 )
	{
	    obj->cost = ( obj->level * 500 );
	    make_cracked( obj, FALSE );
	    strcat( buf_short, "cracked " );
	    strcat( buf_short, obj->short_descr );
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( buf_short );

	    // Add the word 'cracked' to the item name for easy location
	    sprintf( buf_name, "%s cracked", obj->name );
	}

	else
        {
	    // Add the word 'normal' to the item name for easy location
	    sprintf( buf_name, "%s normal", obj->name );
	    obj->cost = ( obj->level * 1000 );
	}
    }

    // Add specially flagged name 'normal', 'special', or 'unique'
    free_string( obj->name );
    obj->name = str_dup( buf_name );

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	while ( combine_affects( obj, paf ) )
	    ;
    }

    return obj;
}
/*
OBJ_DATA *rand_obj3( CHAR_DATA *ch, char *argument )
{
//    AFFECT_DATA *paf;
    OBJ_INDEX_DATA *pObj;
    OBJ_DATA *obj = NULL;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    sh_int level, loop;
    int rand;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
	send_to_char(	"rand3 <level> unique\n\r"
			"rand3 <level> <obj_vnum> [arguments]\n\r"
			"\n\rNotes:\n\r"
			" If object vnum is left blank, one will be randomly chosen.\n\r"
			"\n\rValid arguments:\n\r", ch );
	return NULL;
    }

    level = atoi( arg1 );
    if ( level < 0 || level > 200 )
    {
	send_to_char( "Valid levels are 0 to 200.\n\r", ch );
	return NULL;
    }

    if ( !str_prefix( arg2, "unique" ) )
    {
	for ( loop = 0; loop < 200; loop++ )
	{
	    rand = number_range( mud_stat.unique_vnum[0], mud_stat.unique_vnum[1] );
	    if ( ( obj = create_object( get_obj_index( rand ) ) ) == NULL )
		continue;

	    if ( obj->level <= UMAX( 1, level - 20 ) )
		break;

	    extract_obj( obj );
	}

	return obj;
    }

    if ( is_number( arg2 ) )
    {
	if ( ( pObj = get_obj_index( atoi( arg2 ) ) ) == NULL )
	{
	    send_to_char( "That object does not exist.\n\r", ch );
	    return NULL;
	}

	obj = create_object( pObj );
	argument = one_argument( argument, arg2 );
    } else {
	for ( loop = 0; loop < 200; loop++ )
	{
	    rand = number_range( mud_stat.random_vnum[0], mud_stat.random_vnum[1] );
	    if ( ( obj = create_object( get_obj_index( rand ) ) ) == NULL )
		continue;

	    if ( obj->level <= UMAX( 1, level - 20 ) )
		break;

	    extract_obj( obj );
	}
    }

    if ( obj == NULL )
	return NULL;

    for ( ; ; )
    {
	argument = one_argument( argument, arg1 );

	if ( arg1[0] == '\0' )
	    break;

	else if ( !str_prefix( arg1, "exceptional" ) )
	    make_exceptional( obj, TRUE );

	else if ( !str_prefix( arg1, "cracked" ) )
	    make_cracked( obj, TRUE );

	else
	    act( "Skipping invalid argument: $T.", ch, NULL, arg1, TO_CHAR, POS_DEAD );
    }

    return obj;
}
*/
OBJ_DATA *rand_obj2( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    char buf_name[MAX_INPUT_LENGTH], buf_short[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    bool fNormal = FALSE;
    bool fCracked = FALSE;
    bool fSpecial = FALSE;
    bool fExceptional = FALSE;
    sh_int loop, prefix = 0, suffix = 0, pslevel, rand_number, align = 0;
    sh_int mob_level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' )
    {
	send_to_char(	"Syntax: rand2 <level> unique\n\r"
			"        rand2 <level> normal\n\r"
			"        rand2 <level> cracked\n\r"
			"        rand2 <level> exceptional\n\r"
			"        rand2 <level> special <prefix> <suffix>\n\r"
			"        rand2 <level> cspecial <prefix> <suffix>\n\r"
			"        rand2 <level> especial <prefix> <suffix>\n\r\n\r"
			"  If prefix or suffix is blank, one is randomly chosen.\n\r"
			"  Use \"none\" if you don't want one.\n\r", ch );
	return NULL;
    }

    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
	rand_obj2( ch, "" );
	return NULL;
    }

    mob_level = atoi( arg1 );
    if ( mob_level < 1 || mob_level > 200 )
    {
	send_to_char( "Level range is 1 to 200.\n\r", ch );
	return NULL;
    }

    if ( !str_prefix( arg2, "unique" ) )
    {
	for ( loop = 0; loop < 200; loop++ )
	{
	    rand_number = number_range( mud_stat.unique_vnum[0], mud_stat.unique_vnum[1] );
	    if ( ( obj = create_object( get_obj_index( rand_number ) ) ) == NULL )
		continue;

	    if ( obj->level <= UMAX( 1, mob_level - 20 ) )
		break;

	    extract_obj( obj );
	}

	if ( obj == NULL )
	    return NULL;

	return obj;
    }

    if ( !str_prefix( arg2, "normal" ) )
	fNormal = TRUE;
    else if ( !str_prefix( arg2, "exceptional" ) )
	fExceptional = TRUE;
    else if ( !str_prefix( arg2, "cracked" ) )	
	fCracked = TRUE;
    else if ( !str_prefix( arg2, "special" ) )
	fSpecial = TRUE;
    else if ( !str_prefix( arg2, "cspecial" ) )
    {
	fSpecial = TRUE;
	fCracked = TRUE;
    }
    else if ( !str_prefix( arg2, "especial" ) )
    {
	fSpecial = TRUE;
	fExceptional = TRUE;
    }
    else
    {
	rand_obj2( ch, "" );
	return NULL;
    }

    for ( loop = 0; loop < 200; loop++ )
    {
	rand_number = number_range( mud_stat.random_vnum[0], mud_stat.random_vnum[1] );
	if ( ( obj = create_object( get_obj_index( rand_number ) ) ) == NULL )
	    continue;

	if ( obj->level <= UMAX( 1, mob_level - 20 ) )
	    break;

	extract_obj( obj );
    }

    if ( obj == NULL )
	return NULL;

    if ( fNormal )
    {
	sprintf( buf_name, "%s normal", obj->name );
	free_string( obj->name );
	obj->name = str_dup( buf_name );
	obj->cost = ( obj->level * 75 );
	return obj;
    }

    snprintf( buf_name, MAX_INPUT_LENGTH, "%s", obj->name );
    buf_short[0] = '\0';

    if ( fExceptional )
    {
	mob_level -= 7;
	obj->cost = ( obj->level * 100 );
	make_exceptional( obj, FALSE );
	strcat( buf_short, "exceptional " );
	strcat( buf_name, " exceptional" );
    }

    if ( fCracked )
    {
	mob_level += 2;
	obj->cost = ( obj->level * 50 );
	make_cracked( obj, FALSE );
	strcat( buf_short, "cracked " );
	strcat( buf_name, " cracked" );
    }

    if ( fSpecial )
    {
	bool fPrefix = FALSE, fSuffix = FALSE;

	if ( arg3[0] == '\0' )
	    fPrefix = TRUE;
	else if ( !str_cmp( arg3, "none" ) )
	    prefix = -1;
	else
	{
	    if ( ( prefix = random_lookup( prefix_table, arg3 ) ) == -1 )
	    {
		send_to_char( "Valid prefixes:\n\r", ch );
		show_random_table( ch, prefix_table );
		extract_obj( obj );
		return NULL;
	    }
	}

	if ( argument[0] == '\0' )
	    fSuffix = TRUE;
	else if ( !str_cmp( argument, "none" ) )
	    suffix = -1;
	else
	{
	    if ( ( suffix = random_lookup( suffix_table, argument ) ) == -1 )
	    {
		send_to_char( "Valid suffixes:\n\r", ch );
		show_random_table( ch, suffix_table );
		extract_obj( obj );
		return NULL;
	    }
	}

	mob_level = UMAX( 1, mob_level );
	
	// Pick out Prefix/Suffix and be sure that the levels of the two
	// combined is less than or equal to the mob's level
	for ( loop = 0; loop < 200; loop++ )
	{
	    pslevel = 0;

	    if ( fPrefix )
	    {
		prefix = number_range ( 0, maxPrefix-1 );
		pslevel = prefix_table[prefix].level;
	    }

	    if ( fSuffix )
	    {
		suffix = number_range ( 0, maxSuffix-1 );
		pslevel += suffix_table[suffix].level;
	    }

	    if ( pslevel < mob_level )
		break;
	}

	if ( prefix != -1 )
	{
	    // Add the Prefix
	    strcat( buf_short, prefix_table[prefix].name );
	    strcat( buf_short, " " );

	    strcat( buf_name, " " );
	    strcat( buf_name, prefix_table[prefix].name );

	    // Add the affects of the prefix
	    process_mods( obj, prefix_table, prefix );

	    // Compute align of object with prefix modifier
	    align += prefix_table[prefix].align;
	}

	// Add obj->short_descr to the total buf_short string
	strcat( buf_short, obj->short_descr );

	if ( suffix != -1 )
	{
	    // Add the Suffix
	    strcat( buf_short, " of " );
	    strcat( buf_short, suffix_table[suffix].name );

	    strcat( buf_name, " " );
	    strcat( buf_name, suffix_table[suffix].name );

	    // Add the affects of the suffix
	    process_mods( obj, suffix_table, suffix );

	    // Compute align of object with suffix modifier
	    align += suffix_table[suffix].align;
	}

	// Add alignment restrictions based on object alignment
	if ( align >= 600 )
	{
	    SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	    SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
	}

	else if ( align >= 300 && align < 600 )
	    SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );

	else if ( align > -600 && align <= -300 )
	    SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );

	else if ( align <= -600 )
	{
	    SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	    SET_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
	}

	// If suffix or prefix only, increase suff or pref level to make pslevel
	if ( ( prefix != -1 && suffix == -1 )
	||   ( prefix == -1 && suffix != -1 ) )
	{
	    pslevel *= 5;
	    pslevel /= 3;
	}

	// Set object level to combined pre/suffix level or 101.. whichever is lower
        obj->level = UMAX ( pslevel, obj->level );
        obj->level = UMIN ( obj->level, 101 );

	// Add the word 'special' to the item name for easy location
	strcat( buf_name, " special" );
    }

    else
	strcat( buf_short, obj->short_descr );

    free_string( obj->name );
    obj->name = str_dup( buf_name );

    free_string( obj->short_descr );
    obj->short_descr = str_dup( buf_short );

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	while ( combine_affects( obj, paf ) )
	    ;
    }

    return obj;
}

int alphabetize_randoms( const void *v1, const void *v2 )
{
    RANDOM_DATA *a1 = *(RANDOM_DATA **) v1;
    RANDOM_DATA *a2 = *(RANDOM_DATA **) v2;
    sh_int i;

    for ( i = 0; a1->name[i] != '\0'; i++ )
    {
	if ( a2->name[i] == '\0' )		return 1;
	if ( a1->name[i] == a2->name[i] )	continue;
	if ( a1->name[i] > a2->name[i] )	return 1;
	if ( a1->name[i] < a2->name[i] )	return -1;
    }

    return 0;
}

void save_random( FILE *fp, RANDOM_DATA *random )
{
    MOD_DATA *pMod;

    fprintf( fp, "Name %s~\n",	random->name			);
    fprintf( fp, "Levl %d\n",	random->level			);
    fprintf( fp, "Alig %d\n",	random->align			);
    fprintf( fp, "Affb %s\n",	print_flags( random->affect )	);
    fprintf( fp, "Shld %s\n",	print_flags( random->shield )	);

    for ( pMod = random->mods; pMod != NULL; pMod = pMod->next )
	fprintf( fp, "Mods %d %d %d %d\n",
	    pMod->where, pMod->location, pMod->min, pMod->max	);

    fprintf( fp, "End\n\n"					);
}

void save_random_data( void )
{
    FILE *fp;
    RANDOM_DATA *prefix[maxPrefix], *suffix[maxSuffix];
    sh_int i;

    if ( ( fp = fopen ( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Could not open RANDOMS_FILE for writing.", 0 );
	return;
    }

    for ( i = 0; i < maxPrefix; i++ )
	prefix[i] = &prefix_table[i];

    qsort( prefix, maxPrefix, sizeof( prefix[0] ), alphabetize_randoms );

    fprintf( fp, "#MAX_PREFIX %d\n", maxPrefix );
    fprintf( fp, "#MAX_SUFFIX %d\n\n", maxSuffix );

    for ( i = 0 ; i < maxPrefix ; i++ )
    {
	fprintf( fp, "#PREFIX\n" );
	save_random( fp, prefix[i] );
    }

    for ( i = 0; i < maxSuffix; i++ )
	suffix[i] = &suffix_table[i];

    qsort( suffix, maxSuffix, sizeof( suffix[0] ), alphabetize_randoms );

    for ( i = 0 ; i < maxSuffix ; i++ )
    {
	fprintf( fp, "#SUFFIX\n" );
	save_random( fp, suffix[i] );
    }

    fprintf( fp, "#END\n" );

    fclose( fp );
    rename( TEMP_FILE, RANDOMS_FILE );
    mud_stat.randoms_changed = FALSE;
}

void load_random( FILE *fp, RANDOM_DATA *random, int pos )
{
    random[pos].name	= NULL;
    random[pos].mods	= NULL;
    random[pos].level	= 0;
    random[pos].align	= 0;
    random[pos].affect	= 0;
    random[pos].shield	= 0;

    for ( ; ; )
    {
	char *word = fread_word( fp );

	if ( !str_cmp( word, "End" ) )
	    break;

	switch( UPPER( word[0] ) )
	{
	    case 'A':
		LOAD( "Affb", random[pos].affect,	fread_flag( fp )   );
		LOAD( "Alig", random[pos].align,	fread_number( fp ) );
		break;

	    case 'L':
		LOAD( "Levl", random[pos].level,	fread_number( fp ) );
		break;

	    case 'M':
		if ( !str_cmp( word, "Mods" ) )
		{
		    MOD_DATA *pMod	= new_random_mod( );
		    pMod->where		= fread_number( fp );
		    pMod->location	= fread_number( fp );
		    pMod->min		= fread_number( fp );
		    pMod->max		= fread_number( fp );
		    pMod->next		= random[pos].mods;
		    random[pos].mods	= pMod;
		    break;
		}
		break;

	    case 'N':
		LOAD( "Name", random[pos].name,		fread_string( fp ) );
		break;

	    case 'S':
		LOAD( "Shld", random[pos].shield,	fread_flag( fp )   );
		break;
	}
    }
}

void load_random_data( )
{
    FILE *fp;
    sh_int prefix = -1, suffix = -1;

    if ( ( fp = fopen( RANDOMS_FILE, "r" ) ) == NULL )
    {
	bug( "Load_randoms: No race file found.", 0 );
	return;
    }

    for ( ; ; )
    {
	char *word = fread_word( fp );

	if ( !str_cmp( word, "#MAX_PREFIX" ) )
	{
	    maxPrefix = fread_number( fp );
	    prefix_table = malloc( sizeof( RANDOM_DATA ) * ( maxPrefix+1 ) );
	}

	else if ( !str_cmp( word, "#MAX_SUFFIX" ) )
	{
	    maxSuffix = fread_number( fp );
	    suffix_table = malloc( sizeof( RANDOM_DATA ) * ( maxSuffix+1 ) );
	}

	else if ( !str_cmp( word, "#PREFIX" ) )
	{
	    if ( ++prefix > maxPrefix )
	    {
		bug( "Load_randoms: maxPrefix exceeded.", 0 );
		fclose( fp );
		prefix_table[maxPrefix].name = str_dup( "" );
		suffix_table[maxSuffix].name = str_dup( "" );
		return;
	    }

	    load_random( fp, prefix_table, prefix );
	}

	else if ( !str_cmp( word, "#SUFFIX" ) )
	{
	    if ( ++suffix > maxSuffix )
	    {
		bug( "Load_randoms: maxSuffix exceeded.", 0 );
		fclose( fp );
		prefix_table[maxPrefix].name = str_dup( "" );
		suffix_table[maxSuffix].name = str_dup( "" );
		return;
	    }

	    load_random( fp, suffix_table, suffix );
	}

	else if ( !str_cmp( word, "#End" ) )
	    break;

	else
	{
	    bug( "Load_randoms: bad section name.", 0 );
	    break;
	}
    }

    prefix_table[maxPrefix].name = str_dup( "" );
    suffix_table[maxSuffix].name = str_dup( "" );

    fclose( fp );
}

bool prefix_edit_create( CHAR_DATA *ch, char *argument )
{
    RANDOM_DATA *new_table;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Which prefix do you wish to create?\n\r", ch );
	return FALSE;
    }

    if ( random_lookup( prefix_table, argument ) != -1 )
    {
	send_to_char( "A prefix with that name already exists.\n\r", ch );
	return FALSE;
    }

    maxPrefix++;
    new_table = realloc( prefix_table, sizeof( RANDOM_DATA ) * ( maxPrefix + 1 ) );

    if ( !new_table ) /* realloc failed */
    {
	send_to_char( "Memory allocation failed. Brace for impact.\n\r", ch );
	return FALSE;
    }

    prefix_table = new_table;

    free_string( prefix_table[maxPrefix-1].name );
    prefix_table[maxPrefix-1].name	= str_dup( argument );
    prefix_table[maxPrefix-1].mods	= NULL;
    prefix_table[maxPrefix-1].level	= 0;
    prefix_table[maxPrefix-1].align	= 0;
    prefix_table[maxPrefix-1].affect	= 0;
    prefix_table[maxPrefix-1].shield	= 0;

    prefix_table[maxPrefix].name = str_dup( "" );

    send_to_char( "New prefix added.\n\r", ch );

    ch->desc->editor = ED_PREFIX;
    ch->desc->pEdit = (void *)maxPrefix-1;

    return TRUE;
}

bool suffix_edit_create( CHAR_DATA *ch, char *argument )
{
    RANDOM_DATA *new_table;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Which suffix do you wish to create?\n\r", ch );
	return FALSE;
    }

    if ( random_lookup( suffix_table, argument ) != -1 )
    {
	send_to_char( "A suffix with that name already exists.\n\r", ch );
	return FALSE;
    }

    maxSuffix++;
    new_table = realloc( suffix_table, sizeof( RANDOM_DATA ) * ( maxSuffix + 1 ) );

    if ( !new_table ) /* realloc failed */
    {
	send_to_char( "Memory allocation failed. Brace for impact.\n\r", ch );
	return FALSE;
    }

    suffix_table = new_table;

    free_string( suffix_table[maxSuffix-1].name );
    suffix_table[maxSuffix-1].name	= str_dup( argument );
    suffix_table[maxSuffix-1].mods	= NULL;
    suffix_table[maxSuffix-1].level	= 0;
    suffix_table[maxSuffix-1].align	= 0;
    suffix_table[maxSuffix-1].affect	= 0;
    suffix_table[maxSuffix-1].shield	= 0;

    suffix_table[maxSuffix].name = str_dup( "" );

    send_to_char( "New suffix added.\n\r", ch );

    ch->desc->editor = ED_SUFFIX;
    ch->desc->pEdit = (void *)maxSuffix-1;

    return TRUE;
}

void do_prefix_edit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int sn;

    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: prefix_edit <prefix_name>\n\r"
		      "        prefix_edit <prefix_number>\n\r"
		      "        prefix_edit list\n\r"
		      "        prefix_edit create <new prefix name>\n\r", ch );
	return;
    }

    else if ( !str_cmp( argument, "list" ) )
    {
	show_random_table( ch, prefix_table );
	return;
    }

    sn = is_number( argument ) ? atoi( argument ) :
	random_lookup( prefix_table, argument );

    if ( sn >= 0 && sn < maxPrefix )
    {
	ch->desc->editor = ED_PREFIX;
	ch->desc->pEdit = (void *)sn;
	random_edit_show( ch, "" );
	return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "create" ) )
    {
	if ( prefix_edit_create( ch, argument ) )
	    mud_stat.randoms_changed = TRUE;
	return;
    }

    send_to_char( "Invalid prefix.\n\r\n\r", ch );
    do_prefix_edit( ch, "" );
}

void do_suffix_edit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int sn;

    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: suffix_edit <suffix_name>\n\r"
		      "        suffix_edit <suffix_number>\n\r"
		      "        suffix_edit list\n\r"
		      "        suffix_edit create <new suffix name>\n\r", ch );
	return;
    }

    else if ( !str_cmp( argument, "list" ) )
    {
	show_random_table( ch, suffix_table );
	return;
    }

    sn = is_number( argument ) ? atoi( argument ) :
	random_lookup( suffix_table, argument );

    if ( sn >= 0 && sn < maxSuffix )
    {
	ch->desc->editor = ED_SUFFIX;
	ch->desc->pEdit = (void *)sn;
	random_edit_show( ch, "" );
	return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "create" ) )
    {
	if ( suffix_edit_create( ch, argument ) )
	    mud_stat.randoms_changed = TRUE;
	return;
    }

    send_to_char( "Invalid suffix.\n\r\n\r", ch );
    do_suffix_edit( ch, "" );
}

#define EDIT_RANDOM( ch, table, pos )		\
    if ( ch->desc->editor == ED_PREFIX )	\
	table = prefix_table;			\
    else					\
	table = suffix_table;			\
    pos = (int)ch->desc->pEdit;

bool random_edit_addaffect( CHAR_DATA *ch, char *argument )
{
    MOD_DATA *pMod;
    RANDOM_DATA *table;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    sh_int pos, location, value1, value2;

    EDIT_RANDOM( ch, table, pos );

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax:  addaffect [location] [min] [max]\n\r", ch );
	return FALSE;
    }

    if ( ( location = flag_value( apply_flags, arg1 ) ) == NO_FLAG )
    {
        send_to_char( "Valid affects are:\n\r", ch );
	show_help( ch, "apply" );
	return FALSE;
    }

    value1 = atoi( arg2 );
    value2 = atoi( argument );

    if ( value1 > value2 )
    {
	send_to_char( "Min value must be higher than max value.\n\r", ch );
	return FALSE;
    }

    pMod		= new_random_mod( );
    pMod->where		= TO_OBJECT;
    pMod->location	= location;
    pMod->min		= value1;
    pMod->max		= value2;
    pMod->next		= table[pos].mods;
    table[pos].mods	= pMod;

    send_to_char( "Affect added.\n\r", ch );

    return TRUE;
}

bool random_edit_affects( CHAR_DATA *ch, char *argument )
{
    RANDOM_DATA *table;
    sh_int pos;
    long value;

    EDIT_RANDOM( ch, table, pos );

    if ( argument[0] == '\0'
    ||   ( value = flag_value( affect_flags, argument ) ) == NO_FLAG )
    {
	send_to_char( "Invalid flag.\n\n", ch );
	show_flag_cmds( ch, affect_flags );
	return FALSE;
    }

    table[pos].affect ^= value;
    send_to_char( "Affect flags toggled.\n\r", ch );

    return TRUE;
}

bool random_edit_align( CHAR_DATA *ch, char *argument )
{
    RANDOM_DATA *table;
    sh_int pos, value;

    EDIT_RANDOM( ch, table, pos );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  alignment [number]\n\r", ch );
	return FALSE;
    }

    value = atoi( argument );
    if ( value < -1000 || value > 1000 )
    {
	send_to_char( "Max alignment range is -1000 to 1000.\n\r", ch );
	return FALSE;
    }

    table[pos].align = value;

    send_to_char( "Alignment set.\n\r", ch);
    return TRUE;
}

bool random_edit_create( CHAR_DATA *ch, char *argument )
{
    if ( ch->desc->editor == ED_PREFIX )
	return prefix_edit_create( ch, argument );
    else
	return suffix_edit_create( ch, argument );
}

bool random_edit_dam_mod( CHAR_DATA *ch, char *argument )
{
    MOD_DATA *pMod;
    RANDOM_DATA *table;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    sh_int dam_mod, pos, value1, value2;

    EDIT_RANDOM( ch, table, pos );

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Dam_mod (dam_type) (min_mod) (max_mod).\n\r", ch );
	return FALSE;
    }

    if ( !is_number( arg2 ) || !is_number( argument ) )
    {
	send_to_char( "Min_mod and max_mod must be valid numbers.\n\r", ch );
	return FALSE;
    }

    value1 = atoi( arg2 );
    value2 = atoi( argument );

    if ( value1 > value2 )
    {
	send_to_char( "Min_mod can not be more than max_mod.\n\r", ch );
	return FALSE;
    }

    if ( value1 < -20 || value2 > 20 )
    {
	send_to_char( "Modifiers must be between -20% and 20%.\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( arg1, "all" ) )
	dam_mod = DAM_ALL;

    else if ( ( dam_mod = dam_type_lookup( arg1 ) ) == -1 )
    {
	send_to_char( "Valid damage types are:\n all", ch );
	for ( dam_mod = 0; dam_mod < DAM_MAX; dam_mod++ )
	{
	    sprintf( arg1, " %s", damage_mod_table[dam_mod].name );
	    send_to_char( arg1, ch );
	}

	send_to_char( "\n\r", ch );
	return FALSE;
    }

    pMod		= new_random_mod( );
    pMod->where		= TO_DAM_MODS;
    pMod->location	= dam_mod;
    pMod->min		= value1;
    pMod->max		= value2;
    pMod->next		= table[pos].mods;
    table[pos].mods	= pMod;

    send_to_char( "Damage modifier applied.\n\r", ch );
    return TRUE;
}

bool random_edit_delaffect( CHAR_DATA *ch, char *argument )
{
    MOD_DATA *pMod, *pMod2;
    RANDOM_DATA *table;
    sh_int count = -1, pos, value;

    EDIT_RANDOM( ch, table, pos );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax: delaffect [#]\n\r", ch );
	return FALSE;
    }

    if ( ( value = atoi( argument ) ) < 0 )
    {
	send_to_char( "Negative numbers are not allowed.\n\r", ch );
	return FALSE;
    }

    for ( pMod = table[pos].mods; pMod != NULL; pMod = pMod->next )
    {
	if ( ++count == value )
	{
	    if ( table[pos].mods == pMod )
		table[pos].mods = pMod->next;
	    else
	    {
		for ( pMod2 = table[pos].mods; pMod2 != NULL; pMod2 = pMod2->next )
		{
		    if ( pMod2->next == pMod )
		    {
			pMod2->next = pMod->next;
			break;
		    }
		}
	    }

	    free_random_mod( pMod );
	    send_to_char( "Random modifier deleted.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Modifier not found.\n\r", ch );
    return FALSE;
}

bool random_edit_delete( CHAR_DATA *ch, char *argument )
{
    RANDOM_DATA *table, *new_table;
    sh_int i, j, pos;

    EDIT_RANDOM( ch, table, pos );

    if ( str_cmp( argument, table[pos].name ) )
    {
	send_to_char( "Argument must match random name and you must be editing random.\n\r", ch );
	return FALSE;
    }

    if ( ch->desc->editor == ED_PREFIX )
	new_table = malloc( sizeof( RANDOM_DATA ) * maxPrefix );
    else
	new_table = malloc( sizeof( RANDOM_DATA ) * maxSuffix );

    check_olc_delete( ch );

    if ( !new_table )
    {
	send_to_char( "Memory allocation failed. Brace for impact...\n\r", ch );
	return FALSE;
    }

    if ( ch->desc->editor == ED_PREFIX )
    {
	for ( i = 0, j = 0; i < maxPrefix+1; i++ )
	{
	    if ( i != pos )
	    {
		new_table[j] = prefix_table[i];
		j++;
	    }
	}

	free( prefix_table );
	prefix_table = new_table;
	maxPrefix--; /* Important :() */
    } else {
	for ( i = 0, j = 0; i < maxSuffix+1; i++ )
	{
	    if ( i != pos )
	    {
		new_table[j] = suffix_table[i];
		j++;
	    }
	}

	free( suffix_table );
	suffix_table = new_table;
	maxSuffix--; /* Important :() */
    }

    send_to_char( "That random is history now.\n\r", ch );
    return TRUE;
}

bool random_edit_level( CHAR_DATA *ch, char *argument )
{
    RANDOM_DATA *table;
    sh_int pos, value;

    EDIT_RANDOM( ch, table, pos );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  level [number]\n\r", ch );
	return FALSE;
    }

    value = atoi( argument );
    if ( value < 0 || value > MAX_LEVEL )
    {
	send_to_char( "Level range is 0 to MAX_LEVEL.\n\r", ch );
	return FALSE;
    }

    table[pos].level = atoi( argument );

    send_to_char( "Level set.\n\r", ch);

    return TRUE;
}

bool random_edit_name( CHAR_DATA *ch, char *argument )
{
    RANDOM_DATA *table;
    sh_int pos;

    EDIT_RANDOM( ch, table, pos );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  name [string]\n\r", ch );
	return FALSE;
    }

    free_string( table[pos].name );
    table[pos].name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch);

    return TRUE;
}

bool random_edit_shields( CHAR_DATA *ch, char *argument )
{
    RANDOM_DATA *table;
    sh_int pos;
    long value;

    EDIT_RANDOM( ch, table, pos );

    if ( argument[0] == '\0'
    ||   ( value = flag_value( shield_flags, argument ) ) == NO_FLAG )
    {
	send_to_char( "Invalid flag.\n\n", ch );
	show_flag_cmds( ch, shield_flags );
	return FALSE;
    }

    table[pos].shield ^= value;
    send_to_char( "Shield flags toggled.\n\r", ch );

    return TRUE;
}

bool random_edit_show( CHAR_DATA *ch, char *argument )
{
    BUFFER *final = new_buf( );
    MOD_DATA *pMod;
    RANDOM_DATA *table;
    char buf[MAX_STRING_LENGTH];
    sh_int pos, cnt = 0;

    EDIT_RANDOM( ch, table, pos );

    sprintf( buf, "{qNumber:  {s[{t%d{s]\n\r", pos );
    add_buf( final, buf );

    sprintf( buf, "{qName:    {s[{t%s{s]\n\r", table[pos].name );
    add_buf( final, buf );

    sprintf( buf, "{qLevel:   {s[{t%4d{s]\n\r", table[pos].level );
    add_buf( final, buf );

    sprintf( buf, "{qAlign:   {s[{t%4d{s]\n\r", table[pos].align );
    add_buf( final, buf );

    sprintf( buf, "{qAffects: {s[{t%s{s]\n\r",
	flag_string( affect_flags, table[pos].affect ) );
    add_buf( final, buf );

    sprintf( buf, "{qShields: {s[{t%s{s]\n\r",
	flag_string( shield_flags, table[pos].shield ) );
    add_buf( final, buf );

    for ( pMod = table[pos].mods; pMod != NULL; pMod = pMod->next )
    {
	/*char aff[MAX_STRING_LENGTH];*/

	if ( cnt == 0 )
	{
	    add_buf( final, "\n\r{qNumber   Min   Max Affect\n\r" );
	    add_buf( final, "{s------ ----- ----- -------------\n\r" );
	}

	/*aff[0] = '\0';*/

	if ( pMod->where == TO_DAM_MODS )
	{
	    sprintf( buf, "{s[{t%4d{s] {t%4d%% %4d%% %s damage\n\r",
		cnt, pMod->min, pMod->max,
		pMod->location == DAM_ALL ? "all" :
		damage_mod_table[pMod->location].name );
	    add_buf( final, buf );
	} else {
	    sprintf( buf, "{s[{t%4d{s] {t%5d %5d %s\n\r", cnt,
		pMod->min, pMod->max,
		flag_string( apply_flags, pMod->location ) );
	    add_buf( final, buf );
	}
	cnt++;
    }

    add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}
