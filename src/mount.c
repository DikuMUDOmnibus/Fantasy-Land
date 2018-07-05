/******************************************************************************
 * Locke's   __                      __         NIM Server Software           *
 * ___ ___  (__)__    __ __   __ ___|  | v5     Version 5 (ALPHA)             *
 * |  /   \  __|  \__/  |  | |  |      |                                      *
 * |       ||  |        |  \_|  | ()   |                                      *
 * |    |  ||  |  |__|  |       |      |                                      *
 * |____|__|___|__|  |__|\___/__|______|        http://www.nimud.org/nimud    *
 *   n a m e l e s s i n c a r n a t e          dedicated to chris cool       *
 ******************************************************************************
 * NiMUD copyright (c) 1993-2009 by Herb Gilliland                            *
 * Includes improvements by Chris Woodward (c) 1993-1994                      *
 * Based on Merc 2.1c / 2.2                                                   *
 ******************************************************************************
 * To use this software you must comply with its license.                     *
 * licenses.  See the file 'docs/COPYING' for more information about this.    *
 ******************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,           *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.      *
 *                                                                            *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael             *
 *  Chastain, Michael Quan, and Mitchell Tse.                                 *
 *                                                                            *
 *  Much time and thought has gone into this software and you are             *
 *  benefitting.  We hope that you share your changes too.  What goes         *
 *  around, comes around.                                                     *
 ******************************************************************************/

#if defined(BSD)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "merc.h"
//#include "skills.h"

DECLARE_DO_FUN( do_sneak );


void dismount_char( CHAR_DATA *ch )
{
    CHAR_DATA *vict;

    vict = ch->riding;
    if  ( vict == NULL )
    {
        vict = ch->rider;
        ch->rider = NULL;
        if ( vict == NULL ) return;
        vict->riding = NULL;
    }
    else
    {
        ch->riding = NULL;
        vict->rider = NULL;
    }

    return;
}

void mount_char( CHAR_DATA *ch, CHAR_DATA *victim )
{
   if ( ch == NULL ) return;
   if ( victim == NULL || !IS_NPC(victim) ) return;

   if ( IS_AFFECTED(ch, AFF_SNEAK) )
   do_sneak( ch, "" );

   if ( !IS_SET( victim->act, ACT_MOUNT ) )
   return;

   if ( IS_AFFECTED(victim, AFF_SNEAK) )
   do_sneak( victim, "" );
   
   if ( ch->in_room != victim->in_room )
   log_string( "Mount_char: Ch and victim not in same scene." );

   if ( ch->rider != NULL )
   dismount_char( ch->rider );

   if ( ch->riding != NULL )
   dismount_char( ch );

   if ( victim->rider != NULL )
   dismount_char( victim->rider );

   if ( victim->riding != NULL )
   dismount_char( victim );

   ch->riding = victim;
   victim->rider = ch;

   ch->on=NULL;     //set_furn( ch, NULL );
   victim->on=NULL; //set_furn( victim, NULL );
   return;
}



/*
 * Syntax:  ride [mount]
 */
void do_mount( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, buf );

    if ( buf[0] == '\0' )
    {
        to_actor( "Ride whom?\n\r", ch );
        return;
    }

    if ( !str_cmp( buf, "show" ) && IS_IMMORTAL(ch) )
    {
        to_actor( "Characters with masters:\n\r", ch );
        for ( victim = char_list; victim != NULL; victim = victim->next )
        {
            if ( victim->master == NULL )
            continue;

            sprintf( buf, "[%5d] %s, master: %s, rider: %s\n\r",
                     IS_NPC(victim) ? victim->pIndexData->vnum : -1,
                     victim->short_descr,
                     victim->master ? NAME(victim->master) : "none",
                     victim->rider  ? NAME(victim->rider) : "none" );
            to_actor( buf, ch );
        }

        return;
    }


    if ( ( victim = get_char_room( ch, NULL, buf ) ) == NULL )
    {
        to_actor( "They aren't here.\n\r", ch );
        return;
    }
    
    if ( ch->riding != NULL )
    {
        act( "You are already riding $N!", ch, NULL, ch->riding, TO_ACTOR, POS_DEAD );
        return;
    }

    if ( ch->rider != NULL )
    {
        act( "$N is riding you!", ch, NULL, ch->rider, TO_ACTOR, POS_DEAD );
        return;
    }

    if ( (IS_NPC(victim) && !IS_SET( victim->act, ACT_MOUNT ))
      && !IS_IMMORTAL(ch) )
    {
        to_actor( "You cannot ride that.\n\r", ch );
        return;
    }

    if ( victim->riding != NULL )
    {
        act( "$N is already riding $n.", victim, NULL, victim->rider, TO_ACTOR, POS_DEAD );
        return;
    }

    if ( victim->rider != NULL )
    {
        act( "$N is riding $n.", victim, NULL, victim->rider, TO_ACTOR, POS_DEAD );
        return;
    }

    mount_char( ch, victim );

    act( "$n climb onto $N's back.", ch, NULL, victim, TO_SCENE, POS_DEAD );
    act( "You climb onto $N's back.",  ch, NULL, victim, TO_ACTOR, POS_DEAD );
    act( "$n leaps onto your back.",  ch, NULL, victim, TO_VICT, POS_DEAD );

    if ( victim->position == POS_RESTING
      || victim->position == POS_SITTING  )
    {
        act( "You clamber groggily to your feet.", victim, NULL, NULL, TO_ACTOR, POS_DEAD );
        act( "$n clambers groggily to $s feet.", victim, NULL, NULL, TO_SCENE, POS_DEAD );
        victim->position = POS_STANDING;
    }
    return;
}

/*
 * Syntax: Trample [target]
 */ 
void do_trample( CHAR_DATA *ch, char *argument )
{
}

/*
 * Syntax: Rush [target]
 */ 
void do_rush( CHAR_DATA *ch, char *argument )
{
}



/*
 * Syntax:  dismount
 */
void do_dismount( CHAR_DATA *ch, char *argument )
{
    if ( ch->riding == NULL )
    {
        to_actor( "You're not riding anything.\n\r", ch );
        return;
    }

    act( "$n dismounts from $N.",  ch, NULL, ch->riding, TO_NOTVICT, POS_DEAD );
    act( "You swing your legs aside and dismount $N.",  ch, NULL, ch->riding, TO_ACTOR, POS_DEAD );
    act( "$n dismounts.", ch, NULL, ch->riding, TO_VICT, POS_DEAD );

    dismount_char( ch );
    return;
}




/*
 * Pet shops / stables
 */

/*
 * Stables a pet.
 */
void do_stable( CHAR_DATA *ch, char *argument ) {
}
 
/*
 * Releases a pet.
 */
void do_release( CHAR_DATA *ch, char *argument ) {
}

/*
 * Gifts a pet to another player.  (merge with give)
 */
void do_gift( CHAR_DATA *ch, char *argument ) {
}
