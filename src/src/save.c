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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "recycle.h"

DECLARE_DO_FUN( do_look		);

#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];

void	note_attach	args( ( CHAR_DATA *ch, int type ) );
void	reset_char	args( ( CHAR_DATA *ch ) );
sh_int	devote_lookup	args( ( char *argument ) );


char *initial( const char *str )
{
    static char strint [ MAX_INPUT_LENGTH ];

    strint[0] = LOWER( str[ 0 ] );
    return strint;
}

char *print_flags( int flag )
{
    int count, pos = 0;
    static char buf[52];

    for ( count = 0; count < 32;  count++ )
    {
	if ( IS_SET( flag, 1 << count ) )
	{
	    if ( count < 26 )
		buf[pos] = 'A' + count;
	    else
		buf[pos] = 'a' + ( count - 26 );
	    pos++;
	}
    }

    if ( pos == 0 )
    {
	buf[pos] = '0';
	pos++;
    }

    buf[pos] = '\0';

    return buf;
}

int get_percentage_bonus( CHAR_DATA *ch, float value )
{
    int i, mod = 100;
    float value_old = value;
    bool minus = FALSE;
    
    if ( value < 0 )
    {
        minus = TRUE;
        value *= -1;
        value_old *= -1;
    }

    for ( i = 0; i < ch->pcdata->devote_points[DEVOTE_EQ]; i++ )
    {
        mod += FORGE_SCALE_PERCENT;
    }

    value *= mod;
    value /= 100;
    value -= value_old;

    mod = 0;

    // This bit is to prevent any wierd numbers cropping up and to round properly
    for ( ; value >= 10; )
    {
        mod += 10;
	value -= 10;
    }
    for ( ; value >= 5; )
    {
        mod += 5;
	value -= 5;
    }
    for ( ; value >= 0.5; )
    {
        mod += 1;
	value -= 1;
    }

    if ( minus )
        mod *= -1;

    return mod;
}

void devote_affect_add( CHAR_DATA *ch, AFFECT_DATA *af )
{
    AFFECT_DATA *af_old;

    for ( af_old = ch->affected; af_old != NULL; af_old = af_old->next )
    {
	if ( af->type == af_old->type
	&&   af->location == af_old->location
	&&   af->where == af_old->where )
	{
	    af->modifier += af_old->modifier;
	    affect_remove( ch, af_old );
	    break;
	}
    }
    affect_to_char(ch, af);
    return;
}

void do_devote_assign( CHAR_DATA *ch )
{
//    AFFECT_DATA af;
    int pos, mod;
    long exp, devote_exp;
    
    if ( ch == NULL )
    {
        bug("Devote_assign: NULL ch!",0);
        return;
    }
    
    mod = 100 + DEVOTE_TNL_PERCENT;
    
    for ( pos = 0; pos < DEVOTE_CURRENT; pos++ )
    {
        devote_exp = ch->pcdata->devote[pos];
        ch->pcdata->devote_points[pos] = 0;
        for ( exp = DEVOTE_BASE_EXP; ; )
        {
            if (devote_exp < exp)
                break;
            
            devote_exp -= exp;
            ch->pcdata->devote_points[pos] += 1;
            
            exp = exp*mod/100;
        }
        ch->pcdata->devote_next[pos] = (exp + ch->pcdata->devote[pos] - devote_exp);
    }
/*
    affect_strip(ch, gsn_kailfli);
    af.where	= TO_AFFECTS;
    af.type	= gsn_kailfli;
    af.level	= ch->level;
    af.dur_type	= DUR_TICKS;
    af.duration	= -1;
    af.bitvector= 0;
    
    for ( pos = 0; pos < DEVOTE_CURRENT; pos++ )
    {
        if (ch->pcdata->devote_points[pos] <= 0)
            continue;
            
        switch (pos)
        {
            case DEVOTE_BODY:
                af.location	= APPLY_MOVE;
                af.modifier	= ch->pcdata->devote_points[pos]*40;
                affect_to_char(ch,&af);
                af.location	= APPLY_AC;
                af.modifier	= -ch->pcdata->devote_points[pos]*15;
                affect_to_char(ch,&af);
                break;
            case DEVOTE_MIND:
                af.location	= APPLY_SAVES;
                af.modifier	= -ch->pcdata->devote_points[pos]*2;
                affect_to_char(ch,&af);
                af.location	= APPLY_WIS;
                af.modifier	= ch->pcdata->devote_points[pos]/2;
                affect_to_char(ch,&af);
                break;
            case DEVOTE_SPIRIT:
                af.location	= APPLY_MANA;
                af.modifier	= ch->pcdata->devote_points[pos]*40;
                affect_to_char(ch,&af);
                af.location	= APPLY_INT;
                af.modifier	= ch->pcdata->devote_points[pos]/2;
                affect_to_char(ch,&af);
                break;
            case DEVOTE_GRACE:
                af.location	= APPLY_HITROLL;
                af.modifier	= ch->pcdata->devote_points[pos]*15;
                affect_to_char(ch,&af);
                af.location	= APPLY_DEX;
                af.modifier	= ch->pcdata->devote_points[pos]/2;
                affect_to_char(ch,&af);
                break;
            case DEVOTE_FORCE:
                af.location	= APPLY_DAMROLL;
                af.modifier	= ch->pcdata->devote_points[pos]*15;
                affect_to_char(ch,&af);
                af.location	= APPLY_STR;
                af.modifier	= ch->pcdata->devote_points[pos]/2;
                affect_to_char(ch,&af);
                break;
            case DEVOTE_LIFE:
                af.location	= APPLY_HIT;
                af.modifier	= ch->pcdata->devote_points[pos]*40;
                affect_to_char(ch,&af);
                af.location	= APPLY_CON;
                af.modifier	= ch->pcdata->devote_points[pos]/2;
                affect_to_char(ch,&af);
                break;
            case DEVOTE_EQ:
            case DEVOTE_SKILLS:
            case DEVOTE_SPELLS:
            default:
                break;
        }
    }
*/    
    return;
}

void do_reload( CHAR_DATA *ch, char *argument )
{
    FILE *fp;
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
	send_to_char( "Whos pfile do you wish to reload?\n\r", ch );
	return;
    }

    sprintf( buf, "%s%s/%s", BACKUP_DIR, initial( argument ),
	capitalize( argument ) );

    if ( ( fp = fopen( buf, "r" ) ) == NULL )
    {
	act( "No backup pfile could be found matching '$t'.",
	    ch, argument, NULL, TO_CHAR, POS_DEAD );
	return;
    }
    fclose( fp );

    if ( ( victim = get_pc_world( ch, argument ) ) != NULL )
    {
	d = victim->desc;

	if ( d == NULL )
	{
	    send_to_char("They are currently linkdead.\n\r",ch);
	    return;
	}

	extract_char( victim, TRUE );
	d->character = NULL;
	load_char_obj( d, victim->name, FALSE, TRUE );
	victim = d->character;
	victim->next = char_list;
	char_list = victim;
	victim->pcdata->next_player = player_list;
	player_list = victim;
	free_string( victim->pcdata->socket );
	victim->pcdata->socket = str_dup( d->host );
	if ( victim->in_room != NULL )
	    char_to_room( victim, victim->in_room );
	else
	    char_to_room( victim, get_room_index( ROOM_VNUM_ALTAR ) );
	if ( victim->pet != NULL )
	    char_to_room( victim->pet, victim->in_room );
	send_to_char( "The Immortals have blessed you with a pfile backup.\n\r", victim );
	do_look( victim, "auto" );
	racial_spells( victim, TRUE );
	do_devote_assign( victim );
	act( "You have restored $N's pfile.",
	    ch, NULL, victim, TO_CHAR, POS_DEAD );
    } else {
	sprintf( buf, "cp %s%s/%s %s%s/%s", BACKUP_DIR, initial( argument ),
	    capitalize( argument ), PLAYER_DIR, initial( argument ),
	    capitalize( argument ) );
	system( buf );
	act( "Backup for ' $t ' reinstalled.",
	    ch, argument, NULL, TO_CHAR, POS_DEAD );
    }

    sprintf( buf, "reloads pfile for %s", argument );
    parse_logs( ch, "immortal", buf );
}

void fwrite_char( CHAR_DATA *ch, FILE *fp, bool clan_save )
{
    AFFECT_DATA *paf;
    GRANT_DATA *grant;
    PKILL_DATA *pkill;
    PKILL_RECORD *pk_record;
    int pos;

    if ( !IS_NPC( ch ) )
	fprintf( fp, "#PLAYER\n"				);

    fprintf( fp, "Name %s~\n",	ch->name			);
    fprintf( fp, "Race %s~\n",	race_table[ch->race].name	);
    fprintf( fp, "Levl %d\n",	ch->level			);
    fprintf( fp, "Clas %s~\n",  class_table[ch->class].name	);

    if ( ch->trust != 0 )
	fprintf( fp, "Tru  %d\n", ch->trust );

    if ( ch->invis_level )
	fprintf( fp, "Invi %d\n", ch->invis_level );

    if ( ch->incog_level )
	fprintf( fp, "Inco %d\n", ch->incog_level );

    if ( ch->ghost_level )
	fprintf( fp, "Ghos %d\n", ch->ghost_level );

    if ( ch->pcdata != NULL )
    {
	fprintf( fp, "Pkpt %d\n",	ch->pcdata->pkpoints		);
	fprintf( fp, "TSex %d\n",	ch->pcdata->true_sex		);
	fprintf( fp, "LogO %ld\n",
	    clan_save ? ch->pcdata->llogoff : current_time );

	if ( ch->clan )
	    fprintf( fp, "Clan %s~ %d\n",
		clan_table[ch->clan].name, ch->pcdata->clan_rank );

	if ( ch->pcdata->socket != NULL && ch->pcdata->socket[0] != '\0' )
	    fprintf( fp, "Sock %s~\n", ch->pcdata->socket );

	if ( ch->pcdata->who_descr[0] != '\0' )
	    fprintf( fp, "Whod %s~\n", ch->pcdata->who_descr );

	if ( ch->pcdata->title[0] != '\0' )
	    fprintf( fp, "Titl %s~\n", ch->pcdata->title );

	if ( ch->pcdata->pretitle[0] != '\0' )
	    fprintf( fp, "Ptit %s~\n", ch->pcdata->pretitle );

	if ( ch->pcdata->bounty > 0 )
	    fprintf( fp, "Bnty %d\n", ch->pcdata->bounty );

	if ( ch->pcdata->pkills > 0 )
	    fprintf( fp, "Pkil %d\n", ch->pcdata->pkills );

	if ( ch->pcdata->pdeath > 0 )
	    fprintf( fp, "Pdea %d\n", ch->pcdata->pdeath );

	if ( ch->pcdata->assist > 0 )
	    fprintf( fp, "Asst %d\n", ch->pcdata->assist );

	if ( ch->pcdata->arenawins > 0 )
	    fprintf( fp, "Arwn %d\n", ch->pcdata->arenawins );

	if ( ch->pcdata->arenaloss > 0 )
	    fprintf( fp, "Arls %d\n", ch->pcdata->arenaloss );

	if ( ch->pcdata->arenakills > 0 )
	    fprintf( fp, "Arkl %d\n", ch->pcdata->arenakills );

	if ( ch->pcdata->arenadeath > 0 )
	    fprintf( fp, "Ardt %d\n", ch->pcdata->arenadeath );

	fprintf( fp, "\n#STAT_BREAK\n\n"			);
    }

    fprintf( fp, "Id   %ld\n",  ch->id				);
    fprintf( fp, "Plat %d\n",	ch->platinum			);
    fprintf( fp, "Gold %d\n",	ch->gold			);
    fprintf( fp, "Silv %d\n",	ch->silver			);
    fprintf( fp, "Exp  %ld\n",	ch->exp				);

    fprintf( fp, "Attr %d %d %d %d %d\n",
	ch->perm_stat[STAT_STR], ch->perm_stat[STAT_INT],
	ch->perm_stat[STAT_WIS], ch->perm_stat[STAT_DEX],
	ch->perm_stat[STAT_CON] );

    if ( ch->description[0] != '\0' )
    	fprintf( fp, "Desc %s~\n",	ch->description	);

    if ( ch->prompt != NULL )
	fprintf( fp, "Prom %s~\n",      ch->prompt  	);

    if ( ch->act != 0 )
	fprintf( fp, "Act  %s\n", print_flags( ch->act ) );

    if ( ch->affected_by != 0 )
	fprintf( fp, "AfBy %s\n", print_flags( ch->affected_by ) );

    if ( ch->shielded_by != 0 )
	fprintf( fp, "ShBy %s\n", print_flags( ch->shielded_by ) );

    if ( ch->comm != 0 )
	fprintf( fp, "Comm %s\n", print_flags( ch->comm ) );

    if ( ch->configure != 0 )
	fprintf( fp, "Config %s\n", print_flags( ch->configure ) );

    if ( ch->sound )
	fprintf( fp, "Soun %s\n", print_flags( ch->sound ) );

    if ( ch->wiznet )
    	fprintf( fp, "Wizn %s\n", print_flags( ch->wiznet ) );

    if ( ch->info )
	fprintf( fp, "Info %s\n", print_flags( ch->info ) );

    if ( ch->combat )
	fprintf( fp, "Combat %s\n", print_flags( ch->combat ) );

    if ( ch->position > 0 )
	fprintf( fp, "Pos  %d\n",	
	    ch->position == POS_FIGHTING ? POS_STANDING : ch->position );

    if ( ch->alignment != 0 )
	fprintf( fp, "Alig  %d\n", ch->alignment );

    for ( pos = 0; skill_table[pos].name[0] != '\0'; pos++ )
    {
	if ( ch->learned[pos] > 0 )
	    fprintf( fp, "Sk %d '%s'\n",
		ch->learned[pos], skill_table[pos].name );
    }

    if ( ch->pcdata != NULL )
    {
	fprintf( fp, "MaxDam %d\n",	ch->pcdata->max_damage		);
	fprintf( fp, "LLev %d\n",	ch->pcdata->last_level		);
	fprintf( fp, "Pass %s~\n",	ch->pcdata->pwd			);

	fprintf( fp, "Plyd %d\n",
	    ch->pcdata->played + (int) (current_time - ch->pcdata->logon) );

	fprintf( fp, "Note %ld %ld %ld %ld %ld\n",
	    ch->pcdata->last_note, ch->pcdata->last_idea, ch->pcdata->last_penalty,
	    ch->pcdata->last_news, ch->pcdata->last_changes );

	fprintf( fp, "Room %d\n",
	    ( ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
	    && ch->pcdata->was_in_room != NULL )
	    ? ch->pcdata->was_in_room->vnum
	    : ch->in_room == NULL ? 3001 : ch->in_room->vnum );

	fprintf( fp, "HMVN %d %d %d %d %d %d\n",
	    ch->hit, ch->pcdata->perm_hit,
	    ch->mana, ch->pcdata->perm_mana,
	    ch->move, ch->pcdata->perm_move );

	fprintf( fp, "MaxS" );
	for ( pos = 0; pos < MAX_BANK; pos++ )
	    fprintf( fp, " %d", ch->pcdata->max_storage[pos] );
	fprintf( fp, "\n" );

	fprintf( fp, "Bank" );
	for ( pos = 0; pos < MAX_BANK; pos++ )
	    fprintf( fp, " %d", ch->pcdata->bank_account[pos] );
	fprintf( fp, "\n" );

	fprintf( fp, "Cnd  %d %d %d %d\n",
	    ch->pcdata->condition[0], ch->pcdata->condition[1],
	    ch->pcdata->condition[2], ch->pcdata->condition[3] );

	fprintf( fp, "Deviant %d %d\n",
	    ch->pcdata->deviant_points[0], ch->pcdata->deviant_points[1] );

	if ( ch->pcdata->pnote != NULL )
	    fprintf( fp, "Text %d %s~ %s~ %s~\n",
		ch->pcdata->pnote->type, ch->pcdata->pnote->to_list,
		ch->pcdata->pnote->subject, ch->pcdata->pnote->text );

	for ( grant = ch->pcdata->grants; grant != NULL; grant = grant->next )
	    fprintf( fp, "GRANT %s~ %s~\n", grant->command, grant->granter );

	if ( ch->pcdata->color != 0 )
	    fprintf( fp, "Color %d\n", ch->pcdata->color );

	if ( ch->pcdata->color_auc != 0 )
	    fprintf( fp, "Coauc %d\n", ch->pcdata->color_auc );

	if ( ch->pcdata->color_cht != 0 )
	    fprintf( fp, "Cocht %d\n", ch->pcdata->color_cht );

	if ( ch->pcdata->color_cgo != 0 )
	    fprintf( fp, "Cocgo %d\n", ch->pcdata->color_cgo );

	if ( ch->pcdata->color_cla != 0 )
	    fprintf( fp, "Cocla %d\n", ch->pcdata->color_cla );

	if ( ch->pcdata->color_con != 0 )
	    fprintf( fp, "Cocon %d\n", ch->pcdata->color_con );

	if ( ch->pcdata->color_dis != 0 )
	    fprintf( fp, "Codis %d\n", ch->pcdata->color_dis );

	if ( ch->pcdata->color_fig != 0 )
	    fprintf( fp, "Cofig %d\n", ch->pcdata->color_fig );

	if ( ch->pcdata->color_gos != 0 )
	    fprintf( fp, "Cogos %d\n", ch->pcdata->color_gos );

	if ( ch->pcdata->color_gra != 0 )
	    fprintf( fp, "Cogra %d\n", ch->pcdata->color_gra );

	if ( ch->pcdata->color_gte != 0 )
	    fprintf( fp, "Cogte %d\n", ch->pcdata->color_gte );

	if ( ch->pcdata->color_imm != 0 )
	    fprintf( fp, "Coimm %d\n", ch->pcdata->color_imm );

	if ( ch->pcdata->color_mob != 0 )
	    fprintf( fp, "Comob %d\n", ch->pcdata->color_mob );

	if ( ch->pcdata->color_opp != 0 )
	    fprintf( fp, "Coopp %d\n", ch->pcdata->color_opp );

	if ( ch->pcdata->color_qgo != 0 )
	    fprintf( fp, "Coqgo %d\n", ch->pcdata->color_qgo );

	if ( ch->pcdata->color_que != 0 )
	    fprintf( fp, "Coque %d\n", ch->pcdata->color_que );

	if ( ch->pcdata->color_quo != 0 )
	    fprintf( fp, "Coquo %d\n", ch->pcdata->color_quo );

	if ( ch->pcdata->color_roo != 0 )
	    fprintf( fp, "Coroo %d\n", ch->pcdata->color_roo );

	if ( ch->pcdata->color_say != 0 )
	    fprintf( fp, "Cosay %d\n", ch->pcdata->color_say );

	if ( ch->pcdata->color_sho != 0 )
	    fprintf( fp, "Cosho %d\n", ch->pcdata->color_sho );

	if ( ch->pcdata->color_tel != 0 )
	    fprintf( fp, "Cotel %d\n", ch->pcdata->color_tel );

	if ( ch->pcdata->color_wit != 0 )
	    fprintf( fp, "Cowit %d\n", ch->pcdata->color_wit );

	if ( ch->pcdata->color_wiz != 0 )
	    fprintf( fp, "Cowiz %d\n", ch->pcdata->color_wiz );

	if ( ch->pcdata->color_ooc != 0 )
	    fprintf( fp, "Coooc %d\n", ch->pcdata->color_ooc );

	if ( ch->pcdata->color_rac != 0 )
	    fprintf( fp, "Corac %d\n", ch->pcdata->color_rac );

	if ( ch->pcdata->color_fla != 0 )
	    fprintf( fp, "Cobit %d\n", ch->pcdata->color_fla );

	if ( ch->pcdata->color_her != 0 )
	    fprintf( fp, "Coher %d\n", ch->pcdata->color_her );

	if ( ch->pcdata->color_ic != 0 )
	    fprintf( fp, "Coic %d\n", ch->pcdata->color_ic );

	if ( ch->pcdata->color_pra != 0 )
	    fprintf( fp, "Copra %d\n", ch->pcdata->color_pra );

	if ( ch->pcdata->color_olc1 != 0 )
	    fprintf( fp, "CoOLC1 %d\n", ch->pcdata->color_olc1 );

	if ( ch->pcdata->color_olc2 != 0 )
	    fprintf( fp, "CoOLC2 %d\n", ch->pcdata->color_olc2 );

	if ( ch->pcdata->color_olc1 != 0 )
	    fprintf( fp, "CoOLC3 %d\n", ch->pcdata->color_olc3 );

	if ( ch->pcdata->lag != 0 )
	    fprintf( fp, "Lag %d\n", ch->pcdata->lag );

	if ( ch->pcdata->was_in_room != '\0' )
	    fprintf( fp, "ARoom %d\n", ch->pcdata->was_in_room->vnum );

	if ( ch->pcdata->tier > 1 )
	    fprintf( fp, "Tier %d\n", ch->pcdata->tier );

	if ( ch->pcdata->tells > 0 )
	    fprintf( fp, "Tells %d\n%s~\n", ch->pcdata->tells,
		ch->pcdata->buffer->string );

	if ( ch->pcdata->lines != PAGELEN )
	    fprintf( fp, "Scro %d\n", ch->pcdata->lines );

	if ( ch->pcdata->practice != 0 )
	    fprintf( fp, "Prac %d\n", ch->pcdata->practice );

	if ( ch->pcdata->train != 0 )
	    fprintf( fp, "Trai %d\n", ch->pcdata->train );

	if ( ch->pcdata->bamfin[0] != '\0' )
	    fprintf( fp, "Bin  %s~\n", ch->pcdata->bamfin );

	if ( ch->pcdata->bamfout[0] != '\0' )
	    fprintf( fp, "Bout %s~\n", ch->pcdata->bamfout );

	if ( ch->pcdata->identity[0] != '\0' )
	    fprintf( fp, "Iden %s~\n", ch->pcdata->identity );

	if ( ch->pcdata->who_output[0] != '\0' )
	    fprintf( fp, "WhoOut %s~\n", ch->pcdata->who_output );

	for ( pos = 0; pos < DEVOTE_CURRENT; pos++ )
	{
	    if ( ch->pcdata->devote[pos] != 0 )
		fprintf( fp, "Devote %s %ld\n",
		    devote_table[pos].name, ch->pcdata->devote[pos] );
	}
	fprintf( fp, "Devote current %ld\n", ch->pcdata->devote[DEVOTE_CURRENT] );

	for ( pos = 0; pos < PENALTY_MAX; pos++ )
	{
	    if ( ch->pcdata->penalty_time[pos] != 0 )
	    {
		fprintf( fp, "Penalty" );
		for ( pos = 0; pos < PENALTY_MAX; pos++ )
		    fprintf( fp, " %d", ch->pcdata->penalty_time[pos] != -2 ? 
			ch->pcdata->penalty_time[pos] : 0 );
		fprintf( fp, " -2\n" );
		break;
	    }
	}

	if ( ch->pcdata->dtimer > 0 )
	    fprintf( fp, "Dtim %d\n", ch->pcdata->dtimer );

	if ( ch->pcdata->security > 0 )
	    fprintf( fp, "Sec  %d\n", ch->pcdata->security );

	if ( ch->pcdata->chat_chan > 0 )
	    fprintf( fp, "Chat %d\n", ch->pcdata->chat_chan );

	if ( ch->pcdata->points > 0 )
	    fprintf( fp, "Pnts %d\n", ch->pcdata->points );

	if ( ch->pcdata->mobkills > 0 )
	    fprintf( fp, "Mkil %ld\n", ch->pcdata->mobkills );

	if ( ch->pcdata->mobdeath > 0 )
	    fprintf( fp, "Mdea %ld\n", ch->pcdata->mobdeath );

	if ( ch->pcdata->recall )
	    fprintf( fp, "Reca %d\n", ch->pcdata->recall );

	if ( ch->pcdata->total_questcomplete != 0 )
	    fprintf( fp, "TotalCmpl %d\n", ch->pcdata->total_questcomplete );

	if ( ch->pcdata->total_questpoints != 0 )
	    fprintf( fp, "TotalPnts %d\n", ch->pcdata->total_questpoints );

	if ( ch->pcdata->total_questattempt != 0 )
	    fprintf( fp, "TotalAttm %d\n", ch->pcdata->total_questattempt );

	if ( ch->pcdata->total_questexpire != 0 )
	    fprintf( fp, "TotalExpr %d\n", ch->pcdata->total_questexpire );

	if ( ch->pcdata->total_questfail != 0 )
	    fprintf( fp, "TotalFail %d\n", ch->pcdata->total_questfail );

	if ( ch->pcdata->questpoints != 0 )
	    fprintf( fp, "QuestPnts %d\n", ch->pcdata->questpoints );

	if ( ch->pcdata->nextquest != 0 )
	    fprintf( fp, "QuestNext %d\n", ch->pcdata->nextquest );

	if ( ch->pcdata->countdown != 0 )
	    fprintf( fp, "QCount %d\n", ch->pcdata->countdown );

	if ( ch->pcdata->questobj != 0 )
	    fprintf( fp, "QObj %d\n", ch->pcdata->questobj );

	if ( ch->pcdata->questmob != 0 )
	    fprintf( fp, "QMob %d\n", ch->pcdata->questmob );

	if ( ch->pcdata->questgiver != 0 )
	    fprintf( fp, "QGiv %d\n", ch->pcdata->questgiver );

	if ( ch->pcdata->questroom != NULL )
	    fprintf( fp, "QRoom %s~\n", ch->pcdata->questroom );

	if ( ch->pcdata->questarea != NULL )
	    fprintf( fp, "QArea %s~\n", ch->pcdata->questarea );

	for ( pkill = ch->pcdata->recent_pkills; pkill != NULL; pkill = pkill->next )
	{
	    fprintf( fp, "PKILL %ld %d %s~\n",
		pkill->time, pkill->killer, pkill->player_name );
	}

	for ( pk_record = ch->pcdata->kills_list; pk_record != NULL; pk_record = pk_record->next )
	{
	    fprintf( fp, "KRCRD %s~ %s~ %s~ %s~ %d %d %ld %d %d %s~\n",
		pk_record->killer_name,	pk_record->victim_name,
		pk_record->killer_clan,	pk_record->victim_clan,
		pk_record->level[0],	pk_record->level[1],
		pk_record->pkill_time,	pk_record->pkill_points,
		pk_record->bounty,	pk_record->assist_string );
	}

	for ( pk_record = ch->pcdata->death_list; pk_record != NULL; pk_record = pk_record->next )
	{
	    fprintf( fp, "DRCRD %s~ %s~ %s~ %s~ %d %d %ld %d %d %s~\n",
		pk_record->killer_name,	pk_record->victim_name,
		pk_record->killer_clan,	pk_record->victim_clan,
		pk_record->level[0],	pk_record->level[1],
		pk_record->pkill_time,	pk_record->pkill_points,
		pk_record->bounty,	pk_record->assist_string );
	}

	for ( pos = 0; pos < MAX_FORGET; pos++ )
	{
	    if ( ch->pcdata->forget[pos] == NULL )
		break;

	    fprintf( fp, "Forge %s~\n", ch->pcdata->forget[pos] );
	}

	for ( pos = 0; pos < MAX_ALIAS; pos++ )
	{
	    if ( ch->pcdata->alias[pos] == NULL
	    ||   ch->pcdata->alias_sub[pos] == NULL )
		break;

	    fprintf( fp, "Alias %s %s~\n",
		ch->pcdata->alias[pos], ch->pcdata->alias_sub[pos] );
	}

	for ( pos = 0; group_table[pos].name[0] != '\0'; pos++ )
	{
	    if ( ch->pcdata->group_known[pos] )
		fprintf( fp, "Gr '%s'\n", group_table[pos].name );
	}

	for ( paf = ch->affected; paf != NULL; paf = paf->next )
	{
	    if ( paf->type < 0 || paf->type >= maxSkill )
		continue;

	    fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %3d %10d\n",
		skill_table[paf->type].name,
		paf->where, paf->level, paf->dur_type,
		paf->duration, paf->modifier, paf->location, paf->bitvector );
	}
    }

    fprintf( fp, "End\n\n" );
    return;
}

/* write a pet */
void fwrite_pet( CHAR_DATA *pet, FILE *fp)
{
    AFFECT_DATA *paf;
    
    fprintf(fp,"#PET\n");
    
    fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);
    
    fprintf(fp,"Name %s~\n", pet->name);
    fprintf(fp,"LogO %ld\n", current_time);
    if (pet->short_descr != pet->pIndexData->short_descr)
    	fprintf(fp,"ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
    	fprintf(fp,"LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
    	fprintf(fp,"Desc %s~\n", pet->description);
    if (pet->race != pet->pIndexData->race)
    	fprintf(fp,"Race %s~\n", race_table[pet->race].name);
    if (pet->clan)
        fprintf( fp, "Clan %s~\n",clan_table[pet->clan].name);
    fprintf(fp,"Sex  %d\n", pet->sex);
    if (pet->level != pet->pIndexData->level)
    	fprintf(fp,"Levl %d\n", pet->level);
    fprintf(fp, "HMV  %d %d %d %d %d %d\n",
    	pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
    if (pet->platinum > 0)
    	fprintf(fp,"Plat %d\n",pet->platinum);
    if (pet->gold > 0)
    	fprintf(fp,"Gold %d\n",pet->gold);
    if (pet->silver > 0)
	fprintf(fp,"Silv %d\n",pet->silver);
    if (pet->exp > 0)
    	fprintf(fp, "Exp  %ld\n", pet->exp);
    if (pet->act != pet->pIndexData->act)
    	fprintf(fp, "Act  %s\n", print_flags(pet->act));
    if (pet->affected_by != pet->pIndexData->affected_by)
    	fprintf(fp, "AfBy %s\n", print_flags(pet->affected_by));
    if (pet->shielded_by != pet->pIndexData->shielded_by)
    	fprintf(fp, "ShBy %s\n", print_flags(pet->shielded_by));
    if (pet->comm != 0)
    	fprintf(fp, "Comm %s\n", print_flags(pet->comm));
    fprintf(fp,"Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
    	fprintf(fp, "Save %d\n", pet->saving_throw);
    if (pet->alignment != pet->pIndexData->alignment)
    	fprintf(fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->pIndexData->hitroll)
    	fprintf(fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
    	fprintf(fp, "Dam  %d\n", pet->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n",
    	pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);
    fprintf(fp, "Attr %d %d %d %d %d\n",
    	pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
    	pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
    	pet->perm_stat[STAT_CON] );
    fprintf(fp, "AMod %d %d %d %d %d\n",
    	pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
    	pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
    	pet->mod_stat[STAT_CON] );
    
    for ( paf = pet->affected; paf != NULL; paf = paf->next )
    {
    	if (paf->type < 0 || paf->type >= maxSkill)
    	    continue;
    	    
    	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
    	    skill_table[paf->type].name,
    	    paf->where, paf->level, paf->duration, paf->modifier,paf->location,
    	    paf->bitvector);
    }
    
    fprintf(fp,"End\n\n");
    return;
}
    
void fwrite_value( FILE *fp, OBJ_DATA *obj, int value )
{
    if ( obj->value[value] == obj->pIndexData->value[value] )
	return;

    switch( obj->item_type )
    {
	default:
	    fprintf( fp, "Valu %d %d %d\n",
		value, obj->item_type, obj->value[value] );
	    break;

	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
	    switch( value )
	    {
		default:
		    fprintf( fp, "Valu %d %d %d\n",
			value, obj->item_type, obj->value[value] );
		    break;

		case 2:
		    fprintf( fp, "Valu %d %d '%s'\n",
			value, obj->item_type,
			liq_table[obj->value[value]].liq_name );
		    break;
	    }
	    break;

	case ITEM_CONTAINER:
	    switch( value )
	    {
		default:
		    fprintf( fp, "Valu %d %d %d\n",
			value, obj->item_type, obj->value[value] );
		    break;

		case 1:
		    fprintf( fp, "Valu %d %d %s\n",
			value, obj->item_type,
			print_flags( obj->value[value] ) );
		    break;
	    }
	    break;

	case ITEM_FOOD:
	    switch( value )
	    {
		default:
		    fprintf( fp, "Valu %d %d %d\n",
			value, obj->item_type, obj->value[value] );
		    break;

		case 3:
		    fprintf( fp, "Valu %d %d %s\n",
			value, obj->item_type,
			print_flags( obj->value[value] ) );
		    break;
	    }
	    break;

	case ITEM_PORTAL:
	    switch( value )
	    {
		default:
		    fprintf( fp, "Valu %d %d %d\n",
			value, obj->item_type, obj->value[value] );
		    break;

		case 1:
		case 2:
		    fprintf( fp, "Valu %d %d %s\n",
			value, obj->item_type,
			print_flags( obj->value[value] ) );
		    break;
	    }
	    break;

	case ITEM_FURNITURE:
	    switch( value )
	    {
		default:
		    fprintf( fp, "Valu %d %d %d\n",
			value, obj->item_type, obj->value[value] );
		    break;

		case 2:
		    fprintf( fp, "Valu %d %d %s\n",
			value, obj->item_type,
			print_flags( obj->value[value] ) );
		    break;
	    }
	    break;

	case ITEM_WEAPON:
	    switch( value )
	    {
		default:
		    fprintf( fp, "Valu %d %d %d\n",
			value, obj->item_type, obj->value[value] );
		    break;

		case 0:
		    fprintf( fp, "Valu %d %d %s\n",
			value, obj->item_type,
			flag_string( weapon_class, obj->value[value] ) );
		    break;

		case 3:
		    fprintf( fp, "Valu %d %d %s\n",
			value, obj->item_type,
			attack_table[obj->value[value]].name );
		    break;

		case 4:
		    fprintf( fp, "Valu %d %d %s\n",
			value, obj->item_type,
			print_flags( obj->value[value] ) );
		    break;

	    }
	    break;

	case ITEM_PILL:
	case ITEM_SCROLL:
	case ITEM_POTION:
	    switch( value )
	    {
		default:
		    if ( obj->value[value] > 0 && obj->value[value] < maxSkill )
		    {
			fprintf( fp, "Valu %d %d '%s'\n",
			    value, obj->item_type,
			    skill_table[obj->value[value]].name );
		    }
		    break;

		case 0:
		    fprintf( fp, "Valu %d %d %d\n",
			value, obj->item_type,
			obj->value[value] > 0 ? obj->value[value] : 0 );
		    break;
	    }
	    break;

	case ITEM_TRAP:
	    switch( value )
	    {
		default:
		    fprintf( fp, "Valu %d %d %d\n",
			value, obj->item_type, obj->value[value] );
		    break;

		case 0:
		    fprintf( fp, "Valu %d %d %s\n",
			value, obj->item_type,
			trap_type_table[obj->value[value]].name );
		    break;

		case 1:
		    fprintf( fp, "Valu %d %d %s\n",
			value, obj->item_type,
			damage_mod_table[obj->value[value]].name );
		    break;
	    }
	    break;

	case ITEM_WAND:
	case ITEM_STAFF:
	    switch( value )
	    {
		default:
		    fprintf( fp, "Valu %d %d %d\n",
			value, obj->item_type, obj->value[value] );
		    break;

		case 3:
		    if ( obj->value[value] > 0 && obj->value[value] < maxSkill )
		    {
			fprintf( fp, "Valu %d %d '%s'\n",
			    value, obj->item_type,
			    skill_table[obj->value[value]].name );
		    }
		    break;

	    }
	    break;
    }
}

void fread_value( FILE *fp, OBJ_DATA *obj )
{
    char *word;
    int type, value;

    value		= fread_number( fp );
    type		= fread_number( fp );

    switch( type )
    {
	default:
	    obj->value[value] = fread_number( fp );
	    break;

	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
	    switch( value )
	    {
		default:
		    obj->value[value] = fread_number( fp );
		    break;

		case 2:
		    obj->value[value] = liq_lookup( fread_word( fp ) );
		    break;
	    }
	    break;

	case ITEM_CONTAINER:
	    switch( value )
	    {
		default:
		    obj->value[value] = fread_number( fp );
		    break;

		case 1:
		    obj->value[value] = fread_flag( fp );
		    break;
	    }
	    break;

	case ITEM_FOOD:
	    switch( value )
	    {
		default:
		    obj->value[value] = fread_number( fp );
		    break;

		case 3:
		    obj->value[value] = fread_flag( fp );
		    break;
	    }
	    break;

	case ITEM_PORTAL:
	    switch( value )
	    {
		default:
		    obj->value[value] = fread_number( fp );
		    break;

		case 1:
		case 2:
		    obj->value[value] = fread_flag( fp );
		    break;
	    }
	    break;

	case ITEM_FURNITURE:
	    switch( value )
	    {
		default:
		    obj->value[value] = fread_number( fp );
		    break;

		case 2:
		    obj->value[value] = fread_flag( fp );
		    break;
	    }
	    break;

	case ITEM_WEAPON:
	    switch( value )
	    {
		default:
		    obj->value[value] = fread_number( fp );
		    break;

		case 0:
		    obj->value[value] = weapon_type( fread_word( fp ) );
		    break;

		case 3:
		    obj->value[value] = attack_lookup( fread_word( fp ) );
		    break;

		case 4:
		    obj->value[value] = fread_flag( fp );
		    break;

	    }
	    break;

	case ITEM_PILL:
	case ITEM_SCROLL:
	case ITEM_POTION:
	    switch( value )
	    {
		default:
		    obj->value[value] = skill_lookup( fread_word( fp ) );
		    break;

		case 0:
		    obj->value[value] = fread_number( fp );
		    break;
	    }
	    break;

	case ITEM_TRAP:
	    switch( value )
	    {
		default:
		    obj->value[value] = fread_number( fp );
		    break;

		case 0:
		    word = fread_word( fp );
		    obj->value[value] = URANGE( 0, flag_value( trap_type_table, word ), TRAP_MAX );
		    break;

		case 1:
		    word = fread_word( fp );
		    obj->value[value] = URANGE( 0, dam_type_lookup( word ), DAM_MAX );
		    break;
	    }
	    break;

	case ITEM_WAND:
	case ITEM_STAFF:
	    switch( value )
	    {
		default:
		    obj->value[value] = fread_number( fp );
		    break;

		case 3:
		    obj->value[value] = skill_lookup( fread_word( fp ) );
		    break;

	    }
	    break;
    }
}

void fwrite_obj( OBJ_DATA *obj, FILE *fp, int iNest, int storage, bool special )
{
    AFFECT_DATA *paf;
    EXTRA_DESCR_DATA *ed;
    OBJ_MULTI_DATA *mult;
    sh_int pos;

    if ( obj->next_content != NULL 
    &&   ( !special || obj->next_content->in_room == NULL ) )
	fwrite_obj( obj->next_content, fp, iNest, storage, special );

    if ( storage == -1 )
	fprintf( fp, "#O\n" );
    else
	fprintf( fp, "#S %d\n", storage );

    fprintf( fp, "Vnum %d\n", obj->pIndexData->vnum );

    if ( obj->in_room != NULL )
	fprintf( fp, "Room %d\n", obj->in_room->vnum );

    if ( obj->enchanted )
	fprintf( fp, "Enchanted\n" );

    if ( !special || storage == -1 )
	fprintf( fp, "Nest %d\n", iNest );

    if ( obj->looted_items > 0 )
	fprintf( fp, "Loot %d\n", obj->looted_items );

    if ( obj->name != obj->pIndexData->name )
	fprintf( fp, "Name %s~\n", obj->name );

    if ( obj->short_descr != obj->pIndexData->short_descr )
	fprintf( fp, "ShD  %s~\n", obj->short_descr );

    if ( obj->description != obj->pIndexData->description )
	fprintf( fp, "Desc %s~\n", obj->description );

    for ( mult = obj->multi_data; mult != NULL; mult = mult->next )
	fprintf( fp, "MULTI %s~ %s~ %d\n",
	    mult->dropper, mult->socket, mult->drop_timer );

    if ( obj->owner != NULL )
	fprintf( fp, "Owner %s~\n", obj->owner );

    if ( obj->killer != NULL )
	fprintf( fp, "Killer %s~\n", obj->killer );

    if ( obj->loader != NULL )
	fprintf( fp, "Loader %s~\n", obj->loader );

    if ( obj->extra_flags != obj->pIndexData->extra_flags )
	fprintf( fp, "ExtF %s\n", print_flags( obj->extra_flags ) );

    if ( obj->wear_flags != obj->pIndexData->wear_flags )
	fprintf( fp, "WeaF %s\n", print_flags( obj->wear_flags ) );

    if ( obj->item_type != obj->pIndexData->item_type )
	fprintf( fp, "Ityp %d\n", obj->item_type );

    if ( obj->weight != obj->pIndexData->weight )
	fprintf( fp, "Wt   %d\n", obj->weight );

    if ( obj->level != obj->pIndexData->level )
	fprintf( fp, "Lev  %d\n", obj->level );

    if ( obj->cost != obj->pIndexData->cost )
	fprintf( fp, "Cost %d\n", obj->cost );

    if ( obj->wear_loc != WEAR_NONE )
	fprintf( fp, "Wear %d\n", obj->wear_loc );

    if ( obj->timer != 0 )
	fprintf( fp, "Time %d\n", obj->timer );

    for ( pos = 0; pos < 5; pos++ )
	fwrite_value( fp, obj, pos );

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type < 0 || paf->type >= maxSkill )
	    continue;

	fprintf( fp, "Affc '%s' %d %d %d %d %d %d\n",
	    skill_table[paf->type].name, paf->where, paf->level,
	    paf->duration, paf->modifier, paf->location, paf->bitvector );
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	fprintf( fp, "ExDe %s~ %s~\n", ed->keyword, ed->description );

    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
	fwrite_obj( obj->contains, fp, iNest + 1, storage, special );
}

void save_char_obj( CHAR_DATA *ch, int type )
{
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;
    int pos;
    extern int port;

    if ( IS_NPC(ch) )
	return;

    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    if ( ch->level < 2 && class_table[ch->class].tier == 1 && type != 1 )
	return;

    if ( IS_IMMORTAL(ch) || ch->level >= LEVEL_IMMORTAL )
    {
	sprintf(strsave, "%s%s",GOD_DIR, capitalize(ch->name));
	if ((fp = fopen(strsave,"w")) == NULL)
	{
	    bug("Save_char_obj: fopen",0);
	    perror(strsave);
 	}

	fprintf(fp,"Lev %2d Trust %2d  %s%s\n",
	    ch->level, get_trust(ch), ch->name, ch->pcdata->title);
	fclose( fp );
    }

    if ( type == 1 )
	sprintf( strsave, "%sarena/%s", PLAYER_DIR, capitalize(ch->name) );
    else if ( type == 3 )
	sprintf( strsave, "%s%s/%s", BACKUP_DIR, initial( ch->name ),
	    capitalize( ch->name ) );
    else
    {
	if ( port == MAIN_GAME_PORT )
	    sprintf( strsave, "%s%s/%s", PLAYER_DIR, initial( ch->name ),
		capitalize( ch->name ) );
	else
	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
    }

    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Save_char_obj: fopen", 0 );
	perror( strsave );
    } else {
	fwrite_char( ch, fp, type == 2 ? TRUE : FALSE );
	if ( ch->carrying != NULL )
	    fwrite_obj( ch->carrying, fp, 0, -1, FALSE );
	for ( pos = 0; pos < MAX_BANK; pos++ )
	{
	    if ( ch->pcdata->storage_list[pos] != NULL )
		fwrite_obj( ch->pcdata->storage_list[pos], fp, 0, pos, FALSE );
	}
	if (ch->pet != NULL )
	    fwrite_pet(ch->pet,fp);
	fprintf( fp, "#END\n" );
    }
    fclose( fp );
    rename( TEMP_FILE, strsave );
    return;
}

// 0 = NORMAL, 1 = ARENA, 2 = GET_STATS_ONLY
void fread_char( CHAR_DATA *ch, FILE *fp, sh_int type )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    sh_int pos;
    int count = 0, fcount = 0, lastlogoff = current_time;

    if ( type == 0 && str_cmp( ch->name, "" ) )
    {
	sprintf( buf, "Loading %s.", ch->name );
	log_string( buf );
    }

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER( word[0] ) )
	{
	    case '*':
		fMatch = TRUE;
		fread_to_eol( fp );
		break;

	    case '#':
		if ( !str_cmp( word, "#STAT_BREAK" ) )
		{
		    if ( type == 2 )
			return;

		    fMatch = TRUE;
		}
		break;

	    case 'A':
		KEY( "Act",	ch->act,		fread_flag( fp ) );
		KEY( "AfBy",	ch->affected_by,	fread_flag( fp ) );
		KEY( "Alig",	ch->alignment,		fread_number( fp ) );
		KEY( "Arwn",	ch->pcdata->arenawins,	fread_number( fp ) );
		KEY( "Arls",	ch->pcdata->arenaloss,	fread_number( fp ) );
		KEY( "Arkl",	ch->pcdata->arenakills,	fread_number( fp ) );
		KEY( "Ardt",	ch->pcdata->arenadeath, fread_number( fp ) );
		KEY( "Asst",	ch->pcdata->assist,	fread_number( fp ) );

		if ( !str_cmp( word, "ARoom" ) )
		{
		    ch->pcdata->was_in_room = get_room_index( fread_number( fp ) );
		    fMatch = TRUE;
		    break;
		}

		if ( !str_cmp( word, "Alias" ) )
		{
		    if ( count >= MAX_ALIAS )
		    {
			fread_to_eol( fp );
			fMatch = TRUE;
			break;
		    }

		    ch->pcdata->alias[count]	= str_dup( fread_word( fp ) );
		    ch->pcdata->alias_sub[count]= fread_string( fp );
		    count++;
		    fMatch = TRUE;
		    break;
		}

		if ( !str_cmp( word, "Affc" ) )
		{
		    AFFECT_DATA *paf = new_affect( );
		    int sn;
 
		    if ( ( sn = skill_lookup( fread_word( fp ) ) ) == -1 )
			bug( "Fread_char_affect: unknown skill.", 0 );
		    else
			paf->type = sn;

		    paf->where		= fread_number( fp );
		    paf->level		= fread_number( fp );
		    paf->dur_type	= fread_number( fp );
		    paf->duration	= fread_number( fp );
		    paf->modifier	= fread_number( fp );
		    paf->location	= fread_number( fp );
		    paf->bitvector	= fread_number( fp );
		    paf->next		= ch->affected;
		    ch->affected	= paf;
		    fMatch		= TRUE;
		    break;
		}

		if ( !str_cmp( word, "Attr" ) )
		{
		    for ( pos = 0; pos < MAX_STATS; pos++ )
			ch->perm_stat[pos] = fread_number( fp );
		    fMatch = TRUE;
		    break;
		}
		break;

	    case 'B':
		SKEY( "Bin",	ch->pcdata->bamfin );
		SKEY( "Bout",	ch->pcdata->bamfout );
		KEY( "Bnty",	ch->pcdata->bounty,	fread_number( fp ) );

		if ( !str_cmp( word, "Bank" ) )
		{
		    for ( pos = 0; pos < MAX_BANK; pos++ )
			ch->pcdata->bank_account[pos] = fread_number( fp );
		    fMatch = TRUE;
		    break;
		}
		break;

	    case 'C':
		KEY( "Chat",	ch->pcdata->chat_chan,	fread_number( fp ) );
		KEY( "Color",	ch->pcdata->color,	fread_number( fp ) );
		KEY( "Coauc",	ch->pcdata->color_auc,	fread_number( fp ) );
		KEY( "Cocht",	ch->pcdata->color_cht,	fread_number( fp ) );
		KEY( "Cocgo",	ch->pcdata->color_cgo,	fread_number( fp ) );
		KEY( "Cocla",	ch->pcdata->color_cla,	fread_number( fp ) );
		KEY( "Cocon",	ch->pcdata->color_con,	fread_number( fp ) );
		KEY( "Codis",	ch->pcdata->color_dis,	fread_number( fp ) );
		KEY( "Cofig",	ch->pcdata->color_fig,	fread_number( fp ) );
		KEY( "Cogos",	ch->pcdata->color_gos,	fread_number( fp ) );
		KEY( "Cogra",	ch->pcdata->color_gra,	fread_number( fp ) );
		KEY( "Cogte",	ch->pcdata->color_gte,	fread_number( fp ) );
		KEY( "Coimm",	ch->pcdata->color_imm,	fread_number( fp ) );
		KEY( "Comob",	ch->pcdata->color_mob,	fread_number( fp ) );
		KEY( "Coopp",	ch->pcdata->color_opp,	fread_number( fp ) );
		KEY( "Coqgo",	ch->pcdata->color_qgo,	fread_number( fp ) );
		KEY( "Coque",	ch->pcdata->color_que,	fread_number( fp ) );
		KEY( "Coquo",	ch->pcdata->color_quo,	fread_number( fp ) );
		KEY( "Coroo",	ch->pcdata->color_roo,	fread_number( fp ) );
		KEY( "Cosay",	ch->pcdata->color_say,	fread_number( fp ) );
		KEY( "Cosho",	ch->pcdata->color_sho,	fread_number( fp ) );
		KEY( "Cotel",	ch->pcdata->color_tel,	fread_number( fp ) );
		KEY( "Cowit",	ch->pcdata->color_wit,	fread_number( fp ) );
		KEY( "Cowiz",	ch->pcdata->color_wiz,	fread_number( fp ) );
		KEY( "Coooc",	ch->pcdata->color_ooc,	fread_number( fp ) );
		KEY( "Corac",	ch->pcdata->color_rac,	fread_number( fp ) );
		KEY( "Cobit",	ch->pcdata->color_fla,	fread_number( fp ) );
		KEY( "Coher",	ch->pcdata->color_her,	fread_number( fp ) );
		KEY( "Coic",	ch->pcdata->color_ic,	fread_number( fp ) );
		KEY( "Copra",	ch->pcdata->color_pra,	fread_number( fp ) );
		KEY( "CoOLC1",	ch->pcdata->color_olc1,	fread_number( fp ) );
		KEY( "CoOLC2",	ch->pcdata->color_olc2,	fread_number( fp ) );
		KEY( "CoOLC3",	ch->pcdata->color_olc3,	fread_number( fp ) );

		if ( !str_cmp( word, "Clan" ) )
		{
		    char *tmp = fread_string( fp );

		    ch->clan = clan_lookup( tmp );
		    ch->pcdata->clan_rank = fread_number( fp );
		    free_string( tmp );
		    fMatch = TRUE;
		    break;
		}

		if ( !str_cmp( word, "Clas" ) )
		{
		    char *tmp = fread_string( fp );

		    ch->class = UMAX( 0, class_lookup( tmp ) );
		    free_string( tmp );
		    fMatch = TRUE;
		    break;
		}

		if ( !str_cmp( word, "Cnd" ) )
		{
		    ch->pcdata->condition[0] = fread_number( fp );
		    ch->pcdata->condition[1] = fread_number( fp );
		    ch->pcdata->condition[2] = fread_number( fp );
		    ch->pcdata->condition[3] = fread_number( fp );
		    fMatch = TRUE;
		    break;
		}

		KEY( "Comm",	ch->comm,		fread_flag( fp ) ); 
		KEY( "Config",	ch->configure,		fread_flag( fp ) );
		KEY( "Combat",	ch->combat,		fread_flag( fp ) );
		break;

	    case 'D':
		SKEY( "Desc",	ch->description );
		KEY( "Dtim",	ch->pcdata->dtimer,	fread_number( fp ) );

		if ( !str_cmp( word, "Devote" ) )
		{
		    word = fread_word( fp );
		    pos = devote_lookup( word );
		    if ( pos == -1 )
			ch->pcdata->devote[DEVOTE_CURRENT] = fread_number( fp );
		    else
			ch->pcdata->devote[pos] = fread_number( fp );
		    fMatch = TRUE;
		    break;
		}

		if ( !str_cmp( word, "Deviant" ) )
		{
		    ch->pcdata->deviant_points[0]	= fread_number( fp );
		    ch->pcdata->deviant_points[1]	= fread_number( fp );
		    fMatch = TRUE;
		    break;
		}

		if ( !str_cmp( word, "DRCRD" ) )
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

		    if ( ch->pcdata->death_list == NULL )
			ch->pcdata->death_list = pk_record;
		    else
		    {
			for ( pk_list = ch->pcdata->death_list; pk_list != NULL; pk_list = pk_list->next )
			{
			    if ( pk_list->next == NULL )
			    {
				pk_list->next	= pk_record;
				break;
			    }
			}
		    }

		    fMatch = TRUE;
		    break;
		}
		break;

	    case 'E':
		KEY( "Exp",	ch->exp,		fread_number( fp ) );
		if ( !str_cmp( word, "End" ) )
		{
		    return;
		}
		break;

	    case 'F':
		if ( !str_cmp( word, "Forge" ) )
		{
		    if ( fcount >= MAX_FORGET )
		    {
			fread_to_eol( fp );
			fMatch = TRUE;
			break;
		    }

		    ch->pcdata->forget[fcount] = fread_string( fp );
		    fcount++;
		    fMatch = TRUE;
		}
		break;

	    case 'G':
		KEY( "Ghos",	ch->ghost_level,	fread_number( fp ) );
		KEY( "Gold",	ch->gold,		fread_number( fp ) );

		if ( !str_cmp( word, "GRANT" ) )
		{
		    GRANT_DATA *grant	= new_grant( );
		    grant->command	= fread_string( fp );
		    grant->granter	= fread_string( fp );
		    grant->next		= ch->pcdata->grants;
		    ch->pcdata->grants	= grant;
		    fMatch		= TRUE;
		    break;
		}

		if ( !str_cmp( word, "Gr" ) )
		{
		    char *temp = fread_word( fp ) ;
		    int gn = group_lookup( temp );

		    if ( gn < 0 )
		    {
			sprintf( log_buf, "Fread_char: unknown group: %s.", temp );
			bug( log_buf, 0 );
		    }
		    else
			gn_add( ch, gn );
		    fMatch = TRUE;
		    break;
		}
		break;

	    case 'H':
		if ( !str_cmp( word, "HMVN" ) )
		{
		    ch->hit			= fread_number( fp );
		    ch->pcdata->perm_hit	= fread_number( fp );
		    ch->mana			= fread_number( fp );
		    ch->pcdata->perm_mana	= fread_number( fp );
		    ch->move			= fread_number( fp );
		    ch->pcdata->perm_move	= fread_number( fp );
		    fMatch			= TRUE;
		    break;
		}
		break;

	    case 'I':
		KEY( "Id",	ch->id,			fread_number( fp ) );
		SKEY( "Iden",	ch->pcdata->identity );
		KEY( "Inco",	ch->incog_level,	fread_number( fp ) );
		KEY( "Invi",	ch->invis_level,	fread_number( fp ) );
		KEY( "Info",	ch->info,		fread_flag( fp ) );
		break;

	    case 'K':
		if ( !str_cmp( word, "KRCRD" ) )
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

		    if ( ch->pcdata->kills_list == NULL )
			ch->pcdata->kills_list = pk_record;
		    else
		    {
			for ( pk_list = ch->pcdata->kills_list; pk_list != NULL; pk_list = pk_list->next )
			{
			    if ( pk_list->next == NULL )
			    {
				pk_list->next	= pk_record;
				break;
			    }
			}
		    }

		    fMatch = TRUE;
		    break;
		}
		break;

	    case 'L':
		KEY( "Lag",	ch->pcdata->lag,	fread_number( fp ) );
		KEY( "LLev",	ch->pcdata->last_level, fread_number( fp ) );
		KEY( "Levl",	ch->level,		fread_number( fp ) );

		if ( !str_cmp( word, "LogO" ) )
		{
		    lastlogoff		= fread_number( fp );
		    ch->pcdata->llogoff	= (time_t) lastlogoff;
		    fMatch = TRUE;
		    break;
		}
		break;

	    case 'M':
		KEY( "MaxDam",	ch->pcdata->max_damage,	fread_number( fp ) );
		KEY( "Mdea",	ch->pcdata->mobdeath,   fread_number( fp ) );
		KEY( "Mkil",	ch->pcdata->mobkills,   fread_number( fp ) );

		if ( !str_cmp( word, "MaxS" ) )
		{
		    for ( pos = 0; pos < MAX_BANK; pos++ )
			ch->pcdata->max_storage[pos] = fread_number( fp );
		    fMatch = TRUE;
		    break;
		}
		break;

	    case 'N':
		SKEY( "Name",	ch->name );

		if ( !str_cmp( word, "Note" ) )
		{
		    ch->pcdata->last_note		= fread_number( fp );
		    ch->pcdata->last_idea		= fread_number( fp );
		    ch->pcdata->last_penalty		= fread_number( fp );
		    ch->pcdata->last_news		= fread_number( fp );
		    ch->pcdata->last_changes		= fread_number( fp );
		    fMatch = TRUE;
		    break;
		}
		break;

	    case 'P':
		SKEY( "Pass",	ch->pcdata->pwd				   );
		KEY( "Plat",	ch->platinum,		fread_number( fp ) );
		KEY( "Plyd",	ch->pcdata->played,	fread_number( fp ) );
		KEY( "Pnts",	ch->pcdata->points,	fread_number( fp ) );
		KEY( "Pos",	ch->position,		fread_number( fp ) );
		KEY( "Prac",	ch->pcdata->practice,	fread_number( fp ) );
 		SKEY( "Prom",	ch->prompt				   );
		SKEY( "Ptit",	ch->pcdata->pretitle			   );
		KEY( "Pkpt",	ch->pcdata->pkpoints,	fread_number( fp ) );
		KEY( "Pkil",	ch->pcdata->pkills,	fread_number( fp ) );
		KEY( "Pdea",	ch->pcdata->pdeath,	fread_number( fp ) );

		if ( !str_cmp( word, "Penalty" ) )
		{
		    sh_int dur = fread_number( fp );

		    pos = 0;
		    while( dur != -2 )
		    {
			if ( pos < PENALTY_MAX )
			{
			    ch->pcdata->penalty_time[pos] = dur;
			    pos++;
			}

			dur = fread_number( fp );
		    }
		    fMatch = TRUE;
		    break;
		}

		if ( !str_cmp( word, "PKILL" ) )
		{
		    PKILL_DATA *pkill		= new_pkill( );
		    pkill->time			= fread_number( fp );
		    pkill->killer		= fread_number( fp );
		    pkill->player_name		= fread_string( fp );
		    pkill->next			= ch->pcdata->recent_pkills;
		    ch->pcdata->recent_pkills	= pkill;
		    fMatch			= TRUE;
		    break;
		}
		break;

	    case 'Q':
		KEY( "QuestPnts",ch->pcdata->questpoints,fread_number( fp ) );
		KEY( "QuestNext",ch->pcdata->nextquest,  fread_number( fp ) );
		KEY( "QCount",	ch->pcdata->countdown,	fread_number( fp ) );
		KEY( "QGiv",	ch->pcdata->questgiver,	fread_number( fp ) ); 
		KEY( "QObj",	ch->pcdata->questobj,	fread_number( fp ) );
		KEY( "QMob",	ch->pcdata->questmob,	fread_number( fp ) );

		if ( !str_cmp( word, "QRoom" ) )
		{
		    char *temp = fread_string( fp );

		    ch->pcdata->questroom = temp;
		    free_string( temp );
		    fMatch = TRUE;
		    break;
		}

		if ( !str_cmp( word, "QArea" ) )
		{
		    char *temp = fread_string( fp );

		    ch->pcdata->questarea = temp;
		    free_string( temp );
		    fMatch = TRUE;
		    break;
		}
		break;

	    case 'R':
		KEY( "Reca",	ch->pcdata->recall, fread_number( fp ) );

		if ( !str_cmp( word, "Race" ) )
		{
		    char *tmp = fread_string( fp );

		    ch->race = race_lookup( tmp );
		    free_string( tmp );
		    fMatch = TRUE;
		    break;
		}

		if ( !str_cmp( word, "Room" ) )
		{
		    ch->in_room = get_room_index( fread_number( fp ) );

		    if ( ch->in_room == NULL )
			ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
		    fMatch = TRUE;
		    break;
		}
		break;

	    case 'S':
		KEY( "Scro",	ch->pcdata->lines,	fread_number( fp ) );
		KEY( "ShBy",	ch->shielded_by,	fread_flag( fp )   );
		KEY( "Sec",	ch->pcdata->security,	fread_number( fp ) );
		KEY( "Silv",	ch->silver,		fread_number( fp ) );
		SKEY( "Sock",	ch->pcdata->socket			   );
		KEY( "Soun",	ch->sound,		fread_flag( fp )   );

		if ( !str_cmp( word, "Sk" ) )
		{
		    int sn, value;
		    char *temp;

		    value = fread_number( fp );
		    temp = fread_word( fp ) ;
		    sn = skill_lookup( temp );

		    if ( sn < 0 )
		    {
			sprintf( log_buf, "Fread_char: unknown skill: %s.", temp );
			bug( log_buf, 0 );
		    }
		    else
			ch->learned[sn] = value;
		    fMatch = TRUE;
		}
		break;

	    case 'T':
		KEY( "Tier",	ch->pcdata->tier,	fread_number( fp ) );
		KEY( "TSex",	ch->pcdata->true_sex,   fread_number( fp ) );
		KEY( "Trai",	ch->pcdata->train,	fread_number( fp ) );
		KEY( "Tru",	ch->trust,		fread_number( fp ) );
		KEY( "TotalPnts", ch->pcdata->total_questpoints,	fread_number( fp ) );
		KEY( "TotalAttm", ch->pcdata->total_questattempt,	fread_number( fp ) );
		KEY( "TotalExpr", ch->pcdata->total_questexpire,	fread_number( fp ) );
		KEY( "TotalFail", ch->pcdata->total_questfail,		fread_number( fp ) );
		KEY( "TotalCmpl", ch->pcdata->total_questcomplete,	fread_number( fp ) );

		if ( !str_cmp( word, "Text") )
		{
		    note_attach( ch, fread_number( fp ) );
		    ch->pcdata->pnote->to_list	= fread_string( fp );
		    ch->pcdata->pnote->subject	= fread_string( fp );
		    ch->pcdata->pnote->text	= fread_string( fp );
		    fMatch = TRUE;
		    break;
		}

		if ( !str_cmp( word, "Titl" ) )
		{
		    ch->pcdata->title = fread_string( fp );
		    set_title( ch, ch->pcdata->title );
		    fMatch = TRUE;
		    break;
		}

		if ( !str_cmp( word, "Tells" ) )
		{
		    char *temp;
		    ch->pcdata->tells = fread_number( fp );

		    temp = fread_string( fp );		
		    add_buf( ch->pcdata->buffer, temp );
		    free_string( temp );
		    fMatch = TRUE;
		    break;
		}
		break;

	    case 'W':
		SKEY( "Whod",	ch->pcdata->who_descr			   );
		SKEY( "WhoOut",	ch->pcdata->who_output			   );
		KEY( "Wizn",	ch->wiznet,		fread_flag( fp )   );
		break;
	}

	if ( !fMatch )
	{
	    sprintf( log_buf, "Fread_char: no match: %s.", word );
	    bug( log_buf, 0 );
	    fread_to_eol( fp );
	}
    }
    return;
}

/* load a pet from the forgotten reaches */
void fread_pet( CHAR_DATA *ch, FILE *fp )
{
    char *word;
    CHAR_DATA *pet;
    bool fMatch;
    int lastlogoff = current_time;
    int percent;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "END" : fread_word(fp);
    if (!str_cmp(word,"Vnum"))
    {
    	int vnum;
    	
    	vnum = fread_number(fp);
    	if (get_mob_index(vnum) == NULL)
	{
    	    bug("Fread_pet: bad vnum %d.",vnum);
	    pet = create_mobile(get_mob_index(MOB_VNUM_CORPSE));
	}
    	else
    	    pet = create_mobile(get_mob_index(vnum));
    }
    else
    {
        bug("Fread_pet: no vnum in file.",0);
        pet = create_mobile(get_mob_index(MOB_VNUM_CORPSE));
    }
    
    for ( ; ; )
    {
    	word 	= feof(fp) ? "END" : fread_word(fp);
    	fMatch = FALSE;
    	
    	switch (UPPER(word[0]))
    	{
    	case '*':
    	    fMatch = TRUE;
    	    fread_to_eol(fp);
    	    break;
    		
    	case 'A':
    	    KEY( "Act",		pet->act,		fread_flag(fp));
    	    KEY( "AfBy",	pet->affected_by,	fread_flag(fp));
    	    KEY( "Alig",	pet->alignment,		fread_number(fp));
    	    
    	    if (!str_cmp(word,"ACs"))
    	    {
    	    	int i;
    	    	
    	    	for (i = 0; i < 4; i++)
    	    	    pet->armor[i] = fread_number(fp);
    	    	fMatch = TRUE;
    	    	break;
    	    }

            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_char: unknown skill.",0);
                else
                   paf->type = sn;
 
		paf->where	= fread_number(fp);
                paf->level      = fread_number(fp);
		paf->dur_type	= DUR_TICKS;
                paf->duration   = fread_number(fp);
                paf->modifier   = fread_number(fp);
                paf->location   = fread_number(fp);
                paf->bitvector  = fread_number(fp);
                paf->next       = pet->affected;
                pet->affected   = paf;
                fMatch          = TRUE;
                break;
            }
    	     
    	    if (!str_cmp(word,"AMod"))
    	    {
    	     	int stat;
    	     	
    	     	for (stat = 0; stat < MAX_STATS; stat++)
    	     	    pet->mod_stat[stat] = fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	    }
    	     
    	    if (!str_cmp(word,"Attr"))
    	    {
    	         int stat;
    	         
    	         for (stat = 0; stat < MAX_STATS; stat++)
    	             pet->perm_stat[stat] = fread_number(fp);
    	         fMatch = TRUE;
    	         break;
    	    }
    	    break;
    	     
    	 case 'C':
    	     KEY( "Comm",	pet->comm,		fread_flag(fp));

	     if ( !str_cmp( word, "Clan" ) )
	     {
		char *tmp = fread_string(fp);

		pet->clan = clan_lookup(tmp);
		free_string(tmp);
		fMatch = TRUE;
		break;
	     }
    	     break;
    	     
    	 case 'D':
    	     KEY( "Dam",	pet->damroll,		fread_number(fp));
    	     KEY( "Desc",	pet->description,	fread_string(fp));
    	     break;
    	     
    	 case 'E':
    	     if (!str_cmp(word,"End"))
	     {
		pet->leader = ch;
		pet->master = ch;
		ch->pet = pet;
    		/* adjust hp mana move up  -- here for speed's sake */
    		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);
 
    		if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
    		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
    		{
		    percent = UMIN(percent,100);
    		    pet->hit	+= (pet->max_hit - pet->hit) * percent / 100;
        	    pet->mana   += (pet->max_mana - pet->mana) * percent / 100;
        	    pet->move   += (pet->max_move - pet->move)* percent / 100;
    		}
    	     	return;
	     }
    	     KEY( "Exp",	pet->exp,		fread_number(fp));
    	     break;
    	     
    	 case 'G':
    	     KEY( "Gold",	pet->gold,		fread_number(fp));
    	     break;
    	     
    	 case 'H':
    	     KEY( "Hit",	pet->hitroll,		fread_number(fp));
    	     
    	     if (!str_cmp(word,"HMV"))
    	     {
    	     	pet->hit	= fread_number(fp);
    	     	pet->max_hit	= fread_number(fp);
    	     	pet->mana	= fread_number(fp);
    	     	pet->max_mana	= fread_number(fp);
    	     	pet->move	= fread_number(fp);
    	     	pet->max_move	= fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	     }
    	     break;
    	     
     	case 'L':
    	     KEY( "Levl",	pet->level,		fread_number(fp));
    	     KEY( "LnD",	pet->long_descr,	fread_string(fp));
	     KEY( "LogO",	lastlogoff,		fread_number(fp));
    	     break;
    	     
    	case 'N':
    	    SKEY( "Name",	pet->name				);
    	     break;
    	     
    	case 'P':
    	     KEY( "Plat",	pet->platinum,		fread_number(fp));
    	     KEY( "Pos",	pet->position,		fread_number(fp));
    	     break;
    	     
	case 'R':
	    if ( !str_cmp( word, "Race" ) )
	    {
		char *tmp = fread_string(fp);

		pet->race = race_lookup(tmp);
		free_string(tmp);
		fMatch = TRUE;
		break;
	    }
    	    break;
 	    
    	case 'S' :
    	    KEY( "Save",	pet->saving_throw,	fread_number(fp));
    	    KEY( "Sex",		pet->sex,		fread_number(fp));
    	    KEY( "ShD",		pet->short_descr,	fread_string(fp));
    	    KEY( "ShBy",	pet->shielded_by,	fread_flag(fp));
            KEY( "Silv",        pet->silver,            fread_number( fp ) );
    	    break;
    	    
    	if ( !fMatch )
    	{
    	    bug("Fread_pet: no match.",0);
    	    fread_to_eol(fp);
    	}
    	
    	}
    }
}

void fread_obj( CHAR_DATA *ch, FILE *fp, bool storage )
{
    OBJ_DATA *obj = NULL;
    char *word;
    int iNest = 0, slot = 0;
    bool fMatch, first = TRUE;
    
    if ( storage )
	slot = fread_number( fp );

    word   = feof( fp ) ? "End" : fread_word( fp );
    if ( !str_cmp( word, "Vnum" ) )
    {
        int vnum;
	first = FALSE;  /* fp will be in right place */
 
        vnum = fread_number( fp );
        if (  get_obj_index( vnum ) == NULL )
	{
            bug( "Fread_obj: bad vnum %d.", vnum );
	    obj = create_object(get_obj_index(OBJ_VNUM_BAG));
	} else {
	    obj = create_object(get_obj_index(vnum));
	    obj->affected = NULL;
	}
//	obj->wear_loc = WEAR_NONE;
    }

    if (obj == NULL)  /* either not found or old style */
    {
    	obj = new_obj();
    	obj->name		= str_dup( "" );
    	obj->short_descr	= str_dup( "" );
    	obj->description	= str_dup( "" );
    }

    for ( ; ; )
    {
	if (first)
	    first = FALSE;
	else
	    word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf, *act_paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_obj: unknown skill.",0);
                else
                    paf->type = sn;
 
		paf->where	= fread_number( fp );
                paf->level      = fread_number( fp );
	 	paf->dur_type	= DUR_TICKS;
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->next       = obj->affected;
                obj->affected   = paf;
                fMatch          = TRUE;

		if ( !IS_OBJ_STAT( obj, ITEM_RANDOM_STATS ) )
		{
		    for ( act_paf = obj->pIndexData->affected; act_paf != NULL; act_paf = act_paf->next )
		    {
			if ( act_paf->location == paf->location
			&&   paf->location != APPLY_AC )
			{
			    if ( act_paf->modifier != paf->modifier )
				paf->modifier = act_paf->modifier;
			}
		    }
		}

                break;
            }
	    break;

	case 'C':
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    break;

	case 'D':
	   SKEY( "Desc",	obj->description			   );
	    break;

	case 'E':
	    if ( !str_cmp( word, "Enchanted"))
	    {
		obj->enchanted = TRUE;
	 	fMatch 	= TRUE;
		break;
	    }

	    KEY( "ExtF",	obj->extra_flags,	fread_flag( fp ) );

	    if ( !str_cmp(word,"ExDe") )
	    {
		EXTRA_DESCR_DATA *ed;

		ed = new_extra_descr();

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if ( IS_SET( obj->pIndexData->area->area_flags, AREA_UNLINKED )
		&&   ( ch == NULL || !IS_IMMORTAL( ch ) ) )
		{
		    bug( "Fread_obj: Unlinked vnum %d.",
			obj->pIndexData->vnum );
		    if ( ch != NULL && obj->wear_loc != WEAR_NONE )
			unequip_char( ch, obj );
		    rgObjNest[iNest] = NULL;
		    extract_obj( obj );
		    return;
		}

		if ( iNest == 0 || rgObjNest[iNest-1] == NULL )
		{
		    if ( ch == NULL )
		    {
			if ( obj->last_room != '\0' )
			    obj_to_room( obj, get_room_index( obj->last_room ) );
			else
			    bug( "Fread_obj: NULL LAST ROOM", 0 );
		    } else {
			if ( !storage )
			    obj_to_char( obj, ch );
			else
			{
			    obj->wear_loc = WEAR_NONE;
			    obj->dropped_by = ch;
			    obj->next_content = ch->pcdata->storage_list[slot];
			    ch->pcdata->storage_list[slot] = obj;
			}
		    }
		}
		else
		    obj_to_obj( obj, rgObjNest[iNest-1] );

		return;
	    }
	    break;

	case 'I':
	    KEY( "Ityp",	obj->item_type,		fread_number( fp ) );
	    break;

	case 'K':
	    KEY( "Killer",	obj->killer,		fread_string( fp ) );
	    break;

	case 'L':
	    KEY( "Lev",		obj->level,		fread_number( fp ) );
	    KEY( "Loader",	obj->loader,		fread_string( fp ) );
	    KEY( "Loot",	obj->looted_items,	fread_number( fp ) );
	    break;

	case 'M':
	    if ( !str_cmp( word, "MULTI" ) )
	    {
		OBJ_MULTI_DATA *mult;

		mult			= new_obj_multi( );
		mult->dropper		= fread_string( fp );
		mult->socket		= fread_string( fp );
		mult->drop_timer	= fread_number( fp );
		mult->next		= obj->multi_data;
		obj->multi_data		= mult;
		fMatch			= TRUE;
		break;
	    }
	    break;

	case 'N':
	   SKEY( "Name",	obj->name				   );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		    bug( "Fread_obj: bad nest %d.", iNest );
		else
		    rgObjNest[iNest] = obj;
		fMatch = TRUE;
	    }
	    break;

   	case 'O':
	    KEY( "Owner",	obj->owner,		fread_string( fp ) );

	case 'R':
	    KEY( "Room",	obj->last_room,		fread_number( fp ) );
	    break;

	case 'S':
	   SKEY( "ShD",		obj->short_descr			   );
	    break;

	case 'T':
	    KEY( "Time",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Valu" ) )
	    {
		fread_value( fp, obj );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'W':
	    KEY( "WeaF",	obj->wear_flags,	fread_flag( fp ) );
	    KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Wt",		obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    bug( "Fread_obj: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
}

bool load_char_obj( DESCRIPTOR_DATA *d, char *name, bool arena_load, bool reload )
{
    char strsave[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    extern int port;
    int stat;

    ch = new_char();
    ch->pcdata = new_pcdata();

    d->character			= ch;
    ch->desc				= d;
    ch->name				= str_dup( name );
    ch->id				= get_pc_id();
    ch->race				= race_lookup("human");
    ch->act				= PLR_NOSUMMON;
    ch->comm				= COMM_PROMPT;
    ch->affected_by			= 0;
    ch->shielded_by			= 0;
    ch->size				= 0;
    ch->prompt 				= str_dup("<%Ah %Bm %Cv> [{r%X{x]");

    for ( stat = 0; stat < MAX_STATS; stat++ )
        ch->perm_stat[stat]             = 13;

    found = FALSE;
    
    if (arena_load)
	sprintf( strsave, "%sarena/%s", PLAYER_DIR, capitalize(name) );

    else if (reload)
	sprintf( strsave, "%s%s/%s", BACKUP_DIR, initial( name ),
	    capitalize( name ) );

    else
    {
	if ( port == MAIN_GAME_PORT )
	    sprintf( strsave, "%s%s/%s", PLAYER_DIR, initial( name ),
		capitalize( name ) );
	else
	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
    }

    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
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
	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp, arena_load ? 1 : 0 );
	    else if ( !str_cmp( word, "O"      ) ) fread_obj  ( ch, fp, 0 );
	    else if ( !str_cmp( word, "S"      ) ) fread_obj  ( ch, fp, 1 );
	    else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    if (found)
    {
	int i;

	if ( ch->race == -1 || !race_table[ch->race].pc_race )
	    ch->race = race_lookup("human");

	ch->size += race_table[ch->race].size;

	ch->dam_type = race_table[ch->race].attack;

	if (ch->dam_type == 0)
	    ch->dam_type = 17;

	for ( i = 0; i < MAX_STATS; i++ )
	{
	    stat = get_max_train(ch,i);

	    if ( ch->perm_stat[i] > stat )
		ch->perm_stat[i] = stat;
	}

	for (i = 0; i < 5; i++)
	{
	    if (race_table[ch->race].skills[i] == NULL)
		break;

	    group_add(ch,race_table[ch->race].skills[i],FALSE);
	}

	ch->parts	= race_table[ch->race].parts;
	reset_char( ch );
    }

    if (arena_load)
	unlink(strsave);
    else
	check_roster( ch, FALSE );

    ch->pcdata->tier = class_table[ch->class].tier;

    if ( ch->pcdata->recent_pkills )
    {
	PKILL_DATA *pkill, *pkill_next;

	for ( pkill = ch->pcdata->recent_pkills; pkill != NULL; pkill = pkill_next )
	{
	    pkill_next = pkill->next;

	    if ( difftime( current_time, pkill->time ) > 900 )
	    {
		if ( pkill == ch->pcdata->recent_pkills )
		    ch->pcdata->recent_pkills = pkill->next;
		else
		{
		    PKILL_DATA *list;

		    for ( list = ch->pcdata->recent_pkills; list != NULL; list = list->next )
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

    return found;
}

void save_special_items( void )
{
    FILE *fp;
    OBJ_DATA *obj;
    bool found = FALSE;

    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Save_special_items: can not open file.", 0 );
	return;
    }

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->in_room == NULL )
	    continue;

	if ( ( obj->item_type == ITEM_CORPSE_PC && obj->contains != NULL )
	||   IS_OBJ_STAT( obj, ITEM_SPECIAL_SAVE )
	||   IS_OBJ_STAT( obj, ITEM_AQUEST )
	||   IS_OBJ_STAT( obj, ITEM_FORGED ) )
	{
	    fwrite_obj( obj, fp, 0, -1, TRUE );
	    found = TRUE;
	}
    }

    fprintf( fp, "#END\n" );
    fclose( fp );

    rename( TEMP_FILE, OBJECTS_FILE );

    if ( !found )
	unlink( OBJECTS_FILE );
}

void load_special_items( void )
{
    FILE *fp;
    bool found = FALSE;

    if ( ( fp = fopen( OBJECTS_FILE, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
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
		bug( "Load_special_items: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "O" ) ) fread_obj  ( NULL, fp, FALSE );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_special_items: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    return;
}
