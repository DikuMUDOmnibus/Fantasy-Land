/***************************************************************************
 *  File: olc_act.c                                                        *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "olc.h"
#include "recycle.h"
#include "interp.h"

#define REDIT( fun )	bool fun( CHAR_DATA *ch, char *argument )
#define OEDIT( fun )	bool fun( CHAR_DATA *ch, char *argument )
#define MEDIT( fun )	bool fun( CHAR_DATA *ch, char *argument )
#define MPEDIT( fun )	bool fun( CHAR_DATA *ch, char *argument )
#define OLC( fun )	bool fun( CHAR_DATA *ch, char *argument )

DECLARE_SPELL_FUN( spell_null	);

void	save_area		args( ( AREA_DATA *pArea, bool deleted ) );
void	save_area_list		args( ( void ) );
void	affect_modify		args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
char	*target_save		args( ( sh_int pos ) );
char	*pos_save		args( ( sh_int pos ) );
sh_int	pos_load		args( ( char *pos ) );
sh_int	target_load		args( ( char *pos ) );
void	free_roster		args( ( ROSTER_DATA *roster ) );
int	channel_lookup		args( ( char *argument ) );
AREA_DATA *get_clan_area	args( ( sh_int clan ) );

struct olc_help_type
{
    char *	command;
    const void *structure;
    char *	desc;
};

const char *stat_names[MAX_STATS] =
{
    "strength", "intelligence", "wisdom", "dexterity", "constitution"
};

sh_int stat_lookup( char *argument )
{
    sh_int stat;

    for ( stat = 0; stat < MAX_STATS; stat++ )
    {
	if ( !str_prefix( argument, stat_names[stat] ) )
	    return stat;
    }

    return -1;
}

bool check_argument( CHAR_DATA *ch, char *argument, bool allow_space )
{
    for ( ; *argument != '\0'; argument++ )
    {
	if ( *argument == ' ' )
	{
	    if ( !allow_space )
	    {
		send_to_char( "Spaces not allowed, consider using underscore.\n\r", ch );
		return FALSE;
	    }

	    continue;
	}

	if ( ( *argument != '_' && !isalnum( *argument ) )
	||   isupper( *argument ) )
	{
	    if ( !allow_space )
		send_to_char( "Only lower case letters, numbers and underscores are allowed.\n\r", ch );
	    else
		send_to_char( "Only lower case letters, numbers, underscores and spaces are allowed.\n\r", ch );
	    return FALSE;
	}
    }

    return TRUE;
}

char * double_color( const char *argument )
{
    static char buf[MAX_INPUT_LENGTH];
    char *str;

    buf[0] = '\0';
    str = buf;

    for ( ; *argument != '\0'; argument++ )
    {
	if ( *argument == '{' )
	{
	    *str++ = *argument;
	    *str++ = '{';
	    argument++;
	}

	*str++ = *argument;
    }

    *str++ = '\0';

    return buf;
}

char *show_true_false( bool stat )
{
    if ( stat )
 	return "YES";
    else
	return "NO";
}

void mobile_balance( MOB_INDEX_DATA *pMob, sh_int type )
{
    sh_int level, pos;

    if ( type == 1 )
	level = URANGE( 1, pMob->level-20, 130 );

    else if ( type == 2 )
	level = URANGE( 5, pMob->level, 175 );

    else
	level = URANGE( 10, pMob->level + 20, 202 );

    for ( pos = 0; pos < 4; pos++ )
	pMob->ac[pos]		= -number_range( level * 3, level * 5 );

    pMob->wealth		= number_range( level * 40, level * 60 );
    pMob->hitroll		= number_range( level * 2, level * 3 );
    pMob->hit[0]		= number_range( level * 80, level * 90 );
    pMob->hit[1]		= number_range( level * 90, level * 100 );
    pMob->mana[0]		= number_range( level * 80, level * 90 );
    pMob->mana[1]		= number_range( level * 90, level * 100 );
    pMob->damage[DICE_NUMBER]	= number_range( level / 4, level / 3 );
    pMob->damage[DICE_TYPE]	= number_range( level / 4, level / 3 );
    pMob->damage[DICE_BONUS]	= number_range( level * 2, level * 3 );
}

bool check_affects( OBJ_INDEX_DATA *pObj, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf2;
    bool found = FALSE;

    for ( paf2 = pObj->affected; paf2 != NULL; paf2 = paf2->next )
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

	if ( pObj->affected == paf2 )
	    pObj->affected = paf2->next;
	else
	{
	    for ( paf3 = pObj->affected; paf3 != NULL; paf3 = paf3->next )
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

void object_reset( OBJ_INDEX_DATA *pObj )
{
    OBJ_DATA *obj;
    int pos;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData != pObj )
	    continue;

	switch( pObj->item_type )
	{
	    case ITEM_ARMOR:
		if ( obj->wear_loc != WEAR_NONE )
		{
		    for ( pos = 0; pos < 4; pos++ )
		    {
			obj->carried_by->armor[pos] += apply_ac( obj, obj->wear_loc, pos );
			obj->value[pos] = pObj->value[pos];
			obj->carried_by->armor[pos] -= apply_ac( obj, obj->wear_loc, pos );
		    }
		} else
		    for ( pos = 0; pos < 4; pos++ )
			obj->value[pos] = pObj->value[pos];
		break;

	    default:
		for ( pos = 0; pos < 5; pos++ )
		    obj->value[pos] = pObj->value[pos];
		break;
	}
    }
}

void affect_edit( OBJ_INDEX_DATA *pObj, AFFECT_DATA *paf, sh_int new_value, bool new )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData != pObj )
	    continue;

	if ( obj->carried_by && !new )
	    affect_modify( obj->carried_by, paf, FALSE );

	paf->modifier = new_value;

	if ( obj->carried_by )
	    affect_modify( obj->carried_by, paf, TRUE );
    }

    paf->modifier = new_value;
}

void object_balance( OBJ_INDEX_DATA *pObj, sh_int level )
{
    AFFECT_DATA *paf;
    sh_int pos;

    if ( IS_SET( pObj->extra_flags, ITEM_AQUEST )
    ||   IS_SET( pObj->extra_flags, ITEM_FORGED ) )
    {
	bug( "Object_balance called on quest/forged item!", 0 );
	return;
    }

    for ( paf = pObj->affected; paf != NULL; paf = paf->next )
    {
	while ( check_affects( pObj, paf ) )
	    ;

	if ( paf->where == TO_DAM_MODS )
	{
	    paf->modifier = URANGE( -10, paf->modifier, 10 );
	}

	else
	{
	    if ( paf->location == APPLY_HIT
	    ||   paf->location == APPLY_MANA
	    ||   paf->location == APPLY_MOVE )
		affect_edit( pObj, paf, UMIN( level * 3, paf->modifier ), FALSE );

	    else if ( paf->location == APPLY_HITROLL
		 ||   paf->location == APPLY_DAMROLL )
		affect_edit( pObj, paf, UMIN( (2 * level / 5), paf->modifier ), FALSE );

	    else if ( paf->location == APPLY_SAVES )
		affect_edit( pObj, paf, UMAX( (-level / 25), paf->modifier ), FALSE );

	    else if ( paf->location == APPLY_AC )
		affect_edit( pObj, paf, UMAX( (-level / 4), paf->modifier ), FALSE );

	    else if ( paf->location == APPLY_STR
		 ||   paf->location == APPLY_WIS
		 ||   paf->location == APPLY_INT
		 ||   paf->location == APPLY_DEX
		 ||   paf->location == APPLY_CON )
		affect_edit( pObj, paf, UMIN( 3, paf->modifier ), FALSE );
	}
    }

    if ( pObj->cost == 0 || pObj->cost > level * 250 )
	pObj->cost = number_range( level * 100, level * 250 );

    switch( pObj->item_type )
    {
	default:
	    break;

	case ITEM_ARMOR:
	    for ( pos = 0; pos < 4; pos++ )
		pObj->value[pos] = UMIN(
		    number_range( level / 3, level / 2 ) , pObj->value[pos] );
	    break;

	case ITEM_FURNITURE:
	    pObj->value[0] = UMIN( 2 + ( level / 50 ), pObj->value[0] );
	    pObj->value[3] = UMIN( 50 + ( level * 3 ), pObj->value[3] );
	    pObj->value[4] = UMIN( 50 + ( level * 3 ), pObj->value[4] );
	    break;

	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
	case ITEM_WAND:
	    pObj->value[0] = UMIN( level+15, pObj->value[0] );
	    break;

	case ITEM_WEAPON:
	    if ( pObj->level < 50 )
		pos = 5;
	    else
		pos = 2;
/*
	    if ( IS_SET(pObj->value[4], WEAPON_TWO_HANDS) )
		pos += 2;
	    if ( IS_SET(pObj->value[4], WEAPON_FLAMING) )
		pos -= 2;
	    if ( IS_SET(pObj->value[4], WEAPON_SHARP) )
		pos -= 2;
	    if ( IS_SET(pObj->value[4], WEAPON_SHOCKING) )
		pos -= 2;
	    if ( IS_SET(pObj->value[4], WEAPON_FROST) )
		pos -= 1;
	    if ( IS_SET(pObj->value[4], WEAPON_POISON) )
		pos -= 1;
	    if ( IS_SET(pObj->value[4], WEAPON_VAMPIRIC) )
		pos -= 1;
*/
	    pObj->value[1] = pos + number_range( level / 9, level / 8 );
	    pObj->value[2] = pos + number_range( level / 9, level / 8 );

	    break;
    }

    object_reset( pObj );
}

const struct olc_help_type help_table[] =
{
    {	"area",		area_flags,	 "Area attributes."		 },
    {	"room",		room_flags,	 "Room attributes."		 },
    {	"sector",	sector_type,	 "Sector types, terrain."	 },
    {	"exit",		exit_flags,	 "Exit types."			 },
    {	"type",		type_flags,	 "Types of objects."		 },
    {	"extra",	extra_flags,	 "Object attributes."		 },
    {	"wear",		wear_flags,	 "Where to wear object."	 },
    {	"spec",		spec_table,	 "Available special programs." 	 },
    {	"sex",		sex_flags,	 "Sexes."			 },
    {	"act",		act_flags,	 "Mobile attributes."		 },
    {	"affect",	affect_flags,	 "Mobile affects."		 },
    {   "shield",       shield_flags,    "Mobile shields."               },
    {	"wear-loc",	wear_loc_flags,	 "Where mobile wears object."	 },
    {	"container",	container_flags, "Container status."		 },
    {	"armor",	ac_type,	 "Ac for different attacks."	 },
    {   "apply",	apply_flags,	 "Apply flags"			 },
    {	"part",		part_flags,	 "Mobile body parts."		 },
    {	"size",		size_flags,	 "Mobile size."			 },
    {   "position",     position_flags,  "Mobile positions."             },
    {   "wclass",       weapon_class,    "Weapon class."                 }, 
    {   "wtype",        weapon_type2,    "Special weapon type."          },
    {	"portal",	portal_flags,	 "Portal types."		 },
    {	"furniture",	furniture_flags, "Furniture types."		 },
    {   "liquid",	liq_table,	 "Liquid types."		 },
    {	"apptype",	apply_types,	 "Apply types."			 },
    {	"weapon",	attack_table,	 "Weapon types."		 },
    {	"mprog",	mprog_flags,	 "MobProgram flags."		 },
    {	"oprog",	oprog_flags,	 "ObjProgram flags."		 },
    {	"rprog",	rprog_flags,	 "RoomProgram flags."		 },
    {	"trap",		trap_type_table, "Trap Type Flags."		 },
    {	"skills",	skill_flags,	 "Skill Table Flags."		 },
    {	NULL,		NULL,		 NULL				 }
};

void show_flag_cmds( CHAR_DATA *ch, const struct flag_type *flag_table )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  flag;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
	if ( flag_table[flag].settable )
	{
	    sprintf( buf, "%-19.18s", flag_table[flag].name );
	    strcat( buf1, buf );
	    if ( ++col % 4 == 0 )
		strcat( buf1, "\n\r" );
	}
    }
 
    if ( col % 4 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return;
}

void show_spec_cmds( CHAR_DATA *ch )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  spec;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    send_to_char( "Preceed special functions with 'spec_'\n\r\n\r", ch );
    for (spec = 0; spec_table[spec].function != NULL; spec++)
    {
	sprintf( buf, "%-19.18s", &spec_table[spec].name[5] );
	strcat( buf1, buf );
	if ( ++col % 4 == 0 )
	    strcat( buf1, "\n\r" );
    }
 
    if ( col % 4 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return;
}

void show_damlist( CHAR_DATA *ch, char *argument )
{
    BUFFER *final = new_buf( );
    char buf[MAX_STRING_LENGTH];
    int att, dt = DAM_ALL, pos = 0, value;

    if ( argument[0] == '\0' || !str_cmp( argument, "all" ) )
	value = -1;

    else if ( ( value = dam_type_lookup( argument ) ) == -1 )
    {
	sprintf( buf, "Invalid damage type.\nUse one of the following:\n all \n " );
	for ( value = 0; value < DAM_MAX; value++ )
	{
	    strcat( buf, damage_mod_table[value].name );
	    strcat( buf, "\n " );
	}

	send_to_char( buf, ch );
	return;
    }

    for ( att = 1; attack_table[att].name != NULL; att++ )
    {
	if ( value != -1 && value != attack_table[att].damage )
	    continue;

	if ( attack_table[att].damage != dt )
	{
	    if ( pos != 0 )
		add_buf( final, "\n\r\n\r" );

	    sprintf( buf, "{C%s damage nouns{c:{w",
		capitalize( damage_mod_table[attack_table[att].damage].name ) );
	    add_buf( final, buf );
	    pos = 0;
	}

	if ( pos % 5 == 0 )
	    add_buf( final, "\n\r" );

	sprintf( buf, " %-14s", attack_table[att].name );
	add_buf( final, buf );
	pos++;
	dt = attack_table[att].damage;
    }

    add_buf( final, "{x\n\r" );
    page_to_char( final->string, ch );
    free_buf( final );
}

void show_liqlist(CHAR_DATA *ch)
{
    int liq;
    char buf[MAX_STRING_LENGTH];
    
    send_to_char("Name                 Color          Proof Full Thirst Food Ssize\n\r",ch);
    send_to_char("----------------------------------------------------------------\n\r",ch);

    for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
	sprintf(buf, "%-20s %-14s %5d %4d %6d %4d %5d\n\r",
		liq_table[liq].liq_name,liq_table[liq].liq_color,
		liq_table[liq].liq_affect[0],liq_table[liq].liq_affect[1],
		liq_table[liq].liq_affect[2],liq_table[liq].liq_affect[3],
		liq_table[liq].liq_affect[4] );
	send_to_char(buf,ch);
    }
}

void find_resets( BUFFER *output, int type, int vnum )
{
    BUFFER *final = new_buf( );
    MOB_INDEX_DATA *pMob;
    OBJ_INDEX_DATA *pObj;
    RESET_DATA *pReset;
    ROOM_INDEX_DATA *pRoom;
    char buf[MAX_INPUT_LENGTH];
    sh_int iHash, found = 0;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
	{
	    pMob = NULL;

	    for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
	    {
		switch ( pReset->command )
		{
		    default:
			break;

		    case 'M':
			pMob = get_mob_index( pReset->arg1 );
			if ( type == ED_MOBILE && vnum == pReset->arg1 )
			{
			    sprintf( buf, "{t%2d{s) {qRoom {s[{t%5d{s] {t%s {s[{qW {s= {t%2d{s] [{qR {s= {t%2d{s]\n\r",
				++found, pRoom->vnum, end_string( pRoom->name, 45 ),
				pReset->arg2, pReset->arg4 );
			    add_buf( final, buf );
			}
			break;

		    case 'O':
			if ( type == ED_OBJECT && pReset->arg1 == vnum )
			{
			    sprintf( buf, "{t%2d{s) {qRoom {s[{t%5d{s] {t%s\n\r",
				++found, pRoom->vnum, pRoom->name );
			    add_buf( final, buf );
			}
			break;

		    case 'E':
		    case 'G':
			if ( type == ED_OBJECT && pReset->arg1 == vnum )
			{
			    if ( pMob == NULL )
				sprintf( buf, "{t%2d{s) {qRoom {s[{t%5d{s] {t%s {qMob {s[{t00000{s] {tNULL Mobile!\n\r",
				    ++found, pRoom->vnum, end_string( pRoom->name, 30 ) );
			    else
				sprintf( buf, "{t%2d{s) {qRoom {s[{t%5d{s] {t%s {qMob {s[{t%5d{s] {t%s\n\r",
				    ++found, pRoom->vnum, end_string( pRoom->name, 30 ),
				    pMob->vnum, pMob->short_descr );
			    add_buf( final, buf );
			}
			break;

		    case 'P':
			if ( type == ED_OBJECT && pReset->arg1 == vnum )
			{
			    if ( ( pObj = get_obj_index( pReset->arg3 ) ) == NULL )
				sprintf( buf, "{t%2d{s) {qRoom {s[{t%5d{s] {t%s {qObj {s[{t00000{s] {tNULL OBJ\n\r",
				    ++found, pRoom->vnum, end_string( pRoom->name, 30 ) );
			    else
				sprintf( buf, "{t%2d{s) {qRoom {s[{t%5d{s] {t%s {qObj {s[{t%5d{s] {t%s\n\r",
				    ++found, pRoom->vnum, end_string( pRoom->name, 30 ),
				    pObj->vnum, pObj->short_descr );
			    add_buf( final, buf );
			}
			break;
		}
	    }
	}
    }

    if ( found != 0 )
    {
	add_buf( output, "{s------------------------------------[{qRESETS{s]------------------------------------\n\r" );
	add_buf( output, final->string );
    }

    free_buf( final );
}

bool show_help( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char spell[MAX_INPUT_LENGTH];
    int cnt;

    argument = one_argument( argument, arg );
    one_argument( argument, spell );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax:  ? [command]\n\r\n\r", ch );
	send_to_char( "[command]  [description]\n\r", ch );
	for (cnt = 0; help_table[cnt].command != NULL; cnt++)
	{
	    sprintf( buf, "%-10.10s -%s\n\r",
	        capitalize( help_table[cnt].command ),
		help_table[cnt].desc );
	    send_to_char( buf, ch );
	}
	return FALSE;
    }

    for (cnt = 0; help_table[cnt].command != NULL; cnt++)
    {
        if (  arg[0] == help_table[cnt].command[0]
          && !str_prefix( arg, help_table[cnt].command ) )
	{
	    if ( help_table[cnt].structure == spec_table )
	    {
		show_spec_cmds( ch );
		return FALSE;
	    }
	    else
	    if ( help_table[cnt].structure == liq_table )
	    {
	        show_liqlist( ch );
	        return FALSE;
	    }
	    else
	    if ( help_table[cnt].structure == attack_table )
	    {
	        show_damlist( ch, argument );
	        return FALSE;
	    }
	    else
	    {
		show_flag_cmds( ch, help_table[cnt].structure );
		return FALSE;
	    }
	}
    }

    show_help( ch, "" );
    return FALSE;
}

REDIT( redit_mshow )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  mshow <vnum>\n\r", ch );
	return FALSE;
    }

    if ( !is_number( argument ) )
    {
       send_to_char( "REdit: Vnum is not a number.\n\r", ch);
       return FALSE;
    }

    if ( is_number( argument ) )
    {
	value = atoi( argument );

	if ( !( pMob = get_mob_index( value ) ))
	{
	    send_to_char( "REdit:  That mobile does not exist.\n\r", ch );
	    return FALSE;
	}

	ch->desc->pEdit = (void *)pMob;
    }
 
    medit_show( ch, argument );
    ch->desc->pEdit = (void *)ch->in_room;
    return FALSE; 
}

REDIT( redit_oshow )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  oshow <vnum>\n\r", ch );
	return FALSE;
    }

    if ( !is_number( argument ) )
    {
       send_to_char( "REdit: Vnum is not a number.\n\r", ch);
       return FALSE;
    }

    if ( is_number( argument ) )
    {
	value = atoi( argument );
	if ( !( pObj = get_obj_index( value ) ))
	{
	    send_to_char( "REdit:  That object does not exist.\n\r", ch );
	    return FALSE;
	}

	ch->desc->pEdit = (void *)pObj;
    }
 
    oedit_show( ch, argument );
    ch->desc->pEdit = (void *)ch->in_room;
    return FALSE; 
}

bool check_range( int lower, int upper )
{
    AREA_DATA *pArea;
    int cnt = 0;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        if ( ( lower <= pArea->min_vnum && pArea->min_vnum <= upper )
	||   ( lower <= pArea->max_vnum && pArea->max_vnum <= upper ) )
	    ++cnt;

	if ( cnt > 1 )
	    return FALSE;
    }
    return TRUE;
}

AREA_DATA *get_vnum_area( int vnum )
{
    AREA_DATA *pArea;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        if ( vnum >= pArea->min_vnum
          && vnum <= pArea->max_vnum )
            return pArea;
    }

    return 0;
}

bool aedit_show( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    BUFFER *final = new_buf( );
    char buf[MAX_STRING_LENGTH];
    sh_int count_mob = 0, count_obj = 0, count_room = 0;
    int vnum;

    EDIT_AREA( ch, pArea );

    sprintf( buf, "{qName:          {s[{t%5d{s] [{t%s{s]\n\r",
	pArea->vnum, pArea->name );
    add_buf( final, buf );

    sprintf( buf, "{s                       [{t%s{s]\n\r", double_color( pArea->name ) );
    add_buf( final, buf );

    sprintf( buf, "{qFile:          {s[{t%s{s]\n\r", pArea->file_name );
    add_buf( final, buf );

    sprintf( buf, "{qVnums:         {s[{t%d {sto {t%d{s]\n\r",
	pArea->min_vnum, pArea->max_vnum );
    add_buf( final, buf );

    sprintf( buf, "{qAge:           {s[{t%d{s]\n\r", pArea->age );
    add_buf( final, buf );

    sprintf( buf, "{qPlayers:       {s[{t%d{s]\n\r", pArea->nplayer );
    add_buf( final, buf );

    sprintf( buf, "{qSecurity:      {s[{t%d{s]\n\r", pArea->security );
    add_buf( final, buf );

    sprintf( buf, "{qBuilder:       {s[{t%s{s]\n\r",
	pArea->builder ? pArea->builder : "Unknown" );
    add_buf( final, buf );

    sprintf( buf, "{qDirections:    {s[{t%s{s]\n\r",
	pArea->directions ? pArea->directions : "unknown" );
    add_buf( final, buf );

    sprintf( buf, "{qAlignment:     {s[{t%c{s]\n\r", pArea->alignment );
    add_buf( final, buf );

    sprintf( buf, "{qLevels:        {s[{t%d {sto {t%d{s]\n\r",
	pArea->min_level, pArea->max_level );
    add_buf( final, buf );

    sprintf( buf, "{qClan:          {s[{t%d{s] [{t%s{s]\n\r",
	pArea->clan, clan_table[pArea->clan].color );
    add_buf( final, buf );

    sprintf( buf, "{qFlags:         {s[{t%s{s]\n\r",
	flag_string( area_flags, pArea->area_flags ) );
    add_buf( final, buf );

    if ( pArea->run_vnum != 0 )
    {
	sprintf( buf, "{qRun Vnum:      {s[{t%d{s]\n\r", pArea->run_vnum );
	add_buf( final, buf );
    }

    sprintf( buf, "{qMusic File:    {s[{t%s{s]{x\n\r",
	pArea->music ? pArea->music : "none" );
    add_buf( final, buf );

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( get_mob_index( vnum ) != NULL )
	    count_mob++;

	if ( get_obj_index( vnum ) != NULL )
	    count_obj++;

	if ( get_room_index( vnum ) != NULL )
	    count_room++;
    }

    sprintf( buf, "\n\r{qMobiles Found: {s[{t%d{s]\n\r", count_mob );
    add_buf( final, buf );

    sprintf( buf, "{qObjects Found: {s[{t%d{s]\n\r", count_obj );
    add_buf( final, buf );

    sprintf( buf, "{qRooms Found:   {s[{t%d{s]{x\n\r", count_room );
    add_buf( final, buf );

    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

bool aedit_music( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:   music <file name>\n\r", ch );
	send_to_char( "Syntax:   music none (To remove the file)\n\r",ch);
	return FALSE;
    }

    if ( !str_cmp( argument, "remove" ) || !str_cmp( argument, "none" ) )
    {
	free_string( pArea->music );
	pArea->music = NULL;
	send_to_char("Music file detached.\n\r",ch);
	return TRUE;
    }
	
    free_string( pArea->music );
    pArea->music = str_dup( argument );

    send_to_char( "Background music file set.\n\r", ch );
    send_to_char( "Please refer to your builders guide when using this option.\n\r", ch );
    return TRUE;
}

bool aedit_exit_scan( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
    int pos, vnum, match = 0;

    EDIT_AREA( ch, pArea );

    for ( vnum = 0; match < top_room; vnum++ )
    {
	if ( ( pRoom = get_room_index( vnum ) ) != NULL )
	{
	    match++;

	    for ( pos = 0; pos < MAX_DIR; pos++ )
	    {
		if ( pRoom->area == pArea
		&&   pRoom->exit[pos] != NULL
		&&   pRoom->exit[pos]->u1.to_room->area != pArea )
		{
		    found = TRUE;
		    sprintf( buf, "Exits: [%5d] [%s] %s to [%5d] [%s]\n",
			pRoom->vnum,
			end_string( pRoom->name, 20 ),
			pos == 0 ? "NORTH" : pos == 1 ? " EAST" :
			pos == 2 ? "SOUTH" : pos == 3 ? " WEST" :
			pos == 4 ? "   UP" : " DOWN",
			pRoom->exit[pos]->u1.to_room->vnum,
			pRoom->exit[pos]->u1.to_room->name ); 
		    send_to_char( buf, ch );
		}

		if ( pRoom->area != pArea
		&&   pRoom->exit[pos] != NULL
		&&   pRoom->exit[pos]->u1.to_room->area == pArea )
		{
		    found = TRUE;
		    sprintf( buf, "Enter: [%5d] [%s] %s to [%5d] [%s]\n",
			pRoom->vnum,
			end_string( pRoom->name, 20 ),
			pos == 0 ? "NORTH" : pos == 1 ? " EAST" :
			pos == 2 ? "SOUTH" : pos == 3 ? " WEST" :
			pos == 4 ? "   UP" : " DOWN",
			pRoom->exit[pos]->u1.to_room->vnum,
			pRoom->exit[pos]->u1.to_room->name ); 
		    send_to_char( buf, ch );
		}
	    }
	}
    }

    if ( !found )
	send_to_char( "No external exits found.\n\r", ch );

    return FALSE;
}

bool aedit_balance( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMob;
    OBJ_INDEX_DATA *pObj;
    char buf[MAX_STRING_LENGTH];
    sh_int level, type;
    int vnum;

    EDIT_AREA(ch,pArea);

    if ( argument[0] == '\0' )
    {
	send_to_char("Valid balance arguments are 'easy', 'medium', or 'hard'.\n\r",ch);
	return FALSE;
    }

    if ( !str_prefix(argument,"easy") )
	type = 1;
    else if ( !str_prefix(argument,"medium") )
	type = 2;
    else if ( !str_prefix(argument,"hard") )
	type = 3;
    else
    {
	send_to_char("Valid balance arguments are 'easy', 'medium', or 'hard'.\n\r",ch);
	return FALSE;
    }

    for ( vnum = pArea->min_vnum; vnum < pArea->max_vnum; vnum++ )
    {
	if ( (pObj = get_obj_index(vnum)) != NULL )
	{
	    if ( (level = pObj->level) <= 0 || level > LEVEL_HERO )
	    {
		sprintf(buf,"Skipping object with invalid level of %d, vnum: %d.\n\r",
		    level, vnum);
		send_to_char(buf,ch);
	    } else {
		object_balance( pObj, level );
	    }
	}

	if ( (pMob = get_mob_index(vnum)) != NULL )
	{
	    if ( pMob->level <= 0 || pMob->level > LEVEL_HERO*2 )
	    {
		sprintf(buf,"Skipping mobile with invalid level of %d, vnum: %d.\n\r",
		    pMob->level, vnum );
		send_to_char(buf,ch);
	    } else {
		mobile_balance( pMob, type );
	    }
	}
    }

    send_to_char("Area balanced.\n\r",ch);

    return TRUE;
}

bool aedit_flags( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    int value;

    EDIT_AREA( ch, pArea );

    if ( ( value = flag_value( area_flags, argument ) ) == NO_FLAG )
    {
	send_to_char( "Syntax: flag [flags]\n\r", ch );
	send_to_char( "For a List '? area'\n\r",ch);
	return FALSE;
    }

    TOGGLE_BIT(pArea->area_flags, value);
    send_to_char( "Area flag toggled.\n\r", ch );
    return TRUE;
}

bool aedit_clan( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    sh_int clan;

    EDIT_AREA( ch, pArea );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: clan <clan name>\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( argument, "none" ) )
	clan = 0;

    else if ( ( clan = clan_lookup( argument ) ) == 0 )
    {
	send_to_char( "Invalid clan.\n\r", ch );
	return FALSE;
    }

    pArea->clan = clan;

    send_to_char( "Clan set.\n\r", ch );

    return TRUE;
}

bool aedit_reset( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    CHAR_DATA *wch, *wch_next;
    OBJ_DATA *obj, *obj_next;
    ROOM_INDEX_DATA *pRoom;
    int vnum;

    EDIT_AREA( ch, pArea );

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pRoom = get_room_index( vnum ) ) != NULL )
	{
	    for ( wch = pRoom->people; wch != NULL; wch = wch_next )
	    {
		wch_next = wch->next_in_room;

		if ( wch->pcdata == NULL && wch->fighting == NULL && wch != ch )
		    extract_char( wch, TRUE );
	    }

	    for ( obj = pRoom->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;

		if ( !IS_OBJ_STAT( obj, ITEM_AQUEST )
		&&   !IS_OBJ_STAT( obj, ITEM_FORGED )
		&&   !IS_OBJ_STAT( obj, ITEM_SPECIAL_SAVE )
		&&   obj->disarmed_from == NULL
		&&   obj->item_type != ITEM_CORPSE_PC )
		    extract_obj( obj );
	    }
		
	    reset_room( pRoom, NULL );
	}
    }

    send_to_char( "Area reset.\n\r", ch );

    return FALSE;
}

bool aedit_flag_rooms( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int vnum, value;

    if ( ( value = flag_value( room_flags, argument ) ) == NO_FLAG )
    {
	send_to_char( "Syntax: flagrooms [flags]\n\r", ch );
	send_to_char( "For a List '? room'\n\r",ch);
	return FALSE;
    }

    EDIT_AREA( ch, pArea );

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pRoom = get_room_index( vnum ) ) != NULL )
	    SET_BIT( pRoom->room_flags, value );
    }

    send_to_char( "All rooms flagged.\n\r", ch );

    return TRUE;
}

bool aedit_unflag_rooms( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int vnum, value;

    if ( ( value = flag_value( room_flags, argument ) ) == NO_FLAG )
    {
	send_to_char( "Syntax: unflagrooms [flags]\n\r", ch );
	send_to_char( "For a List '? room'\n\r",ch);
	return FALSE;
    }

    EDIT_AREA( ch, pArea );

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pRoom = get_room_index( vnum ) ) != NULL )
	    REMOVE_BIT( pRoom->room_flags, value );
    }

    send_to_char( "All rooms unflagged.\n\r", ch );

    return TRUE;
}

bool aedit_delete( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea, *prev;
    AUCTION_DATA *auc;
    CHAR_DATA *wch, *wch_next;
    ROOM_INDEX_DATA *pRoom, *pRoom2;
    OBJ_DATA *obj, *obj_next;
    OBJ_INDEX_DATA *pObj, *pObj2;
    MOB_INDEX_DATA *pMob, *pMob2;
    PROG_CODE *pMcode;
    PROG_LIST *mp, *mp_next;
    char buf[MAX_STRING_LENGTH];
    sh_int count_mob = 0, count_obj = 0, count_room = 0;
    sh_int count_mprog = 0, count_oprog = 0, count_rprog = 0;
    int iHash, match = 0, pos, vnum;

    EDIT_AREA( ch, pArea );

    if ( argument[0] == '\0' || str_cmp( argument, pArea->name ) )
    {
	send_to_char( "Argument must match the area name you are editing.\n\r", ch );
	return FALSE;
    }

    if ( pArea->security == 9 )
    {
	send_to_char( "Areas with security level 9 are not allowed to be deleted.\n\r"
		      "These areas are vital for the game to run correctly.\n\r", ch );
	return FALSE;
    }

    if ( ( pRoom = get_room_index( ROOM_VNUM_ALTAR ) ) == NULL )
    {
	send_to_char( "ERROR: ROOM_VNUM_ALTAR not found!\n\r", ch );
	return FALSE;
    }

    check_olc_delete( ch );

    save_area( pArea, TRUE );

    for ( wch = char_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;

	if ( ( wch->in_room && wch->in_room->area == pArea )
	||   ( wch->pIndexData && wch->pIndexData->area == pArea ) )
	{
	    send_to_char( "Your area was just deleted!\n\r", wch );

	    if ( IS_NPC( wch ) )
		extract_char( wch, TRUE );
	    else
	    {
		char_from_room( wch );
		char_to_room( wch, pRoom );
	    }
	}
    }

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next;

	if ( obj->pIndexData->area == pArea )
	{
	    for ( auc = auction_list; auc != NULL; auc = auc->next )
	    {
		if ( auc->item != NULL && obj == auc->item )
		{
		    if ( auc->high_bidder != NULL )
		    {
			add_cost( auc->high_bidder, auc->bid_amount, auc->bid_type );
			send_to_char( "\n\rYour bid has been returned to you.\n\r", auc->high_bidder );
		    }

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
			OBJ_DATA *prev_obj;

			for ( prev_obj = wch->pcdata->storage_list[pos]; prev_obj != NULL; prev_obj = prev_obj->next_content )
			{
			    if ( prev_obj->next_content == obj )
			    {
				prev_obj->next_content = obj->next_content;
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
    }

    for ( vnum = 0; match < top_room; vnum++ )
    {
	if ( ( pRoom = get_room_index( vnum ) ) != NULL )
	{
	    match++;

	    if ( pRoom->area == pArea )
		continue;

	    for ( pos = 0; pos < MAX_DIR; pos++ )
	    {
		if ( pRoom->exit[pos] != NULL
		&&   pRoom->exit[pos]->u1.to_room->area == pArea )
		{
		    free_exit( pRoom->exit[pos] );
		    pRoom->exit[pos] = NULL;
		    SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
		}
	    }
	}
    }

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	iHash = vnum % MAX_KEY_HASH;

	if ( ( pMob = get_mob_index( vnum ) ) != NULL )
	{
	    pMob2 = mob_index_hash[iHash];

	    if ( pMob2->next == NULL )
		mob_index_hash[iHash] = NULL;
	    else if ( pMob2 == pMob )
		mob_index_hash[iHash] = pMob->next;
	    else
	    {
		for ( ; pMob2 != NULL; pMob2 = pMob2->next )
		{
		    if ( pMob2->next == pMob )
		    {
			pMob2->next = pMob->next;
			break;
		    }
		}
	    }

	    count_mob++;
	    free_mob_index( pMob );
	}

	if ( ( pObj = get_obj_index( vnum ) ) != NULL )
	{
	    pObj2 = obj_index_hash[iHash];

	    if ( pObj2->next == NULL )
		obj_index_hash[iHash] = NULL;

	    else if ( pObj2 == pObj )
		obj_index_hash[iHash] = pObj->next;

	    else
	    {
		for ( ; pObj2 != NULL; pObj2 = pObj2->next )
		{
		    if ( pObj2->next == pObj )
		    {
			pObj2->next = pObj->next;
			break;
		    }
		}
	    }

	    count_obj++;
	    free_obj_index( pObj );
	}

	if ( ( pRoom = get_room_index( vnum ) ) != NULL )
	{
	    pRoom2 = room_index_hash[iHash];

	    if ( pRoom2->next == NULL )
		room_index_hash[iHash] = NULL;

	    else if ( pRoom2 == pRoom )
		room_index_hash[iHash] = pRoom->next;

	    else
	    {
		for ( ; pRoom2 != NULL; pRoom2 = pRoom2->next )
		{
		    if ( pRoom2->next == pRoom )
		    {
			pRoom2->next = pRoom->next;
			break;
		    }
		}
	    }

	    count_room++;
	    free_room_index( pRoom );
	}

	if ( ( pMcode = get_prog_index( vnum, PRG_MPROG ) ) != NULL )
	{
	    int new_vnum;
	    match = 0;

	    for ( new_vnum = 0; match < top_mob_index; new_vnum++ )
	    {
		if ( ( pMob = get_mob_index( new_vnum ) ) != NULL )
		{
		    match++;
		    for ( mp = pMob->mprogs; mp != NULL; mp = mp_next )
		    {
			mp_next = mp->next;

			if ( mp->vnum == pMcode->vnum )
			{
			    if ( mp == pMob->mprogs )
				pMob->mprogs = pMob->mprogs->next;
			    else
			    {
				PROG_LIST *prev;

				for ( prev = pMob->mprogs; prev != NULL; prev = prev->next )
				{
				    if ( prev->next == mp )
				    {
					prev->next = mp->next;
					break;
				    }
				}
			    }

			    free_prog( mp );
			}
		    }
		}
	    }

	    if ( pMcode == mprog_list )
		mprog_list = pMcode->next;
	    else
	    {
		PROG_CODE *prev;

		for ( prev = mprog_list; prev != NULL; prev = prev->next )
		{
		    if ( prev->next == pMcode )
		    {
			prev->next = pMcode->next;
			break;
		    }
		}
	    }

	    count_mprog++;
	    free_pcode( pMcode );
	}

	if ( ( pMcode = get_prog_index( vnum, PRG_OPROG ) ) != NULL )
	{
	    int new_vnum;
	    match = 0;

	    for ( new_vnum = 0; match < top_obj_index; new_vnum++ )
	    {
		if ( ( pObj = get_obj_index( new_vnum ) ) != NULL )
		{
		    match++;
		    for ( mp = pObj->oprogs; mp != NULL; mp = mp_next )
		    {
			mp_next = mp->next;

			if ( mp->vnum == pMcode->vnum )
			{
			    if ( mp == pObj->oprogs )
				pObj->oprogs = pObj->oprogs->next;
			    else
			    {
				PROG_LIST *prev;

				for ( prev = pObj->oprogs; prev != NULL; prev = prev->next )
				{
				    if ( prev->next == mp )
				    {
					prev->next = mp->next;
					break;
				    }
				}
			    }

			    free_prog( mp );
			}
		    }
		}
	    }

	    if ( pMcode == oprog_list )
		oprog_list = pMcode->next;
	    else
	    {
		PROG_CODE *prev;

		for ( prev = oprog_list; prev != NULL; prev = prev->next )
		{
		    if ( prev->next == pMcode )
		    {
			prev->next = pMcode->next;
			break;
		    }
		}
	    }

	    count_oprog++;
	    free_pcode( pMcode );
	}

	if ( ( pMcode = get_prog_index( vnum, PRG_RPROG ) ) != NULL )
	{
	    int new_vnum;
	    match = 0;

	    for ( new_vnum = 0; match < top_room; new_vnum++ )
	    {
		if ( ( pRoom = get_room_index( new_vnum ) ) != NULL )
		{
		    match++;
		    for ( mp = pRoom->rprogs; mp != NULL; mp = mp_next )
		    {
			mp_next = mp->next;

			if ( mp->vnum == pMcode->vnum )
			{
			    if ( mp == pRoom->rprogs )
				pRoom->rprogs = pRoom->rprogs->next;
			    else
			    {
				PROG_LIST *prev;

				for ( prev = pRoom->rprogs; prev != NULL; prev = prev->next )
				{
				    if ( prev->next == mp )
				    {
					prev->next = mp->next;
					break;
				    }
				}
			    }

			    free_prog( mp );
			}
		    }
		}
	    }

	    if ( pMcode == rprog_list )
		rprog_list = pMcode->next;
	    else
	    {
		PROG_CODE *prev;

		for ( prev = rprog_list; prev != NULL; prev = prev->next )
		{
		    if ( prev->next == pMcode )
		    {
			prev->next = pMcode->next;
			break;
		    }
		}
	    }

	    count_rprog++;
	    free_pcode( pMcode );
	}
    }

    if ( pArea == area_first )
	area_first = pArea->next;
    else
    {
	for ( prev = area_first; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == pArea )
	    {
		prev->next = pArea->next;
		break;
	    }
	}
    }

    if ( area_last == pArea )
    {
	for ( prev = area_first; prev->next != NULL; prev = prev->next );
	area_last = prev;
    }
    
    for ( prev = area_first; prev != NULL; prev = prev->next )
    {
	if ( prev != pArea && prev->vnum > pArea->vnum )
	    prev->vnum--;
    }

    sprintf( buf, "Deleted 1 area, %d mobile%s, %d object%s, %d room%s, %d mob program%s, %d obj program%s and %d room program%s.\n\r",
	count_mob, count_mob == 1 ? "" : "s",
	count_obj, count_obj == 1 ? "" : "s",
	count_room, count_room == 1 ? "" : "s",
	count_mprog, count_mprog == 1 ? "" : "s",
	count_oprog, count_oprog == 1 ? "" : "s",
	count_rprog, count_rprog == 1 ? "" : "s" );
    send_to_char( buf, ch );

    unlink( pArea->file_name );

    free_area( pArea );
    save_area_list( );

    return TRUE;
}

bool aedit_create( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    pArea		= new_area( );
    area_last->next	= pArea;
    area_last		= pArea;
    ch->desc->pEdit	= (void *)pArea;
    ch->desc->editor	= ED_AREA;

    SET_BIT( pArea->area_flags, AREA_UNLINKED );

    save_area( pArea, FALSE );

    send_to_char( "Area Created.\n\r", ch );
    return FALSE;
}

bool aedit_name( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:   name [$name]\n\r", ch );
	return FALSE;
    }

    free_string( pArea->name );
    pArea->name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch );
    return TRUE;
}

bool aedit_builder( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    EDIT_AREA( ch, pArea );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:   builder [credited builder]\n\r", ch );
	return FALSE;
    }

    free_string( pArea->builder );
    pArea->builder = str_dup( argument );

    send_to_char( "Building credits set.\n\r", ch );
    return TRUE;
}

bool aedit_directions( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char *path;

    EDIT_AREA( ch, pArea );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:   directions [run string]\n\r", ch );
	return FALSE;
    }

    for ( path = argument; *path != '\0'; ++*path )
    {
	if ( !isdigit( *path ) )
	{
	    switch( *path )
	    {
		default:
		    send_to_char( "Invalid directions.\n\r", ch );
		    return FALSE;

		case 'n':
		case 's':
		case 'w':
		case 'e':
		case 'u':
		case 'd':
		    break;
	    }
	}
    }

    free_string( pArea->directions );
    pArea->directions = str_dup( argument );

    send_to_char( "Directions set.\n\r", ch );
    return TRUE;
}

bool aedit_file( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    FILE *fp;
    char buf[MAX_INPUT_LENGTH], file[MAX_INPUT_LENGTH];

    EDIT_AREA( ch, pArea );

    one_argument( argument, file );

    if ( file[0] == '\0' )
    {
	send_to_char( "Syntax:  filename [$file]\n\r", ch );
	return FALSE;
    }

    if ( strlen( file ) > 12 )
    {
	send_to_char( "No more than twelve characters allowed.\n\r", ch );
	return FALSE;
    }
    
    if ( !check_argument( ch, file, FALSE ) )
	return FALSE;

    sprintf( buf, "%s.are", file );

    if ( ( fp = fopen( buf, "r" ) ) != NULL )
    {
	fclose( fp );
	sprintf( buf, "%s appears to be in use already!\n\r", buf );
	send_to_char( buf, ch );
	return FALSE;
    }

    strcpy( buf, pArea->file_name );

    free_string( pArea->file_name );
    strcat( file, ".are" );
    pArea->file_name = str_dup( file );
    save_area( pArea, FALSE );

    unlink( buf );

    send_to_char( "Filename set.\n\r", ch );

    return TRUE;
}

bool aedit_alignment( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    EDIT_AREA( ch, pArea );

    switch( UPPER( *argument ) )
    {
	default:
	    send_to_char( "Syntax:  alignment < G | N | E >.\n\r", ch );
	    return FALSE;

	case 'E': 	pArea->alignment = 'E';		break;
	case 'G':	pArea->alignment = 'G';		break;
	case 'N':	pArea->alignment = 'N';		break;
    }

    return TRUE;
}

bool aedit_run_vnum( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    int  value;

    EDIT_AREA(ch, pArea);

    if ( !is_number( argument ) || argument[0] == '\0' )
    {
	send_to_char( "Syntax:  run_vnum [run_vnum room]\n\r", ch );
	return FALSE;
    }

    value = atoi( argument );

    if ( value > pArea->max_vnum || value < pArea->min_vnum )
    {
	send_to_char( "Beginning of area must be iside the assigned vnums.\n\r",ch);
	return FALSE;
    }

    pArea->run_vnum = value;

    send_to_char( "Run_vnum room set.\n\r", ch );
    return TRUE;
}

bool aedit_security( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char sec[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, sec );

    if ( !is_number( sec ) || sec[0] == '\0' )
    {
	send_to_char( "Syntax:  security [#xlevel]\n\r", ch );
	return FALSE;
    }

    value = atoi( sec );

    if ( value > ch->pcdata->security || value < 0 )
    {
	if ( ch->pcdata->security != 0 )
	{
	    sprintf( buf, "Security is 0-%d.\n\r", ch->pcdata->security );
	    send_to_char( buf, ch );
	}
	else
	    send_to_char( "Security is 0 only.\n\r", ch );
	return FALSE;
    }

    pArea->security = value;

    send_to_char( "Security set.\n\r", ch );
    return TRUE;
}

bool aedit_levels( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char arg[MAX_INPUT_LENGTH];
    sh_int min, max;

    EDIT_AREA( ch, pArea );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0'
    ||   !is_number( arg ) || !is_number( argument ) )
    {
	send_to_char( "Syntax:  levels [lower] [upper]\n\r", ch );
	return FALSE;
    }

    min = atoi( arg );
    max = atoi( argument );

    if ( min < 0 || min > MAX_LEVEL || max < 0 || max > MAX_LEVEL )
    {
	send_to_char( "Valid levels are 0 to MAX_LEVEL.\n\r", ch );
	return FALSE;
    }

    if ( min > max )
    {
	send_to_char( "Max level can not be less that min level.\n\r", ch );
	return FALSE;
    }
    
    pArea->min_level = min;
    pArea->max_level = max;

    send_to_char( "Level range set.\n\r", ch );

    return TRUE;
}

bool aedit_vnum( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    argument = one_argument( argument, lower );
    one_argument( argument, upper );

    if ( !is_number( lower ) || lower[0] == '\0'
    || !is_number( upper ) || upper[0] == '\0' )
    {
	send_to_char( "Syntax:  vnum [#xlower] [#xupper]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = atoi( upper ) ) )
    {
	send_to_char( "AEdit:  Upper must be larger then lower.\n\r", ch );
	return FALSE;
    }
    
    if ( !check_range( atoi( lower ), atoi( upper ) ) )
    {
	send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( ilower )
    && get_vnum_area( ilower ) != pArea )
    {
	send_to_char( "AEdit:  Lower vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->min_vnum = ilower;
    send_to_char( "Lower vnum set.\n\r", ch );

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
	send_to_char( "AEdit:  Upper vnum already assigned.\n\r", ch );
	return TRUE;	/* The lower value has been set. */
    }

    pArea->max_vnum = iupper;
    send_to_char( "Upper vnum set.\n\r", ch );

    return TRUE;
}

bool aedit_lvnum( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char lower[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, lower );

    if ( !is_number( lower ) || lower[0] == '\0' )
    {
	send_to_char( "Syntax:  min_vnum [#xlower]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = pArea->max_vnum ) )
    {
	send_to_char( "AEdit:  Value must be less than the max_vnum.\n\r", ch );
	return FALSE;
    }
    
    if ( !check_range( ilower, iupper ) )
    {
	send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( ilower )
    && get_vnum_area( ilower ) != pArea )
    {
	send_to_char( "AEdit:  Lower vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->min_vnum = ilower;
    send_to_char( "Lower vnum set.\n\r", ch );
    return TRUE;
}

bool aedit_uvnum( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char upper[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, upper );

    if ( !is_number( upper ) || upper[0] == '\0' )
    {
	send_to_char( "Syntax:  max_vnum [#xupper]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = pArea->min_vnum ) > ( iupper = atoi( upper ) ) )
    {
	send_to_char( "AEdit:  Upper must be larger then lower.\n\r", ch );
	return FALSE;
    }
    
    if ( !check_range( ilower, iupper ) )
    {
	send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
	send_to_char( "AEdit:  Upper vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->max_vnum = iupper;
    send_to_char( "Upper vnum set.\n\r", ch );

    return TRUE;
}

bool hedit_clan( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;

    EDIT_HELP( ch, pHelp );

    pHelp->clan = clan_lookup( argument );

    send_to_char( "Clan set.\n\r", ch );

    return TRUE;
}

bool hedit_color_show( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char buf[4*MAX_STRING_LENGTH];
    int count = 0;

    EDIT_HELP( ch, pHelp );

    for ( argument = pHelp->text; *argument != '\0'; argument++ )
    {
	if ( *argument == '{' )
	{
	    buf[count++] = *argument;
	    buf[count++] = '{';
	    argument++;
	}

	buf[count++] = *argument;

	if ( count >= ( MAX_STRING_LENGTH * 4 ) - 5 )
	    break;
    }

    buf[count] = '\0';

    page_to_char( buf, ch );

    return FALSE;
}

bool hedit_make( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;

    if (argument[0] == '\0')
    {
        send_to_char("Syntax: hedit make [keyword(s)]\n\r",ch);
        return FALSE;
    }

    pHelp		= new_help();

    free_string( pHelp->keyword );
    pHelp->keyword	= str_dup(argument);

    pHelp->next = help_first;
    help_first = pHelp;

    ch->desc->pEdit	= (void *)pHelp;
    ch->desc->editor	= ED_HELP;

    send_to_char("New Help Entry Created.\n\r",ch);
    return TRUE;
}

bool hedit_show( CHAR_DATA *ch, char *argument )
{
    BUFFER *final = new_buf( );
    HELP_DATA *pHelp;
    char buf[MAX_STRING_LENGTH];

    EDIT_HELP( ch, pHelp );

    sprintf( buf, "{qLevel:    {s[{t%d{s]\n\r"
		  "{qName:     {s[{t%s{s]\n\r"
                  "{qKeywords: {s[{t%s{s]\n\r"
		  "{qClan      {s[{t%s{s]\n\r\n\r{t",
	pHelp->level, pHelp->name, pHelp->keyword,
	clan_table[pHelp->clan].color );
    add_buf( final, buf );

    add_buf( final, pHelp->text );
    add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

bool hedit_desc( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;

    EDIT_HELP(ch, pHelp);

    if (argument[0] == '\0')
    {
	string_append(ch, &pHelp->text);
	return TRUE;
    }

    send_to_char(" Syntax: desc\n\r",ch);
    return FALSE;
}

bool hedit_keywords( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    EDIT_HELP(ch, pHelp);

    if ( argument[0] == '\0' )
    {
        send_to_char(" Syntax: keywords [keywords]\n\r",ch);
        return FALSE;
    }

    free_string( pHelp->keyword );
    pHelp->keyword = str_dup(argument);
    send_to_char( "Keyword(s) Set.\n\r", ch);
    return TRUE;
}

bool hedit_name( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    EDIT_HELP(ch, pHelp);

    if ( argument[0] == '\0' )
    {
        send_to_char(" Syntax: name [name]\n\r",ch);
        return FALSE;
    }

    free_string(pHelp->name);
    pHelp->name = str_dup(argument);
    send_to_char("Name set.\n\r",ch);
    return TRUE;
}

bool hedit_level( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    sh_int level;

    EDIT_HELP( ch, pHelp );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax:  level [number]\n\r", ch );
        return FALSE;
    }

    if ( ( level = atoi( argument ) ) < 0 || level > MAX_LEVEL )
    {
	send_to_char( "Invalid level.\n\r", ch );
	return FALSE;
    }

    pHelp->level = level;
    send_to_char( "Level set.\n\r", ch);

    return TRUE;
}

bool hedit_delete( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;

    EDIT_HELP( ch, pHelp );

    if ( argument[0] == '\0' || str_cmp( argument, "helpfile" ) )
    {
	send_to_char( "To confirm this type \"delete helpfile\".\n\r", ch );
	return FALSE;
    }

    check_olc_delete( ch );

    send_to_char("Help file deleted.\n\r",ch);

    if ( pHelp == help_first )
	help_first = help_first->next;
    else
    {
	HELP_DATA *pHelp2;

	for ( pHelp2 = help_first; pHelp2 != NULL; pHelp2 = pHelp2->next )
	{
	    if ( pHelp2->next == pHelp )
	    {
		pHelp2->next = pHelp->next;
		break;
	    }
	}
    }

    free_help(pHelp);

    return TRUE;
}

bool gredit_cost( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int i, lvl, sn;

    EDIT_TABLE( ch, sn );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: cost <class> <points>\n\r", ch );
	return FALSE;
    }

    if ( ( i = class_lookup( arg ) ) == -1 )
    {
	send_to_char( "Invalid class.\n\r", ch );
	return FALSE;
    }

    if ( !is_number( argument ) || ( lvl = atoi( argument ) ) < -1 )
    {
	send_to_char( "New points valid must be a valid intiger of -1 or more.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "Group Edit: (%s) Points value for %s changed from %d to %d.\n\r",
	group_table[sn].name, class_table[i].name, group_table[sn].rating[i], lvl );
    send_to_char( buf, ch );
    group_table[sn].rating[i] = lvl;

    mud_stat.classes_changed = TRUE;
    return FALSE;
}

bool gredit_create( CHAR_DATA *ch, char *argument )
{
    struct group_type *new_table;
    char buf[MAX_STRING_LENGTH];
    int sn;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Create which spell group?\n\r", ch );
	return FALSE;
    }

    if ( !check_argument( ch, argument, TRUE ) )
	return FALSE;

    if ( group_lookup( argument ) != -1 )
    {
	send_to_char( "That group already exists.\n\r", ch );
	return FALSE;
    }

    maxGroup++;
    new_table = realloc( group_table, sizeof( struct group_type ) * ( maxGroup + 1 ) );

    if ( !new_table )
    {
	send_to_char( "Memory allocation failed. Brace for impact.\n\r", ch );
	return FALSE;
    }

    group_table				= new_table;
    group_table[maxGroup-1].name	= str_dup( argument );
    group_table[maxGroup-1].rating	= new_short( maxClass, -1 );

    for ( sn = 0; sn < MAX_IN_GROUP; sn++ )
	group_table[maxGroup-1].spells[sn] = NULL;

    group_table[maxGroup].name = str_dup( "" );
    group_table[maxGroup].rating = NULL;

    sprintf( buf, "New group added: %s\n\r", argument );
    send_to_char( buf, ch );

    /* Re-Allocate memory for pcdata and gen_data */
    update_group_data( -1 );

    ch->desc->editor = ED_GROUP;
    ch->desc->pEdit = (void *)maxGroup-1;

    return TRUE;
}

bool gredit_delete( CHAR_DATA *ch, char *argument )
{
    struct group_type *new_table;
    int i, sn, lvl;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0'
    ||   str_cmp( argument, group_table[sn].name ) )
    {
	send_to_char( "Argument must match the group you are editing.\n\r", ch );
	return FALSE;
    }

    new_table = malloc( sizeof( struct group_type ) * maxGroup );

    if ( !new_table )
    {
	send_to_char( "Memory allocation failed. Brace for impact...\n\r", ch );
	return FALSE;
    }

    check_olc_delete( ch );

    free_string( group_table[sn].name );
    group_table[sn].name = NULL;

    free_short( group_table[sn].rating );

    for ( lvl = 0; lvl < MAX_IN_GROUP; lvl++ )
    {
	free_string( group_table[sn].spells[lvl] );
	group_table[sn].spells[lvl] = NULL;
    }

    for ( i = 0, lvl = 0; i < maxGroup+1; i++ )
    {
	if ( i != sn )
	{
	    new_table[lvl] = group_table[i];
	    lvl++;
	}
    }

    maxGroup--; /* Important :() */

    free( group_table );
    group_table = new_table;
		
    // Re-Allocate pcdata known and gendata chosen
    update_group_data( sn );

    send_to_char( "Group deleted.\n\r", ch );

    mud_stat.classes_changed = TRUE;

    return TRUE;
}

bool gredit_groups( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int i, lvl, sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: groups <group_name>.\n\r", ch );
	return FALSE;
    }

    if ( ( i = group_lookup( argument ) ) == -1 )
    {
	send_to_char( "That is not a valid group.\n\r", ch );
	return FALSE;
    }

    for ( lvl = 0; lvl < MAX_IN_GROUP; lvl++ )
    {
	if ( group_table[sn].spells[lvl] == NULL )
	    break;

	if ( found )
	{
	    group_table[sn].spells[lvl-1] = group_table[sn].spells[lvl];
	    group_table[sn].spells[lvl]   = NULL;
	    continue;
	}

	if ( !str_cmp( group_table[sn].spells[lvl], group_table[i].name ) )
	{
	    sprintf( buf, "Group Edit: Group (%s) removed from group (%s).\n\r",
		group_table[i].name, group_table[sn].name );

	    free_string( group_table[sn].spells[lvl] );
	    group_table[sn].spells[lvl] = NULL;
	    found = TRUE;
	}
    }

    if ( !found )
    {
	if ( lvl >= MAX_IN_GROUP )
	{
	    send_to_char( "Group add failed, MAX_IN_GROUP reached.\n\r", ch );
	    return FALSE;
	}

	sprintf( buf, "Group Edit: Group (%s) added to group (%s).\n\r",
	    group_table[i].name, group_table[sn].name );
	group_table[sn].spells[lvl] = str_dup( group_table[i].name );
    }

    send_to_char( buf, ch );

    return TRUE;
}

bool gredit_name( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int pos, sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	send_to_char( "What do you wish to rename the group to?\n\r", ch );
	return FALSE;
    }

    if ( !check_argument( ch, argument, TRUE ) )
	return FALSE;

    if ( group_lookup( argument ) != -1 )
    {
	send_to_char( "That group already exists.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "grep -rl \"Gr '%s'\" ../player/ | xargs perl -pi -e \"s/Gr '%s'/Gr '%s'/\"",
	group_table[sn].name, group_table[sn].name, argument );
    system( buf );

    for ( pos = 0; class_table[pos].name[0] != '\0'; pos++ )
    {
	if ( !str_cmp( class_table[pos].base_group, group_table[sn].name ) )
	{
	    free_string( class_table[pos].base_group );
	    class_table[pos].base_group = str_dup( argument );
	    mud_stat.classes_changed = TRUE;
	}

	if ( !str_cmp( class_table[pos].default_group, group_table[sn].name ) )
	{
	    free_string( class_table[pos].default_group );
	    class_table[pos].default_group = str_dup( argument );
	    mud_stat.classes_changed = TRUE;
	}
    }

    sprintf( buf, "Group Edit: %s renamed to %s.\n\r",
	group_table[sn].name, argument );
    send_to_char( buf, ch );

    free_string( group_table[sn].name );
    group_table[sn].name = str_dup( argument );

    mud_stat.classes_changed = TRUE;
    return TRUE;
}

bool gredit_show( CHAR_DATA *ch, char *argument )
{
    BUFFER *final = new_buf( );
    char buf[MAX_STRING_LENGTH], class_list[maxClass][MAX_STRING_LENGTH];
    int i, lvl, sn;

    EDIT_TABLE( ch, sn );

    sprintf( buf, "{qNumber:     {s[{t%d{s]\n\r", sn );
    add_buf( final, buf );

    sprintf( buf, "{qName:       {s[{t%s{s]\n\r", group_table[sn].name );
    add_buf( final, buf );

    add_buf( final, "\n\r{qSpells:\n\r{s-------------------------\n\r{q" );

    for ( i = 0; i < MAX_IN_GROUP; i++ )
    {
	if ( group_table[sn].spells[i] == NULL )
	    break;

	sprintf( buf, " %s\n\r", group_table[sn].spells[i] );
	add_buf( final, buf );
    }

    add_buf( final, "\n\r" );

    i = 0;
    buf[0] = '\0';
    for ( lvl = 0; class_table[lvl].name[0] != '\0'; lvl++ )
    {
	if ( class_table[lvl].tier > i )
	{
	    strcat( buf, " {s[{qClass{s] {tCost " );
	    i = class_table[lvl].tier;
	}
    }

    add_buf( final, buf );

    for ( i = 0; i < maxClass; i++ )
	class_list[i][0] = '\0';

    for ( lvl = 0; class_table[lvl].name[0] != '\0'; lvl++ )
    {
	sprintf( buf, " {%c%c{%c%-5.5s{q: {s[{t%2d{s] ",
	    class_table[lvl].who_name[1], UPPER( *class_table[lvl].name ),
	    class_table[lvl].who_name[4], class_table[lvl].name+1,
	    group_table[sn].rating[lvl] );
	strcat( class_list[class_table[lvl].sub_class], buf );
    }

    for ( i = 0; i < maxClass; i++ )
    {
	if ( class_list[i][0] != '\0' )
	{
	    add_buf( final, "\n\r" );
	    add_buf( final, class_list[i] );
	}
    }

    add_buf( final, "{x\n\r" );

    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

bool gredit_spells( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int i, lvl, sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: spells <spell_name>.\n\r", ch );
	return FALSE;
    }

    if ( ( i = skill_lookup( argument ) ) == -1 )
    {
	send_to_char( "That is not a valid skill.\n\r", ch );
	return FALSE;
    }

    for ( lvl = 0; lvl < MAX_IN_GROUP; lvl++ )
    {
	if ( group_table[sn].spells[lvl] == NULL )
	    break;

	if ( found )
	{
	    group_table[sn].spells[lvl-1] = group_table[sn].spells[lvl];
	    group_table[sn].spells[lvl]   = NULL;
	    continue;
	}

	if ( !str_cmp( group_table[sn].spells[lvl], skill_table[i].name ) )
	{
	    sprintf( buf, "Group Edit: Skill/Spell (%s) removed from group (%s).\n\r",
		skill_table[i].name, group_table[sn].name);

	    free_string( group_table[sn].spells[lvl] );
	    group_table[sn].spells[lvl] = NULL;
	    found = TRUE;
	}
    }

    if ( !found )
    {
	if ( lvl >= MAX_IN_GROUP )
	{
	    send_to_char( "Skill add failed, MAX_IN_GROUP reached.\n\r", ch );
	    return FALSE;
	}

	sprintf( buf, "Group Edit: Skill/Spell (%s) added to group (%s).\n\r",
	    skill_table[i].name, group_table[sn].name );
	group_table[sn].spells[lvl] = str_dup( skill_table[i].name );
    }

    send_to_char( buf, ch );

    return TRUE;
}

bool skedit_beats( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int i, sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0'
    ||   !is_number( argument )
    ||   ( i = atoi( argument ) ) < 0 )
    {
	send_to_char( "Syntax: beats [number].\n\r", ch );
	return FALSE;
    }

    if ( i %6 != 0 )
    {
	send_to_char( "Beats must be in multiples of 6 ( 1 half round ).\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "Skill Edit: (%s) Beats changed from %d to %d.\n\r",
	skill_table[sn].name, skill_table[sn].beats, i );
    send_to_char( buf, ch );

    skill_table[sn].beats = i;

    return TRUE;
}

bool skedit_clan_cost( CHAR_DATA *ch, char *argument )
{
    COST_DATA *price;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char arg5[MAX_INPUT_LENGTH];
    int cubic, aquest, iquest, level, max, sn;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );
    argument = one_argument( argument, arg5 );

    EDIT_TABLE( ch, sn );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: clan_cost [type] [cubics] [aquest] [iquest] <level> <max>\n\r"
		      "        clan_cost [type] remove\n\r"
		      " \n\rValid type: all, potion, scroll, pill, wand, staff\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	if ( !str_cmp( arg2, "remove" ) )
	{
	    if ( skill_table[sn].cost_potion != NULL )
		free_mem( skill_table[sn].cost_potion, sizeof( COST_DATA ) );

	    if ( skill_table[sn].cost_scroll != NULL )
		free_mem( skill_table[sn].cost_scroll, sizeof( COST_DATA ) );

	    if ( skill_table[sn].cost_pill != NULL )
		free_mem( skill_table[sn].cost_pill, sizeof( COST_DATA ) );

	    if ( skill_table[sn].cost_wand != NULL )
		free_mem( skill_table[sn].cost_wand, sizeof( COST_DATA ) );

	    if ( skill_table[sn].cost_staff != NULL )
		free_mem( skill_table[sn].cost_staff, sizeof( COST_DATA ) );

	    skill_table[sn].cost_potion	= NULL;
	    skill_table[sn].cost_scroll	= NULL;
	    skill_table[sn].cost_pill	= NULL;
	    skill_table[sn].cost_wand	= NULL;
	    skill_table[sn].cost_staff	= NULL;

	    send_to_char( "All clan cost data removed.\n\r", ch );
	    return TRUE;
	}

	cubic	= atoi( arg2 );
	aquest	= atoi( arg3 );
	iquest	= atoi( arg4 );
	level	= *arg5 ? atoi( arg5 ) : 1;
	max	= *argument ? atoi( argument ) : 1;

	if ( cubic < 0 || aquest < 0 || iquest < 0 || level < 0 || max < 0 || max > 4 )
	{
	    skedit_clan_cost( ch, "" );
	    return FALSE;
	}

	if ( cubic == 0 && aquest == 0 && iquest == 0 )
	{
	    send_to_char( "Clan upgrades are not free!\n\r", ch );
	    return FALSE;
	}

	if ( skill_table[sn].cost_potion == NULL )
	    skill_table[sn].cost_potion = alloc_mem( sizeof( COST_DATA ) );

	price		= skill_table[sn].cost_potion;
	price->cubic	= cubic;
	price->aquest	= aquest;
	price->iquest	= iquest;
	price->level	= level;
	price->max	= max;

	if ( skill_table[sn].cost_scroll == NULL )
	    skill_table[sn].cost_scroll = alloc_mem( sizeof( COST_DATA ) );

	price		= skill_table[sn].cost_scroll;
	price->cubic	= cubic;
	price->aquest	= aquest;
	price->iquest	= iquest;
	price->level	= level;
	price->max	= max;

	if ( skill_table[sn].cost_pill == NULL )
	    skill_table[sn].cost_pill = alloc_mem( sizeof( COST_DATA ) );

	price		= skill_table[sn].cost_pill;
	price->cubic	= cubic;
	price->aquest	= aquest;
	price->iquest	= iquest;
	price->level	= level;
	price->max	= max;

	if ( skill_table[sn].cost_wand == NULL )
	    skill_table[sn].cost_wand = alloc_mem( sizeof( COST_DATA ) );

	price		= skill_table[sn].cost_wand;
	price->cubic	= cubic;
	price->aquest	= aquest;
	price->iquest	= iquest;
	price->level	= level;
	price->max	= 1;

	if ( skill_table[sn].cost_staff == NULL )
	    skill_table[sn].cost_staff = alloc_mem( sizeof( COST_DATA ) );

	price		= skill_table[sn].cost_staff;
	price->cubic	= cubic;
	price->aquest	= aquest;
	price->iquest	= iquest;
	price->level	= level;
	price->max	= 1;

	send_to_char( "All clan cost data set.\n\r", ch );
	return TRUE;
    }

    cubic	= atoi( arg2 );
    aquest	= atoi( arg3 );
    iquest	= atoi( arg4 );
    level	= *arg5 ? atoi( arg5 ) : 1;
    max		= *argument ? atoi( argument ) : 1;

    if ( !str_prefix( arg1, "potion" ) )
    {
	if ( !str_cmp( arg2, "remove" ) )
	{
	    if ( skill_table[sn].cost_potion != NULL )
		free_mem( skill_table[sn].cost_potion, sizeof( COST_DATA ) );

	    skill_table[sn].cost_potion = NULL;

	    send_to_char( "Clan cost data removed.\n\r", ch );
	    return TRUE;
	}

	if ( skill_table[sn].cost_potion == NULL )
	    skill_table[sn].cost_potion = alloc_mem( sizeof( COST_DATA ) );

	price = skill_table[sn].cost_potion;
    }

    else if ( !str_prefix( arg1, "pill" ) )
    {
	if ( !str_cmp( arg2, "remove" ) )
	{
	    if ( skill_table[sn].cost_pill != NULL )
		free_mem( skill_table[sn].cost_pill, sizeof( COST_DATA ) );

	    skill_table[sn].cost_pill = NULL;

	    send_to_char( "Clan cost data removed.\n\r", ch );
	    return TRUE;
	}

	if ( skill_table[sn].cost_pill == NULL )
	    skill_table[sn].cost_pill = alloc_mem( sizeof( COST_DATA ) );

	price = skill_table[sn].cost_pill;
    }

    else if ( !str_prefix( arg1, "scroll" ) )
    {
	if ( !str_cmp( arg2, "remove" ) )
	{
	    if ( skill_table[sn].cost_scroll != NULL )
		free_mem( skill_table[sn].cost_scroll, sizeof( COST_DATA ) );

	    skill_table[sn].cost_scroll = NULL;

	    send_to_char( "Clan cost data removed.\n\r", ch );
	    return TRUE;
	}

	if ( skill_table[sn].cost_scroll == NULL )
	    skill_table[sn].cost_scroll = alloc_mem( sizeof( COST_DATA ) );

	price = skill_table[sn].cost_scroll;
    }

    else if ( !str_prefix( arg1, "wand" ) )
    {
	if ( !str_cmp( arg2, "remove" ) )
	{
	    if ( skill_table[sn].cost_wand != NULL )
		free_mem( skill_table[sn].cost_wand, sizeof( COST_DATA ) );

	    skill_table[sn].cost_wand = NULL;

	    send_to_char( "Clan cost data removed.\n\r", ch );
	    return TRUE;
	}

	if ( skill_table[sn].cost_wand == NULL )
	    skill_table[sn].cost_wand = alloc_mem( sizeof( COST_DATA ) );

	price = skill_table[sn].cost_wand;
	max = 1;
    }

    else if ( !str_prefix( arg1, "staff" ) )
    {
	if ( !str_cmp( arg2, "remove" ) )
	{
	    if ( skill_table[sn].cost_staff != NULL )
		free_mem( skill_table[sn].cost_staff, sizeof( COST_DATA ) );

	    skill_table[sn].cost_staff = NULL;

	    send_to_char( "Clan cost data removed.\n\r", ch );
	    return TRUE;
	}

	if ( skill_table[sn].cost_staff == NULL )
	    skill_table[sn].cost_staff = alloc_mem( sizeof( COST_DATA ) );

	price = skill_table[sn].cost_staff;
	max = 1;
    }

    else
    {
	skedit_clan_cost( ch, "" );
	return FALSE;
    }

    if ( cubic < 0 || aquest < 0 || iquest < 0 || level < 0 || max < 0 || max > 4 )
    {
	skedit_clan_cost( ch, "" );
	return FALSE;
    }

    if ( cubic == 0 && aquest == 0 && iquest == 0 )
    {
	send_to_char( "Clan upgrades are not free!\n\r", ch );
	return FALSE;
    }

    price->cubic	= cubic;
    price->aquest	= aquest;
    price->iquest	= iquest;
    price->level	= level;
    price->max		= max;

    send_to_char( "Clan cost data set.\n\r", ch );

    return TRUE;
}

bool skedit_copy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int i, lvl, sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: copy <from skill>.\n\r", ch );
	return FALSE;
    }

    if ( ( i = skill_lookup( argument ) ) == -1 )
    {
	send_to_char( "That skill does not exist.\n\r", ch );
	return FALSE;
    }

    free_string( skill_table[sn].msg_off );
    free_string( skill_table[sn].msg_obj );
    free_string( skill_table[sn].noun_damage );
    free_string( skill_table[sn].sound_cast );
    free_string( skill_table[sn].sound_off );

    skill_table[sn].flags		= skill_table[i].flags;
    skill_table[sn].minimum_position	= skill_table[i].minimum_position;
    skill_table[sn].cost_hp		= skill_table[i].cost_hp;
    skill_table[sn].cost_mana		= skill_table[i].cost_mana;
    skill_table[sn].cost_move		= skill_table[i].cost_move;
    skill_table[sn].beats		= skill_table[i].beats;
    skill_table[sn].msg_off		= str_dup(skill_table[i].msg_off);
    skill_table[sn].msg_obj		= str_dup(skill_table[i].msg_obj);
    skill_table[sn].noun_damage		= str_dup(skill_table[i].noun_damage);
    skill_table[sn].sound_cast		= str_dup(skill_table[i].sound_cast);
    skill_table[sn].sound_off		= str_dup(skill_table[i].sound_off);

    for ( lvl = 0; lvl < maxClass; lvl++ )
    {
	skill_table[sn].skill_level[lvl]= skill_table[i].skill_level[lvl];
	skill_table[sn].rating[lvl]	= skill_table[i].rating[lvl];
    }

    sprintf( buf, "Skill Edited: (%s) Info from (%s) copied.\n\r",
	skill_table[sn].name, skill_table[i].name );
    send_to_char( buf, ch );

    return TRUE;
}

bool skedit_create( CHAR_DATA *ch, char *argument )
{
    struct skill_type *new_table;

    if ( argument[0] == '\0' )
    {
	send_to_char( "SKEDIT create what skill?\n\r", ch );
	return FALSE;
    }

    if ( !check_argument( ch, argument, TRUE ) )
	return FALSE;

    if ( skill_lookup( argument ) != -1 )
    {
	send_to_char( "That skill already exists!\n\r", ch );
	return FALSE;
    }

    maxSkill++;
    new_table = realloc( skill_table, sizeof( struct skill_type ) * ( maxSkill + 1 ) );

    if ( !new_table )
    {
	send_to_char( "Memory allocation failed. Brace for impact.\n\r", ch );
	return FALSE;
    }

    skill_table					= new_table;
    skill_table[maxSkill-1].name		= str_dup( argument );
    skill_table[maxSkill-1].pgsn		= NULL;
    skill_table[maxSkill-1].room_msg		= NULL;
    skill_table[maxSkill-1].sound_cast		= NULL;
    skill_table[maxSkill-1].sound_off		= NULL;
    skill_table[maxSkill-1].cost_potion		= NULL;
    skill_table[maxSkill-1].cost_scroll		= NULL;
    skill_table[maxSkill-1].cost_pill		= NULL;
    skill_table[maxSkill-1].cost_wand		= NULL;
    skill_table[maxSkill-1].cost_staff		= NULL;
    skill_table[maxSkill-1].target		= TAR_IGNORE;
    skill_table[maxSkill-1].minimum_position	= POS_STANDING;
    skill_table[maxSkill-1].spell_fun		= spell_null;
    skill_table[maxSkill-1].cost_hp		= 0;
    skill_table[maxSkill-1].cost_mana		= 0;
    skill_table[maxSkill-1].cost_move		= 0;
    skill_table[maxSkill-1].beats		= 24;
    skill_table[maxSkill-1].flags		= 0;
    skill_table[maxSkill-1].msg_off		= str_dup( "" );
    skill_table[maxSkill-1].msg_obj		= str_dup( "" );
    skill_table[maxSkill-1].noun_damage		= str_dup( "" );
    skill_table[maxSkill-1].skill_level		= new_short( maxClass, LEVEL_IMMORTAL );
    skill_table[maxSkill-1].rating		= new_short( maxClass, 0 );

    SET_BIT( skill_table[maxSkill-1].flags, SKILL_DISABLED );

    skill_table[maxSkill].name = str_dup( "" );
    skill_table[maxSkill].skill_level = NULL;
    skill_table[maxSkill].rating = NULL;

    /* Re-Allocate memory for char, mob and gen_data */
    update_skill_data( -1 );

    ch->desc->editor = ED_SKILL;
    ch->desc->pEdit = (void *)maxSkill-1;

    send_to_char( "New skill created.\n\r", ch );

    return TRUE;
}

bool skedit_dam_noun( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: dam_noun <message>.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "Skill Edit: (%s) Damage noun changed from:\n\r%s\n\rTo:\n\r%s\n\r",
	skill_table[sn].name, skill_table[sn].noun_damage, argument );
    send_to_char( buf, ch );

    free_string( skill_table[sn].noun_damage );
    skill_table[sn].noun_damage = str_dup( argument );

    return TRUE;
}

void check_affect_locations( AFFECT_DATA *paf, int sn )
{
    for ( ; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	{
	    paf->type = 0;
	    paf->duration = 0;
	}

	else if ( paf->type > sn )
	    paf->type--;
    }
}

bool skedit_delete( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *wch;
    DESCRIPTOR_DATA *d;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObj;
    struct skill_type *new_table;
    int i, lvl, sn, vnum, match = 0;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0'
    ||   str_cmp( argument, skill_table[sn].name ) )
    {
	send_to_char( "Argument must match the skill you are editing.\n\r", ch );
	return FALSE;
    }

    check_olc_delete( ch );

    free_string( skill_table[sn].name		);
    free_string( skill_table[sn].room_msg	);
    free_string( skill_table[sn].sound_cast	);
    free_string( skill_table[sn].sound_off	);
    free_string( skill_table[sn].msg_off	);
    free_string( skill_table[sn].msg_obj	);
    free_string( skill_table[sn].noun_damage	);

    free_short( skill_table[sn].skill_level );
    free_short( skill_table[sn].rating );

    if ( skill_table[sn].cost_potion )
	free_mem( skill_table[sn].cost_potion, sizeof( COST_DATA ) );

    if ( skill_table[sn].cost_scroll )
	free_mem( skill_table[sn].cost_scroll, sizeof( COST_DATA ) );

    if ( skill_table[sn].cost_pill )
	free_mem( skill_table[sn].cost_pill, sizeof( COST_DATA ) );

    if ( skill_table[sn].cost_wand )
	free_mem( skill_table[sn].cost_wand, sizeof( COST_DATA ) );

    if ( skill_table[sn].cost_staff )
	free_mem( skill_table[sn].cost_staff, sizeof( COST_DATA ) );

    skill_table[sn].cost_potion		= NULL;
    skill_table[sn].cost_scroll		= NULL;
    skill_table[sn].cost_pill		= NULL;
    skill_table[sn].cost_wand		= NULL;
    skill_table[sn].cost_staff		= NULL;
    skill_table[sn].name		= NULL;
    skill_table[sn].pgsn		= NULL;
    skill_table[sn].room_msg		= NULL;
    skill_table[sn].sound_cast		= NULL;
    skill_table[sn].sound_off		= NULL;
    skill_table[sn].target		= TAR_IGNORE;
    skill_table[sn].minimum_position	= POS_STANDING;
    skill_table[sn].spell_fun		= spell_null;
    skill_table[sn].cost_hp		= 0;
    skill_table[sn].cost_mana		= 0;
    skill_table[sn].cost_move		= 0;
    skill_table[sn].flags		= 0;
    skill_table[sn].beats		= 24;
    skill_table[sn].msg_off		= NULL;
    skill_table[sn].msg_obj		= NULL;
    skill_table[sn].noun_damage		= NULL;

    new_table = malloc( sizeof( struct skill_type ) * maxSkill );

    if ( !new_table )
    {
	send_to_char( "Memory allocation failed. Brace for impact...\n\r", ch );
	return FALSE;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING || d->character == NULL )
	    continue;

	check_affect_locations( d->character->affected, sn );
    }

    for ( wch = char_list; wch != NULL; wch = wch->next )
	check_affect_locations( wch->affected, sn );

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	switch( obj->item_type )
	{
	    default:
		break;

	    case ITEM_PILL:
	    case ITEM_SCROLL:
	    case ITEM_POTION:
		for ( i = 1; i < 5; i++ )
		{
		    if ( obj->value[i] == sn )
			obj->value[i] = 0;

		    else if ( obj->value[i] > sn )
			obj->value[i]--;
		}
		break;

	    case ITEM_WAND:
	    case ITEM_STAFF:
		if ( obj->value[3] == sn )
		    obj->value[3] = 0;
		else if ( obj->value[3] > sn )
		    obj->value[3]--;
		break;
	}

	check_affect_locations( obj->affected, sn );
    }

    for ( vnum = 0; match < top_obj_index; vnum++ )
    {
	if ( ( pObj = get_obj_index( vnum ) ) != NULL )
	{
	    match++;

	    switch( pObj->item_type )
	    {
		default:
		    break;

		case ITEM_PILL:
		case ITEM_SCROLL:
		case ITEM_POTION:
		    for ( i = 1; i < 5; i++ )
		    {
			if ( pObj->value[i] == sn )
			    pObj->value[i] = 0;

			else if ( pObj->value[i] > sn )
			    pObj->value[i]--;
		    }
		    break;

		case ITEM_WAND:
		case ITEM_STAFF:
		    if ( pObj->value[3] == sn )
			pObj->value[3] = 0;
		    else if ( pObj->value[3] > sn )
			pObj->value[3]--;
		    break;
	    }
	}
    }

    for ( i = 0, lvl = 0; i < maxSkill+1; i++ )
    {
	if ( i != sn )
	{
	    new_table[lvl] = skill_table[i];
	    lvl++;
	}
    }

    maxSkill--; /* Important :() */

    free( skill_table );
    skill_table = new_table;

    // Re-Allocate pcdata known and gendata chosen
    update_skill_data( sn );
		
    for ( ; skill_table[sn].name[0] != '\0'; sn++ )
    {
	if ( skill_table[sn].pgsn != NULL )
	    *skill_table[sn].pgsn = sn;
    }

    send_to_char( "Skill deleted.\n\r", ch );

    return TRUE;
}

bool skedit_flags( CHAR_DATA *ch, char *argument )
{
    int sn, value;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0'
    ||   ( value = flag_value( skill_flags, argument ) ) == NO_FLAG )
    {
	send_to_char( "Available skill flags are:\n\r", ch );
	show_help( ch, "skills" );
	return FALSE;
    }

    skill_table[sn].flags ^= value;

    send_to_char( "Skill flag toggled.\n\r", ch);
    return TRUE;
}

bool skedit_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int gn, i, sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: group <group name>.\n\r", ch );
	return FALSE;
    }

    if ( ( gn = group_lookup( argument ) ) == -1 )
    {
	send_to_char( "That is not a valid group.\n\r", ch );
	return FALSE;
    }

    for ( i = 0; i < MAX_IN_GROUP; i++ )
    {
	if ( group_table[gn].spells[i] == NULL )
	    break;

	if ( found )
	{
	    group_table[gn].spells[i-1] = group_table[gn].spells[i];
	    group_table[gn].spells[i]   = NULL;
	    continue;
	}

	if ( !str_cmp( group_table[gn].spells[i], skill_table[sn].name ) )
	{
	    sprintf( buf, "Skill Edit: Spell (%s) removed from group (%s).\n\r",
		skill_table[sn].name, group_table[gn].name );

	    free_string( group_table[gn].spells[i] );
	    group_table[gn].spells[i] = NULL;
	    found = TRUE;
	}
    }

    if ( !found )
    {
	if ( i >= MAX_IN_GROUP )
	{
	    send_to_char( "Skill add failed, MAX_IN_GROUP reached.\n\r", ch );
	    return FALSE;
	}

	sprintf( buf, "Skill Edit: Spell (%s) added to group (%s).\n\r",
	    skill_table[sn].name, group_table[gn].name );
	group_table[gn].spells[i] = str_dup( skill_table[sn].name );
    }

    send_to_char( buf, ch );

    return TRUE;
}

bool skedit_level( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    bool fAll;
    sh_int i = 0, lvl, sn;

    EDIT_TABLE( ch, sn );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Invalid class or level.\n\r", ch );
	return FALSE;
    }

    fAll = !str_cmp( arg, "all" );

    if ( !fAll && ( i = class_lookup( arg ) ) == -1 )
    {
	sprintf( buf, "No class named '%s' could be found.\n\r", arg );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( !is_number( argument ) || (lvl = atoi( argument ) ) < 1 )
    {
	send_to_char( "New level must be a valid intiger.\n\r", ch );
	return FALSE;
    }

    if ( fAll )
    {
	for ( i = 0; i < maxClass; i++ )
	{
	    skill_table[sn].skill_level[i] = lvl;

	    if ( lvl > LEVEL_HERO && skill_table[sn].spell_fun == spell_null )
		skill_table[sn].rating[i] = 0;
	}

	sprintf( buf, "Skill Edit: (%s) Level for all classes changed to %d.\n\r",
	    skill_table[sn].name, lvl );
    } else {
	sprintf( buf, "Skill Edit: (%s) Level for %s changed from %d to %d.\n\r",
	    skill_table[sn].name, class_table[i].name, skill_table[sn].skill_level[i], lvl );
	skill_table[sn].skill_level[i] = lvl;

	if ( lvl > LEVEL_HERO && skill_table[sn].spell_fun == spell_null )
	    skill_table[sn].rating[i] = 0;
    }

    send_to_char( buf, ch );

    mud_stat.classes_changed = TRUE;

    return FALSE;
}

bool skedit_cost_hp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int i, sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: cost_hp <value>.\n\r", ch );
	return FALSE;
    }

    if ( !is_number( argument ) || ( i = atoi( argument ) ) < 0 )
    {
	send_to_char( "Cost setting must be a valid intiger.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "Skill Edit: (%s) Hitpoint cost changed from %d to %d.\n\r",
	skill_table[sn].name, skill_table[sn].cost_hp, i );
    send_to_char( buf, ch );

    skill_table[sn].cost_hp = i;

    return TRUE;
}

bool skedit_cost_mana( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int i, sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: cost_mana <value>.\n\r", ch );
	return FALSE;
    }

    if ( !is_number( argument ) || ( i = atoi( argument ) ) < 0 )
    {
	send_to_char( "Cost setting must be a valid intiger.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "Skill Edit: (%s) Mana cost changed from %d to %d.\n\r",
	skill_table[sn].name, skill_table[sn].cost_mana, i );
    send_to_char( buf, ch );

    skill_table[sn].cost_mana = i;

    return TRUE;
}

bool skedit_cost_move( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int i, sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: cost_move <value>.\n\r", ch );
	return FALSE;
    }

    if ( !is_number( argument ) || ( i = atoi( argument ) ) < 0 )
    {
	send_to_char( "Cost setting must be a valid intiger.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "Skill Edit: (%s) Movement cost changed from %d to %d.\n\r",
	skill_table[sn].name, skill_table[sn].cost_move, i );
    send_to_char( buf, ch );

    skill_table[sn].cost_move = i;

    return TRUE;
}

bool skedit_name( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;
    OBJ_INDEX_DATA *pObj;
    char buf[MAX_STRING_LENGTH];
    int i, pos, gn, sn, vnum;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: name <new name>.\n\r", ch );
	return FALSE;
    }

    if ( !check_argument( ch, argument, TRUE ) )
	return FALSE;

    if ( skill_lookup( argument ) != -1 )
    {
	send_to_char( "That skill already exists!\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "grep -rl \"'%s'\" ../player/ | xargs perl -pi -e \"s/Sk (\\d*) '%s'/Sk \\1 '%s'/\"",
	skill_table[sn].name, skill_table[sn].name, argument );
    system( buf );

    for ( gn = 0; group_table[gn].name[0] != '\0'; gn++ )
    {
	for ( pos = 0; pos < MAX_IN_GROUP; pos++ )
	{
	    if ( group_table[gn].spells[pos] == NULL )
		break;

	    if ( !str_cmp( group_table[gn].spells[pos], skill_table[sn].name ) )
	    {
		free_string( group_table[gn].spells[pos] );
		group_table[gn].spells[pos] = str_dup( argument );
	    }
	}
    }

    for ( i = 0, vnum = 0; i < top_mob_index; vnum++ )
    {
	if ( ( pMob = get_mob_index( vnum ) ) != NULL )
	{
	    i++;
	    if ( pMob->learned[sn] != pMob->skill_percentage )
		SET_BIT( pMob->area->area_flags, AREA_CHANGED );
	}
    }

    for ( i = 0, vnum = 0; i < top_obj_index; vnum++ )
    {
	if ( ( pObj = get_obj_index( vnum ) ) != NULL )
	{
	    i++;
	    switch( pObj->item_type )
	    {
		default:
		    break;

		case ITEM_WAND:
		case ITEM_STAFF:
		    if ( pObj->value[3] == sn )
			SET_BIT( pObj->area->area_flags, AREA_CHANGED );
		    break;

		case ITEM_POTION:
		case ITEM_SCROLL:
		case ITEM_PILL:
		    for ( pos = 1; pos < 5; pos++ )
		    {
			if ( pObj->value[pos] == sn )
			{
			    SET_BIT( pObj->area->area_flags, AREA_CHANGED );
			    break;
			}
		    }
		    break;
	    }
	}
    }

    sprintf( buf, "Skill Edited: %s renamed to %s.\n\r",
	skill_table[sn].name, argument );
    send_to_char( buf, ch );

    free_string( skill_table[sn].name );
    skill_table[sn].name = str_dup( argument );

    mud_stat.classes_changed = TRUE;
    mud_stat.races_changed = TRUE;

    return TRUE;
}

bool skedit_obj_off( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	sprintf(buf,"{GWear Off Obj{w:  {g%s{x\n\r", skill_table[sn].msg_obj);
	send_to_char(buf,ch);
	return FALSE;
    }

    sprintf(buf,"{GSkill Edit{w: ({y%s{w) {GWear off obj message changed from{w:{x\n\r%s\n\r{GTo{w:{x\n\r%s\n\r",
	skill_table[sn].name, skill_table[sn].msg_obj, argument);
    send_to_char( buf, ch );

    free_string(skill_table[sn].msg_obj);
    skill_table[sn].msg_obj = str_dup(argument);

    return TRUE;
}

bool skedit_position( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int i, sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	sprintf(buf,"{GPositi{w:    [{y%d{w] {g%s{x\n\r",
	    skill_table[sn].minimum_position,
	    pos_save(skill_table[sn].minimum_position));
	send_to_char(buf,ch);
	return FALSE;
    }

    if ( (is_number(argument)
    &&    ((i = atoi(argument)) >= POS_DEAD) && i <= POS_STANDING)
    ||    (i = pos_load(argument)) != -1 )
    {
	sprintf(buf,"{GSkill Edited{w: ({y%s{w) {GPosition changed from {g%s {Gto {g%s{G.{x\n\r",
	    skill_table[sn].name, pos_save(skill_table[sn].minimum_position),
	    position_flags[i].name);
	send_to_char( buf, ch );
	skill_table[sn].minimum_position = i;
	return TRUE;
    }

    send_to_char("{GValid positions are:\n\r{y----------------------\n\n{G",ch);

    for ( i = POS_DEAD; i < POS_STANDING+1; i++ )
    {
	sprintf(buf," %s\n\r", position_flags[i].name);
	send_to_char(buf,ch);
    }

    send_to_char("{x",ch);
    return FALSE;
}

bool skedit_rating( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    bool fAll;
    sh_int i = 0, lvl, sn;

    EDIT_TABLE( ch, sn );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Invalid class or rating.\n\r", ch );
	return FALSE;
    }

    fAll = !str_cmp( arg, "all" );

    if ( !fAll && ( i = class_lookup( arg ) ) == -1 )
    {
	sprintf( buf, "No class named '%s' could be found.\n\r", arg );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( !is_number( argument ) || ( lvl = atoi( argument ) ) < 0 )
    {
	send_to_char( "New rating must be a valid intiger.\n\r", ch );
	return FALSE;
    }

    if ( fAll )
    {
	for ( i = 0; i < maxClass; i++ )
	{
	    skill_table[sn].rating[i] = lvl;

	    if ( lvl == 0 )
		skill_table[sn].skill_level[i] = LEVEL_IMMORTAL;
	}

	sprintf( buf, "{GSkill Edit{w: ({y%s{w) {GRating for all classes changed to {g%d{G.{x\n\r",
	    skill_table[sn].name, lvl );
    } else {
	sprintf( buf, "{GSkill Edit{w: ({y%s{w) {GRating for %s changed from {g%d {Gto {g%d{G.{x\n\r",
	    skill_table[sn].name, class_table[i].name, skill_table[sn].rating[i], lvl );
	skill_table[sn].rating[i] = lvl;

	if ( lvl == 0 )
	    skill_table[sn].skill_level[i] = LEVEL_IMMORTAL;
    }

    send_to_char( buf, ch );

    mud_stat.classes_changed = TRUE;

    return FALSE;
}

bool skedit_room_off( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	sprintf(buf,"{GRoom Off Obj{w:  {g%s{x\n\r",
	    skill_table[sn].room_msg ? skill_table[sn].room_msg : "None");
	send_to_char(buf,ch);
	return FALSE;
    }

    if ( !str_cmp(argument,"none") || !str_cmp(argument,"remove") )
    {
	free_string(skill_table[sn].room_msg);
	skill_table[sn].room_msg = NULL;
	sprintf(buf,"Room msg removed.\n\r");
    } else {
	sprintf(buf,"{GSkill Edit{w: ({y%s{w) {GRoom off message changed from{w:{x\n\r%s\n\r{GTo{w:{x\n\r%s\n\r",
	    skill_table[sn].name, skill_table[sn].room_msg ? skill_table[sn].room_msg :
	    "none", argument);
	free_string(skill_table[sn].room_msg);
	skill_table[sn].room_msg = str_dup(argument);
    }

    send_to_char( buf, ch );
    return TRUE;
}

bool skedit_show( CHAR_DATA *ch, char *argument )
{
    BUFFER *final = new_buf();
    char class_list[maxClass][MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
    sh_int i, lvl, sn;
    bool found = FALSE;

    EDIT_TABLE( ch, sn );

    sprintf( buf, "{qNumber:            {s[{t%d{s]\n\r", sn );
    add_buf( final, buf );

    sprintf( buf, "{qName:              {s[{t%s{s]\n\r",
	skill_table[sn].name );
    add_buf( final, buf );

    sprintf( buf, "{qTarget:        {s[{t%d{s] [{t%s{s]\n\r",
	skill_table[sn].target, target_save( skill_table[sn].target ) );
    add_buf( final, buf );

    sprintf( buf, "{qPosition:      {s[{t%d{s] [{t%s{s]\n\r",
	skill_table[sn].minimum_position,
	pos_save( skill_table[sn].minimum_position ) );
    add_buf( final, buf );

    sprintf( buf, "{qFlags:             {s[{t%s{s]\n\r",
	flag_string( skill_flags, skill_table[sn].flags ) );
    add_buf( final, buf );

    sprintf( buf, "{qHit Point Cost:    {s[{t%d{s]\n\r",
	skill_table[sn].cost_hp );
    add_buf( final, buf );

    sprintf( buf, "{qMana Point Cost:   {s[{t%d{s]\n\r",
	skill_table[sn].cost_mana );
    add_buf( final, buf );

    sprintf( buf, "{qMove Point Cost:   {s[{t%d{s]\n\r",
	skill_table[sn].cost_move );
    add_buf( final, buf );

    sprintf( buf, "{qBeats:             {s[{t%d{s]\n\r",
	skill_table[sn].beats );
    add_buf( final, buf );

    sprintf( buf, "{qDamage Noun:       {s[{t%s{s]\n\r",
	skill_table[sn].noun_damage );
    add_buf( final, buf );

    sprintf( buf, "{qWear Off Msg:      {s[{t%s{s]\n\r",
	skill_table[sn].msg_off ? skill_table[sn].msg_off  : "None" );
    add_buf( final, buf );

    sprintf( buf, "{qObj Off Msg:       {s[{t%s{s]\n\r",
	skill_table[sn].msg_obj ? skill_table[sn].msg_obj : "None" );
    add_buf(final,buf);

    sprintf( buf, "{qRoom Off Msg:      {s[{t%s{s]\n\r",
	skill_table[sn].room_msg ? skill_table[sn].room_msg : "None" );
    add_buf( final, buf );

    sprintf( buf, "{qSound Cast:        {s[{t%s{s]\n\r",
	skill_table[sn].sound_cast ? skill_table[sn].sound_cast : "None" );
    add_buf( final, buf );

    sprintf( buf, "{qSound Off:         {s[{t%s{s]\n\r",
	skill_table[sn].sound_off ? skill_table[sn].sound_off : "None" );
    add_buf( final, buf );

    if ( skill_table[sn].cost_potion )
    {
	sprintf( buf, "{qClan Potion Cost:  {s[{t%d cubics, %d aquest, %d iquest, %d level, %d max{s]\n\r",
	    skill_table[sn].cost_potion->cubic,
	    skill_table[sn].cost_potion->aquest,
	    skill_table[sn].cost_potion->iquest,
	    skill_table[sn].cost_potion->level,
	    skill_table[sn].cost_potion->max );
	add_buf( final, buf );
    }

    if ( skill_table[sn].cost_scroll )
    {
	sprintf( buf, "{qClan Scroll Cost:  {s[{t%d cubics, %d aquest, %d iquest, %d level, %d max{s]\n\r",
	    skill_table[sn].cost_scroll->cubic,
	    skill_table[sn].cost_scroll->aquest,
	    skill_table[sn].cost_scroll->iquest,
	    skill_table[sn].cost_scroll->level,
	    skill_table[sn].cost_scroll->max );
	add_buf( final, buf );
    }

    if ( skill_table[sn].cost_pill )
    {
	sprintf( buf, "{qClan Pill Cost:    {s[{t%d cubics, %d aquest, %d iquest, %d level, %d max{s]\n\r",
	    skill_table[sn].cost_pill->cubic,
	    skill_table[sn].cost_pill->aquest,
	    skill_table[sn].cost_pill->iquest,
	    skill_table[sn].cost_pill->level,
	    skill_table[sn].cost_pill->max );
	add_buf( final, buf );
    }

    if ( skill_table[sn].cost_wand )
    {
	sprintf( buf, "{qClan Wand Cost:    {s[{t%d cubics, %d aquest, %d iquest, %d level, %d max{s]\n\r",
	    skill_table[sn].cost_wand->cubic,
	    skill_table[sn].cost_wand->aquest,
	    skill_table[sn].cost_wand->iquest,
	    skill_table[sn].cost_wand->level,
	    skill_table[sn].cost_wand->max );
	add_buf( final, buf );
    }

    if ( skill_table[sn].cost_staff )
    {
	sprintf( buf, "{qClan Staff Cost:   {s[{t%d cubics, %d aquest, %d iquest, %d level, %d max{s]\n\r",
	    skill_table[sn].cost_staff->cubic,
	    skill_table[sn].cost_staff->aquest,
	    skill_table[sn].cost_staff->iquest,
	    skill_table[sn].cost_staff->level,
	    skill_table[sn].cost_staff->max );
	add_buf( final, buf );
    }

    add_buf( final, "\n\r" );

    for ( lvl = 0; race_table[lvl].name[0] != '\0'; lvl++ )
    {
	for ( i = 0; i < 5; i++ )
	{
	    if ( race_table[lvl].skills[i] == NULL )
		break;

	    if ( !str_cmp( skill_table[sn].name, race_table[lvl].skills[i] ) )
	    {
		if ( !found )
		{
		    add_buf( final, "{qSkill of the following race(s):\n\r" );
		    found = TRUE;
		}

		sprintf( buf, " {s[{t%s{s]\n\r", race_table[lvl].name );
		add_buf( final, buf );
		break;
	    }
	}
    }

    if ( found )
	add_buf( final, "\n\r" );

    found = FALSE;
    for ( lvl = 0; group_table[lvl].name[0] != '\0'; lvl++ )
    {
	for ( i = 0; i < MAX_IN_GROUP; i++ )
	{
	    if ( group_table[lvl].spells[i] == NULL )
		break;

	    if ( !str_cmp( skill_table[sn].name, group_table[lvl].spells[i] ) )
	    {
		if ( !found )
		{
		    add_buf( final, "{qMember of the following group(s):\n\r" );
		    found = TRUE;
		}

		sprintf( buf, " {s[{t%s{s]\n\r", group_table[lvl].name );
		add_buf( final, buf );
		break;
	    }
	}
    }

    if ( found )
	add_buf( final, "\n\r" );
    else if ( skill_table[sn].spell_fun != spell_null )
	add_buf( final, "{qMember of the following group(s):\n\r {s[{R! NONE !{s]\n\r\n\r" );

    i = 0;
    buf[0] = '\0';
    for ( lvl = 0; class_table[lvl].name[0] != '\0'; lvl++ )
    {
	if ( class_table[lvl].tier > i )
	{
	    strcat( buf, " {s[{qClass {s|{tLvl{s|{tRtg{s]" );
	    i = class_table[lvl].tier;
	}
    }

    add_buf( final, buf );
    add_buf( final, "\n\r" );

    for ( i = 0; i < maxClass; i++ )
	class_list[i][0] = '\0';

    for ( lvl = 0; class_table[lvl].name[0] != '\0'; lvl++ )
    {
	sprintf( buf, " {s[{%c%c{%c%-5.5s{s|{t%3d{s|{t%2d {s]",
	    class_table[lvl].who_name[1], UPPER( *class_table[lvl].name ),
	    class_table[lvl].who_name[4], class_table[lvl].name+1,
	    skill_table[sn].skill_level[lvl],
	    skill_table[sn].rating[lvl] );
	strcat( class_list[class_table[lvl].sub_class], buf );
    }

    for ( i = 0; i < maxClass; i++ )
    {
	if ( class_list[i][0] != '\0' )
	{
	    sprintf( buf, "%s\n\r", class_list[i] );
	    add_buf( final, buf );
	}
    }

    add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

bool skedit_sound_cast( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	sprintf(buf,"{GSound Cast  {w:  {g%s{x\n\r",
	    skill_table[sn].sound_cast ? skill_table[sn].sound_cast : "None");
	send_to_char(buf,ch);
	return FALSE;
    }

    if ( !str_cmp(argument,"none") || !str_cmp(argument,"remove") )
    {
	free_string(skill_table[sn].sound_cast);
	skill_table[sn].sound_cast = NULL;
	sprintf(buf,"Casting sound removed.\n\r");
    } else {
	sprintf(buf,"{GSkill Edit{w: ({y%s{w) {GSound cast file changed from{w:{x\n\r%s\n\r{GTo{w:{x\n\r%s\n\r",
	    skill_table[sn].name, skill_table[sn].sound_cast ? skill_table[sn].sound_cast :
	    "none", argument);
	free_string(skill_table[sn].sound_cast);
	skill_table[sn].sound_cast = str_dup(argument);
    }

    send_to_char( buf, ch );
    return TRUE;
}

bool skedit_sound_off( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	sprintf(buf,"{GSound Off     {w:  {g%s{x\n\r",
	    skill_table[sn].sound_off ? skill_table[sn].sound_off : "None");
	send_to_char(buf,ch);
	return FALSE;
    }

    if ( !str_cmp(argument,"none") || !str_cmp(argument,"remove") )
    {
	free_string(skill_table[sn].sound_off);
	skill_table[sn].sound_off = NULL;
	sprintf(buf,"Sound off file removed.\n\r");
    } else {
	sprintf(buf,"{GSkill Edit{w: ({y%s{w) {GSound off file changed from{w:{x\n\r%s\n\r{GTo{w:{x\n\r%s\n\r",
	    skill_table[sn].name, skill_table[sn].sound_off ? skill_table[sn].sound_off :
	    "none", argument);
	free_string(skill_table[sn].sound_off);
	skill_table[sn].sound_off = str_dup(argument);
    }

    send_to_char( buf, ch );
    return TRUE;
}

bool skedit_target( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int i, sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	sprintf(buf,"{GTarget{w:    [{y%d{w] {g%s{x\n\r",
	    skill_table[sn].target, target_save(skill_table[sn].target));
	send_to_char(buf,ch);
	return FALSE;
    }

    if ( (is_number(argument)
    &&    ((i = atoi(argument)) >= TAR_IGNORE) && i <= TAR_OBJ_TRAN)
    ||    (i = target_load(argument)) != -1 )
    {
	sprintf(buf,"{GSkill Edited{w: ({y%s{w) {GTarget changed from {g%s {Gto {g%s{G.{x\n\r",
	    skill_table[sn].name, target_save(skill_table[sn].target),
	    target_table[i].name);
	send_to_char( buf, ch );
	skill_table[sn].target = i;
	return TRUE;
    }    

    send_to_char("{GValid targets are:\n\r{y----------------------\n\n{G",ch);

    for ( i = TAR_IGNORE; i < TAR_OBJ_TRAN+1; i++ )
    {
	sprintf(buf," %s\n\r", target_table[i].name);
	send_to_char(buf,ch);
    }

    send_to_char("{x",ch);
    return FALSE;
}

bool skedit_wear_off( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    EDIT_TABLE( ch, sn );

    if ( argument[0] == '\0' )
    {
	sprintf(buf,"{GWear Off Msg{w:  {g%s{x\n\r", skill_table[sn].msg_off);
	send_to_char(buf,ch);
	return FALSE;
    }

    sprintf(buf,"{GSkill Edit{w: ({y%s{w) {GWear off message changed from{w:{x\n\r%s\n\r{GTo{w:{x\n\r%s\n\r",
	skill_table[sn].name, skill_table[sn].msg_off, argument);
    send_to_char( buf, ch );

    free_string(skill_table[sn].msg_off);
    skill_table[sn].msg_off = str_dup(argument);

    return TRUE;
}

REDIT ( redit_addrprog )
{
  int value;
  ROOM_INDEX_DATA *pRoom;
  PROG_LIST *list;
  PROG_CODE *code;
  char trigger[MAX_STRING_LENGTH];
  char phrase[MAX_STRING_LENGTH];
  char num[MAX_STRING_LENGTH];

  EDIT_ROOM(ch, pRoom);
  argument=one_argument(argument, num);
  argument=one_argument(argument, trigger);
  argument=one_argument(argument, phrase);

  if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0' )
  {
        send_to_char("Syntax:   addrprog [vnum] [trigger] [phrase]\n\r",ch);
        return FALSE;
  }

  if ( (value = flag_value (rprog_flags, trigger) ) == NO_FLAG)
  {
        send_to_char("Valid flags are:\n\r",ch);
        show_help( ch, "rprog");
        return FALSE;
  }

  if ( ( code =get_prog_index (atoi(num), PRG_RPROG ) ) == NULL)
  {
        send_to_char("No such ROOMProgram.\n\r",ch);
        return FALSE;
  }

  list                  = new_prog();
  list->vnum            = atoi(num);
  list->trig_type       = value;
  list->trig_phrase     = str_dup(phrase);
  list->code            = code->code;
  SET_BIT(pRoom->rprog_flags,value);
  list->next            = pRoom->rprogs;
  pRoom->rprogs          = list;

  send_to_char( "Rprog Added.\n\r",ch);
  return TRUE;
}

REDIT ( redit_delrprog )
{
    ROOM_INDEX_DATA *pRoom;
    PROG_LIST *list;
    PROG_LIST *list_next;
    char rprog[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;

    EDIT_ROOM(ch, pRoom);

    one_argument( argument, rprog );
    if (!is_number( rprog ) || rprog[0] == '\0' )
    {
       send_to_char("Syntax:  delrprog [#rprog]\n\r",ch);
       return FALSE;
    }

    value = atoi ( rprog );

    if ( value < 0 )
    {
        send_to_char("Only non-negative rprog-numbers allowed.\n\r",ch);
        return FALSE;
    }

    if ( !(list= pRoom->rprogs) )
    {
        send_to_char("REdit:  Non existant rprog.\n\r",ch);
        return FALSE;
    }

    if ( value == 0 )
    {
	REMOVE_BIT(pRoom->rprog_flags, pRoom->rprogs->trig_type);
        list = pRoom->rprogs;
        pRoom->rprogs = list->next;
        free_prog( list );
    }
    else
    {
        while ( (list_next = list->next) && (++cnt < value ) )
                list = list_next;

        if ( list_next )
        {
		REMOVE_BIT(pRoom->rprog_flags, list_next->trig_type);
                list->next = list_next->next;
                free_prog(list_next);
        }
        else
        {
                send_to_char("No such rprog.\n\r",ch);
                return FALSE;
        }
    }

    send_to_char("Rprog removed.\n\r", ch);
    return TRUE;
}

REDIT( redit_show )
{
    BUFFER *final = new_buf( );
    PROG_LIST *list;
    ROOM_DAMAGE_DATA *dam;
    ROOM_INDEX_DATA *pRoom;
    char buf[MAX_STRING_LENGTH], temp[MAX_INPUT_LENGTH];
    char *flags;
    sh_int door;
    
    EDIT_ROOM( ch, pRoom );

    add_buf( final, "{qDescription:" );

    if ( pRoom->description[0] == '\0' )
	add_buf( final, "{s[{tnone{s]\n\r" );
    else
    {
	add_buf( final, "\n\r{t" );
	add_buf( final, pRoom->description );
	add_buf( final, "\n\r" );
    }

    sprintf( buf, "{qName:       {s[{t%s{s]\n\r", pRoom->name );
    add_buf( final, buf );

    sprintf( buf, "            {s[{t%s{s]\n\r", double_color( pRoom->name ) );
    add_buf( final, buf );

    sprintf( buf, "{qArea:       {s[{t%5d{s] {t%s\n\r",
	pRoom->area->vnum, pRoom->area->name );
    add_buf( final, buf );

    sprintf( buf, "{qVnum:       {s[{t%5d{s]\n\r", pRoom->vnum );
    add_buf( final, buf );

    sprintf( buf, "{qSector:     {s[{t%s{s]\n\r",
	flag_string( sector_type, pRoom->sector_type ) );
    add_buf( final, buf );

    flags = flag_string( room_flags, pRoom->room_flags );
    flags = length_argument( flags, temp, 50 );

    sprintf( buf, "{qRoom flags: {s[{t%s{s]\n\r", temp );
    add_buf( final, buf );

    while ( *flags != '\0' )
    {
	flags = length_argument( flags, temp, 50 );
	sprintf( buf, "            {s[{t%s{s]\n\r", temp );
	add_buf( final, buf );
    }

    sprintf( buf, "{qHealth:     {s[{t%d{s]\n\r", pRoom->heal_rate );
    add_buf( final, buf );

    sprintf( buf, "{qMana:       {s[{t%d{s]\n\r", pRoom->mana_rate );
    add_buf( final, buf );
        
    if ( pRoom->music != NULL )
    {
	sprintf( buf, "{qMusic File: {s[{t%s{s]\n\r", pRoom->music );
	add_buf( final, buf );
    }

    sprintf( buf, "{qMax People: {s[{t%d{s] ({t0 is unlimited{s)\n\r",
	pRoom->max_people );
    add_buf( final, buf );

    if ( pRoom->extra_descr )
    {
	EXTRA_DESCR_DATA *ed;

	add_buf( final, "{qDesc Kwds:  {s[{t" );
	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    add_buf( final, ed->keyword );
	    if ( ed->next )
		add_buf( final, " " );
	}
	add_buf( final, "{s]\n\r" );
    }

    for ( door = 0; door < MAX_DIR; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = pRoom->exit[door] ) )
	{
	    char word[MAX_INPUT_LENGTH];
	    char reset_state[MAX_STRING_LENGTH];
	    char *state;
	    int i, length;

	    sprintf( buf, "\n\r{t-{q%-5s to {s[{t%5d{s: {t%s{s] {qKey: {s[{t%5d{s] ",
		capitalize( dir_name[door] ),
		pexit->u1.to_room ? pexit->u1.to_room->vnum : 0,
		end_string( pexit->u1.to_room ? pexit->u1.to_room->name : "Error", 20 ),
		pexit->key );
	    add_buf( final, buf );

	    strcpy( reset_state, flag_string( exit_flags, pexit->rs_flags ) );
	    state = flag_string( exit_flags, pexit->exit_info );
	    add_buf( final, " {qExit flags: {s[{t" );

	    for ( ; ; )
	    {
		state = one_argument( state, word );

		if ( word[0] == '\0' )
		{
		    add_buf( final, "{s]\n\r" );
		    break;
		}

		if ( str_infix( word, reset_state ) )
		{
		    length = strlen(word);
		    for ( i = 0; i < length; i++ )
			word[i] = UPPER( word[i] );
		}

		add_buf( final, word );
		if ( state[0] != '\0' )
		    add_buf( final, " " );
	    }

	    if ( pexit->keyword && pexit->keyword[0] != '\0' )
	    {
		sprintf( buf, "{qKwds: {s[{t%s{s]\n\r", pexit->keyword );
		add_buf( final, buf );
	    }

	    if ( pexit->description && pexit->description[0] != '\0' )
	    {
		sprintf( buf, "   {t%s", pexit->description );
		add_buf( final, buf );
	    }
	}
    }

    for ( dam = pRoom->room_damage; dam != NULL; dam = dam->next )
    {
	sprintf( buf, "{qDamage:     {s[{qType: {t%s{s] [{qRange: {t%d {qto {t%d{s] [{t%d%%{s]\n\r",
	    damage_mod_table[dam->damage_type].name, dam->damage_min,
	    dam->damage_max, dam->success );
	add_buf( final, buf );

	sprintf( buf, "{qVictim msg: {s[{t%s{s]\n\r",
	    dam->msg_victim ? dam->msg_victim : "none" );
	add_buf( final, buf );

	sprintf( buf, "{qRoom msg:   {s[{t%s{s]\n\r",
	    dam->msg_room ? dam->msg_room : "none" );
	add_buf( final, buf );
    }

    if ( pRoom->rprogs )
    {
	int cnt;

	sprintf( buf, "\n\r{qPrograms for{s[{t%5d{s]{q:\n\r", pRoom->vnum);
	add_buf( final, buf );

	for ( cnt = 0, list = pRoom->rprogs; list; list = list->next )
	{
	    if ( cnt == 0 )
	    {
		add_buf( final, "{q Number Vnum Trigger Phrase\n\r" );
		add_buf( final, "{t ------ ---- ------- ------\n\r" );
	    }

	    sprintf( buf, "{s[{t%5d{s] {t%4d %7s %s\n\r", cnt,
		list->vnum,prog_type_to_name(list->trig_type),
		list->trig_phrase);
	    add_buf( final, buf );
	    cnt++;
	}
    }

    add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

char *one_argument_case( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace( *argument ) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}

	*arg_first = *argument;
	arg_first++;
	argument++;
    }

    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

REDIT( redit_snake )
{
    ROOM_INDEX_DATA *in_room, *to_room;
    char arg[MAX_INPUT_LENGTH], arg_next[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
    sh_int count, move[MAX_INPUT_LENGTH], sector = 0;
    sh_int door, pos, health = 100, mana = 100, people = 0;
    long rflags = 0;
    int iHash, vnum;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: snake <directions> [name \"<name>\"] [sector <sector>] [flags \"<flags>\"] [health <hp gain>] [mana <mana gain>] [people <max_people>]\n\r", ch );
	return FALSE;
    }

    for ( count = 0; count < MAX_INPUT_LENGTH; )
    {
	if ( *argument == '\0' || *argument == ' ' )
	{
	    move[count] = -1;
	    break;
	}

	arg[0] = '\0';
	while( isdigit( *argument ) )
	    strncat( arg, argument++, 1 );

	pos = arg[0] == '\0' ? 1 : atoi( arg );

	if ( pos <= 0 )
	{
	    send_to_char( "Invalid directions.\n\r", ch );
	    return FALSE;
	}
	arg[0] = '\0';

	switch ( LOWER( *argument ) )
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

	for ( ; pos > 0 && count < MAX_INPUT_LENGTH; pos-- )
	{
	    move[count] = door;
	    count++;
	}

	(void) *argument++;
    }

    if ( ch->in_room->exit[move[0]] != NULL )
    {
	send_to_char( "Can not snake into an already existing room!\n\r", ch );
	return FALSE;
    }

    for ( ; ; )
    {
	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	    break;

	argument = one_argument_case( argument, arg_next );

	if ( arg_next[0] == '\0' )
	{
	    redit_snake( ch, "" );
	    return FALSE;
	}

	else if ( !str_cmp( arg, "name" ) )
	    strcpy( name, arg_next );

	else if ( !str_cmp( arg, "sector" ) )
	{
	    if ( ( sector = flag_value( sector_type, arg_next ) ) == NO_FLAG )
	    {
		send_to_char( "Invalid sector type.\n\r", ch );
		return FALSE;
	    }
	}

	else if ( !str_cmp( arg, "flags" ) )
	{
	    if ( ( rflags = flag_value( room_flags, arg_next ) ) == NO_FLAG )
	    {
		send_to_char( "Invalid room flags.\n\r", ch );
		return FALSE;
	    }
	}

	else if ( !str_cmp( arg, "health" ) )
	{
	    if ( !is_number( arg_next ) )
	    {
		send_to_char( "Invalid health rate.\n\r", ch );
		return FALSE;
	    }

	    health = atoi( arg_next );
	}

	else if ( !str_cmp( arg, "mana" ) )
	{
	    if ( !is_number( arg_next ) )
	    {
		send_to_char( "Invalid mana rate.\n\r", ch );
		return FALSE;
	    }

	    mana = atoi( arg_next );
	}

	else if ( !str_cmp( arg, "people" ) )
	{
	    if ( !is_number( arg_next ) )
	    {
		send_to_char( "Invalid people.\n\r", ch );
		return FALSE;
	    }

	    people = atoi( arg_next );
	}

	else
	{
	    redit_snake( ch, "" );
	    return FALSE;
	}
    }

    EDIT_ROOM( ch, in_room );

    for ( count = 0; count < MAX_INPUT_LENGTH && move[count] != -1; count++ )
    {
	for ( vnum = ch->in_room->area->min_vnum; ; vnum++ )
	{
	    if ( vnum > ch->in_room->area->max_vnum )
	    {
		printf_to_char( ch, "Ran out of useable vnums for this area, not all rooms added, %d rooms created.\n\r", count );
		return TRUE;
	    }

	    if ( get_room_index( vnum ) == NULL )
		break;
	}

	to_room			= new_room_index( );

	if ( name[0] != '\0' )
	{
	    free_string( to_room->name );
	    to_room->name	= str_dup( name );
	}

	to_room->vnum		= vnum;
	to_room->area		= ch->in_room->area;
	to_room->heal_rate	= health;
	to_room->mana_rate	= mana;
	to_room->room_flags	= rflags;
	to_room->sector_type	= sector;
	to_room->max_people	= people;
	iHash			= vnum % MAX_KEY_HASH;
	to_room->next		= room_index_hash[iHash];
	room_index_hash[iHash]	= to_room;

	in_room->exit[move[count]]		= new_exit( );
	in_room->exit[move[count]]->u1.to_room	= to_room;
	in_room->exit[move[count]]->orig_door	= move[count];

	to_room->exit[rev_dir[move[count]]]		= new_exit( );
	to_room->exit[rev_dir[move[count]]]->u1.to_room	= in_room;
	to_room->exit[rev_dir[move[count]]]->orig_door	= rev_dir[move[count]];

	in_room = to_room;
    }

    printf_to_char( ch, "%d rooms created and linked.\n\r", count );

    return TRUE;
}

REDIT( redit_music )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:   music <file name>\n\r", ch );
	send_to_char( "Syntax:   music remove  (To remove the file)\n\r",ch);
	return FALSE;
    }

    if ( !str_cmp( argument, "remove" ) || !str_cmp( argument, "none" ) )
    {
	free_string( pRoom->music );
	pRoom->music = NULL;
	send_to_char("Music in this room is now off.\n\r",ch);
	return TRUE;
    }

    free_string( pRoom->music );
    pRoom->music = str_dup( argument );

    send_to_char( "Predefined Background Music Changed!\n\r", ch );
    send_to_char( "Please refer to your builders guide when using this option.\n\r", ch);
    return TRUE;
}

bool change_exit( CHAR_DATA *ch, char *argument, int door )
{
    ROOM_INDEX_DATA *pRoom;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int  value;

    EDIT_ROOM(ch, pRoom);

    if ( ( value = flag_value( exit_flags, argument ) ) != NO_FLAG )
    {
	if ( !pRoom->exit[door] )
	{
	    send_to_char("Exit does not exist.\n\r",ch);
	    return FALSE;
	}

	TOGGLE_BIT(pRoom->exit[door]->rs_flags,  value);
	pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;
/*
	if ( pRoom->exit[door]->u1.to_room != NULL
	&&   pRoom->exit[door]->u1.to_room->exit[rev_dir[door]] != NULL
	&&   pRoom->exit[door]->u1.to_room->exit[rev_dir[door]]->u1.to_room == pRoom )
	{
	    TOGGLE_BIT( pRoom->exit[door]->u1.to_room->exit[rev_dir[door]]->rs_flags, value );
	    pRoom->exit[door]->u1.to_room->exit[rev_dir[door]]->exit_info =
	    pRoom->exit[door]->u1.to_room->exit[rev_dir[door]]->rs_flags;
	    SET_BIT( pRoom->exit[door]->u1.to_room->area->area_flags, AREA_CHANGED );
	}
*/
	send_to_char( "Exit flag toggled.\n\r", ch );
	SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
	return TRUE;
    }

    argument = one_argument( argument, command );
    one_argument( argument, arg );

    if ( command[0] == '\0' && argument[0] == '\0' )
    {
	move_char( ch, door, TRUE, FALSE );
	ch->desc->pEdit = (void *)ch->in_room;
	return FALSE;
    }

    if ( command[0] == '?' )
    {
	do_help( ch, "EXIT" );
	return FALSE;
    }

    if ( !str_cmp( command, "delete" ) )
    {
	ROOM_INDEX_DATA *pToRoom;
	int rev;
	
	if ( !pRoom->exit[door] )
	{
	    send_to_char( "REdit:  Cannot delete a null exit.\n\r", ch );
	    return FALSE;
	}

	rev = rev_dir[door];
	pToRoom = pRoom->exit[door]->u1.to_room;
	
	if ( pToRoom && pToRoom->exit[rev] )
	{
	    free_exit( pToRoom->exit[rev] );
	    pToRoom->exit[rev] = NULL;
	}

	free_exit( pRoom->exit[door] );
	pRoom->exit[door] = NULL;

	send_to_char( "Exit unlinked.\n\r", ch );
	SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
	if ( pToRoom )
	    SET_BIT( pToRoom->area->area_flags, AREA_CHANGED );
	return TRUE;
    }

    if ( !str_cmp( command, "link" ) )
    {
	EXIT_DATA *pExit;

	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    send_to_char( "Syntax:  [direction] link [vnum]\n\r", ch );
	    return FALSE;
	}

	value = atoi( arg );

	if ( !get_room_index( value ) )
	{
	    send_to_char( "REdit:  Cannot link to non-existant room.\n\r", ch );
	    return FALSE;
	}

	if ( !IS_BUILDER( ch, get_room_index( value )->area ) )
	{
	    send_to_char( "REdit:  Cannot link to that area.\n\r", ch );
	    return FALSE;
	}

	if ( get_room_index( value )->exit[rev_dir[door]] )
	{
	    send_to_char( "REdit:  Remote side's exit already exists.\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	{
	    pRoom->exit[door] = new_exit();
	}

	pRoom->exit[door]->u1.to_room = get_room_index( value );   /* ROM OLC */
	pRoom->exit[door]->orig_door = door;
	
	pRoom                   = get_room_index( value );
	door                    = rev_dir[door];
	pExit                   = new_exit();
	pExit->u1.to_room       = ch->in_room;
	pExit->orig_door	= door;
	pRoom->exit[door]       = pExit;

	send_to_char( "Two-way link established.\n\r", ch );
	SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
	return TRUE;
    }
        
    if ( !str_cmp( command, "dig" ) )
    {
	char buf[MAX_STRING_LENGTH];
	
	if ( arg[0] == '\0' )
	{
	    int vnum;

	    if ( ch->in_room == NULL )
	    {
		send_to_char("You are in a NULL room!\n\r",ch);
		return FALSE;
	    }

	    for ( vnum = ch->in_room->area->min_vnum;
		  vnum <= ch->in_room->area->max_vnum; vnum++ )
	    {
		if ( vnum > ch->in_room->area->max_vnum )
		{
		    send_to_char("Could not locate empty vnum.\n\r",ch);
		    return FALSE;
		}

		if ( get_room_index(vnum) == NULL )
		{
		    sprintf(buf,"%d",vnum);
		    strcpy(arg,buf);
		    break;
		}
	    }
	}

	else if ( !is_number( arg ) )
	{
	    send_to_char( "Syntax: [direction] dig <vnum>\n\r", ch );
	    return FALSE;
	}
	
	if ( redit_create( ch, arg ) )
	{
	    ch->desc->pEdit = (void *)ch->in_room;
	    sprintf( buf, "link %s", arg );
	    change_exit( ch, buf, door);
	}

	return TRUE;
    }

    if ( !str_cmp( command, "room" ) )
    {
	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    send_to_char( "Syntax:  [direction] room [vnum]\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	{
	    pRoom->exit[door] = new_exit();
	}

	value = atoi( arg );

	if ( !get_room_index( value ) )
	{
	    send_to_char( "REdit:  Cannot link to non-existant room.\n\r", ch );
	    return FALSE;
	}

	pRoom->exit[door]->u1.to_room = get_room_index( value );    /* ROM OLC */
	pRoom->exit[door]->orig_door = door;

	send_to_char( "One-way link established.\n\r", ch );
	SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
	return TRUE;
    }

    if ( !str_cmp( command, "key" ) )
    {
	if ( arg[0] == '\0' )
	{
	    send_to_char( "Syntax:  [direction] key [vnum] ( \"0\" or \"none\" to remove)\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	{
	    send_to_char( "Door does not exist.\n\r", ch );
	    return FALSE;
	}

	if ( !str_cmp( arg, "0" ) || !str_cmp( arg, "none" ) )
	{
	    send_to_char( "Exit key removed.\n\r", ch );
	    pRoom->exit[door]->key = 0;
	    return TRUE;
	}

	else if ( !is_number( arg ) )
	{
	    send_to_char( "Invalid number for key vnum.\n\r", ch );
	    return FALSE;
	}

	value = atoi( arg );

	if ( !get_obj_index( value ) )
	{
	    send_to_char( "REdit:  Item doesn't exist.\n\r", ch );
	    return FALSE;
	}

	if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY )
	{
	    send_to_char( "REdit:  Key doesn't exist.\n\r", ch );
	    return FALSE;
	}

	pRoom->exit[door]->key = value;

	send_to_char( "Exit key set.\n\r", ch );
	SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
	return TRUE;
    }

    if ( !str_cmp( command, "name" ) )
    {
	if ( arg[0] == '\0' )
	{
	    send_to_char( "Syntax:  [direction] name [string]\n\r", ch );
	    send_to_char( "         [direction] name none\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	   {
	   	send_to_char("Invalid exit.\n\r",ch);
	   	return FALSE;
	   }


	free_string( pRoom->exit[door]->keyword );
	if (str_cmp(arg,"none"))
		pRoom->exit[door]->keyword = str_dup( arg );
	else
		pRoom->exit[door]->keyword = str_dup( "" );

	send_to_char( "Exit name set.\n\r", ch );
	return TRUE;
    }

    if ( !str_prefix( command, "description" ) )
    {
	if ( arg[0] == '\0' )
	{
	   if ( !pRoom->exit[door] )
	   {
	   	send_to_char("Salida no existe.\n\r",ch);
	   	return FALSE;
	   }

	    string_append( ch, &pRoom->exit[door]->description );
	    return TRUE;
	}

	send_to_char( "Syntax:  [direction] desc\n\r", ch );
	return FALSE;
    }

    return FALSE;
}



REDIT( redit_north )
{
    if ( change_exit( ch, argument, DIR_NORTH ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_south )
{
    if ( change_exit( ch, argument, DIR_SOUTH ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_east )
{
    if ( change_exit( ch, argument, DIR_EAST ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_west )
{
    if ( change_exit( ch, argument, DIR_WEST ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_up )
{
    if ( change_exit( ch, argument, DIR_UP ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_down )
{
    if ( change_exit( ch, argument, DIR_DOWN ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_ed )
{
    ROOM_INDEX_DATA *pRoom;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, command );
    one_argument( argument, keyword );

    if ( command[0] == '\0' || keyword[0] == '\0' )
    {
	send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
	send_to_char( "         ed edit [keyword]\n\r", ch );
	send_to_char( "         ed delete [keyword]\n\r", ch );
	send_to_char( "         ed format [keyword]\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( command, "add" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
	    return FALSE;
	}

	ed			=   new_extra_descr();
	ed->keyword		=   str_dup( keyword );
	ed->description		=   str_dup( "" );
	ed->next		=   pRoom->extra_descr;
	pRoom->extra_descr	=   ed;

	string_append( ch, &ed->description );

	return TRUE;
    }


    if ( !str_cmp( command, "edit" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed edit [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	}

	if ( !ed )
	{
	    send_to_char( "REdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	string_append( ch, &ed->description );

	return TRUE;
    }


    if ( !str_cmp( command, "delete" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed delete [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	    ped = ed;
	}

	if ( !ed )
	{
	    send_to_char( "REdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	if ( !ped )
	    pRoom->extra_descr = ed->next;
	else
	    ped->next = ed->next;

	free_extra_descr( ed );

	send_to_char( "Extra description deleted.\n\r", ch );
	return TRUE;
    }


    if ( !str_cmp( command, "format" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed format [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	}

	if ( !ed )
	{
	    send_to_char( "REdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	ed->description = format_string( ed->description, atoi( argument ) );

	send_to_char( "Extra description formatted.\n\r", ch );
	return TRUE;
    }

    redit_ed( ch, "" );
    return FALSE;
}

REDIT( redit_delete )
{
    ROOM_INDEX_DATA *pRoom, *pRoom2;
    CHAR_DATA *wch, *wch_next;
    int iHash, match = 0, pos, vnum;

    EDIT_ROOM( ch, pRoom );

    if ( argument[0] == '\0' || !is_number( argument )
    ||   atoi( argument ) != pRoom->vnum )
    {
	send_to_char( "Argument must match the room you are editing.\n\r", ch );
	return FALSE;
    }

    if ( ( pRoom2 = get_room_index( ROOM_VNUM_ALTAR ) ) == NULL )
    {
	send_to_char( "ERROR: ROOM_VNUM_ALTAR not found!\n\r", ch );
	return FALSE;
    }

    check_olc_delete( ch );

    for ( wch = pRoom->people; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next_in_room;

	send_to_char( "Your room was just deleted!\n\r", wch );

	if ( IS_NPC( wch ) )
	    extract_char( wch, TRUE );
	else
	{
	    char_from_room( wch );
	    char_to_room( wch, pRoom2 );
	}
    }

    for ( vnum = 0; match < top_room; vnum++ )
    {
	if ( ( pRoom2 = get_room_index( vnum ) ) != NULL )
	{
	    match++;

	    for ( pos = 0; pos < MAX_DIR; pos++ )
	    {
		if ( pRoom2->exit[pos] != NULL
		&&   pRoom2->exit[pos]->u1.to_room == pRoom )
		{
		    free_exit( pRoom2->exit[pos] );
		    pRoom2->exit[pos] = NULL;
		    SET_BIT( pRoom2->area->area_flags, AREA_CHANGED );
		}
	    }
	}
    }

    SET_BIT( pRoom->area->area_flags, AREA_CHANGED );

    iHash = pRoom->vnum % MAX_KEY_HASH;

    pRoom2 = room_index_hash[iHash];

    if ( pRoom2->next == NULL )
	room_index_hash[iHash] = NULL;

    else if ( pRoom2 == pRoom )
	room_index_hash[iHash] = pRoom->next;

    else
    {
	for ( ; pRoom2 != NULL; pRoom2 = pRoom2->next )
	{
	    if ( pRoom2->next == pRoom )
	    {
		pRoom2->next = pRoom->next;
		break;
	    }
	}
    }

    free_room_index( pRoom );

    send_to_char( "Room deleted.\n\r", ch );
    return TRUE;
}

REDIT( redit_create )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int value;
    int iHash;
    
    EDIT_ROOM(ch, pRoom);

    value = atoi( argument );

    if ( argument[0] == '\0' || value <= 0 )
    {
	send_to_char( "Syntax:  create [vnum > 0]\n\r", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );
    if ( !pArea )
    {
	send_to_char( "REdit:  That vnum is not assigned an area.\n\r", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "REdit:  Vnum in an area you cannot build in.\n\r", ch );
	return FALSE;
    }

    if ( get_room_index( value ) )
    {
	send_to_char( "REdit:  Room vnum already exists.\n\r", ch );
	return FALSE;
    }

    SET_BIT( pArea->area_flags, AREA_CHANGED );

    pRoom			= new_room_index();
    pRoom->area			= pArea;
    pRoom->vnum			= value;

    iHash			= value % MAX_KEY_HASH;
    pRoom->next			= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoom;

    ch->desc->pEdit		= (void *)pRoom;
    ch->desc->editor		= ED_ROOM;

    send_to_char( "Room created.\n\r", ch );
    return TRUE;
}


REDIT( redit_copy )
{
    ROOM_INDEX_DATA *pRoom, *pRoom2;
    EXTRA_DESCR_DATA *ed, *ed_next;
    int vnum;

    if ( argument[0] == '\0' )
    {
	send_to_char("Syntax: copy <vnum> \n\r",ch);
	return FALSE;
    }

    if ( !is_number(argument) )
    {
	send_to_char("REdit: You must enter a number (vnum).\n\r",ch);
	return FALSE;
    }
    else
    {
	vnum = atoi(argument);
	if( !( pRoom2 = get_room_index(vnum) ) )
	{
	    send_to_char("REdit: That room does not exist.\n\r",ch);
	    return FALSE;
	}
    }

    if ( !IS_BUILDER( ch, pRoom2->area ) )
    {
	send_to_char( "REdit:  Vnum in an area you cannot build in.\n\r", ch );
	return FALSE;
    }

    EDIT_ROOM( ch, pRoom );

    free_string( pRoom->description );
    pRoom->description	= str_dup( pRoom2->description );    

    free_string( pRoom->name );
    pRoom->name		= str_dup( pRoom2->name );

    pRoom->sector_type	= pRoom2->sector_type;
    pRoom->room_flags	= pRoom2->room_flags;
    pRoom->heal_rate	= pRoom2->heal_rate;
    pRoom->mana_rate	= pRoom2->mana_rate;

    for ( ed = pRoom->extra_descr; ed != NULL; ed = ed_next )
    {
	ed_next = ed->next;
	free_extra_descr( ed );
    }
    pRoom->extra_descr = NULL;

    for ( ed = pRoom2->extra_descr; ed != NULL; ed = ed->next )
    {
	ed_next			= new_extra_descr( );
	ed_next->keyword	= str_dup( ed->keyword );
	ed_next->description	= str_dup( ed->description );
	ed_next->next		= pRoom->extra_descr;
	pRoom->extra_descr	= ed_next;
    }

    send_to_char( "Room info copied.", ch );
    return TRUE;
}


REDIT( redit_name )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  name [name]\n\r", ch );
	return FALSE;
    }

    free_string( pRoom->name );
    pRoom->name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch );
    return TRUE;
}


REDIT( redit_desc )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
	string_append( ch, &pRoom->description );
	return TRUE;
    }

    send_to_char( "Syntax:  desc\n\r", ch );
    return FALSE;
}

REDIT( redit_heal )
{
    ROOM_INDEX_DATA *pRoom;
    
    EDIT_ROOM(ch, pRoom);
    
    if (is_number(argument))
       {
          pRoom->heal_rate = atoi ( argument );
          send_to_char ( "Health rate set.\n\r", ch);
          return TRUE;
       }

    send_to_char ( "Syntax : health <#xnumber>\n\r", ch);
    return FALSE;
}       

REDIT( redit_mana )
{
    ROOM_INDEX_DATA *pRoom;
    
    EDIT_ROOM(ch, pRoom);
    
    if (is_number(argument))
       {
          pRoom->mana_rate = atoi ( argument );
          send_to_char ( "Mana rate set.\n\r", ch);
          return TRUE;
       }

    send_to_char ( "Syntax : mana <#xnumber>\n\r", ch);
    return FALSE;
}       

REDIT( redit_max_people )
{
    ROOM_INDEX_DATA *pRoom;
    
    EDIT_ROOM( ch, pRoom );
    
    if ( is_number( argument ) )
    {
	pRoom->max_people = atoi ( argument );
	send_to_char( "Max people set, use 0 to reset it.\n\r", ch );
	return TRUE;
    }

    send_to_char( "Syntax : max_people <#xnumber>\n\r", ch );
    return FALSE;
}       

REDIT( redit_format )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    pRoom->description = format_string( pRoom->description, atoi( argument ) );

    send_to_char( "String formatted.\n\r", ch );
    return TRUE;
}


REDIT( redit_sector )
{
    ROOM_INDEX_DATA *room;
    int value;

    EDIT_ROOM(ch, room);

    if ( (value = flag_value( sector_type, argument )) == NO_FLAG )
    {
	send_to_char( "Syntax: sector [flags]\n\r", ch );
	send_to_char( "For a List '? sector'\n\r",ch);
	return FALSE;
    }

    room->sector_type = value;
    send_to_char( "Sector flags toggled.\n\r", ch );
    return TRUE;
}

REDIT( redit_room )
{
    ROOM_INDEX_DATA *room;
    int value;

    EDIT_ROOM(ch, room);

    if ( (value = flag_value( room_flags, argument )) == NO_FLAG )
    {
	send_to_char( "Syntax: room [flags]\n\r", ch );
	send_to_char( "For a List '? room'\n\r",ch);
	return FALSE;
    }

    TOGGLE_BIT(room->room_flags, value);
    send_to_char( "Room flags toggled.\n\r", ch );
    return TRUE;
}

REDIT( redit_mreset )
{
    ROOM_INDEX_DATA	*pRoom;
    MOB_INDEX_DATA	*pMobIndex;
    CHAR_DATA		*newmob;
    char		buf[MAX_STRING_LENGTH];
    char		arg [ MAX_INPUT_LENGTH ];
    char		arg2 [ MAX_INPUT_LENGTH ];
    char		arg3 [ MAX_INPUT_LENGTH ];

    RESET_DATA		*pReset;
    char		output [ MAX_STRING_LENGTH ];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg[0] == '\0' || !is_number( arg ) )
    {
	send_to_char ( "Syntax:  mreset <vnum> <max_world #x> <max_room #x> <percentage>\n\r", ch );
	return FALSE;
    }

    if ( !( pMobIndex = get_mob_index( atoi( arg ) ) ) )
    {
	send_to_char( "REdit: No mobile has that vnum.\n\r", ch );
	return FALSE;
    }

    if ( pMobIndex->area != pRoom->area )
    {
	send_to_char( "REdit: No such mobile in this area.\n\r", ch );
	return FALSE;
    }

    pReset              = new_reset_data();
    pReset->command	= 'M';
    pReset->arg1	= pMobIndex->vnum;
    pReset->arg2	= is_number( arg2 ) ? atoi( arg2 ) : MAX_MOB;
    pReset->arg3	= pRoom->vnum;
    pReset->arg4	= is_number( arg3 ) ? atoi (arg3) : 1;
    pReset->percent	= is_number( argument ) ?
			  URANGE( 1, atoi( argument ), 100 ) : 100;
    add_reset( pRoom, pReset, 0/* Last slot*/ );

    /*
     * Create the mobile.
     */
    newmob = create_mobile( pMobIndex );
    char_to_room( newmob, pRoom );

    sprintf( output, "%s (%d) has been loaded and added to resets at %d%%.\n\r"
	"There will be a maximum of %d loaded to this room.\n\r",
	pMobIndex->short_descr, pMobIndex->vnum, pReset->percent,
	pReset->arg2 );
    send_to_char( output, ch );
    act( "$n has created $N!", ch, NULL, newmob, TO_ROOM,POS_RESTING);
    sprintf(buf,"resets mobile <%d|%s|%s> to room %d.",
    newmob->pIndexData->vnum,newmob->name,newmob->short_descr,ch->in_room->vnum);
    parse_logs( ch, "build", buf );
    return TRUE;
}

struct wear_type
{
    int	wear_loc;
    int	wear_bit;
};

const struct wear_type wear_table[] =
{
    {	WEAR_NONE,	ITEM_TAKE		},
    {	WEAR_LIGHT,	ITEM_LIGHT		},
    {	WEAR_FINGER_L,	ITEM_WEAR_FINGER	},
    {	WEAR_FINGER_R,	ITEM_WEAR_FINGER	},
    {	WEAR_NECK_1,	ITEM_WEAR_NECK		},
    {	WEAR_NECK_2,	ITEM_WEAR_NECK		},
    {	WEAR_BODY,	ITEM_WEAR_BODY		},
    {	WEAR_HEAD,	ITEM_WEAR_HEAD		},
    {	WEAR_LEGS,	ITEM_WEAR_LEGS		},
    {	WEAR_FEET,	ITEM_WEAR_FEET		},
    {	WEAR_HANDS,	ITEM_WEAR_HANDS		},
    {	WEAR_ARMS,	ITEM_WEAR_ARMS		},
    {	WEAR_SHIELD,	ITEM_WEAR_SHIELD	},
    {	WEAR_ABOUT,	ITEM_WEAR_ABOUT		},
    {	WEAR_WAIST,	ITEM_WEAR_WAIST		},
    {	WEAR_WRIST_L,	ITEM_WEAR_WRIST		},
    {	WEAR_WRIST_R,	ITEM_WEAR_WRIST		},
    {	WEAR_WIELD,	ITEM_WIELD		},
    {	WEAR_HOLD,	ITEM_HOLD		},
    {	WEAR_ANKLE_L,	ITEM_WEAR_ANKLE		},
    {	WEAR_ANKLE_R,	ITEM_WEAR_ANKLE		},
    {	WEAR_EAR_L,	ITEM_WEAR_EAR		},
    {   WEAR_EAR_R,	ITEM_WEAR_EAR		},
    {	WEAR_CHEST,	ITEM_WEAR_CHEST		},
    {	WEAR_FLOAT,	ITEM_WEAR_FLOAT		},
    {   WEAR_EYES,	ITEM_WEAR_EYES		},
    {   WEAR_FACE,	ITEM_WEAR_FACE		},
    {	WEAR_CLAN,	ITEM_WEAR_CLAN		},
    {	WEAR_SECONDARY,	ITEM_WIELD		},
    {	WEAR_BACK,	ITEM_WEAR_BACK		},
    {	NO_FLAG,	NO_FLAG			}
};

/*****************************************************************************
 Name:		wear_loc
 Purpose:	Returns the location of the bit that matches the count.
 		1 = first match, 2 = second match etc.
 Called by:	oedit_reset(olc_act.c).
 ****************************************************************************/
int wear_loc(int bits, int count)
{
    int flag;
 
    for (flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++)
    {
        if ( IS_SET(bits, wear_table[flag].wear_bit) && --count < 1)
            return wear_table[flag].wear_loc;
    }
 
    return NO_FLAG;
}

/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit(int loc)
{
    int flag;
 
    for (flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++)
    {
        if ( loc == wear_table[flag].wear_loc )
            return wear_table[flag].wear_bit;
    }
 
    return 0;
}

REDIT( redit_oreset )
{
    ROOM_INDEX_DATA	*pRoom;
    OBJ_INDEX_DATA	*pObjIndex;
    OBJ_DATA		*newobj;
    OBJ_DATA		*to_obj;
    CHAR_DATA		*to_mob;
    char		buf [MAX_STRING_LENGTH];
    char		arg1 [ MAX_INPUT_LENGTH ];
    char		arg2 [ MAX_INPUT_LENGTH ];
    char		arg3 [ MAX_INPUT_LENGTH ];

    RESET_DATA		*pReset;
    char		output [ MAX_STRING_LENGTH ];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
	send_to_char ( "Syntax:  oreset <vnum> <args>\n\r", ch );
	send_to_char ( "        -no_args               = into room\n\r", ch );
	send_to_char ( "        -<obj_name>            = into obj\n\r", ch );
	send_to_char ( "        -<mob_name> <wear_loc> = into mob\n\r", ch );
	return FALSE;
    }

    if ( !( pObjIndex = get_obj_index( atoi( arg1 ) ) ) )
    {
	send_to_char( "REdit: No object has that vnum.\n\r", ch );
	return FALSE;
    }

    if ( pObjIndex->area != pRoom->area )
    {
	send_to_char( "REdit: No such object in this area.\n\r", ch );
	return FALSE;
    }

    if ( arg2[0] == '\0' || is_number( arg2 ) )
    {
	pReset		= new_reset_data();
	pReset->command	= 'O';
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= 0;
	pReset->arg3	= pRoom->vnum;
	pReset->arg4	= 0;
	pReset->percent	= is_number( arg2 ) ?
			  URANGE( 1, atoi ( arg2 ), 100 ) : 100;
	add_reset( pRoom, pReset, 0/* Last slot*/ );

	newobj = create_object( pObjIndex );
	obj_to_room( newobj, pRoom );

	sprintf( output, "%s (%d) has been loaded and added to resets.\n\r",
	    pObjIndex->short_descr,
	    pObjIndex->vnum );
        send_to_char( output, ch );
        sprintf(buf,"resets object <%d|%s|%s> to room <%d>.",
            pObjIndex->vnum,pObjIndex->name,
	    pObjIndex->short_descr,ch->in_room->vnum);
	parse_logs( ch, "build", buf );
    }

    else if ( ( arg3[0] == '\0' || is_number( arg3 ) )
    && ( ( to_obj = get_obj_list( ch, arg2, pRoom->contents ) ) != NULL ) )
    {
	pReset		= new_reset_data();
	pReset->command	= 'P';
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= 1;
	pReset->arg3	= to_obj->pIndexData->vnum;
	pReset->arg4	= 1;
	pReset->percent	= is_number( arg3 ) ?
			  URANGE( 1, atoi( arg3 ), 100 ) :  100;
	add_reset( pRoom, pReset, 0/* Last slot*/ );

	newobj = create_object( pObjIndex );
	newobj->cost = 0;
	obj_to_obj( newobj, to_obj );

	sprintf( output, "%s (%d) has been loaded into "
	    "%s (%d) and added to resets.\n\r",
	    newobj->short_descr,
	    newobj->pIndexData->vnum,
	    to_obj->short_descr,
	    to_obj->pIndexData->vnum );
	send_to_char( output, ch );
        sprintf(buf,"resets object <%d|%s|%s> in object <%d|%s|%s>.",
            newobj->pIndexData->vnum,newobj->pIndexData->name,
	    newobj->pIndexData->short_descr,to_obj->pIndexData->vnum,
            to_obj->pIndexData->name,to_obj->pIndexData->short_descr);
	parse_logs( ch, "build", buf );
    }

    else if ( ( to_mob = get_char_room( ch, NULL, arg2 ) ) != NULL )
    {
	int	wear_loc;

	if ( (wear_loc = flag_value( wear_loc_flags, arg3 )) == NO_FLAG )
	{
	    send_to_char( "REdit: Invalid wear_loc.  '? wear-loc'\n\r", ch );
	    return FALSE;
	}

	if ( !IS_SET( pObjIndex->wear_flags, wear_bit(wear_loc) ) 
	&&   wear_bit(wear_loc != WEAR_HOLD) )
	{
		sprintf( output, "%s (%d) has wear flags: [%s]\n\r",
		    pObjIndex->short_descr, pObjIndex->vnum,
		    flag_string( wear_flags, pObjIndex->wear_flags ) );
		send_to_char( output, ch );
		return FALSE;
	}

	if ( get_eq_char( to_mob, wear_loc ) && ( wear_loc != WEAR_NONE ) )
	{
	    send_to_char( "REdit:  Object already equipped.\n\r", ch );
	    return FALSE;
	}

	pReset		= new_reset_data();
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= wear_loc;
	if ( pReset->arg2 == WEAR_NONE )
	    pReset->command = 'G';
	else
	    pReset->command = 'E';
	pReset->arg3	= wear_loc;
	pReset->percent	= is_number( argument ) ?
			  URANGE( 1, atoi( argument ), 100 ) : 100;

	add_reset( pRoom, pReset, 0/* Last slot*/ );

        newobj = create_object( pObjIndex );

	if ( to_mob->pIndexData->pShop )	/* Shop-keeper? */
	{
	    if ( pReset->arg2 == WEAR_NONE )
		SET_BIT( newobj->extra_flags, ITEM_INVENTORY );
	}

	obj_to_char( newobj, to_mob );
	if ( pReset->command == 'E' )
	    equip_char( to_mob, newobj, pReset->arg3 );

	sprintf( output, "%s (%d) has been loaded "
	    "%s of %s (%d) and added to resets.\n\r",
	    pObjIndex->short_descr,
	    pObjIndex->vnum,
	    flag_string( wear_loc_strings, pReset->arg3 ),
	    to_mob->short_descr,
	    to_mob->pIndexData->vnum );
	send_to_char( output, ch );
        sprintf(buf,"resets object <%d|%s|%s> on mobile <%d|%s|%s>.",
            pObjIndex->vnum,pObjIndex->name, pObjIndex->short_descr,to_mob->pIndexData->vnum,
            to_mob->name,to_mob->short_descr);
	parse_logs( ch, "build", buf );
    }
    else	/* Display Syntax */
    {
	send_to_char( "REdit:  That mobile isn't here.\n\r", ch );
	return FALSE;
    }

    act( "$n has created $p!", ch, newobj, NULL, TO_ROOM,POS_RESTING);
    return TRUE;
}

char * show_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *obj )
{
    static char buf[MAX_STRING_LENGTH];

    switch( obj->item_type )
    {
	default:
	    return "";
	    break;
            
	case ITEM_SLOTS:
	    sprintf( buf, "{s[{tv0{s] {qSilver Jackpot {s[{t%d{s]\n"
			  "{s[{tv1{s] {qGold   Jackpot {s[{t%d{s]\n"
			  "{s[{tv2{s] {qPlat   Jackpot {s[{t%d{s]\n"
			  "{s[{tv3{s] {qTotal  Bars    {s[{t%d{s]\n\r",
		obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    return buf;
	    break;

	case ITEM_COMPONENT_BREW:
	case ITEM_COMPONENT_SCRIBE:
	    sprintf( buf, "{s[{tv0{s] {qMax Bind:   {s[{t%d{s]\n\r",
		obj->value[0] );
	    return buf;
	    break;

	case ITEM_LIGHT:
            if ( obj->value[2] == -1 || obj->value[2] == 999 ) /* ROM OLC */
		sprintf( buf, "{s[{tv2{s] {qLight:  Infinite{s[{t-1{s]\n\r" );
            else
		sprintf( buf, "{s[{tv2{s] {qLight:  {s[{t%d{s]\n\r",
		    obj->value[2] );
	    return buf;
	    break;

	case ITEM_WAND:
	case ITEM_STAFF:
            sprintf( buf,
		"{s[{tv0{s] {qLevel:          {s[{t%d{s]\n\r"
		"{s[{tv1{s] {qCharges Total:  {s[{t%d{s]\n\r"
		"{s[{tv2{s] {qCharges Left:   {s[{t%d{s]\n\r"
		"{s[{tv3{s] {qSpell:          {s[{t%s{s]\n\r",
		obj->value[0],
		obj->value[1],
		obj->value[2],
		obj->value[3] != -1 ? skill_table[obj->value[3]].name
		                    : "none" );
	    return buf;
	    break;

	case ITEM_PORTAL:
	    sprintf( buf,
	        "{s[{tv0{s] {qCharges:        {s[{t%d{s]\n\r"
	        "{s[{tv1{s] {qExit Flags:     {s[{t%s{s]\n\r",
	        obj->value[0],
	        flag_string( exit_flags, obj->value[1]) );
	    sprintf( buf, "%s"
	        "{s[{tv2{s] {qPortal Flags:   {s[{t%s{s]\n\r"
	        "{s[{tv3{s] {qGoes to (vnum): {s[{t%d{s] [{t%s{s]\n\r",
	        buf, flag_string( portal_flags , obj->value[2]),
		obj->value[3],
		get_room_index( obj->value[3] ) == NULL ? "! NULL ROOM !" :
		get_room_index( obj->value[3] )->name );
	    return buf;
	    break;
	    
	case ITEM_FURNITURE:          
	    sprintf( buf,
	        "{s[{tv0{s] {qMax people:      {s[{t%d{s]\n\r"
	        "{s[{tv1{s] {qMax weight:      {s[{t%d{s]\n\r"
	        "{s[{tv2{s] {qFurniture Flags: {s[{t%s{s]\n\r"
	        "{s[{tv3{s] {qHeal bonus:      {s[{t%d{s]\n\r"
	        "{s[{tv4{s] {qMana bonus:      {s[{t%d{s]\n\r",
	        obj->value[0], obj->value[1],
	        flag_string( furniture_flags, obj->value[2]),
	        obj->value[3], obj->value[4] );
	    return buf;
	    break;
         
        case ITEM_DEMON_STONE:
	    sprintf( buf, "{s[{tv0{s] {qCharges: {s[{t%d{s]\n\r",
                 obj->value[0] );
	    return buf;
            break; 

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
            sprintf( buf,
		"{s[{tv0{s] {qLevel:  {s[{t%d{s]\n\r"
		"{s[{tv1{s] {qSpell:  {s[{t%s{s]\n\r"
		"{s[{tv2{s] {qSpell:  {s[{t%s{s]\n\r"
		"{s[{tv3{s] {qSpell:  {s[{t%s{s]\n\r"
		"{s[{tv4{s] {qSpell:  {s[{t%s{s]\n\r",
		obj->value[0],
		obj->value[1] != -1 ? skill_table[obj->value[1]].name
		                    : "none",
		obj->value[2] != -1 ? skill_table[obj->value[2]].name
                                    : "none",
		obj->value[3] != -1 ? skill_table[obj->value[3]].name
		                    : "none",
		obj->value[4] != -1 ? skill_table[obj->value[4]].name
		                    : "none" );
	    return buf;
	    break;

        case ITEM_ARMOR:
	    sprintf( buf,
		"{s[{tv0{s] {qAc pierce       {s[{t%d{s]\n\r"
		"{s[{tv1{s] {qAc bash         {s[{t%d{s]\n\r"
		"{s[{tv2{s] {qAc slash        {s[{t%d{s]\n\r"
		"{s[{tv3{s] {qAc exotic       {s[{t%d{s]\n\r",
		obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    return buf;
	    break;

	case ITEM_WEAPON:
            sprintf( buf,	"{s[{tv0{s] {qWeapon Type:    {s[{t%s{s]\n\r",
		flag_string( weapon_class, obj->value[0] ) );
	    sprintf( buf, "%s"
			  "{s[{tv1{s] {qNumber of dice: {s[{t%d{s]\n\r"
			  "{s[{tv2{s] {qType of dice:   {s[{t%d{s] <{qAve: {t%d{s>\n\r"
			  "{s[{tv3{s] {qDamage Noun:    {s[{t%s{s]\n\r"
			  "{s[{tv4{s] {qWeapon Flags:   {s[{t%s{s]\n\r",
		buf, obj->value[1], obj->value[2],
		( ( 1 + obj->value[2] ) * obj->value[1] / 2 ),
		attack_table[obj->value[3]].name,
		flag_string( weapon_type2,  obj->value[4] ) );
	    return buf;
	    break;

	case ITEM_CONTAINER:
        case       ITEM_PIT:
	    sprintf( buf,
		"{s[{tv0{s] {qWeight:     {s[{t%d kg{s]\n\r"
		"{s[{tv1{s] {qFlags:      {s[{t%s{s]\n\r"
		"{s[{tv2{s] {qKey:        {s[{t%d{s] [{t%s{s]\n\r"
		"{s[{tv3{s] {qCapacity    {s[{t%d{s]\n\r"
		"{s[{tv4{s] {qWeight Mult {s[{t%d{s]\n\r",
		obj->value[0],
		flag_string( container_flags, obj->value[1] ),
                obj->value[2],
                get_obj_index(obj->value[2])
                    ? get_obj_index(obj->value[2])->short_descr
                    : "none",
                obj->value[3],
                obj->value[4] );
	    return buf;
	    break;
           
      
	case ITEM_DRINK_CON:
	    sprintf( buf,
	        "{s[{tv0{s] {qLiquid Total: {s[{t%d{s]\n\r"
	        "{s[{tv1{s] {qLiquid Left:  {s[{t%d{s]\n\r"
	        "{s[{tv2{s] {qLiquid:       {s[{t%s{s]\n\r"
	        "{s[{tv3{s] {qPoisoned:     {s[{t%s{s]\n\r",
	        obj->value[0], obj->value[1],
	        liq_table[obj->value[2]].liq_name,
	        obj->value[3] != 0 ? "Yes" : "No" );
	    return buf;
	    break;

	case ITEM_FOUNTAIN:
	    sprintf( buf,
	        "{s[{tv0{s] {qLiquid Total: {s[{t%d{s]\n\r"
	        "{s[{tv1{s] {qLiquid Left:  {s[{t%d{s]\n\r"
	        "{s[{tv2{s] {qLiquid:       {s[{t%s{s]\n\r",
	        obj->value[0],
	        obj->value[1],
	        liq_table[obj->value[2]].liq_name );
	    return buf;
	    break;
	        
	case ITEM_FOOD:
	    sprintf( buf,
		"{s[{tv0{s] {qFood hours: {s[{t%d{s]\n\r"
		"{s[{tv1{s] {qFull hours: {s[{t%d{s]\n\r"
		"{s[{tv3{s] {qPoisoned:   {s[{t%s{s]\n\r",
		obj->value[0],
		obj->value[1],
		obj->value[3] != 0 ? "Yes" : "No" );
	    return buf;
	    break;

	case ITEM_MONEY:
            sprintf( buf, "{s[{tv0{s] {qGold:   {s[{t%d{s]\n\r",
		obj->value[0] );
	    return buf;
	    break;

	case ITEM_ENGINEER_TOOL:
	    sprintf( buf, "{s[{tv0{s] {qEngineer: {s[{t%s{s]\n\r",
		obj->value[0] == 0 ?
		"build" : obj->value[0] == 1 ? "grenade" : "tinker" );
	    return buf;
	    break;

	case ITEM_TRAP:
	    sprintf( buf,	"{s[{tv0{s] {qTrap Type   {s[{t%s{s]\n\r"
				"{s[{tv1{s] {qDamage Type {s[{t%s{s]\n\r"
				"{s[{tv2{s] {qMin Damage  {s[{t%d{s]\n\r"
				"{s[{tv3{s] {qMax Damage  {s[{t%d{s]\n\r"
				"{s[{tv4{s] {qTrap Timer  {s[{t%d{s]\n\r",
		trap_type_table[obj->value[0]].name,
		damage_mod_table[obj->value[1]].name, obj->value[2],
		obj->value[3], obj->value[4] );
	    return buf;
	    break;

	case ITEM_MINE:
	    break;
    }

    return "";
}

bool set_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, int value_num, char *argument)
{
    switch( pObj->item_type )
    {
        default:
            break;
            
	case ITEM_SLOTS:
	    pObj->value[value_num] = atoi( argument );
	    break;

        case ITEM_COMPONENT_BREW:
        case ITEM_COMPONENT_SCRIBE:
	    switch ( value_num )
	    {
	        default:
		    send_to_char("Syntax: v0 (value)\n\r.",ch);
	            return FALSE;

	        case 0:
		    send_to_char("Max spell bind set.\n\r",ch);
		    pObj->value[0] = URANGE(1, atoi( argument ), 3);
		    break;
	    }
	    break;


        case ITEM_LIGHT:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_LIGHT" );
	            return FALSE;
	        case 2:
	            send_to_char( "HOURS OF LIGHT SET.\n\r\n\r", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	    }
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_STAFF_WAND" );
	            return FALSE;
	        case 0:
	            send_to_char( "SPELL LEVEL SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "TOTAL NUMBER OF CHARGES SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "CURRENT NUMBER OF CHARGES SET.\n\r\n\r", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "SPELL TYPE SET.\n\r", ch );
	            pObj->value[3] = skill_lookup( argument );
	            break;
	    }
            break;
        
        case ITEM_DEMON_STONE:
            switch (value_num )
            {
                default:
                    do_help( ch, "ITEM_DEMON_STONE" );
                    return FALSE;
                case 0:
                    send_to_char( "CURRENT NUMBER OF CHARGES SET.\n\r\n\r", ch );
                    pObj->value[0] = atoi( argument );
                    break;
            }
            break;

        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_SCROLL_POTION_PILL" );
	            return FALSE;
	        case 0:
	            send_to_char( "SPELL LEVEL SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "SPELL TYPE 1 SET.\n\r\n\r", ch );
	            pObj->value[1] = skill_lookup( argument );
	            break;
	        case 2:
	            send_to_char( "SPELL TYPE 2 SET.\n\r\n\r", ch );
	            pObj->value[2] = skill_lookup( argument );
	            break;
	        case 3:
	            send_to_char( "SPELL TYPE 3 SET.\n\r\n\r", ch );
	            pObj->value[3] = skill_lookup( argument );
	            break;
	        case 4:
	            send_to_char( "SPELL TYPE 4 SET.\n\r\n\r", ch );
	            pObj->value[4] = skill_lookup( argument );
	            break;
 	    }
	    break;

        case ITEM_ARMOR:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_ARMOR" );
		    return FALSE;
	        case 0:
		    send_to_char( "AC PIERCE SET.\n\r\n\r", ch );
		    pObj->value[0] = atoi( argument );
		    break;
	        case 1:
		    send_to_char( "AC BASH SET.\n\r\n\r", ch );
		    pObj->value[1] = atoi( argument );
		    break;
	        case 2:
		    send_to_char( "AC SLASH SET.\n\r\n\r", ch );
		    pObj->value[2] = atoi( argument );
		    break;
	        case 3:
		    send_to_char( "AC EXOTIC SET.\n\r\n\r", ch );
		    pObj->value[3] = atoi( argument );
		    break;
	    }
	    break;

        case ITEM_WEAPON:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "OLC_OBJ_WEAPON" );
	            return FALSE;

	        case 0:
		    send_to_char( "WEAPON CLASS SET.\n\r\n\r", ch );
		    pObj->value[0] = flag_value( weapon_class, argument );
		    break;
	        case 1:
	            send_to_char( "NUMBER OF DICE SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "TYPE OF DICE SET.\n\r\n\r", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "WEAPON TYPE SET.\n\r\n\r", ch );
	            pObj->value[3] = attack_lookup( argument );
	            break;
	        case 4:
                    send_to_char( "SPECIAL WEAPON TYPE TOGGLED.\n\r\n\r", ch );
		    pObj->value[4] ^= (flag_value( weapon_type2, argument ) != NO_FLAG
		    ? flag_value( weapon_type2, argument ) : 0 );
		    break;
	    }
            break;

	case ITEM_PORTAL:
	    switch ( value_num )
	    {
	        default:
	            return FALSE;
	            
	    	case 0:
	    	    send_to_char( "CHARGES SET.\n\r\n\r", ch);
	    	    pObj->value[0] = atoi ( argument );
	    	    break;
	    	case 1:
	    	    send_to_char( "EXIT FLAGS TOGGLED.\n\r\n\r", ch);
	            pObj->value[1] ^= (flag_value( exit_flags, argument ) != NO_FLAG
	            ? flag_value( exit_flags, argument ) : 0);
	    	    break;
	    	case 2:
	    	    send_to_char( "PORTAL FLAGS TOGGLED.\n\r\n\r", ch);
	            pObj->value[2] ^= (flag_value( portal_flags, argument ) != NO_FLAG
	            ? flag_value( portal_flags, argument ) : 0);
	    	    break;
	    	case 3:
	    	    send_to_char( "EXIT VNUM SET.\n\r\n\r", ch);
	    	    pObj->value[3] = atoi ( argument );
	    	    break;
	   }
	   break;

	case ITEM_FURNITURE:
	    switch ( value_num )
	    {
	        default:
	            do_help( ch, "ITEM_FURNITURE" );
	            return FALSE;
	            
	        case 0:
	            send_to_char( "NUMBER OF PEOPLE SET.\n\r\n\r", ch);
	            pObj->value[0] = atoi ( argument );
	            break;
	        case 1:
	            send_to_char( "MAX WEIGHT SET.\n\r\n\r", ch);
	            pObj->value[1] = atoi ( argument );
	            break;
	        case 2:
	            send_to_char( "FURNITURE FLAGS TOGGLED.\n\r\n\r", ch);
	            pObj->value[2] ^= (flag_value( furniture_flags, argument ) != NO_FLAG
	            ? flag_value( furniture_flags, argument ) : 0);
	            break;
	        case 3:
	            send_to_char( "HEAL BONUS SET.\n\r\n\r", ch);
	            pObj->value[3] = atoi ( argument );
	            break;
	        case 4:
	            send_to_char( "MANA BONUS SET.\n\r\n\r", ch);
	            pObj->value[4] = atoi ( argument );
	            break;
	    }
	    break;
	   
        case ITEM_CONTAINER:
	    switch ( value_num )
	    {
		int value;
		
		default:
		    do_help( ch, "ITEM_CONTAINER" );
	            return FALSE;
		case 0:
	            send_to_char( "WEIGHT CAPACITY SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
		case 1:
	            if ( ( value = flag_value( container_flags, argument ) )
	              != NO_FLAG )
	        	TOGGLE_BIT(pObj->value[1], value);
		    else
		    {
			do_help ( ch, "ITEM_CONTAINER" );
			return FALSE;
		    }
	            send_to_char( "CONTAINER TYPE SET.\n\r\n\r", ch );
	            break;
		case 2:
		    if ( atoi(argument) != 0 )
		    {
			if ( !get_obj_index( atoi( argument ) ) )
			{
			    send_to_char( "THERE IS NO SUCH ITEM.\n\r\n\r", ch );
			    return FALSE;
			}

			if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY )
			{
			    send_to_char( "THAT ITEM IS NOT A KEY.\n\r\n\r", ch );
			    return FALSE;
			}
		    }
		    send_to_char( "CONTAINER KEY SET.\n\r\n\r", ch );
		    pObj->value[2] = atoi( argument );
		    break;
		case 3:
		    send_to_char( "CONTAINER MAX WEIGHT SET.\n\r", ch);
		    pObj->value[3] = atoi( argument );
		    break;
		case 4:
		    send_to_char( "WEIGHT MULTIPLIER SET.\n\r\n\r", ch );
		    pObj->value[4] = atoi ( argument );
		    break;
	    }
	    break;

        case ITEM_PIT:
            switch ( value_num )
            {
                default:
                    do_help( ch, "ITEM_PIT" );
                    return FALSE;
                case 0:
                    send_to_char( "WEIGHT CAPACITY SET.\n\r\n\r", ch );
                    pObj->value[0] = atoi( argument );
                    break;
                case 1:
		    send_to_char("Pits don't need v1 flags.\n\r",ch);
		    return FALSE;
                    break;
                case 2:
                    if ( atoi(argument) != 0 )
                    {
                        if ( !get_obj_index( atoi( argument ) ) )
                        {
                            send_to_char( "THERE IS NO SUCH ITEM.\n\r\n\r", ch );
                            return FALSE;
                        }

                        if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY )
                        {
                            send_to_char( "THAT ITEM IS NOT A KEY.\n\r\n\r", ch );
                            return FALSE;
                        }
                    }
                    send_to_char( "PIT KEY SET.\n\r\n\r", ch );
                    pObj->value[2] = atoi( argument );
                    break;
                case 3:
                    send_to_char( "PIT MAX WEIGHT SET.\n\r", ch);
                    pObj->value[3] = atoi( argument );
                    break;
                case 4:
                    send_to_char( "WEIGHT MULTIPLIER SET.\n\r\n\r", ch );
                    pObj->value[4] = atoi ( argument );
                    break;
            }
            break;

	case ITEM_DRINK_CON:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_DRINK" );
	            return FALSE;
	        case 0:
	            send_to_char( "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "LIQUID TYPE SET.\n\r\n\r", ch );
	            pObj->value[2] = ( liq_lookup(argument) != -1 ?
	            		       liq_lookup(argument) : 0 );
	            break;
	        case 3:
	            send_to_char( "POISON VALUE TOGGLED.\n\r\n\r", ch );
	            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
	            break;
	    }
            break;

	case ITEM_FOUNTAIN:
	    switch (value_num)
	    {
	    	default:
		    do_help( ch, "ITEM_FOUNTAIN" );
	            return FALSE;
	        case 0:
	            send_to_char( "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "LIQUID TYPE SET.\n\r\n\r", ch );
	            pObj->value[2] = ( liq_lookup( argument ) != -1 ?
	            		       liq_lookup( argument ) : 0 );
	            break;
            }
	break;
		    	
	case ITEM_FOOD:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_FOOD" );
	            return FALSE;
	        case 0:
	            send_to_char( "HOURS OF FOOD SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "HOURS OF FULL SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "POISON VALUE TOGGLED.\n\r\n\r", ch );
	            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
	            break;
	    }
            break;

	case ITEM_MONEY:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_MONEY" );
	            return FALSE;
	        case 0:
	            send_to_char( "GOLD AMOUNT SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
		case 1:
		    send_to_char( "SILVER AMOUNT SET.\n\r\n\r", ch );
		    pObj->value[1] = atoi( argument );
		    break;
	    }
            break;

	case ITEM_ENGINEER_TOOL:
	    switch ( LOWER( *argument ) )
	    {
		case 'b':	pObj->value[0] = 0;	break;
		case 'g':	pObj->value[0] = 1;	break;
		case 't':	pObj->value[0] = 2;	break;
		default:
		    send_to_char( "Use build, grenade, or tinker.\n\r", ch );
		    return FALSE;
	    }
	    break;

	case ITEM_TRAP:
	    switch( value_num )
	    {
		case 0:
		    pObj->value[0] = URANGE( 0, flag_value( trap_type_table, argument ), TRAP_MAX );
		    send_to_char( "Trap type set.\n\r", ch );
		    break;

		case 1:
		    send_to_char( "Damtype set.\n\r", ch );
		    pObj->value[1] = URANGE( 0, dam_type_lookup( argument ), DAM_MAX );
		    break;

		case 2:
		    send_to_char( "Min damage set.\n\r", ch );
		    pObj->value[2] = atoi( argument );
		    break;

		case 3:
		    send_to_char( "Max damage set.\n\r", ch );
		    pObj->value[3] = atoi( argument );
		    break;

		case 4:
		    send_to_char( "Timer set.\n\r", ch );
		    pObj->value[4] = atoi( argument );
		    break;
	    }
	    break;

	case ITEM_MINE:
	    break;
    }

    send_to_char( show_obj_values( ch, pObj ), ch );
    send_to_char( "{x", ch );
    object_reset( pObj );
    return TRUE;
}

OEDIT( oedit_show )
{
    AFFECT_DATA *paf;
    BUFFER *final = new_buf( );
    PROG_LIST *list;
    OBJ_INDEX_DATA *pObj;
    char buf[MAX_STRING_LENGTH], temp[MAX_INPUT_LENGTH];
    char *flags;
    int cnt;

    EDIT_OBJ( ch, pObj );

    if ( pObj->history != NULL && pObj->history[0] != '\0' )
    {
	add_buf( final, "{qHistory:\n\r{t" );
	add_buf( final, pObj->history );
	add_buf( final, "\n\r" );
    }

    sprintf( buf, "{qName:        {s[{t%s{s]\n\r", pObj->name );
    add_buf( final, buf );

    sprintf( buf, "{qArea:        {s[{t%5d{s] [{t%s{s]\n\r",
	!pObj->area ? -1        : pObj->area->vnum,
	!pObj->area ? "No Area" : pObj->area->name );
    add_buf( final, buf );

    sprintf( buf, "{qVnum:        {s[{t%5d{s]\n\r", pObj->vnum );
    add_buf( final, buf );

    sprintf( buf, "{qType:        {s[{t%s{s]\n\r",
	flag_string( type_flags, pObj->item_type ) );
    add_buf( final, buf );

    sprintf( buf, "{qLevel:       {s[{t%5d{s]\n\r", pObj->level );
    add_buf( final, buf );

    sprintf( buf, "{qWear flags:  {s[{t%s{s]\n\r",
	flag_string( wear_flags, pObj->wear_flags ) );
    add_buf( final, buf );

    flags = flag_string( extra_flags, pObj->extra_flags );
    flags = length_argument( flags, temp, 55 );

    sprintf( buf, "{qExtra flags: {s[{t%s{s]\n\r", temp );
    add_buf( final, buf );

    while ( *flags != '\0' )
    {
	flags = length_argument( flags, temp, 55 );
	sprintf( buf, "             {s[{t%s{s]\n\r", temp );
	add_buf( final, buf );
    }

    sprintf( buf, "{qWeight:      {s[{t%5d{s]\n\r", pObj->weight );
    add_buf( final, buf );

    sprintf( buf, "{qCost:        {s[{t%5d{s]\n\r", pObj->cost );
    add_buf( final, buf );

    sprintf( buf, "{qSize:        {s[{t%d{s] [{t%s{s]\n\r",
       pObj->size, flag_string( size_flags, pObj->size ) );
    add_buf( final, buf );

    if ( IS_OBJ_STAT( pObj, ITEM_AQUEST ) )
    {
	sprintf( buf, "{qQuestPoints: {s[{t%d{s]\n\r", pObj->quest_points );
	add_buf( final, buf );
    }

    if ( IS_OBJ_STAT( pObj, ITEM_FORGED ) )
    {
	sprintf( buf, "{qForge Stone: {s[{t%d{s] {qStone Count: {s[{t%d{s]\n\r",
	    pObj->forge_vnum, pObj->forge_count );
	add_buf( final, buf );
    }

    flags = return_classes( pObj->class_can_use );
    flags = length_argument( flags, temp, 55 );

    sprintf( buf, "{q!Classes:    {s[{t%s{s]\n\r", temp );
    add_buf( final, buf );

    while( *flags != '\0' )
    {
	flags = length_argument( flags, temp, 55 );
	sprintf( buf, "             {s[{t%s{s]\n\r", temp );
	add_buf( final, buf );
    }

    if ( pObj->extra_descr )
    {
	EXTRA_DESCR_DATA *ed;

	add_buf( final, "{qEx desc kwd: " );

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    sprintf( buf, "{s[{t%s{s]", ed->keyword );
	    add_buf( final, buf );
	}

	add_buf( final, "\n\r" );
    }

    sprintf( buf, "{qShort desc:  {s[{x%s{s]\n\r", pObj->short_descr );
    add_buf( final, buf );

    sprintf( buf, "             {s[{t%s{s]\n\r",
	double_color( pObj->short_descr ) );
    add_buf( final, buf );

    sprintf( buf, "{qLong desc:   {s[{x%s{s]\n\r", pObj->description );
    add_buf( final, buf );

    sprintf( buf, "             {s[{t%s{s]\n\r",
	double_color( pObj->description ) );
    add_buf( final, buf );

    switch( pObj->item_type )
    {
	default:
	    break;

	case ITEM_POTION:
	case ITEM_SCROLL:
	case ITEM_PILL:
	case ITEM_WAND:
	case ITEM_STAFF:
	    sprintf( buf, "{qSuccess:     {s[{t%d%%{s]\n\r", pObj->success );
	    add_buf( final, buf );

	    sprintf( buf, "{qTargets:     {s[{t%d{s]\n\r", pObj->targets );
	    add_buf( final, buf );
	    break;
    }

    if ( pObj->item_type == ITEM_KEY )
    {
	ROOM_INDEX_DATA *pRoom;
	sh_int door, pos = 0;

	for ( cnt = 0; cnt < MAX_KEY_HASH; cnt++ )
	{
	    for ( pRoom = room_index_hash[cnt]; pRoom != NULL; pRoom = pRoom->next )
	    {
		for ( door = 0; door < MAX_DIR; door++ )
		{
		    if ( pRoom->exit[door] && pRoom->exit[door]->key == pObj->vnum )
		    {
			if ( pos++ == 0 )
			    add_buf( final, "\n\r{qExits This Key Locks:\n\r{s----------------------------------------------------------\n\r" );

			sprintf( buf, "{qRoom: {s[{t%5d{s] [{t%s{s] {qExit: {t%s\n\r",
			    pRoom->vnum, end_string( pRoom->name, 30 ),
			    dir_name[door] );
			add_buf( final, buf );
		    }
		}
	    }
	}
    }

    for ( cnt = 0, paf = pObj->affected; paf; paf = paf->next )
    {
	char aff[MAX_STRING_LENGTH];

	if ( cnt == 0 )
	{
	    add_buf( final, "{qNumber    Affects    Modifier    BitVector   \n\r" );
	    add_buf( final, "{s------ ------------- -------- ---------------\n\r" );
	}

	aff[0] = '\0';

	if ( paf->where == TO_DAM_MODS )
	{
	    sprintf( buf, "{s[{t%4d{s] {t%-13.13s %8d Damage Modifier\n\r",
		cnt, paf->location == DAM_ALL ? "all" :
		damage_mod_table[paf->location].name, paf->modifier );
	    add_buf( final, buf );
	} else {
	    if ( paf->bitvector )
	    {
		switch( paf->where )
		{
		    case TO_AFFECTS:
			sprintf(aff," {s[{qAffect: {t%s{s]",
			    flag_string( affect_flags, paf->bitvector ) );
			break;

		    case TO_SHIELDS:
			sprintf(aff," {s[{qShield: {t%s{s]",
			    flag_string( shield_flags, paf->bitvector ) );
			break;

		    case TO_WEAPON:
			sprintf(aff," {s[{qWeapon: {t%s{s]",
			    flag_string( weapon_type2, paf->bitvector ) );
			break;

		    case TO_OBJECT:
			sprintf(aff," {s[{qObject: {t%s{s]",
			    flag_string( extra_flags, paf->bitvector ) );
			break;

		    default:
			sprintf(aff," {s[{qUnknwn: {t%d {qof {t%d{s]",
			    paf->where, paf->bitvector);
			break;
		}
	    }

	    sprintf( buf, "{s[{t%4d{s] {t%-13.13s %8d%s\n\r", cnt,
		flag_string( apply_flags, paf->location ),
		paf->modifier, aff );
	    add_buf( final, buf );
	}
	cnt++;
    }

    add_buf( final, show_obj_values( ch, pObj ) );

    if ( pObj->oprogs )
    {
	int cnt;

	sprintf( buf, "\n\r{qOBJPrograms for {s[{t%5d{s]{q:\n\r", pObj->vnum );
	add_buf( final, buf );

	for ( cnt = 0, list = pObj->oprogs; list; list = list->next )
	{
	    if ( cnt == 0 )
	    {
		add_buf( final, " {qNumber Vnum Trigger Phrase\n\r" );
		add_buf( final, " {q------ ---- ------- ------\n\r" );
	    }

	    sprintf( buf, "{s[{t%5d{s] {t%4d %7s %s\n\r", cnt,
		list->vnum, prog_type_to_name( list->trig_type ),
		list->trig_phrase );
	    add_buf( final, buf );
	    cnt++;
	}
    }

    find_resets( final, ED_OBJECT, pObj->vnum );

    add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

OEDIT ( oedit_addoprog )
{
  int value;
  OBJ_INDEX_DATA *pObj;
  PROG_LIST *list;
  PROG_CODE *code;
  char trigger[MAX_STRING_LENGTH];
  char phrase[MAX_STRING_LENGTH];
  char num[MAX_STRING_LENGTH];

  EDIT_OBJ(ch, pObj);
  argument=one_argument(argument, num);
  argument=one_argument(argument, trigger);
  argument=one_argument(argument, phrase);

  if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0' )
  {
        send_to_char("Syntax:   addoprog [vnum] [trigger] [phrase]\n\r",ch);
        return FALSE;
  }

  if ( (value = flag_value (oprog_flags, trigger) ) == NO_FLAG)
  {
        send_to_char("Valid flags are:\n\r",ch);
        show_help( ch, "oprog");
        return FALSE;
  }

  if ( ( code =get_prog_index (atoi(num), PRG_OPROG ) ) == NULL)
  {
        send_to_char("No such OBJProgram.\n\r",ch);
        return FALSE;
  }

  list                  = new_prog();
  list->vnum            = atoi(num);
  list->trig_type       = value;
  list->trig_phrase     = str_dup(phrase);
  list->code            = code->code;
  SET_BIT(pObj->oprog_flags,value);
  list->next            = pObj->oprogs;
  pObj->oprogs          = list;

  send_to_char( "Oprog Added.\n\r",ch);
  return TRUE;
}

OEDIT ( oedit_deloprog )
{
    OBJ_INDEX_DATA *pObj;
    PROG_LIST *list;
    PROG_LIST *list_next;
    char oprog[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;

    EDIT_OBJ(ch, pObj);

    one_argument( argument, oprog );
    if (!is_number( oprog ) || oprog[0] == '\0' )
    {
       send_to_char("Syntax:  deloprog [#oprog]\n\r",ch);
       return FALSE;
    }

    value = atoi ( oprog );

    if ( value < 0 )
    {
        send_to_char("Only non-negative oprog-numbers allowed.\n\r",ch);
        return FALSE;
    }

    if ( !(list= pObj->oprogs) )
    {
        send_to_char("OEdit:  Non existant oprog.\n\r",ch);
        return FALSE;
    }

    if ( value == 0 )
    {
	REMOVE_BIT(pObj->oprog_flags, pObj->oprogs->trig_type);
        list = pObj->oprogs;
        pObj->oprogs = list->next;
        free_prog( list );
    }
    else
    {
        while ( (list_next = list->next) && (++cnt < value ) )
                list = list_next;

        if ( list_next )
        {
		REMOVE_BIT(pObj->oprog_flags, list_next->trig_type);
                list->next = list_next->next;
                free_prog(list_next);
        }
        else
        {
                send_to_char("No such oprog.\n\r",ch);
                return FALSE;
        }
    }

    send_to_char("Oprog removed.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_balance )
{
    OBJ_INDEX_DATA *pObj;
    sh_int level;

    EDIT_OBJ( ch, pObj );

    if ( (level = pObj->level) <= 0 || level > LEVEL_HERO )
    {
	send_to_char( "Valid object levels are 1 to HERO.\n\r", ch );
	return FALSE;
    }

    object_balance( pObj, level );

    return TRUE;
}

OEDIT( oedit_addaffect )
{
    AFFECT_DATA *pAf;
    OBJ_INDEX_DATA *pObj;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];
    int value;

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, loc );
    one_argument( argument, mod );

    if ( loc[0] == '\0' || mod[0] == '\0' || !is_number( mod ) )
    {
	send_to_char( "Syntax:  addaffect [location] [#xmod]\n\r", ch );
	return FALSE;
    }

    if ( ( value = flag_value( apply_flags, loc ) ) == NO_FLAG ) /* Hugin */
    {
        send_to_char( "Valid affects are:\n\r", ch );
	show_help( ch, "apply" );
	return FALSE;
    }

    pAf             =   new_affect();
    pAf->location   =   value;
    pAf->modifier   =   atoi( mod );
    pAf->where	    =   TO_OBJECT;
    pAf->type       =   -1;
    pAf->dur_type   =	DUR_TICKS;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char( "Affect added.\n\r", ch);
    affect_edit( pObj, pAf, atoi( mod ), TRUE );
    return TRUE;
}

OEDIT( oedit_class )
{
    OBJ_INDEX_DATA *pObj;
    sh_int class = 0, pos;
    char arg[MAX_INPUT_LENGTH];
    bool fAll, setting;

    EDIT_OBJ( ch, pObj );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Class <class/all> <yes/no>\n\r", ch );
	return FALSE;
    }

    fAll = !str_cmp( arg, "all" );

    if ( !fAll && ( class = class_lookup( arg ) ) == -1 )
    {
	send_to_char( "Invalid class.\n\r", ch );
	return FALSE;
    }

    if ( !str_prefix( argument, "yes" ) )
	setting = TRUE;
    else if ( !str_prefix( argument, "no" ) )
	setting = FALSE;
    else
    {
	oedit_class( ch, "" );
	return FALSE;
    }

    if ( fAll )
    {
	for ( pos = 0; pos < maxClass; pos++ )
	    pObj->class_can_use[pos] = setting;
    } else
	pObj->class_can_use[class] = setting;

    send_to_char( "Object class restrictions set.\n\r", ch );

    return TRUE;
}

OEDIT( oedit_copy )
{
    AFFECT_DATA *paf, *paf_next;
    EXTRA_DESCR_DATA* ed, *ed_next;
    OBJ_INDEX_DATA *pObj, *pObj2;
    int vnum, i;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: copy <vnum>\n\r", ch );
	return FALSE;
    }

    if ( !is_number(argument) )
    {
	send_to_char("OEdit: You must enter a number (vnum).\n\r",ch);
	return FALSE;
    }
    else
    {
	vnum = atoi(argument);
	if( !( pObj2 = get_obj_index(vnum) ) )
	{
	    send_to_char("OEdit: That object does not exist.\n\r",ch);
	    return FALSE;
	}
    }

    if ( !IS_BUILDER( ch, pObj2->area ) )
    {
	send_to_char( "OEdit:  Vnum in an area you cannot build in.\n\r", ch );
	return FALSE;
    }

    EDIT_OBJ(ch, pObj);

    free_string( pObj->name );
    pObj->name		= str_dup( pObj2->name );

    free_string( pObj->short_descr );
    pObj->short_descr	= str_dup( pObj2->short_descr );

    free_string( pObj->description );
    pObj->description	= str_dup( pObj2->description );

    free_string( pObj->history );
    pObj->history	= str_dup( pObj2->history );

    pObj->item_type	= pObj2->item_type;
    pObj->level		= pObj2->level;
    pObj->wear_flags	= pObj2->wear_flags;
    pObj->extra_flags	= pObj2->extra_flags;
    pObj->weight	= pObj2->weight;
    pObj->cost		= pObj2->cost;
    pObj->size		= pObj2->size;
    pObj->quest_points	= pObj2->quest_points;
    pObj->targets	= pObj2->targets;
    pObj->success	= pObj2->success;

    for ( ed = pObj->extra_descr; ed != NULL; ed = ed_next )
    {
	ed_next = ed->next;
	free_extra_descr( ed );
    }
    pObj->extra_descr = NULL;

    for ( ed = pObj2->extra_descr; ed != NULL; ed = ed->next )
    {
	ed_next			= new_extra_descr( );
	ed_next->keyword	= str_dup( ed->keyword );
	ed_next->description	= str_dup( ed->description );
	ed_next->next		= pObj->extra_descr;
	pObj->extra_descr	= ed_next;
    }

    for ( paf = pObj->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;

	free_affect( paf );
    }
    pObj->affected = NULL;

    for ( paf = pObj2->affected; paf != NULL; paf = paf->next )
    {
	paf_next		= new_affect();
	paf_next->location	= paf->location;
	paf_next->modifier	= paf->modifier;
	paf_next->where		= paf->where;
	paf_next->type		= paf->type;
	paf_next->dur_type	= paf->dur_type;
	paf_next->duration	= paf->duration;
	paf_next->bitvector	= paf->bitvector;
	paf_next->level		= paf->level;
	paf_next->next		= pObj->affected;
	pObj->affected		= paf_next;
    }

    for (i = 0; i < 5; i++)
	pObj->value[i] = pObj2->value[i];

    send_to_char( "Object info copied.", ch );
    object_reset( pObj );
    return TRUE;
}

OEDIT( oedit_delete )
{
    AUCTION_DATA *auc;
    OBJ_DATA *obj, *obj_next;
    OBJ_INDEX_DATA *pObj, *iObj;
    RESET_DATA *pReset, *pReset_next;
    ROOM_INDEX_DATA *pRoom;
    char buf[MAX_STRING_LENGTH];
    int count, iHash;

    EDIT_OBJ( ch, pObj );

    if ( argument[0] == '\0' || !is_number( argument )
    ||   atoi( argument ) != pObj->vnum )
    {
	send_to_char( "Argument must match the vnum you are editing.\n\r", ch );
	return FALSE;
    }

    check_olc_delete( ch );

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next;

	if ( obj->pIndexData == pObj )
	{
	    CHAR_DATA *wch;

	    for ( auc = auction_list; auc != NULL; auc = auc->next )
	    {
		if ( auc->item != NULL && obj == auc->item )
		{
		    if ( auc->high_bidder != NULL )
		    {
			add_cost(auc->high_bidder,auc->bid_amount,auc->bid_type);
			send_to_char("\n\rYour bid has been returned to you.\n\r",auc->high_bidder);
		    }

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
    }

    iHash = pObj->vnum % MAX_KEY_HASH;
    iObj = obj_index_hash[iHash];

    if ( iObj->next == NULL )
	obj_index_hash[iHash] = NULL;

    else if ( iObj == pObj )
	obj_index_hash[iHash] = pObj->next;

    else
    {
	for ( ; iObj != NULL; iObj = iObj->next )
	{
	    if ( iObj->next == pObj )
	    {
		iObj->next = pObj->next;
		break;
	    }
	}
    }

    count = 0;
    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
	{
	    for ( pReset = pRoom->reset_first; pReset; pReset = pReset_next )
	    {
		pReset_next = pReset->next;

		switch ( pReset->command )
		{
		    default:
			break;

		    case 'O':
		    case 'E':
		    case 'P':
		    case 'G':
			if ( pReset->arg1 == pObj->vnum
                        ||   (pReset->command == 'P'
			&&    pReset->arg3 == pObj->vnum) )
			{
			    if ( pRoom->reset_first == pReset )
				pRoom->reset_first = pReset->next;
			    else
			    {
				RESET_DATA *prev;
				for ( prev = pRoom->reset_first; prev != NULL; prev = prev->next )
				{
				    if ( prev->next == pReset )
				    {
					prev->next = pReset->next;
					break;
				    }
				}
			    }

			    count++;
			    free_reset_data( pReset );
			    SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
			}
		}
	    }
	}
    }

    sprintf( buf, "Removed object vnum {C%d{x and {C%d{x resets.\n\r",
	pObj->vnum, count );
    send_to_char( buf, ch );

    free_obj_index( pObj );

    return TRUE;
}

OEDIT( oedit_addapply )
{
    AFFECT_DATA *pAf;
    OBJ_INDEX_DATA *pObj;
    char loc[MAX_INPUT_LENGTH];
    char mod[MAX_INPUT_LENGTH];
    char type[MAX_INPUT_LENGTH];
    int bv, value, typ;

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, type );
    argument = one_argument( argument, loc );
    argument = one_argument( argument, mod );

    if ( type[0] == '\0' || ( typ = flag_value( apply_types, type ) ) == NO_FLAG )
    {
    	send_to_char( "Invalid apply type. Valid apply types are:\n\r", ch);
    	show_help( ch, "apptype" );
    	return FALSE;
    }

    if ( loc[0] == '\0' || ( value = flag_value( apply_flags, loc ) ) == NO_FLAG )
    {
        send_to_char( "Valid applys are:\n\r", ch );
	show_help( ch, "apply" );
	return FALSE;
    }

    if ( argument[0] == '\0' || ( bv = flag_value( bitvector_type[typ].table, argument ) ) == NO_FLAG )
    {
    	send_to_char( "Invalid bitvector type.\n\r", ch );
	send_to_char( "Valid bitvector types are:\n\r", ch );
	show_help( ch, bitvector_type[typ].help );
    	return FALSE;
    }

    if ( mod[0] == '\0' || !is_number( mod ) )
    {
	send_to_char( "Syntax:  addapply [type] [location] [#xmod] [bitvector]\n\r", ch );
	return FALSE;
    }

    pAf             =   new_affect();
    pAf->location   =   value;
    pAf->modifier   =   atoi( mod );
    pAf->where	    =   apply_types[typ].bit;
    pAf->type	    =	-1;
    pAf->dur_type   =	DUR_TICKS;
    pAf->duration   =   -1;
    pAf->bitvector  =   bv;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char( "Apply added.\n\r", ch);
    affect_edit( pObj, pAf, atoi( mod ), TRUE );
    return TRUE;
}

void affect_edit_del( OBJ_INDEX_DATA *pObj, AFFECT_DATA *paf )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObj && obj->carried_by )
	    affect_modify( obj->carried_by, paf, FALSE );
    }
}

/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
OEDIT( oedit_delaffect )
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    AFFECT_DATA *pAf_next;
    char affect[MAX_STRING_LENGTH];
    int  value;
    int  cnt = 0;

    EDIT_OBJ(ch, pObj);

    one_argument( argument, affect );

    if ( !is_number( affect ) || affect[0] == '\0' )
    {
	send_to_char( "Syntax:  delaffect [#xaffect]\n\r", ch );
	return FALSE;
    }

    value = atoi( affect );

    if ( value < 0 )
    {
	send_to_char( "Only non-negative affect-numbers allowed.\n\r", ch );
	return FALSE;
    }

    if ( !( pAf = pObj->affected ) )
    {
	send_to_char( "OEdit:  Non-existant affect.\n\r", ch );
	return FALSE;
    }

    if( value == 0 )	/* First case: Remove first affect */
    {
	pAf = pObj->affected;
	pObj->affected = pAf->next;
	affect_edit_del( pObj, pAf );
	free_affect( pAf );
    }
    else		/* Affect to remove is not the first */
    {
	while ( ( pAf_next = pAf->next ) && ( ++cnt < value ) )
	     pAf = pAf_next;

	if( pAf_next )		/* See if it's the next affect */
	{
	    pAf->next = pAf_next->next;
	    affect_edit_del( pObj, pAf_next );
	    free_affect( pAf_next );
	}
	else                                 /* Doesn't exist */
	{
	     send_to_char( "No such affect.\n\r", ch );
	     return FALSE;
	}
    }

    send_to_char( "Affect removed.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_name )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  name [string]\n\r", ch );
	return FALSE;
    }

    free_string( pObj->name );
    pObj->name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_dam_mod )
{
    AFFECT_DATA *paf;
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_INPUT_LENGTH];
    sh_int dam_mod, value;

    EDIT_OBJ( ch, pObj );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Dam_mod (dam_type) (modifier).\n\r", ch );
	return FALSE;
    }

    if ( !is_number( argument )
    ||   ( ( value = atoi( argument ) ) < -10 )
    ||   value > 10 || value == 0 )
    {
	send_to_char( "Modifiers must be between -10% and 10%.\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( arg, "all" ) )
	dam_mod = -1;

    else if ( ( dam_mod = dam_type_lookup( arg ) ) == -1 )
    {
	send_to_char( "Valid damage types are:\n", ch );
	for ( dam_mod = 0; dam_mod < DAM_MAX; dam_mod++ )
	{
	    sprintf( arg, " %s", damage_mod_table[dam_mod].name );
	    send_to_char( arg, ch );
	}

	send_to_char( "\n\r", ch );
	return FALSE;
    }

    paf			= new_affect();
    paf->location	= dam_mod;
    paf->modifier	= value;
    paf->where		= TO_DAM_MODS;
    paf->type		= -1;
    paf->dur_type	= DUR_TICKS;
    paf->duration	= -1;
    paf->bitvector	= 0;
    paf->level		= pObj->level;
    paf->next		= pObj->affected;
    pObj->affected	= paf;

    send_to_char( "Damage modifier applied.\n\r", ch );
    affect_edit( pObj, paf, value, TRUE );
    return TRUE;
}

OEDIT( oedit_size )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_OBJ( ch, pObj );

	if ( ( value = flag_value( size_flags, argument ) ) != NO_FLAG )
	{
	    pObj->size = value;
	    send_to_char( "Size set.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: size [size]\n\r"
		  "Type '? size' for a list of sizes.\n\r", ch );
    return FALSE;
}

OEDIT( oedit_short )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  short [string]\n\r", ch );
	return FALSE;
    }

    free_string( pObj->short_descr );
    pObj->short_descr = str_dup( argument );

    send_to_char( "Short description set.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_long )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  long [string]\n\r", ch );
	return FALSE;
    }
        
    free_string( pObj->description );
    pObj->description = str_dup( argument );

    send_to_char( "Long description set.\n\r", ch);
    return TRUE;
}

bool set_value( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, char *argument, int value )
{
    if ( argument[0] == '\0' )
    {
	set_obj_values( ch, pObj, -1, "" );     /* '\0' changed to "" -- Hugin */
	return FALSE;
    }

    if ( set_obj_values( ch, pObj, value, argument ) )
	return TRUE;

    return FALSE;
}



/*****************************************************************************
 Name:		oedit_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool oedit_values( CHAR_DATA *ch, char *argument, int value )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( set_value( ch, pObj, argument, value ) )
        return TRUE;

    return FALSE;
}


OEDIT( oedit_value0 )
{
    if ( oedit_values( ch, argument, 0 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value1 )
{
    if ( oedit_values( ch, argument, 1 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value2 )
{
    if ( oedit_values( ch, argument, 2 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value3 )
{
    if ( oedit_values( ch, argument, 3 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value4 )
{
    if ( oedit_values( ch, argument, 4 ) )
        return TRUE;

    return FALSE;
}

OEDIT( oedit_weight )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  weight [number]\n\r", ch );
	return FALSE;
    }

    pObj->weight = atoi( argument );

    send_to_char( "Weight set.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_targets )
{
    OBJ_INDEX_DATA *pObj;
    sh_int value;

    EDIT_OBJ( ch, pObj );

    switch( pObj->item_type )
    {
	default:
	    send_to_char( "This setting has no affect on this item type.\n\r", ch );
	    return FALSE;

	case ITEM_POTION:
	case ITEM_SCROLL:
	case ITEM_PILL:
	case ITEM_STAFF:
	    break;
    }

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  targets [number]\n\r", ch );
	return FALSE;
    }

    if ( ( value = atoi( argument ) ) < 0 )
    {
	send_to_char( "Random affect must be at least 0.\n\r", ch );
	return FALSE;
    }

    pObj->targets = value;

    send_to_char( "Targets variable set.\n\r", ch );
    return TRUE;
}

OEDIT( oedit_success )
{
    OBJ_INDEX_DATA *pObj;
    sh_int value;

    EDIT_OBJ( ch, pObj );

    switch( pObj->item_type )
    {
	default:
	    send_to_char( "This setting has no affect on this item type.\n\r", ch );
	    return FALSE;

	case ITEM_POTION:
	case ITEM_SCROLL:
	case ITEM_PILL:
	case ITEM_WAND:
	case ITEM_STAFF:
	    break;
    }

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  success [number]\n\r", ch );
	return FALSE;
    }

    if ( ( value = atoi( argument ) ) < 0
    ||   value > 100 )
    {
	send_to_char( "Success chance must be between 0 and 100 percent.\n\r", ch );
	return FALSE;
    }

    pObj->success = value;

    send_to_char( "Success variable set.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_cost )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  cost [number]\n\r", ch );
	return FALSE;
    }

    pObj->cost = atoi( argument );

    send_to_char( "Cost set.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_create )
{
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    int  value;
    int  iHash;

    value = atoi( argument );
    if ( argument[0] == '\0' || value == 0 )
    {
	send_to_char( "Syntax:  oedit create [vnum]\n\r", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );
    if ( !pArea )
    {
	send_to_char( "OEdit:  That vnum is not assigned an area.\n\r", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "OEdit:  Vnum in an area you cannot build in.\n\r", ch );
	return FALSE;
    }

    if ( get_obj_index( value ) )
    {
	send_to_char( "OEdit:  Object vnum already exists.\n\r", ch );
	return FALSE;
    }
        
    pObj			= new_obj_index( );
    pObj->name			= str_dup( "no name" );
    pObj->short_descr		= str_dup( "(no short description)" );
    pObj->description		= str_dup( "(no description)" );
    pObj->vnum			= value;
    pObj->area			= pArea;
        
    iHash			= value % MAX_KEY_HASH;
    pObj->next			= obj_index_hash[iHash];
    obj_index_hash[iHash]	= pObj;
    ch->desc->pEdit		= (void *)pObj;
    ch->desc->editor		= ED_OBJECT;

    send_to_char( "Object Created.\n\r", ch );
    return TRUE;
}



OEDIT( oedit_ed )
{
    EXTRA_DESCR_DATA *ed;
    OBJ_INDEX_DATA *pObj;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, command );
    one_argument( argument, keyword );

    if ( command[0] == '\0' )
    {
	send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
	send_to_char( "         ed delete [keyword]\n\r", ch );
	send_to_char( "         ed edit [keyword]\n\r", ch );
	send_to_char( "         ed format [keyword]\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( command, "add" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
	    return FALSE;
	}

	ed                  =   new_extra_descr();
	ed->keyword         =   str_dup( keyword );
	ed->next            =   pObj->extra_descr;
	pObj->extra_descr   =   ed;

	string_append( ch, &ed->description );

	return TRUE;
    }

    if ( !str_cmp( command, "edit" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed edit [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	}

	if ( !ed )
	{
	    send_to_char( "OEdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	string_append( ch, &ed->description );

	return TRUE;
    }

    if ( !str_cmp( command, "delete" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed delete [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	    ped = ed;
	}

	if ( !ed )
	{
	    send_to_char( "OEdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	if ( !ped )
	    pObj->extra_descr = ed->next;
	else
	    ped->next = ed->next;

	free_extra_descr( ed );

	send_to_char( "Extra description deleted.\n\r", ch );
	return TRUE;
    }


    if ( !str_cmp( command, "format" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed format [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	    ped = ed;
	}

	if ( !ed )
	{
                send_to_char( "OEdit:  Extra description keyword not found.\n\r", ch );
                return FALSE;
	}

	ed->description = format_string( ed->description, atoi( argument ) );

	send_to_char( "Extra description formatted.\n\r", ch );
	return TRUE;
    }

    oedit_ed( ch, "" );
    return FALSE;
}





/* ROM object functions : */

OEDIT( oedit_extra )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_OBJ( ch, pObj );

	if ( ( value = flag_value( extra_flags, argument ) ) != NO_FLAG )
	{
	    TOGGLE_BIT( pObj->extra_flags, value );

	    send_to_char( "Extra flags toggled.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax:  extra [flag]\n\r"
		  "Type '? extra' for a list of flags.\n\r", ch );
    return FALSE;
}

OEDIT( oedit_wear )
{
    OBJ_INDEX_DATA *pObj;
    OBJ_DATA *obj;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_OBJ( ch, pObj );

	if ( ( value = flag_value( wear_flags, argument ) ) != NO_FLAG )
	{
	    TOGGLE_BIT( pObj->wear_flags, value );
	    send_to_char( "Wear flag toggled.\n\r", ch );

	    for ( obj = object_list; obj != NULL; obj = obj->next )
	    {
		if ( obj->pIndexData == pObj )
		    TOGGLE_BIT( obj->wear_flags, value );
	    }

	    return TRUE;
	}
    }

    send_to_char( "Syntax:  wear [flag]\n\r"
		  "Type '? wear' for a list of flags.\n\r", ch );
    return FALSE;
}

OEDIT( oedit_type )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_OBJ(ch, pObj);

	if ( ( value = flag_value( type_flags, argument ) ) != NO_FLAG )
	{
	    pObj->item_type = value;

	    send_to_char( "Type set.\n\r", ch);

	    /*
	     * Clear the values.
	     */
	    pObj->value[0] = 0;
	    pObj->value[1] = 0;
	    pObj->value[2] = 0;
	    pObj->value[3] = 0;
	    pObj->value[4] = 0;     /* ROM */

	    return TRUE;
	}
    }

    send_to_char( "Syntax:  type [flag]\n\r"
		  "Type '? type' for a list of flags.\n\r", ch );
    return FALSE;
}

OEDIT( oedit_level )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  level [number]\n\r", ch );
	return FALSE;
    }

    pObj->level = atoi( argument );

    send_to_char( "Level set.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_history )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
	string_append( ch, &pObj->history );
	return TRUE;
    }

    send_to_char( "Syntax:  history\n\r", ch );
    return FALSE;
}

OEDIT( oedit_quest )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  quest [number]\n\r", ch );
	return FALSE;
    }

    if ( !IS_OBJ_STAT(pObj,ITEM_AQUEST) )
    {
	send_to_char( "This is reserved for aquest items only.\n\r", ch );
	return FALSE;
    }

    pObj->quest_points = atoi( argument );

    send_to_char( "Quest point value set.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_forge )
{
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_INPUT_LENGTH];

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number( arg )
    ||   argument[0] == '\0' || !is_number( argument )
    ||   get_obj_index( atoi( arg ) ) == NULL )
    {
	send_to_char( "Syntax:  forge [stone_vnum] [stone_count].\n\r", ch );
	return FALSE;
    }

    if ( !IS_OBJ_STAT(pObj,ITEM_FORGED) )
    {
	send_to_char( "This is reserved for forged items only.\n\r", ch );
	return FALSE;
    }

    pObj->forge_vnum	= atoi( arg );
    pObj->forge_count	= atoi( argument );

    send_to_char( "Forge stone values set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_show )
{
    BUFFER *final = new_buf( );
    MOB_INDEX_DATA *pMob;
    PROG_LIST *list;
    char buf[MAX_STRING_LENGTH], temp[MAX_INPUT_LENGTH];
    char *flags;
    sh_int lvl;

    EDIT_MOB( ch, pMob );

    sprintf( buf, "{qName:        {s[{t%s{s]  {qArea: {s[{t%3d{s] {t%s\n\r",
	pMob->player_name,
	!pMob->area ? -1        : pMob->area->vnum,
	!pMob->area ? "No Area" : pMob->area->name );
    add_buf( final, buf );

    sprintf( buf, "{qAct:         {s[{t%s{s]\n\r",
	flag_string( act_flags, pMob->act ) );
    add_buf( final, buf );

    sprintf( buf, "{qVnum:        {s[{t%5d{s]  {qSex: {s[{t%s{s]\n\r",
	pMob->vnum,
	pMob->sex == SEX_MALE    ? "male"   :
	pMob->sex == SEX_FEMALE  ? "female" : 
	pMob->sex == 3           ? "random" : "neutral" );
    add_buf( final, buf );

    sprintf( buf, "{qRace:        {s[{t%s{s]  {qGroup: {s[{t%3d{s]\n\r",
	race_table[pMob->race].name, pMob->group );
    add_buf( final, buf );

    sprintf( buf, "{qLevel:       {s[{t%2d{s] {qExp Mod: {s[{t%d%%{s] {qAlign: {s[{t%4d{s]\n\r",
	pMob->level, pMob->exp_percent, pMob->alignment );
    add_buf( final, buf );

    sprintf( buf, "{qHitroll:     {s[{t%d{s]  {qSaves: {s[{t%d{s]  {qDam Type: {s[{t%s{s]\n\r",
	pMob->hitroll, pMob->saves, attack_table[pMob->dam_type].name );
    add_buf( final, buf );

    sprintf( buf, "{qHit Points:  {s[{t%d {sto {t%d{s]\n\r",
	pMob->hit[0], pMob->hit[1] );
    add_buf( final, buf );

    sprintf( buf, "{qMana Points: {s[{t%d {sto {t%d{s]\n\r",
	pMob->mana[0], pMob->mana[1] );
    add_buf( final, buf );

    sprintf( buf, "{qRegeneration:{s[{t%d {qhp, {t%d {qmana, {t%d {qmove{s]\n\r",
	pMob->regen[0], pMob->regen[1], pMob->regen[2] );
    add_buf( final, buf );

    sprintf( buf, "{qDamage dice: {s[{t%d {sd {t%d {s+ {t%d{s]\n\r",
	pMob->damage[DICE_NUMBER], pMob->damage[DICE_TYPE],
	pMob->damage[DICE_BONUS] );
    add_buf( final, buf );

    sprintf( buf, "{qClass:       {s[{t%s{s]\n\r",
	class_table[pMob->class].name );
    add_buf( final, buf );

    sprintf( buf, "{qDefault Skill{s[{t%d%%{s]\n\r",
	pMob->skill_percentage );
    add_buf( final, buf );

    for ( lvl = 0; skill_table[lvl].name[0] != '\0'; lvl++ )
    {
	if ( pMob->learned[lvl] != pMob->skill_percentage )
	{
	    sprintf( buf, "{qBonus skill: {s[{t%s {sat {t%d%%{s]\n\r",
		skill_table[lvl].name, pMob->learned[lvl] );
	    add_buf( final, buf );
	}
    }

    flags = flag_string( affect_flags, pMob->affected_by );
    flags = length_argument( flags, temp, 65 );

    sprintf( buf, "{qAffected by: {s[{t%s{s]\n\r", temp );
    add_buf( final, buf );

    while ( *flags != '\0' )
    {
	flags = length_argument( flags, temp, 65 );
	sprintf( buf, "             {s[{t%s{s]\n\r", temp );
	add_buf( final, buf );
    }

    flags = flag_string( shield_flags, pMob->shielded_by );
    flags = length_argument( flags, temp, 65 );

    sprintf( buf, "{qShielded by: {s[{t%s{s]\n\r", temp );
    add_buf( final, buf );

    while ( *flags != '\0' )
    {
	flags = length_argument( flags, temp, 65 );
	sprintf( buf, "             {s[{t%s{s]\n\r", temp );
	add_buf( final, buf );
    }

    sprintf( buf, "{qArmor:       {s[{qpierce: {t%d  {qbash: {t%d  {qslash: {t%d  {qmagic: {t%d{s]\n\r",
	pMob->ac[AC_PIERCE], pMob->ac[AC_BASH],
	pMob->ac[AC_SLASH], pMob->ac[AC_EXOTIC] );
    add_buf( final, buf );

    flags = flag_string( part_flags, pMob->parts );
    flags = length_argument( flags, temp, 65 );

    sprintf( buf, "{qParts:       {s[{t%s{s]\n\r", temp );
    add_buf( final, buf );

    while ( *flags != '\0' )
    {
	flags = length_argument( flags, temp, 65 );
	sprintf( buf, "             {s[{t%s{s]\n\r", temp );
	add_buf( final, buf );
    }

    sprintf( buf, "{qSize:        {s[{t%d{s] [{t%s{s]\n\r",
	pMob->size, flag_string( size_flags, pMob->size ) );
    add_buf( final, buf );

    sprintf( buf, "{qPosition:    {s[{qStarting: {t%s {qDefault: {t%s{s]\n\r",
	flag_string( position_flags, pMob->start_pos ),
	flag_string( position_flags, pMob->default_pos ) );
    add_buf( final, buf );

    sprintf( buf, "{qWealth:      {s[{t%8ld{s]\n\r", pMob->wealth );
    add_buf( final, buf );

    if ( pMob->spec_fun )
    {
	sprintf( buf, "{qSpec fun:    {s[{t%s{s]\n\r", 
	    spec_name( pMob->spec_fun ) );
	add_buf( final, buf );
    }

    if ( pMob->die_descr && pMob->die_descr[0] )
    {
	sprintf( buf, "{qDie_desc:    {s[{t%s{s]\n\r", pMob->die_descr );
	add_buf( final, buf );
    }

    if ( pMob->say_descr && pMob->say_descr[0] )
    {
	sprintf( buf, "{qSay_desc:    {s[{t%s{s]\n\r", pMob->say_descr );
	add_buf( final, buf );
    }

    if ( pMob->bank_branch != 0 )
    {
	sprintf( buf, "{qBank Branch: {s[{t%d{s]\n\r", pMob->bank_branch );
	add_buf( final, buf );
    }

    if ( pMob->max_world )
    {
	sprintf( buf, "{qMax world:   {s[{t%d{s]\n\r", pMob->max_world );
	add_buf( final, buf );
    }

    if ( pMob->reflection )
    {
	sprintf( buf, "{qReflection:  {s[{t%d{s]\n\r", pMob->reflection );
	add_buf( final, buf );
    }
    
    if ( pMob->absorption )
    {
	sprintf( buf, "{qMelee Absorb:{s[{t%d{s]\n\r", pMob->absorption );
	add_buf( final, buf );
    }

    sprintf( buf, "{qShort descr: {s[{x%s{s]\n\r", pMob->short_descr );
    add_buf( final, buf );

    sprintf( buf, "             {s[{t%s{s]\n\r",
	double_color( pMob->short_descr ) );
    add_buf( final, buf );

    sprintf( buf, "{qLong descr:  {s[{x%s", pMob->long_descr );
    buf[strlen( buf )-2] = '\0';
    strcat( buf, "{s]\n\r" );
    add_buf( final, buf );

    sprintf( buf, "             {s[{t%s", double_color( pMob->long_descr ) );
    buf[strlen( buf )-2] = '\0';
    strcat( buf, "{s]\n\r" );
    add_buf( final, buf );

    add_buf( final, show_dam_mods( pMob->damage_mod ) );

    add_buf( final, "{qDescription:{t" );
    if ( pMob->description && pMob->description[0] )
    {
	add_buf( final, "\n\r" );
	add_buf( final, pMob->description );
    } else
	add_buf( final, "{s[{tnone{s]\n\r" );

    if ( pMob->pShop )
    {
	SHOP_DATA *pShop;
	int iTrade;

	pShop = pMob->pShop;

	sprintf( buf,
	  "\n\r{qShop data for {s[{t%5d{s]{q:\n\r"
	  "  {qMarkup  (shop sells): {s[{t%d%%{s]\n\r"
	  "  {qMarkdown (shop buys): {s[{t%d%%{s]\n\r",
	    pShop->keeper, pShop->profit_buy, pShop->profit_sell );
	add_buf( final, buf );

	sprintf( buf, "  {qHours: {s[{t%d {qto {t%d{s]\n\r",
	    pShop->open_hour, pShop->close_hour );
	add_buf( final, buf );

	add_buf( final, "  {qNumber Trades Type\n\r" );
	add_buf( final, "  {s------ -----------\n\r" );

	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	{
	    sprintf( buf, "  {s[{t%4d{s] {q%s\n\r", iTrade,
		pShop->buy_type[iTrade] == 0 ? "none" :
		flag_string( type_flags, pShop->buy_type[iTrade] ) );
	    add_buf( final, buf );
	}
    }

    if ( pMob->mprogs )
    {
	int cnt;

	sprintf( buf, "\n\r{qMOBPrograms for {s[{t%5d{s]{q:\n\r", pMob->vnum );
	add_buf( final, buf );

	for ( cnt = 0, list = pMob->mprogs; list; list = list->next )
	{
	    if ( cnt == 0 )
	    {
		add_buf( final, " {qNumber  Vnum Trigger Phrase\n\r" );
		add_buf( final, " {s------ ----- ------- ------\n\r" );
	    }

	    sprintf( buf, " {s[{t%4d{s] {t%-5d %7s %s\n\r",
		cnt, list->vnum,prog_type_to_name( list->trig_type ),
		list->trig_phrase );
	    add_buf( final, buf );
	    cnt++;
	}
    }

    find_resets( final, ED_MOBILE, pMob->vnum );

    add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

MEDIT( medit_exp_mod )
{
    MOB_INDEX_DATA *pMob;
    sh_int value;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax: exp_mod [number]\n\r", ch );
	return FALSE;
    }

    value = atoi( argument );

    if ( value < 0 || value > 300 )
    {
	send_to_char( "Exp mod range is 0 to 300%.\n\r", ch );
	return FALSE;
    }

    send_to_char( "Exp modifier set.\n\r", ch );

    pMob->exp_percent = value;
    return TRUE;
}

MEDIT( medit_balance )
{
    MOB_INDEX_DATA *pMob;
    sh_int type;

    EDIT_MOB(ch,pMob);

    if ( pMob->level <= 0 || pMob->level > LEVEL_HERO*2 )
    {
	char buf[MAX_STRING_LENGTH];

	sprintf(buf,"Valid mobile levels are 1 to %d.\n\r",LEVEL_HERO*2);
	send_to_char(buf,ch);
	return FALSE;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char("Valid balance arguments are 'easy', 'medium', or 'hard'.\n\r",ch);
	return FALSE;
    }

    else if ( !str_prefix(argument,"easy") )
	type = 1;

    else if ( !str_prefix(argument,"medium") )
	type = 2;

    else if ( !str_prefix(argument,"hard") )
	type = 3;

    else
    {
	send_to_char("Valid balance arguments are 'easy', 'medium', or 'hard'.\n\r",ch);
	return FALSE;
    }

    mobile_balance( pMob, type );

    send_to_char("Mobile balanced.\n\r",ch);

    return TRUE;
}

MEDIT( medit_class )
{
    MOB_INDEX_DATA *pMob;
    sh_int class;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: class [class]\n\r", ch );
	return FALSE;
    }

/*    if ( !str_cmp( argument, "random" ) )
	class = -1;

    else */ if ( ( class = class_lookup( argument ) ) == -1 )
    {
	send_to_char( "Invalid class.\n\r", ch );
	return FALSE;
    }

    send_to_char( "Class set.\n\r", ch );
    pMob->class = class;
    return TRUE;
}

MEDIT( medit_skills )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    sh_int percent, sn;

    argument = one_argument( argument, arg );

    EDIT_MOB( ch, pMob );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: medit skills <skill_name> <percent>.\n\r", ch );
	return FALSE;
    }

    if ( ( sn = skill_lookup( arg ) ) == -1 )
    {
	send_to_char( "That is not a valid skill.\n\r", ch );
	return FALSE;
    }

    if ( !is_number( argument )
    ||   ( percent = atoi( argument ) ) < 0
    ||   percent > 100 )
    {
	send_to_char( "Percentage must be 0% to 100%.\n\r", ch );
	return FALSE;
    }

    pMob->learned[sn] = percent;
    send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

MEDIT( medit_skill_percentage )
{
    MOB_INDEX_DATA *pMob;
    sh_int skill_percentage, sn;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax: medit skill_percentage [default percent].\n\r", ch );
	return FALSE;
    }

    skill_percentage = atoi( argument );
    if ( skill_percentage < 0 || skill_percentage > 100 )
    {
	send_to_char( "Default percentage must be between 0% and 100%.\n\r", ch );
	return FALSE;
    }

    pMob->skill_percentage = skill_percentage;

    for ( sn = 0; sn < maxSkill; sn++ )
	pMob->learned[sn] = skill_percentage;

    send_to_char( "Skill percentage set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_dam_mod )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    sh_int dam = -1, value;

    EDIT_MOB( ch, pMob );

    argument = one_argument( argument, arg );

    value = atoi( argument );

    if ( !is_number( argument ) || value < -200 || value > 200 )
    {
	send_to_char( "Proper damage modifiers are -200 to 200%.\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( dam = 0; dam < DAM_MAX; dam++ )
	    pMob->damage_mod[dam] = value;

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
    }

    if ( ( dam = dam_type_lookup( arg ) ) == -1 )
    {
	send_to_char( "Invalid damage modifier.\n\r", ch );
	return FALSE;
    }

    pMob->damage_mod[dam] = value;
    send_to_char( "Ok.\n\r", ch );
    return TRUE;
}

MEDIT( medit_copy )
{
    MOB_INDEX_DATA *pMob, *pMob2;
    int vnum;

    if ( argument[0] == '\0' )
    {
	send_to_char("Syntax: copy <vnum> \n\r",ch);
	return FALSE;
    }

    if ( !is_number(argument) )
    {
	send_to_char("MEdit: You must enter a number (vnum).\n\r",ch);
	return FALSE;
    } else {
	vnum = atoi(argument);
	if( !( pMob2 = get_mob_index(vnum) ) )
	{
	    send_to_char("MEdit: That mob does not exist.\n\r",ch);
	    return FALSE;
	}
    }

    if ( !IS_BUILDER( ch, pMob2->area ) )
    {
	send_to_char( "MEdit:  Vnum in an area you cannot build in.\n\r", ch );
	return FALSE;
    }

    EDIT_MOB(ch, pMob);

    free_string( pMob->player_name );
    pMob->player_name		= str_dup( pMob2->player_name );
    free_string( pMob->short_descr );
    pMob->short_descr		= str_dup( pMob2->short_descr );
    free_string( pMob->long_descr );
    pMob->long_descr		= str_dup( pMob2->long_descr   );
    free_string( pMob->description );
    pMob->description		= str_dup( pMob2->description );
    pMob->act			= pMob2->act;
    pMob->sex			= pMob2->sex;
    pMob->race			= pMob2->race;
    pMob->level			= pMob2->level;
    pMob->alignment		= pMob2->alignment;
    pMob->hitroll		= pMob2->hitroll;
    pMob->dam_type		= pMob2->dam_type;
    pMob->group			= pMob2->group;
    pMob->hit[0]		= pMob2->hit[0];
    pMob->hit[1]		= pMob2->hit[1];
    pMob->damage[DICE_NUMBER]	= pMob2->damage[DICE_NUMBER];
    pMob->damage[DICE_TYPE]	= pMob2->damage[DICE_TYPE];
    pMob->damage[DICE_BONUS]	= pMob2->damage[DICE_BONUS];
    pMob->mana[0]		= pMob2->mana[0];
    pMob->mana[1]		= pMob2->mana[1];
    pMob->affected_by		= pMob2->affected_by;
    pMob->shielded_by		= pMob2->shielded_by;
    pMob->ac[AC_PIERCE]		= pMob2->ac[AC_PIERCE];
    pMob->ac[AC_BASH]		= pMob2->ac[AC_BASH];
    pMob->ac[AC_SLASH]		= pMob2->ac[AC_SLASH];
    pMob->ac[AC_EXOTIC]		= pMob2->ac[AC_EXOTIC];
    pMob->parts			= pMob2->parts;
    pMob->size			= pMob2->size;
    pMob->start_pos		= pMob2->start_pos;
    pMob->default_pos		= pMob2->default_pos;
    pMob->wealth		= pMob2->wealth;
    pMob->spec_fun		= pMob2->spec_fun;
    pMob->class			= pMob2->class;
    pMob->skill_percentage	= pMob2->skill_percentage;
    pMob->exp_percent           = pMob2->exp_percent;
    pMob->reflection            = pMob2->reflection;
    pMob->absorption		= pMob2->absorption;
    pMob->saves			= pMob2->saves;

    for ( vnum = 0; vnum < maxSkill; vnum++ )
	pMob->learned[vnum] = pMob2->learned[vnum];

    for ( vnum = 0; vnum < DAM_MAX; vnum++ )
	pMob->damage_mod[vnum] = pMob2->damage_mod[vnum];

    send_to_char( "Mob info copied.\n\r", ch );
    return TRUE;
}

MEDIT( medit_die_desc )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !str_cmp(argument,"none") )
    {
	send_to_char( "Die description removed.\n\r", ch );
	free_string(pMob->die_descr);
	pMob->die_descr = &str_empty[0];
	return TRUE;
    }

    free_string( pMob->die_descr );
    pMob->die_descr = str_dup( argument );

    send_to_char( "Death description set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_say_desc )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !str_cmp(argument,"none") )
    {
	send_to_char( "Say description removed.\n\r", ch );
	free_string(pMob->say_descr);
	pMob->say_descr = &str_empty[0];
	return TRUE;
    }

    free_string( pMob->say_descr );
    pMob->say_descr = str_dup( argument );

    send_to_char( "Say description set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_saves )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "What do you wish to set the saves to?\n\r", ch );
	return FALSE;
    }

    pMob->saves = atoi( argument );

    send_to_char( "Saves value set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_delete )
{
    CHAR_DATA *wch, *wch_next;
    MOB_INDEX_DATA *pMob, *iMob;
    RESET_DATA *pReset, *pReset_next;
    ROOM_INDEX_DATA *pRoom;
    char buf[MAX_STRING_LENGTH];
    int count, iHash;
    bool foundmob = FALSE;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument )
    ||   atoi( argument ) != pMob->vnum )
    {
	send_to_char( "Argument must match the mobile you are editing.\n\r", ch );
	return FALSE;
    }

    check_olc_delete( ch );

    for ( wch = char_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;

	if ( IS_NPC(wch)
	&&   wch->pIndexData == pMob )
	    extract_char( wch, TRUE );
    }

    iHash = pMob->vnum % MAX_KEY_HASH;

    iMob = mob_index_hash[iHash];

    if ( iMob->next == NULL )
	mob_index_hash[iHash] = NULL;
    else if ( iMob == pMob )
	mob_index_hash[iHash] = pMob->next;
    else
    {
	for ( ; iMob != NULL; iMob = iMob->next )
	{
	    if ( iMob->next == pMob )
	    {
		iMob->next = pMob->next;
		break;
	    }
	}
    }

    count = 0;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
	{
	    for ( pReset = pRoom->reset_first; pReset; pReset = pReset_next )
	    {
		pReset_next = pReset->next;

		switch ( pReset->command )
		{
		    default:
			foundmob = FALSE;
			break;

		    case 'M':
			if ( pReset->arg1 == pMob->vnum )
			{
			    foundmob = TRUE;

			    if ( pRoom->reset_first == pReset )
				pRoom->reset_first = pReset->next;
			    else 
			    {
				RESET_DATA *prev;
				for ( prev = pRoom->reset_first; prev != NULL; prev = prev->next )
				{
				    if ( prev->next == pReset )
				    {
					prev->next = pReset->next;
					break;
				    }
				}
			    }

			    count++;
			    free_reset_data( pReset );
			}
			else
			    foundmob = FALSE;
			break;

		    case 'E':
		    case 'G':
		    case 'P':
			if ( foundmob )
			{
			    if ( pRoom->reset_first == pReset )
				pRoom->reset_first = pReset->next;
			    else
			    {
				RESET_DATA *prev;
				for ( prev = pRoom->reset_first; prev != NULL; prev = prev->next )
				{
				    if ( prev->next == pReset )
				    {
					prev->next = pReset->next;
					break;
				    }
				}
			    }

			    count++;
			    free_reset_data( pReset );
			}
			break;
		}
	    }
	}
    }

    sprintf( buf, "Removed mobile vnum {C%d{x and {C%d{x resets.\n\r",
	pMob->vnum, count );
    send_to_char( buf, ch );

    free_mob_index( pMob );

    return TRUE;
}

MEDIT( medit_create )
{
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int  value;
    int  iHash;

    value = atoi( argument );
    if ( argument[0] == '\0' || value == 0 )
    {
	send_to_char( "Syntax:  medit create [vnum]\n\r", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );

    if ( !pArea )
    {
	send_to_char( "MEdit:  That vnum is not assigned an area.\n\r", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "MEdit:  Vnum in an area you cannot build in.\n\r", ch );
	return FALSE;
    }

    if ( get_mob_index( value ) )
    {
	send_to_char( "MEdit:  Mobile vnum already exists.\n\r", ch );
	return FALSE;
    }

    pMob			= new_mob_index( );
    pMob->vnum			= value;
    pMob->area			= pArea;
    pMob->player_name		= str_dup( "no name" );
    pMob->short_descr		= str_dup( "(no short description)" );
    pMob->long_descr		= str_dup( "(no long description)\n\r" );
    pMob->description		= &str_empty[0];
        
    iHash			= value % MAX_KEY_HASH;
    pMob->next			= mob_index_hash[iHash];
    mob_index_hash[iHash]	= pMob;
    ch->desc->pEdit		= (void *)pMob;
    ch->desc->editor		= ED_MOBILE;

    send_to_char( "Mobile Created.\n\r", ch );
    return TRUE;
}

MEDIT( medit_spec )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  spec [special function]\n\r", ch );
	return FALSE;
    }


    if ( !str_cmp( argument, "none" ) )
    {
        pMob->spec_fun = NULL;

        send_to_char( "Spec removed.\n\r", ch);
        return TRUE;
    }

    if ( spec_lookup( argument ) )
    {
	pMob->spec_fun = spec_lookup( argument );
	send_to_char( "Spec set.\n\r", ch);
	return TRUE;
    }

    send_to_char( "MEdit: No such special function.\n\r", ch );
    return FALSE;
}

MEDIT( medit_damtype )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  damtype [damage message]\n\r", ch );
	send_to_char( "For a list of all available, type '? weapon'.\n\r", ch );
	return FALSE;
    }

    pMob->dam_type = attack_lookup(argument);
    send_to_char( "Damage type set.\n\r", ch);
    return TRUE;
}


MEDIT( medit_align )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  alignment [number]\n\r", ch );
	return FALSE;
    }

    pMob->alignment = atoi( argument );

    send_to_char( "Alignment set.\n\r", ch);
    return TRUE;
}



MEDIT( medit_level )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  level [number]\n\r", ch );
	return FALSE;
    }

    pMob->level = atoi( argument );

    send_to_char( "Level set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_max_world )
{
    MOB_INDEX_DATA *pMob;
    sh_int max;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  max_world [number] (0 to reset)\n\r", ch );
	return FALSE;
    }

    if ( ( max = atoi( argument ) ) < 0 || max > 100 )
    {
	send_to_char( "Values must be 0 to 100.\n\r", ch );
	return FALSE;
    }

    pMob->max_world = max;

    send_to_char( "Max world set, use 0 to reset.\n\r", ch);
    return TRUE;
}

MEDIT( medit_regen_hit )
{
    MOB_INDEX_DATA *pMob;
    sh_int max;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  regen_hit [number] (0 to reset)\n\r", ch );
	return FALSE;
    }

    if ( ( max = atoi( argument ) ) < 0 || max > 500 )
    {
	send_to_char( "Values must be 0 to 500.\n\r", ch );
	return FALSE;
    }

    pMob->regen[0] = max;

    send_to_char( "Mobile hit point regeneration set, use 0 to reset.\n\r", ch );
    return TRUE;
}

MEDIT( medit_regen_mana )
{
    MOB_INDEX_DATA *pMob;
    sh_int max;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  regen_mana [number] (0 to reset)\n\r", ch );
	return FALSE;
    }

    if ( ( max = atoi( argument ) ) < 0 || max > 500 )
    {
	send_to_char( "Values must be 0 to 500.\n\r", ch );
	return FALSE;
    }

    pMob->regen[1] = max;

    send_to_char( "Mobile mana point regeneration set, use 0 to reset.\n\r", ch );
    return TRUE;
}

MEDIT( medit_regen_move )
{
    MOB_INDEX_DATA *pMob;
    sh_int max;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  regen_move [number] (0 to reset)\n\r", ch );
	return FALSE;
    }

    if ( ( max = atoi( argument ) ) < 0 || max > 500 )
    {
	send_to_char( "Values must be 0 to 500.\n\r", ch );
	return FALSE;
    }

    pMob->regen[2] = max;

    send_to_char( "Mobile move point regeneration set, use 0 to reset.\n\r", ch );
    return TRUE;
}

MEDIT( medit_reflection )
{
    MOB_INDEX_DATA *pMob;
    sh_int max;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  reflection [number] (0 to reset)\n\r", ch );
	return FALSE;
    }

    if ( ( max = atoi( argument ) ) < 0 || max > 100 )
    {
	send_to_char( "Values must be 0 to 100.\n\r", ch );
	return FALSE;
    }

    pMob->reflection = max;

    send_to_char( "Reflection percent set, use 0 to reset.\n\r", ch);
    return TRUE;
}

MEDIT( medit_absorption )
{
    MOB_INDEX_DATA *pMob;
    sh_int max;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  absorption [number] (0 to reset)\n\r", ch );
	return FALSE;
    }

    if ( ( max = atoi( argument ) ) < 0 || max > 100 )
    {
	send_to_char( "Values must be 0 to 100.\n\r", ch );
	return FALSE;
    }

    pMob->absorption = max;

    send_to_char( "absorption percent set, use 0 to reset.\n\r", ch);
    return TRUE;
}

MEDIT( medit_bank_branch )
{
    MOB_INDEX_DATA *pMob;
    char buf[MAX_STRING_LENGTH];
    int value;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  bank_branch [number]\n\r", ch );
	return FALSE;
    }

    if ( ( value = atoi( argument ) ) < 0 || value > MAX_BANK )
    {
	sprintf( buf, "Value must be [ 0 (reset) ] or [ 1 to %d ].\n\r",
	    MAX_BANK );
	send_to_char( buf, ch );
	return FALSE;
    }

    pMob->bank_branch = value;

    send_to_char( "Bank Branch set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_desc )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	string_append( ch, &pMob->description );
	return TRUE;
    }

    send_to_char( "Syntax:  desc    - line edit\n\r", ch );
    return FALSE;
}

MEDIT( medit_long )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  long [string]\n\r", ch );
	return FALSE;
    }

    free_string( pMob->long_descr );
    strcat( argument, "\n\r" );
    pMob->long_descr = str_dup( argument );

    send_to_char( "Long description set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_short )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  short [string]\n\r", ch );
	return FALSE;
    }

    free_string( pMob->short_descr );
    pMob->short_descr = str_dup( argument );

    send_to_char( "Short description set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_name )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  name [string]\n\r", ch );
	return FALSE;
    }

    free_string( pMob->player_name );
    pMob->player_name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_shop )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
	if ( pMob->pShop == NULL )
	{
	    send_to_char( "Mobile has no shop assigned.  Use shop create to add one.\n\r", ch );
	    return FALSE;
	}

	ch->desc->pEdit = ( void * ) ( pMob->pShop );
	ch->desc->editor = ED_SHOP;
	return FALSE;
    }

    if ( !str_prefix( argument, "create" ) )
    {
	if ( pMob->pShop )
	{
	    send_to_char( "Mob already has a shop assigned to it.\n\r", ch );
	    return FALSE;
	}

	pMob->pShop		= new_shop();

	if ( !shop_first )
	    shop_first		= pMob->pShop;
	if ( shop_last )
	    shop_last->next	= pMob->pShop;
	shop_last		= pMob->pShop;

	pMob->pShop->keeper	= pMob->vnum;

	send_to_char( "New shop assigned to mobile.\n\r", ch );
        ch->desc->pEdit = ( void * ) ( pMob->pShop );
        ch->desc->editor = ED_SHOP;
	return TRUE;
    }

    if ( !str_prefix( argument, "delete" ) )
    {
        ch->desc->pEdit = ( void * ) ( pMob->pShop );
        ch->desc->editor = ED_SHOP;
	shop_edit_delete( ch, argument );
	return TRUE;
    }

    medit_shop( ch, "" );
    return FALSE;
}

MEDIT( medit_sex )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( sex_flags, argument ) ) != NO_FLAG )
	{
	    pMob->sex = value;

	    send_to_char( "Sex set.\n\r", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: sex [sex]\n\r"
		  "Type '? sex' for a list of flags.\n\r", ch );
    return FALSE;
}


MEDIT( medit_act )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( act_flags, argument ) ) != NO_FLAG )
	{
	    pMob->act ^= value;

	    send_to_char( "Act flag toggled.\n\r", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: act [flag]\n\r"
		  "Type '? act' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_affect )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( affect_flags, argument ) ) != NO_FLAG )
	{
	    pMob->affected_by ^= value;

	    send_to_char( "Affect flag toggled.\n\r", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: affect [flag]\n\r"
		  "Type '? affect' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_shield )     
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_MOB( ch, pMob );

        if ( ( value = flag_value( shield_flags, argument ) ) != NO_FLAG )
       {
            pMob->shielded_by ^= value;

            send_to_char( "Shield flag toggled.\n\r", ch);
            return TRUE;
        }
    }     
    send_to_char( "Syntax: shield [flag]\n\r"
                  "Type '? shield' for a list of flags.\n\r", ch );
    return FALSE;
} 


MEDIT( medit_ac )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    int pierce, bash, slash, exotic;

    do   /* So that I can use break and send the syntax in one place */
    {
	if ( argument[0] == '\0' )  break;

	EDIT_MOB(ch, pMob);
	argument = one_argument( argument, arg );

	if ( !is_number( arg ) )  break;
	pierce = atoi( arg );
	argument = one_argument( argument, arg );

	if ( arg[0] != '\0' )
	{
	    if ( !is_number( arg ) )  break;
	    bash = atoi( arg );
	    argument = one_argument( argument, arg );
	}
	else
	    bash = pMob->ac[AC_BASH];

	if ( arg[0] != '\0' )
	{
	    if ( !is_number( arg ) )  break;
	    slash = atoi( arg );
	    argument = one_argument( argument, arg );
	}
	else
	    slash = pMob->ac[AC_SLASH];

	if ( arg[0] != '\0' )
	{
	    if ( !is_number( arg ) )  break;
	    exotic = atoi( arg );
	}
	else
	    exotic = pMob->ac[AC_EXOTIC];

	pMob->ac[AC_PIERCE] = pierce;
	pMob->ac[AC_BASH]   = bash;
	pMob->ac[AC_SLASH]  = slash;
	pMob->ac[AC_EXOTIC] = exotic;
	
	send_to_char( "Ac set.\n\r", ch );
	return TRUE;
    } while ( FALSE );    /* Just do it once.. */

    send_to_char( "Syntax:  ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n\r"
		  "help MOB_AC  gives a list of reasonable ac-values.\n\r", ch );
    return FALSE;
}

MEDIT( medit_part )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( part_flags, argument ) ) != NO_FLAG )
	{
	    pMob->parts ^= value;
	    send_to_char( "Parts toggled.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: part [flags]\n\r"
		  "Type '? part' for a list of flags.\n\r", ch );
    return FALSE;
}

void smash_space( char *argument )
{
    for ( ; *argument != '\0'; argument++ )
    {
	if ( *argument == ' ' )
	    *argument = '_';
    }

    return;
}

MEDIT( medit_size )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( size_flags, argument ) ) != NO_FLAG )
	{
	    pMob->size = value;
	    send_to_char( "Size set.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: size [size]\n\r"
		  "Type '? size' for a list of sizes.\n\r", ch );
    return FALSE;
}

MEDIT( medit_hitpoints )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    int hit[2];

    EDIT_MOB( ch, pMob );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0'
    ||   !is_number( arg ) || !is_number( argument ) )
    {
	send_to_char( "Syntax: hitpoints (min) (max).\n\r", ch );
	return FALSE;
    }

    if ( ( hit[0] = atoi( arg ) ) < 0
    ||   ( hit[1] = atoi( argument ) ) < 0 )
    {
	send_to_char( "Hitpoint ranges must be positive numbers.\n\r", ch );
	return FALSE;
    }

    if ( hit[0] > hit[1] )
    {
	send_to_char( "Minimum value must be lower than maximum value.\n\r", ch );
	return FALSE;
    }

    pMob->hit[0] = hit[0];
    pMob->hit[1] = hit[1];

    send_to_char( "Hitpoints set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_manapoints )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    int hit[2];

    EDIT_MOB( ch, pMob );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0'
    ||   !is_number( arg ) || !is_number( argument ) )
    {
	send_to_char( "Syntax: manapoints (min) (max).\n\r", ch );
	return FALSE;
    }

    if ( ( hit[0] = atoi( arg ) ) < 0
    ||   ( hit[1] = atoi( argument ) ) < 0 )
    {
	send_to_char( "Manapoint ranges must be positive numbers.\n\r", ch );
	return FALSE;
    }

    if ( hit[0] > hit[1] )
    {
	send_to_char( "Minimum value must be lower than maximum value.\n\r", ch );
	return FALSE;
    }

    pMob->mana[0] = hit[0];
    pMob->mana[1] = hit[1];

    send_to_char( "Manapoints set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_damdice )
{
    static char syntax[] = "Syntax:  damdice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    num = cp = argument;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )  *(cp++) = '\0';

    type = cp;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) ) *(cp++) = '\0';

    bonus = cp;

    while ( isdigit( *cp ) ) ++cp;
    if ( *cp != '\0' ) *cp = '\0';

    if ( !( is_number( num ) && is_number( type ) && is_number( bonus ) ) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    if ( ( !is_number( num   ) || atoi( num   ) < 1 )
    ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
    ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    pMob->damage[DICE_NUMBER] = atoi( num   );
    pMob->damage[DICE_TYPE]   = atoi( type  );
    pMob->damage[DICE_BONUS]  = atoi( bonus );

    send_to_char( "Damdice set.\n\r", ch );
    return TRUE;
}


MEDIT( medit_race )
{
    MOB_INDEX_DATA *pMob;
    int race, pos;

    if ( argument[0] != '\0'
    && ( race = race_lookup( argument ) ) != -1 )
    {
	EDIT_MOB( ch, pMob );

	pMob->race	   = race;
	pMob->affected_by |= race_table[race].aff;
        pMob->shielded_by |= race_table[race].shd;
	pMob->parts       |= race_table[race].parts;

	for ( pos = 0; pos < DAM_MAX; pos++ )
	    pMob->damage_mod[pos] = race_table[race].damage_mod[pos];

	send_to_char( "Race set.\n\r", ch );
	return TRUE;
    }

    if ( argument[0] == '?' )
    {
	char buf[MAX_STRING_LENGTH];

	send_to_char( "Available races are:", ch );

	for ( race = 0; race_table[race].name[0] != '\0'; race++ )
	{
	    if ( ( race % 3 ) == 0 )
		send_to_char( "\n\r", ch );
	    sprintf( buf, " %-15s", race_table[race].name );
	    send_to_char( buf, ch );
	}

	send_to_char( "\n\r", ch );
	return FALSE;
    }

    send_to_char( "Syntax:  race [race]\n\r"
		  "Type 'race ?' for a list of races.\n\r", ch );
    return FALSE;
}

MEDIT( medit_position )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    int value;

    argument = one_argument( argument, arg );

    switch ( UPPER( arg[0] ) )
    {
	default:
	    break;

	case 'S':
	    if ( str_prefix( arg, "start" ) )
		break;

	    if ( ( value = flag_value( position_flags, argument ) ) == NO_FLAG )
		break;

	    EDIT_MOB( ch, pMob );

	    pMob->start_pos = value;
	    send_to_char( "Start position set.\n\r", ch );
	    return TRUE;

	case 'D':
	    if ( str_prefix( arg, "default" ) )
		break;

	    if ( ( value = flag_value( position_flags, argument ) ) == NO_FLAG )
		break;

	    EDIT_MOB( ch, pMob );

	    pMob->default_pos = value;
	    send_to_char( "Default position set.\n\r", ch );
	    return TRUE;
    }

    send_to_char( "Syntax:  position [start/default] [position]\n\r"
		  "Type '? position' for a list of positions.\n\r", ch );
    return FALSE;
}


MEDIT( medit_gold )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  wealth [number]\n\r", ch );
	return FALSE;
    }

    pMob->wealth = atoi( argument );

    send_to_char( "Wealth set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_hitroll )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  hitroll [number]\n\r", ch );
	return FALSE;
    }

    pMob->hitroll = atoi( argument );

    send_to_char( "Hitroll set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_group )
{
    MOB_INDEX_DATA *pMob;
    MOB_INDEX_DATA *pMTemp;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int temp;
    bool found = FALSE;
    
    EDIT_MOB(ch, pMob);
    
    if ( argument[0] == '\0' )
    {
    	send_to_char( "Syntax: group [number]\n\r", ch);
    	send_to_char( "        group show [number]\n\r", ch);
    	return FALSE;
    }
    
    if (is_number(argument))
    {
	pMob->group = atoi(argument);
    	send_to_char( "Group set.\n\r", ch );
	return TRUE;
    }
    
    argument = one_argument( argument, arg );
    
    if ( !strcmp( arg, "show" ) && is_number( argument ) )
    {
	if (atoi(argument) == 0)
	{
		send_to_char( "Are you crazy?\n\r", ch);
		return FALSE;
	}

    	for (temp = 0; temp < 65536; temp++)
    	{
    		pMTemp = get_mob_index(temp);
    		if ( pMTemp && ( pMTemp->group == atoi(argument) ) )
    		{
			found = TRUE;
    			sprintf( buf, "[%5d] %s\n\r", pMTemp->vnum, pMTemp->player_name );
			send_to_char(buf,ch);
    		}
    	}

	if (!found)
	{
		send_to_char( "No mobs in that group.\n\r", ch );
        	return FALSE;
	}
    }
    
    return FALSE;
}

MEDIT ( medit_addmprog )
{
  int value;
  MOB_INDEX_DATA *pMob;
  PROG_LIST *list;
  PROG_CODE *code;
  char trigger[MAX_STRING_LENGTH];
  char phrase[MAX_STRING_LENGTH];
  char num[MAX_STRING_LENGTH];

  EDIT_MOB(ch, pMob);
  argument=one_argument(argument, num);
  argument=one_argument(argument, trigger);
  argument=one_argument(argument, phrase);

  if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0' )
  {
        send_to_char("Syntax:   addmprog [vnum] [trigger] [phrase]\n\r",ch);
        return FALSE;
  }

  if ( (value = flag_value (mprog_flags, trigger) ) == NO_FLAG)
  {
        send_to_char("Valid flags are:\n\r",ch);
        show_help( ch, "mprog");
        return FALSE;
  }

  if ( ( code = get_prog_index (atoi(num), PRG_MPROG ) ) == NULL)
  {
        send_to_char("No such MOBProgram.\n\r",ch);
        return FALSE;
  }

  list                  = new_prog();
  list->vnum            = atoi(num);
  list->trig_type       = value;
  list->trig_phrase     = str_dup(phrase);
  free_string( list->code );
  list->code            = str_dup(code->code);
  SET_BIT(pMob->mprog_flags,value);
  list->next            = pMob->mprogs;
  pMob->mprogs          = list;

  send_to_char( "Mprog Added.\n\r",ch);
  return TRUE;
}

MEDIT ( medit_delmprog )
{
    MOB_INDEX_DATA *pMob;
    PROG_LIST *list;
    PROG_LIST *list_next;
    char mprog[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;

    EDIT_MOB(ch, pMob);

    one_argument( argument, mprog );
    if (!is_number( mprog ) || mprog[0] == '\0' )
    {
       send_to_char("Syntax:  delmprog [#mprog]\n\r",ch);
       return FALSE;
    }

    value = atoi ( mprog );

    if ( value < 0 )
    {
        send_to_char("Only non-negative mprog-numbers allowed.\n\r",ch);
        return FALSE;
    }

    if ( !(list= pMob->mprogs) )
    {
        send_to_char("MEdit:  Non existant mprog.\n\r",ch);
        return FALSE;
    }

    if ( value == 0 )
    {
	REMOVE_BIT(pMob->mprog_flags, pMob->mprogs->trig_type);
        list = pMob->mprogs;
        pMob->mprogs = list->next;
        free_prog( list );
    }
    else
    {
        while ( (list_next = list->next) && (++cnt < value ) )
                list = list_next;

        if ( list_next )
        {
		REMOVE_BIT(pMob->mprog_flags, list_next->trig_type);
                list->next = list_next->next;
                free_prog(list_next);
        }
        else
        {
                send_to_char("No such mprog.\n\r",ch);
                return FALSE;
        }
    }

    send_to_char("Mprog removed.\n\r", ch);
    return TRUE;
}

MPEDIT ( mpedit_create )
{
    PROG_CODE *pMcode;
    int value;

    value = atoi( argument );
    if ( argument[0] == '\0' || value == 0 )
    {
	send_to_char( "Syntax: mpedit create [vnum]\n\r", ch );
	return FALSE;
    }

    if ( get_prog_index( value, PRG_MPROG ) )
    {
	send_to_char( "MPEdit: Code vnum already exists.\n\r", ch );
	return FALSE;
    }

    if ( !get_vnum_area( value ) )
    {
    	send_to_char( "MPEdit: Vnum not assigned an area.\n\r", ch );
    	return FALSE;
    }

    pMcode		= new_pcode( );
    pMcode->author	= str_dup( ch->name );
    pMcode->vnum	= value;
    pMcode->area	= get_vnum_area( value );
    pMcode->next	= mprog_list;
    mprog_list		= pMcode;
    ch->desc->pEdit	= (void *)pMcode;
    ch->desc->editor	= ED_MPCODE;
    SET_BIT( pMcode->area->area_flags, AREA_CHANGED );
    send_to_char( "MobProgram Code Created.\n\r", ch );
    return TRUE;
}

MPEDIT( mpedit_delete )
{
    MOB_INDEX_DATA *pMob;
    PROG_CODE *pMcode;
    PROG_LIST *mp, *mp_next;
    char buf[MAX_STRING_LENGTH];
    int count = 0, match = 0, vnum;

    EDIT_PCODE( ch, pMcode );

    if ( argument[0] == '\0' || !is_number( argument )
    ||   atoi( argument ) != pMcode->vnum )
    {
	send_to_char( "Argument must match the program you are editing.\n\r", ch );
	return FALSE;
    }

    check_olc_delete( ch );

    for ( vnum = 0; match < top_mob_index; vnum++ )
    {
	if ( ( pMob = get_mob_index( vnum ) ) != NULL )
	{
	    match++;
	    for ( mp = pMob->mprogs; mp != NULL; mp = mp_next )
	    {
		mp_next = mp->next;

		if ( mp->vnum == pMcode->vnum )
		{
		    if ( mp == pMob->mprogs )
			pMob->mprogs = pMob->mprogs->next;
		    else
		    {
			PROG_LIST *prev;

			for ( prev = pMob->mprogs; prev != NULL; prev = prev->next )
			{
			    if ( prev->next == mp )
			    {
				prev->next = mp->next;
				break;
			    }
			}
		    }

		    count++;
		    SET_BIT( pMob->area->area_flags, AREA_CHANGED );
		    free_prog( mp );
		}
	    }
	}
    }

    if ( pMcode == mprog_list )
	mprog_list = pMcode->next;
    else
    {
	PROG_CODE *prev;

	for ( prev = mprog_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == pMcode )
	    {
		prev->next = pMcode->next;
		break;
	    }
	}
    }

    sprintf( buf, "Deleted mprog [%d] and removed from %d mobile%s.\n\r",
	pMcode->vnum, count, count == 1 ? "" : "s" );
    send_to_char( buf, ch );

    free_pcode( pMcode );

    return TRUE;
}

MPEDIT( mpedit_show )
{
    CHAR_DATA *wch;
    MOB_INDEX_DATA *pMob;
    PROG_CODE *pMcode;
    PROG_LIST *mp;
    char final[MAX_STRING_LENGTH];
    int match = 0, vnum;

    EDIT_PCODE( ch, pMcode );

    send_to_char( "{w******************************************************************\n\r", ch );

    sprintf( final, "* {BName{D: {m%s {BVnum{D: {C[{c%-5d{C]         {w*\n\r",
	end_string(pMcode->name,34), pMcode->vnum );
    send_to_char( final, ch );

    sprintf( final, "* {BAuthor{D: %-15.15s                                        {w*\n\r",
	pMcode->author );
    send_to_char( final, ch );

    send_to_char( "******************************************************************\n\r", ch );
    send_to_char( "* {BCode{D:                                                          {w*{x\n\r", ch );

    send_to_char( pMcode->code, ch );

    send_to_char( "{w******************************************************************\n\r", ch );
    send_to_char( "* {BMobile Vnums On                                    {BMobile Vnum {w*\n\r", ch );
    send_to_char( "******************************************************************\n\r", ch );

    for ( vnum = 0; match < top_mob_index; vnum++ )
    {
	if ( ( pMob = get_mob_index( vnum ) ) != NULL )
	{
	    match++;
	    for ( mp = pMob->mprogs; mp != NULL; mp = mp->next )
	    {
		if ( mp->vnum == pMcode->vnum )
		{
		    sprintf( final, "* %s{w                    {y%5d {w*\n\r",
			end_string(pMob->short_descr,37),
			pMob->vnum );
		    send_to_char( final, ch );
		}
	    }
	}
    }

    send_to_char( "{w******************************************************************\n\r", ch );
    send_to_char( "* {BMobiles On                           {w* {BRoom Vnum {w* {BMobile Vnum {w*\n\r", ch );
    send_to_char( "******************************************************************\n\r", ch );

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( IS_NPC( wch )
	&&   wch->pIndexData->mprogs != NULL )
	{
	    for ( mp = wch->pIndexData->mprogs; mp != NULL; mp = mp->next )
	    {
		if ( mp->vnum == pMcode->vnum )
		{
		    sprintf( final, "* %s{w*     {y%d {w*        {y%5d {w*\n\r",
			end_string(wch->short_descr,37),
			wch->in_room ? wch->in_room->vnum : 0,
			wch->pIndexData->vnum );
		    send_to_char( final, ch );
		}
	    }
	}
    }

    send_to_char( "******************************************************************{x\n\r", ch );

    return FALSE;
}

MPEDIT( mpedit_code )
{
    PROG_CODE *pMcode;
    EDIT_PCODE(ch, pMcode);

    string_append(ch, &pMcode->code);
    SET_BIT( pMcode->area->area_flags, AREA_CHANGED );
    return TRUE;
}

MPEDIT( mpedit_name )
{
    PROG_CODE *pMcode;
    EDIT_PCODE(ch, pMcode);

    if ( argument[0] != '\0' )
    {
	free_string(pMcode->name);
	pMcode->name = str_dup(argument);
	SET_BIT( pMcode->area->area_flags, AREA_CHANGED );
	send_to_char( "Name description set.\n\r", ch );
	return TRUE;
    }

    send_to_char(" Syntax: mpedit name <name>\n\r",ch);
    return FALSE;
}

MPEDIT( mpedit_author )
{
    PROG_CODE *pMcode;
    EDIT_PCODE(ch, pMcode);

    if ( argument[0] != '\0' )
    {
	free_string(pMcode->author);
	pMcode->author = str_dup(argument);
	SET_BIT( pMcode->area->area_flags, AREA_CHANGED );
	send_to_char( "Author description set.\n\r", ch );
	return TRUE;
    }

    send_to_char(" Syntax: mpedit author <name>\n\r",ch);
    return FALSE;
}

OLC( class_edit_base_group )
{
    char buf[MAX_STRING_LENGTH];
    int class, value;

    EDIT_TABLE( ch, class );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GBase Group{w: {g%s{x\n\r",
	    class_table[class].base_group );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( ( value = group_lookup( argument ) ) == -1 )
    {
	send_to_char( "Invalid group.\n\r", ch );
	return FALSE;
    }

    sprintf(buf,"{GClass Edit{w: ({y%s{w) {Gbase group changed from {g%s {Gto {g%s{G.{x\n\r",
	capitalize( class_table[class].name ),
	class_table[class].base_group, group_table[value].name );
    send_to_char( buf, ch );

    free_string( class_table[class].base_group );
    class_table[class].base_group = str_dup( group_table[value].name );

    return TRUE;

}

OLC( class_edit_copy )
{
    char buf[MAX_STRING_LENGTH];
    int class, value;

    EDIT_TABLE( ch, class );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Which class do you wish to copy?\n\r", ch );
	return FALSE;
    }

    if ( ( value = class_lookup( argument ) ) == -1 )
    {
	send_to_char( "Invalid class to copy.\n\r", ch );
	return FALSE;
    }

    class_table[class].attr_prime	= class_table[value].attr_prime;
    class_table[class].thac0_00		= class_table[value].thac0_00;
    class_table[class].thac0_32		= class_table[value].thac0_32;
    class_table[class].hp_min		= class_table[value].hp_min;
    class_table[class].hp_max		= class_table[value].hp_max;
    class_table[class].mana_percent	= class_table[value].mana_percent;
    class_table[class].sub_class	= class_table[value].sub_class;
    class_table[class].tier		= class_table[value].tier;

    free_string( class_table[class].base_group );
    class_table[class].base_group	= str_dup( class_table[value].base_group );

    free_string( class_table[class].default_group );
    class_table[class].default_group	= str_dup( class_table[value].default_group );

    sprintf(buf,"{GClass Edit{w: ({y%s{w) {GClass info copied from {g%s{G.{x\n\r",
	capitalize( class_table[class].name ), class_table[value].name );
    send_to_char( buf, ch );

    return TRUE;
}

OLC( class_edit_create )
{
    struct class_type *new_table;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
	send_to_char("Which class do you wish to create?\n\r",ch);
	return FALSE;
    }

    if ( !check_argument( ch, argument, FALSE ) )
	return FALSE;

    if ( class_lookup( argument ) != -1 )
    {
	send_to_char( "That class already exists.\n\r", ch );
	return FALSE;
    }

    maxClass++;
    new_table = realloc( class_table, sizeof( struct class_type ) * ( maxClass + 1 ) );

    if ( !new_table )
    {
	send_to_char( "Memory allocation failed. Brace for impact.\n\r", ch );
	return FALSE;
    }

    class_table					= new_table;
    class_table[maxClass-1].name		= str_dup( argument );
    class_table[maxClass-1].attr_prime		= STAT_STR;
    class_table[maxClass-1].thac0_00		= 0;
    class_table[maxClass-1].thac0_32		= 0;
    class_table[maxClass-1].hp_min		= 0;
    class_table[maxClass-1].hp_max		= 0;
    class_table[maxClass-1].mana_percent	= 50;
    class_table[maxClass-1].disabled		= TRUE;
    class_table[maxClass-1].sub_class		= maxClass-1;
    class_table[maxClass-1].tier		= 1;

    strcpy( class_table[maxClass-1].who_name, "{RW{rho{x" );

    sprintf( buf, "%s basics", argument );
    class_table[maxClass-1].base_group		= str_dup( buf );

    sprintf( buf, "%s default", argument );
    class_table[maxClass-1].default_group	= str_dup( buf );

    class_table[maxClass].name = str_dup( "" );

    sprintf(buf,"{GClass Edit{w: {GNew class created{w, {y%s{G.{x\n\r", argument );
    send_to_char( buf, ch );

    update_class_data( -1 );

    ch->desc->editor = ED_CLASS;
    ch->desc->pEdit = (void *)maxClass-1;

    return TRUE;
}

OLC( class_edit_def_group )
{
    char buf[MAX_STRING_LENGTH];
    int class, value;

    EDIT_TABLE( ch, class );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GDefault Group{w: {g%s{x\n\r",
	    class_table[class].default_group );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( ( value = group_lookup( argument ) ) == -1 )
    {
	send_to_char( "Invalid group.\n\r", ch );
	return FALSE;
    }

    sprintf(buf,"{GClass Edit{w: ({y%s{w) {Gdefault group changed from {g%s {Gto {g%s{G.{x\n\r",
	capitalize( class_table[class].name ),
	class_table[class].default_group, group_table[value].name );
    send_to_char( buf, ch );

    free_string( class_table[class].default_group );
    class_table[class].default_group = str_dup( group_table[value].name );

    return TRUE;
}

OLC( class_edit_delete )
{
    CHAR_DATA *wch;
    DESCRIPTOR_DATA *d;
    MOB_INDEX_DATA *pMob;
    struct class_type *new_table;
    int class, i = 0, lvl, vnum;

    EDIT_TABLE( ch, class );

    if ( argument[0] == '\0'
    ||   str_cmp( argument, class_table[class].name ) )
    {
	send_to_char( "Argument must match class name of the class you are editing.\n\r", ch );
	return FALSE;
    }

    check_olc_delete( ch );

    new_table = malloc( sizeof( struct class_type ) * maxClass );

    if ( !new_table )
    {
	send_to_char( "Memory allocation failed. Brace for impact...\n\r", ch );
	return FALSE;
    }

    for ( vnum = 0; i < top_mob_index; vnum++ )
    {
	if ( ( pMob = get_mob_index( vnum ) ) != NULL )
	{
	    i++;
	    if ( pMob->class == class )
		pMob->class = 0;

	    else if ( pMob->class > class )
		pMob->class--;
	}
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING || d->character == NULL )
	    continue;

	else if ( d->character->class == class )
	    d->character->class = 0;

	else if ( d->character->class > class )
	    d->character->class--;
    }

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch->class == class )
	{
	    send_to_char( "Your class was just deleted.  Contact an Immortal for assistance.\n\r", ch );
	    wch->class = 0;
	}

	else if ( wch->class > class )
	    wch->class--;
    }

    for ( lvl = 0; lvl < maxClass; lvl++ )
    {
	if ( class_table[lvl].sub_class == class )
	    class_table[lvl].sub_class = 0;

	else if ( class_table[lvl].sub_class > class )
	    class_table[lvl].sub_class--;
    }

    free_string( class_table[class].name		);
    free_string( class_table[class].base_group		);
    free_string( class_table[class].default_group	);

    class_table[class].name		= NULL;
    class_table[class].attr_prime	= STAT_STR;
    class_table[class].thac0_00		= 0;
    class_table[class].thac0_32		= 0;
    class_table[class].hp_min		= 0;
    class_table[class].hp_max		= 0;
    class_table[class].mana_percent	= 25;
    class_table[class].disabled		= TRUE;
    class_table[class].base_group	= NULL;
    class_table[class].default_group	= NULL;
    class_table[class].sub_class	= 0;
    class_table[class].tier		= 1;

    for ( i = 0, lvl = 0; i < maxClass+1; i++ )
    {
	if ( i != class )
	{
	    new_table[lvl] = class_table[i];
	    lvl++;
	}
    }

    maxClass--; /* Important :() */

    update_class_data( class );

    free( class_table );
    class_table = new_table;
		
    send_to_char( "Class deleted.\n\r", ch );

    return TRUE;
}

OLC( class_edit_disabled )
{
    char buf[MAX_STRING_LENGTH];
    int class;

    EDIT_TABLE( ch, class );

    sprintf( buf, "{GClass Edit{w: ({y%s{w) {Gdisabled_class changed from {g%s {Gto {g%s{G.{x\n\r",
	capitalize( class_table[class].name ),
	show_true_false( class_table[class].disabled ),
	show_true_false( !class_table[class].disabled ) );
    send_to_char( buf, ch );

    class_table[class].disabled = !class_table[class].disabled;

    return TRUE;
}

OLC( class_edit_groups )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int class, cost, gn;

    EDIT_TABLE( ch, class );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number( arg ) || argument[0] == '\0' )
    {
	send_to_char( "Syntax: groups <cost> <group>\n\r", ch );
	return FALSE;
    }

    cost = atoi( arg );
    gn = group_lookup( argument );

    if ( cost < -1 )
    {
	send_to_char( "Invalid cost.\n\r", ch );
	return FALSE;
    }

    if ( gn == -1 )
    {
	send_to_char( "Invalid group.\n\r", ch );
	return FALSE;
    }

    sprintf( buf,	"{qClass edit: {t%s {s[{t%s{s] {qold cost: {t%d{q, new cost: {t%d{x\n\r",
	class_table[class].name, group_table[gn].name,
	group_table[gn].rating[class], cost );
    send_to_char( buf, ch );

    group_table[gn].rating[class] = cost;

    return TRUE;
}

OLC( class_edit_hitpoints )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int class, value[2];

    EDIT_TABLE( ch, class );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	sprintf( buf, "{GHit Points{w:      {g%d {Gto {g%d\n\r",
	    class_table[class].hp_min, class_table[class].hp_max );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( !is_number( arg ) || !is_number( argument ) )
    {
	send_to_char( "Hitpoints (min_hp) (max_hp).\n\r", ch );
	return FALSE;
    }

    value[0] = atoi( arg );
    value[1] = atoi( argument );

    if ( value[0] < 0 || value[0] > 100
    ||   value[1] < 0 || value[1] > 100 )
    {
	send_to_char( "Invalid numbers.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "{GClass Edit{w: ({y%s{w) {GMin HP changed from {g%d {Gto {g%d{G.{x\n\r",
	capitalize( class_table[class].name ),
	class_table[class].hp_min, value[0] );
    send_to_char( buf, ch );

    sprintf( buf, "{GClass Edit{w: ({y%s{w) {GMax HP changed from {g%d {Gto {g%d{G.{x\n\r",
	capitalize( class_table[class].name ),
	class_table[class].hp_max, value[1] );
    send_to_char( buf, ch );

    class_table[class].hp_min = value[0];
    class_table[class].hp_max = value[1];

    return TRUE;
}

OLC( class_edit_mana_class )
{
    char buf[MAX_STRING_LENGTH];
    int class, value;

    EDIT_TABLE( ch, class );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GMana Percent{w: {g%d%%\n\r",
	    class_table[class].mana_percent );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( !is_number( argument )
    ||   ( value = atoi( argument ) ) < 0 || value > 100 )
    {
	send_to_char( "Mana percents must be 0 to 100.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "{GClass Edit{w: ({y%s{w) {Gmana percent changed from {g%d {Gto {g%d{G.{x\n\r",
	capitalize( class_table[class].name ),
	class_table[class].mana_percent, value );
    send_to_char( buf, ch );

    class_table[class].mana_percent = value;

    return TRUE;
}

OLC( class_edit_max_hp )
{
    char buf[MAX_STRING_LENGTH];
    int class, value;

    EDIT_TABLE( ch, class );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GMax HP{w: {g%d{x\n\r",
	    class_table[class].hp_max );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( !is_number( argument )
    ||   ( value = atoi( argument ) ) < 0
    ||   value > 100 )
    {
	send_to_char("Invalid number.\n\r",ch);
	return FALSE;
    }

    sprintf( buf, "{GClass Edit{w: ({y%s{w) {GMax HP changed from {g%d {Gto {g%d{G.{x\n\r",
	capitalize( class_table[class].name ),
	class_table[class].hp_max, value );
    send_to_char( buf, ch );

    class_table[class].hp_max = value;

    return TRUE;
}

OLC( class_edit_min_hp )
{
    char buf[MAX_STRING_LENGTH];
    int class, value;

    EDIT_TABLE( ch, class );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GMin HP{w: {g%d{x\n\r", class_table[class].hp_min );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( !is_number( argument )
    ||   ( value = atoi( argument ) ) < 0
    ||   value > 100 )
    {
	send_to_char("Invalid number.\n\r",ch);
	return FALSE;
    }

    sprintf( buf, "{GClass Edit{w: ({y%s{w) {GMin HP changed from {g%d {Gto {g%d{G.{x\n\r",
	capitalize( class_table[class].name ),
	class_table[class].hp_min, value );
    send_to_char( buf, ch );

    class_table[class].hp_min = value;

    return TRUE;
}

OLC( class_edit_name )
{
    MOB_INDEX_DATA *pMob;
    char buf[MAX_STRING_LENGTH], name[50];
    int class, i, vnum;

    EDIT_TABLE( ch, class );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: name <name>.\n\r", ch );
	return FALSE;
    }

    if ( !check_argument( ch, argument, FALSE ) )
	return FALSE;

    sprintf( buf, "Class Edit: (%s) renamed to %s.\n\r",
	capitalize( class_table[class].name ), argument );
    send_to_char( buf, ch );

    sprintf( buf, "grep -rl 'Clas %s~' ../player/ | xargs perl -pi -e 's/Clas %s~/Clas %s~/'",
	class_table[class].name, class_table[class].name, argument );
    system( buf );

    sprintf( buf, "grep -rl \"Gr '%s basics'\" ../player/ | xargs perl -pi -e \"s/Gr '%s basics'/Gr '%s basics'/\"",
	class_table[class].name, class_table[class].name, argument );
    system( buf );

    sprintf( buf, "grep -rl \"Gr '%s default'\" ../player/ | xargs perl -pi -e \"s/Gr '%s default'/Gr '%s default'/\"",
	class_table[class].name, class_table[class].name, argument );
    system( buf );

    for ( i = 0; group_table[i].name[0] != '\0'; i++ )
    {
	sprintf( name, "%s default", class_table[class].name );
	if ( !str_cmp( group_table[i].name, name ) )
	{
	    sprintf( name, "%s default", argument );
	    free_string( group_table[i].name );
	    group_table[i].name = str_dup( name );
	    mud_stat.skills_changed = TRUE;
	}

	sprintf( name, "%s basics", class_table[class].name );
	if ( !str_cmp( group_table[i].name, name ) )
	{
	    sprintf( name, "%s basics", argument );
	    free_string( group_table[i].name );
	    group_table[i].name = str_dup( name );
	    mud_stat.skills_changed = TRUE;
	}
    }

    free_string( class_table[class].name );
    class_table[class].name = str_dup( argument );

    mud_stat.races_changed = TRUE;

    for ( i = 0, vnum = 0; i < top_mob_index; vnum++ )
    {
	if ( ( pMob = get_mob_index( vnum ) ) != NULL )
	{
	    i++;
	    if ( pMob->class == class )
		SET_BIT( pMob->area->area_flags, AREA_CHANGED );
	}
    }

    return TRUE;
}

OLC( class_edit_show )
{
    BUFFER *final = new_buf( );
    char buf[MAX_STRING_LENGTH];
    int class;

    EDIT_TABLE( ch, class );

    sprintf( buf, "{qNumber:          {s[{t%d{s]\n\r", class );
    add_buf( final, buf );

    sprintf( buf, "{qName:            {s[{t%s{s]\n\r",
	class_table[class].name );
    add_buf( final, buf );

    sprintf( buf, "{qWho Name:        {s[{t%s{s]\n\r"
		  "                 {s[{t%s{s]\n\r",
	class_table[class].who_name,
	double_color( class_table[class].who_name ) );
    add_buf( final, buf );

    sprintf( buf, "{qPrime Attribute: {s[{t%s{s]\n\r",
	stat_names[class_table[class].attr_prime] );
    add_buf( final, buf );

    sprintf( buf, "{qThac:            {s[{q00: {t%d{s, {q32: {t%d{s]\n\r",
	class_table[class].thac0_00, class_table[class].thac0_32 );
    add_buf( final, buf );

    sprintf( buf, "{qHit Points:      {s[{t%d {s- {t%d{s]\n\r",
	class_table[class].hp_min, class_table[class].hp_max );
    add_buf( final, buf );

    sprintf( buf, "{qMana Percent:    {s[{t%d%%{s]\n\r",
	class_table[class].mana_percent );
    add_buf( final, buf );

    sprintf( buf, "{qDisabled Class:  {s[{t%s{s]\n\r",
	show_true_false( class_table[class].disabled ) );
    add_buf( final, buf );

    sprintf( buf, "{qBase Group:      {s[{t%s{s]\n\r",
	class_table[class].base_group );
    add_buf( final, buf );

    sprintf( buf, "{qDefault Group:   {s[{t%s{s]\n\r",
	class_table[class].default_group );
    add_buf( final, buf );

    sprintf( buf, "{qSub Class:       {s[{t%d{s] [{t%s{s]\n\r",
	class_table[class].sub_class,
	class_table[class_table[class].sub_class].name );
    add_buf( final, buf );

    sprintf( buf, "{qTier:            {s[{t%d{s]{x\n\r",
	class_table[class].tier );
    add_buf( final, buf );

    if ( argument[0] != '\0' )
    {
	char skill_list[LEVEL_IMMORTAL][1024], skill_columns[LEVEL_IMMORTAL];
	char spell_list[LEVEL_IMMORTAL][1024], spell_columns[LEVEL_IMMORTAL];
	bool fSkill = FALSE, fSpell = FALSE;
	sh_int pos, lev = 0;

	add_buf( final, "\n\r{qGroups:\n\r" );
	for ( pos = 0; group_table[pos].name[0] != '\0'; pos++ )
	{
	    if ( group_table[pos].rating[class] != -1 )
	    {
		sprintf( buf, "{s({q%2d{s) {t%-20.20s",
		    group_table[pos].rating[class], group_table[pos].name );

		if ( ++lev % 3 == 0 )
		    strcat( buf, "\n\r" );

		add_buf( final, buf );
	    }
	}

	if ( lev % 3 != 0 )
	    add_buf( final, "\n\r" );

	for ( lev = 0; lev < LEVEL_IMMORTAL; lev++ )
	{
	    skill_columns[lev] = 0;
	    skill_list[lev][0] = '\0';
	    spell_columns[lev] = 0;
	    spell_list[lev][0] = '\0';
	}

	for ( pos = 0; skill_table[pos].name[0] != '\0'; pos++ )
	{
	    if ( skill_table[pos].skill_level[class] < LEVEL_IMMORTAL )
	    {
		lev = skill_table[pos].skill_level[class];

		sprintf( buf, "{s({q%2d{s) {t%-16.16s",
		    skill_table[pos].rating[class], skill_table[pos].name );

		if ( skill_table[pos].spell_fun == spell_null )
		{
		    fSkill = TRUE;
		    if ( skill_list[lev][0] == '\0' )
			sprintf( skill_list[lev], "\n\r{qLevel {s[{t%3d{s] %s", lev, buf );
		    else
		    {
			if ( ++skill_columns[lev] % 3 == 0 )
			    strcat( skill_list[lev], "\n\r            " );
			strcat( skill_list[lev], buf );
		    }
		} else {
		    fSpell = TRUE;
		    if ( spell_list[lev][0] == '\0' )
			sprintf( spell_list[lev], "\n\r{qLevel {s[{t%3d{s] %s", lev, buf );
		    else
		    {
			if ( ++spell_columns[lev] % 3 == 0 )
			    strcat( spell_list[lev], "\n\r            " );
			strcat( spell_list[lev], buf );
		    }
		}
	    }
	}

	if ( !fSkill )
	{
	    sprintf( buf, "\n\r{qThe {t%s {qclass knows no skills.",
		class_table[class].name );
	    add_buf( final, buf );
	} else {
	    add_buf( final, "\n\r{qSkills:" );
	    for ( lev = 0; lev < LEVEL_IMMORTAL; lev++ )
	    {
		if ( skill_list[lev][0] != '\0' )
		    add_buf( final, skill_list[lev] );
	    }
	}

	if ( !fSpell )
	{
	    sprintf( buf, "\n\r\n\r{qThe {t%s {qclass knows no spells.",
		class_table[class].name );
	    add_buf( final, buf );
	} else {
	    add_buf( final, "\n\r\n\r{qSpells:" );
	    for ( lev = 0; lev < LEVEL_IMMORTAL; lev++ )
	    {
		if ( spell_list[lev][0] != '\0' )
		    add_buf( final, spell_list[lev] );
	    }
	}

	add_buf( final, "{x\n\r" );
    }

    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

OLC( class_edit_skills )
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int class, level, sn, rating;

    EDIT_TABLE( ch, class );

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0'
    ||   !is_number( arg1 ) || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: skills <level> <rating> <skill>\n\r", ch );
	return FALSE;
    }

    level = atoi( arg1 );
    rating = atoi( arg2 );
    sn = skill_lookup( argument );

    if ( level < 1 || level > MAX_LEVEL )
    {
	send_to_char( "Invalid level.\n\r", ch );
	return FALSE;
    }

    if ( rating < 0 )
    {
	send_to_char( "Invalid rating.\n\r", ch );
	return FALSE;
    }

    if ( sn == -1 )
    {
	send_to_char( "Invalid skill.\n\r", ch );
	return FALSE;
    }

    if ( level > LEVEL_HERO && skill_table[sn].spell_fun == spell_null )
    {
	send_to_char( "Auto setting rating to 0 for Immortal level.\n\r", ch );
	rating = 0;
    }

    if ( rating == 0 && level < LEVEL_IMMORTAL )
    {
	send_to_char( "Auto setting level to Immortal for ungainable rating.\n\r", ch );
	level = LEVEL_IMMORTAL;
    }

    sprintf( buf,	"{qClass edit: {t%s {s[{t%s{s]\n\r"
			"{qOld data: {tlevel {s%d{t, rating {s%d{t.\n\r"
			"{qNew data: {tlevel {s%d{t, rating {s%d{t.{x\n\r",
	class_table[class].name, skill_table[sn].name,
	skill_table[sn].skill_level[class], skill_table[sn].rating[class],
	level, rating );
    send_to_char( buf, ch );

    skill_table[sn].skill_level[class] = level;
    skill_table[sn].rating[class] = rating;

    return TRUE;
}

OLC( class_edit_stat )
{
    char buf[MAX_STRING_LENGTH];
    int class, stat;

    EDIT_TABLE( ch, class );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GPrime Attribute{w: {g%s\n\r",
	    stat_names[class_table[class].attr_prime] );
	send_to_char( buf, ch );
	return FALSE;
    }

    stat = is_number( argument ) ? atoi( argument ) : stat_lookup( argument );

    if ( stat < 0 || stat >= MAX_STATS )
    {
	send_to_char( "Prime Stat must be: str int wis dex con.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "{GClass Edit{w: ({y%s{w) {GPrime Attribute changed from {g%s {Gto {g%s{G.{x\n\r",
	capitalize( class_table[class].name ),
	stat_names[class_table[class].attr_prime], stat_names[stat] );
    send_to_char( buf, ch );

    class_table[class].attr_prime = stat;

    return TRUE;
}

OLC( class_edit_sub_class )
{
    char buf[MAX_STRING_LENGTH];
    int class, value;

    EDIT_TABLE( ch, class );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GSub Class{w: [{g%d{w] {g%s\n\r",
	    class_table[class].sub_class,
	    class_table[class_table[class].sub_class].name );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( ( value = class_lookup( argument ) ) == -1 )
    {
	send_to_char( "Invalid class.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "{GClass Edit{w: ({y%s{w) {GSub Class changed from {g%s {Gto {g%s{G.{x\n\r",
	capitalize( class_table[class].name ),
	class_table[class_table[class].sub_class].name,
	class_table[value].name );
    send_to_char( buf, ch );

    class_table[class].sub_class = value;

    return TRUE;
}

OLC( class_edit_thac )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int class, value[2];

    EDIT_TABLE( ch, class );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	sprintf( buf, "{GThac{w:            {G00{w: {g%d {G32{w: {g%d\n\r",
	    class_table[class].thac0_00, class_table[class].thac0_32 );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( !is_number( arg ) || !is_number( argument ) )
    {
	send_to_char( "Thac (thac00) (thac32).\n\r", ch );
	return FALSE;
    }

    value[0] = atoi( arg );
    value[1] = atoi( argument );

    if ( value[0] < -50 || value[0] > 50
    ||   value[1] < -50 || value[1] > 50 )
    {
	send_to_char( "Invalid numbers.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "{GClass Edit{w: ({y%s{w) {GThac00 changed from {g%d {Gto {g%d{G.{x\n\r",
	capitalize( class_table[class].name ),
	class_table[class].thac0_00, value[0] );
    send_to_char( buf, ch );

    sprintf( buf, "{GClass Edit{w: ({y%s{w) {GThac32 changed from {g%d {Gto {g%d{G.{x\n\r",
	capitalize( class_table[class].name ),
	class_table[class].thac0_32, value[1] );
    send_to_char( buf, ch );

    class_table[class].thac0_00 = value[0];
    class_table[class].thac0_32 = value[1];

    return TRUE;
}

OLC( class_edit_thac00 )
{
    char buf[MAX_STRING_LENGTH];
    int class, value;

    EDIT_TABLE( ch, class );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GThac00{w: {g%d{x\n\r", class_table[class].thac0_00 );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( !is_number( argument )
    ||   ( value = atoi( argument ) ) < -50
    ||   value > 50 )
    {
	send_to_char("Invalid number.\n\r",ch);
	return FALSE;
    }

    sprintf( buf, "{GClass Edit{w: ({y%s{w) {GThac00 changed from {g%d {Gto {g%d{G.{x\n\r",
	capitalize( class_table[class].name ),
	class_table[class].thac0_00, value );
    send_to_char( buf, ch );

    class_table[class].thac0_00 = value;

    return TRUE;
}

OLC( class_edit_thac32 )
{
    char buf[MAX_STRING_LENGTH];
    int class, value;

    EDIT_TABLE( ch, class );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GThac32{w: {g%d{x\n\r", class_table[class].thac0_32 );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( !is_number( argument )
    ||   ( value = atoi( argument ) ) < -50
    ||   value > 50 )
    {
	send_to_char( "Invalid number.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "{GClass Edit{w: ({y%s{w) {GThac32 changed from {g%d {Gto {g%d{G.{x\n\r",
	capitalize( class_table[class].name ),
	class_table[class].thac0_32, value );
    send_to_char( buf, ch );

    class_table[class].thac0_32 = value;

    return TRUE;
}

OLC( class_edit_tier )
{
    char buf[MAX_STRING_LENGTH];
    int class, max_tier = 1, value;

    EDIT_TABLE( ch, class );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GTier{w: {g%d{x\n\r", class_table[class].tier );
	send_to_char( buf, ch );
	return FALSE;
    }

    for ( value = 0; class_table[value].name[0] != '\0'; value++ )
    {
	if ( value != class )
	    max_tier = UMAX( max_tier, class_table[value].tier+1 );
    }

    if ( !is_number( argument )
    ||   ( value = atoi( argument ) ) < 1
    ||   value > max_tier )
    {
	sprintf( buf, "Invalid number [ 1 to %d ].\n\r", max_tier );
	send_to_char( buf, ch );
	return FALSE;
    }

    sprintf( buf, "{GClass Edit{w: ({y%s{w) {GTier changed from {g%d {Gto {g%d{G.{x\n\r",
	capitalize( class_table[class].name ),
	class_table[class].tier, value );
    send_to_char( buf, ch );

    class_table[class].tier = value;

    return TRUE;
}

OLC( class_edit_who_name )
{
    char buf[MAX_STRING_LENGTH];
    int class;

    EDIT_TABLE( ch, class );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "Who_name: %s\n\r", class_table[class].who_name );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( strlen( argument ) > 10 )
    {
	send_to_char( "Who name too long.\n\r", ch );
	return FALSE;
    }

    sprintf(buf,"{GClass Edit{w: ({y%s{w) {Gwho name changed from %s {Gto %s{G.{x\n\r",
	capitalize(class_table[class].name),
	class_table[class].who_name, argument );
    send_to_char( buf, ch );

    strcpy( class_table[class].who_name, argument );

    return TRUE;
}

OLC( race_edit_affects )
{
    int race, value;

    EDIT_TABLE( ch, race );

    if ( argument[0] == '\0'
    ||   ( value = flag_value( affect_flags, argument ) ) == NO_FLAG )
    {
	send_to_char( "Invalid flag.\n\n", ch );
	show_flag_cmds( ch, affect_flags );
	return FALSE;
    }

    race_table[race].aff ^= value;
    send_to_char( "Affect flags toggled.\n\r", ch );

    return TRUE;
}

OLC( race_edit_attack )
{
    char buf[MAX_STRING_LENGTH];
    int race, value;

    EDIT_TABLE( ch, race );

    if ( !race_table[race].pc_race )
    {
	send_to_char( "This setting as no affect on mobile races.\n\r", ch );
	return FALSE;
    }

    if ( argument[0] == '\0'
    ||   ( value = attack_lookup( argument ) ) == 0 )
    {
	send_to_char( "Invalid attack.\n\r", ch );
	show_damlist( ch, "all" );
	return FALSE;
    }

    sprintf( buf, "{GRace Edit{w: ({y%s{w) {GAttack changed from {g%s {Gto {g%s{G.{x\n\r",
	capitalize( race_table[race].name ),
	attack_table[race_table[race].attack].noun, attack_table[value].noun );
    send_to_char( buf, ch );

    race_table[race].attack = value;

    return TRUE;
}

OLC( race_edit_base_stats )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int race, stat, value[MAX_STATS];

    EDIT_TABLE( ch, race );

    if ( !race_table[race].pc_race )
    {
	send_to_char( "This setting as no affect on mobile races.\n\r", ch );
	return FALSE;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: base_stats <stat_name> <value>.\n\r"
		      "        base_stats <# # # # #>.\n\r", ch );
	return FALSE;
    }

    one_argument( argument, arg );

    if ( !is_number( arg ) )
    {
	argument = one_argument( argument, arg );

	if ( ( stat = stat_lookup( arg ) ) == -1 )
	{
	    send_to_char( "Base stat must be: str int wis dex con.\n\r", ch );
	    return FALSE;
	}

	if ( argument[0] == '\0' )
	{
	    race_edit_base_stats( ch, "" );
	    return FALSE;
	}

	if ( !is_number( argument )
	||   ( value[0] = atoi( argument ) ) < 0 || value[0] > 35 )
	{
	    send_to_char( "Values must be between 0 and 35.\n\r", ch );
	    return FALSE;
	}

	sprintf( buf,"{GRace Edit{w: ({y%s{w) {GBase stat {g%s {Gchanged from {g%d {Gto {g%d{G.{x\n\r",
	    capitalize( race_table[race].name ), stat_names[stat],
	    race_table[race].stats[stat], value[0] );
	send_to_char( buf, ch );

	race_table[race].stats[stat] = value[0];
	return TRUE;
    }

    for ( stat = 0; stat < MAX_STATS; stat++ )
    {
	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
	    send_to_char( "Not enough arguments specified to set all stats.\n\r", ch );
	    return FALSE;
	}

	if ( !is_number( arg )
	||   ( value[stat] = atoi( arg ) ) < 0 || value[stat] > 35 )
	{
	    send_to_char( "Invalid number.\n\r", ch );
	    return FALSE;
	}

	value[stat] = atoi( arg );
    }

    sprintf( buf,"{GRace Edit{w: ({y%s{w) {GBase stats changed.{x\n\r",
	capitalize( race_table[race].name ) );
    send_to_char( buf, ch );

    for ( stat = 0; stat < MAX_STATS; stat++ )
	race_table[race].stats[stat] = value[stat];

    return TRUE;
}

OLC( race_edit_class )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int class = 0, race, pos, setting = -1;
    bool classes[maxClass], fAll = FALSE;

    EDIT_TABLE( ch, race );

    if ( argument[0] == '\0' )
    {
	send_to_char(	"Syntax: class <class name> <yes>  [Allows classes]\n\r"
			"        class <class name> <no>   [Blocks classes]\n\r"
			"        class <class name>        [Toggles classes]\n\r\n\r"
			"Note: Class name can be single class, multiple class names, or 'all' for all classes.\n\r"
			"      Yes can be yes, on, true or 1.\n\r"
			"      No can be no, off, false or 0.\n\r", ch );
	return FALSE;
    }

    for ( pos = 0; pos < maxClass; pos++ )
	classes[pos] = FALSE;

    for ( ; ; )
    {
	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	    break;

	if ( !str_cmp( arg, "all" ) )
	{
	    fAll = TRUE;
	    continue;
	}

	if ( !str_cmp( arg, "yes" )
	||   !str_cmp( arg, "true" )
	||   !str_cmp( arg, "on" )
	||   !str_cmp( arg, "1" ) )
	{
	    setting = TRUE;
	    continue;
	}

	if ( !str_cmp( arg, "no" )
	||   !str_cmp( arg, "false" )
	||   !str_cmp( arg, "off" )
	||   !str_cmp( arg, "0" ) )
	{
	    setting = FALSE;
	    continue;
	}

	if ( ( class = class_lookup( arg ) ) != -1 )
	{
	    classes[class] = TRUE;
	    continue;
	}

	printf_to_char( ch, "Skipping invalid class %s.\n\r", arg );
    }

    for ( pos = 0; pos < maxClass; pos++ )
    {
	if ( fAll || classes[pos] )
	{
	    if ( setting == -1 )
		race_table[race].class_can_use[pos] = !race_table[race].class_can_use[pos];
	    else
		race_table[race].class_can_use[pos] = setting;
	}
    }

    send_to_char( "Race class restrictions applied.\n\r", ch );

    return TRUE;
}

OLC( race_edit_create )
{
    struct race_type *new_table;
    char buf[MAX_STRING_LENGTH];
    int lvl;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Which race do you wish to create?\n\r", ch );
	return FALSE;
    }

    if ( !check_argument( ch, argument, FALSE ) )
	return FALSE;

    if ( race_lookup( argument ) != -1 )
    {
	send_to_char( "That race already exists.\n\r", ch );
	return FALSE;
    }

    maxRace++;
    new_table = realloc( race_table, sizeof( struct race_type ) * ( maxRace + 1 ) );

    if ( !new_table ) /* realloc failed */
    {
	send_to_char( "Memory allocation failed. Brace for impact.\n\r", ch );
	return FALSE;
    }

    race_table = new_table;

    race_table[maxRace-1].name		= str_dup( argument );
    race_table[maxRace-1].attack	= attack_lookup( "punch" );
    race_table[maxRace-1].pc_race	= FALSE;
    race_table[maxRace-1].disabled	= TRUE;
    race_table[maxRace-1].size		= SIZE_MEDIUM;
    race_table[maxRace-1].aff		= 0;
    race_table[maxRace-1].shd		= 0;
    race_table[maxRace-1].parts		= 0;
    race_table[maxRace-1].points	= 0;
    race_table[maxRace-1].class_mult	= new_short( maxClass, 100 );
    race_table[maxRace-1].class_can_use	= new_bool( maxClass, TRUE );

    strcpy( race_table[maxRace-1].who_name, "      " );

    for ( lvl = 0; lvl < 5; lvl++ )
	race_table[maxRace-1].skills[lvl] = NULL;

    for ( lvl = 0; lvl < DAM_MAX; lvl++ )
	race_table[maxRace-1].damage_mod[lvl] = 100;

    for ( lvl = 0; lvl < MAX_STATS; lvl++ )
    {
	race_table[maxRace-1].stats[lvl]	= 0;
	race_table[maxRace-1].max_stats[lvl]	= 0;
    }

    race_table[maxRace].name = str_dup( "" );;

    sprintf( buf, "{GRace Edit{w: {GNew race created{w, {y%s{G.{x\n\r", argument );
    send_to_char( buf, ch );

    ch->desc->editor = ED_RACE;
    ch->desc->pEdit = (void *)maxRace-1;

    return TRUE;
}

OLC( race_edit_dam_mod )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int lvl, race, value;

    EDIT_TABLE( ch, race );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == 0 )
    {
	send_to_char( show_dam_mods( race_table[race].damage_mod ), ch );
	return FALSE;
    }

    value = atoi( argument );

    if ( !is_number( argument ) || value < 0 || value > 200 )
    {
	send_to_char( "Proper damage modifiers are 0 to 200%.\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( lvl = 0; lvl < DAM_MAX; lvl++ )
	    race_table[race].damage_mod[lvl] = value;
	sprintf( buf, "{GAll damage modifiers for {y({g%s{y) {Gchanged to %d.{x\n\r",
	    race_table[race].name, value );
    }

    else if ( ( lvl = dam_type_lookup( arg ) ) != -1 )
    {
	sprintf( buf, "{GDamage modifier for {y({g%s{y) {Gchanged: %s from %d to %d.{x\n\r",
	    race_table[race].name, damage_mod_table[lvl].name,
	    race_table[race].damage_mod[lvl], value );
	race_table[race].damage_mod[lvl] = value;
    }

    else
    {
	send_to_char( "Invalid damage modifier.\n\r", ch );
	return FALSE;
    }

    send_to_char( buf, ch );

    return TRUE;
}

OLC( race_edit_delete )
{
    CHAR_DATA *wch;
    DESCRIPTOR_DATA *d;
    MOB_INDEX_DATA *pMob;
    struct race_type *new_table;
    char buf[MAX_STRING_LENGTH];
    int i, lvl, race, vnum;

    EDIT_TABLE( ch, race );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: race_edit_delete (race_name).\n\r", ch );
	return FALSE;
    }

    if ( str_cmp( race_table[race].name, argument ) )
    {
	send_to_char( "You must be editing the race you wish to delete, name does not match argument.\n\r", ch );
	return FALSE;
    }

    if ( ( lvl = race_lookup( "human" ) ) == -1 )
    {
	send_to_char( "Human race not found to default to, delete aborted.\n\r", ch );
	return FALSE;
    }

    new_table = malloc( sizeof( struct race_type ) * maxRace );

    if ( !new_table )
    {
	send_to_char( "Memory allocation failed. Brace for impact...\n\r", ch );
	return FALSE;
    }

    check_olc_delete( ch );

    for ( i = 0, vnum = 0; i < top_mob_index; vnum++ )
    {
	if ( ( pMob = get_mob_index( vnum ) ) != NULL )
	{
	    i++;
	    if ( pMob->race == race )
		pMob->race = lvl;

	    else if ( pMob->race > race )
		pMob->race--;
	}
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING || d->character == NULL )
	    continue;

	else if ( d->character->race == race )
	    d->character->race = lvl;

	else if ( d->character->race > race )
	    d->character->race--;
    }

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch->race == race )
	{
	    send_to_char( "Your race was just deleted.\n\r", wch );
	    wch->race = lvl;
	}

	else if ( wch->race > race )
	    wch->race--;
    }

    for ( i = 0, lvl = 0; i < maxRace+1; i++ )
    {
	if ( i != race )
	{
	    new_table[lvl] = race_table[i];
	    lvl++;
	}
    }

    free( race_table );
    race_table = new_table;
		
    maxRace--; /* Important :() */

    sprintf( buf, "{GRace Edit{w: {GRace deleted{w, {y%s{G.{x\n\r", argument );
    send_to_char( buf, ch );

    return TRUE;
}

OLC( race_edit_disabled )
{
    char buf[MAX_STRING_LENGTH];
    int race;

    EDIT_TABLE( ch, race );

    sprintf( buf, "{GRace Edit{w: ({y%s{w) {GDisabled race changed from {g%s {Gto {g%s{G.{x\n\r",
	capitalize( race_table[race].name ),
	show_true_false( race_table[race].disabled ),
	show_true_false( !race_table[race].disabled ) );
    send_to_char( buf, ch );

    race_table[race].disabled = !race_table[race].disabled;

    return TRUE;
}

OLC( race_edit_max_stats )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int race, stat, value[MAX_STATS];

    EDIT_TABLE( ch, race );

    if ( !race_table[race].pc_race )
    {
	send_to_char( "This setting has no affect on mobile races.\n\r", ch );
	return FALSE;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: max_stats <stat_name> <value>.\n\r"
		      "        max_stats <# # # # #>.\n\r", ch );
	return FALSE;
    }

    one_argument( argument, arg );

    if ( !is_number( arg ) )
    {
	argument = one_argument( argument, arg );

	if ( ( stat = stat_lookup( arg ) ) == -1 )
	{
	    send_to_char( "Max stat must be: str int wis dex con.\n\r", ch );
	    return FALSE;
	}

	if ( argument[0] == '\0' )
	{
	    race_edit_max_stats( ch, "" );
	    return FALSE;
	}

	if ( !is_number( argument )
	||   ( value[0] = atoi( argument ) ) < 0 || value[0] > 35 )
	{
	    send_to_char( "Values must be between 0 and 35.\n\r", ch );
	    return FALSE;
	}

	sprintf( buf,"{GRace Edit{w: ({y%s{w) {GMax stat {g%s {Gchanged from {g%d {Gto {g%d{G.{x\n\r",
	    capitalize( race_table[race].name ), stat_names[stat],
	    race_table[race].max_stats[stat], value[0] );
	send_to_char( buf, ch );

	race_table[race].max_stats[stat] = value[0];
	return TRUE;
    }

    for ( stat = 0; stat < MAX_STATS; stat++ )
    {
	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
	    send_to_char( "Not enough arguments specified to set all stats.\n\r", ch );
	    return FALSE;
	}

	if ( !is_number( arg )
	||   ( value[stat] = atoi( arg ) ) < 0 || value[stat] > 35 )
	{
	    send_to_char( "Invalid number.\n\r", ch );
	    return FALSE;
	}

	value[stat] = atoi( arg );
    }

    sprintf( buf,"{GRace Edit{w: ({y%s{w) {GMax stats changed.{x\n\r",
	capitalize( race_table[race].name ) );
    send_to_char( buf, ch );

    for ( stat = 0; stat < MAX_STATS; stat++ )
	race_table[race].max_stats[stat] = value[stat];

    return TRUE;
}

OLC( race_edit_multiplier )
{
    CHAR_DATA *wch;
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int lvl, race, value;

    EDIT_TABLE( ch, race );

    if ( !race_table[race].pc_race )
    {
	send_to_char( "This setting as no affect on mobile races.\n\r", ch );
	return FALSE;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Multiplier <class> <value>.\n\r", ch );
	return FALSE;
    }

    if ( ( lvl = class_lookup( arg ) ) == -1 )
    {
	send_to_char( "Invalid class.\n\r", ch );
	return FALSE;
    }

    if ( !is_number( argument ) || ( value = atoi( argument ) ) < 0 )
    {
	send_to_char( "Invalid number.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "{GRace Edit{w: ({y%s{w) {GClass multiplier for {g%s {Gchanged from {g%d {Gto {g%d{G.{x\n\r",
	capitalize(race_table[race].name), class_table[lvl].name,
	race_table[race].class_mult[lvl], value );
    send_to_char( buf, ch );

    race_table[race].class_mult[lvl] = value;

    for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
    {
	long tnl;

	if ( wch->race != race || wch->class != lvl )
	    continue;

	tnl = ( ( wch->level + 1 ) * exp_per_level( wch, wch->pcdata->points ) - wch->exp );

	if ( tnl < 0 || tnl > exp_per_level( wch, wch->pcdata->points ) )
	{
	    send_to_char( "\n\r{REXP PROBLEM NOTED, YOUR EXP HAS BEEN RESET.{x\n\r", wch );
	    sprintf( log_buf, "EXP problem noted on %s, EXP reset.", wch->name );
	    bug( log_buf, 0 );
	    wch->exp = exp_per_level( wch, wch->pcdata->points ) * wch->level;
	}
    }

    return TRUE;
}

OLC( race_edit_name )
{
    MOB_INDEX_DATA *pMob;
    char buf[MAX_STRING_LENGTH];
    int i, race, vnum;

    EDIT_TABLE( ch, race );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GName{w: {g%s{x\n\r", race_table[race].name );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( !check_argument( ch, argument, FALSE ) )
	return FALSE;

    if ( race_lookup( argument ) != -1 )
    {
	send_to_char( "That name appears to already exist.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "{GRace Edit{w: ({y%s{w) {Grenamed to {g%s{G.{x\n\r",
	capitalize(race_table[race].name), argument );
    send_to_char( buf, ch );

    sprintf( buf, "grep -rl 'Race %s~' ../player/ | xargs perl -pi -e 's/Race %s~/Race %s~/'",
	race_table[race].name, race_table[race].name, argument );
    system( buf );

    free_string( race_table[race].name );
    race_table[race].name = str_dup(argument);

    for ( i = 0, vnum = 0; i < top_mob_index; vnum++ )
    {
	if ( ( pMob = get_mob_index( vnum ) ) != NULL )
	{
	    i++;
	    if ( pMob->race == race )
		SET_BIT( pMob->area->area_flags, AREA_CHANGED );
	}
    }

    return TRUE;
}

OLC( race_edit_parts )
{
    int race, value;

    EDIT_TABLE( ch, race );

    if ( argument[0] == '\0'
    ||   ( value = flag_value( part_flags, argument ) ) == NO_FLAG )
    {
	send_to_char( "Invalid flag.\n\n", ch );
	show_flag_cmds( ch, part_flags );
	return FALSE;
    }

    race_table[race].parts ^= value;
    send_to_char( "Parts flags toggled.\n\r", ch );

    return TRUE;
}

OLC( race_edit_pc_race )
{
    char buf[MAX_STRING_LENGTH];
    int race;

    EDIT_TABLE( ch, race );

    sprintf( buf, "{GRace Edit{w: ({y%s{w) {Gpc_race changed from {g%s {Gto {g%s{G.{x\n\r",
	capitalize( race_table[race].name ),
	show_true_false( race_table[race].pc_race ),
	show_true_false( !race_table[race].pc_race ) );
    send_to_char( buf, ch );

    race_table[race].pc_race = !race_table[race].pc_race;

    return TRUE;
}

OLC( race_edit_points )
{
    char buf[MAX_STRING_LENGTH];
    int race, value;

    EDIT_TABLE( ch, race );

    if ( !race_table[race].pc_race )
    {
	send_to_char( "This setting as no affect on mobile races.\n\r", ch );
	return FALSE;
    }

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GCreation Points{w: {g%d{x\n\r", race_table[race].points );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( !is_number( argument ) || ( value = atoi( argument ) ) < 0 )
    {
	send_to_char( "Invalid number.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "{GRace Edit{w: ({y%s{w) {Gpoints changed from {g%d {Gto {g%d{G.{x\n\r",
	capitalize(race_table[race].name), race_table[race].points, value );
    send_to_char( buf, ch );

    race_table[race].points = value;

    return TRUE;
}

OLC( race_edit_shields )
{
    int race, value;

    EDIT_TABLE( ch, race );

    if ( argument[0] == '\0'
    ||   ( value = flag_value( shield_flags, argument ) ) == NO_FLAG )
    {
	send_to_char( "Invalid flag.\n\n", ch );
	show_flag_cmds( ch, shield_flags );
	return FALSE;
    }

    race_table[race].shd ^= value;

    send_to_char( "Shield flags toggled.\n\r", ch );

    return TRUE;
}

OLC( race_edit_show )
{
    BUFFER *final = new_buf();
    char buf[MAX_STRING_LENGTH], temp[MAX_INPUT_LENGTH];
    char *flags;
    int race;

    EDIT_TABLE( ch, race );

    sprintf( buf, "{qName:            {s[{t%s{s]\n\r", race_table[race].name );
    add_buf( final, buf );

    sprintf( buf, "{qPc_race:         {s[{t%s{s]\n\r",
	show_true_false( race_table[race].pc_race ) );
    add_buf( final, buf );

    sprintf( buf, "{qDisabled:        {s[{t%s{s]\n\r",
	show_true_false( race_table[race].disabled ) );
    add_buf( final, buf );

    flags = flag_string( shield_flags, race_table[race].shd );
    flags = length_argument( flags, temp, 60 );

    sprintf( buf, "{qShielded by:     {s[{t%s{s]\n\r", temp );
    add_buf( final, buf );

    while ( *flags != '\0' )
    {
	flags = length_argument( flags, temp, 60 );
	sprintf( buf, "                 {s[{t%s{s]\n\r", temp );
	add_buf( final, buf );
    }

    flags = flag_string( affect_flags, race_table[race].aff );
    flags = length_argument( flags, temp, 60 );

    sprintf( buf, "{qAffected by:     {s[{t%s{s]\n\r", temp );
    add_buf( final, buf );

    while ( *flags != '\0' )
    {
	flags = length_argument( flags, temp, 60 );
	sprintf( buf, "                 {s[{t%s{s]\n\r", temp );
	add_buf( final, buf );
    }

    flags = flag_string( part_flags, race_table[race].parts );
    flags = length_argument( flags, temp, 60 );

    sprintf( buf, "{qParts:           {s[{t%s{s]\n\r", temp );
    add_buf( final, buf );

    while ( *flags != '\0' )
    {
	flags = length_argument( flags, temp, 60 );
	sprintf( buf, "                 {s[{t%s{s]\n\r", temp );
	add_buf( final, buf );
    }

    sprintf( buf, "{qSize:            {s[{t%s{s]\n\r",
	flag_string( size_flags, race_table[race].size ) );
    add_buf( final, buf );

    if ( !race_table[race].pc_race )
    {
	add_buf( final, "\n\r" );
	add_buf( final, show_dam_mods( race_table[race].damage_mod ) );
    } else {
	char class_list[maxClass][MAX_STRING_LENGTH];
	sh_int i, lvl;

	sprintf( buf, "{qWho_name:        {s[{t%s{s]\n\r",
	    race_table[race].who_name );
	add_buf( final, buf );

	flags = return_classes( race_table[race].class_can_use );
	flags = length_argument( flags, temp, 60 );

	sprintf( buf, "{q!Classes:        {s[{t%s{s]\n\r", temp );
	add_buf( final, buf );

	while( *flags != '\0' )
	{
	    flags = length_argument( flags, temp, 60 );
	    sprintf( buf, "                 {s[{t%s{s]\n\r", temp );
	    add_buf( final, buf );
	}

	sprintf( buf, "{qAttack:          {s[{t%s{s]\n\r",
	    attack_table[race_table[race].attack].noun );
	add_buf( final, buf );

	sprintf( buf, "{qPoints:          {s[{t%d{s]\n\r",
	    race_table[race].points );
	add_buf( final, buf );

	add_buf( final, "{qBase Stats:     " );
	for ( i = 0; i < MAX_STATS; i++ )
	{
	    sprintf( buf, " {s[{q%c%c%c: {t%2d{s]", stat_names[i][0],
		stat_names[i][1], stat_names[i][2], race_table[race].stats[i] );
	    add_buf( final, buf );
	}

	add_buf( final, "\n\r{qMax Stats:      " );
	for ( i = 0; i < MAX_STATS; i++ )
	{
	    sprintf( buf, " {s[{q%c%c%c: {t%2d{s]", stat_names[i][0],
		stat_names[i][1], stat_names[i][2], race_table[race].max_stats[i] );
	    add_buf( final, buf );
	}

	if ( race_table[race].skills[0] != NULL )
	{
	    add_buf( final, "\n\r\n\r{qRacial Skills:\n\r{t-------------------" );
	    for ( lvl = 0; lvl < 5; lvl++ )
	    {
		if ( race_table[race].skills[lvl] == NULL )
		    break;

		sprintf( buf, "\n\r {s[{t%s{s]", race_table[race].skills[lvl] );
		add_buf( final, buf );
	    }
	}

	add_buf( final, "\n\r\n\r" );

	add_buf( final, show_dam_mods( race_table[race].damage_mod ) );

	add_buf( final, "\n\r{qClass Multipliers:\n\r" );

	for ( i = 0; i < maxClass; i++ )
	    class_list[i][0] = '\0';

	for ( lvl = 0; class_table[lvl].name[0] != '\0'; lvl++ )
	{
	    sprintf( buf, "  {s[%s{%c%c{%c%-7.7s{s: {t%3d%%{s]",
		race_table[race].class_can_use[lvl] ? "{Y+" : "{R-",
		class_table[lvl].who_name[1], UPPER( *class_table[lvl].name ),
		class_table[lvl].who_name[4], class_table[lvl].name+1,
		race_table[race].class_mult[lvl] );
	    strcat( class_list[class_table[lvl].sub_class], buf );
	}

	for ( i = 0; i < maxClass; i++ )
	{
	    if ( class_list[i][0] != '\0' )
	    {
		add_buf( final, class_list[i] );
		add_buf( final, "\n\r" );
	    }
	}
    }

    add_buf( final, "{x" );
    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

OLC( race_edit_size )
{
    char buf[MAX_STRING_LENGTH];
    int race, value;

    EDIT_TABLE( ch, race );

    if ( argument[0] == '\0'
    ||   ( value = flag_value( size_flags, argument ) ) == NO_FLAG )
    {
	send_to_char( "Invalid size.\n\r", ch );
	show_help( ch, "size" );
	return FALSE;
    }

    sprintf( buf, "{GRace Edit{w: ({y%s{w) {Gsize changed from {g%s {Gto {g%s{G.{x\n\r",
	capitalize(race_table[race].name),
	size_flags[race_table[race].size].name, size_flags[value].name );
    send_to_char( buf, ch );

    race_table[race].size = value;

    return TRUE;
}

OLC( race_edit_skills )
{
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int lvl, race, value;

    EDIT_TABLE( ch, race );

    if ( !race_table[race].pc_race )
    {
	send_to_char( "This setting as no affect on mobile races.\n\r", ch );
	return FALSE;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "{GRacial Skills{w:\n\r{y-------------------\n\r", ch );
	for ( lvl = 0; lvl < 5; lvl++ )
	{
	    if ( race_table[race].skills[lvl] == NULL )
		break;

	    sprintf( buf, "{g %s{x\n\r", race_table[race].skills[lvl] );
	    send_to_char( buf, ch );
	}

	return FALSE;
    }

    if ( ( lvl = skill_lookup( argument ) ) == -1 )
    {
	send_to_char( "Invalid_skill.\n\r", ch );
	return FALSE;
    }

    for ( value = 0; value < 5; value++ )
    {
	if ( race_table[race].skills[value] == NULL )
	    break;

	if ( found )
	{
	    race_table[race].skills[value-1] = race_table[race].skills[value];
	    race_table[race].skills[value] = NULL;
	    continue;
	}

	if ( value < 5
	&&   !str_cmp(race_table[race].skills[value], skill_table[lvl].name) )
	{
	    sprintf( buf, "{GRace Edit{w: ({y%s{w) {GSkill {g%s {Gremoved.{x\n\r",
		capitalize(race_table[race].name), skill_table[lvl].name );

	    free_string( race_table[race].skills[value] );
	    race_table[race].skills[value] = NULL;
	    found = TRUE;
	}
    }

    if ( !found )
    {
	if ( value >= 5 )
	{
	    send_to_char("Maximum racial skills: 5.\n\r",ch);
	    return FALSE;
	}

	sprintf( buf, "{GRace Edit{w: ({y%s{w) {GSkill {g%s {Gadded.{x\n\r",
		capitalize(race_table[race].name), skill_table[lvl].name );

	race_table[race].skills[value] = str_dup( skill_table[lvl].name );
    }

    send_to_char( buf, ch );

    return TRUE;
}

OLC( race_edit_who_name )
{
    char buf[MAX_STRING_LENGTH];
    int race;

    EDIT_TABLE( ch, race );

    if ( !race_table[race].pc_race )
    {
	send_to_char( "This setting as no affect on mobile races.\n\r", ch );
	return FALSE;
    }

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GWho Name{w: {g%s{x\n\r", race_table[race].who_name );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( strlen_color( argument ) != 6 )
    {
	send_to_char( "Who name must be 6 characters long.\n\r", ch );
	return FALSE;
    }

    if ( strlen( argument ) > 10 )
    {
	send_to_char( "Who names must not exceed 10 characters.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "{GRace Edit{w: ({y%s{w) {Gwho name changed from {g%s {Gto {g%s{G.{x\n\r",
	capitalize(race_table[race].name), race_table[race].who_name, argument );
    send_to_char( buf, ch );

    strcpy( race_table[race].who_name, argument );

    return TRUE;
}

OLC( channel_edit_arena )
{
    int channel;

    EDIT_TABLE( ch, channel );

    channel_table[channel].arena = !channel_table[channel].arena;

    send_to_char( "Arena permissions toggled.\n\r", ch );

    return TRUE;
}

OLC( channel_edit_censor )
{
    int channel;

    EDIT_TABLE( ch, channel );

    channel_table[channel].censor = !channel_table[channel].censor;

    send_to_char( "Censor status toggled.\n\r", ch );

    return TRUE;
}

OLC( channel_edit_char )
{
    char text[4];
    int channel;

    EDIT_TABLE( ch, channel );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: char_string [string character sees when using channel]\n\r", ch );
	return FALSE;
    }

    sprintf( text, "{%c$t", channel_table[channel].color_variable );

    if ( str_infix_c( text, argument ) )
    {
	char buf[100];
	sprintf( buf, "Char_string must include '%s' variable for color and text spoken on channel.\n\r", double_color( text ) );
	send_to_char( buf, ch );
	return FALSE;
    }

    free_string( channel_table[channel].ch_string );
    channel_table[channel].ch_string = str_dup( argument );

    send_to_char( "Char string set.\n\r", ch );

    return TRUE;
}

OLC( channel_edit_color )
{
    int channel, code;

    EDIT_TABLE( ch, channel );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: color [code name]\n\r", ch );
	return FALSE;
    }

    if ( ( code = find_color( argument ) ) == -1 )
    {
	char buf[100];

	send_to_char( "Invalid color code string.\n\r", ch );

	for ( code = 0; color_table[code].name != NULL; code++ )
	{
	    sprintf( buf, "    %s\n\r", color_table[code].name );
	    send_to_char( buf, ch );
	}
	return FALSE;
    }

    channel_table[channel].color_default = color_table[code].color_code;

    if ( channel == channel_lookup( "ask" ) )
	channel_table[channel_lookup( "answer" )].color_default = color_table[code].color_code;

    else if ( channel == channel_lookup( "answer" ) )
	channel_table[channel_lookup( "ask" )].color_default = color_table[code].color_code;

    send_to_char( "Default color set.\n\r", ch );

    return TRUE;
}

OLC( channel_edit_drunk )
{
    int channel;

    EDIT_TABLE( ch, channel );

    channel_table[channel].drunk = !channel_table[channel].drunk;

    send_to_char( "Drunk text toggled.\n\r", ch );

    return TRUE;
}

OLC( channel_edit_level )
{
    int channel, level;

    EDIT_TABLE( ch, channel );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: level [min level to use channel]\n\r", ch );
	return FALSE;
    }

    level = atoi( argument );

    if ( level < 0 || level > MAX_LEVEL )
    {
	send_to_char( "Invalid level setting.\n\r", ch );
	return FALSE;
    }

    channel_table[channel].level = level;

    send_to_char( "Minimum level set.\n\r", ch );

    return TRUE;
}

OLC( channel_edit_others )
{
    char text[4];
    int channel;

    EDIT_TABLE( ch, channel );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: other_string [string others see when using channel]\n\r", ch );
	return FALSE;
    }

    sprintf( text, "{%c$t", channel_table[channel].color_variable );

    if ( str_infix_c( text, argument ) )
    {
	char buf[100];
	sprintf( buf, "Other_string must include '%s' variable for color and text spoken on channel.\n\r", double_color( text ) );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( str_infix_c( "$n", argument ) )
    {
	send_to_char( "Others string must include '$n' variable so others know who is speaking.\n\r", ch );
	return FALSE;
    }

    free_string( channel_table[channel].other_string );
    channel_table[channel].other_string = str_dup( argument );

    send_to_char( "Others string set.\n\r", ch );

    return TRUE;
}

OLC( channel_edit_pretitle )
{
    int channel;

    EDIT_TABLE( ch, channel );

    channel_table[channel].pretitle = !channel_table[channel].pretitle;

    send_to_char( "Pretitle display toggled.\n\r", ch );

    return TRUE;
}

OLC( channel_edit_quiet )
{
    int channel;

    EDIT_TABLE( ch, channel );

    channel_table[channel].quiet = !channel_table[channel].quiet;

    send_to_char( "Quiet availability toggled.\n\r", ch );

    return TRUE;
}

OLC( channel_edit_show )
{
    BUFFER *final = new_buf( );
    char buf[MAX_STRING_LENGTH];
    int channel;

    EDIT_TABLE( ch, channel );

    sprintf( buf, "{qChannel:        {s[{t%s{s]\n\r",
	channel_table[channel].name );
    add_buf( final, buf );

    sprintf( buf, "{qChar String:    {s[{x%s{s]\n\r"
		  "                [{t%s{s]\n\r",
	channel_table[channel].ch_string,
	double_color( channel_table[channel].ch_string ) );
    add_buf( final, buf );

    sprintf( buf, "{qOther String:   {s[{x%s{s]\n\r"
		  "                [{t%s{s]\n\r",
	channel_table[channel].other_string,
	double_color( channel_table[channel].other_string ) );
    add_buf( final, buf );

    sprintf( buf, "{qColor Variable: {s[{t{{%c{s]\n\r",
	channel_table[channel].color_variable );
    add_buf( final, buf );

    sprintf( buf, "{qColor Default:  {s[{t%s{s]\n\r",
	color_string( channel_table[channel].color_default ) );
    add_buf( final, buf );

    sprintf( buf, "{qArena:          {s[{t%s{s]\n\r",
	show_true_false( channel_table[channel].arena ) );
    add_buf( final, buf );

    sprintf( buf, "{qCensor:         {s[{t%s{s]\n\r",
	show_true_false( channel_table[channel].censor ) );
    add_buf( final, buf );

    sprintf( buf, "{qDrunk:          {s[{t%s{s]\n\r",
	show_true_false( channel_table[channel].drunk ) );
    add_buf( final, buf );

    sprintf( buf, "{qPretitle:       {s[{t%s{s]\n\r",
	show_true_false( channel_table[channel].pretitle ) );
    add_buf( final, buf );

    sprintf( buf, "{qQuiet:          {s[{t%s{s]\n\r",
	show_true_false( channel_table[channel].quiet ) );
    add_buf( final, buf );

    sprintf( buf, "{qLevel:          {s[{t%d{s]\n\r",
	channel_table[channel].level );
    add_buf( final, buf );

    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

OLC( clan_edit_color_name )
{
    char buf[MAX_STRING_LENGTH];
    int clan;

    EDIT_TABLE( ch, clan );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GColor Name{w:   {g%s\n\r", clan_table[clan].color );
	send_to_char( buf, ch );
	return FALSE;
    }

    sprintf( buf, "%s: Old color name: %s, New color name: %s.\n\r",
	clan_table[clan].name, clan_table[clan].color, argument );
    send_to_char( buf, ch );

    free_string( clan_table[clan].color );
    clan_table[clan].color = str_dup( argument );

    return TRUE;
}

OLC( clan_edit_create )
{
    struct clan_type *new_table;
    char buf[MAX_STRING_LENGTH];
    int rank;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: clan_edit create <clan_name>.\n\r", ch );
	return FALSE;
    }

    if ( !check_argument( ch, argument, FALSE ) )
	return FALSE;

    if ( clan_lookup( argument ) != 0 || !str_cmp( argument, "none" ) )
    {
	send_to_char( "That clan already exists.\n\r", ch );
	return FALSE;
    }

    maxClan++;
    new_table = realloc( clan_table, sizeof( struct clan_type ) * ( maxClan + 1 ) );

    if ( !new_table ) /* realloc failed */
    {
	send_to_char( "Memory allocation failed. Brace for impact.\n\r", ch );
	return FALSE;
    }

    clan_table = new_table;

    clan_table[maxClan-1].roster	= NULL;
    clan_table[maxClan-1].kills_list	= NULL;
    clan_table[maxClan-1].death_list	= NULL;
    clan_table[maxClan-1].name		= str_dup( argument );
    clan_table[maxClan-1].who_name	= str_dup( "-----------" );
    clan_table[maxClan-1].color		= str_dup( argument );
    clan_table[maxClan-1].hall		= ROOM_VNUM_ALTAR;
    clan_table[maxClan-1].pit		= OBJ_VNUM_PIT;
    clan_table[maxClan-1].independent	= FALSE;
    clan_table[maxClan-1].pkill		= TRUE;
    clan_table[maxClan-1].exname	= str_dup( "none" );
    clan_table[maxClan-1].max_mem	= 5;
    clan_table[maxClan-1].cubics	= 0;
    clan_table[maxClan-1].kills		= 0;
    clan_table[maxClan-1].deaths	= 0;
    clan_table[maxClan-1].aquest	= 0;
    clan_table[maxClan-1].iquest	= 0;
    clan_table[maxClan-1].crnk[0]	= str_dup( "Unranked" );

    for ( rank = 1; rank < MAX_CRNK; rank++ )
	clan_table[maxClan-1].crnk[rank] = str_dup( "none" );

    clan_table[maxClan].name = str_dup( "" );

    sprintf( buf, "New clan added: %s.\n\r", argument );
    send_to_char( buf, ch );

    ch->desc->editor = ED_CLAN;
    ch->desc->pEdit = (void *)maxClan-1;

    mud_stat.clans_changed = TRUE;

    return TRUE;
}

OLC( clan_edit_delete )
{
    AREA_DATA *pArea;
    CHAR_DATA *wch;
    ROSTER_DATA *list, *list_next;
    struct clan_type *new_table;
    char buf[MAX_STRING_LENGTH];
    int clan, i, j;

    EDIT_TABLE( ch, clan );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: delete <clan_name>\n\r", ch );
	send_to_char( "{RWARNING: This command is irreversible!{x\n\r", ch );
	return FALSE;
    }

    if ( str_cmp( argument, clan_table[clan].name ) )
    {
	send_to_char( "Syntax: delete <clan_name>.\n\r"
		      "You must be editing the clan also.\n\r", ch );
	return FALSE;
    }

    if ( clan <= 0
    ||   clan >= maxClan
    ||   !str_cmp( clan_table[clan].name, "condemned" )
    ||   !str_cmp( clan_table[clan].name, "none" ) )
    {
	send_to_char( "That clan may not be deleted.\n\r", ch );
	return FALSE;
    }

    check_olc_delete( ch );

    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {
	if ( pArea->clan == clan )
	    pArea->clan = 0;

	else if ( pArea->clan > clan )
	    pArea->clan--;
    }

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch->clan == clan )
	{
	    send_to_char( "{GYour clan was just disbanded!{x\n\r", wch );

	    if ( wch->pcdata )
	    {
		wch->pcdata->clan_rank = 0;

		if ( clan_table[clan].pkill )
		{
		    wch->clan = clan_lookup( "condemned" );
		    check_roster( wch, FALSE );
		    update_clanlist( wch, TRUE );
		} else {
		    wch->clan = 0;
		    check_roster( wch, TRUE );
		}
	    }

	    else
		wch->clan = 0;
	}

	else if ( wch->clan > clan )
	    wch->clan--;
    }

    sprintf( buf, "../data/clans/%s", clan_table[clan].name );
    unlink( buf );

    free_string( clan_table[clan].name		);
    free_string( clan_table[clan].who_name	);
    free_string( clan_table[clan].color		);
    free_string( clan_table[clan].exname	);

    clan_table[clan].name	= NULL;
    clan_table[clan].who_name	= NULL;
    clan_table[clan].color	= NULL;
    clan_table[clan].exname	= NULL;

    for ( list = clan_table[clan].roster; list != NULL; list = list_next )
    {
	list_next = list->next;
	free_roster( list );
    }
    clan_table[clan].roster = NULL;

    for ( i = 0; i < MAX_CRNK; i++ )
    {
	free_string( clan_table[clan].crnk[i] );
	clan_table[clan].crnk[i] = NULL;
    }

    new_table = malloc( sizeof( struct clan_type ) * maxClan );

    if ( !new_table )
    {
	send_to_char( "Memory allocation failed!\n\r", ch );
	return FALSE;
    }

    for ( i = 0, j = 0; i < maxClan+1; i++ )
    {
	if ( i != clan )
	{
	    new_table[j] = clan_table[i];
	    j++;
	}
    }

    free( clan_table );
    clan_table = new_table;
	
    maxClan--;	

    sprintf( buf, "Clan deleted: %s.\n\r", argument );
    send_to_char( buf, ch );

    return TRUE;
}

OLC( clan_edit_description )
{
    char buf[MAX_STRING_LENGTH];
    int clan;

    EDIT_TABLE( ch, clan );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GDescription{w:  {c%s\n\r", clan_table[clan].exname );
	send_to_char( buf, ch );
	return FALSE;
    }

    sprintf( buf, "%s: Description changed:\n\r Old: %s\n\r New: %s\n\r",
	clan_table[clan].name, clan_table[clan].exname, argument );
    send_to_char( buf, ch );

    free_string( clan_table[clan].exname );
    clan_table[clan].exname = str_dup( argument );

    return TRUE;
}

OLC( clan_edit_donation )
{
    char buf[MAX_STRING_LENGTH];
    int clan, vnum;

    EDIT_TABLE( ch, clan );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GDonation Pit{w: {g%d\n\r", clan_table[clan].pit );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( !is_number( argument )
    ||   ( vnum = atoi( argument ) ) < 0 )
    {
	send_to_char( "Donation pit vnum must be a valid number\n\r", ch );
	return FALSE;
    }

    if ( get_obj_index( vnum ) == NULL )
    {
	send_to_char( "That object does not exist.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "%s: Old donation pit: %d, New donation pit: %d.\n\r",
	clan_table[clan].name, clan_table[clan].pit, vnum );
    send_to_char( buf, ch );

    clan_table[clan].pit = vnum;

    return TRUE;
}

OLC( clan_edit_independent )
{
    char buf[MAX_STRING_LENGTH];
    int clan;

    EDIT_TABLE( ch, clan );

    sprintf( buf, "%s: Independent rating changed from %s to %s.\n\r",
	clan_table[clan].name, show_true_false( clan_table[clan].independent ),
	show_true_false( !clan_table[clan].independent ) );
    send_to_char( buf, ch );

    clan_table[clan].independent = !clan_table[clan].independent;

    return TRUE;
}

OLC( clan_edit_max_member )
{
    char buf[MAX_STRING_LENGTH];
    int clan, num;

    EDIT_TABLE( ch, clan );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GMax Members{w:  {g%d\n\r", clan_table[clan].max_mem );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( !is_number( argument )
    ||   ( num = atoi( argument ) ) <= 0 || num > 25 )
    {
	send_to_char( "Max_members value must be a valid number.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "%s: Old max_members: %d, New max_members: %d.\n\r",
	clan_table[clan].name, clan_table[clan].max_mem, num );
    send_to_char( buf, ch );

    clan_table[clan].max_mem = num;

    return TRUE;
}

OLC( clan_edit_name )
{
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];
    int clan;

    EDIT_TABLE( ch, clan );

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: name <new name>.\n\r", ch );
	return FALSE;
    }

    if ( !check_argument( ch, argument, FALSE ) )
	return FALSE;

    if ( clan_lookup( argument ) != 0 )
    {
	send_to_char( "That clan already exists.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "Old name: %s, New name: %s.\n\r",
	clan_table[clan].name, argument );
    send_to_char( buf, ch );

    sprintf( buf, "grep -rl 'Clan %s~' ../player/ | xargs perl -pi -e 's/Clan %s~/Clan %s~/'",
	clan_table[clan].name, clan_table[clan].name, argument );
    system( buf );

    free_string( clan_table[clan].name );
    clan_table[clan].name = str_dup( argument );

    if ( ( pArea = get_clan_area( clan ) ) != NULL )
    {
	FILE *fp;
	char name[50];

	sprintf( buf, "%s.cln", clan_table[clan].name );

	if ( ( fp = fopen( buf, "r" ) ) != NULL )
	{
	    sh_int count;

	    fclose( fp );
	    for ( count = 0; ; count++ )
	    {
		sprintf( buf, "%s%d.cln", clan_table[ch->clan].name, count );
		if ( ( fp = fopen( buf, "r" ) ) == NULL )
		    break;
		fclose( fp );
	    }
	}

	strcpy( name, pArea->file_name );
	free_string( pArea->file_name );
	pArea->file_name = str_dup( buf );
	save_area( pArea, FALSE );
	unlink( name );
    }

    return TRUE;
}

OLC( clan_edit_pkill )
{
    char buf[MAX_STRING_LENGTH];
    int clan;

    EDIT_TABLE( ch, clan );

    sprintf( buf, "%s: Pkill rating changed from %s to %s.\n\r",
	clan_table[clan].name, show_true_false( clan_table[clan].pkill ),
	show_true_false( !clan_table[clan].pkill ) );
    send_to_char( buf, ch );

    clan_table[clan].pkill = !clan_table[clan].pkill;

    return TRUE;
}

OLC( clan_edit_rank )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int clan, rank;

    EDIT_TABLE( ch, clan );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( rank = MAX_CRNK-1; rank > 0; rank-- )
	{
	    sprintf( buf, "{GRank {w[{y%2d{w]:   {g%s{x\n\r",
		rank, clan_table[clan].crnk[rank] );
	    send_to_char( buf, ch );
	}
	return FALSE;
    }

    if ( !is_number( arg ) )
    {
	send_to_char( "Syntax: rank (number) (name).\n\r", ch );
	return FALSE;
    }

    if ( ( rank = atoi( arg ) ) <= 0 || rank >= MAX_CRNK )
    {
	sprintf( buf, "Valid rank numbers are 1 - %d.\n\r", MAX_CRNK-1 );
	send_to_char( buf, ch );
	return FALSE;
    }

    sprintf( buf, "%s: Old Rank [%d]: %s, New Rank [%d]: %s\n\r",
	clan_table[clan].name, rank, clan_table[clan].crnk[rank], rank, argument );
    send_to_char( buf, ch );

    free_string( clan_table[clan].crnk[rank] );
    clan_table[clan].crnk[rank] = str_dup( argument );

    return TRUE;
}

OLC( clan_edit_recall )
{
    char buf[MAX_STRING_LENGTH];
    int clan, vnum;

    EDIT_TABLE( ch, clan );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GRecall Vnum{w:  {g%d\n\r", clan_table[clan].hall );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( !is_number( argument )
    ||   ( vnum = atoi( argument ) ) < 0 )
    {
	send_to_char( "Recall vnum must be a valid number\n\r", ch );
	return FALSE;
    }

    if ( get_room_index( vnum ) == NULL )
    {
	send_to_char( "That room does not exist.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "%s: Old recall vnum: %d, New recall vnum: %d.\n\r",
	clan_table[clan].name, clan_table[clan].hall, vnum );
    send_to_char( buf, ch );

    clan_table[clan].hall = vnum;

    return TRUE;
}

OLC( clan_edit_show )
{
    BUFFER *final = new_buf( );
    ROOM_INDEX_DATA *pRoom;
    char buf[MAX_STRING_LENGTH];
    int clan, rank;

    EDIT_TABLE( ch, clan );

    sprintf( buf, "{qNumber:         {s[{t%d{s]\n\r", clan );
    add_buf( final, buf );

    sprintf( buf, "{qName:           {s[{t%s{s]\n\r", clan_table[clan].name );
    add_buf( final, buf );

    sprintf( buf, "{qColor Name:     {s[{t%s{s]\n\r", clan_table[clan].color );
    add_buf( final, buf );

    sprintf( buf, "                {s[{t%s{s]\n\r",
	double_color( clan_table[clan].color ) );
    add_buf( final, buf );

    sprintf( buf, "{qWho Name:       {s[{t%s{s]\n\r", clan_table[clan].who_name );
    add_buf( final, buf );

    sprintf( buf, "                {s[{t%s{s]\n\r",
	double_color( clan_table[clan].who_name ) );
    add_buf( final, buf );

    sprintf( buf, "{qDescription:    {s[{t%s{s]\n\r", clan_table[clan].exname );
    add_buf( final, buf );

    sprintf( buf, "                {s[{t%s{s]\n\r",
	double_color( clan_table[clan].exname ) );
    add_buf( final, buf );

    pRoom = get_room_index( clan_table[clan].hall );
    sprintf( buf, "{qRecall Vnum:    {s[{t%d{s] [{t%s{s]\n\r",
	clan_table[clan].hall,
	pRoom == NULL ? "{R! None !" : pRoom->name );
    add_buf( final, buf );

    pRoom = get_room_index( clan_table[clan].portal_room );
    sprintf( buf, "{qPortal Vnum:    {s[{t%d{s] [{t%s{s]\n\r",
	clan_table[clan].portal_room,
	pRoom == NULL ? "{R! None !" : pRoom->name );
    add_buf( final, buf );

    pRoom = get_room_index( clan_table[clan].two_way_link );
    sprintf( buf, "{qTwo Way Link:   {s[{t%d{s] [{t%s{s]\n\r",
	clan_table[clan].two_way_link,
	pRoom == NULL ? "{R! None !" : pRoom->name );
    add_buf( final, buf );

    sprintf( buf, "{qDonation Pit:   {s[{t%d{s] [{t%s{s]\n\r",
	clan_table[clan].pit,
	get_obj_index( clan_table[clan].pit ) == NULL ? "{R! None !" :
	get_obj_index( clan_table[clan].pit )->short_descr );
    add_buf( final, buf );

    sprintf( buf, "{qMax Members:    {s[{t%d{s]\n\r", clan_table[clan].max_mem );
    add_buf( final, buf );

    sprintf( buf, "{qMembers:        {s[{t%d{s]\n\r", clan_table[clan].members );
    add_buf( final, buf );

    sprintf( buf, "{qKills:          {s[{t%d{s]\n\r", clan_table[clan].kills );
    add_buf( final, buf );

    sprintf( buf, "{qDeaths:         {s[{t%d{s]\n\r", clan_table[clan].deaths );
    add_buf( final, buf );

    sprintf( buf, "{qCubics:         {s[{t%d{s]\n\r", clan_table[clan].cubics );
    add_buf( final, buf );

    sprintf( buf, "{qAQuest:         {s[{t%d{s]\n\r", clan_table[clan].aquest );
    add_buf( final, buf );

    sprintf( buf, "{qIQuest:         {s[{t%d{s]\n\r", clan_table[clan].iquest );
    add_buf( final, buf );

    sprintf( buf, "{qIndependent:    {s[{t%s{s]\n\r",
	show_true_false( clan_table[clan].independent ) );
    add_buf( final, buf );

    sprintf( buf, "{qPkill:          {s[{t%s{s]\n\r",
	show_true_false( clan_table[clan].pkill ) );
    add_buf( final, buf );

    sprintf( buf, "{qData Time Left: {s[{t%s{s]\n\r",
	parse_time( clan_table[clan].edit_clan * 60 ) );
    add_buf( final, buf );

    sprintf( buf, "{qRoom Time Left: {s[{t%s{s]\n\r",
	parse_time( clan_table[clan].edit_room * 60 ) );
    add_buf( final, buf );

    sprintf( buf, "{qObj Time Left:  {s[{t%s{s]\n\r",
	parse_time( clan_table[clan].edit_obj * 60 ) );
    add_buf( final, buf );

    sprintf( buf, "{qMob Time Left:  {s[{t%s{s]\n\r",
	parse_time( clan_table[clan].edit_mob * 60 ) );
    add_buf( final, buf );

    sprintf( buf, "{qHelp Time Left: {s[{t%s{s]\n\r",
	parse_time( clan_table[clan].edit_help * 60 ) );
    add_buf( final, buf );

    sprintf( buf, "{qTwo Way Time:   {s[{t%s{s]\n\r",
	parse_time( clan_table[clan].two_way_time * 60 ) );
    add_buf( final, buf );

    if ( !clan_table[clan].independent )
    {
	for ( rank = MAX_CRNK-1; rank > 0; rank-- )
	{
	    sprintf( buf, "{qRank: {s[{t%2d{s]      [{t%s{s]\n\r"
			  "       %s [{t%s{s]\n\r",
		rank, clan_table[clan].crnk[rank],
		rank == 1 ? "{s[{tLowest{s]" : "        ",
		double_color( clan_table[clan].crnk[rank] ) );
	    add_buf( final, buf );
	}
    }

    add_buf( final, "{x" );
    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

OLC( clan_edit_who_name )
{
    char buf[MAX_STRING_LENGTH];
    int clan;

    EDIT_TABLE( ch, clan );

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "{GWho Name{w:     {g%s\n\r", clan_table[clan].who_name );
	send_to_char( buf, ch );
	return FALSE;
    }

    if ( strlen_color( argument ) != 11 )
    {
	send_to_char( "Clan who names must be exactly 11 characters not counting colors.\n\r", ch );
	return FALSE;
    }

    sprintf( buf, "%s: Old who_name: %s, New who_name: %s.\n\r",
	clan_table[clan].name, clan_table[clan].who_name, argument );
    send_to_char( buf, ch );

    free_string( clan_table[clan].who_name );
    clan_table[clan].who_name = str_dup( argument );

    return TRUE;
}

OLC( social_edit_cfound )
{
    int iSocial;

    EDIT_TABLE( ch, iSocial );

    free_string( social_table[iSocial].char_found );
    social_table[iSocial].char_found = str_dup( argument );

    if ( !argument[0] )
	send_to_char( "The character will now see nothing when a target is found.\n\r", ch );
    else
	printf_to_char( ch, "New message is now:\n\r%s\n\r", argument );

    return TRUE;
}

OLC( social_edit_cnoarg )
{
    int iSocial;

    EDIT_TABLE( ch, iSocial );

    free_string( social_table[iSocial].char_no_arg );
    social_table[iSocial].char_no_arg = str_dup( argument );

    if ( !argument[0] )
	send_to_char( "Character will now see nothing when this social is used without arguments.\n\r", ch );
    else
	printf_to_char( ch, "New message is now:\n\r%s\n\r", argument );

    return TRUE;
}

OLC( social_edit_create )
{
    struct social_type *new_table;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Which social do you wish to create?\n\r", ch );
	return FALSE;
    }

    if ( !check_argument( ch, argument, FALSE ) )
	return FALSE;

    if ( strlen( argument ) > 20 )
    {
	send_to_char( "Social names are limited to 20 characters.\n\r", ch );
	return FALSE;
    }

    if ( social_lookup( argument ) != -1 )
    {
	send_to_char( "A social with that name already exists.\n\r", ch );
	return FALSE;
    }

    maxSocial++;
    new_table = realloc( social_table, sizeof( struct social_type ) * ( maxSocial + 1 ) );

    if ( !new_table ) /* realloc failed */
    {
	send_to_char( "Memory allocation failed. Brace for impact.\n\r", ch );
	return FALSE;
    }

    social_table = new_table;

    strcpy( social_table[maxSocial-1].name, argument );
    social_table[maxSocial-1].char_no_arg	= str_dup( "" );
    social_table[maxSocial-1].others_no_arg	= str_dup( "" );
    social_table[maxSocial-1].char_found	= str_dup( "" );
    social_table[maxSocial-1].others_found	= str_dup( "" );
    social_table[maxSocial-1].vict_found	= str_dup( "" );
    social_table[maxSocial-1].char_auto		= str_dup( "" );
    social_table[maxSocial-1].others_auto	= str_dup( "" );

    social_table[maxSocial].name[0] = '\0';

    send_to_char( "New social added.\n\r", ch );

    ch->desc->editor = ED_SOCIAL;
    ch->desc->pEdit = (void *)maxSocial-1;

    return TRUE;
}

OLC( social_edit_cself )
{
    int iSocial;

    EDIT_TABLE( ch, iSocial );

    free_string( social_table[iSocial].char_auto );
    social_table[iSocial].char_auto = str_dup( argument );

    if ( !argument[0] )
	send_to_char( "Character will now see nothing when targetting self.\n\r", ch );
    else
	printf_to_char( ch, "New message is now:\n\r%s\n\r", argument );

    return TRUE;
}

OLC( social_edit_delete )
{
    int i, iSocial, j;
    struct social_type *new_table;

    EDIT_TABLE( ch, iSocial );

    if ( str_cmp( argument, social_table[iSocial].name ) )
    {
	send_to_char( "Argument must match social name and you must be editing social.\n\r", ch );
	return FALSE;
    }

    new_table = malloc( sizeof( struct social_type ) * maxSocial );

    if ( !new_table )
    {
	send_to_char( "Memory allocation failed. Brace for impact...\n\r", ch );
	return FALSE;
    }

    check_olc_delete( ch );

    for ( i = 0, j = 0; i < maxSocial+1; i++ )
    {
	if ( i != iSocial )
	{
	    new_table[j] = social_table[i];
	    j++;
	}
    }

    free( social_table );
    social_table = new_table;
		
    maxSocial--; /* Important :() */

    send_to_char( "That social is history now.\n\r", ch );

    return TRUE;
}

OLC( social_edit_name )
{
    int iSocial;

    EDIT_TABLE( ch, iSocial );

    if ( argument[0] == '\0' )
    {
	send_to_char( "What do you wish to rename it to?\n\r", ch );
	return FALSE;
    }

    if ( !check_argument( ch, argument, FALSE ) )
	return FALSE;

    if ( strlen( argument ) > 20 )
    {
	send_to_char( "Social names are limited to 20 characters.\n\r", ch );
	return FALSE;
    }

    if ( social_lookup( argument ) != -1 )
    {
	send_to_char( "That social already exists.\n\r", ch );
	return FALSE;
    }

    strcpy( social_table[iSocial].name, argument );

    send_to_char( "Social renamed.\n\r", ch );

    return TRUE;
}

OLC( social_edit_ofound )
{
    int iSocial;

    EDIT_TABLE( ch, iSocial );

    free_string( social_table[iSocial].others_found );
    social_table[iSocial].others_found = str_dup( argument );

    if ( !argument[0] )
	send_to_char( "Others will now see nothing when a target is found.\n\r", ch );
    else
	printf_to_char( ch, "New message is now:\n\r%s\n\r", argument );

    return TRUE;
}

OLC( social_edit_onoarg )
{
    int iSocial;

    EDIT_TABLE( ch, iSocial );

    free_string( social_table[iSocial].others_no_arg );
    social_table[iSocial].others_no_arg = str_dup( argument );

    if ( !argument[0] )
	send_to_char( "Others will now see nothing when this social is used without arguments.\n\r", ch );
    else
	printf_to_char( ch, "New message is now:\n\r%s\n\r", argument );

    return TRUE;
}

OLC( social_edit_oself )
{
    int iSocial;

    EDIT_TABLE( ch, iSocial );

    free_string( social_table[iSocial].others_auto );
    social_table[iSocial].others_auto = str_dup( argument );

    if ( !argument[0] )
	send_to_char( "Others will now see nothing when character targets self.\n\r", ch );
    else
	printf_to_char( ch, "New message is now:\n\r%s\n\r", argument );

    return TRUE;
}

OLC( social_edit_show )
{
    BUFFER *final = new_buf( );
    char buf[MAX_STRING_LENGTH];
    int iSocial;

    EDIT_TABLE( ch, iSocial );

    sprintf( buf, "{qSocial: {s[{t%d{s] {t%s\n\r\n\r",
	iSocial, social_table[iSocial].name );
    add_buf( final, buf );

    add_buf( final, "{s({tcnoarg{s) {qNo argument given, character sees:\n\r" );
    sprintf( buf, "  {t%s\n\r  {x%s\n\r\n\r",
	double_color( social_table[iSocial].char_no_arg ),
	social_table[iSocial].char_no_arg );
    add_buf( final, buf );

    add_buf( final, "{s({tonoarg{s) {qNo argument given, others see:\n\r" );
    sprintf( buf, "  {t%s\n\r  {x%s\n\r\n\r",
	double_color( social_table[iSocial].others_no_arg ),
	social_table[iSocial].others_no_arg );
    add_buf( final, buf );

    add_buf( final, "{s({tcfound{s) {qTarget found, character sees:\n\r" );
    sprintf( buf, "  {t%s\n\r  {x%s\n\r\n\r",
	double_color( social_table[iSocial].char_found ),
	social_table[iSocial].char_found );
    add_buf( final, buf );

    add_buf( final, "{s({tofound{s) {qTarget found, others see:\n\r" );
    sprintf( buf, "  {t%s\n\r  {x%s\n\r\n\r",
	double_color( social_table[iSocial].others_found ),
	social_table[iSocial].others_found );
    add_buf( final, buf );

    add_buf( final, "{s({tvfound{s) {qTarget found, victim sees:\n\r" );
    sprintf( buf, "  {t%s\n\r  {x%s\n\r\n\r",
	double_color( social_table[iSocial].vict_found ),
	social_table[iSocial].vict_found );
    add_buf( final, buf );

    add_buf( final, "{s({tcself{s)  {qTarget is character himself:\n\r" );
    sprintf( buf, "  {t%s\n\r  {x%s\n\r\n\r",
	double_color( social_table[iSocial].char_auto ),
	social_table[iSocial].char_auto );
    add_buf( final, buf );

    add_buf( final, "{s({toself{s)  {qTarget is character himself, others see:\n\r" );
    sprintf( buf, "  {t%s\n\r  {x%s{x\n\r",
	double_color( social_table[iSocial].others_auto ),
	social_table[iSocial].others_auto );
    add_buf( final, buf );

    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

OLC( social_edit_vfound )
{
    int iSocial;

    EDIT_TABLE( ch, iSocial );

    free_string( social_table[iSocial].vict_found );
    social_table[iSocial].vict_found = str_dup( argument );

    if ( !argument[0] )
	send_to_char( "Victim will now see nothing when a target is found.\n\r", ch );
    else
	printf_to_char( ch, "New message is now:\n\r%s\n\r", argument );

    return TRUE;
}

REDIT( redit_damage )
{
    ROOM_INDEX_DATA *pRoom = (ROOM_INDEX_DATA *)ch->desc->pEdit;
    ROOM_DAMAGE_DATA *dam;
    char arg[MAX_INPUT_LENGTH];
    int type;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: damage create <damtype>\n\r"
		      "        damage delete <damtype>\n\r"
		      "        damage edit   <damtype>\n\r", ch );
	return FALSE;
    }

    if ( ( type = dam_type_lookup( argument ) ) == -1 )
    {
	send_to_char( "That damage type does not exist.\n\r", ch );
	return FALSE;
    }

    else if ( !str_prefix( arg, "create" ) )
	return rdam_create( ch, argument );

    else if ( !str_prefix( arg, "delete" ) )
    {
	for ( dam = pRoom->room_damage; dam != NULL; dam = dam->next )
	{
	    if ( dam->damage_type == type )
	    {
		ch->desc->pEdit	= (void *)dam;
		ch->desc->editor = ED_ROOM_DAM;
		return rdam_delete( ch, argument );
	    }
	}

	send_to_char( "That damage type is not in this room.\n\r", ch );
	return FALSE;
    }

    else if ( !str_prefix( arg, "edit" ) )
    {
	for ( dam = pRoom->room_damage; dam != NULL; dam = dam->next )
	{
	    if ( dam->damage_type == type )
	    {
		ch->desc->pEdit	= (void *)dam;
		ch->desc->editor	= ED_ROOM_DAM;
		return FALSE;
	    }
	}

	send_to_char( "That damage type is not in this room.\n\r", ch );
	return FALSE;
    }

    redit_damage( ch, "" );
    return FALSE;
}

OLC( rdam_create )
{
    ROOM_INDEX_DATA *pRoom;
    ROOM_DAMAGE_DATA *dam;

    pRoom = ( ROOM_INDEX_DATA * )ch->desc->pEdit;

    dam		= new_room_damage( dam_type_lookup( argument ) );

    dam->next = pRoom->room_damage;
    pRoom->room_damage = dam;

    ch->desc->pEdit	= (void *)dam;
    ch->desc->editor	= ED_ROOM_DAM;

    return TRUE;
}

OLC( rdam_damtype )
{
    ROOM_DAMAGE_DATA *dam;
    int type;

    EDIT_RDAM( ch, dam );

    if ( argument[0] == '\0'
    ||   ( type = dam_type_lookup( argument ) ) == -1 )
    {
	send_to_char( "Invalid damage type.\n\r", ch );
	return FALSE;
    }

    dam->damage_type = type;

    send_to_char( "Damage type set.\n\r", ch );

    return TRUE;
}

OLC( rdam_delete )
{
    ROOM_DAMAGE_DATA *dam;

    EDIT_RDAM( ch, dam );

    check_olc_delete( ch );

    send_to_char( "Room damage deleted.\n\r", ch );

    ch->desc->pEdit = (void *)ch->in_room;
    ch->desc->editor = ED_ROOM;

    if ( ch->in_room->room_damage == dam )
	ch->in_room->room_damage = dam->next;
    else
    {
	ROOM_DAMAGE_DATA *prev;

	for ( prev = ch->in_room->room_damage; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == dam )
	    {
		prev->next = dam->next;
		break;
	    }
	}
    }

    free_room_damage( dam );

    return TRUE;
}

OLC( rdam_range )
{
    ROOM_DAMAGE_DATA *dam;
    char arg[MAX_INPUT_LENGTH];
    int range[2];

    EDIT_RDAM( ch, dam );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0'
    ||   !is_number( arg ) || !is_number( argument )
    ||   ( range[0] = atoi( arg ) ) < 0
    ||   ( range[1] = atoi( argument ) ) < 0
    ||   range[0] > 500 || range[1] > 500 || range[0] > range[1] )
    {
	send_to_char( "Syntax: range (min damage) (max damage).\n\r", ch );
	return FALSE;
    }

    dam->damage_min = range[0];
    dam->damage_max = range[1];

    send_to_char( "Damage range set.\n\r", ch );

    return TRUE;
}

OLC( rdam_room_msg )
{
    ROOM_DAMAGE_DATA *dam;

    EDIT_RDAM( ch, dam );

    if ( argument[0] == '\0' || !str_cmp( argument, "none" ) )
    {
	send_to_char( "Room message removed.\n\r", ch );
	free_string( dam->msg_room );
	dam->msg_room = str_dup( "" );
    } else {
	send_to_char( "Room message added.\n\r", ch );
	free_string( dam->msg_room );
	dam->msg_room = str_dup( argument );
    }

    return TRUE;
}

OLC( rdam_show )
{
    ROOM_DAMAGE_DATA *dam;
    char buf[MAX_STRING_LENGTH];

    EDIT_RDAM( ch, dam );

    sprintf( buf, "{qDamage type:  {s[{t%s{s]\n\r",
	damage_mod_table[dam->damage_type].name );
    send_to_char( buf, ch );

    sprintf( buf, "{qDamage range: {s[{t%d {sto {t%d{s]\n\r",
	dam->damage_min, dam->damage_max );
    send_to_char( buf, ch );

    sprintf( buf, "{qVictim msg:   {s[{t%s{s]\n\r",
	dam->msg_victim ? dam->msg_victim : "none" );
    send_to_char( buf, ch );

    sprintf( buf, "              {s[{t%s{s]\n\r",
	!dam->msg_victim ? "none" :
	double_color( dam->msg_victim ) );
    send_to_char( buf, ch );

    sprintf( buf, "{qRoom msg:     {s[{t%s{s]\n\r",
	dam->msg_room ? dam->msg_room : "none" );
    send_to_char( buf, ch );

    sprintf( buf, "              {s[{t%s{s]\n\r",
	!dam->msg_room ? "none" :
	double_color( dam->msg_room ) );
    send_to_char( buf, ch );

    sprintf( buf, "{qSuccess Rate: {s[{t%d%%{s]{x\n\r", dam->success );
    send_to_char( buf, ch );

    return FALSE;
}

OLC( rdam_success )
{
    ROOM_DAMAGE_DATA *dam;
    int percent;

    EDIT_RDAM( ch, dam );

    if ( argument[0] == '\0' || !is_number( argument )
    ||   ( percent = atoi( argument ) ) <= 0 || percent > 100 )
    {
	send_to_char( "Syntax: percent (number).   Valid numbers are 1 to 100.\n\r", ch );
	return FALSE;
    }

    dam->success = percent;

    return TRUE;
}

OLC( rdam_victim_msg )
{
    ROOM_DAMAGE_DATA *dam;

    EDIT_RDAM( ch, dam );

    if ( argument[0] == '\0' || !str_cmp( argument, "none" ) )
    {
	send_to_char( "Victim message removed.\n\r", ch );
	free_string( dam->msg_victim );
	dam->msg_victim = str_dup( "" );
    } else {
	send_to_char( "Victim message added.\n\r", ch );
	free_string( dam->msg_victim );
	dam->msg_victim = str_dup( argument );
    }

    return TRUE;
}

OLC( shop_edit_delete )
{
    SHOP_DATA *pShop;

    EDIT_SHOP( ch, pShop );

    check_olc_delete( ch );

    get_mob_index( pShop->keeper )->pShop = NULL;

    if ( pShop == shop_first )
    {
	if ( !pShop->next )
	{
	    shop_first = NULL;
	    shop_last = NULL;
	}

	else
	    shop_first = pShop->next;
    } else {
	SHOP_DATA *ipShop;

	for ( ipShop = shop_first; ipShop; ipShop = ipShop->next )
	{
	    if ( ipShop->next == pShop )
	    {
		if ( !pShop->next )
		{
		    shop_last = ipShop;
		    shop_last->next = NULL;
		}

		else
		    ipShop->next = pShop->next;
	    }
	}
    }

    free_shop( pShop );

    send_to_char( "Mobile is no longer a shopkeeper.\n\r", ch );

    ch->desc->pEdit = ( void * ) ( get_mob_index( pShop->keeper ) );
    ch->desc->editor = ED_MOBILE;

    return TRUE;
}

OLC( shop_edit_hours )
{
    SHOP_DATA *pShop;
    char arg[MAX_INPUT_LENGTH];
    sh_int time1, time2;

    EDIT_SHOP( ch, pShop );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number( arg )
    ||   argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  hours [#xopening] [#xclosing]\n\r", ch );
	return FALSE;
    }

    time1 = atoi( arg );
    time2 = atoi( argument );

    if ( time1 < 0 || time2 > 23 )
    {
	send_to_char( "Shop hours must be between 0 and 23.\n\r", ch );
	return FALSE;
    }

    if ( time1 > time2 )
    {
	send_to_char( "Closing time must be later than opening time.\n\r", ch );
	return FALSE;
    }

    pShop->open_hour	= time1;
    pShop->close_hour	= time2;

    send_to_char( "Shop hours set.\n\r", ch );
    return TRUE;
}

OLC( shop_edit_markdown )
{
    SHOP_DATA *pShop;

    EDIT_SHOP( ch, pShop );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  markdown [#xbuying%]\n\r", ch );
	return FALSE;
    }

    pShop->profit_sell = atoi( argument );

    send_to_char( "Shop markdown set.\n\r", ch );
    return TRUE;
}

OLC( shop_edit_markup )
{
    SHOP_DATA *pShop;

    EDIT_SHOP( ch, pShop );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  markup [#xselling%]\n\r", ch );
	return FALSE;
    }

    pShop->profit_buy = atoi( argument );

    send_to_char( "Shop markup set.\n\r", ch );

    return TRUE;
}

OLC( shop_edit_show )
{
    SHOP_DATA *pShop;
    char buf[MAX_STRING_LENGTH];
    sh_int iTrade;

    EDIT_SHOP( ch, pShop );

    sprintf( buf, "{qShop data for {s[{t%5d{s]{q:\n\r"
		  "  {qMarkup  (shop sells): {s[{t%d%%{s]\n\r"
		  "  {qMarkdown (shop buys): {s[{t%d%%{s]\n\r",
	    pShop->keeper, pShop->profit_buy, pShop->profit_sell );
    send_to_char( buf, ch );

    sprintf( buf, "{q  Hours: {s[{t%d {sto {t%d{s]\n\r",
	pShop->open_hour, pShop->close_hour );
    send_to_char( buf, ch );

    send_to_char( "{q  Number Trades Type\n\r", ch );
    send_to_char( "{t  ------ -----------\n\r", ch );

    for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
    {
	sprintf( buf, "  {s[{t%4d{s] {q%s{x\n\r", iTrade,
	    pShop->buy_type[iTrade] == 0 ? "none" :
	    flag_string( type_flags, pShop->buy_type[iTrade] ) );
	send_to_char( buf, ch );
    }

    return FALSE;
}

OLC( shop_edit_trade )
{
    SHOP_DATA *pShop;
    char arg[MAX_INPUT_LENGTH];
    sh_int number, item;

    EDIT_SHOP( ch, pShop );

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number( arg ) )
    {
	send_to_char(	"Syntax:  trade [#] [object_type]\n\r"
			"         trade [#] none\n\r\n\r"
			"Available Object Types:\n\r", ch );
	show_flag_cmds( ch, type_flags );
	return FALSE;
    }

    number = atoi( arg );

    if ( number < 0 || number >= MAX_TRADE )
    {
	send_to_char( "That is not a valid shop number.\n\r", ch );
	return FALSE;
    }

    if ( argument[0] == '\0' )
    {
	show_flag_cmds( ch, type_flags );
	return FALSE;
    }

    else if ( !str_cmp( argument, "none" ) )
	item = 0;

    else if ( ( item = flag_value( type_flags, argument ) ) == NO_FLAG )
    {
	send_to_char( "That type of item is not known.\n\r", ch );
	return FALSE;
    }

    pShop->buy_type[number] = item;

    send_to_char( "Shop type set.\n\r", ch );

    return TRUE;
}

MPEDIT ( opedit_create )
{
    PROG_CODE *pMcode;
    int value;

    value = atoi( argument );
    if ( argument[0] == '\0' || value == 0 )
    {
	send_to_char( "Syntax: opedit create [vnum]\n\r", ch );
	return FALSE;
    }

    if ( get_prog_index( value, PRG_OPROG ) )
    {
	send_to_char( "OPEdit: Code vnum already exists.\n\r", ch );
	return FALSE;
    }

    if ( !get_vnum_area( value ) )
    {
    	send_to_char( "OPEdit: Vnum not assigned an area.\n\r", ch );
    	return FALSE;
    }

    pMcode		= new_pcode( );
    pMcode->author	= str_dup( ch->name );
    pMcode->vnum	= value;
    pMcode->area	= get_vnum_area( value );
    pMcode->next	= oprog_list;
    oprog_list		= pMcode;
    ch->desc->pEdit	= (void *)pMcode;
    ch->desc->editor	= ED_OPCODE;
    SET_BIT( pMcode->area->area_flags, AREA_CHANGED );
    send_to_char( "Object Program Code Created.\n\r", ch );
    return TRUE;
}

MPEDIT ( rpedit_create )
{
    PROG_CODE *pMcode;
    int value;

    value = atoi( argument );
    if ( argument[0] == '\0' || value == 0 )
    {
	send_to_char( "Syntax: rpedit create [vnum]\n\r", ch );
	return FALSE;
    }

    if ( get_prog_index( value, PRG_RPROG ) )
    {
	send_to_char( "RPEdit: Code vnum already exists.\n\r", ch );
	return FALSE;
    }

    if ( !get_vnum_area( value ) )
    {
    	send_to_char( "RPEdit: Vnum not assigned an area.\n\r", ch );
    	return FALSE;
    }

    pMcode		= new_pcode( );
    pMcode->author	= str_dup( ch->name );
    pMcode->vnum	= value;
    pMcode->area	= get_vnum_area( value );
    pMcode->next	= rprog_list;
    rprog_list		= pMcode;
    ch->desc->pEdit	= (void *)pMcode;
    ch->desc->editor	= ED_RPCODE;
    SET_BIT( pMcode->area->area_flags, AREA_CHANGED );
    send_to_char( "RoomProgram Code Created.\n\r", ch );
    return TRUE;
}

MPEDIT(opedit_show)
{
    PROG_CODE *pOcode;
    char buf[MAX_STRING_LENGTH];

    EDIT_PCODE(ch,pOcode);

    sprintf(buf,
           "{qVnum:       {s[{t%d{s]\n\r"
           "{qCode:\n\r{t%s{x\n\r",
           pOcode->vnum, pOcode->code);
    send_to_char(buf, ch);

    return FALSE;
}

MPEDIT(rpedit_show)
{
    PROG_CODE *pRcode;
    char buf[MAX_STRING_LENGTH];

    EDIT_PCODE(ch,pRcode);

    sprintf(buf,
           "{qVnum:       {s[{t%d{s]\n\r"
           "{qCode:\n\r{t%s{x\n\r",
           pRcode->vnum, pRcode->code);
    send_to_char(buf, ch);

    return FALSE;
}

MPEDIT(opedit_code)
{
    PROG_CODE *pOcode;
    EDIT_PCODE(ch, pOcode);

    if (argument[0] =='\0')
    {
       string_append(ch, &pOcode->code);
       return TRUE;
    }

    send_to_char("Syntax: code\n\r",ch);
    return FALSE;
}

MPEDIT(rpedit_code)
{
    PROG_CODE *pRcode;
    EDIT_PCODE(ch, pRcode);

    if (argument[0] =='\0')
    {
       string_append(ch, &pRcode->code);
       return TRUE;
    }

    send_to_char("Syntax: code\n\r",ch);
    return FALSE;
}

MPEDIT( rpedit_name )
{
    PROG_CODE *pMcode;
    EDIT_PCODE(ch, pMcode);

    if ( argument[0] != '\0' )
    {
	free_string(pMcode->name);
	pMcode->name = str_dup(argument);
	SET_BIT( pMcode->area->area_flags, AREA_CHANGED );
	send_to_char( "Name description set.\n\r", ch );
	return TRUE;
    }

    send_to_char(" Syntax: rpedit name <name>\n\r",ch);
    return FALSE;
}

MPEDIT( rpedit_author )
{
    PROG_CODE *pMcode;
    EDIT_PCODE(ch, pMcode);

    if ( argument[0] != '\0' )
    {
	free_string(pMcode->author);
	pMcode->author = str_dup(argument);
	SET_BIT( pMcode->area->area_flags, AREA_CHANGED );
	send_to_char( "Author description set.\n\r", ch );
	return TRUE;
    }

    send_to_char(" Syntax: rpedit author <name>\n\r",ch);
    return FALSE;
}

MPEDIT( opedit_name )
{
    PROG_CODE *pMcode;
    EDIT_PCODE(ch, pMcode);

    if ( argument[0] != '\0' )
    {
	free_string(pMcode->name);
	pMcode->name = str_dup(argument);
	SET_BIT( pMcode->area->area_flags, AREA_CHANGED );
	send_to_char( "Name description set.\n\r", ch );
	return TRUE;
    }

    send_to_char(" oyntax: mpedit name <name>\n\r",ch);
    return FALSE;
}

MPEDIT( opedit_author )
{
    PROG_CODE *pMcode;
    EDIT_PCODE(ch, pMcode);

    if ( argument[0] != '\0' )
    {
	free_string(pMcode->author);
	pMcode->author = str_dup(argument);
	SET_BIT( pMcode->area->area_flags, AREA_CHANGED );
	send_to_char( "Author description set.\n\r", ch );
	return TRUE;
    }

    send_to_char(" Syntax: opedit author <name>\n\r",ch);
    return FALSE;
}

MPEDIT( opedit_delete )
{
    OBJ_INDEX_DATA *pObj;
    PROG_CODE *pMcode;
    PROG_LIST *mp, *mp_next;
    char buf[MAX_STRING_LENGTH];
    int count = 0, match = 0, vnum;

    EDIT_PCODE( ch, pMcode );

    if ( argument[0] == '\0' || !is_number( argument )
    ||   atoi( argument ) != pMcode->vnum )
    {
	send_to_char( "Argument must match the program you are editing.\n\r", ch );
	return FALSE;
    }

    check_olc_delete( ch );

    for ( vnum = 0; match < top_obj_index; vnum++ )
    {
	if ( ( pObj = get_obj_index( vnum ) ) != NULL )
	{
	    match++;
	    for ( mp = pObj->oprogs; mp != NULL; mp = mp_next )
	    {
		mp_next = mp->next;

		if ( mp->vnum == pMcode->vnum )
		{
		    if ( mp == pObj->oprogs )
			pObj->oprogs = pObj->oprogs->next;
		    else
		    {
			PROG_LIST *prev;

			for ( prev = pObj->oprogs; prev != NULL; prev = prev->next )
			{
			    if ( prev->next == mp )
			    {
				prev->next = mp->next;
				break;
			    }
			}
		    }

		    count++;
		    SET_BIT( pObj->area->area_flags, AREA_CHANGED );
		    free_prog( mp );
		}
	    }
	}
    }

    if ( pMcode == oprog_list )
	oprog_list = pMcode->next;
    else
    {
	PROG_CODE *prev;

	for ( prev = oprog_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == pMcode )
	    {
		prev->next = pMcode->next;
		break;
	    }
	}
    }

    sprintf( buf, "Deleted oprog [%d] and removed from %d object%s.\n\r",
	pMcode->vnum, count, count == 1 ? "" : "s" );
    send_to_char( buf, ch );

    free_pcode( pMcode );

    return TRUE;
}

MPEDIT( rpedit_delete )
{
    ROOM_INDEX_DATA *pRoom;
    PROG_CODE *pMcode;
    PROG_LIST *mp, *mp_next;
    char buf[MAX_STRING_LENGTH];
    int count = 0, match = 0, vnum;

    EDIT_PCODE( ch, pMcode );

    if ( argument[0] == '\0' || !is_number( argument )
    ||   atoi( argument ) != pMcode->vnum )
    {
	send_to_char( "Argument must match the program you are editing.\n\r", ch );
	return FALSE;
    }

    check_olc_delete( ch );

    for ( vnum = 0; match < top_room; vnum++ )
    {
	if ( ( pRoom = get_room_index( vnum ) ) != NULL )
	{
	    match++;
	    for ( mp = pRoom->rprogs; mp != NULL; mp = mp_next )
	    {
		mp_next = mp->next;

		if ( mp->vnum == pMcode->vnum )
		{
		    if ( mp == pRoom->rprogs )
			pRoom->rprogs = pRoom->rprogs->next;
		    else
		    {
			PROG_LIST *prev;

			for ( prev = pRoom->rprogs; prev != NULL; prev = prev->next )
			{
			    if ( prev->next == mp )
			    {
				prev->next = mp->next;
				break;
			    }
			}
		    }

		    count++;
		    SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
		    free_prog( mp );
		}
	    }
	}
    }

    if ( pMcode == rprog_list )
	rprog_list = pMcode->next;
    else
    {
	PROG_CODE *prev;

	for ( prev = rprog_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == pMcode )
	    {
		prev->next = pMcode->next;
		break;
	    }
	}
    }

    sprintf( buf, "Deleted rprog [%d] and removed from %d room%s.\n\r",
	pMcode->vnum, count, count == 1 ? "" : "s" );
    send_to_char( buf, ch );

    free_pcode( pMcode );

    return TRUE;
}

bool game_edit_capslock( CHAR_DATA *ch, char *argument )
{
    mud_stat.capslock = !mud_stat.capslock;

    if ( mud_stat.capslock )
    {
	wiznet( "$N has capslocked the game.", ch, NULL, 0, 0, 0 );
	send_to_char( "Capslock set.\n\r", ch );
    } else {
	wiznet( "$N removes capslock.", ch, NULL, 0, 0, 0 );
	send_to_char( "Capslock removed.\n\r", ch );
    }

    return TRUE;
}

bool game_edit_colorlock( CHAR_DATA *ch, char *argument )
{
    mud_stat.colorlock = !mud_stat.colorlock;

    if ( mud_stat.colorlock )
    {
	wiznet( "$N has colorlocked the game.", ch, NULL, 0, 0, 0 );
	send_to_char( "Colorlock set.\n\r", ch );
    } else {
	wiznet( "$N removes colorlock.", ch, NULL, 0, 0, 0 );
	send_to_char( "Colorlock removed.\n\r", ch );
    }

    return TRUE;
}

bool game_edit_connections( CHAR_DATA *ch, char *argument )
{
    int value;

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax: connections <number>\n\r", ch );
	return FALSE;
    }

    if ( ( value = atoi( argument ) ) < 0 )
    {
	send_to_char( "Connections since reboot must be a positive number.\n\r", ch );
	return FALSE;
    }

    printf_to_char( ch, "Connections since reboot changed from %d to %d people.\n\r",
	mud_stat.connect_since_reboot, value );

    mud_stat.connect_since_reboot = value;

    return TRUE;
}

bool game_edit_evil_god( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: evil_god <new god name>\n\r", ch );
	return FALSE;
    }

    free_string( mud_stat.evil_god_string );
    mud_stat.evil_god_string = str_dup( argument );

    send_to_char( "Evil god string set.\n\r", ch );

    return TRUE;
}

bool game_edit_good_god( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: good_god <new god name>\n\r", ch );
	return FALSE;
    }

    free_string( mud_stat.good_god_string );
    mud_stat.good_god_string = str_dup( argument );

    send_to_char( "Good god string set.\n\r", ch );

    return TRUE;
}

bool game_edit_imm_timer( CHAR_DATA *ch, char *argument )
{
    int value;

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax: imm_timer <number>\n\r", ch );
	return FALSE;
    }

    if ( ( value = atoi( argument ) ) < 0 )
    {
	send_to_char( "Timeout values must be positive.\n\r", ch );
	return FALSE;
    }

    printf_to_char( ch, "Immortal timeout changed from %d to %d ticks.\n\r",
	mud_stat.timeout_immortal, value );
    mud_stat.timeout_immortal = value;

    return TRUE;
}

bool game_edit_ld_immortal( CHAR_DATA *ch, char *argument )
{
    int value;

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax: ld_immortal <number>\n\r", ch );
	return FALSE;
    }

    if ( ( value = atoi( argument ) ) < 0 )
    {
	send_to_char( "Timeout values must be positive.\n\r", ch );
	return FALSE;
    }

    printf_to_char( ch, "Immortal linkdead timeout changed from %d to %d ticks.\n\r",
	mud_stat.timeout_ld_imm, value );
    mud_stat.timeout_ld_imm = value;

    return TRUE;
}

bool game_edit_ld_mortal( CHAR_DATA *ch, char *argument )
{
    int value;

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax: ld_mortal <number>\n\r", ch );
	return FALSE;
    }

    if ( ( value = atoi( argument ) ) < 0 )
    {
	send_to_char( "Timeout values must be positive.\n\r", ch );
	return FALSE;
    }

    printf_to_char( ch, "Mortal linkdead timeout changed from %d to %d ticks.\n\r",
	mud_stat.timeout_ld_mort, value );
    mud_stat.timeout_ld_mort = value;

    return TRUE;
}

bool game_edit_log_all( CHAR_DATA *ch, char *argument )
{
    mud_stat.fLogAll = !mud_stat.fLogAll;

    if ( mud_stat.fLogAll )
	send_to_char( "Log ALL set.\n\r", ch );
    else
	send_to_char( "Log ALL removed.\n\r", ch );

    return TRUE;
}

bool game_edit_max_ever( CHAR_DATA *ch, char *argument )
{
    int value;

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax: max_ever <number>\n\r", ch );
	return FALSE;
    }

    if ( ( value = atoi( argument ) ) < 0 )
    {
	send_to_char( "Max ever must be more than zero.\n\r", ch );
	return FALSE;
    }

    printf_to_char( ch, "Max ever changed from %d to %d people.\n\r",
	mud_stat.max_ever, value );
    mud_stat.max_ever = value;

    return TRUE;
}

bool game_edit_mortal_timer( CHAR_DATA *ch, char *argument )
{
    int value;

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax: mort_timer <number>\n\r", ch );
	return FALSE;
    }

    if ( ( value = atoi( argument ) ) < 0 )
    {
	send_to_char( "Timeout values must be positive.\n\r", ch );
	return FALSE;
    }

    printf_to_char( ch, "Mortal timeout changed from %d to %d ticks.\n\r",
	mud_stat.timeout_mortal, value );
    mud_stat.timeout_mortal = value;

    return TRUE;
}

bool game_edit_most_today( CHAR_DATA *ch, char *argument )
{
    int value;

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax: most_today <number>\n\r", ch );
	return FALSE;
    }

    if ( ( value = atoi( argument ) ) < 0 )
    {
	send_to_char( "Most today must be more than zero.\n\r", ch );
	return FALSE;
    }

    printf_to_char( ch, "Most today changed from %d to %d people.\n\r",
	mud_stat.most_today, value );
    mud_stat.most_today = value;

    return TRUE;
}

bool game_edit_multilock( CHAR_DATA *ch, char *argument )
{
    mud_stat.multilock = !mud_stat.multilock;

    if ( mud_stat.multilock )
    {
	CHAR_DATA *victim, *wch;

	wiznet( "$N has multilocked the game.", ch, NULL, 0, 0, 0 );
	send_to_char( "Multilock set.\n\r", ch );

	for ( victim = player_list; victim != NULL; victim = victim->pcdata->next_player )
	{
	    if ( IS_NPC( victim )
	    ||   IS_IMMORTAL( victim )
	    ||   !str_cmp( victim->name, "Tester" )
	    ||   !str_cmp( victim->name, "Testguy" )
	    ||   victim->pcdata->socket == NULL )
		continue;

	    for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
	    {
		if ( !IS_IMMORTAL( wch )
		&&   str_cmp( wch->name, "Tester" )
		&&   str_cmp( wch->name, "Testguy" )
		&&   str_cmp( victim->name, wch->name )
		&&   wch->pcdata->socket != NULL
		&&   !check_allow( wch->pcdata->socket, ALLOW_ITEMS )
		&&   !check_allow( wch->pcdata->socket, ALLOW_CONNECTS )
		&&   !strcmp( victim->pcdata->socket, wch->pcdata->socket ) )
                {
		    send_to_char( "Sorry, multiplaying has been terminated.\n\r", wch );
		    force_quit( wch, "" );
		    continue;
		}
	    }
	}
    } else {
	wiznet( "$N removes multilock.", ch, NULL, 0, 0, 0 );
	send_to_char( "Multilock removed.\n\r", ch );
    }

    return TRUE;
}

bool game_edit_name( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: name <new mud name>\n\r", ch );
	return FALSE;
    }

    free_string( mud_stat.mud_name_string );
    mud_stat.mud_name_string = str_dup( argument );

    send_to_char( "Mud name set.\n\r", ch );

    return TRUE;
}

bool game_edit_neutral_god( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: neutral_god <new god name>\n\r", ch );
	return FALSE;
    }

    free_string( mud_stat.neut_god_string );
    mud_stat.neut_god_string = str_dup( argument );

    send_to_char( "Neutral god string set.\n\r", ch );

    return TRUE;
}

bool game_edit_newlock( CHAR_DATA *ch, char *argument )
{
    mud_stat.newlock = !mud_stat.newlock;

    if ( mud_stat.newlock )
    {
	wiznet( "$N has newlocked the game.", ch, NULL, 0, 0, 0 );
	send_to_char( "Newlock set.\n\r", ch );
    } else {
	wiznet( "$N removes newlock.", ch, NULL, 0, 0, 0 );
	send_to_char( "Newlock removed.\n\r", ch );
    }

    return TRUE;
}

bool game_edit_quest_exp( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int value[2];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: qexp [min] [max]\n\r", ch );
	return FALSE;
    }

    if ( !is_number( arg ) || !is_number( argument ) )
    {
	send_to_char( "That is not a valid number!\n\r", ch );
	return FALSE;
    }

    value[0] = atoi( arg );
    value[1] = atoi( argument );

    if ( value[0] < 0 || value[1] < 0 )
    {
	send_to_char( "Values must me above zero.\n\r", ch );
	return FALSE;
    }

    if ( value[0] > value[1] )
    {
	send_to_char( "Min value can not be more than max value.\n\r", ch );
	return FALSE;
    }

    sprintf( arg, "{qOld Quest Exp: {s[{t%3d {sto {t%3d{s]{q, New Quest Exp: {s[{t%3d {sto {t%3d{s]{x\n\r",
	mud_stat.quest_exp[0], mud_stat.quest_exp[1],
	value[0], value[1] );
    send_to_char( arg, ch );

    mud_stat.quest_exp[0] = value[0];
    mud_stat.quest_exp[1] = value[1];

    return TRUE;
}

bool game_edit_quest_gold( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int value[2];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: qgold [min] [max]\n\r", ch );
	return FALSE;
    }

    if ( !is_number( arg ) || !is_number( argument ) )
    {
	send_to_char( "That is not a valid number!\n\r", ch );
	return FALSE;
    }

    value[0] = atoi( arg );
    value[1] = atoi( argument );

    if ( value[0] < 0 || value[1] < 0 )
    {
	send_to_char( "Values must me above zero.\n\r", ch );
	return FALSE;
    }

    if ( value[0] > value[1] )
    {
	send_to_char( "Min value can not be more than max value.\n\r", ch );
	return FALSE;
    }

    sprintf( arg, "{qOld Quest Gold: {s[{t%3d {sto {t%3d{s]{q, New Quest Gold: {s[{t%3d {sto {t%3d{s]{x\n\r",
	mud_stat.quest_gold[0], mud_stat.quest_gold[1],
	value[0], value[1] );
    send_to_char( arg, ch );

    mud_stat.quest_gold[0] = value[0];
    mud_stat.quest_gold[1] = value[1];

    return TRUE;
}

bool game_edit_quest_object( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int value[2];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char(	"Syntax: qobject [min] [max]\n\r", ch );
	return FALSE;
    }

    if ( !is_number( arg ) || !is_number( argument ) )
    {
	send_to_char( "That is not a valid number!\n\r", ch );
	return FALSE;
    }

    value[0] = atoi( arg );
    value[1] = atoi( argument );

    if ( value[0] < 0 || value[1] < 0 )
    {
	send_to_char( "Values must me above zero.\n\r", ch );
	return FALSE;
    }

    if ( value[0] > value[1] )
    {
	send_to_char( "Min value can not be more than max value.\n\r", ch );
	return FALSE;
    }

    sprintf( arg, "{qOld Quest Object Range: {s[{t%3d {sto {t%3d{s]{q, New Quest Object Range: {s[{t%3d {sto {t%3d{s]{x\n\r",
	mud_stat.quest_object[0], mud_stat.quest_object[1],
	value[0], value[1] );
    send_to_char( arg, ch );

    mud_stat.quest_object[0] = value[0];
    mud_stat.quest_object[1] = value[1];

    return TRUE;
}

bool game_edit_quest_vnum( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int value[2];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char(	"Syntax: qvnum [min] [max]\n\r", ch );
	return FALSE;
    }

    if ( !is_number( arg ) || !is_number( argument ) )
    {
	send_to_char( "That is not a valid number!\n\r", ch );
	return FALSE;
    }

    value[0] = atoi( arg );
    value[1] = atoi( argument );

    if ( value[0] < 0 || value[1] < 0 )
    {
	send_to_char( "Values must me above zero.\n\r", ch );
	return FALSE;
    }

    if ( value[0] > value[1] )
    {
	send_to_char( "Min value can not be more than max value.\n\r", ch );
	return FALSE;
    }

    sprintf( arg, "{qOld Quest Object Vnum: {s[{t%3d {sto {t%3d{s]{q, New Quest Object Vnum: {s[{t%3d {sto {t%3d{s]{x\n\r",
	mud_stat.quest_obj_vnum[0], mud_stat.quest_obj_vnum[1],
	value[0], value[1] );
    send_to_char( arg, ch );

    mud_stat.quest_obj_vnum[0] = value[0];
    mud_stat.quest_obj_vnum[1] = value[1];

    return TRUE;
}

bool game_edit_quest_points( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int value[2];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: qpoints [min] [max]\n\r", ch );
	return FALSE;
    }

    if ( !is_number( arg ) || !is_number( argument ) )
    {
	send_to_char( "That is not a valid number!\n\r", ch );
	return FALSE;
    }

    value[0] = atoi( arg );
    value[1] = atoi( argument );

    if ( value[0] < 0 || value[1] < 0 )
    {
	send_to_char( "Values must me above zero.\n\r", ch );
	return FALSE;
    }

    if ( value[0] > value[1] )
    {
	send_to_char( "Min value can not be more than max value.\n\r", ch );
	return FALSE;
    }

    sprintf( arg, "{qOld Quest Point Range: {s[{t%3d {sto {t%3d{s]{q, New Quest Point Range: {s[{t%3d {sto {t%3d{s]{x\n\r",
	mud_stat.quest_points[0], mud_stat.quest_points[1],
	value[0], value[1] );
    send_to_char( arg, ch );

    mud_stat.quest_points[0] = value[0];
    mud_stat.quest_points[1] = value[1];

    return TRUE;
}

bool game_edit_quest_pracs( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int value[2];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: qpracs [min] [max]\n\r", ch );
	return FALSE;
    }

    if ( !is_number( arg ) || !is_number( argument ) )
    {
	send_to_char( "That is not a valid number!\n\r", ch );
	return FALSE;
    }

    value[0] = atoi( arg );
    value[1] = atoi( argument );

    if ( value[0] < 0 || value[1] < 0 )
    {
	send_to_char( "Values must me above zero.\n\r", ch );
	return FALSE;
    }

    if ( value[0] > value[1] )
    {
	send_to_char( "Min value can not be more than max value.\n\r", ch );
	return FALSE;
    }

    sprintf( arg, "{qOld Quest Prac Range: {s[{t%3d {sto {t%3d{s]{q, New Quest Prac Range: {s[{t%3d {sto {t%3d{s]{x\n\r",
	mud_stat.quest_pracs[0], mud_stat.quest_pracs[1],
	value[0], value[1] );
    send_to_char( arg, ch );

    mud_stat.quest_pracs[0] = value[0];
    mud_stat.quest_pracs[1] = value[1];

    return TRUE;
}

bool game_edit_random( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int value[2];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: randoms [min_vnum] [max_vnum]\n\r", ch );
	return FALSE;
    }

    if ( !is_number( arg ) || !is_number( argument ) )
    {
	send_to_char( "That is not a valid number!\n\r", ch );
	return FALSE;
    }

    value[0] = atoi( arg );
    value[1] = atoi( argument );

    if ( value[0] < 0 || value[1] < 0 )
    {
	send_to_char( "Values must me above zero.\n\r", ch );
	return FALSE;
    }

    if ( value[0] > value[1] )
    {
	send_to_char( "Min value can not be more than max value.\n\r", ch );
	return FALSE;
    }

    if ( get_obj_index( value[0] ) == NULL )
    {
	send_to_char( "No object defined for lower vnum.\n\r", ch );
	return FALSE;
    }

    if ( get_obj_index( value[1] ) == NULL )
    {
	send_to_char( "No object defined for upper vnum.\n\r", ch );
	return FALSE;
    }

    sprintf( arg, "{qOld Random Object Vnums: {s[{t%5d {sto {t%5d{s]{q, New Random Object Vnums: {s[{t%5d {sto {t%5d{s]{x\n\r",
	mud_stat.random_vnum[0], mud_stat.random_vnum[1],
	value[0], value[1] );
    send_to_char( arg, ch );

    mud_stat.random_vnum[0] = value[0];
    mud_stat.random_vnum[1] = value[1];

    return TRUE;
}

bool game_edit_show( CHAR_DATA *ch, char *argument )
{
    BUFFER *final = new_buf( );
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    add_buf( final, "{s =============================================================================\n\r" );

    sprintf( buf1, "General Data" );
    sprintf( buf2, "| {q%s {s|\n\r", center_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf2, "Mud Name:     {s[{t%s{s]", mud_stat.mud_name_string );
    sprintf( buf1, "%s [{t%s{s]", end_string( buf2, 40 ),
	double_color( mud_stat.mud_name_string ) );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf2, "Good God:     {s[{t%s{s]", mud_stat.good_god_string );
    sprintf( buf1, "%s [{t%s{s]", end_string( buf2, 40 ),
	double_color( mud_stat.good_god_string ) );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf2, "Neutral God:  {s[{t%s{s]", mud_stat.neut_god_string );
    sprintf( buf1, "%s [{t%s{s]", end_string( buf2, 40 ),
	double_color( mud_stat.neut_god_string ) );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf2, "Evil God:     {s[{t%s{s]", mud_stat.evil_god_string );
    sprintf( buf1, "%s [{t%s{s]", end_string( buf2, 40 ),
	double_color( mud_stat.evil_god_string ) );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    add_buf( final, "{s =============================================================================\n\r" );

    sprintf( buf1, "Settings" );
    sprintf( buf2, "| {q%s {s|\n\r", center_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Capslock:     {s[{t%s{s]      {qImmortal TimeOut:  Active: {s[{t%3d{s] {qLinkDead: {s[{t%3d{s]",
	mud_stat.capslock ? " ON" : "OFF",
	mud_stat.timeout_immortal, mud_stat.timeout_ld_imm );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Colorlock:    {s[{t%s{s]        {qMortal TimeOut:  Active: {s[{t%3d{s] {qLinkDead: {s[{t%3d{s]",
	mud_stat.colorlock ? " ON" : "OFF",
	mud_stat.timeout_mortal, mud_stat.timeout_ld_mort );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Multilock:    {s[{t%s{s]",
	mud_stat.multilock ? " ON" : "OFF" );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Newlock:      {s[{t%s{s]",
	mud_stat.newlock ? " ON" : "OFF" );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Wizlock:      {s[{t%s{s]",
	mud_stat.wizlock ? " ON" : "OFF" );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Log All:      {s[{t%s{s]",
	mud_stat.fLogAll ? " ON" : "OFF" );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    add_buf( final, "{s =============================================================================\n\r" );

    sprintf( buf1, "Stat Counters" );
    sprintf( buf2, "| {q%s {s|\n\r", center_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Most Today:   {s[{t%3d{s]",
	mud_stat.most_today );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Max Ever:     {s[{t%3d{s]",
	mud_stat.max_ever );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Connections:  {s[{t%3d{s]",
	mud_stat.connect_since_reboot );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    add_buf( final, "{s =============================================================================\n\r" );

    sprintf( buf1, "Quest Settings" );
    sprintf( buf2, "| {q%s {s|\n\r", center_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Quest Exp:    {s[{t%5d {sto {t%5d{s]",
	mud_stat.quest_exp[0], mud_stat.quest_exp[1] );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Quest Gold:   {s[{t%5d {sto {t%5d{s]",
	mud_stat.quest_gold[0], mud_stat.quest_gold[1] );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Quest Object: {s[{t%5d {sto {t%5d{s]",
	mud_stat.quest_object[0], mud_stat.quest_object[1] );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Object Vnums: {s[{t%5d {sto {t%5d{s]",
	mud_stat.quest_obj_vnum[0], mud_stat.quest_obj_vnum[1] );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Quest Points: {s[{t%5d {sto {t%5d{s]",
	mud_stat.quest_points[0], mud_stat.quest_points[1] );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Quest Pracs:  {s[{t%5d {sto {t%5d{s]",
	mud_stat.quest_pracs[0], mud_stat.quest_pracs[1] );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    add_buf( final, "{s =============================================================================\n\r" );

    sprintf( buf1, "Random Settings" );
    sprintf( buf2, "| {q%s {s|\n\r", center_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Random Vnums: {s[{t%5d {sto {t%5d{s]",
	mud_stat.random_vnum[0], mud_stat.random_vnum[1] );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    sprintf( buf1, "Unique Vnums: {s[{t%5d {sto {t%5d{s]",
	mud_stat.unique_vnum[0], mud_stat.unique_vnum[1] );
    sprintf( buf2, "| {q%s |\n\r", end_string( buf1, 75 ) );
    add_buf( final, buf2 );

    add_buf( final, " {s============================================================================={x\n\r" );

    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}

bool game_edit_unique( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int value[2];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: uniques [min_vnum] [max_vnum]\n\r", ch );
	return FALSE;
    }

    if ( !is_number( arg ) || !is_number( argument ) )
    {
	send_to_char( "That is not a valid number!\n\r", ch );
	return FALSE;
    }

    value[0] = atoi( arg );
    value[1] = atoi( argument );

    if ( value[0] < 0 || value[1] < 0 )
    {
	send_to_char( "Values must me above zero.\n\r", ch );
	return FALSE;
    }

    if ( value[0] > value[1] )
    {
	send_to_char( "Min value can not be more than max value.\n\r", ch );
	return FALSE;
    }

    if ( get_obj_index( value[0] ) == NULL )
    {
	send_to_char( "No object defined for lower vnum.\n\r", ch );
	return FALSE;
    }

    if ( get_obj_index( value[1] ) == NULL )
    {
	send_to_char( "No object defined for upper vnum.\n\r", ch );
	return FALSE;
    }

    sprintf( arg, "{qOld Unique Object Vnums: {s[{t%5d {sto {t%5d{s]{q, New Unique Object Vnums: {s[{t%5d {sto {t%5d{s]{x\n\r",
	mud_stat.unique_vnum[0], mud_stat.unique_vnum[1],
	value[0], value[1] );
    send_to_char( arg, ch );

    mud_stat.unique_vnum[0] = value[0];
    mud_stat.unique_vnum[1] = value[1];

    return TRUE;
}

bool game_edit_wizlock( CHAR_DATA *ch, char *argument )
{
    mud_stat.wizlock = !mud_stat.wizlock;

    if ( mud_stat.wizlock )
    {
	wiznet( "$N has wizlocked the game.", ch, NULL, 0, 0, 0 );
	send_to_char( "Wizlock set.\n\r", ch );
    } else {
	wiznet( "$N removes wizlock.", ch, NULL, 0, 0, 0 );
	send_to_char( "Wizlock removed.\n\r", ch );
    }

    return TRUE;
}

bool command_edit_name( CHAR_DATA *ch, char *argument )
{
    return TRUE;
}

bool command_edit_show( CHAR_DATA *ch, char *argument )
{
    BUFFER *final = new_buf( );
    char buf[MAX_STRING_LENGTH];
    int pos;

    EDIT_TABLE( ch, pos );

    sprintf( buf, "{qNumber:       {s[{t%d{s]\n\r", pos );
    add_buf( final, buf );

    sprintf( buf, "{qName:         {s[{t%s{s]\n\r", cmd_table[pos].name );
    add_buf( final, buf );

//    DO_FUN *            do_fun;

    sprintf( buf, "{qPosition: {s[{t%d{s] [{t%s{s]\n\r",
	cmd_table[pos].position,
	pos_save( cmd_table[pos].position ) );
    add_buf( final, buf );

    sprintf( buf, "{qLevel:        {s[{t%d{s]\n\r", cmd_table[pos].level );
    add_buf( final, buf );

    sprintf( buf, "{qTier:         {s[{t%d{s]\n\r", cmd_table[pos].tier );
    add_buf( final, buf );

    sprintf( buf, "{qLog Value:    {s[{t%d{s]\n\r", cmd_table[pos].log );
    add_buf( final, buf );

    sprintf( buf, "{qShow:         {s[{t%d{s]\n\r", cmd_table[pos].show );
    add_buf( final, buf );

    page_to_char( final->string, ch );
    free_buf( final );

    return FALSE;
}
