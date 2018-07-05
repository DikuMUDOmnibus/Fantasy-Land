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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"

DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_emote);


char *  const   dir_rev     [MAX_DIR]      =
{
    "the south", "the west", "the north", "the east", "below", "above"//,
//    "the southeast", "the southwest", "the northeast", "the northwest"
};

char *  const   dir_ref     [MAX_DIR]      =
{
    "to the north", "to the east", "to the south", "to the west", "above", "below"//,
//    "the southeast", "the southwest", "the northeast", "the northwest"
};

void leave_strings  args( ( CHAR_DATA *ch, OBJ_DATA *prop, int sect, int door, bool fWindow ) );
void arrive_strings args( ( CHAR_DATA *ch, OBJ_DATA *prop, int sect, int door, bool fWindow ) );


void do_stand( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (argument[0] != '\0')
    {
	if (ch->position == POS_FIGHTING)
	{
	    send_to_char("Maybe you should finish fighting first?\n\r",ch);
	    return;
	}
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	if (obj->item_type != ITEM_FURNITURE
	||  (!IS_SET(obj->value[2],STAND_AT)
	&&   !IS_SET(obj->value[2],STAND_ON)
	&&   !IS_SET(obj->value[2],STAND_IN)))
	{
	    send_to_char("You can't seem to find a place to stand.\n\r",ch);
	    return;
	}
	if (ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    act("There's no room to stand on $p.",
		ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
	}
	ch->on = obj;

	if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
	    p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );

    }
    
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }
	
	if (obj == NULL)
	{
	    send_to_char( "You wake and stand up.\n\r", ch );
	    act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	   act("You wake and stand at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	   act("$n wakes and stands at $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    act("You wake and stand on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    act("$n wakes and stands on $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	}
	else 
	{
	    act("You wake and stand in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    act("$n wakes and stands in $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	}
	ch->position = POS_STANDING;
	do_look(ch,"auto");
	break;

    case POS_RESTING: case POS_SITTING:
	if (obj == NULL)
	{
	    send_to_char( "You stand up.\n\r", ch );
	    act( "$n stands up.", ch, NULL, NULL, TO_ROOM,POS_RESTING );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	    act("You stand at $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    act("$n stands at $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    act("You stand on $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    act("$n stands on $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	}
	else
	{
	    act("You stand in $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    act("$n stands on $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	}
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}

/* RW Enter movable exits */
void enter_exit( CHAR_DATA *ch, char *arg)
{    
    ROOM_INDEX_DATA *location; 
    int track;

    if ( ch->fighting != NULL ) 
	return;

    /* nifty portal stuff */
    if (arg[0] != '\0')
    {
        ROOM_INDEX_DATA *old_room;
	OBJ_DATA *portal;
	CHAR_DATA *fch, *fch_next;

        old_room = ch->in_room;

	portal = get_obj_list( ch, arg,  ch->in_room->contents );
	
	if (portal == NULL)
	{
	    send_to_char("Alas, you cannot go that way.\n\r",ch);
	    return;
	}

	if (portal->item_type != ITEM_EXIT) 
	{
	    send_to_char("Alas, you cannot go that way.\n\r",ch);
	    return;
	}

	location = get_room_index(portal->value[0]);

	if (location == NULL
	||  location == old_room
	||  !can_see_room(ch,location) 
	||  (room_is_private(location) && !IS_TRUSTED(ch,MAX_LEVEL)))
	{
	    send_to_char("Alas, you cannot go that way.\n\r",ch);
	    return;
	}


	if ( IS_SET(location->area->area_flags,AREA_UNLINKED)
	&&   ch->pcdata && !IS_IMMORTAL(ch) )
	{
	    send_to_char("I'm sorry, that exit leads to an unlinked area and you may not enter.\n\r",ch);
	    return;
	}

	if ( IS_AFFECTED(ch, AFF_CHARM)
	&&   ch->master != NULL
	&&   old_room == ch->master->in_room )
	{
	    send_to_char( "What?  And leave your beloved master?\n\r", ch );
	    return;
	}

        if (IS_NPC(ch) && IS_SET(ch->act,ACT_AGGRESSIVE)
        &&  IS_SET(location->room_flags,ROOM_LAW))
        {
            send_to_char("You aren't allowed in the city.\n\r",ch);
            return;
        }

	if ( !IS_NPC(ch) )
	{
	    int move;

	    if ( old_room->sector_type == SECT_AIR
	    ||   location->sector_type == SECT_AIR )
	    {
		if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(ch))
		{
		    send_to_char( "You can't fly.\n\r", ch );
		    return;
		}
	    }

	    if (( old_room->sector_type == SECT_WATER_NOSWIM
	    ||    location->sector_type == SECT_WATER_NOSWIM )
  	    &&    !IS_AFFECTED(ch,AFF_FLYING))
	    {
		OBJ_DATA *obj;
		bool found;

		/*
		* Look for a boat.
		*/
		found = FALSE;

		if (IS_IMMORTAL(ch))
		    found = TRUE;

		for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
		{
		    if ( obj->item_type == ITEM_BOAT )
		    {
			found = TRUE;
			break;
		    }
		}
		if ( !found )
		{
		    send_to_char( "You need a boat to go there.\n\r", ch );
		    return;
		}
	    }

	    move = movement_loss[UMIN(SECT_MAX-1, old_room->sector_type)]
	     + movement_loss[UMIN(SECT_MAX-1, location->sector_type)]
	     ;

            move /= 2;  /* i.e. the average */


	    /* conditional effects */
	    if (IS_AFFECTED(ch,AFF_FLYING) || IS_AFFECTED(ch,AFF_HASTE))
		move /= 2;

	    if (IS_AFFECTED(ch,AFF_SLOW))
		move *= 2;

	    if ( ch->move < move )
	    {
		send_to_char( "You are too exhausted.\n\r", ch );
		return;
	    }

	    WAIT_STATE( ch, 1 );
	    ch->move -= move;
	}


	if ( !IS_AFFECTED(ch, AFF_SNEAK)
	&&   ch->invis_level <= LEVEL_HERO
	&&   ch->ghost_level <= LEVEL_HERO)
	{
	    act( "$n leaves $p.", ch,portal,NULL,TO_ROOM,POS_RESTING);
	}

	char_from_room(ch);
	char_to_room(ch, location);
	if (IS_NPC(ch) || !IS_IMMORTAL(ch))
	{
	    for (track = MAX_TRACK-1; track > 0; track--)
	    {
		ch->track_to[track] = ch->track_to[track-1];
		ch->track_from[track] = ch->track_from[track-1];
	    }
	    if (IS_AFFECTED(ch,AFF_FLYING))
	    {
		ch->track_from[0] = 0;
		ch->track_to[0] = 0;
	    } else {
		ch->track_from[0] = old_room->vnum;
		ch->track_to[0] = location->vnum;
	    }
	}

	if ( !IS_AFFECTED(ch, AFF_SNEAK)
	&&   ch->invis_level <= LEVEL_HERO
	&&   ch->ghost_level <= LEVEL_HERO)
	{
	    act("$n has arrived.",ch,NULL,NULL,TO_ROOM,POS_RESTING);
	    if (IS_NPC(ch))
	    {
		if( ch->pIndexData && ch->pIndexData->say_descr[0] != '\0')
		{
		    act( "$n says '{a$T{x'", ch, NULL, ch->pIndexData->say_descr, TO_ROOM, POS_RESTING );
		}
	    }
	}

	do_look(ch,"auto");

	/* protect against circular follows */
	if (old_room == location)
	    return;

    	for ( fch = old_room->people; fch != NULL; fch = fch_next )
    	{
            fch_next = fch->next_in_room;

            if (portal == NULL) 
                continue;
 
            if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
            &&   fch->position < POS_STANDING)
            	do_stand(fch,"");

            if ( fch->master == ch && fch->position == POS_STANDING
	    &&   can_see_room(fch,location))
            {
 
                if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
                &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
                {
                    act("You can't bring $N into the city.",
                    	ch,NULL,fch,TO_CHAR,POS_RESTING);
                    act("You aren't allowed in the city.",
                    	fch,NULL,NULL,TO_CHAR,POS_RESTING);
                    continue;
            	}
 
            	act( "You follow $N.", fch, NULL, ch, TO_CHAR, POS_RESTING );
		enter_exit(fch,arg);
            }
    	}
	return;
    }

    send_to_char("Alas, you cannot go that way.\n\r",ch);
    return;
}

void move_char( CHAR_DATA *ch, int door, bool follow, bool quiet )
{
    static int depth;
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    OBJ_DATA *prop=NULL;
    int track;

    if ( door < 0 || door >= MAX_DIR )
    {
	bug( "Do_move: bad door %d.", door );
	return;
    }

    if ( is_affected( ch, gsn_trapset ) )
    {
	send_to_char( "You seemed to be trapped in this room.\n\r", ch );
	return;
    }
    
    if ( !IS_NPC(ch) 
      && (p_exit_trigger( ch, door, PRG_MPROG ) 
      ||  p_exit_trigger( ch, door, PRG_OPROG )
      ||  p_exit_trigger( ch, door, PRG_RPROG )) )
	return;

    if (!IS_NPC(ch))
    {
	if (ch->pcdata->condition[COND_DRUNK] > 0)
	{
	    if (ch->pcdata->condition[COND_DRUNK] > number_percent())
	    {
		act("You feel a little drunk.. not to mention kind of lost..", ch,NULL,NULL,TO_CHAR,POS_SLEEPING);
		act("$n looks a little drunk.. not to mention kind of lost..", ch,NULL,NULL,TO_ROOM,POS_RESTING);
		door = number_range(0,MAX_DIR);
	    } else {
		act("You feel a little.. drunk..",ch,NULL,NULL,TO_CHAR,POS_SLEEPING);
		act("$n looks a little.. drunk..",ch,NULL,NULL,TO_ROOM,POS_RESTING);
	    }
	}
    } else {
     if ( IS_AFFECTED(ch, AFF_BLIND)
      || (ch->riding
       && IS_AFFECTED(ch->riding, AFF_BLIND) ) )
//       && skill_check(ch, skill_lookup("riding"), 50)) )
     {
        if ( door != DIR_UP && door != DIR_DOWN )
        door = depth != 0 ? door : number_range(0,100) % door;

        if ( door == DIR_UP || door == DIR_DOWN )
        door = number_range( 0, MAX_DIR );
     }
    }

    in_room = ch->in_room;
    if ( ( pexit = in_room->exit[door] ) == NULL)
    {
	if (!quiet)
	{
	    OBJ_DATA *portal;

	    portal = get_obj_list( ch, dir_name[door],  ch->in_room->contents );
	    if (portal != NULL)
	    {
		enter_exit( ch, dir_name[door] );
		return;
	    }
	}
    }

    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL 
    ||   IS_SET(pexit->rs_flags,EX_NOMOVE)
    ||	 !can_see_room(ch,pexit->u1.to_room) )
    {
	if (!quiet)
	    send_to_char( "Alas, you can not go that way.\n\r", ch );
	return;
    }

    if ( ch->riding && ch->riding->move <5 )  {

            to_actor( "Your mount is too exhausted.\n\r", ch );
            do_emote( ch->riding, "pants and gasps from exhaustion." );
       return;
    }

    if ( ch->riding && ch->riding->position < POS_STANDING ) {
      do_stand(ch->riding,"");
    }


    if ( IS_SET(pexit->rs_flags, EX_WINDOW)
      && pexit->key == -1 )
    {
        to_actor( "You cannot go that way.\n\r", ch );
        return;
    }


    if ( IS_SET(to_room->area->area_flags,AREA_UNLINKED)
    &&   ch->pcdata && !IS_IMMORTAL(ch) )
    {
	send_to_char("I'm sorry, that exit leads to an unlinked area and you may not enter.\n\r",ch);
	return;
    }

    if ( !check_builder( ch, to_room->area, TRUE ) )
	return;

    if (to_room == get_room_index(ROOM_VNUM_TEMPLE)
    ||  to_room == get_room_index(ROOM_VNUM_TEMPLEB))
    {
	if (ch->alignment < 0)
	    to_room = get_room_index(ROOM_VNUM_TEMPLEB);
	else
	    to_room = get_room_index(ROOM_VNUM_TEMPLE);
    }

    if (to_room == get_room_index(ROOM_VNUM_ALTAR)
    ||  to_room == get_room_index(ROOM_VNUM_ALTARB))
    {
	if (ch->alignment < 0)
	    to_room = get_room_index(ROOM_VNUM_ALTARB);
	else
	    to_room = get_room_index(ROOM_VNUM_ALTAR);
    }

    if ( IS_SET( pexit->exit_info, EX_CLOSED ) && !IS_IMMORTAL( ch ) )
    {
	if ( IS_SET( pexit->exit_info, EX_NOPASS )
	||   !IS_AFFECTED( ch, AFF_PASS_DOOR ) )
	{
	    if (!quiet)
		act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR, POS_RESTING );
	    return;
	}
    }

    if ( IS_SET(pexit->exit_info, EX_CONCEALED) && !IS_SET(pexit->exit_info, EX_REVEALED) ) {
     SET_BIT(pexit->exit_info,EX_REVEALED);
     act( "An exit $d is revealed.", ch, NULL, dir_name[door], TO_ALL, POS_RESTING );
    }


    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
	if (!quiet)
	    send_to_char( "What?  And leave your beloved master?\n\r", ch );
	return;
    }

    if ( room_is_private( to_room ) )
    {
	if (!quiet)
	    send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) )
    {
	int move;

	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR )
	{
	    if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(ch))
	    {
		if (!quiet)
		    send_to_char( "You can't fly.\n\r", ch );
		return;
	    }
	}

	if (( in_room->sector_type == SECT_WATER_NOSWIM
	||    to_room->sector_type == SECT_WATER_NOSWIM )
  	&&    !IS_AFFECTED(ch,AFF_FLYING))
	{
	    OBJ_DATA *obj;
	    bool found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    if (IS_IMMORTAL(ch))
		found = TRUE;

	    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
                    prop=obj;
		    break;
		}
	    }
	    if ( !found )
	    {
		if (!quiet)
		    send_to_char( "You need a boat to go there.\n\r", ch );
		return;
	    }
	}

	move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
	     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)]
	     ;

        move /= 2;  /* i.e. the average */


	/* conditional effects */
	if (IS_AFFECTED(ch,AFF_FLYING) || IS_AFFECTED(ch,AFF_HASTE))
	    move /= 2;

	if (IS_AFFECTED(ch,AFF_SLOW))
	    move *= 2;

	if ( ch->move < move )
	{
	    if (!quiet)
		send_to_char( "You are too exhausted.\n\r", ch );
	    return;
	}

	ch->move -= move;
    }

    int sect=to_room->sector_type;
    bool fWindow = in_room->exit[door] && IS_SET(in_room->exit[door]->exit_info, EX_WINDOW) ? 1 : 0;

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 0 )
    {
	act("$n stumbles off drunkenly on $s way $T.",ch,NULL,dir_name[door],TO_ROOM,POS_RESTING);
	WAIT_STATE( ch, 2 );
    }

    else
    {
	WAIT_STATE( ch, 1 );           
	if ( !IS_AFFECTED(ch, AFF_SNEAK)
	&&   ch->invis_level <= LEVEL_HERO
	&&   ch->ghost_level <= LEVEL_HERO
	&&   !quiet )
    leave_strings( ch, prop, sect, door, fWindow );
//	    act( "$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM, POS_RESTING );
    }

    char_from_room( ch );
    char_to_room( ch, to_room );


    /*
     * Guard against rider-mount inconsistencies.
     */
    if ( ch->riding != NULL
      && ch->in_room != ch->riding->in_room
      && depth == 0 )
    {
        depth++;
        move_char( ch->riding, door, TRUE, FALSE );
        if ( ch->riding->in_room != ch->in_room )
        {
        ch->riding->rider = NULL;
        ch->riding = NULL;
        to_actor( "You are forced to leave your mount behind.\n\r", ch );
        }
        depth--;
    }
    else
    if ( ch->rider != NULL
      && ch->in_room != ch->rider->in_room
      && depth == 0 )
    {
        depth++;
        move_char( ch->rider, door, TRUE, FALSE );
        if ( ch->rider->in_room != ch->in_room )
        {
        ch->rider->riding = NULL;
        ch->rider = NULL;
        to_actor( "You are forced to leave your rider behind.\n\r", ch );
        }
        depth--;
    }

    if ( ch->pcdata && ch->pcdata->spam_count > 0 )
	ch->pcdata->spam_count = 0;

    if ( is_affected( ch, gsn_camouflage ) )
    {
	send_to_char( "You stand up from your hiding place.\n\r", ch );
	affect_strip( ch, gsn_camouflage );
    }

    if ( is_affected(ch,gsn_obfuscate)
    &&   number_percent() > (get_skill(ch,gsn_obfuscate) - 10) )
    {
	send_to_char("You failed to find a hiding place as you entered.\n\r",ch);
	affect_strip(ch,gsn_obfuscate);
    }

    if ( is_affected(ch,gsn_forest_meld) )
    {
	if ( to_room->sector_type != SECT_FOREST )
	{
	    send_to_char( "You emerge from the forest.\n\r", ch );
	    affect_strip(ch,gsn_forest_meld);
	}

	else if ( number_percent() > (get_skill(ch,gsn_forest_meld) - 10) )
	{
	    send_to_char("You failed to find a hiding place as you entered.\n\r",ch);
	    affect_strip(ch,gsn_forest_meld);
	}
    }

    if (IS_NPC(ch) || !IS_IMMORTAL(ch))
    {
	for (track = MAX_TRACK-1; track > 0; track--)
	{
	    ch->track_to[track] = ch->track_to[track-1];
	    ch->track_from[track] = ch->track_from[track-1];
	}
	if (IS_AFFECTED(ch,AFF_FLYING))
	{
	    ch->track_from[0] = 0;
	    ch->track_to[0] = 0;
	} else {
	    ch->track_from[0] = in_room->vnum;
	    ch->track_to[0] = to_room->vnum;
	}
    }

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 0)
	act("$n stumbles in drunkenly.",ch,NULL,NULL,TO_ROOM,POS_RESTING);
    else
    {
	if ( !IS_AFFECTED(ch, AFF_SNEAK)
	&&   ch->invis_level <= LEVEL_HERO
	&&   ch->ghost_level <= LEVEL_HERO
	&&   !quiet )
        arrive_strings( ch, prop, sect, door, fWindow );
//	    act( "$n has arrived.", ch, NULL, NULL, TO_ROOM, POS_RESTING );

	if (IS_NPC(ch) && ch->pIndexData->say_descr[0] != '\0' && !quiet)
	    act( "$n says '{S$T{x'", ch, NULL, ch->pIndexData->say_descr, TO_ROOM, POS_RESTING );

    }

    if (!quiet)
	do_look( ch, "auto" );

    if (in_room == to_room) /* no circular follows */
	return;

    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
	fch_next = fch->next_in_room;

	if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM) 
	&&   fch->position < POS_STANDING)
	    do_stand(fch,"");

	if ( fch->master == ch && fch->position == POS_STANDING 
	&&   can_see_room(fch,to_room))
	{

	    if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
	    &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
	    {
		act("You can't bring $N into the city.",
		    ch,NULL,fch,TO_CHAR,POS_RESTING);
		act("You aren't allowed in the city.",
		    fch,NULL,NULL,TO_CHAR,POS_RESTING);
		continue;
	    }

	    act( "You follow $N.", fch, NULL, ch, TO_CHAR, POS_RESTING );
	    move_char( fch, door, TRUE, FALSE );
	}
    }
    
    /* 
     * If someone is following the char, these triggers get activated
     * for the followers before the char, but it's safer this way...
     */

    if ( IS_NPC( ch ) && HAS_TRIGGER_MOB( ch, TRIG_ENTRY ) )
	p_percent_trigger( ch, NULL, NULL, NULL, NULL, NULL, TRIG_ENTRY );

    if ( !IS_NPC( ch ) )
    {
    	p_greet_trigger( ch, PRG_MPROG );
	p_greet_trigger( ch, PRG_OPROG );
	p_greet_trigger( ch, PRG_RPROG );
    }

    if ( IS_SET ( to_room->room_flags, ROOM_ICY ) && !IS_IMMORTAL( ch ) )
    {
	track = 45 + get_curr_stat( ch, STAT_DEX );

	if ( number_percent( ) > track )
	{
	    if ( to_room->exit[door] != NULL )
	    {
		if ( IS_SET( to_room->exit[door]->exit_info, EX_CLOSED )
		&&   ( IS_SET( to_room->exit[door]->exit_info, EX_NOPASS )
		||    !IS_SET( ch->affected_by, AFF_PASS_DOOR ) ) )
		{
		    act( "\n\r{CYou slip on a patch of ice and slide right into the $d!{x\n\r\n\r",
			ch, NULL, to_room->exit[door]->keyword, TO_CHAR, POS_RESTING );
		    act( "\n\r{C$n slips on a patch of ice and slide right into the $d!{x\n\r",
			ch, NULL, to_room->exit[door]->keyword, TO_ROOM, POS_RESTING );
		    ch->position = POS_RESTING;
		} else {
		    send_to_char( "\n\r{CYou slip on some ice and slide clear through to the next room!{x\n\r\n\r", ch );
		    act( "\n\r{C$n slips on a patch of ice and slides clear through to the next room!{x\n\r",
			ch, NULL, NULL, TO_ROOM, POS_RESTING );
		    move_char( ch, door, FALSE, FALSE );
		}
	    } else {
		send_to_char( "\n\r{CYou slip on some ice and slide right into the wall!{x\n\r\n\r", ch );
		act( "\n\r{C$n slips on a patch of ice and slides right into the wall!{x\n\r",
		    ch, NULL, NULL, TO_ROOM, POS_RESTING );
		ch->position = POS_RESTING;
	    }
	}
    }

    return;
}

void do_north( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_room;

    was_room = ch->in_room;
    move_char( ch, DIR_NORTH, FALSE, FALSE );
    if (was_room == ch->in_room)
	free_runbuf(ch->desc);
    return;
}

void do_east( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_room;

    was_room = ch->in_room;
    move_char( ch, DIR_EAST, FALSE, FALSE );
    if (was_room == ch->in_room)
	free_runbuf(ch->desc);
    return;
}

void do_south( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_room;

    was_room = ch->in_room;
    move_char( ch, DIR_SOUTH, FALSE, FALSE );
    if (was_room == ch->in_room)
	free_runbuf(ch->desc);
    return;
}

void do_west( CHAR_DATA *ch, char *argument )
{

    ROOM_INDEX_DATA *was_room;

    was_room = ch->in_room;
    move_char( ch, DIR_WEST, FALSE, FALSE );
    if (was_room == ch->in_room)
      free_runbuf(ch->desc);
    return;

}



void do_up( CHAR_DATA *ch, char *argument )
{

    ROOM_INDEX_DATA *was_room;

    was_room = ch->in_room;
    move_char( ch, DIR_UP, FALSE, FALSE );
    if (was_room == ch->in_room)
      free_runbuf(ch->desc);
    return;

}



void do_down( CHAR_DATA *ch, char *argument )
{

    ROOM_INDEX_DATA *was_room;

    was_room = ch->in_room;
    move_char( ch, DIR_DOWN, FALSE, FALSE );
    if (was_room == ch->in_room)
      free_runbuf(ch->desc);
    return;

}

int find_door( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int door;

	 if ( !str_prefix( arg, "north" ) ) door = 0;
    else if ( !str_prefix( arg, "east"  ) ) door = 1;
    else if ( !str_prefix( arg, "south" ) ) door = 2;
    else if ( !str_prefix( arg, "west"  ) ) door = 3;
    else if ( !str_prefix( arg, "up"    ) ) door = 4;
    else if ( !str_prefix( arg, "down"  ) ) door = 5;
    else
    {
	for ( door = 0; door <= 5; door++ )
	{
	    if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword ) )
		return door;
	}
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR, POS_RESTING );
	return -1;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	act( "I see no door $T here.", ch, NULL, arg, TO_CHAR, POS_RESTING );
	return -1;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return -1;
    }

    return door;
}

void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if ( str_prefix(arg,"north") && str_prefix(arg,"south")
    &&   str_prefix(arg,"west")  && str_prefix(arg,"east")
    &&   str_prefix(arg,"up")    && str_prefix(arg,"down") )
    {
	if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
	{
 	    /* open portal */
	    if (obj->item_type == ITEM_PORTAL)
	    {
		if (!IS_SET(obj->value[1], EX_ISDOOR))
		{
		    send_to_char("You can't do that.\n\r",ch);
		    return;
		}

		if (!IS_SET(obj->value[1], EX_CLOSED))
		{
		    send_to_char("It's already open.\n\r",ch);
		    return;
		}

		if (IS_SET(obj->value[1], EX_LOCKED))
		{
		    send_to_char("It's locked.\n\r",ch);
		    return;
		}

		REMOVE_BIT(obj->value[1], EX_CLOSED);
		act("You open $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
		act("$n opens $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
		return;
	    }

	    /* 'open object' */
	    if ( ( obj->item_type != ITEM_CONTAINER )
	    &&   ( obj->item_type != ITEM_PIT ) )
	    {
		send_to_char( "That's not a container.\n\r", ch );
		return;
	    }
	    if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It's already open.\n\r", ch );
		return;
	    }
	    if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    {
		send_to_char( "You can't do that.\n\r", ch );
		return;
	    }
	    if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    {
		send_to_char( "It's locked.\n\r", ch );
		return;
	    }

	    REMOVE_BIT(obj->value[1], CONT_CLOSED);
	    act("You open $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    act( "$n opens $p.", ch, obj, NULL, TO_ROOM, POS_RESTING );
	    return;
	}
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'open door' */
	CHAR_DATA *rch;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;
	bool found = FALSE;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    send_to_char( "It's already open.\n\r", ch );
	    return;
	}
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	{
	    send_to_char( "It's locked.\n\r", ch );
	    return;
	}

        if ( IS_SET(pexit->exit_info, EX_JAMMED) )
        {
            to_actor( "The lock is jammed.\n\r", ch );
            return;
        }

	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
	    {
		if (!IS_NPC(rch) && rch->pcdata->opponent != NULL && !check_pktest(ch,rch))
		{
		    found = TRUE;
		    break;
		}
	    }
	}

	for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
	{
	    if (!IS_NPC(rch) && rch->pcdata->opponent != NULL && !check_pktest(ch,rch))
	    {
		found = TRUE;
		break;
	    }
	}

	if (found)
	{
	    send_to_char("Opening the door would interfere with the pk battle on this / the other side.\n\r",ch);
	    return;
	}

	REMOVE_BIT(pexit->exit_info, EX_CLOSED);

        if ( !MTD(pexit->keyword) ) {
//        act( "$n open$v the $t.", ch, pexit->keyword, NULL, TO_ALL, POS_RESTING );
  	act( "$n opens the $T $t.", ch, dir_ref[door], pexit->keyword, TO_ROOM, POS_RESTING );
	act( "You open the $T $t.", ch, dir_ref[door], pexit->keyword, TO_CHAR, POS_RESTING );
        } else {
  	act( "$n opens the door $t.", ch, dir_ref[door], NULL, TO_ROOM, POS_RESTING );
	act( "You open the door $t.", ch, dir_ref[door], NULL, TO_CHAR, POS_RESTING );
        }

	/* open the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
            if ( !MTD(pexit_rev->keyword) ) {
 	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room ) 
		act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR, POS_RESTING );
            } else {
 	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room ) 
		act( "The door $t opens.", rch, NULL, dir_ref[rev_dir[door]], TO_CHAR, POS_RESTING );
            }
	}
    }

    return;
}

void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }

    if ( str_prefix(arg,"north") && str_prefix(arg,"south")
    &&   str_prefix(arg,"west")  && str_prefix(arg,"east")
    &&   str_prefix(arg,"up")    && str_prefix(arg,"down") )
    {
	if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
	{
	    /* portal stuff */
	    if (obj->item_type == ITEM_PORTAL)
	    {
		if (!IS_SET(obj->value[1],EX_ISDOOR)
		||   IS_SET(obj->value[1],EX_NOCLOSE))
		{
		    send_to_char("You can't do that.\n\r",ch);
		    return;
		}

		if (IS_SET(obj->value[1],EX_CLOSED))
		{
		    send_to_char("It's already closed.\n\r",ch);
		    return;
		}

		SET_BIT(obj->value[1],EX_CLOSED);
		act("You close $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
		act("$n closes $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
		return;
	    }

	    /* 'close object' */
	    if ( ( obj->item_type != ITEM_CONTAINER )
	    &&   ( obj->item_type != ITEM_PIT ) )
	    {
		send_to_char( "That's not a container.\n\r", ch );
		return;
	    }
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It's already closed.\n\r", ch );
		return;
	    }
	    if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    {
		send_to_char( "You can't do that.\n\r", ch );
		return;
	    }

	    SET_BIT(obj->value[1], CONT_CLOSED);
	    act("You close $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    act( "$n closes $p.", ch, obj, NULL, TO_ROOM, POS_RESTING );
	    return;
	}
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	CHAR_DATA *rch;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;
	bool found = FALSE;

	pexit	= ch->in_room->exit[door];
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    send_to_char( "It's already closed.\n\r", ch );
	    return;
	}

	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
	    {
		if (!IS_NPC(rch) && rch->pcdata->opponent != NULL && !check_pktest(ch,rch))
		{
		    found = TRUE;
		    break;
		}
	    }
	}

	for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
	{
	    if (!IS_NPC(rch) && rch->pcdata->opponent != NULL && !check_pktest(ch,rch))
	    {
		found = TRUE;
		break;
	    }
	}

	if (found)
	{
	    send_to_char("Closing the door would interfere with the pk battle on this / the other side.\n\r",ch);
	    return;
	}

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING );
	act( "You close the $d.", ch, NULL, pexit->keyword, TO_CHAR, POS_RESTING );

	/* close the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR, POS_RESTING );
	}
    }
    return;
}

bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	{
	    if ( IS_OBJ_STAT( obj, ITEM_DISINTEGRATE ) )
		obj->timer = 1;
	    return TRUE;
	}
    }

    return FALSE;
}

void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR)
	    ||  IS_SET(obj->value[1],EX_NOCLOSE))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }
	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
	 	return;
	    }

	    if (obj->value[4] < 0 || IS_SET(obj->value[1],EX_NOLOCK))
	    {
		send_to_char("It can't be locked.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already locked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_LOCKED);
	    act("You lock $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    act("$n locks $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    return;
	}

	/* 'lock object' */
	if ( ( obj->item_type != ITEM_CONTAINER )
	&&   ( obj->item_type != ITEM_PIT ) )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	act("You lock $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	act( "$n locks $p.", ch, obj, NULL, TO_ROOM, POS_RESTING );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING );

	/* lock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
 	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("It can't be unlocked.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already unlocked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act("You unlock $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    act("$n unlocks $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    return;
	}

	/* 'unlock object' */
	if ( ( obj->item_type != ITEM_CONTAINER )
	&&   ( obj->item_type != ITEM_PIT ) )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	act("You unlock $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM,POS_RESTING);
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM,POS_RESTING);

	/* unlock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pick what?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	    act( "$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR, POS_RESTING );
	    return;
	}
    }

    if ( !IS_NPC(ch) && number_percent( ) > get_skill(ch,gsn_pick_lock))
    {
	send_to_char( "You failed.\n\r", ch);
	check_improve(ch,gsn_pick_lock,FALSE,2);
	return;
    }

    if ( ( obj = get_obj_here( ch, NULL, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {	
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("It can't be unlocked.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_PICKPROOF))
	    {
		send_to_char("You failed.\n\r",ch);
		return;
	    }

	    TOGGLE_BIT(obj->value[1],EX_LOCKED);
	    act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    check_improve(ch,gsn_pick_lock,TRUE,2);
	    return;
	}

	/* 'pick object' */
	if ( ( obj->item_type != ITEM_CONTAINER )
	&&   ( obj->item_type != ITEM_PIT ) )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	TOGGLE_BIT(obj->value[1], CONT_LOCKED);
        act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
        act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	check_improve(ch,gsn_pick_lock,TRUE,2);
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 && !IS_IMMORTAL(ch))
	    { send_to_char( "It can't be picked.\n\r",     ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
	    { send_to_char( "You failed.\n\r",             ch ); return; }

	TOGGLE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM,POS_RESTING );
	check_improve(ch,gsn_pick_lock,TRUE,2);

	/* pick the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    TOGGLE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}

void do_rest( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (ch->position == POS_FIGHTING)
    {
	send_to_char("You are already fighting!\n\r",ch);
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
        { send_to_char( "You can't wake up!\n\r", ch ); return; }

    /* okay, now that we know we can rest, find an object to rest on */
    if (argument[0] != '\0')
    {
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else obj = ch->on;

    if (obj != NULL)
    {
        if (!IS_SET(obj->item_type,ITEM_FURNITURE) 
    	||  (!IS_SET(obj->value[2],REST_ON)
    	&&   !IS_SET(obj->value[2],REST_IN)
    	&&   !IS_SET(obj->value[2],REST_AT)))
    	{
	    send_to_char("You can't rest on that.\n\r",ch);
	    return;
    	}

        if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
        {
	    act("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
    	}
	
	ch->on = obj;

	if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
	    p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	if (obj == NULL)
	{
	    send_to_char( "You wake up and start resting.\n\r", ch );
	    act ("$n wakes up and starts resting.",ch,NULL,NULL,TO_ROOM,POS_RESTING);
	}
	else if (IS_SET(obj->value[2],REST_AT))
	{
	    act("You wake up and rest at $p.",
		    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
	    act("$n wakes up and rests at $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	}
        else if (IS_SET(obj->value[2],REST_ON))
        {
            act("You wake up and rest on $p.",
                    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
            act("$n wakes up and rests on $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
        }
        else
        {
            act("You wake up and rest in $p.",
                    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
            act("$n wakes up and rests in $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
        }
	ch->position = POS_RESTING;
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_STANDING:
	if (obj == NULL)
	{
	    send_to_char( "You rest.\n\r", ch );
	    act( "$n sits down and rests.", ch, NULL, NULL, TO_ROOM,POS_RESTING );
	}
        else if (IS_SET(obj->value[2],REST_AT))
        {
	    act("You sit down at $p and rest.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    act("$n sits down at $p and rests.",ch,obj,NULL,TO_ROOM,POS_RESTING);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
	    act("You sit on $p and rest.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    act("$n sits on $p and rests.",ch,obj,NULL,TO_ROOM,POS_RESTING);
        }
        else
        {
	    act("You rest in $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    act("$n rests in $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
        }
	ch->position = POS_RESTING;
	break;

    case POS_SITTING:
	if (obj == NULL)
	{
	    send_to_char("You rest.\n\r",ch);
	    act("$n rests.",ch,NULL,NULL,TO_ROOM,POS_RESTING);
	}
        else if (IS_SET(obj->value[2],REST_AT))
        {
	    act("You rest at $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    act("$n rests at $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
	    act("You rest on $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    act("$n rests on $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
        }
        else
        {
	    act("You rest in $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	    act("$n rests in $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	}
	ch->position = POS_RESTING;
	break;
    }


    return;
}


void do_sit (CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (ch->position == POS_FIGHTING)
    {
	send_to_char("Maybe you should finish this fight first?\n\r",ch);
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
        { send_to_char( "You can't wake up!\n\r", ch ); return; }

    /* okay, now that we know we can sit, find an object to sit on */
    if (argument[0] != '\0')
    {
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else obj = ch->on;

    if (obj != NULL)                                                              
    {
	if (!IS_SET(obj->item_type,ITEM_FURNITURE)
	||  (!IS_SET(obj->value[2],SIT_ON)
	&&   !IS_SET(obj->value[2],SIT_IN)
	&&   !IS_SET(obj->value[2],SIT_AT)))
	{
	    send_to_char("You can't sit on that.\n\r",ch);
	    return;
	}

	if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    act("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
	}

	ch->on = obj;

	if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
	    p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
    }

    switch (ch->position)
    {
	case POS_SLEEPING:
            if (obj == NULL)
            {
            	send_to_char( "You wake and sit up.\n\r", ch );
            	act( "$n wakes and sits up.", ch, NULL, NULL, TO_ROOM, POS_RESTING );
            }
            else if (IS_SET(obj->value[2],SIT_AT))
            {
            	act("You wake and sit at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
            }
            else if (IS_SET(obj->value[2],SIT_ON))
            {
            	act("You wake and sit on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
            }
            else
            {
            	act("You wake and sit in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits in $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
            }

	    ch->position = POS_SITTING;
	    break;
	case POS_RESTING:
	    if (obj == NULL)
		send_to_char("You stop resting.\n\r",ch);
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act("You sit at $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
		act("$n sits at $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    }

	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act("You sit on $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
		act("$n sits on $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    }
	    ch->position = POS_SITTING;
	    break;
	case POS_SITTING:
	    send_to_char("You are already sitting down.\n\r",ch);
	    break;
	case POS_STANDING:
	    if (obj == NULL)
    	    {
		send_to_char("You sit down.\n\r",ch);
    	        act("$n sits down on the ground.",ch,NULL,NULL,TO_ROOM,POS_RESTING);
	    }
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act("You sit down at $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
		act("$n sits down at $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    }
	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act("You sit on $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
		act("$n sits on $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    }
	    else
	    {
		act("You sit down in $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
		act("$n sits down in $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    }
    	    ch->position = POS_SITTING;
    	    break;
    }
    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING: 
	if (argument[0] == '\0' && ch->on == NULL)
	{
	    send_to_char( "You go to sleep.\n\r", ch );
	    act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    ch->position = POS_SLEEPING;
	}
	else  /* find an object and sleep on it */
	{
	    if (argument[0] == '\0')
		obj = ch->on;
	    else
	    	obj = get_obj_list( ch, argument,  ch->in_room->contents );

	    if (obj == NULL)
	    {
		send_to_char("You don't see that here.\n\r",ch);
		return;
	    }

	    if (obj->item_type != ITEM_FURNITURE
	    ||  (!IS_SET(obj->value[2],SLEEP_ON) 
	    &&   !IS_SET(obj->value[2],SLEEP_IN)
	    &&	 !IS_SET(obj->value[2],SLEEP_AT)))
	    {
		send_to_char("You can't sleep on that!\n\r",ch);
		return;
	    }

	    if (ch->on != obj && count_users(obj) >= obj->value[0])
	    {
		act("There is no room on $p for you.",
		    ch,obj,NULL,TO_CHAR,POS_DEAD);
		return;
	    }

	    ch->on = obj;

	    if (IS_SET(obj->value[2],SLEEP_AT))
	    {
		act("You go to sleep at $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
		act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    }
	    else if (IS_SET(obj->value[2],SLEEP_ON))
	    {
	        act("You go to sleep on $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
	        act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    }
	    else
	    {
		act("You go to sleep in $p.",ch,obj,NULL,TO_CHAR,POS_RESTING);
		act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM,POS_RESTING);
	    }
	    ch->position = POS_SLEEPING;

	    if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
		p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
	}
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	do_stand( ch, argument );
	return;
    }

    if ( !IS_AWAKE(ch) )
    {
	send_to_char( "You are asleep yourself!\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AWAKE(victim) )
    {
	act( "$N is already awake.", ch, NULL, victim, TO_CHAR, POS_RESTING );
	return;
    }

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
    {
	act( "You can't wake $M!", ch, NULL, victim, TO_CHAR, POS_RESTING );
	return;
    }

    if ( !IS_NPC(victim) && !can_pk(ch,victim) && !IS_IMMORTAL(ch) && !in_group(ch,victim) )
    {
	send_to_char("You can't wake people you can not attack.\n\r",ch);
	return;
    }

    act( "$n wakes you.", ch, NULL, victim, TO_VICT,POS_SLEEPING );
    do_stand(victim,"");
    return;
}

void do_camouflage( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    if ( !IS_NPC( ch )
    &&   ( ch->pcdata->pktimer > 0 || ch->pcdata->opponent != NULL ) )
    {
	send_to_char( "Your heart is racing too much.\n\r", ch );
	return;
    }

    if ( !cost_of_skill( ch, gsn_camouflage ) )
	return;

    affect_strip( ch, gsn_camouflage );

    if ( IS_AFFECTED( ch, AFF_FAERIE_FIRE ) )
    {
	send_to_char( "You glow brightly from behind the bushes.\n\r", ch );
	return;
    }

    if ( number_percent( ) < ( 9 * get_skill( ch, gsn_camouflage ) / 10) )
    {
	af.where     = TO_AFFECTS;
	af.type      = gsn_camouflage;
	af.level     = ch->level;
	af.dur_type  = DUR_TICKS;
	af.duration  = ch->level / 10;
	af.location  = APPLY_DEX;
	af.modifier  = -4;
	af.bitvector = 0;
	affect_to_char( ch, &af );

	send_to_char( "You are now one with your surroundings.\n\r", ch );
	check_improve( ch, gsn_camouflage, TRUE, 3 );
    } else {
	send_to_char( "Your try to blend in, but trip and fall alerting everyone.\n\r", ch );
	check_improve( ch, gsn_camouflage, FALSE, 3 );
    }
}

void do_forest_meld( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    if ( !IS_NPC( ch )
    &&   ( ch->pcdata->pktimer > 0 || ch->pcdata->opponent != NULL ) )
    {
	send_to_char( "Your heart is racing too much.\n\r", ch );
	return;
    }

    if ( ch->in_room == NULL || ch->in_room->sector_type != SECT_FOREST )
    {
	send_to_char( "You must be in a forest to use this.\n\r", ch );
	return;
    }

    if ( !cost_of_skill( ch, gsn_forest_meld ) )
	return;

    affect_strip( ch, gsn_forest_meld );

    if ( IS_AFFECTED( ch, AFF_FAERIE_FIRE ) )
    {
	send_to_char( "You glow brightly from behind the bushes.\n\r", ch );
	return;
    }

    if ( number_percent( ) < ( 9 * get_skill( ch, gsn_forest_meld ) / 10) )
    {
	af.where     = TO_AFFECTS;
	af.type      = gsn_forest_meld;
	af.level     = ch->level;
	af.dur_type  = DUR_TICKS;
	af.duration  = ch->level / 10;
	af.location  = APPLY_DEX;
	af.modifier  = -4;
	af.bitvector = 0;
	affect_to_char( ch, &af );

	send_to_char( "You are now one with the forest.\n\r", ch );
	check_improve( ch, gsn_forest_meld, TRUE, 3 );
    } else {
	send_to_char( "You fail to become one with the forest.\n\r", ch );
	check_improve( ch, gsn_forest_meld, FALSE, 3 );
    }
}

void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    if ( !cost_of_skill( ch, gsn_sneak ) )
	return;

    affect_strip( ch, gsn_sneak );
    REMOVE_BIT( ch->affected_by, AFF_SNEAK );

    if ( number_percent( ) < 9 * get_skill( ch, gsn_sneak ) / 10 )
    {
	af.where     = TO_AFFECTS;
	af.type      = gsn_sneak;
	af.level     = ch->level; 
	af.dur_type  = DUR_TICKS;
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char( ch, &af );

	send_to_char( "You begin moving silently.\n\r", ch );
	check_improve( ch, gsn_sneak, TRUE, 3 );
    } else {
	send_to_char( "You begin moving more loudly than ever.\n\r", ch );
	check_improve( ch, gsn_sneak, FALSE, 3 );
    }
}

void do_hide( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    if ( !cost_of_skill( ch, gsn_hide ) )
	return;

    affect_strip( ch, gsn_hide );
    REMOVE_BIT( ch->affected_by, AFF_HIDE );

    if ( number_percent( ) < 9 * get_skill( ch, gsn_hide ) / 10 )
    {
	af.where     = TO_AFFECTS;
	af.type      = gsn_hide;
	af.level     = ch->level;
	af.dur_type  = DUR_TICKS;
	af.duration  = ch->level / 5;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_HIDE;
	affect_to_char( ch, &af );

	send_to_char( "You fade away into the background.\n\r", ch );
	check_improve( ch, gsn_hide, TRUE, 3 );
    } else {
	send_to_char( "You failed.\n\r", ch );
	check_improve( ch, gsn_hide, FALSE, 3 );
    }
}

void do_obfuscate( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    if ( !IS_NPC( ch )
    &&   ( ch->pcdata->pktimer > 0 || ch->pcdata->opponent != NULL ) )
    {
	send_to_char( "Your heart is racing too much.\n\r", ch );
	return;
    }

    if ( !cost_of_skill( ch, gsn_obfuscate ) )
	return;

    affect_strip( ch, gsn_obfuscate );

    if ( IS_AFFECTED( ch, AFF_FAERIE_FIRE ) )
    {
	send_to_char("You glow brightly from behind the shadows.\n\r", ch );
	return;
    }

    if ( number_percent( ) < ( 9 * get_skill( ch, gsn_obfuscate ) / 10 ) )
    {
	af.where     = TO_AFFECTS;
	af.type      = gsn_obfuscate;
	af.level     = ch->level;
	af.dur_type  = DUR_TICKS;
	af.duration  = ch->level / 10;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char( ch, &af );

	send_to_char( "You disappear in the dark shadows.\n\r", ch );
	check_improve( ch, gsn_obfuscate, TRUE, 3 );
    } else {
	send_to_char( "You failed.\n\r", ch );
	check_improve( ch, gsn_obfuscate, FALSE, 3 );
    }
}

/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
    affect_strip ( ch, gsn_invis			);
    affect_strip ( ch, gsn_mass_invis			);
    affect_strip ( ch, gsn_sneak			);
    affect_strip ( ch, gsn_hide				);
    affect_strip ( ch, gsn_obfuscate			);
    affect_strip ( ch, gsn_camouflage			);
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE		);
    REMOVE_BIT   ( ch->shielded_by, SHD_INVISIBLE	);
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);

    send_to_char( "You emerge from the shadows.\n\r", ch );
    return;
}

void do_recall( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Only players can recall.\n\r", ch );
	return;
    }
  
    act( "$n prays for transportation!", ch, 0, 0, TO_ROOM, POS_RESTING );

    if ( ch->in_room != NULL && IS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
    {
	send_to_char( "You can't escape!\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char("You may not recall while charmed!\n\r",ch);
	return;
    }

    if ( is_clan( ch ) && ch->pcdata->pktimer <= 0 )
	location = get_room_index( clan_table[ch->clan].hall );

    else if ( ch->alignment < 0 )
	location = get_room_index( ROOM_VNUM_TEMPLEB );

    else
	location = get_room_index( ROOM_VNUM_TEMPLE );

    if ( location == NULL )
    {
	send_to_char( "You found a NULL room!\n\r", ch );
	return;
    }

    if ( !check_builder( ch, location->area, TRUE ) )
	return;

    if ( ch->in_room == location )
	return;

    if ( ch->in_room != NULL
    &&   ( IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
    ||     IS_AFFECTED( ch, AFF_CURSE ) )
    &&   ch->level < LEVEL_IMMORTAL )
    {
	act( "$G has forsaken you.", ch, NULL, NULL, TO_CHAR, POS_RESTING );
	return;
    }

    if ( number_percent( ) > get_skill( ch, gsn_recall ) * 9 / 10 )
    {
	check_improve( ch, gsn_recall, FALSE, 4 );
	act( "You failed to impress $G.",
	    ch, NULL, NULL, TO_CHAR, POS_RESTING );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_recall].beats );
	
    ch->move *= .75;

    check_improve( ch, gsn_recall, TRUE, 4 );

    act( "$n disappears.", ch, NULL, NULL, TO_ROOM, POS_RESTING );

    if ( ch->in_room != NULL )
    {
	int track;

	for ( track = MAX_TRACK-1; track > 0; track-- )
	{
	    ch->track_to[track] = ch->track_to[track-1];
	    ch->track_from[track] = ch->track_from[track-1];
	}

	ch->track_from[0] = ch->in_room->vnum;
	ch->track_to[0] = 0;

	char_from_room( ch );
    }

    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM, POS_RESTING );

    CHAR_DATA *pet;
    for ( pet = char_list; pet!=NULL; pet=pet->next ) if ( pet->master == ch && IS_NPC(pet) ) {
	char_from_room( pet );
	char_to_room( pet, ch->in_room );
    }

    do_look( ch, "auto" );
}

/*
 * Contributed by Locke
 */
void do_home( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
	send_to_char( "Only players can recall.\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "set" ) ) {
     if ( !IS_SET(ch->in_room->room_flags, ROOM_SAVE_CONTENTS) ) {
      send_to_char( "You cannot set your home to here.\n\r", ch );
      return;
     }
     ch->pcdata->home=ch->in_room->vnum;
     send_to_char( "Home set to this location.\n\r", ch );
     return;
    }

    ROOM_INDEX_DATA *location=get_room_index( ch->pcdata->home );

    act( "$n prays for transportation!", ch, 0, 0, TO_ROOM, POS_RESTING );

    if ( ch->in_room != NULL && IS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
    {
	send_to_char( "You can't escape!\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char("You may not recall while charmed!\n\r",ch);
	return;
    }

    if ( location == NULL )
    {
	send_to_char( "You can't find your way home.\n\rTip: Use 'home set' to determine where you will go home to.\n\r", ch );
	return;
    }

    if ( ch->in_room == location ) {
        send_to_char( "You're already there.\n\r", ch );
	return;
    }

    if ( ch->in_room != NULL
    &&   ( IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
    ||     IS_AFFECTED( ch, AFF_CURSE ) )
    &&   ch->level < LEVEL_IMMORTAL )
    {
	act( "$G has forsaken you.", ch, NULL, NULL, TO_CHAR, POS_RESTING );
	return;
    }

    if ( number_percent( ) > get_skill( ch, gsn_recall ) * 9 / 10 )
    {
	check_improve( ch, gsn_recall, FALSE, 4 );
	act( "You failed to impress $G.",
	    ch, NULL, NULL, TO_CHAR, POS_RESTING );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_recall].beats );
	
    ch->move *= .75;

    check_improve( ch, gsn_recall, TRUE, 4 );

    act( "$n goes home.", ch, NULL, NULL, TO_ROOM, POS_RESTING );

    if ( ch->in_room != NULL )
    {
	int track;

	for ( track = MAX_TRACK-1; track > 0; track-- )
	{
	    ch->track_to[track] = ch->track_to[track-1];
	    ch->track_from[track] = ch->track_from[track-1];
	}

	ch->track_from[0] = ch->in_room->vnum;
	ch->track_to[0] = 0;

	char_from_room( ch );
    }

    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM, POS_RESTING );

    CHAR_DATA *pet;
    for ( pet = char_list; pet!=NULL; pet=pet->next ) if ( pet->master == ch && IS_NPC(pet) ) {
	char_from_room( pet );
	char_to_room( pet, ch->in_room );
    }

    do_look( ch, "auto" );
}

    
void do_train( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    bool quiet = FALSE;
    int pos, value;

    argument = one_argument( argument, arg );

    if (IS_NPC(ch))
	return;

    for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
    {
	if (IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN))
	    break;
    }

    if (mob == NULL)
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	sprintf(buf,"{CYou have {W%d {Ctraining sessions.\n\r",ch->pcdata->train);
	send_to_char(buf,ch);
	send_to_char("{CYou can train:{W",ch);
	if (ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR)) 
	    send_to_char(" str",ch);
	if (ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT))  
	    send_to_char(" int",ch);
	if (ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS)) 
	    send_to_char(" wis",ch);
	if (ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX))  
	    send_to_char(" dex",ch);
	if (ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON))  
	    send_to_char(" con",ch);
	send_to_char(" hp mana move{C.{x\n\r",ch);
	return;
    }

    if (ch->pcdata->train < 1)
    {
	send_to_char("{CYou don't have enough training sessions.{x\n\r",ch);
	return;
    }

    if (is_number(arg))
    {
	value = atoi(arg);

	if (ch->pcdata->train < value)
	{
	    sprintf(buf,"You only have %d trains!\n\r", ch->pcdata->train);
	    send_to_char(buf,ch);
	    return;
	}

	if (value < 1)
	{
	    send_to_char("Pick a reasonable number please!\n\r",ch);
	    return;
	}

	if ( str_cmp(argument,"str")  && str_cmp(argument,"int")
	&&   str_cmp(argument,"wis")  && str_cmp(argument,"dex")
	&&   str_cmp(argument,"con")  && str_cmp(argument,"hp")
	&&   str_cmp(argument,"mana") && str_cmp(argument,"move") )
	{
	    do_train(ch,"");
	    return;
	}

	for (pos = 0; pos < value; pos++)
	{
	    sprintf(buf,"%s quiet", argument);
	    do_train(ch,buf);
	}

	sprintf(buf,"{CYou train your {W%s {C%d time%s!{x\n\r",
	    argument, value, value > 1 ? "s" : "");
	send_to_char(buf,ch);
	sprintf(buf,"{C$n's trains $s {W%s {C%d times!{x", argument, value);
	act(buf,ch,NULL,NULL,TO_ROOM,POS_RESTING);
	return;
    }

    if (!str_cmp(argument,"quiet"))
	quiet = TRUE;

    if (!str_cmp(arg,"str"))
    {
	if (ch->perm_stat[STAT_STR]  >= get_max_train(ch,STAT_STR))
	{
	    send_to_char("{CYour {Wstrength {Cis already at maximum.{x\n\r",ch);
	    return;
	}
	ch->perm_stat[STAT_STR]++;
	ch->pcdata->train--;
	if (!quiet)
	{
	    sprintf(buf,"{CYour {Wstrength {Cincreases! < {W%d{C({W%d{C) >{x\n\r",
		ch->perm_stat[STAT_STR], get_curr_stat(ch,STAT_STR) );
	    send_to_char(buf,ch);
	    act("{C$n's {Wstrength {Cincreases!{x",ch,NULL,NULL,TO_ROOM,POS_RESTING);
	}
	return;
    }
    if (!str_cmp(arg,"int"))
    {
	if (ch->perm_stat[STAT_INT] >= get_max_train(ch,STAT_INT))
	{
	    send_to_char("{CYour {Wintelligence {Cis already at maximum.{x\n\r",ch);
	    return;
	}
	ch->perm_stat[STAT_INT]++;
	ch->pcdata->train--;
	if (!quiet)
	{
	    sprintf(buf,"{CYour {Wintelligence {Cincreases! < {W%d{C({W%d{C) >{x\n\r",
		ch->perm_stat[STAT_INT], get_curr_stat(ch,STAT_INT) );
	    send_to_char(buf,ch);
	    act( "{C$n's {Wintelligence {Cincreases!{x",ch,NULL,NULL,TO_ROOM,POS_RESTING);
	}
	return;
    }
    if (!str_cmp(arg,"wis"))
    {
	if (ch->perm_stat[STAT_WIS]  >= get_max_train(ch,STAT_WIS))
	{
	    send_to_char("{CYour {Wwisdom {Cis already at maximum.{x\n\r",ch);
	    return;
	}
	ch->perm_stat[STAT_WIS]++;
	ch->pcdata->train--;
	if (!quiet)
	{
	    sprintf(buf,"{CYour {Wwisdom {Cincreases! < {W%d{C({W%d{C) >{x\n\r",
		ch->perm_stat[STAT_WIS], get_curr_stat(ch,STAT_WIS) );
	    send_to_char(buf,ch);
	    act("{C$n's {Wwisdom {Cincreases!{x",ch,NULL,NULL,TO_ROOM,POS_RESTING);
	}
	return;
    }
    if (!str_cmp(arg,"dex"))
    {
	if (ch->perm_stat[STAT_DEX]  >= get_max_train(ch,STAT_DEX))
	{
	    send_to_char("{CYour {Wdexterity {Cis already at maximum.{x\n\r",ch);
	    return;
	}
	ch->perm_stat[STAT_DEX]++;
	ch->pcdata->train--;
	if (!quiet)
	{
	    sprintf(buf,"{CYour {Wdexterity {Cincreases! < {W%d{C({W%d{C) >{x\n\r",
		ch->perm_stat[STAT_DEX], get_curr_stat(ch,STAT_DEX) );
	    send_to_char(buf,ch);
	    act("{C$n's {Wdexterity {Cincreases!{x",ch,NULL,NULL,TO_ROOM,POS_RESTING);
	}
	return;
    }
    if (!str_cmp(arg,"con"))
    {
	if (ch->perm_stat[STAT_CON]  >= get_max_train(ch,STAT_CON))
	{
	    send_to_char("{CYour {Wconstitution {Cis already at maximum.{x\n\r",ch);
	    return;
	}
	ch->perm_stat[STAT_CON]++;
	ch->pcdata->train--;
	if (!quiet)
	{
	    sprintf(buf,"{CYour {Wconstitution {Cincreases! {C< {W%d{C({W%d{C) >{x\n\r",
		ch->perm_stat[STAT_CON], get_curr_stat(ch,STAT_CON) );
	    send_to_char(buf,ch);
	    act("{C$n's {Wconstitution {Cincreases!{x",ch,NULL,NULL,TO_ROOM,POS_RESTING);
	}
	return;
    }
    if (!str_cmp(arg,"hp"))
    {
	ch->pcdata->train--;
        ch->pcdata->perm_hit += 10;
        ch->max_hit += 10;
        ch->hit +=10;
	if (!quiet)
	{
	    act( "{CYour {Wdurability {Cincreases!{x",
		ch,NULL,NULL,TO_CHAR,POS_RESTING);
	    act( "{C$n's {Wdurability {Cincreases!{x",
		ch,NULL,NULL,TO_ROOM,POS_RESTING);
	}
        return;
    } 
    if (!str_cmp(arg,"mana"))
    {
	ch->pcdata->train--;
        ch->pcdata->perm_mana += 10;
        ch->max_mana += 10;
        ch->mana += 10;
	if (!quiet)
	{
	    act( "{CYour {Wpower {Cincreases!{x",
		ch,NULL,NULL,TO_CHAR,POS_RESTING);
	    act( "{C$n's {Wpower {Cincreases!{x",
		ch,NULL,NULL,TO_ROOM,POS_RESTING);
	}
        return;
    }
    if (!str_cmp(arg,"move"))
    {
        ch->pcdata->train--;
        ch->pcdata->perm_move += 10;
        ch->max_move += 10;
        ch->move += 10;
	if (!quiet)
	{
	    act( "{CYour {Wendurance {Cincreases!{x",
		ch,NULL,NULL,TO_CHAR,POS_RESTING);
	    act( "{C$n's {Wendurance {Cincreases!{x",
		ch,NULL,NULL,TO_ROOM,POS_RESTING);
	}
        return;
    }
    do_train(ch,"");
    return;
}

void do_doorbash( CHAR_DATA *ch, char *argument ) 
{
    CHAR_DATA *gch;
    sh_int door, chance;

    if ( ( chance = get_skill( ch, gsn_doorbash ) ) <= 0 )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Which door you wanna bash?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_doorbash].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC( gch ) && IS_AWAKE( gch ) && ch->level + 5 < gch->level )
	{
	    act( "$N is standing too close to the door.",
		ch, NULL, gch, TO_CHAR, POS_RESTING );
	    return;
	}
    }

    if ( number_percent( ) > get_skill( ch, gsn_doorbash ) * 9 / 10 )
    {
	send_to_char( "You failed.\n\r", ch );
	check_improve( ch, gsn_doorbash, FALSE, 2 );
	return;
    }

    if ( ( door = find_door( ch, argument ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit = ch->in_room->exit[door], *pexit_rev;

	if ( !IS_SET( pexit->exit_info, EX_CLOSED ) && !IS_IMMORTAL( ch ) )
	    { send_to_char( "It's not closed.\n\r", ch ); return; }

	if ( pexit->key < 0 && !IS_IMMORTAL( ch ) )
	    { send_to_char( "It can't be broken.\n\r", ch ); return; }

	if ( !IS_SET( pexit->exit_info, EX_LOCKED ) )
	    { send_to_char( "It's already broken.\n\r", ch ); return; }

	if ( IS_SET( pexit->exit_info, EX_BASHPROOF ) && !IS_IMMORTAL( ch ) )
	    { send_to_char( "You failed.\n\r", ch ); return; }

	REMOVE_BIT( pexit->exit_info, EX_LOCKED );

	send_to_char( "The door goes flying as you bash it open.\n\r", ch );

	act( "$n bashes the $d and it goes flying.",
	    ch, NULL, pexit->keyword, TO_ROOM,POS_RESTING );

	check_improve( ch, gsn_doorbash, TRUE, 2 );

	/* pick the other side */
	if ( ( to_room = pexit->u1.to_room ) != NULL
	&& ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&& pexit_rev->u1.to_room == ch->in_room )
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
    }
}

void do_heel( CHAR_DATA *ch, char *argument )
{
    sh_int chance, sn = skill_lookup( "heel" );

    CHAR_DATA *pet;
    for ( pet=char_list; pet!=NULL; pet=pet->next ) if ( pet->master == ch && IS_NPC(ch) && ch->in_room != pet->in_room ) {

    if ( ( chance = get_skill( ch, sn ) ) <= 0 )
    {
	send_to_char( "You let out a startling scream.\n\r", ch );
	return;
    }

    if ( ch->in_room == NULL )
    {
	send_to_char( "Uhh, NULL room?!\n\r", ch );
	return;
    }

    if ( !cost_of_skill( ch, sn ) )
	return;

    if ( number_percent( ) > chance * 4 / 5 )
    {
	act( "$N did not hear your call.",
	    ch, NULL, pet, TO_CHAR, POS_RESTING );
	check_improve( ch, sn, FALSE, 2 );
	return;
    }

    act( "$n looks off into the distance and starts running.",
	pet, NULL, NULL, TO_ROOM, POS_RESTING );

    char_from_room( pet );
    char_to_room( pet, ch->in_room );

    act( "$n comes running into the room.",
	pet, NULL, NULL, TO_ROOM, POS_RESTING );
    }

    check_improve( ch, sn, TRUE, 2 );
}


void leave_strings( CHAR_DATA *ch, OBJ_DATA *prop, int sect, int door, bool fWindow )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;

    if ( ch->position == POS_FIGHTING )
    sprintf( buf, "You turn tail and flee %s", dir_name[door] );
    else
    if ( prop != NULL )
        sprintf( buf, "You use %s to %s %s",
                      can_see_obj( ch, prop ) ? prop->short_descr : "something",
                      sect == SECT_MOUNTAIN       ? "climb" :
                      sect == SECT_CLIMB         ? "climb" :
                      sect == SECT_WATER_NOSWIM   ? "sail" :
                      sect == SECT_AIR            ? "fly" :
//                      sect == MOVE_UNDERWATER     ? "dive" :
                      "move",
                      dir_name[door] );
    else
    if ( ch->rider != NULL )
        sprintf( buf, "You wander %s, rode by %s", dir_name[door],
                         PERS(ch->rider, ch) );
    else
    if ( ch->riding != NULL ) {
//        if ( ch->desc ) sendcli( ch->desc, "PLAYSOUND hooves.wav");
        sprintf( buf, "You ride %s %s", PERS(ch->riding, ch),
                         dir_name[door] );
        }
    else
    if ( IS_AFFECTED(ch, AFF_FLYING) )
        sprintf( buf, "You fly %s", dir_name[door] );
    else
    if ( IS_AFFECTED(ch, AFF_SNEAK) )
        sprintf( buf, "You try to sneak %s", dir_name[door] );
    else
    if ( sect == SECT_WATER_SWIM ) //|| sect == MOVE_UNDERWATER ) {
//        if ( ch->desc ) sendcli( ch->desc, "PLAYSOUND swim.wav" );
        sprintf( buf, "You swim %s", dir_name[door] );
//      }
    else
    if ( sect == SECT_CLIMB || fWindow == TRUE ) {
//        if ( ch->desc ) sendcli( ch->desc, "PLAYSOUND pick.wav" );
        sprintf( buf, "You climb %s", dir_name[door] );
    } else
        sprintf( buf, "You walk %s",  dir_name[door] );
/*    else
    if ( ch->hitched_to == NULL ) {
        if ( ch->desc ) sendcli( ch->desc, "PLAYSOUND footsteps.wav" );
     }
    else
    if ( ch->position == POS_STANDING )
        sprintf( buf, "You drag %s %s",
                      PERSO(ch->hitched_to, ch),
                      dir_name[door] );
 */

    buf[0] = UPPER(buf[0]);
    to_actor( buf, ch );

    if ( ch->in_room->exit[door]
      && !MTD(ch->in_room->exit[door]->keyword) )
    {
        if ( strstr( ch->in_room->exit[door]->keyword, "debris" ) != NULL )
        to_actor( " through ", ch );
        else {
         if ( strstr( ch->in_room->exit[door]->keyword, "path" ) != NULL )
         to_actor( " down a", ch );
         else
         if ( strstr( ch->in_room->exit[door]->keyword, "road" ) != NULL
           || strstr( ch->in_room->exit[door]->keyword, "lane" ) != NULL
           || strstr( ch->in_room->exit[door]->keyword, "concourse" ) != NULL )
         to_actor( " following a", ch );
         else
         if ( strstr( ch->in_room->exit[door]->keyword, "ledge" ) != NULL
           || strstr(ch->in_room->exit[door]->keyword, "promenade" ) != NULL )
         to_actor( " along a", ch );
         else
         if ( strstr( ch->in_room->exit[door]->keyword, "stair" ) != NULL ) {
         char buf2[MAX_STRING_LENGTH];
         sprintf( buf2, " %s%s", door == DIR_UP ? "ascending" : door == DIR_DOWN ? "descending" : "climbing",
              strstr( ch->in_room->exit[door]->keyword, "stairs" ) != NULL ? "" : " a" );
         to_actor( buf2, ch );
         }
         else
         to_actor( " through a", ch );

         if ( IS_SET(ch->in_room->exit[door]->exit_info, EX_ISDOOR) )
         to_actor( "n open ", ch );
         else {if ( IS_VOWEL(ch->in_room->exit[door]->keyword[0]) )
                to_actor( "n ", ch );
           else to_actor( " ", ch );}
        }
        to_actor( ch->in_room->exit[door]->keyword, ch );
    }
    to_actor( ch->position == POS_FIGHTING ? "!\n\r" : ".\n\r", ch );

    for ( fch = ch->in_room->people; fch != NULL; fch = fch_next )
    {
        fch_next = fch->next_in_room;

        if ( ch == fch 
          || ch->riding == fch
          || ch->rider == fch)
        continue;

        if ( !can_see( fch, ch )
          || !IS_AWAKE(fch) )
        continue;

        if ( ch->rider != NULL )
        break;

        if ( ch->position == POS_FIGHTING ) {
         sprintf( buf, "%s turns and flees %s", PERS(ch, fch), dir_name[door] );
  //       if ( ch->desc ) sendcli( ch->desc, "PLAYSOUND flee.wav" );
        }
   else if ( prop != NULL )
        sprintf( buf, "%s uses %s to %s %s",
                      PERS(ch, fch),
                      can_see_obj(fch, prop) ? prop->short_descr : "something",
                      sect == SECT_MOUNTAIN ? "climb" :
                      sect == SECT_WATER_NOSWIM ? "sail" :
                      sect == SECT_AIR ? "fly" :
//                      sect == _UNDERWATER ? "dive" : 
                      "move",
                      dir_name[door] );
/*
   else if ( ch->riding != NULL &&
        (can_see(fch, ch) || can_see(fch, ch->riding)) )
        {
        if ( ch->riding->hitched_to != NULL )
        {
              sprintf( buf, "%s rides %s upon %s, pulling %s behind them",
                            PERS(ch, fch), dir_name[door],
                            PERS(ch->riding, fch),
                            PERSO(ch->riding->hitched_to, fch) );
        }
        else
        {
              sprintf( buf, "%s rides %s %s", PERS(ch, fch),
                            PERS(ch->riding, fch), dir_name[door] );
        }
        }
*/
   else if ( IS_AFFECTED(ch, AFF_FLYING) && can_see(fch, ch) )
           sprintf( buf, "%s flies %s", PERS(ch, fch), dir_name[door] );
   else if ( IS_AFFECTED(ch, AFF_SNEAK) )
        {
            if ( (IS_NPC(fch) && IS_SET(fch->act, PLR_HOLYLIGHT)) )
//              || skill_check(fch, skill_lookup("stealth"), 25) )
           sprintf( buf, "%s tries to sneak %s", PERS(ch, fch), dir_name[door] );
           else buf[0] = '\0';
        }
   else if ( sect == SECT_WATER_SWIM ) //|| sect == MOVE_UNDERWATER )
         sprintf( buf, "%s swims %s",  PERS(ch, fch), dir_name[door] );
   else if ( sect == SECT_CLIMB )
            sprintf( buf, "%s climbs %s",  PERS(ch, fch), dir_name[door] );
   else if ( ch->position == POS_STANDING )
        {
 //       if ( ch->hitched_to == NULL || !can_see_prop(fch, ch->hitched_to) )
        sprintf( buf, "%s walks %s",  PERS(ch, fch), dir_name[door] );
//   else sprintf( buf, "%s walks %s dragging %s",
//                      PERS(ch, fch), dir_name[door],
//                      STR(ch->hitched_to,short_descr) );
        }

        buf[0] = UPPER(buf[0]);
        to_actor( buf, fch );
        if ( buf[0] != '\0' )
        {
            if ( ch->in_room->exit[door]
             && !MTD(ch->in_room->exit[door]->keyword)
             )
            {
                to_actor(
                    !str_infix_2( ch->in_room->exit[door]->keyword, "path" )
                 || !str_infix_2( ch->in_room->exit[door]->keyword, "concourse" )
                 || !str_infix_2( ch->in_room->exit[door]->keyword, "lane" )
                 || !str_infix_2( ch->in_room->exit[door]->keyword, "road" )
                 || !str_infix_2( ch->in_room->exit[door]->keyword, "thoroughfare" )
                 || !str_infix_2( ch->in_room->exit[door]->keyword, "promenade" )
                    ?  " along the " :
                 !str_infix_2( ch->in_room->exit[door]->keyword, "stair" ) ? " taking the " :
                  IS_SET( ch->in_room->exit[door]->exit_info, EX_ISDOOR ) ? " through an open "
                  : " through the ",
                fch );
                to_actor( ch->in_room->exit[door]->keyword, fch );
            }
            to_actor( ch->position == POS_FIGHTING ? "!\n\r" : ".\n\r", fch );
        }

//        sprintf( buf, "%d", door );
//        if (script_update( fch, TYPE_ACTOR, TRIG_MOVES, ch, NULL, buf, NULL ) == 2)
//        return;
    }
}


void arrive_strings( CHAR_DATA *ch, OBJ_DATA *prop, int sect, int door, bool fWindow )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;

    for ( fch = ch->in_room->people; fch != NULL; fch = fch_next )
    {
        fch_next = fch->next_in_room;

        if (ch == fch
         || ch->riding == fch
         || ch->rider == fch)
        continue;

        if ( !can_see( fch, ch )
          || !IS_AWAKE(fch) )
        continue;

        if ( ch->rider != NULL )
        break;

        if ( ch->position == POS_FIGHTING )
        sprintf( buf, "%s flees from %s", PERS(ch, fch), dir_rev[door] );
   else if ( prop != NULL )
        sprintf( buf, "%s uses %s to %s %s",
                 PERS(ch, fch),
                 PERSO(prop, fch),
                 sect == SECT_MOUNTAIN     ? "climb" :
                 sect == SECT_WATER_NOSWIM ? "sail" :
                 sect == SECT_AIR          ? "fly" :
                // sect == MOVE_UNDERWATER   ? "dive" :
                 "move",
                 dir_name[door] );
   else if ( ch->riding != NULL )
        sprintf( buf, "%s rides %s from %s", PERS(ch, fch),
                 PERS(ch->riding, fch), dir_rev[door] );
   else if ( IS_AFFECTED(ch, AFF_FLYING) )
        sprintf( buf, "%s flies in from %s", PERS(ch, fch), dir_rev[door] );
   else if ( IS_AFFECTED(ch, AFF_SNEAK) ) //&& skill_check( fch, skill_lookup("stealth"), 75 ) )
        {
        if ( !IS_NPC(fch) && IS_SET(fch->act, PLR_HOLYLIGHT) )
 //         || skill_check( fch, skill_lookup("stealth"), 25 ) )
        sprintf( buf, "%s tries to sneak in from %s", PERS(ch, fch), dir_rev[door] );
        else buf[0] = '\0';
        }
   else if ( sect == SECT_WATER_SWIM )// || sect == MOVE_UNDERWATER )
        sprintf( buf, "%s swims in from %s",  PERS(ch, fch), dir_rev[door] );
   else if ( sect == SECT_CLIMB )
        sprintf( buf, "%s climbs in from %s",  PERS(ch, fch), dir_rev[door] );
   else if ( ch->position == POS_STANDING )
    {
        if ( fWindow )
        sprintf( buf, "%s climbs in through an open %s", PERS(ch,fch),
             ch->in_room && ch->in_room->exit[door] ?
             ch->in_room->exit[door]->keyword : "window" );
        else
//   else if ( ch->hitched_to == NULL || !can_see_obj(fch, ch->hitched_to) )
        sprintf( buf, "%s walks in from %s",  PERS(ch, fch), dir_rev[door] );
/*   else sprintf( buf, "%s walks in from %s dragging %s",
                 PERS(ch, fch), dir_rev[door],
                 STR(ch->hitched_to,short_descr) ); */
    }

        buf[0] = UPPER(buf[0]);
        to_actor( buf, fch );
        if ( buf[0] != '\0' )
        {
            if ( ch->in_room->exit[rev_dir[door]]
              && !MTD(ch->in_room->exit[rev_dir[door]]->keyword) )
            {
                to_actor( " through the ", fch );
                if ( IS_SET(ch->in_room->exit[rev_dir[door]]->exit_info,EX_ISDOOR)
                 && !IS_SET(ch->in_room->exit[rev_dir[door]]->exit_info,EX_CLOSED) )
                to_actor( "open ", ch );
                to_actor( cut_to( ch->in_room->exit[rev_dir[door]]->keyword ), fch );
            }
            to_actor( ch->position == POS_FIGHTING ? "!\n\r" : ".\n\r", fch );
        }

//        sprintf( buf, "%d", rev_dir[door] );
//        script_update( fch, TYPE_ACTOR, TRIG_ENTER, ch, NULL, buf, NULL );
    }
}

