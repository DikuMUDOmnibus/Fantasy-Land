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
#include <time.h>
#include "merc.h"
#include "recycle.h"

#define AUCTION_LENGTH	5
#define STORAGE_BONUS	15

DECLARE_DO_FUN( do_clantalk	);
DECLARE_DO_FUN( do_say		);
DECLARE_DO_FUN( do_split	);
DECLARE_DO_FUN( do_yell		);

BUFFER *show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
					bool fShort, bool fShowNothing ) );
void	check_trap		args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );

bool can_loot(CHAR_DATA *ch, OBJ_DATA *obj)
{
    if ( IS_IMMORTAL(ch)
    ||   obj->owner == NULL
    ||   !str_cmp(ch->name,obj->owner)
    ||   (obj->killer != NULL && !str_cmp(obj->killer,ch->name)) )
	return TRUE;

    return FALSE;
}

void set_obj_sockets( CHAR_DATA *ch, OBJ_DATA *obj )
{
    OBJ_MULTI_DATA *mult;
    OBJ_DATA *inObj;
    CHAR_DATA *vch;
    bool found = FALSE;

    if ( !IS_NPC(ch) )
	vch = ch;
    else if ( ch->master != NULL && !IS_NPC(ch->master) )
	vch = ch->master;
    else
	return;

    obj->dropped_by = vch;

    for ( mult = obj->multi_data; mult != NULL; mult = mult->next )
    {
	if ( !str_cmp( vch->name, mult->dropper ) )
	{
	    mult->drop_timer = 50;
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	mult			= new_obj_multi( );
	mult->drop_timer	= 50;
	mult->socket		= str_dup(vch->pcdata->socket);
	mult->dropper		= str_dup(vch->name);
	mult->next		= obj->multi_data;
	obj->multi_data		= mult;
    }

    if (obj->contains != NULL)
    {
	for (inObj = obj->contains; inObj != NULL; inObj = inObj->next_content)
	    set_obj_sockets(vch,inObj);
    }

    return;
}

char * get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container, bool multi_obj )
{
    static char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;

    buf[0] = '\0';

    if ( !IS_IMMORTAL( ch ) )
    {
	if ( !IS_NPC(ch)
	&&   !clan_table[ch->clan].pkill
	&&   obj->pIndexData->vnum == OBJ_VNUM_VOODOO )
	{
	    sprintf( buf, "Non pkill players are not permitted to have voodoo dolls.\n\r" );
	    return buf;
	}

	if ( !can_loot( ch, obj ) )
	{
	    sprintf( buf, "Corpse looting is not permitted.\n\r" );
	    return buf;
	}

	if ( !CAN_WEAR( obj, ITEM_TAKE )
	||   (obj->item_type == ITEM_CORPSE_PC
	&&    str_cmp(obj->owner,ch->name)) )
	{
	    sprintf( buf, "You can't take that.\n\r" );
	    return buf;
	}

	if ( obj->item_type == ITEM_WEAPON
	&&   obj->disarmed_from != NULL
	&&   obj->disarmed_from != ch )
	{
	    if ( IS_NPC(ch)
	    ||   !can_pk(ch,obj->disarmed_from) )
	    {
		sprintf( buf, "You can not take %s, because it belongs to %s, whom you my not attack.\n\r",
		    obj->short_descr, PERS( obj->disarmed_from, ch ) );
		return buf;
	    }
	}

	if ( !check_objpktest( ch, obj ) )
	    return buf;

	if ( !obj_droptest( ch, obj ) )
	{
	    act( "You drop $p as it {Rs{rc{Ra{rl{Rd{rs{x you upon touching it!",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	    act( "$n is {Rs{rc{Ra{rl{Rd{re{Rd{x by $p as $e grabs it!",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	    act( "$p {Dv{wa{Dn{wi{Ds{wh{De{ws in a {Dm{wi{Ds{wt{x.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	    act( "$p {Dv{wa{Dn{wi{Ds{wh{De{ws in a {Dm{wi{Ds{wt{x.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	    if ( obj->in_room != NULL )
		obj_from_room( obj );
	    extract_obj( obj );

	    return buf;
	}
    }

    if ( container == NULL
    ||   container->carried_by == NULL
    ||   container->carried_by != ch )
    {
	if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
	{
	    sprintf( buf, "%s: you can't carry that many items.\n\r", obj->short_descr );
	    return buf;
	}
    }

    if ( get_carry_weight( ch ) + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	sprintf( buf, "%s: you can't carry that much weight.\n\r", obj->short_descr );
	return buf;
    }

    if ( obj->in_room != NULL )
    {
	for ( gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room )
	{
	    if ( gch->on == obj )
	    {
		sprintf( buf, "%s appears to be using %s.\n\r", PERS( gch, ch ), obj->short_descr );
		return buf;
	    }
	}
    }

    if ( container != NULL )
    {
	if ( container->pIndexData->item_type == ITEM_PIT
	&&   get_trust(ch) < obj->level )
	{
	    sprintf( buf, "%s: You are not powerful enough to use it.\n\r", obj->short_descr );
	    return buf;
	}

	if ( container->item_type == ITEM_CORPSE_PC
	&&   str_cmp( container->owner, ch->name ) )
	{
	    sh_int points;

	    if ( IS_OBJ_STAT( obj, ITEM_AQUEST )
	    ||   IS_OBJ_STAT( obj, ITEM_FORGED ) )
		points = 5;

	    else if ( obj->item_type == ITEM_MONEY
		 ||   obj->item_type == ITEM_CONTAINER )
		points = 0;

	    else
		points = 1;

	    if ( container->looted_items + points > 5 )
	    {
		sprintf( buf, "You may only loot 5 item points from %s.\n\r",
		    container->short_descr );
		return buf;
	    }
	    container->looted_items += points;
	}

	if ( obj->item_type != ITEM_MONEY
	&&   ( !multi_obj || !IS_SET( ch->configure, CONFIG_LOOT_COMBINE ) ) )
	{
	    if ( !multi_obj )
		act( "$n gets $p from $P.", ch, obj, container, TO_ROOM,POS_RESTING);
	    sprintf( buf, "You get %s from %s.\n\r", obj->short_descr, container->short_descr );
	}

	obj_from_obj( obj );
    } else {
	if ( obj->item_type != ITEM_MONEY
	&&   ( !multi_obj || !IS_SET( ch->configure, CONFIG_LOOT_COMBINE ) ) )
	{
	    sprintf( buf, "You get %s.\n\r", obj->short_descr );
	    if ( !multi_obj )
		act( "$n gets $p.", ch, obj, container, TO_ROOM,POS_RESTING);
	}

        obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_MONEY )
    {
	char bufp[MAX_STRING_LENGTH];
	char bufg[MAX_STRING_LENGTH];
	char bufs[MAX_STRING_LENGTH];
	char buffer[100];
	int members = 0;

	bufp[0] = '\0';
	bufg[0] = '\0';
	bufs[0] = '\0';

	if ( obj->value[0] > 0 )
	{
	    add_cost( ch, obj->value[0], VALUE_SILVER );
	    sprintf( bufs," {c%d {&si{7lv{&er{x,", obj->value[0] );
	}

	if ( obj->value[1] > 0 )
	{
	    add_cost( ch, obj->value[1], VALUE_GOLD );
	    sprintf( bufg," {c%d {Yg{yol{Yd{x,", obj->value[1] );
	}

	if ( obj->value[2] > 0 )
	{
	    add_cost( ch, obj->value[2], VALUE_PLATINUM );
	    sprintf( bufp," {c%d {8pl{7a{&ti{7n{8um{x,", obj->value[2] );
	}

	sprintf( buf, "You quickly count your loot:%s%s%s",
	    bufp, bufg, bufs );
	buf[strlen(buf)-1] = '\0';
	strcat( buf, ".\n\r" );

	if ( IS_SET( ch->act, PLR_AUTOSPLIT )
	&&   ( container == NULL
	||     container->item_type != ITEM_CORPSE_PC ) )
	{
	    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	    {
		if ( !IS_AFFECTED( gch, AFF_CHARM ) && is_same_group( gch, ch ) )
		    members++;
	    }

	    if ( members > 1
	    &&   ( obj->value[0] > 1 || obj->value[1] || obj->value[2] ) )
	    {
		sprintf( buffer, "%d %d %d",
		    obj->value[0], obj->value[1], obj->value[2] );
		do_split( ch, buffer );
	    }
	}

	extract_obj( obj );
    } else {
	obj->disarmed_from = NULL;
        obj_to_char( obj, ch );

	if ( HAS_TRIGGER_OBJ( obj, TRIG_GET ) )
	    p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_GET );

	if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_GET ) )
	    p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_GET );

	if ( ch->pcdata && ch->pcdata->questobj != 0
	&&   ch->pcdata->questobj == obj->pIndexData->vnum )
	    strcat( buf, "{RYou have almost completed your {GQUEST{R!\n\rReturn to the questmaster before your time runs out!{x\n\r" );
    }

    return buf;
}

void do_get( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    OBJ_DATA *obj, *obj_next, *container;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    bool found;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"from"))
	argument = one_argument(argument,arg2);

    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Get what?\n\r", ch );
	return;
    }

    if ( ch->stunned)
    {
        send_to_char( "Can't pick up anything stunned!\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    if ( ( obj = get_obj_list( ch, arg1, ch->in_room->contents ) ) == NULL )
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR, POS_RESTING );
	    else
		send_to_char( get_obj( ch, obj, NULL, FALSE ), ch );
	} else {
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    final = new_buf();

	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    add_buf( final, get_obj( ch, obj, NULL, TRUE ) );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    add_buf( final, "I see nothing here.\n\r" );
		else
		{
		    sprintf( buf, "I see no %s here.\n\r", &arg1[4] );
		    add_buf( final, buf );
		}
	    } else {
		if ( IS_SET( ch->configure, CONFIG_LOOT_COMBINE ) )
		    send_to_char( "You grab everything from within the room.\n\r", ch );
		act("$n quickly grabs everything from within the room.",ch,NULL,NULL,TO_ROOM,POS_RESTING);
	    }

	    page_to_char( final->string, ch );
	    free_buf( final );
	}
    } else {
	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, NULL, arg2 ) ) == NULL )
	{
	    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR,POS_RESTING);
	    return;
	}

	switch ( container->item_type )
	{
	    default:
		send_to_char( "That's not a container.\n\r", ch );
		return;

	    case ITEM_CONTAINER:
	    case ITEM_PIT:
	    case ITEM_CORPSE_NPC:
		break;

	    case ITEM_CORPSE_PC:
		if ( !can_loot( ch, container ) )
		{
		    send_to_char( "Corpse looting not permitted.\n\r", ch );
		    return;
		}
		break;
	}

	if ( IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, container->name, TO_CHAR,POS_RESTING);
	    return;
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj container' */
	    if ( ( obj = get_obj_list( ch, arg1, container->contains ) ) == NULL )
		act( "I see nothing like that in the $T.", ch, NULL, arg2, TO_CHAR,POS_RESTING);
	    else
		send_to_char( get_obj( ch, obj, container, FALSE ), ch );
	} else {
	    /* 'get all container' or 'get all.obj container' */
	    found = FALSE;
	    final = new_buf();

	    for ( obj = container->contains; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;

		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    if ( container->pIndexData->item_type == ITEM_PIT
		    &&   !IS_IMMORTAL(ch))
		    {
			send_to_char("Don't be so greedy!\n\r",ch);
			return;
		    }
		    add_buf( final, get_obj( ch, obj, container, TRUE ) );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		{
		    sprintf( buf, "I see nothing in %s.\n\r", container->short_descr );
		    add_buf( final, buf );
		} else {
		    sprintf( buf, "I see nothing like that in %s.\n\r", container->short_descr );
		    add_buf( final, buf );
		}
	    } else {
		if ( IS_SET( ch->configure, CONFIG_LOOT_COMBINE ) )
		    act( "You grab everything from $p.", ch, container, NULL, TO_CHAR, POS_DEAD );
		act("$n quickly grabs everything from $p.",ch,container,NULL,TO_ROOM,POS_RESTING);
	    }

	    page_to_char( final->string, ch );
	    free_buf( final );
	}
    }
    return;
}

void do_donate( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    bool found = FALSE;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Donate what?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "all" ) || !str_prefix( "all.", arg1 ) )
    {
        send_to_char( "One item at a time please.\n\r", ch );
        return;
    }

    if (ch->in_room == NULL)
    {
	send_to_char("Error, NULL room.\n\r",ch);
	return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    ||   IS_SET(ch->in_room->room_flags, ROOM_WAR) )
    {
	send_to_char("Don't even think about it, asshole.\n\r",ch);
	return;
    }

    for ( container = object_list; container != NULL; container = container->next )
    {
	if ( container->pIndexData->item_type != ITEM_PIT
	|| container->pIndexData->vnum != OBJ_VNUM_PIT )
	    continue;

	found = TRUE;
	break;
    }

    if (!found)
    {
        send_to_char( "I can't seem to find the donation pit!\n\r", ch);
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
    {
	send_to_char("You have a feeling noone's going to want that.\n\r",ch);
	return;
    }

    if ( obj->item_type == ITEM_TRASH )
    {
	send_to_char( "The donation pit is not a trash can.\n\r", ch );
	return;
    }

    obj_from_char( obj );

    set_obj_sockets(ch,obj);

    obj_to_obj( obj, container );
    act( "$p glows {Mpurple{x, then disappears from $n's inventory.", ch, obj, container, TO_ROOM,POS_RESTING);
    act( "$p glows {Mpurple{x, then disappears..", ch, obj, container, TO_CHAR,POS_RESTING);
    return;
}

void do_cdonate( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    bool found = FALSE;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Clan Donate what?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "all" ) || !str_prefix( "all.", arg1 ) )
    {
        send_to_char( "One item at a time please.\n\r", ch );
        return;
    }

    if (ch->in_room == NULL)
    {
	send_to_char("Error, NULL room.\n\r",ch);
	return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    ||   IS_SET(ch->in_room->room_flags, ROOM_WAR) )
    {
	send_to_char("Don't even think about it, asshole.\n\r",ch);
	return;
    }

    for ( container = object_list; container != NULL; container = container->next )
    {
        if ( container->pIndexData->item_type != ITEM_PIT
	||   container->pIndexData->vnum != clan_table[ch->clan].pit )
            continue;

        found = TRUE;
	break;
    }

    if (!found)
    {
        send_to_char( "I can't seem to find the donation pit!\n\r", ch);
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
    {
	send_to_char("You have a feeling noone's going to want that.\n\r",ch);
	return;
    }

    if ( obj->item_type == ITEM_TRASH )
    {
	send_to_char( "The donation pit is not a trash can.\n\r", ch );
	return;
    }

    obj_from_char( obj );

    set_obj_sockets(ch,obj);

    obj_to_obj( obj, container );
    act( "$p glows {Bblue{x, then disappears from $n's inventory.", ch, obj, container, TO_ROOM,POS_RESTING);
    act( "$p glows {Bblue{x, then disappears..", ch, obj, container, TO_CHAR,POS_RESTING);
    return;
}

void do_put( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    int count;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"in") || !str_cmp(arg2,"on"))
	argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Put what in what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    if ( ( container = get_obj_here( ch, NULL, arg2 ) ) == NULL )
    {
	act( "I see no $T here.", ch, NULL, arg2, TO_CHAR,POS_RESTING);
	return;
    }

    if ( ( container->item_type != ITEM_CONTAINER )
    &&   ( container->item_type != ITEM_CORPSE_PC )
    &&   ( container->item_type != ITEM_CORPSE_NPC )
    &&   ( container->item_type != ITEM_PIT ) )
    {
	send_to_char( "That's not a container.\n\r", ch );
	return;
    }

    if ( container->item_type == ITEM_CORPSE_PC
    &&   !can_loot(ch,container) )
    {
	send_to_char("Corpse looting is not permitted, why put stuff there?\n\r",ch);
	return;
    }

    if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
	act( "The $d is closed.", ch, NULL, container->name, TO_CHAR,POS_RESTING);
	return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	/* 'put obj container' */
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( obj == container )
	{
	    send_to_char( "You can't fold it into itself.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

	if ( !can_loot(ch,container) )
	{
	    act("You can't loot $P, so why put $p in it?",
		ch,obj,container,TO_CHAR,POS_RESTING);
	    return;
	}

	if ( ( obj->pIndexData->vnum == OBJ_VNUM_CUBIC )
	   && ( container->pIndexData->vnum != OBJ_VNUM_CPOUCH ) )
	{
	    send_to_char( "Cubic Zirconium may only be placed in silk gem pouches.\n\r", ch);
	    return;
	}

	if ( ( obj->item_type == ITEM_GEM )
	   && ( obj->pIndexData->vnum != OBJ_VNUM_CUBIC )
	   && ( container->pIndexData->vnum != OBJ_VNUM_DPOUCH ) )
	{
	    send_to_char( "Gems may only be placed in leather gem pouches.\n\r", ch);
	    return;
	}

	if ( ( container->pIndexData->vnum == OBJ_VNUM_DPOUCH )
	   && ( obj->item_type != ITEM_GEM ) )
	{
	    send_to_char( "Only gems may be placed in leather gem pouches.\n\r", ch);
	    return;
	}

	if ( ( container->pIndexData->vnum == OBJ_VNUM_CPOUCH )
	   && ( obj->pIndexData->vnum != OBJ_VNUM_CUBIC ) )
	{
	    send_to_char( "Only cubic zirconium may be placed in silk gem pouches.\n\r", ch);
	    return;
	}

	if ( container->item_type != ITEM_CORPSE_PC )
	{
	    if (get_obj_weight( obj ) + get_true_weight( container )
		> (container->value[0] * 10)
	    ||  get_obj_weight(obj) > (container->value[3] * 10))
	    {
		send_to_char( "It won't fit.\n\r", ch );
		return;
	    }
	}

	obj_from_char( obj );

	set_obj_sockets(ch,obj);

	obj_to_obj( obj, container );

	if (IS_SET(container->value[1],CONT_PUT_ON))
	{
	    act("$n puts $p on $P.",ch,obj,container, TO_ROOM,POS_RESTING);
	    act("You put $p on $P.",ch,obj,container, TO_CHAR,POS_RESTING);
	}
	else
	{
	    act( "$n puts $p in $P.", ch, obj, container, TO_ROOM,POS_RESTING);
	    act( "You put $p in $P.", ch, obj, container, TO_CHAR,POS_RESTING);
	}
    }
    else
    {
	/* 'put all container' or 'put all.obj container' */
	/* check for gem or cubic pouches first */
	if (container->pIndexData->vnum == OBJ_VNUM_DPOUCH)
	{
	    count = 0;
	    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;

		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   obj->item_type == ITEM_GEM
		&&   obj->pIndexData->vnum != OBJ_VNUM_CUBIC
		&&   can_see_obj( ch, obj )
		&&   WEIGHT_MULT(obj) == 100
		&&   obj->wear_loc == WEAR_NONE
		&&   obj != container
		&&   can_drop_obj( ch, obj ) )
		{
		    if ( container->item_type != ITEM_CORPSE_PC )
		    {
			if ( get_obj_weight( obj ) + get_true_weight( container )
			    > (container->value[0] * 10)
			||   get_obj_weight(obj) > (container->value[3] * 10) )
			    continue;
		    }

		    obj_from_char( obj );

		    set_obj_sockets(ch,obj);

		    obj_to_obj( obj, container );
		    count++;
		}
	    }
	    if (count != 0)
	    {
		sprintf( buf, "You put %d gems in %s.\n\r", count, container->short_descr );
		send_to_char( buf, ch);
		sprintf( buf, "$n puts %d gems in %s.\n\r", count, container->short_descr );
		act( buf, ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    } else
	    {
		send_to_char( "You are not carrying any gems.\n\r", ch );
	    }
	    return;
	}

	if (container->pIndexData->vnum == OBJ_VNUM_CPOUCH)
	{
	    count = 0;
	    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;

		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   obj->pIndexData->vnum == OBJ_VNUM_CUBIC
		&&   can_see_obj( ch, obj )
		&&   WEIGHT_MULT(obj) == 100
		&&   obj->wear_loc == WEAR_NONE
		&&   obj != container
		&&   can_drop_obj( ch, obj ) )
		{
		    if ( container->item_type != ITEM_CORPSE_PC )
		    {
			if ( get_obj_weight( obj ) + get_true_weight( container )
			    > (container->value[0] * 10)
			||   get_obj_weight(obj) > (container->value[3] * 10) )
			    continue;
		    }


		    set_obj_sockets(ch,obj);

		    obj_from_char( obj );
		    obj_to_obj( obj, container );
		    count++;
		}
	    }
	    if (count != 0)
	    {
		sprintf( buf, "You put %d cubic zirconiums in %s.\n\r", count, container->short_descr );
		send_to_char( buf, ch);
		sprintf( buf, "$n puts %d cubic zirconiums in %s.\n\r", count, container->short_descr );
		act( buf, ch, NULL, NULL, TO_ROOM,POS_RESTING);
	    } else
	    {
		send_to_char( "You are not carrying any cubic zirconiums.\n\r", ch );
	    }
	    return;
	}
	count = 0;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   WEIGHT_MULT(obj) == 100
	    &&   obj->wear_loc == WEAR_NONE
	    &&   obj != container
	    &&   obj->item_type != ITEM_GEM
	    &&   obj->pIndexData->vnum != OBJ_VNUM_CUBIC
	    &&   can_drop_obj( ch, obj ) )
	    {
		if ( container->item_type != ITEM_CORPSE_PC )
		{
		    if ( get_obj_weight( obj ) + get_true_weight( container )
			> (container->value[0] * 10)
		    ||   get_obj_weight(obj) > (container->value[3] * 10) )
			continue;
		}

		obj_from_char( obj );

		set_obj_sockets(ch,obj);

		obj_to_obj( obj, container );
		count++;
	    }
	}
	if (count != 0)
	{
	    if (IS_SET(container->value[1],CONT_PUT_ON))
	    {
		act("$n puts some things on $P.",ch,NULL,container, TO_ROOM,POS_RESTING);
	    } else
	    {
		act("$n puts some things in $P.",ch,NULL,container, TO_ROOM,POS_RESTING);
	    }
	    sprintf(buf, "You put some things in %s.\n\r",container->short_descr);
	    send_to_char(buf,ch);
	}
    }

    return;
}

void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *t_obj,*n_obj;
    OBJ_DATA *obj_next;
    bool found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drop what?\n\r", ch );
	return;
    }

    if ( is_number( arg ) )
    {
	/* 'drop NNNN coins' */
	int amount, platinum = 0, gold = 0, silver = 0;

	amount   = atoi(arg);
	argument = one_argument( argument, arg );
	if ( amount <= 0
	|| ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) &&
	     str_cmp( arg, "gold"  ) && str_cmp( arg, "silver")
	&&   str_cmp( arg, "platinum") ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	if (amount > 50000)
	{
	    send_to_char("You can't drop that much at once.\n\r",ch);
	    return;
	}

	if ( !str_cmp( arg, "coins") || !str_cmp(arg,"coin")
	||   !str_cmp( arg, "silver"))
	{
	    if ((ch->silver+(100*ch->gold)+(10000*ch->platinum)) < amount)
	    {
		send_to_char("You don't have that much silver.\n\r",ch);
		return;
	    }

	    deduct_cost(ch,amount,VALUE_SILVER);
	    silver = amount;
	}

	else if ( !str_cmp( arg, "gold") )
	{
	    if ((ch->silver+(100*ch->gold)+(10000*ch->platinum)) < amount*100)
	    {
		send_to_char("You don't have that much gold.\n\r",ch);
		return;
	    }

	    deduct_cost(ch,amount,VALUE_GOLD);
  	    gold = amount;
	}

	else if ( !str_cmp( arg, "platinum" ) )
	{
	    if ((ch->silver+(100*ch->gold)+(10000*ch->platinum)) < amount*10000)
	    {
		send_to_char("You don't have that much platinum.\n\r",ch);
		return;
	    }

	    deduct_cost(ch,amount,VALUE_PLATINUM);
  	    platinum = amount;
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    switch ( obj->pIndexData->vnum )
	    {
	    case OBJ_VNUM_SILVER_ONE:
		silver += 1;
		extract_obj(obj);
		break;

	    case OBJ_VNUM_GOLD_ONE:
		gold += 1;
		extract_obj( obj );
		break;

	    case OBJ_VNUM_PLATINUM_ONE:
		platinum += 1;
		extract_obj( obj );
		break;

	    case OBJ_VNUM_SILVER_SOME:
		silver += obj->value[0];
		extract_obj(obj);
		break;

	    case OBJ_VNUM_GOLD_SOME:
		gold += obj->value[1];
		extract_obj( obj );
		break;

	    case OBJ_VNUM_PLATINUM_SOME:
		platinum += obj->value[2];
		extract_obj( obj );
		break;

	    case OBJ_VNUM_COINS:
		silver += obj->value[0];
		gold += obj->value[1];
		platinum += obj->value[2];
		extract_obj(obj);
		break;
	    }
	}

	while (silver >= 100)
	{
	    gold++;
	    silver -= 100;
	}
	while (gold >= 100)
	{
	    platinum++;
	    gold -= 100;
	}
	if (platinum > 50000)
	{
	    platinum = 50000;
	}
	obj = create_money( platinum, gold, silver );

	set_obj_sockets(ch,obj);
	set_arena_obj( ch, obj );
	obj_to_room( obj, ch->in_room );
	act( "$n drops some money.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	send_to_char( "OK.\n\r", ch );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

	obj_from_char( obj );

	set_obj_sockets(ch,obj);
	set_arena_obj( ch, obj );
	obj_to_room( obj, ch->in_room );

	act( "$n drops $p.", ch, obj, NULL, TO_ROOM,POS_RESTING);
	act( "You drop $p.", ch, obj, NULL, TO_CHAR,POS_RESTING);

	if ( HAS_TRIGGER_OBJ( obj, TRIG_DROP ) )
	    p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_DROP );
	if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_DROP ) )
	    p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_DROP );

	if ( obj && IS_OBJ_STAT(obj,ITEM_MELT_DROP))
	{
            if (obj->contains)
            {
                for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
                {
                    n_obj = t_obj->next_content;
                    obj_from_obj(t_obj);
                    t_obj->arena_number = obj->arena_number;
                    if (obj->in_room != NULL)
                        obj_to_room(t_obj,obj->in_room);
                    else if (obj->carried_by != NULL)
                        obj_to_room(t_obj,obj->carried_by->in_room);
                    else
                    {
                        extract_obj(t_obj);
                        continue;
                    }
                }
            }

	    act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    extract_obj(obj);
	}
    } else {
	BUFFER *final = new_buf();
	char buf[MAX_STRING_LENGTH];

	/* 'drop all' or 'drop all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj ) )
	    {
		found = TRUE;
		obj_from_char( obj );

		set_obj_sockets(ch,obj);
		set_arena_obj( ch, obj );
		obj_to_room( obj, ch->in_room );

		if ( !IS_SET( ch->configure, CONFIG_LOOT_COMBINE ) )
		{
		    sprintf( buf, "You drop %s.\n\r", obj->short_descr );
		    add_buf( final, buf );
//		    act( "$n drops $p.", ch, obj, NULL, TO_ROOM, POS_RESTING );
		}

		if ( HAS_TRIGGER_OBJ( obj, TRIG_DROP ) )
		    p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_DROP );
		if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_DROP ) )
		    p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_DROP );

		if ( obj && IS_OBJ_STAT(obj,ITEM_MELT_DROP))
        	{
		    if (obj->contains)
		    {
			for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
	                {
	                    n_obj = t_obj->next_content;
         	            obj_from_obj(t_obj);
	                    t_obj->arena_number = obj->arena_number;
	                    if (obj->in_room != NULL)
	                        obj_to_room(t_obj,obj->in_room);
	                    else if (obj->carried_by != NULL)
	                        obj_to_room(t_obj,obj->carried_by->in_room);
	                    else
	       		    {
	                	extract_obj(t_obj);
	                	continue;
			    }
		        }
		    }
		    sprintf( buf, "%s disolves into smoke.\n\r", obj->short_descr );
		    add_buf( final, buf );
            	    extract_obj(obj);
        	}
	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		add_buf( final, "You are not carrying anything.\n\r" );
	    else
	    {
		sprintf( buf, "You are not carrying any %s.\n\r", &arg[4] );
		add_buf( final, buf );
	    }
	} else {
	    if ( IS_SET( ch->configure, CONFIG_LOOT_COMBINE ) )
		add_buf( final, "You drop some things.\n\r" );
	    act( "$n drops some things.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	}

	page_to_char( final->string, ch );
	free_buf( final );
    }

    return;
}

void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Give what to whom?\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) )
    {
	/* 'give NNNN coins victim' */
	int amount, cost;
	long fullamount;
	int silver = 0, gold = 0, platinum = 0;

	amount   = atoi(arg1);
	if ( amount <= 0
	|| ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" )
	&&   str_cmp( arg2, "gold"  ) && str_cmp( arg2, "silver")
	&&   str_cmp( arg2, "platinum") ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	if (!str_cmp(arg2,"gold") )
	{
	    gold = amount;
	    fullamount = amount*100;
	}
	else if (!str_cmp(arg2,"platinum"))
	{
	    platinum = amount;
	    fullamount = amount*10000;
	}
	else
	{
	    silver = amount;
	    fullamount = amount;
	}

	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Give what to whom?\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if (ch->silver + (ch->gold * 100) + (ch->platinum * 10000) < fullamount )
	{
	    send_to_char( "You haven't got that much.\n\r", ch );
	    return;
	}

	if (amount > 50000)
	{
	    send_to_char( "You can't give that much all at once.\n\r", ch );
	    return;
	}

	if (!check_pktest(ch,victim))
	    return;

	cost = 0;
	if (silver != 0)
	{
	    cost = silver;
	    deduct_cost(ch,cost,VALUE_SILVER);
	    add_cost(victim,cost,VALUE_SILVER);
	}
	else if (gold != 0)
	{
	    cost = gold;
	    deduct_cost(ch,cost,VALUE_GOLD);
	    add_cost(victim,cost,VALUE_GOLD);
	}
	else
	{
	    cost = platinum;
	    deduct_cost(ch,cost,VALUE_PLATINUM);
	    add_cost(victim,cost,VALUE_PLATINUM);
	}
	act( "$n gives $N some money.",  ch, NULL, victim, TO_NOTVICT,POS_RESTING);
	if (platinum != 0)
	{
	    sprintf(buf,"$n gives you %d platinum.",platinum);
	    act( buf, ch, NULL, victim, TO_VICT,POS_RESTING);
	    sprintf(buf,"You give $N %d platinum.",platinum);
	    act( buf, ch, NULL, victim, TO_CHAR,POS_RESTING);
	}
	else if (gold != 0)
	{
	    sprintf(buf,"$n gives you %d gold.",gold);
	    act( buf, ch, NULL, victim, TO_VICT,POS_RESTING);
	    sprintf(buf,"You give $N %d gold.",gold);
	    act( buf, ch, NULL, victim, TO_CHAR,POS_RESTING);
	}
	else
	{
	    sprintf(buf,"$n gives you %d silver.",silver);
	    act( buf, ch, NULL, victim, TO_VICT,POS_RESTING);
	    sprintf(buf,"You give $N %d silver.",silver);
	    act( buf, ch, NULL, victim, TO_CHAR,POS_RESTING);
	}

    	/*
	 * Bribe trigger
	 */
	if ( IS_NPC(victim) && HAS_TRIGGER_MOB( victim, TRIG_BRIBE ) )
	    p_bribe_trigger( victim, ch, silver ? amount : amount * 100 );

	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    {
	act("$N tells you '{aSorry, you'll have to sell that{x'.",
	    ch,NULL,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if ( IS_NPC(victim)
    &&   victim->spec_fun == spec_lookup( "spec_stringer" ) )
    {
	act("You give $p to $N.",ch,obj,victim,TO_CHAR,POS_RESTING);
	if ( obj->pIndexData->vnum == 9
	||   obj->pIndexData->vnum == 19 )
	{
	    act("$N tells you '{UYou can pay me after my services have been performed{x'",
		ch,NULL,victim,TO_CHAR,POS_RESTING);
	}
	act("$N gives you $p.",ch,obj,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( obj->pIndexData->vnum == OBJ_VNUM_VOODOO
    &&   ch->level <= LEVEL_HERO )
    {
	send_to_char( "You can't give voodoo dolls.\n\r", ch);
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

    if (IS_NPC( ch)
    &&  ch->pIndexData->vnum >= 5
    &&  ch->pIndexData->vnum <= 20 )
    {
        obj_from_char( obj );
        obj_to_char( obj, victim );
        act( "$N gets $p.", ch, obj, victim, TO_NOTVICT,POS_RESTING);
        act( "You get $p.",   ch, obj, victim, TO_VICT,POS_RESTING);
        act( "You give $p to $N.", ch, obj, victim, TO_CHAR,POS_RESTING);
        return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act( "$N can't see it.", ch, NULL, victim, TO_CHAR,POS_RESTING);
	return;
    }

    if (!check_pktest(ch,victim))
	return;

    if (!obj_droptest(victim,obj))
    {
	act("You drop $p as it {Rs{rc{Ra{rl{Rd{rs{x you upon touching it!",victim,obj,NULL,TO_CHAR,POS_RESTING);
	act("$n is {Rs{rc{Ra{rl{Rd{re{Rd{x by $p as $e grabs it!",victim,obj,NULL,TO_ROOM,POS_RESTING);
	act("$p {Dv{wa{Dn{wi{Ds{wh{De{ws in a {Dm{wi{Ds{wt{x.",victim,obj,NULL,TO_CHAR,POS_RESTING);
	act("$p {Dv{wa{Dn{wi{Ds{wh{De{ws in a {Dm{wi{Ds{wt{x.",victim,obj,NULL,TO_ROOM,POS_RESTING);
	obj_from_char(obj);
	extract_obj(obj);
	return;
    }

    obj_from_char( obj );

    set_obj_sockets(ch,obj);

    obj_to_char( obj, victim );
    MOBtrigger = FALSE;
    act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT,POS_RESTING);
    act( "$n gives you $p.",   ch, obj, victim, TO_VICT,POS_RESTING);
    act( "You give $p to $N.", ch, obj, victim, TO_CHAR,POS_RESTING);
    MOBtrigger = TRUE;

    /*
     * Give trigger
     */
    if ( HAS_TRIGGER_OBJ( obj, TRIG_GIVE ) )
	p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_GIVE );

    if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_GIVE ) )
	p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_GIVE );

    if ( IS_NPC(victim) && HAS_TRIGGER_MOB( victim, TRIG_GIVE ) )
	p_give_trigger( victim, NULL, NULL, ch, obj, TRIG_GIVE );

    return;
}

void envenom_weapon( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA af;

    af.where		= TO_WEAPON;
    af.type		= gsn_poison;
    af.level		= ch->level;
    af.dur_type		= DUR_TICKS;
    af.duration		= ch->level/4;
    af.location		= 0;
    af.modifier		= 0;
    af.bitvector	= WEAPON_POISON;
    affect_to_obj(obj,&af);

    act("$n coats $p with deadly venom.",ch,obj,NULL,TO_ROOM,POS_RESTING);
    act("You coat $p with venom.",ch,obj,NULL,TO_CHAR,POS_RESTING);
    check_improve(ch,gsn_envenom,TRUE,3);
}

void do_envenom(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    int skill;

    if ( argument == '\0' )
    {
	send_to_char("Envenom what item?\n\r",ch);
	return;
    }

    obj = get_obj_list(ch,argument,ch->carrying);

    if ( obj == NULL )
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if ( (skill = get_skill(ch,gsn_envenom)) < 1 )
    {
	send_to_char("Are you crazy? You'd poison yourself!\n\r",ch);
	return;
    }

    WAIT_STATE(ch,skill_table[gsn_envenom].beats);

    if ( obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON )
    {
	if ( IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF) )
	{
	    act("You fail to poison $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	}

	if ( number_percent() < skill )
	{
	    act("$n treats $p with deadly poison.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    act("You treat $p with deadly poison.",ch,obj,NULL,TO_CHAR,POS_RESTING);

	    if ( !obj->value[3] )
	    {
		obj->value[3] = 1;
		check_improve(ch,gsn_envenom,TRUE,4);
	    }
	    return;
	}

	act("You fail to poison $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);

	if (!obj->value[3])
	    check_improve(ch,gsn_envenom,FALSE,4);

	return;
    }

    if ( obj->item_type == ITEM_WEAPON )
    {
	if ( IS_WEAPON_STAT(obj,WEAPON_POISON) )
	{
	    act("$p is already envenomed.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    return;
	}

	if ( number_percent() < skill )
	{
	    envenom_weapon( ch, obj );
	    return;
        } else {
	    act("You fail to envenom $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    check_improve(ch,gsn_envenom,FALSE,3);
	    return;
	}
    }

    act("You can't poison $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
    return;
}

void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    found = FALSE;
    for ( fountain = ch->in_room->contents; fountain != NULL;
	fountain = fountain->next_content )
    {
	if ( fountain->item_type == ITEM_FOUNTAIN )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char( "There is no fountain here!\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "You can't fill that.\n\r", ch );
	return;
    }

    if ( obj->value[1] != 0 && obj->value[2] != fountain->value[2] )
    {
	send_to_char( "There is already another liquid in it.\n\r", ch );
	return;
    }

    if ( obj->value[1] >= obj->value[0] )
    {
	send_to_char( "Your container is full.\n\r", ch );
	return;
    }

    if ( !strcmp(liq_table[fountain->value[2]].liq_name, "blood"))
    {
	sprintf(buf,"You get some %s from $P.",
	    liq_table[fountain->value[2]].liq_name);
	act( buf, ch, obj, fountain, TO_CHAR,POS_RESTING);
	sprintf(buf,"$n gets some %s from $P.",
	    liq_table[fountain->value[2]].liq_name);
	act( buf, ch, obj, fountain, TO_ROOM,POS_RESTING);
	obj->value[2] = fountain->value[2];
	obj->value[1]++;
	extract_obj( fountain );
	return;
    }

    sprintf(buf,"You fill $p with %s from $P.",
	liq_table[fountain->value[2]].liq_name);
    act( buf, ch, obj,fountain, TO_CHAR,POS_RESTING);
    sprintf(buf,"$n fills $p with %s from $P.",
	liq_table[fountain->value[2]].liq_name);
    act(buf,ch,obj,fountain,TO_ROOM,POS_RESTING);
    obj->value[2] = fountain->value[2];
    obj->value[1] = obj->value[0];
    return;
}

void do_pour (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
    OBJ_DATA *out, *in;
    CHAR_DATA *vch = NULL;
    int amount;

    argument = one_argument(argument,arg);

    if (arg[0] == '\0' || argument[0] == '\0')
    {
	send_to_char("Pour what into what?\n\r",ch);
	return;
    }


    if ((out = get_obj_carry(ch,arg)) == NULL)
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if (out->item_type != ITEM_DRINK_CON)
    {
	send_to_char("That's not a drink container.\n\r",ch);
	return;
    }

    if (!str_cmp(argument,"out"))
    {
	if (out->value[1] == 0)
	{
	    send_to_char("It's already empty.\n\r",ch);
	    return;
	}

	out->value[1] = 0;
	out->value[3] = 0;
	sprintf(buf,"You invert $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,out,NULL,TO_CHAR,POS_RESTING);

	sprintf(buf,"$n inverts $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,out,NULL,TO_ROOM,POS_RESTING);
	return;
    }

    if ((in = get_obj_here(ch,NULL,argument)) == NULL)
    {
	vch = get_char_room( ch, NULL, argument );

	if (vch == NULL)
	{
	    send_to_char("Pour into what?\n\r",ch);
	    return;
	}

	in = get_eq_char(vch,WEAR_HOLD);

	if (in == NULL)
	{
	    send_to_char("They aren't holding anything.",ch);
 	    return;
	}
    }

    if (in->item_type != ITEM_DRINK_CON)
    {
	send_to_char("You can only pour into other drink containers.\n\r",ch);
	return;
    }

    if (in == out)
    {
	send_to_char("You cannot change the laws of physics!\n\r",ch);
	return;
    }

    if (in->value[1] != 0 && in->value[2] != out->value[2])
    {
	send_to_char("They don't hold the same liquid.\n\r",ch);
	return;
    }

    if (out->value[1] == 0)
    {
	act("There's nothing in $p to pour.",ch,out,NULL,TO_CHAR,POS_RESTING);
	return;
    }

    if (in->value[1] >= in->value[0])
    {
	act("$p is already filled to the top.",ch,in,NULL,TO_CHAR,POS_RESTING);
	return;
    }

    amount = UMIN(out->value[1],in->value[0] - in->value[1]);

    in->value[1] += amount;
    out->value[1] -= amount;
    in->value[2] = out->value[2];

    if (vch == NULL)
    {
    	sprintf(buf,"You pour %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
    	act(buf,ch,out,in,TO_CHAR,POS_RESTING);
    	sprintf(buf,"$n pours %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
    	act(buf,ch,out,in,TO_ROOM,POS_RESTING);
    }
    else
    {
        sprintf(buf,"You pour some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_CHAR,POS_RESTING);
	sprintf(buf,"$n pours you some %s.",
	    liq_table[out->value[2]].liq_name);
	act(buf,ch,NULL,vch,TO_VICT,POS_RESTING);
        sprintf(buf,"$n pours some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_NOTVICT,POS_RESTING);

    }
}

void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
	    if ( obj->item_type == ITEM_FOUNTAIN )
		break;
	}

	if ( obj == NULL )
	{
	    send_to_char( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, NULL, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
    {
	send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
	return;
    }

    switch ( obj->item_type )
    {
    default:
	send_to_char( "You can't drink from that.\n\r", ch );
	return;

    case ITEM_FOUNTAIN:
        if ( ( liquid = obj->value[2] )  < 0 )
        {
            bug( "Do_drink: bad liquid number %d.", liquid );
            liquid = obj->value[2] = 0;
        }
	amount = liq_table[liquid].liq_affect[4] * 3;
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "It is already empty.\n\r", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] )  < 0 )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

        amount = liq_table[liquid].liq_affect[4];
        amount = UMIN(amount, obj->value[1]);
	break;
     }
    if (!IS_NPC(ch) && !IS_IMMORTAL(ch)
    &&  ch->pcdata->condition[COND_FULL] > 45)
    {
	send_to_char("You're too full to drink more.\n\r",ch);
	return;
    }

    act( "$n drinks $T from $p.",
	ch, obj, liq_table[liquid].liq_name, TO_ROOM,POS_RESTING);
    act( "You drink $T from $p.",
	ch, obj, liq_table[liquid].liq_name, TO_CHAR,POS_RESTING);

    gain_condition( ch, COND_DRUNK,
	amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36 );
    gain_condition( ch, COND_FULL,
	amount * liq_table[liquid].liq_affect[COND_FULL] / 4 );
    gain_condition( ch, COND_THIRST,
	amount * liq_table[liquid].liq_affect[COND_THIRST] / 10 );
    gain_condition(ch, COND_HUNGER,
	amount * liq_table[liquid].liq_affect[COND_HUNGER] / 2 );

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	send_to_char( "You feel drunk.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
	send_to_char( "You are full.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
	send_to_char( "Your thirst is quenched.\n\r", ch );

    if ( !strcmp( liq_table[liquid].liq_name, "blood" )
    &&   !strcmp( race_table[ch->race].name, "vampire" ) )
    {
	ch->hit += ch->max_hit/20;
	ch->hit = UMIN( ch->hit, ch->max_hit );
	ch->mana += ch->max_mana/15;
	ch->mana = UMIN( ch->mana, ch->max_mana );
	ch->move += ch->max_move/15;
	ch->move = UMIN( ch->move, ch->max_move );
    }

    if ( obj->value[3] != 0 )
    {
	/* The drink was poisoned ! */
	AFFECT_DATA af;

	act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM,POS_RESTING);
	send_to_char( "You choke and gag.\n\r", ch );
	af.where     = TO_AFFECTS;
	af.type      = gsn_poison;
	af.level     = amount;
	af.dur_type  = DUR_TICKS;
	af.duration  = 3 * amount;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_POISON;
	affect_join( ch, &af );
    }

    if (obj->value[0] > 0)
        obj->value[1] -= amount;

    switch ( obj->item_type )
    {
    default:
	send_to_char( "You can't drink from that.\n\r", ch );
	return;

    case ITEM_FOUNTAIN:
	if ( obj->value[1] <= 0 && obj->value[0] > 0 )
	    extract_obj(obj);
	break;

    case ITEM_DRINK_CON:
	break;
     }

    return;
}

void do_restring( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *diamond = NULL;
    OBJ_DATA *obj;
    CHAR_DATA *stringer;
    bool found = FALSE;

    if (IS_NPC(ch))
	return;

    argument = one_argument( argument, arg );

    for ( stringer = ch->in_room->people; stringer != NULL; stringer = stringer->next_in_room )
    {
        if (!IS_NPC(stringer))
            continue;
        if (stringer->spec_fun == spec_lookup( "spec_stringer" ))
	{
            found = TRUE;
            break;
	}
    }

    if (!found)
    {
        send_to_char("You can't find the stringer mob!\n\r",ch);
        return;
    }

    if (!str_cmp(arg,"confirm"))
    {
	if ( !ch->pcdata->confirm_restring
	||   ch->pcdata->restring_item == NULL )
	{
	    send_to_char("You have no restring to confirm.\n\r",ch);
	    return;
	}
	send_to_char("You have confirmed your new restring.\n\r",ch);
	ch->pcdata->confirm_restring = FALSE;
	ch->pcdata->restring_item = NULL;
	return;
    }

    if (!ch->pcdata->confirm_restring)
    {
	found = FALSE;

	for ( diamond = ch->carrying; diamond != NULL; diamond = diamond->next_content )
	{
            if ( diamond->pIndexData->vnum == 1207 )
            {
        	found = TRUE;
        	break;
	    }
	}

	if (!found)
	{
	    act("$N says '{SI charge one {rr{Re{rs{Rt{rr{Ri{rn{Rg {Wd{wi{Wa{wm{Wo{wn{Wd{S for my services.{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
	    act("$N says '{SCome back when you have one ready and waiting!{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
	    return;
	}
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	act("$N says '{SType: \"restring <obj name> <new obj look>\".{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act("$N says '{SI would love to do that, but you don't seem to have that object.{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
	return;
    }

    if (ch->pcdata->confirm_restring && ch->pcdata->restring_item != obj)
    {
	act("$N says '{SNo freebies dumbass!!{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
	act("$N says '{SI will only restring {x$p{S for free!{x'",ch,ch->pcdata->restring_item,stringer,TO_CHAR,POS_DEAD);
	act("$N says '{SIf you wish to restring a different item, please confirm your first.{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	act("$N says '{SIt would sure help if you removed it first.{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
	return;
    }

    if (!ch->pcdata->confirm_restring)
    {
	obj_from_char(diamond);
	extract_obj(diamond);
    }

    smash_tilde( argument );
    sprintf(buf, "%s{x", argument );
    act("You give $p to $N.", ch, obj, stringer, TO_CHAR,POS_RESTING);
    act("$n gives $p to $N.", ch, obj, stringer, TO_NOTVICT,POS_RESTING);
    free_string( obj->short_descr );
    obj->short_descr = str_dup( buf );
    act("$N gives $p to you.", ch, obj, stringer, TO_CHAR,POS_RESTING);
    act("$N gives $p to $n.", ch, obj, stringer, TO_NOTVICT,POS_RESTING);
    act("$N says '{SIf you fucked up you can fix it for free.{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
    act("$N says '{STo confirm this restring type \"restring confirm\".{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
    ch->pcdata->confirm_restring = TRUE;
    ch->pcdata->restring_item = obj;
    return;
}

sh_int count_spells( bool spells[4] )
{
    sh_int count = 0, pos;

    for ( pos = 0; pos < 4; pos++ )
    {
	if ( spells[pos] == TRUE )
	    count++;
    }

    return count;
}

void do_eat( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    sh_int pos;
    bool spells[4];

    if ( argument[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if ( ch->stunned )
    {
	send_to_char( "You're still a little woozy.\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL( ch ) )
    {
	if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
	{
	    send_to_char( "That's not edible.\n\r", ch );
	    return;
	}

	if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] > 40 )
	{
	    send_to_char( "You are too full to eat more.\n\r", ch );
	    return;
	}

	if ( obj->level > ch->level )
	{
	    sprintf( buf, "You must be level %d to eat %s.\n\r",
		obj->level, obj->short_descr );
	    send_to_char( buf, ch );
	    return;
	}
    }

    act( "$n eats $p.", ch, obj, NULL, TO_ROOM, POS_RESTING );
    act( "You eat $p.", ch, obj, NULL, TO_CHAR, POS_RESTING );

    switch ( obj->item_type )
    {
	case ITEM_FOOD:
	    if ( !IS_NPC( ch ) )
	    {
		int condition;

		condition = ch->pcdata->condition[COND_HUNGER];
		gain_condition( ch, COND_FULL, obj->value[0] );
		gain_condition( ch, COND_HUNGER, obj->value[1]);

		if ( condition == 0 && ch->pcdata->condition[COND_HUNGER] > 0 )
		    send_to_char( "You are no longer hungry.\n\r", ch );

		else if ( ch->pcdata->condition[COND_FULL] > 40 )
		    send_to_char( "You are full.\n\r", ch );
	    }

	    if ( obj->value[3] != 0 )
	    {
		/* The food was poisoned! */
		AFFECT_DATA af;

		act( "$n chokes and gags.", ch, 0, 0, TO_ROOM, POS_RESTING );
		send_to_char( "You choke and gag.\n\r", ch );

		af.where	= TO_AFFECTS;
		af.type		= gsn_poison;
		af.level	= obj->value[0];
		af.dur_type	= DUR_TICKS;
		af.duration	= 2 * obj->value[0];
		af.location	= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= AFF_POISON;
		affect_join( ch, &af );
	    }
	    break;

	case ITEM_PILL:
	    if ( arena_flag( ch, ARENA_NO_PILL ) )
		return;

	    for ( pos = 0; pos < 4; pos++ )
		spells[pos] = TRUE;

	    if ( obj->pIndexData->targets != 0 )
	    {
		while ( count_spells( spells ) > obj->pIndexData->targets )
		    spells[number_range( 0, 4 )] = FALSE;
	    }

	    for ( pos = 0; pos < 4; pos++ )
	    {
		if ( obj->value[pos+1] > 0
		&&   spells[pos] == TRUE )
		{
		    if ( number_percent( ) > obj->success )
			send_to_char( "Spell affect failed.\n\r", ch );
		    else
			obj_cast_spell( obj->value[pos+1], obj->value[0], ch, ch, NULL, NULL );
		}
	    }

	    break;
    }

    extract_obj( obj );
    return;
}

bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace, bool show )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace )
	return FALSE;

    if ( IS_SET( obj->extra_flags, ITEM_NOREMOVE )
      && ( ch->level < LEVEL_IMMORTAL ) )
    {
	act( "You can't remove $p.", ch, obj, NULL, TO_CHAR,POS_RESTING);
	return FALSE;
    }

    if ( show )
    {
	act( "$n stops using $p.", ch, obj, NULL, TO_ROOM,POS_RESTING);
	act( "You stop using $p.", ch, obj, NULL, TO_CHAR,POS_RESTING);
    }

    unequip_char( ch, obj );

    if (IS_NPC(ch))
	return TRUE;

    if ((obj->item_type == ITEM_DEMON_STONE) && (ch->pet != NULL)
    && (ch->pet->pIndexData->vnum == MOB_VNUM_DEMON))
    {
        act("$N slowly fades away.",ch,NULL,ch->pet,TO_CHAR,POS_RESTING);
	nuke_pets(ch);
    }

    return TRUE;
}

void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, bool show )
{
    OBJ_DATA *shieldobj;
    char buf[MAX_STRING_LENGTH];

    if ( ch->level < obj->level )
    {
	if ( show )
	{
	    sprintf( buf, "You must be level %d to use this object.\n\r",
		obj->level );
	    send_to_char( buf, ch );

	    act( "$n tries to use $p, but is too inexperienced.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	}

	return;
    }

    if ( obj->size > -1 && obj->size < 6 )
    {
	if ( obj->size > ch->size+1 )
	{
	    act( "$p is too big for you to use!",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	    return;
	}

	if ( obj->size < ch->size-1 )
	{
	    act( "$p is too small for you to use!",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	    return;
	}
    }

    if ( !obj->pIndexData->class_can_use[ch->class] )
    {
	act( "$p was not designed for your class.",
	    ch, obj, NULL, TO_CHAR, POS_RESTING );
	return;
    }

    if ( obj->item_type == ITEM_LIGHT )
    {
	if ( !remove_obj( ch, WEAR_LIGHT, fReplace, TRUE ) )
	    return;

	if ( show )
	{
	    act( "$n lights $p and holds it.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );

	    act( "You light $p and hold it.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	}

	equip_char( ch, obj, WEAR_LIGHT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace, TRUE )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace, TRUE ) )
	    return;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
	{
	    if ( show )
	    {
		act( "$n wears $p on $s left finger.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );

		act( "You wear $p on your left finger.",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
	    }

	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
	{
	    if ( show )
	    {
		act( "$n wears $p on $s right finger.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );

		act( "You wear $p on your right finger.",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
	    }

	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}

	send_to_char( "You already wear two rings.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace, TRUE )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace, TRUE ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
	    if ( show )
	    {
		act( "$n wears $p around $s neck.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );

		act( "You wear $p around your neck.",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
	    }

	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
	    if ( show )
	    {
		act( "$n wears $p around $s neck.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );

		act( "You wear $p around your neck.",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
	    }

	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}

	send_to_char( "You already wear two neck items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
	if ( !remove_obj( ch, WEAR_BODY, fReplace, TRUE ) )
	    return;

	if ( show )
	{
	    act( "$n wears $p on $s torso.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );

	    act( "You wear $p on your torso.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	}

	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BACK ) )
    {
	if ( !remove_obj( ch, WEAR_BACK, fReplace, TRUE ) )
	    return;

	if ( show )
	{
	    act( "$n wears $p on $s back.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );

	    act( "You wear $p on your back.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	}

        equip_char( ch, obj, WEAR_BACK );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FACE ) )
    {
        if ( !remove_obj( ch, WEAR_FACE, fReplace, TRUE ) )
            return;

	if ( show )
	{
	    act( "$n wears $p on $s face.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	    act( "You wear $p on your face.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	}

        equip_char( ch, obj, WEAR_FACE );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
	if ( !remove_obj( ch, WEAR_HEAD, fReplace, TRUE ) )
	    return;

	if ( show )
	{
	    act( "$n wears $p on $s head.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );

	    act( "You wear $p on your head.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	}

	equip_char( ch, obj, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
	if ( !remove_obj( ch, WEAR_LEGS, fReplace, TRUE ) )
	    return;

	if ( show )
	{
	    act( "$n wears $p on $s legs.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );

	    act( "You wear $p on your legs.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	}

	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
	if ( !remove_obj( ch, WEAR_FEET, fReplace, TRUE ) )
	    return;

	if ( show )
	{
	    act( "$n wears $p on $s feet.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );

	    act( "You wear $p on your feet.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	}

	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
	if ( !remove_obj( ch, WEAR_HANDS, fReplace, TRUE ) )
	    return;

	if ( show )
	{
	    act( "$n wears $p on $s hands.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );

	    act( "You wear $p on your hands.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	}

	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
	if ( !remove_obj( ch, WEAR_ARMS, fReplace, TRUE ) )
	    return;

	if ( show )
	{
	    act( "$n wears $p on $s arms.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );

	    act( "You wear $p on your arms.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	}

	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace, TRUE ) )
	    return;

	if ( show )
	{
	    act( "$n wears $p about $s torso.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );

	    act( "You wear $p about your torso.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	}

	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
	if ( !remove_obj( ch, WEAR_WAIST, fReplace, TRUE ) )
	    return;

	if ( show )
	{
	    act( "$n wears $p about $s waist.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );

	    act( "You wear $p about your waist.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	}

	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ANKLE ) )
    {
	if ( get_eq_char( ch, WEAR_ANKLE_L ) != NULL
	&&   get_eq_char( ch, WEAR_ANKLE_R ) != NULL
	&&   !remove_obj( ch, WEAR_ANKLE_L, fReplace, TRUE )
	&&   !remove_obj( ch, WEAR_ANKLE_R, fReplace, TRUE ) )
	    return;

        if ( get_eq_char( ch, WEAR_ANKLE_L ) == NULL )
        {
	    if ( show )
	    {
		act( "$n wears $p on $s left ankle.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );

		act( "You wear $p on your left ankle.",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
	    }

            equip_char( ch, obj, WEAR_ANKLE_L );
            return;
        }

        if ( get_eq_char( ch, WEAR_ANKLE_R ) == NULL )
        {
	    if ( show )
	    {
		act( "$n wears $p on $s right ankle.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );

		act( "You wear $p on your right ankle.",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
	    }

            equip_char( ch, obj, WEAR_ANKLE_R );
            return;
        }

        send_to_char( "You already wear two ankle items.\n\r", ch );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_EAR ) )
    {
	if ( get_eq_char( ch, WEAR_EAR_L ) != NULL
	&&   get_eq_char( ch, WEAR_EAR_R ) != NULL
	&&   !remove_obj( ch, WEAR_EAR_L, fReplace, TRUE )
	&&   !remove_obj( ch, WEAR_EAR_R, fReplace, TRUE ) )
	    return;

        if ( get_eq_char( ch, WEAR_EAR_L ) == NULL )
        {
	    if ( show )
	    {
		act( "$n wears $p on $s left ear.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );

		act( "You wear $p on your left ear.",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
	    }

            equip_char( ch, obj, WEAR_EAR_L );
            return;
        }

        if ( get_eq_char( ch, WEAR_EAR_R ) == NULL )
        {
	    if ( show )
	    {
		act( "$n wears $p on $s right ear.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );

		act( "You wear $p on your right ear.",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
	    }

	    equip_char( ch, obj, WEAR_EAR_R );
	    return;
        }

        send_to_char( "You already wear two ear items.\n\r", ch );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace, TRUE )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace, TRUE ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
	{
	    if ( show )
	    {
		act( "$n wears $p around $s left wrist.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );

		act( "You wear $p around your left wrist.",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
	    }

	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
	{
	    if ( show )
	    {
		act( "$n wears $p around $s right wrist.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );

		act( "You wear $p around your right wrist.",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
	    }

	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}

	send_to_char( "You already wear two wrist items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
	OBJ_DATA *weapon;

	if ( !remove_obj( ch, WEAR_SHIELD, fReplace, TRUE ) )
	    return;

	if ( (weapon = get_eq_char(ch,WEAR_WIELD)) != NULL
	&&   IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS)
	&&   get_skill(ch,gsn_shield_levitate) <= 0 )
	{
	    send_to_char("Your hands are tied up with your weapon!\n\r",ch);
	    return;
	}

        if ( get_eq_char (ch, WEAR_SECONDARY) != NULL
	&&   get_skill(ch,gsn_shield_levitate) <= 0 )
        {
            send_to_char ("You cannot use a shield while using 2 weapons.\n\r",ch);
	    return;
	}

	if ( show )
	{
	    if ((weapon != NULL
	    &&  IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS))
	    ||  (get_eq_char (ch, WEAR_SECONDARY) != NULL))
	    {
		act( "$n levitates $p in front of $m.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );

		act( "You levitate $p in front of you.",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
	    } else {
		act( "$n wears $p as a shield.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );

		act( "You wear $p as a shield.",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
	    }
	}

	equip_char( ch, obj, WEAR_SHIELD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	int sn, skill;

	if ( !remove_obj( ch, WEAR_WIELD, fReplace, TRUE ) )
	    return;

	if ( !IS_NPC(ch)
	&& get_obj_weight(obj) > (str_app[get_curr_stat(ch,STAT_STR)].wield
		* 10))
	{
	    send_to_char( "It is too heavy for you to wield.\n\r", ch );
	    return;
	}

	if (!IS_NPC(ch)
	&&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
 	&&  (get_eq_char(ch,WEAR_SHIELD) != NULL
	||  get_eq_char(ch,WEAR_SECONDARY) != NULL))
	{
	    if ( get_skill(ch,gsn_shield_levitate) <= 0 )
	    {
	        send_to_char("You need two hands free for that weapon.\n\r",ch);
	        return;
	    }

	    if (!IS_NPC(ch)
		&&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
		&&  get_eq_char(ch,WEAR_SECONDARY) != NULL)
	    {
                send_to_char("You need two hands free for that weapon.\n\r",ch);
                return;
            }

	    else if ( show )
	    {
		shieldobj = get_eq_char(ch, WEAR_SHIELD);
		act( "$n levitates $p in front of $m.", ch, shieldobj, NULL, TO_ROOM,POS_RESTING);
		act( "You levitate $p in front of you.", ch, shieldobj, NULL, TO_CHAR,POS_RESTING);
	    }
	}

	if ( show )
	{
	    act( "$n wields $p.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );

	    act( "You wield $p.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	}

	equip_char( ch, obj, WEAR_WIELD );

	if ( show )
	{
	    sn = get_weapon_sn(ch,FALSE);

	    if ( sn == gsn_hand_to_hand )
		return;

	    skill = get_weapon_skill(ch,sn);

	    if (skill >= 100)
		act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    else if (skill > 85)
		act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    else if (skill > 70)
		act("You are skilled with $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    else if (skill > 50)
		act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    else if (skill > 25)
		act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    else if (skill > 1)
		act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    else
		act("You don't even know which end is up on $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	}

	return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
	if ( !remove_obj( ch, WEAR_HOLD, fReplace, TRUE ) )
	    return;

        if ( get_eq_char (ch, WEAR_SECONDARY) != NULL )
        {
            send_to_char ("You cannot hold an item while using 2 weapons.\n\r",ch);
            return;
        }

	if ( show )
	{
	    act( "$n holds $p in $s hand.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );

	    act( "You hold $p in your hand.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	}

	equip_char( ch, obj, WEAR_HOLD );
	return;
    }

    if ( CAN_WEAR(obj,ITEM_WEAR_SOUL) )
    {
	sh_int soul_pos;

	for ( soul_pos = WEAR_SOUL1; soul_pos <= WEAR_SOUL15; soul_pos++ )
	{
	    if ( get_eq_char( ch, soul_pos ) == NULL )
	    {
		if ( show )
		{
		    act("$n proudly displays $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
		    act("You proudly display $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
		}

		equip_char(ch,obj,soul_pos);
		return;
	    }
	}

	send_to_char( "You may only display 15 souls at a time.\n\r", ch );
	return;
    }

    if ( CAN_WEAR(obj,ITEM_WEAR_CLAN) )
    {
        if( !remove_obj(ch,WEAR_CLAN, fReplace, TRUE) )
           return;

	if ( show )
	{
	    act("$n proudly displays membership with $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    act("You proudly display membership with $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	}

        equip_char(ch,obj,WEAR_CLAN);
        return;
    }

    if ( CAN_WEAR(obj,ITEM_WEAR_EYES) )
    {
        if ( !remove_obj(ch,WEAR_EYES, fReplace, TRUE) )
           return;

	if ( show )
	{
	    act("$n shields $s eyes with $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    act("You shield your eyes with $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	}

        equip_char(ch,obj,WEAR_EYES);
        return;
    }

    if ( CAN_WEAR(obj,ITEM_WEAR_CHEST) )
    {
	if(!remove_obj(ch,WEAR_CHEST, fReplace, TRUE) )
	   return;

	if ( show )
	{
	    act("$n sews $p to his chest.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    act("You sew $p to your chest.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	}

	equip_char(ch,obj,WEAR_CHEST);
	return;
    }

    if ( CAN_WEAR(obj,ITEM_WEAR_FLOAT) )
    {
	if (!remove_obj(ch,WEAR_FLOAT, fReplace, TRUE) )
	    return;

	if ( show )
	{
	    act("$n releases $p to float next to $m.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    act("You release $p and it floats next to you.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	}

	equip_char(ch,obj,WEAR_FLOAT);
	return;
    }

    if ( fReplace )
	send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

    return;
}

void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wear, wield, or hold what?\n\r", ch );
	return;
    }

    if ( ch->stunned )
    {
	send_to_char( "You can't wear stuff stunned!\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
		wear_obj( ch, obj, FALSE, TRUE );
	}

//	act("$n wears many things.",ch,NULL,NULL,TO_ROOM,POS_RESTING);
//	send_to_char("You wear many things.\n\r",ch);
	return;
    } else {
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	wear_obj( ch, obj, TRUE, TRUE );

    }

    return;
}

void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove what?\n\r", ch );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
        if ( ( obj = get_obj_wear( ch, arg, TRUE ) ) == NULL )
        {
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
        }
        remove_obj( ch, obj->wear_loc, TRUE, TRUE );
    }
    else
    {
        found = FALSE;
        for ( obj = ch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;

            if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
            &&   can_see_obj( ch, obj )
            &&   obj->wear_loc != WEAR_NONE )
            {
                found = TRUE;
                remove_obj( ch, obj->wear_loc, TRUE, TRUE );
            }
        }
// 	act("$n removes some things.",ch,NULL,NULL,TO_ROOM,POS_RESTING);
//	send_to_char("You remove some items.\n\r",ch);

        if ( !found )
        {
            if ( arg[3] == '\0' )
                act( "You are not wearing anything.",
                    ch, NULL, arg, TO_CHAR,POS_RESTING);
            else
                act( "You are not wearing any $T.",
                    ch, NULL, &arg[4], TO_CHAR,POS_RESTING );
        }
    }

    return;
}



void do_sacrifice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int silver;
    int total;

    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];


    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
	act( "$n offers $mself to $G, who graciously declines.",ch, NULL, NULL, TO_ROOM,POS_RESTING);
	act( "$G appreciates your offer and may accept it later.", ch, NULL, NULL, TO_CHAR,POS_RESTING);
	return;
    }


    if (!str_cmp("all",arg) || !str_prefix("all.",arg))
    {
        OBJ_DATA *obj_next;
        bool found = FALSE;
	total = 0;

        for (obj = ch->in_room->contents;obj;obj = obj_next)
        {
            obj_next = obj->next_content;

            if (arg[3] != '\0' && !is_name(&arg[4],obj->name))
                continue;

            if ( !CAN_WEAR(obj,ITEM_TAKE)
            ||   (obj->item_type == ITEM_CORPSE_PC && obj->contains != NULL)
	    ||   (obj->disarmed_from != NULL && !can_pk(ch,obj->disarmed_from)
		 && obj->disarmed_from != ch)
	    ||   IS_OBJ_STAT(obj,ITEM_NO_SAC)
	    ||   IS_OBJ_STAT(obj,ITEM_AQUEST)
	    ||   IS_OBJ_STAT(obj,ITEM_FORGED) )
                continue;

            silver = UMAX(1,obj->level * 3);

            if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
                silver = UMIN(silver,obj->cost);

            found = TRUE;

	    total += silver;
            extract_obj( obj );

	}
        if (found)
	{
            if (IS_SET(ch->act,PLR_AUTOSPLIT))
            {
                members = 0;
                for (gch = ch->in_room->people;gch;gch = gch->next_in_room)
                    if (is_same_group(ch,gch))
                        members++;
                    if (members > 1 && total > 1)
                    {
                        sprintf(buf,"%d",total);
                        do_split( ch, buf);
                    }
	    }
	    sprintf(buf,"$G gives you {g%d{x silver for your sacrifices.",total);
	    act(buf,ch,NULL,NULL,TO_CHAR,POS_RESTING);
	    act("$n sacrifices some things in the room to $G.",ch,NULL,NULL,TO_ROOM,POS_RESTING);
	    add_cost(ch,total,VALUE_SILVER);
            wiznet("$N sends up everything in that room as a burnt offering.",ch,obj,WIZ_SACCING,0,0);
        }
        else
            send_to_char("There is nothing sacrificable in this room.\n\r",ch);
        return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents );
    if ( obj == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }

    if ( obj->item_type == ITEM_CORPSE_PC )
    {
	if (obj->contains)
        {
  	   act( "$G wouldn't like that.",ch,NULL,NULL,TO_CHAR,POS_RESTING);
	   return;
        }
    }

    if ( !CAN_WEAR(obj, ITEM_TAKE)	|| IS_OBJ_STAT(obj, ITEM_NO_SAC)
    ||  IS_OBJ_STAT(obj,ITEM_AQUEST)	|| IS_OBJ_STAT(obj, ITEM_FORGED) )
    {
	act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR,POS_RESTING);
	return;
    }

    if ( obj->disarmed_from != NULL
    &&   obj->disarmed_from != ch
    &&   !can_pk(ch,obj->disarmed_from) )
    {
	act("You can not attack $N, so therefore you can not sacfirice $p, $S weapon.",
	    ch,obj,obj->disarmed_from,TO_CHAR,POS_DEAD);
  	return;
    }

    silver = UMAX(1,obj->level * 3);

    if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    	silver = UMIN(silver,obj->cost);

    if (silver == 1)
    {
        act("$G gives you one silver coin for your sacrifice.",ch,NULL,NULL,TO_CHAR,POS_RESTING);
    }
    else
    {
	sprintf(buf,"$G gives you {g%d{x silver coins for your sacrifice of %s.",
		silver, obj->short_descr);
	act(buf,ch,NULL,NULL,TO_CHAR,POS_RESTING);
    }

    add_cost(ch,silver,VALUE_SILVER);

    if (IS_SET(ch->act,PLR_AUTOSPLIT) )
    { /* AUTOSPLIT code */
    	members = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	{
    	    if ( is_same_group( gch, ch ) )
            members++;
    	}

	if ( members > 1 && silver > 1)
	{
	    sprintf(buffer,"%d",silver);
	    do_split(ch,buffer);
	}
    }

    act( "$n sacrifices $p to $G.", ch, obj, NULL, TO_ROOM,POS_RESTING);
    wiznet("$N sends up $p as a burnt offering.",
	   ch,obj,WIZ_SACCING,0,0);
    extract_obj( obj );
    return;
}

void do_quaff( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    sh_int pos;
    bool spells[4];

    if ( argument[0] == '\0' )
    {
	send_to_char( "Quaff what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
    {
	send_to_char( "You do not have that potion.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION )
    {
	send_to_char( "You can quaff only potions.\n\r", ch );
	return;
    }

    if ( ch->stunned )
    {
	send_to_char( "You're still a little woozy.\n\r", ch );
	return;
    }

    if ( ch->level < obj->level )
    {
	send_to_char( "This liquid is too powerful for you to drink.\n\r", ch );
	return;
    }

    if ( arena_flag( ch, ARENA_NO_POTION ) )
	return;

    act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM, POS_RESTING );
    act( "You quaff $p.", ch, obj, NULL ,TO_CHAR, POS_RESTING );

    for ( pos = 0; pos < 4; pos++ )
	spells[pos] = TRUE;

    if ( obj->pIndexData->targets != 0 )
    {
	while ( count_spells( spells ) > obj->pIndexData->targets )
	    spells[number_range( 0, 4 )] = FALSE;
    }

    for ( pos = 0; pos < 4; pos++ )
    {
	if ( obj->value[pos+1] > 0
	&&   spells[pos] == TRUE )
	{
	    if ( number_percent( ) > obj->success )
		send_to_char( "Spell quaff failed.\n\r", ch );
	    else
		obj_cast_spell( obj->value[pos+1], obj->value[0], ch, ch, NULL, NULL );
	}
    }

    extract_obj( obj );
    return;
}



void do_recite( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim = NULL;
    OBJ_DATA *obj, *scroll;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    sh_int pos;
    bool spells[4];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( scroll = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that scroll.\n\r", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	send_to_char( "You can recite only scrolls.\n\r", ch );
	return;
    }

    if ( ch->stunned )
    {
	send_to_char( "You're still a little woozy.\n\r", ch );
	return;
    }

    if ( ch->level < scroll->level )
    {
	send_to_char( "This scroll is too complex for you to comprehend.\n\r", ch );
	return;
    }

    if ( arena_flag( ch, ARENA_NO_SCROLL ) )
	return;

    if ( is_affected( ch, skill_lookup( "silence" ) ) )
    {
	send_to_char( "Your lips fail to move.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_scrolls].beats );

    obj = NULL;
    if ( arg2[0] == '\0' )
	victim = ch;

    else if ( str_cmp( arg2, "NULL" ) )
    {
	if ( ( victim = get_char_room ( ch, NULL, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, NULL, arg2 ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    act( "$n recites $p.", ch, scroll, NULL, TO_ROOM, POS_RESTING );
    act( "You recite $p.", ch, scroll, NULL, TO_CHAR, POS_RESTING );

    if ( number_percent( ) >= 20 + get_skill( ch, gsn_scrolls ) * 4 / 5 )
    {
	send_to_char( "You mispronounce a syllable.\n\r", ch );
	check_improve( ch, gsn_scrolls, FALSE, 2 );
    }

    else
    {
	for ( pos = 0; pos < 4; pos++ )
	    spells[pos] = TRUE;

	if ( scroll->pIndexData->targets != 0 )
	{
	    while ( count_spells( spells ) > scroll->pIndexData->targets )
		spells[number_range( 0, 4 )] = FALSE;
	}

	for ( pos = 0; pos < 4; pos++ )
	{
	    if ( scroll->value[pos+1] > 0
	    &&   spells[pos] == TRUE )
	    {
		if ( number_percent( ) > scroll->success )
		    send_to_char( "Spell recite failed.\n\r", ch );
		else
		    obj_cast_spell( scroll->value[pos+1], scroll->value[0], ch, victim, obj, argument );
	    }
	}

	check_improve( ch, gsn_scrolls, TRUE, 2 );
    }

    extract_obj( scroll );
    return;
}

void do_brandish( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch, *vch_next;
    OBJ_DATA *staff;
    int sn;

    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char( "You can brandish only with a staff.\n\r", ch );
	return;
    }

    if ( ch->stunned )
    {
	send_to_char( "You're still a little woozy.\n\r", ch );
	return;
    }

    if ( ( sn = staff->value[3] ) < 0
    ||   sn >= maxSkill
    ||   skill_table[sn].spell_fun == 0 )
    {
	bug( "Do_brandish: bad sn %d.", sn );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_staves].beats );

    if ( staff->value[2] > 0 )
    {
	sh_int match = 0;

	act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM, POS_RESTING );
	act( "You brandish $p.",  ch, staff, NULL, TO_CHAR, POS_RESTING );

	if ( number_percent( ) >= 20 + get_skill( ch, gsn_staves ) * 4 / 5 )
 	{
	    act( "You fail to invoke $p.",
		ch, staff, NULL, TO_CHAR, POS_RESTING );
	    act( "...and nothing happens.",
		ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    check_improve( ch, gsn_staves, FALSE, 2 );
	}

	else for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;

	    switch ( skill_table[sn].target )
	    {
		default:
		    bug( "Do_brandish: bad target for sn %d.", sn );
		    return;

		case TAR_IGNORE:
		    if ( vch != ch )
			continue;
		    break;

		case TAR_CHAR_OFFENSIVE:
		    if ( IS_NPC( ch ) ? IS_NPC( vch ) : !IS_NPC( vch ) )
			continue;
		    break;

		case TAR_CHAR_DEFENSIVE:
		    if ( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) )
			continue;
		    break;

		case TAR_CHAR_SELF:
		    if ( vch != ch )
			continue;
		    break;
	    }

	    if ( number_percent( ) > staff->success )
		act( "The magic of $p failed to reach $N.",
		    ch, staff, vch, TO_CHAR, POS_RESTING );
	    else
		obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL, NULL );
	    check_improve( ch, gsn_staves, TRUE, 2 );

	    if ( ++match >= staff->pIndexData->targets )
		break;
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM,POS_RESTING);
	act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR,POS_RESTING);
	extract_obj( staff );
    }

    return;
}

void do_zap( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj, *wand;

    if ( argument[0] == '\0' && ch->fighting == NULL )
    {
	send_to_char( "Zap whom or what?\n\r", ch );
	return;
    }

    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
	send_to_char( "You can zap only with a wand.\n\r", ch );
	return;
    }

    if ( ch->stunned )
    {
	send_to_char( "You're still a little woozy.\n\r", ch );
	return;
    }

    obj = NULL;
    if ( argument[0] == '\0' )
    {
	if ( ch->fighting != NULL )
	    victim = ch->fighting;
	else
	{
	    send_to_char( "Zap whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, NULL, argument ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, NULL, argument ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    WAIT_STATE( ch, skill_table[gsn_wands].beats );

    if ( wand->value[2] > 0 )
    {
	if ( victim != NULL )
	{
	    act( "$n zaps $N with $p.", ch, wand, victim, TO_ROOM, POS_RESTING );
	    act( "You zap $N with $p.", ch, wand, victim, TO_CHAR, POS_RESTING );
	} else {
	    act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM, POS_RESTING );
	    act( "You zap $P with $p.", ch, wand, obj, TO_CHAR, POS_RESTING );
	}

	if ( number_percent( ) >= 20 + get_skill( ch, gsn_wands ) * 4 / 5 )
	{
	    act( "Your efforts with $p produce only smoke and sparks.",
		 ch, wand, NULL, TO_CHAR, POS_RESTING );
	    act( "$n's efforts with $p produce only smoke and sparks.",
		 ch, wand, NULL, TO_ROOM, POS_RESTING );
	    check_improve( ch, gsn_wands, FALSE, 2 );
	}

	else
	{
	    if ( number_percent( ) > wand->success )
		send_to_char( "Wand spell failed.\n\r", ch );
	    else
		obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj, NULL );
	    check_improve( ch, gsn_wands, TRUE, 2 );
	}
    }

    if ( --wand->value[2] <= 0 )
    {
	act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM, POS_RESTING );
	act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR, POS_RESTING );
	extract_obj( wand );
    }

    return;
}

void do_steal( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Steal what from whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if (IS_IMMORTAL(victim))
    {
	send_to_char("Very, very bad idea.\n\r",ch);
        act( "$n tried to steal from you.", ch, NULL, victim, TO_VICT,POS_RESTING);
        act( "$n tried to steal from $N.",  ch, NULL, victim, TO_NOTVICT,POS_RESTING);
	return;
    }

    if (is_safe(ch,victim))
	return;

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

    WAIT_STATE( ch, skill_table[gsn_steal].beats );
    percent  = number_percent();
    if (get_skill(ch,gsn_steal) >= 1)
    percent  += ( IS_AWAKE(victim) ? 10 : -50 );

    if ( !IS_NPC(ch) && percent > get_skill(ch,gsn_steal) )
    {
	/*
	 * Failure.
	 */
	affect_strip(ch,gsn_obfuscate);
	affect_strip(ch,gsn_camouflage);
	affect_strip(ch,gsn_forest_meld);

	send_to_char( "Oops.\n\r", ch );
	act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT,POS_RESTING);
	act( "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT,POS_RESTING);
	switch(number_range(0,3))
	{
	case 0 :
	   sprintf( buf, "{z{R%s{x{R is a lousy thief!{x", PERS(ch,victim) );
	   break;
        case 1 :
	   sprintf( buf, "{z{R%s{x{R couldn't rob %s way out of a paper bag!{x",
		PERS(ch,victim), (ch->sex == 2) ? "her" : "his");
	   break;
	case 2 :
	    sprintf( buf,"{z{R%s{x{R tried to rob me!{x",PERS(ch,victim) );
	    break;
	case 3 :
	    sprintf(buf,"{RKeep your hands out of there, {z%s{x{R!{x",
		PERS(ch,victim) );
	    break;
        }
	do_yell( victim, buf );
	if ( !IS_NPC(ch) )
	{
	    if ( !IS_NPC(victim) )
	    {
		sprintf(buf,"{w({RPK{w) {V$N tried to steal %s from %s.",
		    arg1, victim->name);
		wiznet(buf,ch,NULL,WIZ_PKILLS,0,0);
		check_pktimer( ch, victim, TRUE );
	    }
	    check_improve(ch,gsn_steal,FALSE,2);
	    multi_hit( victim, ch, TYPE_UNDEFINED, TRUE );
	}

	return;
    }

    if ( !str_cmp( arg1, "coin"  )
    ||   !str_cmp( arg1, "coins" )
    ||   !str_cmp( arg1, "gold"  )
    ||	 !str_cmp( arg1, "silver"))
    {
	int gold, silver;

	gold = victim->gold * number_range(1, ch->level) / LEVEL_HERO;
	silver = victim->silver * number_range(1,ch->level) / LEVEL_HERO;
	if ( gold <= 0 && silver <= 0 )
	{
	    send_to_char( "You couldn't get any coins.\n\r", ch );
	    if (!IS_NPC(victim))
	    {
		sprintf(buf,"{w({RPK{w) {V$N tries to steal coins from %s.",
		    victim->name);
		wiznet(buf,ch,NULL,WIZ_PKILLS,0,0);
		check_pktimer( ch, victim, TRUE );
	    }
	    return;
	}

	ch->gold     	+= gold;
	ch->silver   	+= silver;
	victim->silver 	-= silver;
	victim->gold 	-= gold;
	if (silver <= 0)
	    sprintf( buf, "Bingo!  You got {g%d{x gold coins.\n\r", gold );
	else if (gold <= 0)
	    sprintf( buf, "Bingo!  You got {g%d{x silver coins.\n\r",silver);
	else
	    sprintf(buf, "Bingo!  You got {g%d{x silver and {g%d{x gold coins.\n\r",
		    silver,gold);

	send_to_char( buf, ch );
	check_improve(ch,gsn_steal,TRUE,2);
	if (!IS_NPC(victim))
	{
	    sprintf(buf,"{w({RPK{w) {V$N steals %d silver|%d gold from %s.",
		silver, gold, victim->name);
	    wiznet(buf,ch,NULL,WIZ_PKILLS,0,0);
	    check_pktimer( ch, victim, TRUE );
	}
	return;
    }

    if ( ( obj = get_obj_carry( victim, arg1 ) ) == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj )
    ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
    ||   IS_SET(obj->extra_flags, ITEM_AQUEST)
    ||   IS_SET(obj->extra_flags, ITEM_FORGED) )
    {
	send_to_char( "You can't pry it away.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	send_to_char( "You have your hands full.\n\r", ch );
	return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	send_to_char( "You can't carry that much weight.\n\r", ch );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    check_improve(ch,gsn_steal,TRUE,2);

    act( "You have stolen $p from $N!", ch, obj, victim, TO_CHAR, POS_RESTING );

    if (!IS_NPC(victim))
    {
	sprintf(buf,"{w({RPK{w) {V$N steals $p {Vfrom %s.",
	    victim->name);
	wiznet(buf,ch,obj,WIZ_PKILLS,0,0);
	check_pktimer( ch, victim, TRUE );
    }
    return;
}

CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

    pShop = NULL;
    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
	if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
	    break;
    }

    if ( pShop == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return NULL;
    }

    /*
     * Undesirables.
     */
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_TWIT) )
    {
	do_say( keeper, "{aTwits are not welcome!{x" );
	sprintf( buf, "{a%s the {z{RTWIT{x is over here!{x\n\r", ch->name );
	do_yell( keeper, buf );
	return NULL;
    }
    /*
     * Shop hours.
     */
    if ( time_info.hour < pShop->open_hour
    ||   time_info.hour > pShop->close_hour )
    {
	do_say( keeper, "{aSorry, I am closed. Come back later.{x" );
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
	do_say( keeper, "{aI don't trade with folks I can't see.{x" );
	return NULL;
    }

    if ( keeper->clan != 0 && keeper->clan != ch->clan )
    {
	sprintf( buf, "I only deal with members of %s{S.",
	    clan_table[keeper->clan].color );
	do_say( keeper, buf );

	sprintf( buf, "HELP!! We are being invaded by %s!", ch->name );
	do_clantalk( keeper, buf );
	return NULL;
    }

    return keeper;
}

void obj_to_keeper( OBJ_DATA *obj, CHAR_DATA *ch )
{
    OBJ_DATA *t_obj, *t_obj_next;

    /* see if any duplicates are found */
    for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next)
    {
	t_obj_next = t_obj->next_content;

	if (obj->pIndexData == t_obj->pIndexData
	&&  !str_cmp(obj->short_descr,t_obj->short_descr))
	{
	    /* if this is an unlimited item, destroy the new one */
	    if (IS_OBJ_STAT(t_obj,ITEM_INVENTORY))
	    {
		extract_obj(obj);
		return;
	    }
	    obj->cost = t_obj->cost; /* keep it standard */
	    break;
	}
    }

    if (t_obj == NULL)
    {
	obj->next_content = ch->carrying;
	ch->carrying = obj;
    }
    else
    {
	obj->next_content = t_obj->next_content;
	t_obj->next_content = obj;
    }

    obj->carried_by      = ch;
    obj->in_room         = NULL;
    obj->in_obj          = NULL;
    ch->carry_number    += get_obj_number( obj );
    ch->carry_weight    += get_obj_weight( obj );
}

OBJ_DATA *get_obj_keeper( CHAR_DATA *ch, CHAR_DATA *keeper, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = keeper->carrying; obj != NULL; obj = obj->next_content )
    {
        if (obj->wear_loc == WEAR_NONE
        &&  can_see_obj( keeper, obj )
	&&  can_see_obj(ch,obj)
        &&  is_name( arg, obj->name ) )
        {
            if ( ++count == number )
                return obj;

	    /* skip other objects of the same name */
	    while (obj->next_content != NULL
	    && obj->pIndexData == obj->next_content->pIndexData
	    && !str_cmp(obj->short_descr,obj->next_content->short_descr))
		obj = obj->next_content;
        }
    }

    return NULL;
}

int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
    SHOP_DATA *pShop;
    int cost;

    if ( obj == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
	return 0;

    if ( fBuy )
    {
	cost = obj->cost * pShop->profit_buy  / 100;
    }
    else
    {
	OBJ_DATA *obj2;
	int itype;

	cost = 0;
	for ( itype = 0; itype < MAX_TRADE; itype++ )
	{
	    if ( obj->item_type == pShop->buy_type[itype] )
	    {
		cost = obj->cost * pShop->profit_sell / 100;
		break;
	    }
	}

	if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
	{
	    for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
	    {
	    	if ( obj->pIndexData == obj2->pIndexData
		&&   !str_cmp(obj->short_descr,obj2->short_descr) )
		{
	 	    if (IS_OBJ_STAT(obj2,ITEM_INVENTORY))
			cost /= 2;
		    else
                    	cost = cost * 3 / 4;
		}
	    }
	}
    }

    if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
    {
	if (obj->value[1] == 0)
	    cost /= 4;
	else
	    cost = cost * obj->value[2] / obj->value[1];
    }

    return cost;
}



void do_buy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cost,roll;
    long multicost;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Buy what?\n\r", ch );
	return;
    }

    smash_tilde( argument );

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;

	if ( IS_NPC(ch) )
	    return;

	argument = one_argument(argument,arg);

	/* hack to make new thalos pets work */
	if (ch->in_room->vnum == 9621)
	    pRoomIndexNext = get_room_index(9706);
	else
	    pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet         = get_char_room( ch, NULL, arg );
	ch->in_room = in_room;

	if ( pet == NULL || !IS_NPC( pet ) || !IS_SET(pet->act, ACT_PET) )
	{
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	if ( ch->pet != NULL )
	{
	    send_to_char("You already own a pet.\n\r",ch);
	    return;
	}

 	cost = 15 * pet->level * pet->level;

	if ( (ch->silver + (100 * ch->gold) + (10000 * ch->platinum) ) < cost )
	{
	    send_to_char( "You can't afford it.\n\r", ch );
	    return;
	}

	if ( ch->level < pet->pIndexData->level )
	{
	    send_to_char(
		"You're not powerful enough to master this pet.\n\r", ch );
	    return;
	}

	/* haggle */
	roll = number_percent();
	if (roll < get_skill(ch,gsn_haggle))
	{
	    cost -= cost / 2 * roll / 100;
	    sprintf(buf,"You haggle the price down to {g%d{x coins.\n\r",cost);
	    send_to_char(buf,ch);
	    check_improve(ch,gsn_haggle,TRUE,4);

	}

	deduct_cost(ch,cost,VALUE_SILVER);
	pet		= create_mobile( pet->pIndexData );
	pet->platinum	= 0;
	pet->gold	= 0;
	pet->silver	= 0;
	SET_BIT(pet->act, ACT_PET);
	SET_BIT(pet->affected_by, AFF_CHARM);

	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' )
	{
/*	    sprintf( buf, "%s %s", pet->name, arg );
	    free_string( pet->name );
	    pet->name = str_dup( buf );*/
	    send_to_char("Your pet would be more happy with its own name.\n\r",ch);
	}

	sprintf( buf, "%sA neck tag says '{cI belong to %s{x'.\n\r",
	    pet->description, ch->name );
	free_string( pet->description );
	pet->description = str_dup( buf );

	char_to_room( pet, ch->in_room );
	add_follower( pet, ch );
	pet->leader = ch;
	ch->pet = pet;
	pet->alignment = ch->alignment;
	send_to_char( "Enjoy your pet.\n\r", ch );
	act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM,POS_RESTING);
	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj,*t_obj;
	char arg[MAX_INPUT_LENGTH];
	int number, count = 1;

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;

	if ( !can_use_clan_mob(ch,keeper) )
	    return;

	number = mult_argument(argument,arg);
	obj  = get_obj_keeper( ch,keeper, arg );
	cost = get_cost( keeper, obj, TRUE );

	if ( cost <= 0 || !can_see_obj( ch, obj ) )
	{
	    act( "$n tells you '{aI don't sell that -- try '{Mlist{a'{x'.",
		keeper, NULL, ch, TO_VICT,POS_RESTING);
	    return;
	}

	if (number < 0 || number > 100)
	{
	    act("$n tells you '{aNice try, jackass!{x'.",
		keeper,NULL,ch,TO_VICT,POS_RESTING);
	    return;
	}
	if (number == 0)
		number = 1;

	if ( !IS_OBJ_STAT(obj,ITEM_INVENTORY) )
	{
	    for (t_obj = obj->next_content;
	     	 count < number && t_obj != NULL;
	     	 t_obj = t_obj->next_content)
	    {
	    	if (t_obj->pIndexData == obj->pIndexData
	    	&&  !str_cmp(t_obj->short_descr,obj->short_descr))
		    count++;
	    	else
		    break;
	    }

	    if (count < number)
	    {
	    	act("$n tells you '{aI don't have that many in stock{x'.",
		    keeper,NULL,ch,TO_VICT,POS_RESTING);
	    	return;
	    }
	}

	if ( (ch->silver + (ch->gold * 100) + (ch->platinum * 10000) ) < cost * number )
	{
	    if (number > 1)
		act("$n tells you '{aYou can't afford to buy that many{x'.",
		    keeper,obj,ch,TO_VICT,POS_RESTING);
	    else
	    	act( "$n tells you '{aYou can't afford to buy $p{x'.",
		    keeper, obj, ch, TO_VICT,POS_RESTING);
	    return;
	}

	if ( obj->level > ch->level )
	{
	    act( "$n tells you '{aYou can't use $p {ayet{x'.",
		keeper, obj, ch, TO_VICT,POS_RESTING );
	    return;
	}

	if (ch->carry_number +  number * get_obj_number(obj) > can_carry_n(ch))
	{
	    send_to_char( "You can't carry that many items.\n\r", ch );
	    return;
	}

	if ( ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch))
	{
	    send_to_char( "You can't carry that much weight.\n\r", ch );
	    return;
	}

	/* haggle */
	roll = number_percent();
	if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT)
	&& roll < get_skill(ch,gsn_haggle))
	{
	    cost -= obj->cost / 2 * roll / 100;
	    act("You haggle with $N.",ch,NULL,keeper,TO_CHAR,POS_RESTING);
	    check_improve(ch,gsn_haggle,TRUE,4);
	}

	if (number > 1)
	{
	    sprintf(buf,"$n buys $p[%d].",number);
	    act(buf,ch,obj,NULL,TO_ROOM,POS_RESTING);
	    sprintf(buf,"You buy $p[%d] for {g%d{x silver.",number,cost * number);
	    act(buf,ch,obj,NULL,TO_CHAR,POS_RESTING);
	}
	else
	{
	    act( "$n buys $p.", ch, obj, NULL, TO_ROOM ,POS_RESTING);
	    sprintf(buf,"You buy $p for {g%d{x silver.",cost);
	    act( buf, ch, obj, NULL, TO_CHAR,POS_RESTING );
	}
	multicost = cost*number;
	while (multicost >= 100000)
	{
	    deduct_cost(ch,10,VALUE_PLATINUM);
	    add_cost(keeper,10,VALUE_PLATINUM);
	    multicost -= 100000;
	}
	while (multicost >= 10000)
	{
	    deduct_cost(ch,1,VALUE_PLATINUM);
	    add_cost(keeper,1,VALUE_PLATINUM);
	    multicost -= 10000;
	}
	while (multicost >= 1000)
	{
	    deduct_cost(ch,10,VALUE_GOLD);
	    add_cost(keeper,10,VALUE_GOLD);
	    multicost -= 1000;
	}
	while (multicost >= 100)
	{
	    deduct_cost(ch,1,VALUE_GOLD);
	    add_cost(keeper,1,VALUE_GOLD);
	    multicost -= 100;
	}
	if (multicost > 0)
	{
	    roll = multicost;
	    deduct_cost(ch,roll,VALUE_SILVER);
	    add_cost(keeper,roll,VALUE_SILVER);
	}

	for (count = 0; count < number; count++)
	{
	    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    	t_obj = create_object( obj->pIndexData );
	    else
	    {
		t_obj = obj;
		obj = obj->next_content;
	    	obj_from_char( t_obj );
	    }

	    if (!obj_droptest(ch,t_obj))
	    {
		act("You drop $p as it {Rs{rc{Ra{rl{Rd{rs{x you upon touching it!",ch,t_obj,NULL,TO_CHAR,POS_RESTING);
		act("$n is {Rs{rc{Ra{rl{Rd{re{Rd{x by $p as $e grabs it!",ch,t_obj,NULL,TO_ROOM,POS_RESTING);
		act("$p {Dv{wa{Dn{wi{Ds{wh{De{ws in a {Dm{wi{Ds{wt{x.",ch,t_obj,NULL,TO_CHAR,POS_RESTING);
		act("$p {Dv{wa{Dn{wi{Ds{wh{De{ws in a {Dm{wi{Ds{wt{x.",ch,t_obj,NULL,TO_ROOM,POS_RESTING);
		if (t_obj->in_room != NULL)
		    obj_from_room(t_obj);
		extract_obj(t_obj);
		return;
	    }

	    if ( IS_IMMORTAL( ch ) )
		set_obj_loader( ch, t_obj, "IBUY" );

	    obj_to_char( t_obj, ch );
	    if (cost < t_obj->cost)
	    	t_obj->cost = cost;
	}
    }
}

void do_browse( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int percent_lore, percent_id, sn;

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( !can_use_clan_mob( ch, keeper ) )
	return;

    if ( argument[0] == '\0' )
    {
	act( "$n tells you '{aWhat do you want to look at?{x'.",
	    keeper, NULL, ch, TO_VICT, POS_RESTING );
	return;
    }

    if ( ( obj = get_obj_keeper( ch, keeper, argument ) ) == NULL
    ||   !can_see_obj( ch, obj )
    ||   get_cost( keeper, obj, TRUE ) <= 0 )
    {
	act( "$n tells you '{aI don't sell that -- try '{Mlist{a'{x'.",
	    keeper, NULL, ch, TO_VICT, POS_RESTING );
	return;
    }

    if ( obj->level > ch->level )
    {
	act( "$n tells you '{aYou can't use $p {ayet{x'.",
	    keeper, obj, ch, TO_VICT, POS_RESTING );
	return;
    }

    act( "$N allows $n to look over $p.",
	ch, obj, keeper, TO_ROOM ,POS_RESTING );
    act( "$N allows you to look over $p.",
	ch, obj, keeper, TO_CHAR, POS_RESTING );

    sn = skill_lookup( "identify" );
    percent_lore = get_skill( ch, gsn_lore );
    percent_id = get_skill( ch, sn );

    if ( percent_lore >= percent_id )
	sn = gsn_lore;

    if ( !cost_of_skill( ch, sn ) )
	return;

    if ( ( sn == gsn_lore && number_percent( ) > percent_lore )
    ||   number_percent( ) > percent_id )
    {
	act( "You see nothing special about $p.",
	    ch, obj, NULL, TO_CHAR, POS_RESTING );
	check_improve( ch, sn, FALSE, 4 );
	return;
    }

    final = display_stats( obj, ch, FALSE );
    page_to_char( final->string, ch );
    free_buf( final );
    check_improve( ch, sn, TRUE, 4 );
}

void do_list( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	ROOM_INDEX_DATA *pRoomIndexNext;
	CHAR_DATA *pet;
	bool found;

        if (ch->in_room->vnum == 9621)
            pRoomIndexNext = get_room_index(9706);
        else
            pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );

	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	found = FALSE;
	final = new_buf();

	for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
	{
	    if ( IS_SET(pet->act, ACT_PET) )
	    {
		if ( !found )
		{
		    found = TRUE;
		    add_buf( final, "Pets for sale:\n\r" );
		}
		sprintf( buf, "{y[{w%3d{y] {G%8d {w- %s{x\n\r",
		    pet->pIndexData->level,
		    15 * pet->level * pet->level,
		    pet->short_descr );
		add_buf( final, buf );
	    }
	}

	if ( !found )
	{
	    send_to_char( "Sorry, we're out of pets right now.\n\r", ch );
	    free_buf( final );
	    return;
	}

	page_to_char( final->string, ch );
	free_buf( final );
	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost,count;
	bool found;
	char arg[MAX_INPUT_LENGTH];

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;

        one_argument(argument,arg);

	if ( !can_use_clan_mob(ch,keeper) )
	    return;

	found = FALSE;
	final = new_buf();

	for ( obj = keeper->carrying; obj; obj = obj->next_content )
	{
	    if ( obj->wear_loc == WEAR_NONE
	    &&   can_see_obj( ch, obj )
	    &&   ( cost = get_cost( keeper, obj, TRUE ) ) > 0
	    &&   ( arg[0] == '\0'
 	    ||  is_name(arg,obj->name) ))
	    {
		if ( !found )
		{
		    found = TRUE;
		    add_buf( final, "{y[{wLvl  {GPrice {wQty{y]{x Item\n\r" );
		}

		if (IS_OBJ_STAT(obj,ITEM_INVENTORY))
		    sprintf(buf,"{y[{w%3d {G%6d {w-- {y] {x%s\n\r",
			obj->level,cost,obj->short_descr);
		else
		{
		    count = 1;

		    while (obj->next_content != NULL
		    && obj->pIndexData == obj->next_content->pIndexData
		    && !str_cmp(obj->short_descr,
			        obj->next_content->short_descr))
		    {
			obj = obj->next_content;
			count++;
		    }
		    sprintf(buf,"{y[{w%3d {G%6d {w%2d {y] {x%s\n\r",
			obj->level,cost,count,obj->short_descr);
		}
		add_buf( final, buf );
	    }
	}

	if ( !found )
	{
	    send_to_char( "You can't buy anything here.\n\r", ch );
	    free_buf( final );
	    return;
	}

	page_to_char( final->string, ch );
	free_buf( final );
	return;
    }
}

void do_sell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost,roll;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Sell what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( !can_use_clan_mob(ch,keeper) )
	return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( "$n tells you '{aYou don't have that item{x'.",
	    keeper, NULL, ch, TO_VICT ,POS_RESTING);
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "{RYou can't let go of it{z!!{x\n\r", ch );
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
	act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT,POS_RESTING);
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT,POS_RESTING );
	return;
    }
    if ( cost > (keeper->silver + (100 * keeper->gold) + (10000 * keeper->platinum) ) )
    {
	act("$n tells you '{aI'm afraid I don't have enough wealth to buy $p{x'.",
	    keeper,obj,ch,TO_VICT,POS_RESTING);
	return;
    }

    act( "$n sells $p.", ch, obj, NULL, TO_ROOM,POS_RESTING );
    /* haggle */
    roll = number_percent();
    if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) && roll < get_skill(ch,gsn_haggle))
    {
        send_to_char("You haggle with the shopkeeper.\n\r",ch);
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN(cost,95 * get_cost(keeper,obj,TRUE) / 100);
	cost = UMIN(cost,(keeper->silver + (100 * keeper->gold) + (10000 * keeper->platinum)));
        check_improve(ch,gsn_haggle,TRUE,4);
    }
    sprintf( buf, "You sell $p for {g%d{x silver piece%s.",
	cost, cost == 1 ? "" : "s" );
    act( buf, ch, obj, NULL, TO_CHAR,POS_RESTING );

    while (cost >= 10000)
    {
	deduct_cost(keeper,1,VALUE_PLATINUM);
	add_cost(ch,1,VALUE_PLATINUM);
	cost -= 10000;
    }
    while (cost >= 1000)
    {
	deduct_cost(keeper,10,VALUE_GOLD);
	add_cost(ch,10,VALUE_GOLD);
	cost -= 1000;
    }
    while (cost >= 100)
    {
	deduct_cost(keeper,1,VALUE_GOLD);
	add_cost(ch,1,VALUE_GOLD);
	cost -= 100;
    }
    if (cost > 0)
    {
	deduct_cost(keeper,cost,VALUE_SILVER);
	add_cost(ch,cost,VALUE_SILVER);
    }

    if ( obj->item_type == ITEM_TRASH || IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
    {
	extract_obj( obj );
    }
    else
    {
	obj_from_char( obj );

	set_obj_sockets(ch,obj);

	obj_to_keeper( obj, keeper );
    }

    return;
}



void do_value( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Value what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( "$n tells you '{aYou don't have that item{x'.",
	    keeper, NULL, ch, TO_VICT,POS_RESTING );
	return;
    }

    if ( !can_use_clan_mob(ch,keeper) )
	return;

    if (!can_see_obj(keeper,obj))
    {
        act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT,POS_RESTING);
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT,POS_RESTING );
	return;
    }

    sprintf( buf,
	"$n tells you '{aI'll give you {g%d{a silver coin%s for $p{x'.",
	cost, cost == 1 ? "" : "s" );
    act( buf, keeper, obj, ch, TO_VICT,POS_RESTING );

    return;
}

void do_second (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    OBJ_DATA *shieldobj;
    int skill, sn;
    char buf[MAX_STRING_LENGTH];

    if ( get_skill(ch,gsn_dual_wield) == 0 )
    {
	send_to_char("You must first learn the skill of dual wield!\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
        send_to_char ("Wear which weapon in your off-hand?\n\r",ch);
        return;
    }

    obj = get_obj_carry (ch, argument);

    if (obj == NULL)
    {
        send_to_char ("You have no such thing in your backpack.\n\r",ch);
        return;
    }

    if ( ch->level < obj->level )
    {
        sprintf( buf, "You must be level %d to use this object.\n\r",
            obj->level );
        send_to_char( buf, ch );
        act( "$n tries to use $p, but is too inexperienced.",
            ch, obj, NULL, TO_ROOM,POS_RESTING );
        return;
    }

    if (get_eq_char (ch, WEAR_WIELD) == NULL)
    {
        send_to_char ("You need to wield a primary weapon, before using a secondary one!\n\r",ch);
        return;
    }

    if ( !CAN_WEAR( obj, ITEM_WIELD ) )
    {
	send_to_char ("You can't second that!\n\r", ch);
	return;
    }

    if ( get_eq_char(ch, WEAR_SHIELD) != NULL
    &&   get_skill(ch,gsn_shield_levitate) <= 0 )
    {
	send_to_char ("You cannot use a secondary weapon while using a shield.\n\r",ch);
	return;
    }

    if (get_eq_char(ch, WEAR_HOLD))
    {
        send_to_char ("You cannot use a secondary weapon while holding an item.\n\r",ch);
	return;
    }

    if (IS_WEAPON_STAT(get_eq_char(ch,WEAR_WIELD),WEAPON_TWO_HANDS))
    {
	send_to_char ("Your primary weapon requires {z{Bboth{x hands!\n\r",ch);
	return;
    }

    if (IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS))
    {
        send_to_char ("This weapon requires {z{Bboth{x hands!\n\r",ch);
        return;
    }

    if ( get_obj_weight( obj ) > ( str_app[get_curr_stat(ch,STAT_STR)].wield * 8 ) )
    {
        send_to_char( "This weapon is too heavy to be used as a secondary weapon by you.\n\r", ch );
        return;
    }

    if ( obj->size > -1 && obj->size < 6 )
    {
	if (obj->size > ch->size+1)
	{
	    act("$p is too big for you to use!",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    return;
	}

	if (obj->size < ch->size-1)
	{
	    act("$p is too small for you to use!",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    return;
	}
    }

    if (!remove_obj(ch, WEAR_SECONDARY, TRUE, TRUE))
	return;

    if ( (shieldobj = get_eq_char(ch, WEAR_SHIELD)) != NULL )
    {
	act ("$n levitates $p in front of $m.",ch,shieldobj,NULL,TO_ROOM,POS_RESTING);
	act ("You levitate $p in front of you.",ch,shieldobj,NULL,TO_CHAR,POS_RESTING);
    }

    act ("$n wields $p in $s off-hand.",ch,obj,NULL,TO_ROOM,POS_RESTING);
    act ("You wield $p in your off-hand.",ch,obj,NULL,TO_CHAR,POS_RESTING);
    equip_char ( ch, obj, WEAR_SECONDARY);

    sn = get_weapon_sn(ch,TRUE);

    if (sn == gsn_hand_to_hand)
	return;

    skill = ((get_weapon_skill(ch,sn)*get_skill(ch,gsn_dual_wield))/100);

    if (skill >= 100)
	act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR,POS_RESTING);
    else if (skill > 85)
	act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
    else if (skill > 70)
	act("You are skilled with $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
    else if (skill > 50)
	act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR,POS_RESTING);
    else if (skill > 25)
	act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR,POS_RESTING);
    else if (skill > 1)
	act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
    else
	act("You don't even know which end is up on $p.",
	    ch,obj,NULL,TO_CHAR,POS_RESTING);

    return;
}

void do_junk( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int silver;
    int total;

    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];

    if ( argument[0] == '\0' )
    {
	send_to_char( "Junk what?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if (!str_cmp("all",arg) || !str_prefix("all.",arg))
    {
        OBJ_DATA *obj_next;
        bool found = FALSE;
	total = 0;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;

            if (arg[3] != '\0' && !is_name(&arg[4],obj->name))
                continue;

            if (  (!CAN_WEAR(obj,ITEM_TAKE)  || IS_OBJ_STAT(obj,ITEM_NO_SAC))
	    ||  IS_OBJ_STAT(obj,ITEM_AQUEST) || IS_OBJ_STAT(obj,ITEM_FORGED)
	    || (!can_drop_obj(ch,obj))
	    || obj->wear_loc != WEAR_NONE
            || (obj->item_type == ITEM_CORPSE_PC && obj->contains))
                continue;

            silver = UMAX(1,obj->level * 3);

            if (obj->item_type != ITEM_CORPSE_NPC &&
                obj->item_type != ITEM_CORPSE_PC)
                silver = UMIN(silver,obj->cost);

                found = TRUE;
		total += silver;
		obj_from_char( obj );
                extract_obj( obj );

        }
        if (found)
	{
            if (IS_SET(ch->act,PLR_AUTOSPLIT))
            {
                members = 0;
                for (gch = ch->in_room->people;gch;gch = gch->next_in_room)
                    if (is_same_group(ch,gch))
                        members++;
                    if (members > 1 && total > 1)
                    {
                        sprintf(buf,"%d",total);
                        do_split( ch, buf);
                    }
	    }
	    sprintf(buf,"$G gives you {g%d{x silver for your sacrifices.",total);
	    act(buf,ch,NULL,NULL,TO_CHAR,POS_RESTING);
	    act("$n sacrifices some things $e carries to $G.",ch,NULL,NULL,TO_ROOM,POS_RESTING);
	    add_cost(ch,total,VALUE_SILVER);
            wiznet("$N sends up everything in that room as a burnt offering.",ch,obj,WIZ_SACCING,0,0);
	}
        else
            send_to_char("You carry nothing sacrificable.\n\r",ch);
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( obj->item_type == ITEM_CORPSE_PC )
    {
        if (obj->contains)
        {
           act("$G wouldn't like that.",ch,NULL,NULL,TO_CHAR,POS_RESTING);
           return;
        }
    }

    if ( !CAN_WEAR(obj, ITEM_TAKE) || IS_OBJ_STAT(obj, ITEM_NO_SAC)
    ||  IS_OBJ_STAT(obj,ITEM_AQUEST) || IS_OBJ_STAT(obj,ITEM_FORGED) )
    {
        act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR,POS_RESTING );
        return;
    }

    silver = UMAX(1,obj->level * 3);

    if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
        silver = UMIN(silver,obj->cost);

    if (silver == 1)
        act("$G gives you one silver coin for your sacrifice.",ch,NULL,NULL,TO_CHAR,POS_RESTING);
    else
    {
        sprintf(buf,"$G gives you {g%d{x silver coins for your sacrifice of %s.",
                silver, obj->short_descr);
        act(buf,ch,NULL,NULL,TO_CHAR,POS_RESTING);
    }

    add_cost(ch,silver,VALUE_SILVER);

   if (IS_SET(ch->act,PLR_AUTOSPLIT) )
   { /* AUTOSPLIT code */
        members = 0;
        for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
        {
            if ( is_same_group( gch, ch ) )
            members++;
        }

        if ( members > 1 && silver > 1)
        {
            sprintf(buffer,"%d",silver);
            do_split(ch,buffer);
        }
    }

    act( "$n sacrifices $p to $G.", ch, obj, NULL, TO_ROOM,POS_RESTING );
    wiznet("$N sends up $p as a burnt offering.",
           ch,obj,WIZ_SACCING,0,0);
    obj_from_char( obj );
    extract_obj( obj );
    return;
}

bool check_objpktest( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( obj->dropped_by == NULL
    ||   obj->dropped_by == ch
    ||   (!IS_NPC(ch) && ch->pcdata->opponent == NULL)
    ||   (IS_NPC(ch) && (ch->master == NULL
		     ||  ch->master->pcdata->opponent == NULL))
    ||   (ch->master != NULL && IS_NPC(ch->master)) )
        return TRUE;

    if (IS_NPC(ch))
    {
	if ( ch->master->pcdata->opponent == obj->dropped_by )
	    return TRUE;

	if (!can_pk(ch->master->pcdata->opponent,obj->dropped_by))
	{
	    printf_to_char(ch->master,"Object: %s was dropped by %s, whom may not assist you in fighting %s.\n\r", obj->short_descr, obj->dropped_by->name, ch->master->pcdata->opponent->name);
	    printf_to_char(ch->master,"You are therefore, not permitted to pick up %s.\n\r", obj->short_descr);
	    return FALSE;
	}
    }

    if (ch->pcdata->opponent == obj->dropped_by)
	return TRUE;

    if (!can_pk(ch->pcdata->opponent,obj->dropped_by))
    {
	printf_to_char(ch,"Object: %s was dropped by %s, whom may not assist you in fighting %s.\n\r", obj->short_descr, obj->dropped_by->name, ch->pcdata->opponent->name);
	printf_to_char(ch,"You are therefore, not permitted to pick up %s.\n\r", obj->short_descr);
	return FALSE;
    }
    return TRUE;
}

bool obj_droptest( CHAR_DATA *ch, OBJ_DATA *obj )
{
    OBJ_MULTI_DATA *mult;
    char buf[MAX_STRING_LENGTH];

    if ( !mud_stat.multilock
    ||   obj->multi_data == NULL )
	return TRUE;

    if (IS_NPC(ch) && ch->master != NULL)
    {
	if (check_allow(ch->master->pcdata->socket,ALLOW_ITEMS))
	    return TRUE;

	for ( mult = obj->multi_data; mult != NULL; mult = mult->next )
	{
	    if ( mult->dropper == NULL
	    ||   mult->socket == NULL
	    ||   !str_cmp(mult->dropper,ch->master->name) )
		continue;

	    if (!str_cmp(mult->socket,ch->master->pcdata->socket))
	    {
		send_to_char("{RNo Multiplaying {zDumbass{x{R!{x\n\r",ch->master);
		sprintf(buf,"BUG: Attempted OBJ_MULTIPLAY: %s [%d] -- %s|%s",
		    ch->master->pcdata->socket, obj->pIndexData->vnum, ch->name, mult->dropper );
		log_string(buf);
		wiznet(buf,NULL,NULL,WIZ_OTHER,0,0);
		append_file( ch, "../log/multiplay.txt", buf );
		return FALSE;
	    }
	}
    }

    if ( IS_NPC(ch) || check_allow(ch->pcdata->socket,ALLOW_ITEMS) )
	return TRUE;

    for ( mult = obj->multi_data; mult != NULL; mult = mult->next )
    {
	if ( mult->dropper == NULL
	||   mult->socket == NULL
	||   !str_cmp(mult->dropper,ch->name) )
	    continue;

	if (!str_cmp(mult->socket,ch->pcdata->socket))
	{
	    send_to_char("{RNo Multiplaying {zDumbass{x{R!{x\n\r",ch);
	    sprintf(buf,"BUG: Attempted OBJ_MULTIPLAY: %s [%d] -- %s|%s",
		ch->pcdata->socket, obj->pIndexData->vnum, ch->name, mult->dropper );
	    log_string(buf);
	    wiznet(buf,NULL,NULL,WIZ_OTHER,0,0);
	    append_file( ch, "../log/multiplay.txt", buf );
	    return FALSE;
	}
    }

    return TRUE;
}

void show_apply_flags( AFFECT_DATA *paf, BUFFER *output, CHAR_DATA *ch, bool enchanted )
{
    char buf[MAX_STRING_LENGTH], final[MAX_STRING_LENGTH];

    for ( ; paf != NULL; paf = paf->next )
    {
	if ( IS_IMMORTAL( ch )
	||   ( paf->location != APPLY_NONE && paf->modifier != 0 ) )
	{
	    if ( paf->where == TO_DAM_MODS )
	    {
		sprintf( buf, "Modifies damage from {t%s {qby {t%d{q%%",
		    paf->location == DAM_ALL ? "all" :
		    damage_mod_table[paf->location].name, paf->modifier );
		sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
		add_buf( output, final );
	    } else {
		sprintf( buf, "Affects {t%s {qby {t%d{q",
		    flag_string( apply_flags, paf->location ), paf->modifier );

		if ( paf->bitvector )
		{
		    switch( paf->where )
		    {
			case TO_AFFECTS:
			    sprintf( final, " with {t%s {qaffect",
				flag_string( affect_flags, paf->bitvector ) );
			    break;

			case TO_OBJECT:
			    sprintf( final, " with {t%s {qobject flag",
				flag_string( extra_flags, paf->bitvector ) );
			    break;

			case TO_WEAPON:
			    sprintf( final, " with {t%s {qweapon flag",
				flag_string( weapon_type2, paf->bitvector ) );
			    break;

			case TO_SHIELDS:
			    sprintf( final, " with {t%s {qshield",
				flag_string( shield_flags, paf->bitvector ) );
			    break;

			default:
			    sprintf( final, " with unknown bit {t%d{q: %d",
				paf->where, paf->bitvector );
			    break;
		    }

		    strcat( buf, final );
		}

		if ( paf->duration > -1 && enchanted )
		    sprintf( final, " for {t%d {qhours.", paf->duration );
		else
		    sprintf( final, "." );
		strcat( buf, final );

		sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
		add_buf( output, final );
	    }
	}
    }
}

BUFFER * display_stats( OBJ_DATA *obj, CHAR_DATA *ch, bool contents )
{
    BUFFER *output = new_buf( );
    char *flags;
    char buf[MAX_STRING_LENGTH], final[MAX_STRING_LENGTH];
    bool place = FALSE;
    sh_int pos;

    sprintf( buf, "( {t%s {s)", obj->short_descr );
    str_replace( buf, "{x", "{t" );
    str_replace( buf, "{0", "{t" );

    while ( strlen_color( buf ) < 77 )
    {
	if ( place )
	    strcat( buf, "=" );
	else
	{
	    sprintf( final, "=%s", buf );
	    strcpy( buf, final );
	}
	place = !place;
    }

    sprintf( final, "{s %s\n\r", buf );
    add_buf( output, final );

    if ( IS_IMMORTAL( ch ) )
    {
	OBJ_DATA *in_obj;

	if ( IS_TRUSTED( ch, MAX_LEVEL ) )
	{
	    if ( obj->loader != NULL )
	    {
		sprintf( buf, "Loaded: {t%s", obj->loader );
		sprintf( final, "| {q%s{s|\n\r", end_string( buf, 76 ) );
		add_buf( output, final );
	    }

	    if ( obj->disarmed_from != NULL )
	    {
		sprintf( buf, "Disarmed From: {t%s",
		    obj->disarmed_from->name );
		sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
		add_buf( output, final );
	    }

	    if ( obj->multi_data != NULL )
	    {
		OBJ_MULTI_DATA *obj_mult;

		for ( obj_mult = obj->multi_data; obj_mult != NULL; obj_mult = obj_mult->next )
		{
		    sprintf( buf, "Dropped: {t%s{q@{t%s{q: {t%d",
			obj_mult->dropper, obj_mult->socket, obj_mult->drop_timer );
		    sprintf( final, "| {q%s{s|\n\r", end_string( buf, 76 ) );
		    add_buf( output, final );
		}
	    }
	}

	sprintf( buf, "Vnum: {t%-5d       {qTimer: {t%-5d      {qResets: {t%-5d",
	    obj->pIndexData->vnum, obj->timer, obj->pIndexData->reset_num );
	sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	add_buf( output, final );

	sprintf( buf, "Area: {t%s", obj->pIndexData->area->name );
	sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	add_buf( output, final );

	sprintf( buf, "Value[{t0{q]: {t%-7d {qValue[{t1{q]: {t%-7d {qValue[{t2{q]: {t%-7d",
	    obj->value[0], obj->value[1], obj->value[2] );
	sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	add_buf( output, final );

	sprintf( buf, "Value[{t3{q]: {t%-7d {qValue[{t4{q]: {t%-7d {qWeight: {t%d{q/{t%d{q/{t%d {q(10th pounds)",
	    obj->value[3], obj->value[4],
	    obj->weight, get_obj_weight( obj ),get_true_weight(obj) );
	sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	add_buf( output, final );

	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	sprintf( buf, "In Room: {t%-5d    {qIn Object: {t%s",
	    in_obj->in_room != NULL ? in_obj->in_room->vnum :
	    in_obj->carried_by == NULL ? 0 :
	    in_obj->carried_by->in_room != NULL ?
	    in_obj->carried_by->in_room->vnum : 0,
	    obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr );
	sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	add_buf( output, final );

	sprintf( buf, "Wear Loc: {t%-3d     {qCarried by: {t%s",
	    obj->wear_loc,
	    in_obj->carried_by == NULL ? "(none)" :
	    PERS( in_obj->carried_by, ch ) );
	sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	add_buf( output, final );

	if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
	{
	    EXTRA_DESCR_DATA *ed;

	    sprintf( buf, "Extra Description Keywords: '{t" );

	    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	    {
		strcat( buf, ed->keyword );
		if ( ed->next != NULL )
		    strcat( buf, " " );
	    }

	    for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	    {
		strcat( buf, ed->keyword );
		if ( ed->next != NULL )
		    strcat( buf, " " );
	    }

	    strcat( buf, "{q'" );
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );
	}

	add_buf( output, " =============================================================================\n\r" );
    }

    if ( obj->pIndexData->history != NULL
    &&   obj->pIndexData->history[0] != '\0' )
    {
	char *rdesc;

	buf[0] = '\0';
	final[0] = '\0';

	for ( rdesc = obj->pIndexData->history; *rdesc; rdesc++ )
	{
	    if ( *rdesc != '{' && *rdesc != '\n' )
	    {
		sprintf( buf, "%c", rdesc[0] );
		strcat( final, buf );
	    }

	    else if ( *rdesc != '\n' )
	    {
		sprintf( buf,"%c%c", rdesc[0], rdesc[1] );
		strcat( final, buf );
		rdesc++;
	    }

	    if ( *rdesc == '\n' && *(rdesc + 1) )
	    {
		sprintf( buf,"| {q%s {s|\n\r",
		    end_string( final, 75 ) );
		add_buf( output, buf );

		buf[0] = '\0';
		final[0] = '\0';
		rdesc++;
	    }
	}

	add_buf( output, " =============================================================================\n\r" );
    }

    sprintf( buf, "Keywords: {t%s", obj->name );
    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
    add_buf( output, final );

    flags = obj->description;
    flags = length_argument( flags, buf, 70 );

    sprintf( final, "| {qLong: {t%s{s|\n\r", end_string( buf, 70 ) );
    str_replace( final, "{x", "{t" );
    str_replace( final, "{0", "{t" );
    add_buf( output, final );

    while ( *flags != '\0' )
    {
	flags = length_argument( flags, buf, 70 );
	sprintf( final, "|       {t%s{s|\n\r", end_string( buf, 70 ) );
	str_replace( final, "{x", "{t" );
	str_replace( final, "{0", "{t" );
	add_buf( output, final );
    }

    sprintf( buf, "Type: {t%s", flag_string( type_flags, obj->item_type ) );
    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
    add_buf( output, final );

    flags = flag_string( extra_flags, obj->extra_flags );
    flags = length_argument( flags, buf, 69 );

    sprintf( final, "| {qFlags: {t%-69s{s|\n\r", buf );
    add_buf( output, final );

    while ( *flags != '\0' )
    {
	flags = length_argument( flags, buf, 69 );
	sprintf( final, "|        {t%-69s{s|\n\r", buf );
	add_buf( output, final );
    }

    sprintf( buf, "Location: {t%s", flag_string( wear_flags, obj->wear_flags ) );
    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
    add_buf( output, final );

    sprintf( buf, "Size: {t%d {q[{t%s{q]",
	obj->size, flag_string( size_flags, obj->size ) );
    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
    add_buf( output, final );
    
    sprintf( buf, "Weight: {t%d{q, Value: {t%d{q, Level: {t%d",
	obj->weight, obj->cost, obj->level );
    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
    add_buf( output, final );

    flags = return_classes( obj->pIndexData->class_can_use );
    flags = length_argument( flags, buf, 60 );

    sprintf( final, "{s| {qClass Restrict: {t%-60s{s|\n\r", buf );
    add_buf( output, final );

    while( *flags != '\0' )
    {
	flags = length_argument( flags, buf, 60 );
	sprintf( final, "{s|                 {t%-60s{s|\n\r", buf );
	add_buf( output, final );
    }

    switch ( obj->item_type )
    {
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
	    if ( obj->pIndexData->targets != 0 )
		sprintf( buf, "Has {t%d%% {qchance of supplying any {t%d {qof the level {t%d {qspells of:",
		    obj->success, obj->pIndexData->targets, obj->value[0] );
	    else
		sprintf( buf, "Has {t%d%% {qchance of supplying level {t%d {qspells of:",
		    obj->success, obj->value[0] );
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );

	    buf[0] = '\0';

	    for ( pos = 1; pos <= 4; pos++ )
	    {
		if ( obj->value[pos] > 0 && obj->value[pos] < maxSkill )
		{
		    strcat( buf, " {q'{t" );
		    strcat( buf, skill_table[obj->value[pos]].name );
		    strcat( buf, "{q'" );
		}
	    }

	    strcat( buf, "." );
	    sprintf( final, "| %s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );
	    break;

	case ITEM_FURNITURE:
	    sprintf( buf, "Flags: {t%s",
		flag_string( furniture_flags, obj->value[2] ) );
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );

	    sprintf( buf, "Maximum People: {t%d", obj->value[0] );
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );

	    sprintf( buf, "Maximum Weight: {t%d", obj->value[1] );
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );

	    sprintf( buf, "Healing rates {t-- {qHp: {t%d{q, Mana: {t%d",
		obj->value[3], obj->value[4] );
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );
	    break;

	case ITEM_WAND:
	case ITEM_STAFF:
	    sprintf( buf, "Chance: {t%d%%{q, Charges: {t%d{q, Charge level: {t%d{q, Spell: ",
		obj->success, obj->value[2], obj->value[0] );

	    if ( obj->value[3] >= 0 && obj->value[3] < maxSkill )
	    {
		strcat( buf, "'{t" );
		strcat( buf, skill_table[obj->value[3]].name );
		strcat( buf, "{q'" );
	    }

	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );

	    if ( obj->item_type == ITEM_STAFF
	    &&   obj->pIndexData->targets != 0 )
	    {
		sprintf( buf, "Max Targets: {t%d",
		    obj->pIndexData->targets );
		sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
		add_buf( output, final );
	    }
	    break;

	case ITEM_DRINK_CON:
	    sprintf( buf, "It holds {t%s{q-colored {t%s{q.",
		liq_table[obj->value[2]].liq_color,
		liq_table[obj->value[2]].liq_name );
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );
	    break;

	case ITEM_CONTAINER:
	case ITEM_PIT:
	    sprintf( buf, "Capacity: {t%d#{q, Maximum Weight: {t%d#",
		obj->value[0], obj->value[3] );
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );

	    if ( obj->value[4] != 100 )
	    {
		sprintf( buf, "Weight Multiplier: {t%d%%", obj->value[4] );
		sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
		add_buf( output, final );
	    }

	    sprintf( buf, "Flags: {t%s",
		flag_string( container_flags, obj->value[1] ) );
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );
	    break;

	case ITEM_WEAPON:
	    sprintf( buf, "Weapon type is %s.",
		flag_string( weapon_class, obj->value[0] ) );
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );

	    sprintf( buf, "Damage is {t%d {qd {t%d {q(average {t%d{q).",
		obj->value[1], obj->value[2],
		( 1 + obj->value[2] ) * obj->value[1] / 2 );
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );

	    sprintf( buf, "Damage message: {t%s",
		attack_table[obj->value[3]].noun );
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );

	    if ( obj->value[4] )
	    {
		sprintf( buf, "Flags: {t%s",
		    flag_string( weapon_type2, obj->value[4] ) );
		sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
		add_buf( output, final );
	    }
	    break;

	case ITEM_ARMOR:
	    sprintf( buf, "Armor Values: Pierce: {t%d{q, Bash: {t%d{q, Slash: {t%d{q, Magic: {t%d{q.",
		obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );
	    break;
	    
	case ITEM_TRAP:
	    if ( obj->value[0] == TRAP_DAMAGE )
	        sprintf( buf, "Trap type: {t%s {q({t%s{q)",
	            capitalize( trap_type_table[obj->value[0]].name ),
	            damage_mod_table[obj->value[1]].name );
	    else
	        sprintf( buf, "Trap type: {t%s{q",
	            capitalize( trap_type_table[obj->value[0]].name ) );
	    
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );
	    
	    sprintf( buf, "Trap timer: {t%d", obj->value[4] );
	    sprintf( final, "| {q%s {s|\n\r", end_string( buf, 75 ) );
	    add_buf( output, final );
	    
	    break;
    }

    if ( !obj->enchanted )
	show_apply_flags( obj->pIndexData->affected, output, ch, FALSE );
    show_apply_flags( obj->affected, output, ch, TRUE );

    sprintf( buf, "( {x%s {s)", obj->short_descr );
    str_replace( buf, "{x", "{t" );
    str_replace( buf, "{0", "{t" );

    place = FALSE;
    while ( strlen_color( buf ) < 77 )
    {
	if ( place )
	    strcat( buf, "=" );
	else
	{
	    sprintf( final, "=%s", buf );
	    strcpy( buf, final );
	}
	place = !place;
    }

    sprintf( final, "{s %s{x\n\r", buf );
    add_buf( output, final );

    if ( contents )
    {
	if ( obj->item_type == ITEM_CONTAINER
	&&   IS_SET( obj->value[1], CONT_CLOSED ) )
	{
	    sprintf( buf, "\n\r{x%s {Aappears to be closed.\n\r",
		obj->short_descr );
	    add_buf( output, buf );
	}

	if ( ( obj->item_type == ITEM_CONTAINER
	&&     !IS_SET( obj->value[1], CONT_CLOSED ) )
	||   obj->item_type == ITEM_CORPSE_NPC
	||   obj->item_type == ITEM_CORPSE_PC )
	{
	    BUFFER *stats = show_list_to_char( obj->contains, ch, TRUE, TRUE );

	    sprintf( buf, "\n\r{x%s {Aholds{w:{x\n\r", obj->short_descr );
	    add_buf( output, buf );
	    add_buf( output, stats->string );
	    free_buf( stats );
	}
    }

    return output;
}

void sharpen_weapon( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA af;

    af.where	= TO_WEAPON;
    af.type		= gsn_sharpen;
    af.level	= ch->level;
    af.dur_type	= DUR_TICKS;
    af.duration	= ch->level/4;
    af.location	= 0;
    af.modifier	= 0;
    af.bitvector= WEAPON_SHARP;
    affect_to_obj(obj,&af);

    act("$n sharpens $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
    act("You sharpen $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
    check_improve(ch,gsn_sharpen,TRUE,3);
}

void do_sharpen(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    int skill;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Sharpen what item?\n\r", ch );
	return;
    }

    obj = get_obj_list(ch,argument,ch->carrying);

    if ( obj == NULL )
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if ( (skill = get_skill(ch,gsn_sharpen)) < 1 )
    {
	send_to_char("You sliced your finger!\n\r",ch);
	return;
    }

    if ( obj->item_type != ITEM_WEAPON )
    {
	send_to_char("You can only sharpen weapons!\n\r",ch);
	return;
    }

    if ( IS_WEAPON_STAT(obj,WEAPON_SHARP) )
    {
	act("$p is already sharpened.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	return;
    }

    if ( number_percent() < skill )
    {
	sharpen_weapon( ch, obj );
    } else {
	act("You fail to sharpen $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	check_improve(ch,gsn_sharpen,FALSE,3);
    }

    WAIT_STATE(ch,skill_table[gsn_sharpen].beats);
    return;
}

void hone_weapon( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA af;

    af.where	= TO_WEAPON;
    af.type	= gsn_hone;
    af.level	= ch->level;
    af.dur_type	= DUR_TICKS;
    af.duration	= ch->level/6;
    af.location	= 0;
    af.modifier	= 0;
    af.bitvector= WEAPON_VORPAL;
    affect_to_obj( obj, &af );

    act( "$n hones $p.", ch, obj, NULL, TO_ROOM, POS_RESTING );
    act( "You hone $p.", ch, obj, NULL, TO_CHAR, POS_RESTING );
    check_improve( ch, gsn_hone, TRUE, 3 );
}

void do_hone( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int skill;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Hone what item?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_list( ch, argument, ch->carrying ) ) == NULL )
    {
	send_to_char( "You don't have that item.\n\r", ch );
	return;
    }

    if ( ( skill = get_skill( ch, gsn_hone ) ) < 1 )
    {
	send_to_char( "You sliced your finger!\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_WEAPON )
    {
	send_to_char( "You can only hone weapons!\n\r", ch );
	return;
    }

    if ( IS_WEAPON_STAT( obj, WEAPON_VORPAL ) )
    {
	act( "$p is already honed.", ch, obj, NULL, TO_CHAR, POS_RESTING );
	return;
    }

    if ( number_percent( ) < skill )
    {
	hone_weapon( ch, obj );
    } else {
	act( "You fail to hone $p.", ch, obj, NULL, TO_CHAR, POS_RESTING );
	check_improve( ch, gsn_hone, FALSE, 3 );
    }

    WAIT_STATE( ch, skill_table[gsn_hone].beats );
    return;
}

void do_revive( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mobile;
    OBJ_DATA *obj, *token = NULL;
    char buf[MAX_STRING_LENGTH];
    bool free = FALSE, found = FALSE;
    sh_int count = 0;

    if ( IS_NPC(ch) )
	return;

    for ( mobile = ch->in_room->people; mobile != NULL; mobile = mobile->next_in_room )
    {
	if ( IS_NPC(mobile)
	&&   mobile->spec_fun == spec_lookup( "spec_mortician" ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char("You can't find a mortician.\n\r",ch);
        return;
    }

    if ( ch->pcdata->tier == 1 && ch->level <= 50 )
	free = TRUE;

    if ( !free )
    {
	for ( token = ch->carrying; token != NULL; token = token->next_content )
	{
	    if ( token->pIndexData->vnum == 1206 )
	    {
		found = FALSE;
		break;
	    }
	}

	if ( found )
	{
	    do_say(mobile,"I require a {cr{Ce{cv{Ci{cv{Ca{cl {Ds{wt{Do{wn{De{S.");
	    return;
	}
    }

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->item_type == ITEM_CORPSE_PC
	&&   obj->carried_by == NULL
	&&   obj->in_room != NULL )
	{
	    char arg[MAX_INPUT_LENGTH];

	    one_argument(obj->name,arg);

	    if ( !str_cmp(arg,ch->name) )
	    {
		count++;
		obj_from_room( obj );
		obj_to_char( obj, ch );
	    }
	}

    }

    if ( count == 0 )
    {
	sprintf(buf,"I'm sorry %s, but I could not locate any corpses in your name.",
	    ch->name);
	do_say(mobile,buf);
	return;
    }

    if ( !free )
    {
	obj_from_char( token );
	extract_obj( token );
    }

    sprintf(buf,"%s, I found %d corpse%s with your name branded on %s.",
	ch->name, count, count == 1 ? "" : "s", count == 1 ? "it" : "them");
    do_say(mobile,buf);
    return;
}

void do_slip( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    sh_int skill;

    argument = one_argument( argument, arg1 );

    if ( (skill = get_skill(ch,gsn_slip)) <=0 )
    {
	send_to_char("You have not been trained to do that.\n\r",ch);
	return;
    }

    if ( arg1[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char("Slip what to whom?\n\r",ch);
	return;
    }

    if ( (obj = get_obj_carry(ch,arg1)) == NULL )
    {
	send_to_char("You are not carrying that.\n\r",ch);
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char("You can't give items you are currently wearing.\n\r",ch);
	return;
    }

    if ( (victim = get_char_room(ch,NULL,argument)) == NULL )
    {
	send_to_char("They are not here.\n\r",ch);
	return;
    }

    if ( victim == ch )
    {
	send_to_char("What would that accomplish?\n\r",ch);
	return;
    }

    if ( !can_drop_obj(ch,obj) )
    {
	send_to_char("You can't let go of it!\n\r",ch);
	return;
    }

    if ( obj->pIndexData->vnum == OBJ_VNUM_VOODOO && !IS_IMMORTAL(ch) )
    {
	send_to_char("You can't slip voodoo dolls around.\n\r",ch);
	return;
    }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
	act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR, POS_RESTING );
	return;
    }

    if ( get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w( victim ) )
    {
	act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR, POS_RESTING );
	return;
    }

    if ( !check_pktest(ch,victim)
    ||   is_safe(ch,victim) )
	return;

    WAIT_STATE(ch, skill_table[gsn_slip].beats);

    MOBtrigger = FALSE;

    if ( number_percent() > 9 * skill / 10 )
    {
	affect_strip(ch,gsn_obfuscate);
	affect_strip(ch,gsn_camouflage);
	affect_strip(ch,gsn_forest_meld);

	switch ( number_range(1,3) )
	{
	    case 1:
		obj_from_char( obj );
		obj_to_char( obj, victim );
		act( "You slip $p into $N's inventory.",
		    ch, obj, victim, TO_CHAR, POS_RESTING );
		act( "You notice $n slipping $p to $N.",
		    ch, obj, victim, TO_NOTVICT, POS_RESTING );
		act( "You notice $n slipping $p into your inventory.",
		    ch, obj, victim, TO_VICT, POS_RESTING );
		break;
	    case 2:
		act( "You fail so badly to slip $p to $N, that you drop it.",
		    ch, obj, victim, TO_CHAR, POS_RESTING );
		act( "$n fails so badly to slip $p to $N, that $e drops it.",
		    ch, obj, victim, TO_NOTVICT, POS_RESTING );
		act( "You notice $n attempting to slip $p into your inventory and slap $s hand!",
		    ch, obj, victim, TO_VICT, POS_RESTING );
		do_drop(ch,arg1);
		break;
	    case 3:
		act( "You fail to slip $p into $N's inventory.",
		    ch, obj, victim, TO_CHAR, POS_RESTING );
		act( "$n fails to slip $p to $N.",
		    ch, obj, victim, TO_NOTVICT, POS_RESTING );
		act( "$n fails to slip $p into your inventory.",
		    ch, obj, victim, TO_VICT, POS_RESTING );
		break;
	}
	check_improve(ch,gsn_slip,FALSE,2);
    } else {
	obj_from_char( obj );
	obj_to_char( obj, victim );

	act( "You slip $p into $N's inventory.",
	    ch, obj, victim, TO_CHAR, POS_RESTING );
	check_improve(ch,gsn_slip,TRUE,2);
    }

    MOBtrigger = TRUE;

    return;
}

int stone_lookup( char *argument )
{
    if ( !str_prefix( argument, "adamantium" ) )	return FORGE_STONE_ADAMANTIUM;
    else if ( !str_prefix( argument, "mithril" ) )	return FORGE_STONE_MITHRIL;
    else if ( !str_prefix( argument, "titanium" ) )	return FORGE_STONE_TITANIUM;
    else if ( !str_prefix( argument, "steel" ) )	return FORGE_STONE_STEEL;
    else if ( !str_prefix( argument, "bronze" ) )	return FORGE_STONE_BRONZE;
    else						return -1;
}

void do_forge( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *forger;
    OBJ_DATA *obj, *stone, *stone_next;
    OBJ_INDEX_DATA *pObjIndex = NULL;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    bool found = FALSE;
    int count = 0, lvl, vnum, stone_vnum;

    argument = one_argument( argument, arg1 );

    if ( IS_NPC(ch) )
	return;

    for ( forger = ch->in_room->people; forger != NULL; forger = forger->next_in_room )
    {
        if ( !IS_NPC(forger) )
            continue;

        if ( forger->spec_fun == spec_lookup("spec_forger") )
	{
            found = TRUE;
            break;
	}
    }

    if ( !found )
    {
	send_to_char("This requires a forging blacksmith.\n\r",ch);
        return;
    }

    if ( arg1[0] == '\0' )
    {
	send_to_char("Syntax:\n\r"
		     "  forge list <stone_type|all>\n\r"
		     "  forge identify <item>\n\r"
		     "  forge <stone_type> <item>\n\r", ch );
	return;
    }

    if ( !str_prefix(arg1,"list") )
    {
	BUFFER *final = new_buf();

	if ( argument[0] == '\0' || !str_cmp( argument, "all") )
	    lvl = -1;
	else if ( ( lvl = stone_lookup( argument ) ) == -1 )
	{
	    send_to_char( "Invalid stone type.\n\r", ch );
	    return;
	}

	add_buf(final,"[Level] [Stones] Item\n\r");

        for ( vnum = 0; count < top_obj_index; vnum++ )
        {
            if ( (pObjIndex = get_obj_index(vnum) ) != NULL )
            {
                count++;

                if ( IS_OBJ_STAT(pObjIndex,ITEM_FORGED)
		&&   (lvl == -1 || pObjIndex->forge_vnum == lvl) )
                {
		    sprintf(buf,"[ %3d ] [  %2d  ] %s\n\r",
			pObjIndex->level, pObjIndex->forge_count,
			pObjIndex->short_descr);
		    add_buf(final,buf);
		}
	    }
	}

	page_to_char(final->string,ch);
	free_buf(final);
	return;
    }

    if ( arg1[0] == '\0' || argument[0] == '\0' )
    {
	do_forge(ch,"");
	return;
    }

    if ( !str_prefix( arg1, "identify" ) )
    {
	found = FALSE;
	for ( vnum = 0; count < top_obj_index; vnum++ )
	{
	    if ( ( pObjIndex = get_obj_index( vnum )) != NULL )
	    {
		count++;

		if ( IS_OBJ_STAT( pObjIndex, ITEM_FORGED )
		&&   is_name( argument, pObjIndex->name ) )
		{
		    found = TRUE;
		    break;
		}
	    }
	}

	if ( !found )
	{
	    send_to_char("That item is not available.\n\r",ch);
	    return;
	}

	if ( ( obj = create_object( pObjIndex ) ) != NULL )
	{
	    BUFFER * final = display_stats( obj, ch, FALSE );
	    page_to_char( final->string, ch );
	    free_buf( final );
	    extract_obj( obj );
	}
	
	return;
    }

    if ( (stone_vnum = stone_lookup( arg1 )) == -1 )
    {
	send_to_char("That is not a valid stone type.\n\r", ch );
	return;
    }

    found = FALSE;
    for ( vnum = 0; count < top_obj_index; vnum++ )
    {
	if ( (pObjIndex = get_obj_index( vnum )) != NULL )
	{
	    count++;

	    if ( IS_OBJ_STAT( pObjIndex, ITEM_FORGED )
	    &&   pObjIndex->forge_vnum == stone_vnum
	    &&   is_name( argument, pObjIndex->name ) )
	    {
		found = TRUE;
		break;
	    }
	}
    }

    if ( !found )
    {
	send_to_char("That item is not available.\n\r",ch);
	return;
    }

    lvl = 0;
    for ( stone = ch->carrying; stone != NULL; stone = stone->next_content )
    {
	if ( stone->pIndexData->vnum == stone_vnum )
	    lvl++;

	if ( lvl >= pObjIndex->forge_count )
	    break;
    }

    if ( lvl < pObjIndex->forge_count )
    {
	send_to_char("You don't have enough stones.\n\r",ch);
	return;
    }

    obj = create_object( pObjIndex );
    set_obj_loader( ch, obj, IS_IMMORTAL( ch ) ? "IFRG" : "FORG" );
    obj_to_char(obj,ch);

    lvl = 0;
    for ( stone = ch->carrying; stone != NULL; stone = stone_next )
    {
	stone_next = stone->next_content;

	if ( stone->pIndexData->vnum == stone_vnum )
	{
	    obj_from_char( stone );
	    extract_obj( stone );
	    lvl++;
	}

	if ( lvl >= pObjIndex->forge_count )
	    break;
    }

    sprintf(buf,"$N takes %d stones and forges you $p!",lvl);
    act(buf,ch,obj,forger,TO_CHAR,POS_RESTING);
    sprintf(buf,"$N takes %d stones and forges $n $p!",lvl);
    act(buf,ch,obj,forger,TO_ROOM,POS_RESTING);
}

char * get_obj_storage( CHAR_DATA *ch, OBJ_DATA *obj, bool multi, int pos )
{
    OBJ_DATA *prev;
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	sprintf( buf, "%s: you can't carry that many items.\n\r",
	    obj->short_descr );
	return buf;
    }

    if ( get_carry_weight( ch ) + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	sprintf( buf, "%s: you can't carry that much weight.\n\r",
	    obj->short_descr );
	return buf;
    }

    if ( ch->pcdata->storage_list[pos] == obj )
	ch->pcdata->storage_list[pos] = obj->next_content;
    else
    {
	for ( prev = ch->pcdata->storage_list[pos]; prev != NULL; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}
    }

    if ( !multi || !IS_SET( ch->configure, CONFIG_LOOT_COMBINE ) )
    {
	sprintf( buf, "You get %s from storage.\n\r", obj->short_descr );
	if ( !multi )
	    act( "$n gets $p from storage.", ch, obj, NULL, TO_ROOM, POS_RESTING );
    }

    obj->next_content = NULL;
    obj_to_char( obj, ch );

    return buf;
}

sh_int count_objects( OBJ_DATA *obj )
{
    OBJ_DATA *pos;
    sh_int count = 0;

    for ( pos = obj; pos != NULL; pos = pos->next_content )
    {
	count++;
	if ( pos->contains != NULL )
	    count += count_objects( pos->contains );
    }

    return count;
}

void do_storage( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    OBJ_DATA *obj, *obj_next;
    BUFFER *final;
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int slot;

    argument = one_argument ( argument, arg );

    if ( IS_NPC( ch ) )
	return;

    for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
    {
        if ( mob->pIndexData && mob->pIndexData->bank_branch > 0 )
            break;
    }

    if ( mob == NULL || ( ( slot = mob->pIndexData->bank_branch ) <= 0 )
    || slot > MAX_BANK )
    {
        send_to_char( "Does this look like the bank to you?\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax: storage list      \n\r"
		      "        storage count     \n\r"
		      "        storage buy (500 quest points per 5 slots)\n\r"
		      "        storage add <item>\n\r"
		      "        storage get <item>\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "list" ) )
    {
	BUFFER *final = show_list_to_char( ch->pcdata->storage_list[slot], ch, TRUE, TRUE );
	send_to_char( "Your storage box contains:\n\r", ch );
	page_to_char( final->string, ch );
	free_buf( final );
	return;
    }


    if ( !str_prefix( arg, "count" ) )
    {
	char buf[MAX_INPUT_LENGTH];

	sprintf( buf, "You currently have %d object%s in storage.\n\r"
		      "Your limit is %d items.\n\r",
	    count_objects( ch->pcdata->storage_list[slot] ),
	    count_objects( ch->pcdata->storage_list[slot] ) == 1 ? "" : "s",
	    ch->pcdata->max_storage[slot] + STORAGE_BONUS );
	send_to_char( buf, ch );
	return;
    }

    if ( !str_cmp( arg, "buy" ) )
    {
	if ( ch->pcdata->questpoints < 500 )
	{
	    send_to_char( "It requires 500 questpoints for this.\n\r", ch );
	    return;
	}

	if ( ch->pcdata->max_storage[slot] >= 30 )
	{
	    send_to_char( "You can't have that many slots for storage here.\n\r", ch );
	    return;
	}

	sprintf( buf, "Your storage count has been increased from %d to %d items.\n\r",
	    ch->pcdata->max_storage[slot] + STORAGE_BONUS,
	    ch->pcdata->max_storage[slot] + STORAGE_BONUS + 5 );
	send_to_char( buf, ch );

	ch->pcdata->max_storage[slot] += 5;
	ch->pcdata->questpoints -= 500;

	return;
    }

    if ( argument[0] == '\0' )
    {
	do_storage( ch, "" );
	return;
    }

    if ( !str_prefix( arg, "add" ) )
    {
	sh_int cnt, count = count_objects( ch->pcdata->storage_list[slot] );

	if ( str_cmp( argument, "all" ) && str_prefix( "all.", argument ) )
	{
	    /* 'storage add obj' */
	    if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
	    {
		send_to_char( "You do not have that item.\n\r", ch );
		return;
	    }

	    if ( !can_drop_obj( ch, obj ) )
	    {
		send_to_char( "You can't let go of it.\n\r", ch );
		return;
	    }

	    if ( count + count_objects( obj->contains ) >= ch->pcdata->max_storage[slot] + STORAGE_BONUS )
	    {
		sprintf( buf, "Your storage box will only hold %d items.\n\r",
		    ch->pcdata->max_storage[slot] + STORAGE_BONUS );
		send_to_char( buf, ch );
		return;
	    }

	    obj_from_char( obj );

	    obj->dropped_by		= ch;
	    obj->next_content		= ch->pcdata->storage_list[slot];
	    ch->pcdata->storage_list[slot] = obj;

	    act( "$n puts $p in storage.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	    act( "You put $p in storage.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	} else {
	    bool found = FALSE;
	    final = new_buf();

	    /* 'storage add all' or 'storage add all.obj' */
	    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;

		if ( (argument[3] == '\0' || is_name( &argument[4], obj->name ))
		&&   can_see_obj( ch, obj )
		&&   obj->wear_loc == WEAR_NONE
		&&   can_drop_obj( ch, obj ) )
		{
		    found = TRUE;
		    cnt = 1+count_objects( obj->contains );
		    if ( count + cnt > ch->pcdata->max_storage[slot] + STORAGE_BONUS )
		    {
			sprintf( buf, "Your storage box will only hold %d items.\n\r",
			    ch->pcdata->max_storage[slot] + STORAGE_BONUS );
			add_buf( final, buf );
			if ( obj->contains == NULL )
			    break;
			else
			    continue;
		    }

		    count += cnt;
		    obj_from_char( obj );
		    obj->dropped_by		= ch;
		    obj->next_content		= ch->pcdata->storage_list[slot];
		    ch->pcdata->storage_list[slot] = obj;

		    if ( !IS_SET( ch->configure, CONFIG_LOOT_COMBINE ) )
		    {
			sprintf( buf, "You put %s in storage.\n\r",
			    obj->short_descr );
			add_buf( final, buf );
		    }
		}
	    }

	    if ( !found )
	    {
		if ( argument[3] == '\0' )
		    add_buf( final, "You are not carrying anything.\n\r" );
		else
		{
		    sprintf( buf, "You are not carrying any %s.\n\r", &argument[4] );
		    add_buf( final, buf );
		}
	    } else {
		if ( IS_SET( ch->configure, CONFIG_LOOT_COMBINE ) )
		    add_buf( final, "You put some things into storage.\n\r" );
		act( "$n puts some things into storage.",
		    ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    }

	    page_to_char( final->string, ch );
	    free_buf( final );
	}

	return;
    }

    if ( !str_prefix( arg, "get" ) )
    {
	if ( str_cmp( argument, "all" ) && str_prefix( "all.", argument ) )
	{
	    /* 'storage remove obj' */
	    if ( (obj = get_obj_list( ch, argument, ch->pcdata->storage_list[slot] )) == NULL )
		act( "Your storage contains no $T.",
		    ch, NULL, argument, TO_CHAR,POS_RESTING);
	    else
		send_to_char( get_obj_storage( ch, obj, FALSE, slot ), ch );
	} else {
	    bool found = FALSE;
	    final = new_buf();

	    /* 'storage remove all' or 'storage remove all.obj' */
	    for ( obj = ch->pcdata->storage_list[slot]; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( argument[3] == '\0' || is_name( &argument[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    add_buf( final, get_obj_storage( ch, obj, TRUE, slot ) );
		}
	    }

	    if ( !found )
	    {
		if ( argument[3] == '\0' )
		    add_buf( final, "Your storage is empty.\n\r" );
		else
		{
		    sprintf( buf, "You don't have any %s in storage.\n\r", &argument[4] );
		    add_buf( final, buf );
		}
	    } else {
		if ( IS_SET( ch->configure, CONFIG_LOOT_COMBINE ) )
		    add_buf( final, "You grab everything from storage.\n\r" );
		act( "$n quickly grabs everything from storage.",
		    ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    }
	    page_to_char( final->string, ch );
	    free_buf( final );
	}

	return;
    }

    do_storage( ch, "" );
}

void do_repair( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    int chance;

    if ( ( chance = get_skill( ch, gsn_repair ) ) <= 0 )
    {
	send_to_char( "You know not how to repair objects.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Which object would you like to repair?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_repair].beats );

    if ( 9 * chance / 10 > number_percent() )
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_DAMAGE )
	    {
		paf->modifier  -= number_range( 1, 3 );
		if ( paf->modifier <= 0 )
		{
		    act( "You fully repair $p.",
			ch, obj, NULL, TO_CHAR, POS_DEAD );
		    affect_remove_obj( obj, paf );
		} else {
		    act( "You repair $p.",
			ch, obj, NULL, TO_CHAR, POS_DEAD );
		}
		act( "$n repairs $p.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );
		check_improve( ch, gsn_repair, TRUE, 5 );
		return;
	    }
	}

	act( "You notice no damage to $p.",
	    ch, obj, NULL, TO_CHAR, POS_DEAD );
    } else {
	if ( number_percent() < 40 + ( chance / 2 ) )
	{
	    act( "Your attempt to repair $p fails.",
		ch, obj, NULL, TO_CHAR, POS_DEAD );
	    act( "$n attemps to repair $p, but fails.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	    check_improve( ch, gsn_repair, FALSE, 5 );
	} else {
	    act( "Your attempt to repair $p etches it more!",
		ch, obj, NULL, TO_CHAR, POS_DEAD );
	    act( "$n attemps to repair $p and etches it more!",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	    check_improve( ch, gsn_repair, FALSE, 5 );

	    for ( paf = obj->affected; paf != NULL; paf = paf->next)
	    {
		if ( paf->location == APPLY_DAMAGE )
		{
		    paf->modifier  += number_range( 1, 4 );
		    paf->level	= UMAX( paf->level, ch->level );
		    return;
		}
	    }

	    paf = new_affect();

	    paf->type       = 0;
	    paf->level      = ch->level;
	    paf->dur_type   = DUR_TICKS;
	    paf->duration   = -1;
	    paf->location   = APPLY_DAMAGE;
	    paf->modifier   = number_range( 1, 4 );
	    paf->bitvector  = 0;
	    paf->next       = obj->affected;
	    obj->affected   = paf;
	}
    }
}

void set_obj_loader( CHAR_DATA *ch, OBJ_DATA *obj, char *string )
{
    char buf[MAX_INPUT_LENGTH], time[100];

    strftime( time, 100, "(%H:%M-%m/%d/%Y)", localtime( &current_time ) );

    sprintf( buf,"%s: %s %s", string, time, ch->name );

    free_string( obj->loader );
    obj->loader = str_dup( buf );
}

#define WEAPON_CHECK( obj, flag )	\
    if ( !IS_WEAPON_STAT( obj, flag ) )	\
    {					\
	SET_BIT( obj->value[4], flag );	\
	add++;				\
	break;				\
    }

void do_engineer( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj, *new;
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int chance;

    argument = one_argument( argument, arg );

    if ( ( chance = get_skill( ch, gsn_engineer ) ) == 0 )
    {
	send_to_char( "You do not know how to engineer things.\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char(	"Syntax: engineer build\n\r"
			"        engineer grenade\n\r"
			"        engineer tinker <weapon>\n\r", ch );
	return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next )
    {
	if ( obj->item_type == ITEM_ENGINEER_TOOL )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char( "You lack the proper tools to engineer things.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_engineer].beats );

    if ( number_percent( ) > 9 * chance / 10 )
    {
	send_to_char( "You start working on your creation and it shatters!\n\r", ch );
	act( "$n starts working on a creation and shatters it!",
	    ch, NULL, NULL, TO_ROOM, POS_RESTING );
	check_improve( ch, gsn_engineer, FALSE, 5 );
	obj_from_char( obj );
	extract_obj( obj );
	return;
    }

    if ( !str_prefix( arg, "build" ) )
    {
	if ( obj->value[0] != 0 )
	{
	    send_to_char( "You do not have the correct engineering tool for that.\n\r", ch );
	    return;
	}
	if ( ch->pet != NULL )
	    send_to_char( "You failed.\n\r", ch );
	else
	{
	    CHAR_DATA *pet = create_mobile( get_mob_index( MOB_VNUM_ENGINEER ) );

	    if ( !IS_SET( pet->act, ACT_PET ) )
		SET_BIT( pet->act, ACT_PET );
	    if ( !IS_SET( pet->affected_by, AFF_CHARM ) )
		SET_BIT( pet->affected_by, AFF_CHARM );

	    sprintf( buf, "%s{GThe mark of %s is on its forehead.{x\n\r",
		pet->description, ch->name );
	    free_string( pet->description );
	    pet->description = str_dup( buf );

	    char_to_room( pet, ch->in_room );
	    add_follower( pet, ch );

	    pet->leader = ch;
	    ch->pet = pet;
	    pet->alignment = ch->alignment;
	    pet->level = ch->level;
	    pet->max_hit = number_range( ch->mana * 2 / 3, ch->mana * 3 / 2 );
	    pet->hit = pet->max_hit;
	    pet->max_mana = number_range( ch->mana * 2 / 3, ch->mana * 3 / 2 );
	    pet->mana = pet->max_mana;
	    pet->damage[DICE_NUMBER] = ch->level / 10;
	    pet->damage[DICE_TYPE] = ch->level / 10;
	    pet->damroll = number_range( ch->level * 2, ch->level * 3 );
	    pet->hitroll = number_range( ch->level * 2, ch->level * 3 );
	    pet->armor[0] = ch->level/2;
	    pet->armor[1] = ch->level/2;
	    pet->armor[2] = ch->level/2;
	    pet->armor[3] = ch->level/2;

	    act( "You start to build and create $N!",
		ch, NULL, pet, TO_CHAR, POS_DEAD );
	    act( "$n starts to build and creates $N!",
		ch, NULL, pet, TO_ROOM, POS_RESTING );
	}
    }

    else if ( !str_prefix( arg, "grenade" ) )
    {
	if ( obj->value[0] != 1 )
	{
	    send_to_char( "You do not have the correct engineering tool for that.\n\r", ch );
	    return;
	}

	if ( ( new = create_object( get_obj_index( OBJ_VNUM_ENGINEER_GRENADE ) ) ) == NULL )
	{
	    send_to_char( "Obj vnum missing! Contact Immortal!\n\r", ch );
	    return;
	}

	new->level = ch->level;
	new->item_type = ITEM_GRENADE;
	new->value[0] = ch->level;
	obj_to_char( new, ch );
    }

    else if ( !str_prefix( arg, "tinker" ) )
    {
	if ( obj->value[0] != 2 )
	{
	    send_to_char( "You do not have the correct engineering tool for that.\n\r", ch );
	    return;
	}

	if ( ( new = get_obj_carry( ch, argument ) ) == NULL )
	    send_to_char( "You do not have that weapon.\n\r", ch );
	else if ( new->item_type != ITEM_WEAPON )
	    send_to_char( "That is not a weapon.\n\r", ch );
	else
	{
	    int add = 0, pos;

	    new->timer = 15;
	    for ( pos = 0; pos < 50 && add < 3; pos++ )
	    {
		switch( number_range( 0, 8 ) )
		{
		    default:
			break;
		    case 0:
			WEAPON_CHECK( new, WEAPON_FLAMING );
			break;
		    case 1:
			WEAPON_CHECK( new, WEAPON_FROST );
			break;
		    case 2:
			WEAPON_CHECK( new, WEAPON_VAMPIRIC );
			break;
		    case 3:
			WEAPON_CHECK( new, WEAPON_SHARP );
			break;
		    case 4:
			WEAPON_CHECK( new, WEAPON_VORPAL );
			break;
		    case 5:
			WEAPON_CHECK( new, WEAPON_SHOCKING );
			break;
		    case 6:
			WEAPON_CHECK( new, WEAPON_POISON );
			break;
		}
	    }
	}

    }

    else
    {
	do_engineer( ch, "" );
	return;
    }

    obj_from_char( obj );
    extract_obj( obj );
    check_improve( ch, gsn_engineer, TRUE, 5 );
}

#undef WEAPON_CHECK

void do_hurl( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *to_room = ch->in_room;
    CHAR_DATA *vch;
    int chance, dam, door;

    if ( ( chance = get_skill( ch, gsn_hurl ) ) == 0 )
    {
	send_to_char( "You do not know how to hurl things.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL
    ||   obj->item_type != ITEM_GRENADE )
    {
	send_to_char( "You are not holding a grenade.\n\r", ch );
	return;
    }

    switch ( LOWER( *argument ) )
    {
	default:	send_to_char( "Invalid direction.\n\r", ch );	return;
	case 'n':	door = DIR_NORTH;	break;
	case 's':	door = DIR_SOUTH;	break;
	case 'w':	door = DIR_WEST;	break;
	case 'e':	door = DIR_EAST;	break;
	case 'u':	door = DIR_UP;		break;
	case 'd':	door = DIR_DOWN;	break;
    }

    WAIT_STATE( ch, skill_table[gsn_hurl].beats );

    if ( number_percent( ) > chance )
    {
	check_improve( ch, gsn_hurl, FALSE, 4 );
	switch( number_range( 1, 3 ) )
	{
	    case 1:
		act( "You fumble with $p and drop it.",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
		act( "$n fumbles with $p and drops it.",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		return;

	    case 2:
	    case 3:
		act( "You fumble with $p and drop a live grenade!",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
		act( "$n fumbles with $p and drops a live grenade!",
		    ch, obj, NULL, TO_ROOM, POS_RESTING );
		to_room = ch->in_room;
		break;
	}
    } else {
	if ( ch->in_room->exit[door] != NULL
	&&   ( to_room = ch->in_room->exit[door]->u1.to_room ) != NULL
	&&   can_see_room( ch, to_room ) )
	{
	    if ( number_percent( ) > 9 * chance / 10 )
	    {
		act( "You take careful aim, then ricochet $p off the doorway!",
		    ch, obj, NULL, TO_CHAR, POS_RESTING );
		act( "$n takes careful aim, then ricochets $p off the doorway!",
		    ch, obj, NULL, TO_ROOM, POS_RESTING  );
		to_room = ch->in_room;
	    } else {
		act( "You hurl $p $T.",
		    ch, obj, dir_name[door], TO_CHAR, POS_RESTING );
		act( "$n hurls $p $T.",
		    ch, obj, dir_name[door], TO_CHAR, POS_RESTING );
	    }
	} else {
	    act( "You hurl $p right into the wall!",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	    act( "$n hurls $p right into the wall!",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	    to_room = ch->in_room;
	}

	check_improve( ch, gsn_hurl, TRUE, 4 );
    }

    for ( vch = to_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if ( vch != ch )
	    act( "You see $p streaking through the air, heading this way.",
		vch, obj, NULL, TO_CHAR, POS_RESTING );

	if ( is_safe_spell( ch, vch, FALSE ) )
	    continue;

	dam = dice( obj->level / 2, obj->level / 3 );
	damage( ch, vch, dam, gsn_hurl, DAM_IRON, TRUE, FALSE, NULL );
    }

    obj_from_char( obj );
    extract_obj( obj );
}

const char * cost_string[MAX_AUCTION_PAYMENT] =
{
    "silver",
    "gold",
    "platinum",
    "quest points",
    "deviant points"
};

sh_int cost_lookup( char *argument )
{
    sh_int stat;

    for ( stat = 0; stat < MAX_AUCTION_PAYMENT; stat++ )
    {
	if ( !str_prefix( argument, cost_string[stat] ) )
	    return stat;
    }

    return -1;
}

bool can_cover_bid( CHAR_DATA *ch, int type, int amount )
{
    switch( type )
    {
	case VALUE_SILVER:
	    if ( ch->silver + ( 100 * ch->gold ) + ( 10000 * ch->platinum ) < amount )
		return FALSE;
	    break;

	case VALUE_GOLD:
	    if ( ch->gold + ( 100 * ch->platinum ) < amount )
		return FALSE;
	    break;

	case VALUE_PLATINUM:
	    if ( ch->platinum < amount )
		return FALSE;
	    break;

	case VALUE_QUEST_POINT:
	    if ( ch->pcdata == NULL || ch->pcdata->questpoints < amount )
		return FALSE;
	    break;

	case VALUE_IQUEST_POINT:
	    if ( !ch->pcdata || ch->pcdata->deviant_points[0] < amount )
		return FALSE;
	    break;

	default:
	    bug( "can_cover_bid: invalid bid type.", 0 );
	    return FALSE;
    }

    return TRUE;
}

AUCTION_DATA * auction_lookup( int ticket )
{
    AUCTION_DATA *auction;

    for ( auction = auction_list; auction != NULL; auction = auction->next )
    {
	if ( auction->slot == ticket )
	    return auction;
    }

    return NULL;
}

void auction_channel( CHAR_DATA *ch, AUCTION_DATA *auc, char *msg )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    sprintf( buf, "{A<{WAUCTION{A> Ticket [{Y%d{A], %s{x", auc->slot, msg );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING && victim
	&&   !IS_SET( victim->comm, COMM_NOAUCTION )
	&&   !IS_SET( victim->comm, COMM_QUIET ) )
	    act( buf, victim, auc->item, ch, TO_CHAR, POS_DEAD );
    }
}

void do_auction( CHAR_DATA *ch, char * argument )
{
    AUCTION_DATA *auc, *pos_auc;
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    sh_int found = 0;

    if ( ch == NULL || IS_NPC( ch ) )
	return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	TOGGLE_BIT( ch->comm, COMM_NOAUCTION );

	if ( IS_SET( ch->comm, COMM_NOAUCTION ) )
	    send_to_char( "Auction channel disabled.\n\r", ch );
	else
	    send_to_char( "Auction channel enabled.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOAUCTION )
    ||   IS_SET( ch->comm, COMM_QUIET ) )
    {
	send_to_char( "You can't use the auction command with quiet or noauction set.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg1, "info" ) || !str_prefix( arg1, "show" )
    ||   !str_prefix( arg1, "list" ) || !str_prefix( arg1, "items" ) )
    {
	BUFFER *final, *stats;
	char seller[MAX_INPUT_LENGTH];

	if ( arg2[0] == '\0' )
	{
	    if ( auction_list == NULL )
	    {
		send_to_char( "There is currently nothing up for auction.\n\r", ch );
		return;
	    }

	    final = new_buf( );

	    add_buf( final, "{s =============================( {qAUCTION LISTING {s)============================= \n\r"
			    "| {tNu {s| {tSeller     {s| {tItem                         {s| {tMn/Cr Bid {s|   {tBidder   {s| {tT {s|\n\r"
			    " ============================================================================= \n\r" );

	    for ( auc = auction_list; auc != NULL; auc = auc->next )
	    {
		strcpy( seller, end_string( PERS( auc->owner, ch ), 10 ) );

		sprintf( buf, "{s| {t%2d {s| {t%s {s| {t%s {s|{t%9d {R%c{s| {t%s {s| {t%d {s|\n\r",
		    auc->slot, seller, can_see_obj( ch, auc->item ) ?
		    end_string( auc->item->short_descr, 28 ) :
		    "Something                   ",
		    auc->bid_amount, UPPER( *cost_string[auc->bid_type] ),
		    auc->high_bidder == NULL ? "----------" :
		    begin_string( PERS( auc->high_bidder, ch ), 10 ),
		    ( AUCTION_LENGTH - auc->status ) );
		add_buf( final, buf );
	    }

	    add_buf( final, " =============================( {qAUCTION LISTING {s)============================={x\n\r" );

	    page_to_char( final->string, ch );
	    free_buf( final );
	    return;
	}

	if ( !is_number( arg2 ) )
	{
	    send_to_char( "Syntax: auction info <ticket>.\n\r", ch );
	    return;
	}

	if ( ( auc = auction_lookup( atoi( arg2 ) ) ) == NULL )
	{
	    send_to_char( "Invalid auction ticket.\n\r", ch );
	    return;
	}

	if ( !auc->item )
	{
	    send_to_char( "Error, NULL object.\n\r", ch );
	    return;
	}

	else
	{
	    final = new_buf( );

	    if ( !can_see_obj( ch, auc->item )
	    ||   ( !IS_IMMORTAL( ch ) && ch == auc->owner ) )
	    {
		sprintf( buf, "{AThe current bid is {Y%d {A%s by %s{A.{x\n\r\n\r",
		    auc->bid_amount, cost_string[auc->bid_type],
		    auc->high_bidder == NULL ? "no one" :
		    PERS( auc->high_bidder, ch ) );
		add_buf( final, buf );
	    } else {
		sprintf( buf, "{AThe current bid is {Y%d {A%s by %s{A.{x\n\r\n\r",
		    auc->bid_amount, cost_string[auc->bid_type],
		    auc->high_bidder == NULL ? "no one" :
		    PERS( auc->high_bidder, ch ) );
		add_buf( final, buf );

		stats = display_stats( auc->item, ch, TRUE );
		add_buf( final, stats->string );
		free_buf( stats );
	    }

	    page_to_char( final->string, ch );
	    free_buf( final );
	}
 	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA )
    ||   IS_SET( ch->in_room->room_flags, ROOM_WAR ) )
    {
	send_to_char( "You do not have access to this from within a bloody cell.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "stop" ) )
    {
	if ( arg2[0] == '\0' || !is_number( arg2 ) )
	{
	    if ( !str_cmp( arg2, "all" ) )
	    {
		for ( auc = auction_list; auc != NULL; auc = pos_auc )
		{
		    pos_auc = auc->next;

		    sprintf( buf, "stop %d", auc->slot );
		    do_auction( ch, buf );
		}

		return;
	    }

	    send_to_char( "Syntax: auction stop <[ticket #] / all>.\n\r", ch );
	    return;
	}

	if ( ( auc = auction_lookup( atoi( arg2 ) ) ) == NULL )
	{
	    send_to_char( "That is not a valid auction ticket number.\n\r", ch );
	    return;
	}

	if ( !IS_IMMORTAL( ch ) && auc->owner != ch )
	{
	    send_to_char( "You may only stop your own auctions.\n\r", ch );
	    return;
	}

	auction_channel( ch, auc, "{x$p{A, has been stopped by $N." );
	
	act( "You stop the auction of $p.",
	    ch, auc->item, NULL, TO_CHAR, POS_DEAD );
	obj_to_char( auc->item, ch );

        if ( auc->high_bidder != NULL )
        {
	    add_cost( auc->high_bidder, auc->bid_amount, auc->bid_type );
	    send_to_char( "Your bid has been returned to you.\n\r",
		auc->high_bidder );
        }

	free_auction( auc );

	return;
    }

    if ( !str_prefix( arg1, "bid" ) )
    {
	int bid;

	if ( arg2[0] == '\0' || !is_number( arg2 )
	||   argument[0] == '\0' || !is_number( argument ) )
	{
	    send_to_char( "Syntax: auction bid <ticket> <amount>.\n\r", ch );
	    return;
	}

	if ( ( auc = auction_lookup( atoi( arg2 ) ) ) == NULL )
	{
	    send_to_char( "Invalid auction ticket.\n\r", ch );
	    return;
	}

	if ( auc->item == NULL )
	{
	    send_to_char( "Error, NULL Object!\n\r", ch );
	    return;
	}

	if ( !can_see_obj( ch, auc->item ) )
	{
	    send_to_char( "{AYou can't see that item.{x\n\r", ch );
	    return;
	}

	if ( ch == auc->high_bidder )
	{
	    send_to_char( "{AYou already have the highest bid.{x\n\r", ch );
	    return;
	}

	if ( ch == auc->owner )
	{
	    send_to_char( "{AWhy bid on something you auctioned?{x\n\r", ch );
	    return;
	}

	bid = atoi( argument );

	if ( bid < auc->bid_amount || bid <= 0
	||   ( auc->high_bidder != NULL && bid == auc->bid_amount ) )
	{
	    sprintf( buf, "{AYou must bid above the current bid of {Y%d {A%s.{x\n\r",
		auc->bid_amount, cost_string[auc->bid_type] );
            send_to_char( buf, ch );
	    return;
	}

	if ( !can_cover_bid( ch, auc->bid_type, bid ) )
	{
	    send_to_char( "{AYou can not cover that bid.{x\n\r", ch );
	    return;
	}

	if ( ch->carry_number + get_obj_number( auc->item ) > can_carry_n( ch ) )
	{
	    send_to_char( "You can't carry that many items.\n\r", ch );
	    return;
	}

	if ( get_carry_weight( ch ) + get_obj_weight( auc->item ) > can_carry_w( ch ) )
	{
	    send_to_char( "You can't carry that much weight.\n\r", ch );
	    return;
	}

	sprintf( buf, "$N {Abids {Y%d {A%s for ticket [{Y%d{A], {x$p{A.",
	    bid, cost_string[auc->bid_type], auc->slot );
	auction_channel( ch, auc, buf );

	if ( auc->high_bidder != NULL )
	    add_cost( auc->high_bidder, auc->bid_amount, auc->bid_type );

	deduct_cost( ch, bid, auc->bid_type );

	auc->high_bidder	= ch;
	auc->bid_amount		= bid;
	auc->status	 	= 0;
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "{AYou aren't carrying that item.{x\n\r", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "{AYou can't let go of that item.{x\n\r", ch );
	return;
    }

    if ( atoi( arg2 ) < 0 )
    {
	send_to_char( "{ASet the minimum just a little higher.{x\n\r", ch );
	return;
    }

    switch ( obj->item_type )
    {
	default:
	    break;

	case ITEM_CORPSE_PC:
	case ITEM_CORPSE_NPC:
	    send_to_char( "You can not auction that!\n\r", ch );
	    return;
    }

    if ( !IS_IMMORTAL( ch ) )
    {
	for ( auc = auction_list; auc != NULL; auc = auc->next )
	{
	    if ( auc->owner == ch )
	    {
		if ( ++found >= 5 )
		{
		    send_to_char( "Max auctions per person reached.\n\r", ch );
		    return;
		}
	    }
	}
    }

    if ( argument[0] != '\0' )
    {
	if ( ( found = cost_lookup( argument ) ) == -1 )
	{
	    send_to_char( "Minbid must be one of: silver, gold, platinum, quest point or Deviant point.\n\r", ch );
	    return;
	}
    }
    else
	found = VALUE_GOLD;

    auction_ticket++;

    auc = new_auction( );

    if ( arg2[0] == '\0' )
	auc->bid_amount = 0;
    else
	auc->bid_amount = atoi( arg2 );

    auc->owner		= ch;
    auc->item		= obj;
    auc->slot		= auction_ticket;
    auc->bid_type	= found;

    sprintf( buf, "$N {Ais now taking bids on{x $p{A. < {Y%d{A %s >",
	auc->bid_amount, cost_string[auc->bid_type] );
    auction_channel( ch, auc, buf );
    
    obj_from_char( obj );

    if ( auction_list == NULL )
	auction_list = auc;
    else
    {
	for ( pos_auc = auction_list; pos_auc != NULL; pos_auc = pos_auc->next )
	{
	    if ( pos_auc->next == NULL )
	    {
		pos_auc->next = auc;
		break;
	    }
	}
    }
}

void auction_update( )
{
    AUCTION_DATA *auc, *auc_next;
    char buf[MAX_STRING_LENGTH];

    for ( auc = auction_list; auc != NULL; auc = auc_next )
    {
	auc_next = auc->next;

	if ( auc->item == NULL )
	    continue;

	if ( ++auc->status == AUCTION_LENGTH )
	{
	    if ( auc->high_bidder == NULL )
	    {
		auction_channel( auc->owner, auc, "No bids on{x $p {A- item removed.{x" );
		obj_to_char( auc->item, auc->owner );
		free_auction( auc );
	    } else {
		sprintf( buf, "{x$p {ASOLD to $N {Afor {Y%d {A%s.{x",
		    auc->bid_amount, cost_string[auc->bid_type] );
		auction_channel( auc->high_bidder, auc, buf );
		
		add_cost( auc->owner, auc->bid_amount, auc->bid_type );

		sprintf( buf, "You sell $p for %d %s.",
		    auc->bid_amount, cost_string[auc->bid_type] );
		act( buf, auc->owner, auc->item, NULL, TO_CHAR, POS_DEAD );

		obj_to_char( auc->item, auc->high_bidder );

		act( "{x$p {Aappears in your hands.{x",
		    auc->high_bidder, auc->item, NULL, TO_CHAR, POS_DEAD );

		if ( !obj_droptest( auc->high_bidder, auc->item ) )
		{
		    act( "You drop $p as it {Rs{rc{Ra{rl{Rd{rs{x you upon touching it!",
			auc->high_bidder, auc->item, NULL, TO_CHAR, POS_RESTING );
		    act( "$n is {Rs{rc{Ra{rl{Rd{re{Rd{x by $p as $e grabs it!",
			auc->high_bidder, auc->item, NULL, TO_ROOM, POS_RESTING );
		    act( "$p {Dv{wa{Dn{wi{Ds{wh{De{ws in a {Dm{wi{Ds{wt{x.",
			auc->high_bidder, auc->item, NULL, TO_CHAR, POS_RESTING );
		    act( "$p {Dv{wa{Dn{wi{Ds{wh{De{ws in a {Dm{wi{Ds{wt{x.",
			auc->high_bidder, auc->item, NULL, TO_ROOM, POS_RESTING );
		    extract_obj( auc->item );
		}
		free_auction( auc );
	    }
	} else {
	    if ( auc->high_bidder == NULL )
	    {
		if ( auc->status == AUCTION_LENGTH - 1 )
		    auction_channel( auc->owner, auc,
			"{ANo bids on{x $p {A- item going twice.{x" );

		else if ( auc->status == AUCTION_LENGTH - 2 )
		    auction_channel( auc->owner, auc,
			"{ANo bids on{x $p {A- item going once.{x" );
	    } else {
		if ( auc->status == AUCTION_LENGTH - 1 )
		{
		    sprintf( buf, "{x$p {A- going twice at {Y%d {A%s to $N{A.{x",
			auc->bid_amount, cost_string[auc->bid_type] );
		    auction_channel( auc->high_bidder, auc, buf );
		}
		else if ( auc->status == AUCTION_LENGTH - 2 )
		{
		    sprintf( buf, "{x$p {A- going once at {Y%d {A%s to $N{A.{x",
			auc->bid_amount, cost_string[auc->bid_type] );
		    auction_channel( auc->high_bidder, auc, buf );
		}
	    }
	}
    }
}

void do_sheath( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int chance;

    if ( ( chance = get_skill( ch, gsn_quickdraw ) ) == 0 )
    {
	send_to_char( "You don't know how to sheath weapons.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Do you wish to sheath your primary or secondary weapon?\n\r", ch );
	return;
    }

    if ( !cost_of_skill( ch, gsn_quickdraw ) )
	return;

    if ( !str_prefix( argument, "primary" ) )
    {
	if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
	{
	    send_to_char( "You have no primary weapon to sheath.\n\r", ch );
	    return;
	}

	if ( get_eq_char( ch, WEAR_SHEATH_1 ) != NULL )
	{
	    send_to_char( "You already have a weapon sheathed on your primary side.\n\r", ch );
	    return;
	}

	if ( number_percent( ) > chance )
	{
	    obj_from_char( obj );

	    switch( number_range( 1, 3 ) )
	    {
		case 1:
		case 2:
		    act( "{jYou fumble with $p {jand nearly drop it!{x",
			ch, obj, NULL, TO_CHAR, POS_RESTING );
		    act( "$n fumbles with $p and nearly drops it!",
			ch, obj, NULL, TO_ROOM, POS_RESTING );
		    obj_to_char( obj, ch );
		    break;

		case 3:
		    act( "{jYou fumble with $p {jand drop it!{x",
			ch, obj, NULL, TO_CHAR, POS_RESTING );
		    act( "$n fumbles with $p and drops it!",
			ch, obj, NULL, TO_ROOM, POS_RESTING );

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
			if ( IS_NPC( ch )
			&&   can_see_obj( ch, obj ) )
			    send_to_char( get_obj( ch, obj, NULL, FALSE ), ch );
		    }
		    break;
	    }
	    check_improve( ch, gsn_quickdraw, FALSE, 4 );
	} else {
	    obj->wear_loc = WEAR_SHEATH_1;
	    act( "$n sheaths $p on $s primary side.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	    act( "You sheath $p on your primary side.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	    check_improve( ch, gsn_quickdraw, TRUE, 4 );
	}
    }

    else if ( !str_prefix( argument, "secondary" ) )
    {
	if ( ( obj = get_eq_char( ch, WEAR_SECONDARY ) ) == NULL )
	{
	    send_to_char( "You have no secondary weapon to sheath.\n\r", ch );
	    return;
	}

	if ( get_eq_char( ch, WEAR_SHEATH_2 ) != NULL )
	{
	    send_to_char( "You already have a weapon sheathed on your secondary side.\n\r", ch );
	    return;
	}

	if ( number_percent( ) > chance )
	{
	    obj_from_char( obj );

	    switch( number_range( 1, 3 ) )
	    {
		case 1:
		case 2:
		    act( "{jYou fumble with $p {jand nearly drop it!{x",
			ch, obj, NULL, TO_CHAR, POS_RESTING );
		    act( "$n fumbles with $p and nearly drops it!",
			ch, obj, NULL, TO_ROOM, POS_RESTING );
		    obj_to_char( obj, ch );
		    break;

		case 3:
		    act( "{jYou fumble with $p {jand drop it!{x",
			ch, obj, NULL, TO_CHAR, POS_RESTING );
		    act( "$n fumbles with $p and drops it!",
			ch, obj, NULL, TO_ROOM, POS_RESTING );

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
			if ( IS_NPC( ch )
			&&   can_see_obj( ch, obj ) )
			    send_to_char( get_obj( ch, obj, NULL, FALSE ), ch );
		    }
		    break;
	    }
	    check_improve( ch, gsn_quickdraw, FALSE, 4 );
	} else {
	    obj->wear_loc = WEAR_SHEATH_2;
	    act( "$n sheaths $p on $s secondary side.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	    act( "You sheath $p on your secondary side.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	    check_improve( ch, gsn_quickdraw, TRUE, 4 );
	}
    }

    else
	send_to_char( "Do you wish to sheath your primary or secondary weapon?\n\r", ch );
}

void do_draw( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int chance;

    if ( ( chance = get_skill( ch, gsn_quickdraw ) ) == 0 )
    {
	send_to_char( "You don't know how to draw weapons.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Do you wish to draw your primary or secondary weapon?\n\r", ch );
	return;
    }

    if ( !cost_of_skill( ch, gsn_quickdraw ) )
	return;

    if ( !str_prefix( argument, "primary" ) )
    {
	if ( ( obj = get_eq_char( ch, WEAR_SHEATH_1 ) ) == NULL )
	{
	    send_to_char( "You have no primary sheath to draw.\n\r", ch );
	    return;
	}

	if ( get_eq_char( ch, WEAR_WIELD ) != NULL )
	{
	    send_to_char( "You already have a weapon wielded in your primary hand.\n\r", ch );
	    return;
	}

	if ( number_percent( ) > chance )
	{
	    obj_from_char( obj );

	    switch( number_range( 1, 3 ) )
	    {
		case 1:
		case 2:
		    act( "{jYou fumble with $p {jand nearly drop it!{x",
			ch, obj, NULL, TO_CHAR, POS_RESTING );
		    act( "$n fumbles with $p and nearly drops it!",
			ch, obj, NULL, TO_ROOM, POS_RESTING );
		    obj_to_char( obj, ch );
		    break;

		case 3:
		    act( "{jYou fumble with $p {jand drop it!{x",
			ch, obj, NULL, TO_CHAR, POS_RESTING );
		    act( "$n fumbles with $p and drops it!",
			ch, obj, NULL, TO_ROOM, POS_RESTING );

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
			if ( IS_NPC( ch )
			&&   can_see_obj( ch, obj ) )
			    send_to_char( get_obj( ch, obj, NULL, FALSE ), ch );
		    }
		    break;
	    }

	    check_improve( ch, gsn_quickdraw, FALSE, 4 );
	} else {
	    obj->wear_loc = WEAR_WIELD;
	    act( "{j$n {jdraws $p {jto $s primary hand.{x",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	    act( "{jYou draw $p {jto your primary hand.{x",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	    check_improve( ch, gsn_quickdraw, TRUE, 4 );
	}
    }

    else if ( !str_prefix( argument, "secondary" ) )
    {
	if ( ( obj = get_eq_char( ch, WEAR_SHEATH_2 ) ) == NULL )
	{
	    send_to_char( "You have no secondary sheath to draw.\n\r", ch );
	    return;
	}

	if ( get_eq_char( ch, WEAR_SECONDARY ) != NULL )
	{
	    send_to_char( "You already have a weapon wielded in your secondary hand.\n\r", ch );
	    return;
	}

	if ( number_percent( ) > chance )
	{
	    obj_from_char( obj );

	    switch( number_range( 1, 3 ) )
	    {
		case 1:
		case 2:
		    act( "{jYou fumble with $p {jand nearly drop it!{x",
			ch, obj, NULL, TO_CHAR, POS_RESTING );
		    act( "$n fumbles with $p and nearly drops it!",
			ch, obj, NULL, TO_ROOM, POS_RESTING );
		    obj_to_char( obj, ch );
		    break;

		case 3:
		    act( "{jYou fumble with $p {jand drop it!{x",
			ch, obj, NULL, TO_CHAR, POS_RESTING );
		    act( "$n fumbles with $p and drops it!",
			ch, obj, NULL, TO_ROOM, POS_RESTING );

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
			if ( IS_NPC( ch )
			&&   can_see_obj( ch, obj ) )
			    send_to_char( get_obj( ch, obj, NULL, FALSE ), ch );
		    }
		    break;
	    }
	    check_improve( ch, gsn_quickdraw, FALSE, 4 );
	} else {
	    obj->wear_loc = WEAR_SECONDARY;
	    act( "{j$n draws $p {jto $s secondary hand.{x",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	    act( "{jYou draw $p {jto your secondary hand.{x",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	    check_improve( ch, gsn_quickdraw, TRUE, 4 );
	}
    }

    else
	send_to_char( "Do you wish to sheath your primary or secondary weapon?\n\r", ch );
}

void do_trap( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int chance, max, count = 0;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(	"Syntax: trap set <object> - set a trap\n\r"
			"        trap disarm <object> - disarm a trap\n\r"
			"        trap conceal - conceal any traps of yours that are set in this room\n\r"
			"        trap status - see the traps that have been set by you\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "set" )
    &&   argument[0] != '\0' )
    {
	if ( ( chance = get_skill( ch, gsn_trapset ) ) <= 0 )
	{
	    send_to_char( "You don't know how to set a trap.\n\r", ch );
	    return;
	}

	if ( ch->pcdata && ch->pcdata->pktimer > 0 )
	{
	    send_to_char( "Your heart is racing too much right now.\n\r", ch );
	    return;
	}

	for ( obj = object_list; obj != NULL; obj = obj->next )
	{
	    if ( obj->item_type == ITEM_TRAP
	    &&   obj->in_room != NULL
	    &&   obj->dropped_by == ch
	    &&   !IS_SET( obj->wear_flags, ITEM_TAKE ) )
	        count++;
	}
	
	if ( !IS_NPC(ch) )
	{
	    if ( ch->pcdata->devote_points[DEVOTE_SKILLS] <= 0
	    &&   count > 0 )
	    {
	        send_to_char( "You lack the skill to set more than one trap.\n\r", ch );
		return;
	    }
	    else if ( count > (max = (ch->pcdata->devote_points[DEVOTE_SKILLS]/5)) )
	    {
		sprintf( buf, "You lack the skill to set more than %d traps.\n\r", max );
		send_to_char( buf,ch );
		return;
	    }
	    else if ( count > 10 )
	    {
	        send_to_char( "You cannot set more than 10 traps.\n\r", ch );
		return;
	    }
	}
	else if ( count > 1 )
	{
	    send_to_char( "You already have enough traps set.\n\r", ch );
	    return;
	}
	
	if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
	{
	    send_to_char( "You do not have that trap.\n\r", ch );
	    return;
	}

	if ( obj->item_type != ITEM_TRAP )
	{
	    act( "$p is not a trap.", ch, obj, NULL, TO_CHAR, POS_RESTING );
	    return;
	}
	
	if ( ch->level < obj->level )
	{
	    act( "$p is too complex for you to use.", ch, obj, NULL, TO_CHAR, POS_RESTING );
	    return;
	}
	
	if ( !cost_of_skill( ch, gsn_trapset ) )
	    return;

	if ( number_percent( ) > chance )
	{
	    send_to_char( "You failed to set your trap.\n\r", ch );
	    act( "$n fumbles around with $p.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	    obj_from_char( obj );
	    extract_obj( obj );
	    check_improve( ch, gsn_trapset, FALSE, 3 );
	} else {
	    REMOVE_BIT( obj->wear_flags, ITEM_TAKE );
	    obj->timer = obj->value[4];
	    obj->dropped_by = ch;
	    obj_from_char( obj );
	    obj_to_room( obj, ch->in_room );
	    act( "You carefully place $p on the ground.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	    act( "$n carefully places $p on the ground.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	    check_improve( ch, gsn_trapset, TRUE, 3 );
	}
    }

    else if ( !str_prefix( arg, "disarm" )
    &&    argument[0] == '\0' )
    {
	if ( ( chance = get_skill( ch, gsn_trapdisarm ) ) <= 0 )
	{
	    send_to_char( "You don't know how to disarm a trap.\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, NULL, argument ) ) == NULL )
	{
	    send_to_char( "You do not see that trap here.\n\r", ch );
	    return;
	}

	if ( obj->item_type != ITEM_TRAP )
	{
	    act( "$p is not a trap.", ch, obj, NULL, TO_CHAR, POS_RESTING );
	    return;
	}

	if ( IS_SET( obj->wear_flags, ITEM_TAKE ) )
	{
	    act( "$p never seems to have been armed.", ch, obj, NULL, TO_CHAR, POS_RESTING );
	    return;
	}

	if ( ch->level < obj->level )
	{
	    act( "$p is too complex for you to disarm.", ch, obj, NULL, TO_CHAR, POS_RESTING );
	    return;
	}

	if ( obj->dropped_by != ch
	&&   !IS_NPC(ch)
	&&   !IS_NPC(obj->dropped_by) )
	{
	    if ( !can_pk(ch, obj->dropped_by) )
	    {
	        send_to_char( "You cannot attack the person who set this trap, so you can't disarm it.\n\r", ch );
	        return;
	    }
	}

	if ( !cost_of_skill( ch, gsn_trapdisarm ) )
	    return;

	if ( number_percent( ) > chance )
	{
	    send_to_char( "You failed to disarm your trap.\n\r", ch );
	    act( "$n fumbles around with $p.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	    check_trap( ch, obj );
	    check_improve( ch, gsn_trapdisarm, FALSE, 3 );
	} else {
	    SET_BIT( obj->wear_flags, ITEM_TAKE );
	    REMOVE_BIT( obj->extra_flags, ITEM_DARK );
	    obj->timer = 0;
	    obj->dropped_by = NULL;
	    obj_from_room( obj );
	    obj_to_char( obj, ch );
	    act( "You carefully grab $p from the ground.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	    act( "$n carefully grabs $p from the ground.",
		ch, obj, NULL, TO_ROOM, POS_RESTING );
	    check_improve( ch, gsn_trapdisarm, TRUE, 3 );
	}
    }

    else if ( !str_prefix(arg, "conceal") )
    {
	if ( ( chance = get_skill( ch, gsn_trapset ) ) <= 0 )
	{
	    send_to_char( "You don't know how to conceal traps.\n\r", ch );
	    return;
	}
	
	if ( !IS_NPC(ch) )
	{
	    if ( ch->pcdata->devote_points[DEVOTE_SKILLS] <= 0 )
	    {
	        send_to_char( "You need to devote more time to developing your skills.\n\r", ch );
	        return;
	    }
	    chance -= 40;
	    chance += ch->pcdata->devote_points[DEVOTE_SKILLS];
	}
	
	if ( number_percent( ) > chance )
	{
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
	    {
	        if ( obj->item_type == ITEM_TRAP
	        &&   obj->dropped_by == ch
	        &&   !IS_SET( obj->wear_flags, ITEM_TAKE ) )
	        {
	            if ( ch->in_room != obj->in_room )
	                break;
	            send_to_char( "You failed to conceal your trap.\n\r", ch );
	            act( "$n fumbles around with $p.",
		        ch, obj, NULL, TO_ROOM, POS_RESTING );
	            check_trap( ch, obj );
	            check_improve( ch, gsn_trapset, FALSE, 3 );
	        }
	    }
	} else {
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
	    {
	        if ( obj->item_type == ITEM_TRAP
	        &&   obj->dropped_by == ch
	        &&   !IS_SET( obj->wear_flags, ITEM_TAKE )
	        &&   !IS_SET( obj->extra_flags, ITEM_DARK ) )
	        {
	    	    act( "Whistling innocently, you carefully push $p into a corner with your foot.",
			ch, obj, NULL, TO_CHAR, POS_RESTING );
	    	    act( "Whistling innocently, $n carefully pushes $p into a corner with $s foot.",
			ch, obj, NULL, TO_ROOM, POS_RESTING );
	            SET_BIT( obj->extra_flags, ITEM_DARK );
	    	    check_improve( ch, gsn_trapset, TRUE, 3 );
	        }
	    }
	}
    }

    else if ( !str_prefix(arg, "status") )
    {
	for ( obj = object_list; obj != NULL; obj = obj->next )
	{
	    if ( obj->item_type == ITEM_TRAP
	    &&   obj->in_room != NULL
	    &&   obj->dropped_by == ch
	    &&   !IS_SET( obj->wear_flags, ITEM_TAKE ) )
	    {
	        count++;
	        if ( count == 1 )
        	    send_to_char( "{qNo. Timer Type       Room\n\r"
        	    		  "{e---------------------------------------------------------------------------\n\r", ch );
        	    		  
	        sprintf( buf, "{w%-2d  %-5d %-10s %s\n\r",
	            count, obj->timer,
	            capitalize(trap_type_table[obj->value[0]].name),
	            end_string(obj->in_room->name, 54) );
	        send_to_char( buf,ch );
	    }
	}
	if ( count == 0 )
	    send_to_char( "You have no traps set.\n\r", ch );
    }
    
    else
	do_trap( ch, "" );
}

void do_disguise( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];
    int chance;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Disguise what as what?\n\r", ch );
	return;
    }

    if ( ( chance = get_skill( ch, gsn_disguise ) ) <= 0 )
    {
	send_to_char( "You attempt to disguise ... nothing!\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION
    &&   obj->item_type != ITEM_SCROLL
    &&   obj->item_type != ITEM_PILL
    &&   obj->item_type != ITEM_FOOD )
    {
	send_to_char( "You may only disguise potions, scrolls, pills and food.\n\r", ch );
	return;
    }

    if ( !cost_of_skill( ch, gsn_disguise ) )
	return;

    act( "$n starts fiddling with $p.", ch, obj, NULL, TO_ROOM, POS_RESTING );

    if ( number_percent( ) > chance )
    {
	if ( number_percent( ) > chance / 2 )
	{
	    act( "Your attempt destroys $p.",
		ch, obj, NULL, TO_CHAR, POS_RESTING );
	    extract_obj( obj );
	}
	else
	    send_to_char( "You failed.\n\r", ch );
	check_improve( ch, gsn_disguise, FALSE, 3 );
    } else {
	if ( obj->timer == 0 || obj->timer > ch->level / 3 )
	    obj->timer = ch->level / 3;

	act( "You have successfully disguised $p as $T!",
	    ch, obj, argument, TO_CHAR, POS_RESTING );

	free_string( obj->short_descr );
	obj->short_descr = str_dup( argument );

	if ( strlen_color( argument ) + strlen( obj->pIndexData->name ) < MAX_INPUT_LENGTH )
	{
	    sprintf( arg, "%s %s", strip_color( argument ), obj->pIndexData->name );
	    free_string( obj->name );
	    obj->name = str_dup( arg );
	}

	check_improve( ch, gsn_disguise, TRUE, 3 );
    }
}
