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
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "olc.h"
#include "rand_obj.h"
#include "recycle.h"

DECLARE_DO_FUN( do_say		);
DECLARE_DO_FUN( do_stat         );
DECLARE_DO_FUN( do_nohelp       );

bool	is_gq_target	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
char *	str_replace_who	args( ( char *astr, char *bstr, char *cstr ) );
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp, sh_int type ) );
void	show_damlist	args( ( CHAR_DATA *ch, char *argument ) );

void scan_direction     args( ( CHAR_DATA *ch, int dir ) );
void show_room_to_char  args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pScene, int dist, int dir ) );


#if defined(UNIX)
bool    write_to_descriptor     args( ( DESCRIPTOR_DATA *d, char *txt, int length ) );
#endif

char *  const	where_name	[MAX_WEAR] =
{
    "{R|{r----{R>{cused for light{R<{r----{R|{x  ",
    "{R|{r---{R>{cworn on l-finger{R<{r---{R|{x  ",
    "{R|{r---{R>{cworn on r-finger{R<{r---{R|{x  ",
    "{R|{r--{R>{cworn around neck-1{R<{r--{R|{x  ",
    "{R|{r--{R>{cworn around neck-2{R<{r--{R|{x  ",
    "{R|{r---{R>{cworn about torso{R<{r---{R|{x  ",
    "{R|{r-----{R>{cworn on head{R<{r-----{R|{x  ",
    "{R|{r-----{R>{cworn on legs{R<{r-----{R|{x  ",
    "{R|{r-----{R>{cworn on feet{R<{r-----{R|{x  ",
    "{R|{r----{R>{cbound on hands{R<{r----{R|{x  ",
    "{R|{r-----{R>{cworn on arms{R<{r-----{R|{x  ",
    "{R|{r----{R>{cworn as shield{R<{r----{R|{x  ",
    "{R|{r--{R>{cwrapped about body{R<{r--{R|{x  ",
    "{R|{r---{R>{cworn about waist{R<{r---{R|{x  ",
    "{R|{r-{R>{cencompassing l-wrist{R<{r-{R|{x  ",
    "{R|{r-{R>{cencompassing r-wrist{R<{r-{R|{x  ",
    "{R|{r----{R>{cprimary weapon{R<{r----{R|{x  ",
    "{R|{r---{R>{cclutched tightly{R<{r---{R|{x  ",
    "{R|{r----{R>{cfloating close{R<{r----{R|{x  ",
    "{R|{r---{R>{csecondary weapon{R<{r---{R|{x  ",
    "{R|{r-----{R>{cworn on face{R<{r-----{R|{x  ",
    "{R|{r----{R>{ccovering l-ear{R<{r----{R|{x  ",
    "{R|{r----{R>{ccovering r-ear{R<{r----{R|{x  ",
    "{R|{r-----{R>{cworn on back{R<{r-----{R|{x  ",
    "{R|{r--{R>{cspan about l-ankle{R<{r--{R|{x  ",
    "{R|{r--{R>{cspan about r-ankle{R<{r--{R|{x  ",
    "{R|{r----{R>{ccovering chest{R<{r----{R|{x  ",
    "{R|{r---{R>{cplaced over eyes{R<{r---{R|{x  ",
    "{R|{r--{R>{cclan member symbol{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r--{R>{cproudly displaying{R<{r--{R|{x  ",
    "{R|{r----{R>{cprimary sheath{R<{r----{R|{x  ",
    "{R|{r---{R>{csecondary sheath{R<{r---{R|{x  "
};

sh_int const  where_order	[] =
{
    WEAR_LIGHT,
    WEAR_FLOAT,
    WEAR_HEAD,
    WEAR_FACE,
    WEAR_EYES,
    WEAR_EAR_L,
    WEAR_EAR_R,
    WEAR_NECK_1,
    WEAR_NECK_2,
    WEAR_CHEST,
    WEAR_BODY,
    WEAR_BACK,
    WEAR_SHIELD,
    WEAR_ARMS,
    WEAR_WRIST_L,
    WEAR_WRIST_R,
    WEAR_HANDS,
    WEAR_FINGER_L,
    WEAR_FINGER_R,
    WEAR_HOLD,
    WEAR_WIELD,
    WEAR_SECONDARY,
    WEAR_WAIST,
    WEAR_ABOUT,
    WEAR_SHEATH_1,
    WEAR_SHEATH_2,
    WEAR_LEGS,
    WEAR_ANKLE_L,
    WEAR_ANKLE_R,
    WEAR_FEET,
    WEAR_CLAN,
    WEAR_SOUL1,
    WEAR_SOUL2,
    WEAR_SOUL3,
    WEAR_SOUL4,
    WEAR_SOUL5,
    WEAR_SOUL6,
    WEAR_SOUL7,
    WEAR_SOUL8,
    WEAR_SOUL9,
    WEAR_SOUL10,
    WEAR_SOUL11,
    WEAR_SOUL12,
    WEAR_SOUL13,
    WEAR_SOUL14,
    WEAR_SOUL15
};

/* for do_count */
bool is_pm = FALSE;

/*
 * Local functions.
 */
char *	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );

char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if ( ( fShort && ( obj->short_descr == NULL || obj->short_descr[0] == '\0' ) )
    ||   ( obj->description == NULL || obj->description[0] == '\0' ) )
	return buf;

    if ( !IS_SET(ch->configure, CONFIG_LONG) )
    {
	strcat( buf, "{x[{y.{R.{B.{M.{Y.{W.{x]");
	if ( IS_OBJ_STAT(obj, ITEM_INVIS)	)   buf[5] = 'V';
	if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
	&&   IS_OBJ_STAT(obj, ITEM_EVIL)	)   buf[8] = 'E';
	if ( IS_AFFECTED(ch, AFF_DETECT_GOOD)
	&&   IS_OBJ_STAT(obj,ITEM_BLESS)	)   buf[11] = 'B';
	if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
	&&   IS_OBJ_STAT(obj, ITEM_MAGIC)	)   buf[14] = 'M';
	if ( IS_OBJ_STAT(obj, ITEM_GLOW)	)   buf[17] = 'G';
	if ( IS_OBJ_STAT(obj, ITEM_HUM)		)   buf[20] = 'H';
	if ( !strcmp(buf, "{x[{y.{R.{B.{M.{Y.{W.{x]") )
	    buf[0] = '\0';
    } else {
	if ( IS_OBJ_STAT(obj, ITEM_INVIS)	)   strcat(buf, "({yInvis{x)");
	if ( IS_OBJ_STAT(obj, ITEM_DARK)	)   strcat(buf, "({DHidden{x)");
	if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
	&&   IS_OBJ_STAT(obj, ITEM_EVIL)	)   strcat(buf, "({RRed Aura{x)");
	if ( IS_AFFECTED(ch, AFF_DETECT_GOOD)
	&&   IS_OBJ_STAT(obj,ITEM_BLESS)	)   strcat(buf,"({BBlue Aura{x)");
	if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
	&&   IS_OBJ_STAT(obj, ITEM_MAGIC)	)   strcat(buf, "({yMagical{x)");
	if ( IS_OBJ_STAT(obj, ITEM_GLOW)	)   strcat(buf, "({YGlowing{x)");
	if ( IS_OBJ_STAT(obj, ITEM_HUM)		)   strcat(buf, "({yHumming{x)");
    }

    if (buf[0] != '\0')
	strcat(buf, " ");

    if ( ch->pcdata != NULL
    &&   ch->pcdata->questobj != 0
    &&   ch->pcdata->questobj == obj->pIndexData->vnum )
	strcat( buf, "{R[TARGET]{x ");

    if ( fShort )
    {
	AFFECT_DATA *paf;

	for ( paf = obj->affected; paf != NULL; paf = paf->next )
	{
	    if ( paf->location == APPLY_DAMAGE )
	    {
		strcat( buf, "{R(Damaged){x " );
		break;
	    }
	}

	if ( obj->short_descr != NULL )
	    strcat( buf, obj->short_descr );
    } else {
	if ( obj->description != NULL)
	    strcat( buf, obj->description );
    }

    return buf;
}

BUFFER *show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    BUFFER *final = new_buf();
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    char **prgpstrShow, *pstrShow;
    bool fCombine;
    int *prgnShow, nShow, iShow, count = 0;

    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( ++count >= 8500 )
	{
	    add_buf( final, "{RToo many objects found.{x\n\r" );
	    return final;
	}
    }

    prgpstrShow = alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int   ) );
    nShow	= 0;

    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;

	    if ( IS_NPC( ch ) || !IS_SET( ch->configure, CONFIG_NO_INV_COMBINE ) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if ( prgpstrShow[iShow][0] == '\0' )
	{
	    free_string( prgpstrShow[iShow] );
	    continue;
	}

	if ( IS_NPC( ch ) || !IS_SET( ch->configure, CONFIG_NO_INV_COMBINE ) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		add_buf( final, buf );
	    }
	    else
		add_buf( final, "     " );
	}

	add_buf( final, prgpstrShow[iShow] );
	add_buf( final, "\n\r" );
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC( ch ) || !IS_SET( ch->configure, CONFIG_NO_INV_COMBINE ) )
	    add_buf( final, "     " );
	add_buf( final, "Nothing.\n\r" );
    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return final;
}

char * show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    static char buf[MAX_STRING_LENGTH];
    char  message[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if ( !IS_SET( ch->configure, CONFIG_LONG ) )
    {
	strcat( buf, "[{y.{D.{c.{W.{g.{Y.{C.{r.{B.{w.{y.{W.{G.{R.{c.{x] ");

	if ( IS_SHIELDED(victim, SHD_INVISIBLE)   ) buf[3] = 'V';
	if ( IS_AFFECTED(victim, AFF_HIDE)        ) buf[6] = 'H';
	if ( IS_AFFECTED(victim, AFF_CHARM)       ) buf[9] = 'C';

	if ( IS_EVIL(victim) && IS_AFFECTED(ch, AFF_DETECT_EVIL) )
	{
	    buf[11] = 'R';
	    buf[12] = 'E';
	}

	else if ( IS_GOOD(victim) && IS_AFFECTED(ch, AFF_DETECT_GOOD) )
	{
	    buf[11] = 'Y';
	    buf[12] = 'G';
	}

	else if ( IS_NEUTRAL( victim ) && IS_AFFECTED(ch, AFF_DETECT_NEUTRAL) )
	    buf[12] = 'N';

	if ( IS_SHIELDED(victim, SHD_ACID)	  ) buf[15] = 'A';
	if ( IS_SHIELDED(victim, SHD_DIVINE_AURA) ) buf[18] = 'D';
	if ( IS_SHIELDED(victim, SHD_ICE)  	  ) buf[21] = 'I';
	if ( IS_SHIELDED(victim, SHD_FIRE)        ) buf[24] = 'F';
	if ( IS_SHIELDED(victim, SHD_SHOCK)       ) buf[27] = 'L';
	if ( IS_SHIELDED(victim, SHD_SHRAPNEL)	  ) buf[30] = 'P';
	if ( IS_SHIELDED(victim, SHD_ROCK)	  ) buf[33] = 'R';
	if ( IS_SHIELDED(victim, SHD_SANCTUARY)   ) buf[36] = 'S';
	if ( IS_SHIELDED(victim, SHD_DIVINITY)	  ) buf[36] = 'D';
	if ( IS_SHIELDED(victim, SHD_THORN)	  ) buf[39] = 'T';
	if ( IS_SHIELDED(victim, SHD_VAMPIRIC)	  ) buf[42] = 'V';
	if ( IS_SHIELDED(victim, SHD_WATER)	  ) buf[45] = 'W';

	if ( !strcmp( buf, "[{y.{D.{c.{W.{g.{Y.{C.{r.{B.{w.{y.{W.{G.{R.{c.{x] " ) )
	    buf[0] = '\0';

	if ( IS_SET(victim->comm,COMM_AFK  )      ) strcat( buf, "[{BAFK{x] ");
	if ( victim->invis_level >= LEVEL_HERO    ) strcat( buf, "{!({&W{7i{&z{7i{!){x ");
    } else {
	if ( IS_SET(victim->comm,COMM_AFK  )      ) strcat( buf, "[{CAFK{x]");
	if ( IS_SHIELDED(victim, SHD_INVISIBLE)   ) strcat( buf, "({yInvis{x)");
	if ( victim->invis_level >= LEVEL_HERO    ) strcat( buf, "({WWizi{x)");
	if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "({DHide{x)");
	if ( IS_AFFECTED(victim, AFF_CHARM)       ) strcat( buf, "({cCharmed{x)");
	if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "({bTranslucent{x)");
	if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "({wPink Aura{x)");
	if ( IS_SHIELDED(victim, SHD_ICE)         ) strcat( buf, "({DGrey Aura{x)");
	if ( IS_SHIELDED(victim, SHD_FIRE)        ) strcat( buf, "({rOrange Aura{x)");
	if ( IS_SHIELDED(victim, SHD_SHOCK)       ) strcat( buf, "({BBlue Aura{x)");
	if ( IS_EVIL(victim)
	&&   IS_AFFECTED(ch, AFF_DETECT_EVIL)     ) strcat( buf, "({RRed Aura{x)");
	if ( IS_GOOD(victim)
	&&   IS_AFFECTED(ch, AFF_DETECT_GOOD)     ) strcat( buf, "({YGolden Aura{x)");
	if ( IS_SHIELDED(victim, SHD_DIVINITY)    ) strcat( buf, "({WB_White Aura{x)");
	else if ( IS_SHIELDED(victim, SHD_SANCTUARY) ) strcat( buf, "({WWhite Aura{x)");
	if ( victim->pcdata
	&&   victim->pcdata->on_quest )		    strcat( buf, "({GQuest{x)");
    }

    if ( (IS_NPC(victim)
    &&	  ch->pcdata
    &&    ch->pcdata->questmob > 0
    &&    victim->pIndexData->vnum == ch->pcdata->questmob
    &&    (ch->pcdata->questobj == 0 || (victim->carrying
    &&     victim->carrying->pIndexData->vnum == ch->pcdata->questobj))) )
	strcat(buf, "{R[TARGET]{x ");

    if ( is_gq_target( ch, victim ) )
	strcat( buf, "{G[TARGET]{x " );

    if ( !IS_NPC(victim) )
    {
	if ( victim->desc == NULL )
	    strcat( buf,"{r({RLinkdead{r){x " );

	if ( victim->pcdata->match != NULL )
	{
	    sprintf(message,"[{G%d{x][{GTeam %d{x] ",
		victim->pcdata->match->number, victim->pcdata->team+1);
	    strcat(buf,message);
	}

	if ( victim->pcdata->pktimer > 0 )
	{
	    if ( !victim->pcdata->attacker )
		strcat( buf, "( {RHunted{x ) ");
	    else
		strcat( buf, "( {RHunter{x ) ");
	}

	if ( victim->pcdata->dtimer > 0 )
	    strcat( buf, "{G[ {BDecease{Bd {G]{x " );

	if ( IS_SET(victim->act, PLR_TWIT) )
	    strcat( buf, "({rTWIT{x) ");
    }

    if (buf[0] != '\0')
    {
	strcat( buf, "" );
    }

    if ( victim->position == victim->start_pos
    &&   victim->long_descr[0] != '\0' )
    {
	strcat( buf, victim->long_descr );
	return buf;
    }

    strcat( buf, PERS( victim, ch ) );

    switch ( victim->position )
    {
    case POS_DEAD:     strcat( buf, " is DEAD!!" );              break;
    case POS_MORTAL:   strcat( buf, " is mortally wounded." );   break;
    case POS_INCAP:    strcat( buf, " is incapacitated." );      break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned." ); break;
    case POS_SLEEPING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],SLEEP_AT))
  	    {
		sprintf(message," is sleeping at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],SLEEP_ON))
	    {
		sprintf(message," is sleeping on %s.",
		    victim->on->short_descr); 
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message, " is sleeping in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else 
	    strcat(buf," is sleeping here.");
	break;
    case POS_RESTING:  
        if (victim->on != NULL)
	{
            if (IS_SET(victim->on->value[2],REST_AT))
            {
                sprintf(message," is resting at %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else if (IS_SET(victim->on->value[2],REST_ON))
            {
                sprintf(message," is resting on %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else 
            {
                sprintf(message, " is resting in %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
	}
        else
	    strcat( buf, " is resting here." );       
	break;
    case POS_SITTING:  
        if (victim->on != NULL)
        {
            if (IS_SET(victim->on->value[2],SIT_AT))
            {
                sprintf(message," is sitting at %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else if (IS_SET(victim->on->value[2],SIT_ON))
            {
                sprintf(message," is sitting on %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else
            {
                sprintf(message, " is sitting in %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
        }
        else
	    strcat(buf, " is sitting here.");
	break;
    case POS_STANDING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],STAND_AT))
	    {
		sprintf(message," is standing at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],STAND_ON))
	    {
		sprintf(message," is standing on %s.",
		   victim->on->short_descr);
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message," is standing in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else
	    strcat( buf, " is here." );               
	break;
    case POS_FIGHTING:
	strcat( buf, " is here, fighting " );
	if ( victim->fighting == NULL )
	    strcat( buf, "thin air??" );
	else if ( victim->fighting == ch )
	    strcat( buf, "YOU!" );
	else if ( victim->in_room == victim->fighting->in_room )
	{
	    strcat( buf, PERS( victim->fighting, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "someone who left??" );
	break;
    }

    strcat( buf, "\n\r" );
    buf[0] = UPPER(buf[0]);
    return buf;
}

BUFFER *show_char_list( CHAR_DATA *list, CHAR_DATA *ch )
{
    BUFFER *final = new_buf( );
    CHAR_DATA *vch;
    char buf[MAX_STRING_LENGTH];
    char **prgpstrShow, *pstrShow;
    bool fCombine;
    int *prgnShow, nShow, iShow, count = 0;

    for ( vch = list; vch != NULL; vch = vch->next_in_room )
    {
	if ( ++count >= 8500 )
	{
	    add_buf( final, "{RToo many people found.{x\n\r" );
	    return final;
	}
    }

    prgpstrShow = alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int   ) );
    nShow	= 0;

    for ( vch = list; vch != NULL; vch = vch->next_in_room )
    { 
	if ( ch == vch
	||   get_trust( ch ) < vch->invis_level
	||   get_trust( ch ) < vch->ghost_level )
	    continue;

	if ( can_see( ch, vch ) )
	    pstrShow = show_char_to_char_0( vch, ch );

	else if ( room_is_dark( ch->in_room ) && IS_AFFECTED( vch, AFF_INFRARED ) )
	    pstrShow = "You see {Rglowing red{x eyes watching YOU!\n\r";

	else
	    continue;

	pstrShow = show_char_to_char_0( vch, ch );
	fCombine = FALSE;

	if ( IS_NPC( ch ) || IS_SET( ch->configure, CONFIG_CHAR_COMBINE ) )
	{
	    /*
	     * Look for duplicates, case sensitive.
	     * Matches tend to be near end so run loop backwords.
	     */
	    for ( iShow = nShow - 1; iShow >= 0; iShow-- )
	    {
		if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		{
		    prgnShow[iShow]++;
		    fCombine = TRUE;
		    break;
		}
	    }
	}

	/*
	 * Couldn't combine, or didn't want to.
	 */
	if ( !fCombine )
	{
	    prgpstrShow [nShow] = str_dup( pstrShow );
	    prgnShow    [nShow] = 1;
	    nShow++;
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if ( prgpstrShow[iShow][0] == '\0' )
	{
	    free_string( prgpstrShow[iShow] );
	    continue;
	}

	if ( IS_NPC( ch ) || IS_SET( ch->configure, CONFIG_CHAR_COMBINE ) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "[%2d] ", prgnShow[iShow] );
		add_buf( final, buf );
	    }
	    else
		add_buf( final, "     " );
	}

	add_buf( final, prgpstrShow[iShow] );
	free_string( prgpstrShow[iShow] );
    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return final;
}

void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    BUFFER *final = new_buf();
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int oWear;
    bool found;

    if ( can_see( victim, ch )
    && get_trust(victim) >= ch->ghost_level)
    {
	if (ch == victim)
	    act( "$n{x looks at $mself.", ch,NULL,NULL,TO_ROOM,POS_RESTING);
	else
	{
	    act( "$n{x looks at you.", ch, NULL, victim, TO_VICT,POS_RESTING);
	    act( "$n{x looks at $N{x.", ch, NULL, victim, TO_NOTVICT,POS_RESTING);
	}
    }

    if ( !IS_NPC(victim) )
    {
	sprintf(buf,"%s is a %s sized %s.\n\r", victim->name,
	    size_flags[victim->size].name,
	    race_table[victim->race].name);
	add_buf(final,buf);
    }

    if ( victim->description[0] != '\0' )
	add_buf(final,victim->description);
    else
    {
	sprintf(buf,"{CYou see nothing special about %s.{x\n\r", NAME(victim));
	add_buf(final,buf);
    }

    add_buf( final, show_condition( ch, victim, VALUE_HIT_POINT ) );

    if ( IS_SHIELDED(victim, SHD_ACID))
    {
        sprintf( buf, "%s is surrounded by an {Ga{gc{Gi{gd{Gi{gc{x shield.\n\r", PERS(victim, ch));
	buf[0] = UPPER( buf[0] );
	add_buf(final,buf);
    }

    if ( IS_SHIELDED(victim, SHD_DIVINE_AURA))
    {
        sprintf( buf, "%s is surrounded by a {Yd{yi{Yv{yi{Yn{ye{x aura.\n\r", PERS(victim, ch));
	buf[0] = UPPER( buf[0] );
	add_buf(final,buf);
    }

    if ( IS_SHIELDED(victim, SHD_ICE))
    {
 	sprintf( buf, "%s is surrounded by an {Cicy{x shield.\n\r", PERS(victim, ch));
	buf[0] = UPPER( buf[0] );
	add_buf(final,buf);
    }

    if ( IS_SHIELDED(victim, SHD_FIRE))
    {
	sprintf( buf, "%s is surrounded by a {Rfiery{x shield.\n\r", PERS(victim, ch));
	buf[0] = UPPER( buf[0] );
	add_buf(final,buf);

    }

    if ( IS_SHIELDED(victim, SHD_ROCK))
    {
        sprintf( buf, "%s is surrounded by a {yrocky{x shield.\n\r", PERS(victim, ch));
	buf[0] = UPPER( buf[0] );
	add_buf(final,buf);
    }

    if ( IS_SHIELDED(victim, SHD_SHOCK))
    {
	sprintf( buf, "%s is surrounded by a {Bcrackling{x shield.\n\r", PERS(victim, ch));
	buf[0] = UPPER( buf[0] );
	add_buf(final,buf);
    }

    if ( IS_SHIELDED(victim, SHD_SHRAPNEL))
    {
        sprintf( buf, "%s is surrounded by a {8s{7h{8r{7a{8p{7n{8e{7l{x shield.\n\r", PERS(victim, ch));
	buf[0] = UPPER( buf[0] );
	add_buf(final,buf);
    }

    if ( IS_SHIELDED(victim, SHD_THORN))
    {
        sprintf( buf, "%s is surrounded by a {gt{yh{go{yr{gn{yy{x shield.\n\r", PERS(victim, ch));
	buf[0] = UPPER( buf[0] );
	add_buf(final,buf);
    }

    if ( IS_SHIELDED(victim, SHD_VAMPIRIC))
    {
        sprintf( buf, "%s is surrounded by a {Wv{wa{Wm{wp{Wi{wr{Wi{wc{x shield.\n\r", PERS(victim, ch));
	buf[0] = UPPER( buf[0] );
	add_buf(final,buf);
    }

    if ( IS_SHIELDED(victim, SHD_WATER) )
    {
	sprintf( buf, "%s is surrounded by a {Bwatery{x shield.\n\r", PERS(victim, ch) );
	buf[0] = UPPER( buf[0] );
	add_buf(final,buf);
    }

    if ( victim->pIndexData != NULL )
    {
	sprintf( buf, "This Boot: Killed {R%d{x time%s, Killed {R%d{x player%s.\n"
		      "Total    : Killed {R%ld{x time%s, Killed {R%ld{x player%s.\n\r",
	    victim->pIndexData->mob_pc_deaths[0],
	    victim->pIndexData->mob_pc_deaths[0] == 1 ? "" : "s",
	    victim->pIndexData->mob_pc_deaths[1],
	    victim->pIndexData->mob_pc_deaths[1] == 1 ? "" : "s",
	    victim->pIndexData->perm_mob_pc_deaths[0],
	    victim->pIndexData->perm_mob_pc_deaths[0] == 1 ? "" : "s",
	    victim->pIndexData->perm_mob_pc_deaths[1],
	    victim->pIndexData->perm_mob_pc_deaths[1] == 1 ? "" : "s" );
	add_buf( final, buf );
    }

    if ( ( !IS_IMMORTAL( ch ) || !can_over_ride( ch, victim, TRUE ) )
    &&   !IS_NPC( victim ) && IS_SET( victim->act, PLR_CLOAKED_EQ ) )
    {
	sprintf( buf, "\n\r{GA magical cloak surrounds %s's equipment.{x\n\r",
	    PERS( victim, ch ) );
	add_buf( final, buf );
    } else {
	found = FALSE;
	for ( oWear = 0; oWear < MAX_WEAR; oWear++ )
	{
	    iWear = where_order[oWear];
	    if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	    &&   can_see_obj( ch, obj ) )
	    {
		if ( !found )
		{
		    sprintf( buf, "\n\r{G%s is using:{x\n\r", 
			IS_NPC(victim) ? victim->short_descr : victim->name );
		    add_buf(final,buf);
		    found = TRUE;
		}

		sprintf( buf, "%s%s\n\r", where_name[iWear], format_obj_to_char( obj, ch, TRUE ));
		add_buf(final,buf);
	    }
	}
    }

    if ( victim != ch
    &&   !IS_NPC(ch)
    &&   number_percent( ) < get_skill(ch,gsn_peek)
    &&   IS_SET(ch->act,PLR_AUTOPEEK))
    {
	BUFFER *output = show_list_to_char(victim->carrying,ch,TRUE,TRUE);

	add_buf(final,"\n\r{GYou peek at the inventory:{x\n\r");
	check_improve(ch,gsn_peek,TRUE,4);
	add_buf(final, output->string);
	free_buf(output);
    }
    page_to_char(final->string,ch);
    free_buf(final);
    return;
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;

    send_to_char( "You are carrying:\n\r", ch );
    final = show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    page_to_char(final->string,ch);
    free_buf(final);
    return;
}

void do_peek( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if (IS_NPC(ch))
	return;

    if (arg[0] == '\0')
    {
	send_to_char("Peek at who?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim == ch)
    {
	do_inventory(ch,"");
	return;
    }

    if ( can_see( victim, ch )
    && get_trust(victim) >= ch->ghost_level)
    {
	act( "$n peers intently at you.", ch, NULL, victim, TO_VICT,POS_RESTING);
	act( "$n peers intently at $N.",  ch, NULL, victim, TO_NOTVICT,POS_RESTING);
    }

    if (number_percent( ) < get_skill(ch,gsn_peek))
    {
        send_to_char( "\n\r{GYou peek at the inventory:{x\n\r", ch );
        check_improve(ch,gsn_peek,TRUE,4);
	final = show_list_to_char( victim->carrying, ch, TRUE, TRUE );
	page_to_char(final->string,ch);
	free_buf(final);
    } else {
	send_to_char("{RYou fail to see anything.{x\n\r",ch);
	check_improve(ch,gsn_peek,FALSE,2);
    }
    return;
}

bool check_blind( CHAR_DATA *ch )
{

    if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    { 
	send_to_char( "You can't see a thing!\n\r", ch ); 
	return FALSE; 
    }

    return TRUE;
}

void do_scroll(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    if ( ch->pcdata == NULL )
	return;

    one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
	if (ch->pcdata->lines == 0)
	    send_to_char("You do not page long messages.\n\r",ch);
	else
	{
	    sprintf(buf,"You currently display %d lines per page.\n\r",
		    ch->pcdata->lines + 2);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("You must provide a number.\n\r",ch);
	return;
    }

    lines = atoi(arg);

    if (lines == 0)
    {
        send_to_char("Paging disabled.\n\r",ch);
        ch->pcdata->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
	send_to_char("You must provide a reasonable number.\n\r",ch);
	return;
    }

    sprintf(buf,"Scroll set to %d lines.\n\r",lines);
    send_to_char(buf,ch);
    ch->pcdata->lines = lines - 2;
}

void do_socials(CHAR_DATA *ch, char *argument)
{
    BUFFER *final = new_buf();
    char buf[MAX_STRING_LENGTH];
    int iSocial, col = 0;

    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
	sprintf(buf,"%-15s",social_table[iSocial].name);
	add_buf(final,buf);

	if ( ++col %5 == 0 )
	    add_buf( final, "\n\r" );
    }


    if ( col % 5 != 0 )
	add_buf( final, "\n\r" );

    page_to_char(final->string,ch);
    free_buf(final);
    return;
}

void do_autoaction( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
    {
	send_to_char( "Automatic actions are for players!\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' || !str_prefix(argument, "list") )
    {
	BUFFER *final = new_buf();

	add_buf( final, "{Raction        status\n\r" );
	add_buf( final, "{R---------------------\n\r" );
 
	add_buf( final, "{wauto assist     " );
	if ( IS_SET(ch->act,PLR_AUTOASSIST) )
	    add_buf( final, "{gON\n\r" );
	else
	    add_buf( final, "{yOFF\n\r" ); 

	add_buf( final, "{wauto exit       " );
	if ( IS_SET(ch->act,PLR_AUTOEXIT) )
	    add_buf( final, "{gON\n\r" );
	else
	    add_buf( final, "{yOFF\n\r" );

	add_buf( final, "{wauto gold       " );
	if ( IS_SET(ch->act,PLR_AUTOGOLD) )
	    add_buf( final, "{gON\n\r" );
	else
	    add_buf( final, "{yOFF\n\r" );

	add_buf(final,"{wauto loot       " );
	if ( IS_SET(ch->act,PLR_AUTOLOOT) )
	    add_buf( final, "{gON\n\r" );
	else
	    add_buf( final, "{yOFF\n\r" );

	add_buf(final,"{wauto sac        " );
	if ( IS_SET(ch->act,PLR_AUTOSAC) )
	    add_buf( final, "{gON\n\r" );
	else
	    add_buf( final, "{yOFF\n\r" );

	add_buf( final, "{wauto split      " );
	if ( IS_SET(ch->act,PLR_AUTOSPLIT) )
	    add_buf( final, "{gON\n\r" );
	else
	    add_buf( final, "{yOFF\n\r" );

	add_buf( final, "{wauto peek       " );
	if (IS_SET(ch->act,PLR_AUTOPEEK) )
	    add_buf( final, "{gON\n\r" );
	else
	    add_buf( final, "{yOFF\n\r" );

	add_buf( final, "{wdetailed_exits  " );
	if ( IS_SET(ch->act,PLR_DETAIL_EXIT) )
	    add_buf( final, "{gON\n\r" );
	else
	    add_buf( final, "{yOFF\n\r" );

	add_buf( final, "{wprompt          " );
	if ( IS_SET(ch->comm,COMM_PROMPT) )
	    add_buf( final, "{gON\n\r" );
	else
	    add_buf( final, "{yOFF\n\r" );

	add_buf( final, "{R---------------------\n\r" );

	add_buf( final, "{wnosummon        " );
	if ( IS_SET(ch->act,PLR_NOSUMMON) )
	    add_buf( final, "{gON\n\r" );
	else
	    add_buf( final, "{yOFF\n\r" );

	add_buf( final, "{wnofollow        " );
	if (IS_SET(ch->act,PLR_NOFOLLOW) )
	    add_buf( final, "{gON\n\r" );
	else
	    add_buf( final, "{yOFF\n\r" );

	add_buf( final, "{wnocancel        " );
	if ( IS_SET(ch->act,PLR_NOCANCEL) )
	    add_buf( final, "{gON\n\r" );
	else
	    add_buf( final, "{yOFF\n\r" );

	add_buf( final, "{wnotran          " );
	if ( IS_SET(ch->act,PLR_NOTRAN) )
	    add_buf( final, "{gON\n\r" );
	else
	    add_buf( final, "{yOFF\n\r" );

	add_buf( final, "{wnoexp           " );
	if ( IS_SET(ch->act, PLR_NOEXP) )
	    add_buf( final, "{gON\n\r" );
	else
	    add_buf( final, "{yOFF\n\r" );
	add_buf( final, "{R---------------------{x\n\r\n\r"
			" Toggle options with a space now (i.e. auto assist)\n\r" );

	page_to_char( final->string, ch );
	free_buf( final );
    }

    else if ( !str_prefix(argument, "assist") )
    {
	if ( IS_SET(ch->act,PLR_AUTOASSIST) )
	{
	    send_to_char( "Autoassist removed.\n\r", ch );
	    REMOVE_BIT( ch->act,PLR_AUTOASSIST );
	} else {
	    send_to_char( "You will now assist when needed.\n\r", ch );
	    SET_BIT( ch->act,PLR_AUTOASSIST );
	}
    }

    else if ( !str_prefix(argument, "detailed_exits" ) )
    {
	if ( IS_SET(ch->act,PLR_DETAIL_EXIT) )
	{
	    send_to_char( "Detailed exits will no longer be displayed.\n\r", ch );
	    REMOVE_BIT( ch->act,PLR_DETAIL_EXIT );
	} else {
	    send_to_char( "Detailed exits will now be displayed.\n\r", ch );
	    SET_BIT( ch->act,PLR_DETAIL_EXIT );
	}
    }

    
    else if ( !str_prefix(argument, "exit") )
    {
	if ( IS_SET(ch->act,PLR_AUTOEXIT) )
	{
	    send_to_char( "Exits will no longer be displayed.\n\r", ch );
	    REMOVE_BIT( ch->act,PLR_AUTOEXIT );
	} else {
	    send_to_char( "Exits will now be displayed.\n\r", ch );
	    SET_BIT( ch->act,PLR_AUTOEXIT );
	}
    }

    else if ( !str_prefix(argument, "gold") )
    {
	if ( IS_SET(ch->act,PLR_AUTOGOLD) )
	{
	    send_to_char( "Autogold removed.\n\r", ch );
	    REMOVE_BIT( ch->act,PLR_AUTOGOLD );
	} else {
	    send_to_char( "Automatic gold looting set.\n\r", ch );
	    SET_BIT( ch->act,PLR_AUTOGOLD );
	}
    }

    else if ( !str_prefix(argument, "loot") )
    {
	if ( IS_SET(ch->act,PLR_AUTOLOOT) )
	{
	    send_to_char( "Autolooting removed.\n\r", ch );
	    REMOVE_BIT( ch->act,PLR_AUTOLOOT );
	} else {
	    send_to_char( "Automatic corpse looting set.\n\r", ch );
	    SET_BIT( ch->act,PLR_AUTOLOOT );
	}
    }

    else if ( !str_prefix(argument, "peek") )
    {
	if ( IS_SET(ch->act,PLR_AUTOPEEK) )
	{
	    send_to_char( "Autopeek removed.\n\r", ch );
	    REMOVE_BIT( ch->act,PLR_AUTOPEEK );
	} else {
	    send_to_char( "Automatic peek set.\n\r", ch );
	    SET_BIT( ch->act,PLR_AUTOPEEK );
	}
    }

    else if ( !str_prefix(argument, "sacrifice") )
    {
	if ( IS_SET(ch->act,PLR_AUTOSAC) )
	{
	    send_to_char( "Autosacrificing removed.\n\r", ch );
	    REMOVE_BIT( ch->act,PLR_AUTOSAC );
	} else {
	    send_to_char( "Automatic corpse sacrificing set.\n\r", ch );
	    SET_BIT( ch->act,PLR_AUTOSAC );
	}
    }

    else if ( !str_prefix(argument, "split") )
    {
	if ( IS_SET(ch->act,PLR_AUTOSPLIT) )
	{
	    send_to_char( "Autosplitting removed.\n\r", ch );
	    REMOVE_BIT( ch->act,PLR_AUTOSPLIT );
	} else {
	    send_to_char( "Automatic gold splitting set.\n\r", ch );
	    SET_BIT( ch->act,PLR_AUTOSPLIT );
	}
    }
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];
 
   if ( argument[0] == '\0' )
   {
	if (IS_SET(ch->comm,COMM_PROMPT))
   	{
      	    send_to_char("You will no longer see prompts.\n\r",ch);
      	    REMOVE_BIT(ch->comm,COMM_PROMPT);
    	}
    	else
    	{
      	    send_to_char("You will now see prompts.\n\r",ch);
      	    SET_BIT(ch->comm,COMM_PROMPT);
    	}
       return;
   }

#if defined(UNIX)
   if( !strcmp( argument, "show") && ch->desc ) {
    send_to_char("Your prompt is currently set to:\n\r", ch);
    write_to_descriptor(ch->desc,ch->prompt,strlen(ch->prompt));
    send_to_char("\n\r\n\r",ch);
    return;
   }
#endif

   if( !strcmp( argument, "all" ) )
	strcpy( buf, "{x<{R%A{x/{r%H{xhp {B%B{x/{b%M{xm {G%Cmv {x<%X{Wxp{x>%c");
/*
      strcpy( buf, "{x<%Ahp %Bm %Cmv{x> [{1%X{x]%c");
*/
   else
   {
      if ( strlen(argument) > 150 )
         argument[150] = '\0';
      strcpy( buf, argument );
      smash_tilde( buf );
      if (str_suffix("%c",buf))
	strcat(buf,"{x ");
	
   }
 
   free_string( ch->prompt );
   ch->prompt = str_dup( buf );
   sprintf(buf,"Prompt set to %s\n\r",ch->prompt );
   send_to_char(buf,ch);
   return;
}

void do_nofollow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
	return;
 
    if (IS_AFFECTED(ch,AFF_CHARM))
    {
	send_to_char("You may not use this command while charmed\n\r",ch);
	return;
    }

    if (IS_SET(ch->act,PLR_NOFOLLOW))
    {
	send_to_char("You now accept followers.\n\r",ch);
	REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    }
    else
    {
	send_to_char("You no longer accept followers.\n\r",ch);
	SET_BIT(ch->act,PLR_NOFOLLOW);
	die_follower( ch );
    }
    return;
}

void do_nocancel(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;

    if (IS_SET(ch->act,PLR_NOCANCEL))
    {
      send_to_char("You may now have cancellation casted on you.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NOCANCEL);
    }
    else
    {
      send_to_char("You are now protected against cancellation.\n\r",ch);
      SET_BIT(ch->act,PLR_NOCANCEL);
    }
}

void do_nosummon(CHAR_DATA *ch, char *argument)
{
    if ( IS_NPC( ch ) )
	return;

    if ( IS_SET(ch->act,PLR_NOSUMMON) )
    {
	send_to_char("You are no longer immune to summon.\n\r",ch);
	REMOVE_BIT(ch->act,PLR_NOSUMMON);
    } else {
	send_to_char("You are now immune to summoning.\n\r",ch);
	SET_BIT(ch->act,PLR_NOSUMMON);
    }
}

void do_notran(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
	return;
    }
    else
    {
      if (IS_SET(ch->act,PLR_NOTRAN))
      {
        send_to_char("You are no longer immune to transport.\n\r",ch);
        REMOVE_BIT(ch->act,PLR_NOTRAN);
      }
      else
      {
        send_to_char("You are now immune to transport.\n\r",ch);
        SET_BIT(ch->act,PLR_NOTRAN);
      }
    }
}

void do_exits( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found, round, fAuto;
    int door, outlet;

    fAuto = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    if ( str_cmp( argument, "details" ) )
    {
	if (fAuto)
            sprintf(buf,"[Exits:");
	else if (IS_IMMORTAL(ch))
	    sprintf(buf,"Obvious exits from room %d:\n\r",ch->in_room->vnum);
	else
	    sprintf(buf,"Obvious exits:\n\r");
    }

    else
	buf[0] = '\0';

    found = FALSE;
    for ( door = 0; door < 6; door++ )
    {
	round = FALSE;
	outlet = door;
	if ( ( pexit = ch->in_room->exit[outlet] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   can_see_room(ch,pexit->u1.to_room)
        && !( IS_SET(pexit->exit_info,EX_CONCEALED) && !IS_SET(pexit->exit_info,EX_REVEALED) )
        && !( IS_SET(pexit->exit_info,EX_SECRET) && !IS_SET(pexit->exit_info,EX_REVEALED) )
        )
	{
	    found = TRUE;
	    round = TRUE;
	    if ( fAuto )
	    {
		if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
		{
		    strcat( buf, " " );
		    strcat( buf, dir_name[outlet] );
		}

		else if ( ( !IS_SET(pexit->exit_info, EX_HIDDEN) && !IS_SET(pexit->exit_info,EX_CONCEALED) && !IS_SET(pexit->exit_info,EX_SECRET) )
		     ||   IS_IMMORTAL( ch )
                     || IS_SET(pexit->exit_info, EX_REVEALED) )
		{
		    strcat( buf, " (" );
		    strcat( buf, dir_name[outlet] );
		    strcat( buf, ")" );
		}
	    }
	    else
	    {
		if ( !IS_SET( pexit->exit_info, EX_CLOSED ) )
		{
		    if (IS_IMMORTAL(ch) )
			sprintf(buf + strlen(buf), " [Room: %5d]",pexit->u1.to_room->vnum );

		    sprintf( buf + strlen(buf), "  %-5s  - %s\n\r",
			capitalize( dir_name[outlet] ),
			room_is_dark( pexit->u1.to_room )
			?  "Too dark to tell"
			: pexit->u1.to_room->name );
		}

		else if ( ( !IS_SET(pexit->exit_info, EX_HIDDEN) && !IS_SET(pexit->exit_info,EX_CONCEALED) && !IS_SET(pexit->exit_info,EX_SECRET) )
		     ||   IS_IMMORTAL( ch )
                     || IS_SET(pexit->exit_info, EX_REVEALED) )
		{
		    if (IS_IMMORTAL(ch) )
			sprintf(buf + strlen(buf), " [Room: %5d]",pexit->u1.to_room->vnum);

		    sprintf( buf + strlen(buf), " (%-5s) - %s\n\r",
			capitalize( dir_name[outlet] ),
			room_is_dark( pexit->u1.to_room )
			?  "Too dark to tell"
			: pexit->u1.to_room->name );
		}
	    }
	}
	if (!round)
	{
	    OBJ_DATA *portal;
	    ROOM_INDEX_DATA *to_room;

	    portal = get_obj_exit( dir_name[door], ch->in_room->contents );
	    if (portal != NULL)
	    {
		found = TRUE;
		round = TRUE;
		if ( fAuto )
		{
		    strcat( buf, " " );
		    strcat( buf, dir_name[door] );
		}
		else
		{
		    to_room = get_room_index(portal->value[0]);

		    if (IS_IMMORTAL(ch))
			sprintf(buf + strlen(buf), 
			    " [Room: %5d]\n\r",to_room->vnum);

		    sprintf( buf + strlen(buf), " %-5s - %s\n\r",
			capitalize( dir_name[door] ),
			room_is_dark( to_room )
			    ?  "Too dark to tell"
			    : to_room->name
			);
		}
	    }
	}
    }

    OBJ_DATA *prop;
    char buf2[MAX_STRING_LENGTH];
    for ( prop = ch->in_room->contents;  prop != NULL;  prop =prop->next_content )
    {
        if ( prop->item_type == ITEM_PORTAL
          && can_see_obj( ch, prop ) )
        {
                if ( !fAuto )
                sprintf( buf2, "%s", prop->short_descr );
                else
                sprintf( buf2, " (%s)", prop->short_descr );

                strcat( buf, buf2 );
                found = TRUE;
        }
    }

    if ( !found )
	strcat( buf, fAuto ? " none" : "None.\n\r" );

    if (fAuto)
        strcat( buf, "]\n\r" );

    send_to_char( buf, ch );

    return;
}

void do_look( CHAR_DATA *ch, char *argument )
{
    BUFFER *final, *chars;
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char arg4 [MAX_INPUT_LENGTH];
    char arg5 [MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    char *pdesc;
    int door;
    int number,count;

    if ( ch->desc == NULL )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   room_is_dark( ch->in_room ) )
    {
	send_to_char( "It is pitch black ... \n\r", ch );
	final = show_char_list( ch->in_room->people, ch );
	page_to_char( final->string, ch );
	free_buf( final );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    argument = one_argument( argument, arg4 );
    argument = one_argument( argument, arg5 );
    count = 0;

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
        /* 'look' or 'look auto' */
        sprintf( buf, "{e%s%s{x", ch->in_room->name, IS_SET(ch->in_room->room_flags,ROOM_SAVE_CONTENTS)?"*":"" );
        if ( IS_SET( ch->configure, CONFIG_AREA_NAME ) )
        {
            strcat( buf, " [" );
            strcat( buf, ch->in_room->area->name );
            strcat( buf, "]" );
        }

        send_to_char( buf, ch );

        if (IS_IMMORTAL(ch) && (IS_NPC(ch) || IS_SET(ch->act,PLR_HOLYLIGHT)))
        {
            sprintf(buf," [Room %d]",ch->in_room->vnum);
            send_to_char(buf,ch);
        }

        send_to_char( "\n\r", ch );

        if ( arg1[0] == '\0'
        || ( !IS_NPC(ch) && !IS_SET(ch->configure, CONFIG_BRIEF) ) )
        {
            if (ch->desc && !ch->desc->run_buf)
            {
              char b[MSL];
              sprintf( b, "ROOM_%d", ch->in_room->vnum );
              display_ascii( b, ch );
              send_to_char( "  ",ch);
              send_to_char( ch->in_room->description, ch );
            }
        }

        if ( !IS_NPC(ch) )
        {
            if ( IS_SET(ch->act, PLR_DETAIL_EXIT) )
            {
                send_to_char( "\n\r", ch );
                do_exits( ch, "details" );
                send_to_char( "\n\r", ch );
            }

            else if ( IS_SET(ch->act, PLR_AUTOEXIT) )
            {
                send_to_char("\n\r",ch);
                do_exits( ch, "auto" );
            }
        }

	final = show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
	chars = show_char_list( ch->in_room->people, ch );
	add_buf( final, chars->string );
	free_buf( chars );
	page_to_char( final->string, ch );
	free_buf( final );
	return;
    }

    if ( !str_cmp( arg4, "i" ) || !str_cmp(arg4, "in")  || !str_cmp(arg4,"on"))
    {
	/* 'look in' */
	if ( arg5[0] == '\0' )
	{
	    send_to_char("Which contained object do you wish to scan?\n\r",ch);
	    return;
	}

	if ( ( obj = get_obj_here( ch, NULL, arg5 ) ) == NULL )
	{
	    send_to_char("First object not locatable.\n\r",ch);
	    return;
	}

	if ( ( in_obj = get_obj_list( ch, arg2, obj->contains ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( in_obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( in_obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
		break;
	    }

	    sprintf( buf, "It's %sfilled with  a %s liquid.\n\r",
		in_obj->value[1] <     in_obj->value[0] / 4
		    ? "less than half-" :
		in_obj->value[1] < 3 * in_obj->value[0] / 4
		    ? "about half-"     : "more than half-",
		liq_table[in_obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	case ITEM_PIT:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(in_obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	    act( "$p holds:", ch, in_obj, NULL, TO_CHAR,POS_RESTING);
	    final = show_list_to_char( in_obj->contains, ch, TRUE, TRUE );
	    page_to_char(final->string, ch);
	    free_buf(final);
	    break;
	}
	return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp(arg1, "in")  || !str_cmp(arg1,"on"))
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, NULL, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
		break;
	    }

	    sprintf( buf, "It's %sfilled with  a %s liquid.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than half-" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about half-"     : "more than half-",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	case ITEM_PIT:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	    act( "$p holds:", ch, obj, NULL, TO_CHAR,POS_RESTING);
	    final = show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    page_to_char(final->string,ch);
	    free_buf(final);
	    break;
	}
	return;
    }

    if ( str_cmp( arg1, "s" ) && str_cmp( arg1, "south" )
      && str_cmp( arg1, "n" ) && str_cmp( arg1, "north" )
      && str_cmp( arg1, "e" ) && str_cmp( arg1, "east" )
      && str_cmp( arg1, "w" ) && str_cmp( arg1, "west" )
      && str_cmp( arg1, "u" ) && str_cmp( arg1, "up" )
      && str_cmp( arg1, "d" ) && str_cmp( arg1, "down" ) ) {

    if ( ( victim = get_char_room( ch, NULL, arg1 ) ) != NULL )
    {
        if ( IS_NPC(victim) && victim->pIndexData ) {
          char b[MSL]; sprintf(b,"MOB_%d", victim->pIndexData->vnum );
          display_ascii( b, ch );
        }
	show_char_to_char_1( victim, ch );
	return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{  /* player can see object */
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    {
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}
	    	else continue;
	    }

 	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
 	    if ( pdesc != NULL )
	    {
 	    	if (++count == number)
 	    	{	
		    send_to_char( pdesc, ch );
		    return;
	     	}
		else continue;
	    }
	    if ( is_name( arg3, obj->name ) )
	    	if (++count == number)
	    	{
              char b[MSL];
              sprintf( b, "OBJ_%d", obj->pIndexData->vnum );
              display_ascii( b, ch );
	    	    send_to_char( obj->description, ch );
	    	    send_to_char( "\n\r",ch);
		    return;
		  }
	  }
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}

	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}
	if ( is_name( arg3, obj->name ) )
	    if (++count == number)
	    {
              char b[MSL];
              sprintf( b, "OBJ_%d", obj->pIndexData->vnum );
              display_ascii( b, ch );
	    	send_to_char( obj->description, ch );
	    	send_to_char("\n\r",ch);
	    	return;
	    }
	}
    }
    }

    pdesc = get_extra_descr(arg3,ch->in_room->extra_descr);
    if (pdesc != NULL)
    {
	if (++count == number)
	{
	    send_to_char(pdesc,ch);
	    return;
	}
    }

    if (count > 0 && count != number)
    {
    	if (count == 1)
    	    sprintf(buf,"You only see one %s here.\n\r",arg3);
    	else
    	    sprintf(buf,"You only see %d of those here.\n\r",count);
    	send_to_char(buf,ch);
    	return;
    }

    if ( ( door = get_dir( arg1 ) ) == MAX_DIR )
    {
        to_actor( "You do not see that here.\n\r", ch );
        return;
    }

    ROOM_INDEX_DATA *in_scene=ch->in_room;

    /* 'look direction' */
    if ( ( pexit = in_scene->exit[door] ) == NULL )
    {
//        cmd_time( ch, "internal" );
        sprintf( buf, "There is no exit to the %s.\n\r",
                      dir_name[door] );
        send_to_char( buf, ch );
        return;
    }

    if ( pexit->u1.to_room != NULL && room_is_dark( pexit->u1.to_room ) )
    {
        if (door!=DIR_DOWN && door!=DIR_UP)
        sprintf( buf, "To the %s is darkness.\n\r", dir_name[door] );
        else sprintf( buf, "%s is darkness.\n\r", dir_name[door] );
        send_to_char( buf, ch );
        return;
    }

    if ( !MTD(pexit->description) )  to_actor( pexit->description, ch );
    else
    if ( MTD(pexit->keyword) || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
    {
        sprintf( buf, "There is nothing of note %sward from here.\n\r",
                      dir_name[door] );
        to_actor( buf, ch );
    }

    if ( !MTD(pexit->keyword)
      && !IS_SET(pexit->exit_info, EX_SECRET)
      && !IS_SET(pexit->exit_info, EX_CONCEALED) )
    {
        if ( IS_SET(pexit->exit_info, EX_CLOSED) )
        act( "The $T is closed.", ch, NULL, pexit->keyword, TO_ACTOR, POS_RESTING );
        else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
        act( "The $T is open.",   ch, NULL, pexit->keyword, TO_ACTOR, POS_RESTING );
    }

    if ( (( IS_SET(pexit->exit_info, EX_WINDOW)
      && !IS_SET(pexit->exit_info, EX_CLOSED) )
      || IS_SET(pexit->exit_info, EX_TRANSPARENT) )
      && pexit->u1.to_room != NULL )
    {
        act( "Through the $t you see $T:", ch, pexit->keyword, pexit->u1.to_room->name, TO_ACTOR, POS_RESTING );

        if ( !MTD(pexit->u1.to_room->description) )
        {
        to_actor( "   ", ch ); /* indent */
        send_to_char( pexit->u1.to_room->description, ch );
        }

//        cmd_time( ch, "internal" );
        show_list_to_char( pexit->u1.to_room->contents, ch, FALSE, FALSE );
/*        show_actor_to_actor( pexit->to_scene->people,   ch ); */
    }

    if ( !IS_SET(pexit->exit_info, EX_CLOSED)
      && !IS_SET(pexit->exit_info, EX_CONCEALED) )
    {
         if ( !IS_SET(pexit->exit_info, EX_WINDOW) )
         act( "$n glances $t.", ch, dir_name[door], NULL, TO_SCENE, POS_RESTING );
         else if ( strlen(pexit->keyword) )
         act( "$n peers through the $t.", ch, pexit->keyword, NULL, TO_SCENE, POS_RESTING );
         else
         act( "$n peers $t.", ch, dir_name[door], NULL, TO_SCENE, POS_RESTING );
    }

    scan_direction( ch, door );


    return;
}

void do_read (CHAR_DATA *ch, char *argument )
{
    do_look(ch,argument);
    return;
}

void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "What the hell are you looking at?\n\r", ch );
	return;
    }

    do_look( ch, arg1 );

    if ( ( obj = get_obj_here( ch, NULL, arg1 ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_SLOTS:
	    send_to_char("Type pull <wager> <money_value>\n\r",ch);
	    send_to_char("Example: pull 20 platinum\n\r",ch);
	    break;

	case ITEM_MONEY:
	    if (obj->value[0] == 0)
	    {
	        if (obj->value[1] == 0)
		    sprintf(buf,"Odd...there's no coins in the pile.\n\r");
		else if (obj->value[1] == 1)
		    sprintf(buf,"Wow. One gold coin.\n\r");
		else
		    sprintf(buf,"There are %d gold coins in the pile.\n\r",
			obj->value[1]);
	    }
	    else if (obj->value[1] == 0)
	    {
		if (obj->value[0] == 1)
		    sprintf(buf,"Wow. One silver coin.\n\r");
		else
		    sprintf(buf,"There are %d silver coins in the pile.\n\r",
			obj->value[0]);
	    }
	    else
		sprintf(buf,
		    "There are %d gold and %d silver coins in the pile.\n\r",
		    obj->value[1],obj->value[0]);
	    send_to_char(buf,ch);
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_PIT:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    sprintf(buf,"in %s",argument);
	    do_look( ch, buf );
	}
    }

    return;
}

void do_worth( CHAR_DATA *ch, char *argument )
{
    BUFFER *final = new_buf();
    char buf[MAX_STRING_LENGTH];

    add_buf( final, "{s =======================================================================\n\r" );
    sprintf( buf, "{s|{t Money: {q%9d {DPl{wa{Wti{wn{Dum    {s| {tExperience: {q%9ld {tgained (lvl {q%3d{t) {s|\n\r",
        ch->platinum, ch->exp, ch->level );
    add_buf( final, buf );

    sprintf( buf, "{s|          {q%7d {D{qG{Yol{qd        {s|                {q%6ld {ttill next level  {s|\n\r",
        ch->gold, IS_NPC(ch) ? 0 :
        ((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp ) );
    add_buf( final, buf );

    sprintf( buf, "{s|          {q%7d {WSi{wlv{Wer      {s|                {q%6ld {tbase per level   {s|{X\n\r", ch->silver, IS_NPC(ch) ? 0:
        (exp_per_level(ch,ch->pcdata->points)) );
    add_buf( final, buf );

    if (!IS_NPC (ch))
    {
    add_buf( final, "{s|=======================================================================|\n\r" );

        if (is_pkill (ch))
        {
            sprintf( buf, "{s| {RPK:        {g%5d {tkills       {s| {RArena:          {g%5d {tkills            {s|\n\r",
                ch->pcdata->pkills, ch->pcdata->arenakills );
            add_buf( final, buf );

            sprintf( buf, "{s|            {r%5d {tdeaths      {s|                 {r%5d {tdeaths           {s|\n\r",
                ch->pcdata->pdeath, ch->pcdata->arenadeath );
            add_buf( final, buf );

			sprintf( buf, "{s|            {y%5d {tassists     {s|                 {g%5d {twins             {s|\n\r",
                ch->pcdata->assist, ch->pcdata->arenawins );
            add_buf( final, buf );

			sprintf( buf, "{s|            {y%5d {tPoints      {s|                 {r%5d {tlosses           {s|\n\r",
                ch->pcdata->pkpoints, ch->pcdata->arenaloss );
            add_buf( final, buf );

        } else {
            sprintf( buf, "{s| {RPK:            {qNON-PK        {s| {RArena:          {g%5d {tkills            {s|\n\r",
                ch->pcdata->arenakills );
            add_buf( final, buf );

            sprintf( buf, "{s|                {qNON-PK        {s|                 {r%5d {tdeaths           {s|\n\r",
                ch->pcdata->arenadeath );
            add_buf( final, buf );

			sprintf( buf, "{s|                {qNON-PK        {s|                 {g%5d {twins             {s|\n\r",
                ch->pcdata->arenawins );
            add_buf( final, buf );

			sprintf( buf, "{s|                {qNON-PK        {s|                 {r%5d {tlosses           {s|\n\r",
                ch->pcdata->arenaloss );
            add_buf( final, buf );
        }

        add_buf( final, "{s|=======================================================================|\n\r" );

        if (ch->pcdata->questpoints)
        {
            sprintf( buf, "{s| {tAQuest:   {q%6d{t points      ",
                ch->pcdata->questpoints );
            add_buf( final, buf );
        } else {
            add_buf( final, "{s| {tAQuest:       {qno{t points      " );
        }

        if (ch->pcdata->deviant_points[0])
        {
            sprintf( buf, "{s| {tDeviant:        {q%5d{t points           {s|\n\r",
                ch->pcdata->deviant_points[0] );
            add_buf( final, buf );
        } else {
            add_buf( final, "{s| {tDeviant:           {qno{t points           {s|\n\r" );
        }

		add_buf( final, "{s|=======================================================================|\n\r{s| " );

        sprintf( buf, "{tYou have killed {g%ld{t mobs and have been killed {r%ld{t times.",
            ch->pcdata->mobkills, ch->pcdata->mobdeath );
        add_buf( final, end_string( buf, 70 ) );
		

        if (ch->pcdata->bounty > 0)
        {
			sprintf( buf, "{s|\n\r| {tYou have a {q%d{t platinum bounty on your head.",
                ch->pcdata->bounty );
            add_buf( final, end_string( buf, 75 ) );
        }

        if (IS_SET(ch->act, PLR_NOEXP) )
        {
			add_buf( final, "{s|\n\r| {qNOEXP{t is {RON{t: You {rwill not{t receive experience for kills.               {s|\n\r" );
        } else {
			add_buf( final, "{s|\n\r| {qNOEXP{t is {GOFF{t: You {gwill{t receive experience for kills.                  {s|\n\r" );
        }

    }

	add_buf( final, "{s ======================================================================={x\n\r" );

    page_to_char(final->string,ch);
    free_buf(final);

    return;
}

char target_color( int target )
{
    switch( target )
    {
	default:
	    return 'w';

	case TAR_CHAR_OFFENSIVE:
	case TAR_OBJ_CHAR_OFF:
	    return 'q';

	case TAR_CHAR_DEFENSIVE:
	case TAR_CHAR_SELF:
	case TAR_OBJ_CHAR_DEF:
	    return 't';
    }
}

BUFFER *display_affects( CHAR_DATA *ch, bool brief )
{
    BUFFER *final = new_buf();
    AFFECT_DATA *paf, *paf_last = NULL;
//    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    char affect[MAX_INPUT_LENGTH];
    char color;
//    int flag;

    if ( !brief )
    {
	if ( !IS_NPC(ch) )
	{
	    if ( ch->pcdata->pktimer > 0 )
	    {
		sprintf( buf, "{gYour {w({RPK{w) {gtimer lasts for {G%d {gmore seconds.{x\n\r\n\r", ch->pcdata->pktimer );
		add_buf( final, buf );
	    }

	    if ( ch->pcdata->dtimer > 0 )
	    {
		sprintf( buf, "{gYour {RKilled{x flag will last %d more ticks.{x\n\r\n\r", ch->pcdata->dtimer );
		add_buf( final, buf );
	    }
	}

	add_buf( final, show_dam_mods( ch->damage_mod ) );
    }

    if ( ch->affected_by != 0 || ch->shielded_by != 0 || ch->affected != NULL )
    {
	add_buf( final, "{s ----------------------------------------------------------------------------\n\r" );
	add_buf( final, "{s|     {tAffect      {s|               {tModifies                {s| {tMod. {s|  {tDuration {s|\n\r" );
	add_buf( final, "{s|----------------------------------------------------------------------------|\n\r" );
/*
	for ( flag = 0; object_affects[flag].spell != NULL; flag++ )
	{
	    bool found = FALSE;

	    if ( ( object_affects[flag].location == TO_AFFECTS
	    &&     IS_SET( ch->affected_by, object_affects[flag].bit ) )
	    ||   ( object_affects[flag].location == TO_SHIELDS
	    &&     IS_SET( ch->shielded_by, object_affects[flag].bit ) ) )
	    {
		for ( paf = ch->affected; paf != NULL; paf = paf->next )
		{
		    if ( paf->where == object_affects[flag].location
		    &&   paf->bitvector & object_affects[flag].bit )
		    {
			found = TRUE;
			break;
		    }
		}

		if ( !found )
		{
		    strcpy( affect, "unknown affect" );

		    if ( object_affects[flag].location == TO_AFFECTS
		    &&   IS_SET( race_table[ch->race].aff,object_affects[flag].bit ) )
			strcpy( affect, "racial affect" );

		    else if ( object_affects[flag].location == TO_SHIELDS
			 &&   IS_SET( race_table[ch->race].shd, object_affects[flag].bit ) )
			strcpy( affect, "racial shield" );

		    else
		    {
			for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
			{
			    if ( obj->wear_loc != WEAR_NONE )
			    {
				if ( !obj->enchanted )
				{
				    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
				    {
					if ( paf->where == object_affects[flag].location
					&&   object_affects[flag].bit & paf->bitvector )
					{
					    strcpy( affect, obj->short_descr );
					    found = TRUE;
					    break;
					}
				    }
				}

				if ( !found )
				{
				    for ( paf = obj->affected; paf != NULL; paf = paf->next )
				    {
					if ( paf->where == object_affects[flag].location
					&&   object_affects[flag].bit & paf->bitvector )
					{
					    strcpy( affect, obj->short_descr );
					    found = TRUE;
					    break;
					}
				    }
				}
			    }

			    if ( found )
				break;
			}
		    }

		    sprintf( buf, "{s| {t%-15.15s {s| {t%s {s| {t   0 {s| {tpermanent {s|\n\r",
			object_affects[flag].spell, end_string( affect, 37 ) );
		    add_buf( final, buf );
		}
	    }
	}
*/
	for ( paf = ch->affected; paf != NULL ; paf = paf->next )
	{
	    color = target_color( skill_table[paf->type].target );

	    if ( paf_last != NULL && paf->type == paf_last->type )
		sprintf( buf, "{s|                 |" );
	    else
		sprintf( buf, "{s| {%c%-15.15s {s|",
		    color, skill_table[paf->type].name );
	    add_buf( final, buf );
		
	    if ( paf->where == TO_DAM_MODS )
	    {
		sprintf( affect, "%s damage", paf->location == DAM_ALL ? "all" :
		    damage_mod_table[paf->location].name );
		sprintf( buf, " {%c%-37.37s {s|{t%4d%% {s|",
		    color, affect, paf->modifier );
	    }
	    else
		sprintf( buf, " {%c%-37.37s {s|{%c%5d%s{s|",
		    color, flag_string( apply_flags, paf->location ),
		    color, paf->modifier, paf->modifier > -10000 ? " " : "" );
	    add_buf( final, buf );

	    if ( paf->duration < 0 )
		sprintf( buf, " {%cpermanent {s|\n\r", color );
	    else if ( paf->dur_type == DUR_ROUNDS )
		sprintf( buf, "{%c%3d round%s {s|\n\r", color, paf->duration,
		    paf->duration == 1 ? " " : "s" );
	    else
		sprintf( buf, "{%c%4d hour%s {s|\n\r", color, paf->duration,
		    paf->duration == 1 ? " " : "s" );
	    add_buf( final, buf );

	    paf_last = paf;
	}

	add_buf( final,"{s ----------------------------------------------------------------------------{x\n\r" );
    } else
	add_buf( final, "{xYou are not affected by any spells.\n\r" );

    return final;
}

char * armor_display( int value )
{
    static char buf[40];

    sprintf( buf, "{w%6d ", value );

    if ( value >= 200 )
	strcat( buf, "{m[{r--------------------{m]" );

    else if ( value >= 150 )
	strcat( buf, "{m[{R*{r-------------------{m]" );

    else if ( value >= 100 )
	strcat( buf, "{m[{R**{r------------------{m]" );

    else if ( value >= 0 )
	strcat( buf, "{m[{R***{r-----------------{m]" );

    else if ( value >= -150 )
	strcat( buf, "{m[{R****{r----------------{m]" );

    else if ( value >= -200 )
	strcat( buf, "{m[{R*****{r---------------{m]" );

    else if ( value >= -250 )
	strcat( buf, "{m[{R******{r--------------{m]" );

    else if ( value >= -300 )
	strcat( buf, "{m[{R*******{r-------------{m]" );

    else if ( value >= -400 )
	strcat( buf, "{m[{R********{r------------{m]" );

    else if ( value >= -500 )
	strcat( buf, "{m[{R*********{r-----------{m]" );

    else if ( value >= -600 )
	strcat( buf, "{m[{R**********{r----------{m]" );

    else if ( value >= -700 )
	strcat( buf, "{m[{R***********{r---------{m]" );

    else if ( value >= -800 )
	strcat( buf, "{m[{R************{r--------{m]" );

    else if ( value >= -900 )
	strcat( buf, "{m[{R*************{r-------{m]" );

    else if ( value >= -1000 )
	strcat( buf, "{m[{R**************{r------{m]" );

    else if ( value >= -1100 )
	strcat( buf, "{m[{R***************{r-----{m]" );

    else if ( value >= -1200 )
	strcat( buf, "{m[{R****************{r----{m]" );

    else if ( value >= -1300 )
	strcat( buf, "{m[{R*****************{r---{m]" );

    else if ( value >= -1400 )
	strcat( buf, "{m[{R******************{r--{m]" );

    else if ( value>= -1500 )
	strcat( buf, "{m[{R*******************{r-{m]" );

    else
	strcat( buf, "{m[{R********************{m]" );

    return buf;
}

BUFFER *score1( CHAR_DATA *ch )
{
    BUFFER *final = new_buf( );
    char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

    add_buf( final, "\r{D+=============================================================================+\n\r|" );

    sprintf( buf1, "{W%s%s{x%s",
	ch->pcdata ? ch->pcdata->pretitle : "",
	ch->name,
	ch->pcdata ? ch->pcdata->title : " the mobile." );

    add_buf( final, center_string( buf1, 77 ) );

    add_buf( final, "{D|\n\r+----------------------+-----------------------+------------------------------+\n\r" );

    sprintf( buf1, "| {cRace{x:  {w%-14s{D|{c Practices{x:{w     %4d   {D|{c Next Level{x:  {w%12ld    {D|\n\r",
	capitalize(race_table[ch->race].name), 
	!ch->pcdata ? 0 : ch->pcdata->practice,
	!ch->pcdata ? 0 : 
	( (ch->level + 1) * exp_per_level( ch, ch->pcdata->points ) - ch->exp) );
    add_buf( final, buf1 );

    sprintf( buf1, "| {cClass{x: {w%-14s{D|{c Trains{x:{w        %4d   {D|{c Experience{x: {w%13ld    {D|\n\r",
	capitalize(class_table[ch->class].name),
	!ch->pcdata ? 0 : ch->pcdata->train, ch->exp );
    add_buf( final, buf1 );

    sprintf( buf1, "| {cLevel{x: {w%-4d          {D|{c Magic Power{x:{w  %5d   {D|{w P{Wlatinu{wm{x:          {w%6d    {D|\n\r",
	ch->level, ch->magic_power, ch->platinum );
    add_buf( final, buf1 );

    sprintf( buf1, "| {cSex{x:   {w%-8s      {D|{c Played Hours{x:{w %5d   {D|{y Gold{x:              {w%6d    {D|\n\r",
	ch->sex == 0 ? "Sexless" : ch->sex == 1 ? "Male" : "Female",
	ch->pcdata == NULL ? 0 :
	(int) (ch->pcdata->played + current_time - ch->pcdata->logon) / 3600,
	ch->gold );
    add_buf( final, buf1 );

    sprintf( buf1, "| {cAlign{x: {w%-5d         {D|{c Age{x:{w          %5d   {D|{x Silver:            {w%6d    {D|\n\r",
	ch->alignment, get_age( ch ), ch->silver );
    add_buf( final, buf1 );

    add_buf( final, "+----------------------+-----------------------+------------------------------+\n\r" );

    sprintf( buf2, "%d/%d", ch->hit, ch->max_hit );
    sprintf( buf1, "| {cHit{x:   {w%s{D|{c Hitroll{x:{w     %6d   {D|{c Clan{x: {w%s    {D|\n\r",
	end_string( buf2, 14 ), GET_HITROLL( ch ),
	begin_string( clan_table[ch->clan].color, 19 ) );
    add_buf( final, buf1 );

    sprintf( buf2, "%d/%d", ch->mana, ch->max_mana );
    sprintf( buf1, "| {cMana{x:  {w%s{D|{c Damroll{x:{w     %6d   {D|{c Rank{x: {w%s    {D|\n\r",
	end_string( buf2, 14 ), GET_DAMROLL( ch ), !ch->pcdata ? "none               " :
	begin_string( clan_table[ch->clan].crnk[ch->pcdata->clan_rank], 19 ) );
    add_buf( final, buf1 );

    sprintf( buf2, "%d/%d", ch->move, ch->max_move );
    sprintf( buf1, "| {cMoves{x: {w%s{D|{c Saves{x:{w        %5d   {D|{c Servant of{x:{w%s    {D|\n\r",
	end_string( buf2, 14 ), ch->saving_throw,
	IS_GOOD( ch ) ? begin_string( mud_stat.good_god_string, 14 ) :
	IS_EVIL( ch ) ? begin_string( mud_stat.evil_god_string, 14 ) :
	begin_string( mud_stat.neut_god_string, 14 ) );
    add_buf( final, buf1 );

    add_buf( final, "+----------------------+-----------------------+------------------------------+\n\r" );

    sprintf( buf1, "| {cStr{x:   {w%2d{m({w%2d{m)        {D|{c PKills{x:{w        %4d   {D|{c Quest Attempts{x:{w    %6d    {D|\n\r",
	ch->perm_stat[STAT_STR], get_curr_stat( ch, STAT_STR ),
	ch->pcdata ? ch->pcdata->pkills : 0,
	ch->pcdata ? ch->pcdata->total_questattempt : 0 );
    add_buf( final, buf1 );

    sprintf( buf1, "| {cInt{x:   {w%2d{m({w%2d{m)        {D|{c PDeaths{x:{w       %4d   {D|{c Quest Fails{x:{w       %6d    {D|\n\r",
	ch->perm_stat[STAT_INT], get_curr_stat( ch, STAT_INT ),
	ch->pcdata ? ch->pcdata->pdeath : 0,
	ch->pcdata ? ch->pcdata->total_questfail : 0 );
    add_buf( final, buf1 );

    sprintf( buf1, "| {cWis{x:   {w%2d{m({w%2d{m)        {D|{c PKPoints{x:{w      %4d   {D|{c Next Quest{x:{w           %3d    {D|\n\r",
	ch->perm_stat[STAT_WIS], get_curr_stat( ch, STAT_WIS ),
	ch->pcdata ? ch->pcdata->pkpoints : 0,
	ch->pcdata ? ch->pcdata->nextquest : 0 );
    add_buf( final, buf1 );

    sprintf( buf1, "| {cDex{x:   {w%2d{m({w%2d{m)        {D|{c Mob Kills{x:{w%9ld   {D|{c Aquest Points{x:{w     %6d    {D|\n\r",
	ch->perm_stat[STAT_DEX], get_curr_stat( ch, STAT_DEX ),
	ch->pcdata ? ch->pcdata->mobkills : 0,
	ch->pcdata ? ch->pcdata->questpoints : 0 );
    add_buf( final, buf1 );

    sprintf( buf1, "| {cCon{x:   {w%2d{m({w%2d{m)        {D|{c Mob Deaths{x:{w%8ld   {D|{c Deviant Points{x:{w     %5d    {D|\n\r",
	ch->perm_stat[STAT_CON], get_curr_stat( ch, STAT_CON ),
	ch->pcdata ? ch->pcdata->mobdeath : 0,
	ch->pcdata ? ch->pcdata->deviant_points[0] : 0 );
    add_buf( final, buf1 );

    add_buf( final, "+----------------------+-----------------------+------------------------------+\n\r" );

    sprintf( buf1, "| {cPierce{x:%s",
	armor_display( GET_AC( ch, AC_PIERCE ) ) );
    add_buf( final, buf1 );

    sprintf( buf1, "    {cBash{x: %s {D|\n\r",
	armor_display( GET_AC( ch, AC_BASH ) ) );
    add_buf( final, buf1 );

    sprintf( buf1, "| {cSlash{x: %s",
	armor_display( GET_AC( ch, AC_SLASH ) ) );
    add_buf( final, buf1 );

    sprintf( buf1, "    {cMagic{x:%s {D|\n\r",
	armor_display( GET_AC( ch, AC_EXOTIC ) ) );
    add_buf( final, buf1 );

    add_buf( final, "+-----------------------------------------------------------------------------+\n\r| " );

    sprintf( buf1, "{cYou are carrying {w%d/%d{c items with weight {w%d/%d{c lbs.",
	ch->carry_number, can_carry_n(ch),
	get_carry_weight(ch) / 10, can_carry_w(ch) / 10 );
    add_buf( final, end_string( buf1, 76 ) );

    add_buf( final, "{D|\n\r+=============================================================================+{x\n\r" );

    return final;
}

BUFFER *score2( CHAR_DATA *ch )
{
    BUFFER *final = new_buf();
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int i;

    if ( IS_NPC(ch) )
    {   
	add_buf( final, "This score screen is reserved for players.\n\r" );
	return final;
    }

    add_buf(final,"+-----------------------+---------------------+----------------------+\n\r");
    sprintf(buf, "                       {!%s {7the %s %s{x.\n\r",
	ch->name, race_table[ch->race].name, class_table[ch->class].name);
    add_buf(final,buf);

    add_buf(final,"+-----------------------+---------------------+----------------------+\n\r");
    sprintf(buf, "|{^S{6trength{x     : %-2d({$%-2d{x)  | {!R{1ace  {x: {#%-12s{x| {MP{mractices  {x: {@%d{x\n\r", 
	ch->perm_stat[STAT_STR],
	get_curr_stat(ch,STAT_STR), 
	race_table[ch->race].name,
	ch->pcdata->practice);
    add_buf(final,buf);

    sprintf(buf, "|{^I{6ntellect{x    : %-2d({$%-2d{x)  | {!C{1lass {x: {#%-12s{x| {MT{mrains    {x : {@%d{x\n\r",
	ch->perm_stat[STAT_INT],
	get_curr_stat(ch,STAT_INT),
	class_table[ch->class].name,
	ch->pcdata->train);
    add_buf(final,buf);

    sprintf(buf, "|{^W{6isdom{x       : %-2d({$%-2d{x)  | {!S{1ex   {x: {#%-12s{x| {wD{Devian{wt{rP{Rnt{rs{x: {@%d{x\n\r",
	ch->perm_stat[STAT_WIS],
	get_curr_stat(ch,STAT_WIS),
	ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
	ch->pcdata->deviant_points[0]);
    add_buf(final,buf);

    sprintf(buf, "|{^D{6exterity{x    : %-2d({$%-2d{x)  | {!L{1evel {x: {#%-12d{x|{M P{mlayed    {x : {@%d {8Hrs{x\n\r",
        ch->perm_stat[STAT_DEX],
        get_curr_stat(ch,STAT_DEX),
	ch->level,
 	(int) (ch->pcdata->played + current_time - ch->pcdata->logon) / 3600);
    add_buf(final,buf);

    sprintf(buf, "|{^C{6onstitution{x : %-2d({$%-2d{x)  | {!A{1ge   {x: {#%-12d{x| {MC{mlan      {x : %s\n\r",
	ch->perm_stat[STAT_CON],
	get_curr_stat(ch,STAT_CON),
	get_age(ch),
	clan_table[ch->clan].who_name == 0 ? " None" : clan_table[ch->clan].color); 
    add_buf(final,buf);

    add_buf(final,"+-----------------------+---------------------+----------------------+\n\r");

    sprintf(buf, "| {^H{6it    {x:{w%6d/{$%-7d{x| {!S{1aves     {x: {#%-8d{x| {MN{mext {ML{mevel {x: {@%ld{x\n\r",
  	ch->hit, ch->max_hit,
 	ch->saving_throw, IS_NPC(ch) ? 0 : 
	((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp) );
    add_buf(final,buf);

    if ( is_pkill(ch) )
    {
	sprintf(buf, "| {^M{6ana   {x:{w%6d/{$%-7d{x| {!P{1kills    {x: {#%-8d{x| {ME{mxperience {x: {@%ld{x\n\r", 
	    ch->mana, ch->max_mana, ch->pcdata->pkills, ch->exp );
	add_buf(final,buf);

	sprintf(buf, "| {^V{6p     {x:{w%6d/{$%-7d{x| {!P{1deaths   {x: {#%-8d{x| {8P{&latinum {x  : {@%d{x\n\r",
 	    ch->move, ch->max_move, ch->pcdata->pdeath, ch->platinum );
	add_buf(final,buf);
    } else {
	sprintf(buf, "| {^M{6ana   {x:{w%6d/{$%-7d{x|                     | {ME{mxperience {x: {@%ld{x\n\r", 
	    ch->mana, ch->max_mana, ch->exp );
	add_buf(final,buf);

	sprintf(buf, "| {^V{6p     {x:{w%6d/{$%-7d{x|                     | {8P{&latinum {x  : {@%d{x\n\r",
 	    ch->move, ch->max_move, ch->platinum );
	add_buf(final,buf);
    }

    sprintf(buf, "|{^H{6itroll{x : {w%-13d{x| {!Q{1uest{!P{1nts {x: {#%-8d{x| {3G{#old     {x  : {@%d{x\n\r", 
 	GET_HITROLL(ch),
	ch->pcdata->questpoints,
	ch->gold );
    add_buf(final,buf);

    sprintf(buf, "|{^D{6amroll{x : {w%-13d{x| {!N{1ext{!Q{1uest {x: {#%-8d{x| {&S{7ilver   {x  : {@%d{x\n\r", 
 	GET_DAMROLL(ch),
 	ch->pcdata->nextquest,
	ch->silver );
    add_buf(final,buf);

    add_buf(final,"+-----------------------+---------------------+----------------------+\n\r");

    sprintf(buf, "{$[{7Alignment: {8%d{$]	{$[{7Items/Weight: {8%d{&({7%d{&) {8%d{&({7%d{&){$]	{x\n\r",
	ch->alignment,ch->carry_number, can_carry_n(ch), get_carry_weight(ch) / 10, can_carry_w(ch) / 10); 
    add_buf(final,buf);

    if (ch->pcdata->bounty > 0 )
    {
	sprintf(buf, "{wWanted {Rd{re{Ra{rd {wfor {Y%d {8pl{7a{&ti{7n{8um{x\n\r",ch->pcdata->bounty);
	add_buf(final,buf);
    }

    sprintf(buf,"{BMob {RKills{w: {g%-10ld {BMob {RDeaths{w: {g%-10ld\n\r",
	ch->pcdata->mobkills, ch->pcdata->mobdeath);
    add_buf(final,buf);

    add_buf(final, "{7[{&*{$Armor Class{&*{7]{x\n\r");

    for (i = 0; i < 4; i++)
    {
	char * temp;

        switch(i)
        {
            case(AC_PIERCE):    temp = "piercing.";      break;
            case(AC_BASH):      temp = "bashing. ";      break;
            case(AC_SLASH):     temp = "slashing.";      break;
            case(AC_EXOTIC):    temp = "exotic.  ";      break;
            default:            temp = "error.   ";      break;
        }
        add_buf(final,"{xYou are ");

        if      (GET_AC(ch,i) >=  101 )
            sprintf(buf,"{Rnot safe from a dull butter knife{x against %s{x",temp);
        else if (GET_AC(ch,i) >= 50)
            sprintf(buf,"{Rdefenseless{x against %s{x",temp);
        else if (GET_AC(ch,i) >= 20)
            sprintf(buf,"{Rbarely protected{x from %s{x",temp);
        else if (GET_AC(ch,i) >= -20)
            sprintf(buf,"{yslightly armored{x against %s{x",temp);
        else if (GET_AC(ch,i) >= -50)
            sprintf(buf,"{ysomewhat armored{x against %s{x",temp);
        else if (GET_AC(ch,i) >= -100)
            sprintf(buf,"{yslightly armored{x against %s{x",temp);
        else if (GET_AC(ch,i) >= -150)
            sprintf(buf,"{ywell-armored{x against %s{x",temp);
        else if (GET_AC(ch,i) >= -200)
            sprintf(buf,"{yvery well-armored{x against %s{x",temp);
        else if (GET_AC(ch,i) >= -250)
            sprintf(buf,"{Bheavily armored{x against %s{x",temp);
        else if (GET_AC(ch,i) >= -300)
            sprintf(buf,"{Bsuperbly armored{x against %s{x",temp);
        else if (GET_AC(ch,i) >= -375)
            sprintf(buf,"{Ynearly invulnerable{x to %s{x",temp);
        else if (GET_AC(ch,i) >= -450)
            sprintf(buf,"{Yalmost invulnerable{x to %s{x",temp);
        else if (GET_AC(ch,i) >= -525)
            sprintf(buf,"{Wdivinely protected{x against %s{x",temp);
        else if (GET_AC(ch,i) >= -600)
            sprintf(buf,"{Whe{wave{Wnly protected{x against %s{x",temp);
        else if (GET_AC(ch,i) >= -700)
            sprintf(buf,"{#{8like a deadly {xarmored {Mtank{x against %s{x",temp);
        else if (GET_AC(ch,i) >= -800)
            sprintf(buf,"{8nearly {Mimpervious {xto {8damage{x against %s{x",temp);
        else if (GET_AC(ch,i) >= -950)
            sprintf(buf,"{8a fortress {xbuilt for {Wone{x against %s{x",temp);
        else
            sprintf(buf,"{8armored {7to the {&bone{x against %s{x",temp);

	sprintf(buf2, "%s {$[{7%d{$]{x\n\r",end_string(buf,58),
	    GET_AC(ch,i));
	add_buf(final,buf2);
    }

    if (ch->pcdata->invited)
    {
	sprintf( buf, "{RYou have been invited to join clan {x<%s>\n\r",
            clan_table[ch->pcdata->invited].color);
	add_buf(final,buf);
    }

    return final;
}

BUFFER *score3( CHAR_DATA *ch )
{
    BUFFER *final = new_buf();
    char buf[MAX_STRING_LENGTH];
    int i;

    sprintf( buf,
	"You are %s%s{x%s{x\n\r",
        IS_NPC(ch) ? "" : ch->pcdata->pretitle,
	ch->name,
	IS_NPC(ch) ? ", the mobile." : ch->pcdata->title);
    add_buf(final,buf);

    sprintf( buf,
	"Level {B%d{x,  {B%d{x years old.\n\r",
	ch->level, get_age(ch) );
    add_buf(final,buf);

    if ( get_trust( ch ) != ch->level )
    {
	sprintf( buf, "You are trusted at level {B%d{x.\n\r",
	    get_trust( ch ) );
	add_buf(final,buf);
    }

    sprintf(buf, "Race: {M%s{x  Sex: {M%s{x  Class:  {M%s{x\n\r",
	race_table[ch->race].name,
	ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
 	IS_NPC(ch) ? "mobile" : class_table[ch->class].name);
    add_buf(final,buf);
	
    sprintf( buf,
	"You have {G%d{x/{B%d{x hit, {G%d{x/{B%d{x mana, {G%d{x/{B%d{x movement.\n\r",
	ch->hit,  ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);
    add_buf(final,buf);

    if(!IS_NPC(ch))
    {
	sprintf( buf, "You have {B%d{x practices and {B%d{x training sessions.\n\r",
	    ch->pcdata->practice, ch->pcdata->train);
	add_buf(final,buf);

        sprintf(buf,"[{b{BARENA STATS{x] {gw{Gins{x: {y%d{x  {rl{Rosses{x: {y%d  {gk{Gills{x: {y%d  {rd{Reaths{x: {y%d{x\n\r",
        ch->pcdata->arenawins, ch->pcdata->arenaloss,
        ch->pcdata->arenakills, ch->pcdata->arenadeath);
	add_buf(final,buf);
    }

    sprintf( buf,
	"You are carrying {G%d{x/{B%d{x items with weight {G%d{x/{B%d{x pounds.\n\r",
	ch->carry_number, can_carry_n(ch),
	get_carry_weight(ch) / 10, can_carry_w(ch) /10 );
    add_buf(final,buf);

    sprintf( buf,
	"Str: {R%d{x({r%d{x)  Int: {R%d{x({r%d{x)  Wis: {R%d{x({r%d{x)  Dex: {R%d{x({r%d{x)  Con: {R%d{x({r%d{x)\n\r",
	ch->perm_stat[STAT_STR],
	get_curr_stat(ch,STAT_STR),
	ch->perm_stat[STAT_INT],
	get_curr_stat(ch,STAT_INT),
	ch->perm_stat[STAT_WIS],
	get_curr_stat(ch,STAT_WIS),
	ch->perm_stat[STAT_DEX],
	get_curr_stat(ch,STAT_DEX),
	ch->perm_stat[STAT_CON],
	get_curr_stat(ch,STAT_CON) );
    add_buf(final,buf);

    sprintf( buf,
	"You have {Y%d{x platinum, {Y%d{x gold and {Y%d{x silver coins.\n\r",
	ch->platinum, ch->gold, ch->silver);
    add_buf(final,buf);

    if (!IS_NPC(ch) && ch->level == LEVEL_HERO)
    {
	sprintf( buf,"You have scored {C%ld exp{x.\n\r",ch->exp);
	add_buf(final,buf);
    }
    else if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
    {
	sprintf( buf,"You have scored {C%ld exp{x. You need {C%ld exp{x to level.\n\r",
	    ch->exp, ((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp));
	add_buf(final,buf);
    }
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10 )
    {
	add_buf(final,"{yYou are drunk.{x\n\r");
    }
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
    {
	add_buf(final,"{yYou are thirsty.{x\n\r");
    }
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_HUNGER]   ==  0 )
    {
	add_buf(final,"{yYou are hungry.{x\n\r");
    }

    switch ( ch->position )
    {
    case POS_DEAD:     
	add_buf(final,"{RYou are DEAD!!{x\n\r");
	break;
    case POS_MORTAL:
	add_buf(final,"{RYou are mortally wounded.{x\n\r");
	break;
    case POS_INCAP:
	add_buf(final,"{RYou are incapacitated.{x\n\r");
	break;
    case POS_STUNNED:
	add_buf(final,"{RYou are stunned.{x\n\r");
	break;
    case POS_SLEEPING:
	add_buf(final,"{BYou are sleeping.{x\n\r");
	break;
    case POS_RESTING:
	add_buf(final,"{BYou are resting.{x\n\r");
	break;
    case POS_STANDING:
	add_buf(final,"{BYou are standing.{x\n\r");
	break;
    case POS_FIGHTING:
	add_buf(final,"{RYou are fighting.{x\n\r");
	break;
    }


    if (ch->level >= 25)
    {	
	sprintf( buf,"Armor: pierce: {G%d{x  bash: {G%d{x  slash: {G%d{x  magic: {G%d{x\n\r",
		 GET_AC(ch,AC_PIERCE),
		 GET_AC(ch,AC_BASH),
		 GET_AC(ch,AC_SLASH),
		 GET_AC(ch,AC_EXOTIC));
	add_buf(final,buf);
    }

    for (i = 0; i < 4; i++)
    {
	char * temp;

	switch(i)
	{
	    case(AC_PIERCE):	temp = "piercing";	break;
	    case(AC_BASH):	temp = "bashing";	break;
	    case(AC_SLASH):	temp = "slashing";	break;
	    case(AC_EXOTIC):	temp = "magic";		break;
	    default:		temp = "error";		break;
	}
	
	add_buf(final,"You are ");

	if      (GET_AC(ch,i) >=  101 ) 
	    sprintf(buf,"{Rhopelessly vulnerable{x to %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 80) 
	    sprintf(buf,"{Rdefenseless{x against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 60)
	    sprintf(buf,"{Rbarely protected{x from %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 40)
	    sprintf(buf,"{yslightly armored{x against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 20)
	    sprintf(buf,"{ysomewhat armored{x against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= 0)
	    sprintf(buf,"{yarmored{x against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -20)
	    sprintf(buf,"{ywell-armored{x against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -40)
	    sprintf(buf,"{yvery well-armored{x against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -60)
	    sprintf(buf,"{Bheavily armored{x against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -80)
	    sprintf(buf,"{Bsuperbly armored{x against %s.\n\r",temp);
	else if (GET_AC(ch,i) >= -100)
	    sprintf(buf,"{Balmost invulnerable{x to %s.\n\r",temp);
	else
	    sprintf(buf,"{Wdivinely armored{x against %s.\n\r",temp);
	add_buf(final,buf);
    }

    if ( IS_IMMORTAL(ch))
    {
	add_buf(final,"Holy Light: ");
      if (IS_SET(ch->act,PLR_HOLYLIGHT))
	add_buf(final,"on");
      else
        add_buf(final,"off");

      if (ch->invis_level)
      {
        sprintf( buf, "  Invisible: level %d",ch->invis_level);
	add_buf(final,buf);
      }

      if (ch->incog_level)
      {
	sprintf(buf,"  Incognito: level %d",ch->incog_level);
	add_buf(final,buf);
      }
      add_buf(final,"\n\r");
    }

    if ( ch->level >= 15 )
    {
	sprintf( buf, "Hitroll: {G%d{x  Damroll: {G%d{x  Saves: {G%d{x.\n\r",
	    GET_HITROLL(ch), GET_DAMROLL(ch), ch->saving_throw );
	add_buf(final,buf);
    }
    
    if ( ch->level >= 10 )
    {
	sprintf( buf, "Alignment: {B%d{x.  ", ch->alignment );
	add_buf(final,buf);
    }

    add_buf(final,"You are ");
         if ( ch->alignment >  900 ) sprintf(buf, "{Wangelic{x.\n\r");
    else if ( ch->alignment >  700 ) sprintf(buf, "{Wsaintly{x.\n\r");
    else if ( ch->alignment >  350 ) sprintf(buf, "{wgood{x.\n\r");
    else if ( ch->alignment >  100 ) sprintf(buf, "kind.\n\r");
    else if ( ch->alignment > -100 ) sprintf(buf, "neutral.\n\r");
    else if ( ch->alignment > -350 ) sprintf(buf, "mean.\n\r");
    else if ( ch->alignment > -700 ) sprintf(buf, "{revil{x.\n\r");
    else if ( ch->alignment > -900 ) sprintf(buf, "{Rdemonic{x.\n\r");
    else                             sprintf(buf, "{Rsatanic{x.\n\r");

    add_buf(final,buf);

    if (!IS_NPC( ch ))
    {
	if (ch->pcdata->bounty > 0)
	{
	    sprintf( buf, "Bounty: {#%d{x\n\r",ch->pcdata->bounty);
	    add_buf(final,buf);
	}
	if (is_pkill(ch))
	{
	    sprintf( buf, "{8S{7o{8u{7l{8s {1C{!a{1p{!t{1u{!r{1e{!d{7: {8%d{x\n\r"
	    		  "{8S{7o{8u{7l{8s {1S{!u{1r{!r{1e{!n{1d{!e{1r{!e{1d{7: {8%d{x\n\r",
		ch->pcdata->pkills, ch->pcdata->pdeath);
	    add_buf(final,buf);
	}
	sprintf(buf,"{BMob {RKills{w: {g%7ld {BMob {RDeaths{w: {g%7ld{x\n\r",
            ch->pcdata->mobkills, ch->pcdata->mobdeath);
	add_buf(final,buf);

	if (ch->pcdata->deviant_points[0])
	{
	    if (ch->pcdata->deviant_points[0] == 1)
		sprintf( buf, "You have {D1 {wD{Devian{wt {rP{Roin{rt{x.\n\r" );
	    else
		sprintf( buf, "You have {8%d {wD{Devian{wt {rP{Roint{rs{x.\n\r", ch->pcdata->deviant_points[0] );
	    add_buf(final,buf);
	}
    }

    if (ch->pcdata && ch->pcdata->questpoints)
    {
        if (ch->pcdata->questpoints == 1)
            sprintf( buf, "You have {R1 {GQ{guest {GP{goint{x.\n\r" );
        else
            sprintf( buf, "You have {R%d {GQ{guest {GP{goints{x.\n\r", ch->pcdata->questpoints );
	add_buf(final,buf);
    }
    if (ch->pcdata && ch->pcdata->invited)
    {
        sprintf( buf, "{RYou have been invited to join clan {x<%s>\n\r",
            clan_table[ch->pcdata->invited].color);
	add_buf(final,buf);
    }

    return final;
}

BUFFER *score4( CHAR_DATA * ch )
{
    BUFFER *final = new_buf();
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC (ch))
	sprintf (buf, "\n\r%s\n\r", ch->name);
    else
	sprintf (buf, "\n\r%s%s{x%s\n\r",ch->pcdata->pretitle,ch->name, ch->pcdata->title);
    add_buf(final,buf);

    add_buf(final,"{y _____________________________________________________________________________\n\r");
    add_buf(final,"|\\___________________________________________________________________________/|\n\r");

    sprintf (buf, "||{WLevel{x: %-3d       {WRace{x: %-13s  {WGender{x: %-6s {WClass{x: %-10s     {y||\n\r",
	ch->level, race_table[ch->race].name,
	ch->sex == 0 ? "None" : ch->sex == 1 ? "Male" : "Female",
	IS_NPC (ch) ? "Mobile" : class_table[ch->class].name );
    add_buf(final,buf);

    if (!IS_NPC (ch))
    {
	sprintf (buf, "||{WPractices{x: %-5d {WTrains{x: %-5d {WExperience{x: %-9ld {WNeeded to level{x: %-5ld{y||\n\r",
	    ch->pcdata->practice, ch->pcdata->train, ch->exp, 
	    IS_NPC(ch) ? 0 : 
	    ((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp) );
	add_buf(final,buf);
    }

    if (get_trust (ch) > LEVEL_HERO)
    {
	sprintf (buf, "||{WTrust{x: %-3d {WHoly Light{x: %-3s {WWizInvis Level{x: %-3d       {WIncognito Level{x: %-3d  {y||\n\r",
	    get_trust (ch), IS_SET (ch->act, PLR_HOLYLIGHT) ? "ON" : "OFF",
	    ch->invis_level, ch->incog_level);
	add_buf(final,buf);
    }

    add_buf(final,"{y||___________________________________________________________________________||\n\r");
    add_buf(final,"|/___________________________________________________________________________\\|\n\r");
    add_buf(final,"|\\_______________________________/| |\\_______________________________________/|\n\r");
    sprintf (buf,"||{WStr{x: {Y%-2d{x({C%2d{x)  {WHit{x: %-2d {WDam{x: %-2d   {y|| || {WHit{x: %6d/%-6d {WSaves: %4d        {y||\n\r",
	ch->perm_stat[STAT_STR], get_curr_stat (ch, STAT_STR),
	str_app[get_curr_stat(ch, STAT_STR)].tohit,
	str_app[get_curr_stat (ch, STAT_STR)].todam, ch->hit, ch->max_hit,
	ch->saving_throw);
    add_buf(final,buf);

    sprintf (buf,"||{x{WInt{x: {Y%-2d{x({C%2d{x)  {WLearn{x: %-2d%%        {y|| ||{x {WMana{x:%6d/%-6d {WMove{x:%6d/%-6d {y||\n\r",
	ch->perm_stat[STAT_INT], get_curr_stat (ch,STAT_INT),
	int_app[get_curr_stat (ch, STAT_INT)].learn, ch->mana, ch->max_mana,
	ch->move, ch->max_move);
    add_buf(final,buf);

    sprintf (buf,"||{x{WWis{x: {Y%-2d{x({C%2d{x)  {WPracs{x: %-8d   {y|| ||{x {WHitroll{x: %-5d     {WDamroll{x: %-5d     {y||\n\r",
	ch->perm_stat[STAT_WIS], get_curr_stat (ch, STAT_WIS),
	wis_app[get_curr_stat (ch, STAT_WIS)].practice, GET_HITROLL (ch),
	GET_DAMROLL (ch));
    add_buf(final,buf);

    sprintf (buf,"||{x{WDex{x: {Y%-2d{x({C%2d{x)  {WACmod{x: %-4d       {y|| ||{x {WACpierce{x: %-5d    {WACbash{x: %-5d      {y||\n\r",
	ch->perm_stat[STAT_DEX], get_curr_stat (ch, STAT_DEX),
	dex_app[get_curr_stat (ch, STAT_DEX)].defensive,
	GET_AC (ch, AC_PIERCE), GET_AC (ch, AC_BASH));
    add_buf(final,buf);

    sprintf (buf,"||{x{WCon{x: {Y%-2d{x({C%2d{x)  {WHP{x: %-2d {WSS{x: %-2d%%    {y|| ||{x {WACslash{x: %-8d  {WACexotic{x: %-5d    {y||\n\r",
	ch->perm_stat[STAT_CON], get_curr_stat (ch,STAT_CON),
	con_app[get_curr_stat (ch, STAT_CON)].hitp,
	con_app[get_curr_stat (ch, STAT_CON)].shock, GET_AC (ch, AC_SLASH),
	GET_AC (ch, AC_EXOTIC));
    add_buf(final,buf);

    add_buf(final,"||_______________________________|| ||_______________________________________||\n\r");
    add_buf(final,"|/_______________________________\\| |/_______________________________________\\|\n\r");
    add_buf(final,"|\\___________________________________________________________________________/|\n\r");
    sprintf(buf,"||{WPlatinum{x: %-5d              {WGold{x: %-5d              {WSilver{x: %-7d      {y||\n\r",
	ch->platinum, ch->gold, ch->silver);
    add_buf(final,buf);

    sprintf(buf,"||{WItems{x: %4d/%-4d             {WWeight{x: %7d/%-7d  {WAlign{x: %-5d         {y||\n\r",
	ch->carry_number, can_carry_n (ch),
	get_carry_weight (ch) / 10, can_carry_w (ch) / 10, ch->alignment);
    add_buf(final,buf);

    sprintf(buf,"||{WSize:{x %-9s              {WDeviant Points:{x %-5d    {WQuest Points:{x %-6d {y||\n\r",
	size_flags[ch->size].name,
	IS_NPC(ch) ? 0 : ch->pcdata->deviant_points[0],
	ch->pcdata->questpoints);
    add_buf(final,buf);

    if (is_clan (ch))
    {
	if (is_pkill(ch))
	{
	    sprintf(buf,"||{WCLAN: %s    {WPkill:{x %-4d              {WPdeath:{x %-4d         {y||\n\r",
		end_string(clan_table[ch->clan].color,19),
		ch->pcdata->pkills, ch->pcdata->pdeath);
	} else {
	    sprintf(buf,"||{WCLAN: %s                                                  {y||\n\r",
		end_string(clan_table[ch->clan].color,19));
	}
	add_buf(final,buf);
    }

    if (!IS_NPC (ch))
    {
	sprintf(buf,"||{WMob Kills:{x %-10ld        {WMob Deaths:{x %-10ld   {WAge:{x %3d             {y||\n\r",
	    ch->pcdata->mobkills, ch->pcdata->mobdeath, get_age( ch ) );
	add_buf(final,buf);

	sprintf(buf,"||[{^Arena Stats{3] {WWins:{x %-5d    {WLosses:{x %-5d            {WHours: {x%-5d         {y||\n\r",
	    ch->pcdata->arenawins, ch->pcdata->arenaloss,
	    (int) (ch->pcdata->played + current_time - ch->pcdata->logon) / 3600);
	add_buf(final,buf);

	sprintf(buf,"||              {WKills:{x %-8d{WDeaths{x: %-8d                              {y||\n\r", 
	    ch->pcdata->arenakills, ch->pcdata->arenadeath);
	add_buf(final,buf);

    }

    if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	add_buf(final,"|| {xYou are {ydrunk.                                                            ||\n\r");
    if (!IS_NPC (ch) && ch->pcdata->condition[COND_THIRST] == 0)
	add_buf(final,"|| {xYou are {^thirsty.                                                          {y||\n\r");
    if (!IS_NPC (ch) && ch->pcdata->condition[COND_HUNGER] == 0)
	add_buf(final,"|| {xYou are {#hungry.                                                           {y||\n\r");

    switch (ch->position)
    {
	case POS_DEAD:
	    add_buf(final,"||{xYou are {z{!DEAD!!{x                                                             {y||\n\r");
	    break;
	case POS_MORTAL:
 	    add_buf(final,"||{xYou are {%mortally wounded!!                                                 {y||\n\r");
	    break;
	case POS_INCAP:
	    add_buf(final,"||{xYou are {1incapacitated.                                                     {y||\n\r");
	    break;
	case POS_STUNNED:
	    add_buf(final,"||{xYou are {@stunned.                                                           {y||\n\r");
	    break;
	case POS_SLEEPING:
	    add_buf(final,"||{xYou are {$sleeping.                                                          {y||\n\r");
	    break;
	case POS_RESTING:
	    add_buf(final,"||{xYou are {4resting.                                                           {y||\n\r");
	    break;
	case POS_STANDING:
	    add_buf(final,"||{xYou are {5standing.                                                          {y||\n\r");
	    break;
	case POS_FIGHTING:
	    add_buf(final,"||{xYou are {&fighting.                                                          {y||\n\r");
	    break;
    }
    add_buf(final,"||___________________________________________________________________________||\n\r");
    add_buf(final,"|/___________________________________________________________________________\\|{x\n\r");

    return final;
}

BUFFER *score5( CHAR_DATA *ch )
{
    BUFFER *final = new_buf();
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    {
        add_buf(final,"This score screen is reserved for players.\n\r");
        return final;
    }

    add_buf(final,"{c+{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c+\n\r");

    sprintf(buf2,"%s%s{x%s{x",ch->pcdata->pretitle,ch->name,ch->pcdata->title);

    sprintf(buf,"{D|{cName{C:{x %s{D|\n\r",end_string(buf2,71));
    add_buf(final,buf);

    sprintf(buf,"|{cAge{C:{x %-3d                {cHours{C:{x %-5d          {cAlignent{C:{x %-5d                {D|\n\r",
	get_age(ch),(int) (ch->pcdata->played + current_time - ch->pcdata->logon) / 3600, ch->alignment);
    add_buf(final,buf);

    sprintf(buf,"|{cRace{C:{x %-18s{cClass{C:{x %-15s{cTrust{C:{x %-4d                    {D|\n\r",
	race_table[ch->race].name, class_table[ch->class].name, get_trust(ch));
    add_buf(final,buf);

    sprintf(buf,"|{cLevel{C:{x %3d              {cGender{C:{x %-10s                                   {D|\n\r",
	ch->level, ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female");
    add_buf(final,buf);

    add_buf(final,"{c+{D-----------------------------------------------------------------------------{c+\n\r");

    sprintf(buf,"{D|{cHit Points{C:{x %6d{C/{x%-6d   {cMana{C: {x%6d{C/{x%-6d    {cMovement{C:{x %6d{C/{x%-6d   {D|\n\r",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);
    add_buf(final,buf);

    sprintf(buf,"|{cPractices{C:{x %-5d      {cExperience{C:{x %-16ld {cMobs Killed{C:{x  %-10ld  {D|\n\r",
	ch->pcdata->practice, ch->exp, ch->pcdata->mobkills);
    add_buf(final,buf);

    sprintf(buf,"|{cTrainings{C:{x %-5d      {cNext Level{C:{x %-16ld {cMobs Died to{C:{x %-10ld  {D|\n\r",
	ch->pcdata->train, IS_NPC(ch) ? 0 : 
	((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp),
	ch->pcdata->mobdeath);
    add_buf(final,buf);

    add_buf(final,"{c+{D-----------------------------------------------------------------------------{c+\n\r");

    sprintf(buf,"{D|{cStrength{C:       {w%2d {D( {w%2d{D)  {cModifies Hit{C: {x%-3d {C/ {cDamage{C:{x %-3d                    {D|\n\r",
	ch->perm_stat[STAT_STR], get_curr_stat (ch, STAT_STR),
	str_app[get_curr_stat(ch, STAT_STR)].tohit,
	str_app[get_curr_stat(ch, STAT_STR)].todam);
    add_buf(final,buf);

    sprintf(buf,"|{cIntelligence{C:   {w%2d {D( {w%2d{D)  {cModifies Practice Percentages by {x%-2d{C%%               {D|\n\r",
	ch->perm_stat[STAT_INT], get_curr_stat (ch, STAT_INT),
	int_app[get_curr_stat (ch, STAT_INT)].learn);
    add_buf(final,buf);

    sprintf(buf,"|{cWisdom{C:         {w%2d {D( {w%2d{D)  {cModifies Practices Per Level by {x%-3d                {D|\n\r",
	ch->perm_stat[STAT_WIS], get_curr_stat (ch, STAT_WIS),
	wis_app[get_curr_stat (ch, STAT_WIS)].practice);
    add_buf(final,buf);

    sprintf(buf,"|{cDexterity{C:      {w%2d {D( {w%2d{D)  {cModifies Armor Class by {x%-4d                       {D|\n\r",
	ch->perm_stat[STAT_DEX], get_curr_stat (ch, STAT_DEX),
	dex_app[get_curr_stat (ch, STAT_DEX)].defensive);
    add_buf(final,buf);

    sprintf(buf,"|{cConstitution{C:   {w%2d {D( {w%2d{D)  {cModifies Hit Point Gains by {x%-3d                    {D|\n\r",
	ch->perm_stat[STAT_CON], get_curr_stat (ch,STAT_CON),
	con_app[get_curr_stat (ch, STAT_CON)].hitp);
    add_buf(final,buf);

    add_buf(final,"{c+{D-----------------------------------------------------------------------------{c+\n\r");

    sprintf(buf,"{D| {cTo Hit{C:    {x%5d              {cArmor Versus Piercing{C:  {x%5d                 {D|\n\r",
	GET_HITROLL(ch), GET_AC(ch,AC_PIERCE));
    add_buf(final,buf);

    sprintf(buf,"| {cTo Damage{C: {x%5d              {cArmor Versus Bashing{C:   {x%5d                 {D|\n\r",
	GET_DAMROLL(ch), GET_AC(ch,AC_BASH));
    add_buf(final,buf);

    sprintf(buf,"| {cTo Saves{C:  {x%5d              {cArmor Versus Slashing{C:  {x%5d                 {D|\n\r",
	ch->saving_throw, GET_AC(ch,AC_SLASH));
    add_buf(final,buf);

    sprintf(buf,"|                               {cArmor Versus Magic{C:     {x%5d                 {D|\n\r",
	GET_AC(ch,AC_EXOTIC));
    add_buf(final,buf);

    add_buf(final,"{c+{D-----------------------------------------------------------------------------{c+\n\r");

    sprintf(buf,"{D|               {cPlatinum{C: {x%5d          {cQuest Points{C:     {x%10d         {D|\n\r",
	ch->platinum, ch->pcdata->questpoints);
    add_buf(final,buf);

    sprintf(buf,"|               {cGold{C:     {x%5d          {cQuest Time{C:              {x%3d         {D|\n\r",
	ch->gold, ch->pcdata->nextquest);
    add_buf(final,buf);

    sprintf(buf,"|               {cSilver{C:   {x%5d          {cDeviant Points{C: {x%12d         {D|\n\r",
	ch->silver, ch->pcdata->deviant_points[0]);
    add_buf(final,buf);

    add_buf(final,"{c+{D-----------------------------------------------------------------------------{c+\n\r");

    if (is_pkill(ch))
    {
	sprintf(buf,"{D|{cPK Kills{C:   {x%4d   {cArena Wins{C:    {x%4d        {cItems{C:    {x%7d {C/ {x%-7d    {D|\n\r",
	    ch->pcdata->pkills, ch->pcdata->arenawins, ch->carry_number, can_carry_n (ch));
	add_buf(final,buf);

	sprintf(buf,"|{cPK Deaths{C:  {x%4d   {cArena Losses{C:  {x%4d        {cWeight{C:   {x%7d {C/ {x%-7d    {D|\n\r",
	    ch->pcdata->pdeath, ch->pcdata->arenaloss, get_carry_weight (ch) / 10, can_carry_w (ch) / 10);
	add_buf(final,buf);
    } else {
	sprintf(buf,"{D|                   {cArena Wins{C:    {x%4d        {cItems{C:    {x%7d {C/ {x%-7d    {D|\n\r",
	    ch->pcdata->arenawins, ch->carry_number, can_carry_n (ch));
	add_buf(final,buf);

	sprintf(buf,"|                   {cArena Losses{C:  {x%4d        {cWeight{C:   {x%7d {C/ {x%-7d    {D|\n\r",
	    ch->pcdata->arenaloss, get_carry_weight (ch) / 10, can_carry_w (ch) / 10);
	add_buf(final,buf);
    }

    add_buf(final,"{c+{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c-{D={c+\n\r");

    if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	add_buf(final,"{cYou are {ydrunk{c.\n\r");
    if (!IS_NPC (ch) && ch->pcdata->condition[COND_THIRST] == 0)
	add_buf(final,"{cYou are {Bthirsty{c.\n\r");
    if (!IS_NPC (ch) && ch->pcdata->condition[COND_HUNGER] == 0)
	add_buf(final,"{cYou are {Yhungry{c.\n\r");

    switch (ch->position)
    {
	case POS_DEAD:
	    add_buf(final,"{cYou are {RDEAD{c!!{x\n\r");
	    break;
	case POS_MORTAL:
	    add_buf(final,"{cYou are {Rmortally wounded{c!{x\n\r");
	    break;
	case POS_INCAP:
	    add_buf(final,"{cYou are {Rincapacitated{c!{x\n\r");
	    break;
	case POS_STUNNED:
	    add_buf(final,"{cYou are {Gstunned{c.{x\n\r");
	    break;
	case POS_SLEEPING:
	    add_buf(final,"{cYou are {bsleeping{c.{x\n\r");
	    break;
	case POS_RESTING:
	    add_buf(final,"{cYou are {gresting{c.{x\n\r");
	    break;
	case POS_STANDING:
	    add_buf(final,"{cYou are {wstanding{c.{x\n\r");
	    break;
	case POS_FIGHTING:
	    add_buf(final,"{cYou are {Rfighting{c.{x\n\r");
	    break;
    }

    return final;
}

void do_score( CHAR_DATA *ch, char *argument )
{
    BUFFER *final, *affects;

    switch ( *argument )
    {
	default:
	case '1':	final = score1( ch );	break;
	case '2':	final = score2( ch );	break;
	case '3':	final = score3( ch );	break;
	case '4':	final = score4( ch );	break;
	case '5':	final = score5( ch );	break;
    }

    if ( IS_SET( ch->configure, CONFIG_SHOW_AFFECTS ) )
    {
	add_buf( final, "\n\r" );
	affects = display_affects( ch, TRUE );
	add_buf( final, affects->string );
	free_buf( affects );
    }

    page_to_char( final->string, ch );
    free_buf( final );
}

void do_affects(CHAR_DATA *ch, char *argument )
{
    BUFFER *output;

    output = display_affects( ch, FALSE );
    page_to_char( output->string, ch );
    free_buf( output );
    return;
}

char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[128];
    char buf[MAX_STRING_LENGTH];
    char time[100];
    char *suf;
    int day;

    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    sprintf( buf, "{cIt is {w%d {co'clock {w%s{c, Day of {w%s{c, the {w%d%s {cMonth of {w%s{c.{x\n\r",
	( time_info.hour % 12 == 0 ) ? 12 : time_info.hour % 12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name[day % 7],
	day, suf, month_name[time_info.month] );
    send_to_char( buf, ch );

    strftime( time, 100, "%A{c, {w%B %d{c, {w%Y{c, at {w%I{c:{w%M{c:{w%S{c %p",
	localtime( &ch->id ) );

    sprintf( buf, "\n\r{cYou entered this world on {w%s{c.\n\r", time );
    send_to_char( buf, ch );

    strftime( time, 100, "%A{c, {w%B %d{c, {w%Y{c, at {w%I{c:{w%M{c:{w%S{c %p",
	localtime( &current_time ) );

    sprintf( buf, "\n\r%s {cstarted: {w%s{c.\n\r%s {ccurrent: {w%s{c.{x\n\r",
	mud_stat.mud_name_string, str_boot_time,
	mud_stat.mud_name_string, time );
    send_to_char( buf, ch );

    return;
}

void do_weather( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    static char * const sky_look[4] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You can't see the weather indoors.\n\r", ch );
	return;
    }

    sprintf( buf, "The sky is %s and %s.\n\r",
	sky_look[weather_info.sky],
	weather_info.change >= 0
	? "a warm southerly breeze blows"
	: "a cold northern gust blows" );
    send_to_char( buf, ch );
    return;
}

void do_index( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    HELP_DATA *pHelp;
    bool fAll;
    char buf[MAX_STRING_LENGTH];
    sh_int i = 0;

    if ( argument[0] == '\0' )
    {
	send_to_char(	"Syntax: index all\n\r"
			"        index <keywords>\n\r", ch );
	return;
    }

    fAll = !str_cmp( argument, "all" );

    final = new_buf( );

    add_buf( final, "{s[{tNum{s] {qNAME                           {s[{tKEYWORDS                                {s]\n\r"
		    "{s-------------------------------------------------------------------------------\n\r" );

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	if ( pHelp->level > get_trust( ch )
	||   ( !fAll && !is_name( argument, pHelp->keyword ) ) )
	    continue;

	i++;

	sprintf( buf, "{s[{t%3d{s] {q%s {s[{t%-40.40s{s]\n\r",
	    i, end_string( pHelp->name, 30 ), pHelp->keyword );
	add_buf( final, buf );
    }

    add_buf( final, "{x" );

    if ( i == 0 )
	send_to_char( "No matches found.\n\r", ch );
    else
	page_to_char( final->string, ch );

    free_buf( final );
}

void do_help( CHAR_DATA *ch, char *argument )
{
    BUFFER *output;
    HELP_DATA *pHelp;
    char *rdesc;
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH], final[MAX_STRING_LENGTH];
//    bool type = TRUE;
    int i, width;

    if ( argument[0] == '\0' )
	argument = "summary";

    if ( argument[0] == '#' ) {
      char b[MAX_STRING_LENGTH];
      bool fFound = FALSE;

      argument++;
      argument=skip_spaces(argument);
      if ( *argument == '\0' ) { 
        page_to_char( "No search string entered, try HELP #keyword\n\r", ch );
        page_to_char( "or HELP #'phrase'\n\r", ch );
        return;
      }

      sprintf( b, "Possibly there were these related topics to your request `%s`:\n\r", argument );
      pHelp = help_first;
      while ( pHelp != NULL ) {
         if ( pHelp->level > get_trust( ch ) ) {
            pHelp=pHelp->next;
            continue;
         }

         if ( strlen(b) > MAX_STRING_LENGTH/4 ) { strcat( b, "Too many topics found, please narrow your search.\n\r" ); break; }

         if ( (pHelp->keyword && !str_infix_2( argument, pHelp->keyword ))
           || (pHelp->name && !str_infix_2( argument, pHelp->name ))
           || (pHelp->text && !str_infix_2( argument, pHelp->text ))
           || (IS_IMMORTAL(ch) && pHelp->immtext && !str_infix_2( argument, pHelp->immtext)) ) {
                  fFound = TRUE;
                  if ( pHelp->name && strlen(pHelp->name) < 50 ) strcat( b, pHelp->name ); else strcat( b, "(null!)" );
                  if ( pHelp->keyword && strlen(pHelp->keyword) > 0 && strlen(pHelp->keyword) < 50 ) {
                  strcat( b, " [" );
                  strcat( b, pHelp->keyword );
                  strcat( b, "]"  );
                  }
                  strcat( b, "\n\r" );
         }
        pHelp = pHelp->next;
      }
      page_to_char( b, ch );
      if ( !fFound ) { 
       page_to_char( "None matched.", ch );
       do_nohelp( ch, "" );
      }
      return;
   }

    bool fFound=FALSE;

  //  type = TRUE;

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	if ( pHelp->level > get_trust( ch ) )
	    continue;
        if ( !str_cmp( pHelp->keyword, argument ) ) {
            fFound=TRUE;
	    output = new_buf();

	    if ( pHelp->name == NULL
	    ||   !str_cmp(pHelp->name,"(null)") )
	    {
		add_buf(output,pHelp->keyword);
		add_buf(output,"\n\r");
	    }

	    if (strlen_color(pHelp->name) % 2 == 0)
		width = 74;
	    else
		width = 75;

	    sprintf(buf,"{R| {W%s {R|{D",pHelp->name);
	    for (i = 0; strlen_color(buf) < (width+2); i++)
	    {
		strcat(buf,"-");
		sprintf(final,"-%s", buf);
		strcpy(buf,final);
	    }
	    sprintf(final," {D%s \n\r", buf);
	    add_buf(output,final);

	    buf[0]   = '\0';
	    final[0] = '\0';

	    for (rdesc = pHelp->text; *rdesc; rdesc++)
	    {
		if (*rdesc != '{' && *rdesc != '\n')
		{ 
		    sprintf(buf, "%c", rdesc[0]);
		    strcat(final,buf);
		}
		else if (*rdesc != '\n')
		{
		    sprintf(buf,"%c%c", rdesc[0], rdesc[1]);
		    strcat(final,buf);
		    rdesc++;
		}
		if (*rdesc == '\n' && *(rdesc + 1))
		{
		    sprintf(buf,"| {w%s {D|\n\r",
			end_string(final,width));
		    add_buf(output,buf);
		    buf[0]   = '\0';
		    final[0] = '\0';
		    rdesc++;
		}
	    }

	    if ( width == 74 )
		add_buf( output, " ----------------------------------------------------------------------------{x \n\r" );
	    else
		add_buf( output, " -----------------------------------------------------------------------------{x \n\r" );

	    page_to_char(output->string,ch);
            if ( IS_IMMORTAL(ch) ) page_to_char(pHelp->immtext,ch);
	    free_buf(output);
	    return;
        }
    }

    if ( fFound ) return;

    argall[0] = '\0';

    while (argument[0] != '\0' )
    {
	argument = one_argument(argument,argone);
	if (argall[0] != '\0')
	    strcat(argall," ");
	strcat(argall,argone);
    }


    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	if ( pHelp->level > get_trust( ch ) )
	    continue;

	if ( is_name( argall, pHelp->keyword ) )
	{
	    output = new_buf();

	    if ( pHelp->name == NULL
	    ||   !str_cmp(pHelp->name,"(null)") )
	    {
		add_buf(output,pHelp->keyword);
		add_buf(output,"\n\r");
	    }

	    if (strlen_color(pHelp->name) % 2 == 0)
		width = 74;
	    else
		width = 75;

	    sprintf(buf,"{R| {W%s {R|{D",pHelp->name);
	    for (i = 0; strlen_color(buf) < (width+2); i++)
	    {
		strcat(buf,"-");
		sprintf(final,"-%s", buf);
		strcpy(buf,final);
	    }
	    sprintf(final," {D%s \n\r", buf);
	    add_buf(output,final);

	    buf[0]   = '\0';
	    final[0] = '\0';

	    for (rdesc = pHelp->text; *rdesc; rdesc++)
	    {
		if (*rdesc != '{' && *rdesc != '\n')
		{ 
		    sprintf(buf, "%c", rdesc[0]);
		    strcat(final,buf);
		}
		else if (*rdesc != '\n')
		{
		    sprintf(buf,"%c%c", rdesc[0], rdesc[1]);
		    strcat(final,buf);
		    rdesc++;
		}
		if (*rdesc == '\n' && *(rdesc + 1))
		{
		    sprintf(buf,"| {w%s {D|\n\r",
			end_string(final,width));
		    add_buf(output,buf);
		    buf[0]   = '\0';
		    final[0] = '\0';
		    rdesc++;
		}
	    }

	    if ( width == 74 )
		add_buf( output, " ----------------------------------------------------------------------------{x \n\r" );
	    else
		add_buf( output, " -----------------------------------------------------------------------------{x \n\r" );

	    page_to_char(output->string,ch);
            if ( IS_IMMORTAL(ch) ) page_to_char(pHelp->immtext,ch);
	    free_buf(output);
	    return;
	}
    }

    send_to_char( "Help file not found by precise or partial search.\n\rIf you can't find what you're looking for,\n\r use the `nohelp` command to request it.\n\r", ch );
    char beef[MAX_STRING_LENGTH];
    sprintf( beef, "#%s", argument );
    do_help( ch, argument );
    return;
}

void do_motd(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"motd");
    return;
}

void do_imotd(CHAR_DATA *ch, char *argument)
{  
    do_help(ch,"imotd");
    return;
}

void do_rules(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"rules");
    return;
}

void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "diku" );
    return;
}

char * display_who_custom( CHAR_DATA *ch, CHAR_DATA *wch )
{
    static char buf2	[MAX_STRING_LENGTH];
    char buf	[MAX_STRING_LENGTH];
    bool same	= FALSE;

    if ( ch == NULL || wch == NULL )
	return "NULL";

    if ( wch == ch )
	same = TRUE;

    sprintf(buf2, "%s", ch->pcdata->who_output);

    if ( ch->pcdata->who_output == NULL || buf2[0] == '\0' )
    {
	send_to_char("Null who output.\n\r",ch);
	return ("Null");
    }

    sprintf(buf, "%s", IS_SET(wch->comm,COMM_AFK) ? "[{yA{YF{yK{x]" : "");
    str_replace_who(buf2, "%a", buf);

    sprintf(buf, "%s", wch->pcdata->pretitle);
    str_replace_who(buf2, "%b", buf);

    sprintf(buf, "%s", wch->in_room->vnum != ROOM_VNUM_CORNER ? "" :
		       !IS_IMMORTAL(wch) ? "({RCorner{x)" : "");
    str_replace_who(buf2, "%B", buf);

    sprintf(buf, "%s", clan_table[wch->clan].who_name);
    str_replace_who(buf2, "%c", buf);

    if (!same && wch->pcdata->who_descr[0] != '\0')
	sprintf(buf, "---");
    else
	sprintf(buf, "%s", class_table[wch->class].who_name);
    str_replace_who(buf2, "%C", buf);

    if ( wch->pcdata->match == NULL )
    {
	buf[0] = '\0';
    } else {
	sprintf(buf, "[{G%d{x][{GTeam %d{x]", wch->pcdata->match->number,
	    wch->pcdata->team+1);
    }
    str_replace_who(buf2, "%A", buf);

    sprintf(buf, "%s", wch->pcdata->dtimer ? "{R(Killed){x{x" : "");
    str_replace_who(buf2, "%d", buf);

    sprintf(buf, "%s", wch->ghost_level ? "(G)" : "");
    str_replace_who(buf2, "%g", buf);

    sprintf(buf, "%s", wch->incog_level ? "(I)" : "");
    str_replace_who(buf2, "%i", buf);

    if (!same && wch->pcdata->who_descr[0] != '\0')
	sprintf(buf, "---");
    else
	sprintf(buf, "%3d", wch->level);
    str_replace_who(buf2, "%l", buf);

    sprintf(buf, "%s", wch->pcdata->clan_rank == MAX_CRNK-1 ? "{G*{x" : "");
    str_replace_who(buf2, "%L", buf);

    sprintf(buf, "%s", wch->name);
    str_replace_who(buf2, "%n", buf);

    sprintf(buf, "%s", can_pk(ch,wch) ? "{R*{x" : "");
    str_replace_who(buf2, "%p", buf);

    sprintf(buf, "%s", wch->pcdata->pktimer <= 0 ? "" :
	wch->pcdata->attacker ? "({WHunter{x)" :
	"({WHunted{x) ");
    str_replace_who(buf2, "%P", buf);

    sprintf(buf, "%s", IS_SET(wch->comm, COMM_QUIET) ? "({GQuiet{x)" : "");
    str_replace_who(buf2, "%q", buf);

    sprintf(buf, "%s", wch->pcdata->on_quest ? "{G(Q){x" : "");
    str_replace_who(buf2, "%Q", buf);

    if (!same && wch->pcdata->who_descr[0] != '\0')
	sprintf(buf, "------");
    else
	sprintf(buf, "%s", race_table[wch->race].who_name);
    str_replace_who(buf2, "%r", buf);

    sprintf(buf, "\n\r");
    str_replace_who(buf2, "%R", buf);

    sprintf(buf, "%s{x", wch->sex == 1 ? "{CM" : wch->sex == 2 ? "{MF" : "{WN");
    str_replace_who(buf2, "%s", buf);

    sprintf(buf, "%s",
	wch->pcdata->who_descr[0] == '\0' ? "" : wch->pcdata->who_descr);
    str_replace_who(buf2, "%S", buf);

    sprintf(buf, "%s", IS_SET(wch->act, PLR_TWIT) ? "({RTwit{x)" : "");
    str_replace_who(buf2, "%t", buf);

    sprintf(buf, "%s", wch->pcdata->title);
    str_replace_who(buf2, "%T", buf);

    sprintf(buf, "%s", wch->invis_level ? "(W)" : "");
    str_replace_who(buf2, "%W", buf);

    return buf2;
}

char * display_who_default( CHAR_DATA *ch, CHAR_DATA *wch, bool fAll )
{
    static char buf[MAX_STRING_LENGTH];
    char string[MAX_STRING_LENGTH];
    char arena[30], level[7], wizi[20], incog[20], ghost[20], quest[20];

    switch ( wch->level )
    {
	default:		sprintf( level, "{B%3d ", wch->level );	break;
	case MAX_LEVEL:		sprintf( level, "{BIMP " );		break;
	case MAX_LEVEL - 1 :	sprintf( level, "{BCRE " );		break;
	case MAX_LEVEL - 2 :	sprintf( level, "{BSUP " );		break;
	case MAX_LEVEL - 3 :	sprintf( level, "{BDEI " );		break;
	case MAX_LEVEL - 4 :	sprintf( level, "{BGOD " );		break;
	case MAX_LEVEL - 5 :	sprintf( level, "{BIMM " );		break;
	case MAX_LEVEL - 6 :	sprintf( level, "{BDEM " );		break;
	case MAX_LEVEL - 7 :	sprintf( level, "{BKNI " );		break;
	case MAX_LEVEL - 8 :	sprintf( level, "{BSQU " );		break;
	case MAX_LEVEL - 9 :	sprintf( level, "{BHRO " );		break;
    }

    if ( can_pk( ch, wch ) )
	level[1] = 'R';

    if ( wch->pcdata->who_descr[0] != '\0' && !fAll )
	sprintf( string, "%s", wch->pcdata->who_descr );
    else
    {
	sprintf( string, "%3s{x%s %s ", level,
	    race_table[wch->race].who_name,
	    class_table[wch->class].who_name );

	if ( wch->sex == SEX_MALE )
	    strcat( string, "{CM" );

	else if ( wch->sex == SEX_FEMALE )
	    strcat( string, "{MF" );

	else
	    strcat( string, "{WN" );
    }

    if ( wch->pcdata->match == NULL )
	arena[0] = '\0';
    else
	sprintf( arena, "[{g({G%d{g) Team {G%d{x] ",
	    wch->pcdata->match->number, wch->pcdata->team+1 );

    if ( wch->invis_level )
	sprintf( wizi, "(W:{B%d{x) ", wch->invis_level );
    else
	wizi[0] = '\0';

    if ( wch->incog_level )
	sprintf( incog, "(I:{B%d{x) ", wch->incog_level );
    else
	incog[0] = '\0';

    if ( wch->ghost_level )
	sprintf( ghost, "(G:{B%d{x) ", wch->ghost_level );
    else
	ghost[0] = '\0';

    if ( wch->pcdata->on_quest )
	sprintf( quest, "{GQ{x" );
    else if ( !can_pk( ch, wch ) )
	sprintf( quest, "{x " );
    else
	sprintf( quest, "{R*{x" );

    sprintf( buf, "{w-{{%s{w}-%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
	string, quest,
        clan_table[wch->clan].who_name,
	wch->pcdata->clan_rank == MAX_CRNK-1 ? "{G*{x" : " ",
	wch->pcdata->pktimer <= 0 ? "" :
	wch->pcdata->attacker ? "( {RHunter{x ) " : "( {RHunted{x ) ",
	arena, wizi, incog, ghost,
	IS_SET( wch->comm, COMM_AFK ) ? "[{BAFK{x] " : "",
	IS_SET( wch->comm, COMM_QUIET ) ? "[{GQuiet{x] " : "",
	wch->pcdata->dtimer > 0 ? "[{BDeceased{x] " : "",
	wch->in_room == NULL ? "" :
	wch->in_room->vnum == ROOM_VNUM_CORNER ? "[{RCORNER{x] " : "",
	wch->pcdata->pretitle,
	wch->name,
	wch->pcdata->title );

    return buf;
}

void whois_finger( BUFFER *final, CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf3[MAX_STRING_LENGTH];
    char buf2[12];
    bool whoString = FALSE;

    switch(victim->level)
    {
        case MAX_LEVEL:
            sprintf(buf2, "{q({sIMP{q)");
            break;
        case MAX_LEVEL - 1:
            sprintf(buf2, "{q({sCRE{q)");
            break;
        case MAX_LEVEL - 2:
            sprintf(buf2, "{q({sSUP{q)");
            break;
        case MAX_LEVEL - 3:
            sprintf(buf2, "{q({sDEI{q)");
            break;
        case MAX_LEVEL - 4:
            sprintf(buf2, "{q({sGOD{q)");
            break;
        case MAX_LEVEL - 5:
            sprintf(buf2, "{q({sIMM{q)");
            break;
        case MAX_LEVEL - 6:
            sprintf(buf2, "{q({sDEM{q)");
            break;
        case MAX_LEVEL - 7:
            sprintf(buf2, "{q({sKNI{q)");
            break;
        case MAX_LEVEL - 8:
	    sprintf(buf2, "{q({sSQU{q)");
            break;
        case MAX_LEVEL - 9:
	    sprintf(buf2, "{q({sHRO{q)");
            break;
	default:
	    sprintf(buf2, "     ");
    }

    add_buf( final, "{s ================================================================\n\r" );

    sprintf ( buf3, "{s|{W %s%s%s",
	victim->pcdata ? victim->pcdata->pretitle : "",
	victim->name,
	victim->pcdata ? victim->pcdata->title : "");
	
    add_buf ( final, end_string( buf3, 65 ) );

    add_buf( final, "{s|\n\r|================================================================|\n\r" );

    if (victim->pcdata && victim->pcdata->who_descr[0] != '\0')
    {
        if ( (!IS_IMMORTAL(ch)
        &&    victim != ch)
        ||   !can_over_ride(ch,victim,TRUE) )
	    whoString = TRUE;

	sprintf( buf3, "{s| {tCustom who-title: %-16s                             {s|\n\r",
		victim->pcdata->who_descr );
	add_buf( final, buf3 );
	add_buf( final, "{s|================================================================|\n\r" );
    }

    if ( ((victim->ghost_level >= LEVEL_HERO)&&(ch->level >= victim->level))
    ||   (victim->incog_level >= LEVEL_HERO&&(ch->level >= victim->level))
    ||   (victim->invis_level >= LEVEL_HERO&&(ch->level >= victim->level))
    ||   IS_SET(victim->comm, COMM_AFK)
    ||   IS_SET(victim->comm, COMM_QUIET)
    ||   IS_SET(victim->act, PLR_TWIT) )
    {
	sprintf( buf3, "{s| {tFlags:{x %s%s%s%s%s%s",
	    (victim->ghost_level >= LEVEL_HERO&&(ch->level >= victim->level)) ? "{tG{Dh{to{Ds{tt " : "",
	    (victim->incog_level >= LEVEL_HERO&&(ch->level >= victim->level)) ? "{mIncog " : "",
	    (victim->invis_level >= LEVEL_HERO&&(ch->level >= victim->level)) ? "{CW{ci{Cz{ci{Cn{cv{Ci{cs " : "",
	    IS_SET(victim->comm, COMM_AFK) ? "{qAFK " : "",
	    IS_SET(victim->comm, COMM_QUIET) ? "{GQuiet " : "",
	    IS_SET(victim->act,PLR_TWIT) ? "{RTWIT" : "" );
	add_buf( final, end_string( buf3, 65 ) );
		
	add_buf( final, "{s|\n\r|================================================================|\n\r" );
    }
	
    if ( whoString )
        sprintf( buf3, "{s| {tLevel:{q    ---                 {s|    {RPK Stats    {s|  {RArena Stats  {s|\n\r" );
    else
	sprintf( buf3, "{s| {tLevel:{q    %-4d %s          {s|    {RPK Stats    {s|  {RArena Stats  {s|\n\r",
	    victim->level, buf2 );
    
    add_buf( final, buf3 );

    if (is_pkill (victim))
    {
	sprintf( buf3, "{s| {tGender:   %-6s              {s| {tKills:   {g%5d {s| {tWins:   {g%5d {s|\n\r",
	    victim->pcdata->true_sex == 1 ? "{CM{cale  " :
	    victim->pcdata->true_sex == 2 ? "{MF{memale" : "{qNone  ",
	    victim->pcdata->pkills, victim->pcdata->arenawins );
	add_buf( final, buf3 );

	sprintf( buf3, "{s| {tRace:     {q%-14s      {s| {tDeaths:  {r%5d {s| {tLosses: {r%5d {s|\n\r",
	    whoString ? "---" : capitalize(race_table[victim->race].name),
	    victim->pcdata->pdeath, victim->pcdata->arenaloss );
	add_buf( final, buf3 );

	sprintf( buf3, "{s| {tClass:    {q%-14s      {s| {tPoints:  {q%5d {s| {tKills:  {g%5d {s|\n\r",
	    whoString ? "---" : capitalize(class_table[victim->class].name),
	    victim->pcdata->pkpoints, victim->pcdata->arenakills );
	add_buf( final, buf3 );

	sprintf( buf3, "{s| {tClan:     %s {s| {tAssists: {q%5d {s| {tDeaths: {r%5d {s|\n\r",
	    end_string( clan_table[victim->clan].color, 19 ),
	    victim->pcdata->assist, victim->pcdata->arenadeath );
	add_buf( final, buf3 );

    } else {
	sprintf( buf3, "{s| {tGender:   %-6s              {s| {q    NON-PK     {s| {tWins:   {g%5d {s|\n\r",
	    victim->pcdata->true_sex == 1 ? "{CM{cale  " :
	    victim->pcdata->true_sex == 2 ? "{MF{memale" : "{qNone  ",
	    victim->pcdata->arenawins );
	add_buf( final, buf3 );

	sprintf( buf3, "{s| {tRace:     {q%-14s      {s| {q    NON-PK     {s| {tLosses: {r%5d {s|\n\r",
	    whoString ? "---" : capitalize(race_table[victim->race].name),
	    victim->pcdata->arenaloss );
	add_buf( final, buf3 );

	sprintf( buf3, "{s| {tClass:    {q%-14s      {s| {q    NON-PK     {s| {tKills:  {g%5d {s|\n\r",
	    whoString ? "---" : capitalize(class_table[victim->class].name),
	    victim->pcdata->arenakills );
	add_buf( final, buf3 );

	sprintf( buf3, "{s| {tClan:     %s {s| {q    NON-PK     {s| {tDeaths: {r%5d {s|\n\r",
	    end_string( clan_table[victim->clan].color, 19 ),
	    victim->pcdata->arenadeath );
	add_buf( final, buf3 );
    }

    sprintf( buf3, "{s| {tRank:{q     %s {s| {tBounty:    {R%10d {DPl{wa{Wti{wn{Dum {s|\n\r",
	end_string( clan_table[victim->clan].crnk[victim->pcdata->clan_rank], 19 ),
	victim->pcdata->bounty );
    add_buf(final, buf3 );
    return;
}

void do_whois( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;

    if ( argument[0] == '\0' )
    {
        send_to_char( "You must provide a name.\n\r", ch );
        return;
    }

    final = new_buf();

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;

        if ( d->connected != CON_PLAYING )
            continue;

        wch = ( d->original != NULL ) ? d->original : d->character;

        if  ( str_prefix( argument, wch->name )
        ||    !can_see( ch, wch ) )
            continue;
	
        found = TRUE;

	whois_finger( final, ch, wch );

	if ( IS_TRUSTED(ch,MAX_LEVEL-1)
	&&   can_see( ch, wch )
	&&   can_over_ride( ch, wch, TRUE )
	&&   wch->in_room != NULL )
        {
	    add_buf( final, "{s|================================================================|\n\r" );
	    sprintf( buf, "{s| {tIn area:   {q%s {s|\n\r",
	        end_string(wch->in_room->area->name, 51));
            add_buf(final,buf);

	    sprintf( buf, "{s| {tIn room:   {q%s {s|\n\r",
	        end_string(wch->in_room->name, 51));
            add_buf(final,buf);

	    sprintf(buf, "{s| {tRoom vnum: {q%-9d                                           {s|\n\r",
                wch->in_room->vnum);
            add_buf(final, buf);
        }

        add_buf( final, "{s ================================================================{x\n\r" );
    }

    if ( !found )
        send_to_char( "No one of that name is playing.\n\r", ch );
    else
        page_to_char( final->string, ch );

    free_buf( final );
}

void do_who( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    DESCRIPTOR_DATA *d;
    char buf		[MAX_STRING_LENGTH];
    bool rgfClass	[maxClass];
    bool rgfRace	[maxRace];
    bool rgfClan	[maxClan];
    bool fClassRestrict	= FALSE;
    bool fClanRestrict	= FALSE;
    bool fClan		= FALSE;
    bool fRaceRestrict	= FALSE;
    //bool fImmortalOnly	= FALSE;
    bool fPkillOnly	= FALSE;
    bool fArenaOnly	= FALSE;
    bool fHuntedOnly	= FALSE;
    bool fAll		= FALSE;
    bool nWho		= FALSE;
    int iLevelLower	= 0;
    int iLevelUpper	= MAX_LEVEL;
    int nNumber		= 0;
    int nMatch		= 0;
    int count		= 0;
    int countimm	= 0;
    int total_players	= 0;
    int wlevel;
    int iClass;
    int iRace;
    int iClan;
    int hour;

    if ( !IS_NPC(ch)
    &&   IS_SET(ch->act, PLR_CUSTOM_WHO) 
    &&   ch->pcdata->who_output != NULL
    &&   ch->pcdata->who_output[0] != '\0' )
	nWho = TRUE;

    for ( iClass = 0; iClass < maxClass; iClass++ )
	rgfClass[iClass] = FALSE;

    for ( iRace = 0; iRace < maxRace; iRace++ )
	rgfRace[iRace] = FALSE;

    for ( iClan = 0; iClan < maxClan; iClan++ )
	rgfClan[iClan] = FALSE;
 
    for ( ; ; )
    {
	char arg[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	    break;
 
	if ( is_number( arg ) )
	{
	    switch ( ++nNumber )
	    {
		case 1: iLevelLower = atoi( arg ); break;
		case 2: iLevelUpper = atoi( arg ); break;
		default:
		    send_to_char( "Only two level numbers allowed.\n\r", ch );
		    return;
	    }
	} else {
//	    if ( !str_prefix(arg,"immortals") )
//		fImmortalOnly = TRUE;
	  /*  else */ if ( !str_prefix(arg,"pkill") )
		fPkillOnly = TRUE;
	    else if ( !str_prefix(arg,"arena") )
		fArenaOnly = TRUE;
	    else if ( !str_prefix(arg,"hunter") || !str_prefix(arg,"hunted") )
		fHuntedOnly = TRUE;
	    else if ( IS_TRUSTED(ch,MAX_LEVEL-1) && !str_prefix(arg,"show") )
		fAll = TRUE;
            else
            {
		iClass = class_lookup(arg);

		if (iClass == -1)
		{
		    iRace = race_lookup(arg);
 
		    if (iRace == -1 || iRace >= maxRace)
		    {
			if (!str_prefix(arg,"clan"))
			    fClan = TRUE;
			else
			{
			    iClan = clan_lookup(arg);

			    if (iClan)
			    {
				fClanRestrict = TRUE;
				rgfClan[iClan] = TRUE;
			    } else {
				send_to_char("That's not a valid race, class, or clan.\n\r",ch);
				return;
			    }
			}
		    } else {
			fRaceRestrict = TRUE;
			rgfRace[iRace] = TRUE;
		    }
		} else {
		    fClassRestrict = TRUE;
		    rgfClass[iClass] = TRUE;
		}
	    }
	}
    }
 
    buf[0]	 = '\0';
    final	 = new_buf();

    for( wlevel = MAX_LEVEL; wlevel > LEVEL_HERO; wlevel-- )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    CHAR_DATA *wch;
 
	    if ( d->connected != CON_PLAYING )
		continue;
 
	    wch = d->original ? d->original : d->character;

	    if ( wch->level != wlevel
	    ||   !can_see(ch,wch)
	    ||   fPkillOnly
	    ||  (fArenaOnly && wch->pcdata->match == NULL )
	    ||  (fHuntedOnly && wch->pcdata->pktimer <= 0)
	    ||   wch->level < iLevelLower
	    ||   wch->level > iLevelUpper
	    ||  (fClassRestrict && !rgfClass[wch->class])
	    ||  (fClassRestrict && wch->pcdata->who_descr[0] != '\0' && !fAll)
	    ||  (fRaceRestrict && !rgfRace[wch->race])
	    ||  (fRaceRestrict && wch->pcdata->who_descr[0] != '\0' && !fAll)
  	    ||  (fClan && !is_clan(wch))
	    ||  (fClanRestrict && !rgfClan[wch->clan])
	    ||  (iLevelLower > 0 && wch->pcdata->who_descr[0] != '\0' && !fAll)
	    ||  (iLevelUpper < MAX_LEVEL && wch->pcdata->who_descr[0] != '\0'
					 && !fAll) )
		continue;
 
	    countimm++;
	    nMatch++;

	    if (countimm == 1)
		add_buf( final, "{w< {DCurrent {gI{Gm{gm{Go{gr{Gt{ga{Gl{gs {w>\n\r" );

	    if ( nWho )
		add_buf( final, display_who_custom( ch, wch ) );
	    else
		add_buf( final, display_who_default( ch, wch, fAll ) );

	    add_buf( final, "\n\r" );
	}
    }

    buf[0] = '\0';

    if ( !fAll )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    CHAR_DATA *wch;

	    if ( d->connected != CON_PLAYING )
		continue;

	    wch = d->original ? d->original : d->character;

	    if ( wch->level >= LEVEL_IMMORTAL
	    ||   wch->pcdata->who_descr[0] == '\0'
	    ||   iLevelLower > 0
	    ||   iLevelUpper < MAX_LEVEL )
		continue;

	    if ( !can_see( ch, wch ) )
	    {
		total_players++;
		continue;
	    }

	    count++;

	    if ( (fHuntedOnly && wch->pcdata->pktimer <= 0)
	    ||   (fPkillOnly  && !can_pk(ch,wch))
	    ||   (fArenaOnly  && wch->pcdata->match == NULL)
	    ||   fClassRestrict
	    ||   fRaceRestrict
	    ||   (fClan && !is_clan(wch))
	    ||   (fClanRestrict && !rgfClan[wch->clan]) )
		continue;

	    nMatch++;

	    if ( nMatch - countimm == 1 )
	    {
		if ( countimm != 0 )
		    add_buf(final,"\n\r");
		add_buf( final, "< {DCurrent {gM{Go{gr{Gt{ga{Gl{gs {w>\n\r" );
	    }

	    if ( nWho )
		add_buf( final, display_who_custom( ch, wch ) );
	    else
		add_buf( final, display_who_default( ch, wch, fAll ) );
	    add_buf( final, "\n\r" );
	}
    }

    for ( wlevel = LEVEL_HERO; wlevel > 0; wlevel-- )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    CHAR_DATA *wch;
 
	    if ( d->connected != CON_PLAYING )
		continue;

	    wch = d->original ? d->original : d->character;

	    if ( wch->level != wlevel 
	    ||   (wch->pcdata->who_descr[0] != '\0' && !fAll) )
		continue;

	    if ( !can_see( ch, wch ) )
	    {
		total_players++;
		continue;
	    }
 
	    count++;

	    if ( (fPkillOnly && !can_pk(ch,wch))
	    ||   (fArenaOnly && wch->pcdata->match == NULL)
	    ||   wch->level < iLevelLower
	    ||   wch->level > iLevelUpper
	    ||   (fClassRestrict && !rgfClass[wch->class])
	    ||   (fRaceRestrict && !rgfRace[wch->race])
	    ||   (fClan && !is_clan(wch))
	    ||   (fClanRestrict && !rgfClan[wch->clan])
	    ||   (fHuntedOnly && wch->pcdata->pktimer <= 0) )
		continue;

	    nMatch++;

	    if ( nMatch - countimm == 1 )
	    {
		if ( countimm != 0 )
		    add_buf(final,"\n\r");
		add_buf( final, "< {DCurrent {gM{Go{gr{Gt{ga{Gl{gs {w>\n\r" );
	    }

	    if ( nWho )
		add_buf( final, display_who_custom( ch, wch ) );
	    else
		add_buf( final, display_who_default( ch, wch, fAll ) );
	    add_buf( final, "\n\r" );
	}
    }

    count += countimm;
    total_players += count;

    hour = (int) (struct tm *)localtime(&current_time)->tm_hour;

    if (hour < 12)
    {
	if (is_pm)
	{
	    is_pm = FALSE;
	    mud_stat.most_today = 0;
	    mud_stat.changed = TRUE;
	    expire_notes();
	}
    } else {
	is_pm = TRUE;
    }

    if ( total_players > mud_stat.most_today )
    {
	mud_stat.most_today = total_players;
	mud_stat.changed = TRUE;
    }

    if ( mud_stat.most_today > mud_stat.max_ever )
    {
	mud_stat.max_ever = mud_stat.most_today;
	mud_stat.changed = TRUE;
    }

    if ( nMatch <= 0 )
    {
	send_to_char( "No players were located that matched the specifications.\n\r", ch );
	free_buf(final);
	return;
    }

    if ( nMatch != count )
    {
	sprintf(buf,"\n\r{BMatches Located{w: {W%d\n\r", nMatch);
	add_buf(final,buf);
    } else {
	sprintf(buf,"\n\r{BPlayers found{w: {W%d   {BMost on today{w: {W%d   {BMost on ever{w: {W%d",
	    count, mud_stat.most_today, mud_stat.max_ever );
	add_buf(final,buf);

	sprintf(buf,"\n\r{BConnections Since Startup{w: {W%d   {BTotal Players Connected{w: {W%d\n\r",
	    mud_stat.connect_since_reboot, total_players );
	add_buf(final,buf);
    }

    if ( mud_stat.exp_mod[0] != 1 || mud_stat.exp_mod[1] != 1 )
    {
        if ( mud_stat.exp_mod[1] == 1 )
            sprintf( buf, "{BThe world is experiencing an {RXP{B multiplier of {c%d{B.{x\n\r",
                mud_stat.exp_mod[0] );
        else
            sprintf( buf, "{BThe world is experiencing an {RXP{B multiplier of {c%d{B/{c%d{B.{x\n\r",
                mud_stat.exp_mod[0], mud_stat.exp_mod[1] );
        add_buf( final, buf );
    }
    
    if ( global_quest == QUEST_ON )
	add_buf( final, "{wThe {Gquest {wflag is on.{x\n\r" );

    else if ( global_quest == QUEST_LOCK )
	add_buf( final, "{wThe {Gquest {wis locked.{x\n\r" );

    page_to_char( final->string, ch );
    free_buf( final );
}

void do_count ( CHAR_DATA *ch, char *argument )
{
    int count;
    int hour;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    count = 0;
    hour = (int) (struct tm *)localtime(&current_time)->tm_hour;
    if (hour < 12)
    {
	if (is_pm)
	{
	    is_pm = FALSE;
	    mud_stat.most_today = 0;
	    mud_stat.changed = TRUE;
	    expire_notes();
	}
    }
    else
	is_pm = TRUE;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
	    count++;

    if (mud_stat.most_today == count)
        sprintf(buf,"{BThere are {W%d {Bcharacters on, the most so far today.{x\n\r",
	    count);
    else
	sprintf(buf,"{BThere are {W%d {Bcharacters on, the most on today was {W%d{x.\n\r",
	    count,mud_stat.most_today);

    send_to_char(buf,ch);
}

void do_equipment( CHAR_DATA *ch, char *argument )
{
    BUFFER *final = new_buf();
    OBJ_DATA *obj;
    int iWear;
    int oWear;

    add_buf( final, "You are using:\n\r" );

    for ( oWear = 0; oWear < MAX_WEAR; oWear++ )
    {
	iWear = where_order[oWear];

	if ( (iWear == WEAR_HOLD
	&&    !IS_NPC(ch)
	&&    get_eq_char(ch,WEAR_HOLD) == NULL
	&&    get_eq_char(ch,WEAR_SECONDARY) != NULL)
	||   (iWear == WEAR_SECONDARY
	&&    !IS_NPC(ch)
	&&    get_eq_char(ch,WEAR_SECONDARY) == NULL
	&&    get_eq_char(ch,WEAR_HOLD) != NULL)
	||   (iWear >= WEAR_SOUL1 && iWear <= WEAR_SOUL15
	&&    get_eq_char(ch,iWear) == NULL)
	||   (get_skill(ch,gsn_quickdraw) == 0
	&&    (iWear == WEAR_SHEATH_1 || iWear == WEAR_SHEATH_2))
	||   (iWear == WEAR_CLAN && get_eq_char(ch,WEAR_CLAN) == NULL) )
	    continue;

	if ( (!IS_NPC(ch)
	&&   (get_eq_char(ch,WEAR_SHIELD) != NULL
	||    get_eq_char(ch,WEAR_SECONDARY) != NULL)) )
	{
	    if ( get_skill(ch,gsn_shield_levitate) <= 0 )
	    {
		if ( (iWear == WEAR_SHIELD
		&&   get_eq_char(ch,WEAR_SHIELD) == NULL
		&&   get_eq_char(ch,WEAR_SECONDARY) != NULL)
		||   (iWear == WEAR_SECONDARY
		&&   get_eq_char(ch,WEAR_SECONDARY) == NULL
		&&   get_eq_char(ch,WEAR_SHIELD) != NULL) )
		    continue;
	    }
	}

	add_buf( final, where_name[iWear] );

	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	{
	    add_buf( final, "{R|{r--{R>{cNothing{R<{r--{R|{x\n\r" );
	    continue;
	}

	if ( can_see_obj( ch, obj ) )
	{
	    add_buf( final, format_obj_to_char( obj, ch, TRUE ) );
	    add_buf( final, "\n\r" );
	}
	else
	{
	    add_buf( final, "something.\n\r" );
	}
    }
    page_to_char( final->string, ch );
    free_buf( final );
    return;
}

void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1, *obj2;
    int value1, value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (arg2[0] == '\0')
    {
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
	    if (obj2->wear_loc != WEAR_NONE
	    &&  can_see_obj(ch,obj2)
	    &&  obj1->item_type == obj2->item_type
	    &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if (obj2 == NULL)
	{
	    send_to_char("You aren't wearing anything comparable.\n\r",ch);
	    return;
	}
    } 

    else if ( (obj2 = get_obj_carry(ch,arg2) ) == NULL )
    {
	send_to_char("You do not have that item.\n\r",ch);
	return;
    }

    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	    break;

	case ITEM_WEAPON:
	    value1 = (1 + obj1->value[2]) * obj1->value[1];
	    value2 = (1 + obj2->value[2]) * obj2->value[1];
	    break;
	}
    }

    if ( msg == NULL )
    {
	     if ( value1 == value2 ) msg = "$p and $P look about the same.";
	else if ( value1  > value2 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act( msg, ch, obj1, obj2, TO_CHAR,POS_RESTING);
    return;
}

void do_where( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found = FALSE;

    one_argument( argument, arg );
/*
    if ( IS_AFFECTED( ch, AFF_BLIND ) )
    {
	send_to_char( "You have no idea where you are.\n\r", ch );
	return;
    }
*/
    final = new_buf( );

    if ( !str_cmp( arg, "trainer" ) || !str_cmp( arg, "practitioner" ) ) {
     CHAR_DATA *a;
     for ( a=char_list; a!=NULL; a=a->next)
      if ( IS_NPC(a) && a->in_room
        && ( IS_SET(a->act,ACT_TRAIN) || IS_SET(a->act,ACT_PRACTICE) )
      ) {
      if ( IS_SET(a->act,ACT_TRAIN) ) {
       sprintf( buf, "%s trains attributes in %s\n\r", a->pIndexData->short_descr,
        a->in_room->name );
       send_to_char( buf, ch );
      }
      if ( IS_SET(a->act,ACT_PRACTICE) ) {
       sprintf( buf, "%s teaches skills in %s\n\r", a->pIndexData->short_descr,
        a->in_room->name );
       send_to_char( buf, ch );
      }
     }
     return;
    }

    if ( arg[0] == '\0' )
    {
        sprintf( buf, "{xPlayers near you in {^%s{x:\n\r", ch->in_room->area->name );
	add_buf( final, buf );

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC( victim )
	    &&   victim->in_room != NULL
	    &&   !IS_SET( victim->in_room->room_flags, ROOM_NOWHERE )
	    &&   victim->in_room->area == ch->in_room->area
	    &&   get_trust( ch ) >= victim->ghost_level
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;

		if ( !can_pk( ch, victim ) )
		{
		    sprintf( buf, "     %-24s %s\n\r",
			victim->name, victim->in_room->name );
		    add_buf( final, buf );
		} else {
		    sprintf( buf, "{w({RPK{w){x %-24s %s\n\r",
		        victim->name, victim->in_room->name );
		    add_buf( final, buf );
		}
	    }
	}

	if ( !found )
	    add_buf( final, "None\n\r" );
	page_to_char( final->string, ch );
    }

    else if ( !str_prefix( argument, "PKILL" ) )
    {
        sprintf( buf, "{RP{rk{Ri{rl{Rl{x targets near you in {^%s{x:\n\r", ch->in_room->area->name );
	add_buf( final, buf );

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   ( victim = d->character ) != NULL
	    &&   !IS_NPC( victim )
	    &&   victim->in_room != NULL
	    &&   !IS_SET( victim->in_room->room_flags, ROOM_NOWHERE )
	    &&   victim->in_room->area == ch->in_room->area
	    &&   get_trust( ch ) >= victim->ghost_level
	    &&   can_see( ch, victim )
	    &&   can_pk( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "{w({RPK{w){x %-24s %s\n\r",
		    victim->name, victim->in_room->name );
		add_buf( final, buf );
	    }
	}

	if ( !found )
	    add_buf( final, "{wNo ({RPK{w) targets detected.{x\n\r" );

	page_to_char( final->string, ch );
    }

    else
    {
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   !IS_AFFECTED( victim, AFF_HIDE )
	    &&   !IS_SET( victim->in_room->room_flags, ROOM_NOWHERE )
	    &&   !IS_SET( victim->act, ACT_NO_WHERE )
	    &&   get_trust( ch ) >= victim->ghost_level
	    &&   can_see( ch, victim )
	    &&   is_name( arg, victim->name ) )
	    {
		found = TRUE;
		sprintf( buf, "%-29s %s\n\r",
		    PERS( victim, ch ), victim->in_room->name );
		send_to_char( buf, ch );
		break;
	    }
	}

	if ( !found )
	    act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR, POS_RESTING );
    }

    free_buf( final );
    return;
}

void do_track( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *in_room;
    sh_int track_vnum;
    int door, move, chance, track;

    one_argument( argument, arg );

    if ((chance = get_skill(ch,gsn_track)) == 0)
    {
	send_to_char( "You don't know how to track.\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Track whom?\n\r", ch );
	return;
    }

    in_room = ch->in_room;
    track_vnum = in_room->vnum;
    move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)];
    if ( ch->move < move )
    {
	send_to_char( "You are too exhausted.\n\r", ch );
	return;
    }

    if (number_percent() < (100-chance))
    {
	sprintf(buf, "You can find no recent tracks for %s.\n\r", arg);
	send_to_char(buf, ch);
	check_improve(ch,gsn_track,FALSE,1);
	WAIT_STATE( ch, 1 );
	ch->move -= move/2;
	return;
    }

    for ( victim = player_list; victim != NULL; victim = victim->pcdata->next_player )
    {
	if ( victim->in_room != NULL
	&&   can_see( ch, victim )
	&&   is_name( arg, victim->name ) )
	{
	    if (victim->in_room->vnum == track_vnum)
	    {
		act( "The tracks end right under $S feet.", ch, NULL, victim, TO_CHAR,POS_RESTING);
		return;
	    }
	    for (track = 0; track < MAX_TRACK; track++)
	    {
		if (victim->track_from[track] == track_vnum)
		{
		    for (door = 0; door < 12; door++)
		    {
			if ( ( pexit = in_room->exit[door] ) != NULL)
			{
			    if (pexit->u1.to_room->vnum == victim->track_to[track])
			    {
				sprintf(buf, "Some tracks lead off to the %s.\n\r", dir_name[door]);
				send_to_char(buf, ch);
				check_improve(ch,gsn_track,TRUE,1);
				WAIT_STATE( ch, 1 );
				ch->move -= move;
				return;
			    }
			}
		    }
		    act("$N seems to have vanished here.", ch, NULL, victim, TO_CHAR,POS_RESTING);
		    check_improve(ch,gsn_track,TRUE,1);
		    WAIT_STATE( ch, 1 );
		    ch->move -= move;
		    return;
		}
	    }
	    act("You can find no recent tracks for $N.", ch, NULL, victim, TO_CHAR,POS_RESTING);
	    check_improve(ch,gsn_track,FALSE,1);
	    WAIT_STATE( ch, 1 );
	    ch->move -= move/2;
	    return;
	}
    }
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
	if ( (victim->in_room != NULL)
	&&   IS_NPC(victim)
	&&   can_see( ch, victim )
	&&   (victim->in_room->area == ch->in_room->area)
	&&   is_name( arg, victim->name ) )
	{
	    if (victim->in_room->vnum == track_vnum)
	    {
		act( "The tracks end right under $S feet.", ch, NULL, victim, TO_CHAR,POS_RESTING);
		return;
	    }
	    for (track = 0; track < MAX_TRACK; track++)
	    {
		if (victim->track_from[track] == track_vnum)
		{
		    for (door = 0; door < 12; door++)
		    {
			if ( ( pexit = in_room->exit[door] ) != NULL)
			{
			    if (pexit->u1.to_room->vnum == victim->track_to[track])
			    {
				sprintf(buf, "Some tracks lead off to the %s.\n\r", dir_name[door]);
				send_to_char(buf, ch);
				check_improve(ch,gsn_track,TRUE,1);
				WAIT_STATE( ch, 1 );
				ch->move -= move;
				return;
			    }
			}
		    }
		    act("$N seems to have vanished here.", ch, NULL, victim, TO_CHAR,POS_RESTING);
		    check_improve(ch,gsn_track,TRUE,1);
		    WAIT_STATE( ch, 1 );
		    ch->move -= move;
		    return;
		}
	    }
	}
    }
    sprintf(buf, "You can find no recent tracks for %s.\n\r", arg);
    send_to_char(buf, ch);
    check_improve(ch,gsn_track,FALSE,1);
    WAIT_STATE( ch, 1 );
    ch->move -= move/2;
    return;
}

void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int diff, vac, cac;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    diff = ((victim->hit / 50) - (ch->hit / 50));
    vac = -(GET_AC(victim,AC_PIERCE)+GET_AC(victim,AC_BASH)+GET_AC(victim,AC_SLASH)+GET_AC(victim,AC_EXOTIC));
    cac = -(GET_AC(ch,AC_PIERCE)+GET_AC(ch,AC_BASH)+GET_AC(ch,AC_SLASH)+GET_AC(ch,AC_EXOTIC));
    diff += (vac - cac);
    diff += (GET_DAMROLL(victim) - GET_DAMROLL(ch));
    diff += (GET_HITROLL(victim) - GET_HITROLL(ch));
    diff += (get_curr_stat(victim,STAT_STR) - get_curr_stat(ch,STAT_STR));

         if ( diff <=  -110 ) msg = "You can kill $N naked and weaponless.";
    else if ( diff <=  -70 )  msg = "$N is no match for you.";
    else if ( diff <=  -20 )  msg = "$N looks like an easy kill.";
    else if ( diff <=  20 )   msg = "The perfect match!";
    else if ( diff <=  70 )   msg = "$N says '{aDo you feel lucky, punk?{x'.";
    else if ( diff <=  110 )  msg = "$N laughs at you mercilessly.";
    else                      msg = "Death will thank you for your gift.";

    act( msg, ch, NULL, victim, TO_CHAR,POS_RESTING);
    return;
}

void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( ( title[0] == '{' && title[1] != '{'
    &&     ( title[2] == '.' || title[2] == ',' || title[2] == '-'
    ||       title[2] == '!' || title[2] == '?' || title[2] == '_'
    ||       title[2] == '\'' ) )
    ||   title[0] == '.' || title[0] == ',' || title[0] == '-'
    ||   title[0] == '!' || title[0] == '?' || title[0] == '_'
    ||   title[0] == '\'' )
	strcpy( buf, title );
    else
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}

void do_title( CHAR_DATA *ch, char *argument )
{
    int value;

    if ( IS_NPC(ch) )
	return;

    if ( ch->pcdata->penalty_time[PENALTY_NOTITLE] != 0 )
	return;

    if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
    && (!IS_IMMORTAL(ch)))
    {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }

    if ( strlen_color(argument) + strlen_color(ch->pcdata->pretitle) +
	 strlen( ch->name ) > 50 )
    {
	send_to_char("Your pretitle + name + title may not exceed 50 characters (excluding color).\n\r",ch);
	return;
    }

    value = strlen(argument);
    argument[value] = '{';
    argument[value+1] = 'x';
    argument[value+2] = '\0';

    smash_tilde( argument );

    argument = channel_parse( ch, argument, TRUE );

    set_title( ch, argument );
    send_to_char( "Ok.\n\r", ch );
}



void do_description( CHAR_DATA *ch, char *argument )
{
    char buf[4*MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
	send_to_char(	"Syntax: description show          [shows you your description]\n\r"
			"        description edit          [use string editor to edit]\n\r"
			"        description + <new line>  [add a new line]\n\r"
			"        description -             [removes last line]\n\r", ch );
	return;
    }

    else if ( !str_prefix( argument, "show" ) )
    {
	send_to_char( "Your description reads:\n\r\n\r", ch );
	page_to_char( ch->description, ch );
	return;
    }

    else if ( !str_prefix( argument, "edit" ) || !str_prefix( argument, "write" ) )
    {
	string_append( ch, &ch->description );
	return;
    }

    else if ( argument[0] == '-' )
    {
	bool found = FALSE;
	int len;
 
	if ( ch->description == NULL || ch->description[0] == '\0' )
	{
	    send_to_char( "No lines left to remove.\n\r", ch );
	    return;
	}
	
	strcpy( buf, ch->description );

	for ( len = strlen( buf ); len > 0; len-- )
	{
	    if ( buf[len] == '\r' )
	    {
		if ( !found )  /* back it up */
		{
		    if ( len > 0 )
			len--;
		    found = TRUE;
		}

		else /* found the second one */
		{
		    buf[len + 1] = '\0';
		    free_string( ch->description );
		    ch->description = str_dup( buf );

		    send_to_char( "Your description is:\n\r\n\r", ch );
		    page_to_char( ch->description ? ch->description : "(None).\n\r", ch );
		    return;
		}
	    }
	}

	free_string( ch->description );
	ch->description = str_dup( "" );
	send_to_char( "Description cleared.\n\r", ch );
	return;
    }

    else if ( argument[0] == '+' )
    {
	buf[0] = '\0';
	if ( ch->description != NULL )
	    strcpy( buf, ch->description );

	argument++;
	while ( isspace( *argument ) )
	    argument++;

	if ( strlen( buf ) + strlen( argument ) >= 4*MAX_STRING_LENGTH-10 )
	{
	    send_to_char( "Description too long.\n\r", ch );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\n\r" );
	free_string( ch->description );
	ch->description = str_dup( buf );
	send_to_char( "Your description is:\n\r\n\r", ch );
	page_to_char( ch->description ? ch->description : "(None).\n\r", ch );
	return;
    }

    do_description( ch, "" );
    return;
}

void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf, "I have %d/%d hp %d/%d mana %d/%d mv %ld xp.",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana,
	ch->move, ch->max_move, ch->exp );
    do_say( ch, buf );
}

void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
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
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */

    pwdnew = crypt( arg2, ch->name );

    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );

    if ( ch->level > 4 )
        save_char_obj( ch, 0 );

    send_to_char( "Ok.\n\r", ch );
    return;
}

void set_pretitle( CHAR_DATA *ch, char *pretitle )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
        bug( "Set_pretitle: NPC.", 0 );
        return;
    }
    else
    {
        strcpy( buf, pretitle );
    }

    free_string( ch->pcdata->pretitle );
    ch->pcdata->pretitle = str_dup( buf );
    return;
}

void do_pretitle( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *stringer;
    OBJ_DATA *diamond = NULL;
    bool found = FALSE;
    int value;

    smash_tilde( argument );

    if (IS_NPC(ch))
    {
	send_to_char("Mobiles don't need pretitles doof!\n\r",ch);
	return;
    }

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

    if (!str_cmp(argument,"confirm"))
    {
	if (!ch->pcdata->confirm_pretitle)
	{
	    send_to_char("You have no pretitle to confirm!\n\r",ch);
	    return;
	}
	send_to_char("You have confirmed your new pretitle.\n\r",ch);
	ch->pcdata->confirm_pretitle = FALSE;
	return;
    }

    if (!ch->pcdata->confirm_pretitle)
    {
	found = FALSE;

	for ( diamond = ch->carrying; diamond != NULL; diamond = diamond->next_content )
	{
	    if ( diamond->pIndexData->vnum == 1209 )
	    {
		found = TRUE;
		break;
	    }
	}

	if (!found)
	{
	    act("$N says '{SI charge one {rp{Rr{re{Rt{ri{Rt{rl{Re {Wd{wi{Wa{wm{Wo{wn{Wd {Sfor my services.{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
	    act("$N says '{SCome back when you have one ready and waiting!{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
	    return;
	}
    }

    if (argument[0] == '\0')
    {
	act("$N says '{SThe correct syntax is \"pretitle <string you want>\"{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
	return;
    }

    if ( strlen_color(argument) + strlen_color(ch->pcdata->title) +
	 strlen( ch->name ) > 50 )
    {
	send_to_char("Your pretitle + name + title may not exceed 50 characters (excluding color).\n\r",ch);
	return;
    }

    if (!ch->pcdata->confirm_pretitle)
    {
	obj_from_char(diamond);
	extract_obj(diamond);
    }

    value = strlen(argument);

    if (argument[value-1] == '{' && argument[value-2] != '{')
	argument[value] = 'x';

    set_pretitle( ch, argument );
    ch->pcdata->confirm_pretitle = TRUE;

    act("$N says '{SYour pretitle now reads \" $t$n {S\".{x'",ch,ch->pcdata->pretitle,stringer,TO_CHAR,POS_RESTING);
    act("$N says '{SIf you fucked up just use pretitle again.{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
    act("$N says '{SYou have until you confirm to change this for free if you like.{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
    act("$N says '{SType \"pretitle confirm\" to confirm your new pretitle.{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
    return;
}

void do_bounty( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
	
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Place a bounty on who's head?\n\r"
		      "Syntax:  Bounty <victim> <amount>\n\r", ch );
	return;
    }
	
    if ( ( victim = get_pc_world( ch, arg1 ) ) == NULL)
    {
	send_to_char( "They are currently not logged in!\n\r", ch );
	return;
    }
  
    if (ch == victim)
    {
	send_to_char("You may not bounty yourself.\n\r",ch);
	return;
    }

    if ( is_number( arg2 ) )
    {
	int amount;
	amount   = atoi(arg2);

	if (ch->platinum < amount)
	{
	    send_to_char( "You don't have that much platinum!\n\r", ch );
	    return;
	}

	if (amount < 1 )
	{
	    send_to_char("Bounties must be greater than 0!\n\r",ch);
	    return;
	}

	ch->platinum -= amount;
	victim->pcdata->bounty +=amount;

	sprintf( buf, "You have placed a {#%d {8pl{7a{&ti{7n{8um{x bounty on %s{x.\n\r%s now has a bounty of {#%d {8pl{7a{&ti{7n{8um{x.\n\r",
	    amount,victim->name,victim->name,victim->pcdata->bounty );
	send_to_char(buf,ch);
	sprintf( buf, "%s has added {Y%d {8pl{7a{&ti{7n{8um{x to your bounty.\n\rYou are now wanted {Rd{re{Ra{rd{x for {Y%d {8pl{7a{&ti{7n{8um{x.\n\r",
	    ch->name,amount,victim->pcdata->bounty );
	send_to_char(buf,victim);
	sprintf( buf, "{GBOUNTY{x: %s has added {Y%d {8pl{7a{&ti{7n{8um{x to %s'%s bounty.\n\r{GBOUNTY{x: %s is now wanted {Rd{re{Ra{rd{x for {Y%d {8pl{7a{&ti{7n{8um{x.",
	    ch->name,amount,victim->name,victim->name[strlen(victim->name)-1] ==
	    's' ? "" : "s", victim->name,victim->pcdata->bounty );
	info(buf,ch,victim,INFO_BOUNTY);
	rank_chart(victim,"bounty",victim->pcdata->bounty);
	return;
    }
}

void do_balance( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
    {
	send_to_char( "You don't need the bank.\n\r", ch );
	return;
    }

    for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
    {
        if ( mob->pIndexData && mob->pIndexData->bank_branch > 0 )
            break;
    }

    if ( mob == NULL )
    {
        send_to_char( "Does this look like the bank to you?\n\r", ch );
        return;
    }

    sprintf( buf, "$n says '{SGreetings $N, your account contains {6%d {8pl{7a{&ti{7n{8um{S.{x'",
	ch->pcdata->bank_account[mob->pIndexData->bank_branch-1] );
    act( buf, mob, NULL, ch, TO_VICT, POS_RESTING );
}

void do_withdraw( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    char buf [MAX_STRING_LENGTH];
    int amount;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "You don't need the bank.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "withdraw <amount>.\n\r", ch );
        return;
    }

    for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
    {
        if ( mob->pIndexData && mob->pIndexData->bank_branch > 0 )
            break;
    }

    if ( mob == NULL )
    {
        send_to_char( "You attempt to produce money from thin air, but fail.\n\r", ch );
        return;
    }

    amount = atoi( argument );

    if ( amount < 0 )
    {
	act( "$n says '{SHey $N, withdrawls must be made in positive amounts!{x'",
	    mob, NULL, ch, TO_VICT, POS_RESTING );
	return;
    }

    if ( ch->pcdata->bank_account[mob->pIndexData->bank_branch-1] < amount )
    {
	sprintf(buf, "$n says '{SHey $N, it is kind of hard to withdraw {c%d {8pl{7a{&ti{7n{8um {Swhen your account only contains {c%d {8pl{7a{&ti{7n{8um{S!{x'",
	    amount, ch->pcdata->bank_account[mob->pIndexData->bank_branch-1] );
	act( buf, mob, NULL, ch, TO_VICT, POS_RESTING );
	return;
    }

    ch->pcdata->bank_account[mob->pIndexData->bank_branch-1] -= amount;
    ch->platinum += amount;

    sprintf( buf, "$n says '{SThank you for your business, $N!{x'\n\r"
		  "$n says '{SHere is the {c%d {8pl{7a{&ti{7n{8um {Sthat you requested.{x'\n\r"
		  "$n says '{SYour new balance is {c%d {8pl{7a{&ti{7n{8um{S.{x'\n\r"
		  "$n gives you %d platinum.\n\r"
		  "$n shakes your hand and smiles happily.", amount,
	ch->pcdata->bank_account[mob->pIndexData->bank_branch-1], amount );
    act( buf, mob, NULL, ch, TO_VICT, POS_RESTING );
}

void do_deposit( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    char buf [MAX_STRING_LENGTH];
    int amount;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "You don't need the bank.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "deposit <amount>.\n\r", ch );
	return;
    }

    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( mob->pIndexData && mob->pIndexData->bank_branch > 0 )
	    break;
    }

    if ( mob == NULL )
    {
        send_to_char( "You attempt to deposit your funds into thin air, but fail.\n\r", ch );
        return;
    }

    amount = atoi( argument );

    if ( amount < 0 )
    {
	act( "$n says '{SHey $N, deposits must be made in positive amounts!{x'",
	    mob, NULL, ch, TO_VICT, POS_RESTING );
	return;
    }

    if ( ch->platinum < amount )
    {
	sprintf(buf,"$n says '{SHey $N, how do you expect to deposit {c%d {8pl{7a{&ti{7n{8um {Swhen you only have {c%d {8pl{7a{&ti{7n{8um{S?{x'",
	    amount, ch->platinum );
	act( buf, mob, NULL, ch, TO_VICT, POS_RESTING );
	return;
    }

    if ( ( amount + ch->pcdata->bank_account[mob->pIndexData->bank_branch-1] ) > 5000 )
    {
	act( "$n says '{SDue to increasing customers and limited storage we can not hold more than 5000 {8pl{7a{&ti{7n{8um{S per customer.{x'\n\r"
	     "$n says '{SSorry for any inconveniences this may cause you, $N.{x'",
	    mob, NULL, ch, TO_VICT, POS_RESTING );
	return;
    }

    ch->pcdata->bank_account[mob->pIndexData->bank_branch-1] += amount;
    ch->platinum -= amount;

    sprintf( buf, "You give $n %d platinum.\n\r"
		  "$n says '{SGreetings, $N.{x'\n\r"
		  "$n takes your platinum to the vault and deposits it.\n\r"
		  "$n says '{SYour new balance, after depositing {c%d {8pl{7a{&ti{7n{8um{S, is {c%d {8pl{7a{&ti{7n{8um{S.{x'\n\r"
		  "$n says '{SThank you for choosing %s{S, for your banking needs.{x'\n\r"
		  "$n says '{SHave a nice day.{x'",
	amount, amount, ch->pcdata->bank_account[mob->pIndexData->bank_branch-1], ch->in_room->name );
    act( buf, mob, NULL, ch, TO_VICT, POS_RESTING );
}

void do_lore( CHAR_DATA *ch, char *argument )
{
    BUFFER *output;
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument,arg );

    if ( get_skill(ch,gsn_lore) == 0)
    {
	send_to_char("Lore, what is that?\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	send_to_char("Lore what?\n\r",ch);
	return;
    }

    if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_lore].beats );
    if ( number_percent( ) < get_skill(ch,gsn_lore))
    {
	output = display_stats( obj, ch, FALSE );
	check_improve(ch,gsn_lore,TRUE,3);
	page_to_char(output->string,ch);
	free_buf(output);
	return;
    }

    act("You see nothing special about $p.",
	ch,obj,NULL,TO_CHAR,POS_RESTING);

    check_improve(ch,gsn_lore,FALSE,3);
}

void do_whostring( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *stringer;
    OBJ_DATA *diamond = NULL;
    bool found = FALSE;

    smash_tilde( argument );

    if (IS_NPC(ch))
    {
	send_to_char("Mobiles don't need whostrings doof!\n\r",ch);
	return;
    }

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

    if (!str_cmp(argument,"confirm"))
    {
	if (!ch->pcdata->confirm_whostring)
	{
	    send_to_char("You have no whostring to confirm!\n\r",ch);
	    return;
	}
	send_to_char("You have confirmed your new whostring.\n\r",ch);
	ch->pcdata->confirm_whostring = FALSE;
	return;
    }

    if (!ch->pcdata->confirm_whostring)
    {
	found = FALSE;

	for ( diamond = ch->carrying; diamond != NULL; diamond = diamond->next_content )
	{
	    if ( diamond->pIndexData->vnum == 1208 )
	    {
		found = TRUE;
		break;
	    }
	}

	if (!found)
	{
	    act("$N says '{SI charge one {rw{Rh{ro{Rs{rt{Rr{ri{Rn{rg {Wd{wi{Wa{wm{Wo{wn{Wd {Sfor my services.{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
	    act("$N says '{SCome back when you have one ready and waiting!{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
	    return;
	}
    }

    if (argument[0] == '\0')
    {
	act("$N says '{SThe correct syntax is \"whostring <string you want>\"{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
	return;
    }

    if (!ch->pcdata->confirm_whostring)
    {
	obj_from_char(diamond);
	extract_obj(diamond);
    }

    free_string( ch->pcdata->who_descr );

    if ( argument[0] == '\0' )
	ch->pcdata->who_descr = str_dup( "" );
    else
	ch->pcdata->who_descr = str_dup( end_string(argument,16) );

    ch->pcdata->confirm_whostring = TRUE;

    act("$N says '{SYour whostring now reads \" $t {S\".{x'",ch,ch->pcdata->who_descr,stringer,TO_CHAR,POS_RESTING);
    act("$N says '{SIf you fucked up just use whostring again.{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
    act("$N says '{SYou have until you confirm to change this for free if you like.{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
    act("$N says '{SType \"whostring confirm\" to confirm your new whostring.{x'",ch,NULL,stringer,TO_CHAR,POS_RESTING);
}

char *const distance[4] =
{
    "right here.", "nearby to the %s.", "not far %s.", "far to the %s."
};

void scan_char( CHAR_DATA *victim, CHAR_DATA *ch, sh_int depth, sh_int door )
{
    extern char *const dir_name[];
    extern char *const distance[];
    char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

    buf[0] = '\0';

    if ( IS_AFFECTED( ch, AFF_FARSIGHT ) || depth == 0 )
    {
	strcat( buf, PERS( victim, ch ) );
	strcat( buf, ", " );
    } else
        strcat( buf, "Something is moving " );

    sprintf( buf2, distance[depth], dir_name[door] );
    strcat( buf, buf2 );
    strcat( buf, "\n\r" );

    send_to_char( buf, ch );
    return;
}

void scan_list( ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, sh_int depth, sh_int door )
{
    CHAR_DATA *rch;

    if ( scan_room == NULL )
	return;

    for ( rch = scan_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch
	||   ( !IS_NPC( rch ) && rch->invis_level > get_trust( ch ) )
	||   get_trust( ch ) < rch->ghost_level )
	    continue;

	if ( can_see( ch, rch ) )
	    scan_char( rch, ch, depth, door );
    }

    return;
}

void do_scan( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pExit;
    ROOM_INDEX_DATA *scan_room;
    char arg1[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    extern char *const dir_name[];
    sh_int door, depth, outlet;

    argument = one_argument( argument, arg1 );

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
    {
	send_to_char( "Maybe it would help if you could see?\n\r", ch );
	return;
    }

    if ( arg1[0] == '\0' )
    {
	act( "$n looks all around.", ch, NULL, NULL, TO_ROOM, POS_RESTING );
	send_to_char( "Looking around you see:\n\r", ch );
	scan_list( ch->in_room, ch, 0, -1 );

	for ( door = 0; door < 6; door++ )
	{
	    outlet = door;
	    if ( ( pExit = ch->in_room->exit[outlet] ) != NULL
	    &&   !IS_SET( pExit->exit_info, EX_NO_SCAN ) )
		scan_list( pExit->u1.to_room, ch, 1, outlet );
	}

	return;
    }

    else if ( !str_prefix( arg1, "north" ) )	door = 0;
    else if ( !str_prefix( arg1, "east" ) )	door = 1;
    else if ( !str_prefix( arg1, "south" ) )	door = 2;
    else if ( !str_prefix( arg1, "west" ) )	door = 3;
    else if ( !str_prefix( arg1, "up" ) )	door = 4;
    else if ( !str_prefix( arg1, "down" ) )	door = 5;
    else
    {
	send_to_char( "Which way do you want to scan?\n\r", ch );
	return;
    }

    act( "You peer intently $T.", ch, NULL, dir_name[door], TO_CHAR, POS_RESTING );
    act( "$n peers intently $T.", ch, NULL, dir_name[door], TO_ROOM, POS_RESTING );
    sprintf( buf, "Looking %s you see:\n\r", dir_name[door] );

    scan_room = ch->in_room;

    for ( depth = 1; depth < 4; depth++ )
    {
	outlet = door;

	if ( ( pExit = scan_room->exit[outlet] ) != NULL )
	{
	    scan_room = pExit->u1.to_room;

	    if ( !can_see_room( ch, scan_room )
	    ||   IS_SET( pExit->exit_info, EX_NO_SCAN ) )
		break;

	    scan_list( pExit->u1.to_room, ch, depth, outlet );
	}
    }

    return;
}

void do_checkstats( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    void *old_edit;
    int edit, info;

    if ( ch->desc == NULL )
    {
	send_to_char( "Descriptor not found.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char(	"Syntax: checkstats race    < race name >\n\r"
			"        checkstats class   < class name >\n\r"
			"        checkstats skill   < skill name >\n\r"
			"        checkstats group   < group name >\n\r"
			"        checkstats prefix  < list | prefix name >\n\r"
			"        checkstats suffix  < list | suffix name >\n\r"
			"        checkstats damages < all  | [dam_type] >\n\r", ch );
    }

    else if ( !str_prefix( arg, "damages" ) )
	show_damlist( ch, argument );

    else if ( !str_prefix( arg, "race" ) )
    {
	if ( ( info = race_lookup( argument ) ) == -1 )
	{
	    send_to_char( "No such race exists.\n\r", ch );
	    return;
	}

	old_edit = ch->desc->pEdit;
	ch->desc->pEdit = (void *)info;
	race_edit_show( ch, "" );
	ch->desc->pEdit = old_edit;
    }

    else if ( !str_prefix( arg, "prefix" ) )
    {
	if ( !str_cmp( argument, "list" ) )
	{
	    show_random_table( ch, prefix_table );
	    return;
	}

	if ( ( info = random_lookup( prefix_table, argument ) ) == -1 )
	{
	    send_to_char( "No such prefix exists.\n\r", ch );
	    return;
	}

	old_edit = ch->desc->pEdit;
	edit = ch->desc->editor;
	ch->desc->pEdit = (void *)info;
	ch->desc->editor = ED_PREFIX;
	random_edit_show( ch, "" );
	ch->desc->pEdit = old_edit;
	ch->desc->editor = edit;
    }

    else if ( !str_prefix( arg, "suffix" ) )
    {
	if ( !str_cmp( argument, "list" ) )
	{
	    show_random_table( ch, suffix_table );
	    return;
	}

	if ( ( info = random_lookup( suffix_table, argument ) ) == -1 )
	{
	    send_to_char( "No such suffix exists.\n\r", ch );
	    return;
	}

	old_edit = ch->desc->pEdit;
	edit = ch->desc->editor;
	ch->desc->pEdit = (void *)info;
	ch->desc->editor = ED_SUFFIX;
	random_edit_show( ch, "" );
	ch->desc->pEdit = old_edit;
	ch->desc->editor = edit;
    }

    else if ( !str_prefix( arg, "groups" ) )
    {
	if ( ( info = group_lookup( argument ) ) == -1 )
	{
	    send_to_char( "No such group exists.\n\r", ch );
	    return;
	}

	old_edit = ch->desc->pEdit;
	ch->desc->pEdit = (void *)info;
	gredit_show( ch, "" );
	ch->desc->pEdit = old_edit;
    }

    else if ( !str_prefix( arg, "skill" ) || !str_prefix( arg, "spell" ) )
    {
	if ( ( info = skill_lookup( argument ) ) == -1 )
	{
	    send_to_char( "No such skill exists.\n\r", ch );
	    return;
	}

	old_edit = ch->desc->pEdit;
	ch->desc->pEdit = (void *)info;
	skedit_show( ch, "" );
	ch->desc->pEdit = old_edit;
    }

    else if ( !str_prefix( arg, "class" ) )
    {
	if ( ( info = class_lookup( argument ) ) == -1 )
	{
	    send_to_char( "No such class exists.\n\r", ch );
	    return;
	}

	old_edit = ch->desc->pEdit;
	ch->desc->pEdit = (void *)info;
	class_edit_show( ch, "skills" );
	ch->desc->pEdit = old_edit;
    }

    else
	do_checkstats( ch, "" );
}

void do_finger( CHAR_DATA *ch, char *argument )
{
    FILE *fp;
    BUFFER *final;
    CHAR_DATA *victim;
    char arg    [MAX_INPUT_LENGTH];
    char buf    [MAX_STRING_LENGTH];
    char logbuf   [MAX_STRING_LENGTH];
    int i;
    bool backup = FALSE;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Finger whom?\n\r", ch );
        return;
    }

    smash_dot_slash( argument );

    one_argument( argument, arg );

    if ( IS_TRUSTED(ch,MAX_LEVEL-1) && !str_cmp(arg,"backup") )
    {
        argument = one_argument( argument, arg );
        backup = TRUE;

        if ( argument[0] == '\0' )
        {
            send_to_char( "Finger whom?\n\r", ch );
            return;
        }
    }

    if ( !backup && ( victim = get_pc_world( ch, arg ) ) != NULL )
    {
        act( "$N is on right now!", ch, NULL, victim, TO_CHAR,POS_RESTING);
        do_stat( ch, victim->name );
        return;
    }

    victim = new_char();
    victim->pcdata = new_pcdata();

    if ( !backup )
        sprintf( buf, "%s%s/%s", PLAYER_DIR, initial( argument ),
            capitalize( argument ) );
    else
        sprintf( buf, "%s%s/%s", BACKUP_DIR, initial( argument ),
            capitalize( argument ) );

    if ( ( fp = fopen( buf, "r" ) ) == NULL )
    {
        sprintf( buf, "%s%s", PLAYER_DIR,
            capitalize( argument ) );
    }
    if ( ( fp = fopen( buf, "r" ) ) == NULL )
    {
        send_to_char("No player by that name exists.\n\r",ch);
        free_char(victim);
        return;
    }
        for ( ; ; )
        {
            char letter;
            char *word;

            letter = fread_letter( fp );
            if ( letter == '*' )
            {
                fread_to_eol( fp );
                continue;
            }

            if ( letter != '#' )
            {
                bug( "Load_char_obj: # not found.", 0 );
                break;
            }
            word = fread_word( fp );
            if ( !str_cmp( word, "PLAYER" ) )
            {
                fread_char( victim, fp, 2 );
                break;
            } else {
                bug( "Load_char_obj: bad section.", 0 );
                break;
            }

        }
        fclose( fp );

    if ( victim->level > LEVEL_HERO && !can_over_ride( ch, victim, TRUE ) )
    {
        send_to_char("The gods wouldn't like that.\n\r",ch);
        free_char(victim);
        return;
    }

    final = new_buf();

    whois_finger( final, ch, victim );

    add_buf( final, "{s|================================================================|\n\r" );

    if ( IS_TRUSTED(ch,MAX_LEVEL-1)
    &&   victim->pcdata->socket != NULL
    &&   victim->pcdata->socket[0] != '\0' )
    {
	sprintf(buf, "{s| {tLast socket:{q %s {s|\n\r",
	    end_string(victim->pcdata->socket, 49) );
        add_buf(final,buf);
    }

    sprintf( logbuf, "{q%s", ctime(&victim->pcdata->llogoff));

    for (i = 0; i < strlen(logbuf); i++)
    {
	if ( logbuf[i] == '\n'
	||   logbuf[i] == '\r' )
	    logbuf[i] = '\0';
    }

    sprintf(buf, "{s| {tLast logged off:{q %s {s|\n\r",
	end_string(logbuf, 45) );
    add_buf(final,buf);

    add_buf( final, "{s ================================================================\n\r" );

    page_to_char(final->string,ch);
    free_buf(final);
    free_char(victim);
    return;
}

void show_pk_record_short( PKILL_RECORD *pk_record, CHAR_DATA *ch )
{
    BUFFER *final = new_buf();
    char buf[MAX_STRING_LENGTH], time[100];
    int pos = 0;

    if ( pk_record == NULL )
    {
	send_to_char( "No PK records found to display.\n\r", ch );
	return;
    }

    add_buf( final ,"{C -{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={R( {rPkill Records{R ){c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C- \n\r"
		    "{C| {c*************************************************************************** {C|\n\r"
		    "{C| {c* {wNum{c) {wKiller        {c[{wLvl{c] * {wVictim        {c[{wLvl{c] * {wTime                   {c* {C|\n\r"
		    "{C| {c*************************************************************************** {C|\n\r" );

    for ( ; pk_record != NULL; pk_record = pk_record->next )
    {
	pos++;
	strftime( time, 100, "{R({w%m{R/{w%d{R/{w%Y{R) {w%I{R:{w%M{R:{w%S {R%p",
	    localtime(&pk_record->pkill_time) );
	sprintf( buf, "{C| {c* {w%3d{c) {w%-13s {c[{w%3d{c] * {w%-13s {c[{w%3d{c] *{w%s{c* {C|\n\r",
	    pos, pk_record->killer_name, pk_record->level[0],
	    pk_record->victim_name, pk_record->level[1], time );
	add_buf( final, buf );
    }

    add_buf( final, "{C| {c*************************************************************************** {C|\n\r"
		    "{C -{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={R( {rPkill Records{R ){c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{x \n\r" );

    page_to_char( final->string, ch );
    free_buf( final );
}

void show_pk_record_long( PKILL_RECORD *pk_record, CHAR_DATA *ch, int pos )
{
    BUFFER *final;
    char buf[MAX_STRING_LENGTH], time[100];
    int num = 0;

    if ( pk_record == NULL )
    {
	send_to_char( "No PK records found to display.\n\r", ch );
	return;
    }

    for ( ; pk_record != NULL; pk_record = pk_record->next )
    {
	if ( ++num < pos )
	    continue;

	final = new_buf();

	sprintf( buf, "Killer: [%3d] %s < %s >\n\r",
	    pk_record->level[0], pk_record->killer_name, pk_record->killer_clan );
	add_buf( final, buf );

	sprintf( buf, "Victim: [%3d] %s < %s >\n\r",
	    pk_record->level[1], pk_record->victim_name, pk_record->victim_clan );
	add_buf( final, buf );

	sprintf( buf, "Assistant Killers: %s\n\r", pk_record->assist_string );
	add_buf( final, buf );

	sprintf( buf, "Pkill Points: %d\n\r", pk_record->pkill_points );
	add_buf( final, buf );

	sprintf( buf, "Bounty Amount: %d\n\r", pk_record->bounty );
	add_buf( final, buf );

	strftime( time, 100, "{R({w%m{R/{w%d{R/{w%Y{R) {w%H{R:{w%M{R:{w%S",
	    localtime(&pk_record->pkill_time) );
	sprintf( buf, "Time: %s\n\r", time );
	add_buf( final, buf );

	page_to_char( final->string, ch );
	free_buf( final );
	return;
    }

    send_to_char( "No PK records found to display.\n\r", ch );
}

void do_pkcheck( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int clan, pos = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char(	"Syntax:\n\r"
			"  Recent kills List:    pkcheck recent\n\r"
			"  Recent kills Details: pkcheck recent < # >\n\r\n\r"
			"  For List:    pkcheck < argument > < type >\n\r"
			"  For Details: pkcheck < argument > < type > < # >\n\r\n\r"
			"Valid arguments: any clan name, and connected player.\n\r"
			"Valid types: kills, deaths.\n\r", ch );
	return;
    }

    if ( argument[0] != '\0' && is_number( argument ) )
	pos = atoi( argument );

    if ( !str_prefix( arg1, "recent" ) )
    {
	if ( arg2[0] != '\0' && is_number( arg2 ) )
	    pos = atoi( arg2 );

	if ( pos > 0 )
	    show_pk_record_long( recent_list, ch, pos );
	else
	    show_pk_record_short( recent_list, ch );
    }

    else if ( arg2[0] == '\0' )
	do_pkcheck( ch, "" );

    else if ( ( clan = clan_lookup( arg1 ) ) != 0 )
    {
	if ( !str_prefix( arg2, "kills" ) )
	{
	    if ( pos > 0 )
		show_pk_record_long( clan_table[clan].kills_list, ch, pos );
	    else
		show_pk_record_short( clan_table[clan].kills_list, ch );
	}

	else if ( !str_prefix( arg2, "deaths" ) )
	{
	    if ( pos > 0 )
		show_pk_record_long( clan_table[clan].death_list, ch, pos );
	    else
		show_pk_record_short( clan_table[clan].death_list, ch );
	}
    }

    else if ( ( victim = get_pc_world( ch, arg1 ) ) != NULL )
    {
	if ( !str_prefix( arg2, "kills" ) )
	{
	    if ( pos > 0 )
		show_pk_record_long( victim->pcdata->kills_list, ch, pos );
	    else
		show_pk_record_short( victim->pcdata->kills_list, ch );
	}

	else if ( !str_prefix( arg2, "deaths" ) )
	{
	    if ( pos > 0 )
		show_pk_record_long( victim->pcdata->death_list, ch, pos );
	    else
		show_pk_record_short( victim->pcdata->death_list, ch );
	}
    }

    else
	do_pkcheck( ch, "" );
}

void do_showskill( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int class, col = 0, sn;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: showskill <skill name>\n\r", ch );
	return;
    }

    if ( ( sn = skill_lookup( argument ) ) == -1 )
    {
	send_to_char( "No skill or spell by that name exists.\n\r", ch );
	return;
    }

    sprintf( buf, "Classes and levels for %s:\n\r", skill_table[sn].name );
    send_to_char( buf, ch );

    for ( class = 0; class_table[class].name[0] != '\0'; class++ )
    {
	if ( skill_table[sn].skill_level[class] < LEVEL_IMMORTAL )
        {
	    sprintf( buf, "{w%-12s{x: {m%-4d{x", class_table[class].name,
		skill_table[sn].skill_level[class] );
	    send_to_char( buf, ch );

	    if ( ++col % 3 == 0 )
		send_to_char( "\n\r", ch );
	}
    }

    if ( col %3 != 0 )
	send_to_char( "\n\r", ch );
}

void do_showclass( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    sh_int class, col = 0, lev = 0, skill_lev, sn;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: showclass <class name>\n\r", ch );
	return;
    }

    if ( ( class = class_lookup( argument ) ) == -1 )
    {
	send_to_char( "No class by that name exists.\n\r", ch );
	return;
    }

    sprintf( buf, "Spells/Skills for %s:\n\r", class_table[class].name );
    send_to_char( buf, ch );

    for ( lev = 1; lev < LEVEL_IMMORTAL; lev++ )
    {
	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    skill_lev = skill_table[sn].skill_level[class];

	    if ( skill_lev == lev )
	    {
		sprintf( buf, "{DLevel {m%-3d{x: {W%-18s{x",
		    lev, skill_table[sn].name );
		send_to_char( buf, ch );

		if ( ++col % 2 == 0 )
		    send_to_char( "\n\r", ch );
	    }
	}
    }

    if ( col % 2 != 0 )
	send_to_char( "\n\r", ch );
}

sh_int devote_lookup( char *argument )
{
    sh_int pos;

    for ( pos = 0; pos < DEVOTE_CURRENT; pos++ )
    {
	if ( !str_prefix( argument, devote_table[pos].name ) )
	    return pos;
    }

    return -1;
}

void do_devote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    sh_int pos;

    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: devote <discipline>\n\r"
		      "        devote status\n\r"
		      "        devote recalculate\n\r", ch );
        return;
    }

    if ( !str_prefix( argument, "status" ) )
    {
        BUFFER *final= new_buf();
        
	add_buf( final, "{e ==========================================================================\n\r");
	add_buf( final, "{e|{q                        {tDevotion to Improving Oneself                     {e|\n\r");
	add_buf( final, "{e|==========================================================================|\n\r");
	add_buf( final, "{e| {qName      {e| {qLevel {e|   {qCurrent  {e| {qNext Level {e| {qTotal Req. {e| {qBonus Effects {e|\n\r");
	
	
	for ( pos = 0; pos < DEVOTE_CURRENT; pos++ )
	{
	    
	    sprintf( buf, "{e| {t%-8s  {e|   {t%3d {e| {t%10ld {e| {t%10ld {e| {t%10ld {e| {t%-13s {e|\n\r", 
	        capitalize(devote_table[pos].name),
	        ch->pcdata->devote_points[pos],
	        ch->pcdata->devote[pos],
	        (ch->pcdata->devote_next[pos] - ch->pcdata->devote[pos]),
	        ch->pcdata->devote_next[pos],
	        devote_table[pos].bonus_name );
	    add_buf(final,buf);
	}
	
	add_buf( final, "{e|==========================================================================|\n\r");
	sprintf( buf, "{e| {tYou are currently improving your {q%s{t.",
	    devote_table[ch->pcdata->devote[DEVOTE_CURRENT]].name );
	add_buf(final, end_string(buf, 75));
	
	add_buf( final, "{e|\n\r{e| {tTo change your devotion discipline, use {qdevote <type>{t.                   {e|\n\r");
	
	add_buf( final, "{e =========================================================================={x\n\r");
	send_to_char( final->string, ch );
	free_buf(final);
	
	return;
    }

    if ( !str_prefix( argument, "recalculate" ) )
    {
	do_devote_assign( ch );
	return;
    }

    if ( ( pos = devote_lookup( argument ) ) == -1 )
    {
	send_to_char( "Invalid devotion type.\n\r", ch );
	return;
    }

    ch->pcdata->devote[DEVOTE_CURRENT] = pos;

    sprintf( buf, "Devotion discipline set to {q%s{x.\n\r", devote_table[ch->pcdata->devote[DEVOTE_CURRENT]].name );

    send_to_char( buf, ch );

    return;
}



/*
 * Brand new version. -Locke
 */
void scan_direction( CHAR_DATA *ch, int dir )
{
    ROOM_INDEX_DATA *pScene;
    EXIT_DATA *pExit;
    char buf[MAX_STRING_LENGTH];
    int dist;

    if ( (pExit = ch->in_room->exit[dir]) == NULL
      || IS_SET(pExit->exit_info, EX_CLOSED) )
    return;

    for( dist = 1; dist < 5; dist++ )
    {
        if ( (pScene = pExit->u1.to_room) == NULL )
        break;

        show_room_to_char( ch, pScene, dist, dir );

        if ( (pExit = pScene->exit[dir]) == NULL )
        break;

        if ( IS_SET(pExit->exit_info, EX_CLOSED)
          || IS_SET(pExit->exit_info, EX_CONCEALED) )
        {
            if ( !IS_SET(pExit->exit_info, EX_CONCEALED) )
            {
                sprintf( buf, "A closed %s%s blocks your view.\n\r",
                         pExit->keyword,
                         dist == 1 ? " nearby"          :
                         dist == 2 ? " close by"        :
                         dist == 3 ? " in the distance" : "" );
                page_to_char( buf, ch );
            }

            break;
        }
        else
        if ( IS_SET(pExit->exit_info, EX_ISDOOR)
          && !IS_SET(pExit->exit_info, EX_CONCEALED) )
        {
            sprintf( buf, "You can see an opened %s%s.\n\r",
                     pExit->keyword,
                     dist == 1 ? " nearby"          :
                     dist == 2 ? " close by"        :
                     dist == 3 ? " in the distance" : "" );
            page_to_char( buf, ch );
        }
        else
        if ( !IS_SET(pExit->exit_info, EX_ISDOOR)
          && !IS_SET(pExit->exit_info, EX_CONCEALED)
          && !MTD(pExit->keyword) )
        {
            sprintf( buf, "You are able to peer through a%s %s.\n\r",
                     IS_VOWEL(pExit->keyword[0]) ? "n" : "",
                     pExit->keyword );
            page_to_char( buf, ch );
        }
    }

    return;
}



void show_room_to_char( CHAR_DATA *ch, ROOM_INDEX_DATA *pScene, int dist, int dir )
{
    CHAR_DATA *rch;
    char buf[MAX_STRING_LENGTH];
    //char *p;
    int count;

    /*
     * Count the number of people.
     */
    count = 0;
    for ( rch = pScene->people; rch != NULL;  rch = rch->next_in_room )
    {
       if ( rch != ch && can_see( ch, rch ) )
       {
           count++;
           if ( IS_NPC(ch) && IS_SET(ch->act, ACT_NOSCAN) )  count--;
       }
    }

    if ( room_is_dark( pScene )
      || !check_blind( ch )
      || count <= 0 )
    return;

    switch ( dist )
    {
       case 1: sprintf( buf, "Nearby %s you can see", dir_name[dir] ); break;
       case 2: sprintf( buf, "%s of here, you see", capitalize(dir_name[dir]) ); break;
       case 3: sprintf( buf, "In the distance to the %s is", dir_name[dir] ); break;
       case 4: sprintf( buf, "Far away %s from you is",      dir_name[dir] ); break;
      default: break;
    }

    bool found=FALSE;

    for( rch = pScene->people;  rch != NULL;  rch = rch->next_in_room )
    {

        if ( !can_see( ch, rch )
          || (IS_NPC(ch) && IS_SET(ch->act, ACT_NOSCAN))
          || ch == rch )
        continue;

        if ( --count < 0 ) break;

        strcat( buf, " " );
        strcat( buf, NAME(rch) );
        found=TRUE;

        if ( count > 1 )
        strcat( buf, "," );

        if ( count == 1 )
        strcat( buf, " and" );

        if ( count == 0 )
        strcat( buf, ".\n\r" );
    }
    if ( !found ) strcat( buf, " no one at all.\n\r");

//    p = format_string( str_dup(buf) );
    page_to_char( buf, ch );
 //   free_string( p );
    return;
}

void display_ascii( char *name, CHAR_DATA *ch ) {
 if ( !IS_SET( ch->configure, CONFIG_ASCII ) ) return;
 HELP_DATA *pHelp;
 char keyword[MSL];
 sprintf( keyword, "ASCII_%s", name );
 for ( pHelp= help_first; pHelp; pHelp=pHelp->next ) {
  if ( !str_cmp( keyword, pHelp->name ) || !str_cmp( keyword, pHelp->keyword ) ) {
   send_to_char( pHelp->text, ch );
   return;
  }
 }
}
